#pragma once
#include <ntddk.h>
#include "VTCore.h"

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++
/**
 * effects:A Default Implementation of Initialize Segment Selector.
 */
NTSTATUS NTAPI MadDog_InitializeSegmentSelector (
    SEGMENT_SELECTOR *pSegmentSelector,
    USHORT Selector,
    PUCHAR GdtBase
);