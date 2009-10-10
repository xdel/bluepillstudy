#ifndef __KERN_MONITOR_H
#define __KERN_MONITOR_H

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

struct Trapframe;

// Activate the kernel monitor,
// optionally providing a trap frame indicating the current state
// (NULL if none).
void monitor(struct Trapframe *tf);

// Functions implementing monitor commands.
int mon_help(int argc, char **argv, struct Trapframe *tf);
int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
int mon_backtrace(int argc, char **argv, struct Trapframe *tf);
int mon_exit(int argc, char **argv, struct Trapframe *tf);
int mon_reboot( int argc, char **argv, struct Trapframe *tf );
int mon_cpuid( int argc, char **argv, struct Trapframe *tf );
int mon_cpuinfo( int argc, char **argv, struct Trapframe *tf );
int mon_memcheck( int argc, char **argv, struct Trapframe *tf );
int mon_meminfo ( int argc, char **argv, struct Trapframe *tf );
int mon_int3 (int argc, char **argv, struct Trapframe *tf);
int mon_startvmx (int argc, char **argv, struct Trapframe *tf);
int mon_LoadKernel (int argc, char **argv, struct Trapframe *tf);
int mon_GetHDInfo (int argc, char **argv, struct Trapframe *tf);
int mon_HDRead (int argc, char **argv, struct Trapframe *tf);

#endif	
