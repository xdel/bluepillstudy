#include <afxwin.h>         // MFC core and standard components
#include <Tlhelp32.h>
#include "psapi.h"

#include "ParentProcess.h"
#include "NTDefinitions.h"

typedef NTSTATUS (NTAPI *fcnZwQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

#define _countof(array) \
(sizeof(array)/sizeof(array[0]))

BOOL FindFileNamefromID( DWORD dwProcessId, CString &retStr);

//List of searched strings, names must be all caps
char Parents[][MAX_PATH]={
	"OLLY", 
	"SOFTICE", 
	"PROCEXPL", 
	"LORDPE",
	"PEDUMPER",
	"FILEMON",
	"REGMON",
	"IMPORTREC",
	"DEBUGAPI",
	"PEXPLORER", 
	"MSDEV", 
	"ZDEBUGGER", 
	"MASM", 
	"TASM", 
	"IDA"
};

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
     return TRUE ;
}

EXPORT Result tst_ParentProcess(char *message)
{

	FARPROC addrIDP;
	HINSTANCE hKer;
	fcnZwQueryInformationProcess fcn;
	
	hKer = GetModuleHandle("NTDLL");
	addrIDP = GetProcAddress(hKer, "ZwQueryInformationProcess");
	
	//Check API
	if (addrIDP==NULL) {
		return UNKNOWN;
	}

	fcn=(fcnZwQueryInformationProcess)addrIDP;

	HANDLE hproc=OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	BOOL bRet=FALSE;
	
	if(hproc) {
		ULONG bytes=0;
		PROCESS_BASIC_INFORMATION ProcessInfo;
		
		NTSTATUS ntstatus = fcn(hproc, ProcessBasicInformation, (PVOID)&ProcessInfo, 
			sizeof(PROCESS_BASIC_INFORMATION), &bytes);
		
		if(ntstatus != STATUS_SUCCESS) {
			CloseHandle(hproc);
			return UNKNOWN;
		}
		
		if(bytes<=0) {
			CloseHandle(hproc);
			return UNKNOWN;
		}
		
		CString strParentProcName;
		if (!FindFileNamefromID(ProcessInfo.InheritedFromUniqueProcessId, strParentProcName)) 
		{
			if(message!=NULL) sprintf(message, "ParentUniqueProcessId=%.8X. Cannot resolve process's name");
			return UNKNOWN;
		}

		if(message!=NULL) sprintf(message, "Process's Parent \"%s\"", strParentProcName);
		
		strParentProcName.MakeUpper();

		//Processes the black list of debugger
		for (int idx=0; idx<_countof(Parents); idx++)
			if(strParentProcName.Find(Parents[idx])!=-1) {
				if(message!=NULL) sprintf(message, "Detected an invalid Parent \"%s\"", strParentProcName);
				CloseHandle(hproc);
				return POSITIVE;
			}

		CloseHandle(hproc);
	}
	
	return ((bRet)?(POSITIVE):(NEGATIVE));
}

EXPORT char* tst_ParentProcess_description()
{
	return "Test looking if the ParentProcess is a debugger";

}

EXPORT char* tst_ParentProcess_name()
{
	return "ParentProcess Test";
}

EXPORT char* tst_ParentProcess_about() {
	return "Implemented by Shub-Nigurrath";
}


BOOL FindFileNamefromID( DWORD dwProcessId, CString &retStr) {
    HANDLE         hProcessSnap = NULL; 
    BOOL           bRet      = FALSE; 
	DWORD		   ProcIDRet = NULL;
    PROCESSENTRY32 pe32      = {0}; 
	
    //  Take a snapshot of all processes in the system. 
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	
    if (hProcessSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 
	
    //  Fill in the size of the structure before using it. 
    pe32.dwSize = sizeof(PROCESSENTRY32); 
	
    //  Walk the snapshot of the processes, and for each process, 
    //  display information. 
    if (Process32First(hProcessSnap, &pe32)) 
    { 
        //DWORD         dwPriorityClass; 
        BOOL          bGotModule = FALSE; 
        MODULEENTRY32 me32       = {0}; 
		
        do 
        { 
			CString str=pe32.szExeFile;
			if (pe32.th32ProcessID==dwProcessId) {
				retStr.Format("%s",pe32.szExeFile);
				break;
			}
        } 
        while (Process32Next(hProcessSnap, &pe32)); 
        bRet = TRUE; 
    } 
    else 
        bRet = FALSE;    // could not walk the list of processes 
	
    // Do not forget to clean up the snapshot object. 
    CloseHandle (hProcessSnap); 
	
	if (bRet==FALSE)
		TRACE("Could Not Walk the List of Processes\n");
	
	return bRet; 
	
}

