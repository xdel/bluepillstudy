#pragma once
#include <ntddk.h>

#ifdef _X86_
#define ULONG ULONG32
#else

#if defined(_AMD64_)|| defined(_IA64_)
#define ULONG ULONG64
#endif

#endif

//+++++++++++++++++++++Global Definitions+++++++++++++++++++++++++++
#define LAB_TAG 'VSL' 	//Define the tag. This value will be used in allocate pool memory. 
			// 'VSL'- Virtualization Security Lab
//CURRENT_STATE_HYPERVISOR & CURRENT_STATE_GUEST are used for bCurrentMachineState
#define CURRENT_STATE_HYPERVISOR 0
#define CURRENT_STATE_GUEST 1

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

//+++++++++++++++++++++Other Definitions+++++++++++++++++++++++++++
typedef NTSTATUS (NTAPI * PCALLBACK_PROC) (PVOID Param);


