#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
//typedef unsigned int ULONG32;
unsigned int numCore;

ULONG64 __stdcall NBPCall (ULONG32 knock) {
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
			mov eax, [ebp + 8H];
			cpuid;
			mov SavedEax,eax;
			mov SavedEdx,edx;
		}
	}
	SetProcessAffinityMask(hProcess, dwProcessAffinityMask);
	__asm { 
	mov eax,SavedEax;
	mov edx,SavedEdx; //Restore the result returned by cpuid instruction	
	}

return;
}
int __cdecl main(int argc, char **argv) {
	ULONG64 knock;
	SYSTEM_INFO si;

	if (argc != 2) {
		printf ("bpknock <magic knock>\n");
		return 0;
	}
	
  	GetSystemInfo(&si);
  	numCore = si.dwNumberOfProcessors;
	
	knock = strtoul (argv[1], 0, 0);

	__try {
		printf ("knock answer: %d\n", NBPCall (knock));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	
	return 0;

}
