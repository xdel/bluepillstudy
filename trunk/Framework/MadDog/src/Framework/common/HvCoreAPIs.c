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
 
 /* Copyright (C) 2010 Trusted Computing Lab in Shanghai Jiaotong University
 * 
 * 09/10/11	Miao Yu <superymkfounder@hotmail.com>
 */

#include "HvCoreAPIs.h"
#include "hvm.h"
#include "traps.h"

//+++++++++++++++++++++Global Variables Definition+++++++++++++++
PMadDog_Control g_HvmControl;		//Global handlers, include how to setep VMCB etc.
PDRIVER_OBJECT g_HypervisorDrvObj;	//Global ref to the 3rd party hypervisor driver object, can be used when retrieving driver info from it.
//BOOLEAN bCurrentMachineState = CURRENT_STATE_GUEST; 	//true means it is in guest OS now, otherwise in hypervisor


/**
 * effects: Install HelloWorld VMM hypervisor.
 */
NTSTATUS NTAPI MadDog_InstallHypervisor(PMadDog_Control mdCtl,PDRIVER_OBJECT DriverObject)
{
	g_HvmControl = mdCtl;
	g_HypervisorDrvObj = DriverObject;
	return HvmSwallowBluepill();
}

/**
 * effects: Uninstall HelloWorld VMM Hypervisor
 */
NTSTATUS NTAPI MadDog_UninstallHypervisor()
{
	return HvmSpitOutBluepill();
}

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
NTSTATUS NTAPI MadDog_HypervisorInit()
{
	BOOLEAN ArchIsOK = FALSE;
	ArchIsOK = HvmSupport();
	if (!ArchIsOK) {
		Print(("HvmInit(): Your Intel CPU doesn't either support VT Technology or isn't an Intel CPU at all.\n"));
		return STATUS_NOT_SUPPORTED;
	} else {
		Print(("HvmInit(): Your Intel CPU supports VT Technology.\n"));
	}
	return HvmInit();
}

/**
 * effects:Build and Initialize General Trap struct (which is also a Trap struct).
 */
NTSTATUS NTAPI HvInitializeGeneralTrap (
  PCPU Cpu,
  ULONG TrappedVmExit,
  BOOLEAN ForwardTrap, /* True if need following traps to continue handling this event.*/
  UCHAR RipDelta,
  NBP_TRAP_CALLBACK TrapCallback,
  PNBP_TRAP *pInitializedTrap,
  ULONG uDebugTag //Pool Debug Tag
)
{
	return TrInitializeGeneralTrap(Cpu, TrappedVmExit, ForwardTrap, RipDelta,TrapCallback,pInitializedTrap,uDebugTag);
}

/**
 * effects: Register trap struct.
 */
NTSTATUS NTAPI MadDog_RegisterTrap (
  PCPU Cpu,
  PNBP_TRAP Trap
)
{
	return TrRegisterTrap(Cpu, Trap);
}