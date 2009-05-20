#include "HvCore.h"

#ifdef USE_MEMORY_DEFAULT_STRATEGY

/**
 * effects: Allocate <uNumberOfPages> pages from memory.
 */
PVOID NTAPI MmAllocatePages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA
)
{
  PVOID PageVA, FirstPage;
  PHYSICAL_ADDRESS PagePA;
  NTSTATUS Status;
  ULONG i;

  if (!uNumberOfPages)
    return NULL;

  FirstPage = PageVA = ExAllocatePoolWithTag (NonPagedPool, uNumberOfPages * PAGE_SIZE, LAB_TAG);
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
PVOID NTAPI MmAllocateContiguousPages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA
)
{
    return MmAllocateContiguousPagesSpecifyCache(
        uNumberOfPages,
        pFirstPagePA,
        MmCached);
}
/**
 * effects: Allocate Contiguous Pages from memory with the indicated cache strategy.
 */
PVOID NTAPI MmAllocateContiguousPagesSpecifyCache (
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
#endif