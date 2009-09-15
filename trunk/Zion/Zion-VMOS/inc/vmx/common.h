#ifndef ZION_COMMON_H
#define ZION_COMMON_H

#include <inc/types.h>
#include <inc/kern/mmu.h>
#include <inc/vmx/vmx.h>

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
/*
* Generic CPUID function
* clear %ecx since some cpus (Cyrix MII) do not set or clear %ecx
* resulting in stale register contents being returned.
*/
#define cpuid(_op,_eax,_ebx,_ecx,_edx)          \
	asm volatile( "cpuid"                               \
	: "=a" (*(int32_t *)(_eax)),              \
	"=b" (*(int32_t *)(_ebx)),              \
	"=c" (*(int32_t *)(_ecx)),              \
	"=d" (*(int32_t *)(_edx))               \
	: "0" (_op), "2" (0) )

#define BYTES_TO_PAGES(Size)  (((Size) >> PGSHIFT) + \
                               (((Size) & (PGSIZE - 1)) != 0))
							   
							   
							   
///#pragma pack (pop)

#define LA_ACCESSED		0x01
#define LA_READABLE		0x02    // for code segments
#define LA_WRITABLE		0x02    // for data segments
#define LA_CONFORMING	0x04    // for code segments
#define LA_EXPANDDOWN	0x04    // for data segments
#define LA_CODE			0x08
#define LA_STANDARD		0x10
#define LA_DPL_0		0x00
#define LA_DPL_1		0x20
#define LA_DPL_2		0x40
#define LA_DPL_3		0x60
#define LA_PRESENT		0x80

#define LA_LDT64		0x02
#define LA_ATSS64		0x09
#define LA_BTSS64		0x0b
#define LA_CALLGATE64	0x0c
#define LA_INTGATE64	0x0e
#define LA_TRAPGATE64	0x0f

#define HA_AVAILABLE	0x01
#define HA_LONG			0x02
#define HA_DB			0x04
#define HA_GRANULARITY	0x08

#define P_PRESENT			0x01
#define P_WRITABLE			0x02
#define P_USERMODE			0x04
#define P_WRITETHROUGH		0x08
#define P_CACHE_DISABLED	0x10
#define P_ACCESSED			0x20
#define P_DIRTY				0x40
#define P_LARGE				0x80
#define P_GLOBAL			0x100

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

#define	REG_R8	REG_GP_ADDITIONAL | 0
#define	REG_R9	REG_GP_ADDITIONAL | 1
#define	REG_R10	REG_GP_ADDITIONAL | 2
#define	REG_R11	REG_GP_ADDITIONAL | 3
#define	REG_R12	REG_GP_ADDITIONAL | 4
#define	REG_R13	REG_GP_ADDITIONAL | 5
#define	REG_R14	REG_GP_ADDITIONAL | 6
#define	REG_R15	REG_GP_ADDITIONAL | 7

#define REG_CR0	REG_CONTROL | 0
#define REG_CR2	REG_CONTROL | 2
#define REG_CR3	REG_CONTROL | 3
#define REG_CR4	REG_CONTROL | 4
#define REG_CR8	REG_CONTROL | 8

#define REG_DR0	REG_DEBUG | 0
#define REG_DR1	REG_DEBUG | 1
#define REG_DR2	REG_DEBUG | 2
#define REG_DR3	REG_DEBUG | 3
#define REG_DR6	REG_DEBUG | 6
#define REG_DR7	REG_DEBUG | 7

/*
 * EFLAGS bits
 */
#define X86_EFLAGS_CF	0x00000001      /* Carry Flag */
#define X86_EFLAGS_PF	0x00000004      /* Parity Flag */
#define X86_EFLAGS_AF	0x00000010      /* Auxillary carry Flag */
#define X86_EFLAGS_ZF	0x00000040      /* Zero Flag */
#define X86_EFLAGS_SF	0x00000080      /* Sign Flag */
#define X86_EFLAGS_TF	0x00000100      /* Trap Flag */
#define X86_EFLAGS_IF	0x00000200      /* Interrupt Flag */
#define X86_EFLAGS_DF	0x00000400      /* Direction Flag */
#define X86_EFLAGS_OF	0x00000800      /* Overflow Flag */
#define X86_EFLAGS_IOPL	0x00003000      /* IOPL mask */
#define X86_EFLAGS_NT	0x00004000      /* Nested Task */
#define X86_EFLAGS_RF	0x00010000      /* Resume Flag */
#define X86_EFLAGS_VM	0x00020000      /* Virtual Mode */
#define X86_EFLAGS_AC	0x00040000      /* Alignment Check */
#define X86_EFLAGS_VIF	0x00080000      /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIP	0x00100000      /* Virtual Interrupt Pending */
#define X86_EFLAGS_ID	0x00200000      /* CPUID detection flag */
//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

//typedef bool (
  //ZVMAPI * ARCH_IS_HVM_IMPLEMENTED
//) (
//);

typedef ZVMSTATUS (
  ZVMAPI * ARCH_INITIALIZE
) (
  PCPU Cpu,
  void* GuestRip,
  void* GuestRsp
);
typedef ZVMSTATUS (
  ZVMAPI * ARCH_VIRTUALIZE
) (
  PCPU Cpu
);
//typedef ZVMSTATUS (
  //ZVMAPI * ARCH_SHUTDOWN
//) (
  //PCPU Cpu,
  //PGUEST_REGS GuestRegs,
  //bool bSetupTimeBomb
//);

typedef bool (
  ZVMAPI * ARCH_IS_NESTED_EVENT
) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);
typedef void (
  ZVMAPI * ARCH_DISPATCH_NESTED_EVENT
) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);
typedef void (
  ZVMAPI * ARCH_DISPATCH_EVENT
) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);
typedef void (
  ZVMAPI * ARCH_ADJUST_RIP
) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  uint64_t Delta
);
typedef ZVMSTATUS ( ZVMAPI * ARCH_REGISTER_TRAPS)
(
  PCPU Cpu
);
typedef bool (
  ZVMAPI * ARCH_IS_TRAP_VALID
) (
  uint32_t TrappedVmExit
);                            

typedef struct
{
  uint8_t Architecture;

  //ARCH_IS_HVM_IMPLEMENTED ArchIsHvmImplemented;

  ARCH_INITIALIZE ArchInitialize;
  ARCH_VIRTUALIZE ArchVirtualize;
  //ARCH_SHUTDOWN ArchShutdown;

  ARCH_IS_NESTED_EVENT ArchIsNestedEvent;
  ARCH_DISPATCH_NESTED_EVENT ArchDispatchNestedEvent;
  ARCH_DISPATCH_EVENT ArchDispatchEvent;
  ARCH_ADJUST_RIP ArchAdjustRip;
  ARCH_REGISTER_TRAPS ArchRegisterTraps;
  ARCH_IS_TRAP_VALID ArchIsTrapValid;
} HVM_DEPENDENT,
 *PHVM_DEPENDENT;


//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

bool ZVMAPI CmIsBitSet (
  uint32_t v,
  uint8_t bitNo
);

/**
* effects:Raise the interruption level to dispatch level, then
* install VM Root hypervisor by call <CallbackProc>
*/
ZVMSTATUS ZVMAPI CmDeliverToProcessor (
									   int8_t cProcessorNumber,
									   PCALLBACK_PROC CallbackProc,
									   void* CallbackParam,
									   PZVMSTATUS pCallbackStatus
									   );

ZVMSTATUS ZVMAPI CmInitializeSegmentSelector (
									   SEGMENT_SELECTOR * SegmentSelector,
									   uint16_t Selector,
                                       uint8_t* GdtBase
);

//++++++++++++++++++++asm volatile Functions++++++++++++++++++++++++++++
ZVMSTATUS ZVMAPI CmSubvert (void *);

void CmSlipIntoMatrix();

void CmReloadGdtr(PSEGMENT_DESCRIPTOR gdtbase, uint8_t gdtlimit);

void CmReloadIdtr(void* idtbase, uint8_t idtlimit);

uint32_t VmxRead(uint32_t field);

uint64_t MsrRead(uint64_t reg);

void VmxWrite(uint32_t field, uint32_t value);

void MsrWrite(uint32_t field, uint32_t value);

void GetCpuIdInfo(uint32_t fn,uint32_t* ret_eax,uint32_t* ret_ebx,uint32_t* ret_ecx,uint32_t* ret_edx);

void set_in_cr4(uint32_t mask);

void clear_in_cr4(uint32_t mask);

uint64_t get_cr4();

void VmxTurnOn(ZION_PHYSICAL_ADDRESS addr);

void VmxTurnOff();

void VmxClear(ZION_PHYSICAL_ADDRESS addr);

void VmxPtrld(ZION_PHYSICAL_ADDRESS addr);

void VmxLaunch();

uint64_t RegGetRflags();

uint64_t RegGetRsp();

uint64_t RegGetCr0();

uint64_t RegGetCr3();

uint64_t RegGetCr4();

uint64_t RegGetFs();

uint64_t RegGetGs();

uint64_t RegGetEs();

uint64_t RegGetCs();

uint64_t RegGetSs();

uint64_t RegGetDs();

uint64_t GetTrSelector();

uint8_t GetGdtLimit();

uint8_t GetIdtLimit();

uint32_t GetGdtBase();

uint32_t GetLdtr();

uint32_t GetIdtBase();

uint32_t CmIOIn(uint32_t port);

void CmIOOutB(uint32_t port, uint32_t data);

void CmIOOutW(uint32_t port, uint32_t data);

void CmIOOutD(uint32_t port, uint32_t data);


//++++++++++++++++++++DDK Function+++++++++++++++++++++++++++++
void InitializeListHead(PZION_LIST_ENTRY ListHead);

void InsertTailList(PZION_LIST_ENTRY ListHead, PZION_LIST_ENTRY Entry);

//Just for complie
//void * MmAlloc(uint32_t size,void * physicaladdress);

ZION_PHYSICAL_ADDRESS MmGetPhysicalAddress(void* addr);

#endif /* !ZION_COMMON_H */
