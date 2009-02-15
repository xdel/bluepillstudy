#include <windows.h>
//#include <iostream>
#include <stdio.h>
#include <imagehlp.h>
#include <psapi.h>
#pragma pack(1)
#pragma comment(lib, "imagehlp.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "user32.lib")

#pragma comment(linker,"/STACK:10240,2048")

//unsigned long LoadPEFile(char *FileName, char **Buffer)
//{
//    FILE *fp = fopen(FileName, "rb");
//    fseek(fp, 0, SEEK_END);
//    unsigned long len = ftell(fp);
//    fseek(fp, 0, SEEK_SET);
//    *Buffer = new char[len + 4];
//    memset(*Buffer, 0x0, len + 4);
//    unsigned long i = 0;
//    while(i < len)
//    {
//        fread(*Buffer + i, 4, 1, fp);
//        i+=4;
//    }
//    fclose(fp);
//    return len;
//}


int main()
{
	char *Buffer = NULL;
	HMODULE hmod = NULL;
	char PEFile[MAX_PATH] = {0};
	unsigned long OEP = 0;
    //strcpy(PEFile, "c:\\windows\\system32\\cmd.exe"); 
    //strcpy(PEFile, "C:\\Program Files\\WinRAR\\Winrar.exe"); 
	strcpy(PEFile, "C:\\Documents and Settings\\Superymk\\Desktop\\PE\\PE32\\Debug\\PE32.exe");
	//strcpy(PEFile, "c:\\windows\\system32\\winmine.exe"); 
    char lpBuffer[MAX_PATH] = {0};
    char *p = NULL;
    GetFullPathName(PEFile, MAX_PATH, lpBuffer, &p);
    char *s = lpBuffer + (p - lpBuffer);
    s[0] = '\0';
    printf("path:%s\n", lpBuffer);
    SetCurrentDirectory(lpBuffer);
    //unsigned long len = LoadPEFile(PEFile, &Buffer);

   /* IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)Buffer;
    IMAGE_NT_HEADERS *pINH = (IMAGE_NT_HEADERS *)((DWORD)dos_header + dos_header->e_lfanew);
    pINH->FileHeader.Characteristics |= IMAGE_FILE_DLL;
    pINH->FileHeader.Characteristics |= IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP;
    OEP = pINH->OptionalHeader.AddressOfEntryPoint;
    printf("OldEntryPoint:0x%.8x\n", pINH->OptionalHeader.AddressOfEntryPoint);*/
    //pINH->OptionalHeader.AddressOfEntryPoint = 0x0;
    //printf("pINH->OptionalHeader.AddressOfEntryPoint:0x%.8x\n", pINH->OptionalHeader.AddressOfEntryPoint);
   // printf("len:%d\n", len);
   // delete []Buffer;
  //  Buffer = NULL;
   // printf("Load\n");
    hmod = LoadLibrary(PEFile);
    if (!hmod)
        printf("LoadLibrary Error %.8X\n", GetLastError());
    else
    {
        printf("base address:0x%.8x\n", hmod);
        MODULEINFO dumpinfo;
        DWORD dw = 0;
        GetModuleInformation(GetCurrentProcess(), hmod, &dumpinfo, sizeof MODULEINFO);
        IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)hmod;
        IMAGE_NT_HEADERS *pINH = (IMAGE_NT_HEADERS *)((DWORD)dos_header + dos_header->e_lfanew);
        if(!VirtualProtect((char *)pINH, sizeof(IMAGE_NT_HEADERS), PAGE_EXECUTE_READWRITE, &dw)) return 1;
        //pINH->OptionalHeader.AddressOfEntryPoint = OEP;
		OEP = pINH->OptionalHeader.AddressOfEntryPoint;
       /* printf("pINH->FileHeader.Characteristics:0x%.8x\n", pINH->FileHeader.Characteristics);
        pINH->FileHeader.Characteristics &= ~IMAGE_FILE_DLL;
        printf("pINH->FileHeader.Characteristics:0x%.8x\n", pINH->FileHeader.Characteristics);*/
        if(!VirtualProtect((char *)pINH, sizeof(IMAGE_NT_HEADERS), PAGE_EXECUTE_READ, &dw)) return 1;
        printf("Call\n");
        DWORD id = 0;
        //if(argc == 3)
        //    HookAPI(1);
        //else
            //HookAPI(0);
        OEP += (DWORD)hmod;
        HANDLE hThread = CreateThread(NULL, 1024*1024, (LPTHREAD_START_ROUTINE)OEP, 0, 0, &id);
		WaitForSingleObject(hThread,20000);
	}
    //delete []lpNewBaseOfDll;
    //delete []lpNewBaseOfDll1;
    FreeLibrary(hmod);
    //DeleteFile(PEFile);
	return 0;
}
