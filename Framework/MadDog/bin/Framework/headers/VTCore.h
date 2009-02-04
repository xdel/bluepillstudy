#pragma once

#include <ntddk.h>

#define DEBUG_VTCORE

#ifdef DEBUG_VTCORE
#include "VTCoreDebugger.h"
#endif
//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Install our VM  hypervisor on the fly.
 */
NTSTATUS NTAPI InstallVMM();

/**
 * effects: Uninstall HelloWorld VMM
 */
NTSTATUS NTAPI UninstallVMM();

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
NTSTATUS NTAPI VMMInit();