#pragma once

#include <ntddk.h>
#include "HvCoreDefs.h"
#include "HvCoreTypes.h"
#include "Regs.h"
#include "Msr.h"

//#define DEBUG_HVCORE

//#ifdef DEBUG_HVCORE
#include "HvCoreDebugger.h"
//#endif

//+++++++++++++++++++++Global Variables Declaration+++++++++++++++
//extern BOOLEAN bCurrentMachineState; //true means it is in guest OS now, otherwise in hypervisor

//+++++++++++++++++++++Structs Definitions+++++++++++++++++++++

/**
 * This struct contains all the callback func pointer.
 * User must initialize this struct first to install MadDog.
 */
typedef struct _MadDog_Control
{
	NTSTATUS (*UserInitialization)();//This always happens before VMXON, You don't need to handle the initialization on multi processor 
	NTSTATUS (*UserFinalization)();//This always happens after Hypervisor removed, You don't need to handle the initialization on multi processor 
	NTSTATUS (*SetupVMCB)(PCPU Cpu, PVOID GuestEip, PVOID GuestEsp);
	NTSTATUS (*ApplyTraps) (PCPU Cpu);
} MadDog_Control,
 *PMadDog_Control;

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Install HelloWorld VMM hypervisor.
 */
NTSTATUS NTAPI MadDog_InstallHypervisor(PMadDog_Control mdCtl,PDRIVER_OBJECT DriverObject);

/**
 * effects: Uninstall HelloWorld VMM Hypervisor
 */
NTSTATUS NTAPI MadDog_UninstallHypervisor();

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
NTSTATUS NTAPI MadDog_HypervisorInit();

/**
 * effects:Build and Initialize General Trap struct (which is also a Trap struct).
 */
NTSTATUS NTAPI HvInitializeGeneralTrap (
	PCPU Cpu,
	ULONG TrappedVmExit,
	BOOLEAN ForwardTrap, /* True if need following traps to continue handling this event.*/
	UCHAR RipDelta,
	NBP_TRAP_CALLBACK TrapCallback,
	PNBP_TRAP * pInitializedTrap,
    ULONG uDebugTag //Pool Debug Tag
);

/**
 * effects: Register trap struct.
 */
NTSTATUS NTAPI MadDog_RegisterTrap (
	PCPU Cpu,
	PNBP_TRAP Trap
);