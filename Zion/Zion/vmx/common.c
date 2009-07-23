#include "common.h"

/**
 * effects:To see if the indicated bit is set or not.
 * requires: 0<=bitNo<=63
 */
bool ZVMAPI CmIsBitSet (
  uint32_t v,
  uint8_t bitNo
)
{//Finished
  uint32_t mask = (uint32_t) 1 << bitNo;
  return (bool) ((v & mask) != 0);
}

/**
* effects:Raise the interruption level to dispatch level, then
* install VM Root hypervisor by call <CallbackProc>
*/
ZVMSTATUS ZVMAPI CmDeliverToProcessor (
										int8_t cProcessorNumber,
										PCALLBACK_PROC CallbackProc,
										void* CallbackParam,
										PZVMSTATUS pCallbackStatus
										)
{ //Finish
	ZVMSTATUS CallbackStatus;
	//KIRQL OldIrql;
	//EFI_TPL OldTpl;

	if (!CallbackProc)
		return ZVM_INVALID_PARAMETER;

	if (pCallbackStatus)
		*pCallbackStatus = ZVM_UNSUCCESSFUL;

	// AffinityThread not finished by zhumin
	//KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));

	//OldIrql = KeRaiseIrqlToDpcLevel ();
	//OldTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);
	CallbackStatus = CallbackProc (CallbackParam);

	//KeLowerIrql (OldIrql);
	//gBS->RestoreTPL(OldTpl);

	// AffinityThread not finished by zhumin
	//KeRevertToUserAffinityThread ();

	// save the status of the callback which has run on the current core
	if (pCallbackStatus)
		*pCallbackStatus = CallbackStatus;

	return ZVMSUCCESS;
}

