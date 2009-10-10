#ifndef ZION_HVM_H
#define ZION_HVM_H

#include <inc/lib/stdio.h>
#include <inc/kern/mmu.h>
#include <inc/lib/string.h>
#include <inc/vmx/vmx.h>
#include <inc/vmx/common.h>

extern HVM_DEPENDENT Vmx;

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++

//+++++++++++++++++++++Temp Definitions++++++++++++++++++++++
#define Zion_PageSize   4096

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++
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
