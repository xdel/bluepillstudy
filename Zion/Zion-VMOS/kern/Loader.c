#define __LOADER_DEBUG__

#include <inc/kern/Loader.h>
#include <inc/lib/stdio.h>
#include <inc/kern/hdd.h>
#include <inc/kern/common.h>

void 
LoadFile ( uint64_t NrStartSector, uint32_t MemBase, uint32_t Size )
{
	uint8_t hdbuf[SECTOR_SIZE * NUM_SECTOR_ONCE_READ];
	
	hd_rw(HD_READ, hdbuf, NrStartSector, NUM_SECTOR_ONCE_READ);
	
#ifdef __LOADER_DEBUG__
	output_buf(hdbuf, (SECTOR_SIZE * NUM_SECTOR_ONCE_READ));
#endif
}//LoadFile()
