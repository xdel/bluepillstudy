#ifndef _NTDEFINITIONS_H
#define _NTDEFINITIONS_H
	
typedef PVOID *PPVOID;
typedef LONG NTSTATUS;
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L) 

typedef struct _LSA_UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _RTL_DRIVE_LETTER_CURDIR {
	
	USHORT Flags; 
	USHORT Length; 
	ULONG TimeStamp; 
	UNICODE_STRING DosPath;
	
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	
	ULONG MaximumLength; 
	ULONG Length; 
	ULONG Flags; 
	ULONG DebugFlags; 
	PVOID ConsoleHandle; 
	ULONG ConsoleFlags; 
	HANDLE StdInputHandle; 
	HANDLE StdOutputHandle; 
	HANDLE StdErrorHandle; 
	UNICODE_STRING CurrentDirectoryPath; 
	HANDLE CurrentDirectoryHandle; 
	UNICODE_STRING DllPath; 
	UNICODE_STRING ImagePathName; 
	UNICODE_STRING CommandLine; 
	PVOID Environment; 
	ULONG StartingPositionLeft; 
	ULONG StartingPositionTop; 
	ULONG Width; 
	ULONG Height; 
	ULONG CharWidth; 
	ULONG CharHeight; 
	ULONG ConsoleTextAttributes; 
	ULONG WindowFlags; 
	ULONG ShowWindowFlags; 
	UNICODE_STRING WindowTitle; 
	UNICODE_STRING DesktopName; 
	UNICODE_STRING ShellInfo; 
	UNICODE_STRING RuntimeData; 
	RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
	
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef enum _PROCESSINFOCLASS {
	ProcessBasicInformation,
		ProcessQuotaLimits,
		ProcessIoCounters,
		ProcessVmCounters,
		ProcessTimes,
		ProcessBasePriority,
		ProcessRaisePriority,
		ProcessDebugPort,
		ProcessExceptionPort,
		ProcessAccessToken,
		ProcessLdtInformation,
		ProcessLdtSize,
		ProcessDefaultHardErrorMode,
		ProcessIoPortHandlers, // Note: this is kernel mode only
		ProcessPooledUsageAndLimits,
		ProcessWorkingSetWatch,
		ProcessUserModeIOPL,
		ProcessEnableAlignmentFaultFixup,
		ProcessPriorityClass,
		ProcessWx86Information,
		ProcessHandleCount,
		ProcessAffinityMask,
		ProcessPriorityBoost,
		ProcessDeviceMap,
		ProcessSessionInformation,
		ProcessForegroundInformation,
		ProcessWow64Information,
		ProcessImageFileName,
		ProcessLUIDDeviceMapsEnabled,
		ProcessBreakOnTermination,
		ProcessDebugObjectHandle,
		ProcessDebugFlags,
		ProcessHandleTracing,
		MaxProcessInfoClass
} PROCESSINFOCLASS;

typedef struct _tagPROCESS_BASIC_INFORMATION
{
    DWORD ExitStatus;
    DWORD PebBaseAddress;
    DWORD AffinityMask;
    DWORD BasePriority;
    ULONG UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
}   PROCESS_BASIC_INFORMATION;

#endif