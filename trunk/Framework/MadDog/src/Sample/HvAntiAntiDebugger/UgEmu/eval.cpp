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

#ifdef b32BitBuild
ULONG Evaluate (char * str,DebugStruc * DebugDataExchange) {
ULONG bRes = 0;
#else
ULONGLONG Evaluate (char * str,DebugStruc * DebugDataExchange) {
ULONGLONG bRes = 0;
#endif
	char Value [128] = "";
	char str_arguments [_MAX_PATH] = "";
	char str_operators [_MAX_PATH] = "";
	char TmpStr [_MAX_PATH] = "";
	strcpy_s (str_arguments,_countof(str_arguments),str);
	strcpy_s (str_operators,_countof(str_operators),str);
	strcpy_s (TmpStr,_countof(TmpStr),str);

	int argc = 0;
	char * argv[128];

	bool allocated[128];
	memset (&allocated,0,_countof(allocated));

	int argc_op = 0;
	char * argv_op[128];

	char * next_token;
	char * pch = strtok_s  (str_arguments,"+-*/^|&",&next_token);
	while (pch != NULL) {
		argv[argc] = pch;
		argc++;
		pch = strtok_s (NULL, "+-*/^|&",&next_token);
	}


	pch = strtok_s  (str_operators,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_?:@ ",&next_token);
	while (pch != NULL) {
		argv_op[argc_op] = pch;
		argc_op++;
		pch = strtok_s (NULL, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_?:@ ",&next_token);
	}

	if ((argc-1) != argc_op) {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: Error in the expression found");
		return false;
	}

	for (int j = 0; j < argc; j++) {
		bool SymbolFound = false;

		if (DebugDataExchange->PDBLoaded == true) {
			wchar_t argument[_MAX_PATH];
			size_t CharsConverted;

			mbstowcs_s (&CharsConverted,argument,_countof (argument),argv[j],strlen(argv[j]));

			ULONG dwRVA = FindSymbolByName(argument);
			if (dwRVA != false) {
				argv[j] = new char [MAX_PATH];
				allocated[j] = true;

#ifdef b32BitBuild
				sprintf_s (argv[j],MAX_PATH,"%x",dwRVA+DebugDataExchange->ImageBase);
#else
				sprintf_s (argv[j],MAX_PATH,"%llx",dwRVA+DebugDataExchange->ImageBase);
#endif
				SymbolFound = true;
			}
		}




		if (SymbolFound == false) {
			PDllTable MyDllTable = (PDllTable)DebugDataExchange->DllTablePtr;
			if (MyDllTable != NULL) {
				while (MyDllTable->FunctionsAvailable != NULL) {
					PApiTable MyApiTable = MyDllTable->Apis;
					if (MyApiTable != NULL) {
						for (unsigned int i=0;i<MyDllTable->FunctionsAvailable;i++) {
							if (_stricmp ((const char *)MyApiTable->ApiAscii,argv[j]) == 0) {
								argv[j] = new char [MAX_PATH];
								allocated[j] = true;

#ifdef b32BitBuild
								sprintf_s (argv[j],MAX_PATH,"%x",MyApiTable->Offset);
#else
								sprintf_s (argv[j],MAX_PATH,"%llx",MyApiTable->Offset);
#endif
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

		if (SymbolFound == false) {
			for (int i = 0; i < _countof (Registers); i++) {
				if (_stricmp (argv[j],Registers[i]) == NULL) {
					CONTEXT CTX;
					if (ReadContext (&CTX,DebugDataExchange) == true) {
/*
char Registers[42][4] = { "eax","ebx","ecx","edx","esi","edi","ebp","esp","eip",
						  "rax","rbx","rcx","rdx","rsi","rdi","rbp","rsp","rip","r8","r9","r10","r11","r12","r13","r14","r15",
						  "al","ah","ax","bl","bh","bx","cl","ch","cx","dl","dh","dx","si","di","bp","sp"};
*/
#ifdef b32BitBuild
						ULONG RegValue[] = {CTX.Eax,CTX.Ebx,CTX.Ecx,CTX.Edx,CTX.Esi,CTX.Edi,CTX.Ebp,CTX.Esp,CTX.Eip,
											CTX.Eax,CTX.Ebx,CTX.Ecx,CTX.Edx,CTX.Esi,CTX.Edi,CTX.Ebp,CTX.Esp,CTX.Eip,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,CTX.Eax,
											CTX.Eax,CTX.Eax,CTX.Eax,CTX.Ebx,CTX.Ebx,CTX.Ebx,CTX.Ecx,CTX.Ecx,CTX.Ecx,CTX.Edx,CTX.Edx,CTX.Edx,CTX.Esi,CTX.Edi,CTX.Ebp,CTX.Esp};
						ULONG AddValue = 0;
#else
						ULONGLONG RegValue[] = {CTX.Rax,CTX.Rbx,CTX.Rcx,CTX.Rdx,CTX.Rsi,CTX.Rdi,CTX.Rbp,CTX.Rsp,CTX.Rip,
												CTX.Rax,CTX.Rbx,CTX.Rcx,CTX.Rdx,CTX.Rsi,CTX.Rdi,CTX.Rbp,CTX.Rsp,CTX.Rip,CTX.R8,CTX.R9,CTX.R10,CTX.R11,CTX.R12,CTX.R13,CTX.R14,CTX.R15,
												CTX.Rax,CTX.Rax,CTX.Rax,CTX.Rbx,CTX.Rbx,CTX.Rbx,CTX.Rcx,CTX.Rcx,CTX.Rcx,CTX.Rdx,CTX.Rdx,CTX.Rdx,CTX.Rsi,CTX.Rdi,CTX.Rbp,CTX.Rsp};
						ULONGLONG AddValue = 0;
#endif
						
						AddValue = RegValue[i];

						if ((strlen (argv[j]) == 2) && ((argv[j][0] != 'r') || (argv[j][0] != 'R'))) {
							if ((argv[j][1] == 'l') || (argv[j][1] == 'L') || (argv[j][1] == 'h') || (argv[j][1] == 'H')) {
								if ((argv[j][1] == 'l') || (argv[j][1] == 'L')) {
									AddValue &= 0xFF;
								} else {
									AddValue >>= 8;
									AddValue &= 0xFF;

								}
							} else {
								AddValue &= 0xFFFF;
							}
						}
						if ((strlen (argv[j]) == 3) && ((argv[j][0] == 'e') || (argv[j][0] == 'E'))) {
							AddValue &= 0xFFFFFFFF;
						}

						argv[j] = new char [MAX_PATH];
						allocated[j] = true;
#ifdef b32BitBuild
						sprintf_s (argv[j],MAX_PATH,"%x",AddValue);
#else
						sprintf_s (argv[j],MAX_PATH,"%llx",AddValue);
#endif
						break;
					}
				}
			}
		}
	}

#ifdef b32BitBuild
	ULONG Calc = 0;
	sscanf_s (argv[0],"%x",&Calc);
#else
	ULONGLONG Calc = 0;
	sscanf_s (argv[0],"%llx",&Calc);
#endif
	bRes = Calc;

	for (int i = 0; i < argc_op; i++) {
#ifdef b32BitBuild
		sscanf_s (argv[i+1],"%x",&Calc);
#else
		sscanf_s (argv[i+1],"%llx",&Calc);
#endif
		if (strcmp (argv_op[i],"+") == NULL) {
			bRes += Calc;
			continue;
		} 
		if (strcmp (argv_op[i],"-") == NULL) {
			bRes -= Calc;
			continue;
		} 
		if (strcmp (argv_op[i],"/") == NULL) {
			bRes /= Calc;
			continue;
		} 
		if (strcmp (argv_op[i],"*") == NULL) {
			bRes *= Calc;
			continue;
		} 
		if (strcmp (argv_op[i],"^") == NULL) {
			bRes ^= Calc;
			continue;
		} 
		if (strcmp (argv_op[i],"&") == NULL) {
			bRes &= Calc;
			continue;
		} 
		if (strcmp (argv_op[i],"|") == NULL) {
			bRes |= Calc;
			continue;
		} 
	}

	// cleanup
	for (int i = 0; i < argc; i++) {
		if (allocated[i] == true) delete argv[i];
	}
	return bRes;
}

// *****************************************************************
//  PERFORM A VA-TO-STRING LOOKUP
// 
// *****************************************************************
#ifdef b32BitBuild
char * PerformVALookup (ULONG Address,DebugStruc * DebugDataExchange)
#else
char * PerformVALookup (ULONGLONG Address,DebugStruc * DebugDataExchange)
#endif
{
	DllTable * MyDllTable = DebugDataExchange->DllTablePtr;
	while (MyDllTable->FunctionsAvailable != NULL) {
		PApiTable MyApiTable = MyDllTable->Apis;
		for(unsigned int i = 0; i < MyDllTable->FunctionsAvailable; i++) {
			if (MyApiTable->Offset == Address) return (char *)MyApiTable->ApiAscii;
			MyApiTable++;
		}
		MyDllTable++;
	}
	return false;
}