#include <arch/x86.h>
#include <inc/mmu.h>
#include <inc/assert.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>

static struct Taskstate ts;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};


static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Falt",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if ((size_t) trapno < sizeof(excnames) / sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";

	return "(unknown trap)";
}

void
idt_init(void)
{
	extern struct Segdesc gdt[];
	
	// LAB 2: Your code here.
//----- Qian edit (for Challenge!) -----
	extern uint32_t 	idt_entry[];

	// Initialize idt to point to each entry points.
	for( int i=0; i<=T_SIMDERR; i++ ) {
		SETGATE(idt[i], 0, GD_KT, idt_entry[i], 0);
	}//for
//----- Qian edit (for Challenge!) [end] -----

//----- Qian edit -----
/* // Declaration of trap entry point.
	extern char brkpt[];
	extern char divide[];
	extern char debug[];
	extern char nmi[];
	extern char oflow[];
	extern char bound[];
	extern char illop[];
	extern char device[];
	extern char dblflt[];
	extern char tss[];
	extern char segnp[];
	extern char stack[];
	extern char gpflt[];
	extern char pgflt[];
	extern char fperr[];
	extern char align[];
	extern char mchk[];
	extern char simderr[];
	extern char syscall[];

	// Initialize idt to point to each entry points.
	SETGATE(idt[T_DIVIDE], 0, GD_KT, divide, 0);		// Divide by zero
	SETGATE(idt[T_DEBUG], 1, GD_KT, debug, 0);		// Debug
	SETGATE(idt[T_BRKPT], 1, GD_KT, brkpt, 0);		// Breakpoint
	SETGATE(idt[T_NMI], 0, GD_KT, nmi, 0);			// NMI
	SETGATE(idt[T_OFLOW], 1, GD_KT, oflow, 0);		// Overflow
	SETGATE(idt[T_BOUND], 0, GD_KT, bound, 0);		// Bound
	SETGATE(idt[T_ILLOP], 0, GD_KT, illop, 0);		// Illegal opcode
	SETGATE(idt[T_DEVICE], 0, GD_KT, device, 0);		// Device not available
	SETGATE(idt[T_DBLFLT], 0, GD_KT, dblflt, 0);		// Double fault
	SETGATE(idt[T_TSS], 0, GD_KT, tss, 0);			// Invalid tss
	SETGATE(idt[T_SEGNP], 0, GD_KT, segnp, 0);		// Segment not present
	SETGATE(idt[T_STACK], 0, GD_KT, stack, 0);		// Stack exception
	SETGATE(idt[T_GPFLT], 0, GD_KT, gpflt, 0);		// General protection fault
	SETGATE(idt[T_PGFLT], 0, GD_KT, pgflt, 0);		// Page fault
	SETGATE(idt[T_FPERR], 0, GD_KT, fperr, 0);		// Floating point error
	SETGATE(idt[T_ALIGN], 0, GD_KT, align, 0);		// Alignment check
	SETGATE(idt[T_MCHK], 0, GD_KT, mchk, 0);			// Machine check
	SETGATE(idt[T_SIMDERR], 0, GD_KT, simderr, 0);	// SIMD floating point error
	SETGATE(idt[T_SYSCALL], 0, GD_KT, syscall, 0);	// System call
//----- Qian edit [end] -----
*/
	// Set a gate for the system call interrupt.
	// Hint: Must this gate be accessible from userlevel?
	// LAB 3: Your code here.
	
	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KSTACKTOP;
	ts.ts_ss0 = GD_KD;

	// Initialize the TSS field of the gdt.
	gdt[GD_TSS >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),
					sizeof(struct Taskstate), 0);
	gdt[GD_TSS >> 3].sd_s = 0;

	// Load the TSS
	ltr(GD_TSS);

	// Load the IDT
	asm volatile("lidt idt_pd");
}//idt_init()


void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	cprintf("  err  0x%08x\n", tf->tf_err);
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	cprintf("  esp  0x%08x\n", tf->tf_esp);
	cprintf("  ss   0x----%04x\n", tf->tf_ss);
}

void
print_regs(struct Registers *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

extern "C" {
void
trap(struct Trapframe *tf)
{
	// Dispatch based on what type of trap occurred
	switch (tf->tf_trapno) {

	// LAB 2: Your code here.
//----- Qian edit -----	
	case T_BRKPT:	// Breakpoint
		cprintf("TEST: break entry.\n");
		monitor(tf);	// Invoke monitor.
		break;
//----- Qian edit [end] -----

	default:
		// Unexpected trap: The user process or the kernel has a bug.
		print_trapframe(tf);
		if (tf->tf_cs == GD_KT)
			panic("unhandled trap in kernel");
		else
			panic("unhandled trap in user mode");
	}
}
}
