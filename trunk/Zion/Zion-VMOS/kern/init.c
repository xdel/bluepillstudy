#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/monitor.h>
#include <kern/console.h>
#include <kern/pmap.h>
#include <kern/kclock.h>
#include <kern/trap.h>


extern "C" {
void i386_init(void)
{
	extern char edata[], end[];
	extern const uint32_t sctors[], ectors[];
	const uint32_t *ctorva;
	// Initialize the console.
	// Can't call cprintf until after we do this!
	cons_init();

	// Then call any global constructors.
	// This relies on linker script magic to define the 'sctors' and
	// 'ectors' symbols; see kern/kernel.ld.
	// Call after cons_init() so we can cprintf() if necessary.
	for (ctorva = ectors; ctorva > sctors; )
		((void(*)()) *--ctorva)();

	// Print starting message.
	cprintf("\n");
	cprintf("    -----------------------------------------------------------------\n");
	cprintf("    | ***** Zion Virtual Machine Operating System (Zion VMOS) ***** |\n");
	cprintf("    | COPYRIGHT @ School of Software, Shanghai Jiao Tong University |\n");
	cprintf("    | Version: 9.09.05-2                                              |\n");
	cprintf("    -----------------------------------------------------------------\n");
	cprintf("\n");


	// Memory management initialization.
	mem_init();

	// Interrupt and gate descriptor initialization.
	idt_init();
	
//	__asm__ __volatile__("int3");

	// Drop into the kernel monitor.
	while (1)
		monitor(NULL);
}//i386_init()
}


/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
static const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void
_panic(const char *file, int line, const char *fmt, ...)
{
	va_list ap;

	if (panicstr)
		goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf("kernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

dead:
	/* break into the kernel monitor */
	while (1)
		monitor(NULL);
}

/* like panic, but don't */
void
_warn(const char *file, int line, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf("kernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}
