#pragma once
#include <ntddk.h>
#include "HvStrategies.h"
/**
 * effects: Initialize the memory manager.
 */
NTSTATUS NTAPI HvMmInitManager (
);

/**
 * effects: Allocate <uNumberOfPages> pages from memory.
 */
PVOID NTAPI HvMmAllocatePages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  ULONG uDebugTag,
  PALLOCATED_PAGE * pAllocatedPage
);

/**
 * effects: Allocate Contiguous Pages from memory.
 */
PVOID NTAPI HvMmAllocateContiguousPages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  PALLOCATED_PAGE * pAllocatedPage //Out, return pointer of *AllocatePage Here.
);

/**
 * effects: Allocate Contiguous Pages from memory with the indicated cache strategy.
 */
PVOID NTAPI HvMmAllocateContiguousPagesSpecifyCache (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  ULONG CacheType,
  PALLOCATED_PAGE * pAllocatedPage
);

/**
 * effects: Shutdown the memory manager.
 */
NTSTATUS NTAPI HvMmShutdownManager (
);
