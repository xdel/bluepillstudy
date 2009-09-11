#include <include/stdio.h>
#include <include/types.h>
#include <include/mmu.h>
#include <include/string.h>
#include <include/mm.h>

ZVMSTATUS ZVMAPI MmInitManager(uint32_t *kern_pgdir, uint32_t *hostcr3va);
