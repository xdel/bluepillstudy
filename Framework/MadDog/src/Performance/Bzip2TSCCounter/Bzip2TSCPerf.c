#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define MAGIC_KNOCK 100
#define TEST_LOOP	5
ULONG64 NBPCall (char* exefile) {
	ULONG32 HighStart,LowStart,HighEnd,LowEnd;
	ULONG64 start,end;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE hRead,hWrite;
	char filePathName[256]="d:\\test\\";
	__asm { 

	cpuid ;force all previous instructions to complete

	rdtsc
	mov HighStart,edx;
	mov LowStart,eax ;Timer 1
	}

	start = (((ULONG64)HighStart)<<32) + (ULONG64)LowStart;

	//system(exefile);

	si.cb =sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError=hWrite;
	si.hStdOutput=hWrite;
	si.wShowWindow=SW_HIDE;
	si.dwFlags =STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	CreateProcess(NULL,TEXT(strcat(filePathName,exefile)),NULL,NULL,FALSE,NULL,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	__asm{
	rdtsc		;Timer 2
	mov HighEnd,edx
	mov LowEnd,eax;
	}

	end = (((ULONG64)HighEnd)<<32) + (ULONG64)LowEnd;

	return (end - start);

}
int __cdecl main(int argc, char **argv) {
	ULONG64 i,result;
	
	//HANDLE hProcess = GetCurrentProcess(); 
	//DWORD dwProcessAffinityMask, dwSystemAffinityMask; 
	//GetProcessAffinityMask( hProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ); 

	//SetProcessAffinityMask( hProcess, 0 );

	result = 0;
	__try {
  		NBPCall(argv[1]); 
		system("bunzip2 -f a.mp3.bz2");
		for(i = 0 ; i < TEST_LOOP ;i++)
		{
			result += NBPCall(argv[1]);
			//system("del a.mp3");
			system("bunzip2 -f a.mp3.bz2");
		}
		//printf("%d, %d, %d, %d\n", eax, ebx, ecx, edx);
		printf ("Total Delta TSC: %lu %ld\n",(ULONG32)(result>>32),(ULONG32)(result));
		printf ("Average Delta TSC: %lu %ld\n", (ULONG32)((result/TEST_LOOP)>>32), (ULONG32)((result/TEST_LOOP)));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUID caused exception");
		return 0;
	}
	
	return 0;

}
