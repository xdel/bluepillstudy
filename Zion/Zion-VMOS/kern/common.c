#include <inc/kern/common.h>
#include <inc/lib/stdio.h>

void 
debug_warning ( char const *msg )
{
	cprintf(msg);
}//debug_warning()



void 
output_buf ( u8 *buf, u32 size8_t )
{
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

