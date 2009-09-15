#ifndef ZION_HVM_H
#define ZION_HVM_H

#include <inc/lib/stdio.h>
#include <inc/kern/mmu.h>
#include <inc/lib/string.h>
#include <inc/vmx/vmx.h>
#include <inc/vmx/common.h>
//PHVM_DEPENDENT Hvm;
extern HVM_DEPENDENT Vmx;

////+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
//#define	HOST_STACK_SIZE_IN_PAGES	16

//// this must be synchronized with CmSetBluepillSelectors() (common-asm volatile.asm volatile)
//#define	BP_GDT64_CODE		KGDT64_R0_CODE  // cs
//#define BP_GDT64_DATA		KGDT64_R0_DATA  // ds, es, ss
//#define BP_GDT64_SYS_TSS	KGDT64_SYS_TSS  // tr
//#define BP_GDT64_PCR		KGDT64_R0_DATA  // gs

//#define BP_GDT_LIMIT	0x6f
//#define BP_IDT_LIMIT	0xfff
//#define BP_TSS_LIMIT	0x68    // 0x67 min


//#define	ARCH_VMX	2
//+++++++++++++++++++++Temp Definitions++++++++++++++++++++++
#define Zion_PageSize   4096
//#define	ARCH_VMX	2

// PHVM_DEPENDENT Hvm;
// extern HVM_DEPENDENT Vmx;


//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: install our VM root hypervisor on the fly.
 */
//EFI_STATUS EFIAPI HvmSwallowBluepill();

/**
 * effects:Ð¶ÔØHelloWorld VM
 */
//EFI_STATUS EFIAPI HvmSpitOutBluepill();

/**
 * Initialize Hvm Core.e.g. global lock
 */
ZVMSTATUS ZVMAPI HvmInit();

/**
 * Check if this cpu supports Intel VT Technology.
 */
bool ZVMAPI HvmSupport();

void HvmEventCallback(PCPU Cpu, PGUEST_REGS GuestRegs);

ZVMSTATUS HvmResumeGuest();

/**
  * Intialize the CPU struct and start VM by invoking VmxVirtualize()
  * requires: a valid <GuestRsp>
  **/

 ZVMSTATUS ZVMAPI HvmSubvertCpu (void * GuestRsp);
 
static ZVMSTATUS HvmSetupIdt (PCPU Cpu);

static ZVMSTATUS HvmSetupGdt (PCPU Cpu);

 
 /**
  * Called by cmdelivertoprocess to build VM on such CPU
  **/
  ZVMSTATUS ZVMAPI HvmSwallowBluepill();

#endif /* !ZION_HVM_H */
