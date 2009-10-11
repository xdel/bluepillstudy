#include <inc/kern/common.h>
#include <inc/lib/stdio.h>
#include <inc/memlayout.h>

#define 	ADDR_OFFSET  	KERNBASE

void 
debug_warning ( char const *msg )
{
	cprintf(msg);
}//debug_warning()



void 
output_buf ( void *__buf, u32 size8_t )
{
	uint8_t *buf = (uint8_t *)__buf;
	
	cprintf("[ Base address=0x%08x, Size=%ld bytes ]\n", (buf - ADDR_OFFSET), size8_t);
	
	cprintf("%04xh: %08x ", 0, *(u32 *)buf); 		// Output the first word.
	
	for ( u32 i=4; i<size8_t ; i+=4 ) {
		if ( (i % 0x200) == 0) {
			cprintf("\n");
			getchar();
		}
		if ( (i % 0x20) == 0 ) {
			cprintf("\n%04xh: ", i);
		}//if
		cprintf("%08x ", *(u32 *)(buf+i));
	}//for
	cprintf("\n");
}//output_buf()

