#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define MAGIC_KNOCK 100
#define TEST_LOOP	10000
ULONG64 __declspec(naked) NBPCall (ULONG32 knock) {
	__asm { 
	push 	ebp
	mov	ebp, esp
	
	push esi
	push edi

	cpuid
	rdtsc
	mov esi,edx
	mov edi,eax ;Timer 1

		cpuid

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
  		NBPCall(MAGIC_KNOCK); 
		for(i = 0 ; i < TEST_LOOP ;i++)
		{
			result += NBPCall(MAGIC_KNOCK);
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
