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
#include "cursor.h"
#include "split.h"

char StatusText[_MAX_PATH] = "Enter a Command (h for Help)";
AutoStruc AutoExec;

void HandleKeyboardInputCursor (RETBUFFER * InFile, SDL_Event * event,DebugStruc * DebugDataExchange) {
	if ((event->key.state == SDL_PRESSED) && ((!(event->key.keysym.mod & KMOD_CTRL)) || (event->key.keysym.unicode == SDLK_BACKSLASH) || (event->key.keysym.unicode == SDLK_AT))) {
		if (event->key.keysym.sym == SDLK_TAB) {
			int argc;
			char* argv[128];
			char* linecopy;

			linecopy = _strdup(InputBuffer);
			argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
			if(argc) {
				if((strlen (argv[argc-1]) > 0) && (InputBuffer[CursorPos-1] != ' ')) {
					if (argc == 1) {
						int i = 0;
						while (strlen (Command[i]) != 0) {
							if (_strnicmp (argv[argc-1],Command[i],strlen(argv[argc-1])) == NULL) {
								char * CompletePtr = Command[i];
								strcat_s (InputBuffer,_countof(InputBuffer),&CompletePtr[strlen(argv[argc-1])]);
								CursorPos = strlen (InputBuffer);
								break;
							}
							i++;
						}
					} else {
						if (argc > 1) {
							// Resolve Api -> we display all possibilities in the statusbar
							PDllTable MyDllTable = (PDllTable)DebugDataExchange->DllTablePtr;
							if (MyDllTable != NULL) {
								while (MyDllTable->FunctionsAvailable != NULL) {
									PApiTable MyApiTable = MyDllTable->Apis;
									if (MyApiTable != NULL) {
										for (unsigned int i=0;i<MyDllTable->FunctionsAvailable;i++) {
											if (_strnicmp (argv[argc-1],(const char *)MyApiTable->ApiAscii,strlen(argv[argc-1])) == NULL) {
												char * CompletePtr = (char *)MyApiTable->ApiAscii;
												strcat_s (InputBuffer,_countof(InputBuffer),&CompletePtr[strlen(argv[argc-1])]);
												CursorPos = strlen (InputBuffer);
												free (linecopy);
												return;
											}
											MyApiTable++;
										}
									}
									MyDllTable++;
								}
							}
						}
					}
				} 
			}
			free (linecopy);
			return;
		}
		
		if (event->key.keysym.sym == SDLK_UP) {
			if (HistoryEntry > 1) HistoryEntry --;
			// browse history;
			if (HistoryList != NULL) {
				HistoryBuffer * HistoryListTmp = HistoryList;
				while (HistoryListTmp != 0) {
					if (HistoryListTmp->CommandNr == HistoryEntry) {
						strcpy_s (InputBuffer,_countof(InputBuffer),HistoryListTmp->Command);
						CursorPos = strlen (InputBuffer);
						break;
					}
					HistoryListTmp = HistoryListTmp->NextEntry;
				}
			}
			return;
		}

		if (event->key.keysym.sym == SDLK_DOWN) {
			// browse history;
			if (HistoryEntry < HistoryMax-1) HistoryEntry++;

			if (HistoryList != NULL) {
				HistoryBuffer * HistoryListTmp = HistoryList;
				while (HistoryListTmp != 0) {
					if (HistoryListTmp->CommandNr == HistoryEntry) {
						strcpy_s (InputBuffer,_countof(InputBuffer),HistoryListTmp->Command);
						CursorPos = strlen (InputBuffer);
						break;
					}
					HistoryListTmp = HistoryListTmp->NextEntry;
				}
			}
			return;
		}


		if (event->key.keysym.sym == SDLK_LEFT) {
			if (CursorPos > 0) CursorPos --;
			return;
		}

		if (event->key.keysym.sym == SDLK_HOME) {
			CursorPos = 0;
			return;
		}

		if (event->key.keysym.sym == SDLK_END) {
			CursorPos = strlen (InputBuffer);
			return;
		}

		if (event->key.keysym.sym == SDLK_RIGHT) {
			if (strlen (InputBuffer) > CursorPos) CursorPos++;
			return;
		}

		if (event->key.keysym.sym == SDLK_DELETE) {
			char Temp[MAX_PATH];
			strcpy_s (Temp,_countof(Temp),&InputBuffer[CursorPos+1]);
			InputBuffer[CursorPos] = 0;
			strcat_s (InputBuffer,_countof(InputBuffer),Temp);
			return;
		}

		if (event->key.keysym.sym == SDLK_BACKSPACE) {
			if (CursorPos > 0) {
				char Temp[MAX_PATH];
				strcpy_s (Temp,_countof(Temp),&InputBuffer[CursorPos]);
				InputBuffer[CursorPos-1] = 0;
				strcat_s (InputBuffer,_countof(InputBuffer),Temp);
				if (CursorPos > 0) CursorPos --;
			}
			return;
		}

		if (event->key.keysym.sym != SDLK_RETURN) {
			if (strlen (InputBuffer) < 78) {
				char ToAdd[2]="";
				char Temp[MAX_PATH];
				char ch = event->key.keysym.unicode & 0x7f;
				size_t OldValue = strlen (InputBuffer);

				strcpy_s (Temp,_countof(Temp),&InputBuffer[CursorPos]);
				strncpy_s (ToAdd,_countof(ToAdd),&ch,1);
				InputBuffer[CursorPos] = 0;
				strcat_s (InputBuffer,_countof(InputBuffer),ToAdd);
				strcat_s (InputBuffer,_countof(InputBuffer),Temp);

				if (strlen(InputBuffer) > OldValue)	CursorPos ++;
			}
		} else {
			if (strlen (InputBuffer) > 0) {
				// Init history, if initiated let it grow
				if (HistoryList == NULL) {
					HistoryList = (HistoryBuffer *)calloc (1,sizeof(HistoryBuffer));
					if (HistoryList == NULL) {
						*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Sorry but there is no memory for having a command history");
						return;
					}
					HistoryBuffer * HistoryListTmp = HistoryList;
					strcpy_s (HistoryListTmp->Command,_countof(HistoryListTmp->Command),InputBuffer);
					HistoryListTmp->CommandNr = 1;
					HistoryEntry = 2;
					HistoryListTmp->PreviousEntry = 0;

				} else {
					HistoryBuffer * HistoryListTmp = HistoryList;
					int CommandNr = 0;
					while (HistoryListTmp->NextEntry != 0) {
						HistoryListTmp = HistoryListTmp->NextEntry;
					}
					CommandNr = HistoryListTmp->CommandNr;
					CommandNr ++;
					HistoryListTmp->NextEntry = (HistoryBuffer *)calloc (1,sizeof(HistoryBuffer));
					HistoryBuffer * HistoryListPrevious = HistoryListTmp;
					HistoryListTmp = HistoryListTmp->NextEntry;
					HistoryListTmp->CommandNr = CommandNr;
					HistoryEntry = CommandNr+1;
					HistoryMax = HistoryEntry;
					HistoryListTmp->PreviousEntry = HistoryListPrevious;
					strcpy_s (HistoryListTmp->Command,_countof(HistoryListTmp->Command),InputBuffer);
				}
				// Send command to parser
				CommandParser (InFile,DebugDataExchange,InputBuffer);
				// Clear the inputbuffer
				memset (&InputBuffer,0,_countof(InputBuffer));
				// Reset cursorpos
				CursorPos = 0;
			}
		}
	}
	return;
}

void Draw_InputBuffer (RETBUFFER * InFile,SDL_Surface * screen,int WindowSizeY,DebugStruc * DebugDataExchange) {
	// First we gotta handle the AutoExec commands
	if ((AutoExec.Executed == false) && (strlen (AutoExec.AutoExecString) != 0)) {
		int argc;
		char* argv[128];
		char* linecopy;

		linecopy = _strdup(AutoExec.AutoExecString);
		argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
		if(argc) {
			for (int i = 0; i < argc; i++) {
				strcpy_s (InputBuffer,_countof(InputBuffer),argv[i]);
				if (_strnicmp (argv[i],"debug",5) == NULL) {
					char * TempStr1 = argv[i];
					char * TempStr2 = _strdup (&TempStr1[6]);
					sprintf_s(InputBuffer,_countof(InputBuffer),"debug \"%s\"",TempStr2);
					free (TempStr2);
				}
				// Send command to parser
				int bRes = CommandParser (InFile,DebugDataExchange,InputBuffer);
				if (_strnicmp (argv[i],"debug",5) == NULL && bRes == TRUE) {
					while (DebugDataExchange->Active == false) Sleep (1);
				}
				// Clear the inputbuffer
				memset (&InputBuffer,0,_countof(InputBuffer));
				// Reset cursorpos
				CursorPos = 0;
			}
		}
		AutoExec.Executed = true;
		free (linecopy);
	}

	// DRAW INPUTBUFFER
	BiosTextOut (InFile,screen,RIGHT_MARGEIN,WindowSizeY-LINE_H-15,192,192,192,":%s",InputBuffer);

	int argc;
	char* argv[128];
	char* linecopy;

	strcpy_s (StatusText,_countof(StatusText),"Enter a Command (h for Help)");

	linecopy = _strdup(InputBuffer);
	argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
	if(argc) {
		// Resolve Command -> we display all possibilities in the statusbar
		if((strlen (argv[argc-1]) > 0) && (InputBuffer[CursorPos-1] != ' ')) {
			StatusText[0] = '\0';
			if (argc == 1) {
				int i = 0;
				while (strlen (Command[i]) != 0) {
					if (_strnicmp (argv[argc-1],Command[i],strlen(argv[argc-1])) == NULL) {
						if (strlen (StatusText) == NULL) {
							strcat_s (StatusText,_countof(StatusText),Command[i]);
						} else {
							if (strlen (StatusText) < 50) {
								strcat_s (StatusText,_countof(StatusText),", ");
								strcat_s (StatusText,_countof(StatusText),Command[i]);
							}
						}
					}
					i++;
				}
			} else {
				if (argc > 1) {
					// Resolve Api -> we display all possibilities in the statusbar
					PDllTable MyDllTable = (PDllTable)DebugDataExchange->DllTablePtr;
					if (MyDllTable != NULL) {
						while (MyDllTable->FunctionsAvailable != NULL) {
							PApiTable MyApiTable = MyDllTable->Apis;
							if (MyApiTable != NULL) {
								for (unsigned int i=0;i<MyDllTable->FunctionsAvailable;i++) {
									if (_strnicmp (argv[argc-1],(const char *)MyApiTable->ApiAscii,strlen(argv[argc-1])) == NULL) {
										if (strlen (StatusText) == NULL) {
											strcat_s (StatusText,_countof(StatusText),(const char *)MyApiTable->ApiAscii);
										} else {
											if (strlen (StatusText) < 50) {
												strcat_s (StatusText,_countof(StatusText),", ");
												strcat_s (StatusText,_countof(StatusText),(const char *)MyApiTable->ApiAscii);
											}
										}
									}
									MyApiTable++;
								}
							}
							MyDllTable++;
						}
					}
				}
			}
			if (_stricmp (argv[argc-1],StatusText) == 0) StatusText[0] = '\0';
		} 
	}

	// DRAW HELPTEXT
	int Shifter = 0;
	if (_stricmp (StatusText,"Enter a Command (h for Help)") == 0) {
		Shifter = 40;
	}
	BiosTextOut (InFile,screen,RIGHT_MARGEIN+Shifter,screen->h-15,0,0,0,"%s",StatusText);


	free(linecopy);

	return;
}

Uint32 CursorTimer(Uint32 interval, void *param) {
	if (Set == true) Set = false; else Set = true;

	SDL_Event event;
	SDL_UserEvent userevent;
	userevent.type = SDL_USEREVENT;
	userevent.code = 0x100;
	event.type = SDL_USEREVENT;
	event.user = userevent;
	SDL_PushEvent (&event);

	return interval;
}

void Draw_Cursor (SDL_Surface * screen,int WindowSizeY,RETBUFFER * InFile,int Interval) {
	if (Cursor == NULL)
		Cursor = SDL_AddTimer(Interval, CursorTimer, NULL);

	if (Set == true) {
		// Draw the cursor
		size_t XDistance = CursorPos*8;
		SDL_Rect cursor_rect;
		cursor_rect.h = 8;
		cursor_rect.w = 8;
		cursor_rect.x = (Sint16)XDistance+RIGHT_MARGEIN+8;
		cursor_rect.y = WindowSizeY-LINE_H-15;
		Uint32 c_Grey = SDL_MapRGB(screen->format, 192, 192, 192); 

		SDL_FillRect (screen,&cursor_rect,c_Grey);
	}
	return;
}


void ShiftCursorRight (unsigned int LineCounter,DebugStruc * DebugDataExchange) 
{
	if (CursorPosData < (16*2+14)) CursorPosData ++; else {
		CursorPosData = 0;
		if (CursorPosYData+1 < LineCounter) CursorPosYData++; else DebugDataExchange->DisplayMemoryOffset += 0x10;
	}
	if ((CursorPosData+1)%3 == NULL) CursorPosData++;
}

void HandleKeyboardInputCursorData (RETBUFFER * InFile, SDL_Event * event,DebugStruc * DebugDataExchange) {
	// calculate linecounter to see if we must scroll
	unsigned int LineCounter = (GetDisassemblyYPos() - GetDataYPos ()) / LINE_H;
	if (event->key.keysym.sym == SDLK_UP) {
		if (CursorPosYData > 0) CursorPosYData--; else DebugDataExchange->DisplayMemoryOffset -= 0x10;
	}
	if (event->key.keysym.sym == SDLK_DOWN) {
		if (CursorPosYData+1 < LineCounter) CursorPosYData++; else DebugDataExchange->DisplayMemoryOffset += 0x10;
	}
	if (event->key.keysym.sym == SDLK_RIGHT) {
		ShiftCursorRight (LineCounter,DebugDataExchange);
	}
	if (event->key.keysym.sym == SDLK_LEFT) {
		if (CursorPosData > 0) CursorPosData --; else {
			CursorPosData = 16*2+14;
			if (CursorPosYData > 0) CursorPosYData--; else DebugDataExchange->DisplayMemoryOffset -= 0x10;
		}
		if ((CursorPosData+1)%3 == NULL) CursorPosData--;
	}
	if (event->key.keysym.sym != SDLK_RETURN) {
		unsigned char ch = event->key.keysym.unicode & 0x7f;
		if (isalnum (ch) != NULL) {
			// LIMIT input now
			ch -= 0x30;
			if (ch > 10) ch -= 0x27;
			if (ch <= 0xF) {
				unsigned char ByteBuffer[2] = "";
				if (ReadMem(DebugDataExchange->DisplayMemoryOffset+(CursorPosYData*0x10)+((CursorPosData-(CursorPosData/3))/2),1,&ByteBuffer,&DebugDataExchange->ProcessInfo) == true) {
					switch (CursorPosData%3) {
						case 0:
							ByteBuffer[0] &= 0x0F;
							ByteBuffer[0] |= (ch << 4);
							break;
						case 1:
							ByteBuffer[0] &= 0xF0;
							ByteBuffer[0] |= ch;
							break;
					}
					if (WriteMem (DebugDataExchange->DisplayMemoryOffset+(CursorPosYData*0x10)+((CursorPosData-(CursorPosData/3))/2),1,&ByteBuffer,&DebugDataExchange->ProcessInfo) == true) {
						ShiftCursorRight (LineCounter,DebugDataExchange);
					}
				}
			}
		}
	}

	return;
}



void Draw_Cursor_Data (SDL_Surface * screen, RETBUFFER * InFile,int Interval) {
	// we don't need a seperated timer cause there's one running already
	unsigned int LineCounter = (GetDisassemblyYPos() - GetDataYPos ()) / LINE_H;
	if (CursorPosYData >= LineCounter) return;
	if (Set == true) {
		// Draw the cursor
		size_t XDistance = CursorPosData*8;
		size_t YDistance = CursorPosYData*LINE_H;
		SDL_Rect cursor_rect;
		cursor_rect.h = 8;
		cursor_rect.w = 8;
		cursor_rect.x = (Sint16)XDistance+RIGHT_MARGEIN;
#ifdef b32BitBuild
		cursor_rect.x += 8*14;
#else
		cursor_rect.x += 8*22;
#endif
		cursor_rect.y = GetDataYPos()+(Sint16)YDistance;
		Uint32 c_Grey = SDL_MapRGB(screen->format, 192, 192, 192); 

		SDL_FillRect (screen,&cursor_rect,c_Grey);
	}
	return;
}