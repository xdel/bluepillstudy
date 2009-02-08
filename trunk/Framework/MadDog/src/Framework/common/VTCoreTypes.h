#pragma once
#include <ntddk.h>
//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

/* 
* Attribute for segment selector. This is a copy of bit 40:47 & 52:55 of the
* segment descriptor. 
*/
typedef union
{
  USHORT UCHARs;
  struct
  {
    USHORT type:4;              /* 0;  Bit 40-43 */
    USHORT s:1;                 /* 4;  Bit 44 */
    USHORT dpl:2;               /* 5;  Bit 45-46 */
    USHORT p:1;                 /* 7;  Bit 47 */
    // gap!       
    USHORT avl:1;               /* 8;  Bit 52 */
    USHORT l:1;                 /* 9;  Bit 53 */
    USHORT db:1;                /* 10; Bit 54 */
    USHORT g:1;                 /* 11; Bit 55 */
    USHORT Gap:4;
  } fields;
} SEGMENT_ATTRIBUTES;

typedef struct
{
  USHORT sel;
  SEGMENT_ATTRIBUTES attributes;
  ULONG32 limit;
  ULONG64 base;
} SEGMENT_SELECTOR;

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

typedef struct _CPU *PCPU;
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
} CPU;