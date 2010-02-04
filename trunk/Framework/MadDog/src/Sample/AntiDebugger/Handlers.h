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
 * 09/10/11	Miao Yu <superymkfounder@hotmail.com>
 */
 
#pragma once
#include <ntddk.h>
#include "HvCore.h"

#define VMX_EXCEPTION_PAGEFAULT		( 1 << 14)
#define VMX_EXCEPTION_INT1			( 1 << 1)
#define VMX_EXCEPTION_INT3			( 1 << 3)
NTSTATUS HvmSetupVMControlBlock (
    PCPU Cpu,
    PVOID GuestEip,
    PVOID GuestEsp
);

//+++++++++++++++++++++Inner Functions++++++++++++++++++++++++

// make the ctl code legal
static ULONG32 NTAPI VmxAdjustControls (
    ULONG32 Ctl,
    ULONG32 Msr
);

static NTSTATUS NTAPI VmxFillGuestSelectorData (
    PVOID GdtBase,
    ULONG Segreg,
    USHORT Selector
);