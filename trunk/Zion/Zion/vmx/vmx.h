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

#include <include/stdio.h>
#include <include/mm.h>
#include <vmx/vmcs.h>
//#include <vmx/vmxtraps.h>
//#include <vmx/hvm.h>


//#define ZVM_SUCCESS(a)	((a) == ZVMSUCCESS)
//#define ZVMAPI

///* Success or Error Number*/
//#define	TRUE	1
//#define	FALSE	0
//#define	ZVMSUCCESS	((ZVMSTATUS)0x00000000L)
//#define	ZVM_UNSUCCESSFUL ((ZVMSTATUS)0xC0000001L)
//#define	ZVM_NOT_SUPPORTED ((ZVMSTATUS)0xC00000BBL)
//#define   ZVM_INVALID_PARAMETER ((ZVMSTATUS)0xC000000DL)

//typedef struct LIST_ENTRY
   //{
    //struct LIST_ENTRY *Flink;
    //struct LIST_ENTRY *Blink;
   //} 	ZION_LIST_ENTRY,*PZION_LIST_ENTRY;

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
 	//struct {
 		//uint32_t LowPart;
 		//int32_t HighPart;
 	//} DUMMYSTRUCTNAME;
 	//struct {
 		uint32_t LowPart;
 		int32_t HighPart;
 	//} u;
	uint64_t QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER ZION_PHYSICAL_ADDRESS;
//+++++++++++++++++++++VMX Structs++++++++++++++++++++++++++++++++

typedef struct _VMX
{
  ZION_PHYSICAL_ADDRESS VmcsToContinuePA;    // MUST go first in the structure; refer to SvmVmrun() for details
  void* _2mbVmcbMap;

  ZION_PHYSICAL_ADDRESS OriginalVmcsPA;
  void* OriginalVmcs;           // VMCS which was originally built by the BP for the guest OS
  ZION_PHYSICAL_ADDRESS OriginalVmxonRPA;    // Vmxon Region which was originally built by the BP for the guest OS
  void* OriginaVmxonR;

  ZION_PHYSICAL_ADDRESS IOBitmapAPA; // points to IOBitMapA.
  void* IOBitmapA;

  ZION_PHYSICAL_ADDRESS IOBitmapBPA; // points to IOBitMapB
  void* IOBitmapB;

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
  ZION_LIST_ENTRY IoTrapsList;

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

  //union
 // {
    NBP_TRAP_DATA_GENERAL General;
    NBP_TRAP_DATA_MSR Msr;
    NBP_TRAP_DATA_IO Io;
  //}NBP_TRAP_UNION;

  NBP_TRAP_CALLBACK TrapCallback;
  bool bForwardTrapToGuest;  // FALSE if guest hypervisor doesn't want to intercept this in its own guest.
  // This will be TRUE for TRAP_MSR record when we're going to intercept MSR "rw"
  // but the guest wants to intercept only "r" or "w".
  // Check Msr.GuestTrappedMsrAccess for correct event forwarding.
} NBP_TRAP;

enum SEGREGS
{
  ES = 0,
  CS,
  SS,
  DS,
  FS,
  GS,
  LDTR,
  TR
};

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
#define	HOST_STACK_SIZE_IN_PAGES	16

// this must be synchronized with CmSetBluepillSelectors() (common-asm volatile.asm volatile)
#define	BP_GDT64_CODE		KGDT64_R0_CODE  // cs
#define BP_GDT64_DATA		KGDT64_R0_DATA  // ds, es, ss
#define BP_GDT64_SYS_TSS	KGDT64_SYS_TSS  // tr
#define BP_GDT64_PCR		KGDT64_R0_DATA  // gs

///#define BP_GDT_LIMIT	0x6f
#define BP_GDT_LIMIT 0xfff
#define BP_IDT_LIMIT	0xfff
#define BP_TSS_LIMIT	0x68    // 0x67 min


#define	ARCH_VMX	2

/*
 * VMX Exit Reasons
 */

#define VMX_EXIT_REASONS_FAILED_VMENTRY 0x80000000

#define EXIT_REASON_EXCEPTION_NMI       0
#define EXIT_REASON_EXTERNAL_INTERRUPT  1
#define EXIT_REASON_TRIPLE_FAULT        2
#define EXIT_REASON_INIT                3
#define EXIT_REASON_SIPI                4
#define EXIT_REASON_IO_SMI              5
#define EXIT_REASON_OTHER_SMI           6
#define EXIT_REASON_PENDING_INTERRUPT   7

#define EXIT_REASON_TASK_SWITCH         9
#define EXIT_REASON_CPUID               10
#define EXIT_REASON_HLT                 12
#define EXIT_REASON_INVD                13
#define EXIT_REASON_INVLPG              14
#define EXIT_REASON_RDPMC               15
#define EXIT_REASON_RDTSC               16
#define EXIT_REASON_RSM                 17
#define EXIT_REASON_VMCALL              18
#define EXIT_REASON_VMCLEAR             19
#define EXIT_REASON_VMLAUNCH            20
#define EXIT_REASON_VMPTRLD             21
#define EXIT_REASON_VMPTRST             22
#define EXIT_REASON_VMREAD              23
#define EXIT_REASON_VMRESUME            24
#define EXIT_REASON_VMWRITE             25
#define EXIT_REASON_VMXOFF              26
#define EXIT_REASON_VMXON               27
#define EXIT_REASON_CR_ACCESS           28
#define EXIT_REASON_DR_ACCESS           29
#define EXIT_REASON_IO_INSTRUCTION      30
#define EXIT_REASON_MSR_READ            31
#define EXIT_REASON_MSR_WRITE           32

#define EXIT_REASON_INVALID_GUEST_STATE 33
#define EXIT_REASON_MSR_LOADING         34

#define EXIT_REASON_MWAIT_INSTRUCTION   36
#define EXIT_REASON_MONITOR_INSTRUCTION 39
#define EXIT_REASON_PAUSE_INSTRUCTION   40

#define EXIT_REASON_MACHINE_CHECK       41

#define EXIT_REASON_TPR_BELOW_THRESHOLD 43

#define VMX_MAX_GUEST_VMEXIT	EXIT_REASON_TPR_BELOW_THRESHOLD

/*
 * Intel CPU flags in CR0
 */
#define X86_CR0_PE              0x00000001      /* Enable Protected Mode    (RW) */
#define X86_CR0_MP              0x00000002      /* Monitor Coprocessor      (RW) */
#define X86_CR0_EM              0x00000004      /* Require FPU Emulation    (RO) */
#define X86_CR0_TS              0x00000008      /* Task Switched            (RW) */
#define X86_CR0_ET              0x00000010      /* Extension type           (RO) */
#define X86_CR0_NE              0x00000020      /* Numeric Error Reporting  (RW) */
#define X86_CR0_WP              0x00010000      /* Supervisor Write Protect (RW) */
#define X86_CR0_AM              0x00040000      /* Alignment Checking       (RW) */
#define X86_CR0_NW              0x20000000      /* Not Write-Through        (RW) */
#define X86_CR0_CD              0x40000000      /* Cache Disable            (RW) */
#define X86_CR0_PG              0x80000000      /* Paging                   (RW) */


/*
 * Intel CPU features in CR4
 */
#define X86_CR4_VME		0x0001  /* enable vm86 extensions */
#define X86_CR4_PVI		0x0002  /* virtual interrupts flag enable */
#define X86_CR4_TSD		0x0004  /* disable time stamp at ipl 3 */
#define X86_CR4_DE		0x0008  /* enable debugging extensions */
#define X86_CR4_PSE		0x0010  /* enable page size extensions */
#define X86_CR4_PAE		0x0020  /* enable physical address extensions */
#define X86_CR4_MCE		0x0040  /* Machine check enable */
#define X86_CR4_PGE		0x0080  /* enable global pages */
#define X86_CR4_PCE		0x0100  /* enable performance counters at ipl 3 */
#define X86_CR4_OSFXSR		0x0200  /* enable fast FPU save and restore */
#define X86_CR4_OSXMMEXCPT	0x0400  /* enable unmasked SSE exceptions */
#define X86_CR4_VMXE		0x2000  /* enable VMX */


/*
 * Intel CPU  MSR
 */
        /* MSRs & bits used for VMX enabling */

#define MSR_IA32_VMX_BASIC   		0x480
#define MSR_IA32_FEATURE_CONTROL 		0x03a
#define MSR_IA32_VMX_PINBASED_CTLS		0x481
#define MSR_IA32_VMX_PROCBASED_CTLS		0x482
#define MSR_IA32_VMX_EXIT_CTLS		0x483
#define MSR_IA32_VMX_ENTRY_CTLS		0x484

#define MSR_IA32_SYSENTER_CS		0x174
#define MSR_IA32_SYSENTER_ESP		0x175
#define MSR_IA32_SYSENTER_EIP		0x176
#define MSR_IA32_DEBUGCTL			0x1d9

/* x86-64 MSR */

//#define MSR_EFER 0xc0000080           /* extended feature register */
//#define MSR_STAR 0xc0000081           /* legacy mode SYSCALL target */
//#define MSR_LSTAR 0xc0000082          /* long mode SYSCALL target */
//#define MSR_CSTAR 0xc0000083          /* compatibility mode SYSCALL target */
//#define MSR_SYSCALL_MASK 0xc0000084   /* EFLAGS mask for syscall */
//#define MSR_FS_BASE 0xc0000100                /* 64bit FS base */
//#define MSR_GS_BASE 0xc0000101                /* 64bit GS base */
//#define MSR_SHADOW_GS_BASE  0xc0000102        /* SwapGS GS shadow */ 


/*
 * Exit Qualifications for MOV for Control Register Access
 */
#define CONTROL_REG_ACCESS_NUM          0xf     /* 3:0, number of control register */
#define CONTROL_REG_ACCESS_TYPE         0x30    /* 5:4, access type */
#define CONTROL_REG_ACCESS_REG          0xf00   /* 10:8, general purpose register */
#define LMSW_SOURCE_DATA                (0xFFFF << 16)  /* 16:31 lmsw source */

/* XXX these are really VMX specific */
#define TYPE_MOV_TO_DR          (0 << 4)
#define TYPE_MOV_FROM_DR        (1 << 4)
#define TYPE_MOV_TO_CR          (0 << 4)
#define TYPE_MOV_FROM_CR        (1 << 4)
#define TYPE_CLTS               (2 << 4)
#define TYPE_LMSW               (3 << 4)

#define	VMX_VMCS_SIZE_IN_PAGES	1
#define	VMX_IOBitmap_SIZE_IN_PAGES	1
#define	VMX_MSRBitmap_SIZE_IN_PAGES	1

#define	VMX_VMXONR_SIZE_IN_PAGES	2


typedef ZVMSTATUS (ZVMAPI * PCALLBACK_PROC) (void* Param);


static bool ZVMAPI VmxIsTrapVaild (
  uint32_t TrappedVmExit
);

static bool ZVMAPI VmxIsNestedEvent (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);

static void ZVMAPI VmxDispatchEvent (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);

static void ZVMAPI VmxDispatchNestedEvent (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);

static void ZVMAPI VmxAdjustRip (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  uint64_t Delta
);

static ZVMSTATUS ZVMAPI VmxInitialize (
  PCPU Cpu,
  void* GuestRip,
  void* GuestRsp
);

static ZVMSTATUS ZVMAPI VmxVirtualize (
  PCPU Cpu
);


//ZVMSTATUS start_vmx(void);




#endif /* !ZION_VMX_H */
