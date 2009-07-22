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
#include <windows.h>
#include <shlwapi.h>
#include <psapi.h>

#ifdef b32BitBuild
#include ".\\SDL-1.2.13\\include\\SDL.h"
#include ".\\SDL-1.2.13\\include\\SDL_thread.h"
#include ".\\SDL-1.2.13\\include\\SDL_mutex.h"
#include ".\\SDL-1.2.13\\include\\SDL_endian.h"
#include ".\\SDL_draw-1.2.13\\include\\SDL_draw.h"
#include ".\\SDL-1.2.13\\include\\SDL_syswm.h"
#include ".\\distorm64\\disasm\\distorm.h"
// Link the library into our project.
#pragma comment(lib, ".\\distorm64\\disasm\\distorm.lib")
#else
#include ".\\SDL-1.2.13_x64\\include\\SDL.h"
#include ".\\SDL-1.2.13_x64\\include\\SDL_thread.h"
#include ".\\SDL-1.2.13_x64\\include\\SDL_mutex.h"
#include ".\\SDL-1.2.13_x64\\include\\SDL_endian.h"
#include ".\\SDL_draw-1.2.13\\include\\SDL_draw.h"
#include ".\\SDL-1.2.13_x64\\include\\SDL_syswm.h"
#include ".\\distorm64\\disasm\\distorm.h"
// Link the library into our project.
#pragma comment(lib, ".\\distorm64\\disasm\\distorm_x64.lib")
#endif

#define LINE_H 10
#define OVERFLOW 11
#define DIRECTION 10
#define INTERRUPT 9
#define SIGN 7
#define ZERO 6
#define ADJUST 4
#define PARITY 2
#define CARRY 0
#define RIGHT_MARGEIN 7
#define SIZE_OF_HEXBUFF 0x100

// The number of the array of instructions the decoder function will use to return the disassembled instructions.
// Play with this value for performance...
#define MAX_INSTRUCTIONS (1000)

typedef struct RETBUFFER {
	BYTE * FileOffset;
	DWORD FileSize;
	DWORD ErrCode;
	DWORD Bread;
	DWORD SpecialAddSize;
	char FileName[_MAX_PATH];
} RETBUFFER, *PRETBUFFER;

typedef struct FileList {
	FileList * NextEntry;
	WIN32_FIND_DATA Data;
} FileList, *PFileList;

typedef struct FileAttachList {
	FileAttachList * NextEntry;
	DWORD PID;
	unsigned char ProcessName[MAX_PATH+1];
} FileAttachList, *PFileAttachList;

typedef struct Messages {
	Messages * NextEntry;
	int MessageNr;
	unsigned char Message[128];
} Messages, *PMessages;

typedef struct Font {
	BYTE Font1;
	BYTE Font2;
	BYTE Font3;
	BYTE Font4;
	BYTE Font5;
	BYTE Font6;
	BYTE Font7;
	BYTE Font8;
} Font, *PFont;


extern unsigned char BpxBuffer[0x5000];

typedef struct BreakpointTable
{
#ifdef b32BitBuild
	ULONG Offset;
#else
	ULONGLONG Offset;
#endif
	BYTE OrigByte;
	BOOL Enabled;
	ULONG Type;							// BPX / BPM
	ULONG Type2;						// R|W|X
	bool F2Type;						// Indicate deletion on HIT
} BreakpointTable, *PBreakpointTable;


typedef struct ApiTable
{
#ifdef b32BitBuild
	ULONG Offset;
#else
	ULONGLONG Offset;
#endif
	unsigned char ApiAscii[MAX_PATH+1];
} ApiTable, *PApiTable;

typedef struct DllTable
{
	ULONG FunctionsAvailable;
	unsigned char DllAscii[MAX_PATH+1];
	FARPROC DebugEventCallback;
	PApiTable Apis;
} DllTable, *PDllTable;

#ifdef b32BitBuild
// DEFINE SOME STUFF NEEDED FOR OUR DEBUGGER I386/
typedef struct DebugStruc {
	RETBUFFER Debuggee;
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	DEBUG_EVENT DbgEvent;
	CONTEXT * CTX;
	char FileName[MAX_PATH];
	Messages * Queue;
	bool * ShowLatestMessages;
	DWORD ImageSize;
	DWORD ImageBase;
	DWORD dwProcessId;
	DWORD Entrypoint;
	bool PollEvent;
	SDL_Thread * Thread;
	int * WindowSizeX;
	int * WindowSizeY;
	ULONG DisassembleFromOffset;
	ULONG DisplayMemoryOffset;
	bool AllowScrolling;
	bool AllowInt1;
	bool Active;
	bool RunTraceActive;
	bool BpmTrigger;
	bool PDBLoaded;
	DllTable * DllTablePtr;
} DebugStruc, *PDebugStruc;
#else
// DEFINE SOME STUFF NEEDED FOR OUR DEBUGGER AMD64/
typedef struct DebugStruc {
	RETBUFFER Debuggee;
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	DEBUG_EVENT DbgEvent;
	CONTEXT * CTX;
	char FileName[MAX_PATH];
	Messages * Queue;
	bool * ShowLatestMessages;
	DWORD ImageSize;
	DWORD64 ImageBase;
	DWORD dwProcessId;
	ULONGLONG Entrypoint;
	bool PollEvent;
	SDL_Thread * Thread;
	int * WindowSizeX;
	int * WindowSizeY;
	ULONGLONG DisassembleFromOffset;
	ULONGLONG DisplayMemoryOffset;
	bool AllowScrolling;
	bool AllowInt1;
	bool Active;
	bool RunTraceActive;
	bool BpmTrigger;
	bool PDBLoaded;
	DllTable * DllTablePtr;
} DebugStruc, *PDebugStruc;
#endif

typedef struct config_result {
	bool IsInteger;
	int Value;
	char String[_MAX_PATH];
} config_result, *Pconfig_result;

typedef struct AutoStruc {
	bool Executed;
	char AutoExecString[_MAX_PATH];
} AutoStruc, *PAutoStruc;

extern AutoStruc AutoExec;

typedef struct SDK_Result {
		ULONG dwContinueStatus;
		bool WaitForInput;
} SDK_Result, *PSDK_Result;

extern bool instructionHex;

extern char Command[500][50];
extern char Syntax[_countof(Command)][30];
extern char Registers[42][4];
extern char HelpText[_countof(Command)][50];
extern unsigned char PluginTableBuffer[0x1000];
extern char Options[4][20];
extern bool OptionSettings[_countof(Options)];
extern bool BreakFound;

typedef struct StringList {
	char Line[128];
} StringList, *PStringList;

typedef struct SourceFileList {
	RETBUFFER File;
	StringList * StringListPtr;
} SourceFileList, *PSourceFileList;

typedef void (__cdecl *SDK)(DebugStruc * DebugDataExchange, int argc, char * argv[]);
typedef char * (__cdecl *SDK_Char)(DebugStruc * DebugDataExchange, int argc, char * argv[]);
typedef ULONG (__cdecl *SDK_DebugEventCallback)(DebugStruc * DebugDataExchange, const LPDEBUG_EVENT DebugEv, SDK_Result * SDK_Res);
typedef DWORD (__cdecl *SDK_PEiD) (HWND hMainDlg, char *szFname, DWORD lpReserved, LPVOID lpParam);

extern bool GoActive;
void InitCharTable (RETBUFFER * InFile,SDL_Surface *screen);

extern "C" __declspec(dllexport) int bit_tst( unsigned long *flags, int bit );
extern "C" __declspec(dllexport) void bit_set( unsigned long *flags, int bit );
extern "C" __declspec(dllexport) void bit_clr( unsigned long *flags, int bit );
extern "C" __declspec(dllexport) void ReEnableBreakpoints (const LPDEBUG_EVENT DebugEv,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) int  ReadFileMem (char * FileName,RETBUFFER * tescht);
extern "C" __declspec(dllexport) int  WriteFileMem (char * FileName,RETBUFFER * tescht);
extern "C" __declspec(dllexport) void BiosTextOut (RETBUFFER * InFile,SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G,Uint8 B,char * Text,...);
extern "C" __declspec(dllexport) void CenterWindow (HWND hwnd, int WindowSizeX, int WindowSizeY);
extern "C" __declspec(dllexport) void MoveDebuggerWindow (HWND hwnd, int X, int Y, int WindowSizeX, int WindowSizeY);
extern "C" __declspec(dllexport) void UpdateRegisters (SDL_Surface *screen,CONTEXT * CTX, CONTEXT * CTX_Compare, RETBUFFER InFile, int REGISTERS_Y_POS, bool x64BitMode);
#ifdef b32BitBuild
extern "C" __declspec(dllexport) ULONG Evaluate (char * str,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) char * PerformVALookup (ULONG Address,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) BYTE SetInt3BreakPoint (ULONG Offset,PROCESS_INFORMATION * PI);
extern "C" __declspec(dllexport) ULONG FetchOpcodeSize (ULONG Offset, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) bool ReadMem (ULONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI);
extern "C" __declspec(dllexport) void UpdateDisassembly (SDL_Surface *screen, CONTEXT * CTX, ULONG Offset, _DecodeType dt,bool instructionHex,RETBUFFER InFile, int DISASSEMBLY_Y_POS, int LOGTEXT_Y_POS, bool x64BitMode, DebugStruc * DebugDataExchange, int WindowSizeX);
extern "C" __declspec(dllexport) void UpdateMemory (SDL_Surface *screen, ULONG Offset,PROCESS_INFORMATION * PI, CONTEXT *CTX, RETBUFFER InFile, int DATA_Y_POS, int DISASSEMBLY_Y_POS, bool x64BitMode, int WindowSizeX);
extern "C" __declspec(dllexport) void EnumerateSectionOffsetFromData (SDL_Surface * screen,ULONG Offset,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX);
extern "C" __declspec(dllexport) char * FetchOpcodeAscii (ULONG Offset, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) bool WriteMem (ULONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI);
extern "C" __declspec(dllexport) _DecodedInst * FetchOpcode (ULONG Offset, DebugStruc * DebugDataExchange);
#else
extern "C" __declspec(dllexport) ULONGLONG Evaluate (char * str,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) char * PerformVALookup (ULONGLONG Address,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) BYTE SetInt3BreakPoint (ULONGLONG Offset,PROCESS_INFORMATION * PI);
extern "C" __declspec(dllexport) ULONGLONG FetchOpcodeSize (ULONGLONG Offset, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) bool ReadMem (ULONGLONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI);
extern "C" __declspec(dllexport) void UpdateDisassembly (SDL_Surface *screen, CONTEXT * CTX, ULONGLONG Offset, _DecodeType dt,bool instructionHex,RETBUFFER InFile, int DISASSEMBLY_Y_POS, int LOGTEXT_Y_POS, bool x64BitMode, DebugStruc * DebugDataExchange, int WindowSizeX);
extern "C" __declspec(dllexport) void UpdateMemory (SDL_Surface *screen, ULONGLONG Offset,PROCESS_INFORMATION * PI, CONTEXT *CTX, RETBUFFER InFile, int DATA_Y_POS, int DISASSEMBLY_Y_POS, bool x64BitMode, int WindowSizeX);
extern "C" __declspec(dllexport) void EnumerateSectionOffsetFromData (SDL_Surface * screen,ULONGLONG Offset,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX);
extern "C" __declspec(dllexport) char * FetchOpcodeAscii (ULONGLONG Offset, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) bool WriteMem (ULONGLONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI);
extern "C" __declspec(dllexport) _DecodedInst * FetchOpcode (ULONGLONG Offset, DebugStruc * DebugDataExchange);
#endif
extern "C" __declspec(dllexport) Messages * InitQueue ();
extern "C" __declspec(dllexport) bool DisplayMessage (Messages * Queue, char * Message,...);
extern "C" __declspec(dllexport) void UpdateLogWindow (RETBUFFER InFile,SDL_Surface *screen,Messages * Queue,int StartMessage, int LOGTEXT_Y_POS, int INPUTWINDOW_Y_POS, DebugStruc * DebugDataExchange, int WindowSizeX);
extern "C" __declspec(dllexport) FileList * InitFileList ();
extern "C" __declspec(dllexport) bool GetDirectoryContent(FileList * DirectoryContent,char * CurrentPath);
extern "C" __declspec(dllexport) bool DestroyFileList (FileList * DirectoryContent);
extern "C" __declspec(dllexport) bool DestroyQueue (Messages * Queue);
extern "C" __declspec(dllexport) void DisplayFileDialog (SDL_Surface * backbuffer, int WindowSizeX, int WindowSizeY,char * CurrentPath,int * FileHighlighter, int SkipEntries,bool * ShowLatestMessages, Messages * Queue, RETBUFFER InFile, Uint32 c_SiceBorder, Uint32 c_Black);
extern "C" __declspec(dllexport) bool ScrollUp (int * FileHighlighter, int * SkipEntries);
extern "C" __declspec(dllexport) bool ScrollDown (int * FileHighlighter, int * SkipEntries, int WindowSizeY, char * CurrentPath, int * FilesPresent, Messages * Queue, bool * ShowLatestMessages);
extern "C" __declspec(dllexport) bool ScrollUpAttach (int * FileHighlighter, int * SkipEntries);
extern "C" __declspec(dllexport) bool ScrollDownAttach (int * FileHighlighter, int * SkipEntries, int WindowSizeY, char * CurrentPath, int * FilesPresent, Messages * Queue, bool * ShowLatestMessages);
extern "C" __declspec(dllexport) bool InitializeCore (char * FileName, DWORD dwPID, RETBUFFER * Debuggee, Messages * Queue, bool * ShowLatestMessages,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) void EnterDebugLoop(const LPDEBUG_EVENT DebugEv,PROCESS_INFORMATION * PI, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) int EnableTrapflag (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) int DisableTrapflag (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) void StepOver (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) void ClearHwBreakPoint (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) void DisplayAttachDialog (SDL_Surface * backbuffer, int WindowSizeX, int WindowSizeY,int * ProcessesPresent,DWORD * RunningPIDs,int FileHighlighter, int SkipEntries,bool * ShowLatestMessages, Messages * Queue, RETBUFFER InFile, Uint32 c_SiceBorder, Uint32 c_Black);
extern "C" __declspec(dllexport) void EnumerateSectionOffset (SDL_Surface * screen,CONTEXT * CTX,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX);
extern "C" __declspec(dllexport) void HandleKeyboardInputCursor (RETBUFFER * InFile, SDL_Event * event,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) void Draw_InputBuffer (RETBUFFER * InFile,SDL_Surface * screen,int WindowSizeY,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) void Draw_Cursor (SDL_Surface * screen,int WindowSizeY,RETBUFFER * InFile,int Interval);
extern "C" __declspec(dllexport) int CommandParser (RETBUFFER * InFile,DebugStruc * DebugDataExchange,char * InputBuffer);
extern "C" __declspec(dllexport) char * stringReplace(char *search, char *replace, char *string);
extern "C" __declspec(dllexport) bool ReadContext (CONTEXT * CTX,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) bool WriteContext (CONTEXT * CTX,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) bool LoadConfig (char * FileName, char * Entry, config_result * result);
extern "C" __declspec(dllexport) bool SaveConfig (char * FileName, char * Entry, config_result * result);
extern "C" __declspec(dllexport) bool ReLoadConfig (char * FileName);
extern "C" __declspec(dllexport) bool ReSaveConfig (char * FileName);
extern "C" __declspec(dllexport) void SetVideoMode (Messages * Queue);
extern "C" __declspec(dllexport) void Cleanup (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) void HandleFileDialog (SDL_Event * ereignis, char * CurrentPath, SDL_mutex * FileDialogue, int * FilesPresent, int * SkipEntries, int * FileHighlighter, DebugStruc * DebugDataExchange, RETBUFFER * Debuggee, int * WindowSizeX, int * WindowSizeY, Messages * Queue, bool * ShowLatestMessages, ULONG * FileDialoguePtr);
extern "C" __declspec(dllexport) int DumpFix (unsigned char * Offset,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) bool BuildPluginApiTree (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) void DisplaySyntax (char * CommandName, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllexport) bool GetOption (char * CommandName);
extern "C" __declspec(dllexport) RECT * GetScreenPos (HWND hwnd);
extern "C" __declspec(dllexport) int GetCurrentWidth ();
extern "C" __declspec(dllexport) int GetLogYPos ();
extern "C" __declspec(dllexport) int GetDisassemblyYPos ();
extern "C" __declspec(dllexport) int GetDataYPos ();
extern "C" __declspec(dllexport) int GetInputYPos ();
extern "C" __declspec(dllexport) int GetCurrentHeight ();
extern "C" __declspec(dllexport) _DecodeType GetDt ();
extern "C" __declspec(dllexport) bool OpenPdb(wchar_t *FileName);
extern "C" __declspec(dllexport) void CleanupDia();
extern "C" __declspec(dllexport) bool FindSymbolByRva(ULONG RVA, char * OutString, size_t OutLength);
extern "C" __declspec(dllexport) ULONG FindSymbolByName(wchar_t * InString);
extern "C" __declspec(dllexport) bool CopyQueueToClipboard (Messages * Queue);
extern "C" __declspec(dllexport) ULONG FindLineByRva (ULONG RVA, char * OutString, size_t OutLength);
extern "C" __declspec(dllexport) bool TextFile_getNumberofLines (char * FileName, ULONG * LineCounter);
extern "C" __declspec(dllexport) bool TextFile_ReadFile (char * FileName, StringList * StringListPtr);
extern "C" __declspec(dllexport) bool GetHomePath (char * OutString, size_t len);
extern "C" __declspec(dllexport) void HighlightCode (Uint8 * R, Uint8 * G, Uint8 * B, char * Mnemnomnic);
extern "C" __declspec(dllexport) BOOL RegisterSelf (char * Value);
extern "C" __declspec(dllexport) ULONG ProcSpeedRead();
extern "C" __declspec(dllexport) BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey);
extern "C" __declspec(dllexport) char * ProcessorInfo(char * Id);
extern "C" __declspec(dllexport) BOOL IsDoubleClick (void);
extern "C" __declspec(dllexport) void SetCurrentWidth (int Size);
extern "C" __declspec(dllexport) void SetCurrentHeight (int Size);
extern "C" __declspec(dllexport) void SetLogYPos (int Size);
extern "C" __declspec(dllexport) void SetDataYPos (int Size);
extern "C" __declspec(dllexport) void SetDisassemblyYPos (int Size);
extern "C" __declspec(dllexport) void Draw_Cursor_Data (SDL_Surface * screen, RETBUFFER * InFile,int Interval);
extern "C" __declspec(dllexport) void HandleKeyboardInputCursorData (RETBUFFER * InFile, SDL_Event * event,DebugStruc * DebugDataExchange);
extern char FileNameLookup[MAX_PATH];

