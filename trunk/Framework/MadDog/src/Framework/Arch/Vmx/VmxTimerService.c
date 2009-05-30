#include "Arch/Vmx/VTPlatform.h"
#include "msr.h"
#include "HvCoreAPIs.h"

#define PIN_BASED_VMX_TIMER_MASK				0x00000040  //bit 6
#define VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE	0x0000482E	//VMCS Field Encoding
#define VM_EXIT_SAVE_TIMER_VALUE_ON_EXIT		0x00400000  //bit 22
#define EXIT_REASON_VMXTIMER_EXPIRED			52
/*
 * Intel CPU  MSR
 */
/* MSRs & bits used for VMX enabling */
#define MSR_IA32_VMX_MISC	0x485


static ULONG32 NTAPI VmxAdjustControls (
  ULONG32 Ctl,
  ULONG32 Msr
)
{
  LARGE_INTEGER MsrValue;

  MsrValue.QuadPart = MsrRead (Msr);
  Ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
  Ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
  return Ctl;
}

/*
 * effects: This service introduced in VMX Preemption
 * Timer function to the VMCS.
 * returns: If VMX-Preemption Timer is not supported on the current platform, 
 * returns HVSTATUS_UNSUPPORTED_FEATURE.
 */
HVSTATUS PtVMXSetTimerInterval(
	PCPU Cpu,
	ULONG32 Ticks, /* After how many ticks the VMX Timer will be expired, THIS VALUE IS FIXED TO BE 32 BITS LONG*/
	BOOLEAN SaveTimerValueOnVMEXIT,
	NBP_TRAP_CALLBACK TrapCallback /* If this is null, we won't register a callback function*/
)
{
	ULONG32 Interceptions;
	ULONG32 Ratio;
	LARGE_INTEGER MsrValue,MsrValue1,MsrValue2;
	PNBP_TRAP Trap;
	NTSTATUS Status;
	
	//Step 0. Check if the current platform supports VMX-Preemption Timer
	if(!(Cpu->Vmx.FeaturesMSR.VmxPinBasedCTLs.HighPart & PIN_BASED_VMX_TIMER_MASK))
		return HVSTATUS_UNSUPPORTED_FEATURE;

	//Step 1. Activate VMX-Preemption Timer in PIN_BASED_VM_EXEC_CONTROL
	Interceptions = VmxRead(PIN_BASED_VM_EXEC_CONTROL);
	VmxWrite(PIN_BASED_VM_EXEC_CONTROL, Interceptions|PIN_BASED_VMX_TIMER_MASK);
									// Activate VMX-preemption Timer

	//Step 2. Get the Ratio of TSC-VMX Timer Tick in IA32_VMX_MISC MSR
	MsrValue.QuadPart = MsrRead(MSR_IA32_VMX_MISC);
	Ratio = MsrValue.LowPart & (~0xffffffe0);
	
	//Step 3. Set the VMX-Preemption Timer Value
	VmxWrite(VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE, Ticks>>Ratio);
	
	//Step 4. Set whether saving VMX-Preemption Timer Value on VMEXIT
	if(SaveTimerValueOnVMEXIT)
	{
		//Save the value
		Interceptions = VmxRead(VM_EXIT_CONTROLS);
		VmxWrite(VM_EXIT_CONTROLS, Interceptions|VM_EXIT_SAVE_TIMER_VALUE_ON_EXIT); 
	}

	//Step 5. Register a callback function
	if(TrapCallback)
	{
		Status = MadDog_InitializeGeneralTrap(
			Cpu, 
			EXIT_REASON_VMXTIMER_EXPIRED, 
			0, // length of the instruction, 0 means length need to be get from vmcs later. 
			TrapCallback, 
			&Trap,
			LAB_TAG
		);
		if (!NT_SUCCESS (Status)) 
		{
			Print(("HvVMXSetTimerInterval(): Failed to register HvVMXSetTimerInterval Callback with status 0x%08hX\n", Status));
			return Status;
		}
		MadDog_RegisterTrap (Cpu, Trap);
	}

	return HVSTATUS_SUCCESS;
}
