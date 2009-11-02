//#include <stdio.h>
//#include <string.h>
#include <stdarg.h>
#include "PrintInfos.h"

/**************Inner Functions**************/

static NTSTATUS _CreateDebugWindow(ULONG32 numContinuousPages);

static VOID _AppendStringToAddress(PUCHAR str,ULONG32 strLength);

/**
 * Effects: Append the string <str> into the end of the debug window
 * If the debug window not exists, then it will be created at first.
 * 把<str>续写到调试窗口末尾，如果调试窗口不存在，则创建调试窗口
 **/
static NTSTATUS NTAPI _WriteInfo(PUCHAR str,ULONG32 strLength);

//Forward Function Lookup
extern VOID NTAPI CmInitSpinLock (
  PPRINTSPIN_LOCK BpSpinLock
);

extern VOID NTAPI CmAcquireSpinLock (
  PPRINTSPIN_LOCK BpSpinLock
);

extern VOID NTAPI CmReleaseSpinLock (
  PPRINTSPIN_LOCK BpSpinLock
);

/**************Definitions**************/
#define snprintf _snprintf

PVOID g_debugWindowAddrVA = 0;

ULONG32 appendIndex = 0;// The index the next string should append.

ULONG32 bufferLength = 0;//Total Buffer Length, used in buffer overflow check.

static PRINTSPIN_LOCK g_PrintSpinLock;

/**************Implementations***********/
/**
 * Effects: Write info with format.
 * 带格式写String
 **/
NTSTATUS NTAPI WriteDbgInfo (PUCHAR fmt,...)
{
	va_list args;
	UCHAR str[1024] = { 0 };
	ULONG32 length ;

	va_start (args, fmt);
	//length = snprintf (str,sizeof(str),fmt,args);
	length = vsnprintf (str, sizeof (str), (PUCHAR) fmt, args);

	return _WriteInfo(str,length);

}

void NTAPI WriteInfoInit()
{
	CmInitSpinLock (&g_PrintSpinLock);
}

void NTAPI WriteInfoDispose()
{
	if(g_debugWindowAddrVA)
	{
		ExFreePoolWithTag(g_debugWindowAddrVA,DEBUG_WINDOW_TAG);
	}
}
static NTSTATUS _CreateDebugWindow(ULONG32 numContinuousPages)
{
	PHYSICAL_ADDRESS l1, l2, l3;
	l1.QuadPart = 0;
	l2.QuadPart = -1;
	l3.QuadPart = 0x200000;

	//g_debugWindowAddrVA = MmAllocateContiguousMemorySpecifyCache (numContinuousPages * PAGE_SIZE, l1, l2, l3, MmCached);
	g_debugWindowAddrVA = ExAllocatePoolWithTag (NonPagedPool, numContinuousPages * PAGE_SIZE, DEBUG_WINDOW_TAG);
	RtlZeroMemory (g_debugWindowAddrVA, numContinuousPages * PAGE_SIZE);
	
	if(!g_debugWindowAddrVA)
	{
		bufferLength = 0;
		return STATUS_UNSUCCESSFUL;
	}

	DbgPrint("debugWindowAddrVA :0x%llX\n",g_debugWindowAddrVA);
	bufferLength = numContinuousPages * PAGE_SIZE;
	return STATUS_SUCCESS;
}

static VOID _AppendStringToAddress(PUCHAR str,ULONG32 strLength)
{
	//Step 1.Overflow Check
	if(appendIndex+strLength+2>=bufferLength)
	{
		return;
	}
	//Step 1.Append <str> at the end of the debug window.
	memcpy(&((PUCHAR)g_debugWindowAddrVA)[appendIndex],(PVOID)str,strLength);
	//Step 2.Append a '\0' at the end of this str.
	appendIndex += (strLength+2);
	((PUCHAR)g_debugWindowAddrVA)[appendIndex-2] = '\r';
	((PUCHAR)g_debugWindowAddrVA)[appendIndex-1] = '\n';
}

/**
 * Effects: Append the string <str> into the end of the debug window
 * If the debug window not exists, then it will be created at first.
 * 把<str>续写到调试窗口末尾，如果调试窗口不存在，则创建调试窗口
 **/
static NTSTATUS NTAPI _WriteInfo(PUCHAR str,ULONG32 strLength)
{
	CmAcquireSpinLock (&g_PrintSpinLock);
	//Step 1. Check if the debug window exists.
	if(!g_debugWindowAddrVA)
	{
		if(!NT_SUCCESS(_CreateDebugWindow(NUM_DEBUG_PAGES)))
		{
			//Continous Memory Allocate Failed!
			return STATUS_UNSUCCESSFUL;
		}
	}
	//Step 2.Append the <str> at the end of the debug window.
	_AppendStringToAddress(str,strLength);
	CmReleaseSpinLock (&g_PrintSpinLock);
	return STATUS_SUCCESS;
}