#include "Hook.h"
#include <string.h>
#include "Util.h"
#include "Vmxtraps.h"

PVOID KDEEntryAddr;
ULONG g_uCr0;

BYTE g_HookCode[5] = { 0xe9, 0, 0, 0, 0 };
BYTE g_OrigCode[5] = { 0 }; 
BYTE jmp_orig_code[7] = { 0xEA, 0, 0, 0, 0, 0x08, 0x00 }; 

BOOL g_bHooked = FALSE;

PVOID KDECallRetAddr;

VOID fake_KiDispatchException (
    IN PVOID ExceptionRecord,
    IN PVOID ExceptionFrame,
    IN PVOID TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance);
   
VOID Proxy_KiDispatchException (
     IN PVOID ExceptionRecord,
    IN PVOID ExceptionFrame,
    IN PVOID TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance);


PVOID GetKiDispatchExceptionAddr()
{
	BYTE 	KDEFuncSignaturePart1[61] = {
		0x89, 0x45, 0xe4, 0x8b, 0x75, 0x08, 0x89, 0xb5,
		0x14, 0xfd, 0xff, 0xff, 0x8b, 0x4d, 0x0c,0x89,
		0x8d, 0x10, 0xfd, 0xff, 0xff, 0x8b, 0x5d, 0x10,
		0x89, 0x9d, 0x08, 0xfd, 0xff, 0xff, 0x64, 0xa1,
		0x20, 0x00, 0x00, 0x00, 0xff, 0x80, 0x04, 0x05,
		0x00, 0x00, 0xc7, 0x85, 0x18, 0xfd, 0xff, 0xff,
		0x17, 0x00, 0x01, 0x00, 0x80, 0x7d, 0x14, 0x01,
		0x74, 0x09, 0x80, 0x3d, 0x41};
	BYTE 	KDEFuncSignaturePart2[17] = {
		0x55, 0x80, 0x00, 0x74, 0x1d, 0xc7, 0x85, 0x18, 
		0xfd, 0xff, 0xff, 0x1f, 0x00, 0x01, 0x00, 0x80, 
		0x3d};
		
	BYTE 	KDEFuncSignaturePart3[24] = {
		0x56, 0x80, 0x00, 0x74, 0x0a, 0xc7, 0x85, 0x18, 
		0xfd, 0xff, 0xff, 0x3f, 0x00, 0x01, 0x00, 0x8d, 
		0x85, 0x18, 0xfd, 0xff, 0xff, 0x50, 0x51, 0x53
	};
	char* p;

	p=Memsearch((char*)(0x804f0000),(char*)KDEFuncSignaturePart1,16*4096,61);
	if(p)
	{
		//DbgPrint("Found Part1!");
		//if(strncmp((p+1),KDEFuncSignaturePart2,17))
		{
			if( *(p-20) == 0x68)
				return (PVOID)(p-20);
		}
	}

	return 0;

	
}

//
// inline hook --  KiDispatchException
//
VOID HookKiDispatchException ()
{
	AcquireSpinLock();

	if(g_bHooked)
		return;

	KDEEntryAddr = GetKiDispatchExceptionAddr();
	DbgPrint("Initialization: KDEEntryAddr = 0x%llX\n", KDEEntryAddr);

    if (KDEEntryAddr == 0) {
        DbgPrint("KiDispatchException == NULL\n");
        return;
    }

    Memcpy (g_OrigCode, (BYTE*)KDEEntryAddr, 5);//&iexcl;&iuml;
    *( (ULONG*)(g_HookCode + 1) ) = (ULONG)fake_KiDispatchException - (ULONG)KDEEntryAddr - 5;//&iexcl;&iuml;
     
	WPOFF();
 
    Memcpy ( (BYTE*)KDEEntryAddr, g_HookCode, 5 );
    *( (ULONG*)(jmp_orig_code + 1) ) = (ULONG) ( (BYTE*)KDEEntryAddr + 5 );//&iexcl;&iuml;
   
    Memcpy ( (BYTE*)Proxy_KiDispatchException, g_OrigCode, 5);
    Memcpy ( (BYTE*)Proxy_KiDispatchException + 5, jmp_orig_code, 7);
    
    WPON();
	

    g_bHooked = TRUE;

	ReleaseSpinLock();
}

//
//
VOID UnHookKiDispatchException ()
{

	//AcquireSpinLock();

	if(!g_bHooked)
		return;

    WPOFF();
      
    Memcpy ( (BYTE*)KDEEntryAddr, g_OrigCode, 5 );

    WPON();
	
    g_bHooked = FALSE;

	//ReleaseSpinLock();
}

__declspec (naked) VOID KDEReturnTo()
{

	__asm
	{
		pushfd;
		pushad; //Backup 
	}

	//Hypercall
	__asm
	{
		mov eax,FN_EXITKDE; //FN_EXITKDE Hypercall
		cpuid;
	}

	 //DbgPrint("inline hook --  KiDispatchException Exit\n");

	__asm
	{
		popad;
		popfd; //Restore
	}
	__asm
	{
		jmp KDECallRetAddr;//Jump to the KDE function caller.
	}
}
//
//
ULONG KDEEnterESP;
__declspec (naked)
VOID
fake_KiDispatchException (
    IN PVOID ExceptionRecord,
    IN PVOID ExceptionFrame,
    IN PVOID TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance)
{
	__asm
	{
		mov KDEEnterESP,esp;
	}
	__asm
	{
		pushfd;
		pushad;
	}
	__asm
	{
		mov ebx,esp; //Backup esp, restore it later.

		mov esp,KDEEnterESP;

		mov eax,[esp];
		mov KDECallRetAddr,eax;//Store the KiDispatchException return address

		mov eax, KDEReturnTo
		mov [esp],eax ;//Modify Return Address

		mov esp,ebx; //Restore esp to point to the top of 'pushad' 
		
	}


	//Hypercall
	__asm
	{
		mov eax,FN_ENTERKDE; //FN_ENTERKDE Hypercall
		cpuid;
	}

    //DbgPrint("inline hook --  KiDispatchException Entry\n");
 

	//Restore & Jump to KDE
    __asm
    {
		popad;
		popfd;
               jmp Proxy_KiDispatchException   
    }
}

//
//
__declspec (naked)
VOID
Proxy_KiDispatchException (
    IN PVOID ExceptionRecord,
    IN PVOID ExceptionFrame,
    IN PVOID TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance)
{
 
    __asm {  
            _emit 0x90
            _emit 0x90
            _emit 0x90
            _emit 0x90
            _emit 0x90  //
            _emit 0x90  //
            _emit 0x90
            _emit 0x90
            _emit 0x90
            _emit 0x90 
            _emit 0x90 
            _emit 0x90 // 
    }
}


