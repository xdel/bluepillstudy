/* 
 * Copyright holder: Invisible Things Lab
 * 
 * This software is protected by domestic and International
 * copyright laws. Any use (including publishing and
 * distribution) of this software requires a valid license
 * from the copyright holder.
 *
 * This software is provided for the educational use only
 * during the Black Hat training. This software should not
 * be used on production systems.
 *
 */
 
 /* Copyright (C) 2010 Trusted Computing Lab in Shanghai Jiaotong University
 * 
 * 09/10/11	Miao Yu <superymkfounder@hotmail.com>
 */

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
 **/
NTSTATUS NTAPI DbgPrintInfo (PUCHAR fmt,...);

/**
 * Effects: Initialize SpinLock, must be called before invoke WriteDbgInfo function
 **/
void NTAPI DbgInitComponent();

void NTAPI DbgDisposeComponent();