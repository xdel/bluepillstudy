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
#include "cvconst.h"
#include "dia2.h"
#include "diacreate.h"
#include "callback.h"

const wchar_t *g_szFilename;
IDiaDataSource *g_pDiaDataSource;
IDiaSession *g_pDiaSession;
IDiaSymbol *g_pGlobalSymbol;
DWORD g_dwMachineType = CV_CFL_80386;
#define freeString LocalFree

SourceFileList * FileList = 0;

bool LoadDataFromPdb(
    const wchar_t    *szFilename,
    IDiaDataSource  **ppSource,
    IDiaSession     **ppSession,
    IDiaSymbol      **ppGlobal)
{
  wchar_t wszExt[MAX_PATH];
  wchar_t *wszSearchPath = L"SRV**\\\\symbols\\symbols"; // Alternate path to search for debug data
  DWORD dwMachType = 0;

  HRESULT hr = CoInitialize(NULL);
  // Obtain access to the provider
  hr = CoCreateInstance(__uuidof(DiaSource),NULL,CLSCTX_INPROC_SERVER,__uuidof(IDiaDataSource),(void **) ppSource);

  if (FAILED(hr)) {
    printf("CoCreateInstance failed - HRESULT = %08X\n", hr);
    return false;
  }

  _wsplitpath_s(szFilename, NULL, 0, NULL, 0, NULL, 0, wszExt, MAX_PATH);

  if (!_wcsicmp(wszExt, L".pdb")) {
    // Open and prepare a program database (.pdb) file as a debug data source
    hr = (*ppSource)->loadDataFromPdb(szFilename);
    if (FAILED(hr)) {
      printf("loadDataFromPdb failed - HRESULT = %08X\n", hr);
      return false;
    }
  }

  else {
    CCallback callback; // Receives callbacks from the DIA symbol locating procedure,
                        // thus enabling a user interface to report on the progress of
                        // the location attempt. The client application may optionally
                        // provide a reference to its own implementation of this
                        // virtual base class to the IDiaDataSource::loadDataForExe method.
    callback.AddRef();

    // Open and prepare the debug data associated with the executable
    hr = (*ppSource)->loadDataForExe(szFilename, wszSearchPath, &callback);

    if (FAILED(hr)) {
      printf("loadDataForExe failed - HRESULT = %08X\n", hr);
      return false;
    }
  }

  // Open a session for querying symbols
  hr = (*ppSource)->openSession(ppSession);
  if (FAILED(hr)) {
    printf("openSession failed - HRESULT = %08X\n", hr);
    return false;
  }

  // Retrieve a reference to the global scope
  hr = (*ppSession)->get_globalScope(ppGlobal);
  if (hr != S_OK) {
    printf("get_globalScope failed\n");
    return false;
  }

  // Set Machine type for getting correct register names
  if ((*ppGlobal)->get_machineType(&dwMachType) == S_OK) {
    switch (dwMachType) {
      case IMAGE_FILE_MACHINE_I386 : g_dwMachineType = CV_CFL_80386; break;
      case IMAGE_FILE_MACHINE_IA64 : g_dwMachineType = CV_CFL_IA64; break;
      case IMAGE_FILE_MACHINE_AMD64 : g_dwMachineType = CV_CFL_AMD64; break;
    }
  }
  return true;
}

void CleanupDia()
{
  if (g_pGlobalSymbol) {
    g_pGlobalSymbol->Release();
    g_pGlobalSymbol = NULL;
  }

  if (g_pDiaSession) {
    g_pDiaSession->Release();
    g_pDiaSession = NULL;
  }

  CoUninitialize();
}


bool OpenPdb(wchar_t *FileName)
{
  FILE *pFile;

  if (_wfopen_s(&pFile, FileName, L"r") || !pFile) {
    // invalid file name or file does not exist
    return false;
  }

  fclose(pFile);
  g_szFilename = FileName;

  // CoCreate() and initialize COM objects
  if (!LoadDataFromPdb(g_szFilename, &g_pDiaDataSource, &g_pDiaSession, &g_pGlobalSymbol)) {
    return false;
  }
  return true;
}

bool FindSymbolByRva(ULONG RVA, char * OutString, size_t OutLength)
{
	ULONG celt = 0;
	IDiaEnumSymbolsByAddr *pEnumByAddr;
	IDiaSymbol *pSym;
	if (FAILED (g_pDiaSession->getSymbolsByAddr (&pEnumByAddr))) {
	  return false;
	}
	if (FAILED (pEnumByAddr->symbolByRVA (RVA,&pSym))) {
	  pEnumByAddr->Release();
	  return false;
	}

	DWORD rva;
	if ( pSym->get_relativeVirtualAddress( &rva ) != S_OK )
	  rva = 0;   // No RVA; must be an absolute value.
	DWORD tag;
	pSym->get_symTag( &tag );
	BSTR name;
	if ( pSym->get_name( &name ) != S_OK ) {
		pSym->Release();
		pEnumByAddr->Release();
		SysFreeString(name);
		return false;
	} else {
		if (rva == RVA) {
			size_t CharConverted = 0;
			wcstombs_s (&CharConverted,OutString,OutLength,name,_TRUNCATE);
			LONG Ret = 0;

			SysFreeString(name);
			pSym->Release();
			pEnumByAddr->Release();
			return true;
		} else {
			SysFreeString(name);
			pSym->Release();
			pEnumByAddr->Release();
			return false;
		}
	}
	return false;
}

ULONG FindLineByRva (ULONG RVA, char * OutString, size_t OutLength)
{
	ULONG celt = 0;
	DWORD dwRVA;
	DWORD dwSeg;
	DWORD dwOffset;
	DWORD dwLinenum;
	DWORD dwSrcId;
	DWORD dwLength;

	DWORD dwSrcIdLast = (DWORD)(-1);

	IDiaEnumLineNumbers * pEnumByAddr;
	IDiaLineNumber * pLine;

	if (FAILED (g_pDiaSession->findLinesByRVA (RVA,30,&pEnumByAddr))) {
	  return false;
	}

	while (SUCCEEDED(pEnumByAddr->Next(1, &pLine, &celt)) && (celt == 1)) {
		if ((pLine->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
			(pLine->get_addressSection(&dwSeg) == S_OK) &&
			(pLine->get_addressOffset(&dwOffset) == S_OK) &&
			(pLine->get_lineNumber(&dwLinenum) == S_OK) &&
			(pLine->get_sourceFileId(&dwSrcId) == S_OK) &&
			(pLine->get_length(&dwLength) == S_OK)) {

			IDiaSourceFile *pSource;
			if (dwRVA == RVA && dwLength != 0) {
				if (pLine->get_sourceFile (&pSource) == S_OK) {
					BSTR Name;
					if (pSource->get_fileName (&Name) == S_OK) {
						char FileName[_MAX_PATH+1] = "";
						size_t CharsConverted;
						wcstombs_s (&CharsConverted,FileName,_countof(FileName),Name,_MAX_PATH);

						// INIT NOW IF NOT AVAILABLE
						if (FileList == 0) {
							FileList = (SourceFileList *)GlobalAlloc (GMEM_ZEROINIT,5000*sizeof (RETBUFFER));
						}
						if (FileList != NULL) {
							SourceFileList * FileListTmp = FileList;
							bool LookupDone = false;
							while (strlen (FileListTmp->File.FileName) != 0) {
								if (_stricmp (FileName,FileListTmp->File.FileName) == 0) {
									// ENTRY FOUND
									LookupDone = true;
									break;
								}
								FileListTmp++;
							}
							if (LookupDone == false) {
								if (ReadFileMem (FileName,&FileListTmp->File) == TRUE) {
									ULONG Lines = 0;
									if (TextFile_getNumberofLines (FileName,&Lines) == true) {
										FileListTmp ->StringListPtr = (StringList *)GlobalAlloc (GMEM_ZEROINIT,Lines*sizeof (StringList));
										if (FileListTmp->StringListPtr != NULL) {
											if (TextFile_ReadFile (FileName,FileListTmp->StringListPtr) == true) {
												LookupDone = true;
											}
										}
									}
								} else {
									// SET ENTRY ANYWAY TO AVOID TRYING TO RELOAD THIS FILE
									strcpy_s (FileListTmp->File.FileName,_countof (FileListTmp->File.FileName),FileName);
								}
							}
							if (LookupDone == true) {
								if (FileListTmp->StringListPtr != NULL) {
									memcpy (OutString,FileListTmp->StringListPtr[dwLinenum-1].Line,_countof (FileListTmp->StringListPtr[dwLinenum-1].Line));
									pEnumByAddr->Release();
									SysFreeString (Name);
									pSource->Release ();
									pLine->Release();
									return dwLinenum;
								}
							}
						}
						SysFreeString (Name);
						pSource->Release ();
					} else {
						SysFreeString (Name);
						pSource->Release ();
					}
				}
			}
			pLine->Release();
		}
	}
	pEnumByAddr->Release();
	return false;
}

ULONG FindSymbolByName(wchar_t * InString)
{
	IDiaEnumSymbols *pEnumSymbols;

	if (FAILED(g_pGlobalSymbol->findChildren(SymTagPublicSymbol, NULL, nsNone, &pEnumSymbols))) {
		return false;
	}

	IDiaSymbol *pSymbol;
	ULONG celt = 0;

	while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1)) {
		DWORD dwSymTag;
		DWORD dwRVA;
		BSTR bstrName;

		if (pSymbol->get_symTag(&dwSymTag) != S_OK) {
			continue;
		}

		if (pSymbol->get_relativeVirtualAddress(&dwRVA) != S_OK) {
			dwRVA = 0xFFFFFFFF;
		}

		if (dwSymTag == SymTagThunk) {
			if (pSymbol->get_name(&bstrName) == S_OK) {
				SysFreeString(bstrName);
				if (_wcsicmp (bstrName,InString) == NULL) {
					pSymbol->Release();
					pEnumSymbols->Release();
					return dwRVA;
				}
			} else {
				if (pSymbol->get_targetRelativeVirtualAddress(&dwRVA) != S_OK) {
					dwRVA = 0xFFFFFFFF;
				}
			}
		} else {
			// must be a function or a data symbol
			BSTR bstrUndname;

			if (pSymbol->get_name(&bstrName) == S_OK) {
				if (pSymbol->get_undecoratedName(&bstrUndname) == S_OK) {
					if ((_wcsicmp (bstrName,InString) == NULL) || (_wcsicmp (bstrUndname,InString) == NULL)) {
						pSymbol->Release();
						pEnumSymbols->Release();
						SysFreeString(bstrName);
						SysFreeString(bstrUndname);
						return dwRVA;
					}
					SysFreeString(bstrUndname);
				} else {
					if (_wcsicmp (bstrName,InString) == NULL) {
						pSymbol->Release();
						pEnumSymbols->Release();
						SysFreeString(bstrName);
						return dwRVA;
					}
				}
				SysFreeString(bstrName);
			}
		}
		pSymbol->Release();
	}
	pEnumSymbols->Release();
	return false;
}
