#pragma once

#include <ntddk.h>
#include "common.h"
#include "traps.h"
//#include "hypercalls.h"

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects: Register traps in this function
 * requires: <Cpu> is valid
 */
NTSTATUS NTAPI VmxRegisterTraps (
  PCPU Cpu
);

//+++++++++++++++++++++Static Functions++++++++++++++++++++++++
