#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

void print(char *s) {
int i;
for (i = 0; i < 12; i++)
  printf("%c", s[i]);
}

ULONG32 __declspec(naked) NBPCall (ULONG32 knock) {
	__asm { 
	push 	ebp
	mov	ebp, esp
		cpuid

	push    edx;
	push	ebx;
	push	eax;
	mov ecx, esp;
	push	ecx;
	call print;
	add	esp, 16;
	mov	esp, ebp
	pop	ebp
	ret
	}
}
int __cdecl main(int argc, char **argv) {
	ULONG32 knock;
	if (argc != 2) {
		printf ("bpknock <magic knock>\n");
		return 0;
	}
	knock = strtoul (argv[1], 0, 0);

	__try {
  		NBPCall(knock); 
		//printf("%d, %d, %d, %d\n", eax, ebx, ecx, edx);
		//printf ("knock answer: %#x\n", NBPCall (knock));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	
	return 0;

}
