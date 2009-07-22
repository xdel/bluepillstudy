#include "defs.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved){
        return TRUE;
}

char* __cdecl tst_sidt_description()
{
	return "Test using int frequency";
}

char* __cdecl tst_sidt_name()
{
	return "ex-SIDT test";
}

char* __cdecl tst_sidt_about()
{
	return "Implemented by deroko, Shub-Nigurrath of ARTeam";
}


char* __cdecl tst_drx_description()
{
	return "Test using driver to query drX";
}

char* __cdecl tst_drx_name()
{
	return "drX check";
}

char* __cdecl tst_drx_about()
{
	return "Implemented by deroko of ARTeam";
}


// xadt_int
//
Result __cdecl tst_sidt(char *message){
        
	Result bRet=NEGATIVE;	

	HANDLE  hdevice;
    DWORD   dummy;

	//things used for timetesting things..
	ULONG   FirstCounts[3];
	ULONG	RepeatedAquisitions[3];
	DWORD	iStartTime = 0;
	DWORD	dwTick = 0;

    hdevice = CreateFileA("\\\\.\\xadt_int", GENERIC_READ | GENERIC_WRITE, 0,0, OPEN_EXISTING, 0,0);
    if (hdevice == INVALID_HANDLE_VALUE){
            if (LoadDriver()){
                    strcpy(message, "can't load driver");
                    return UNKNOWN;
            }
            hdevice = CreateFileA("\\\\.\\xadt_int", GENERIC_READ | GENERIC_WRITE, 0,0, OPEN_EXISTING, 0,0);
    }

    DeviceIoControl(hdevice, get_ints, 0,0, &FirstCounts, 3 * sizeof(ULONG), &dummy, 0);
    
	// record the value and perform yet another query...
    // FirstCounts[0] = values of int01 DEBUG_TRAP
    // FirstCounts[1] = values of int03 BREAKPOINT_TRAP
    // FirstCounts[2] = values of int2d   DEBUG_OUTPUT
    
	iStartTime = GetTickCount();

	int repetitions=0;
	while(repetitions++<SIDT_REPETITIONS) {
		Sleep(SIDT_TIME_INTERVAL);
	
		DeviceIoControl(hdevice, get_ints, 0,0, &RepeatedAquisitions, 3 * sizeof(ULONG), &dummy, 0);

		dwTick = GetTickCount();
		dwTick -= iStartTime;
		dwTick /= 1000;

		//real check of the trap we want to test periodically..
		//FirstCounts[DEBUG_TRAP]
		//FirstCounts[BREAKPOINT_TRAP]
		//FirstCounts[DEBUG_OUTPUT]

#ifdef _DEBUG
		char dbgmessage[1024];
		sprintf(dbgmessage,"FirstCounts[DEBUG_TRAP]=%d\n"
			"FirstCounts[BREAKPOINT_TRAP]=%d\n"
			"FirstCounts[DEBUG_OUTPUT]=%d\n"
			"--------------------------\n"
			"RepeatedAquisitions[DEBUG_TRAP]=%d\n"
			"RepeatedAquisitions[BREAKPOINT_TRAP]=%d\n"
			"RepeatedAquisitions[DEBUG_OUTPUT]=%d\n", 
			FirstCounts[DEBUG_TRAP],
			FirstCounts[BREAKPOINT_TRAP],
			FirstCounts[DEBUG_OUTPUT],
			RepeatedAquisitions[DEBUG_TRAP],
			RepeatedAquisitions[BREAKPOINT_TRAP],
			RepeatedAquisitions[DEBUG_OUTPUT]
		);
		::MessageBox(NULL,(LPCSTR)dbgmessage,"SIDT xADT Test",MB_OK);
#endif

		if(FirstCounts[DEBUG_TRAP]!=RepeatedAquisitions[DEBUG_TRAP]) {
			sprintf(message,"DEBUG_TRAP changed within %x ticks", dwTick);
			bRet=POSITIVE;
		}

		if(FirstCounts[BREAKPOINT_TRAP]!=RepeatedAquisitions[BREAKPOINT_TRAP]) {
			sprintf(message,"BREAKPOINT_TRAP changed within %x ticks", dwTick);
			bRet=POSITIVE;
		}

		if(FirstCounts[DEBUG_OUTPUT]!=RepeatedAquisitions[DEBUG_OUTPUT]) {
			sprintf(message,"DEBUG_OUTPUT changed within %x ticks", dwTick);
			bRet=POSITIVE;
		}

	}
	
	return bRet;
}

Result __cdecl tst_drx(char *message){
        HANDLE  hdevice;  
        DWORD   dummy;
        ULONG   drx[6], i;

        hdevice = CreateFileA("\\\\.\\xadt_int", GENERIC_READ | GENERIC_WRITE, 0,0, OPEN_EXISTING, 0,0);
        if (hdevice == INVALID_HANDLE_VALUE){
                if (LoadDriver()){
                        strcpy(message, "can't load driver");
                        return UNKNOWN;
                }
                hdevice = CreateFileA("\\\\.\\xadt_int", GENERIC_READ | GENERIC_WRITE, 0,0, OPEN_EXISTING, 0,0);
        }

        DeviceIoControl(hdevice, get_drs, 0,0, &drx, sizeof(ULONG)*6, &dummy, 0);
        for (i = 0; i < 4; i++){
                if (drx[i]){
                        sprintf(message, "dr%d is not 0", i);
                        CloseHandle(hdevice);
                        return POSITIVE;
                }
        }

        if (drx[4] != 0xFFFF0FF0){
                strcpy(message, "dr6 is not 0xFFFF0FF0");
                CloseHandle(hdevice);
                return POSITIVE;
        }

        if (drx[5] != 0x400){
                strcpy(message, "dr7 is not 0x400");
                CloseHandle(hdevice);         
                return POSITIVE;
        }

        CloseHandle(hdevice);
        return NEGATIVE;
}