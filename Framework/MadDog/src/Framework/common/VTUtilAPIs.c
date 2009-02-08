#include "VTUtilAPIs.h"
#include "cpuid.h"
/**
 * effects:A Default Implementation of Initialize Segment Selector.
 */
NTSTATUS NTAPI MadDog_InitializeSegmentSelector (
    SEGMENT_SELECTOR *pSegmentSelector,
    USHORT Selector,
    PUCHAR GdtBase
)
{//Finished
    PSEGMENT_DESCRIPTOR SegDesc;

    if (!pSegmentSelector)
        return STATUS_INVALID_PARAMETER;

    if (Selector & 0x4) 
    {
		KdPrint(("Helloworld:CmInitializeSegmentSelector(): Given selector (0x%X) points to LDT\n", Selector));
        return STATUS_INVALID_PARAMETER;
    }

    SegDesc = (PSEGMENT_DESCRIPTOR) ((PUCHAR) GdtBase + (Selector & ~0x7));

    pSegmentSelector->sel = Selector;
    pSegmentSelector->base = SegDesc->base0 | SegDesc->base1 << 16 | SegDesc->base2 << 24;
    pSegmentSelector->limit = SegDesc->limit0 | (SegDesc->limit1attr1 & 0xf) << 16;
    pSegmentSelector->attributes.UCHARs = SegDesc->attr0 | (SegDesc->limit1attr1 & 0xf0) << 4;

    //if (!(SegDesc->attr0 & LA_STANDARD)) 
    //{
    //    ULONG64 tmp;
    //    // this is a TSS or callgate etc, save the base high part
    //    tmp = (*(PULONG64) ((PUCHAR) SegDesc + 8));
    //    SegmentSelector->base = (SegmentSelector->base & 0xffffffff) | (tmp << 32);
    //}

    if (pSegmentSelector->attributes.fields.g) 
    {
        // 4096-bit granularity is enabled for this segment, scale the limit
        pSegmentSelector->limit = (pSegmentSelector->limit << 12) + 0xfff;
    }

    return STATUS_SUCCESS;
}

VOID NTAPI MadDog_GetCpuIdInfo (
  ULONG32 fn,
  OUT PULONG32 ret_eax,
  OUT PULONG32 ret_ebx,
  OUT PULONG32 ret_ecx,
  OUT PULONG32 ret_edx
)
{
	GetCpuIdInfo(fn,ret_eax,ret_ebx,ret_ecx,ret_edx);
}

VOID NTAPI MadDog_CpuidWithEcxEdx (
  IN OUT PULONG32 ret_ecx,
  IN OUT PULONG32 ret_edx
)
{
	CpuidWithEcxEdx(ret_ecx,ret_edx);
}
