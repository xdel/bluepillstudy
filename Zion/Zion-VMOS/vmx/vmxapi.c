#include <inc/vmx/vmxapi.h>
#include <inc/lib/malloc.h>

uint32_t HostCr3,guestcr3;
void *HostCr3VA;
extern pde_t *kern_pgdir;
//Global handlers, include how to setep VMCB etc.

/**
* effects: Install HelloWorld VMM hypervisor.
*/
ZVMSTATUS ZVMAPI Zion_InstallHypervisor()
{
	return HvmSwallowBluepill();
}

ZVMSTATUS ZVMAPI Zion_MapPageTable()
{
	ZVMSTATUS status;
	uint32_t *tmp;
	tmp = (uint32_t *)HostCr3VA;
	status = MmInitManager(kern_pgdir,tmp);
	return status;
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


ZVMSTATUS start_vmx(void)
{
	ZVMSTATUS Status;
    
	asm("movl %%cr3,%0":"=r"(guestcr3));
   	HostCr3VA = MmAllocPages(1,&HostCr3);
   
    //~ if(!ZVM_SUCCESS(Status = Zion_MapPageTable()))
	//~ {
		//~ cprintf("Zion Hypervisor: Zion_MapPageTable() failed...\n");
		//~ return Status;
	//~ }
  

	//~ asm("movl %0,%%cr3"::"r"(HostCr3));
	
	if (!ZVM_SUCCESS (Status = Zion_HypervisorInit()))
	{
		cprintf("Zion Hypervisor: Zion_HypervisorInit() failed with status 0x%08hX\n");
		return Status;
	}

    
	
	if (!ZVM_SUCCESS (Status = Zion_InstallHypervisor()))
	{
		cprintf("Zion Hypervisor: Zion_InstallHypervisor() failed with status 0x%08hX\n");
		return Status;
	}
	
    if(!ZVM_SUCCESS(Status = Zion_MapPageTable()))
	{
		cprintf("Zion Hypervisor: Zion_MapPageTable() failed...\n");
		return Status;
	}


	return Status;
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
