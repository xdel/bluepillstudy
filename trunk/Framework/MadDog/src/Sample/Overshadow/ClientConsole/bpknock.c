#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define START_RECORDING		1000 //Used to tell the hypervisor to start run the target program
#define PING				1500 //Used to retrieve data while the target program is still running
#define END_RECORDING		2000 //Used to tell the hypervisor the target program should be killed
#define TEST_PASSINVALUE	8888 //Only Used in Debug Mode. Hypervisor won't change its current recording state.

typedef struct _Parameter
{
	//monitor the indicated process, 0 means all processes. 
	ULONG32 pid;
} Parameter,*PParameter;

typedef struct _Result
{
	ULONG32 pid;
} Result, *PResult;

unsigned int numCore;

/**
 * Pass <pParameter> Argument to Context Counter
 * Return: The virtual address of result struct.
 * The content of the structure <pParameter> points to will be copied to the kernel memory.
 * <actionType>:either to be START_RECORD or END_RECORD or PING
 */
ULONG32 __stdcall HyperCall (ULONG32 actionType, PParameter pParameter) {
	ULONG32 SavedEax;
	ULONG32 SavedEdx;
	unsigned int cProcessorNumber;
	HANDLE hProcess = GetCurrentProcess(); 
	DWORD dwProcessAffinityMask, dwSystemAffinityMask; 
	GetProcessAffinityMask( hProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ); 

	for (cProcessorNumber = 0; cProcessorNumber < numCore; cProcessorNumber++) 
	{
		//KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));
		SetProcessAffinityMask( hProcess, (ULONG)(1<<cProcessorNumber) );
		__asm { 
			mov eax, actionType; get <actionType> value
			mov edx, pParameter; get pParameter value
			cpuid	
			mov SavedEax,eax;
			mov SavedEdx,edx;
		}
	}
	//SetProcessAffinityMask(hProcess, dwProcessAffinityMask);
	__asm { 
		mov eax,SavedEax;
		mov edx,SavedEdx; //Restore the result returned by cpuid instruction	
	}
	return;
}
int main(int argc, char **argv) {
	Parameter passin;
	SYSTEM_INFO si;
	if (argc != 2) {
		printf ("ContextCounter <process id>\n");
		return 0;
	}
	//Retrieve Global Environment Configurations - Get Total Core Number.
	GetSystemInfo(&si);
  	numCore = si.dwNumberOfProcessors;

	//Construct Parameter struct
	passin.pid = strtoul (argv[1], 0, 0);
	
	__try {
	ULONG32 resultVadr = HyperCall(TEST_PASSINVALUE,&passin); 
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	
	return 0;

}
