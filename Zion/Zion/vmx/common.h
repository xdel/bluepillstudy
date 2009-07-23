#ifndef ZION_COMMON_H
#define ZION_COMMON_H

#include <include/types.h>
#include <vmx/vmx.h>

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
/*
* Generic CPUID function
* clear %ecx since some cpus (Cyrix MII) do not set or clear %ecx
* resulting in stale register contents being returned.
*/
#define cpuid(_op,_eax,_ebx,_ecx,_edx)          \
	asm volatile( "cpuid"                               \
	: "=a" (*(int32_t *)(_eax)),              \
	"=b" (*(int32_t *)(_ebx)),              \
	"=c" (*(int32_t *)(_ecx)),              \
	"=d" (*(int32_t *)(_edx))               \
	: "0" (_op), "2" (0) )


//+++++++++++++++++++++Structs++++++++++++++++++++++++++++++++

// typedef struct
// {
//   uint8_t Architecture;
//   ARCH_IS_HVM_IMPLEMENTED ArchIsHvmImplemented;
//   ARCH_INITIALIZE ArchInitialize;
//   ARCH_VIRTUALIZE ArchVirtualize;
//   ARCH_SHUTDOWN ArchShutdown;
//   ARCH_DISPATCH_EVENT ArchDispatchEvent;
//   ARCH_ADJUST_RIP ArchAdjustRip;
//   ARCH_IS_TRAP_VALID ArchIsTrapValid;
// } HVM_DEPENDENT,
//  *PHVM_DEPENDENT;


//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

bool ZVMAPI CmIsBitSet (
  uint32_t v,
  uint8_t bitNo
);

/**
* effects:Raise the interruption level to dispatch level, then
* install VM Root hypervisor by call <CallbackProc>
*/
ZVMSTATUS ZVMAPI CmDeliverToProcessor (
									   int8_t cProcessorNumber,
									   PCALLBACK_PROC CallbackProc,
									   void* CallbackParam,
									   PZVMSTATUS pCallbackStatus
									   );

#endif /* !ZION_COMMON_H */