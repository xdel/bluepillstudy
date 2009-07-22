// NtQueryInfoProc_hook_detection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NtQueryInfoProc_hook_detection.h"
#include "NtDefinitions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef 
NTSYSAPI 
NTSTATUS
(NTAPI *fcnNtQueryInformationProcess)(
IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, 
OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength ); 

// The one and only application object

CWinApp theApp;

using namespace std;

int _main(int argc, TCHAR* argv[]);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		nRetCode=_main(argc, argv);
	}

	return nRetCode;
}

char gPath[MAX_PATH];
STARTUPINFO gStartup;
PROCESS_INFORMATION gProcInfo;
DEBUG_EVENT gEvent;
int gDebug=-1;
char gDll[]="ntdll";
char gApi[]="NtQueryInformationProcess";

BOOL lCheck();

//real function!
int _main(int argc, char* argv[]) {
	
	BOOL retcode=TRUE;

 	if(IsDebuggerPresent()) {
 		retcode=TRUE;
 		return retcode;
 	}

	GetModuleFileName(::GetModuleHandle(NULL), gPath, MAX_PATH);

	CreateProcess(gPath, NULL, NULL, NULL, NULL, CREATE_SUSPENDED|DEBUG_PROCESS, NULL, NULL, &gStartup, &gProcInfo);

	DebugActiveProcess(gProcInfo.dwProcessId);
	ResumeThread(gProcInfo.hThread);

	WaitForDebugEvent(&gEvent, INFINITE);
	ContinueDebugEvent(gProcInfo.dwProcessId, gProcInfo.dwThreadId, DBG_CONTINUE);

	switch(gEvent.dwDebugEventCode) {
		case CREATE_PROCESS_DEBUG_EVENT: {
			lCheck();
		} break;
		case EXIT_PROCESS_DEBUG_EVENT: {
			::ExitProcess(0);
		}
	}

	return retcode;
}

BOOL lCheck() {
	
	fcnNtQueryInformationProcess fcn=(fcnNtQueryInformationProcess)::GetProcAddress(::GetModuleHandle(gDll), gApi);

	if(fcn==NULL)
		return FALSE;

	gDebug=-1;

// 	NtQueryInformationProcess(
// 	IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, 
// 	OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength 
//  When called with ProcessInformationClass set to 7 (ProcessDebugPort constant), the system 
//  will set ProcessInformation to -1 if the process is debugged.
	NTSTATUS dwStatus=fcn(gProcInfo.hProcess, ProcessDebugPort, &gDebug, sizeof(gDebug), NULL);

	BOOL retcode=(gDebug!=0)?FALSE:TRUE; //inverted logic, if I can debug myself then I'm not debugged by another one
	
	::MessageBox(NULL, ((gDebug!=0)?("Child is debugged!, OK"):("Child is not debugged!, KO")),"Poc_c",MB_OK);

	return retcode;
}