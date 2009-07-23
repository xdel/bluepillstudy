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
#include <include/stdio.h>

#include <vmx/vmx.h>
#include <vmx/vmxapi.h>

static Zion_Control hvm_Control =
{
		//&HvmSetupVMControlBlock,
		NULL,
		NULL						// not finished!!!!!
};

ZVMSTATUS start_vmx(void)
{
	ZVMSTATUS Status;

	if (!ZVM_SUCCESS (Status = Zion_HypervisorInit()))
	{
		cprintf("Zion Hypervisor: Zion_HypervisorInit() failed with status 0x%08hX\n", Status);
		return Status;
	}
	if (!ZVM_SUCCESS (Status = Zion_InstallHypervisor(&hvm_Control)))
	{
		cprintf("Zion Hypervisor: Zion_InstallHypervisor() failed with status 0x%08hX\n", Status);
		return Status;
	}

	return Status;
}
