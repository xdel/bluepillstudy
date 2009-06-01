#pragma once

#include <ntddk.h>
#include "HvCoreDefs.h"

/*
 * effects: This service introduced in VMX Preemption
 * Timer function to the VMCS.
 * returns: if the <ratio> larger than 31, then returns STATUS_INVALID_PARAMETERS
 */
HVSTATUS PtVmxCPUIDInterception(
	PCPU Cpu,
	ULONG32 Ticks, /* After how many ticks the VMX Timer will be expired, THIS VALUE IS FIXED TO BE 32 BITS LONG*/
	BOOLEAN SaveTimerValueOnVMEXIT,
	NBP_TRAP_CALLBACK TrapCallback /* If this is null, we won't register a callback function*/
);