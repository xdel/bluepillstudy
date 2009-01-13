//#include "cpuid.h"
//#include "common.h"
//#include "vmcs.h"
#include "vmx.h"
#include "vmxtraps.h"

ULONG g_HostStackBaseAddress; //4     // FIXME: this is ugly -- we should move it somewhere else
extern ULONG g_uSubvertedCPUs;

HVM_DEPENDENT Vmx = {
  ARCH_VMX,
  VmxIsImplemented,
  VmxInitialize,
  VmxVirtualize,
  VmxShutdown,
  //VmxIsNestedEvent,
  //VmxDispatchNestedEvent,
  VmxDispatchEvent,
  VmxAdjustRip,
  VmxRegisterTraps,
  VmxIsTrapVaild
};

extern VOID VmxHandleInterception (
    PCPU Cpu,
    PGUEST_REGS GuestRegs,
    BOOLEAN WillBeAlsoHandledByGuestHv
);
/**
 * effects:	Check if Intel VT Technology is implemented in this CPU
 *			return false if not, otherwise true.
 **/
static BOOLEAN NTAPI VmxIsImplemented()
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

/**
 * effects: Initialize the guest VM with the callback eip and the esp
 * 构建Guest VM，传入的<GuestEip>和<GuestEsp>指明了再次进入Guest VM模式下继续执行的指令地址
 * 和堆栈地址。
 */
static NTSTATUS NTAPI VmxInitialize (
    PCPU Cpu,
    PVOID GuestEip,//points to the next instruction in the guest os.
    PVOID GuestEsp //points to the guest environment-protection register file.
)
{//Finished
    PHYSICAL_ADDRESS AlignedVmcsPA;
    ULONG VaDelta;
    NTSTATUS Status;

//#ifndef _X86_
    PVOID tmp = MmAllocateContiguousPages (1, NULL);
    g_HostStackBaseAddress = (ULONG) tmp;
//#endif
    // do not deallocate anything here; MmShutdownManager will take care of that
	
    //Allocate VMXON region
    Cpu->Vmx.OriginaVmxonR = MmAllocateContiguousPages(
        VMX_VMXONR_SIZE_IN_PAGES, 
        &Cpu->Vmx.OriginalVmxonRPA);
    if (!Cpu->Vmx.OriginaVmxonR) 
    {
		Print(("Helloworld:VmxInitialize(): Failed to allocate memory for original VMCS\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Print(("Helloworld:VmxInitialize(): OriginaVmxonR VA: 0x%x\n", Cpu->Vmx.OriginaVmxonR));
    Print(("Helloworld:VmxInitialize(): OriginaVmxonR PA: 0x%llx\n", Cpu->Vmx.OriginalVmxonRPA.QuadPart));
    //Allocate VMCS	
    Cpu->Vmx.OriginalVmcs = MmAllocateContiguousPages(
        VMX_VMCS_SIZE_IN_PAGES, 
        &Cpu->Vmx.OriginalVmcsPA);
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
    /*Cpu->Vmx.IOBitmapA = MmAllocateContiguousPages (
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

    Cpu->Vmx.IOBitmapB = MmAllocateContiguousPages(
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

    Cpu->Vmx.MSRBitmap = MmAllocateContiguousPages(
        VMX_MSRBitmap_SIZE_IN_PAGES, 
        &Cpu->Vmx.MSRBitmapPA);
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
static NTSTATUS NTAPI VmxVirtualize (
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
    VmxDumpVmcs();
    VmxLaunch ();

    // never returns

    return STATUS_UNSUCCESSFUL;
}


/**
 * effects: Check if the VM Exit trap is valid by <TrappedVmExit> value
 * If <TrappedVmExit> >VMX_MAX_GUEST_VMEXIT(43),return false, otherwise true.
 * requires: a valid <TrappedVmExit>
 */
static BOOLEAN NTAPI VmxIsTrapVaild (
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
    SEGMENT_SELECTOR SegmentSelector;
    PHYSICAL_ADDRESS VmcsToContinuePA;
    NTSTATUS Status;
    PVOID GdtBase;
    ULONG32 Interceptions;

    if (!Cpu || !Cpu->Vmx.OriginalVmcs)
        return STATUS_INVALID_PARAMETER;

    VmcsToContinuePA = Cpu->Vmx.VmcsToContinuePA;
    // load the vmcs
    VmxClear (VmcsToContinuePA);
    VmxPtrld (VmcsToContinuePA);

    /*16BIT Fields */

    /*16BIT Host-Statel Fields. */
    VmxWrite (HOST_ES_SELECTOR, RegGetEs () & 0xf8);
    VmxWrite (HOST_CS_SELECTOR, RegGetCs () & 0xf8);
    VmxWrite (HOST_SS_SELECTOR, RegGetSs () & 0xf8);
    VmxWrite (HOST_DS_SELECTOR, RegGetDs () & 0xf8);

    VmxWrite (HOST_FS_SELECTOR, (RegGetFs () & 0xf8));
    VmxWrite (HOST_GS_SELECTOR, (RegGetGs () & 0xf8));
    VmxWrite (HOST_TR_SELECTOR, (GetTrSelector () & 0xf8));

    ///*64BIT Control Fields. */
    //VmxWrite (IO_BITMAP_A, Cpu->Vmx.IOBitmapAPA.LowPart);
    //VmxWrite (IO_BITMAP_A_HIGH, Cpu->Vmx.IOBitmapBPA.HighPart);
    //VmxWrite (IO_BITMAP_B, Cpu->Vmx.IOBitmapBPA.LowPart);

    // FIXME???
    //*(((unsigned char*)(Cpu->Vmx.IOBitmapB))+((0xc880-0x8000)/8))=0xff;  //0xc880-0xc887  

    //VmxWrite (IO_BITMAP_B_HIGH, Cpu->Vmx.IOBitmapBPA.HighPart);

    //VmxWrite (MSR_BITMAP, Cpu->Vmx.MSRBitmapPA.LowPart);
    //VmxWrite (MSR_BITMAP_HIGH, Cpu->Vmx.MSRBitmapPA.HighPart);

    //VM_EXIT_MSR_STORE_ADDR          = 0x00002006,  //no init
    //VM_EXIT_MSR_STORE_ADDR_HIGH     = 0x00002007,  //no init
    //VM_EXIT_MSR_LOAD_ADDR           = 0x00002008,  //no init
    //VM_EXIT_MSR_LOAD_ADDR_HIGH      = 0x00002009,  //no init
    //VM_ENTRY_MSR_LOAD_ADDR          = 0x0000200a,  //no init
    //VM_ENTRY_MSR_LOAD_ADDR_HIGH     = 0x0000200b,  //no init

   /* VmxWrite (TSC_OFFSET, 0);
    VmxWrite (TSC_OFFSET_HIGH, 0);*/

    //VIRTUAL_APIC_PAGE_ADDR          = 0x00002012,   //no init
    //VIRTUAL_APIC_PAGE_ADDR_HIGH     = 0x00002013,   //no init

    /*64BIT Guest-State Fields. */
    VmxWrite (VMCS_LINK_POINTER, 0xffffffff);
    VmxWrite (VMCS_LINK_POINTER_HIGH, 0xffffffff);

    VmxWrite (GUEST_IA32_DEBUGCTL, MsrRead (MSR_IA32_DEBUGCTL) & 0xffffffff);
    VmxWrite (GUEST_IA32_DEBUGCTL_HIGH, MsrRead (MSR_IA32_DEBUGCTL) >> 32);

    /*32BIT Control Fields. */
    //disable Vmexit by Extern-interrupt,NMI and Virtual NMI
    // Pin-based VM-execution controls
    VmxWrite (PIN_BASED_VM_EXEC_CONTROL, VmxAdjustControls (0, MSR_IA32_VMX_PINBASED_CTLS));//<------------------5.1 Finished

    Interceptions = 0;
    // if MSR_BITMAP is not enable, rdmsr/wrmsr always cause vm exit
#ifdef VMX_ENABLE_MSR_BITMAP
    Interceptions |= CPU_BASED_ACTIVATE_MSR_BITMAP;
#endif
    // Primary processor-based VM-execution controls
    VmxWrite (CPU_BASED_VM_EXEC_CONTROL, VmxAdjustControls (Interceptions, MSR_IA32_VMX_PROCBASED_CTLS));

#ifdef INTERCEPT_PAGEFAULT
    VmxWrite (EXCEPTION_BITMAP, (ULONG)1 << 14);  // intercept #PF
#endif
    //VmxWrite (PAGE_FAULT_ERROR_CODE_MASK, 2);   // W/R
    //VmxWrite (PAGE_FAULT_ERROR_CODE_MATCH, 2);  // write cause the fault
    VmxWrite (PAGE_FAULT_ERROR_CODE_MASK, 0);
    VmxWrite (PAGE_FAULT_ERROR_CODE_MATCH, 0xFFFFFFFF);

    VmxWrite (CR3_TARGET_COUNT, 0);

    // VM-exit controls
    // bit 15, Acknowledge interrupt on exit
    VmxWrite (VM_EXIT_CONTROLS, 
        VmxAdjustControls (VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS));
    // VM-entry controls
    VmxWrite (VM_ENTRY_CONTROLS, 
        VmxAdjustControls (0, MSR_IA32_VMX_ENTRY_CTLS));

    VmxWrite (VM_EXIT_MSR_STORE_COUNT, 0);
    VmxWrite (VM_EXIT_MSR_LOAD_COUNT, 0);

    VmxWrite (VM_ENTRY_MSR_LOAD_COUNT, 0);
    VmxWrite (VM_ENTRY_INTR_INFO_FIELD, 0);

    //VM_ENTRY_EXCEPTION_ERROR_CODE   = 0x00004018,  //no init
    //VM_ENTRY_INSTRUCTION_LEN        = 0x0000401a,  //no init
    //TPR_THRESHOLD                   = 0x0000401c,  //no init

    /*32BIT Read-only Fields:need no setup */

    /*32BIT Guest-Statel Fields. */

    VmxWrite (GUEST_GDTR_LIMIT, GetGdtLimit ());
    VmxWrite (GUEST_IDTR_LIMIT, GetIdtLimit ());

    VmxWrite (GUEST_INTERRUPTIBILITY_INFO, 0);
    VmxWrite (GUEST_ACTIVITY_STATE, 0);   //Active state          
    //GUEST_SM_BASE          = 0x98000,   //no init
    VmxWrite (GUEST_SYSENTER_CS, MsrRead (MSR_IA32_SYSENTER_CS));

    /*32BIT Host-Statel Fields. */

    VmxWrite (HOST_IA32_SYSENTER_CS, MsrRead (MSR_IA32_SYSENTER_CS));     //no use

    /* NATURAL Control State Fields:need not setup. */
    // CR0 guest/host mask
    //VmxWrite (CR0_GUEST_HOST_MASK, X86_CR0_PG);   //X86_CR0_WP
    VmxWrite (CR0_GUEST_HOST_MASK, X86_CR0_WP);
    // CR0 read shadow
    //VmxWrite (CR0_READ_SHADOW, (RegGetCr4 () & X86_CR0_PG) | X86_CR0_PG);
    // if PG is clear, a vmexit will be caused
    VmxWrite (CR0_READ_SHADOW, X86_CR0_WP);

    //VmxWrite(CR4_GUEST_HOST_MASK, X86_CR4_VMXE|X86_CR4_PAE|X86_CR4_PSE);
    // disable vmexit 0f mov to cr4 expect for X86_CR4_VMXE
    VmxWrite (CR4_GUEST_HOST_MASK, X86_CR4_VMXE); 
    VmxWrite (CR4_READ_SHADOW, 0);

    // CR3_TARGET_COUNT is 0, mov to CR3 always cause a vmexit
    VmxWrite (CR3_TARGET_VALUE0, 0);      //no use
    VmxWrite (CR3_TARGET_VALUE1, 0);      //no use                        
    VmxWrite (CR3_TARGET_VALUE2, 0);      //no use
    VmxWrite (CR3_TARGET_VALUE3, 0);      //no use

    /* NATURAL Read-only State Fields:need not setup. */

    /* NATURAL GUEST State Fields. */

    VmxWrite (GUEST_CR0, RegGetCr0 ());
    VmxWrite (GUEST_CR3, RegGetCr3 ());
    VmxWrite (GUEST_CR4, RegGetCr4 ());

    GdtBase = (PVOID) GetGdtBase ();

    // Setup guest selectors
    VmxFillGuestSelectorData (GdtBase, ES, RegGetEs ());//<----------------------5.2 Finished
    VmxFillGuestSelectorData (GdtBase, CS, RegGetCs ());
    VmxFillGuestSelectorData (GdtBase, SS, RegGetSs ());
    VmxFillGuestSelectorData (GdtBase, DS, RegGetDs ());
    VmxFillGuestSelectorData (GdtBase, FS, RegGetFs ());
    VmxFillGuestSelectorData (GdtBase, GS, RegGetGs ());
    VmxFillGuestSelectorData (GdtBase, LDTR, GetLdtr ());
    VmxFillGuestSelectorData (GdtBase, TR, GetTrSelector ());

    // LDTR/TR bases have been set in VmxFillGuestSelectorData()
    VmxWrite (GUEST_GDTR_BASE, (ULONG) GdtBase);
    VmxWrite (GUEST_IDTR_BASE, GetIdtBase ());

    VmxWrite (GUEST_DR7, 0x400);
    VmxWrite (GUEST_RSP, (ULONG) GuestEsp);     //setup guest sp
    VmxWrite (GUEST_RIP, (ULONG) GuestEip);     //setup guest ip:CmSlipIntoMatrix
    VmxWrite (GUEST_RFLAGS, RegGetRflags ());
    //VmxWrite(GUEST_PENDING_DBG_EXCEPTIONS, 0);//no init
    VmxWrite (GUEST_SYSENTER_ESP, (ULONG)MsrRead (MSR_IA32_SYSENTER_ESP));
    VmxWrite (GUEST_SYSENTER_EIP, (ULONG)MsrRead (MSR_IA32_SYSENTER_EIP));

    /* HOST State Fields. */
    VmxWrite (HOST_CR0, RegGetCr0 ());

#ifdef VMX_USE_PRIVATE_CR3
    // private cr3
    VmxWrite (HOST_CR3, g_PageMapBasePhysicalAddress.QuadPart);
#else
    VmxWrite (HOST_CR3, RegGetCr3 ());
#endif
    VmxWrite (HOST_CR4, RegGetCr4 ());

    // unchecked
    //VmxWrite (HOST_FS_BASE, MsrRead (MSR_FS_BASE));
    //VmxWrite (HOST_GS_BASE, MsrRead (MSR_GS_BASE));
	//注意这里只处理FS和GS两个段寄存器。
    CmInitializeSegmentSelector (&SegmentSelector, RegGetFs (), (PVOID) GetGdtBase ());//<----------------------5.3 Finish
    VmxWrite (HOST_FS_BASE, SegmentSelector.base);

    CmInitializeSegmentSelector (&SegmentSelector, RegGetGs (), (PVOID) GetGdtBase ());
    VmxWrite (HOST_GS_BASE, SegmentSelector.base);

    // TODO: we must setup our own TSS
    // FIXME???

    CmInitializeSegmentSelector (&SegmentSelector, GetTrSelector (), (PVOID) GetGdtBase ());
    VmxWrite (HOST_TR_BASE, SegmentSelector.base);

    // unchecked
    //VmxWrite (HOST_GDTR_BASE, (ULONG64) Cpu->GdtArea);
    //VmxWrite (HOST_IDTR_BASE, (ULONG64) Cpu->IdtArea);

    // FIXME???
    VmxWrite(HOST_GDTR_BASE, GetGdtBase());
    VmxWrite(HOST_IDTR_BASE, GetIdtBase());

    VmxWrite (HOST_IA32_SYSENTER_ESP, (ULONG)MsrRead (MSR_IA32_SYSENTER_ESP));
    VmxWrite (HOST_IA32_SYSENTER_EIP, (ULONG)MsrRead (MSR_IA32_SYSENTER_EIP));

    VmxWrite (HOST_RSP, g_HostStackBaseAddress + 0x0C00); //setup host sp at vmxLaunch(...)

    // setup host ip:CmSlipIntoMatrix
    VmxWrite (HOST_RIP, (ULONG) VmxVmexitHandler); //setup host ip:CmSlipIntoMatrix

	Print(("Helloworld:VmxSetupVMCS(): Exit\n"));

    return STATUS_SUCCESS;
}
/**
 * effects: 用于填充VMCB中Guest状态描述中的段选择器部分
 */
static NTSTATUS NTAPI VmxFillGuestSelectorData (
    PVOID GdtBase,
    ULONG Segreg,//SEGREGS枚举中的段选择符号，用于描述要Fill哪个段选择器
    USHORT Selector
)
{//Finished
    SEGMENT_SELECTOR SegmentSelector = {0};
    ULONG uAccessRights;
    CmInitializeSegmentSelector (&SegmentSelector, Selector, GdtBase);//<--------------------6.1 Finished
    uAccessRights = 
        ((PUCHAR)&SegmentSelector.attributes)[0] + (((PUCHAR)&SegmentSelector.attributes)[1] << 12);

    if (!Selector)
        uAccessRights |= 0x10000;

    VmxWrite (GUEST_ES_SELECTOR + Segreg * 2, Selector);
    VmxWrite (GUEST_ES_LIMIT + Segreg * 2, SegmentSelector.limit);
    VmxWrite (GUEST_ES_AR_BYTES + Segreg * 2, uAccessRights);

    //if ((Segreg == LDTR) || (Segreg == TR))
    {
        // don't setup for FS/GS - their bases are stored in MSR values
        // for x64?
        VmxWrite (GUEST_ES_BASE + Segreg * 2, SegmentSelector.base);
    }

    return STATUS_SUCCESS;
}

// make the ctl code legal
static ULONG32 NTAPI VmxAdjustControls (
    ULONG32 Ctl,
    ULONG32 Msr
)
{//Finished
    LARGE_INTEGER MsrValue;

    MsrValue.QuadPart = MsrRead (Msr);
    Ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
    Ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
    return Ctl;
}
/**
 * VM Exit Event Dispatcher
 * VMExit事件分发逻辑
 */
static VOID NTAPI VmxDispatchEvent (
    PCPU Cpu,
    PGUEST_REGS GuestRegs
)
{//Finished
#if DEBUG_LEVEL>2
  Print(("VmxDispatchEvent(): exitcode = %x\n", VmxRead (VM_EXIT_REASON)));
#endif

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
  //CmReloadGdtr ((PVOID) VmxRead (GUEST_GDTR_BASE), (ULONG) VmxRead (GUEST_GDTR_LIMIT));

  //MsrWrite (MSR_GS_BASE, VmxRead (GUEST_GS_BASE));
  //MsrWrite (MSR_FS_BASE, VmxRead (GUEST_FS_BASE));

  // FIXME???
  // restore ds, es
  //CmSetDS((USHORT)VmxRead(GUEST_DS_SELECTOR));
  //CmSetES((USHORT)VmxRead(GUEST_ES_SELECTOR));

  // cs and ss must be the same with the guest OS in this implementation

  // restore old IDTR
  //CmReloadIdtr ((PVOID) VmxRead (GUEST_IDTR_BASE), (ULONG) VmxRead (GUEST_IDTR_LIMIT));

  return;
}

/**
 * Shutdown VM
 * 关闭虚拟机
 */
static NTSTATUS NTAPI VmxShutdown (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  BOOLEAN bSetupTimeBomb
)
{	//Finished
	UCHAR Trampoline[0x600];

	Print(("VmxShutdown(): CPU#%d\n", Cpu->ProcessorNumber));

	#if DEBUG_LEVEL>2
		VmxDumpVmcs ();
	#endif
	InterlockedDecrement (&g_uSubvertedCPUs);

	// The code should be updated to build an approproate trampoline to exit to any guest mode.
	VmxGenerateTrampolineToGuest (Cpu, GuestRegs, Trampoline, bSetupTimeBomb);

	Print(("VmxShutdown(): Trampoline generated\n", Cpu->ProcessorNumber));
	VmxDisable ();
	((VOID (*)()) & Trampoline) ();

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

#if DEBUG_LEVEL>2
    Print(("VmxHandleInterception(): Exitcode %x\n", Exitcode));
#endif

    if (Exitcode == EXIT_REASON_CR_ACCESS
        && GuestRegs->eax == BP_EXIT_EAX)
    {
        // to uninstall
        VmxShutdown(Cpu, GuestRegs, FALSE);
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
static VOID NTAPI VmxAdjustRip (
    PCPU Cpu,
    PGUEST_REGS GuestRegs,
    ULONG Delta
)
{ //Finished
    VmxWrite (GUEST_RIP, VmxRead (GUEST_RIP) + Delta);
    return;
}