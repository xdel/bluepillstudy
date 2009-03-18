#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
//typedef unsigned int ULONG32;

ULONG64 __stdcall NBPCall (ULONG32 knock) {
	__asm { 
	mov eax, [ebp + 8H]
	push	ebx
	push	ecx
	cpuid
	pop	ecx
	pop	ebx
	
	}
return;
}
int __cdecl main(int argc, char **argv) {
	ULONG32 knock;
	if (argc != 2) {
		printf ("bpknock <magic knock>\n");
		return 0;
	}
	knock = strtoul (argv[1], 0, 0);

	__try {
		printf ("knock answer: %d\n", NBPCall (knock));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	
	return 0;

}
