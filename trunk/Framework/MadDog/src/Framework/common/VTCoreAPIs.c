#include "VTCoreAPIs.h"
#include "hvm.h"

/**
 * effects: Install HelloWorld VMM hypervisor.
 */
NTSTATUS NTAPI InstallHypervisor()
{
	return HvmSwallowBluepill();
}

/**
 * effects: Uninstall HelloWorld VMM Hypervisor
 */
NTSTATUS NTAPI UninstallHypervisor()
{
	return HvmSpitOutBluepill();
}

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
NTSTATUS NTAPI HypervisorInit()
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