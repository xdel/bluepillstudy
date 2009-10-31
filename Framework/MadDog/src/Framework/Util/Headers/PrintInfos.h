#pragma once

#include <ntddk.h>
#include "snprintf.h"

#define NUM_DEBUG_PAGES 200

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