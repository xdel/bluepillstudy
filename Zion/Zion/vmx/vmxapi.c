#include <vmx/vmxapi.h>

//Global handlers, include how to setep VMCB etc.
PZion_Control g_HvmControl;

/**
* effects: Install HelloWorld VMM hypervisor.
*/
ZVMSTATUS ZVMAPI Zion_InstallHypervisor(PZion_Control hvmCtl)
{
	g_HvmControl = hvmCtl;
	//return HvmSwallowBluepill();
	return ZVMSUCCESS;
}

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
ZVMSTATUS ZVMAPI Zion_HypervisorInit()
{
	bool ArchIsOK = FALSE;
	ArchIsOK = HvmSupport();
	if (!ArchIsOK) {
		cprintf("HvmInit(): Your Intel CPU doesn't either support VT Technology or isn't an Intel CPU at all.\n");
		return FALSE;
	} else {
		cprintf("HvmInit(): Your Intel CPU supports VT Technology.\n");
	}
	return HvmInit();
}

/**
* effects:Build and Initialize General Trap struct (which is also a Trap struct).
*/
// EFI_STATUS EFIAPI MadDog_InitializeGeneralTrap (
// 	PCPU Cpu,
// 	UINT32 TrappedVmExit,
// 	UCHAR RipDelta,
// 	NBP_TRAP_CALLBACK TrapCallback,
// 	PNBP_TRAP *pInitializedTrap
// 	)
// {
// 	return TrInitializeGeneralTrap(Cpu,TrappedVmExit,RipDelta,TrapCallback,pInitializedTrap);
// }

/**
* effects: Register trap struct.
*/
// EFI_STATUS EFIAPI MadDog_RegisterTrap (
// 									PCPU Cpu,
// 									PNBP_TRAP Trap
// 									)
// {
// 	return TrRegisterTrap(Cpu, Trap);
// }
