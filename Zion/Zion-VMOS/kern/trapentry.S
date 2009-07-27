#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

###################################################################
# exceptions/interrupts
###################################################################

/* The TRAPHANDLER macro defines a globally-visible function for handling
 * a trap.  It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 */ 
#define old_TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define old_TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

/* 
 * Redifine macro TRAPHANDLER and TRAPHANDLER_NOEC for code improvement.
 */ 
#define TRAPHANDLER(name, num)						\
.data; 												\
	.long 	name;									\
.text;												\
name:												\
	pushl 	$(num);									\
	jmp 	_alltraps

#define TRAPHANDLER_NOEC(name, num)					\
.data;												\
	.long 	name; 									\
.text;												\
name:												\
	pushl 	$0;										\
	pushl 	$(num);									\
	jmp 	_alltraps

.data
	.globl 	idt_entry



idt_entry:
.text
/* Generate entry points for traps. */
TRAPHANDLER_NOEC(divide, T_DIVIDE)
TRAPHANDLER_NOEC(debug, T_DEBUG)
TRAPHANDLER_NOEC(nmi, T_NMI)
TRAPHANDLER_NOEC(brkpt, T_BRKPT)
TRAPHANDLER_NOEC(oflow, T_OFLOW)
TRAPHANDLER_NOEC(bound, T_BOUND)
TRAPHANDLER_NOEC(illop, T_ILLOP)
TRAPHANDLER_NOEC(device, T_DEVICE)
TRAPHANDLER(dblflt, T_DBLFLT)
TRAPHANDLER_NOEC(coproc, T_COPROC)
TRAPHANDLER(tss, T_TSS)
TRAPHANDLER(segnp, T_SEGNP)
TRAPHANDLER(stack, T_STACK)
TRAPHANDLER(gpflt, T_GPFLT)
TRAPHANDLER(pgflt, T_PGFLT)
TRAPHANDLER_NOEC(res, T_RES)
TRAPHANDLER_NOEC(fperr, T_FPERR)
TRAPHANDLER(align, T_ALIGN)
TRAPHANDLER_NOEC(mchk, T_MCHK)
TRAPHANDLER_NOEC(simderr, T_SIMDERR)
TRAPHANDLER_NOEC(syscall, T_SYSCALL)



_alltraps:	
	/* Build trap frame. */
	pushl 	%ds
	pushl 	%es
	pushal

	/* Load kernel descriptors. */
	movl 	$GD_KD, %eax
	movw 	%ax, %ds
	movw 	%ax, %es
	/* Pass a pointer to the trapframe as an argument to trap(). */
	pushl 	%esp

	/* Call trap(). */
	call 	trap

	/* Pop the pushed stack pointer. */
	popl 	%esp
	popal
	popl 	%es
	popl 	%ds
	
	/* Get rid of the trap number and the error code */
	addl 	$0x8, %esp
	
	iret

