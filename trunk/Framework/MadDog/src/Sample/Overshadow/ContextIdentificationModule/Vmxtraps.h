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
#define TEST_PASSINVALUE_EAX	8888 //Only Used in Debug Mode. Hypervisor won't change its current recording state.
//TODO - Set this value on the runtime in the future.
#define CoreCount				32	//Used to define the amount of the CPU core on the target machine, default is 32

//+++++++++++++++++++++Structs+++++++++++++++++++++++++++
typedef struct _Parameter
{
	//monitor the indicated process, 0 means all processes. 
	ULONG32 pid;
} Parameter,*PParameter;

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Register traps in this function
 * requires: <Cpu> is valid
 */
NTSTATUS NTAPI VmxRegisterTraps (
  PCPU Cpu
);
