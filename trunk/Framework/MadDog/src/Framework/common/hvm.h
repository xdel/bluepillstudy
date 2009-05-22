#pragma once

#include <ntddk.h>
#include "common.h"
//#include "memory.h"
#include "HvCore.h"
//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
#define	HOST_STACK_SIZE_IN_PAGES	16

#define BP_GDT_LIMIT	0x6f
#define BP_IDT_LIMIT	0xfff

//#define	ARCH_VMX	2

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