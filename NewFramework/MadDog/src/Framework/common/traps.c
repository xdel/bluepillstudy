#include "traps.h"

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects:Build and Initialize General Trap struct (which is also a Trap struct).
 */
NTSTATUS NTAPI TrInitializeGeneralTrap (
    PCPU Cpu,
    ULONG TrappedVmExit,
    UCHAR RipDelta,
    NBP_TRAP_CALLBACK TrapCallback,
    PNBP_TRAP *pInitializedTrap
)
{//Finish
    PNBP_TRAP Trap;
	Print(("HelloWorld:TrInitializeGeneralTrap():TrappedVmExit 0x%x\n", TrappedVmExit));

    if (!Cpu || 
        !TrapCallback || 
        !Hvm->ArchIsTrapValid (TrappedVmExit) ||//<----------------5.1 Finish
        !pInitializedTrap)
    {
        return STATUS_INVALID_PARAMETER;
    }

    Trap = MmAllocatePages (BYTES_TO_PAGES (sizeof (NBP_TRAP)), NULL);
    if (!Trap) 
    {
        Print(("HelloWorld:TrInitializeGeneralTrap(): Failed to allocate NBP_TRAP structure (%d bytes)\n", sizeof (NBP_TRAP)));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (Trap, sizeof (NBP_TRAP));

    Trap->TrapType = TRAP_GENERAL;
    Trap->General.TrappedVmExit = TrappedVmExit;
    Trap->General.RipDelta = RipDelta;
    Trap->TrapCallback = TrapCallback;

    *pInitializedTrap = Trap;

    return STATUS_SUCCESS;
}

/**
 * effects: Register trap struct.
 */
NTSTATUS NTAPI TrRegisterTrap (
  PCPU Cpu,
  PNBP_TRAP Trap
)
{//Finished
  PLIST_ENTRY TrapList;

  if (!Cpu || !Trap)
    return STATUS_INVALID_PARAMETER;

  switch (Trap->TrapType) {
  case TRAP_GENERAL:
    TrapList = &Cpu->GeneralTrapsList;
    break;
  case TRAP_MSR:
    TrapList = &Cpu->MsrTrapsList;
    break;
  //case TRAP_IO:
  //  TrapList = &Cpu->IoTrapsList;
  //  break;
  default:
	  Print(("Helloworld:TrRegisterTrap(): Unknown TRAP_TYPE code: %d\n", (char) Trap->TrapType));
    return STATUS_UNSUCCESSFUL;
  }

  InsertTailList (TrapList, &Trap->le);
  return STATUS_SUCCESS;
}

NTSTATUS NTAPI TrExecuteGeneralTrapHandler (
    PCPU Cpu,
    PGUEST_REGS GuestRegs,
    PNBP_TRAP Trap,
    BOOLEAN WillBeAlsoHandledByGuestHv
)
{//Finished
    if (!Cpu || !GuestRegs || !Trap || (Trap->TrapType != TRAP_GENERAL))
        return STATUS_INVALID_PARAMETER;

    if (Trap->TrapCallback (Cpu, GuestRegs, Trap, WillBeAlsoHandledByGuestHv)) 
    {
        // trap handler wants us to adjust guest's RIP
        Hvm->ArchAdjustRip(Cpu, GuestRegs, Trap->General.RipDelta);
    }

    return STATUS_SUCCESS;
}
/**
 * Search Registered Traps
 * ²éÑ¯ÒÑ×¢²áµÄTraps
 */
NTSTATUS NTAPI TrFindRegisteredTrap (
    PCPU Cpu,
    PGUEST_REGS GuestRegs,
    ULONG exitcode,
    PNBP_TRAP *pTrap
)
{//Finished
    TRAP_TYPE TrapType;
    PLIST_ENTRY TrapList;
    PNBP_TRAP Trap;

    if (!Cpu || !GuestRegs || !pTrap)
        return STATUS_INVALID_PARAMETER;

    TrapType = TRAP_GENERAL;
    TrapList = &Cpu->GeneralTrapsList;

    Trap = (PNBP_TRAP) TrapList->Flink;
    while (Trap != (PNBP_TRAP) TrapList) 
    {
        Trap = CONTAINING_RECORD (Trap, NBP_TRAP, le);

        if ((Trap->TrapType == TrapType) && Trap->TrapCallback) 
        {

            if ((Trap->TrapType == TRAP_MSR) 
                && (Trap->Msr.TrappedMsr == (ULONG32)(GuestRegs->ecx))) 
            {
                // return the Trap even if doesn't intercept current access for this MSR
                // (e.g. TrappedMsrAccess==MSR_INTERCEPT_READ and trapped instructions is WRMSR)
                *pTrap = Trap;
                return STATUS_SUCCESS;
            }

            if ((Trap->TrapType == TRAP_GENERAL) 
                && (Trap->General.TrappedVmExit == exitcode)) 
            {
                *pTrap = Trap;
                return STATUS_SUCCESS;
            }

        }
        Trap = (PNBP_TRAP) Trap->le.Flink;
    }

    return STATUS_NOT_FOUND;
}
