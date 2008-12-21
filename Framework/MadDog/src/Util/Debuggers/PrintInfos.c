#include "PrintInfos.h"

PVOID g_debugWindowAddrVA = 0;

ULONG32 appendIndex = 0;// The index the next string should append.

/**
 * Effects: Append the string <str> into the end of the debug window
 * If the debug window not exists, then it will be created at first.
 * 把<str>续写到调试窗口末尾，如果调试窗口不存在，则创建调试窗口
 **/
NTSTATUS NTAPI WriteInfo(char *str,ULONG32 strLength)
{
	//Step 1. Check if the debug window exists.
	if(!g_debugWindowAddrVA)
	{
		if(!NT_SUCCESS(CreateDebugWindow(NUM_DEBUG_PAGES)))
		{
			//Continous Memory Allocate Failed!
			return STATUS_UNSUCCESSFUL;
		}
	}
	//Step 2.Append the <str> at the end of the debug window.
	AppendStringToAddress(g_debugWindowAddrVA,str,strLength);
	return STATUS_SUCCESS;
}

static NTSTATUS CreateDebugWindow(ULONG32 numContinuousPages)
{
	PHYSICAL_ADDRESS l1, l2, l3;
	l1.QuadPart = 0;
	l2.QuadPart = -1;
	l3.QuadPart = 0x200000;

	g_debugWindowAddrVA = MmAllocateContiguousMemorySpecifyCache (numContinuousPages * PAGE_SIZE, l1, l2, l3, MmCached);
	if(!g_debugWindowAddrVA)
		return STATUS_UNSUCCESSFUL;
	return STATUS_SUCCESS;
}

static VOID AppendStringToAddress(PVOID addrVA,char* str,ULONG32 strLength)
{
	//Step 1.Append <str> at the end of the debug window.
	memcpy(((char *)g_debugWindowAddrVA)[appendIndex],(PVOID)str,strLength);
	//Step 2.Append a '\0' at the end of this str.
	appendIndex += strLength+1;
	((char *)g_debugWindowAddrVA)[appendIndex-1] = 0;
}