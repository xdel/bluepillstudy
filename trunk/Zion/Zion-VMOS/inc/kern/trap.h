#ifndef __KERN_TRAP_H
#define __KERN_TRAP_H

#include <inc/trap.h>
#include <inc/kern/mmu.h>

/* The kernel's interrupt descriptor table */
extern struct Gatedesc idt[];

void idt_init(void);
void print_regs(struct Registers *regs);
void print_trapframe(struct Trapframe *tf);
void page_fault_handler(struct Trapframe *);
void backtrace(struct Trapframe *);
const char *trapname(int trapno);

#endif
