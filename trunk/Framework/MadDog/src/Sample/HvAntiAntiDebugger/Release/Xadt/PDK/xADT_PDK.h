#ifndef _XADT_PDK_H_
#define _XADT_PDK_H_
	
#ifdef __cplusplus
#undef EXPORT
#define EXPORT extern "C" __declspec (dllexport)
#else
#undef EXPORT
#define EXPORT __declspec (dllexport)
#endif

typedef enum {UNKNOWN, NEGATIVE, WARNING, POSITIVE} Result;

//////////////////////////////////////////////////////////////////////////
//Exports of xADT for the plugins
//************************************
// Method:    xADT_PluginFolder
// FullName:  xADT_PluginFolder
// Access:    public 
// Returns:   EXPORT void
// Qualifier: reports the folder where the plugins are loaded
// Parameter: char* strOut, must be allocated by caller, max lenght equal to MAX_PATH
// Calling convention must be __cdecl!
//************************************
EXPORT void __cdecl xADT_PluginFolder(char* strOut);
//////////////////////////////////////////////////////////////////////////

//Note1: message wrote MUST NOT be longer than MAX_PATH (260) characters!!
//Note2: message might be NULL, check it before using.
//Note3: a return at the end of the message is useless, just insert your message.
typedef Result (*fcnTester) (char *message); 
#endif //_XADT_PDK_H_