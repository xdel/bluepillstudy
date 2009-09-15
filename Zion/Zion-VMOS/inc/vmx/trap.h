#include <inc/lib/stdio.h>
#include <inc/vmx/vmx.h>
#include <inc/vmx/common.h>
//#include <vmx/hvm.h>



ZVMSTATUS ZVMAPI TrInitializeGeneralTrap (
  PCPU Cpu,
  uint32_t TrappedVmExit,
  uint8_t RipDelta,
  NBP_TRAP_CALLBACK TrapCallback,
  PNBP_TRAP * pInitializedTrap
);

ZVMSTATUS ZVMAPI TrRegisterTrap (
  PCPU Cpu,
  PNBP_TRAP Trap
);

ZVMSTATUS ZVMAPI TrFindRegisteredTrap (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  uint64_t exitcode,
  PNBP_TRAP * pTrap
);

ZVMSTATUS ZVMAPI TrExecuteGeneralTrapHandler (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  bool WillBeAlsoHandledByGuestHv
);

//ZVMSTATUS ZVMAPI TrExecuteMsrTrapHandler (
  //PCPU Cpu,
  //PGUEST_REGS GuestRegs,
  //PNBP_TRAP Trap,
  //bool WillBeAlsoHandledByGuestHv
//);
