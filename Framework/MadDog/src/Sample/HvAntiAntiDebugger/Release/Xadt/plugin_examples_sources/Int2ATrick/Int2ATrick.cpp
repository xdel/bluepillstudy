#include <windows.h>
#include <stdio.h>
#include "xADT_PDK.h"

#define TIME_INTERVAL	50

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	return TRUE ;
}

DWORD WINAPI KiGetTickCount(void) {
	DWORD res=0;

	__asm {
		//int 2Ah returns "the number of milliseconds that have elapsed since
		//the system was started" in eax register, it also modifies edx register.
		int     2ah             //call nt!KiGetTickCount
		mov		res, eax
	}
	return res;
}

EXPORT Result tst_Int2ATrick(char *message)
{
	Result bRet=NEGATIVE;

	DWORD	dwTick=0, iStartTime=0;

	/* >= NT 4.0 */
	if( GetVersion() >= 0x80000000 ) {
		sprintf_s(message, 60, "Windows NT 4.0 or higher is required to use this plugin!");
		return bRet;
	}

	iStartTime=KiGetTickCount();
	
	Sleep(TIME_INTERVAL);

	dwTick=KiGetTickCount();
	dwTick -= iStartTime;
	
	if(dwTick>TIME_INTERVAL) //if I'm debugging it takes longer..
		return POSITIVE;
	
	return NEGATIVE;
}

EXPORT char* tst_Int2ATrick_description()
{
	return "Test using the assembler int 2A and KiGetTickCount";
}

EXPORT char* tst_Int2ATrick_name()
{
	return "Int2ATrick Test using KiGetTickCount";
}

EXPORT char* tst_Int2ATrick_about()
{
	return "Implemented by Shub-Nigurrath, from an idea ReWolf";
}
