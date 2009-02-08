#pragma once

//++++++++++++++Cpu Related Structs(Common Structs)++++++++++++++++
typedef struct _CPU *PCPU;
typedef struct _GUEST_REGS *PGUEST_REGS;

//+++++++++++++++++++++Traps Definitions+++++++++++++++++++++++++++

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