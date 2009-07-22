#include <windows.h>
#include "FindWindow_and_Time.h"

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
     return TRUE ;
}

EXPORT Result tst_FindWindow(char *message)
{

	//It's just and example of test, of course the test is quite obvious, even if several fails to check it!
	//Anyway this code can be improved to detect the OllyDbg windows and child windows characteristics
	if (FindWindow("OLLYDBG",NULL)!=NULL)
		return POSITIVE;
	else
		return NEGATIVE;

}

EXPORT char* tst_FindWindow_description()
{
	return "Test using FindWindow OllyDbg";

}

EXPORT char* tst_FindWindow_name()
{
	return "FindWindow OllyDbg";
}

EXPORT Result tst_GetSystemTime(char *message)
{
	
	//from an idea of zyzygy
	WORD sec1=0, sec2=0;
	
	SYSTEMTIME st;
	GetSystemTime(&st);
	sec1=st.wSecond;

	GetSystemTime(&st);
	sec2=st.wSecond;
	
	if((sec1-sec2)==0)
		return NEGATIVE;
	
	return POSITIVE;
	
}

EXPORT char* tst_GetSystemTime_description()
{
	return "Test using GetSystemTime OllyDbg";
	
}

EXPORT char* tst_GetSystemTime_name()
{
	return "GetSystemTime OllyDbg";
}

