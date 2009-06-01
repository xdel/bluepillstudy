#pragma once
#include <ntddk.h>

//+++++++++++++++++++++Segment Structs++++++++++++++++++++++++++++++++
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

//+++++++++++++++++++++VMX Structs++++++++++++++++++++++++++++++++

//Used to record the virtual-machine extensions support configuation
//on the current platform.
typedef struct _VMXFEATURESMSR
{
	LARGE_INTEGER VmxPinBasedCTLs;
	
    //When reference the value <VmxTruePinBasedCTLs>, 
	//always check the boolean first. 
	BOOLEAN EnableVmxTruePinBasedCTLs;
	LARGE_INTEGER VmxTruePinBasedCTLs;
	

} VMXFEATURESMSR,*PVMXFEATURESMSR;

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

  VMXFEATURESMSR FeaturesMSR;

} VMX,
 *PVMX;

//++++++++++++++Cpu Related Structs(Common Structs)++++++++++++++++
//This struct stores the memory used by both Guest OS and Hypervisor,
//So never applying memory-hiding mechnism to this struct and related memory.
typedef struct _WORMHOLE
{
	ULONG ProcessorNumber;
	UCHAR Trampoline[0x600];
} WORMHOLE,*PWORMHOLE;

typedef struct _CPU
{

	PCPU SelfPointer;             // MUST go first in the structure; refer to interrupt handlers for details
	
	VMX Vmx;

	ULONG ProcessorNumber;

	//  ULONG64 TotalTscOffset;

	//  LARGE_INTEGER LapicBaseMsr;
	//  PHYSICAL_ADDRESS LapicPhysicalBase;
	//  PUCHAR LapicVirtualBase;

	// [Superymk 6/1/2009] Use hash table to store and handle Trap elements 
	//LIST_ENTRY GeneralTrapsList;  // list of BP_TRAP structures
	//LIST_ENTRY MsrTrapsList;      //
	// LIST_ENTRY IoTrapsList;       //
	LIST_ENTRY TrapsList[NUM_VMEXITS];
	// [Superymk 6/1/2009] End

	// PVOID SparePage;              // a single page which was allocated just to get an unused PTE.
	// PHYSICAL_ADDRESS SparePagePA; // original PA of the SparePage
	// PULONG SparePagePTE;

	PSEGMENT_DESCRIPTOR GdtArea;
	PVOID IdtArea;

	PVOID HostStack;              // note that CPU structure reside in this memory region
	PWORMHOLE HypervisorGuestPipe; 
	// BOOLEAN Nested;

	// ULONG64 ComPrintLastTsc;
} CPU;

typedef struct _GUEST_REGS
{
  ULONG32 eax;                  // 0x00         // NOT VALID FOR SVM
  ULONG32 ecx;
  ULONG32 edx;                  // 0x08
  ULONG32 ebx;
  ULONG32 esp;                  // esp is not stored here on SVM
  ULONG32 ebp;
  ULONG32 esi;
  ULONG32 edi;
} GUEST_REGS;

//+++++++++++++++++++++Traps Structs++++++++++++++++++++++++++++++++

// [Superymk 6/1/2009] Store less info in NBP_TRAP
typedef enum
{
  TRAP_DISABLED = 0,
  TRAP_GENERAL = 1,
  TRAP_MSR = 2,
  TRAP_IO = 3
} TRAP_TYPE;

// The following three will be used as trap's data structure.
// 下面的这三个是_NBP_TRAP_中的存放关键数据的数据结构

//typedef struct _NBP_TRAP_DATA_GENERAL
//{
//  ULONG TrappedVmExit;
//  ULONG RipDelta;             // this value will be added to rip to skip the trapped instruction
//} NBP_TRAP_DATA_GENERAL,
// *PNBP_TRAP_DATA_GENERAL;
//
typedef struct _NBP_TRAP_DATA_MSR
{
	ULONG32 TrappedMsr;
	UCHAR TrappedMsrAccess;
	UCHAR GuestTrappedMsrAccess;
} NBP_TRAP_DATA_MSR,
 *PNBP_TRAP_DATA_MSR;

typedef struct _NBP_TRAP_DATA_IO
{
	ULONG TrappedPort;
} NBP_TRAP_DATA_IO,
 *PNBP_TRAP_DATA_IO;
// [Superymk 6/1/2009] End

typedef struct _NBP_TRAP
{
	LIST_ENTRY le;
	
	TRAP_PRIORITY Priority;

	// [Superymk 6/1/2009] Store less info in NBP_TRAP
	TRAP_TYPE TrapType;
	TRAP_TYPE SavedTrapType;
		
	union
	{
		//NBP_TRAP_DATA_GENERAL General;
		NBP_TRAP_DATA_MSR Msr;
		NBP_TRAP_DATA_IO Io;
	};

	ULONG TrappedVmExit;
	ULONG RipDelta; 
	// [Superymk 6/1/2009] End

	NBP_TRAP_CALLBACK TrapCallback;
	BOOLEAN bForwardTrapToGuest;  // FALSE if guest hypervisor doesn't want to intercept this in its own guest.
	// This will be TRUE for TRAP_MSR record when we're going to intercept MSR "rw"
	// but the guest wants to intercept only "r" or "w". 
	// Check Msr.GuestTrappedMsrAccess for correct event forwarding.
} NBP_TRAP,
 *PNBP_TRAP;