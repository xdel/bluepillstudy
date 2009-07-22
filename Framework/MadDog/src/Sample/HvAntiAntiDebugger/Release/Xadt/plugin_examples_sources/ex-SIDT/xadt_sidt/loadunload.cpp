#include "defs.h"

NTSTATUS (__stdcall *NtLoadDriver)(IN PUNICODE_STRING) = NULL;
NTSTATUS (__stdcall *RtlAdjustPrivilege)(DWORD, DWORD, DWORD, DWORD *) = NULL;
void     (__stdcall *RtlInitUnicodeString)(IN OUT PUNICODE_STRING, PCWSTR);

NTSTATUS LoadDriver(){
	UNICODE_STRING doer_us;
	NTSTATUS status;
	WCHAR *servicename = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\xadt_int";
	DWORD dummy;
	HKEY hKey;
	char fullpathname[512];
	char currentdir[512];
	*(void**)&NtLoadDriver = (void *)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtLoadDriver");
	*(void**)&RtlAdjustPrivilege = (void *)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlAdjustPrivilege");
	*(void**)&RtlInitUnicodeString = (void  *)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlInitUnicodeString");

	//load driver...by directly adding data to registry... no need to use SCM manager when like this
	//it is much easier...
	RegCreateKeyA(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\xadt_int", &hKey);
	dummy = SERVICE_KERNEL_DRIVER;
	RegSetValueExA(hKey,"Type", 0, REG_DWORD, (const BYTE *)&dummy, 4);
	dummy = SERVICE_ERROR_NORMAL;
	RegSetValueExA(hKey,"Error",0, REG_DWORD, (const BYTE *)&dummy, 4);
	dummy = SERVICE_DEMAND_START;
	RegSetValueExA(hKey,"Start",0, REG_DWORD, (const BYTE *)&dummy, 4);
        
        xADT_PluginFolder(currentdir);
	_snprintf(fullpathname, 512, "\\??\\%s\\xadt_int.sys", currentdir);
	
	RegSetValueExA(hKey,"ImagePath", 0, REG_SZ, (const BYTE *)fullpathname, strlen(fullpathname)+1);
	RtlAdjustPrivilege(10, 1, 0, &dummy);
	RtlInitUnicodeString(&doer_us, servicename);
	status = NtLoadDriver(&doer_us);

	RegDeleteKeyA(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\xadt_int\\Enum");
	RegDeleteKeyA(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\xadt_int\\Security");
	RegDeleteKeyA(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\xadt_int");
	RegCloseKey(hKey);

	return (DWORD)status;
}