#pragma once

#include <ntddk.h>
#include "vmx.h" 
#include "common.h"
#include "regs.h"
#include "memory.h"
#include "msr.h"
//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
#define	HOST_STACK_SIZE_IN_PAGES	16

#define BP_GDT_LIMIT	0x6f
#define BP_IDT_LIMIT	0xfff

#define	ARCH_VMX	2

//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

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



PHVM_DEPENDENT Hvm;
extern HVM_DEPENDENT Vmx;


//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: install our VM root hypervisor on the fly.
 */
NTSTATUS NTAPI HvmSwallowBluepill();

/**
 * effects:Ð¶ÔØHelloWorld VM
 */
NTSTATUS NTAPI HvmSpitOutBluepill();

/**
 * Initialize Hvm Core.e.g. global lock
 */
NTSTATUS NTAPI HvmInit();

/**
 * Check if this cpu supports Intel VT Technology.
 */
BOOLEAN NTAPI HvmSupport();

//+++++++++++++++++++++Static Functions++++++++++++++++++++++++