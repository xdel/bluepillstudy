/*
 * Copyright (c) 2010, Trusted Computing Lab in Shanghai Jiaotong University.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Copyright (C) Miao Yu <superymkfounder@hotmail.com>
 */
#include "Arch/Vmx/VTPlatform.h"
#include "msr.h"
#include "HvCoreAPIs.h"
#include "Arch/Vmx/Vmx.h"

#define PIN_BASED_VMX_TIMER_MASK				0x00000040  //bit 6
#define VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE	0x0000482E	//VMCS Field Encoding
#define VM_EXIT_SAVE_TIMER_VALUE_ON_EXIT		0x00400000  //bit 22
#define EXIT_REASON_VMXTIMER_EXPIRED			52

/**
 * This function is used to set value safely according to MSR register.
 * make the <Ctl> values legal.
 * e.g some Vmx Settings use MSR_IA32_VMX_PINBASED_CTLS & MSR_IA32_VMX_TRUE_PINBASED_CTLS.
 */
ULONG32 NTAPI PtVmxAdjustControls (
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
HVSTATUS PtVmxSetTimerInterval(
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
		Status = HvInitializeGeneralTrap(
			Cpu, 
			EXIT_REASON_VMXTIMER_EXPIRED, 
			FALSE,
			0, // length of the instruction, 0 means length need to be get from vmcs later. 
			TrapCallback, 
			&Trap,
			LAB_TAG
		);
		if (!NT_SUCCESS (Status)) 
		{
			Print(("PtVmxSetTimerInterval(): Failed to register PtVmxSetTimerInterval Callback with status 0x%08hX\n", Status));
			return Status;
		}
		MadDog_RegisterTrap (Cpu, Trap);
	}

	return HVSTATUS_SUCCESS;
}
