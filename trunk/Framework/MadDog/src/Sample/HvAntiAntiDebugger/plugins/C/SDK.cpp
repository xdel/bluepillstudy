// SDK.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include "sdl.h"
#include "sdl_thread.h"
#include "SDK.h"
#include <psapi.h>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

SDK_API void GfxCallback (SDL_Surface * Screen)
{
	return;
}

// The name of the command is Hello (which is being stripped from the export table name)
SDK_API void UGDbg_hello (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"Hello World");
	return;
}

SDK_API char * Help_hello (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	return "hello world help text";
}

// Variables needed by tracex
bool tracex_Active = false;
bool HandleSingleStep = false;
ULONG OldProtect;
#ifdef b32BitBuild
ULONG low;
ULONG high;
#else
ULONGLONG low;
ULONGLONG high;
#endif

// The name of the command is Hello (which is being stripped from the export table name)
SDK_API void UGDbg_tracex (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	if (tracex_Active == true) {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] %s: already active",argv[0]);
		return;
	}
	tracex_Active = false;
	if (DebugDataExchange->Active == true) {
		if (argc == 3) {
			// analyze syntax first
			low = Evaluate (argv[1],DebugDataExchange);
			high = Evaluate (argv[2],DebugDataExchange);
			
			if (low >= high) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] %s: lowval (%x) must be lower than highval (%x)",argv[0],low,high);
				return;
			}

			if (VirtualProtectEx (DebugDataExchange->ProcessInfo.hProcess,(LPVOID)low,high-low,PAGE_NOACCESS,&OldProtect) != NULL) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] %s: starting... be patient",argv[0]);
				tracex_Active = true;
				// CALL GO NOW
				UGDbg_go (DebugDataExchange,argc,argv);

				return;
			} else {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] %s: failed to start... (1)",argv[0]);
			}
		} else {
			if (argc > 3) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[x] %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[x] %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			return;
		}
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[x] %s: is only available when the debugger is active",argv[0]);
	}
	return;
}

SDK_API char * Syntax_tracex (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
#ifdef b32BitBuild	
	return "<low eip> <high eip>";
#else
	return "<low rip> <high rip>";
#endif
}


SDK_API ULONG DebugEventCallback (DebugStruc * DebugDataExchange, const LPDEBUG_EVENT DebugEv, SDK_Result * SDK_Res) {
	ULONG dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

	switch (DebugEv->dwDebugEventCode) 
	{ 
		case EXCEPTION_DEBUG_EVENT: 
			switch(DebugEv->u.Exception.ExceptionRecord.ExceptionCode)
			{ 
				case EXCEPTION_INVALID_HANDLE:
					break;
				case EXCEPTION_ACCESS_VIOLATION:
					if (tracex_Active == true) {
						if ((DebugEv->u.Exception.ExceptionRecord.ExceptionAddress >= (PVOID)low) && (DebugEv->u.Exception.ExceptionRecord.ExceptionAddress <= (PVOID)high)) {
#ifdef b32BitBuild
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] tracex: we are in range, %x",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#else
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] tracex: we are in range, %llx",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress);
#endif
							VirtualProtectEx (DebugDataExchange->ProcessInfo.hProcess,(LPVOID)low,high-low,OldProtect,&OldProtect);
							dwContinueStatus = DBG_CONTINUE;
							SDK_Res->WaitForInput = true;
							tracex_Active = false;
						} else {
							if ((DebugEv->u.Exception.ExceptionRecord.ExceptionInformation[1] >= low) && (DebugEv->u.Exception.ExceptionRecord.ExceptionInformation[1] <= high)) {
/*
#ifdef b32BitBuild
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] tracex: range access from %x -> %x",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress,DebugEv->u.Exception.ExceptionRecord.ExceptionInformation[1]);
#else
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] tracex: range access from %llx -> %llx",DebugEv->u.Exception.ExceptionRecord.ExceptionAddress,DebugEv->u.Exception.ExceptionRecord.ExceptionInformation[1]);
#endif
								*/
								VirtualProtectEx (DebugDataExchange->ProcessInfo.hProcess,(LPVOID)low,high-low,OldProtect,&OldProtect);
								dwContinueStatus = DBG_CONTINUE;
								EnableTrapflag (DebugDataExchange);
								HandleSingleStep = true;
								SDK_Res->WaitForInput = false;
							} else {
								dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
							}
						}
					}
					break;
				case EXCEPTION_BREAKPOINT:
					break;
				case EXCEPTION_SINGLE_STEP:
					if (HandleSingleStep == true) {
						VirtualProtectEx (DebugDataExchange->ProcessInfo.hProcess,(LPVOID)low,high-low,PAGE_NOACCESS,&OldProtect);
						HandleSingleStep = false;
						dwContinueStatus = DBG_CONTINUE;
						SDK_Res->WaitForInput = false;
					}
					break;
			}
	}
	return dwContinueStatus;
}

SDK_API char * Help_tracex (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	return "autotraces till given offset range";
}


// The name of the command is go (which is being stripped from the export table name)
SDK_API void UGDbg_go (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	if (DebugDataExchange->Active == true) {
		DebugDataExchange->AllowScrolling = false;
		DebugDataExchange->AllowInt1 = false;
		DisableTrapflag (DebugDataExchange);
		DebugDataExchange->PollEvent = false;
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[x] this command is only available when you debug a process...");	
	}
	return;
}

SDK_API char * Help_go (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	return "run";
}

// The name of the command is trace_sample (which is being stripped from the export table name)
SDK_API void UGDbg_trace (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	if (DebugDataExchange->Active == true) {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] We single step 5 commands using the inbuilt parser");
		for (int i = 0; i < 5; i++) {
			// we have to wait for the debugger to be ready to accept commands
			do {
				Sleep (1);
			} while (DebugDataExchange->PollEvent == false);
			CommandParser (&DebugDataExchange->Debuggee,DebugDataExchange,"t");
		}
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] completed...");

		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] We single step 5 commands using raw access");

		for (int i = 0; i < 5; i++) {
			// we have to wait for the debugger to be ready to accept commands
			do {
				Sleep (1);
			} while (DebugDataExchange->PollEvent == false);
			// Enable Trap Flag
			EnableTrapflag (DebugDataExchange);
			// Continue
			DebugDataExchange->AllowScrolling = false;
			DebugDataExchange->AllowInt1 = true;
			DebugDataExchange->PollEvent = false;
		}

		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] completed...");
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[x] this command is only available when you debug a process...");
	}
	return;
}

SDK_API char * Help_trace (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	return "auto-singlestep sdk demonstration";
}


// The name of the command is pret (which is being stripped from the export table name)
SDK_API void UGDbg_pret (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	if (DebugDataExchange->Active == true) {
		for (;;) {
			// we have to wait for the debugger to be ready to accept commands
			do {
				Sleep (1);
			} while (DebugDataExchange->PollEvent == false);

			CONTEXT CTX;
			if (ReadContext (&CTX,DebugDataExchange) == true) {
				BYTE RByte = 0;
#ifdef b32BitBuild
				if (ReadMem (CTX.Eip,1,&RByte,&DebugDataExchange->ProcessInfo) == true) {
#else
				if (ReadMem (CTX.Rip,1,&RByte,&DebugDataExchange->ProcessInfo) == true) {
#endif
					if ((RByte != 0xC2) && (RByte != 0xC3))	{
						CommandParser (&DebugDataExchange->Debuggee,DebugDataExchange,"p");
					} else {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[i] return reached");
						return;
					}
				}
			} else {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[x] failed to read context (pret)");
			}
		}
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"[x] this command is only available when you debug a process...");
	}
	return;
}

SDK_API char * Help_pret (DebugStruc * DebugDataExchange, int argc, char * argv[])
{
	return "traces till next 'return' is hit";
}

