#include "Arch/Vmx/VTPlatform.h"
#include "msr.h"
#include "HvCoreAPIs.h"
#include "Arch/Vmx/Vmx.h"

/*
 * effects: Allow Hypervisor intercept reading CR3 register.
 * When using this interception more than once, it's important to keep the <NumCR3TargetCtls> and <CR3TargetCtls>
 * remain unchanged, otherwise only the latest setting will take effect.
 */
HVSTATUS PtVmxCR3AccessInterception(
	PCPU	Cpu,
	ULONG	NumCR3TargetCtls,/* length of CR3TargetCtls[], 0 means all CR3 access will cause #VMEXIT. Setting this
							 value may overwrite the previous settings. Currently <NumCR3TargetCtls> is limit to not 
							 larger than 4*/
	PULONG	CR3TargetCtls, /* These CR3 target address won't cause a #VMEXIT. Setting this
							 value may overwrite the previous settings.*/
	BOOLEAN ForwardTrap, /* True if need following traps to continue handling this event.*/
	NBP_TRAP_CALLBACK TrapCallback /* If this is null, we won't register a callback function*/
)
{
	ULONG AllowedNumCR3TargetCtls;
	LARGE_INTEGER MsrValue;
	PNBP_TRAP Trap;
	NTSTATUS Status;
	ULONG i;

	MsrValue.QuadPart =  MsrRead(MSR_IA32_VMX_MISC);
	AllowedNumCR3TargetCtls = (MsrValue.LowPart & 0x0FFF0000)>>16;

	//Step 0. Check the legalty of the parameters.
	//TODO
	//We temporarilly limit the 0<=NumCR3TargetCtls<4, Modify this in the future.
	if( (NumCR3TargetCtls >4 )|| (NumCR3TargetCtls > AllowedNumCR3TargetCtls) 
		|| (AllowedNumCR3TargetCtls > 256))
		return HVSTATUS_INVALID_PARAMETERS;

	//Step 1. Set CR3_TARGET_COUNT in VMCS
	VmxWrite (CR3_TARGET_COUNT, NumCR3TargetCtls);

	//Step 2. Set CR3_TARGET_VALUE[0..3] in VMCS
	if(NumCR3TargetCtls == 0 || !CR3TargetCtls)
	{
		VmxWrite (CR3_TARGET_VALUE0, 0);  
		VmxWrite (CR3_TARGET_VALUE1, 0);  
		VmxWrite (CR3_TARGET_VALUE2, 0);  
		VmxWrite (CR3_TARGET_VALUE3, 0);  
	}
	else
	{
		for(i = 0; i < NumCR3TargetCtls ; i++)
		{
			switch(i)
			{
				case 0:
					VmxWrite (CR3_TARGET_VALUE0, CR3TargetCtls[0]);      
					break;
				case 1:
					VmxWrite (CR3_TARGET_VALUE1, CR3TargetCtls[1]);       
					break;
				case 2:
					VmxWrite (CR3_TARGET_VALUE2, CR3TargetCtls[2]);   
					break;
				case 3:
					VmxWrite (CR3_TARGET_VALUE3, CR3TargetCtls[3]);      
					break;
			}
		}
	}
	//Step 5. Register a callback function
	if(TrapCallback)
	{
		Status = HvInitializeGeneralTrap(
			Cpu, 
			EXIT_REASON_CR_ACCESS, 
			FALSE,
			0, // length of the instruction, 0 means length need to be get from vmcs later. 
			TrapCallback, 
			&Trap,
			LAB_TAG
		);

		if (!NT_SUCCESS (Status)) 
		{
			Print(("PtVmxCR3AccessInterception(): Failed to register PtVmxCR3AccessInterception Callback with status 0x%08hX\n", Status));
			return Status;
		}

		Trap->TrapType = TRAP_CR;
		Trap->Cr.CRNo = 3;

		MadDog_RegisterTrap (Cpu, Trap);
	}
	
	return HVSTATUS_SUCCESS;
}
