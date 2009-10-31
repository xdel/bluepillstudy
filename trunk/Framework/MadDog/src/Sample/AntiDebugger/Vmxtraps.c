#include "vmxtraps.h"
#include "Hook.h"
#include "Util.h"

BOOLEAN KDEHappen = FALSE;
BOOLEAN INTDebugHappen = FALSE;

//BOOLEAN ModifyKDE = FALSE;
//PBYTE KDEEntryAddr,KDERetAddr;
//PVOID JmpBackAddr;
//UCHAR OriginFuncHeader[10];

ULONG timer;

static BOOLEAN NTAPI VmxDispatchCpuid (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

static BOOLEAN NTAPI VmxDispatchVmxInstrDummy (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

static BOOLEAN NTAPI VmxDispatchINVD (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

static BOOLEAN NTAPI VmxDispatchMsrRead (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

static BOOLEAN NTAPI VmxDispatchMsrWrite (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

static BOOLEAN NTAPI VmxDispatchCrAccess (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

static BOOLEAN NTAPI VmxDispatchException (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

static BOOLEAN NTAPI VmxDispatchInterrupt (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);
static void myFunc(
    IN PVOID ExceptionRecord,
    IN PVOID ExceptionFrame,
    IN PVOID TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
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

	Status = HvInitializeGeneralTrap ( //<----------------4.1 Finish
		Cpu, 
		EXIT_REASON_EXCEPTION_NMI, //Exception or NMI
		FALSE,
		0, // length of the instruction, 0 means length need to be get from vmcs later. 
		VmxDispatchException, //<----------------4.2 Finish
		&Trap,
		LAB_TAG);
	if (!NT_SUCCESS (Status)) 
	{
		Print(("VmxRegisterTraps(): Failed to register VmxDispatchException with status 0x%08hX\n", Status));
		return Status;
	}
	MadDog_RegisterTrap (Cpu, Trap);

	Status = HvInitializeGeneralTrap ( //<----------------4.1 Finish
		Cpu, 
		EXIT_REASON_EXTERNAL_INTERRUPT, //Exception or NMI
		FALSE,
		0, // length of the instruction, 0 means length need to be get from vmcs later. 
		VmxDispatchInterrupt, //<----------------4.2 Finish
		&Trap,
		LAB_TAG);
	if (!NT_SUCCESS (Status)) 
	{
		Print(("VmxRegisterTraps(): Failed to register VmxDispatchInterrupt with status 0x%08hX\n", Status));
		return Status;
	}
	MadDog_RegisterTrap (Cpu, Trap);

	Status = HvInitializeGeneralTrap ( 
		Cpu, 
		EXIT_REASON_CPUID, 
		FALSE,
		0,
		VmxDispatchCpuid, 
		&Trap,
		LAB_TAG);
	if (!NT_SUCCESS (Status)) 
	{
		Print(("VmxRegisterTraps(): Failed to register VmxDispatchCpuid with status 0x%08hX\n", Status));
		return Status;
	}
	MadDog_RegisterTrap (Cpu, Trap);

	Status = HvInitializeGeneralTrap (
		Cpu, 
		EXIT_REASON_MSR_READ,
		FALSE,
		0,
		VmxDispatchMsrRead, 
		//VmxDispatchVmxInstrDummy,
		&Trap,
		LAB_TAG);
	if (!NT_SUCCESS (Status)) 
	{
		Print(("VmxRegisterTraps(): Failed to register VmxDispatchMsrRead with status 0x%08hX\n", Status));
		return Status;
	}
	MadDog_RegisterTrap (Cpu, Trap);

	Status = HvInitializeGeneralTrap (
		Cpu, 
		EXIT_REASON_MSR_WRITE, 
		FALSE,
		0,   // length of the instruction, 0 means length need to be get from vmcs later. 
		VmxDispatchMsrWrite, 
		//VmxDispatchVmxInstrDummy,
		&Trap,
		LAB_TAG);
	if (!NT_SUCCESS (Status)) 
	{
		Print(("VmxRegisterTraps(): Failed to register VmxDispatchMsrWrite with status 0x%08hX\n", Status));
		return Status;
	}
	MadDog_RegisterTrap (Cpu, Trap);

	Status = HvInitializeGeneralTrap (
		Cpu, 
		EXIT_REASON_CR_ACCESS,
		FALSE,
		0,  // length of the instruction, 0 means length need to be get from vmcs later. 
		VmxDispatchCrAccess, 
		&Trap,
		LAB_TAG);
	if (!NT_SUCCESS (Status)) 
	{
		Print(("VmxRegisterTraps(): Failed to register VmxDispatchCrAccess with status 0x%08hX\n", Status));
		return Status;
	}
	MadDog_RegisterTrap (Cpu, Trap);

	Status = HvInitializeGeneralTrap (
		Cpu, 
		EXIT_REASON_INVD, 
		FALSE,
		0,  // length of the instruction, 0 means length need to be get from vmcs later. 
		VmxDispatchINVD, 
		&Trap,
		LAB_TAG);
	if (!NT_SUCCESS (Status)) 
	{
		Print(("VmxRegisterTraps(): Failed to register VmxDispatchINVD with status 0x%08hX\n", Status));
		return Status;
	}
	MadDog_RegisterTrap (Cpu, Trap);

	Status = HvInitializeGeneralTrap (
		Cpu, 
		EXIT_REASON_EXCEPTION_NMI,
		FALSE,
		0,  // length of the instruction, 0 means length need to be get from vmcs later. 
		VmxDispatchVmxInstrDummy,//VmxDispatchPageFault, 
		&Trap,
		LAB_TAG);
	if (!NT_SUCCESS (Status)) 
	{
		Print(("VmxRegisterTraps(): Failed to register VmxDispatchPageFault with status 0x%08hX\n", Status));
		return Status;
	}
	MadDog_RegisterTrap (Cpu, Trap);

	// set dummy handler for all VMX intercepts if we compile without nested support
	for (i = 0; i < sizeof (TableOfVmxExits) / sizeof (ULONG32); i++) 
	{
		Status = HvInitializeGeneralTrap (
			Cpu, 
			TableOfVmxExits[i],
			FALSE,
			0,    // length of the instruction, 0 means length need to be get from vmcs later. 
			VmxDispatchVmxInstrDummy, 
			&Trap,
			LAB_TAG);
		if (!NT_SUCCESS (Status)) 
		{
			Print(("VmxRegisterTraps(): Failed to register VmxDispatchVmon with status 0x%08hX\n", Status));
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
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
)//Finished//same
{
	ULONG32 fn, eax, ebx, ecx, edx;
	
	ULONG inst_len;

	if (!Cpu || !GuestRegs)
		return TRUE;
	fn = GuestRegs->eax;

	#if DEBUG_LEVEL>1
		Print(("Helloworld:VmxDispatchCpuid(): Passing in Value(Fn): 0x%x\n", fn));
	#endif

	inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
	if (Trap->RipDelta == 0)
		Trap->RipDelta = inst_len;

	if (fn == FN_ENTERKDE) 
	{
		KDEHappen = TRUE;

		MadDog_GetCpuIdInfo (fn, &eax, &ebx, &ecx, &edx);
		GuestRegs->eax = eax;
		GuestRegs->ebx = ebx;
		GuestRegs->ecx = ecx;
		GuestRegs->edx = edx;
		return TRUE;
	}else if (fn == FN_EXITKDE) 
	{
		KDEHappen = FALSE;
		INTDebugHappen = FALSE;

		MadDog_GetCpuIdInfo (fn, &eax, &ebx, &ecx, &edx);
		GuestRegs->eax = eax;
		GuestRegs->ebx = ebx;
		GuestRegs->ecx = ecx;
		GuestRegs->edx = edx;
		return TRUE;
	}

	//else if(fn == 1)
	//{
	//MadDog_GetCpuIdInfo (fn, &eax, &ebx, &ecx, &edx);
	//GuestRegs->eax = 0x000126c5; //Core i7
	//GuestRegs->eax = 0x000106A5; //Core i7
	//GuestRegs->ebx = ebx;
	//GuestRegs->ecx = ecx;
	//GuestRegs->edx = edx;
	//return TRUE;
	//}

	ecx = (ULONG) GuestRegs->ecx;
	MadDog_GetCpuIdInfo (fn, &eax, &ebx, &ecx, &edx);
	GuestRegs->eax = eax;
	GuestRegs->ebx = ebx;
	GuestRegs->ecx = ecx;
	GuestRegs->edx = edx;

	//Print(("Helloworld:Missed Magic knock:EXIT_REASON_CPUID fn 0x%x 0x%x 0x%x 0x%x 0x%x \n", fn, eax, ebx, ecx, edx));
	return TRUE;
}

static BOOLEAN NTAPI VmxDispatchVmxInstrDummy (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
)
{
	ULONG32 inst_len;
	ULONG32 addr;

	if (!Cpu || !GuestRegs)
		return TRUE;
	Print(("VmxDispatchVminstructionDummy(): Nested virtualization not supported in this build!\n"));

	inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
	Trap->RipDelta = inst_len;

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
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
)
{
	ULONG inst_len;

	if (!Cpu || !GuestRegs)
		return TRUE;

	inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
	if (Trap->RipDelta == 0)
		Trap->RipDelta = inst_len;

	return TRUE;
}

static BOOLEAN NTAPI VmxDispatchMsrRead (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
)
{
	LARGE_INTEGER MsrValue;
	ULONG32 ecx;
	ULONG inst_len;

	if (!Cpu || !GuestRegs)
		return TRUE;

	inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
	if (Trap->RipDelta == 0)
		Trap->RipDelta = inst_len;

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
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
)
{
  LARGE_INTEGER MsrValue;
  ULONG32 ecx;
  ULONG inst_len;

  if (!Cpu || !GuestRegs)
    return TRUE;

  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  if (Trap->RipDelta == 0)
    Trap->RipDelta = inst_len;

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
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
)
{
    ULONG32 exit_qualification;
    ULONG32 gp, cr;
    ULONG value;
    ULONG inst_len;
	PULONG pKDEEntryPte;

    if (!Cpu || !GuestRegs)
        return TRUE;
	//
	//if(!ModifyKDE)
	//{
	//	//Hook KiDispatchException by insert an CPUID
	//	KDEEntryAddr = (PBYTE)GetKiDispatchExceptionAddr();
	//	Print(("VmxDispatchCrAccess:Initialization: KDEEntryAddr = 0x%llX\n", KDEEntryAddr));

	//	//pKDEEntryPte  = (PULONG)GET_PTE_VADDRESS(KDEEntryAddr);
	//	//Print(("VmxDispatchCrAccess:Initialization: KDEEntryPte = 0x%llX\n", *pKDEEntryPte));

	//	//Backup the origin function header.
	//	Memcpy(OriginFuncHeader,KDEEntryAddr,10);
	//	
	//	AcquireSpinLock();
	//	WPOFF();

	//	KDEEntryAddr[0] =0xB8;
	//	KDEEntryAddr[1] =0x00;
	//	KDEEntryAddr[2] =0x10;
	//	KDEEntryAddr[3] =0x00;
	//	KDEEntryAddr[4] =0x00; //Write a "mov eax,0x1000"
	//	KDEEntryAddr[5] =0x0F; 
	//	KDEEntryAddr[6] =0xA2; //Write a "cpuid"
	//	KDEEntryAddr[7] =0x90; //Write a "nop"
	//	KDEEntryAddr[8] =0x90; //Write a "nop"
	//	KDEEntryAddr[9] =0x90; //Write a "nop"

	//	WPON();
	//	ReleaseSpinLock();

	//	ModifyKDE = TRUE;
	//}

    inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
    if (Trap->RipDelta == 0)
        Trap->RipDelta = inst_len;

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
			if(KDEHappen && INTDebugHappen)
			{
				Print(("Kernel Debugger Detected!\n"));
			}
			else if (INTDebugHappen &&  !KDEHappen)
			{
				DbgPrint(("Ice Debugger Detected!\n"));
			}
            Cpu->Vmx.GuestCR3 = *(((PULONG) GuestRegs) + gp);

            //if (Cpu->Vmx.GuestCR0 & X86_CR0_PG)       //enable paging
            {
#if DEBUG_LEVEL>2
                Print(("VmxDispatchCrAccess(): TYPE_MOV_TO_CR cr3:0x%x\n", *(((PULONG64) GuestRegs) + gp)));
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
            Print(("VmxDispatchCrAccess(): TYPE_MOV_FROM_CR cr3:0x%x\n", value));
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


static BOOLEAN NTAPI VmxDispatchException(
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
)
{
	ULONG32 fn, eax, ebx, ecx, edx;
	ULONG inst_len,intrInfo;
	PINTERUPTION_INFORMATION_FIELD pinject_event;
       PINTERUPTION_INFORMATION_FIELD pint;

	if (!Cpu || !GuestRegs)
		return TRUE;
	
	intrInfo = VmxRead(VM_EXIT_INTR_INFO);
	pinject_event = (PINTERUPTION_INFORMATION_FIELD)&intrInfo;

	INTDebugHappen = TRUE;

	VmxWrite(VM_ENTRY_INTR_INFO_FIELD, intrInfo);//Event inject back

	inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
	VmxWrite(VM_ENTRY_INSTRUCTION_LEN,inst_len);  //softice properly catches int3

	Trap->RipDelta = 0;//Don't modify guest eip
	return TRUE;
	
}

VOID InjectNMI(VOID){
        ULONG dummy;
        PINTERUPTION_INFORMATION_FIELD pinject_event;
        
        //nmi was delivered from 2nd CPU while we were in vmx-root 
        //so inject NMI to this processor so it can be handled by
        //NMI in vmx-non-root code... 
        
        dummy = 0;
        pinject_event = (PINTERUPTION_INFORMATION_FIELD)&dummy;     
        pinject_event->Vector = 2;
        pinject_event->InteruptionType = 2;    //NMI
        pinject_event->DeliverErrorCode = 0;
        pinject_event->Valid = 1;
        VmxWrite(VM_ENTRY_INTR_INFO_FIELD, dummy); 
        
        //dummy = VmRead(0x4824);
        //__asm bts dummy, 3
        //VmWrite(0x4824, dummy); 
        //inject_nmi[ccpu] = 0;    
}

static BOOLEAN NTAPI VmxDispatchInterrupt(
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
)
{
	ULONG32 fn, eax, ebx, ecx, edx;
	ULONG inst_len,intrInfo;
	PINTERUPTION_INFORMATION_FIELD pinject_event;
       PINTERUPTION_INFORMATION_FIELD pint;

	if (!Cpu || !GuestRegs)
		return TRUE;
	
	intrInfo = VmxRead(VM_EXIT_INTR_INFO);
	pinject_event = (PINTERUPTION_INFORMATION_FIELD)&intrInfo;


	if (pinject_event->InteruptionType == 2)
		InjectNMI();

	if(pinject_event->Vector ==0x2d);
		INTDebugHappen = TRUE;

	VmxWrite(VM_ENTRY_INTR_INFO_FIELD, intrInfo);//Event inject back

	inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
	VmxWrite(VM_ENTRY_INSTRUCTION_LEN,inst_len);  //softice properly catches int3

       if (Trap->RipDelta == 0)
             Trap->RipDelta = inst_len;
	return TRUE;
	
}


