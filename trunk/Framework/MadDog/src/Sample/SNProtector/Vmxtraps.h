#pragma once

#include <ntddk.h>
#include "HvCore.h"

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
// BPKNOCK backdoor
#define BP_KNOCK
#ifdef BP_KNOCK
	#define BP_KNOCK_EAX	100
	#define BP_KNOCK_EAX_ANSWER 0x6c6c6548	//Hell
	#define BP_KNOCK_EBX_ANSWER 0x6f57206f  //o Wo
	#define BP_KNOCK_EDX_ANSWER 0x21646c72	//rld!

	//#define BP_EXIT_EAX		200
#endif // BP_KNOCK

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Register traps in this function
 * requires: <Cpu> is valid
 */
NTSTATUS NTAPI VmxRegisterTraps (
  PCPU Cpu
);

