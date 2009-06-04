#include "HvCore.h"
#include "AllocPageMgr.h"

#ifdef USE_MEMORY_DEFAULT_STRATEGY

/**
 * effects: Initialize the memory manager.
 */
NTSTATUS NTAPI HvMmInitManager (
)
{
	NTSTATUS Status;

	Status = MmAPMInit();
	return Status;
}

/**
 * effects: Allocate <uNumberOfPages> pages from memory.
 */
PVOID NTAPI HvMmAllocatePages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  ULONG uDebugTag,
  PALLOCATED_PAGE * pAllocatedPage //Always point to the <AllocatedPage> of first page..
)
{
	PVOID PageVA, FirstPage;
	PHYSICAL_ADDRESS PagePA;
	NTSTATUS Status;
	ULONG i;
	PALLOCATED_PAGE Alloced = NULL;

	if (!uNumberOfPages)
	return NULL;

	FirstPage = PageVA = ExAllocatePoolWithTag (NonPagedPool, uNumberOfPages * PAGE_SIZE, uDebugTag);
	if (!PageVA)
		return NULL;

	if (pFirstPagePA)
		*pFirstPagePA = MmGetPhysicalAddress (PageVA);

	RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);

	//Step 2. Construct <AllocatedPage> Struct.
	for (i = 0; i < uNumberOfPages; i++) 
	{
		// save pages
		PagePA = MmGetPhysicalAddress (PageVA);
		Status = MmAPMSavePage (
			PagePA, 
			PageVA, 
			PageVA, 
			i == 0 ? PAT_POOL : PAT_DONT_FREE, 
			uNumberOfPages, 
			0,
			&Alloced);

		if(i==0 && pAllocatedPage)
			*pAllocatedPage = Alloced;
		
		PageVA = (PUCHAR) PageVA + PAGE_SIZE;
	}

	

	

	return FirstPage;
}

/**
 * effects: Allocate Contiguous Pages from memory.
 */
PVOID NTAPI HvMmAllocateContiguousPages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  PALLOCATED_PAGE * pAllocatedPage //Always point to the <AllocatedPage> of first page.
)
{
    return HvMmAllocateContiguousPagesSpecifyCache(
        uNumberOfPages,
        pFirstPagePA,
        MmCached,
		pAllocatedPage);
}

/**
 * effects: Allocate Contiguous Pages from memory with the indicated cache strategy.
 */
PVOID NTAPI HvMmAllocateContiguousPagesSpecifyCache (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  ULONG CacheType,
  PALLOCATED_PAGE * pAllocatedPage //Always point to the  <AllocatedPage> of first page.
)
{
	PVOID PageVA, FirstPage;
	PHYSICAL_ADDRESS PagePA, l1, l2, l3;
	NTSTATUS Status;
	ULONG i;
	PALLOCATED_PAGE Alloced = NULL;

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
	
	PagePA = MmGetPhysicalAddress (PageVA);
	if (pFirstPagePA)
		*pFirstPagePA = PagePA;

	RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);

	//Step 2. Construct <AllocatedPage> Struct.
	for (i = 0; i < uNumberOfPages; i++) 
	{
		// save pages
		Status = MmAPMSavePage (
			PagePA, 
			PageVA, 
			PageVA, 
			i == 0 ? PAT_CONTIGUOUS : PAT_DONT_FREE, 
			uNumberOfPages, 
			0,
			&Alloced);

		if(i==0)
			*pAllocatedPage = Alloced;
		
		PageVA = (PUCHAR) PageVA + PAGE_SIZE;
		PagePA.QuadPart += PAGE_SIZE;
	}

	
	return FirstPage;
}

/**
 * effects: Shutdown the memory manager.
 */
NTSTATUS NTAPI HvMmShutdownManager (
)
{
	NTSTATUS Status;

	Status = MmAPMFinalize();
	
	return Status;
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