#pragma once

#include <ntddk.h>
#include "HvCore.h"

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
//Command Definitions
#define SNPROTECTOR_VERIFY		1000 //Used to tell the hypervisor to start run the target program
#define SNPROTECTOR_HIDEDRV 	2000 //Used to hide the hypervisor code in the kernel space
#define SNPROTECTOR_UNHIDEDRV 	2500 //Used to reveal the hypervisor code in the kernel space
//Verification Parameter
typedef struct _Parameter
{
	CHAR UserName[4];
	CHAR SerialNumber[4];
} Parameter,*PParameter;

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

