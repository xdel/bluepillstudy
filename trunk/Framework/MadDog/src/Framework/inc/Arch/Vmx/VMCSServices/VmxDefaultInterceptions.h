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
 /* Copyright (C) 2010 Trusted Computing Lab in Shanghai Jiaotong University
 * 
 * 09/10/11	Miao Yu <superymkfounder@hotmail.com> , Add Basic Interceptions
 */		
#pragma once

#include <ntddk.h>
#include "HvCoreDefs.h"

/*
 * effects: Allow Hypervisor intercept CPUID instruction.
 */
HVSTATUS PtVmxCPUIDInterception(
	PCPU Cpu,
	BOOLEAN ForwardTrap, /* True if need following traps to continue handling this event.*/
	NBP_TRAP_CALLBACK TrapCallback /* If this is null, we won't register a callback function*/
);

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
);

