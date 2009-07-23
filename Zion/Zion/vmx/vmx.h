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

#ifndef ZION_VMX_H
#define ZION_VMX_H

#include <include/types.h>

#define ZVM_SUCCESS(a)	((a) == ZVMSUCCESS)
#define ZVMAPI

/* Success or Error Number*/
#define	TRUE	1
#define	FALSE	0
#define	ZVMSUCCESS	((ZVMSTATUS)0x00000000L)
#define	ZVM_UNSUCCESSFUL ((ZVMSTATUS)0xC0000001L)
#define	ZVM_NOT_SUPPORTED ((ZVMSTATUS)0xC00000BBL)
#define   ZVM_INVALID_PARAMETER ((ZVMSTATUS)0xC000000DL)
typedef long ZION_LIST_ENTRY;

//+++++++++++++++++++++Segment Structs++++++++++++++++++++++++++++++++
/*
* Attribute for segment selector. This is a copy of bit 40:47 & 52:55 of the
* segment descriptor.
*/
typedef union
{
  uint16_t UCHARs;
  struct
  {
    uint16_t type:4;              /* 0;  Bit 40-43 */
    uint16_t s:1;                 /* 4;  Bit 44 */
    uint16_t dpl:2;               /* 5;  Bit 45-46 */
    uint16_t p:1;                 /* 7;  Bit 47 */
    // gap!
    uint16_t avl:1;               /* 8;  Bit 52 */
    uint16_t l:1;                 /* 9;  Bit 53 */
    uint16_t db:1;                /* 10; Bit 54 */
    uint16_t g:1;                 /* 11; Bit 55 */
    uint16_t Gap:4;
  } fields;
} SEGMENT_ATTRIBUTES;

typedef struct
{
  uint16_t sel;
  SEGMENT_ATTRIBUTES attributes;
  uint32_t limit;
  uint64_t base;
} SEGMENT_SELECTOR;

typedef struct
{
  uint16_t limit0;
  uint16_t base0;
  uint8_t base1;
  uint8_t attr0;
  uint8_t limit1attr1;
  uint8_t base2;
} SEGMENT_DESCRIPTOR,
 *PSEGMENT_DESCRIPTOR;

typedef union _LARGE_INTEGER {
 	struct {
 		uint32_t LowPart;
 		int32_t HighPart;
 	} DUMMYSTRUCTNAME;
 	struct {
 		uint32_t LowPart;
 		int32_t HighPart;
 	} u;
	uint64_t QuadPart;
} LARGE_INTEGER;
//+++++++++++++++++++++VMX Structs++++++++++++++++++++++++++++++++

typedef struct _VMX
{
  ZION_PHYSICAL_ADDRESS VmcsToContinuePA;    // MUST go first in the structure; refer to SvmVmrun() for details
  void* _2mbVmcbMap;

  ZION_PHYSICAL_ADDRESS OriginalVmcsPA;
  void* OriginalVmcs;           // VMCS which was originally built by the BP for the guest OS
  ZION_PHYSICAL_ADDRESS OriginalVmxonRPA;    // Vmxon Region which was originally built by the BP for the guest OS
  void* OriginaVmxonR;

  //EFI_PHYSICAL_ADDRESS IOBitmapAPA; // points to IOBitMapA.
  //void* IOBitmapA;

  //EFI_PHYSICAL_ADDRESS IOBitmapBPA; // points to IOBitMapB
  //void* IOBitmapB;

  ZION_PHYSICAL_ADDRESS MSRBitmapPA; // points to MsrBitMap
  void* MSRBitmap;

  uint32_t GuestCR0;             //Guest's CR0.
  uint32_t GuestCR3;             //Guest's CR3. for storing guest cr3 when guest diasble paging.
  uint32_t GuestCR4;             //Guest's CR4.
  uint64_t GuestEFER;
  uint8_t GuestStateBeforeInterrupt[0xc00];

} VMX,
 *PVMX;

//++++++++++++++Cpu Related Structs(Common Structs)++++++++++++++++

typedef struct _CPU *PCPU;

typedef struct _CPU
{

  PCPU SelfPointer;             // MUST go first in the structure; refer to interrupt handlers for details

  VMX Vmx;

  uint32_t ProcessorNumber;

  ZION_LIST_ENTRY GeneralTrapsList;  // list of BP_TRAP structures
  ZION_LIST_ENTRY MsrTrapsList;      //

  PSEGMENT_DESCRIPTOR GdtArea;
  void* IdtArea;

  void* HostStack;              // note that CPU structure reside in this memory region
 } CPU;

typedef struct _GUEST_REGS
{
  uint32_t eax;                  // 0x00         // NOT VALID FOR SVM
  uint32_t ecx;
  uint32_t edx;                  // 0x08
  uint32_t ebx;
  uint32_t esp;                  // esp is not stored here on SVM
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
} GUEST_REGS, *PGUEST_REGS;

//+++++++++++++++++++++Traps Structs++++++++++++++++++++++++++++++++

typedef enum
{
  TRAP_DISABLED = 0,
  TRAP_GENERAL = 1,
  TRAP_MSR = 2,
  TRAP_IO = 3
} TRAP_TYPE;

// The following three will be used as trap's data structure.
typedef struct _NBP_TRAP_DATA_GENERAL
{
  uint32_t TrappedVmExit;
  uint32_t RipDelta;             // this value will be added to rip to skip the trapped instruction
} NBP_TRAP_DATA_GENERAL,
 *PNBP_TRAP_DATA_GENERAL;

typedef struct _NBP_TRAP_DATA_MSR
{
  uint32_t TrappedMsr;
  uint8_t TrappedMsrAccess;
  uint8_t GuestTrappedMsrAccess;
} NBP_TRAP_DATA_MSR,
 *PNBP_TRAP_DATA_MSR;

typedef struct _NBP_TRAP_DATA_IO
{
  uint32_t TrappedPort;
} NBP_TRAP_DATA_IO,
 *PNBP_TRAP_DATA_IO;

typedef struct _NBP_TRAP *PNBP_TRAP;

typedef bool(
  ZVMAPI * NBP_TRAP_CALLBACK
) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  bool WillBeAlsoHandledByGuestHv
);

typedef struct _NBP_TRAP
{
  ZION_LIST_ENTRY le;

  TRAP_TYPE TrapType;
  TRAP_TYPE SavedTrapType;

  union
  {
    NBP_TRAP_DATA_GENERAL General;
    NBP_TRAP_DATA_MSR Msr;
    NBP_TRAP_DATA_IO Io;
  }NBP_TRAP_UNION;

  NBP_TRAP_CALLBACK TrapCallback;
  bool bForwardTrapToGuest;  // FALSE if guest hypervisor doesn't want to intercept this in its own guest.
  // This will be TRUE for TRAP_MSR record when we're going to intercept MSR "rw"
  // but the guest wants to intercept only "r" or "w".
  // Check Msr.GuestTrappedMsrAccess for correct event forwarding.
} NBP_TRAP;

typedef ZVMSTATUS (ZVMAPI * PCALLBACK_PROC) (void* Param);

ZVMSTATUS start_vmx(void);

#endif /* !ZION_VMX_H */
