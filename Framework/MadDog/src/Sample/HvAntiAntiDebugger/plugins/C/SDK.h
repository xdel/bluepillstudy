// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SDK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SDK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SDK_EXPORTS
#define SDK_API extern "C" __declspec(dllexport)
#else
#define SDK_API __declspec(dllexport)
#endif

typedef struct RETBUFFER {
	BYTE * FileOffset;
	DWORD FileSize;
	DWORD ErrCode;
	DWORD Bread;
	DWORD SpecialAddSize;
	char FileName[_MAX_PATH];
} RETBUFFER, *PRETBUFFER;

typedef struct Messages {
	Messages * NextEntry;
	int MessageNr;
	unsigned char Message[128];
} Messages, *PMessages;

#ifdef b32BitBuild
typedef struct ApiTable
{
	ULONG Offset;
	unsigned char ApiAscii[MAX_PATH+1];
} ApiTable, *PApiTable;
#else
typedef struct ApiTable
{
	ULONGLONG Offset;
	unsigned char ApiAscii[MAX_PATH+1];
} ApiTable, *PApiTable;
#endif
typedef struct DllTable
{
	ULONG FunctionsAvailable;
	unsigned char DllAscii[MAX_PATH+1];
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

typedef struct SDK_Result {
		ULONG dwContinueStatus;
		bool WaitForInput;
} SDK_Result, *PSDK_Result;

extern "C" __declspec(dllimport) int bit_tst( unsigned long *flags, int bit );
extern "C" __declspec(dllimport) void bit_set( unsigned long *flags, int bit );
extern "C" __declspec(dllimport) void bit_clr( unsigned long *flags, int bit );
extern "C" __declspec(dllimport) void ReEnableBreakpoints (const LPDEBUG_EVENT DebugEv,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) int  ReadFileMem (char * FileName,RETBUFFER * tescht);
extern "C" __declspec(dllimport) int  WriteFileMem (char * FileName,RETBUFFER * tescht);
extern "C" __declspec(dllimport) void BiosTextOut (RETBUFFER * InFile,SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G,Uint8 B,char * Text,...);
extern "C" __declspec(dllimport) void CenterWindow (HWND hwnd, int WindowSizeX, int WindowSizeY);
extern "C" __declspec(dllimport) void MoveDebuggerWindow (HWND hwnd, int X, int Y, int WindowSizeX, int WindowSizeY);
extern "C" __declspec(dllimport) void UpdateRegisters (SDL_Surface *screen,CONTEXT * CTX, CONTEXT * CTX_Compare, RETBUFFER InFile, int REGISTERS_Y_POS, bool x64BitMode);
#ifdef b32BitBuild
extern "C" __declspec(dllimport) ULONG Evaluate (char * str,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) char * PerformVALookup (ULONG Address,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) BYTE SetInt3BreakPoint (ULONG Offset,PROCESS_INFORMATION * PI);
extern "C" __declspec(dllimport) ULONG FetchOpcodeSize (ULONG Offset, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) bool ReadMem (ULONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI);
//extern "C" __declspec(dllimport) void UpdateDisassembly (SDL_Surface *screen, CONTEXT * CTX, ULONG Offset, _DecodeType dt,bool instructionHex,RETBUFFER InFile, int DISASSEMBLY_Y_POS, int LOGTEXT_Y_POS, bool x64BitMode, DebugStruc * DebugDataExchange, int WindowSizeX);
extern "C" __declspec(dllimport) void UpdateMemory (SDL_Surface *screen, ULONG Offset,PROCESS_INFORMATION * PI, CONTEXT *CTX, RETBUFFER InFile, int DATA_Y_POS, int DISASSEMBLY_Y_POS, bool x64BitMode, int WindowSizeX);
extern "C" __declspec(dllimport) void EnumerateSectionOffsetFromData (SDL_Surface * screen,ULONG Offset,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX);
extern "C" __declspec(dllimport) char * FetchOpcodeAscii (ULONG Offset, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) bool WriteMem (ULONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI);
//extern "C" __declspec(dllimport) _DecodedInst * FetchOpcode (ULONG Offset, DebugStruc * DebugDataExchange);
#else
extern "C" __declspec(dllimport) ULONGLONG Evaluate (char * str,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) char * PerformVALookup (ULONGLONG Address,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) BYTE SetInt3BreakPoint (ULONGLONG Offset,PROCESS_INFORMATION * PI);
extern "C" __declspec(dllimport) ULONGLONG FetchOpcodeSize (ULONGLONG Offset, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) bool ReadMem (ULONGLONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI);
//extern "C" __declspec(dllimport) void UpdateDisassembly (SDL_Surface *screen, CONTEXT * CTX, ULONGLONG Offset, _DecodeType dt,bool instructionHex,RETBUFFER InFile, int DISASSEMBLY_Y_POS, int LOGTEXT_Y_POS, bool x64BitMode, DebugStruc * DebugDataExchange, int WindowSizeX);
extern "C" __declspec(dllimport) void UpdateMemory (SDL_Surface *screen, ULONGLONG Offset,PROCESS_INFORMATION * PI, CONTEXT *CTX, RETBUFFER InFile, int DATA_Y_POS, int DISASSEMBLY_Y_POS, bool x64BitMode, int WindowSizeX);
extern "C" __declspec(dllimport) void EnumerateSectionOffsetFromData (SDL_Surface * screen,ULONGLONG Offset,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX);
extern "C" __declspec(dllimport) char * FetchOpcodeAscii (ULONGLONG Offset, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) bool WriteMem (ULONGLONG Offset,ULONG Size,void * lpBuf,PROCESS_INFORMATION * PI);
//extern "C" __declspec(dllimport) _DecodedInst * FetchOpcode (ULONGLONG Offset, DebugStruc * DebugDataExchange);
#endif
extern "C" __declspec(dllimport) Messages * InitQueue ();
extern "C" __declspec(dllimport) bool DisplayMessage (Messages * Queue, char * Message,...);
extern "C" __declspec(dllimport) void UpdateLogWindow (RETBUFFER InFile,SDL_Surface *screen,Messages * Queue,int StartMessage, int LOGTEXT_Y_POS, int INPUTWINDOW_Y_POS, DebugStruc * DebugDataExchange, int WindowSizeX);
//extern "C" __declspec(dllimport) FileList * InitFileList ();
//extern "C" __declspec(dllimport) bool GetDirectoryContent(FileList * DirectoryContent,char * CurrentPath);
//extern "C" __declspec(dllimport) bool DestroyFileList (FileList * DirectoryContent);
extern "C" __declspec(dllimport) bool DestroyQueue (Messages * Queue);
extern "C" __declspec(dllimport) void DisplayFileDialog (SDL_Surface * backbuffer, int WindowSizeX, int WindowSizeY,char * CurrentPath,int FileHighlighter, int SkipEntries,bool * ShowLatestMessages, Messages * Queue, RETBUFFER InFile, Uint32 c_SiceBorder, Uint32 c_Black);
extern "C" __declspec(dllimport) bool ScrollUp (int * FileHighlighter, int * SkipEntries);
extern "C" __declspec(dllimport) bool ScrollDown (int * FileHighlighter, int * SkipEntries, int WindowSizeY, char * CurrentPath, int * FilesPresent, Messages * Queue, bool * ShowLatestMessages);
extern "C" __declspec(dllimport) bool ScrollUpAttach (int * FileHighlighter, int * SkipEntries);
extern "C" __declspec(dllimport) bool ScrollDownAttach (int * FileHighlighter, int * SkipEntries, int WindowSizeY, char * CurrentPath, int * FilesPresent, Messages * Queue, bool * ShowLatestMessages);
extern "C" __declspec(dllimport) bool InitializeCore (char * FileName, DWORD dwPID, RETBUFFER * Debuggee, Messages * Queue, bool * ShowLatestMessages,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) void EnterDebugLoop(const LPDEBUG_EVENT DebugEv,PROCESS_INFORMATION * PI, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) int EnableTrapflag (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) int DisableTrapflag (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) void StepOver (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) void ClearHwBreakPoint (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) void DisplayAttachDialog (SDL_Surface * backbuffer, int WindowSizeX, int WindowSizeY,int * ProcessesPresent,DWORD * RunningPIDs,int FileHighlighter, int SkipEntries,bool * ShowLatestMessages, Messages * Queue, RETBUFFER InFile, Uint32 c_SiceBorder, Uint32 c_Black);
extern "C" __declspec(dllimport) void EnumerateSectionOffset (SDL_Surface * screen,CONTEXT * CTX,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX);
extern "C" __declspec(dllimport) void HandleKeyboardInputCursor (RETBUFFER * InFile, SDL_Event * event,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) void Draw_InputBuffer (RETBUFFER * InFile,SDL_Surface * screen,int WindowSizeY,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) void Draw_Cursor (SDL_Surface * screen,int WindowSizeY,RETBUFFER * InFile);
extern "C" __declspec(dllimport) int CommandParser (RETBUFFER * InFile,DebugStruc * DebugDataExchange,char * InputBuffer);
extern "C" __declspec(dllimport) char * stringReplace(char *search, char *replace, char *string);
extern "C" __declspec(dllimport) bool ReadContext (CONTEXT * CTX,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) bool WriteContext (CONTEXT * CTX,DebugStruc * DebugDataExchange);
//extern "C" __declspec(dllimport) bool LoadConfig (char * FileName, char * Entry, config_result * result);
//extern "C" __declspec(dllimport) bool SaveConfig (char * FileName, char * Entry, config_result * result);
extern "C" __declspec(dllimport) bool ReLoadConfig (char * FileName);
extern "C" __declspec(dllimport) bool ReSaveConfig (char * FileName);
extern "C" __declspec(dllimport) void SetVideoMode (Messages * Queue);
extern "C" __declspec(dllimport) void Cleanup (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) void HandleFileDialog (SDL_Event * ereignis, char * CurrentPath, SDL_mutex * FileDialogue, int * FilesPresent, int * SkipEntries, int * FileHighlighter, DebugStruc * DebugDataExchange, RETBUFFER * Debuggee, int * WindowSizeX, int * WindowSizeY, Messages * Queue, bool * ShowLatestMessages, ULONG * FileDialoguePtr);
extern "C" __declspec(dllimport) int DumpFix (unsigned char * Offset,DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) bool BuildPluginApiTree (DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) void DisplaySyntax (char * CommandName, DebugStruc * DebugDataExchange);
extern "C" __declspec(dllimport) bool GetOption (char * CommandName);
extern "C" __declspec(dllimport) RECT * GetScreenPos (HWND hwnd);
extern "C" __declspec(dllimport) int GetCurrentWidth ();
extern "C" __declspec(dllimport) int GetLogYPos ();
extern "C" __declspec(dllimport) int GetDisassemblyYPos ();
extern "C" __declspec(dllimport) int GetDataYPos ();
extern "C" __declspec(dllimport) int GetInputYPos ();
//extern "C" __declspec(dllimport) _DecodeType GetDt ();
extern "C" __declspec(dllimport) bool OpenPdb(wchar_t *FileName);
extern "C" __declspec(dllimport) void CleanupDia();
extern "C" __declspec(dllimport) bool FindSymbolByRva(ULONG RVA, char * OutString, size_t OutLength);
extern "C" __declspec(dllimport) ULONG FindSymbolByName(wchar_t * InString);
extern "C" __declspec(dllimport) bool CopyQueueToClipboard (Messages * Queue);
extern "C" __declspec(dllimport) ULONG FindLineByRva (ULONG RVA, char * OutString, size_t OutLength);
extern "C" __declspec(dllimport) bool TextFile_getNumberofLines (char * FileName, ULONG * LineCounter);
//extern "C" __declspec(dllimport) bool TextFile_ReadFile (char * FileName, StringList * StringListPtr);




SDK_API void UGDbg_go (DebugStruc * DebugDataExchange, int argc, char * argv[]);
