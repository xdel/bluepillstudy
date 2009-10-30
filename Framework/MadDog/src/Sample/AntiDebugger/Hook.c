#include "Hook.h"
#include <string.h>
#include "Util.h"

static PVOID pfn_Target;//Target func. address

static UCHAR orign_code[5]={0x00,0x00,0x00,0x00,0x00};//Used to store the origin func. header

static UCHAR myjmp_code[5]={0xE9,0x00,0x00,0x00,0x00};//The jump inst. to be written

static UCHAR jmpToOrign_code[5]={0xE9,0x00,0x00,0x00,0x00};

static ULONG offset_jmpToMyFunc;

static ULONG offset_jmpToOrign;

static PUCHAR pfn_jmpbk;
ULONG dummy_fn[] = {0x90909090, 0x90909090, 0x90909090};


static void copyfnHeader(PVOID fnaddr, PVOID newFunc)

{

         pfn_jmpbk=(PUCHAR)dummy_fn;

         RtlCopyMemory(orign_code,fnaddr,5);//保存原始指令

         //计算跳转偏移

         offset_jmpToMyFunc =(ULONG)newFunc-(ULONG)fnaddr-5;

        /*
       */

         offset_jmpToOrign =(ULONG)fnaddr-(ULONG)pfn_jmpbk-5;

         RtlCopyMemory(myjmp_code+1,&offset_jmpToMyFunc,4);

         RtlCopyMemory(jmpToOrign_code+1,&offset_jmpToOrign,4);

}

PVOID Hook(PVOID fnaddr, PVOID newFunc)

{

	NTSTATUS status = STATUS_SUCCESS;

	//KIRQL old_irql;         

	if (fnaddr==NULL|| newFunc==NULL)
		return NULL;

	copyfnHeader(fnaddr, newFunc);


	//old_irql=KeRaiseIrqlToDpcLevel();
	AcquireSpinLock();

	//__asm {cli}

	RtlCopyMemory(pfn_jmpbk,orign_code,5);//Origin func header

	RtlCopyMemory(pfn_jmpbk+5,jmpToOrign_code,5);

	RtlCopyMemory(fnaddr,myjmp_code,5);

	//__asm {sti}

	//KeLowerIrql(old_irql);
	ReleaseSpinLock();

	return (PVOID)dummy_fn;

}

VOID UnHook(PVOID fnaddr)

{

	//KIRQL old_irql;         


	if (fnaddr==NULL)
		return;
	//old_irql=KeRaiseIrqlToDpcLevel();
	AcquireSpinLock();

	// __asm {cli}

	RtlCopyMemory(fnaddr,orign_code,5);//Restore func header

	//__asm {sti}

	//KeLowerIrql(old_irql);
	ReleaseSpinLock();

	return;

}

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


