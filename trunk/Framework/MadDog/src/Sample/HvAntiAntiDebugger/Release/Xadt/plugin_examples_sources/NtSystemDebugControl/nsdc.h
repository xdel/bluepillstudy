// nsdc.h Contains declarations requested by NtSystemDebugControl()

typedef LONG NTSTATUS;
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

// DEBUG_CONTROL_CODE

typedef enum _DEBUG_CONTROL_CODE {
	DebugSysGetTraceInformation=1,
	DebugSysSetInternalBreakpoint,
	DebugSysSetSpecialCall,
	DebugSysClerSpecialCalls,
	DebugSysQuerySpecialCalls, 
	DebugSysBreakpointWithStatus,
	DebugSysGetVersion,
	DebugSysReadVirtual = 8,
    DebugSysWriteVirtual = 9,
    DebugSysReadPhysical = 10,
    DebugSysWritePhysical = 11,	
	DebugSysReadControlSpace=12,
	DebugSysWriteControlSpace,
	DebugSysReadIoSpace,
	DebugSysSysWriteIoSpace,
	DebugSysReadMsr,
	DebugSysWriteMsr,
	DebugSysReadBusData,
	DebugSysWriteBusData,
	DebugSysCheckLowMemory, 
} DEBUG_CONTROL_CODE;

//SYSDBG_VIRTUAL

typedef struct _SYSDBG_VIRTUAL {
    PVOID Address;
    PVOID Buffer;
    ULONG Request;
} SYSDBG_VIRTUAL, *PSYSDBG_VIRTUAL;

// NtSystemDebugControl Declaration

extern "C"
__declspec(dllimport) 
ULONG
__stdcall 
NtSystemDebugControl(
	DEBUG_CONTROL_CODE ControlCode,
    PVOID InputBuffer,
	ULONG InputBufferLength,
	PVOID OutputBuffer,
    ULONG OutputBufferLength,
	PULONG ReturnLength
);

typedef ULONG (__stdcall *fcnNtSystemDebugControl) (DEBUG_CONTROL_CODE, PVOID, ULONG, PVOID, ULONG, PULONG);
