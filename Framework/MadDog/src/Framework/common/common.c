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

#include "common.h"

/**
 * effects:To see if the indicated bit is set or not.
 * requires: 0<=bitNo<=63
 */
BOOLEAN CmIsBitSet (
  ULONG64 v,
  UCHAR bitNo
)
{//Finished
  ULONG64 mask = (ULONG64) 1 << bitNo;
  return (BOOLEAN) ((v & mask) != 0);
}

/**
 * effects:Raise the interruption level to dispatch level, then
 * install VM Root hypervisor by call <CallbackProc>
 */
NTSTATUS NTAPI CmDeliverToProcessor (
  CCHAR cProcessorNumber,
  PCALLBACK_PROC CallbackProc,
  PVOID CallbackParam,
  PNTSTATUS pCallbackStatus,
  BOOLEAN needRaiseIRQL
)
{ //Finish//SAME
  NTSTATUS CallbackStatus;
  KIRQL OldIrql;

  if (!CallbackProc)
    return STATUS_INVALID_PARAMETER;

  if (pCallbackStatus)
    *pCallbackStatus = STATUS_UNSUCCESSFUL;

  KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));
  if(needRaiseIRQL)
  	OldIrql = KeRaiseIrqlToDpcLevel ();
  CallbackStatus = CallbackProc (CallbackParam);
  if(needRaiseIRQL)
  	KeLowerIrql (OldIrql);

  KeRevertToUserAffinityThread ();

  // save the status of the callback which has run on the current core
  if (pCallbackStatus)
    *pCallbackStatus = CallbackStatus;

  return STATUS_SUCCESS;
}

// generate binary code
NTSTATUS NTAPI CmGenerateMovReg (
  PUCHAR pCode,
  PULONG pGeneratedCodeLength,
  ULONG Register,
  ULONG Value
)
{ //Finished
    ULONG uCodeLength;

    if (!pCode || !pGeneratedCodeLength)
        return STATUS_INVALID_PARAMETER;

    switch (Register & ~REG_MASK) 
    {
    case REG_GP:
        pCode[0] = 0xb8 | (UCHAR) (Register & REG_MASK);
        memcpy (&pCode[1], &Value, 4);
        uCodeLength = 5;
        break;

    case REG_GP_ADDITIONAL:
        pCode[0] = 0xb8 | (UCHAR) (Register & REG_MASK);
        memcpy (&pCode[1], &Value, 4);
        uCodeLength = 5;
        break;

    case REG_CONTROL:
        uCodeLength = *pGeneratedCodeLength;
        CmGenerateMovReg (pCode, pGeneratedCodeLength, REG_RAX, Value);
        // calc the size of the "mov rax, value"
        uCodeLength = *pGeneratedCodeLength - uCodeLength;
        pCode += uCodeLength;

        // mov crX, rax

        pCode[0] = 0x0f;
        pCode[1] = 0x22;
        pCode[2] = 0xc0 | (UCHAR) ((Register & REG_MASK) << 3);

        // *pGeneratedCodeLength has already been adjusted to the length of the "mov rax"
        uCodeLength = 3;
    }

    if (pGeneratedCodeLength)
        *pGeneratedCodeLength += uCodeLength;

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI CmGeneratePushReg (
    PUCHAR pCode,
    PULONG pGeneratedCodeLength,
    ULONG Register
)
{
    if (!pCode || !pGeneratedCodeLength)
        return STATUS_INVALID_PARAMETER;

    if ((Register & ~REG_MASK) != REG_GP)
        return STATUS_NOT_SUPPORTED;

    pCode[0] = 0x50 | (UCHAR) (Register & REG_MASK);
    *pGeneratedCodeLength += 1;

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI CmGenerateIretd (
    PUCHAR pCode,
    PULONG pGeneratedCodeLength
)
{
    if (!pCode || !pGeneratedCodeLength)
        return STATUS_INVALID_PARAMETER;

    pCode[0] = 0xcf;
    *pGeneratedCodeLength += 1;

    return STATUS_SUCCESS;
};