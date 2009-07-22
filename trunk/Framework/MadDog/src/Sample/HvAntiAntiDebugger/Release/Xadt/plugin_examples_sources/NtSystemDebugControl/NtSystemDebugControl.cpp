/* Original idea by Giuseppe 'Evilcry' Bonfa'
* Website: http://evilcry.altervista.org
* Website: http://evilcry.netsons.org
*
* Adapted for xADT by Shub-Nigurrath
*
* NOTE: This test could be extended and improved to better use the information 
*       returned by the API NTSystemDebugControl!
*
*/

#define VS60 defined(_MSC_VER) && _MSC_VER<1300 //used to allow specific code for VS 6.0, otherwise is VS2008

#include <afxwin.h>
#include <stdio.h>
#include "nsdc.h"
#include "myDOS.h"
#include "xADT_PDK.h"

#pragma comment(lib,"advapi32.lib")

#define PKPCR 0xffdff000

#if VS60 
#define printf_s printf
using namespace MyDOS;
#endif

fcnNtSystemDebugControl GetNtSystemDebugControl() {
	HMODULE hmod=::LoadLibrary("ntdll.dll");
	if(hmod!=NULL) {
		HANDLE hproc=GetProcAddress(hmod, (LPCSTR)TEXT("NtSystemDebugControl"));
		if (hproc== NULL) {      
			printf("[KO] failed to get NtSystemDebugControl\n");
			return NULL;
		}
		else {
			return (fcnNtSystemDebugControl)hproc;
		}
	}
	
	::FreeLibrary(hmod);
	
	return NULL;
}

fcnNtSystemDebugControl _gfcn;


Result GainPrivileges(void)
{
	HANDLE hToken;
	BOOL bEnablePRivilege = TRUE;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		printf_s("[Warn] Can't Gain Privileges \n\n");
		return(UNKNOWN);
	}

	if(!LookupPrivilegeValueA(NULL,"SeDebugPrivilege",&luid))
	{
		printf_s("[Warn] SeDebugPrivilege Setup Failed \n\n");
		return(UNKNOWN);
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),
		FALSE,FALSE))
	{
		return(UNKNOWN);
	}

	CloseHandle(hToken);
	return (NEGATIVE);
}

Result DetectionMethod1(void) //PEB: IsDebuggerPresent
{	
	SYSDBG_VIRTUAL Memory;
	ULONG Status;
	ULONG Addr;
	UINT8 BDbgd = NULL;
	bool Detected = false;

	//**********************************************

	Memory.Address = (PVOID)(PKPCR+0x124); // KPCRB -> _KTHREAD
	Memory.Buffer = &Addr;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An Error Occurred in Struct Navigation\n\n");
		return (UNKNOWN);
	}

	//**********************************************

	Memory.Address = (PVOID)(Addr+0x220); //ETHREAD -> ThreadsProcess
	Memory.Buffer = &Addr;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An Error Occurred in Struct Navigation\n\n");
		return (UNKNOWN);
	}

	//**********************************************

	Memory.Address = (PVOID)(Addr+0x1B0); // _PEB
	Memory.Buffer = &Addr;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An Error Occurred in Struct Navigation\n\n");
		return (UNKNOWN);
	}

	//**********************************************

	Memory.Address = (PVOID)(Addr+0x002); // _PEB -> Being Debugged
	Memory.Buffer = &BDbgd;
	Memory.Request = sizeof(UINT8);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An Error Occurred in Struct Navigation\n\n");
		return (UNKNOWN);
	}

	if ( BDbgd != NULL )	
		Detected = true;

	return ((Detected)?POSITIVE:NEGATIVE);
}

Result DetectionMethod2(void) //PEB: NtGlobalFlags
{	
	SYSDBG_VIRTUAL Memory;
	ULONG Status;
	ULONG Addr;
	ULONG NtGF = NULL;
	bool Detected = false;

	//**********************************************

	Memory.Address = (PVOID)(PKPCR+0x124); // KPCRB -> _KTHREAD
	Memory.Buffer = &Addr;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An Error Occurred in Struct Navigation\n\n");
		return (UNKNOWN);
	}

	//**********************************************

	Memory.Address = (PVOID)(Addr+0x220); //ETHREAD -> ThreadsProcess
	Memory.Buffer = &Addr;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An Error Occurred in Struct Navigation\n\n");
		return (UNKNOWN);
	}

	//**********************************************

	Memory.Address = (PVOID)(Addr+0x1B0); // _PEB
	Memory.Buffer = &Addr;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An Error Occurred in Struct Navigation\n\n");
		return (UNKNOWN);
	}

	//**********************************************

	Memory.Address = (PVOID)(Addr+0x068); // _PEB -> NtGlobalFlags
	Memory.Buffer = &NtGF;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An Error Occurred in Struct Navigation\n\n");
		return (UNKNOWN);
	}

	if ( NtGF == 0x70 )
		Detected = true;

	return ((Detected)?POSITIVE:NEGATIVE);
}

Result DetectionMethod3(void)
{
	SYSDBG_VIRTUAL Memory;
	ULONG Status;
	ULONG Addr;
	ULONG DebugPort = NULL;
	bool Detected = false;

	Memory.Address = (PVOID)(PKPCR+0x124); // _KPCRB Struct -> _KTHREAD
	Memory.Buffer = &Addr;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory, 
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An error occurred in the Struct Navigation\n\n");
		return (UNKNOWN);
	}	

	//***************************************************

	Memory.Address = (PVOID)(Addr+0x220); //ETHREAD -> ThreadsProcess
	Memory.Buffer = &Addr;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An error occurred in the Struct Navigation\n\n");
		return (UNKNOWN);
	}

	//***************************************************

	Memory.Address = (PVOID)(Addr+0x0BC);
	Memory.Buffer = &DebugPort;
	Memory.Request = sizeof(ULONG);

	Status = _gfcn(DebugSysReadVirtual, &Memory,
		sizeof(SYSDBG_VIRTUAL), NULL, 0, NULL);

	if ( Status != STATUS_SUCCESS )
	{
		printf_s("[Warn] An error occurred in the Struct Navigation\n\n");
		return (UNKNOWN);
	}

	if ( DebugPort != NULL )
		Detected = true;

	return ((Detected)?POSITIVE:NEGATIVE);
}

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	return TRUE ;
}

EXPORT Result tst_NtSystemDebugControl(char *message)
{
	Result res=NEGATIVE;
	UINT positivetests=0;

	MyDOS::AllocConsole();
	MyDOS::DOSclrscr();
	
	if ( GainPrivileges() == UNKNOWN )
	{
		printf_s("[Warn] Can't Gain Privileges\n Exiting..\n");
		return (UNKNOWN);
	}

	_gfcn=GetNtSystemDebugControl();

	//////////////////////////////////////////////////////////////////////////
	//Demonstration of Detect Methods

	//method 1
	printf_s("---------------------------------------\n"
			 "Detection Method 1 (IsDebuggerPresent):\n");
	if ( DetectionMethod1() == true )
		res=POSITIVE;
	else
		res=NEGATIVE;

	if(res==POSITIVE)
		positivetests++;
	printf_s( "[Result] The Process %s Debugged\n\n", ((res==POSITIVE)?"IS":"IS NOT") );
	

	//method2
	printf_s("-------------------------------\n"
		"Detection Method 2 (NtGlobalFlags):\n");
	if ( DetectionMethod2() == true )
		res=POSITIVE;
	else
	res=NEGATIVE;

	if(res==POSITIVE)
		positivetests++;
	printf_s( "[Result] The Process %s Debugged!\n\n", ((res==POSITIVE)?"IS":"IS NOT") );
	
	//method3
	printf_s("-------------------------------\n"
		     "Detection Method 2 (DebugPort):\n");
	if ( DetectionMethod3() == true )
		res=POSITIVE;
	else
	res=NEGATIVE;

	if(res==POSITIVE)
		positivetests++;
	printf_s( "[Result] The Process %s Debugged!\n\n", ((res==POSITIVE)?"IS":"IS NOT") );


	//////////////////////////////////////////////////////////////////////////
	//summing up results!
	if(positivetests) {
	#if VS60
		sprintf(message, "You have been caught %d times (see DOS window for details)! Hide better", positivetests);
	#else
		sprintf_s(message, MAX_PATH, "You have been caught %d times (see DOS window for details)! Hide better\n\n", positivetests);
	#endif
		printf_s("\n%s\n", message);
		::MessageBox(NULL, message, "xADT",MB_OK|MB_SETFOREGROUND);

	}
	else {
	#if VS60
		sprintf(message, "Well done you successfully performed all the test without being catched!");
	#else
		sprintf_s(message, MAX_PATH, "Well done you successfully performed all the test without being catched!\n\n");
	#endif
		printf_s("\n%s\n", message);
		::MessageBox(NULL, message, "xADT",MB_OK|MB_SETFOREGROUND);

	}

	MyDOS::FreeConsole();

	return ((positivetests>0)?POSITIVE:NEGATIVE);
}

EXPORT char* tst_NtSystemDebugControl_description()
{
	return "A collection of Tests using the NtSystemDebugControl API";
}

EXPORT char* tst_NtSystemDebugControl_name()
{
	return "NtSystemDebugControl Tests";
}

EXPORT char* tst_NtSystemDebugControl_about()
{
	return "Implemented by Shub-Nigurrath, from an idea of evilcry (10x)";
}
