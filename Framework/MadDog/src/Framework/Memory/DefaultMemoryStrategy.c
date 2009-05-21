#include "HvCore.h"

#ifdef USE_MEMORY_DEFAULT_STRATEGY

/**
 * effects: Initialize the memory manager.
 */
NTSTATUS NTAPI HvMmInitManager (
)
{
	Print(("Do Nothing To Initialize Default Memory Manager"));
	return STATUS_SUCCESS;
}

/**
 * effects: Allocate <uNumberOfPages> pages from memory.
 */
PVOID NTAPI HvMmAllocatePages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  ULONG uDebugTag
)
{
  PVOID PageVA, FirstPage;
  PHYSICAL_ADDRESS PagePA;
  NTSTATUS Status;
  ULONG i;

  if (!uNumberOfPages)
    return NULL;

  FirstPage = PageVA = ExAllocatePoolWithTag (NonPagedPool, uNumberOfPages * PAGE_SIZE, uDebugTag);
  if (!PageVA)
    return NULL;
  RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);

  if (pFirstPagePA)
    *pFirstPagePA = MmGetPhysicalAddress (PageVA);

  return FirstPage;
}

/**
 * effects: Allocate Contiguous Pages from memory.
 */
PVOID NTAPI HvMmAllocateContiguousPages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA
)
{
    return HvMmAllocateContiguousPagesSpecifyCache(
        uNumberOfPages,
        pFirstPagePA,
        MmCached);
}

/**
 * effects: Allocate Contiguous Pages from memory with the indicated cache strategy.
 */
PVOID NTAPI HvMmAllocateContiguousPagesSpecifyCache (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  ULONG CacheType
)
{
  PVOID PageVA, FirstPage;
  PHYSICAL_ADDRESS PagePA, l1, l2, l3;
  NTSTATUS Status;
  ULONG i;

  if (!uNumberOfPages)
    return NULL;

  l1.QuadPart = 0;
  l2.QuadPart = -1;
  l3.QuadPart = 0x200000;    // 0x10000 ?

  FirstPage = PageVA = MmAllocateContiguousMemorySpecifyCache (
      uNumberOfPages * PAGE_SIZE, 
      l1, 
      l2, 
      l3, 
      CacheType);
  if (!PageVA)
    return NULL;

  RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);

  PagePA = MmGetPhysicalAddress (PageVA);
  if (pFirstPagePA)
    *pFirstPagePA = PagePA;

  return FirstPage;
}

/**
 * effects: Shutdown the memory manager.
 */
NTSTATUS NTAPI HvMmShutdownManager (
)
{
	Print(("Do Nothing To Initialize Default Memory Manager"));
	return STATUS_SUCCESS;
}

/**
 * effects: Return the value of Host CR3
 */
ULONG NTAPI HvMmGetHostCR3 (
)
{
	return RegGetCr3();
}

/**
 * effects: Return the origin value of Guest CR3 before install the hypervisor
 */
ULONG NTAPI HvMmGetOriginGuestCR3 (
)
{
	return RegGetCr3();
}

#endif