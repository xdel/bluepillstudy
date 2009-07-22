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
#include "stdafx.h"
#include "helper.h"
#include <strsafe.h>


/* \doc Set |bit| in |flags|. */
void bit_set( unsigned long *flags, int bit ) {
   *flags |= (1UL << bit);
}

/* \doc Clear |bit| in |flags|. */
void bit_clr( unsigned long *flags, int bit ) {
   *flags &= ~(1UL << bit);
}

/* \doc Test |bit| in |flags|, returning non-zero if the bit is set and
   zero if the bit is clear. */
int bit_tst( unsigned long *flags, int bit ) {
   return (*flags & (1UL << bit));
}

/* \doc Return a count of the number of bits set in |flags|. */
int bit_cnt( unsigned long *flags ) {
   unsigned long x = *flags;

#if SIZEOF_LONG == 4
   x = (x >> 1  & 0x55555555) + (x & 0x55555555);
   x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
   x = ((x >> 4) + x) & 0x0f0f0f0f;
   x = ((x >> 8) + x);
   return (x + (x >> 16)) & 0xff;
#else
#if SIZEOF_LONG == 8
   x = (x >> 1  & 0x5555555555555555) + (x & 0x5555555555555555);
   x = ((x >> 2) & 0x3333333333333333) + (x & 0x3333333333333333);
   x = ((x >> 4) + x) & 0x0f0f0f0f0f0f0f0f;
   x = ((x >> 8) + x) & 0x00ff00ff00ff00ff;
   x = ((x >> 16) + x) & 0x0000ffff0000ffff;
   return (x + (x >> 32)) & 0xff;
#else
#endif
#endif
}





// ************************************************************************
//	READ FILE IN MEM
//
//  Return Codes:  != 0 -> error
//
// ************************************************************************
int  ReadFileMem (char * FileName,RETBUFFER * tescht)
{
	PRETBUFFER StrPtr = (PRETBUFFER) (tescht);
	HANDLE File;
	File = CreateFile (FileName,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if (File == INVALID_HANDLE_VALUE) return -1;
	StrPtr->FileSize = GetFileSize (File,0);
	if (StrPtr->FileSize == 0) return -2;
	StrPtr->FileOffset = (unsigned char *)GlobalAlloc (GMEM_ZEROINIT,StrPtr->FileSize+StrPtr->SpecialAddSize+0x1000);	
	if (StrPtr->FileOffset == 0) return -3;
	if (ReadFile (File,StrPtr->FileOffset,StrPtr->FileSize,&StrPtr->Bread,0) == 0) return -4;
	StrPtr->ErrCode = 1;
	CloseHandle (File);
	strcpy_s (StrPtr->FileName,_MAX_PATH,FileName);
	return true;
}


int WriteFileMem (char * FileName,RETBUFFER * tescht)
{
	PRETBUFFER StrPtr = (PRETBUFFER) (tescht);
	HANDLE File;
	File = CreateFile (FileName,GENERIC_READ | GENERIC_WRITE,0,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (File == INVALID_HANDLE_VALUE) return -1;
	if (StrPtr->FileSize == 0) return -1;
	if (StrPtr->FileOffset == 0) return -1;
	if (WriteFile (File,StrPtr->FileOffset,StrPtr->FileSize+StrPtr->SpecialAddSize,&StrPtr->Bread,0) == 0) return -1;
	StrPtr->ErrCode = 1;
	CloseHandle (File);
	return 0;
}

// *****************************************************************
//  Function: FetchOpcodeSize
// 
//  In: ULONG OFFSET
//  
//  Out: -
//
//	Use: Retrieve size of opcode
//       
// *****************************************************************
#ifdef b32BitBuild
ULONG FetchOpcodeSize (ULONG Offset, DebugStruc * DebugDataExchange)
#else
ULONGLONG FetchOpcodeSize (ULONGLONG Offset, DebugStruc * DebugDataExchange)
#endif
{
	_DecodeResult res = DECRES_NONE;
	_DecodedInst decodedInstructions[MAX_INSTRUCTIONS];
	unsigned int decodedInstructionsCount = 0;
#ifdef b32BitBuild
	_DecodeType dt = Decode32Bits;
#else
	_DecodeType dt = Decode64Bits;
#endif
	_OffsetType offset = 0;

	unsigned char InstructionBuffer[MAX_INSTRUCTIONS*16];

#ifdef b32BitBuild
	ULONG OffsetTmp = Offset;
#else
	ULONGLONG OffsetTmp = Offset;
#endif
	ReadMem (OffsetTmp,16,&InstructionBuffer,&DebugDataExchange->ProcessInfo);
	res = distorm_decode(Offset, (const unsigned char*)InstructionBuffer, 16, dt, decodedInstructions, 15, &decodedInstructionsCount);
	if (res == DECRES_SUCCESS) {
		return decodedInstructions[0].size; 
	} else {
		if (decodedInstructionsCount != 0) {
			return decodedInstructions[0].size; 
		}
		return -1;
	}
	return -1;
}


// *****************************************************************
//  Function: FetchOpcodeAscii
// 
//  In: ULONG OFFSET
//  
//  Out: -
//
//	Use: Retrieve size of opcode
//       
// *****************************************************************
#ifdef b32BitBuild
char * FetchOpcodeAscii (ULONG Offset, DebugStruc * DebugDataExchange)
#else
char * FetchOpcodeAscii (ULONGLONG Offset, DebugStruc * DebugDataExchange)
#endif
{
	_DecodeResult res = DECRES_NONE;
	_DecodedInst decodedInstructions[MAX_INSTRUCTIONS];
	unsigned int decodedInstructionsCount = 0;
#ifdef b32BitBuild
	_DecodeType dt = Decode32Bits;
#else
	_DecodeType dt = Decode64Bits;
#endif
	_OffsetType offset = 0;

	unsigned char InstructionBuffer[MAX_INSTRUCTIONS*16];
#ifdef b32BitBuild
	ULONG OffsetTmp = Offset;
#else
	ULONGLONG OffsetTmp = Offset;
#endif
	for (unsigned int i = 0;i < 2; i ++) {
		ReadMem (OffsetTmp,16,&InstructionBuffer[i*16],&DebugDataExchange->ProcessInfo);
		OffsetTmp += 16;
	}

	res = distorm_decode(Offset, (const unsigned char*)InstructionBuffer, 16, dt, decodedInstructions, 15, &decodedInstructionsCount);
	if (res == DECRES_SUCCESS) {
		return (char *)decodedInstructions[0].mnemonic.p; 
	} else {
		if (decodedInstructionsCount != 0) {
			return (char *)decodedInstructions[0].mnemonic.p; 
		}
		return (char *)-1;
	}
}

// *****************************************************************
//  Function: FetchOpcode
// 
//  In: ULONG OFFSET
//  
//  Out: -
//
//	Use: Retrieve size of opcode
//       
// *****************************************************************
#ifdef b32BitBuild
_DecodedInst * FetchOpcode (ULONG Offset, DebugStruc * DebugDataExchange)
#else
_DecodedInst * FetchOpcode (ULONGLONG Offset, DebugStruc * DebugDataExchange)
#endif
{
	_DecodeResult res = DECRES_NONE;
	_DecodedInst decodedInstructions[MAX_INSTRUCTIONS];
	unsigned int decodedInstructionsCount = 0;
#ifdef b32BitBuild
	_DecodeType dt = Decode32Bits;
#else
	_DecodeType dt = Decode64Bits;
#endif
	_OffsetType offset = 0;

	unsigned char InstructionBuffer[MAX_INSTRUCTIONS*16];
#ifdef b32BitBuild
	ULONG OffsetTmp = Offset;
#else
	ULONGLONG OffsetTmp = Offset;
#endif
	for (unsigned int i = 0;i < 2; i ++) {
		ReadMem (OffsetTmp,16,&InstructionBuffer[i*16],&DebugDataExchange->ProcessInfo);
		OffsetTmp += 16;
	}

	res = distorm_decode(Offset, (const unsigned char*)InstructionBuffer, 16, dt, decodedInstructions, 15, &decodedInstructionsCount);
	if (res == DECRES_SUCCESS) {
		return &decodedInstructions[0]; 
	} else {
		if (decodedInstructionsCount != 0) {
			return &decodedInstructions[0];
		}
		return NULL;
	}
}



char * stringReplace(char *search, char *replace, char *string) {
	char *tempString, *searchStart;
	__int64 len=0;

	tempString = (char*) malloc(_MAX_PATH);
	if(tempString == NULL) {
		return NULL;
	}

	searchStart = strstr(string, search);
	if(searchStart == NULL) {
		free (tempString);
		return NULL;
	}
	strcpy_s(tempString,_MAX_PATH, string);

	len = searchStart - string;
	string[len] = '\0';

	strcat_s(string, _MAX_PATH, replace);

	len += strlen(search);
	strcat_s(string,_MAX_PATH, (char*)tempString+len);
	free(tempString);

	return string;
}

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) {
        do {
            StringCchCopy (lpEnd, MAX_PATH*2, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;
            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************
BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    TCHAR szDelKey[MAX_PATH*2];

    StringCchCopy (szDelKey, MAX_PATH*2, lpSubKey);
    return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

// THX pusher
BOOL RegisterSelf (char * Value) {
	HKEY Key, Key2;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT,Value,0,"",REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&Key,NULL) == ERROR_SUCCESS) {
		char Value_[MAX_PATH] = "";
		sprintf_s (Value_,_countof(Value_),"%s%s",Value,"\\command");
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT,Value_,0,"",REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&Key2,NULL) == ERROR_SUCCESS) {
			char Home[MAX_PATH] = "";
			char Home2[MAX_PATH] = "";
			GetHomePath (Home2,_countof(Home2));
			sprintf_s (Home,_countof(Home),"\"%s\\UgEmu.exe\" \"%%1\"",Home2);
			if (RegSetValueEx(Key2,"",0,REG_SZ,(const BYTE *)Home,(DWORD)strlen(Home)) == ERROR_SUCCESS) {
				RegCloseKey(Key2);
				RegCloseKey(Key);
				return true;
			}
			RegCloseKey(Key2);
		}
		RegCloseKey(Key);
	}
	return false;
}

ULONG ProcSpeedRead()
{
	DWORD BufSize = sizeof(DWORD);
	DWORD dwMHz = 0;
	HKEY hKey;

	// open the key where the proc speed is hidden:
	long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",0,KEY_READ,&hKey);
    
    if(lError != ERROR_SUCCESS) {
           return false;
    }

    RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE) &dwMHz, &BufSize);
	return dwMHz;
}

char * ProcessorInfo(char * Id)
{
	DWORD BufSize = _MAX_PATH;
	static char Name[MAX_PATH] = "";
	HKEY hKey;

	// open the key where the proc speed is hidden:
	long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",0,KEY_READ,&hKey);
    
    if(lError != ERROR_SUCCESS) {
           return false;
    }

    RegQueryValueEx (hKey, Id, NULL, NULL, (LPBYTE) &Name, &BufSize);
	return Name;
}

void Cleanup (DebugStruc * DebugDataExchange) {
	GlobalFree (DebugDataExchange->Debuggee.FileOffset);
	if (DebugDataExchange->DllTablePtr != NULL) 
	{
		DllTable * FreeTable = DebugDataExchange->DllTablePtr;
		while (strlen ((const char *)FreeTable->DllAscii) != 0)
		{
			VirtualFree (FreeTable->Apis,NULL,MEM_RELEASE);
			FreeTable++;
		}
	}
	return;
}

