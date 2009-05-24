//#include "Memory/Memory.h"
#include "HvCore.h"
#include <ntddk.h>

//+++++++++++++++Public Definitions+++++++++++++++++

typedef VOID (NTAPI * MmAPMCALLBACK_PROC) (
	PALLOCATED_PAGE Param,
	... //Pass in arguments
);

//+++++++++++++++Public Functions+++++++++++++++++

/**
 * Init AllocPage Manager.
 */
NTSTATUS NTAPI MmAPMInit (
);

/**
 * Finalize AllocPage Manager.
 */
NTSTATUS NTAPI MmAPMFinalize (
);

/**
 * Pop up each AllocatePage element and operates on it.
 *  Each<AllocatedPage> will be freed automatically.
 */
NTSTATUS NTAPI MmAPMPopDoCallback(
	MmAPMCALLBACK_PROC CallBack,
	... //Pass in arguments
);

/**
 * Do operations on each of the AllocatePage elements.
 */
NTSTATUS NTAPI MmAPMDoCallback(
	MmAPMCALLBACK_PROC CallBack,
	... //Pass in arguments
);

NTSTATUS NTAPI MmAPMSavePage (
  PHYSICAL_ADDRESS PhysicalAddress, //Machine Physical Address
  PVOID HostAddress, //Guest Physical Address
  PVOID GuestAddress, //Guest Virtual Address
  PAGE_ALLOCATION_TYPE AllocationType, 
  ULONG uNumberOfPages,
  ULONG Flags,
  PALLOCATED_PAGE *pAllocatedPage
);

NTSTATUS NTAPI MmAPMFindPageByHostVA (
  PVOID HostAddress,
  PALLOCATED_PAGE * pAllocatedPage
);

NTSTATUS NTAPI MmAPMFindPageByPA (
  PHYSICAL_ADDRESS PhysicalAddress,
  PALLOCATED_PAGE * pAllocatedPage
);