//#include "cpuid.h"
//#include "common.h"
//#include "vmcs.h"
#include "VmxCore.h"
//#include "vmxtraps.h"

//+++++++++++++++++++++Inner Functions++++++++++++++++++++++++

static VOID VmxHandleInterception (
    PCPU Cpu,
    PGUEST_REGS GuestRegs,
    BOOLEAN WillBeAlsoHandledByGuestHv
);

/**
 * effects: Check if the VM Exit trap is valid by <TrappedVmExit> value
 * If <TrappedVmExit> >VMX_MAX_GUEST_VMEXIT(43),return false, otherwise true.
 * requires: a valid <TrappedVmExit>
 */
static BOOLEAN NTAPI PtVmxIsTrapVaild (
  ULONG TrappedVmExit
);

/**
 * effects:	Check if Intel VT Technology is implemented in this CPU
 *			return false if not, otherwise true.
 **/
static BOOLEAN NTAPI PtVmxIsImplemented();

/**
 * effects: Initialize the guest VM with the callback eip and the esp
 */
static NTSTATUS NTAPI PtVmxInitialize (
  PCPU Cpu,
  PVOID GuestEip,//points to the next instruction in the guest os.
  PVOID GuestEsp //points to the guest environment-protection register file.
);
/**
 * effects:启动VMCB块对应的Guest Machine
 */
static NTSTATUS NTAPI PtVmxVirtualize (
  PCPU Cpu
);

/**
 * effects: Build the VMCS struct.
 */
static NTSTATUS VmxSetupVMCS (
    PCPU Cpu,
    PVOID GuestEip,
    PVOID GuestEsp
);

/**
 * VM Exit Event Dispatcher
 * VMExit事件分发逻辑
 */
static VOID NTAPI PtVmxDispatchEvent (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);
/**
 * Adjust Rip
 */
static VOID NTAPI PtVmxAdjustRip (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  ULONG Delta
);

/**
 * Shutdown VM
 * 关闭虚拟机
 */
static NTSTATUS NTAPI PtVmxShutdown (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  BOOLEAN bSetupTimeBomb
);

static VOID NTAPI _PtVmxInitFeatureMSR(
	PVMX Vmx
);

//+++++++++++++++++++++Definitions++++++++++++++++++++++++
ULONG g_HostStackBaseAddress; //4     // FIXME: this is ugly -- we should move it somewhere else
extern ULONG g_uSubvertedCPUs;
extern PMadDog_Control g_HvmControl;

HVM_DEPENDENT Vmx = {
  ARCH_VMX,
  PtVmxIsImplemented,
  PtVmxInitialize,
  PtVmxVirtualize,
  PtVmxShutdown,
  //VmxIsNestedEvent,
  //VmxDispatchNestedEvent,
  PtVmxDispatchEvent,
  PtVmxAdjustRip,
  //VmxRegisterTraps,
  PtVmxIsTrapVaild
};


//+++++++++++++++++++++Implementations++++++++++++++++++++++++
/**
 * effects:	Check if Intel VT Technology is implemented in this CPU
 *			return false if not, otherwise true.
 **/
static BOOLEAN NTAPI PtVmxIsImplemented()
{
	ULONG32 eax, ebx, ecx, edx;
	GetCpuIdInfo (0, &eax, &ebx, &ecx, &edx);
	if (eax < 1) 
	{
		Print(("VmxIsImplemented(): Extended CPUID functions not implemented\n"));
		return FALSE;
	}
	if (!(ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69)) 
	{
		Print(("VmxIsImplemented(): Not an INTEL processor\n"));
		return FALSE;
	}

	// intel cpu use fun_0x1 to test VMX.
	// CPUID.1:ECX.VMX[bit 5] = 1
	GetCpuIdInfo (0x1, &eax, &ebx, &ecx, &edx);
	return (BOOLEAN) (CmIsBitSet (ecx, 5));
}

VOID NTAPI _PtVmxInitFeatureMSR(
	PVMX Vmx
)
{
	//Set Vmx->FeaturesMSR.VmxPinBasedCTLs
	Vmx->FeaturesMSR.VmxPinBasedCTLs.QuadPart = MsrRead(MSR_IA32_VMX_PINBASED_CTLS);

	//Set Vmx->FeaturesMSR.VmxTruePinBasedCTLs
	Vmx->FeaturesMSR.EnableVmxTruePinBasedCTLs = (BOOLEAN)(Vmx->FeaturesMSR.VmxPinBasedCTLs.HighPart & (1<<23));
	if(Vmx->FeaturesMSR.EnableVmxTruePinBasedCTLs)
		Vmx->FeaturesMSR.VmxTruePinBasedCTLs.QuadPart = MsrRead(MSR_IA32_VMX_TRUE_PINBASED_CTLS);
}
/**
 * effects: Initialize the guest VM with the callback eip and the esp
 * 构建Guest VM，传入的<GuestEip>和<GuestEsp>指明了再次进入Guest VM模式下继续执行的指令地址
 * 和堆栈地址。
 */
static NTSTATUS NTAPI PtVmxInitialize (
    PCPU Cpu,
    PVOID GuestEip,//points to the next instruction in the guest os.
    PVOID GuestEsp //points to the guest environment-protection register file.
)
{//Finished
    PHYSICAL_ADDRESS AlignedVmcsPA;
    ULONG VaDelta;
    NTSTATUS Status;
	PALLOCATED_PAGE AllocatedPage;

//#ifndef _X86_
    PVOID tmp = HvMmAllocateContiguousPages (1, NULL,&AllocatedPage);
    g_HostStackBaseAddress = (ULONG) tmp;
	
	#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY
		//MmChangeRequireHidingAllocPage(AllocatedPage,FALSE);
	#endif
//#endif
    // do not deallocate anything here; MmShutdownManager will take care of that

	//Fill the virtual-machine extensions support configuation
	//on the current platform by reading MSR register.
	_PtVmxInitFeatureMSR(&Cpu->Vmx);

    //Allocate VMXON region
    Cpu->Vmx.OriginaVmxonR = HvMmAllocateContiguousPages(
        VMX_VMXONR_SIZE_IN_PAGES, 
        &Cpu->Vmx.OriginalVmxonRPA,
		&AllocatedPage);
    if (!Cpu->Vmx.OriginaVmxonR) 
    {
		Print(("Helloworld:VmxInitialize(): Failed to allocate memory for original VMCS\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Print(("Helloworld:VmxInitialize(): OriginaVmxonR VA: 0x%x\n", Cpu->Vmx.OriginaVmxonR));
    Print(("Helloworld:VmxInitialize(): OriginaVmxonR PA: 0x%llx\n", Cpu->Vmx.OriginalVmxonRPA.QuadPart));
    //Allocate VMCS	
    Cpu->Vmx.OriginalVmcs = HvMmAllocateContiguousPages(
        VMX_VMCS_SIZE_IN_PAGES, 
        &Cpu->Vmx.OriginalVmcsPA,
		&AllocatedPage);
    if (!Cpu->Vmx.OriginalVmcs) 
    {
		Print(("Helloworld:VmxInitialize(): Failed to allocate memory for original VMCS\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Print(("Helloworld:VmxInitialize(): Vmcs VA: 0x%x\n", Cpu->Vmx.OriginalVmcs));
    Print(("Helloworld:VmxInitialize(): Vmcs PA: 0x%llx\n", Cpu->Vmx.OriginalVmcsPA.QuadPart));

    // these two PAs are equal if there're no nested VMs
    Cpu->Vmx.VmcsToContinuePA = Cpu->Vmx.OriginalVmcsPA;

    // init IOBitmap and MsrBitmap
    /*Cpu->Vmx.IOBitmapA = HvMmAllocateContiguousPages (
        VMX_IOBitmap_SIZE_IN_PAGES, 
        &Cpu->Vmx.IOBitmapAPA);
    if (!Cpu->Vmx.IOBitmapA) 
    {
        _KdPrint (("VmxInitialize(): Failed to allocate memory for IOBitmapA\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory (Cpu->Vmx.IOBitmapA, PAGE_SIZE);

    _KdPrint (("VmxInitialize(): IOBitmapA VA: 0x%x\n", Cpu->Vmx.IOBitmapA));
    _KdPrint (("VmxInitialize(): IOBitmapA PA: 0x%llx\n", Cpu->Vmx.IOBitmapAPA.QuadPart));

    Cpu->Vmx.IOBitmapB = HvMmAllocateContiguousPages(
        VMX_IOBitmap_SIZE_IN_PAGES, 
        &Cpu->Vmx.IOBitmapBPA);
    if (!Cpu->Vmx.IOBitmapB) 
    {
        _KdPrint (("VmxInitialize(): Failed to allocate memory for IOBitmapB\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory (Cpu->Vmx.IOBitmapB, PAGE_SIZE);

    _KdPrint (("VmxInitialize(): IOBitmapB VA: 0x%x\n", Cpu->Vmx.IOBitmapB));
    _KdPrint (("VmxInitialize(): IOBitmapB PA: 0x%llx\n", Cpu->Vmx.IOBitmapBPA.QuadPart));*/

    Cpu->Vmx.MSRBitmap = HvMmAllocateContiguousPages(
        VMX_MSRBitmap_SIZE_IN_PAGES, 
        &Cpu->Vmx.MSRBitmapPA,
		&AllocatedPage);
    if (!Cpu->Vmx.MSRBitmap) 
    {
        Print(("VmxInitialize(): Failed to allocate memory for  MSRBitmap\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory (Cpu->Vmx.MSRBitmap, PAGE_SIZE);

    Print(("VmxInitialize(): MSRBitmap VA: 0x%x\n", Cpu->Vmx.MSRBitmap));
    Print(("VmxInitialize(): MSRBitmap PA: 0x%llx\n", Cpu->Vmx.MSRBitmapPA.QuadPart));

    // call VMXON, should fill the version first
    if (!NT_SUCCESS (VmxEnable (Cpu->Vmx.OriginaVmxonR))) //<----------------4.1 Finished
    {
        Print( ("Helloworld:VmxInitialize(): Failed to enable Vmx\n"));
        return STATUS_UNSUCCESSFUL;
    }

    // version
    *((ULONG64 *)(Cpu->Vmx.OriginalVmcs)) = 
        (MsrRead (MSR_IA32_VMX_BASIC) & 0xffffffff); //set up vmcs_revision_id      

    // fill the VMCS struct
    //Status = VmxSetupVMCS (Cpu, GuestEip, GuestEsp);//<----------------4.2 Finished
	Status = VmxSetupVMCS (Cpu, GuestEip, GuestEsp);//<----------------4.2 Finished
    if (!NT_SUCCESS (Status)) 
    {
        Print(("Helloworld:Vmx(): VmxSetupVMCS() failed with status 0x%08hX\n", Status));
        VmxDisable();
        return Status;
    }

    Print(("Helloworld:VmxInitialize(): Vmx enabled\n"));

    Cpu->Vmx.GuestEFER = MsrRead (MSR_EFER);
    Print(("Helloworld:Guest MSR_EFER Read 0x%llx \n", Cpu->Vmx.GuestEFER));

    Cpu->Vmx.GuestCR0 = RegGetCr0 ();
    Cpu->Vmx.GuestCR3 = RegGetCr3 ();
    Cpu->Vmx.GuestCR4 = RegGetCr4 ();

    CmCli ();
    return STATUS_SUCCESS;
}
/**
 * effects:启动VMCB块对应的Guest Machine
 */
static NTSTATUS NTAPI PtVmxVirtualize (
  PCPU Cpu
)
{//Finished
    ULONG esp;
    if (!Cpu)
        return STATUS_INVALID_PARAMETER;

    Print(("VmxVirtualize(): VmxRead: 0x%X \n", VmxRead (VM_INSTRUCTION_ERROR)));
    Print(("VmxVirtualize(): EFlags before vmxLaunch: 0x%x \n", RegGetRflags ()));
    Print(("VmxVirtualize(): PCPU: 0x%x \n", Cpu));
    esp = RegGetEsp ();
    Print(("VmxVirtualize(): Rsp: 0x%x \n", esp));

//#ifndef _X86_
    *((PULONG) (g_HostStackBaseAddress + 0x0C00)) = (ULONG) Cpu;
//#endif

	#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY
		//MmHidingStrategyHideGuestPages((PVOID)g_HostStackBaseAddress,1);
		MmHidingStrategyHideAllAllocatedGuestPages();
	#endif
	
    //VmxDumpVmcs();
    VmxLaunch ();

    // never returns

    return STATUS_UNSUCCESSFUL;
}


/**
 * effects: Check if the VM Exit trap is valid by <TrappedVmExit> value
 * If <TrappedVmExit> >VMX_MAX_GUEST_VMEXIT(43),return false, otherwise true.
 * requires: a valid <TrappedVmExit>
 */
static BOOLEAN NTAPI PtVmxIsTrapVaild (
  ULONG TrappedVmExit
)//Finished
{
  if (TrappedVmExit > VMX_MAX_GUEST_VMEXIT)
    return FALSE;
  return TRUE;
}

/**
 * effects: Enable the VMX and turn on the VMX
 * thus we are in the VM Root from now on (on this processor).
 */
NTSTATUS NTAPI VmxEnable (
    PVOID VmxonVA
)
{//Finished
    ULONG cr4;
    ULONG64 vmxmsr;//ULONG32
    ULONG flags;
    PHYSICAL_ADDRESS VmxonPA;

    // set cr4, enable vmx
	// 设置cr4位，为启用VM模式做准备
    set_in_cr4 (X86_CR4_VMXE);
    cr4 = get_cr4 ();
    Print(("Helloworld:VmxEnable(): CR4 after VmxEnable: 0x%llx\n", cr4));
    if (!(cr4 & X86_CR4_VMXE))
        return STATUS_NOT_SUPPORTED;

    // check msr(3ah) bit2
    vmxmsr = MsrRead (MSR_IA32_FEATURE_CONTROL);
    if (!(vmxmsr & 4)) 
    {
        Print(("Helloworld:VmxEnable(): VMX is not supported: IA32_FEATURE_CONTROL is 0x%llx\n", vmxmsr));
        return STATUS_NOT_SUPPORTED;
    }

    vmxmsr = MsrRead (MSR_IA32_VMX_BASIC);
    *((ULONG64 *) VmxonVA) = (vmxmsr & 0xffffffff);       //set up vmcs_revision_id
    VmxonPA = MmGetPhysicalAddress (VmxonVA);
    Print(("Helloworld:VmxEnable(): VmxonPA:  0x%llx\n", VmxonPA.QuadPart));
    //VmxTurnOn (MmGetPhysicalAddress (VmxonVA));
	VmxTurnOn(VmxonPA);
    flags = RegGetRflags ();
    Print(("Helloworld:VmxEnable(): vmcs_revision_id: 0x%x  Eflags: 0x%x \n", vmxmsr, flags));

	//Step Final. Update bCurrentMachineState
	//bCurrentMachineState = CURRENT_STATE_HYPERVISOR;
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI VmxDisable (
)
{
    ULONG cr4;
    VmxTurnOff ();
    cr4 = get_cr4 ();
    clear_in_cr4 (X86_CR4_VMXE);
    cr4 = get_cr4 ();
    Print(("VmxDisable(): CR4 after VmxDisable: 0x%llx\n", cr4));
    return STATUS_SUCCESS;
}

/**
 * effects: Build the VMCS struct.
 */
static NTSTATUS VmxSetupVMCS (
    PCPU Cpu,
    PVOID GuestEip,
    PVOID GuestEsp
)
{ //Finished
    //SEGMENT_SELECTOR SegmentSelector;
    PHYSICAL_ADDRESS VmcsToContinuePA;
    NTSTATUS Status;
    //PVOID GdtBase;
    //ULONG32 Interceptions;

    if (!Cpu || !Cpu->Vmx.OriginalVmcs)
        return STATUS_INVALID_PARAMETER;

    VmcsToContinuePA = Cpu->Vmx.VmcsToContinuePA;
    //Step 1. Load the vmcs
    VmxClear (VmcsToContinuePA);
    VmxPtrld (VmcsToContinuePA);
	
	//Step 2. Call user method to fill the VMCB.
	Status = g_HvmControl->SetupVMCB(Cpu,GuestEip,GuestEsp);
	
	//Step 3. Set Key Host Environment Info in the VMCB
    VmxWrite (HOST_RSP, g_HostStackBaseAddress + 0x0C00); //setup host sp at vmxLaunch(...)
    // setup host ip:CmSlipIntoMatrix
    VmxWrite (HOST_RIP, (ULONG) VmxVmexitHandler); //setup host ip:CmSlipIntoMatrix

	Print(("MadDog Framework:VmxSetupVMCS(): Exit\n"));

    return Status;
}

/**
 * VM Exit Event Dispatcher
 * VMExit事件分发逻辑
 */
static VOID NTAPI PtVmxDispatchEvent (
    PCPU Cpu,
    PGUEST_REGS GuestRegs
)
{//Finished
  Print(("VmxDispatchEvent(): exitcode = %x\n", VmxRead (VM_EXIT_REASON)));

  VmxHandleInterception(
      Cpu, 
      GuestRegs, 
      FALSE /* this intercept will not be handled by guest hv */);
}

static VOID VmxGenerateTrampolineToGuest (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PUCHAR Trampoline,
  BOOLEAN bSetupTimeBomb
)
{
  ULONG uTrampolineSize = 0;
  ULONG NewRsp;

  if (!Cpu || !GuestRegs)
    return;

  // assume Trampoline buffer is big enough


  VmxWrite (GUEST_RFLAGS, VmxRead (GUEST_RFLAGS) & ~0x100);     // disable TF

  if (bSetupTimeBomb) 
  {
  } 
  else 
  {
    CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RCX, GuestRegs->ecx);
    CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RDX, GuestRegs->edx);
  }

  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RBX, GuestRegs->ebx);
  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RBP, GuestRegs->ebp);
  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RSI, GuestRegs->esi);
  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RDI, GuestRegs->edi);

  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_CR0, VmxRead (GUEST_CR0));
  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_CR3, VmxRead (GUEST_CR3));
  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_CR4, VmxRead (GUEST_CR4));

  NewRsp = VmxRead (GUEST_RSP);

  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RSP, NewRsp);

  // construct stack frame for IRETQ:
  // [TOS]        rip
  // [TOS+0x08]   cs
  // [TOS+0x10]   rflags
  // [TOS+0x18]   rsp
  // [TOS+0x20]   ss

  // construct stack frame for IRETD:
  // [TOS]        rip
  // [TOS+0x4]    cs
  // [TOS+0x8]    rflags

  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RAX, VmxRead (GUEST_RFLAGS));
  CmGeneratePushReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RAX);
  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RAX, VmxRead (GUEST_CS_SELECTOR));
  CmGeneratePushReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RAX);

  if (bSetupTimeBomb) 
  {
  } 
  else 
  {
    CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RAX,
                      VmxRead (GUEST_RIP) + VmxRead (VM_EXIT_INSTRUCTION_LEN));
  }

  CmGeneratePushReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RAX);

  CmGenerateMovReg (&Trampoline[uTrampolineSize], &uTrampolineSize, REG_RAX, GuestRegs->eax);

  CmGenerateIretd (&Trampoline[uTrampolineSize], &uTrampolineSize);

  // restore old GDTR
  CmReloadGdtr ((PVOID) VmxRead (GUEST_GDTR_BASE), (ULONG) VmxRead (GUEST_GDTR_LIMIT));

  //MsrWrite (MSR_GS_BASE, VmxRead (GUEST_GS_BASE));
  //MsrWrite (MSR_FS_BASE, VmxRead (GUEST_FS_BASE));

  // FIXME???
  // restore ds, es
  //CmSetDS((USHORT)VmxRead(GUEST_DS_SELECTOR));
  //CmSetES((USHORT)VmxRead(GUEST_ES_SELECTOR));

  // cs and ss must be the same with the guest OS in this implementation

  // restore old IDTR
  CmReloadIdtr ((PVOID) VmxRead (GUEST_IDTR_BASE), (ULONG) VmxRead (GUEST_IDTR_LIMIT));

  return;
}

/**
 * Shutdown VM
 * 关闭虚拟机
 */
static NTSTATUS NTAPI PtVmxShutdown (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  BOOLEAN bSetupTimeBomb
)
{	//Finished
	//UCHAR Trampoline[0x600];
	PWORMHOLE HvGuestPipe;
	HvGuestPipe = Cpu->HypervisorGuestPipe;

	Print(("VmxShutdown(): CPU#%d\n", Cpu->ProcessorNumber));

	#if DEBUG_LEVEL>2
		VmxDumpVmcs ();
	#endif
	InterlockedDecrement (&g_uSubvertedCPUs);

	// The code should be updated to build an approproate trampoline to exit to any guest mode.
	VmxGenerateTrampolineToGuest (Cpu, GuestRegs, HvGuestPipe->Trampoline, bSetupTimeBomb);

	#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY
		//MmHidingStrategyRevealHiddenPages((PVOID)g_HostStackBaseAddress,1);
	#endif
	
	Print(("VmxShutdown(): Trampoline generated\n", Cpu->ProcessorNumber));
	VmxDisable ();
	((VOID (*)()) & (HvGuestPipe->Trampoline)) ();

	// never returns
	return STATUS_UNSUCCESSFUL;
}

/**
 * VMExit事件处理逻辑
 */
static VOID VmxHandleInterception (
    PCPU Cpu,
    PGUEST_REGS GuestRegs,
    BOOLEAN WillBeAlsoHandledByGuestHv
)
{ //Finished
    NTSTATUS Status;
    ULONG Exitcode;
    PNBP_TRAP Trap;

    if (!Cpu || !GuestRegs)
        return;

    Exitcode = VmxRead (VM_EXIT_REASON);

    Print(("VmxHandleInterception(): Exitcode %x\n", Exitcode));

    if (Exitcode == EXIT_REASON_CR_ACCESS
        && GuestRegs->eax == MADDOG_EXIT_EAX)
    {
        // to uninstall
        PtVmxShutdown(Cpu, GuestRegs, FALSE);
    }

    // search for a registered trap for this interception
    Status = TrFindRegisteredTrap (Cpu, GuestRegs, Exitcode, &Trap);//<----------------------1.1 Finished!
    if (!NT_SUCCESS (Status)) 
    {
        Print(("VmxHandleInterception(): TrFindRegisteredTrap() failed for exitcode 0x%llX\n", Exitcode));
        VmxCrash (Cpu, GuestRegs);//<-------------1.2 Finished
        return;
    }

    // we found a trap handler
    Status = TrExecuteGeneralTrapHandler(//<-------------1.3 Finished
        Cpu, 
        GuestRegs, 
        Trap, 
        WillBeAlsoHandledByGuestHv);
    if (!NT_SUCCESS (Status)) 
    {
        Print(("VmxHandleInterception(): HvmExecuteGeneralTrapHandler() failed with status 0x%08hX\n", Status));
    }
}

/**
 * Adjust Rip
 */
static VOID NTAPI PtVmxAdjustRip (
    PCPU Cpu,
    PGUEST_REGS GuestRegs,
    ULONG Delta
)
{ //Finished
    VmxWrite (GUEST_RIP, VmxRead (GUEST_RIP) + Delta);
    return;
}