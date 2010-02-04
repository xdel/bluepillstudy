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

#include "AllocPage.h" 
#include "MemoryHidingStrategy.h" 
#include "MemOps.h" 
#include "MemRegs.h"

//+++++++++++++++++++++++Definitions+++++++++++++++++++++
#define AP_PAGETABLE	1       // used to mark allocations of host pagetables
#define AP_PTE		2
#define AP_PDE		4

#define     ALIGN_4KPAGE_MASK   0xfffff000