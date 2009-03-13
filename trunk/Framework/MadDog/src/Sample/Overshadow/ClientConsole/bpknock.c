#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define START_RECORDING		1000 //Used to tell the hypervisor to start run the target program
#define PING				1500 //Used to retrieve data while the target program is still running
#define END_RECORDING		2000 //Used to tell the hypervisor the target program should be killed

typedef struct _Parameter
{
	//either to be START_RECORD or END_RECORD or PING
	ULONG32 actionType;
	//monitor the indicated process, 0 means all processes. 
	ULONG32 pid;
} Parameter,*PParameter

typedef struct _Result
{
	ULONG32 pid;
}
/**
 * Pass <pParameter> Argument to Context Counter
 * Return: The virtual address of result struct.
 */
ULONG32 __stdcall HyperCall (PParameter pParameter) {
	__asm { 

		mov eax, [ebp + 8H]
		cpuid	
	}
	return;
}
int main(int argc, char **argv) {
	ULONG32 pid;
	if (argc != 2) {
		printf ("ContextCounter <process id>\n");
		return 0;
	}
	pid = strtoul (argv[1], 0, 0);

	__try {
	ULONG32 resultVadr = PassArgs(pid); 
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	
	return 0;

}
