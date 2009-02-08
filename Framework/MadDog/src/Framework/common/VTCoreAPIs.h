#pragma once

#include <ntddk.h>
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

NTSTATUS HvmSetupVMControlBlock (
    PCPU Cpu,
    PVOID GuestEip,
    PVOID GuestEsp
);