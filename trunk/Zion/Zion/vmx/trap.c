#include <vmx/trap.h>

extern PHVM_DEPENDENT Hvm;

ZVMSTATUS ZVMAPI TrInitializeGeneralTrap (
  PCPU Cpu,
  uint32_t TrappedVmExit,
  uint8_t RipDelta,
  NBP_TRAP_CALLBACK TrapCallback,
  PNBP_TRAP * pInitializedTrap
)
{
  PNBP_TRAP Trap;

  if (!Cpu || !TrapCallback || !Hvm->ArchIsTrapValid (TrappedVmExit) || !pInitializedTrap)
    //return STATUS_INVALID_PARAMETER;
	return -1;
	
  Trap = (PNBP_TRAP)MmAllocPages(BYTES_TO_PAGES(sizeof (NBP_TRAP)), NULL);
  if (!Trap) {
    cprintf ("TrInitializeGeneralTrap(): Failed to allocate NBP_TRAP structure (%d bytes)\n");
    return -1;
  }

  //RtlZeroMemory (Trap, sizeof (NBP_TRAP));

  Trap->TrapType = TRAP_GENERAL;
  Trap->General.TrappedVmExit = TrappedVmExit;
  Trap->General.RipDelta = RipDelta;
  Trap->TrapCallback = TrapCallback;

  *pInitializedTrap = Trap;

  return ZVMSUCCESS;
}


ZVMSTATUS ZVMAPI TrRegisterTrap (
  PCPU Cpu,
  PNBP_TRAP Trap
)
{
  PZION_LIST_ENTRY TrapList;

  if (!Cpu || !Trap)
    return ZVM_INVALID_PARAMETER;

  switch (Trap->TrapType) {
  case TRAP_GENERAL:
    TrapList = &Cpu->GeneralTrapsList;
    break;
  case TRAP_MSR:
    TrapList = &Cpu->MsrTrapsList;
    break;
  case TRAP_IO:
    TrapList = &Cpu->IoTrapsList;
    break;
  default:
    cprintf("TrRegisterTrap(): Unknown TRAP_TYPE code: %d\n");
    return ZVM_UNSUCCESSFUL;
  }

  InsertTailList (TrapList, &Trap->le);
  return ZVMSUCCESS;
}


ZVMSTATUS ZVMAPI TrFindRegisteredTrap (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  uint64_t exitcode,
  PNBP_TRAP * pTrap
)
{
  TRAP_TYPE TrapType;
  PZION_LIST_ENTRY TrapList;
  PNBP_TRAP Trap;

  if (!Cpu || !GuestRegs || !pTrap)
  {
	cprintf("ZVM_INVALID_PARAMETER\n");
    return ZVM_INVALID_PARAMETER;
  }
  
    TrapType = TRAP_GENERAL;
    TrapList = &Cpu->GeneralTrapsList;
	///cprintf("TrapList is 0x%x\n", TrapList);

  Trap = (PNBP_TRAP) TrapList->Flink;
  while (Trap != (PNBP_TRAP) TrapList) 
  {
	  ///cprintf("looking for trap in while...\n");
      if ((Trap->TrapType == TRAP_GENERAL) && (Trap->General.TrappedVmExit == exitcode)) 
	  {

        *pTrap = Trap;
        return ZVMSUCCESS;
      }
    Trap = (PNBP_TRAP) Trap->le.Flink;
   }
    
  ///cprintf(" Didn't Got it!\n");
  ///return ZVMSUCCESS;
  return ZVM_UNSUCCESSFUL;
  //return STATUS_NOT_FOUND;
}


ZVMSTATUS ZVMAPI TrExecuteGeneralTrapHandler (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  bool WillBeAlsoHandledByGuestHv
)
{

  ///cprintf("Executing trap...\n");
  if (!Cpu || !GuestRegs || !Trap || (Trap->TrapType != TRAP_GENERAL))
    return ZVM_INVALID_PARAMETER;

  if (Trap->TrapCallback (Cpu, GuestRegs, Trap, WillBeAlsoHandledByGuestHv)) {
    // trap handler wants us to adjust guest's RIP
    Hvm->ArchAdjustRip (Cpu, GuestRegs, Trap->General.RipDelta);
  }

  return ZVMSUCCESS;
}
