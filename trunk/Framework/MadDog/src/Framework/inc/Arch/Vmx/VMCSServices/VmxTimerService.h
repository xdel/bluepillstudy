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
#pragma once

#include <ntddk.h>
#include "HvCoreDefs.h"

/*
 * effects: This service introduced in VMX Preemption
 * Timer function to the VMCS.
 * returns: if the <ratio> larger than 31, then returns STATUS_INVALID_PARAMETERS
 */
HVSTATUS PtVmxSetTimerInterval(
	PCPU Cpu,
	ULONG32 Ticks, /* After how many ticks the VMX Timer will be expired, THIS VALUE IS FIXED TO BE 32 BITS LONG*/
	BOOLEAN SaveTimerValueOnVMEXIT,
	NBP_TRAP_CALLBACK TrapCallback /* If this is null, we won't register a callback function*/
);