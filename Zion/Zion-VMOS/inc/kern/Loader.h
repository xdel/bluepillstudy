#ifndef __INC_KERN_LOADER_H
#define __INC_KERN_LOADER_H

#include <inc/types.h>

#define 	NUM_SECTOR_ONCE_READ 	8

void LoadFile ( uint64_t NrStartSector, uint32_t MemBase, uint32_t Size );

#endif
