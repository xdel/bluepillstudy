#ifndef __KERN_MONITOR_H
#define __KERN_MONITOR_H
#ifndef ZION_KERNEL
# error "This is a Zion kernel header; user programs should not #include it"
#endif

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
int mon_cr0check (int argc, char **argv, struct Trapframe *tf);

#endif	
