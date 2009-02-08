#pragma once

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
/*
 * Intel CPU  MSR
 */
        /* MSRs & bits used for VMX enabling */

#define MSR_IA32_VMX_BASIC   		0x480
#define MSR_IA32_FEATURE_CONTROL 		0x03a
#define MSR_IA32_VMX_PINBASED_CTLS		0x481
#define MSR_IA32_VMX_PROCBASED_CTLS		0x482
#define MSR_IA32_VMX_EXIT_CTLS		0x483
#define MSR_IA32_VMX_ENTRY_CTLS		0x484

#define MSR_IA32_SYSENTER_CS		0x174
#define MSR_IA32_SYSENTER_ESP		0x175
#define MSR_IA32_SYSENTER_EIP		0x176
#define MSR_IA32_DEBUGCTL			0x1d9

//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

typedef enum SEGREGS
{
  ES = 0,
  CS,
  SS,
  DS,
  FS,
  GS,
  LDTR,
  TR
};

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

ULONG32 NTAPI VmxRead (//Here should 
  ULONG64 field
);

VOID NTAPI VmxWrite (
  ULONG64 field,
  ULONG64 value
);