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
 * 09/10/11	Miao Yu <superymkfounder@hotmail.com> , Add Basic Interceptions
 */
#pragma once

#include <ntddk.h>
#include "common.h"
#include "hvm.h"

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects:Build and Initialize General Trap struct (which is also a Trap struct).
 */
NTSTATUS NTAPI TrInitializeGeneralTrap (
  PCPU Cpu,
  ULONG TrappedVmExit,
  BOOLEAN ForwardTrap,
  UCHAR RipDelta,
  NBP_TRAP_CALLBACK TrapCallback,
  PNBP_TRAP * pInitializedTrap,
  ULONG uDebugTag //Pool Debug Tag
);

/**
 * effects:Build and Initialize MSR Trap struct.
 */
NTSTATUS NTAPI TrInitializeMSROpTrap (
    PCPU Cpu,
	ULONG MSROpType,
    UCHAR RipDelta,
    NBP_TRAP_CALLBACK TrapCallback,
    PNBP_TRAP *pInitializedTrap,
	ULONG uDebugTag //Pool Debug Tag
);

/**
 * effects: Register trap struct.
 */
NTSTATUS NTAPI TrRegisterTrap (
  PCPU Cpu,
  PNBP_TRAP Trap
);
/**
 * Search Registered Traps
 */
NTSTATUS NTAPI TrFindRegisteredTrap (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  ULONG exitcode,
  PNBP_TRAP * pTrap
);

NTSTATUS NTAPI TrExecuteGeneralTrapHandler (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);
