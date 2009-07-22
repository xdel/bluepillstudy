#include <windows.h>
#include <stdio.h>
#include <xADT_PDK.h>
#include "NtDefinitions.h"

typedef 
NTSYSAPI 
NTSTATUS
(NTAPI *fcnNtQueryInformationProcess)(
	IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, 
	OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength );

char gPath[MAX_PATH];
STARTUPINFO gStartup;
PROCESS_INFORMATION gProcInfo;
DEBUG_EVENT gEvent;
int gDebug=-1;
char gDll[]="ntdll";
char gApi[]="NtQueryInformationProcess";

Result lCheck();

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	return TRUE ;
}

EXPORT Result tst_NtQueryInfoProc_hook_detection(char *message)
{
	Result bRet=UNKNOWN;
	
	BOOL retcode=TRUE;

	if(IsDebuggerPresent()) {
		bRet=POSITIVE;
		return bRet;
	}

	GetModuleFileName(::GetModuleHandle(NULL), gPath, MAX_PATH);

	CreateProcess(gPath, NULL, NULL, NULL, NULL, CREATE_SUSPENDED|DEBUG_PROCESS, NULL, NULL, &gStartup, &gProcInfo);

	DebugActiveProcess(gProcInfo.dwProcessId);
	ResumeThread(gProcInfo.hThread);

	WaitForDebugEvent(&gEvent, INFINITE);
	ContinueDebugEvent(gProcInfo.dwProcessId, gProcInfo.dwThreadId, DBG_CONTINUE);

	switch(gEvent.dwDebugEventCode) {
		case CREATE_PROCESS_DEBUG_EVENT: {
			bRet=lCheck();
		 } break;
		case EXIT_PROCESS_DEBUG_EVENT: {
			return bRet;
	    }
	}
	return bRet;

}

EXPORT char* tst_NtQueryInfoProc_hook_detection_description()
{
	return "Test creating a debugged child process and querying it using NtQueryInfoProcess";
}

EXPORT char* tst_NtQueryInfoProc_hook_detection_name()
{
	return "NtQueryInfoProc_hook_detection Test";
}

EXPORT char* tst_NtQueryInfoProc_hook_detection_about()
{
	return "Implemented by Shub-Nigurrath, from an idea of Metr0/SnD";
}

Result lCheck() {

	fcnNtQueryInformationProcess fcn=(fcnNtQueryInformationProcess)::GetProcAddress(::GetModuleHandle(gDll), gApi);

	if(fcn==NULL)
		return UNKNOWN;

	gDebug=-1;

	// 	NtQueryInformationProcess(
	// 	IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, 
	// 	OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength 
	//  When called with ProcessInformationClass set to 7 (ProcessDebugPort constant), the system 
	//  will set ProcessInformation to -1 if the process is debugged.
	NTSTATUS dwStatus=fcn(gProcInfo.hProcess, ProcessDebugPort, &gDebug, sizeof(gDebug), NULL);

	Result retcode=(gDebug!=0)?NEGATIVE:POSITIVE; //inverted logic, if I can debug myself then I'm not debugged by another one

	::MessageBox(NULL, ((gDebug!=0)?("Child is debugged!, OK"):("Child is not debugged!, KO")),"Poc_c",MB_OK);

	return retcode;
}