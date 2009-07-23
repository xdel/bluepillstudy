/* See COPYRIGHT for copyright information. */

#include <include/stdio.h>
#include <include/string.h>
#include <include/assert.h>

#include <kernel/monitor.h>
#include <kernel/console.h>
#include <kernel/kclock.h>
#include <kernel/trap.h>

#include <mm/pmap.h>

#include <vmx/vmx.h>

extern uint32_t arg0;
extern uint32_t arg1;
extern uint32_t arg2;
extern uint32_t arg3;
extern uint32_t arg4;
extern uint32_t arg5;
extern uint32_t arg6;
extern uint32_t arg7;
extern uint32_t arg8;

// Test the stack backtrace function (lab 1 only)
void
test_backtrace(int x)
{
	cprintf("entering test_backtrace %d\n", x);
	if (x > 0)
		test_backtrace(x-1);
	else
		mon_backtrace(0, 0, 0);
	cprintf("leaving test_backtrace %d\n", x);
}

extern "C" {
void
i386_init(void)
{
	extern char edata[], end[];
	extern const uint32_t sctors[], ectors[];
	const uint32_t *ctorva;

	// Initialize the console.
	// Can't call cprintf until after we do this!
	cons_init();

	// Then call any global constructors.
	// This relies on linker script magic to define the 'sctors' and
	// 'ectors' symbols; see kernel/kernel.ld.
	// Call after cons_init() so we can cprintf() if necessary.
	for (ctorva = ectors; ctorva > sctors; )
		((void(*)()) *--ctorva)();

	cprintf("Welcome to our Virtual Machine World!\n");

	// Lab 2 memory management initialization functions
	mem_init();

	// Lab 2 interrupt and gate descriptor initialization functions
	idt_init();

	// Initialize VM and Turn on VMM
	start_vmx();

	// Drop into the kernel monitor.
	while (1)
		monitor(NULL);
}
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
