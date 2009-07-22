#include <windows.h>
#include <stdio.h>
#include "xADT_PDK.h"

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	return TRUE ;
}

EXPORT Result tst_DeleteFiber(char *message)
{
	char fib[1024] = {0};	
	try { 
		DeleteFiber(fib); 
	}
	catch(...) {
		return NEGATIVE;
	}

	if(GetLastError() == 0x00000057)
		return NEGATIVE;
	else
		return POSITIVE;

	return NEGATIVE;
}

EXPORT char* tst_DeleteFiber_description()
{
	return "Test using the DeleteFiber API";
}

EXPORT char* tst_DeleteFiber_name()
{
	return "DeleteFiber Test";
}

EXPORT char* tst_DeleteFiber_about()
{
	return "Implemented by Shub-Nigurrath, from an idea of evilcry (10x)";
}
