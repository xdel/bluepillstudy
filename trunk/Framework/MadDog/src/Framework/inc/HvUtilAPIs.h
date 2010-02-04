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
#include "HvCore.h"
#include "msr.h"
#include "regs.h"

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++
/**
 * effects:A Default Implementation of Initialize Segment Selector.
 */
NTSTATUS NTAPI MadDog_InitializeSegmentSelector (
    SEGMENT_SELECTOR *pSegmentSelector,
    USHORT Selector,
    PUCHAR GdtBase
);

VOID NTAPI MadDog_GetCpuIdInfo (
  ULONG32 fn,
  OUT PULONG32 ret_eax,
  OUT PULONG32 ret_ebx,
  OUT PULONG32 ret_ecx,
  OUT PULONG32 ret_edx
);

VOID NTAPI MadDog_CpuidWithEcxEdx (
  IN OUT PULONG32 ret_ecx,
  IN OUT PULONG32 ret_edx
);

/**
 * effects: Let the indicated processor run the function.
 */
NTSTATUS NTAPI MadDog_DeliverToProcessor (
  CCHAR cProcessorNumber,
  PCALLBACK_PROC CallbackProc,
  PVOID CallbackParam,
  PNTSTATUS pCallbackStatus,
  BOOLEAN needRaiseIRQL
);

/**
 * effects: Let all processor run the function.
 */
NTSTATUS NTAPI MadDog_DeliverToAllProcessors (
  PCALLBACK_PROC CallbackProc,
  PVOID CallbackParam,
  BOOLEAN needRaiseIRQL
);