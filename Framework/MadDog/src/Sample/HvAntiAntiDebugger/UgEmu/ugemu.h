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
// CUSTOM OPTIONS
bool instructionHex = true;

// Variables
int WindowSizeX = 660;
int WindowSizeY = 660;

SDL_Surface *screen;
SDL_Surface *backbuffer;
SDL_Surface *grey_surface;
SDL_Surface *load_button;

int Fader = 0;
SDL_mutex *FileDialogue = 0;
SDL_mutex *AttachDialogue = 0;

int Cursor_Interval = 200;
// this indicates the current cursor workmode (register/inputwindow/data)
int Cursor_Region = 0;

CONTEXT CTX;
CONTEXT PREVIOUS_CTX;

RETBUFFER InFile;
RETBUFFER Debuggee;

HANDLE CurrentThread;

char inputfile[MAX_PATH+1];
char outputfile[MAX_PATH+1];

int FileHighlighter = 0;
int FilesPresent = 0;
int ProcessHighlighter = 0;
int ProcessesPresent = 0;
DWORD RunningPIDs[1024];

int SkipEntries = 0;
int SkipEntriesProcesses = 0;

bool KeepTracing = false;

char CurrentPath[MAX_PATH+1];
int ScreenshotCounter = 0;
char ScreenshotFilename[MAX_PATH+1];

//*****************************************
// WORKING MODE 64/32Bit
bool x64BitMode = false; 
//*****************************************

	// Default decoding mode is 32 bits, could be set by command line.
#ifdef b32BitBuild
	_DecodeType dt = Decode32Bits;
#else
	_DecodeType dt = Decode64Bits;
#endif


//*****************************************
// LOGTEXT WINDOW RELATED
ULONG ShowLogMessagesStartingByNumber = 1;
bool ShowLatestMessages;
//*****************************************

bool MouseButtonPressed = false;
bool MouseButtonPressedLog = false;

//*****************************************
// DISPLAY OFFSETS
int DATA_Y_POS = 42;
int LOGTEXT_Y_POS = 506;
int REGISTERS_Y_POS = 7;
int DISASSEMBLY_Y_POS = 200;
int INPUTWINDOW_Y_POS = 610+26;
int LINE_UPPER = DATA_Y_POS - 4;
int LINE_MIDDLE = 194;
int LINE_LOWER = 502;
//*****************************************

DebugStruc DebugDataExchange;
