#pragma once

#include <ntddk.h>

/*
 * effects: This service introduced in VMX Preemption
 * Timer function to the VMCS.
 */
VOID HvVMXSetTimerInterval(
	ULONG32 Ticks,
	ULONG Ratio
);