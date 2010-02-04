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

#include "Memory/Memory.h"
#include "HvCompileSwitches.h"

#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY

/**
 * effects: Change pAllocatedPage->bRequireHiding to the indicated value.
 * Might use one bit of an integer to present this setting in the future.
 */
VOID NTAPI MmChangeRequireHidingAllocPage(
	PALLOCATED_PAGE pAllocatedPage,
	BOOLEAN bCurrentSetting
)
{
	pAllocatedPage->bRequireHiding = bCurrentSetting;
}

/**
 * effects: Get the value of PALLOCATED_PAGE->bIsCurrentlyHided
 */
BOOLEAN NTAPI MmGetIsCurrentlyHidedAllocPage(
	PALLOCATED_PAGE pAllocatedPage
)
{
	return pAllocatedPage->bIsCurrentlyHided;
}

/**
 * effects: Change the value of PALLOCATED_PAGE->bIsCurrentlyHided
 */
VOID NTAPI MmChangeIsCurrentlyHidedAllocPage(
	PALLOCATED_PAGE pAllocatedPage,
	BOOLEAN bCurrentSetting
)
{
	pAllocatedPage->bIsCurrentlyHided = bCurrentSetting;
}

#endif