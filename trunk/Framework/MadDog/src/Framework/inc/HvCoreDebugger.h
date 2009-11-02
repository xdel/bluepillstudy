#pragma once

#include <ntddk.h>

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
#define USE_DEBUG_LIBRUARY
#ifdef USE_DEBUG_LIBRUARY
#define Print(x) DbgPrintInfo x
#else
#define Print(x) {}
#endif
#define HvmPrint(x) DbgPrint x

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * Effects: Write info with format.
 * 带格式写String
 **/
NTSTATUS NTAPI DbgPrintInfo (PUCHAR fmt,...);

/**
 * Effects: Initialize SpinLock, must be called before invoke WriteDbgInfo function
 * 初始化写信息自旋锁,必须在调用WriteDbgInfo方法前调用
 **/
void NTAPI DbgInitComponent();

void NTAPI DbgDisposeComponent();