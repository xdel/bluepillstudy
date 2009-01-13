#pragma once

#include <ntddk.h>
#include "snprintf.h"

#define NUM_DEBUG_PAGES 20

extern PVOID g_debugWindowAddrVA;
typedef ULONG PRINTSPIN_LOCK, *PPRINTSPIN_LOCK;
#define DEBUG_WINDOW_TAG 'DBG'
/*************************Public Functions*****************/

/**
 * Effects: Write info with format.
 * 带格式写String
 **/
NTSTATUS NTAPI WriteDbgInfo (PUCHAR fmt,...);

/**
 * Effects: Initialize SpinLock, must be called before invoke WriteDbgInfo function
 * 初始化写信息自旋锁,必须在调用WriteDbgInfo方法前调用
 **/
void NTAPI WriteInfoInit();

void NTAPI WriteInfoDispose();

/**************Private Functions**************/

static NTSTATUS _CreateDebugWindow(ULONG32 numContinuousPages);

static VOID _AppendStringToAddress(PUCHAR str,ULONG32 strLength);

/**
 * Effects: Append the string <str> into the end of the debug window
 * If the debug window not exists, then it will be created at first.
 * 把<str>续写到调试窗口末尾，如果调试窗口不存在，则创建调试窗口
 **/
static NTSTATUS NTAPI _WriteInfo(PUCHAR str,ULONG32 strLength);