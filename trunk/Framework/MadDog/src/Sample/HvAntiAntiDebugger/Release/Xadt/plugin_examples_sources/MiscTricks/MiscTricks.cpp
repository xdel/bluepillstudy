#include <afxwin.h>
#include <stdio.h>
#include <process.h>
#include "xADT_PDK.h"
#include "myDOS.h"

#define TIME_INTERVAL	100 //msec

#define VS60 defined(_MSC_VER) && _MSC_VER<1300 //used to allow specific code for VS 6.0, otherwise is VS2008

#if VS60 
#define printf_s printf
using namespace MyDOS;
#endif

//prototypes of threads used for some tests
void _cdecl CallsNtSetInformationThread(void*);
void _cdecl CallCtrlCException (void* val);
BOOL isClosedThread=FALSE;

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

//////////////////////////////////////////////////////////////////////////
//Int 2A Trick calling KiGetTickCount

EXPORT Result tst_Int2ATrick(char *message)
{
	Result bRet=NEGATIVE;

	DWORD	dwTick=0, iStartTime=0;

	/* >= NT 4.0 */
	if( GetVersion() >= 0x80000000 ) {
		#if VS60
		sprintf(message, "Windows NT 4.0 or higher is required to use this plugin!");
		#else
		sprintf_s(message, 60, "Windows NT 4.0 or higher is required to use this plugin!");
		#endif
		return bRet;
	}

	iStartTime=KiGetTickCount();
	
	Sleep(TIME_INTERVAL);

	dwTick=KiGetTickCount();
	dwTick -= iStartTime;

	#if VS60
	sprintf(message,"execution time %d msec", dwTick);
	#else
	sprintf_s(message,80,"execution time %d msec", dwTick);
	#endif
	
	if(dwTick>TIME_INTERVAL*1.1) //if I'm debugging it takes longer than 10% ..
		return POSITIVE;
	
	return NEGATIVE;
}

EXPORT char* tst_Int2ATrick_description()
{
	return "Test using int 2A to call KiGetTickCount";
}

EXPORT char* tst_Int2ATrick_name()
{
	return "Int2ATrick way to KiGetTickCount";
}

EXPORT char* tst_Int2ATrick_about()
{
	return "Implemented by Shub-Nigurrath, from an idea ReWolf";
}

//////////////////////////////////////////////////////////////////////////
//usage of PEB!NtGlobalFlags

EXPORT Result tst_Rootkitties(char *message) {

	Result res=NEGATIVE;
	UINT positivetests=0;
	BOOL skiptest=FALSE;

	MyDOS::AllocConsole();
	MyDOS::DOSclrscr();
	
	//////////////////////////////////////////////////////////////////////////
	// NtGlobalFlags field
	//
	// When a process is created, the system sets some flags that will define how various APIs will behave for this program. 
	// Those flags can be read in the PEB, in the DWORD located at offset 0x68 (see the reference).
	// By default, different flags are set depending if the process is created under a debugger or not. 
	// If the process is debugged, some flags controlling the heap manipulation routines in ntdll will be set: 
	// FLG_HEAP_ENABLE_TAIL_CHECK, FLG_HEAP_ENABLE_FREE_CHECK and FLG_HEAP_VALIDATE_PARAMETERS.
	// This anti-debug can be bypassed by resetting the NtGlobalFlags field
	res=NEGATIVE;
	__asm {
		mov		eax, fs:[30h]
		mov		eax, [eax+68h]
		and		eax, 0x70
		test	eax, eax
		jne		_debuggerfound1 
		mov		res, NEGATIVE
		jmp		_out1
	_debuggerfound1:
		mov		res, POSITIVE
	_out1:
		nop
	}

	//return results: a counter with resulting positive tests and a prints to the local Dll console window
	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 1: Check of some NtGlobalFlags elements \n"
			"\t -> %s\n",(res==POSITIVE)?"POSITIVE":"NEGATIVE" );

	//////////////////////////////////////////////////////////////////////////
	// As explained previously, NtGlobalFlags informs how the heap routines will behave 
	// (among other things). Though it is easy to modify the PEB field, if the heap does 
	// not behave the same way as it should when the process is not debugged, this could be 
	// problematic. It is a powerful anti-debug, as process heaps are numerous, and their chunks 
	// can be individually affected by the FLG_HEAP_* flags (such as chunk tails). Heap headers 
	// would be affected as well. For instance, checking the field ForceFlags in a heap header 
	// (offset 0x10) can be used to detect the presence of a debugger.
	//
	// There are two easy ways to circumvent it:
	//	- Create a non-debugged process, and attach the debugger once the process has been created 
	//    (an easy solution is to create the process suspended, run until the entry-point is reached, 
	//    patch it to an infinite loop, resume the process, attach the debugger, and restore the original 
	//    entry-point).
	//
	// - Force the NtGlobalFlags for the process that we want to debug, via the registry key 
	//   "HKLM\Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options": Create a 
	//   subkey (not value) named as your process name, and under this subkey, a String value 
	//   "GlobalFlags" set to nothing.
	res=NEGATIVE;
	__asm {
		mov		eax, fs:[30h]
		mov		eax, [eax+18h]	//process heap
		mov		eax, [eax+10h]	//heap flags
		test eax, eax
		jne _debuggerfound2 
		mov		res, NEGATIVE
		jmp		_out2
	_debuggerfound2:
		mov		res, POSITIVE
	_out2:
		nop
	}

	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 2: Checking the field ForceFlags in a heap header (offset 0x10).\n"
			"This can be used to detect the presence of a debugger \n"
			"\t -> %s\n",(res==POSITIVE)?"POSITIVE":"NEGATIVE" );


	//////////////////////////////////////////////////////////////////////////
	// NtQueryInformationProcess
	//
	// 	ntdll!NtQueryInformationProcess is a wrapper around the ZwQueryInformationProcess syscall. 
	// Its prototype is the following:
	// 	
	//  NTSYSAPI NTSTATUS NTAPI NtQueryInformationProcess(
	// 		IN HANDLE ProcessHandle,
	// 		IN PROCESS_INFORMATION_CLASS ProcessInformationClass,
	// 		OUT PVOID ProcessInformation,
	// 		IN ULONG ProcessInformationLength,
	// 		OUT PULONG ReturnLength
	// 	);
	// 
	// When called with ProcessInformationClass set to 7 (ProcessDebugPort constant), the system 
	// will set ProcessInformation to -1 if the process is debugged.
	// It is a powerful anti-debug, and there is no easy way to circumvent it. However, if the 
	// program is traced, ProcessInformation can be modified when the syscall returns.
	// 
	// Another solution is to use a system driver that would hook the ZwNtQueryInformationProcess 
	// syscall. Circumventing NtQueryInformationProcess will bypass many anti-debug techniques 
	// (such as CheckRemoteDebuggerPresent or UnhandledExceptionFilter).

	int *isdebugged=new int;
	*isdebugged=0;
	skiptest=FALSE;
	HMODULE hNTModule = GetModuleHandle((LPCSTR)"ntdll.dll");
	if (hNTModule == NULL) {
		printf("[KO] failed to open ntdll.dll\n");
		skiptest=TRUE;
	}    
	HANDLE _NtQueryInformationProcess = GetProcAddress(hNTModule,(LPCSTR)TEXT("NtQueryInformationProcess"));    
	if (_NtQueryInformationProcess == NULL) 
	{      
		printf("[KO] failed to get NtQueryInformationProcess\n");
		skiptest=TRUE;;
	}  
	    
	if(!skiptest) {
		res=NEGATIVE;
		__asm {
			push	0
			push	4
			push	isdebugged
			push	7	//ProcessDebugPort
			push	-1
			call	_NtQueryInformationProcess
			test	eax, eax
			jne		_exiterror
			mov		eax, dword ptr [isdebugged]
			cmp		[eax], 0
			jne		_debuggerfound3
			mov		res, NEGATIVE
			jmp		_out3
		_debuggerfound3:
			mov		res, POSITIVE
			jmp		_out3
		_exiterror:
			mov		res, NEGATIVE
		_out3:
			nop
		}
	}

	delete isdebugged; //free memory

	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 3: Check calling NtQueryInformationProcess.\n"
			"When called with ProcessInformationClass set to 7 (ProcessDebugPort\n" 
			"constant), the system will set ProcessInformation to -1 if the process\n"
			"is debugged.\n"
			"\t -> %s\n",(res==POSITIVE)?"POSITIVE":"NEGATIVE" );

	//////////////////////////////////////////////////////////////////////////
	// NtSetInformationThread
	//
	// 		ntdll!NtSetInformationThread is a wrapper around the ZwSetInformationThread syscall. 
	// Its prototype is the following:
	//  NTSYSAPI NTSTATUS NTAPI NtSetInformationThread(
	// 	 IN HANDLE ThreadHandle,
	// 	 IN THREAD_INFORMATION_CLASS ThreadInformationClass,
	// 	 IN PVOID ThreadInformation,
	// 	 IN ULONG ThreadInformationLength
	// 	 );
	// 
	// When called with ThreadInformationClass set to 0x11 (ThreadHideFromDebugger constant), 
	// the thread will be detached from the debugger.
	// 
	// Similarly to ZwQueryInformationProcess, circumventing this anti-debug requires 
	// either modifying ZwSetInformationThread parameters before it's called, or hooking the 
	// syscall directly with the use of a kernel driver.
	
	//monitorval=0 -> thread still not started
	//monitorval=1 -> thread just started 
	//monitorval=2 -> monitor val terminated but not by calling NtSetInformationThread
	UINT monitorvar=0;

	res=NEGATIVE;	
	_beginthread(CallsNtSetInformationThread, 0, &monitorvar);
	
	//wait thread turn to run
	while(monitorvar<1)
		;
	Sleep(100); //wait a little more

	if(monitorvar!=2)
		res=POSITIVE;
	else
		res=NEGATIVE;

	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 4: Check calling NtSetInformationThread.\n"
			"When called with ThreadInformationClass set to 0x11 (ThreadHideFromDebugger\n"
			"constant), the thread will be detached from the debugger.\n"
			"\t -> %s\n",(res==POSITIVE)?"POSITIVE":"NEGATIVE" );


	//////////////////////////////////////////////////////////////////////////
	// kernel32!CloseHandle and NtClose
	// 
	// APIs making user of the ZwClose syscall (such as CloseHandle, indirectly) can be used to 
	// detect a debugger. 
	// When a process is debugged, calling ZwClose with an invalid handle will generate a 
	// STATUS_INVALID_HANDLE (0xC0000008) exception.
	// 
	// As with all anti-debugs that rely on information made directly available from the kernel 
	// (therefore involving a syscall), the only proper way to bypass the "CloseHandle" anti-debug 
	// is to either modify the syscall data from ring3, before it is called, or set up a kernel 
	// hook.
	// 
	// This anti-debug, though extremely powerful, does not seem to be widely used by malicious 
	// programs.
 
	res=NEGATIVE;
	__try {
		CloseHandle((HANDLE)0x1234);
	}
	__except( GetExceptionCode() == STATUS_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
			res=POSITIVE;
	}
	
	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 5: Check calling CloseHandle.\n"
			  "APIs making user of the ZwClose syscall (such as CloseHandle, indirectly)\n"
			  "can be used to detect a debugger. When a process is debugged, calling ZwClose\n"
			  "with an invalid handle will generate a STATUS_INVALID_HANDLE (0xC0000008)\n"
			  "exception\n"
			  "\t -> %s\n",(res==POSITIVE)?"POSITIVE":"NEGATIVE" );


	//////////////////////////////////////////////////////////////////////////
	// kernel32!OutputDebugStringA
	// 
	// This anti-debug is quite original, I have encountered it only once, in files packed with 
	// ReCrypt v0.80. The trick consists of calling OutputDebugStringA, with a valid ASCII string. 
	// If the program is run under control of a debugger, the return value will be the address of 
	// the string passed as a parameter. In normal conditions, the return value should be 1.

	skiptest=FALSE;
	char *szHello=new char[256];
	#if VS60
	strncpy(szHello,"Shubby",256);
	#else
	strcpy_s(szHello,256,"Shubby");
	#endif
	hNTModule = GetModuleHandle((LPCSTR)"kernel32.dll");
	if (hNTModule == NULL) {
		printf("[KO] failed to open kernel32.dll\n");
		skiptest=TRUE;
	}    
	HANDLE _OutputDebugStringA = GetProcAddress(hNTModule,(LPCSTR)TEXT("OutputDebugStringA"));    
	if (_OutputDebugStringA == NULL) {      
		printf("[KO] failed to get OutputDebugStringA\n");
		skiptest=TRUE;
	}  

	if(!skiptest) {
		res=NEGATIVE;
		__asm {
	 		xor		eax, eax
	 		push	szHello
	 		call	_OutputDebugStringA
	 		cmp		eax, 1
	 		jne		_debuggerfound4
			mov		res, NEGATIVE
			jmp		_out5
		_debuggerfound4:
			mov		res, POSITIVE;
		_out5:
			nop
		}
	}

	delete szHello;
	
	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 6: Check calling OutputDebugStringA.\n"
			  "calling OutputDebugStringA, with a valid ASCII string. \n"
			  "If the program is run under control of a debugger, the \n"
			  "return value will be the address of the string passed as a parameter.\n"
			  "In normal conditions, the return value should be 1.\n"
			  "\t -> %s\n",(res==POSITIVE)?"POSITIVE":"NEGATIVE" );

	
	//////////////////////////////////////////////////////////////////////////
	// Kernel-mode timers
	// 
	// kernel32!QueryPerformanceCounter is an efficient anti-debug. This API calls 
	// ntdll!NtQueryPerformanceCounter which wraps the ZwQueryPerformanceCounter syscall.
	// 
	// Again, there is no easy way to circumvent this anti-tracing trick.

	//fre HPC ticks per second
	//t0 start counter
	//t1 end counter
	LARGE_INTEGER t0, t1, freq;
	double micros=0.0;

	res=NEGATIVE;
	if(QueryPerformanceFrequency(&freq)) {
		if(QueryPerformanceCounter(&t0)) {
			Sleep(TIME_INTERVAL);
		}
		if(QueryPerformanceCounter(&t1)) {
			micros = (double)(t1.QuadPart - t0.QuadPart) * 1e6 / (double)freq.QuadPart;
		}
	}
	else
		printf_s("[KO] QueryPerformanceFrequency failed!\n");

	if(micros>(double)TIME_INTERVAL*1.1*1e3) //conversion msec to usec needed and a little of tollerance 10%
		res=POSITIVE;
	else
		res=NEGATIVE;

	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 7: Check calling QueryPerformanceCounter.\n"
			  "QueryPerformanceCounter is an efficient anti-debug.\n"
			  "This API calls ntdll!NtQueryPerformanceCounter which wraps\n"
			  "the ZwQueryPerformanceCounter syscall.\n"
			  "Elapsed Time: %.2f usec (>%.2f usec)\n"
			  "\t -> %s\n", micros, TIME_INTERVAL*1.1*1e3,
			  (res==POSITIVE)?"POSITIVE":"NEGATIVE" );

	//////////////////////////////////////////////////////////////////////////
	// Ctrl-C
	// 
	// When a console program is debugged, a Ctrl-C signal will throw a EXCEPTION_CTL_C exception, 
	// whereas the signal handler would be called directly is the program is not debugged.

	skiptest=FALSE;
	res=NEGATIVE;

	_beginthread(CallCtrlCException, 0, &res);
	while(!isClosedThread)
		;

	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 8: Check using Ctrl-C signal.\n"
			   "When a console program is debugged, a Ctrl-C signal will throw\n"
			   "a EXCEPTION_CTL_C exception, whereas the signal handler would\n"
			   "be called directly is the program is not debugged\n"
			   "\t -> %s\n", (res==POSITIVE)?"POSITIVE":"NEGATIVE" );

	
	//////////////////////////////////////////////////////////////////////////
	// DebugBreak
	//
	// Try calling DebugBreak, if the program is debugged then the program stops, otherwise continue..
	
	res=POSITIVE;
	__try 
	{
		DebugBreak();
	}
	__except(GetExceptionCode() == EXCEPTION_BREAKPOINT ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
		// No debugger is attached, so return FALSE 
		// and continue.
		res=NEGATIVE;
	}
	
	if(res==POSITIVE)
		positivetests++;
	printf_s( "\nMethod 9: Check using DebugBreak.\n"
		"Call DebuBreak API, if a debugger is attached then the even is handled and the\n"
		"result is positive, otherwise the exception is directly handled by the program\n"
		"and the test result is negative.\n"
		"\t -> %s\n", (res==POSITIVE)?"POSITIVE":"NEGATIVE" );

			
	//////////////////////////////////////////////////////////////////////////
	//summing up results!
	if(positivetests) {
		#if VS60
		sprintf(message, "You have been caught %d times (see DOS window for details)! Hide better", positivetests);
		#else
		sprintf_s(message, MAX_PATH, "You have been caught %d times (see DOS window for details)! Hide better", positivetests);
		#endif
		printf_s("\n%s\n", message);
		::MessageBox(NULL, message, "xADT",MB_OK|MB_SETFOREGROUND);
	
	}
	else {
		#if VS60
		sprintf(message, "Well done you successfully performed all the test without being catched!");
		#else
		sprintf_s(message, MAX_PATH, "Well done you successfully performed all the test without being catched!");
		#endif
		printf_s("\n%s\n", message);
		::MessageBox(NULL, message, "xADT",MB_OK|MB_SETFOREGROUND);
		
	}

	MyDOS::FreeConsole();

	return ((positivetests>0)?POSITIVE:NEGATIVE);
}

void _cdecl CallCtrlCException (void* val) {
	Result res=UNKNOWN;
	static Result *pRes=NULL;
	
	BOOL skiptest=FALSE;

	isClosedThread=FALSE;

	HMODULE hNTModule = GetModuleHandle((LPCSTR)"ntdll.dll");
	if (hNTModule == NULL) {
		printf("[KO] failed to open ntdll.dll\n");
		skiptest=TRUE;
	}    
	HANDLE _RtlAddVectoredExceptionHandler= GetProcAddress(hNTModule,(LPCSTR)TEXT("RtlAddVectoredExceptionHandler"));
	if (_RtlAddVectoredExceptionHandler == NULL) {      
		printf("[KO] failed to get RtlAddVectoredExceptionHandler\n");
		skiptest=TRUE;
	}
	
	pRes=(Result*)val;

	skiptest=TRUE;
	
	if(!skiptest) {
		__asm {
			push	exhandler
			push	1
			call	_RtlAddVectoredExceptionHandler
			push	1
			push	sighandler
			call	SetConsoleCtrlHandler
			push	0
			push	CTRL_C_EVENT
			call	GenerateConsoleCtrlEvent
			push	10000
			call	Sleep
			jmp		_out6 //shouldn't ever been called..
		exhandler:
			//check if EXCEPTION_CTL_C, if it is,
			//debugger detected, should exit process
			mov		res, POSITIVE;
			jmp		_out6
		sighandler:
			//continue 
			mov		res, NEGATIVE;
		_out6:
			nop
		}
	}

	isClosedThread=TRUE;
	
	//*((Result*)(val))=res;
	*pRes=res;
	_endthread();
}

void _cdecl CallsNtSetInformationThread(void* val) {
	UINT *running=(UINT*)val;
	
	BOOL skiptest=FALSE;

	*running=0;

	HMODULE hNTModule = GetModuleHandle((LPCSTR)"ntdll.dll");
	if (hNTModule == NULL) {
		printf("[KO] failed to open ntdll.dll\n");
		skiptest=TRUE;
	}    
	HANDLE _NtSetInformationThread = GetProcAddress(hNTModule,(LPCSTR)TEXT("NtSetInformationThread"));    
	if (_NtSetInformationThread == NULL) {      
		printf("[KO] failed to get NtSetInformationThread\n");
		skiptest=TRUE;
	} 
	
	//If the thread is terminated the val value never becomes 2 (see below) then I'm debugged!
	*running=1;

	if(!skiptest) {
		__asm {
			push	0
			push	0
			push	11h //ThreadHideFromDebugger
			push	-2
			call	_NtSetInformationThread
			//;thread detached if debugged 
			nop
		}
	}

	*running=2; //I am here only if not debugging the process!

	_endthread(); //just ot be sure!
}

EXPORT char* tst_Rootkitties_description() {
	return "Several checks found in some Rootkits and compressors. Several tests, see opened DOS window for details";
}

EXPORT char* tst_Rootkitties_name() {
	return "Some Rootkits typical tests";
}

EXPORT char* tst_Rootkitties_about() {
	return "Implemented by Shub-Nigurrath ideas from www.securityfocus.com/infocus/1893";	
}


