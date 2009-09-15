#ifndef __MALLOC_H
#define __MALLOC_H

#include <inc/types.h>

void*	malloc(size_t n);
void	free(void *v);
void* MmAllocPages(size_t page_num, uint32_t *physicaladdr);

#endif 
