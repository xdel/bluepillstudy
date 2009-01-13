#pragma once

#include <ntddk.h>
#include "common.h"
#include "hvm.h"
//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++

typedef struct _NBP_TRAP *PNBP_TRAP;

// returns FALSE if the adjustment of guest RIP is not needed
typedef BOOLEAN (
  NTAPI * NBP_TRAP_CALLBACK
) (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  BOOLEAN WillBeAlsoHandledByGuestHv
);


//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

typedef enum
{
  TRAP_DISABLED = 0,
  TRAP_GENERAL = 1,
  TRAP_MSR = 2,
  TRAP_IO = 3
} TRAP_TYPE;

// The following three will be used as trap's data structure.
// 下面的这三个是_NBP_TRAP_中的存放关键数据的数据结构
typedef struct _NBP_TRAP_DATA_GENERAL
{
  ULONG TrappedVmExit;
  ULONG RipDelta;             // this value will be added to rip to skip the trapped instruction
} NBP_TRAP_DATA_GENERAL,
 *PNBP_TRAP_DATA_GENERAL;

typedef struct _NBP_TRAP_DATA_MSR
{
  ULONG32 TrappedMsr;
  UCHAR TrappedMsrAccess;
  UCHAR GuestTrappedMsrAccess;
} NBP_TRAP_DATA_MSR,
 *PNBP_TRAP_DATA_MSR;

typedef struct _NBP_TRAP_DATA_IO
{
  ULONG TrappedPort;
} NBP_TRAP_DATA_IO,
 *PNBP_TRAP_DATA_IO;


typedef struct _NBP_TRAP
{
  LIST_ENTRY le;

  TRAP_TYPE TrapType;
  TRAP_TYPE SavedTrapType;

  union
  {
    NBP_TRAP_DATA_GENERAL General;
    NBP_TRAP_DATA_MSR Msr;
    NBP_TRAP_DATA_IO Io;
  };

  NBP_TRAP_CALLBACK TrapCallback;
  BOOLEAN bForwardTrapToGuest;  // FALSE if guest hypervisor doesn't want to intercept this in its own guest.
  // This will be TRUE for TRAP_MSR record when we're going to intercept MSR "rw"
  // but the guest wants to intercept only "r" or "w". 
  // Check Msr.GuestTrappedMsrAccess for correct event forwarding.
} NBP_TRAP,
 *PNBP_TRAP;

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects:Build and Initialize General Trap struct (which is also a Trap struct).
 */
NTSTATUS NTAPI TrInitializeGeneralTrap (
  PCPU Cpu,
  ULONG TrappedVmExit,
  UCHAR RipDelta,
  NBP_TRAP_CALLBACK TrapCallback,
  PNBP_TRAP * pInitializedTrap
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
 * 查询已注册的Traps
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

//+++++++++++++++++++++Static Functions++++++++++++++++++++++++