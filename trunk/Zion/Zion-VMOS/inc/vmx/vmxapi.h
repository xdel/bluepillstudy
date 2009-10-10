#ifndef ZION_VMXAPI_H
#define ZION_VMXAPI_H

#include <inc/lib/stdio.h>
#include <inc/vmx/hvm.h>
#include <inc/vmx/memory.h>


//+++++++++++++++++++++Structs Definitions+++++++++++++++++++++

//++++++++++++++++++++Public Functions+++++++++++++++++++++++
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

#endif /* !ZION_VMXAPI_H */
