/**
 * File:vmx/vmx.h------>common/hvmUtil.h
 * 
 * This file contains all the VMX-related functions which will be needed 
 * in the hvm file.
 */

#pragma once
#include <ntddk.h>
#include "common.h"
#include "traps.h"
#include "Vmx.h"
#include "vmcs.h"
#include "cpuid.h"
//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++

#define	ARCH_VMX	2

#define	VMX_VMCS_SIZE_IN_PAGES	1
#define	VMX_MSRBitmap_SIZE_IN_PAGES	1
#define	VMX_VMXONR_SIZE_IN_PAGES	2

//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

//typedef struct _VMX
//{
//  PHYSICAL_ADDRESS VmcsToContinuePA;    // MUST go first in the structure; refer to SvmVmrun() for details
//  PVOID _2mbVmcbMap;
//
//  PHYSICAL_ADDRESS OriginalVmcsPA;
//  PVOID OriginalVmcs;           // VMCS which was originally built by the BP for the guest OS
//  PHYSICAL_ADDRESS OriginalVmxonRPA;    // Vmxon Region which was originally built by the BP for the guest OS
//  PVOID OriginaVmxonR;
//
//  //PHYSICAL_ADDRESS IOBitmapAPA; // points to IOBitMapA.
//  //PVOID IOBitmapA;
//
//  //PHYSICAL_ADDRESS IOBitmapBPA; // points to IOBitMapB
//  //PVOID IOBitmapB;
//
//  PHYSICAL_ADDRESS MSRBitmapPA; // points to MsrBitMap
//  PVOID MSRBitmap;
//
//  ULONG GuestCR0;             //Guest's CR0. 
//  ULONG GuestCR3;             //Guest's CR3. for storing guest cr3 when guest diasble paging.
//  ULONG GuestCR4;             //Guest's CR4. 
//  ULONG64 GuestEFER;
//  UCHAR GuestStateBeforeInterrupt[0xc00];
//
//} VMX,
// *PVMX;

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++
//Implemented in vmx-asm.asm
ULONG NTAPI get_cr4 (
);

VOID NTAPI set_in_cr4 (
  ULONG32 mask
);

VOID NTAPI clear_in_cr4 (
  ULONG32 mask
);

VOID NTAPI VmxVmCall (
  ULONG32 HypercallNumber
);

VOID NTAPI VmxPtrld (
  PHYSICAL_ADDRESS VmcsPA
);

VOID NTAPI VmxPtrst (
  PHYSICAL_ADDRESS VmcsPA
);

VOID NTAPI VmxClear (
  PHYSICAL_ADDRESS VmcsPA
);

VOID NTAPI VmxTurnOff (
);

VOID NTAPI VmxTurnOn (
  PHYSICAL_ADDRESS VmxonPA
);

VOID NTAPI VmxLaunch (
);
VOID NTAPI VmxResume (
);

VOID NTAPI VmxVmexitHandler (
  VOID
);


/**
 * effects:	Check if Intel VT Technology is implemented in this CPU
 *			return false if not, otherwise true.
 **/
static BOOLEAN NTAPI VmxIsImplemented();

/**
 * effects: Initialize the guest VM with the callback eip and the esp
 */
static NTSTATUS NTAPI VmxInitialize (
  PCPU Cpu,
  PVOID GuestEip,//points to the next instruction in the guest os.
  PVOID GuestEsp //points to the guest environment-protection register file.
);
/**
 * effects:启动VMCB块对应的Guest Machine
 */
static NTSTATUS NTAPI VmxVirtualize (
  PCPU Cpu
);

/**
 * effects: Check if the VM Exit trap is valid by <TrappedVmExit> value
 * If <TrappedVmExit> >VMX_MAX_GUEST_VMEXIT(43),return false, otherwise true.
 * requires: a valid <TrappedVmExit>
 */
static BOOLEAN NTAPI VmxIsTrapVaild (
  ULONG TrappedVmExit
);
/**
 * effects: Enable the VMX and turn on the VMX
 * thus we are in the VM Root from now on (on this processor).
 */
NTSTATUS NTAPI VmxEnable (
    PVOID VmxonVA
);

NTSTATUS NTAPI VmxDisable (
);

VOID NTAPI VmxCrash (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);

VOID DumpMemory (
  PUCHAR Addr,
  ULONG64 Len
);

VOID NTAPI VmxDumpVmcs (
);

//+++++++++++++++++++++Static Functions++++++++++++++++++++++++

/**
 * effects: Build the VMCS struct.
 */
static NTSTATUS VmxSetupVMCS (
    PCPU Cpu,
    PVOID GuestEip,
    PVOID GuestEsp
);



/**
 * VM Exit Event Dispatcher
 * VMExit事件分发逻辑
 */
static VOID NTAPI VmxDispatchEvent (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
);
/**
 * Adjust Rip
 */
static VOID NTAPI VmxAdjustRip (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  ULONG Delta
);

/**
 * Shutdown VM
 * 关闭虚拟机
 */

static NTSTATUS NTAPI VmxShutdown (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  BOOLEAN bSetupTimeBomb
);
