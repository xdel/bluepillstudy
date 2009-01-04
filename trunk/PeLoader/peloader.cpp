/*********************************************************/
//PELoader.exe v1.0
//Luke msn:msfocus@hotmail.com
//2007.7.25
/*********************************************************/
#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>
#include <psapi.h>
#pragma pack(1)
#pragma comment(lib, "imagehlp.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(linker,"/STACK:10240,2048")


HMODULE hmod = NULL;
char *lpNewBaseOfDll = NULL;
char *lpNewBaseOfDll1 = NULL;
MODULEINFO mi;
MODULEINFO mi1;
HMODULE OldKernel32Address = NULL;
HMODULE OldUser32Address = NULL;
char PEFile[MAX_PATH] = {0};
unsigned long OEP = 0;
char *addr_GetModuleHandleExA = NULL;
char *addr_GetModuleHandleExW = NULL;
unsigned long LoadPEFile(char *FileName, char **Buffer)
{
    FILE *fp = fopen(FileName, "rb");
    fseek(fp, 0, SEEK_END);
    unsigned long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    *Buffer = new char[len + 4];
    memset(*Buffer, 0x0, len + 4);
    unsigned long i = 0;
    while(i < len)
    {
        fread(*Buffer + i, 4, 1, fp);
        i+=4;
    }
    fclose(fp);
    return len;
}

void SaveAs(char *FileName, char *Buffer, unsigned long len)
{
    FILE *fp = fopen(FileName, "wb");
    unsigned long i = 0;
    while(i < len)
    {
        fwrite(Buffer + i, 4, 1, fp);
        fflush(fp);
        i+=4;
    }
    fclose(fp);
}

void WINAPI DumpFile(char *FileName)
{
   /* MODULEINFO dumpinfo;
    DWORD dw = 0;
    GetModuleInformation(GetCurrentProcess(), hmod, &dumpinfo, sizeof MODULEINFO);
   // printf("dump size:%d\n", dumpinfo.SizeOfImage);
   // SaveAs(FileName, (char *)dumpinfo.lpBaseOfDll, dumpinfo.SizeOfImage);*/
}

BOOL WINAPI MyGetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule)
{
    printf("in MyGetModuleHandleExA\n");
    BOOL realbool = false;
    char *lpm = new char[MAX_PATH];
    memset(lpm, 0x0, MAX_PATH);
    if(lpModuleName == NULL)
        strcpy(lpm, PEFile);
    else
        strcpy(lpm, lpModuleName);
    DWORD pNewFunc = (DWORD)addr_GetModuleHandleExA - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push phModule
    __asm push lpm
    __asm push dwFlags
    __asm call pNewFunc
    __asm mov realbool, eax
    printf("realbool:%d\n", realbool);
    delete []lpm;
//    if(*phModule == OldKernel32Address)
//        *phModule = (HMODULE)lpNewBaseOfDll;
    printf("out MyGetModuleHandleExA\n");
    return realbool;
}

BOOL WINAPI MyGetModuleHandleExW(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule)
{
    printf("in MyGetModuleHandleExW\n");
    BOOL realbool = false;
    WCHAR *lpm = new WCHAR[MAX_PATH];
    memset(lpm, 0x0, sizeof(WCHAR) * MAX_PATH);
    if(lpModuleName == NULL)
    {
        swprintf(lpm, L"%s", PEFile);
    //    wcscpy(lpm, L"c:\\a.exe");
    }
    else
        wcscpy(lpm, lpModuleName);
    DWORD pNewFunc = (DWORD)addr_GetModuleHandleExW - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push phModule
    __asm push lpm
    __asm push dwFlags
    __asm call pNewFunc
    __asm mov realbool, eax
    printf("realbool:%d\n", realbool);
    delete []lpm;
//    if(*phModule == OldKernel32Address)
//        *phModule = (HMODULE)lpNewBaseOfDll;
    printf("out MyGetModuleHandleExW\n");
    return realbool;
}

DWORD WINAPI MyGetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
    printf("in MyGetModuleFileNameA\n");
    printf("in MyGetModuleFileNameA:hModule:0x%.8x, lpFilename:%s\n", hModule, lpFilename);
    DWORD realdword = 0;
    if(!hModule)
        hModule = hmod;
    printf("new hmod:0x%.8x\n", hModule);
//    if(hModule == (HMODULE)lpNewBaseOfDll)
//    {
//        strcpy(lpFilename, "Kernel32.dll");
//        return strlen("Kernel32.dll");
//    }
    DWORD pNewFunc = (DWORD)GetModuleFileNameA - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push nSize
    __asm push lpFilename
    __asm push hModule
    __asm call pNewFunc
    __asm mov realdword, eax
    printf("File:%s\n", lpFilename);
    printf("realdword:%d\n", realdword);
    printf("out MyGetModuleFileNameA\n");
    return realdword;
}

DWORD WINAPI MyGetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
    printf("in MyGetModuleFileNameW\n");
    DWORD realdword = 0;
    if(!hModule)
        hModule = hmod;
    printf("hModule:0x%.8x\n", hModule);
//    if(hModule == (HMODULE)lpNewBaseOfDll)
//    {
//        wcscpy(lpFilename, L"Kernel32.dll");
//        return wcslen(L"Kernel32.dll");
//    }
    DWORD pNewFunc = (DWORD)GetModuleFileNameW - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push nSize
    __asm push lpFilename
    __asm push hModule
    __asm call pNewFunc
    __asm mov realdword, eax
    printf("realdword:%d, lpFilename:%S\n", realdword, lpFilename);
    printf("out MyGetModuleFileNameW\n");
    return realdword;
}

HMODULE WINAPI MyGetModuleHandleA(LPCTSTR lpModuleName)
{
    //DumpFile("c:\\ps.exe");
    //printf("in MyGetModuleHandleA\n");
    char *lpm = new char[MAX_PATH];
    memset(lpm, 0x0, MAX_PATH);
    HMODULE realhmod = NULL;
    printf("in MyGetModuleHandleA:%s\n", lpModuleName);
    if(lpModuleName == NULL)
        strcpy(lpm, PEFile);
    else
        strcpy(lpm, lpModuleName);
    DWORD pNewFunc = (DWORD)GetModuleHandleA - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push lpm
    __asm call pNewFunc
    __asm mov realhmod, eax
    delete []lpm;
//    if(realhmod == OldKernel32Address)
//        realhmod = (HMODULE)lpNewBaseOfDll;
    printf("realhmod:0x%.8x\n", realhmod);
    printf("out MyGetModuleHandleA\n");
    return realhmod;
}

HMODULE WINAPI MyGetModuleHandleW(LPCWSTR lpModuleName)
{
    printf("in MyGetModuleHandleW\n");
    WCHAR *lpm = new WCHAR[MAX_PATH];
    memset(lpm, 0x0, sizeof(WCHAR) * MAX_PATH);
    HMODULE realhmod = NULL;
    printf("in MyGetModuleHandleW:%S\n", lpModuleName);
    if(lpModuleName == NULL)
    {
        swprintf(lpm, L"%s", PEFile);
        //wcscpy(lpm, L"c:\\a.exe");
    }
    else
        wcscpy(lpm, lpModuleName);
    DWORD pNewFunc = (DWORD)GetModuleHandleW - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push lpm
    __asm call pNewFunc
    __asm mov realhmod, eax
    delete []lpm;
//    if(realhmod == OldKernel32Address)
//        realhmod = (HMODULE)lpNewBaseOfDll;
    printf("realhmod:0x%.8x\n", realhmod);
    printf("out MyGetModuleHandleW\n");
    return realhmod;
}

HGLOBAL WINAPI MyLoadResource(HMODULE hModule, HRSRC hResInfo)
{
    printf("in MyLoadResource\n");
    HGLOBAL glb = NULL;
    printf("In MyLoadResource\n");
    if(!hModule)
        hModule = hmod;
    DWORD pNewFunc = (DWORD)LoadResource - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push hResInfo
    __asm push hModule
    __asm call pNewFunc
    __asm mov glb, eax
    printf("out MyLoadResource\n");
    return glb;//LoadResource(hModule, hResInfo);
}

HRSRC WINAPI MyFindResourceA(HMODULE hModule, LPCSTR lpName, LPCSTR lpType)
{
    printf("in MyFindResourceA\n");
    HRSRC src;
    if(!hModule)
        hModule = hmod;
    DWORD pNewFunc = (DWORD)FindResourceA - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push lpType
    __asm push lpName
    __asm push hModule
    __asm call pNewFunc
    __asm mov src, eax
    printf("MyFindResourceA\n");
    printf("out MyFindResourceA\n");
    return src;
}

HRSRC WINAPI MyFindResourceW(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
{
    printf("in MyFindResourceW\n");
    HRSRC src;
    if(!hModule)
        hModule = hmod;
    DWORD pNewFunc = (DWORD)FindResourceW - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push lpType
    __asm push lpName
    __asm push hModule
    __asm call pNewFunc
    __asm mov src, eax
    printf("MyFindResourceW\n");
    printf("out MyFindResourceW\n");
    return src;
}

HRSRC WINAPI MyFindResourceExA(HMODULE hModule, LPCSTR lpType, LPCSTR lpName, WORD wLanguage)
{
    printf("in MyFindResourceExA\n");
    HRSRC src;
    if(!hModule)
        hModule = hmod;
    DWORD pNewFunc = (DWORD)FindResourceExA - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push wLanguage
    __asm push lpName
    __asm push lpType
    __asm push hModule
    __asm call pNewFunc
    __asm mov src, eax
    printf("MyFindResourceExA\n");
    printf("out MyFindResourceExA\n");
    return src;
}

HRSRC WINAPI MyFindResourceExW(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, WORD wLanguage)
{
    printf("in MyFindResourceExW\n");
    HRSRC src = NULL;
    if(!hModule)
        hModule = hmod;
    DWORD pNewFunc = (DWORD)FindResourceExW - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push wLanguage
    __asm push lpName
    __asm push lpType
    __asm push hModule
    __asm call pNewFunc
    __asm mov src, eax
    //printf("MyFindResourceExW\n");
    printf("out MyFindResourceExW\n");
    return src;
}

VOID WINAPI MyExitProcess(UINT uExitCode)
{
    printf("ExitProcess\n");
    
	//DumpFile("c:\\ps.exe");
    DWORD pNewFunc = (DWORD)ExitProcess - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push uExitCode
    __asm call pNewFunc
    return;
}

BOOL WINAPI MyTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
    printf("TerminateProcess\n");
    //DumpFile("c:\\ps.exe");
    DWORD pNewFunc = (DWORD)TerminateProcess - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push uExitCode
    __asm push hProcess
    __asm call pNewFunc
    return true;
}

DWORD WINAPI MyFormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list *Arguments)
{
    printf("FormatMessageA:0x%.8x, %d\n", dwFlags, nSize);
    DWORD retdword = 0;
    if(!lpSource)
        lpSource = hmod;
    if(dwFlags == 0x1900)
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE;
    DWORD pNewFunc = (DWORD)FormatMessageA - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push Arguments
    __asm push nSize
    __asm push lpBuffer
    __asm push dwLanguageId
    __asm push lpSource
    __asm push dwFlags
    __asm call pNewFunc
    __asm mov retdword, eax
    printf("msg:%s\n", lpBuffer);
    return retdword;
}

HICON WINAPI MyLoadIconA(HINSTANCE hInstance, LPCSTR lpIconName)
{
    HICON reticon = NULL;
    DWORD pNewFunc = (DWORD)LoadIconA - (DWORD)mi1.lpBaseOfDll + (DWORD)lpNewBaseOfDll1;
    if(hInstance == NULL)
        hInstance = hmod;
    __asm push lpIconName
    __asm push hInstance
    __asm call pNewFunc
    __asm mov reticon, eax
    return reticon;
}

HANDLE WINAPI MyLoadImageA(HINSTANCE hinst, LPCTSTR lpszName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad)
{
    HANDLE rethand = NULL;
    DWORD pNewFunc = (DWORD)LoadImageA - (DWORD)mi1.lpBaseOfDll + (DWORD)lpNewBaseOfDll1;
    if(hinst == NULL)
        hinst = hmod;
    __asm push fuLoad
    __asm push cyDesired
    __asm push cxDesired
    __asm push uType
    __asm push lpszName
    __asm push hinst
    __asm call pNewFunc
    __asm mov rethand, eax
    return rethand;
}

HANDLE WINAPI MyLoadImageW(HINSTANCE hinst, LPCWSTR lpszName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad)
{
    HANDLE rethand = NULL;
    DWORD pNewFunc = (DWORD)LoadImageW - (DWORD)mi1.lpBaseOfDll + (DWORD)lpNewBaseOfDll1;
    if(hinst == NULL)
        hinst = hmod;
    __asm push fuLoad
    __asm push cyDesired
    __asm push cxDesired
    __asm push uType
    __asm push lpszName
    __asm push hinst
    __asm call pNewFunc
    __asm mov rethand, eax
    return rethand;
}

HICON WINAPI MyLoadIconW(HINSTANCE hInstance, LPCWSTR lpIconName)
{
    HICON reticon = NULL;
    DWORD pNewFunc = (DWORD)LoadIconW - (DWORD)mi1.lpBaseOfDll + (DWORD)lpNewBaseOfDll1;
    if(hInstance == NULL)
        hInstance = hmod;
    __asm push lpIconName
    __asm push hInstance
    __asm call pNewFunc
    __asm mov reticon, eax
    return reticon;
}

DWORD WINAPI MyGetVersion()
{
    printf("================================GetVersion=====================================\n");
    DWORD retdword = 0;
    //exit(1);
    //DumpFile("c:\\ps.exe");
    DWORD pNewFunc = (DWORD)GetVersion - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm call pNewFunc
    __asm mov retdword, eax
    return retdword;
}

DWORD WINAPI MyGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    printf("================================GetProcAddress=================================\n");
    DWORD retdword = 0;
    if(lpProcName)
        printf("GetProcAddress:%s\n", lpProcName);
//    else
//        printf("GetProcAddress:NULL\n");
    //exit(1);
    DWORD pNewFunc = (DWORD)GetProcAddress - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push lpProcName
    __asm push hModule
    __asm call pNewFunc
    __asm mov retdword, eax
    printf("GetProcAddress:%s,0x%.8x\n", lpProcName, retdword);
    return retdword;
}

DWORD WINAPI MyFormatMessageW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPWSTR lpBuffer, DWORD nSize, va_list *Arguments)
{
    printf("FormatMessageW:0x%.8x, %d\n", dwFlags, nSize);
    if(!lpSource)
        lpSource = hmod;
    DWORD retdword = 0;
    if(dwFlags == 0x1900)
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE;
    DWORD pNewFunc = (DWORD)FormatMessageW - (DWORD)mi.lpBaseOfDll + (DWORD)lpNewBaseOfDll;
    __asm push Arguments
    __asm push nSize
    __asm push lpBuffer
    __asm push dwLanguageId
    __asm push lpSource
    __asm push dwFlags
    __asm call pNewFunc
    __asm mov retdword, eax
    printf("msg:%S\n", lpBuffer);
    return retdword;
}

typedef struct{BYTE mov_eax;LPVOID address;WORD jump_eax;}ASMJUMP, *PASMJUMP;//0xB8//0xE0FF

void WINAPI HookAPI(int s)
{
    OldKernel32Address = GetModuleHandle("Kernel32.dll");
    OldUser32Address = GetModuleHandle("User32.dll");
    char *pGetVersion = (char *)GetProcAddress(OldKernel32Address, "GetVersion");
    char *pLoadIconA = (char *)GetProcAddress(OldUser32Address, "LoadIconA");
    char *pLoadIconW = (char *)GetProcAddress(OldUser32Address, "LoadIconW");
    char *pLoadImageA = (char *)GetProcAddress(OldUser32Address, "LoadImageA");
    char *pLoadImageW = (char *)GetProcAddress(OldUser32Address, "LoadImageW");
    char *pFormatMessageA = (char *)GetProcAddress(OldKernel32Address, "FormatMessageA");
    char *pFormatMessageW = (char *)GetProcAddress(OldKernel32Address, "FormatMessageW");
    char *pLoadResource = (char *)GetProcAddress(OldKernel32Address, "LoadResource");
    char *pGetModuleHandleW = (char *)GetProcAddress(OldKernel32Address, "GetModuleHandleW");
    char *pGetModuleHandleA = (char *)GetProcAddress(OldKernel32Address, "GetModuleHandleA");
    char *pExitProcess = (char *)GetProcAddress(OldKernel32Address, "ExitProcess");
    char *pTerminateProcess = (char *)GetProcAddress(OldKernel32Address, "TerminateProcess");
    char *pGetModuleHandleExA = (char *)GetProcAddress(OldKernel32Address, "GetModuleHandleExA");
    char *pGetModuleHandleExW = (char *)GetProcAddress(OldKernel32Address, "GetModuleHandleExW");
    addr_GetModuleHandleExA = pGetModuleHandleExA;
    addr_GetModuleHandleExW = pGetModuleHandleExW;
    char *pGetModuleFileNameA = (char *)GetProcAddress(OldKernel32Address, "GetModuleFileNameA");
    char *pGetModuleFileNameW = (char *)GetProcAddress(OldKernel32Address, "GetModuleFileNameW");
    char *pFindResourceA = (char *)GetProcAddress(OldKernel32Address, "FindResourceA");
    char *pFindResourceW = (char *)GetProcAddress(OldKernel32Address, "FindResourceW");
    char *pFindResourceExA = (char *)GetProcAddress(OldKernel32Address, "FindResourceExA");
    char *pFindResourceExW = (char *)GetProcAddress(OldKernel32Address, "FindResourceExW");
    if(!GetModuleInformation(GetCurrentProcess(), OldKernel32Address, &mi, sizeof MODULEINFO)) return;
    if(!GetModuleInformation(GetCurrentProcess(), OldUser32Address, &mi1, sizeof MODULEINFO)) return;
    MEMORY_BASIC_INFORMATION mbi;
    ASMJUMP    jmpcode;
    jmpcode.mov_eax = (BYTE)0xB8;
    jmpcode.jump_eax = (WORD)0xE0FF;
    DWORD dw;
    //lpNewBaseOfDll = new char[mi.SizeOfImage];
    if(!lpNewBaseOfDll) return;
    lpNewBaseOfDll1 = new char[mi1.SizeOfImage];
    if(!lpNewBaseOfDll) return;
        memcpy(lpNewBaseOfDll, mi.lpBaseOfDll, mi.SizeOfImage);
    if(!lpNewBaseOfDll1) return;
        memcpy(lpNewBaseOfDll1, mi1.lpBaseOfDll, mi1.SizeOfImage);
    {
        if(!VirtualQuery((void *)pGetVersion, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyGetVersion;
        memcpy((void *)pGetVersion, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pLoadImageA, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyLoadImageA;
        memcpy((void *)pLoadImageA, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pLoadImageW, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyLoadImageW;
        memcpy((void *)pLoadImageW, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pLoadIconA, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyLoadIconA;
        memcpy((void *)pLoadIconA, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pLoadIconW, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyLoadIconW;
        memcpy((void *)pLoadIconW, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pFormatMessageW, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyFormatMessageW;
        memcpy((void *)pFormatMessageW, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pFormatMessageA, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyFormatMessageA;
        memcpy((void *)pFormatMessageA, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pGetModuleHandleExA, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyGetModuleHandleExA;
        memcpy((void *)pGetModuleHandleExA, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pGetModuleHandleExW, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyGetModuleHandleExW;
        memcpy((void *)pGetModuleHandleExW, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pGetModuleFileNameA, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyGetModuleFileNameA;
        memcpy((void *)pGetModuleFileNameA, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pGetModuleFileNameW, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyGetModuleFileNameW;
        memcpy((void *)pGetModuleFileNameW, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pLoadResource, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyLoadResource;
        memcpy((void *)pLoadResource, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pFindResourceA, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyFindResourceA;
        memcpy((void *)pFindResourceA, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pFindResourceW, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyFindResourceW;
        memcpy((void *)pFindResourceW, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pFindResourceExA, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyFindResourceExA;
        memcpy((void *)pFindResourceExA, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pFindResourceExW, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyFindResourceExW;
        memcpy((void *)pFindResourceExW, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pGetModuleHandleW, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyGetModuleHandleW;
        memcpy((void *)pGetModuleHandleW, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pGetModuleHandleA, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyGetModuleHandleA;
        memcpy((void *)pGetModuleHandleA, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pExitProcess, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyExitProcess;
        memcpy((void *)pExitProcess, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)pTerminateProcess, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyTerminateProcess;
        memcpy((void *)pTerminateProcess, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    {
        if(!VirtualQuery((void *)GetProcAddress, &mbi, sizeof MEMORY_BASIC_INFORMATION)) return;
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dw)) return;
        jmpcode.address = (void *)MyGetProcAddress;
        if(s == 1)
            memcpy((void *)GetProcAddress, (unsigned char *)&jmpcode, sizeof ASMJUMP);
        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dw)) return;
    }
    return;
}

void main(int argc, char **argv)
{
    char *Buffer = NULL;
    if(argc >= 2)
        strcpy(PEFile, argv[1]);
    else
        //strcpy(PEFile, "C:\\VMUnpacker\\VMUnpacker.exe");
    // strcpy(PEFile, "c:\\windows\\system32\\cmd.exe");
       strcpy(PEFile, "c:\\windows\\system32\\notepad.exe"); 
	//strcpy(PEFile, "C:\\Program Files\\Microsoft Office\\Office12\\winword.EXE");
    char lpBuffer[MAX_PATH] = {0};
    char *p = NULL;
    GetFullPathName(PEFile, MAX_PATH, lpBuffer, &p);
    char *s = lpBuffer + (p - lpBuffer);
    s[0] = '\0';
    printf("path:%s\n", lpBuffer);
    SetCurrentDirectory(lpBuffer);
    unsigned long len = LoadPEFile(PEFile, &Buffer);
    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)Buffer;
    IMAGE_NT_HEADERS *pINH = (IMAGE_NT_HEADERS *)((DWORD)dos_header + dos_header->e_lfanew);
//    long PESignOffset = *(long *)(Buffer + 0x3c);
//    IMAGE_NT_HEADERS *pINH = (IMAGE_NT_HEADERS *)(Buffer + PESignOffset);
    pINH->FileHeader.Characteristics |= IMAGE_FILE_DLL;
    pINH->FileHeader.Characteristics |= IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP;
    OEP = pINH->OptionalHeader.AddressOfEntryPoint;
    printf("OldEntryPoint:0x%.8x\n", pINH->OptionalHeader.AddressOfEntryPoint);
    pINH->OptionalHeader.AddressOfEntryPoint = 0x0;
    printf("pINH->OptionalHeader.AddressOfEntryPoint:0x%.8x\n", pINH->OptionalHeader.AddressOfEntryPoint);
    printf("len:%d\n", len);
/*    unsigned long HeaderSum = 0;
    unsigned long CheckSum = 0;
    pINH->OptionalHeader.CheckSum = 0;
    CheckSumMappedFile(Buffer, len, &HeaderSum, &CheckSum);
    pINH->OptionalHeader.CheckSum = CheckSum;
*/   //strcat(PEFile, ".new.exe");
    //SaveAs(PEFile, Buffer, len);
    delete []Buffer;
    Buffer = NULL;
    printf("Load\n");
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
        if(!VirtualProtect((char *)pINH, sizeof(IMAGE_NT_HEADERS), PAGE_EXECUTE_READWRITE, &dw)) return;
        pINH->OptionalHeader.AddressOfEntryPoint = OEP;
        printf("pINH->FileHeader.Characteristics:0x%.8x\n", pINH->FileHeader.Characteristics);
        pINH->FileHeader.Characteristics &= ~IMAGE_FILE_DLL;
        printf("pINH->FileHeader.Characteristics:0x%.8x\n", pINH->FileHeader.Characteristics);
        if(!VirtualProtect((char *)pINH, sizeof(IMAGE_NT_HEADERS), PAGE_EXECUTE_READ, &dw)) return;
        printf("Call\n");
        DWORD id = 0;
        if(argc == 3)
            HookAPI(1);
        else
            HookAPI(0);
        OEP += (DWORD)hmod;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OEP, 0, 0, &id);
       // printf("OK!\n");
        HANDLE hOut;
        COORD Position;
        DWORD Written;
        char string[80] = {0};
        WORD c = FOREGROUND_RED | FOREGROUND_INTENSITY;
        Position.X = 0;
        Position.Y = 0;
        while(1)
        {
            Position.X = 0;
            hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            SYSTEMTIME t;
            GetLocalTime(&t);
            
           // sprintf(string, "PELoader ver 1.0 PID:%d HookGetProcAddress:%d Time:%d/%d/%d %d:%d:%d:%d    ", GetCurrentProcessId(), argc == 3?1:0, t.wMonth, t.wDay, t.wYear, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
            SetConsoleTextAttribute(hOut, FOREGROUND_GREEN);
            WriteConsoleOutputCharacter(hOut, string, strlen(string), Position, &Written);
            while(Position.X <= strlen(string))
            {
                WriteConsoleOutputAttribute(hOut, &c, 1, Position, &Written);
                Position.X++;
            }
            Sleep(50);
        }
    }
    delete []lpNewBaseOfDll;
    delete []lpNewBaseOfDll1;
    FreeLibrary(hmod);
    DeleteFile(PEFile);
}