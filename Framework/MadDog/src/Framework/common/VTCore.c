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
	return HvmInit();
}