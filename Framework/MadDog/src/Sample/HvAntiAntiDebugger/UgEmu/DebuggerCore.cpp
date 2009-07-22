/*
This file is part of UgDbg.
 
UgDbg is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
UgDbg is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with UgDbg.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "helper.h"
#include <tlhelp32.h>
#include <strsafe.h>
#include <psapi.h>

char Command[500][50] = {"Help","bpm","bpx","bc","bd","be","bl","debug","dump","dumppe","cls","d","u","r","what",".","loadconfig","saveconfig",
						 "set","quit","t","stealth","zap","loadpdb","unloadpdb","poke","s","p","code","break","register","unregister","map32",
						 "inject","rp"};
char Syntax[_countof(Command)][30] = {"","<offset> <r|w|x>","<offset>","<nr>","<nr>","<nr>","","<file>","<low> <high> <filename>","<filename>","","<offset>","<offset>","<register> <value>",
									  "<offset>","","[filename]","[filename]","[<option> <on|off>]","","","","","","","<offset> <T\"string\"|xx>","<ofs> <len> <T\"string\"|xx>","","[on|off]",
									  "","","","","[offset] <file>",""};
char HelpText[_countof(Command)][50] = {"displays the help dialog","hardware breakpoint (drX)","software breakpoint","clears a breakpoint","disables a breakpoint","enables a breakpoint",
										"lists all breakpoints","loads a file to debug","dumps defined memory area to disk","dumps the currently loaded image to disk",
										"clears the logwindow","display memory","disassemble memory","set register","tries to identify the value","disassembles from current (e)rip",
										"loads the specified configuration", "saves the current configuration","set option true or false or list","terminate debugger", "single step",
										"hides the debugger from the debuggee","nops out int1 / int3","tries to load the pdb","unloads the pdb","write <xx> to memory","search memory","step over",
										"display hex instructions","debug break","register to the context","unregister from the context","display sections","injects file into debuggee",
										"refresh plugins"};
char Registers[42][4] = { "eax","ebx","ecx","edx","esi","edi","ebp","esp","eip",
						  "rax","rbx","rcx","rdx","rsi","rdi","rbp","rsp","rip","r8","r9","r10","r11","r12","r13","r14","r15",
						  "al","ah","ax","bl","bh","bx","cl","ch","cx","dl","dh","dx","si","di","bp","sp"};

char Options[4][20] = {"dumpfix","i3here","i1here","highlight"};
bool OptionSettings[_countof(Options)] = { false , false, false, false};
bool BreakFound = false;

unsigned char BpxBuffer[0x5000]="";
unsigned char DllTableBuffer[0x10000]="";
unsigned char PluginTableBuffer[0x1000]="";
char DbgMsg[MAX_PATH];
unsigned char OrigByteBuffer[1]="";
#ifdef b32BitBuild
ULONG HWBreakpointAddr = 0;
#else
ULONGLONG HWBreakpointAddr = 0;
#endif

// *****************************************************************
//  Retrieve Option (true/false) from OptionSettings (i3here etc)
// 
// *****************************************************************
bool GetOption (char * CommandName)
{
	for (int i = 0; i < _countof (Options); i++) {
		if (_stricmp (Options[i],CommandName) == 0) {
			return OptionSettings[i];
		}
	}
	return false;
}

// *****************************************************************
//  READ MEMORY OFFSET FROM TARGET
// 
// *****************************************************************
#ifdef b32BitBuild
bool ReadMem (ULONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI)
#else
bool ReadMem (ULONGLONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI)
#endif
{
	SIZE_T BRead;
	if (PI->hProcess != NULL) {
		int bRes = ReadProcessMemory (PI->hProcess,(LPCVOID)Offset,(LPVOID)lpBuf,Size,&BRead);
		if (bRes != NULL) {
			return true;
		}
	}
	return false;
}

// ************************************************************************
//	READ THREAD CONTEXT
//
//  Return Codes:  false on error, else true
//
// ************************************************************************
bool ReadContext (CONTEXT * CTX,DebugStruc * DebugDataExchange)
{
	ZeroMemory (CTX,sizeof (CONTEXT));
	CTX->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	HANDLE CurrentThread = OpenThread (THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,false,DebugDataExchange->DbgEvent.dwThreadId);
	if (CurrentThread == NULL) return false;

	if (SuspendThread (CurrentThread) == -1) return false;
	if (GetThreadContext (CurrentThread,CTX) != 0)	{
		if (ResumeThread (CurrentThread) == -1) return false;
		CloseHandle (CurrentThread);
		return true;
	}
	if (ResumeThread (CurrentThread) == -1) return false;
	CloseHandle (CurrentThread);
	return false;
}

// ************************************************************************
//	WRITE THREAD CONTEXT
//
//  Return Codes:  false on error, else true
//
// ************************************************************************
bool WriteContext (CONTEXT * CTX,DebugStruc * DebugDataExchange)
{
	HANDLE CurrentThread = OpenThread (THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,false,DebugDataExchange->DbgEvent.dwThreadId);
	if (CurrentThread == NULL) return false;

	if (SuspendThread (CurrentThread) == -1) return false;
	if (SetThreadContext (CurrentThread,CTX) != 0)	{
		if (ResumeThread (CurrentThread) == -1) return false;
		CloseHandle (CurrentThread);
		return true;
	}
	if (ResumeThread (CurrentThread) == -1) return false;
	CloseHandle (CurrentThread);
	return false;
}


// **************************************************************************************
//
//	WHEN A BREAKPOINT HITS IT NEEDS TO BE DISABLED AND REENABLED AUTOMATICLY
//
// **************************************************************************************
void ReEnableBreakpoints (const LPDEBUG_EVENT DebugEv,DebugStruc * DebugDataExchange)
{
	// (RE)ACTIVATE BPX
	PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
	while (BpxTable->Offset != 0) 
	{
		if (DebugEv->u.Exception.ExceptionRecord.ExceptionAddress != (PVOID)BpxTable->Offset)
		{
			if ((BpxTable->Enabled == TRUE) && (BpxTable->Type == 0)) {
				SetInt3BreakPoint (BpxTable->Offset,&DebugDataExchange->ProcessInfo);
			}
			
			if ((BpxTable->Enabled == TRUE) && (BpxTable->Type == 1)) {
				CONTEXT CTX;
				if (ReadContext (&CTX,DebugDataExchange) == true) {
					if ((CTX.Dr0 == BpxTable->Offset) || ((CTX.Dr0 != BpxTable->Offset) && (CTX.Dr1 != BpxTable->Offset) && (CTX.Dr2 != BpxTable->Offset) && (CTX.Dr3 != BpxTable->Offset))) {
						CTX.Dr0 = BpxTable->Offset;
						CTX.Dr7 |= 0x101;
						if (BpxTable->Type2 == 0) CTX.Dr7 &= 0xFFFCFFFF;
						if (BpxTable->Type2 == 2) CTX.Dr7 |= 0x00010000;
						if (BpxTable->Type2 == 1) CTX.Dr7 |= 0x00030000;
					} else {
						if (CTX.Dr1 == BpxTable->Offset) {
							CTX.Dr1 = BpxTable->Offset;
							CTX.Dr7 |= 0x104;
							if (BpxTable->Type2 == 0) CTX.Dr7 &= 0xFFCFFFFF;
							if (BpxTable->Type2 == 2) CTX.Dr7 |= 0x00100000;
							if (BpxTable->Type2 == 1) CTX.Dr7 |= 0x00300000;
						} else {
							if (CTX.Dr2 == BpxTable->Offset) {
								CTX.Dr2 = BpxTable->Offset;
								CTX.Dr7 |= 0x110;
								if (BpxTable->Type2 == 0) CTX.Dr7 &= 0xFCFFFFFF;
								if (BpxTable->Type2 == 2) CTX.Dr7 |= 0x01000000;
								if (BpxTable->Type2 == 1) CTX.Dr7 |= 0x03000000;
							} else {
								if (CTX.Dr3 == BpxTable->Offset) {
									CTX.Dr3 = BpxTable->Offset;
									CTX.Dr7 |= 0x140;
									if (BpxTable->Type2 == 0) CTX.Dr7 &= 0xCFFFFFFF;
									if (BpxTable->Type2 == 2) CTX.Dr7 |= 0x10000000;
									if (BpxTable->Type2 == 1) CTX.Dr7 |= 0x30000000;
								} else {
								}
							}
						}
					}
					CTX.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
					if (WriteContext (&CTX,DebugDataExchange) == true) {
					} else {
					}
				}
			}
		}
		BpxTable++;
	}
	return;
}



// *****************************************************************
//  WRITE MEMORY OFFSET FROM TARGET
// 
// *****************************************************************
#ifdef b32BitBuild
bool WriteMem (ULONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI)
#else
bool WriteMem (ULONGLONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI)
#endif
{
	SIZE_T BWrite;
	ULONG OldProtect;
	VirtualProtectEx (PI->hProcess,(LPVOID)Offset,Size,PAGE_READWRITE,&OldProtect);
	if (WriteProcessMemory (PI->hProcess,(LPVOID)Offset,(LPCVOID)lpBuf,Size,&BWrite) != NULL) {
		VirtualProtectEx (PI->hProcess,(LPVOID)Offset,Size,OldProtect,&OldProtect);
		return true;
	}
	VirtualProtectEx (PI->hProcess,(LPVOID)Offset,Size,OldProtect,&OldProtect);
	FlushInstructionCache (PI->hProcess,(LPCVOID)Offset,32);
	return false;
}

// *****************************************************************
//  Get Path of UgDbg.exe
// 
// *****************************************************************
bool GetHomePath (char * OutString, size_t len)
{
	char FullPath[MAX_PATH] = "";
	if (GetModuleFileName (GetModuleHandle(NULL),FullPath,_countof(FullPath)) == NULL) return false;
	char drive[_MAX_DRIVE] = "";
	char dir[_MAX_DIR] = "";
	char fname[_MAX_FNAME] = "";
	char ext[_MAX_EXT] = "";
	_splitpath_s (FullPath,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

	strcpy_s (OutString,len,drive);
	strcat_s (OutString,len,dir);
	return true;
}

// *****************************************************************
//  Get Naked FileName
// 
// *****************************************************************
bool GetFileName (char * InString, size_t len)
{
	char drive[_MAX_DRIVE] = "";
	char dir[_MAX_DIR] = "";
	char fname[_MAX_FNAME] = "";
	char ext[_MAX_EXT] = "";
	_splitpath_s (InString,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	memset (InString,0,len);
	memcpy (InString,fname,strlen (fname));
	return true;
}

// *****************************************************************
//  Build Plugin API tree
// 
// *****************************************************************
bool BuildPluginApiTree (DebugStruc * DebugDataExchange)
{
	PDllTable MyDllTable = (PDllTable)&PluginTableBuffer;
	char PluginFile [MAX_PATH+1] = "";

	GetHomePath (PluginFile,_countof(PluginFile));
	strcat_s (PluginFile,_countof(PluginFile),"Plugins\\*.dll");

	HANDLE fHandle; 
	WIN32_FIND_DATA fd; 
	memset (&fd,0,sizeof(WIN32_FIND_DATA));

#ifdef b32BitBuild
	ULONG base;
#else 
	ULONGLONG base;
#endif

	fHandle=FindFirstFile((LPCSTR)PluginFile,&fd); 
	do { 
		if ((!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) && (strlen (fd.cFileName) != 0)) { 
			GetHomePath (PluginFile,_countof(PluginFile));
			strcat_s (PluginFile,_countof(PluginFile),"Plugins\\");
			strcat_s (PluginFile,_countof(PluginFile),(const char *)fd.cFileName);


			char CurrentPathShortened[MAX_PATH+1];
			memset (&CurrentPathShortened,0,_countof(CurrentPathShortened));
			if (strlen (PluginFile) > 57) {
				PathCompactPathEx (CurrentPathShortened,PluginFile, 57, NULL);
			}

			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: trying to load '%s'",strlen(PluginFile) > 57 ? CurrentPathShortened : PluginFile);
#ifdef b32BitBuild
			base = (ULONG)LoadLibrary ((LPCSTR)PluginFile);
#else
			base = (ULONGLONG)LoadLibrary ((LPCSTR)PluginFile);
#endif
			if (base != NULL) {
				IMAGE_DOS_HEADER *doh = (IMAGE_DOS_HEADER *) (BYTE *)base;
				IMAGE_NT_HEADERS *peh = (IMAGE_NT_HEADERS*) (BYTE *)(base + doh->e_lfanew);
#ifdef b32BitBuild
				ULONG exprva = peh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
#else
				ULONGLONG exprva = peh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
#endif
				if(exprva) {
					IMAGE_EXPORT_DIRECTORY * exp = (IMAGE_EXPORT_DIRECTORY *)(exprva + base);
					if(exp->NumberOfNames > 0) {
						DWORD*	funcs = (DWORD*)(exp->AddressOfFunctions + base);
						DWORD*	names = (DWORD*)(exp->AddressOfNames + base);
						WORD*	ordinals = (WORD*)(exp->AddressOfNameOrdinals + base);

						strcpy_s ((char *)MyDllTable->DllAscii,_countof (MyDllTable->DllAscii),(const char *)fd.cFileName);
						MyDllTable->FunctionsAvailable = exp->NumberOfNames;
						MyDllTable->DebugEventCallback = GetProcAddress ((HMODULE)base,"DebugEventCallback");
						if (MyDllTable->DebugEventCallback != NULL) {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: PLUGIN: DebugEvent handler installed for %s",fd.cFileName);
						}
						MyDllTable->Apis = (PApiTable) VirtualAlloc (NULL,exp->NumberOfNames*sizeof(ApiTable),MEM_COMMIT,PAGE_READWRITE);
						PApiTable MyApiTable = MyDllTable->Apis;

						if (MyApiTable != NULL) {
							for(DWORD i = 0;i < exp->NumberOfNames;i++) {
#ifdef b32BitBuild
								MyApiTable->Offset = funcs[ordinals[i]] + (ULONG)base;
#else
								MyApiTable->Offset = funcs[ordinals[i]] + (ULONGLONG)base;
#endif
								strcpy_s ((char *)MyApiTable->ApiAscii,_countof(MyApiTable->ApiAscii),(const char *)(names[i] + base));
								MyApiTable++;
							}
						}
						MyDllTable++;
					}
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: PLUGIN: %s has no exports",fd.cFileName);
				}
			} else {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: PLUGIN: %s has failed to load",fd.cFileName);
			}
		} 
	} while (FindNextFile(fHandle,&fd));
	FindClose(fHandle);

	// ADD ALL FOUND EXPORTS TO THE CMDHELPER ARRAY
	MyDllTable = (PDllTable)&PluginTableBuffer;
	while (MyDllTable->FunctionsAvailable != NULL) {
		PApiTable MyApiTable = MyDllTable->Apis;
		for(unsigned int i = 0; i < MyDllTable->FunctionsAvailable; i++) {
			int CmdNr = 0;
			bool already_stored = false;
			bool IsOne = false;
			if (_strnicmp((const char *)MyApiTable->ApiAscii,"UGDbg_",6) == NULL) {
				IsOne = true;
			}

			// PEiD PLUGIN SUPPORT
			if (_stricmp((const char *)MyApiTable->ApiAscii,"DoMyJob") == NULL) {
				char FileName [MAX_PATH] = "";
				strcpy_s (FileName,_countof(FileName),(const char *)MyDllTable->DllAscii);
				GetFileName (FileName,MAX_PATH);
				sprintf_s ((char *)MyApiTable->ApiAscii,_countof(MyApiTable->ApiAscii),"PEID__%s",FileName);
				IsOne = true;
			}

			while (strlen(Command[CmdNr]) != 0) {
				if (_stricmp ((const char *)Command[CmdNr],(const char *)&MyApiTable->ApiAscii[6]) == 0) {
					already_stored = true;
					break;
				}
				CmdNr++;
			}

			if (already_stored == false && IsOne == true) {
				strcpy_s ((char *)Command[CmdNr],50,(const char *)&MyApiTable->ApiAscii[6]);
				// add Help + Syntax if available
				char SyntaxName[MAX_PATH] = "Syntax_";
				strcat_s (SyntaxName,_countof(SyntaxName),(const char *)&MyApiTable->ApiAscii[6]);
				SDK_Char GetSyntax = (SDK_Char)GetProcAddress ((HMODULE)base,SyntaxName);
				if (GetSyntax != NULL) {
					__try {
						strcpy_s (Syntax[CmdNr],_countof(Syntax[CmdNr]),GetSyntax (DebugDataExchange,0,0));
					} __finally {
					}
				}
				char HelpName[MAX_PATH] = "Help_";
				strcat_s (HelpName,_countof(SyntaxName),(const char *)&MyApiTable->ApiAscii[6]);
				SDK_Char GetHelp = (SDK_Char)GetProcAddress ((HMODULE)base,HelpName);
				if (GetHelp != NULL) {
					__try {
						strcpy_s (HelpText[CmdNr],_countof(HelpText[CmdNr]),GetHelp (DebugDataExchange,0,0));
					} __finally {
					}
				}
			}

			MyApiTable++;
		}
		MyDllTable++;
	}
	return true;
}

// *****************************************************************
//  Build API tree
// 
// *****************************************************************
bool BuildApiTree (DWORD dwPID, DebugStruc * DebugDataExchange)
{
	//return true;
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if(hModuleSnap == INVALID_HANDLE_VALUE) {
		return(FALSE);
	}

	me32.dwSize = sizeof(MODULEENTRY32);
	if( !Module32First(hModuleSnap, &me32)) {
		CloseHandle(hModuleSnap);     // Must clean up the snapshot object!
		return(FALSE);
	}

	PDllTable MyDllTable = (PDllTable)&DllTableBuffer;
	do {
		_strlwr_s (me32.szModule,_countof(me32.szModule));
		if (strstr ((const char *)me32.szModule,".exe") == 0) {
#ifdef b32BitBuild
			DWORD base = (DWORD)GetModuleHandle (me32.szModule);
#else
			ULONGLONG base = (ULONGLONG)GetModuleHandle (me32.szModule);
#endif
			if (base != NULL) {
				IMAGE_NT_HEADERS *peh = (IMAGE_NT_HEADERS*)(((IMAGE_DOS_HEADER*)base)->e_lfanew + base);
#ifdef b32BitBuild
				DWORD exprva = peh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
#else
				ULONGLONG exprva = peh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
#endif
				if(exprva) {
					IMAGE_EXPORT_DIRECTORY* exp = (IMAGE_EXPORT_DIRECTORY*)(exprva + base);
					if(exp->NumberOfNames > 0) {

						DWORD*	funcs = (DWORD*)(exp->AddressOfFunctions + base);
						DWORD*	names = (DWORD*)(exp->AddressOfNames + base);
						WORD*	ordinals = (WORD*)(exp->AddressOfNameOrdinals + base);

						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s has %d exports",me32.szModule,exp->NumberOfNames);
						strcpy_s ((char *)MyDllTable->DllAscii,_countof(MyDllTable->DllAscii),(const char *)me32.szModule);
						_strupr_s ((char *)MyDllTable->DllAscii,_countof(MyDllTable->DllAscii));
						MyDllTable->FunctionsAvailable = exp->NumberOfNames;
						MyDllTable->Apis = (PApiTable) VirtualAlloc (NULL,exp->NumberOfNames*sizeof(ApiTable),MEM_COMMIT,PAGE_READWRITE);
						PApiTable MyApiTable = MyDllTable->Apis;

						if (MyApiTable != NULL) {
							for(DWORD i = 0;i < exp->NumberOfNames;i++) {
								MyApiTable->Offset = funcs[ordinals[i]] + base;
								strcpy_s ((char *)MyApiTable->ApiAscii,_countof(MyDllTable->DllAscii),(const char *)(names[i] + base));
								MyApiTable++;
							}
						}
						MyDllTable++;
					}
				}
			}
		}
	} while(Module32Next(hModuleSnap, &me32));

	// BUBBLE-SORT ALL ENTRIES NOW!
	*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: sorting export table entries");
	MyDllTable = (PDllTable)&DllTableBuffer;
	while (MyDllTable->FunctionsAvailable != NULL) {
		PApiTable MyApiTable = MyDllTable->Apis;
		for(unsigned int i = 0; i < MyDllTable->FunctionsAvailable; i++) {
			PApiTable MyApiTable = MyDllTable->Apis;
			for(unsigned int j = 0; j < (MyDllTable->FunctionsAvailable - 1); j++) {
#ifdef b32BitBuild
				ULONG Offset1 = MyApiTable->Offset;
#else
				ULONGLONG Offset1 = MyApiTable->Offset;
#endif
				MyApiTable++;
#ifdef b32BitBuild
				ULONG Offset2 = MyApiTable->Offset;
#else
				ULONGLONG Offset2 = MyApiTable->Offset;
#endif
				MyApiTable--;
				ApiTable TempApiTable1;
				ApiTable TempApiTable2;
				ZeroMemory (&TempApiTable1,sizeof (ApiTable));
				ZeroMemory (&TempApiTable2,sizeof (ApiTable));
				if(Offset1 < Offset2) {
					TempApiTable1.Offset = MyApiTable->Offset;
					strcpy_s ((char *)TempApiTable1.ApiAscii,_countof(TempApiTable1.ApiAscii),(const char *)MyApiTable->ApiAscii);
					MyApiTable++;
					TempApiTable2.Offset = MyApiTable->Offset;
					strcpy_s ((char *)TempApiTable2.ApiAscii,_countof(TempApiTable2.ApiAscii),(const char *)MyApiTable->ApiAscii);
					MyApiTable--;
					MyApiTable->Offset = TempApiTable2.Offset;
					strcpy_s ((char *)MyApiTable->ApiAscii,_countof(MyApiTable->ApiAscii),(const char *)TempApiTable2.ApiAscii);
					MyApiTable++;
					MyApiTable->Offset = TempApiTable1.Offset;
					strcpy_s ((char *)MyApiTable->ApiAscii,_countof(MyApiTable->ApiAscii),(const char *)TempApiTable1.ApiAscii);
					MyApiTable--;
				}
				MyApiTable++;
			}
		}
		MyDllTable++;
	}
	CloseHandle(hModuleSnap);     // Must clean up the snapshot object!
	*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: sorting completed");
	return true;
}

// *****************************************************************
//  CLEAR HARDWARE BREAKPOINT IN TARGET
// 
// *****************************************************************
void ClearHwBreakPoint (DebugStruc * DebugDataExchange)
{
	CONTEXT CTX;
	if (ReadContext (&CTX,DebugDataExchange) == true) {
		CTX.Dr0 = NULL;
		CTX.Dr7 |= 0x101;
		CTX.Dr7 &= 0xFFFCFFFF;
		CTX.Dr6 &= 0xFFFFFFF0;
		CTX.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
		HWBreakpointAddr = NULL;
		if (WriteContext (&CTX,DebugDataExchange) == true){
			return;
		} else {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to set context");
		}
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to get context");
	}
	return;
}



// *****************************************************************
//  SET HARDWARE BREAKPOINT
// 
// *****************************************************************
#ifdef b32BitBuild
void SetHwBreakPoint (ULONG Offset,DebugStruc * DebugDataExchange)
#else
void SetHwBreakPoint (ULONGLONG Offset,DebugStruc * DebugDataExchange)
#endif
{
	CONTEXT CTX;
	if (ReadContext (&CTX,DebugDataExchange) == true)	{
/*
		;-------------------------------------------------------------------------------------
		;
		; DEBUG REGISTER 7
		;
		; LEN3 R/W3 LEN2 R/W2 LEN1 R/W1 LEN0 R/W0 0 0 GD 0 0 1 GE LE G3 L3 G2 L2 G1 L1 G0 L0
		;
		;  00   00   00   00   00   00   00   00  0 0 0  0 0 0 0  1  0  1  0  1  0  1  0  1
		;
		; we enable here all "local-breakpoint-enable" bits
		; and the "local-exact-breakpoint-enable" bit
		;
		;-------------------------------------------------------------------------------------
*/
		CTX.Dr0 = Offset;
		CTX.Dr7 |= 0x101;
		CTX.Dr7 &= 0xFFFCFFFF;
		CTX.Dr6 &= 0xFFFFFFF0;
		HWBreakpointAddr = Offset;
		CTX.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
		
		if (WriteContext (&CTX,DebugDataExchange) == true) {
			return;
		} else {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to set context");
		}
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to get context");
	}
	return;
}

// *****************************************************************
//  PERFORM A STEPOVER WITH F10
// 
// *****************************************************************
void StepOver (DebugStruc * DebugDataExchange)
{
	CONTEXT CTX;
	if (ReadContext (&CTX,DebugDataExchange) == true) {
#ifdef b32BitBuild
		char * Mnemnomnic = FetchOpcodeAscii (CTX.Eip,DebugDataExchange);
#else
		char * Mnemnomnic = FetchOpcodeAscii (CTX.Rip,DebugDataExchange);
#endif
		if ((_stricmp (Mnemnomnic,"CALL") == 0) || (_strnicmp (Mnemnomnic,"REP",3) == 0) || (_stricmp (Mnemnomnic,"LOOP") == 0)) {   // 0xe8 = call | 0xff15 call [] | 0xf3 rep prefix
#ifdef b32BitBuild
			DWORD bRes = FetchOpcodeSize (CTX.Eip,DebugDataExchange);
			bRes += CTX.Eip;
#else
			ULONGLONG bRes = FetchOpcodeSize (CTX.Rip,DebugDataExchange);
			bRes += CTX.Rip;
#endif
			SetHwBreakPoint (bRes,DebugDataExchange);
			return;
		}
		EnableTrapflag (DebugDataExchange);
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to get context (stepover)");
	}
	return;
}

// *****************************************************************
//  SET TRAPFLAG IN TARGET PROCESS
// 
// *****************************************************************
int EnableTrapflag (DebugStruc * DebugDataExchange)
{
	CONTEXT CTX;
	if (ReadContext (&CTX,DebugDataExchange) == true) {
		CTX.EFlags |= 0x100;
		if (WriteContext (&CTX,DebugDataExchange) == true) {
			return true;
		} else {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to set context (enable trapflag)");
		}
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to get context (enable trapflag)");
	}
	return false;
}

// *****************************************************************
//  DISABLE TRAPFLAG IN TARGET PROCESS
// 
// *****************************************************************
int DisableTrapflag (DebugStruc * DebugDataExchange)
{
	CONTEXT CTX;
	if (ReadContext (&CTX,DebugDataExchange) == true) {
		CTX.EFlags &= 0xFFFFFEFF;
		if (WriteContext (&CTX,DebugDataExchange) == true) {
			return true;
		} else {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to set context (disable trapflag)");
		}
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to get context (disable trapflag)");
	}
	return false;
}

void ErrorExit(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    //MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
	//printf ("UGDBG: %s\n",lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}



// *****************************************************************
//  SET INT3 IN TARGET PROCESS
// 
// *****************************************************************
#ifdef b32BitBuild
BYTE SetInt3BreakPoint (ULONG Offset,PROCESS_INFORMATION * PI)
#else
BYTE SetInt3BreakPoint (ULONGLONG Offset,PROCESS_INFORMATION * PI)
#endif
{
	SIZE_T BRead = 0;
	SIZE_T BWrite;
	DWORD old;
	unsigned char LocalBuffer[1];
	unsigned char BreakPoint[1]={0xCC};

	if (VirtualProtectEx (PI->hProcess,(LPVOID)Offset,0x10,PAGE_READWRITE,&old) == NULL) ErrorExit ("VirtualProtectEx");
	if (ReadProcessMemory (PI->hProcess,(LPCVOID)Offset,&LocalBuffer,1,&BRead) != NULL)	{
		if (WriteProcessMemory (PI->hProcess,(void *)Offset,&BreakPoint,1,&BWrite) != NULL) {
			if (VirtualProtectEx (PI->hProcess,(LPVOID)Offset,0x10,old,&old) == NULL) ErrorExit ("VirtualProtectEx");
			FlushInstructionCache (PI->hProcess,(LPCVOID)Offset,32);
			return LocalBuffer[0];
		} else {
			return false;
		}
	} else {
		return false;
	}
}


// *****************************************************************
//  THIS IS THE MAIN DEBUGGER LOOP
// 
// *****************************************************************
void EnterDebugLoop(const LPDEBUG_EVENT DebugEv,PROCESS_INFORMATION * PI, DebugStruc * DebugDataExchange)
{
	DWORD InitStatus = DBG_CONTINUE;
	DWORD dwContinueStatus;
	SDK_Result SDK_Res;

	for(;;) 
	{ 	
		PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
		dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
		WaitForDebugEvent(DebugEv, INFINITE);

		// *****************************************************************
		// Check if we can query the context else we *MUST* continue
			CONTEXT CTX_Test;
			if (ReadContext (&CTX_Test,DebugDataExchange) == false) {
				ContinueDebugEvent(DebugEv->dwProcessId,DebugEv->dwThreadId,dwContinueStatus);
				continue;
			}
		// *****************************************************************


		// *****************************************************************
		//  Call all plugin callbacks before we try to handle them
		// 
		// *****************************************************************
		DllTable * MyDllTable = (PDllTable)&PluginTableBuffer;
		while (MyDllTable->FunctionsAvailable != NULL) {
			SDK_DebugEventCallback DbgCallback = (SDK_DebugEventCallback)MyDllTable->DebugEventCallback;
			if (DbgCallback != NULL) {
				__try {
					memset (&SDK_Res,0,sizeof(SDK_Result));
					dwContinueStatus = DbgCallback (DebugDataExchange,DebugEv,&SDK_Res);
					if (dwContinueStatus == DBG_CONTINUE) break;
				} __finally {
				}
			}
			MyDllTable++;
		}

		if (dwContinueStatus == DBG_CONTINUE) {
			if (SDK_Res.WaitForInput == true) {
				Sleep (50);
				// Set Message and wait
				DebugDataExchange->PollEvent = true;
				SDL_Event event;
				SDL_UserEvent userevent;
				userevent.type = SDL_USEREVENT;
				userevent.code = 0;
				event.type = SDL_USEREVENT;
				event.user = userevent;
				SDL_PushEvent (&event);
				userevent.type = SDL_USEREVENT;
				userevent.code = 1;
				event.type = SDL_USEREVENT;
				event.user = userevent;
				SDL_PushEvent (&event);
				userevent.type = SDL_USEREVENT;
				userevent.code = 2;
				event.type = SDL_USEREVENT;
				event.user = userevent;
				SDL_PushEvent (&event);

				while (DebugDataExchange->PollEvent == true) {
					Sleep (1);
				}
			}
			ContinueDebugEvent(DebugEv->dwProcessId,DebugEv->dwThreadId,dwContinueStatus);
			continue;
		}


		// *****************************************************************
		//  Internal handler(s)
		// 
		// *****************************************************************
		switch (DebugEv->dwDebugEventCode) 
		{ 
			dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
			case EXCEPTION_DEBUG_EVENT: 
				switch(DebugEv->u.Exception.ExceptionRecord.ExceptionCode)
				{ 
					case EXCEPTION_INVALID_HANDLE:
						break;
					case EXCEPTION_ACCESS_VIOLATION: 
						break;
					case EXCEPTION_BREAKPOINT:
						// **************************************************************************************
						//
						//	CHECK IF WE GOT ATTACHED TO AN APPLICATION
						//
						//	HAPPENS JUST 1 TIME
						//
						// **************************************************************************************
						if ((DebugDataExchange->Entrypoint == NULL) && (DebugDataExchange->Active == false)) {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: breakpoint reached");

							DebugDataExchange->Active = true;

							// WE SHOULD RETRIEVE ALL EXPORTS NOW
							BuildApiTree (DebugEv->dwProcessId,DebugDataExchange);
							// RESTORE ORIGINAL BYTE NOW!
							CONTEXT CTX;
							if (ReadContext (&CTX,DebugDataExchange) == true)	{
								// Set upcoming int3's to unhandled!
								InitStatus = DBG_EXCEPTION_NOT_HANDLED;
								// Set Message and wait
								DebugDataExchange->PollEvent = true;
								SDL_Event event;
								SDL_UserEvent userevent;
								userevent.type = SDL_USEREVENT;
								userevent.code = 0;
								event.type = SDL_USEREVENT;
								event.user = userevent;
								SDL_PushEvent (&event);
								userevent.type = SDL_USEREVENT;
								userevent.code = 1;
								event.type = SDL_USEREVENT;
								event.user = userevent;
								SDL_PushEvent (&event);
								userevent.type = SDL_USEREVENT;
								userevent.code = 2;
								event.type = SDL_USEREVENT;
								event.user = userevent;
								SDL_PushEvent (&event);

								while (DebugDataExchange->PollEvent == true) {
									Sleep (1);
								}
							} else {
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to get context");
							}
							dwContinueStatus = DBG_CONTINUE;
							break;
						}


						// **************************************************************************************
						//
						//	CHECK IF WE REACHED THE ENTRYPOINT OF THE APPLICATION
						//
						//	HAPPENS JUST 1 TIME
						//
						// **************************************************************************************
						if ((DebugEv->u.Exception.ExceptionRecord.ExceptionAddress == (PVOID)(DebugDataExchange->Entrypoint)) && (DebugDataExchange->Active == false)) {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: entrypoint reached");
							DebugDataExchange->Active = true;

							// WE SHOULD RETRIEVE ALL EXPORTS NOW
							BuildApiTree (PI->dwProcessId,DebugDataExchange);

							// RESTORE ORIGINAL BYTE NOW!
							CONTEXT CTX;
							if (ReadContext (&CTX,DebugDataExchange) == true) {
#ifdef b32BitBuild
								CTX.Eip--;
								if (WriteMem (CTX.Eip,1,&OrigByteBuffer,PI) == true) {
#else
								CTX.Rip--;
								if (WriteMem (CTX.Rip,1,&OrigByteBuffer[0],PI) == true) {
#endif
									if (WriteContext (&CTX,DebugDataExchange) == true) {
										// Set upcoming int3's to unhandled!
										InitStatus = DBG_EXCEPTION_NOT_HANDLED;
										// Set Message and wait
										DebugDataExchange->PollEvent = true;
										SDL_Event event;
										SDL_UserEvent userevent;
										userevent.type = SDL_USEREVENT;
										userevent.code = 0;
										event.type = SDL_USEREVENT;
										event.user = userevent;
										SDL_PushEvent (&event);
										userevent.type = SDL_USEREVENT;
										userevent.code = 1;
										event.type = SDL_USEREVENT;
										event.user = userevent;
										SDL_PushEvent (&event);
										userevent.type = SDL_USEREVENT;
										userevent.code = 2;
										event.type = SDL_USEREVENT;
										event.user = userevent;
										SDL_PushEvent (&event);

										while (DebugDataExchange->PollEvent == true) {
											Sleep (1);
										}
									} else {
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to set context");
									}
								} else {
									*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed writing to memory");
								}
							} else {
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to get context");
							}
							dwContinueStatus = DBG_CONTINUE;
							break;
						}
						dwContinueStatus = InitStatus;

						// **************************************************************************************
						//
						//	BPX ?
						//
						//
						// **************************************************************************************
						BreakFound = false;
						while (BpxTable->Offset != 0) {
							if (DebugEv->u.Exception.ExceptionRecord.ExceptionAddress == (PVOID)BpxTable->Offset) {
#ifdef b32BitBuild
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (bpx) triggered at %x",BpxTable->Offset);
#else
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (bpx) triggered at %llx",BpxTable->Offset);
#endif
								// RESTORE ORIGINAL BYTE NOW!
								CONTEXT CTX;
								if (ReadContext (&CTX,DebugDataExchange) == true) {
#ifdef b32BitBuild
									CTX.Eip--;
									if (WriteMem (CTX.Eip,1,&BpxTable->OrigByte,PI) == true) {
#else
									CTX.Rip--;
									if (WriteMem (CTX.Rip,1,&BpxTable->OrigByte,PI) == true) {
#endif
										if (WriteContext (&CTX,DebugDataExchange) == true) {
											dwContinueStatus = DBG_CONTINUE;
											// Set Message and wait
											DebugDataExchange->PollEvent = true;
											SDL_Event event;
											SDL_UserEvent userevent;
											userevent.type = SDL_USEREVENT;
											userevent.code = 0;
											event.type = SDL_USEREVENT;
											event.user = userevent;
											SDL_PushEvent (&event);
											BreakFound = true;
											while (DebugDataExchange->PollEvent == true) {
												Sleep (1);
											}
											break;
										} else {
											*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to write context");
										}
									} else {
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to write mem");
									}
								} else {
									*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to read context");
								}
							}
							BpxTable++;
						}
						if (BreakFound == true) break;

						// **************************************************************************************
						//
						//	break ?!
						//
						//
						// **************************************************************************************
						CONTEXT CTX_R;
						if ((ReadContext (&CTX_R,DebugDataExchange) == true) && (DebugDataExchange->Active == true)) {
							HMODULE NtDll = GetModuleHandle ("ntdll.dll");
#ifdef b32BitBuild
							if (CTX_R.Eip-1 == (DWORD)GetProcAddress (NtDll,"DbgBreakPoint")) {
#else
							if (CTX_R.Rip-1 == (ULONGLONG)GetProcAddress (NtDll,"DbgBreakPoint")) {
#endif
								// Set Message and wait
								DebugDataExchange->PollEvent = true;
								SDL_Event event;
								SDL_UserEvent userevent;
								userevent.type = SDL_USEREVENT;
								userevent.code = 0;
								event.type = SDL_USEREVENT;
								event.user = userevent;
								SDL_PushEvent (&event);
								while (DebugDataExchange->PollEvent == true) {
									Sleep (1);
								}
								dwContinueStatus = DBG_CONTINUE;
								break;
							}
						}


						// **************************************************************************************
						//
						//	i3here on/off ?!
						//
						//
						// **************************************************************************************
						if ((GetOption ("i3here") == true) && (DebugDataExchange->Active == true)) {
#ifdef b32BitBuild
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (embedded int3) triggered at %x",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#else
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (embedded int3) triggered at %llx",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#endif
							// Set Message and wait
							DebugDataExchange->PollEvent = true;
							SDL_Event event;
							SDL_UserEvent userevent;
							userevent.type = SDL_USEREVENT;
							userevent.code = 0;
							event.type = SDL_USEREVENT;
							event.user = userevent;
							SDL_PushEvent (&event);
							while (DebugDataExchange->PollEvent == true) {
								Sleep (1);
							}
							dwContinueStatus = DBG_CONTINUE;
						}
						break;

					case EXCEPTION_DATATYPE_MISALIGNMENT: 
						break;

					case EXCEPTION_SINGLE_STEP:
						// **************************************************************************************
						//
						//	STEPOVER ?
						//
						//
						// **************************************************************************************
						// OUR TRACER (OR NOT ?!) HIT :)
						// STEPOVER CODE
						if (DebugDataExchange->DbgEvent.u.Exception.ExceptionRecord.ExceptionAddress == (PVOID)(HWBreakpointAddr)) {
							if (BreakFound == true) BreakFound = false;
							ReEnableBreakpoints (DebugEv,DebugDataExchange);
							ClearHwBreakPoint (DebugDataExchange);
							dwContinueStatus = DBG_CONTINUE;
							// Set Message and wait
							DebugDataExchange->PollEvent = true;
							SDL_Event event;
							SDL_UserEvent userevent;
							userevent.type = SDL_USEREVENT;
							userevent.code = 0;
							event.type = SDL_USEREVENT;
							event.user = userevent;
							SDL_PushEvent (&event);
							while (DebugDataExchange->PollEvent == true) {
								Sleep (1);
							}
							break;
						}

						// **************************************************************************************
						//
						//	BPM ?
						//
						//
						// **************************************************************************************
						// BPM TRIGGERED ?
						CONTEXT CTX;
						if (ReadContext (&CTX,DebugDataExchange) == true) {
							if ((CTX.Dr6 & 0xF) != 0) {
								bool BpmsAreActive = false;
								// CHECK IF WE DO HAVE ANY BPM'S ACTIVE!
								while (BpxTable->Offset != 0) {
									if ((BpxTable->Type == 1) && (BpxTable->Enabled == TRUE)) {
										BpmsAreActive = true;
									}
									BpxTable++;
								}

								if (BpmsAreActive == true) {
									// DR0 TRIGGERED
									if ((CTX.Dr6 & 1) != 0)	{
#ifdef b32BitBuild
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (dr0) triggered at %x",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#else
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (dr0) triggered at %llx",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#endif
										CTX.Dr7 &= 0xFFFFFFFE;
									}
									// DR1 TRIGGERED
									if ((CTX.Dr6 & 2) != 0)	{
#ifdef b32BitBuild
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (dr1) triggered at %x",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#else
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (dr1) triggered at %llx",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#endif
										CTX.Dr7 &= 0xFFFFFFFB;
									}
									// DR2 TRIGGERED
									if ((CTX.Dr6 & 4) != 0)	{
#ifdef b32BitBuild
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (dr2) triggered at %x",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#else
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (dr2) triggered at %llx",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#endif
										CTX.Dr7 &= 0xFFFFFFEF;
									}
									// DR3 TRIGGERED
									if ((CTX.Dr6 & 8) != 0)	{
#ifdef b32BitBuild
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (dr3) triggered at %x",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#else
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Breakpoint (dr3) triggered at %llx",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#endif
										CTX.Dr7 &= 0xFFFFFFBF;
									}
									CTX.Dr6 &= 0xFFFFFFF0;
									WriteContext (&CTX,DebugDataExchange);

									DebugDataExchange->PollEvent = true;
									SDL_Event event;
									SDL_UserEvent userevent;
									userevent.type = SDL_USEREVENT;
									userevent.code = 0;
									event.type = SDL_USEREVENT;
									event.user = userevent;
									SDL_PushEvent (&event);
									DebugDataExchange->BpmTrigger = true;
									while (DebugDataExchange->PollEvent == true) {
										Sleep (1);
									}
									dwContinueStatus = DBG_CONTINUE;
									DebugDataExchange->BpmTrigger = false;
									ReEnableBreakpoints (DebugEv,DebugDataExchange);
									break;
								} else {
									dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
									break;
								}
							}
						}

						// **************************************************************************************
						//
						//	SINGLESTEP EMULATION FOR BREAKPOINTS
						//
						//
						// **************************************************************************************
						if (BreakFound == true) {
							ReEnableBreakpoints (DebugEv,DebugDataExchange);
							// Set Message and wait
							BreakFound = false;
							// we need to disable the massive sdl queue flood for runtrace else no other events can be catched!
							if (DebugDataExchange->AllowInt1 == true) {
								DebugDataExchange->PollEvent = true;
								SDL_Event event;
								SDL_UserEvent userevent;
								userevent.type = SDL_USEREVENT;
								userevent.code = 0;
								event.type = SDL_USEREVENT;
								event.user = userevent;
								SDL_PushEvent (&event);
								while (DebugDataExchange->PollEvent == true) {
									Sleep (1);
								}
							}
							dwContinueStatus = DBG_CONTINUE;
							break;
						}

						if (DebugDataExchange->AllowInt1 == true) {
							// Set Message and wait
							DebugDataExchange->PollEvent = true;
							// we need to disable the massive sdl queue flood for runtrace else no other events can be catched!
							if (DebugDataExchange->RunTraceActive == false) {
								SDL_Event event;
								SDL_UserEvent userevent;
								userevent.type = SDL_USEREVENT;
								userevent.code = 0;
								event.type = SDL_USEREVENT;
								event.user = userevent;
								SDL_PushEvent (&event);
							}
							while (DebugDataExchange->PollEvent == true) {
								Sleep (1);
							}
							dwContinueStatus = DBG_CONTINUE;
							break;
						}
						//printf ("unhandled int 1\n");
						
						break;

					case DBG_CONTROL_C: 
						break;

					default:
						break;
				};
			break;
			case CREATE_THREAD_DEBUG_EVENT: 
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Thread got born with ID:%x",DebugEv->dwThreadId);
				dwContinueStatus = DBG_CONTINUE;
				break;
			case CREATE_PROCESS_DEBUG_EVENT: 
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Process got born with ID:%x",DebugEv->dwProcessId);
#ifdef b32BitBuild
				DebugDataExchange->Entrypoint = (ULONG)DebugEv->u.CreateProcessInfo.lpStartAddress;
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Entrypoint : %8.8x",DebugEv->u.CreateProcessInfo.lpStartAddress);
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: ImageBase  : %8.8x",DebugEv->u.CreateProcessInfo.lpBaseOfImage);
				DebugDataExchange->ImageBase = (DWORD)DebugEv->u.CreateProcessInfo.lpBaseOfImage;
#else
				DebugDataExchange->Entrypoint = (ULONGLONG)DebugEv->u.CreateProcessInfo.lpStartAddress;
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Entrypoint : %16.16llx",DebugEv->u.CreateProcessInfo.lpStartAddress);
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: ImageBase  : %16.16llx",DebugEv->u.CreateProcessInfo.lpBaseOfImage);
				DebugDataExchange->ImageBase = (DWORD64)DebugEv->u.CreateProcessInfo.lpBaseOfImage;
#endif
				// SET BREAKPOINT ON ENTRYPOINT IF LOADED BY FILE DIALOG
				if (DebugEv->u.CreateProcessInfo.lpStartAddress != NULL) {
					OrigByteBuffer[0] = SetInt3BreakPoint (DebugDataExchange->Entrypoint,&DebugDataExchange->ProcessInfo);
				} else	{ // ELSE PREPARE STRUCTURE INFORMATION IF WE ATTACH TO A PROCESS
					DebugDataExchange->ProcessInfo.dwProcessId = DebugEv->dwProcessId;
					DebugDataExchange->ProcessInfo.dwThreadId = DebugEv->dwThreadId;
					DebugDataExchange->ProcessInfo.hProcess = DebugEv->u.CreateProcessInfo.hProcess;
					DebugDataExchange->ProcessInfo.hThread = DebugEv->u.CreateProcessInfo.hThread;
				}
				dwContinueStatus = DBG_CONTINUE;
				break;
			case EXIT_THREAD_DEBUG_EVENT: 
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Thread died with ID:%x",DebugEv->dwThreadId);
				dwContinueStatus = DBG_CONTINUE;
				break;
			case EXIT_PROCESS_DEBUG_EVENT: 
				dwContinueStatus = DBG_CONTINUE;
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Process died with ID:%x",DebugEv->dwProcessId);
				DebugDataExchange->Active = false;
				DebugDataExchange->DisassembleFromOffset = NULL;
				DebugDataExchange->DisplayMemoryOffset = NULL;
				break;
			case LOAD_DLL_DEBUG_EVENT: 
				dwContinueStatus = DBG_CONTINUE;
				//*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Dll loaded");
				break;
			case UNLOAD_DLL_DEBUG_EVENT: 
				dwContinueStatus = DBG_CONTINUE;
				//*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Dll unloaded");
				break;
			case OUTPUT_DEBUG_STRING_EVENT: 
				if (DebugEv->u.DebugString.nDebugStringLength < 60) {
#ifdef b32BitBuild
					ReadMem ((ULONG)DebugEv->u.DebugString.lpDebugStringData,DebugEv->u.DebugString.nDebugStringLength,&DbgMsg,PI);
#else
					ReadMem ((ULONGLONG)DebugEv->u.DebugString.lpDebugStringData,DebugEv->u.DebugString.nDebugStringLength,&DbgMsg,PI);
#endif
				} else { 
#ifdef b32BitBuild
					ReadMem ((ULONG)DebugEv->u.DebugString.lpDebugStringData,60,&DbgMsg,PI);
#else
					ReadMem ((ULONGLONG)DebugEv->u.DebugString.lpDebugStringData,60,&DbgMsg,PI);
#endif
				}
				DbgMsg[60] = 0;
				while (stringReplace("%","",DbgMsg) != NULL); // thx libx
				for (unsigned int i = 0; i < strlen (DbgMsg); i ++) {
					if (DbgMsg[i] == '\n') DbgMsg[i] = ' ';
				}
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s",DbgMsg);
				dwContinueStatus = DBG_CONTINUE;
				break;
			case RIP_EVENT:
				dwContinueStatus = DBG_CONTINUE;
				break;
		}
		if (dwContinueStatus == DBG_EXCEPTION_NOT_HANDLED) {
			//*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: exception (%x) unhandled pass on to debuggee",DebugEv->u.Exception.ExceptionRecord.ExceptionCode);
		}
		ContinueDebugEvent(DebugEv->dwProcessId,DebugEv->dwThreadId,dwContinueStatus);
	}
}


int DebuggerThread (void *p)
{
	memset (&BpxBuffer,0,_countof(BpxBuffer));
	DebugStruc * DebugDataExchange = (DebugStruc *)p;
	if (DebugDataExchange->dwProcessId == NULL) {
		// SETUP CREATEPROCESS
		ZeroMemory(&DebugDataExchange->StartupInfo, sizeof(STARTUPINFO) );
		DebugDataExchange->StartupInfo.cb = sizeof(STARTUPINFO);
		ZeroMemory( &DebugDataExchange->ProcessInfo, sizeof(PROCESS_INFORMATION) );

		BOOL bResult = CreateProcess (DebugDataExchange->FileName,NULL,NULL,NULL,FALSE,DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS,NULL,NULL,&DebugDataExchange->StartupInfo,&DebugDataExchange->ProcessInfo);
		if (bResult != NULL) {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Created Debug Process");
			// SET A BREAKPOINT AT THE TARGETS ENTRYPOINT
			PIMAGE_DOS_HEADER DosHdr = (PIMAGE_DOS_HEADER) (BYTE*)DebugDataExchange->Debuggee.FileOffset;
			PIMAGE_NT_HEADERS NtHdr = (PIMAGE_NT_HEADERS) (BYTE*)(DebugDataExchange->Debuggee.FileOffset+DosHdr->e_lfanew);

	#ifdef b32BitBuild	
			if (NtHdr->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Machine type other than i386 is unsupported");
	#else
			if (NtHdr->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Machine type other than AMD64 is unsupported");
	#endif
				return false;
			}

			DebugDataExchange->ImageSize = NtHdr->OptionalHeader.SizeOfImage;
			DebugDataExchange->ImageBase = NtHdr->OptionalHeader.ImageBase;
			DebugDataExchange->dwProcessId = DebugDataExchange->ProcessInfo.dwProcessId;
			EnterDebugLoop (&DebugDataExchange->DbgEvent,&DebugDataExchange->ProcessInfo, DebugDataExchange);
		} else { 
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Failed to create Process");
		}
		GlobalFree (DebugDataExchange->Debuggee.FileOffset);
	} else {
		int bRes = DebugActiveProcess (DebugDataExchange->dwProcessId);
		if (bRes != NULL) {
			EnterDebugLoop (&DebugDataExchange->DbgEvent,&DebugDataExchange->ProcessInfo, DebugDataExchange);
		} else {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Failed to attach to Process");
		}
	}
	return true;
}

bool InitializeCore (char * FileName,DWORD dwPID, RETBUFFER * Debuggee, Messages * Queue, bool * ShowLatestMessages,DebugStruc * DebugDataExchange)
{
	memset (Debuggee,0,sizeof(Debuggee));
	if ((FileName != NULL) && (dwPID == NULL)) {
		char FileNameShortened[MAX_PATH+1] = "";
		PathCompactPathEx (FileNameShortened,FileName, 30, NULL);

		int bRes = ReadFileMem (FileName,Debuggee);
		if (bRes == (int)true) {
			*ShowLatestMessages = DisplayMessage (Queue,"UGDBG: loaded %s into memory",FileNameShortened);

			memcpy (&DebugDataExchange->Debuggee,Debuggee,sizeof(RETBUFFER));
			memset (&DllTableBuffer,0,_countof(DllTableBuffer));
			DebugDataExchange->Queue = Queue;
			DebugDataExchange->ShowLatestMessages = ShowLatestMessages;
			DebugDataExchange->PollEvent = false;
			DebugDataExchange->AllowInt1 = false;
			DebugDataExchange->PDBLoaded = false;
			DebugDataExchange->DllTablePtr = (DllTable *)&DllTableBuffer;
			strcpy_s (DebugDataExchange->FileName,MAX_PATH,FileName);

			SDL_Thread * DbgThread = SDL_CreateThread (DebuggerThread,DebugDataExchange);
			DebugDataExchange->Thread = DbgThread;
		} else {
			*ShowLatestMessages = DisplayMessage (Queue,"UGDBG: failed to load %s into memory errorcode: %d",FileNameShortened,bRes);
			return false;
		}
	} else {
		if ((dwPID != NULL) && (FileName == NULL)) {
			*ShowLatestMessages = DisplayMessage (Queue,"UGDBG: trying to attach...");

			memcpy (&DebugDataExchange->Debuggee,Debuggee,sizeof(RETBUFFER));
			memset (&DllTableBuffer,0,_countof(DllTableBuffer));
			DebugDataExchange->Queue = Queue;
			DebugDataExchange->ShowLatestMessages = ShowLatestMessages;
			DebugDataExchange->PollEvent = false;
			DebugDataExchange->AllowInt1 = false;
			DebugDataExchange->PDBLoaded = false;
			DebugDataExchange->DllTablePtr = (DllTable *)&DllTableBuffer;
			DebugDataExchange->dwProcessId = dwPID;
			SDL_Thread * DbgThread = SDL_CreateThread (DebuggerThread,DebugDataExchange);
			DebugDataExchange->Thread = DbgThread;
		}
	}
	return true;
}