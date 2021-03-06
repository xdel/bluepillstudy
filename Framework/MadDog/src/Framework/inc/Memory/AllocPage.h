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

#pragma once
#include <ntddk.h>
#include "HvCompileSwitches.h"

typedef enum
{
  PAT_DONT_FREE = 0,
  PAT_POOL,
  PAT_CONTIGUOUS
} PAGE_ALLOCATION_TYPE;

typedef struct _ALLOCATED_PAGE
{

	LIST_ENTRY le;

	ULONG Flags;

	PAGE_ALLOCATION_TYPE AllocationType;
	ULONG uNumberOfPages;         // for PAT_CONTIGUOUS only

	PHYSICAL_ADDRESS PhysicalAddress; //Machine Physical Address
	PVOID HostAddress; //Guest Physical Address
	PVOID GuestAddress;//Guest Virtual Address

	#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY
	BOOLEAN bIsCurrentlyHided;// This flag means if this page is hided currently.
	BOOLEAN bRequireHiding; // if it is true, the page will be hided, otherwise it will never be hided
	ULONG FromCR3; //allocate this memory from which CR3 page table.	 
	#endif

} ALLOCATED_PAGE,
 *PALLOCATED_PAGE;

//++++++++++++++++++++Public Functions+++++++++++++++++++
#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY

/**
 * effects: Change the value of PALLOCATED_PAGE->bRequireHiding
 */
VOID NTAPI MmChangeRequireHidingAllocPage(
	PALLOCATED_PAGE pAllocatedPage,
	BOOLEAN bCurrentSetting
);

/**
 * effects: Get the value of PALLOCATED_PAGE->bIsCurrentlyHided
 */
BOOLEAN NTAPI MmGetIsCurrentlyHidedAllocPage(
	PALLOCATED_PAGE pAllocatedPage
);

/**
 * effects: Change the value of PALLOCATED_PAGE->bIsCurrentlyHided
 */
VOID NTAPI MmChangeIsCurrentlyHidedAllocPage(
	PALLOCATED_PAGE pAllocatedPage,
	BOOLEAN bCurrentSetting
);

#endif