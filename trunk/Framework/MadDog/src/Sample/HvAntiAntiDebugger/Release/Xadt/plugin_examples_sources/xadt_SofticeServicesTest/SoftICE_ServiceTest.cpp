#include <windows.h>
#include "xADT_PDK.h"


BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved){
	return TRUE;
}

// note that all xADT callbacks are __cdecl
// as __cdecl is in this project set as default calling convention
// there is no need to put __cdecl in fron of a function!!
EXPORT char* tst_SoftIceServiceTest_description()
{
	return "Test using OpenServiceA/EnumServicesStatusA/EnumServicesStatusExA";
}

EXPORT char* tst_SoftIceServiceTest_name()
{
	return "SoftICE Service Test";
}

EXPORT char* tst_SoftIceServiceTest_about()
{
	return "Implemented by deroko of ARTeam";
}

EXPORT Result tst_SoftIceServiceTest(char *message){
	SC_HANDLE scm, ice;
	DWORD num_entries, rhandle = 0, dwBytesNeeded, dummy;
	SERVICE_STATUS status;
	LPENUM_SERVICE_STATUS pEnumStatus;
	LPENUM_SERVICE_STATUS_PROCESS pEnumStatusEx;
	
	scm = OpenSCManager(0,0, SC_MANAGER_ALL_ACCESS);
	// if we can't get handler for SCM, then return
	if (!scm)
		return NEGATIVE;
	ice = OpenServiceA(scm, "NTICE", SERVICE_ALL_ACCESS);
	if (ice){
		if (QueryServiceStatus(ice, &status)){
			if (status.dwCurrentState == SERVICE_RUNNING){
				CloseServiceHandle(ice);
				CloseServiceHandle(scm);
				return POSITIVE;
			}
		}
		CloseServiceHandle(ice);
	}

	// check via EnumServicesStatusA
	EnumServicesStatusA(scm, SERVICE_DRIVER, SERVICE_ACTIVE, (LPENUM_SERVICE_STATUSA)&dummy, sizeof(DWORD), &dwBytesNeeded, &num_entries, &rhandle);
	// we know that it will fail as input size is 0
	dwBytesNeeded += 0x1000;
	pEnumStatus = (LPENUM_SERVICE_STATUS)GlobalAlloc(GPTR, dwBytesNeeded);
	rhandle = 0;
	EnumServicesStatusA(scm, SERVICE_DRIVER, SERVICE_ACTIVE, pEnumStatus, dwBytesNeeded, &dwBytesNeeded, &num_entries, &rhandle);

	for (DWORD i = 0; i <num_entries; i++){
		if (!::_strcmpi(pEnumStatus[i].lpServiceName, "NTice")){
			GlobalFree(pEnumStatus);
			CloseServiceHandle(ice);
		    return POSITIVE;
		}
	}
	
	rhandle = 0;
	EnumServicesStatusExA(scm, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_ACTIVE, (LPBYTE)&dummy, sizeof(DWORD), &dwBytesNeeded, &num_entries, &rhandle, NULL);
	dwBytesNeeded += 0x1000;
	pEnumStatusEx = (LPENUM_SERVICE_STATUS_PROCESS)GlobalAlloc(GPTR, dwBytesNeeded);
	rhandle = 0;
	EnumServicesStatusExA(scm, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_ACTIVE, (LPBYTE)pEnumStatusEx, dwBytesNeeded, &dwBytesNeeded, &num_entries, &rhandle, NULL);
	for (DWORD i = 0; i <num_entries; i++){
		if (!::_strcmpi(pEnumStatusEx[i].lpServiceName, "NTice")){
			GlobalFree(pEnumStatusEx);
			CloseServiceHandle(scm);
			return POSITIVE;
		}
	}

	return NEGATIVE;
}

	