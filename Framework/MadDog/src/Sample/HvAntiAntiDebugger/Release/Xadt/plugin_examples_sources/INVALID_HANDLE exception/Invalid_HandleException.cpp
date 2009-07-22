#include "Invalid_HandleException.h"

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	return TRUE ;
}

void internalfcn(char *message, Result *bRet);

EXPORT Result tst_Invalid_HandleException(char *message) {

	Result bRet=UNKNOWN;
	
	try {
		internalfcn(message, &bRet);
	}
	catch(...) {
		strcat(message,"7!");
		return bRet;
		
	}
	return bRet;
}


void internalfcn(char *message, Result *bRet) {
	
	HANDLE hproc=OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
		
	strcat(message,"steps:");
	if(hproc) {
		__try {
			strcat(message,"1, ");
			*bRet=NEGATIVE;
			RaiseException(0x0C0000008,0,0,NULL);
			//If goes here means something has not went like expected, debugger catched exception.
			strcat(message,"2, ");
			CloseHandle(hproc);
			*bRet=POSITIVE;
		}
		__finally {
			DWORD deadcode=0x0DEADC0DE;
			__try{
				strcat(message,"3, ");
				*bRet=POSITIVE;
				CloseHandle((LPVOID)deadcode); //raise exception, if handled by the debugger does not go on next instruction
				*bRet=NEGATIVE;
				strcat(message,"4, ");
			}
			__finally {
				*bRet=NEGATIVE;
				strcat(message,"5, ");
				CloseHandle(hproc);
			}
		}
	}

	strcat(message,"6, ");
	*bRet=POSITIVE;
	CloseHandle(hproc);
}

EXPORT char* tst_Invalid_HandleException_description()
{
	return "Test looking if the Invalid_Handle Exception is caught or not";

}

EXPORT char* tst_Invalid_HandleException_name()
{
	return "Invalid_Handle Exception Test";
}

EXPORT char* tst_Invalid_HandleException_about() {
	return "Implemented by Shub-Nigurrath, from an idea of deroko";
}
