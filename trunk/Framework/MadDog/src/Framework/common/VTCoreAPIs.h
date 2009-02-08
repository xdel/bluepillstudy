#pragma once

#include <ntddk.h>
#include "VTCoreDefs.h"
#include "VTCoreTypes.h"
#include "Regs.h"
#include "Msr.h"

#define DEBUG_VTCORE

#ifdef DEBUG_VTCORE
#include "VTCoreDebugger.h"
#endif

//+++++++++++++++++++++Structs Definitions+++++++++++++++++++++

/**
 * This struct contains all the callback func pointer.
 * User must initialize this struct first to install MadDog.
 */
typedef struct _MadDog_Control
{
	NTSTATUS (*SetupVMCB)(PCPU Cpu, PVOID GuestEip, PVOID GuestEsp);
	NTSTATUS (*ApplyTraps) (PCPU Cpu);
} MadDog_Control,
 *PMadDog_Control;

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Install HelloWorld VMM hypervisor.
 */
NTSTATUS NTAPI MadDog_InstallHypervisor(PMadDog_Control mdCtl);

/**
 * effects: Uninstall HelloWorld VMM Hypervisor
 */
NTSTATUS NTAPI MadDog_UninstallHypervisor();

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
NTSTATUS NTAPI MadDog_HypervisorInit();

/**
 * effects:Build and Initialize General Trap struct (which is also a Trap struct).
 */
NTSTATUS NTAPI MadDog_InitializeGeneralTrap (
  PCPU Cpu,
  ULONG TrappedVmExit,
  UCHAR RipDelta,
  NBP_TRAP_CALLBACK TrapCallback,
  PNBP_TRAP * pInitializedTrap
);

/**
 * effects: Register trap struct.
 */
NTSTATUS NTAPI MadDog_RegisterTrap (
  PCPU Cpu,
  PNBP_TRAP Trap
);