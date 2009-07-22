#include "myDOS.h"

namespace MyDOS {
	
	void AllocConsole(void) {
		//Allocate DOS Console
		int hCrt;
		FILE *hf;

		::AllocConsole();
		hCrt = _open_osfhandle(
			(long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT );
		hf = _fdopen( hCrt, "w" );
		*stdout = *hf;
		int i = setvbuf( stdout, NULL, _IONBF, 0 );

		SetConsoleTitle("xADT Rootkit Tests plugin Results Console");
		HANDLE hConsole=NULL;
		hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
		if(hConsole!=NULL)
			SetConsoleTextAttribute(hConsole,FOREGROUND_INTENSITY|FOREGROUND_GREEN);

	}

	void DOSclrscr(void)
	{
		HANDLE hout;
		DWORD nc, ncw;
		CONSOLE_SCREEN_BUFFER_INFO sbi;
		COORD home;
		home.X=home.Y=0;
		hout=GetStdHandle(STD_OUTPUT_HANDLE);
		if(hout==INVALID_HANDLE_VALUE) return;
		GetConsoleScreenBufferInfo(hout,&sbi);
		nc=sbi.dwSize.X*sbi.dwSize.Y;
		FillConsoleOutputAttribute(hout,sbi.wAttributes,nc,home,&ncw);
		FillConsoleOutputCharacter(hout,' ',nc,home,&ncw);
		SetConsoleCursorPosition(hout,home);
	}

	void FreeConsole(void) {
		::FreeConsole();
	}
}
