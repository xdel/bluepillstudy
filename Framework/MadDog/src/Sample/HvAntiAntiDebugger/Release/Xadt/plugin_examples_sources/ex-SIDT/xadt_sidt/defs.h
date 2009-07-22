#define _CRT_SECURE_NO_DEPRECATE

#include        <windows.h>
#include        <winioctl.h>
#include        <ntsecapi.h>
#include        <stdio.h>
#include        "xADT_PDK.h"

#pragma comment (lib, "xADT.lib")

NTSTATUS LoadDriver();
#define get_ints CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define get_drs  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifndef _countof
#define _countof(array) \
	(sizeof(array)/sizeof(array[0]))
#endif

enum {DEBUG_TRAP=0, BREAKPOINT_TRAP, DEBUG_OUTPUT };

#define SIDT_TIME_INTERVAL	50
#define SIDT_REPETITIONS	10