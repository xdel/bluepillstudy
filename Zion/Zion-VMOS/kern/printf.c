// Simple implementation of cprintf console output for the kernel,
// based on printfmt() and the kernel console's cputchar().

#include <inc/types.h>
#include <inc/lib/stdio.h>
#include <inc/stdarg.h>

//#define 	ZION_COLOR

static void
putch(int ch, void *thunk)
{
#ifdef ZION_COLOR
	// Add color.
	// NOTE: 	bit0-7: 	character ASCII code
	// 			bit8-11:	font color
	// 			bit12-15: 	background color
	switch( ch ) {
		case 'a': ch |= 0x0100; 	break;
		case 'b': ch |= 0x0100; 	break;
		case 'c': ch |= 0x0200; 	break;
		case 'd': ch |= 0x0200; 	break;
		case 'e': ch |= 0x0300; 	break;
		case 'f': ch |= 0x0300; 	break;
		case 'g': ch |= 0x0400; 	break;
		case 'h': ch |= 0x0400; 	break;
		case 'i': ch |= 0x0500; 	break;
		case 'j': ch |= 0x0500; 	break;
		case 'k': ch |= 0x0600; 	break;
		case 'l': ch |= 0x0600; 	break;
		case 'm': ch |= 0x0700; 	break;
		case 'n': ch |= 0x0700; 	break;
		case 'o': ch |= 0x0800; 	break;
		case 'p': ch |= 0x0800; 	break;
		case 'q': ch |= 0x0900; 	break;
		case 'r': ch |= 0x0900; 	break;
		case 's': ch |= 0x0a00; 	break;
		case 't': ch |= 0x0a00; 	break;
		case 'u': ch |= 0x0b00; 	break;
		case 'v': ch |= 0x0b00; 	break;
		case 'w': ch |= 0x0c00; 	break;
		case 'x': ch |= 0x0d00; 	break;
		case 'y': ch |= 0x0e00; 	break;
		case 'z': ch |= 0x0f00; 	break;
		default: break;
	}//switch
#endif

	cputchar(ch);
	int *cntptr = (int *) thunk;
	(*cntptr)++;
}

int
vcprintf(const char *fmt, va_list ap)
{
	int cnt = 0;

	vprintfmt(putch, &cnt, fmt, ap);
	return cnt;
}

int
cprintf(const char *fmt, ...)
{
	va_list ap;
	int cnt;

	va_start(ap, fmt);
	cnt = vcprintf(fmt, ap);
	va_end(ap);

	return cnt;
}

