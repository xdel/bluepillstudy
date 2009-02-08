#pragma once

#include <ntddk.h>
#include "common.h"
#include "hvm.h"

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects:Build and Initialize General Trap struct (which is also a Trap struct).
 */
NTSTATUS NTAPI TrInitializeGeneralTrap (
  PCPU Cpu,
  ULONG TrappedVmExit,
  UCHAR RipDelta,
  NBP_TRAP_CALLBACK TrapCallback,
  PNBP_TRAP * pInitializedTrap
);

/**
 * effects: Register trap struct.
 */
NTSTATUS NTAPI TrRegisterTrap (
  PCPU Cpu,
  PNBP_TRAP Trap
);
/**
 * Search Registered Traps
 * ²éÑ¯ÒÑ×¢²áµÄTraps
 */
NTSTATUS NTAPI TrFindRegisteredTrap (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  ULONG exitcode,
  PNBP_TRAP * pTrap
);

NTSTATUS NTAPI TrExecuteGeneralTrapHandler (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);
