#ifndef __INC_HARDCODING_PARAM_H
#define __INC_HARDCODING_PARAM_H

#include <inc/bochs.h>

/* Define i/o port base for hard disk manipulation. */
// For Qian: 0x18D0
// For Yu: 0xCC00
#ifndef __BOCHS_DEBUG__
	#define 		HD_IOPORT_BASE 	0x18D0 		// for SATA
#else
	#define 		HD_IOPORT_BASE 	0x1F0 			// for ATA IDE: 0x1F0
#endif

#endif
