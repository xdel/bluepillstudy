#pragma once
#include <ntddk.h>
//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

typedef struct _CPU *PCPU;


typedef struct
{
  USHORT limit0;
  USHORT base0;
  UCHAR base1;
  UCHAR attr0;
  UCHAR limit1attr1;
  UCHAR base2;
} SEGMENT_DESCRIPTOR,
 *PSEGMENT_DESCRIPTOR;

typedef struct _VMX
{
  PHYSICAL_ADDRESS VmcsToContinuePA;    // MUST go first in the structure; refer to SvmVmrun() for details
  PVOID _2mbVmcbMap;

  PHYSICAL_ADDRESS OriginalVmcsPA;
  PVOID OriginalVmcs;           // VMCS which was originally built by the BP for the guest OS
  PHYSICAL_ADDRESS OriginalVmxonRPA;    // Vmxon Region which was originally built by the BP for the guest OS
  PVOID OriginaVmxonR;

  //PHYSICAL_ADDRESS IOBitmapAPA; // points to IOBitMapA.
  //PVOID IOBitmapA;

  //PHYSICAL_ADDRESS IOBitmapBPA; // points to IOBitMapB
  //PVOID IOBitmapB;

  PHYSICAL_ADDRESS MSRBitmapPA; // points to MsrBitMap
  PVOID MSRBitmap;

  ULONG GuestCR0;             //Guest's CR0. 
  ULONG GuestCR3;             //Guest's CR3. for storing guest cr3 when guest diasble paging.
  ULONG GuestCR4;             //Guest's CR4. 
  ULONG64 GuestEFER;
  UCHAR GuestStateBeforeInterrupt[0xc00];

} VMX,
 *PVMX;

typedef struct _CPU
{

  PCPU SelfPointer;             // MUST go first in the structure; refer to interrupt handlers for details

  VMX Vmx;

  ULONG ProcessorNumber;
//  ULONG64 TotalTscOffset;

//  LARGE_INTEGER LapicBaseMsr;
//  PHYSICAL_ADDRESS LapicPhysicalBase;
//  PUCHAR LapicVirtualBase;

  LIST_ENTRY GeneralTrapsList;  // list of BP_TRAP structures
  LIST_ENTRY MsrTrapsList;      //
 // LIST_ENTRY IoTrapsList;       //

 // PVOID SparePage;              // a single page which was allocated just to get an unused PTE.
 // PHYSICAL_ADDRESS SparePagePA; // original PA of the SparePage
 // PULONG SparePagePTE;

  PSEGMENT_DESCRIPTOR GdtArea;
  PVOID IdtArea;

    PVOID HostStack;              // note that CPU structure reside in this memory region
 // BOOLEAN Nested;

 // ULONG64 ComPrintLastTsc;
} CPU, *PCPU;

