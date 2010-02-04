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
 * 09/06/15   Peijie, Yu <yupeijie1012@gmail.com>
 * 09/10/11	Miao Yu <superymkfounder@hotmail.com>
 * 
 */
#pragma once
#include <ntddk.h>

#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY

/**
 * effects:Hide the hidden physical pages from the guest page table.
 */
VOID NTAPI MmHidingStrategyHideGuestPages (
	PVOID GuestAddress,
	ULONG uNumberOfPages
);

/**
 * effects: remap back the indicated hidden physical pages to the guest page table.
 */
VOID NTAPI MmHidingStrategyRevealHiddenPages (
	PVOID GuestAddress,
	ULONG uNumberOfPages
);

/**
 * effects:Hide all the hypervisor allocated physical pages from the guest page table.
 */
VOID NTAPI MmHidingStrategyHideAllAllocatedGuestPages (
);

NTSTATUS NTAPI MmMapGuestPages (
  PVOID FirstPage,
  ULONG uNumberOfPages
);
#endif