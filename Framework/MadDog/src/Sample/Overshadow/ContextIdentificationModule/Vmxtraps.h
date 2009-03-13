#pragma once

#include <ntddk.h>
#include "VTCore.h"
#include "vmx/Vmx.h"
#include "vmx/Vmcs.h"
#include "VTUtilAPIs.h"
//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
#define START_RECORDING_EAX		1000 //Used to tell the hypervisor to start run the target program
#define PING_EAX				1500 //Used to retrieve data while the target program is still running
#define END_RECORDING_EAX		2000 //Used to tell the hypervisor the target program should be killed

#define SysenterCounter_USE_FAKE_TRAP //use fake sysenter trap to count sysenter happenning times.

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Register traps in this function
 * requires: <Cpu> is valid
 */
NTSTATUS NTAPI VmxRegisterTraps (
  PCPU Cpu
);

