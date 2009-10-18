#ifndef __INC_HARDCODING_PARAM_H
#define __INC_HARDCODING_PARAM_H

#include <inc/bochs.h>
#include <inc/memlayout.h>

/* Define i/o port base for hard disk manipulation. */
// For Qian: 0x18D0
// For Yu: 0xCC00
#ifndef __BOCHS_DEBUG__
	#define 		HD_IOPORT_BASE 	0x18D0 		// for SATA
#else
	#define 		HD_IOPORT_BASE 	0x1F0 			// for ATA IDE: 0x1F0
#endif

#define 	ADDR_OFFSET  			KERNBASE 		// Address offset

/* Memory mapping information */
#define 	MemSize_paddr 			(0x8000 + ADDR_OFFSET)
#define 	MCRNumber_paddr 		(0x8008 + ADDR_OFFSET)
#define 	MemInfo_paddr 			(0x8010 + ADDR_OFFSET)



#endif
