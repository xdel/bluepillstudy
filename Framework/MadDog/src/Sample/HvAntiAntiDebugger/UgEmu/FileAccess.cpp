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

// *****************************************************************
//  Function: InitQueue
// 
//  In: -
//  
//  Out: Messages * Ptr
//
//	Use: Initializes the Message Queue
//       
// *****************************************************************
FileList * InitFileList () {
	return (FileList *)GlobalAlloc (GMEM_ZEROINIT,sizeof (FileList));
}

// *****************************************************************
//  Function: InitFileAttachList
// 
//  In: -
//  
//  Out: Messages * Ptr
//
//	Use: Initializes the Message Queue
//       
// *****************************************************************
FileAttachList * InitFileAttachList () {
	return (FileAttachList *)GlobalAlloc (GMEM_ZEROINIT,sizeof (FileAttachList));
}

// *****************************************************************
//  Function: DestroyFileList
// 
//  In: FileList * DirectoryContent
//  
//  Out: bool
//
//	Use: Frees the memory
//       
// *****************************************************************
bool DestroyFileList (FileList * DirectoryContent)
{
	while (DirectoryContent != 0) {
		FileList * DirectoryContentDestroy = DirectoryContent;
		DirectoryContent = DirectoryContent->NextEntry;
		GlobalFree (DirectoryContentDestroy);
	}
	return true;
}

// *****************************************************************
//  Function: GetDirectoryContent
// 
//  In: FileList * DirectoryContent
//		 
//  
//  Out: bool
//
//	Use: Reads the directory content of the defined path
//       
// *****************************************************************

bool GetDirectoryContent(FileList * DirectoryContent, char * CurrentPath) {
	FileList * DirectoryContentTmp = DirectoryContent;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char CurrentPathFinal[MAX_PATH+5];
	memset (&CurrentPathFinal,0,_countof(CurrentPathFinal));
	strcpy_s (CurrentPathFinal,_countof(CurrentPathFinal),CurrentPath);
	strcat_s (CurrentPathFinal,_countof(CurrentPathFinal),"\\*.*");

	hFind = FindFirstFile(CurrentPathFinal, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
	  return false;
	} 

	do {
		if (strcmp (ffd.cFileName,".") != 0) {
			DirectoryContentTmp = DirectoryContent;
			memcpy ((void *)&DirectoryContent->Data,(void *)&ffd,sizeof(WIN32_FIND_DATA));
			DirectoryContent->NextEntry = (FileList *)GlobalAlloc (GMEM_ZEROINIT,sizeof (FileList));
			if (DirectoryContent->NextEntry == 0) return false;
			DirectoryContent = DirectoryContent->NextEntry;
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);
	GlobalFree (DirectoryContentTmp->NextEntry);
	DirectoryContentTmp->NextEntry = 0;
	FindClose (hFind);
	return true;
}

// *****************************************************************
//  Function: ScrollUp
// 
//  In: int * FileHighlighter, int * SkipEntries
//		 
//  
//  Out: bool
//
//	Use: Scrolls the bar 1 entry up in the file dialog
//       
// *****************************************************************
bool ScrollUp (int * FileHighlighter, int * SkipEntries)
{
	if (*FileHighlighter > 0) {
		*FileHighlighter -= 1;
	} else {
		if (*SkipEntries > 0) *SkipEntries -=1;
	}
	return true;
}

// *****************************************************************
//  Function: ScrollUpAttach
// 
//  In: int * FileHighlighter, int * SkipEntries
//		 
//  
//  Out: bool
//
//	Use: Scrolls the bar 1 entry up in the file dialog
//       
// *****************************************************************
bool ScrollUpAttach (int * FileHighlighter, int * SkipEntries)
{
	if (*FileHighlighter > 0) {
		*FileHighlighter -= 1;
	} else {
		if (*SkipEntries > 0) *SkipEntries -=1;
	}
	return true;
}

// *****************************************************************
//  Function: ScrollDown
// 
//  In: (int * FileHighlighter, int * SkipEntries, 
//		 int WindowSizeY, char * CurrentPath, 
//		 int * FilesPresent, Messages * Queue, bool * ShowLatestMessages
//		 
//  
//  Out: bool
//
//	Use: Scrolls the bar 1 entry down in the file dialog
//       
// *****************************************************************
bool ScrollDown (int * FileHighlighter, int * SkipEntries, int WindowSizeY, char * CurrentPath, int * FilesPresent, Messages * Queue, bool * ShowLatestMessages)
{
	if (*FileHighlighter < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -2) {
		*FileHighlighter += 1;
	} else {
		*SkipEntries +=1;
	}

	FileList * DirectoryContent = InitFileList ();
	FileList * DirectoryContentTmp = DirectoryContent;
	if (GetCurrentDirectory (MAX_PATH-1,CurrentPath) != 0)	{
		bool bRes = GetDirectoryContent (DirectoryContent, CurrentPath);
		*FilesPresent = 0;
		if (bRes == true) {
			while (DirectoryContent != 0) {
				*FilesPresent += 1;
				DirectoryContent = DirectoryContent->NextEntry;
			}
			if (*FileHighlighter >= *FilesPresent) {
				*FileHighlighter -=1;
			}
		}
	} else {
		*ShowLatestMessages = DisplayMessage (Queue,"UGDBG: failed to retrieve current directory");
	}

	// THIS LIMITS THE SCROLLING TO THE BOTTOM IF OUR LIST IS LARGER THAN 1 SCREEN
	if (*FileHighlighter < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -2) {
	} else {
		if ((*SkipEntries + *FileHighlighter) >= *FilesPresent) {
			*SkipEntries -=1;
		}
	}

	DestroyFileList (DirectoryContentTmp);
	return true;
}

// *****************************************************************
//  Function: ScrollDownAttach
// 
//  In: (int * FileHighlighter, int * SkipEntries, 
//		 int WindowSizeY, char * CurrentPath, 
//		 int * FilesPresent, Messages * Queue, bool * ShowLatestMessages
//		 
//  
//  Out: bool
//
//	Use: Scrolls the bar 1 entry down in the file dialog
//       
// *****************************************************************
bool ScrollDownAttach (int * FileHighlighter, int * SkipEntries, int WindowSizeY, char * CurrentPath, int * FilesPresent, Messages * Queue, bool * ShowLatestMessages)
{
	if (*FileHighlighter < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -2) {
		*FileHighlighter += 1;
	} else {
		*SkipEntries +=1;
	}

	if (*FileHighlighter >= *FilesPresent) {
		*FileHighlighter -=1;
	}

	// THIS LIMITS THE SCROLLING TO THE BOTTOM IF OUR LIST IS LARGER THAN 1 SCREEN
	if (*FileHighlighter < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -2) {
	} else {
		if ((*SkipEntries + *FileHighlighter) >= *FilesPresent) {
			*SkipEntries -=1;
		}
	}
	return true;
}

// *****************************************************************
//  Function: DisplayFileDialog
// 
//  In: SDL_Surface * backbuffer
//		int WindowSizeX, WindowSizeY,
//		CurrentPath, FileHighlighter,
//		SkipEntries
//		bool ShowLatestMessages
//		Messages * Queue
//		RETBUFFER InFile
//		Uint32 c_SiceBorder, c_Black
//
//  Out: -
//
//	Use: Draws the file dialog to the backbuffer
//       
// *****************************************************************
static char FileNameLookup[MAX_PATH] = "";
int FileNamePos = 0;

void DisplayFileDialog (SDL_Surface * backbuffer, int WindowSizeX, int WindowSizeY,char * CurrentPath,int * FileHighlighter, int SkipEntries,bool * ShowLatestMessages, Messages * Queue, RETBUFFER InFile, Uint32 c_SiceBorder, Uint32 c_Black)
{
	Draw_FillRect (backbuffer,RIGHT_MARGEIN*5,RIGHT_MARGEIN*5,WindowSizeX-(RIGHT_MARGEIN*2*5),WindowSizeY-(RIGHT_MARGEIN*2*5),c_SiceBorder);
	BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,WindowSizeY-RIGHT_MARGEIN*6-4,0,0,0,"%s",FileNameLookup);
	// RETRIEVE DIRECTORY FILELIST
	if (GetCurrentDirectory (MAX_PATH-1,CurrentPath) != 0)	{
		FileList * DirectoryContent = InitFileList ();
		FileList * DirectoryContentTmp = DirectoryContent;

		bool bRes = GetDirectoryContent (DirectoryContent,CurrentPath);
		if (bRes == true) {

			// WRITE DIRECTORY NAME // SHORTEN IF NECESSARY
			char CurrentPathShortened[MAX_PATH+1];
			memset (&CurrentPathShortened,0,_countof(CurrentPathShortened));
			if (strlen (CurrentPath) > unsigned (((WindowSizeX-(RIGHT_MARGEIN*7)-2)-RIGHT_MARGEIN*6) / 8)) {
				PathCompactPathEx (CurrentPathShortened,CurrentPath, (((WindowSizeX-(RIGHT_MARGEIN*7)-2)-RIGHT_MARGEIN*6) / 8), NULL);
			} else (strcpy_s (CurrentPathShortened,_countof(CurrentPathShortened)-1,CurrentPath));
			BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,RIGHT_MARGEIN*6,0,0,0,"%s",CurrentPathShortened);

			BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,RIGHT_MARGEIN*7,0,0,0,"%c",2);
			BiosTextOut (&InFile,backbuffer,WindowSizeX-(RIGHT_MARGEIN*7)-2,RIGHT_MARGEIN*7,0,0,0,"%c",7);

			BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,WindowSizeY-(RIGHT_MARGEIN*7)-2,0,0,0,"%c",5);
			BiosTextOut (&InFile,backbuffer,WindowSizeX-(RIGHT_MARGEIN*7)-2,WindowSizeY-(RIGHT_MARGEIN*7)-2,0,0,0,"%c",6);

			for (int i = 0; i < ((WindowSizeX-(RIGHT_MARGEIN*6*2))) / (LINE_H-3)-2; i++)
			{
				BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*7+i*(LINE_H-3),RIGHT_MARGEIN*7,0,0,0,"%c",3);
				BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*7+i*(LINE_H-3),WindowSizeY-(RIGHT_MARGEIN*7)-2,0,0,0,"%c",3);
			}
			int CurrentEntry = 0;
			bool bSuccess = false;
			for (int i = 0; i < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -1 ; i++) {
				if (DirectoryContent != 0) {
					if (CurrentEntry >= SkipEntries) {
						if (strlen (FileNameLookup) > 0) {
							if (_strnicmp (FileNameLookup,DirectoryContent->Data.cFileName,strlen(FileNameLookup)) != NULL) {
							} else {
								if (bSuccess == false) *FileHighlighter = i;
								bSuccess = true;
							}
						}

						// CONVERT TIME
						SYSTEMTIME SystemTime;
						memset (&SystemTime,0,sizeof (SYSTEMTIME));
						FileTimeToSystemTime (&DirectoryContent->Data.ftCreationTime,&SystemTime);
						int R = 0, G = 0, B = 128;
						
						if (*FileHighlighter == i) {
							// DRAW HIGHLIGHTER
							Draw_FillRect (backbuffer,RIGHT_MARGEIN*8-2,(*FileHighlighter*(LINE_H-2))-1+RIGHT_MARGEIN*8,WindowSizeX-(RIGHT_MARGEIN*2*8),LINE_H,c_Black);
							R = 255; G = 255; B = 255;
						} 

						if (strlen (DirectoryContent->Data.cFileName) < 40) {
							if (DirectoryContent->Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
								BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*8,RIGHT_MARGEIN*8+i*(LINE_H-2),R,G,B,"%40s|%10s|%2.2d-%2.2d-%4.4d|%2.2d:%2.2d",DirectoryContent->Data.cFileName,"<DIR>",SystemTime.wDay,SystemTime.wMonth,SystemTime.wYear,SystemTime.wHour,SystemTime.wMinute);
							} else BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*8,RIGHT_MARGEIN*8+i*(LINE_H-2),R,G,B,"%40s|%10d|%2.2d-%2.2d-%4.4d|%2.2d:%2.2d",DirectoryContent->Data.cFileName,DirectoryContent->Data.nFileSizeLow,SystemTime.wDay,SystemTime.wMonth,SystemTime.wYear,SystemTime.wHour,SystemTime.wMinute);
						} else BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*8,RIGHT_MARGEIN*8+i*(LINE_H-2),R,G,B,"%40s|%10d|%2.2d-%2.2d-%4.4d|%2.2d:%2.2d",DirectoryContent->Data.cAlternateFileName,DirectoryContent->Data.nFileSizeLow,SystemTime.wDay,SystemTime.wMonth,SystemTime.wYear,SystemTime.wHour,SystemTime.wMinute);
					} else i--;
					CurrentEntry++;

					DirectoryContent = DirectoryContent->NextEntry;
				}
			}

			for (int i = 0; i < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*1) / (LINE_H-2)) -2 ; i++) {
				if (i != 0) BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,RIGHT_MARGEIN*7+i*(LINE_H-2),0,0,0,"%c",1);
				if (i != 0) BiosTextOut (&InFile,backbuffer,WindowSizeX-(RIGHT_MARGEIN*7)-2,RIGHT_MARGEIN*7+i*(LINE_H-2),0,0,0,"%c",1);
			}
		}
		DestroyFileList (DirectoryContentTmp);
	} else {
		*ShowLatestMessages = DisplayMessage (Queue,"UGDBG: failed to retrieve current directory");
	}

	return;
}


// *****************************************************************
//  Function: DisplayAttachDialog
// 
//  In: SDL_Surface * backbuffer
//		int WindowSizeX, WindowSizeY,
//		CurrentPath, FileHighlighter,
//		SkipEntries
//		bool ShowLatestMessages
//		Messages * Queue
//		RETBUFFER InFile
//		Uint32 c_SiceBorder, c_Black
//
//  Out: -
//
//	Use: Draws the file dialog to the backbuffer
//       
// *****************************************************************
void DisplayAttachDialog (SDL_Surface * backbuffer, int WindowSizeX, int WindowSizeY,int * ProcessesPresent,DWORD * RunningPIDs, int FileHighlighter, int SkipEntries,bool * ShowLatestMessages, Messages * Queue, RETBUFFER InFile, Uint32 c_SiceBorder, Uint32 c_Black)
{
	*ProcessesPresent = 0;
	Draw_FillRect (backbuffer,RIGHT_MARGEIN*5,RIGHT_MARGEIN*5,WindowSizeX-(RIGHT_MARGEIN*2*5),WindowSizeY-(RIGHT_MARGEIN*2*5),c_SiceBorder);
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    

	FileAttachList * FileAttachListActive = InitFileAttachList ();
	FileAttachList * FileAttachListActiveTmp = FileAttachListActive;
    
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
        return;

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

	memcpy (RunningPIDs,aProcesses,cProcesses * sizeof(DWORD));

    // Print the name and process identifier for each process.
	for (unsigned int i = 0; i < cProcesses; i++ ) {
		if( aProcesses[i] != 0 ) {
			TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
			// Get a handle to the process.
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
			// Get the process name.
			if (NULL != hProcess) {
				HMODULE hMod;
				DWORD cbNeeded;
				if (EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
					GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR));
				}
			}

			strcpy_s ((char *)FileAttachListActiveTmp->ProcessName,_countof(FileAttachListActiveTmp->ProcessName),szProcessName);
			FileAttachListActiveTmp->PID = aProcesses[i];
			FileAttachListActiveTmp->NextEntry = (FileAttachList *)GlobalAlloc (GMEM_ZEROINIT,sizeof (FileAttachList));
			if (FileAttachListActiveTmp->NextEntry == 0) return;
			FileAttachListActiveTmp = FileAttachListActiveTmp->NextEntry;
			*ProcessesPresent += 1;
			CloseHandle( hProcess );

		}
	}
	GlobalFree (FileAttachListActiveTmp->NextEntry);
	FileAttachListActiveTmp->NextEntry = 0;

	// DRAW HIGHLIGHTER
	Draw_FillRect (backbuffer,RIGHT_MARGEIN*8-2,(FileHighlighter*(LINE_H-2))-1+RIGHT_MARGEIN*8,WindowSizeX-(RIGHT_MARGEIN*2*8),LINE_H,c_Black);

	// SET TOP TEXT
	BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,RIGHT_MARGEIN*6,0,0,0,"Select Process to Debug - %d running processes found",*ProcessesPresent);

	BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,RIGHT_MARGEIN*7,0,0,0,"%c",2);
	BiosTextOut (&InFile,backbuffer,WindowSizeX-(RIGHT_MARGEIN*7)-2,RIGHT_MARGEIN*7,0,0,0,"%c",7);

	BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,WindowSizeY-(RIGHT_MARGEIN*7)-2,0,0,0,"%c",5);
	BiosTextOut (&InFile,backbuffer,WindowSizeX-(RIGHT_MARGEIN*7)-2,WindowSizeY-(RIGHT_MARGEIN*7)-2,0,0,0,"%c",6);

	for (int i = 0; i < ((WindowSizeX-(RIGHT_MARGEIN*6*2))) / (LINE_H-3)-2; i++)
	{
		BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*7+i*(LINE_H-3),RIGHT_MARGEIN*7,0,0,0,"%c",3);
		BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*7+i*(LINE_H-3),WindowSizeY-(RIGHT_MARGEIN*7)-2,0,0,0,"%c",3);
	}
	int CurrentEntry = 0;
	FileAttachListActiveTmp = FileAttachListActive;
	for (int i = 0; i < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -1 ; i++) {
		if ((FileAttachListActiveTmp != 0) && (i < *ProcessesPresent)) {
			if (CurrentEntry >= SkipEntries) {
				int R = 0, G = 0, B = 128;
			
				if (FileHighlighter == i) {
					R = 255; G = 255; B = 255;
				} 

				if (strlen ((const char *)FileAttachListActiveTmp->ProcessName) < 40) {
					BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*8,RIGHT_MARGEIN*8+i*(LINE_H-2),R,G,B,"%40s %10s%4x",FileAttachListActiveTmp->ProcessName,"PID:",FileAttachListActiveTmp->PID);
				}
			} else i--;
			CurrentEntry++;

			FileAttachListActiveTmp = FileAttachListActiveTmp->NextEntry;
		}
	}
	for (int i = 0; i < (((WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*1) / (LINE_H-2)) -2 ; i++) {
		if (i != 0) BiosTextOut (&InFile,backbuffer,RIGHT_MARGEIN*6,RIGHT_MARGEIN*7+i*(LINE_H-2),0,0,0,"%c",1);
		if (i != 0) BiosTextOut (&InFile,backbuffer,WindowSizeX-(RIGHT_MARGEIN*7)-2,RIGHT_MARGEIN*7+i*(LINE_H-2),0,0,0,"%c",1);
	}
	DestroyFileList ((FileList *)FileAttachListActive);

	return;
}


// *****************************************************************
//  Function: HandleFileDialog
// 
//  In: heh see below 
//
//  Out: -
//
//	Use: Handles all the keyboard input + logic from the file dialog
//       
// *****************************************************************
void HandleFileDialog (SDL_Event * ereignis, char * CurrentPath, SDL_mutex * FileDialogue, int * FilesPresent, int * SkipEntries, int * FileHighlighter,
					   DebugStruc * DebugDataExchange, RETBUFFER * Debuggee, int * WindowSizeX, int * WindowSizeY, Messages * Queue, bool * ShowLatestMessages, ULONG * FileDialoguePtr)
{
	//***************************************
	//
	// HERE WE HANDLE THE USER INTERFACE INPUT - FILE OPEN DIALOG
	//
	//***************************************
	switch (ereignis->type) 
	{
		case SDL_KEYDOWN:

			if (ereignis->key.keysym.sym == SDLK_BACKSPACE) {
				if (FileNamePos > 0) {
					char Temp[MAX_PATH];
					strcpy_s (Temp,_countof(Temp),&FileNameLookup[FileNamePos]);
					FileNameLookup[FileNamePos-1] = 0;
					strcat_s (FileNameLookup,_countof(FileNameLookup),Temp);
					if (FileNamePos > 0) FileNamePos --;
				}
				return;
			}

			if (ereignis->key.keysym.sym != SDLK_RETURN) {
				if (strlen (FileNameLookup) < 78) {
					char ToAdd[2]="";
					char Temp[MAX_PATH];
					char ch = ereignis->key.keysym.unicode & 0x7f;
					if (isalnum(ch) != NULL) {
						size_t OldValue = strlen (FileNameLookup);

						strcpy_s (Temp,_countof(Temp),&FileNameLookup[FileNamePos]);
						strncpy_s (ToAdd,_countof(ToAdd),&ch,1);
						FileNameLookup[FileNamePos] = 0;
						strcat_s (FileNameLookup,_countof(FileNameLookup),ToAdd);
						strcat_s (FileNameLookup,_countof(FileNameLookup),Temp);

						if (strlen(FileNameLookup) > OldValue)	FileNamePos ++;
					}
				}
			}

			// SWITCH DRIVES
			if (ereignis->key.keysym.mod & KMOD_ALT) {
				char * Key = SDL_GetKeyName (ereignis->key.keysym.sym);
				if ((*Key >= 0x61) && (*Key <= 0x7a)) {
					strcpy_s (CurrentPath,_MAX_PATH-1,Key);
					strcat_s (CurrentPath,_MAX_PATH-1,":\\");
					SetCurrentDirectory (CurrentPath);
					GetCurrentDirectory (_MAX_PATH-1,CurrentPath);
				}
				break;
			}


			if (ereignis->key.keysym.sym == SDLK_ESCAPE) {
				SDL_DestroyMutex (FileDialogue);
				*FileDialoguePtr = 0;
				break;
			}

			// RETRIEVE SELECTION
			if (ereignis->key.keysym.sym == SDLK_RETURN) {
				FileList * DirectoryContent = InitFileList ();
				FileList * DirectoryContentTmp = DirectoryContent;
				if (GetCurrentDirectory (_MAX_PATH-1,CurrentPath) != 0)	{
					bool bRes = GetDirectoryContent (DirectoryContent, CurrentPath);
					*FilesPresent = 0;
					if (bRes == true) {
						while (DirectoryContent != 0) {
							if (*FilesPresent >= *SkipEntries) {
								if (*FilesPresent == (*FileHighlighter + *SkipEntries)) {
									if (DirectoryContent->Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	{
										strcat_s (CurrentPath,_MAX_PATH,"\\");
										strcat_s (CurrentPath,_MAX_PATH,DirectoryContent->Data.cFileName);
										SetCurrentDirectory (CurrentPath);
										GetCurrentDirectory (_MAX_PATH-1,CurrentPath);
										*SkipEntries = 0;
										*FileHighlighter = 0;
									} else {
										CleanupDia ();
										// TERMINATE CURRENTLY ACTIVE PROCESS!
										SDL_KillThread (DebugDataExchange->Thread);
										Cleanup (DebugDataExchange);
										memset (DebugDataExchange,0,sizeof(DebugStruc));

										SDL_DestroyMutex (FileDialogue);
										*FileDialoguePtr = 0;
										// FIRE DEBUGGER CORE
										char File[MAX_PATH+1] = "";
										strcpy_s (File,_countof(File)-1,CurrentPath);
										strcat_s (File,_countof(File)-1,"\\");
										strcat_s (File,_countof(File)-1,DirectoryContent->Data.cFileName);

										DebugDataExchange->DisassembleFromOffset = 0;
										memset (DebugDataExchange,0,sizeof (DebugStruc));
										DebugDataExchange->WindowSizeX = WindowSizeX;
										DebugDataExchange->WindowSizeY = WindowSizeY;
										DebugDataExchange->AllowScrolling = true;
										InitializeCore (File,NULL,Debuggee,Queue,ShowLatestMessages,DebugDataExchange);
									}
								}
							}
							*FilesPresent +=1;
							DirectoryContent = DirectoryContent->NextEntry;
						}
					}
				} else {
					*ShowLatestMessages = DisplayMessage (Queue,"UGDBG: failed to retrieve current directory");
				}
				DestroyFileList (DirectoryContentTmp);
				break;
			}

			// SCROLL BAR DOWN
			if (ereignis->key.keysym.sym == SDLK_DOWN) {
				ScrollDown (FileHighlighter, SkipEntries, *WindowSizeY, CurrentPath, FilesPresent, Queue, ShowLatestMessages);
				break;
			}

			// SCROLL BAR UP
			if (ereignis->key.keysym.sym == SDLK_UP)	{
				ScrollUp (FileHighlighter,SkipEntries);
				break;
			}
			
			// SCROLL BAR UP BY 1 PAGE
			if (ereignis->key.keysym.sym == SDLK_PAGEUP)	{
				for (int i = 0; i < (((*WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -2; i++)	{
					ScrollUp (FileHighlighter,SkipEntries);
				}
				break;
			}

			// SCROLL BAR DOWN BY 1 PAGE
			if (ereignis->key.keysym.sym == SDLK_PAGEDOWN)	{
				for (int i = 0; i <  (((*WindowSizeY-(RIGHT_MARGEIN*2*5))-RIGHT_MARGEIN*4) / (LINE_H-2)) -2; i++) {
					ScrollDown (FileHighlighter, SkipEntries, *WindowSizeY, CurrentPath, FilesPresent, Queue, ShowLatestMessages);
				}
				break;
			}


			break;
	}
}
