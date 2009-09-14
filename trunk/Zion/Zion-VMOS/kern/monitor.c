#include <inc/lib/stdio.h>
#include <inc/lib/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/lib/stdlib.h>
#include <inc/arch/x86.h>
#include <inc/kern/console.h>
#include <inc/kern/monitor.h>
#include <inc/kern/kdebug.h>
#include <inc/kern/trap.h>
#include <inc/arch/cpu.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

//extern char 	cmdrec[][];
//extern int 		cmdcnt, cmdlen[];

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Backtrace stack", mon_backtrace },
	{ "exit", "Monitor exit", mon_exit },
	{ "reboot", "Restart the system.", mon_reboot },
	{ "cpuid", "Display processor identification values.", mon_cpuid },
	{ "cpuinfo", "Display CPU feature information.", mon_cpuinfo },
	{ "x", "Check the memory. ", mon_memcheck },
	{ "meminfo", "Display memory information.", mon_meminfo },
};
#define NCOMMANDS (int) (sizeof(commands)/sizeof(commands[0]))


int 
mon_cpuid( int argc, char **argv, struct Trapframe *tf )
{
	uint32_t 	eax, ebx, ecx, edx;
	uint32_t 	op, op_max_idx;

	// Basic information.
	cpuid(0, &op_max_idx, 0, 0, 0);
	for( op=0; op<=op_max_idx; op++) {
		cpuid(op, &eax, &ebx, &ecx, &edx);
		cprintf("[%08x H]: eax=0x%08x  ebx=0x%08x  ecx=0x%08x  edx=0x%08x\n", op, eax, ebx, ecx, edx);
	}//for
	
	// Extended function information.
	cpuid(0x80000000, &op_max_idx, 0, 0, 0);
	for( op=0x80000000; op<=op_max_idx; op++) {
		cpuid(op, &eax, &ebx, &ecx, &edx);
		cprintf("[%08x H]: eax=0x%08x  ebx=0x%08x  ecx=0x%08x  edx=0x%08x\n", op, eax, ebx, ecx, edx);
	}//for

	return 0;
}//mon_cpuinfo()



int 
mon_cpuinfo(int argc, char **argv, struct Trapframe *tf)
{
	if( cpuinfo() ) {
		return -1;
	}

	return 0;
}//mon_cpuinfo()



/***** Implementations of kernel monitor commands *****/
int 
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}



int 
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start %08x (virt)  %08x (phys)\n", _start, _start - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		(end-_start+1023)/1024);
	return 0;
}



#define 	FUNC_NAME_MAX_LEN 	100
int mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t 				ebp, eip;
	uint32_t 				arg[4];
	uint32_t 				LineNum = 0;
	struct Eipdebuginfo 	dbginfo;
	char 					func_name[FUNC_NAME_MAX_LEN];
	int						i;
	extern char bootstack[];	// Lowest addr in boot-time kernel stack

	cprintf("Stack backtrace:\n");	// Print title.

	ebp = read_ebp(); 	// Read current EBP register.
	do {
		eip = *((uint32_t *)(ebp + 4)); 	// Read EIP from stack, which is prior to the pushed EBP.
		for( i=0; i<4; i++) { 	// Read four pushed elements before call.
			arg[i] = *((uint32_t *)(ebp + 8 + 4*i));
		}//for

		// Print backtrace info.
		cprintf("  %d: ebp %08x  eip %08x  args %08x %08x %08x %08x\n", \
				LineNum++, ebp, eip, arg[0], arg[1], arg[2], arg[3]);

		debuginfo_eip((uintptr_t)eip, &dbginfo); 	// Read debug info via pushed EIP.

		// Translate calling function's name.
		for( i=0; i<dbginfo.eip_fn_namelen && i<FUNC_NAME_MAX_LEN; i++ ) { 	
			func_name[i] = dbginfo.eip_fn_name[i];
		}//for
		func_name[i] = '\0';

		// Print debug info.
		cprintf("      %s:%x: %s+%2x (%d arg)\n", dbginfo.eip_file, dbginfo.eip_line, \
				func_name, (eip - dbginfo.eip_fn_addr), dbginfo.eip_fn_narg);

		// Refresh ebp to prior level call.
		ebp = *((uint32_t *)ebp);
	} while( ebp != 0 );	// Continue until reach top calling.

	return 0;
}//mon_backtrace()



/* 
 * Monitor exit
 */
int 
mon_exit(int argc, char **argv, struct Trapframe *tf)
{
	if( tf==NULL )	// Skip if this function runs in normal monitor.
		return 0;

	// Trap relevant dispossal.
	switch( tf->tf_trapno ) {
		case T_BRKPT: 
			return -1;
			break;
		default: break;
	}//switch

	return 0;
}//mon_exit()



int 
mon_reboot( int argc, char** argv, struct Trapframe* tf )
{
	cprintf("\n[ System Restarting! ]\n");
	outb(0x92, 0x3);

	return 0;
}//mon_reboot()



int
mon_memcheck( int argc, char **argv, struct Trapframe *tf )
{
	uint32_t		n, *vaddr;
    
	if ( argc < 2 || argc > 3 ) { 		// Argument arbitration
		return 0;
	}//if
	
	if ( argc == 2 ) { 	// for 2 arguments case
		n = 1;
		vaddr = (uint32_t *)(str2addr(argv[1]));
	} else { 	// for 3 arguments case
		n = str2num(argv[1]);
		vaddr = (uint32_t *)(str2addr(argv[2]));
	}//if...else
	
	for ( uint32_t i=0; i<n; ) {
		for ( uint32_t j = 0; j < 2 && i < n; j++, vaddr++, i+=4  ) {
			cprintf("\t[0x%08x]: %08x ", vaddr, *vaddr);
		}//for(j)
		cprintf("\n");
	}//for(i)
	
	return 0;
}//mon_memcheck()



#define 	MemSize_paddr 			0xf0009000
#define 	MCRNumber_paddr 		0xf0009008
#define 	MemInfo_paddr 			0xf0009010
int
mon_meminfo ( int argc, char **argv, struct Trapframe *tf )
{
	uint32_t 		MemSize = *((uint32_t *)MemSize_paddr);
	uint32_t 		MCRNumber = *((uint32_t *)MCRNumber_paddr);
	uint32_t 		*pMemInfo = (uint32_t *)MemInfo_paddr;
	
	// Print table title.
	cprintf("\tBaseAddrLow  BaseAddrHigh LengthLow    LengthHigh   Type\n");
	
	for ( uint32_t i=0; i<MCRNumber; i++ ) {
		// 每次得到一个ARDS(Address Range Descriptor Structure)结构
		for ( uint32_t j=0; j<5; j++, pMemInfo++)	 {		// 每次得到一个ARDS中的成员，共5个成员
			// 依次显示：BaseAddrLow，BaseAddrHigh，LengthLow，LengthHigh，Type
			cprintf("\t%08x ", *pMemInfo);
		}//for(j)
		cprintf("\n");
	}//for(i)
	
	cprintf("\t--------------------------------------------------------------\n");
	cprintf("\tMemory Size: %u MB (%u bytes)\n", MemSize/0x100000, MemSize);
	
	return 0;
}//mon_meminfo()



/***** Kernel monitor command interpreter *****/
#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void 
monitor (struct Trapframe *tf)
{
	char *buf;

	cprintf("\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("Zion:> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}//while
}//monitor()
