#include "vmxtraps.h"

BOOLEAN StartRecording[CoreCount];
Parameter g_CommandInfo;
static ULONG lock;//This is used as a spin-lock in CcFakeSysenterTrap()
static ULONG plock;//Points to the <lock> variable

ULONG64 CcSysenterTimes;//Record Sysenter instruction happen times.

//MSR backup
ULONG32 CcOriginSysenterEIP[CoreCount];
ULONG32 CcOriginSysenterESP[CoreCount];

/**
 * Used to Setup ourselves sysenter entry.
 */
static NTSTATUS NTAPI CcSetupSysenterTrap(int cProcessorNumber);

/**
 * Used to restore the origin sysenter entry.
 */
static NTSTATUS NTAPI CcDestroySysenterTrap(int cProcessorNumber);

/**
 * When start recording, This function is used to initialize the members which is important 
 * in all recording-related scenrios.
 */
static VOID NTAPI GeneralInitialization(PGUEST_REGS GuestRegs);

static BOOLEAN NTAPI VmxDispatchCpuid (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);

static BOOLEAN NTAPI VmxDispatchVmxInstrDummy (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);

static BOOLEAN NTAPI VmxDispatchINVD (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);

static BOOLEAN NTAPI VmxDispatchMsrRead (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);

static BOOLEAN NTAPI VmxDispatchMsrWrite (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);

static BOOLEAN NTAPI VmxDispatchCrAccess (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);

/**
 * effects: Register traps in this function
 * requires: <Cpu> is valid
 */
NTSTATUS NTAPI VmxRegisterTraps (
  PCPU Cpu
)
{//Finished
  NTSTATUS Status;
  PNBP_TRAP Trap;

  // used to set dummy handler for all VMX intercepts when we compile without nested support
  ULONG32 i, TableOfVmxExits[] = {
    EXIT_REASON_VMCALL,
    EXIT_REASON_VMCLEAR,
    EXIT_REASON_VMLAUNCH,
    EXIT_REASON_VMRESUME,
    EXIT_REASON_VMPTRLD,
    EXIT_REASON_VMPTRST,
    EXIT_REASON_VMREAD,
    EXIT_REASON_VMWRITE,
    EXIT_REASON_VMXON,
    EXIT_REASON_VMXOFF
  };
    Status = MadDog_InitializeGeneralTrap ( //<----------------4.1 Finish
        Cpu, 
        EXIT_REASON_CPUID, 
        0, // length of the instruction, 0 means length need to be get from vmcs later. 
        VmxDispatchCpuid, //<----------------4.2 Finish
        &Trap,
		LAB_TAG);
  if (!NT_SUCCESS (Status)) 
  {
    HvmPrint(("VmxRegisterTraps(): Failed to register VmxDispatchCpuid with status 0x%08hX\n", Status));
    return Status;
  }
  MadDog_RegisterTrap (Cpu, Trap);//<----------------4.3//Finish

  Status = MadDog_InitializeGeneralTrap (
        Cpu, 
        EXIT_REASON_MSR_READ, 
        0, // length of the instruction, 0 means length need to be get from vmcs later. 
        VmxDispatchMsrRead, 
		//VmxDispatchVmxInstrDummy,
        &Trap,
		LAB_TAG);
  if (!NT_SUCCESS (Status)) 
  {
    HvmPrint(("VmxRegisterTraps(): Failed to register VmxDispatchMsrRead with status 0x%08hX\n", Status));
    return Status;
  }
  MadDog_RegisterTrap (Cpu, Trap);

  Status = MadDog_InitializeGeneralTrap (
      Cpu, 
      EXIT_REASON_MSR_WRITE, 
      0,   // length of the instruction, 0 means length need to be get from vmcs later. 
      VmxDispatchMsrWrite, 
	  //VmxDispatchVmxInstrDummy,
      &Trap,
	  LAB_TAG);
  if (!NT_SUCCESS (Status)) 
  {
    HvmPrint(("VmxRegisterTraps(): Failed to register VmxDispatchMsrWrite with status 0x%08hX\n", Status));
    return Status;
  }
  MadDog_RegisterTrap (Cpu, Trap);

  Status = MadDog_InitializeGeneralTrap (
      Cpu, 
      EXIT_REASON_CR_ACCESS, 
      0,  // length of the instruction, 0 means length need to be get from vmcs later. 
      VmxDispatchCrAccess, 
      &Trap,
	  LAB_TAG);
  if (!NT_SUCCESS (Status)) 
  {
    HvmPrint(("VmxRegisterTraps(): Failed to register VmxDispatchCrAccess with status 0x%08hX\n", Status));
    return Status;
  }
  MadDog_RegisterTrap (Cpu, Trap);

  Status = MadDog_InitializeGeneralTrap (
      Cpu, 
      EXIT_REASON_INVD, 
      0,  // length of the instruction, 0 means length need to be get from vmcs later. 
      VmxDispatchINVD, 
      &Trap,
	  LAB_TAG);
  if (!NT_SUCCESS (Status)) 
  {
    HvmPrint(("VmxRegisterTraps(): Failed to register VmxDispatchINVD with status 0x%08hX\n", Status));
    return Status;
  }
  MadDog_RegisterTrap (Cpu, Trap);

  Status = MadDog_InitializeGeneralTrap (
      Cpu, 
      EXIT_REASON_EXCEPTION_NMI, 
      0,  // length of the instruction, 0 means length need to be get from vmcs later. 
      VmxDispatchVmxInstrDummy,//VmxDispatchPageFault, 
      &Trap,
	  LAB_TAG);
  if (!NT_SUCCESS (Status)) 
  {
    HvmPrint(("VmxRegisterTraps(): Failed to register VmxDispatchPageFault with status 0x%08hX\n", Status));
    return Status;
  }
  MadDog_RegisterTrap (Cpu, Trap);

  // set dummy handler for all VMX intercepts if we compile without nested support
  for (i = 0; i < sizeof (TableOfVmxExits) / sizeof (ULONG32); i++) 
  {
      Status = MadDog_InitializeGeneralTrap (
          Cpu, 
          TableOfVmxExits[i], 
          0,    // length of the instruction, 0 means length need to be get from vmcs later. 
          VmxDispatchVmxInstrDummy, 
          &Trap,
		  LAB_TAG);
    if (!NT_SUCCESS (Status)) 
    {
      HvmPrint(("VmxRegisterTraps(): Failed to register VmxDispatchVmon with status 0x%08hX\n", Status));
      return Status;
    }
    MadDog_RegisterTrap (Cpu, Trap);
  }

  return STATUS_SUCCESS;
}


//+++++++++++++++++++++Static Functions++++++++++++++++++++++++
/**
 * effects: Defines the handler of the VM Exit Event which is caused by CPUID.
 * In this function we will return "Hello World!" by pass value through eax,ebx
 * and edx registers.
 */
static BOOLEAN NTAPI VmxDispatchCpuid (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
)//Finished//same
{
	ULONG32 fn, eax, ebx, ecx, edx;
	ULONG inst_len;
	ULONG32 cProcessorNumber;
	NTSTATUS status;
	if (!Cpu || !GuestRegs)
	return TRUE;
	fn = GuestRegs->eax;

	#if DEBUG_LEVEL>1
	Print(("Helloworld:VmxDispatchCpuid(): Passing in Value(Fn): 0x%x\n", fn));
	#endif

	inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
	if (Trap->General.RipDelta == 0)
	Trap->General.RipDelta = inst_len;

	if (fn == START_RECORDING_EAX) //Start Recording
	{
		cProcessorNumber = KeGetCurrentProcessorNumber();
		HvmPrint(("Hypervisor: Start Recording\n"));
		//Step 1. Initialize members for generialize use.
		GeneralInitialization(GuestRegs);
		
		//Check if ContextCounter is recording currently
		//If true, ContextCounter Hypervisor should do nothing
		//We only allow only 1 ContextCounter instance to record at one time.
		if(StartRecording[cProcessorNumber] == FALSE)
		{
			HvmPrint(("ContextCounter: Hacking sysenter entry\n"));
			status = CcSetupSysenterTrap(cProcessorNumber);//Setup Sysenter trap
			if(status == STATUS_SUCCESS)
			{
				StartRecording[cProcessorNumber] = TRUE;
				CcSysenterTimes = 0;
			}		
		}		
	}
	else if(fn == END_RECORDING_EAX) //End Recording
	{
		cProcessorNumber = KeGetCurrentProcessorNumber();
		HvmPrint(("Hypervisor: End Recording\n"));	

		//Check if ContextCounter is recording currently
		//If false, ContextCounter Hypervisor should do nothing
		//We only allow only 1 ContextCounter instance to record at one time.
		if(StartRecording[cProcessorNumber] == TRUE)
		{
			HvmPrint(("ContextCounter: Releasing sysenter entry\n"));
			status = CcDestroySysenterTrap(cProcessorNumber);
			if(status == STATUS_SUCCESS)
			{
				StartRecording[cProcessorNumber] = FALSE;
			}
		}
	
		GuestRegs->eax = (ULONG32) CcSysenterTimes;
		GuestRegs->edx = (ULONG32) (CcSysenterTimes>>32);
		return TRUE;
	}
	else if(fn == TEST_PASSINVALUE_EAX)
	{
		HvmPrint(("ContextCounter: In Testing Mode\n"));
		GeneralInitialization(GuestRegs);
	}

	ecx = (ULONG) GuestRegs->ecx;
	MadDog_GetCpuIdInfo (fn, &eax, &ebx, &ecx, &edx);
	GuestRegs->eax = eax;
	GuestRegs->ebx = ebx;
	GuestRegs->ecx = ecx;
	GuestRegs->edx = edx;

	//VmxDumpVmcs()();
	Print(("Helloworld:Missed Magic knock:EXIT_REASON_CPUID fn 0x%x 0x%x 0x%x 0x%x 0x%x \n", fn, eax, ebx, ecx, edx));
	return TRUE;
}

static BOOLEAN NTAPI VmxDispatchVmxInstrDummy (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
)
{
  ULONG32 inst_len;
  ULONG32 addr;
  
  if (!Cpu || !GuestRegs)
    return TRUE;
  HvmPrint(("VmxDispatchVminstructionDummy(): Nested virtualization not supported in this build!\n"));

  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  Trap->General.RipDelta = inst_len;

  addr = GUEST_RIP;
  Print(("VmxDispatchVminstructionDummy(): GUEST_RIP 0x%X: 0x%llX\n", addr, VmxRead (addr)));
  addr = VM_EXIT_INTR_INFO;
  Print(("VmxDispatchVminstructionDummy(): EXIT_INTR 0x%X: 0x%llX\n", addr, VmxRead (addr)));
  addr = EXIT_QUALIFICATION;
  Print(("VmxDispatchVminstructionDummy(): QUALIFICATION 0x%X: 0x%llX\n", addr, VmxRead (addr)));
  addr = EXCEPTION_BITMAP;
  Print(("VmxDispatchVminstructionDummy(): EXCEPTION_BITMAP 0x%X: 0x%llX\n", addr, VmxRead (addr)));

  //VmxWrite (GUEST_RFLAGS, VmxRead (GUEST_RFLAGS) & (~0x8d5) | 0x1 /* VMFailInvalid */ );
  return TRUE;
}

static BOOLEAN NTAPI VmxDispatchINVD (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
)
{
  ULONG inst_len;

  if (!Cpu || !GuestRegs)
    return TRUE;

  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  if (Trap->General.RipDelta == 0)
    Trap->General.RipDelta = inst_len;

  return TRUE;
}

static BOOLEAN NTAPI VmxDispatchMsrRead (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
)
{
  LARGE_INTEGER MsrValue;
  ULONG32 ecx;
  ULONG inst_len;

  HvmPrint(("In VmxDispatchMsrRead(),ECX: %x\n",GuestRegs->ecx));
  if (!Cpu || !GuestRegs)
    return TRUE;

  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  if (Trap->General.RipDelta == 0)
    Trap->General.RipDelta = inst_len;

  ecx = GuestRegs->ecx;

  switch (ecx) 
  {
  case MSR_IA32_SYSENTER_CS:
    MsrValue.QuadPart = VmxRead (GUEST_SYSENTER_CS);
    break;
  case MSR_IA32_SYSENTER_ESP:
    MsrValue.QuadPart = VmxRead (GUEST_SYSENTER_ESP);
    break;
  case MSR_IA32_SYSENTER_EIP:
    MsrValue.QuadPart = VmxRead (GUEST_SYSENTER_EIP);
    Print(("VmxDispatchMsrRead(): Guest EIP: 0x%x read MSR_IA32_SYSENTER_EIP value: 0x%x \n", 
        VmxRead(GUEST_RIP), 
        MsrValue.QuadPart));
    break;
  case MSR_GS_BASE:
    MsrValue.QuadPart = VmxRead (GUEST_GS_BASE);
    break;
  case MSR_FS_BASE:
    MsrValue.QuadPart = VmxRead (GUEST_FS_BASE);
    break;
  case MSR_EFER:
    MsrValue.QuadPart = Cpu->Vmx.GuestEFER;
    //_KdPrint(("Guestip 0x%llx MSR_EFER Read 0x%llx 0x%llx \n",VmxRead(GUEST_RIP),ecx,MsrValue.QuadPart));
    break;
  default:
    if (ecx <= 0x1fff
        || (ecx >= 0xC0000000 && ecx <= 0xC0001fff))
    {
        MsrValue.QuadPart = MsrRead (ecx);
    }
  }

  GuestRegs->eax = MsrValue.LowPart;
  GuestRegs->edx = MsrValue.HighPart;

  return TRUE;
}


static BOOLEAN NTAPI VmxDispatchMsrWrite (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
)
{
  LARGE_INTEGER MsrValue;
  ULONG32 ecx;
  ULONG inst_len;

  if (!Cpu || !GuestRegs)
    return TRUE;

  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  if (Trap->General.RipDelta == 0)
    Trap->General.RipDelta = inst_len;

  ecx = GuestRegs->ecx;

  MsrValue.LowPart = (ULONG32) GuestRegs->eax;
  MsrValue.HighPart = (ULONG32) GuestRegs->edx;

  switch (ecx) 
  {
  case MSR_IA32_SYSENTER_CS:
    VmxWrite (GUEST_SYSENTER_CS, MsrValue.QuadPart);
    break;
  case MSR_IA32_SYSENTER_ESP:
    VmxWrite (GUEST_SYSENTER_ESP, MsrValue.QuadPart);
    break;
  case MSR_IA32_SYSENTER_EIP:
    VmxWrite (GUEST_SYSENTER_EIP, MsrValue.QuadPart);
    Print(("VmxDispatchMsrRead(): Guest EIP: 0x%x want to write MSR_IA32_SYSENTER_EIP value: 0x%x \n", 
        VmxRead(GUEST_RIP), 
        MsrValue.QuadPart));
    break;
  case MSR_GS_BASE:
    VmxWrite (GUEST_GS_BASE, MsrValue.QuadPart);
    break;
  case MSR_FS_BASE:
    VmxWrite (GUEST_FS_BASE, MsrValue.QuadPart);
    break;
  case MSR_EFER:
    //_KdPrint(("Guestip 0x%llx MSR_EFER write 0x%llx 0x%llx\n",VmxRead(GUEST_RIP),ecx,MsrValue.QuadPart)); 
    Cpu->Vmx.GuestEFER = MsrValue.QuadPart;
    MsrWrite (MSR_EFER, (MsrValue.QuadPart) | EFER_LME);
    break;
  default:
    if (ecx <= 0x1fff
        || (ecx >= 0xC0000000 && ecx <= 0xC0001fff))
    {
        MsrWrite (ecx, MsrValue.QuadPart);
    }
  }

  return TRUE;
}

static BOOLEAN NTAPI VmxDispatchCrAccess (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
)
{
    ULONG32 exit_qualification;
    ULONG32 gp, cr;
    ULONG value;
    ULONG inst_len;

    if (!Cpu || !GuestRegs)
        return TRUE;

#if DEBUG_LEVEL>2
    Print(("VmxDispatchCrAccess()\n"));
#endif

    inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
    if (Trap->General.RipDelta == 0)
        Trap->General.RipDelta = inst_len;

    //For MOV CR, the general-purpose register:
    //  0 = RAX
    //  1 = RCX
    //  2 = RDX
    //  3 = RBX
    //  4 = RSP
    //  5 = RBP
    //  6 = RSI
    //  7 = RDI
    //  8¨C15 represent R8-R15, respectively (used only on processors that support
    //  Intel 64 architecture)
    exit_qualification = (ULONG32) VmxRead (EXIT_QUALIFICATION);
    gp = (exit_qualification & CONTROL_REG_ACCESS_REG) >> 8;
    cr = exit_qualification & CONTROL_REG_ACCESS_NUM;

#if DEBUG_LEVEL>1
    Print(("VmxDispatchCrAccess(): gp: 0x%x cr: 0x%x exit_qualification: 0x%x\n", gp, cr, exit_qualification));
#endif

    //Access type:
    //  0 = MOV to CR
    //  1 = MOV from CR
    //  2 = CLTS
    //  3 = LMSW
    switch (exit_qualification & CONTROL_REG_ACCESS_TYPE) 
    {
    case TYPE_MOV_TO_CR:
        /*if (cr == 0) 
        {
            if (!(*(((PULONG) GuestRegs) + gp) & X86_CR0_WP))
            {
                // someone want to turn off the write protect
#if DEBUG_LEVEL>1
                _KdPrint (("VmxDispatchCrAccess(): Turnoff WP, gp: 0x%x cr: 0x%x\n", gp, cr));
#endif
                return TRUE;
            }

            Cpu->Vmx.GuestCR0 = *(((PULONG) GuestRegs) + gp);
            VmxWrite (GUEST_CR0, Cpu->Vmx.GuestCR0);
            return TRUE;

//======================================================================
            if (Cpu->Vmx.GuestCR0 & X86_CR0_PG) //enable paging
            {
                //_KdPrint(("VmxDispatchCrAccess():paging\n"));
                VmxWrite (GUEST_CR3, Cpu->Vmx.GuestCR3);
                if (Cpu->Vmx.GuestEFER & EFER_LME)
                    Cpu->Vmx.GuestEFER |= EFER_LMA;
                else
                    Cpu->Vmx.GuestEFER &= ~EFER_LMA;
            } 
            else    //disable paging
            {
                //_KdPrint(("VmxDispatchCrAccess():disable paging\n"));                         
                Cpu->Vmx.GuestCR3 = VmxRead (GUEST_CR3);
                VmxWrite (GUEST_CR3, g_IdentityPageTableBasePhysicalAddress_Legacy.QuadPart);
                Cpu->Vmx.GuestEFER &= ~EFER_LMA;
            }
#ifdef _X86_
            VmxWrite (CR0_READ_SHADOW, Cpu->Vmx.GuestCR0 & X86_CR0_PG);
#else
            VmxWrite (CR0_READ_SHADOW, Cpu->Vmx.GuestCR0 & X86_CR0_PG);
#endif
            VmxUpdateGuestEfer (Cpu);
            return FALSE;
//======================================================================
        }*/

        if (cr == 3) 
        {
            Cpu->Vmx.GuestCR3 = *(((PULONG) GuestRegs) + gp);

            if (Cpu->Vmx.GuestCR0 & X86_CR0_PG)       //enable paging
            {
#if DEBUG_LEVEL>2
                HvmPrint(("VmxDispatchCrAccess(): TYPE_MOV_TO_CR cr3:0x%x\n", *(((PULONG64) GuestRegs) + gp)));
#endif
                VmxWrite (GUEST_CR3, Cpu->Vmx.GuestCR3);

            }
            return TRUE;
        }
        /*if (cr == 4) 
        {

            //if(debugmode)
            //_KdPrint(("VmxDispatchCrAccess(): TYPE_MOV_TO_CR Cpu->Vmx.GuestEFER:0x%x Cpu->Vmx.GuestCR0:0x%x cr4:0x%x\n",Cpu->Vmx.GuestEFER,Cpu->Vmx.GuestCR0,*(((PULONG64)GuestRegs)+gp)));
            //Nbp need enabele VMXE. so guest try to clear cr4_vmxe, it would be mask.
#ifdef _X86_
            Cpu->Vmx.GuestCR4 = *(((PULONG32) GuestRegs) + gp);
            VmxWrite (CR4_READ_SHADOW, Cpu->Vmx.GuestCR4 & (X86_CR4_VMXE | X86_CR4_PAE));
            VmxWrite (GUEST_CR4, Cpu->Vmx.GuestCR4 | X86_CR4_VMXE);

#else
            //VmxWrite(CR4_READ_SHADOW, (*(((PULONG64)GuestRegs)+gp)) & (X86_CR4_VMXE|X86_CR4_PAE|X86_CR4_PSE));
            VmxWrite (CR4_READ_SHADOW, (*(((PULONG64) GuestRegs) + gp)) & (X86_CR4_VMXE));

            Cpu->Vmx.GuestCR4 = *(((PULONG64) GuestRegs) + gp);
            VmxWrite (GUEST_CR4, (*(((PULONG64) GuestRegs) + gp)) | X86_CR4_VMXE);
#endif

            return FALSE;
        }*/
        break;
    case TYPE_MOV_FROM_CR:
        if (cr == 3) 
        {
            value = Cpu->Vmx.GuestCR3;
#if DEBUG_LEVEL>2
            HvmPrint(("VmxDispatchCrAccess(): TYPE_MOV_FROM_CR cr3:0x%x\n", value));
#endif
            *(((PULONG32) GuestRegs) + gp) = (ULONG32) value;

        }
        break;
    case TYPE_CLTS:
        break;
    case TYPE_LMSW:
        break;
    }

    return TRUE;
}

/**
 * When start recording, This function is used to initialize the members which is important 
 * in all recording-related scenrios.
 */
VOID NTAPI GeneralInitialization(PGUEST_REGS GuestRegs)
{
	plock=(ULONG)&lock;
	
	//Copy the Parameter Struct from user space to kernel space.
	g_CommandInfo = *((PParameter)(GuestRegs->edx));
	HvmPrint(("g_CommandInfo->pid value:%d",g_CommandInfo.pid));
}

void __declspec(naked) CcFakeSysenterTrap()
{
	
	__asm{
	loop_down:
		lock	bts dword ptr [plock], 0
		jb	loop_down; Acquire A Spin Lock
	}

	__asm{
		push eax
		push ebx
		push ecx
		push edx
	}

	CcSysenterTimes++;

	__asm{
		pop edx
		pop ecx
		pop ebx
		pop eax
		lock	btr dword ptr [plock], 0; Release the Spin Lock
		jmp CcOriginSysenterEIP[0]
	}
}
static NTSTATUS NTAPI CcSetupSysenterTrap(int cProcessorNumber)
{
	PVOID newSysenterESP;
	
	CcOriginSysenterEIP[cProcessorNumber] = VmxRead(GUEST_SYSENTER_EIP);
	CcOriginSysenterESP[cProcessorNumber] = VmxRead(GUEST_SYSENTER_ESP);
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, OriginSysenterEIP:%x\n",cProcessorNumber,CcOriginSysenterEIP[cProcessorNumber]));
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, OriginSysenterESP:%x\n",cProcessorNumber,CcOriginSysenterESP[cProcessorNumber]));
	newSysenterESP = ExAllocatePoolWithTag (NonPagedPool, 4 * PAGE_SIZE, LAB_TAG);

	//Begin to rewrite the entry in MSR
	VmxWrite (GUEST_SYSENTER_EIP, (ULONG)&CcFakeSysenterTrap);
	VmxWrite (GUEST_SYSENTER_ESP, newSysenterESP);

	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, NewSysenterEntry:%x\n",cProcessorNumber,VmxRead(GUEST_SYSENTER_EIP)));

	return STATUS_SUCCESS;
}

static NTSTATUS NTAPI CcDestroySysenterTrap(int cProcessorNumber)
{
	//Step 1.Verify if the current sysenter entry is our CcFakeSysenterTrap()
	//This verify enables nested sysenter trap.
	ULONG currentSysEnterEIP = VmxRead(GUEST_SYSENTER_EIP);
	HvmPrint(("ContextCounter: In CcDestroySysenterTrap(): currentSysEnterEIP:%x\n",currentSysEnterEIP));
	HvmPrint(("ContextCounter: In CcDestroySysenterTrap(): CcFakeSysenterTrap:%x\n",&CcFakeSysenterTrap));
	if(currentSysEnterEIP == (ULONG)(&CcFakeSysenterTrap))//It is our fake entry
	{
		HvmPrint(("ContextCounter: In CcDestroySysenterTrap(): Restore origin sysenter entry\n"));
		cProcessorNumber = KeGetCurrentProcessorNumber();
		//Step 2. Replace the entry with the origin sysenter entry address.
		VmxWrite (GUEST_SYSENTER_EIP,CcOriginSysenterEIP[cProcessorNumber]);
		VmxWrite (GUEST_SYSENTER_ESP,CcOriginSysenterESP[cProcessorNumber]);

		return STATUS_SUCCESS;
	}
	HvmPrint(("ContextCounter: In CcDestroySysenterTrap(): Can't Restore origin sysenter entry, it has been substituded by other app.\n"));
	return STATUS_UNSUCCESSFUL;
}
