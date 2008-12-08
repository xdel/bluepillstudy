#include "vmxtraps.h"

extern BOOLEAN NTAPI VmxDispatchCpuid (
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
    Status = TrInitializeGeneralTrap ( //<----------------4.1 Finish
        Cpu, 
        EXIT_REASON_CPUID, 
        0, // length of the instruction, 0 means length need to be get from vmcs later. 
        VmxDispatchCpuid, //<----------------4.2 Finish
        &Trap);
  if (!NT_SUCCESS (Status)) 
  {
    DbgPrint("VmxRegisterTraps(): Failed to register VmxDispatchCpuid with status 0x%08hX\n", Status);
    return Status;
  }
  TrRegisterTrap (Cpu, Trap);//<----------------4.3//Finish

  //  Status = TrInitializeGeneralTrap (
  //      Cpu, 
  //      EXIT_REASON_MSR_READ, 
  //      0, // length of the instruction, 0 means length need to be get from vmcs later. 
  //      VmxDispatchMsrRead, 
  //      &Trap);
  //if (!NT_SUCCESS (Status)) 
  //{
  //  _KdPrint (("VmxRegisterTraps(): Failed to register VmxDispatchMsrRead with status 0x%08hX\n", Status));
  //  return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //Status = TrInitializeGeneralTrap (
  //    Cpu, 
  //    EXIT_REASON_MSR_WRITE, 
  //    0,   // length of the instruction, 0 means length need to be get from vmcs later. 
  //    VmxDispatchMsrWrite, 
  //    &Trap);
  //if (!NT_SUCCESS (Status)) 
  //{
  //  _KdPrint (("VmxRegisterTraps(): Failed to register VmxDispatchMsrWrite with status 0x%08hX\n", Status));
  //  return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //Status = TrInitializeGeneralTrap (
  //    Cpu, 
  //    EXIT_REASON_CR_ACCESS, 
  //    0,  // length of the instruction, 0 means length need to be get from vmcs later. 
  //    VmxDispatchCrAccess, 
  //    &Trap);
  //if (!NT_SUCCESS (Status)) 
  //{
  //  _KdPrint (("VmxRegisterTraps(): Failed to register VmxDispatchCrAccess with status 0x%08hX\n", Status));
  //  return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //Status = TrInitializeGeneralTrap (
  //    Cpu, 
  //    EXIT_REASON_INVD, 
  //    0,  // length of the instruction, 0 means length need to be get from vmcs later. 
  //    VmxDispatchINVD, 
  //    &Trap);
  //if (!NT_SUCCESS (Status)) 
  //{
  //  _KdPrint (("VmxRegisterTraps(): Failed to register VmxDispatchINVD with status 0x%08hX\n", Status));
  //  return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //Status = TrInitializeGeneralTrap (
  //    Cpu, 
  //    EXIT_REASON_EXCEPTION_NMI, 
  //    0,  // length of the instruction, 0 means length need to be get from vmcs later. 
  //    VmxDispatchPageFault, 
  //    &Trap);
  //if (!NT_SUCCESS (Status)) 
  //{
  //  _KdPrint (("VmxRegisterTraps(): Failed to register VmxDispatchPageFault with status 0x%08hX\n", Status));
  //  return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //// set dummy handler for all VMX intercepts if we compile wihtout nested support
  //for (i = 0; i < sizeof (TableOfVmxExits) / sizeof (ULONG32); i++) 
  //{
  //    Status = TrInitializeGeneralTrap (
  //        Cpu, 
  //        TableOfVmxExits[i], 
  //        0,    // length of the instruction, 0 means length need to be get from vmcs later. 
  //        VmxDispatchVmxInstrDummy, 
  //        &Trap);
  //  if (!NT_SUCCESS (Status)) 
  //  {
  //    _KdPrint (("VmxRegisterTraps(): Failed to register VmxDispatchVmon with status 0x%08hX\n", Status));
  //    return Status;
  //  }
  //  TrRegisterTrap (Cpu, Trap);
  //}

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
)//Finished
{
  ULONG32 fn, eax, ebx, ecx, edx;
  ULONG inst_len;

  if (!Cpu || !GuestRegs)
    return TRUE;
  fn = GuestRegs->eax;

#if DEBUG_LEVEL>1
  DbgPrint("Helloworld:VmxDispatchCpuid(): Passing in Value(Fn): 0x%x\n", fn);
#endif

  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  if (Trap->General.RipDelta == 0)
    Trap->General.RipDelta = inst_len;

  if (fn == BP_KNOCK_EAX) 
  {
    DbgPrint("Helloworld:Magic knock received: %p\n", BP_KNOCK_EAX);
    GuestRegs->eax = BP_KNOCK_EAX_ANSWER;
	GuestRegs->ebx = BP_KNOCK_EBX_ANSWER;
	GuestRegs->edx = BP_KNOCK_EDX_ANSWER;
    return TRUE;
  }

  ecx = (ULONG) GuestRegs->ecx;
  GetCpuIdInfo (fn, &eax, &ebx, &ecx, &edx);
  GuestRegs->eax = eax;
  GuestRegs->ebx = ebx;
  GuestRegs->ecx = ecx;
  GuestRegs->edx = edx;

  DbgPrint("Helloworld:Missed Magic knock:EXIT_REASON_CPUID fn 0x%x 0x%x 0x%x 0x%x 0x%x \n", fn, eax, ebx, ecx, edx);
  return TRUE;
}
