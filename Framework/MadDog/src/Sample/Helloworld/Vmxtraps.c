#include "vmxtraps.h"

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

//static BOOLEAN NTAPI VmxDispatchVmxTaskDispatch(
//  PCPU Cpu,
//  PGUEST_REGS GuestRegs,
//  PNBP_TRAP Trap,
//  BOOLEAN WillBeAlsoHandledByGuestHv,
//  ...
//);
//
//static void HandleTaskSwitch(
//	PPUSHAD_REGS x86, 
//	PTASK_SWITCH_EQUALIFICATION ts
//	);
//
//static ULONG GetSegmentLimitGuest(ULONG SegmentSelector);

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
        EXIT_REASON_CPUID, 
        FALSE,
        0, // length of the instruction, 0 means length need to be get from vmcs later. 
        VmxDispatchCpuid, //<----------------4.2 Finish
        &Trap,
		LAB_TAG);
  if (!NT_SUCCESS (Status)) 
  {
    Print(("VmxRegisterTraps(): Failed to register VmxDispatchCpuid with status 0x%08hX\n", Status));
    return Status;
  }
  MadDog_RegisterTrap (Cpu, Trap);//<----------------4.3//Finish

    Status = HvInitializeGeneralTrap (
        Cpu, 
        EXIT_REASON_MSR_READ,
        FALSE,
        0, // length of the instruction, 0 means length need to be get from vmcs later. 
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

 // 	Status = HvInitializeGeneralTrap ( //<----------------4.1 Finish
	//	Cpu, 
	//	EXIT_REASON_TASK_SWITCH, //Exception or NMI
	//	FALSE,
	//	0, // length of the instruction, 0 means length need to be get from vmcs later. 
	//	VmxDispatchVmxTaskDispatch, //<----------------4.2 Finish
	//	&Trap,
	//	LAB_TAG);
	//if (!NT_SUCCESS (Status)) 
	//{
	//	Print(("VmxRegisterTraps(): Failed to register VmxDispatchInterrupt with status 0x%08hX\n", Status));
	//	return Status;
	//}
	//MadDog_RegisterTrap (Cpu, Trap);

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

	if (fn == BP_KNOCK_EAX) 
	{
		Print(("Helloworld:Magic knock received: %p\n", BP_KNOCK_EAX));
		GuestRegs->eax = BP_KNOCK_EAX_ANSWER;
		GuestRegs->ebx = BP_KNOCK_EBX_ANSWER;
		GuestRegs->edx = BP_KNOCK_EDX_ANSWER;
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

	//VmxDumpVmcs()();
	Print(("Helloworld:Missed Magic knock:EXIT_REASON_CPUID fn 0x%x 0x%x 0x%x 0x%x 0x%x \n", fn, eax, ebx, ecx, edx));
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

    if (!Cpu || !GuestRegs)
        return TRUE;

#if DEBUG_LEVEL>2
    Print(("VmxDispatchCrAccess()\n"));
#endif

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
            Cpu->Vmx.GuestCR3 = *(((PULONG) GuestRegs) + gp);

            if (Cpu->Vmx.GuestCR0 & X86_CR0_PG)       //enable paging
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

//
//
//ULONG GetSegmentLimitGuest(ULONG SegmentSelector){
//        ULONG ret_value = 0;
//        PGDT_ENTRY pgdt_entry;
//        GDT_BASE gdt_base;
//        ULONG index;
//        
//        pgdt_entry = (PGDT_ENTRY)VmxRead(0x6816);
//        index = SegmentSelector >> 3;
//        
//        ret_value = pgdt_entry[index].SegmentLimitLo + (pgdt_entry[index].SegmentLimitHi << 16);
//        
//        if (pgdt_entry[index].Granularity)
//                ret_value *= 0x1000;
//        return ret_value;
//}
//
//
//ULONG SegmentSelectorToAccessRightsGuest(ULONG SegmentSelector){
//        ULONG ret_value = 0;
//        PSEGMENT_ACCESS_RIGHTS pAccessRights;
//        PGDT_ENTRY pgdt_entry;
//        GDT_BASE gdt_base;
//        ULONG index;
//        
//        index = SegmentSelector >> 3;
//        
//        pgdt_entry = (PGDT_ENTRY)VmxRead(0x6816);
//        
//        pAccessRights = (PSEGMENT_ACCESS_RIGHTS)&ret_value;
//        
//        __asm{
//                pushad
//                mov     eax, pgdt_entry
//                mov     ecx, index
//                shl     ecx, 3
//                add     eax, ecx
//                add     eax, 5
//                mov     eax, [eax]
//                and     eax, 0F0FFh
//                mov     ret_value, eax
//                popad
//        }
//                
//        
//        /*
//        pAccessRights->SegmentType = pgdt_entry[index].Type;
//        pAccessRights->DescriptorType = pgdt_entry[index].DescriptorType;
//        pAccessRights->Dpl = pgdt_entry[index].Dpl;
//        pAccessRights->Present = pgdt_entry[index].Present;
//        pAccessRights->Available = pgdt_entry[index].Available;
//        pAccessRights->DefaultOperationSize = pgdt_entry[index].DefaultOperationSize;
//        */
//        if (SegmentSelector == 0x30)
//                pAccessRights->Granularity = 0; //pgdt_entry[index].Granularity;
//        //if (SegmentSelector == 0x3B)
//        //        pAccessRights->Granularity = 0;
//                
//        
//        
//                     
//        return ret_value;
//}   
//
//
//ULONG GetSegmentBaseGuest(ULONG SegmentSelector){
//        ULONG ret_value = 0;
//        PGDT_ENTRY pgdt_entry;
//        GDT_BASE gdt_base;
//        ULONG index;
//        
//        index = SegmentSelector >> 3;
//        
//        pgdt_entry = (PGDT_ENTRY)VmxRead(0x6816);
//         
//        ret_value = pgdt_entry[index].BaseLow + 
//                    (pgdt_entry[index].BaseMid << 16) + 
//                    (pgdt_entry[index].BaseHi << 24);
//        
//        return ret_value;
//}             
//
///*
//
//        As this engine doesn't support MP system when SoftIce is active, I leave
//        this code here as a refference on how TaskSwitch should be handled!!!!
//
//*/
////this is required when using SoftICE on mp systems as Sice uses
////NMI to stop/resume other cpus...
////If a task switch causes a VM exit, none of the following are modified by the
////task switch: old task-state segment (TSS); new TSS; old TSS descriptor; new
////TSS descriptor; RFLAGS.NT or the TR register
//
//void HandleTaskSwitch(PPUSHAD_REGS x86, PTASK_SWITCH_EQUALIFICATION ts){
//        GDT_BASE gdt_base;
//        PGDT_ENTRY pgdt_entry;
//        PKTSS ptss, ptss_prev;
//        ULONG index, index_prev, dummy;
//        ULONG tr_old;
//        
//        //if (ts->SourceOfTaskSwitch != 3)
//        //        return;
//        
//        //first all TSS has to be filed with current state to allow 
//        //return to TASK!!!!...
//        //__asm sgdt gdt_base
//        //pgdt_entry = (PGDT_ENTRY)(gdt_base.BaseLo + (gdt_base.BaseHi << 16));        
//        
//        pgdt_entry = (PGDT_ENTRY)VmxRead(0x6816);        //get GDT base for Guest...
//        
//        //Guest TR selector 000000111B 0000080EH
//        tr_old = VmxRead(0x80E);
//        index_prev = tr_old >> 3;
//        
//        ptss_prev = (PKTSS)(pgdt_entry[index_prev].BaseLow +
//                           (pgdt_entry[index_prev].BaseMid << 16) +
//                           (pgdt_entry[index_prev].BaseHi << 24));
//                    
//        ptss_prev->Eax = x86->regEax;
//        ptss_prev->Ecx = x86->regEcx;
//        ptss_prev->Ebx = x86->regEbx;
//        ptss_prev->Edx = x86->regEdx;
//        ptss_prev->Ebp = x86->regEbp;
//        ptss_prev->Esi = x86->regEsi;
//        ptss_prev->Edi = x86->regEdi;
//        
//        //Guest RSP 000001110B 0000681CH
//        ptss_prev->Esp = VmxRead(0x681C);
//        //Guest RIP 000001111B 0000681EH
//        ptss_prev->Eip = VmxRead(0x681E);
//        ptss_prev->Eip -= VmxRead(0x440C);       //<---- exit instruction length... blah...
//                                                //      as it was set up earlier... so we have to
//                                                //      sub it here...
//                                                
//        //Guest RFLAGS 000010000B 00006820H
//        ptss_prev->EFlags = VmxRead(0x6820);  
//        //Guest CR3 000000001B 00006802H
//        //ptss_prev->CR3 = VmxRead(0x6802);
//        
//        //Guest ES selector 000000000B 00000800H
//        ptss_prev->Es = (UINT16)VmxRead(0x800);
//        //Guest CS selector 000000001B 00000802H
//        ptss_prev->Cs = (UINT16)VmxRead(0x802);
//        //Guest SS selector 000000010B 00000804H
//        ptss_prev->Ss = (UINT16)VmxRead(0x804);
//        //Guest DS selector 000000011B 00000806H
//        ptss_prev->Ds = (UINT16)VmxRead(0x806);
//        //Guest FS selector 000000100B 00000808H
//        ptss_prev->Fs = (UINT16)VmxRead(0x808);
//        //Guest GS selector 000000101B 0000080AH
//        ptss_prev->Gs = (UINT16)VmxRead(0x80A);
//        //Guest LDTR selector 000000110B 0000080CH
//        ptss_prev->LDT = (UINT16)VmxRead(0x80C);          
//          
//        //now clear busy flag from this task...
//        if (ts->SourceOfTaskSwitch == 1)                //iret clears Busy flag in task...
//                pgdt_entry[index_prev].Type = 9;        //so clear it... task switch only
//                                                        //occurs here if NT flag is set...
//                
//        
//        index = ts->Selector;
//                
//        index = index >> 3;
//        
//                
//        ptss = (PKTSS)(pgdt_entry[index].BaseLow +
//                      (pgdt_entry[index].BaseMid << 16) +
//                      (pgdt_entry[index].BaseHi << 24));
//        
//        x86->regEax = ptss->Eax;
//        x86->regEcx = ptss->Ecx;
//        x86->regEdx = ptss->Edx;
//        x86->regEbx = ptss->Ebx;
//        x86->regEbp = ptss->Ebp;
//        x86->regEsi = ptss->Esi;
//        x86->regEdi = ptss->Edi;
//        
//        //issue sequence of VmWrites to properly set needed fields...
//        //
//        
//        //Guest ES selector 000000000B 00000800H
//        VmxWrite(0x800, ptss->Es);
//        //Guest CS selector 000000001B 00000802H
//        VmxWrite(0x802, ptss->Cs);
//        //Guest SS selector 000000010B 00000804H
//        VmxWrite(0x804, ptss->Ss);
//        //Guest DS selector 000000011B 00000806H
//        VmxWrite(0x806, ptss->Ds);
//        //Guest FS selector 000000100B 00000808H
//        VmxWrite(0x808, ptss->Fs);
//        //Guest GS selector 000000101B 0000080AH
//        VmxWrite(0x80A, ptss->Gs);
//        //Guest LDTR selector 000000110B 0000080CH
//        VmxWrite(0x80C, ptss->LDT);
//        //Guest TR selector 000000111B 0000080EH
//        VmxWrite(0x80E, ts->Selector);
//        
//        //set access rights...
//        //Guest ES limit 000000000B 00004800H
//        VmxWrite(0x4800, 0xFFFFFFFF);
//        //Guest CS limit 000000001B 00004802H
//        VmxWrite(0x4802, 0xFFFFFFFF);
//        //Guest SS limit 000000010B 00004804H
//        VmxWrite(0x4804, 0xFFFFFFFF);
//        //Guest DS limit 000000011B 00004806H
//        VmxWrite(0x4806, 0xFFFFFFFF);
//        //Guest FS limit 000000100B 00004808H
//        dummy = GetSegmentLimitGuest(ptss->Fs);
//        VmxWrite(0x4808, dummy);
//        //Guest GS limit 000000101B 0000480AH
//        VmxWrite(0x480A, 0xFFFFFFFF);
//        //Guest LDTR limit 000000110B 0000480CH
//        VmxWrite(0x480C, 0);
//        //Guest TR limit 000000111B 0000480EH
//        dummy = GetSegmentLimitGuest(ts->Selector);
//        VmxWrite(0x480E, dummy);
//        
//        //before setting Access Rights set TaskState to Busy...
//        pgdt_entry[index].Type = 11;
//        
//        //Guest ES access rights 000001010B 00004814H
//        dummy = SegmentSelectorToAccessRightsGuest(ptss->Es);
//        VmxWrite(0x4814, dummy);
//        //Guest CS access rights 000001011B 00004816H
//        dummy = SegmentSelectorToAccessRightsGuest(ptss->Cs);
//        VmxWrite(0x4816, dummy);
//        //Guest SS access rights 000001100B 00004818H
//        dummy = SegmentSelectorToAccessRightsGuest(ptss->Ss);
//        VmxWrite(0x4818, dummy);
//        //Guest DS access rights 000001101B 0000481AH
//        dummy = SegmentSelectorToAccessRightsGuest(ptss->Ds);
//        VmxWrite(0x481A, dummy);
//        //Guest FS access rights 000001110B 0000481CH
//        dummy = SegmentSelectorToAccessRightsGuest(ptss->Fs);
//        VmxWrite(0x481C, dummy);
//        //Guest GS access rights 000001111B 0000481EH
//        dummy = 0;
//        __asm bts dummy, 16
//        VmxWrite(0x481E, dummy);
//        //Guest LDTR access rights 000010000B 00004820H
//        dummy = 0;
//        __asm bts dummy, 16
//        VmxWrite(0x4820, dummy);
//        //Guest TR access rights 000010001B 00004822H
//        dummy = SegmentSelectorToAccessRightsGuest(ts->Selector);
//        VmxWrite(0x4822, dummy);
//        
//        //Guest ES base 000000011B 00006806H
//        dummy = GetSegmentBaseGuest(ptss->Es);
//        VmxWrite(0x6806, dummy);
//        //Guest CS base 000000100B 00006808H
//        dummy = GetSegmentBaseGuest(ptss->Cs);
//        VmxWrite(0x6808, dummy);
//        //Guest SS base 000000101B 0000680AH
//        dummy = GetSegmentBaseGuest(ptss->Ss);
//        VmxWrite(0x680A, dummy);
//        //Guest DS base 000000110B 0000680CH
//        dummy = GetSegmentBaseGuest(ptss->Ds);
//        VmxWrite(0x680C, dummy);
//        //Guest FS base 000000111B 0000680EH
//        dummy = GetSegmentBaseGuest(ptss->Fs);
//        VmxWrite(0x680E, dummy);
//        //Guest GS base 000001000B 00006810H
//        dummy = GetSegmentBaseGuest(ptss->Gs);
//        VmxWrite(0x6810, dummy);
//        //Guest LDTR base 000001001B 00006812H
//        VmxWrite(0x6812, 0);
//        //Guest TR base 000001010B 00006814H
//        dummy = GetSegmentBaseGuest(ts->Selector);
//        VmxWrite(0x6814, dummy);
//        
//        
//        //set eflags, stack, cr3 and eip...
//        //Guest RSP 000001110B 0000681CH
//        VmxWrite(0x681C, ptss->Esp);
//        //Guest RIP 000001111B 0000681EH
//        VmxWrite(0x681E, ptss->Eip);
//        //Guest RFLAGS 000010000B 00006820H
//        if (ts->SourceOfTaskSwitch == 3){               //is this task switch trough IDT...
//                dummy = ptss->EFlags;                  
//                __asm bts dummy, 14                     //set NT flag...
//                __asm bts dummy, 1                      //set reserved bit just in case...
//                VmxWrite(0x6820, dummy);
//        }else{
//                dummy = ptss->EFlags;                  
//                VmxWrite(0x6820, dummy);
//        }  
//        
//        
//        //Guest interruptibility state 000010010B 00004824H
//        //Guest activity state 000010011B 00004826H
//        
//        if (ts->SourceOfTaskSwitch == 1){
//                dummy = VmxRead(0x4824);
//                __asm btr dummy, 3
//                VmxWrite(0x4824, dummy);
//        }else{
//                dummy = VmxRead(0x4824);
//                __asm bts dummy, 3
//                VmxWrite(0x4824, dummy);
//        }  
//        
//        
//        //VmxWrite(0x4826, 0);
//        
//        //Guest CR3 000000001B 00006802H
//        VmxWrite(0x6802, ptss->CR3);
//        
//        if (ts->SourceOfTaskSwitch == 3)
//                ptss->Backlink = (UINT16)tr_old;        //iretd doesn't update backlink field...
//        return;
//}
//
//static BOOLEAN NTAPI VmxDispatchVmxTaskDispatch(
//  PCPU Cpu,
//  PGUEST_REGS GuestRegs,
//  PNBP_TRAP Trap,
//  BOOLEAN WillBeAlsoHandledByGuestHv,
//  ...
//)
//{
//	ULONG32 ExitQualification;
//	PUSHAD_REGS regs;
//	if (!Cpu || !GuestRegs)
//		return TRUE;
//
//	ExitQualification= VmxRead(0x6400);
//
//	regs.regEdi = GuestRegs->edi;
//    regs.regEsi = GuestRegs->esi;
//    regs.regEbp = GuestRegs->ebp;
//    regs.regEsp = VmxRead(GUEST_RSP);
//    regs.regEbx = GuestRegs->ebx;
//    regs.regEdx = GuestRegs->edx;
//    regs.regEcx = GuestRegs->ecx;
//    regs.regEax = GuestRegs->eax;
//	HandleTaskSwitch(&regs,(PTASK_SWITCH_EQUALIFICATION)&ExitQualification);
//
//	VmxWrite(GUEST_RSP,regs.regEsp);
//	GuestRegs->edi = regs.regEdi;
//    GuestRegs->esi = regs.regEsi;
//    GuestRegs->ebp = regs.regEbp;
//    GuestRegs->ebx = regs.regEbx;
//    GuestRegs->edx = regs.regEdx;
//    GuestRegs->ecx = regs.regEcx;
//    GuestRegs->eax = regs.regEax;
//
//	//inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
//	//Trap->RipDelta = inst_len;
//
//	//VmxWrite (GUEST_RFLAGS, VmxRead (GUEST_RFLAGS) & (~0x8d5) | 0x1 /* VMFailInvalid */ );
//	return TRUE;
//}