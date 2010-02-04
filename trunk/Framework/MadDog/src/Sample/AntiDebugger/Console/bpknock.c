#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
//#include <dos.h>
#include <conio.h>

#define FN_PROTECT			0x500
#define FN_UNPROTECT		0x501

VOID __stdcall ProtectSomeone(ULONG PID){
	__asm{
		mov eax, FN_PROTECT
		mov ecx, PID
		cpuid
	}
}
VOID __stdcall UnProtectSomeone(ULONG PID){
	__asm{
		mov eax, FN_UNPROTECT
		mov ecx, PID
		cpuid
	}
}

int __cdecl main(int argc, char **argv) {
	ULONG PID,FUNC;
	if (argc != 3 && argc !=1) {
		printf ("Console <1/0> <PID>\n");
		return 0;
	}

	FUNC = atoi(argv[1]);
	PID = atoi(argv[2]);
	
	__try {
		if(FUNC)
		{
			printf ("Godmode is on\n");
			ProtectSomeone(PID);
		}
		else
		{
			printf ("Godmode is off\n");
			UnProtectSomeone(PID);
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	return 0;
}
