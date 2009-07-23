#ifndef ZION_HVM_H
#define ZION_HVM_H

#include <include/types.h>
#include <include/stdio.h>
#include <vmx/vmx.h>
#include <vmx/common.h>

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
#define	HOST_STACK_SIZE_IN_PAGES	16

#define BP_GDT_LIMIT	0x6f
#define BP_IDT_LIMIT	0xfff

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

#endif /* !ZION_HVM_H */
