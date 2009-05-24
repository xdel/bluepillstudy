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