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

#include "HvCore.h"
#include <ntddk.h>

//+++++++++++++++Public Definitions+++++++++++++++++

typedef VOID (NTAPI * MmAPMCALLBACK_PROC) (
	PALLOCATED_PAGE Param,
	va_list argp //Pass in arguments
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