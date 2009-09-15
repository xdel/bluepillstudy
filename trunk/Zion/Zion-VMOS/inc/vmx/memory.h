#include <inc/lib/stdio.h>
#include <inc/types.h>
#include <inc/kern/mmu.h>
#include <inc/lib/string.h>
#include <inc/kern/pmap.h>

ZVMSTATUS ZVMAPI MmInitManager(uint32_t *kern_pgdir, uint32_t *hostcr3va);
