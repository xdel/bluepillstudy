#pragma once
#include <ntddk.h>

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
  ULONG uDebugTag
);

/**
 * effects: Allocate Contiguous Pages from memory.
 */
PVOID NTAPI HvMmAllocateContiguousPages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA
);

/**
 * effects: Allocate Contiguous Pages from memory with the indicated cache strategy.
 */
PVOID NTAPI HvMmAllocateContiguousPagesSpecifyCache (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  ULONG CacheType
);

/**
 * effects: Shutdown the memory manager.
 */
NTSTATUS NTAPI HvMmShutdownManager (
);