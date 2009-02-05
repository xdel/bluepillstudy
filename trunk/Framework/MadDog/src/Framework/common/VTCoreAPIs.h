#pragma once

#include <ntddk.h>
#include "VTCoreTypes.h"

#define DEBUG_VTCORE

#ifdef DEBUG_VTCORE
#include "VTCoreDebugger.h"
#endif
//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Install HelloWorld VMM hypervisor.
 */
NTSTATUS NTAPI InstallHypervisor();

/**
 * effects: Uninstall HelloWorld VMM Hypervisor
 */
NTSTATUS NTAPI UninstallHypervisor();

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
NTSTATUS NTAPI HypervisorInit();

NTSTATUS HvmSetupVMControlBlock (
    PCPU Cpu,
    PVOID GuestEip,
    PVOID GuestEsp
);