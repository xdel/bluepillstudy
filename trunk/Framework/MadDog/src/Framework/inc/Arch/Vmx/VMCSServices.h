/**************************************************************
 * Original:
 * This file is added to gather all the VMCS structure services.
 *
 * These services can help configure a VMCS structure instead of
 * use a series of VMXWrite instruction to setup one function.
 *
 * The VMCS Services suppose that a user has executed the VMXPTRLD
 * instruction before and already set the processor's current-VMCS
 * pointer.
 *
 * Because of the possiblity of conflict with prior VMCS setup using 
 * VMXWrite instruction, all these services can only be used at the 
 * final step of configuring VMCS struct.

 * Superymk Wed May 25 2:00 PM 2009
 **************************************************************/
#pragma once

#include "VMCSServices/VMXTimerService.h"
#include "VMCSServices/VmxDefaultInterceptions.h"

/**
 * This function is used to set value safely according to MSR register.
 * make the <Ctl> values legal.
 * e.g some Vmx Settings use MSR_IA32_VMX_PINBASED_CTLS & MSR_IA32_VMX_TRUE_PINBASED_CTLS.
 */
ULONG32 NTAPI PtVmxAdjustControls (
	ULONG32 Ctl,
	ULONG32 Msr
);