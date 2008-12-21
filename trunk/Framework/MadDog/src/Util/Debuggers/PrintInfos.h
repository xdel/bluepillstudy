#pragma once

#include <ntddk.h>

#define NUM_DEBUG_PAGES 10

extern PVOID g_debugWindowAddrVA;

/**
 * Effects: Append the string <str> into the end of the debug window
 * If the debug window not exists, then it will be created at first.
 * 把<str>续写到调试窗口末尾，如果调试窗口不存在，则创建调试窗口
 **/
NTSTATUS NTAPI WriteInfo(char *str,ULONG32 strLength);

static NTSTATUS CreateDebugWindow(ULONG32 numContinuousPages);

static VOID AppendStringToAddress(PVOID addrVA,char* str,ULONG32 strLength);