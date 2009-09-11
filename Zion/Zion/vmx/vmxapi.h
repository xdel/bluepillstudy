#ifndef ZION_VMXAPI_H
#define ZION_VMXAPI_H

//#include <include/types.h>
#include <include/stdio.h>

//#include <vmx/vmx.h>
#include <vmx/hvm.h>
#include <vmx/memory.h>


//+++++++++++++++++++++Structs Definitions+++++++++++++++++++++

/**
 * This struct contains all the callback func pointer.
 * User must initialize this struct first to install MadDog.
 */
/*
typedef struct _Zion_Control
{
	ZVMSTATUS (*SetupVMCB)(PCPU Cpu, void* GuestEip, void* GuestEsp);
	ZVMSTATUS (*ApplyTraps) (PCPU Cpu);
} Zion_Control,
*PZion_Control;
*/
//+++++++++++++++++++++Public Functions++++++++++++++++++++++++
/**
 * effects: Copy the PageTable
 */ 
ZVMSTATUS ZVMAPI Zion_MapPageTable();

/**
 * effects: Install HelloWorld VMM hypervisor.
 */
ZVMSTATUS ZVMAPI Zion_InstallHypervisor();

/**
 * effects: Uninstall HelloWorld VMM Hypervisor
 */
//EFI_STATUS EFIAPI MadDog_UninstallHypervisor();

/**
 * effects: Check if this cpu supports Intel VT Technology. Initialize
 */
ZVMSTATUS ZVMAPI Zion_HypervisorInit();

ZVMSTATUS ZVMAPI start_vmx();

/**
 * effects:Build and Initialize General Trap struct (which is also a Trap struct).
 */
// EFI_STATUS EFIAPI MadDog_InitializeGeneralTrap (
//   PCPU Cpu,
//   UINT TrappedVmExit,
//   UCHAR RipDelta,
//   NBP_TRAP_CALLBACK TrapCallback,
//   PNBP_TRAP * pInitializedTrap
// );

/**
 * effects: Register trap struct.
 */
// EFI_STATUS EFIAPI MadDog_RegisterTrap (
//   PCPU Cpu,
//   PNBP_TRAP Trap
// );

#endif /* !ZION_VMXAPI_H */
