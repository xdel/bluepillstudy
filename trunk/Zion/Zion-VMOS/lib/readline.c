#include <inc/lib/stdio.h>
#include <inc/error.h>
#include <inc/lib/string.h>

#define BUFLEN 128
static char buf[BUFLEN];

//---------- Added by Gao Shang: for upper arrow ----------
//char cmdrec[16][BUFLEN];
//int cmdcnt,cmdlen[16];
//---------- Added by Gao Shang: for upper arrow ----------

char *
readline(const char *prompt)
{
	int 	i, j, c, echoing;
	int 	upcnt = 0;
		
	if ( prompt != NULL )
		cprintf("%s", prompt);

	i = 0;
	echoing = (iscons(0) > 0);
	while (1) {
		c = getchar();

//---------- Added by Gao Shang: for upper arrow ----------
		//if ( c == 0xE2 ) {
		//upcnt++;
		//j = 0;		
		//if (upcnt > cmdcnt){
			//cprintf(" \nerr ");
		//}
		//for(j=0; j<BUFLEN; j++){
			//buf[j] = cmdrec[(cmdcnt-upcnt) % 16][j];
		//}	
		//i = cmdlen[(cmdcnt - upcnt) % 16];
		//cprintf("%s", prompt);
		//for (j=0; j<i; j++){cputchar(buf[j]);}
		//}
//---------- Added by Gao Shang: for upper arrow ----------

		if (c < 0) {
			cprintf("read error: %e\n", c);
			return NULL;
		} else if (c >= ' ' && i < BUFLEN-1) {
			if (echoing)
				cputchar(c);
			buf[i++] = c;
		} else if (c == '\b' && i > 0) {
			if (echoing)
				cputchar(c);
			i--;
		} else if (c == '\n' || c == '\r') {
			if (echoing)
				cputchar(c);
			buf[i] = 0;

//---------- Added by Gao Shang: for upper arrow ----------
			//for ( j=0; j<i; j++ ) {
				//cmdrec[cmdcnt % 16][j] = buf[j];
			//}//for
			//cmdlen[cmdcnt % 16] = i;		
			//cmdcnt++;	
//---------- Added by Gao Shang: for upper arrow ----------

			return buf;
		}//if..else
	}//while
}//readline()
