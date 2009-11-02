#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define MAGIC_KNOCK 100
#define TEST_LOOP	5
ULONG64 __declspec(naked) NBPCall (char* exefile) {
	__asm { 
	push 	ebp
	mov	ebp, esp
	
	push esi
	push edi

	cpuid ;force all previous instructions to complete

	rdtsc
	mov esi,edx
	mov edi,eax ;Timer 1
	}


	system(exefile);
	__asm{


	rdtsc		;Timer 2
	sub edx,esi
	sub eax,edi ;Tdelta

	pop edi
	pop esi

	pop	ebp
	ret
	}
}
int __cdecl main(int argc, char **argv) {
	ULONG64 i,result;
	
	HANDLE hProcess = GetCurrentProcess(); 
	DWORD dwProcessAffinityMask, dwSystemAffinityMask; 
	GetProcessAffinityMask( hProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ); 

	SetProcessAffinityMask( hProcess, 0 );

	result = 0;
	__try {
  		NBPCall(argv[1]); 
		for(i = 0 ; i < TEST_LOOP ;i++)
		{
			result += NBPCall(argv[1]);
			system("del a.mp3");
			system("bunzip2 a.mp3.bz2");
		}
		//printf("%d, %d, %d, %d\n", eax, ebx, ecx, edx);
		printf ("Total Delta TSC: %d\n", result);
		printf ("Average Delta TSC: %d\n", result/TEST_LOOP);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	
	return 0;

}
