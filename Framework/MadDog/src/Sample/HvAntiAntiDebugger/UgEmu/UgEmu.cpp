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

// UgEmu.cpp
//
// v0.1 - initial code
//
// overview: load font
//			 init front/backsurface
//			 load target
//			 enter main loop
//
// todo list: 
//            stealth
//			  step over 80%
//			  tracex
//			  breakpoint table load/save
//			  normalize PE
//			  fix tbs int1 handling - most likely due to missing pushf(d) emulation
//			  fix the memory leaks
//			  fix ctrl+r repeat
//			  cleanup ugemu.cpp will be absolutely necessary!
//#define RELEASE
#include "stdafx.h"
#include "helper.h"
#include "ugemu.h"

bool GoActive = false;
bool Press = false;
// next is used for instruction's offset synchronization.
// Decoded instruction information.
_DecodedInst decodedInstructions[MAX_INSTRUCTIONS];

void SetCurrentWidth (int Size) {
	WindowSizeX = Size;
	return;
}

void SetCurrentHeight (int Size) {
	WindowSizeY = Size;
	return;
}

void SetLogYPos (int Size) {
	LOGTEXT_Y_POS = Size;
	return;
}
void SetDataYPos (int Size) {
	DATA_Y_POS = Size;
	return;
}
void SetDisassemblyYPos (int Size) {
	DISASSEMBLY_Y_POS = Size;
	return;
}
int GetCurrentWidth () {
	return WindowSizeX;
}

int GetCurrentHeight () {
	return WindowSizeY;
}

int GetDataYPos () {
	return DATA_Y_POS;
}

int GetLogYPos () {
	return LOGTEXT_Y_POS;
}

int GetDisassemblyYPos () {
	return DISASSEMBLY_Y_POS;
}

int GetInputYPos () {
	return INPUTWINDOW_Y_POS;
}

_DecodeType GetDt () {
	return dt;
}

void SetVideoMode (Messages * Queue) {
	SDL_Surface * screen_tmp = SDL_SetVideoMode (WindowSizeX, WindowSizeY, 32, SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_NOFRAME);
	if (screen_tmp == NULL) {
		ShowLatestMessages = DisplayMessage (Queue,"UGDBG: Failed to set Video mode");
	} else {
		SDL_FreeSurface (screen);
		screen = screen_tmp;
		SDL_FreeSurface (backbuffer);
		backbuffer = SDL_CreateRGBSurface (screen->flags,screen->w,screen->h,32,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
		SDL_DisplayFormat (backbuffer);
		INPUTWINDOW_Y_POS = screen->h-24;
		ShowLatestMessages = DisplayMessage (Queue,"UGDBG: Videomode changed X: %d Y: %d",WindowSizeX,WindowSizeY);
	}
}

//***************************************************
// Auto Tracer
//***************************************************
SDL_Thread * TracerThreadVar = 0;
int TracerThread (void *p)
{
	int Counter = 20;
	for (;;) {
		while (DebugDataExchange.PollEvent == false) Sleep (1);

		CONTEXT CTX;
		BYTE CheckByte = 0;

		if (Counter == 20) {
			if (ReadContext (&CTX,&DebugDataExchange) == true) {
#ifdef b32BitBuild
				if (ReadMem (CTX.Eip,1,&CheckByte,&DebugDataExchange.ProcessInfo) == true) {
#else
				if (ReadMem (CTX.Rip,1,&CheckByte,&DebugDataExchange.ProcessInfo) == true) {
#endif
					if (CheckByte == 0xF3) {
						// STEP OVER
						StepOver (&DebugDataExchange);
						// Continue
						DebugDataExchange.AllowInt1 = true;
						DebugDataExchange.AllowScrolling = false;
						DebugDataExchange.PollEvent = false;
					} else {
						// Enable Trap Flag
						EnableTrapflag (&DebugDataExchange);
						// Continue
						DebugDataExchange.AllowScrolling = false;
						DebugDataExchange.AllowInt1 = true;
						DebugDataExchange.PollEvent = false;
					}
				}
			}
		} else
		{
			// Enable Trap Flag
			EnableTrapflag (&DebugDataExchange);
			// Continue
			DebugDataExchange.AllowScrolling = false;
			DebugDataExchange.AllowInt1 = true;
			DebugDataExchange.PollEvent = false;
		}

		if (Counter > 0) Counter--;
		if (Counter == 0) {
			DebugDataExchange.RunTraceActive = false;
			Counter = 20;
		} else DebugDataExchange.RunTraceActive = true;
	}
	return true;
}




int _tmain(int argc, _TCHAR* argv[])
{
	// Init Debugger Console
	printf ("UGDBG: UgDbg v0.1 pre-alpha Build "__DATE__" "__TIME__"\n");

	memset (&InFile,0,sizeof(RETBUFFER));
	HRSRC Font = FindResource (GetModuleHandle(NULL),MAKEINTRESOURCE (101),RT_RCDATA);
	if (Font != NULL) {
		InFile.FileOffset = (BYTE *)LoadResource (GetModuleHandle(NULL),Font);
		InFile.FileSize = SizeofResource (GetModuleHandle(NULL),Font);
	}

	// Setup Cursor blinking speed according to the CPU power
	int ProcSpeed = ProcSpeedRead();
	if (ProcSpeed != NULL) {
		if (ProcSpeed < 2800) {
			Cursor_Interval = 500;
			if (ProcSpeed < 1500) {
				Cursor_Interval = 1000;
			}
		} else {
			Cursor_Interval = 200;
		}
	} else {
		// in case we can't read the processor speed this is the default value
		Cursor_Interval = 500;
	}

	ReLoadConfig ("sample.cfg");

	// HANDLE CONTEXT LOADING
	if (argc > 1) {
		char UserFile[MAX_PATH] = "";
		strcpy_s (UserFile,_countof(UserFile),argv[1]);
		while (stringReplace("\\","/",UserFile));
		char * cpy = _strdup (UserFile);
		sprintf_s (UserFile,_countof(UserFile)," \"debug %s\"",cpy);
		free (cpy);
		strcat_s (AutoExec.AutoExecString,_countof(AutoExec.AutoExecString),UserFile);
	}


#ifdef b32BitBuild
	x64BitMode = false;
#else
	x64BitMode = true;
#endif

	if (x64BitMode == true) {
		WindowSizeX = 725;
		DATA_Y_POS += 2*LINE_H;
		LINE_UPPER += 2*LINE_H;
	}

	_putenv("SDL_VIDEO_WINDOW_POS=center");
	_putenv("SDL_VIDEO_CENTERED=1");
	_putenv("OANOCACHE=1");
	if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1) {
		printf ("UGDBG: SDL could not be initialized: %s\n", SDL_GetError ());
		exit (1);
    }

	atexit (SDL_Quit);
	screen = SDL_SetVideoMode (GetCurrentWidth (), GetCurrentHeight(), 32, SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_NOFRAME);
	if (screen == NULL) {
		printf ("UGDBG: Failed to set Video mode: %s\n", SDL_GetError());
		exit (1);
    }

	InitCharTable (&InFile,screen);

	// CREATE THE BLOCK CURSOR
    Uint8 mask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    Uint8 data[] = {0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
    SDL_Cursor* cursor = SDL_CreateCursor(data, mask, 16, 8, 8, 4);
    SDL_SetCursor (cursor);


	// WELL A LITTLE "HACK" FOR THE DYNAMIC VIDEO SETTING (CONFIGURATION FILE)
	INPUTWINDOW_Y_POS = screen->h-24;
	backbuffer = SDL_CreateRGBSurface (screen->flags,screen->w,screen->h,32,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
	SDL_DisplayFormat (backbuffer);

	Uint32 c_Black = SDL_MapRGB(backbuffer->format, 0, 0, 0); 
	Uint32 c_SiceBorder = SDL_MapRGB(backbuffer->format, 0, 128, 128); 
	Uint32 c_SiceGreen = SDL_MapRGB(backbuffer->format, 0, 128, 0); 


	const char OurTitle[64] = "[Debugger Build "__DATE__"-"__TIME__"]";
	char * Title;
	char * Icon;
	SDL_WM_GetCaption (&Title,&Icon);
	SDL_WM_SetCaption (OurTitle,Icon);

	// Disassembler version.
	// Version of used compiled library.
	unsigned long dver = distorm_version();
	printf("UGDBG: initializing message queue\n");
	Messages * Queue;
	Queue = InitQueue ();
	if (Queue == 0) {
		printf ("UGDBG: failed to initialize message queue\n");
		exit (0);
	}
	printf ("UGDBG: switching output to user interface\n");

	ShowLatestMessages = DisplayMessage (Queue,"UgDbg v0.1 Pre-Alpha build "__DATE__" "__TIME__" by Someone");
	ShowLatestMessages = DisplayMessage (Queue,"UGDBG: using YASM, SDL, SDL_Draw, Libconfig, Distorm64 (%d.%d.%d)", (dver >> 16), ((dver) >> 8) & 0xff, dver & 0xff);
	ShowLatestMessages = DisplayMessage (Queue,"UGDBG: CPU: %s",ProcessorInfo("ProcessorNameString"));
	ShowLatestMessages = DisplayMessage (Queue,"UGDBG: CPU: %s",ProcessorInfo("Identifier"));
	ShowLatestMessages = DisplayMessage (Queue,":");

	memset (&DebugDataExchange,0,sizeof(DebugStruc));

	DebugDataExchange.PollEvent = false;
	DebugDataExchange.Queue = Queue;
	DebugDataExchange.ShowLatestMessages = &ShowLatestMessages;
	DebugDataExchange.WindowSizeX = &WindowSizeX;
	DebugDataExchange.WindowSizeY = &WindowSizeY;
	DebugDataExchange.CTX = &CTX;
	ReadContext (&CTX,&DebugDataExchange);
	ZeroMemory (&PREVIOUS_CTX,sizeof (CONTEXT));

#ifdef RELEASE
	FreeConsole ();
#endif

#ifdef WIN32
	SDL_SysWMinfo i;
	SDL_VERSION(&i.version);
	if (SDL_GetWMInfo ( &i)) {
	  CenterWindow (i.window,WindowSizeX,WindowSizeY);
	}
#endif // WIN32

	BuildPluginApiTree (&DebugDataExchange);

	// ENTER LOOP
	memset (&CurrentPath,0,_countof(CurrentPath));

	SDL_EnableKeyRepeat (500,50);
	SDL_EnableUNICODE(1);
	SDL_Event ereignis;

	for (;;) {	
		SDL_WaitEvent (&ereignis);
		// THIS IS A VERY IMPORTANT CHECK TO AVOID REDRAWING IF THE MOUSE JUST MOVES OVER THE WINDOW
		// WITHOUT ANY ACTION TAKING PLACE
		if (ereignis.type == SDL_MOUSEMOTION) {
			if (ereignis.motion.state == NULL) {
				continue;
			}
		}

		// THIS IS NEEDED FOR LETTING "GO" WORK IN COMBINATION WITH BREAKPOINTS
		if (DebugDataExchange.BpmTrigger == true) GoActive = false;
		if (GoActive == true) {
			switch (BreakFound) {
				case true:
					EnableTrapflag (&DebugDataExchange);
					ReEnableBreakpoints (&DebugDataExchange.DbgEvent,&DebugDataExchange);
					// Continue
					DebugDataExchange.AllowScrolling = false;
					DebugDataExchange.AllowInt1 = true;
					DebugDataExchange.PollEvent = false;
					GoActive = false;
					Sleep (10);
					break;

				case false:
					// Enable Trap Flag
					DisableTrapflag (&DebugDataExchange);
					ReEnableBreakpoints (&DebugDataExchange.DbgEvent,&DebugDataExchange);
					// Continue
					DebugDataExchange.AllowScrolling = false;
					DebugDataExchange.AllowInt1 = false;
					DebugDataExchange.PollEvent = false;
					GoActive = false;
					Sleep (10);
					break;
			}
			continue;
		}
			//***************************************
			//
			// MOUSE HANDLER
			//
			//***************************************
			int MouseState, MousePosX, MousePosY;
			MouseState = SDL_GetMouseState(&MousePosX,&MousePosY);
			if (MouseState & SDL_BUTTON_LMASK) {
				if ((((MousePosY < DISASSEMBLY_Y_POS) && (MousePosY > DISASSEMBLY_Y_POS-10))) || (MouseButtonPressed == true)) {
					int movex, movey;
					movex = 0;
					movey = 0;
					SDL_GetRelativeMouseState (&movex,&movey);
					if ((movey <= 5) && (movey > 0)) {
						if (DISASSEMBLY_Y_POS < LOGTEXT_Y_POS-LINE_H) DISASSEMBLY_Y_POS += LINE_H;
						SDL_WarpMouse (MousePosX,DISASSEMBLY_Y_POS-5);
						MouseButtonPressed = true;
					} 
					if ((movey >= -5) && (movey <0)) {
						if (DISASSEMBLY_Y_POS > DATA_Y_POS+LINE_H) DISASSEMBLY_Y_POS -= LINE_H;
						SDL_WarpMouse (MousePosX,DISASSEMBLY_Y_POS-5);
						MouseButtonPressed = true;
					}
				}

				if ((((MousePosY < LOGTEXT_Y_POS) && (MousePosY > LOGTEXT_Y_POS-10))) || (MouseButtonPressedLog == true)) {
					int movex, movey;
					movex = 0;
					movey = 0;
					SDL_GetRelativeMouseState (&movex,&movey);
					if ((movey <= 5) && (movey > 0)) {
						if (LOGTEXT_Y_POS < INPUTWINDOW_Y_POS-LINE_H) LOGTEXT_Y_POS += LINE_H;
						SDL_WarpMouse (MousePosX,LOGTEXT_Y_POS-5);
						MouseButtonPressedLog = true;
					} else {
						if ((movey >= -5) && (movey <0)) {
							if (LOGTEXT_Y_POS > DISASSEMBLY_Y_POS+LINE_H) LOGTEXT_Y_POS -= LINE_H;
							SDL_WarpMouse (MousePosX,LOGTEXT_Y_POS-5);
							MouseButtonPressedLog = true;
						}
					}
				}

				//***************************************
				//
				// MOUSE HANDLER - SCROLL BY ARROWS (LOGWINDOW)
				//
				//***************************************
				int LineCounter = ((INPUTWINDOW_Y_POS - LOGTEXT_Y_POS)/LINE_H);
				int LineNum = ((MousePosY-LOGTEXT_Y_POS) / LINE_H);
				if ((LineNum >= 0) && (LineCounter > LineNum)) {
					if (MousePosX >= WindowSizeX-1-(RIGHT_MARGEIN * 2)) {
						if (LineCounter >= 2) {
							if (LineCounter >= 4) {
								if (LineNum == 0) {
									// -1 LINE
									ShowLatestMessages = false;
									if (ShowLogMessagesStartingByNumber > 1) ShowLogMessagesStartingByNumber--;
								}
								if (LineNum == 1) {
									// -1 PAGE
									ShowLatestMessages = false;
									if (((signed)ShowLogMessagesStartingByNumber-LineCounter) > 1) ShowLogMessagesStartingByNumber -= LineCounter; else ShowLogMessagesStartingByNumber = 1;
								}
								if (LineNum == LineCounter-1) {
									ShowLatestMessages = false;
									ShowLogMessagesStartingByNumber++;
									// +1 LINE
								}
								if (LineNum == LineCounter-2) {
									// +1 PAGE
									ShowLatestMessages = false;
									ShowLogMessagesStartingByNumber +=LineCounter;
								}
							} else {
								if (LineNum == 0) {
									// -1 LINE
									ShowLatestMessages = false;
									if (ShowLogMessagesStartingByNumber > 1) ShowLogMessagesStartingByNumber--;
								}
								if (LineNum == LineCounter-1) {
									ShowLatestMessages = false;
									ShowLogMessagesStartingByNumber++;
									// +1 LINE
								}
							}
						}
					}
				}

				//***************************************
				//
				// MOUSE HANDLER - SCROLL BY ARROWS (DISASSEMBLY)
				//
				//***************************************
				LineCounter = ((LOGTEXT_Y_POS - DISASSEMBLY_Y_POS)/LINE_H);
				LineNum = ((MousePosY-DISASSEMBLY_Y_POS) / LINE_H);
				if ((LineNum >= 0) && (LineCounter > LineNum)) {
					if (MousePosX >= WindowSizeX-1-(RIGHT_MARGEIN * 2)) {
						if (LineCounter >= 2) {
							if (LineCounter >= 4) {
								if (LineNum == 0) {
									// -1 LINE
									DebugDataExchange.DisassembleFromOffset --;
									DebugDataExchange.AllowScrolling = true;
								}
								if (LineNum == 1) {
									// -1 PAGE
									DebugDataExchange.DisassembleFromOffset -= 0x10;
									DebugDataExchange.AllowScrolling = true;
								}
								if (LineNum == LineCounter-1) {
									// Get size of current opcode
#ifdef b32BitBuild
									DebugDataExchange.DisassembleFromOffset += FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
#else
									DebugDataExchange.DisassembleFromOffset += (ULONGLONG)FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
#endif
									DebugDataExchange.AllowScrolling = true;
									// +1 LINE
								}
								if (LineNum == LineCounter-2) {
									// +1 PAGE
									for (int i = 0; i < LineCounter; i++) {
										// Get size of current opcode
#ifdef b32BitBuild
										DebugDataExchange.DisassembleFromOffset += FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
#else
										DebugDataExchange.DisassembleFromOffset += (ULONGLONG)FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
#endif
										DebugDataExchange.AllowScrolling = true;
									}
								}
							} else {
								if (LineNum == 0) {
									// -1 LINE
									DebugDataExchange.DisassembleFromOffset --;
									DebugDataExchange.AllowScrolling = true;
								}
								if (LineNum == LineCounter-1) {
									// Get size of current opcode
#ifdef b32BitBuild
									DebugDataExchange.DisassembleFromOffset += FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
#else
									DebugDataExchange.DisassembleFromOffset += (ULONGLONG)FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
#endif
									DebugDataExchange.AllowScrolling = true;
									// +1 LINE
								}
							}
						}
					}
				}

				//***************************************
				//
				// MOUSE HANDLER - SCROLL BY ARROWS (MEMORY WINDOW)
				//
				//***************************************
				LineCounter = ((DISASSEMBLY_Y_POS - DATA_Y_POS)/LINE_H);
				LineNum = ((MousePosY-DATA_Y_POS) / LINE_H);
				if ((LineNum >= 0) && (LineCounter > LineNum)) {
					if (MousePosX >= WindowSizeX-1-(RIGHT_MARGEIN * 2)) {
						if (LineCounter >= 2) {
							if (LineCounter >= 4) {
								if (LineNum == 0) {
									// -1 LINE
									DebugDataExchange.DisplayMemoryOffset -= 0x10;
								}
								if (LineNum == 1) {
									// -1 PAGE
									DebugDataExchange.DisplayMemoryOffset -= 0x10*LineCounter;
								}
								if (LineNum == LineCounter-1) {
									DebugDataExchange.DisplayMemoryOffset += 0x10;
									// +1 LINE
								}
								if (LineNum == LineCounter-2) {
									// +1 PAGE
									DebugDataExchange.DisplayMemoryOffset += 0x10*LineCounter;
								}
							} else {
								if (LineNum == 0) {
									// -1 LINE
									DebugDataExchange.DisplayMemoryOffset -= 0x10;
								}
								if (LineNum == LineCounter-1) {
									DebugDataExchange.DisplayMemoryOffset += 0x10;
									// +1 LINE
								}
							}
						}
					}
				}


			} else {
				MouseButtonPressed = false;
				MouseButtonPressedLog = false;
			}

			//***************************************
			//
			// MOUSE HANDLER - COPY TO CLIPBOARD
			//
			//***************************************
			if ((MouseState & SDL_BUTTON_LMASK) && (MousePosY > GetLogYPos()) && (MousePosY < GetInputYPos ()) && (IsDoubleClick () == TRUE) && (Press == false)
				&& (MousePosX < GetCurrentWidth()-RIGHT_MARGEIN*2) && (Cursor_Region == 0)) {
				ShowLatestMessages = DisplayMessage (Queue,"UGDBG: Log %scopied to clipboard",CopyQueueToClipboard (Queue) == true ? "": "not ");		
				Press = true;
			}
			//***************************************
			//
			// MOUSE HANDLER - SWITCH CURSOR WINDOWS
			//
			//***************************************
			if ((MouseState & SDL_BUTTON_LMASK) && (MousePosY > GetLogYPos()) && (MousePosY < GetInputYPos ())) {
				Cursor_Region = 0;
			}
			if ((MouseState & SDL_BUTTON_LMASK) && (MousePosY > GetDataYPos()) && (MousePosY < GetDisassemblyYPos() )) {
				Cursor_Region = 1;
			}

			//***************************************
			//
			// MOUSE HANDLER - BPX
			//
			//***************************************
			if ((MouseState & SDL_BUTTON_LMASK) && (Press == false)) {
				if ((MousePosY >= DISASSEMBLY_Y_POS) && (MousePosY <= LOGTEXT_Y_POS) && (IsDoubleClick() == TRUE) && (MousePosX < WindowSizeX-1-(2*RIGHT_MARGEIN))) {
					int OpcodeCounter = ((MousePosY-DISASSEMBLY_Y_POS) / LINE_H);
#ifdef b32BitBuild
					ULONG OpcodePtr = DebugDataExchange.DisassembleFromOffset;
#else
					ULONGLONG OpcodePtr = DebugDataExchange.DisassembleFromOffset;
#endif
					unsigned char InstructionBuffer[MAX_INSTRUCTIONS*16] = "";

					// Holds the result of the decoding.
					_DecodeResult res;
					// decodedInstructionsCount holds the count of filled instructions' array by the decoder.
					unsigned int decodedInstructionsCount = 0;
					// Default offset for buffer is 0, could be set in command line.
					_OffsetType offset = 0;

					for (int i = 0;i < OpcodeCounter; i ++) {
						int OpcLen = (int)FetchOpcodeSize (OpcodePtr,&DebugDataExchange);
						ReadMem (OpcodePtr,OpcLen,&InstructionBuffer[OpcodePtr-DebugDataExchange.DisassembleFromOffset],&DebugDataExchange.ProcessInfo);
						// RESTORE INT3
						if (InstructionBuffer[OpcodePtr-DebugDataExchange.DisassembleFromOffset] == 0xCC) {
							PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
							while (BpxTable->Offset != 0) {
								if (BpxTable->Offset == OpcodePtr) {
									// READ MAX
									ReadMem (OpcodePtr,16,&InstructionBuffer[OpcodePtr-DebugDataExchange.DisassembleFromOffset],&DebugDataExchange.ProcessInfo);
									InstructionBuffer[OpcodePtr-DebugDataExchange.DisassembleFromOffset] = BpxTable->OrigByte;
									distorm_decode(OpcodePtr, (const unsigned char*)&InstructionBuffer[OpcodePtr-DebugDataExchange.DisassembleFromOffset], 16, dt, decodedInstructions, 15, &decodedInstructionsCount);
									OpcLen = decodedInstructions[0].size;
								}
								BpxTable++;
							}			
						}
						OpcodePtr += OpcLen;
					}


					// Decode the buffer at given offset (virtual address).
					res = distorm_decode(DebugDataExchange.DisassembleFromOffset, (const unsigned char*)InstructionBuffer, (((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H)+1)*16, dt, decodedInstructions, MAX_INSTRUCTIONS, &decodedInstructionsCount);

					int ScrollFixup = 0;

					// IF WE WANT TO DISPLAY SYMBOLS WE SHOULD MANIPULATE THE decodedInstructions structure
					int LineCounter = ((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H);
					int LineCounterTemp = ((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H);
					for (int i = 0; i < LineCounter; i++) {
						// CHECK ALL OFFSETS AGAINST SYMBOLS, IF FOUND INSERT IT
						PDllTable MyDllTable = (PDllTable)DebugDataExchange.DllTablePtr;
						if (MyDllTable != NULL) {
							bool SymbolFound = false;
							while (MyDllTable->FunctionsAvailable != NULL) {
								PApiTable MyApiTable = MyDllTable->Apis;
								if (MyApiTable != NULL) {
									for (unsigned int j=0;j<MyDllTable->FunctionsAvailable;j++) {
										if (MyApiTable->Offset == decodedInstructions[i].offset) {
											char ReplaceChar[MAX_PATH] = "";
											strcat_s (ReplaceChar,_countof(ReplaceChar),(const char *)MyDllTable->DllAscii);
											stringReplace (".dll","",ReplaceChar);
											_strupr_s (ReplaceChar,_countof(ReplaceChar));
											strcat_s (ReplaceChar,_countof(ReplaceChar),"!");
											strcat_s (ReplaceChar,_countof(ReplaceChar),(const char *)MyApiTable->ApiAscii);
											strcat_s (ReplaceChar,_countof(ReplaceChar),"");

											// perform the shift
											_DecodedInst decodedInstructionsTemp[MAX_INSTRUCTIONS];
											for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
												memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
											}
											
											strcpy_s ((char *)&decodedInstructions[i].mnemonic.p,_countof(decodedInstructions[i].mnemonic.p),ReplaceChar);
											decodedInstructions[i].size = 0;
											decodedInstructions[i].instructionHex.p[0] = 0;
											decodedInstructions[i].operands.p[0] = 0;
											LineCounterTemp--;

											for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
												memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
											}
											i++;

											SymbolFound = true;
											break;
										}
										MyApiTable++;
									}
									if (SymbolFound == true) break;
								}
								if (SymbolFound == true) break;
								MyDllTable++;
							}
						}
					}

					// PDB SYMBOL LOOKUP (INSTRUCTION LABELS + CALLS / JMPS)
					if (DebugDataExchange.PDBLoaded == true) {
						for (int i = 0; i < LineCounter+1; i++) {
							char ReturnString[_MAX_PATH];
							memset (&ReturnString,0,_countof(ReturnString));
							if ((_strnicmp ((const char *)&decodedInstructions[i].instructionHex.p,"e8",1) == 0) || (_strnicmp ((const char *)&decodedInstructions[i].instructionHex.p,"e9",1) == 0)) {
				#ifdef b32BitBuild
								ULONG OffsetToRead = 0;
								if (ReadMem ((ULONG)decodedInstructions[i].offset+1,4,&OffsetToRead,&DebugDataExchange.ProcessInfo) == true) {
									OffsetToRead += (ULONG)decodedInstructions[i].offset;
				#else
								ULONGLONG OffsetToRead = 0;
								if (ReadMem (decodedInstructions[i].offset+1,4,&OffsetToRead,&DebugDataExchange.ProcessInfo) == true) {
									OffsetToRead += decodedInstructions[i].offset;
				#endif
									OffsetToRead += 5; // Size of call/jmp
									OffsetToRead -= DebugDataExchange.ImageBase;
									memset (&ReturnString,0,_countof(ReturnString));
									bool bRes = FindSymbolByRva ((ULONG)OffsetToRead,(char *)&ReturnString,_countof(ReturnString));
									if (bRes == true) {
										strcpy_s ((char *)&decodedInstructions[i].operands.p,60,ReturnString);
									}
								}
							}


				#ifdef b32BitBuild
							ULONG Offset = (ULONG)decodedInstructions[i].offset;
				#else
							ULONGLONG Offset = decodedInstructions[i].offset;
				#endif
							Offset -= DebugDataExchange.ImageBase;
							memset (&ReturnString,0,_countof(ReturnString));

							bool bRes = FindSymbolByRva ((ULONG)Offset,(char *)&ReturnString,_countof(ReturnString));
							if (bRes == true)
							{	
								// perform the shift
								_DecodedInst decodedInstructionsTemp[MAX_INSTRUCTIONS];
								for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
									memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
								}
								
								strcpy_s ((char *)&decodedInstructions[i].mnemonic.p,_countof(decodedInstructions[i].mnemonic.p),ReturnString);
								decodedInstructions[i].size = 0;
								decodedInstructions[i].instructionHex.p[0] = 0;
								decodedInstructions[i].operands.p[0] = 0;
								LineCounterTemp--;

								for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
									memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
								}
								i++;
							}


							ULONG bResLine = FindLineByRva ((ULONG)Offset,(char *)&ReturnString,_countof(ReturnString));

							if (bResLine != NULL) {
								for (unsigned int j = 0; j < strlen (ReturnString); j++) {
									if (isprint (ReturnString[j]) == 0) ReturnString[j] = ' ';
								}

								char EmptyChar [_MAX_PATH] = "";
								_DecodedInst decodedInstructionsTemp[MAX_INSTRUCTIONS];
								// perform the shift
								for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
									memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
								}
								
								strcpy_s ((char *)&decodedInstructions[i].mnemonic.p,_countof(decodedInstructions[i].mnemonic.p),EmptyChar);
								decodedInstructions[i].size = 0;
								decodedInstructions[i].instructionHex.p[0] = 0;
								decodedInstructions[i].operands.p[0] = 0;
								LineCounterTemp--;

								for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
									memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
								}
								i++;

								// perform the shift
								int LoopCtr = (int)strlen (ReturnString);
								int Shifter = 0;
								do {
									for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
										memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
									}
									
									memcpy ((char *)&decodedInstructions[i].mnemonic.p,&ReturnString[Shifter],_countof(decodedInstructions[i].mnemonic.p)-1);
									decodedInstructions[i].size = 0;
									decodedInstructions[i].instructionHex.p[0] = 0;
									decodedInstructions[i].operands.p[0] = 0;
									LineCounterTemp--;

									for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
										memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
									}
									i++;
									Shifter += 59;
									LoopCtr -= 59;
								} while (LoopCtr > 0);


								// perform the shift
								for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
									memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
								}
								
								strcpy_s ((char *)&decodedInstructions[i].mnemonic.p,_countof(decodedInstructions[i].mnemonic.p),EmptyChar);
								decodedInstructions[i].size = 0;
								decodedInstructions[i].instructionHex.p[0] = 0;
								decodedInstructions[i].operands.p[0] = 0;
								LineCounterTemp--;

								for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
									memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
								}
								i++;

							}
						}
					}



#ifdef b32BitBuild
					OpcodePtr = (ULONG)decodedInstructions[OpcodeCounter].offset;
#else
					OpcodePtr = (ULONGLONG)decodedInstructions[OpcodeCounter].offset;
#endif

					unsigned char HexBuff[1] = "";
					if (ReadMem (OpcodePtr,1,&HexBuff,&DebugDataExchange.ProcessInfo) == true) {
						PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
						bool AlreadyActive = false;
						while (BpxTable->Offset != 0) {
							if (BpxTable->Offset == OpcodePtr) {
								AlreadyActive = true;
								break;
							}
							BpxTable++;
						}
						if (AlreadyActive == false) {
							BpxTable->Offset = OpcodePtr;
							BpxTable->OrigByte = SetInt3BreakPoint (OpcodePtr,&DebugDataExchange.ProcessInfo);
							BpxTable->Enabled = true;
							BpxTable->Type = 0;
							BpxTable->Type2 = 0;	// ONLY NEEDED FOR BPM!
#ifdef b32BitBuild
							ShowLatestMessages = DisplayMessage (Queue,"UGDBG: bpx: set on %8.8x",OpcodePtr);		
#else
							ShowLatestMessages = DisplayMessage (Queue,"UGDBG: bpx: set on %16.16llx",OpcodePtr);		
#endif
						} else {
							if (BpxTable->Enabled == false)
							{
								BpxTable->Enabled = true;
								BpxTable->OrigByte = SetInt3BreakPoint (OpcodePtr,&DebugDataExchange.ProcessInfo);
#ifdef b32BitBuild
								ShowLatestMessages = DisplayMessage (Queue,"UGDBG: bpx: enabled on %8.8x",OpcodePtr);		
#else
								ShowLatestMessages = DisplayMessage (Queue,"UGDBG: bpx: enabled on %16.16llx",OpcodePtr);		
#endif
							} else {
								// ALREADY SET, DISABLE IT
								BpxTable->Enabled = false;
								WriteMem (OpcodePtr,1,&BpxTable->OrigByte,&DebugDataExchange.ProcessInfo);
								FlushInstructionCache (DebugDataExchange.ProcessInfo.hProcess,(LPCVOID)OpcodePtr,32);
#ifdef b32BitBuild
								ShowLatestMessages = DisplayMessage (Queue,"UGDBG: bpx: disabled on %8.8x",OpcodePtr);		
#else	
								ShowLatestMessages = DisplayMessage (Queue,"UGDBG: bpx: disabled on %16.16llx",OpcodePtr);		
#endif
							}
						}
					}
				}
				Press = true;
			}

			if (!(MouseState & SDL_BUTTON_LMASK)) {
				Press = false;
			}


			//***************************************
			//
			// MOUSE HANDLER - SCROLL WINDOWS
			//
			//***************************************
			if ((MousePosX >= 5) && (MousePosX <= (WindowSizeX-5))) {
				switch (ereignis.button.button)
				{
					case SDL_BUTTON_WHEELDOWN:
						if ((MousePosY >= DISASSEMBLY_Y_POS) && (MousePosY <= LOGTEXT_Y_POS)) {
							DebugDataExchange.DisassembleFromOffset += FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
							DebugDataExchange.AllowScrolling = true;
						}
						if ((MousePosY >= DATA_Y_POS) && (MousePosY <= DISASSEMBLY_Y_POS)) {
							DebugDataExchange.DisplayMemoryOffset +=0x10;
						}
						if ((MousePosY >= LOGTEXT_Y_POS) && (MousePosY <= INPUTWINDOW_Y_POS)) {
							ShowLatestMessages = false;
							ShowLogMessagesStartingByNumber++;
						}
						break;
					case SDL_BUTTON_WHEELUP:
						if ((MousePosY >= DISASSEMBLY_Y_POS) && (MousePosY <= LOGTEXT_Y_POS)) {
							DebugDataExchange.DisassembleFromOffset --;
							DebugDataExchange.AllowScrolling = true;
						}
						if ((MousePosY >= DATA_Y_POS) && (MousePosY <= DISASSEMBLY_Y_POS)) {
							DebugDataExchange.DisplayMemoryOffset -=0x10;
						}
						if ((MousePosY >= LOGTEXT_Y_POS) && (MousePosY <= INPUTWINDOW_Y_POS)) {
							ShowLatestMessages = false;
							if (ShowLogMessagesStartingByNumber > 1) ShowLogMessagesStartingByNumber--;
						}
						break;
				}
			}


			//***************************************
			//
			// HERE WE HANDLE THE USER INTERFACE INPUT IN STANDARD MODE (NO dialogues present)
			//
			//***************************************
			if ((FileDialogue == NULL) && (AttachDialogue == NULL)) {
				switch (ereignis.type) 
				{
					case SDL_USEREVENT:
						// THIS EVENT UPDATES THE REGISTERS
						if ((ereignis.user.code == 0) && (GoActive == false)) {
							// SAVE REGISTERS THIS IS FOR LETTING THE REGISTER HIGHLIGHTER FIND THE OLD VALUES TO SEE IF STH HAS CHANGED
#ifdef b32BitBuild
							PREVIOUS_CTX.Eax = CTX.Eax;
							PREVIOUS_CTX.Ebx = CTX.Ebx;
							PREVIOUS_CTX.Ecx = CTX.Ecx;
							PREVIOUS_CTX.Edx = CTX.Edx;
							PREVIOUS_CTX.Esi = CTX.Esi;
							PREVIOUS_CTX.Edi = CTX.Edi;
							PREVIOUS_CTX.Ebp = CTX.Ebp;
							PREVIOUS_CTX.Eip = CTX.Eip;
							PREVIOUS_CTX.Esp = CTX.Esp;
#else
							PREVIOUS_CTX.Rax = CTX.Rax;
							PREVIOUS_CTX.Rbx = CTX.Rbx;
							PREVIOUS_CTX.Rcx = CTX.Rcx;
							PREVIOUS_CTX.Rdx = CTX.Rdx;
							PREVIOUS_CTX.Rsi = CTX.Rsi;
							PREVIOUS_CTX.Rdi = CTX.Rdi;
							PREVIOUS_CTX.Rbp = CTX.Rbp;
							PREVIOUS_CTX.Rip = CTX.Rip;
							PREVIOUS_CTX.Rsp = CTX.Rsp;
							PREVIOUS_CTX.R8 = CTX.R8;
							PREVIOUS_CTX.R9 = CTX.R9;
							PREVIOUS_CTX.R10 = CTX.R10;
							PREVIOUS_CTX.R11 = CTX.R11;
							PREVIOUS_CTX.R12 = CTX.R12;
							PREVIOUS_CTX.R13 = CTX.R13;
							PREVIOUS_CTX.R14 = CTX.R14;
							PREVIOUS_CTX.R15 = CTX.R15;
#endif

							if (ReadContext (&CTX,&DebugDataExchange) == false) {
								ShowLatestMessages = DisplayMessage (Queue,"UGDBG: failed to read context");
							}
							//continue;
							break;
						}
						// THIS EVENT UPDATES THE DISASSEMBLY
						if (ereignis.user.code == 1) {
							if (ReadContext (&CTX,&DebugDataExchange) == true) {
#ifdef b32BitBuild
								DebugDataExchange.DisassembleFromOffset = CTX.Eip;
#else
								DebugDataExchange.DisassembleFromOffset = CTX.Rip;
#endif
							}
							break;
						}
						// THIS EVENT UPDATES THE MEMORY DISPLAY
						if (ereignis.user.code == 2) {
							if (ReadContext (&CTX,&DebugDataExchange) == true) {
#ifdef b32BitBuild
								DebugDataExchange.DisplayMemoryOffset = CTX.Eip;
#else
								DebugDataExchange.DisplayMemoryOffset = CTX.Rip;
#endif
							}
							break;
						}
						break;
					case SDL_KEYDOWN:
						if ((ereignis.key.keysym.sym == SDLK_F12) && (ereignis.key.keysym.mod & KMOD_CTRL) && (ereignis.key.keysym.mod & KMOD_ALT))	{
							CenterWindow (i.window,WindowSizeX,WindowSizeY);
							break;
						}

						if ((ereignis.key.keysym.sym == SDLK_F12) && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							sprintf_s (ScreenshotFilename,_countof(ScreenshotFilename),"screenshot-%4.4d.bmp",ScreenshotCounter);
							SDL_SaveBMP (screen,ScreenshotFilename);
							ScreenshotCounter++;
							ShowLatestMessages = DisplayMessage (Queue,"UGDBG: saved Screenshot to: %s",ScreenshotFilename);
							break;
						}

						// MOVE DEBUGGER WINDOW UP			
						if ((ereignis.key.keysym.sym == SDLK_UP) && (ereignis.key.keysym.mod & KMOD_CTRL) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							MoveDebuggerWindow (i.window,0,-10,WindowSizeX,WindowSizeY);
							break;
						}
						// MOVE DEBUGGER WINDOW DOWN			
						if ((ereignis.key.keysym.sym == SDLK_DOWN) && (ereignis.key.keysym.mod & KMOD_CTRL) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							MoveDebuggerWindow (i.window,0,+10,WindowSizeX,WindowSizeY);
							break;
						}
						// MOVE DEBUGGER WINDOW LEFT
						if ((ereignis.key.keysym.sym == SDLK_LEFT) && (ereignis.key.keysym.mod & KMOD_CTRL) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							MoveDebuggerWindow (i.window,-10,0,WindowSizeX,WindowSizeY);
							break;
						}
						// MOVE DEBUGGER WINDOW RIGHT
						if ((ereignis.key.keysym.sym == SDLK_RIGHT) && (ereignis.key.keysym.mod & KMOD_CTRL) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							MoveDebuggerWindow (i.window,+10,0,WindowSizeX,WindowSizeY);
							break;
						}
						// MODIFY MEMORY WINDOW
						if ((ereignis.key.keysym.sym == SDLK_RIGHT) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							DebugDataExchange.DisplayMemoryOffset +=1;
							break;
						}
						// MODIFY MEMORY WINDOW
						if ((ereignis.key.keysym.sym == SDLK_LEFT) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							DebugDataExchange.DisplayMemoryOffset -=1;
							break;
						}
						// MODIFY MEMORY WINDOW
						if ((ereignis.key.keysym.sym == SDLK_UP) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							DebugDataExchange.DisplayMemoryOffset -=0x10;
							break;
						}
						// MODIFY MEMORY WINDOW
						if ((ereignis.key.keysym.sym == SDLK_DOWN) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							DebugDataExchange.DisplayMemoryOffset +=0x10;
							break;
						}
						// MODIFY MEMORY WINDOW
						if ((ereignis.key.keysym.sym == SDLK_PAGEDOWN) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							DebugDataExchange.DisplayMemoryOffset +=((DISASSEMBLY_Y_POS - DATA_Y_POS) / LINE_H)*0x10;

							break;
						}
						// MODIFY MEMORY WINDOW
						if ((ereignis.key.keysym.sym == SDLK_PAGEUP) && (ereignis.key.keysym.mod & KMOD_ALT)) {
							DebugDataExchange.DisplayMemoryOffset -=((DISASSEMBLY_Y_POS - DATA_Y_POS) / LINE_H)*0x10;
							break;
						}

						// SCROLL LOG WINDOW UP
						if ((ereignis.key.keysym.sym == SDLK_UP) && (ereignis.key.keysym.mod & KMOD_SHIFT)) {
							ShowLatestMessages = false;
							if (ShowLogMessagesStartingByNumber > 1) ShowLogMessagesStartingByNumber--;
							break;
						}
						// SCROLL LOG WINDOW DOWN
						if ((ereignis.key.keysym.sym == SDLK_DOWN) && (ereignis.key.keysym.mod & KMOD_SHIFT)) {
							ShowLatestMessages = false;
							ShowLogMessagesStartingByNumber++;
							break;
						}


						// RESIZE DEBUGGER WINDOW
						if ((ereignis.key.keysym.sym == SDLK_F1)) {

							WindowSizeY += LINE_H;
							SetVideoMode (Queue);
							break;
						}

						if ((ereignis.key.keysym.sym == SDLK_F2)) {
							if (WindowSizeY > 120) {
								WindowSizeY -= LINE_H;

								// BUG FIX 
								if ((LOGTEXT_Y_POS + LINE_H*3) > WindowSizeY) {
									LOGTEXT_Y_POS -= LINE_H;
									if ((DISASSEMBLY_Y_POS + LINE_H) > LOGTEXT_Y_POS) DISASSEMBLY_Y_POS -= LINE_H;
								}

								SetVideoMode (Queue);
							} else ShowLatestMessages = DisplayMessage (Queue,"UGDBG: Cannot go smaller, sorry");
							break;
						}



						if (ereignis.key.keysym.sym == SDLK_F6) {
							if (instructionHex == false) instructionHex = true; else instructionHex = false;
							break;
						}

						if ((ereignis.key.keysym.sym == SDLK_PAGEDOWN) && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							// Get size of current opcode
							for (int j = 0;j < ((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H);j++) {
	#ifdef b32BitBuild
								DebugDataExchange.DisassembleFromOffset += FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
	#else
								DebugDataExchange.DisassembleFromOffset += (ULONGLONG)FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
	#endif
							}
							DebugDataExchange.AllowScrolling = true;
							break;
						}

						if ((ereignis.key.keysym.sym == SDLK_DOWN) && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							// Get size of current opcode
	#ifdef b32BitBuild
							DebugDataExchange.DisassembleFromOffset += FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
	#else
							DebugDataExchange.DisassembleFromOffset += (ULONGLONG)FetchOpcodeSize (DebugDataExchange.DisassembleFromOffset,&DebugDataExchange);
	#endif
							DebugDataExchange.AllowScrolling = true;
							break;
						}

						if ((ereignis.key.keysym.sym == SDLK_UP) && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							DebugDataExchange.DisassembleFromOffset --;
							DebugDataExchange.AllowScrolling = true;
							break;
						}
						if ((ereignis.key.keysym.sym == SDLK_F3) && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							if (DISASSEMBLY_Y_POS > DATA_Y_POS+LINE_H) DISASSEMBLY_Y_POS -= LINE_H;
							break;
						}
						if (ereignis.key.keysym.sym == SDLK_F3) {
							if (DISASSEMBLY_Y_POS < LOGTEXT_Y_POS-LINE_H) DISASSEMBLY_Y_POS += LINE_H;
							break;
						}
						if ((ereignis.key.keysym.sym == SDLK_F4) && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							if (LOGTEXT_Y_POS > DISASSEMBLY_Y_POS+LINE_H) LOGTEXT_Y_POS -= LINE_H;
							break;
						}
						if (ereignis.key.keysym.sym == SDLK_F4)	{
							if (LOGTEXT_Y_POS < INPUTWINDOW_Y_POS-LINE_H) LOGTEXT_Y_POS += LINE_H;
							break;
						}

						if ((ereignis.key.keysym.sym == SDLK_l)  && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							if (FileDialogue != 0) break;
							ShowLatestMessages = DisplayMessage (Queue,"UGDBG: executing file open dialogue");
							FileDialogue = SDL_CreateMutex();
							break;
						}
						if ((ereignis.key.keysym.sym == SDLK_a)  && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							if (AttachDialogue != 0) break;
							ShowLatestMessages = DisplayMessage (Queue,"UGDBG: executing file attach dialogue");
							AttachDialogue = SDL_CreateMutex();
							break;
						}

						if ((ereignis.key.keysym.sym == SDLK_r)  && (ereignis.key.keysym.mod & KMOD_CTRL)) {
							// CHECK IF USER HAD A FILE LOADED BEFORE
							if (strlen (DebugDataExchange.FileName) != 0) {
								char TmpFileName[MAX_PATH+1];
								strcpy_s (TmpFileName,_countof(TmpFileName),DebugDataExchange.FileName);

								CleanupDia ();
								// TERMINATE CURRENTLY ACTIVE PROCESS!
								SDL_KillThread (DebugDataExchange.Thread);
								Cleanup (&DebugDataExchange);
								memset (&DebugDataExchange,0,sizeof(DebugStruc));

								// FIRE DEBUGGER CORE
								DebugDataExchange.DisassembleFromOffset = 0;
								memset (&DebugDataExchange,0,sizeof (DebugStruc));
								DebugDataExchange.WindowSizeX = &WindowSizeX;
								DebugDataExchange.WindowSizeY = &WindowSizeY;
								DebugDataExchange.AllowScrolling = true;
								InitializeCore (TmpFileName,NULL,&Debuggee,Queue,&ShowLatestMessages,&DebugDataExchange);
							}
						}

						if ((ereignis.key.keysym.sym == SDLK_F12) && (DebugDataExchange.PollEvent == true)) {
							
							if (KeepTracing == true) {
								KeepTracing = false;
								SDL_KillThread (TracerThreadVar);
								TracerThreadVar = 0;
								DebugDataExchange.RunTraceActive = false;
							} else {
								KeepTracing = true;
							}
							break;
						}


						if (DebugDataExchange.PollEvent == true) {
							if (ereignis.key.keysym.sym == SDLK_F5) {
								if (DebugDataExchange.BpmTrigger == true) 
								{
									// Enable Trap Flag
									DebugDataExchange.AllowScrolling = false;
									DebugDataExchange.AllowInt1 = true;
									DebugDataExchange.PollEvent = false;
									break;
								}
								bool HitFound = false;
								PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
								if (BreakFound == false) {
									while (BpxTable->Offset != 0) {
#ifdef b32BitBuild
										if ((CTX.Eip == BpxTable->Offset) && (BpxTable->Enabled == TRUE)) {
#else	
										if ((CTX.Rip == BpxTable->Offset) && (BpxTable->Enabled == TRUE)) {
#endif
											HitFound = true;
											break;
										}
										BpxTable++;
									}
								}
								// Enable Trap Flag
								EnableTrapflag (&DebugDataExchange);
								ReEnableBreakpoints (&DebugDataExchange.DbgEvent,&DebugDataExchange);
								// Continue
								DebugDataExchange.AllowScrolling = false;
								DebugDataExchange.AllowInt1 = true;
								DebugDataExchange.PollEvent = false;
								if (HitFound == false) GoActive = true;
								break;
							}

							if (ereignis.key.keysym.sym == SDLK_F8) {
/*
								PUSHFD emulation
								CONTEXT CTX;
								if (ReadContext (&CTX,&DebugDataExchange) == true) {
									char MyBuff[1] = "";
									if (ReadMem (CTX.Eip,1,&MyBuff,&DebugDataExchange.ProcessInfo) == true) {
										if (MyBuff[0] == 0x9C) {
											CTX.Esp -= 4;
											CTX.Eip ++;
											CTX.EFlags &= 0xFFFFFEFF;
											WriteMem (CTX.Esp,4,&CTX.EFlags,&DebugDataExchange.ProcessInfo);
										}
									}
								}
								*/
								// Enable Trap Flag
								EnableTrapflag (&DebugDataExchange);
								ReEnableBreakpoints (&DebugDataExchange.DbgEvent,&DebugDataExchange);
								// Continue
								DebugDataExchange.AllowScrolling = false;
								DebugDataExchange.AllowInt1 = true;
								DebugDataExchange.PollEvent = false;
								// don't redraw!
								continue;
							}
							if (ereignis.key.keysym.sym == SDLK_F10) {
								DisableTrapflag (&DebugDataExchange);
								StepOver (&DebugDataExchange);
								ReEnableBreakpoints (&DebugDataExchange.DbgEvent,&DebugDataExchange);
								// Continue
								DebugDataExchange.AllowInt1 = true;
								DebugDataExchange.AllowScrolling = false;
								DebugDataExchange.PollEvent = false;
								// don't redraw!
								continue;
							}
						}
						if (ereignis.key.keysym.sym == SDLK_ESCAPE)	{
							switch (Cursor_Region) {
								case 0:
									FileDialogue = SDL_CreateMutex ();
									exit(0);
									break;
								case 1:
									Cursor_Region = 0;
									continue;
									break;
							}
						}


						//
						// HANDLE KEYBOARD (CURSOR)
						// 
						if ((AttachDialogue == NULL) && (FileDialogue == NULL)) {
							switch (Cursor_Region) {
								case 0:
									HandleKeyboardInputCursor (&InFile,&ereignis,&DebugDataExchange);
									break;
								case 1:
									HandleKeyboardInputCursorData (&InFile,&ereignis,&DebugDataExchange);
									break;
							}
						}


						break;
					case SDL_VIDEORESIZE:
						break;
					case SDL_QUIT:
						exit(0);
				}
			} else {

				if (AttachDialogue != NULL) {
					//***************************************
					//
					// HERE WE HANDLE THE USER INTERFACE INPUT - FILE ATTACH DIALOG
					//
					//***************************************
					switch (ereignis.type) 
					{
						case SDL_KEYDOWN:
							if (ereignis.key.keysym.sym == SDLK_ESCAPE)	{
								SDL_DestroyMutex (AttachDialogue);
								AttachDialogue = 0;
								break;
							}

							// RETRIEVE SELECTION
						if (ereignis.key.keysym.sym == SDLK_RETURN)	{
								CleanupDia ();
								// TERMINATE CURRENTLY ACTIVE PROCESS!
								SDL_KillThread (DebugDataExchange.Thread);
								Cleanup (&DebugDataExchange);
								memset (&DebugDataExchange,0,sizeof(DebugStruc));

								SDL_DestroyMutex (AttachDialogue);
								AttachDialogue = 0;

								DebugDataExchange.DisassembleFromOffset = 0;
								memset (&DebugDataExchange,0,sizeof (DebugStruc));
								DebugDataExchange.WindowSizeX = &WindowSizeX;
								DebugDataExchange.WindowSizeY = &WindowSizeY;
								DebugDataExchange.AllowScrolling = true;
								ShowLatestMessages = DisplayMessage (Queue,"UGDBG: selected PID: %x",RunningPIDs[ProcessHighlighter+1]);
								InitializeCore (NULL,RunningPIDs[ProcessHighlighter+1],&Debuggee,Queue,&ShowLatestMessages,&DebugDataExchange);							
								break;
							}


							// SCROLL BAR DOWN
							if (ereignis.key.keysym.sym == SDLK_DOWN) {
								ScrollDownAttach (&ProcessHighlighter, &SkipEntriesProcesses, WindowSizeY, CurrentPath, &ProcessesPresent, Queue, &ShowLatestMessages);
								break;
							}

							// SCROLL BAR UP
							if (ereignis.key.keysym.sym == SDLK_UP)	{
								ScrollUpAttach (&ProcessHighlighter,&SkipEntriesProcesses);
								break;
							}
							
							// SCROLL BAR UP BY 1 PAGE
							if (ereignis.key.keysym.sym == SDLK_PAGEUP)	{
								for (int i = 0; i < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -2; i++)	{
									ScrollUpAttach (&ProcessHighlighter,&SkipEntriesProcesses);
								}
								break;
							}

							// SCROLL BAR DOWN BY 1 PAGE
							if (ereignis.key.keysym.sym == SDLK_PAGEDOWN)	{
								for (int i = 0; i <  (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -2; i++) {
									ScrollDownAttach (&ProcessHighlighter, &SkipEntriesProcesses, WindowSizeY, CurrentPath, &ProcessesPresent, Queue, &ShowLatestMessages);
								}
								break;
							}
							break;
					}
				}

				if (FileDialogue != NULL) {
					HandleFileDialog (&ereignis,(char *)&CurrentPath,FileDialogue,&FilesPresent,&SkipEntries,&FileHighlighter,&DebugDataExchange,&Debuggee,&WindowSizeX,&WindowSizeY,Queue,&ShowLatestMessages,(ULONG *)&FileDialogue);
				}
			}
			if (KeepTracing == true){
				if (TracerThreadVar == 0) {
					TracerThreadVar = SDL_CreateThread (TracerThread,NULL);
				}
			}

			//***************************************
			//
			// DRAW TO THE BACKBUFFER
			//
			//***************************************		

			// Delete + Draw things to the backbuffer
			SDL_FillRect (backbuffer,0,SDL_MapRGB(backbuffer->format, 0,0,0));

			Draw_Line (backbuffer, 7, LINE_UPPER,5*8+RIGHT_MARGEIN,LINE_UPPER, c_SiceGreen);
			Draw_Rect (backbuffer, 3, 3, backbuffer->w-6, backbuffer->h-6, c_SiceBorder);
			Draw_Rect (backbuffer, 4, 4, backbuffer->w-8, backbuffer->h-8, c_SiceBorder);
			// DRAW A LINE BETWEEN THE DATA AND THE DISASSEMBLY
			Draw_Line (backbuffer, 7, DISASSEMBLY_Y_POS - 5,backbuffer->w - 7,DISASSEMBLY_Y_POS - 5, c_SiceGreen);
			// DRAW A LINE BETWEEN THE DISASSEMBLY AND THE LOG WINDOW
			Draw_Line (backbuffer, 7, LOGTEXT_Y_POS - 5,(WindowSizeX/2)-12,LOGTEXT_Y_POS - 5, c_SiceGreen);
			// DRAW STATUSBAR
			Draw_FillRect (backbuffer,7,backbuffer->h-15,backbuffer->w-14,8,c_SiceBorder);
			// DRAW KEYBOARD INPUTBUFFER + CURSOR
			Draw_InputBuffer (&InFile,backbuffer,WindowSizeY,&DebugDataExchange);
			
			switch (Cursor_Region) {
				case 0:
					Draw_Cursor (backbuffer,WindowSizeY,&InFile,Cursor_Interval);
					break;
				case 1:
					Draw_Cursor_Data (backbuffer,&InFile,Cursor_Interval);
					break;
			}
			UpdateRegisters (backbuffer,&CTX,&PREVIOUS_CTX,InFile, REGISTERS_Y_POS, x64BitMode);
			UpdateDisassembly (backbuffer,&CTX,DebugDataExchange.DisassembleFromOffset,dt,instructionHex,InFile, GetDisassemblyYPos(),GetLogYPos(), x64BitMode, &DebugDataExchange,GetCurrentWidth());
			UpdateMemory (backbuffer,DebugDataExchange.DisplayMemoryOffset,&DebugDataExchange.ProcessInfo,&CTX,InFile,GetDataYPos(),GetDisassemblyYPos(), x64BitMode, GetCurrentWidth());
			EnumerateSectionOffset (backbuffer,&CTX,&DebugDataExchange,InFile,GetLogYPos()-8,GetCurrentWidth());
			EnumerateSectionOffsetFromData (backbuffer,DebugDataExchange.DisplayMemoryOffset,&DebugDataExchange,InFile,GetDataYPos()-8,GetCurrentWidth());

			if (DebugDataExchange.Queue != Queue) {
				Queue = DebugDataExchange.Queue;
				ShowLogMessagesStartingByNumber = 1;
			}
			//***************************************
			//
			// DISPLAY TEXTLOG FROM THE CORRECT POSITION
			//
			//***************************************
			if (Queue != 0) {
				if (ShowLatestMessages == true) {
					Messages * CountQueue = Queue;
					while (CountQueue->NextEntry != 0) {
						if (CountQueue->MessageNr >= ((GetInputYPos() - GetLogYPos())/LINE_H)) {
							ShowLogMessagesStartingByNumber = CountQueue->MessageNr - ((GetInputYPos() - GetLogYPos())/LINE_H) +1 ;
						}
						CountQueue = CountQueue->NextEntry;
					}
					ShowLatestMessages = false;
				}
				UpdateLogWindow (InFile,backbuffer,Queue,ShowLogMessagesStartingByNumber,LOGTEXT_Y_POS,INPUTWINDOW_Y_POS, &DebugDataExchange,WindowSizeX);
			}

			//***************************************
			//
			// HANDLE FILE DIALOG
			//
			//***************************************
			if (FileDialogue != 0) {
				DisplayFileDialog (backbuffer,GetCurrentWidth(),GetCurrentHeight(),CurrentPath,&FileHighlighter,SkipEntries,&ShowLatestMessages,Queue,InFile,c_SiceBorder,c_Black);
			}

			//***************************************
			//
			// HANDLE ATTACH DIALOG
			//
			//***************************************
			if (AttachDialogue != 0) {
				DisplayAttachDialog (backbuffer,GetCurrentWidth(),GetCurrentHeight(),&ProcessesPresent,(DWORD*)&RunningPIDs,ProcessHighlighter,SkipEntriesProcesses,&ShowLatestMessages,Queue,InFile,c_SiceBorder,c_Black);
			}


			//***************************************
			//
			// IF DEBUGGER IS INACTIVE WE DARKEN THE SURFACE
			//
			//***************************************
	//		if ((FileDialogue == NULL) && (AttachDialogue == NULL) && (DebugDataExchange.Active == false)){
	//		SDL_SetAlpha (grey_surface,SDL_SRCALPHA,Fader);
	//		SDL_BlitSurface (grey_surface,NULL,backbuffer,NULL);
	//		}
			//***************************************
			//
			// BLIT BACKBUFFER TO SCREEN
			//
			//***************************************
			// Blit Backbuffer to screen (avoid flickering)
			SDL_BlitSurface(backbuffer, NULL, screen, NULL);
			// Flip Front/Backbuffer
			SDL_Flip(screen);
		
	}
	return 0;
}

