#pragma once
#include <ntddk.h>

#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY

/**
 * effects:Hide the hidden physical pages from the guest page table.
 */
VOID NTAPI MmHidingStrategyHideGuestPages (
	PVOID GuestAddress,
	ULONG uNumberOfPages
);

/**
 * effects: remap back the indicated hidden physical pages to the guest page table.
 */
VOID NTAPI MmHidingStrategyRevealHiddenPages (
	PVOID GuestAddress,
	ULONG uNumberOfPages
);

/**
 * effects:Hide all the hypervisor allocated physical pages from the guest page table.
 */
VOID NTAPI MmHidingStrategyHideAllAllocatedGuestPages (
);
#endif