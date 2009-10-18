#define __LOADER_DEBUG__

#include <inc/kern/Loader.h>
#include <inc/lib/stdio.h>
#include <inc/lib/stdlib.h>
#include <inc/kern/hdd.h>
#include <inc/kern/common.h>
#include <inc/lib/elf.h>

void 
LoadFile ( uint64_t NrStartSector, uint32_t MemBase, uint32_t Size )
{
	uint8_t 	hdbuf[SECTOR_SIZE * NUM_SECTOR_ONCE_READ];
	uint64_t 	sector_idx = NrStartSector;
	uint8_t 	*pMem = (uint8_t *)MemBase;
	
	for ( uint32_t i=0; i<Size; i+=NUM_SECTOR_ONCE_READ ) {
		hd_rw(HD_READ, hdbuf, NrStartSector, NUM_SECTOR_ONCE_READ);
		memcpy(pMem, hdbuf, SECTOR_SIZE * NUM_SECTOR_ONCE_READ);
		sector_idx += NUM_SECTOR_ONCE_READ;
		pMem += SECTOR_SIZE * NUM_SECTOR_ONCE_READ;
	}//for()
	
#ifdef __LOADER_DEBUG__
	output_buf((void *)MemBase, (SECTOR_SIZE * 4));
#endif
}//LoadFile()



uint32_t *
ELF_Remap ( uint64_t NrStartSector, const void *__FileBase )
{
//	const uint8_t *FileBase = (const uint8_t *)__FIleBase;
	
	uint8_t 	buf[SECTOR_SIZE];
	struct ELF *ELFHeader = (struct ELF *) buf;
//	struct ProgHeader
	
	return 0;
};

