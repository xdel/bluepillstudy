#include "VTCoreAPIs.h"
#include "hvm.h"

//Global handlers, include how to setep VMCB etc.
PMadDog_Control g_HvmControl;

/**
 * effects: Install HelloWorld VMM hypervisor.
 */
NTSTATUS NTAPI MadDog_InstallHypervisor(PMadDog_Control mdCtl)
{
	g_HvmControl = mdCtl;
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