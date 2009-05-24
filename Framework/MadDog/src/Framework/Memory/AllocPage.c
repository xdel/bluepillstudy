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