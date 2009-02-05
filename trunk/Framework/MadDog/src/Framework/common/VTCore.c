#include "VTCore.h"
#include "hvm.h"

/**
 * effects: Install our VM  hypervisor on the fly.
 */
NTSTATUS NTAPI InstallVMM()
{
	return HvmSwallowBluepill();
}

/**
 * effects: Uninstall HelloWorld VMM
 */
NTSTATUS NTAPI UninstallVMM()
{
	return HvmSpitOutBluepill();
}

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
NTSTATUS NTAPI VMMInit()
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