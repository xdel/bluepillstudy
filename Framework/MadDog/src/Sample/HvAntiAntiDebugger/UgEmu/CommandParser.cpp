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
#include "split.h"

void DisplaySyntax (char * CommandName, DebugStruc * DebugDataExchange)
{
	for (int i = 0; i < _countof(Command); i++) {	
		if (_stricmp (Command[i],CommandName) == 0) {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Syntax: %s %s",Command[i],strlen(Syntax[i]) != 0 ? Syntax[i] : "n/a");
			return;
		}
	}
	return;
}

// ************************************************************************
//	ALIGN (DWORD RVA) align's offsets
//
//  Return Codes:  != 0 -> RAW POSITION
//
// ************************************************************************
#ifdef b32BitBuild
ULONG AlignOffset (ULONG Rva,ULONG Base)
#else
ULONGLONG AlignOffset (ULONGLONG Rva,ULONGLONG Base)
#endif
{
	// ALIGN ADDRESS
#ifdef b32BitBuild
	ULONG ROFS = Rva;
#else
	ULONGLONG ROFS = Rva;
#endif
	if (ROFS % Base !=0) {
		ROFS = ROFS + (Base - (ROFS % Base));
	}
	return ROFS;
}


extern "C" __declspec(dllexport) int CommandParser (RETBUFFER * InFile,DebugStruc * DebugDataExchange,char * CommandBuffer)
{
	int argc;
	char* argv[128];
	char* linecopy;

	*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,":%s",CommandBuffer);		


	linecopy = _strdup(CommandBuffer);
	argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
	if(!argc) {
		free(linecopy);
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Tokenizer failed to split the command");
		return false;
	}

	// *****************************************************************
	//  SDK Implementation, check if we have an user implemented API
	//  if so call it now
	// *****************************************************************
	PDllTable MyDllTable = (PDllTable)&PluginTableBuffer;
	while (MyDllTable->FunctionsAvailable != NULL) {
		PApiTable MyApiTable = MyDllTable->Apis;
		for(unsigned int i = 0; i < MyDllTable->FunctionsAvailable; i++) {
			if (_stricmp((const char *)&MyApiTable->ApiAscii[6],argv[0]) == NULL) {
				if (_strnicmp((const char *)&MyApiTable->ApiAscii[0],"PEID",4) == NULL) {
					SDK_PEiD PEiD_Fn;
					PEiD_Fn = (SDK_PEiD)MyApiTable->Offset;
					__try {
						SDL_SysWMinfo i;
						SDL_VERSION(&i.version);
						if (SDL_GetWMInfo ( &i)) {
							PEiD_Fn (i.window,DebugDataExchange->FileName,0x50456944,NULL);
						}
					} __finally {
					}
				} else {
					SDK SDK_Call;
					SDK_Call = (SDK)MyApiTable->Offset;
					__try {
						SDK_Call (DebugDataExchange,argc,argv);
					} __finally {
					}
				}
				free (linecopy);
				return true;
			}
			MyApiTable++;
		}
		MyDllTable++;
	}

	// *****************************************************************
	//  HELP
	// 
	// *****************************************************************
	if ((_stricmp (argv[0],"help") == 0) || (_stricmp (argv[0],"h") == 0)) {
		int i = 0;
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"<COLOR> User commands");
		while (strlen (Command[i]) != 0) {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-11s %-26s %-30s",Command[i],strlen(Syntax[i]) != 0 ? Syntax[i] : "n/a",strlen(HelpText[i]) != 0 ? HelpText[i] : "n/a");
			i++;
		}
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"<COLOR> Mouse commands");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Dblclick inside code window","set(enable)|disable bpx on opcode");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Dblclick inside log window","copy logbuffer to clipboard");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Mousewheel up|down any window","scroll up | down");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Button on a green line","resize inner windows");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"<COLOR> Control commands");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-Ctrl-Cursorkeys","Move Debugger Window");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-Ctrl-F12","Center Debugger Window");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","F1","Increase Window Size");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","F2","Decrease Window Size");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","F3","Increase Data Size");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Ctrl-F3","Decrease Data Size");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","F4","Increase Disassembly Size");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Ctrl-F4","Decrease Disassembly Size");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","F5","Go");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","F6","Code [on|off]");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","F8","Single Step");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","F10","Step Over");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Ctrl-F12","Save screenshot");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Ctrl-UP","Scroll Disassembly up");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Ctrl-DOWN","Scroll Disassembly down");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-UP","Scroll Data up (row)");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-DOWN","Scroll Data down (row)");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-LEFT","Scroll Data fwd");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-RIGHT","Scroll Data bckwd");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-PGUP","Scroll Data bckwd (page)");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-PGDN","Scroll Data fwd (page)");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Shift-UP","Scroll Logwindow up");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Shift-DOWN","Scroll Logwindow down");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Ctrl-R","Reload last debuggee");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Ctrl-L","File load dialog");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Alt-Letter (File dialog only)","Change drive according to letter");
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-38s %-30s","Ctrl-A","Attach process dialog");

		free(linecopy);
		return true;
	}


	// *****************************************************************
	//  QUIT
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"quit") == 0) {
		SDL_Quit ();
		ExitProcess (true);
	}

	// *****************************************************************
	//  Register
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"register") == 0) {
		if (RegisterSelf ("exefile\\shell\\Debug with UGDbg") == TRUE) {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: registered successfully",argv[0]);
		} else {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: registering failed",argv[0]);
		}
		free(linecopy);
		return true;
	}

	// *****************************************************************
	//  UnRegister
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"unregister") == 0) {
		BOOL bSuccess;
		bSuccess = RegDelnode(HKEY_CLASSES_ROOT, TEXT("exefile\\shell\\Debug with UGDbg"));

		if(bSuccess)
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: unregistered successfully",argv[0]);
		else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: unregistering failed",argv[0]);
		free(linecopy);
		return true;
	}

	// *****************************************************************
	//  t
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"t") == 0) {
		// Enable Trap Flag
		EnableTrapflag (DebugDataExchange);
		// Continue
		DebugDataExchange->AllowScrolling = false;
		DebugDataExchange->AllowInt1 = true;
		DebugDataExchange->PollEvent = false;
		free(linecopy);
		return true;
	}

	// *****************************************************************
	//  p
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"p") == 0) {
		DisableTrapflag (DebugDataExchange);
		StepOver (DebugDataExchange);
		// Continue
		DebugDataExchange->AllowInt1 = true;
		DebugDataExchange->AllowScrolling = false;
		DebugDataExchange->PollEvent = false;
		free(linecopy);
		return true;
	}



	// *****************************************************************
	//  DEBUG
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"debug") == 0) {
		if (argc == 2) {
			RETBUFFER Debuggee;
			memset (&Debuggee,0,sizeof(RETBUFFER));
			// TERMINATE CURRENTLY ACTIVE PROCESS!
			int * WindowSizeX = DebugDataExchange->WindowSizeX;
			int * WindowSizeY = DebugDataExchange->WindowSizeY;
			bool * ShowLatestMessages = DebugDataExchange->ShowLatestMessages;
			Messages * Queue = DebugDataExchange->Queue;

			CleanupDia ();
			SDL_KillThread (DebugDataExchange->Thread);
			Cleanup (DebugDataExchange);

			// FIRE DEBUGGER CORE
			memset (DebugDataExchange,0,sizeof (DebugStruc));
			DebugDataExchange->WindowSizeX = WindowSizeX;
			DebugDataExchange->WindowSizeY = WindowSizeY;
			DebugDataExchange->AllowScrolling = true;
			DebugDataExchange->ShowLatestMessages = ShowLatestMessages;
			DebugDataExchange->Queue = Queue;
			bool bRes = InitializeCore (argv[1],NULL,&Debuggee,DebugDataExchange->Queue,DebugDataExchange->ShowLatestMessages,DebugDataExchange);
			free(linecopy);
			return bRes;
		} else {
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
		}
		free(linecopy);
		return false;
	}


	// *****************************************************************
	//  CLS
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"cls") == 0) {
		DestroyQueue (DebugDataExchange->Queue);
		DebugDataExchange->Queue = InitQueue();
		free(linecopy);
		return true;
	}

	// *****************************************************************
	//  LOADCONFIG
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"loadconfig") == 0) {
		bool bRes = false;
		if (argc == 1) {
			bRes = ReLoadConfig ("sample.cfg");
		} else bRes = ReLoadConfig (argv[1]);
		if (bRes == true) {
			SetVideoMode(DebugDataExchange->Queue);
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: configuration loaded");
		} else {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to load configuration, maybe a switch of the current directory ?");
		}
		free(linecopy);
		return true;
	}

	// *****************************************************************
	//  SAVECONFIG
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"saveconfig") == 0) {
		bool bRes = false;
		if (argc == 1) {
			bRes = ReSaveConfig ("sample.cfg");
		} else {
			bRes = ReSaveConfig (argv[1]);
		}
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: configuration %s",bRes != true ? " failed to save" : "saved");
		free(linecopy);
		return true;
	}

	// *****************************************************************
	//  SET
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"set") == 0) {
		if (argc == 1) {
			// LIST SETTINGS
			for (int i = 0; i < _countof(Options); i++) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s=%s",Options[i],OptionSettings[i] != true ? "off" : "on");
			}
		} else {
			if (argc == 3) {
				// SET OPTION
				for (int i = 0; i < _countof(Options); i++) {
					if (_stricmp (argv[1],Options[i]) == 0) {
						if (_stricmp (argv[2],"on") == 0) OptionSettings[i] = true;
						if (_stricmp (argv[2],"off") == 0) OptionSettings[i] = false;
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s=%s",Options[i],OptionSettings[i] != true ? "off" : "on");
						free(linecopy);
						return true;
					}
				}
			}
			if (argc > 3) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}
		free(linecopy);
		return true;
	}

	// *****************************************************************
	//  RP
	//  
	// *****************************************************************
	if (_stricmp (argv[0],"rp") == 0) {
		if (argc == 1) {
			BuildPluginApiTree (DebugDataExchange);
			free(linecopy);
			return false;
		}
		if (argc > 1) {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);		
		} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
		DisplaySyntax (argv[0],DebugDataExchange);
		free(linecopy);
		return false;			
	}


	// *****************************************************************
	//  ?
	// 
	// *****************************************************************
	if (_stricmp (argv[0],"?") == 0) {
		if (argc == 2) {
			char FinalVal[10] = "";
#ifdef b32BitBuild
			ULONG FinalValDword = Evaluate (argv[1],DebugDataExchange);
#else
			ULONGLONG FinalValDword = Evaluate (argv[1],DebugDataExchange);
#endif
			memcpy (&FinalVal[0],&FinalValDword,sizeof(FinalValDword));
			int g = 0;
			while (isprint (FinalVal[g]) != 0) g++;
			FinalVal[g] = 0;

#ifdef b32BitBuild
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%8.8X %8.8d \"%s\"",FinalValDword,FinalValDword,FinalVal);
#else
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%16.16llX %16.16d \"%s\"",FinalValDword,FinalValDword,FinalVal);
#endif
			free(linecopy);
			return true;
		}
		if (argc > 2) {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
		} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
		DisplaySyntax (argv[0],DebugDataExchange);
		free(linecopy);
		return false;
	}



	if (DebugDataExchange->Active == true) {

		// *****************************************************************
		//  BPM
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"bpm") == 0) {
			if (argc == 3) {
#ifdef b32BitBuild
				ULONG Api;
#else
				ULONGLONG Api;
#endif
				CONTEXT CTX;
				if (ReadContext (&CTX,DebugDataExchange) == true) {
					Api = Evaluate (argv[1],DebugDataExchange);
					if (Api != false) {
						PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
						bool AlreadyActive = false;
						while (BpxTable->Offset != 0) {
							if (BpxTable->Offset == Api) {
								AlreadyActive = true;
								break;
							}
							BpxTable++;
						}
						if (AlreadyActive == false)	{
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

							if (CTX.Dr0 == 0) {
								CTX.Dr0 = Api;
								CTX.Dr7 |= 0x101;
								if (_stricmp (argv[2],"x") == 0) {
									CTX.Dr7 &= 0xFFFCFFFF;
								}
								if (_stricmp (argv[2],"w") == 0) {
									CTX.Dr7 |= 0x00010000;
								}
								if (_stricmp (argv[2],"r") == 0) {
									CTX.Dr7 |= 0x00030000;
								}
							} else {
								if (CTX.Dr1 == 0) {
									CTX.Dr1 = Api;
									CTX.Dr7 |= 0x104;
									if (_stricmp (argv[2],"x") == 0) {
										CTX.Dr7 &= 0xFFCFFFFF;
									}
									if (_stricmp (argv[2],"w") == 0) {
										CTX.Dr7 |= 0x00100000;
									}
									if (_stricmp (argv[2],"r") == 0) {
										CTX.Dr7 |= 0x00300000;
									}
								} else {
									if (CTX.Dr2 == 0) {
										CTX.Dr2 = Api;
										CTX.Dr7 |= 0x110;
										if (_stricmp (argv[2],"x") == 0) {
											CTX.Dr7 &= 0xFCFFFFFF;
										}
										if (_stricmp (argv[2],"w") == 0) {
											CTX.Dr7 |= 0x01000000;
										}
										if (_stricmp (argv[2],"r") == 0) {
											CTX.Dr7 |= 0x03000000;
										}
									} else {
										if (CTX.Dr3 == 0) {
											CTX.Dr3 = Api;
											CTX.Dr7 |= 0x140;
											if (_stricmp (argv[2],"x") == 0) {
												CTX.Dr7 &= 0xCFFFFFFF;
											}
											if (_stricmp (argv[2],"w") == 0) {
												CTX.Dr7 |= 0x10000000;
											}
											if (_stricmp (argv[2],"r") == 0) {
												CTX.Dr7 |= 0x30000000;
											}
										} else {
											*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: breakpoint limit exceeded",argv[0]);		
											free(linecopy);
											return false;
										}
									}
								}
							}
							BpxTable->Offset = Api;
							BpxTable->Enabled = true;
							BpxTable->Type = 1;
							if (_stricmp (argv[2],"x") == 0) {
								BpxTable->Type2 = 0;	// ONLY NEEDED FOR BPM!
							}
							if (_stricmp (argv[2],"r") == 0) {
								BpxTable->Type2 = 1;	// ONLY NEEDED FOR BPM!
							}
							if (_stricmp (argv[2],"w") == 0) {
								BpxTable->Type2 = 2;	// ONLY NEEDED FOR BPM!
							}
							if (WriteContext (&CTX,DebugDataExchange) == true) {
								free(linecopy);
								return true;
							} else {
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to write context",argv[0]);		
								free(linecopy);
								return false;
							}
						}
						free(linecopy);
						return false;
					}
					free(linecopy);
					return false;
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to read context",argv[0]);		
					free(linecopy);
					return false;
				}

			}

			if (argc > 3) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);		
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;

		}




		// *****************************************************************
		//  BPX
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"bpx") == 0) {
			if (argc == 2) {
				unsigned char HexBuff[1] = "";
#ifdef b32BitBuild
				ULONG BpxOffset = 0;
#else
				ULONGLONG BpxOffset = 0;
#endif
				BpxOffset = Evaluate (argv[1],DebugDataExchange);
				if (ReadMem (BpxOffset,1,&HexBuff,&DebugDataExchange->ProcessInfo) == true) {
					PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
					bool AlreadyActive = false;
					while (BpxTable->Offset != 0) {
						if (BpxTable->Offset == BpxOffset) {
							AlreadyActive = true;
							break;
						}
						BpxTable++;
					}
					if (AlreadyActive == false) {
						BpxTable->Offset = BpxOffset;
						BpxTable->OrigByte = SetInt3BreakPoint (BpxOffset,&DebugDataExchange->ProcessInfo);
						BpxTable->Enabled = true;
						BpxTable->Type = 0;
						BpxTable->Type2 = 0;	// ONLY NEEDED FOR BPM!
					} else {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: breakpoint already set");
					}
				}
#ifdef b32BitBuild
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: set on %8.8x",argv[0],BpxOffset);		
#else
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: set on %16.16llx",argv[0],BpxOffset);		
#endif
				free(linecopy);
				return true;
			}
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  BL
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"bl") == 0)
		{
			if (argc == 1) {
				int i = 0;
				char * bRes;
				PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
				//*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"<COLOR>Breakpoint list");
				while (BpxTable->Offset != 0) {
					bRes = PerformVALookup (BpxTable->Offset,DebugDataExchange);
					switch (BpxTable->Type)	{
						case 0:
							if (bRes != NULL) {
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPX %s",i,BpxTable->Enabled == TRUE ? " " : "*", bRes);		
							} else {
#ifdef b32BitBuild
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPX #%8.8X",i,BpxTable->Enabled == TRUE ? " " : "*", BpxTable->Offset);		
#else
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPX #%16.16llX",i,BpxTable->Enabled == TRUE ? " " : "*", BpxTable->Offset);
#endif
							}
							break;
						case 1:
							if (bRes != NULL) {
								switch (BpxTable->Type2) {
									case 0: 
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM %s X",i,BpxTable->Enabled == TRUE ? " " : "*",bRes);
										break;
									case 1:
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM %s R",i,BpxTable->Enabled == TRUE ? " " : "*",bRes);
										break;
									case 2:
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM %s W",i,BpxTable->Enabled == TRUE ? " " : "*",bRes);
										break;
										
								}
							} else {
								switch (BpxTable->Type2) {
									case 0:
#ifdef b32BitBuild
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM #%8.8X X",i,BpxTable->Enabled == TRUE ? " " : "*",BpxTable->Offset);
#else
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM #%16.16llX X",i,BpxTable->Enabled == TRUE ? " " : "*",BpxTable->Offset);
#endif
										break;
									case 1:
#ifdef b32BitBuild
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM #%8.8X R",i,BpxTable->Enabled == TRUE ? " " : "*",BpxTable->Offset);
#else
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM #%16.16llX R",i,BpxTable->Enabled == TRUE ? " " : "*",BpxTable->Offset);
#endif
										break;
									case 2:
#ifdef b32BitBuild
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM #%8.8X W",i,BpxTable->Enabled == TRUE ? " " : "*",BpxTable->Offset);
#else
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%2.2d) %s BPM #%16.16llX W",i,BpxTable->Enabled == TRUE ? " " : "*",BpxTable->Offset);
#endif
										break;

								}
							}

							break;
						case 2:
#ifdef b32BitBuild
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: BPR #%d: %8.8x %-20s",i,BpxTable->Offset, BpxTable->Enabled == TRUE ? "-active" : "-inactive");		
#else
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: BPR #%d: %16.16llx %-20s",i,BpxTable->Offset, BpxTable->Enabled == TRUE ? "-active" : "-inactive");		
#endif
							break;
					}
					i++;
					BpxTable++;
				}
				free(linecopy);
				return true;
			}
			if (argc > 1) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  BC
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"bc") == 0) {
			if (argc == 2) {
				int Offset;
				int i = 0;
				int nr = 0;
				PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
				sscanf_s (argv[1],"%d",&Offset);
				while (BpxTable->Offset != 0) {
					// make the condition always true if the user wants to disable all active breakpoints
					if (_stricmp (argv[1],"*") == 0) Offset = nr;
					if (Offset == nr) {
						switch (BpxTable->Type) {
							case 0:
								WriteMem (BpxTable->Offset,1,&BpxTable->OrigByte,&DebugDataExchange->ProcessInfo);
								break;
							case 1:
								CONTEXT CTX;
								if (ReadContext (&CTX,DebugDataExchange) == true) {
									if (CTX.Dr0 == BpxTable->Offset) CTX.Dr0 = 0;
									if (CTX.Dr1 == BpxTable->Offset) CTX.Dr1 = 0;
									if (CTX.Dr2 == BpxTable->Offset) CTX.Dr2 = 0;
									if (CTX.Dr3 == BpxTable->Offset) CTX.Dr3 = 0;
									if (WriteContext (&CTX,DebugDataExchange) == true) {
									} else {
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to write context",argv[0]);
									}
								} else {
									*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to read context",argv[0]);
								}
								break;

						}
						while (BpxTable->Offset != 0) {
							i++;
							BpxTable++;
						}
						BpxTable -= i;
						memcpy (BpxTable,BpxTable+1,sizeof(BreakpointTable)*i);
						BpxTable += i-1;
						BpxTable->Offset = 0;
						BpxTable->OrigByte = 0;
					}
					BpxTable++;
					nr++;
					// make the condition always true if the user wants to disable all active breakpoints
					if (_stricmp (argv[1],"*") == 0) BpxTable = (PBreakpointTable)&BpxBuffer;
				}
				free(linecopy);
				return true;
			}
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}


		// *****************************************************************
		//  BD
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"bd") == 0) {
			if (argc == 2) {
				int DisableNr;
				int i = 0;
				int nr = 0;
				PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
				sscanf_s (argv[1],"%d",&DisableNr);
				while (BpxTable->Offset != 0) {
					// make the condition always true if the user wants to disable all active breakpoints
					if (_stricmp (argv[1],"*") == 0) DisableNr = nr;
					if (DisableNr == nr) {
						switch (BpxTable->Type) {
							case 0:
								WriteMem (BpxTable->Offset,1,&BpxTable->OrigByte,&DebugDataExchange->ProcessInfo);
								FlushInstructionCache (DebugDataExchange->ProcessInfo.hProcess,(LPCVOID)BpxTable->Offset,32);
								BpxTable->Enabled = false;
								break;
							case 1:
								CONTEXT CTX;
								if (ReadContext (&CTX,DebugDataExchange) == true) {
									if ((CTX.Dr0 == BpxTable->Offset)) CTX.Dr7 &= 0xFFFFFFFE;
									if ((CTX.Dr1 == BpxTable->Offset)) CTX.Dr7 &= 0xFFFFFFFB;
									if ((CTX.Dr2 == BpxTable->Offset)) CTX.Dr7 &= 0xFFFFFFEF;
									if ((CTX.Dr3 == BpxTable->Offset)) CTX.Dr7 &= 0xFFFFFFBF;
									CTX.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
									if (WriteContext (&CTX,DebugDataExchange) == true) {
									} else {
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to write context",argv[0]);
									}
								} else {
									*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to read context",argv[0]);
								}
								BpxTable->Enabled = false;
								break;
						}
					}
					BpxTable++;
					nr++;
				}
				free(linecopy);
				return true;
			}
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}


		// *****************************************************************
		//  BE
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"be") == 0)
		{
			if (argc == 2) {
				int Offset;
				int i = 0;
				int nr = 0;
				PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
				sscanf_s (argv[1],"%d",&Offset);
				while (BpxTable->Offset != 0) {
					// make the condition always true if the user wants to disable all active breakpoints
					if (_stricmp (argv[1],"*") == 0) Offset = nr;
					if (Offset == nr) {
						switch (BpxTable->Type) {
							case 0:
								SetInt3BreakPoint (BpxTable->Offset,&DebugDataExchange->ProcessInfo);
								BpxTable->Enabled = true;
								break;
							case 1:
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
													*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: breakpoint limit exceeded",argv[0]);
													free(linecopy);
													return false;
												}
											}
										}
									}
									CTX.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
									if (WriteContext (&CTX,DebugDataExchange) == true) {
									} else {
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to write context",argv[0]);
										free(linecopy);
										return false;
									}
									BpxTable->Enabled = true;
								}
								break;
						}
					}
					BpxTable++;
					nr++;
				}
				free(linecopy);
				return true;
			}
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}


		// *****************************************************************
		//  D
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"d") == 0) {
			if (argc == 2) {
				int LineCounter = (GetDisassemblyYPos () - GetDataYPos()) / LINE_H;
#ifdef b32BitBuild
				ULONG Offset;
#else
				ULONGLONG Offset;
#endif
				Offset = Evaluate (argv[1],DebugDataExchange);
				if (LineCounter > 0) {
					DebugDataExchange->DisplayMemoryOffset = Offset;
				} else {
					LineCounter = (GetInputYPos() - GetLogYPos ()) / LINE_H;
					unsigned char HexBuff[100*16+1];
					CONTEXT CTX;

					if (ReadContext (&CTX,DebugDataExchange) == true) {
						if (ReadMem (Offset,LineCounter*16,&HexBuff,&DebugDataExchange->ProcessInfo) == true) {

							for (int i = 0; i < LineCounter; i++) {
								char Final[120] = "";
								char TmpString[80] = "";
#ifdef b32BitBuild
								sprintf_s (TmpString,80,"%4.4X:%8.8X ",CTX.SegDs,Offset+(i*16));
#else
								sprintf_s (TmpString,80,"%4.4X:%16.16llX ",CTX.SegDs,Offset+(i*16));
#endif
								strcpy_s (Final,_countof(Final),TmpString);

								for (int j = 0; j < 16; j++) {
									sprintf_s (TmpString,80,"%2.2X ",HexBuff[j+(i*16)]);
									strcat_s (Final,_countof(Final),TmpString);
								}
								strcat_s (Final,_countof(Final)," ");
								for (int j = 0; j < 16; j++) {
									if (isprint(HexBuff[j+(i*16)]) != NULL) {  
										sprintf_s (TmpString,80,"%c",HexBuff[j+(i*16)]);
										strcat_s (Final,_countof(Final),TmpString);
									} else {
										strcat_s (Final,_countof(Final),".");
									}
								}
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%s",Final);
							}
						}
					}
				}
				free(linecopy);
				return true;
			}
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  u
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"u") == 0) {
			if (argc == 2) {
				int LineCounter = (GetLogYPos () - GetDisassemblyYPos()) / LINE_H;
#ifdef b32BitBuild
				ULONG Offset;
#else
				ULONGLONG Offset;
#endif
				Offset = Evaluate (argv[1],DebugDataExchange);
				if (LineCounter > 0) {
					DebugDataExchange->DisassembleFromOffset = Offset;
					DebugDataExchange->AllowScrolling = true;
				} else {
					LineCounter = (GetInputYPos() - GetLogYPos ()) / LINE_H;
					CONTEXT CTX;
					if (ReadContext (&CTX,DebugDataExchange) == true) {
						for (int i = 0; i < LineCounter; i ++) {
							_DecodedInst * decodedInstructions = FetchOpcode (Offset,DebugDataExchange);
							while (stringReplace ("0x","",(char *)&decodedInstructions->operands.p) != NULL);
							_strupr_s ((char *)&decodedInstructions->operands.p,60);

							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%4.4X:%0*I64X %-20s %-9s%s%s",CTX.SegCs, GetDt() != Decode64Bits ? 8 : 16, decodedInstructions->offset, decodedInstructions->instructionHex.p, decodedInstructions->mnemonic.p, decodedInstructions->operands.length != 0 ? " " : "", decodedInstructions->operands.p);
							Offset += decodedInstructions->size;
							
						}
					}
				}
				free(linecopy);
				return true;
			}
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);		
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  .
		// 
		// *****************************************************************
		if (_stricmp (argv[0],".") == 0) {
			if (argc == 1) {
				CONTEXT CTX;
				if (ReadContext (&CTX,DebugDataExchange) == true) {
#ifdef b32BitBuild
					DebugDataExchange->DisassembleFromOffset = CTX.Eip;
#else
					DebugDataExchange->DisassembleFromOffset = CTX.Rip;
#endif
					free(linecopy);
					return true;
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to read context");
				}
			}
			if (argc > 1) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
				DisplaySyntax (argv[0],DebugDataExchange);
			} 
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  DUMP
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"dump") == 0) {
			if (argc == 4) {
				ULONG DumpSize;
				sscanf_s ((const char *)(argv[2]),"%X",&DumpSize);
#ifdef b32BitBuild
				ULONG DumpOffset;
				sscanf_s ((const char *)(argv[1]),"%X",&DumpOffset);
#else
				ULONGLONG DumpOffset;
				sscanf_s ((const char *)(argv[1]),"%llX",&DumpOffset);
#endif
				RETBUFFER DumpFile;
				ZeroMemory (&DumpFile,sizeof(RETBUFFER));

				void * MyDumpBuffer = VirtualAlloc (NULL,DumpSize,MEM_COMMIT,PAGE_READWRITE);

				if (MyDumpBuffer != NULL)
				{
					if (ReadMem (DumpOffset,DumpSize,MyDumpBuffer,&DebugDataExchange->ProcessInfo) == true)
					{
						DumpFile.FileSize = DumpSize;
						DumpFile.FileOffset = (BYTE *)MyDumpBuffer;
						if (WriteFileMem (argv[3],&DumpFile) == 0) {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Dump successful");
						} else {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: WriteFileMem failed");
						}
					} else {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: readprocessmemory failed");
					}
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: memory allocation failed");
				}
				VirtualFree (MyDumpBuffer,DumpSize,MEM_DECOMMIT);
				free(linecopy);
				return true;
			}
			if (argc > 4) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}


		// *****************************************************************
		//  DUMPPE
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"dumppe") == 0) {
			if (argc == 2) {
				ULONG DumpSize = DebugDataExchange->ImageSize;
#ifdef b32BitBuild
				ULONG DumpOffset = DebugDataExchange->ImageBase;
#else
				ULONGLONG DumpOffset = DebugDataExchange->ImageBase;
#endif
				RETBUFFER DumpFile;
				ZeroMemory (&DumpFile,sizeof(RETBUFFER));

				void * MyDumpBuffer = VirtualAlloc (NULL,DumpSize,MEM_COMMIT,PAGE_READWRITE);
				if (MyDumpBuffer != NULL)
				{
					if (ReadMem (DumpOffset,DumpSize,MyDumpBuffer,&DebugDataExchange->ProcessInfo) == true)
					{
						DumpFile.FileSize = DumpSize;
						DumpFile.FileOffset = (BYTE *)MyDumpBuffer;
						// DUMPFIX IF SET
						if (GetOption("dumpfix") == true) DumpFix (DumpFile.FileOffset,DebugDataExchange);
						if (WriteFileMem (argv[1],&DumpFile) == 0) {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Dump successful");
						} else {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: WriteFileMem failed");
						}
					} else {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: readprocessmemory failed");
					}
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: memory allocation failed");
				}
				VirtualFree (MyDumpBuffer,DumpSize,MEM_DECOMMIT);
				free(linecopy);
				return true;
			}
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);		
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  WHAT
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"what") == 0) {
			if (argc == 2) {
				char * bRes = PerformVALookup (Evaluate(argv[1],DebugDataExchange),DebugDataExchange);
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: %s = %s",argv[0],argv[1],bRes == false ? "unidentified" : bRes);		
				free(linecopy);
				return true;
			}
			if (argc > 2) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  INJECT
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"inject") == 0) {
			if (argc >= 2) {
				RETBUFFER injectee;
				memset (&injectee,0,sizeof(RETBUFFER));
				if (ReadFileMem (argv[argc-1],&injectee) == TRUE) {
					size_t Offset = 0;
					if (argc == 3) {
						Offset = Evaluate (argv[1],DebugDataExchange);
					}

					LPVOID Memory = VirtualAllocEx (DebugDataExchange->ProcessInfo.hProcess,(LPVOID)Offset,injectee.FileSize,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
					if (Memory != NULL) {
#ifdef b32BitBuild
						if (WriteMem ((ULONG)Memory,injectee.FileSize,injectee.FileOffset,&DebugDataExchange->ProcessInfo) == TRUE) {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: injected file at: %8.8x",argv[0],Memory);
#else
						if (WriteMem ((ULONGLONG)Memory,injectee.FileSize,injectee.FileOffset,&DebugDataExchange->ProcessInfo) == TRUE) {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: injected file at: %16.16llx",argv[0],Memory);
#endif
						} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to write to process memory",argv[0]);
					} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to allocate remote memory",argv[0]);
				} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to read input file",argv[0]);
				GlobalFree (injectee.FileOffset);
				free(linecopy);
				return true;
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  ZAP
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"zap") == 0)
		{
			if (argc == 1) {
				unsigned char TmpOpcBuf[2];
				ZeroMemory (&TmpOpcBuf,2);
				CONTEXT CTX;
				if (ReadContext (&CTX,DebugDataExchange) == true) {
#ifdef b32BitBuild
					if (ReadMem (CTX.Eip-2,2,&TmpOpcBuf,&DebugDataExchange->ProcessInfo) == true)	{
#else
					if (ReadMem (CTX.Rip-2,2,&TmpOpcBuf,&DebugDataExchange->ProcessInfo) == true)	{
#endif
						if (((TmpOpcBuf[0] == 0xCD) && ((TmpOpcBuf[1] == 0x03) || (TmpOpcBuf[1] == 0x01))) || (TmpOpcBuf[1] == 0xCC)) {
							if (TmpOpcBuf[1] != 0xCC) TmpOpcBuf[0] = 0x90;
							TmpOpcBuf[1] = 0x90;
#ifdef b32BitBuild
							if (WriteMem (CTX.Eip-2,2,&TmpOpcBuf,&DebugDataExchange->ProcessInfo) == false) {
#else
							if (WriteMem (CTX.Rip-2,2,&TmpOpcBuf,&DebugDataExchange->ProcessInfo) == false) {
#endif
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: could not zap int 1/3 out",argv[0]);
								free(linecopy);
								return false;
							} else {
								free(linecopy);
								return true;
							}
						} else {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: only possible after an int 1/3",argv[0]);		
							free(linecopy);
							return false;
						}
					} else {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to read memory (%s)",argv[0]);
						free(linecopy);
						return false;
					}
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to read context (%s)",argv[0]);
					free(linecopy);
					return false;
				}
			}
			if (argc > 1) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);		
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;			
		}

		// *****************************************************************
		//  R
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"r") == 0) {
			if (argc == 3) {
				if (_stricmp(argv[1],"fl") == 0) {
					CONTEXT CTX;
					if (ReadContext (&CTX,DebugDataExchange) == true) {
						int Flag = 0;
						if (_stricmp(argv[2],"z") == 0) Flag = ZERO;
						if (_stricmp(argv[2],"d") == 0) Flag = DIRECTION;
						if (_stricmp(argv[2],"i") == 0) Flag = INTERRUPT;
						if (_stricmp(argv[2],"s") == 0) Flag = SIGN;
						if (_stricmp(argv[2],"a") == 0) Flag = ADJUST;
						if (_stricmp(argv[2],"p") == 0) Flag = PARITY;
						if (_stricmp(argv[2],"c") == 0) Flag = CARRY;
						if (_stricmp(argv[2],"o") == 0) Flag = OVERFLOW;

						if (bit_tst (&CTX.EFlags,Flag) == 0) {
							bit_set (&CTX.EFlags,Flag);
						} else {
							bit_clr (&CTX.EFlags,Flag);
						}

						if (WriteContext (&CTX,DebugDataExchange) == true) {
							SDL_Event event;
							SDL_UserEvent userevent;
							userevent.type = SDL_USEREVENT;
							userevent.code = 0;
							event.type = SDL_USEREVENT;
							event.user = userevent;
							SDL_PushEvent (&event);

							free(linecopy);
							return true;
						} else {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to write context (%s)",argv[0]);
						}
					} else {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to read context (%s)",argv[0]);
					}
					free(linecopy);
					return false;			
				}
				for (int i = 0; i < _countof(Registers); i++) {
					if (_stricmp (argv[1],Registers[i]) == NULL) {
						CONTEXT CTX;
						if (ReadContext (&CTX,DebugDataExchange) == true) {
#ifdef b32BitBuild
							ULONG RegValue[] = {CTX.Eax,CTX.Ebx,CTX.Ecx,CTX.Edx,CTX.Esi,CTX.Edi,CTX.Ebp,CTX.Esp,CTX.Eip,
												CTX.Eax,CTX.Ebx,CTX.Ecx,CTX.Edx,CTX.Esi,CTX.Edi,CTX.Ebp,CTX.Esp,CTX.Eip,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,
												CTX.Eax,CTX.Eax,CTX.Eax,CTX.Ebx,CTX.Ebx,CTX.Ebx,CTX.Ecx,CTX.Ecx,CTX.Ecx,CTX.Edx,CTX.Edx,CTX.Edx,CTX.Esi,CTX.Edi,CTX.Ebp,CTX.Esp};
#else
							ULONGLONG RegValue[] = {CTX.Rax,CTX.Rbx,CTX.Rcx,CTX.Rdx,CTX.Rsi,CTX.Rdi,CTX.Rbp,CTX.Rsp,CTX.Rip,
													CTX.Rax,CTX.Rbx,CTX.Rcx,CTX.Rdx,CTX.Rsi,CTX.Rdi,CTX.Rbp,CTX.Rsp,CTX.Rip,CTX.R8,CTX.R9,CTX.R10,CTX.R11,CTX.R12,CTX.R13,CTX.R14,CTX.R15,
													CTX.Rax,CTX.Rax,CTX.Rax,CTX.Rbx,CTX.Rbx,CTX.Rbx,CTX.Rcx,CTX.Rcx,CTX.Rcx,CTX.Rdx,CTX.Rdx,CTX.Rdx,CTX.Rsi,CTX.Rdi,CTX.Rbp,CTX.Rsp};
#endif


							RegValue[i] = Evaluate (argv[2],DebugDataExchange);
#ifdef b32BitBuild
							CTX.Eax = RegValue[0];
							CTX.Ebx = RegValue[1];
							CTX.Ecx = RegValue[2];
							CTX.Edx = RegValue[3];
							CTX.Esi = RegValue[4];
							CTX.Edi = RegValue[5];
							CTX.Ebp = RegValue[6];
							CTX.Esp = RegValue[7];
							CTX.Eip = RegValue[8];
							if (i == 26) CTX.Eax = (CTX.Eax & 0xFFFFFF00) + (RegValue[i] & 0xFF); // AL
							if (i == 27) CTX.Eax = (CTX.Eax & 0xFFFF00FF) + ((RegValue[i] << 8) & 0xFF00); // AH
							if (i == 28) CTX.Eax = (CTX.Eax & 0xFFFF0000) + (RegValue[i] & 0xFFFF); // AX
							if (i == 29) CTX.Ebx = (CTX.Ebx & 0xFFFFFF00) + (RegValue[i] & 0xFF); // BL
							if (i == 30) CTX.Ebx = (CTX.Ebx & 0xFFFF00FF) + ((RegValue[i] << 8) & 0xFF00); // BH
							if (i == 31) CTX.Ebx = (CTX.Ebx & 0xFFFF0000) + (RegValue[i] & 0xFFFF); // BX
							if (i == 32) CTX.Ecx = (CTX.Ecx & 0xFFFFFF00) + (RegValue[i] & 0xFF); // CL
							if (i == 33) CTX.Ecx = (CTX.Ecx & 0xFFFF00FF) + ((RegValue[i] << 8) & 0xFF00); // CH
							if (i == 34) CTX.Ecx = (CTX.Ecx & 0xFFFF0000) + (RegValue[i] & 0xFFFF); // CX
							if (i == 35) CTX.Edx = (CTX.Edx & 0xFFFFFF00) + (RegValue[i] & 0xFF); // DL
							if (i == 36) CTX.Edx = (CTX.Edx & 0xFFFF00FF) + ((RegValue[i] << 8) & 0xFF00); // DH
							if (i == 37) CTX.Edx = (CTX.Edx & 0xFFFF0000) + (RegValue[i] & 0xFFFF); // DX
							if (i == 38) CTX.Esi = (CTX.Esi & 0xFFFF0000) + (RegValue[i] & 0xFFFF); // SI
							if (i == 39) CTX.Edi = (CTX.Edi & 0xFFFF0000) + (RegValue[i] & 0xFFFF); // DI
							if (i == 40) CTX.Ebp = (CTX.Ebp & 0xFFFF0000) + (RegValue[i] & 0xFFFF); // BP
							if (i == 41) CTX.Esp = (CTX.Esp & 0xFFFF0000) + (RegValue[i] & 0xFFFF); // SP
#else
							CTX.Rax = RegValue[9];
							CTX.Rbx = RegValue[10];
							CTX.Rcx = RegValue[11];
							CTX.Rdx = RegValue[12];
							CTX.Rsi = RegValue[13];
							CTX.Rdi = RegValue[14];
							CTX.Rbp = RegValue[15];
							CTX.Rsp = RegValue[16];
							CTX.Rip = RegValue[17];
							CTX.R8 = RegValue[18];
							CTX.R9 = RegValue[19];
							CTX.R10 = RegValue[20];
							CTX.R11 = RegValue[21];
							CTX.R12 = RegValue[22];
							CTX.R13 = RegValue[23];
							CTX.R14 = RegValue[24];
							CTX.R15 = RegValue[25];

							if (i == 26) CTX.Rax = (CTX.Rax & 0xFFFFFFFFFFFFFF00) + (RegValue[i] & 0xFF); // AL
							if (i == 27) CTX.Rax = (CTX.Rax & 0xFFFFFFFFFFFF00FF) + ((RegValue[i] << 8) & 0xFF00); // AH
							if (i == 28) CTX.Rax = (CTX.Rax & 0xFFFFFFFFFFFF0000) + (RegValue[i] & 0xFFFF); // AX
							if (i == 29) CTX.Rbx = (CTX.Rbx & 0xFFFFFFFFFFFFFF00) + (RegValue[i] & 0xFF); // BL
							if (i == 30) CTX.Rbx = (CTX.Rbx & 0xFFFFFFFFFFFF00FF) + ((RegValue[i] << 8) & 0xFF00); // BH
							if (i == 31) CTX.Rbx = (CTX.Rbx & 0xFFFFFFFFFFFF0000) + (RegValue[i] & 0xFFFF); // BX
							if (i == 32) CTX.Rcx = (CTX.Rcx & 0xFFFFFFFFFFFFFF00) + (RegValue[i] & 0xFF); // CL
							if (i == 33) CTX.Rcx = (CTX.Rcx & 0xFFFFFFFFFFFF00FF) + ((RegValue[i] << 8) & 0xFF00); // CH
							if (i == 34) CTX.Rcx = (CTX.Rcx & 0xFFFFFFFFFFFF0000) + (RegValue[i] & 0xFFFF); // CX
							if (i == 35) CTX.Rdx = (CTX.Rdx & 0xFFFFFFFFFFFFFF00) + (RegValue[i] & 0xFF); // DL
							if (i == 36) CTX.Rdx = (CTX.Rdx & 0xFFFFFFFFFFFF00FF) + ((RegValue[i] << 8) & 0xFF00); // DH
							if (i == 37) CTX.Rdx = (CTX.Rdx & 0xFFFFFFFFFFFF0000) + (RegValue[i] & 0xFFFF); // DX
							if (i == 38) CTX.Rsi = (CTX.Rsi & 0xFFFFFFFFFFFF0000) + (RegValue[i] & 0xFFFF); // SI
							if (i == 39) CTX.Rdi = (CTX.Rdi & 0xFFFFFFFFFFFF0000) + (RegValue[i] & 0xFFFF); // DI
							if (i == 40) CTX.Rbp = (CTX.Rbp & 0xFFFFFFFFFFFF0000) + (RegValue[i] & 0xFFFF); // BP
							if (i == 41) CTX.Rsp = (CTX.Rsp & 0xFFFFFFFFFFFF0000) + (RegValue[i] & 0xFFFF); // SP
#endif
							if (WriteContext(&CTX,DebugDataExchange) == true) {

								SDL_Event event;
								SDL_UserEvent userevent;
								userevent.type = SDL_USEREVENT;
								userevent.code = 0;
								event.type = SDL_USEREVENT;
								event.user = userevent;
								SDL_PushEvent (&event);

								free(linecopy);
								return true;
							} else {
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to write context (%s)",argv[0]);
							}
						} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: failed to read context (%s)",argv[0]);
						free(linecopy);
						return false;
						break;
					}
				}
				free(linecopy);
				return false;
			}
			if (argc > 3) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  LOADPDB
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"loadpdb") == 0) {

			if (DebugDataExchange->PDBLoaded == true) {
				CleanupDia ();
				DebugDataExchange->PDBLoaded = false;
			}

			char FileNameShortened[MAX_PATH+1] = "";
			wchar_t FileNameUnicode[MAX_PATH+1];
			size_t CharsConverted;

			mbstowcs_s (&CharsConverted,FileNameUnicode,_countof (FileNameUnicode),DebugDataExchange->FileName,MAX_PATH);
			PathCompactPathEx (FileNameShortened,DebugDataExchange->FileName, 30, NULL);

			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"trying to load pdb for %s",FileNameShortened);
			bool bRes = OpenPdb(FileNameUnicode);
			if (bRes == true) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"pdb loaded successfully");
				DebugDataExchange->PDBLoaded = true;
			} else {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"failed to load pdb");
			}
			free(linecopy);
			return true;
		}
		// *****************************************************************
		//  UNLOADPDB
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"unloadpdb") == 0) {
			if (DebugDataExchange->PDBLoaded == true) {
				CleanupDia ();
				DebugDataExchange->PDBLoaded = false;
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"unloaded pdb successfully");
			} else {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"pdb was not loaded");
				DisplaySyntax (argv[0],DebugDataExchange);
			}
			free(linecopy);
			return true;
		}

		// *****************************************************************
		//  CODE
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"code") == 0) {
			if (argc == 1) {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"code is %s",instructionHex == false ? "off" : "on");
			} else {
				if (argc == 2) {
					if (_stricmp (argv[1],"on") == NULL) {
						instructionHex = true;
					}
					if (_stricmp (argv[1],"off") == NULL) {
						instructionHex = false;
					}
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"code is %s",instructionHex == false ? "off" : "on");
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);
				}
			}
			free(linecopy);
			return true;
		}

		// *****************************************************************
		//  A (assemble)
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"a") == 0) {
			if (argc > 2) {
#ifdef b32BitBuild				
				ULONG Offset;
				ULONG AlignedOrg;
#else
				ULONGLONG Offset;
				ULONGLONG AlignedOrg;
#endif
				Offset = Evaluate (argv[1],DebugDataExchange);

				char Final[MAX_PATH] = "";
				for (int j = 2; j < argc; j++) {
					strcat_s (Final,_countof(Final),argv[j]);
					strcat_s (Final,_countof(Final)," ");
				}
				// BUILD IN / OUT FILES
				char InputFile [MAX_PATH] = "";
				char OutputFile [MAX_PATH] = "";
				char ErrorMsg [MAX_PATH] = "";
				if (GetTempPath (_countof(InputFile),InputFile) != NULL) {
					strcpy_s (OutputFile,_countof(OutputFile),InputFile);
					strcpy_s (ErrorMsg,_countof(ErrorMsg),InputFile);
					if (GetTempFileName (InputFile,"asm",NULL,InputFile) != NULL) {
						if (GetTempFileName (OutputFile,"asm",NULL,OutputFile) != NULL) {
							GetTempFileName (ErrorMsg,"asm",NULL,ErrorMsg);
							// CALCULATE ORG
							AlignedOrg = AlignOffset (Offset, sizeof (ULONG));;
							if (AlignedOrg != Offset) {
								if (AlignedOrg > Offset) AlignedOrg -= sizeof (ULONG);
							}
							FILE *file;
							ULONG bRes = 0;
							fopen_s (&file,InputFile, "w");
							if (file != NULL) {
#ifdef b32BitBuild
								fprintf (file,"BITS 32\n");
								fprintf (file,"ORG 0%xh\n",AlignedOrg);
#else
								fprintf (file,"BITS 64\n");
								fprintf (file,"ORG 0%llxh\n",AlignedOrg);
#endif
								fprintf (file,"section .text\n");
								for (unsigned int j = 0; j < Offset-AlignedOrg; j++)	{
									fprintf (file,"nop\n");
								}
								fprintf (file,"%s\n",Final);
								fclose (file);
								
								// start yasm now
								char HomePath[MAX_PATH] = "";
								if (GetHomePath (HomePath,_countof(HomePath)) == true) {
									strcat_s (HomePath,_countof(HomePath),"yasm.exe");
									STARTUPINFO Startup;
									Startup.cb = sizeof (Startup);
									PROCESS_INFORMATION ProcessInfo;
									char CmdLine[MAX_PATH] = "";
									sprintf_s (CmdLine,_countof(CmdLine),"%s -E %s -o %s %s",HomePath,ErrorMsg,OutputFile,InputFile);
									if (CreateProcess (HomePath,CmdLine,NULL,NULL,false,NULL,NULL,NULL,&Startup,&ProcessInfo) != NULL) {
										WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
										ULONG ExitCode;
										if (GetExitCodeProcess (ProcessInfo.hProcess, &ExitCode) != NULL) {
											if (ExitCode == NULL) {
												RETBUFFER Binary;
												memset (&Binary,0,sizeof(Binary));
												if (ReadFileMem (OutputFile,&Binary) == TRUE) {
													size_t sub = Offset-AlignedOrg;
													if (WriteMem (Offset,Binary.FileSize-(ULONG)sub,&Binary.FileOffset[Offset-AlignedOrg],&DebugDataExchange->ProcessInfo) == true) {
													} else {
														*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to writemem (binary)",argv[0]);		
													}
													GlobalFree (Binary.FileOffset);
												} else {
													*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to open binary stream",argv[0]);		
												}
											} else {
												FILE *file;
												ULONG bRes = 0;
												fopen_s (&file,ErrorMsg, "r");
												if (file != NULL) {
													static char line[8192];
													while (fgets (line, sizeof line, file ) != NULL ) {
														stringReplace (InputFile,"",line);
														for (unsigned int j = 0; j < strlen ((const char *)line); j++) {
															if (line[j] == '\n') line[j] = 0;
														}
														*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"YASM: %s",line);		
													}
													fclose (file);
												}
											}
										} else {
											*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: getexitcode failed",argv[0]);		
										}
										CloseHandle (ProcessInfo.hProcess);
										CloseHandle (ProcessInfo.hThread);
									} else {
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to start yasm",argv[0]);		
										*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"Please copy yasm.exe to the ugdbg directory in case you havent done so yet",argv[0]);		
									}
								} else {
									*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to get home path",argv[0]);		
								}
							} else {
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to open input file",argv[0]);		
							}
						} else {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: GetTempFileName failed",argv[0]);		
						}
					} else {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: GetTempFileName failed",argv[0]);		
					}
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: GetTempPath failed",argv[0]);		
				}

				DeleteFile (InputFile);
				DeleteFile (OutputFile);
				DeleteFile (ErrorMsg);

				free (linecopy);
				return true;
			}
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  BREAK
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"break") == 0) {
			DebugBreakProcess (DebugDataExchange->ProcessInfo.hProcess);
			free (linecopy);
			return true;
		}

		// *****************************************************************
		//  MAP32
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"map32") == 0) {
			if (argc == 1) {
				char LocalBuff[0x1000];
				if (ReadMem (DebugDataExchange->ImageBase,0x1000,&LocalBuff,&DebugDataExchange->ProcessInfo) == true) {
					PIMAGE_DOS_HEADER DosHdr = (PIMAGE_DOS_HEADER) (BYTE*)LocalBuff;
					PIMAGE_NT_HEADERS NtHdr = (PIMAGE_NT_HEADERS) (BYTE*)(LocalBuff+DosHdr->e_lfanew);
					PIMAGE_SECTION_HEADER SecHdr = (PIMAGE_SECTION_HEADER) (BYTE*)(LocalBuff+DosHdr->e_lfanew+sizeof(IMAGE_NT_HEADERS));
					if ((DosHdr != NULL) && (DosHdr->e_magic == IMAGE_DOS_SIGNATURE)) {
						if (NtHdr->Signature == IMAGE_NT_SIGNATURE) {
							for (int i = 0; i < NtHdr->FileHeader.NumberOfSections; i ++) {
								SecHdr->SizeOfRawData = SecHdr->Misc.VirtualSize;
								SecHdr->PointerToRawData = SecHdr->VirtualAddress;
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"%-8s ROfs: %8.8x VOfs: %8.8x RSize: %8.8x VSize: %8.8x",SecHdr->Name,SecHdr->PointerToRawData,SecHdr->VirtualAddress,SecHdr->SizeOfRawData,SecHdr->Misc.VirtualSize);
								SecHdr++;
							}
						} else {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: NT Signature not present...");
						}
					} else {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: DOS Signature not present...");
					}
				}
			} else {
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too many arguments specified",argv[0]);		
				DisplaySyntax (argv[0],DebugDataExchange);
			}
			free (linecopy);
			return true;
		}


		// *****************************************************************
		//  POKE
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"poke") == 0) {
			if (argc > 2) {
#ifdef b32BitBuild
				ULONG Offset;
#else
				ULONGLONG Offset;
#endif
				Offset = Evaluate (argv[1],DebugDataExchange);
				for (int h = 2; h < argc; h++) {
					char * Tmp = argv[h];
					if (Tmp[0] == 'T') {
						if (WriteMem (Offset,(ULONG)strlen (argv[h])-1,&Tmp[1],&DebugDataExchange->ProcessInfo) == false) {
							*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to write to memory",argv[0]);		
						}
						Offset += strlen (argv[h])-1;
					} else {
						if (strlen (argv[h]) < 3) {
							char Value[2];
							sscanf_s (argv[h],"%x",&Value,_countof(Value));
							if (WriteMem (Offset,1,&Value,&DebugDataExchange->ProcessInfo) == false) {
								*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: failed to write to memory",argv[0]);		
							}
							Offset ++;
						}
					}
				}
				free(linecopy);
				return true;
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}

		// *****************************************************************
		//  SEARCH
		// 
		// *****************************************************************
		if (_stricmp (argv[0],"s") == 0) {
			if (argc > 3) {
#ifdef b32BitBuild
				ULONG OffsetLow;
#else
				ULONGLONG OffsetLow;
#endif
				OffsetLow = Evaluate (argv[1],DebugDataExchange);
				size_t Size = Evaluate (argv[2],DebugDataExchange);

				SYSTEM_INFO SysInfo;
				GetSystemInfo (&SysInfo);

				char PageBuffer[0x2000];
				if (SysInfo.dwPageSize <= 0x1000) {
					while (Size > 0) {
						if (ReadMem (OffsetLow,SysInfo.dwPageSize,PageBuffer,&DebugDataExchange->ProcessInfo) == true) {
							for (unsigned int j = 0; j < SysInfo.dwPageSize; j++) {
								bool HitFound = true;
								size_t Ctr = 0;
								for (int h = 3; h < argc; h++) {
									char * Tmp = argv[h];
									if (Tmp[0] == 'T') {
										if (memcmp (&PageBuffer[j+Ctr], &Tmp[1], strlen (argv[h])-1) != NULL) {
											HitFound = false;
											Ctr = 0;
										} else Ctr += strlen (argv[h])-1;
									} else {
										if (strlen (argv[h]) < 3) {
											char Value[2];
											sscanf_s (argv[h],"%x",&Value,_countof(Value));
											if (PageBuffer[j+Ctr] != Value[0]) {
												HitFound = false;
												Ctr = 0;
											} else Ctr++;
										}
									}
								}
								if (HitFound == true) {
#ifdef b32BitBuild
									*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: match - %x",argv[0],OffsetLow+j);		
#else
									*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: match - %llx",argv[0],OffsetLow+j);		
#endif
									free (linecopy);
									return true;
								}
							}
						}
						OffsetLow += SysInfo.dwPageSize;
						Size -= SysInfo.dwPageSize;
					}
				} else {
					*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: PAGE_SIZE is too large (%x)",argv[0],SysInfo.dwPageSize);		
				}
			} else *DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %s: too less arguments specified",argv[0]);		
			DisplaySyntax (argv[0],DebugDataExchange);
			free(linecopy);
			return false;
		}
	}
	*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"Invalid command");		
	free(linecopy);
	return false;
}