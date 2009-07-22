#include <afx.h>
#include <stdio.h>
#include "SICETricks.h"

#define _countof(array) \
(sizeof(array)/sizeof(array[0]))



int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	return TRUE ;
}

EXPORT char* tst_SICETricks_description()
{
	return "Uses several ways to detect SICE";
}

EXPORT char* tst_SICETricks_name()
{
	return "SICE Presence Tests";
}

EXPORT char* tst_SICETricks_about()
{
	return "Implemented by Shub-Nigurrath, partially from CrackLatinos tutorials code";
}


//////////////////////////////////////////////////////////////////////////
//Tricks
//////////////////////////////////////////////////////////////////////////


// Función: SoftIceCommondDrivers
// Descripción: Comprueba si el controlador VXD del SoftIce para NT está cargado en memória, para ello utiliza la función
// CreateFile del API de Windows, esta función se encarga (entre otras cosas) de establecer comunicación con los
// dispositivos VXD.
// Retorna: TRUE si SoftIce está en memoria.
__inline bool SoftIceCommondDrivers()
{
    HANDLE hFile; 
	
    //checked drivers names with escape sequences
	char drivers[][MAX_PATH]={"\\\\.\\SICE", "\\\\.\\NTICE", "\\\\.\\SIWVID", "\\\\.\\FROGICE"};
	
	for (int idx=0; idx <_countof(drivers); idx++)
	{	
		hFile = CreateFile( drivers[idx],
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	
		if( hFile != INVALID_HANDLE_VALUE )
		{
	        CloseHandle(hFile);
			return true;
		}
	}
	
    return false;
}


//These two keys are created by SoftICE
//HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Uninstall\SoftICE
//- #2 : HKEY_LOCAL_MACHINE\Software\NuMega\SoftICE
bool FindSoftIceRegistryKeys() {

	const char key1[]="Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\SoftICE";
	const char key2[]="Software\\NuMega\\SoftICE";

	HKEY hkRead;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, key1, 0, KEY_READ,&hkRead)==ERROR_SUCCESS)
		return true;
	
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, key2, 0, KEY_READ,&hkRead)==ERROR_SUCCESS)
		return true;
	
	return false;
}	


bool BoundsCheckerDetected() {

	bool bret=false;

	try {
		_asm {
				push  ebp
				mov   ebp, 'BCHK'
				mov   ax, 4
				int   3
				cmp   ax, 4
				pop   ebp
				jne   softice_detected
				mov   ax, 0
				jmp   all_ok
			softice_detected:
				mov   bret, 1
			all_ok:
		}
	}
	catch(...) {
		return false;
	}

	return bret;
}

void __SoftIceINT01(bool *bret) {

	*bret=false;

	//	With a DPL of 1 (no SoftICE present) the generated exception is an EXCEPTION_ACCESS_VIOLATION (C0000005h) type fault, 
	//  whereas DPL=3 results in an EXCEPTION_SINGLE_STEP (80000004h) type trap.
	// SoftICE -> EXCEPTION_SINGLE_STEP
	// !SoftIce -> EXCEPTION_ACCESS_VIOLATION

	__try
	{
		_asm {int 01h}
	}
	__except(GetExceptionCode()==EXCEPTION_SINGLE_STEP ? 
				EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		*bret=true;
	}
}



bool SoftIceINT01() {
	//I must call the real call into a try-catch block of another function because stack gets mad on previous call so this fix
	//all the things without having to worry and without loosing the test result.
	//Moreover the final ASM code is a little more complex to follow!
	bool bret=0;
	try{
		__SoftIceINT01(&bret);
	}
	catch(...) {
		return bret;
	}

	return bret;
}

void __SoftIceINT41(bool *bret) {
	
	*bret=false;
	
	//	With a DPL of 1 (no SoftICE present) the generated exception is an EXCEPTION_ACCESS_VIOLATION (C0000005h) type fault, 
	//  whereas DPL=3 results in an EXCEPTION_SINGLE_STEP (80000004h) type trap.
	// SoftICE -> EXCEPTION_SINGLE_STEP
	// !SoftIce -> EXCEPTION_ACCESS_VIOLATION
	
	__try
	{
		_asm {int 41h}
	}
	__except(GetExceptionCode()==EXCEPTION_SINGLE_STEP ? 
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		*bret=true;
	}
}

bool SoftIceINT41() {
	//I must call the real call into a try-catch block of another function because stack gets mad on previous call so this fix
	//all the things without having to worry and without loosing the test result.
	//Moreover the final ASM code is a little more complex to follow!
	bool bret=0;
	try{
		__SoftIceINT41(&bret);
	}
	catch(...) {
		return bret;
	}

	return bret;
}


void __SoftIceVXDDetectedINT2F(bool *bret) {

	*bret=false;
 
//  INT 2F - MS Windows - GET DEVICE API ENTRY POINT
// 	AX = 1684h
// 	BX = virtual device (VxD) ID (see #1325)
// 	ES:DI = 0000h:0000h
//  Return: ES:DI -> VxD API entry point, or 0:0 if the VxD does not support an API
//  Note:	some Windows enhanced-mode virtual devices provide services that
// 			applications can access.  For example, the Virtual Display Device
// 			(VDD) provides an API used in turn by WINOLDAP.
	  
	__try {
		_asm {
			mov   ax, 1684h  
			mov   bx, 0202h  // VXD ID for SoftIce
			xor   di, di     //set DI=0
			mov   es, di     //set ES=0
			int   2Fh
		}
	}
	__finally {
		//I must use local extra ASM variables because direct ASM branches are not allowed inside a __finally block
		//the compiler generates an error and doesn't allow compilation.

		WORD wDi=0;
		_asm{
			mov   ax, es
			add   di, ax
			mov   wDi, di
		}
		if(wDi!=0x23) {
			*bret=true;
			char str[20];
			sprintf(str,"wDi=%x",wDi);
			::MessageBox(NULL,str,"xADT",MB_OK);
		}
		else 
		{
			bret=false;
			_asm{
				mov   ax, 0
			}
		}
	}

}

bool SoftIceVXDDetectedINT2F() {
	
	//I must call the real call into a try-catch block of another function because stack gets mad on previous call so this fix
	//all the things without having to worry and without loosing the test result.
	//Moreover the final ASM code is a little more complex to follow!
	bool bret=0;
	try{
		__SoftIceVXDDetectedINT2F(&bret);
	}
	catch(...) {
		return bret;
	}

	return bret;
}

//////////////////////////////////////////////////////////////////////////

//Placing the function after the tests allows to not write prototypes, which is boring!
EXPORT Result tst_SICETricks(char *message)
{
	Result bRet=NEGATIVE;
	
	int idx=0;
	char results_names[][MAX_PATH] = {"SoftIceCommondDrivers", "FindSoftIceRegistryKeys", 
									  "BoundsCheckerDetected", "SoftIceVXDDetectedINT2F",
									  "SoftIceINT01", "SoftIceINT41"};
	bool *results=NULL;

	int i=_countof(results_names);
	results=(bool*)malloc(sizeof(bool)*_countof(results_names));
	if(results==NULL)
		return UNKNOWN;
	
	memset(results,0,_countof(results_names));
	
	idx=0;
	results[idx++]=SoftIceCommondDrivers();
	results[idx++]=FindSoftIceRegistryKeys();
	results[idx++]=BoundsCheckerDetected();
	results[idx++]=SoftIceVXDDetectedINT2F();
	results[idx++]=SoftIceINT01();
	results[idx++]=SoftIceINT41();
	
	sprintf(message, "Positive tests: ");
	for (idx=0; idx<_countof(results_names); idx++) {
		if(results[idx]==true) {
			strcat(message,results_names[idx]);
			strcat(message, ", ");
		}
	}

	//Just checks if all the tests were negative then properly writes the output string.
	for (idx=0; idx<_countof(results_names); idx++) {
		if(results[idx]==true) 
			break;
	}
	if(idx==_countof(results_names))
		strcat(message,"nothing");

	
	//Final Check, checks if at least one test is positive
	for (idx=0; idx<_countof(results_names); idx++)
		if(results[idx]!=0)
			bRet=POSITIVE;
		return bRet;
		
}
