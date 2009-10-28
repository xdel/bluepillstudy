#include "Hooker.h"

PVOID GetKernelBase()
{
	PSYSTEM_MODULE_INFORMATION_EX	info;
	NTSTATUS						status;
	ULONG							req_len;
	PVOID							base;

	info = NULL; base = NULL;
	do
	{
		status = ZwQuerySystemInformation(
			SystemModuleInformation, NULL, 0, &req_len
			);

		if (status != STATUS_INFO_LENGTH_MISMATCH) {
			break;
		}

		if ( (info = ExAllocatePoolWithTag (NonPagedPool, BYTES_TO_PAGES(req_len),'ITL')) == NULL ) {
			break;
		}
		/*if ( (info = mem_alloc(req_len)) == NULL ) {
			break;
		}*/

		status = ZwQuerySystemInformation(
			SystemModuleInformation, info, req_len, NULL
			);

		if (NT_SUCCESS(status) == FALSE) {
			break;
		}

		base = info->Modules[0].Base;
	} while (0);

	if (info != NULL) {
		ExFreePool(info);
	}

	return base;
}