#pragma once
#include <ntddk.h>

/**
 * effects: Return the value of Host CR3
 */
ULONG NTAPI HvMmGetHostCR3 (
);

/**
 * effects: Return the origin value of Guest CR3 before install the hypervisor
 */
ULONG NTAPI HvMmGetOriginGuestCR3 (
);