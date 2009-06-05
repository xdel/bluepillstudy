#pragma once

#include <ntddk.h>
#include "HvCore.h"
//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
//Command Definitions
#define SNPROTECTOR_VERIFY	1000 //Used to tell the hypervisor to start run the target program

//Verification Parameter
typedef struct _Parameter
{
	CHAR UserName[4];
	CHAR SerialNumber[4];
} Parameter,*PParameter;

//+++++++++++++++++++Ntifs Definitions+++++++++++++++++++++++
typedef struct _KPROCESS *PKPROCESS ,*PRKPROCESS;

typedef struct _KAPC_STATE {
    LIST_ENTRY ApcListHead[MaximumMode];
    struct _KPROCESS *Process;
    BOOLEAN KernelApcInProgress;
    BOOLEAN KernelApcPending;
    BOOLEAN UserApcPending;
} KAPC_STATE, *PKAPC_STATE, *PRKAPC_STATE;
//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Register traps in this function
 * requires: <Cpu> is valid
 */
NTSTATUS NTAPI VmxRegisterTraps (
  PCPU Cpu
);

BOOLEAN NTAPI VmxDispatchTimerExpired (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

BOOLEAN NTAPI PtVmxDispatchCR3Access (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv,
  ...
);

