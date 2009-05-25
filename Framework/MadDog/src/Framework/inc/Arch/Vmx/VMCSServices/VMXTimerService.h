#pragma once

#include <ntddk.h>

VOID HvVMXSetTimerInterval(
	PVOID Vmcs,
	ULONG32 Ticks,
	ULONG Radio
);