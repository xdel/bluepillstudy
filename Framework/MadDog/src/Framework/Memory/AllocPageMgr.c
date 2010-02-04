/* 
 * Copyright holder: Invisible Things Lab
 * 
 * This software is protected by domestic and International
 * copyright laws. Any use (including publishing and
 * distribution) of this software requires a valid license
 * from the copyright holder.
 *
 * This software is provided for the educational use only
 * during the Black Hat training. This software should not
 * be used on production systems.
 *
 */
 
 /* Copyright (C) 2010 Trusted Computing Lab in Shanghai Jiaotong University
 * 
 * 09/10/11	Miao Yu <superymkfounder@hotmail.com>
 */

#include "AllocPageMgr.h"
#include <stdarg.h>

static LIST_ENTRY g_PageTableList;
static KSPIN_LOCK g_PageTableListLock;

/**
 * Init AllocPage Manager.
 */
NTSTATUS NTAPI MmAPMInit (
)
{
	InitializeListHead (&g_PageTableList);
	KeInitializeSpinLock (&g_PageTableListLock);
	return STATUS_SUCCESS;
}

/**
 * Finalize AllocPage Manager.
 */
NTSTATUS NTAPI MmAPMFinalize (
)
{
	PALLOCATED_PAGE AllocatedPage;
	while (AllocatedPage = (PALLOCATED_PAGE) ExInterlockedRemoveHeadList (&g_PageTableList, &g_PageTableListLock))
	{
        AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);
		ExFreePool (AllocatedPage);
	}
	return STATUS_SUCCESS;
}

/**
 * Pop up each AllocatePage element and operates on it.
 *  Each<AllocatedPage> will be freed automatically.
 */
NTSTATUS NTAPI MmAPMPopDoCallback(
	MmAPMCALLBACK_PROC CallBack,
	... //Pass in arguments
)
{
	PALLOCATED_PAGE AllocatedPage;
	va_list args;

	va_start (args, CallBack);

	while (AllocatedPage = (PALLOCATED_PAGE) ExInterlockedRemoveHeadList (&g_PageTableList, &g_PageTableListLock))
	{
        AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);

		CallBack(AllocatedPage, args);

		ExFreePool (AllocatedPage);
	}
	return STATUS_SUCCESS;
}

/**
 * Do operations on each of the AllocatePage elements, Thread-safe execution.
 */
NTSTATUS NTAPI MmAPMDoCallback(
	MmAPMCALLBACK_PROC CallBack,
	... //Pass in arguments
)
{
	PALLOCATED_PAGE AllocatedPage;
	KIRQL OldIrql;
	va_list args;

	va_start (args, CallBack);
	KeAcquireSpinLock (&g_PageTableListLock, &OldIrql);

	AllocatedPage = (PALLOCATED_PAGE)g_PageTableList.Flink;
	while(AllocatedPage != (PALLOCATED_PAGE)&g_PageTableList)
	{
		AllocatedPage = CONTAINING_RECORD(AllocatedPage,ALLOCATED_PAGE,le);
		
		CallBack(AllocatedPage, args);

		AllocatedPage=(PALLOCATED_PAGE)AllocatedPage->le.Flink;
	}

	KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI MmAPMSavePage (
  PHYSICAL_ADDRESS PhysicalAddress, //Machine Physical Address
  PVOID HostAddress, //Guest Physical Address
  PVOID GuestAddress, //Guest Virtual Address
  PAGE_ALLOCATION_TYPE AllocationType, 
  ULONG uNumberOfPages,
  ULONG Flags,
  PALLOCATED_PAGE *pAllocatedPage
)
{
	PALLOCATED_PAGE AllocatedPage;

	if (!GuestAddress)
	return STATUS_INVALID_PARAMETER;

	AllocatedPage = ExAllocatePoolWithTag (NonPagedPool, sizeof (ALLOCATED_PAGE), LAB_TAG);
	if (!AllocatedPage)
		return STATUS_INSUFFICIENT_RESOURCES;
	RtlZeroMemory (AllocatedPage, sizeof (ALLOCATED_PAGE));

	PhysicalAddress.QuadPart = PhysicalAddress.QuadPart & ALIGN_4KPAGE_MASK;
	HostAddress = (PVOID) ((ULONG) HostAddress & ALIGN_4KPAGE_MASK);

	AllocatedPage->AllocationType = AllocationType;
	AllocatedPage->PhysicalAddress = PhysicalAddress;
	AllocatedPage->HostAddress = HostAddress;
	AllocatedPage->GuestAddress = GuestAddress;
	AllocatedPage->uNumberOfPages = uNumberOfPages;
	AllocatedPage->Flags = Flags;

	#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY
	AllocatedPage->bIsCurrentlyHided = FALSE; //Hypervisor doesn't hide this page yet.
	AllocatedPage->bRequireHiding = TRUE; //Hypervisor doesn't hide this page yet.
	AllocatedPage->FromCR3 = RegGetCr3();
	#endif
	
	ExInterlockedInsertTailList (&g_PageTableList, &AllocatedPage->le, &g_PageTableListLock);

	/*
	 DbgPrint("MmSavePage(): PA 0x%X, HostVA 0x%p, GuestVA 0x%p, AT %d, FL 0x%X\n",
	 PhysicalAddress.QuadPart,
	 HostAddress,
	 GuestAddress,
	 AllocationType,
	 Flags);
	*/
	if(pAllocatedPage)
		*pAllocatedPage = AllocatedPage;
	return STATUS_SUCCESS;
}


NTSTATUS NTAPI MmAPMFindPageByPA (
  PHYSICAL_ADDRESS PhysicalAddress,
  PALLOCATED_PAGE * pAllocatedPage
)
{
	PALLOCATED_PAGE AllocatedPage;
	KIRQL OldIrql;

	if (!pAllocatedPage)
	return STATUS_INVALID_PARAMETER;

	KeAcquireSpinLock (&g_PageTableListLock, &OldIrql);

	PhysicalAddress.QuadPart = PhysicalAddress.QuadPart & ALIGN_4KPAGE_MASK;

	AllocatedPage = (PALLOCATED_PAGE) g_PageTableList.Flink;
	while (AllocatedPage != (PALLOCATED_PAGE) & g_PageTableList) 
	{
		AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);

		if (AllocatedPage->PhysicalAddress.QuadPart == PhysicalAddress.QuadPart) 
		{
			*pAllocatedPage = AllocatedPage;
			KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
			return STATUS_SUCCESS;
		}

		AllocatedPage = (PALLOCATED_PAGE) AllocatedPage->le.Flink;
	}

	KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS NTAPI MmAPMFindPageByHostVA (
  PVOID HostAddress,
  PALLOCATED_PAGE * pAllocatedPage
)
{
	PALLOCATED_PAGE AllocatedPage;
	KIRQL OldIrql;

	if (!pAllocatedPage)
	return STATUS_INVALID_PARAMETER;

	KeAcquireSpinLock (&g_PageTableListLock, &OldIrql);

	HostAddress = (PVOID) ((ULONG) HostAddress & ALIGN_4KPAGE_MASK);

	AllocatedPage = (PALLOCATED_PAGE) g_PageTableList.Flink;
	while (AllocatedPage != (PALLOCATED_PAGE) & g_PageTableList) 
	{
		AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);

		if (AllocatedPage->HostAddress == HostAddress) 
		{
			*pAllocatedPage = AllocatedPage;
			KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
			return STATUS_SUCCESS;
		}

		AllocatedPage = (PALLOCATED_PAGE) AllocatedPage->le.Flink;
	}

	KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
	return STATUS_UNSUCCESSFUL;
}