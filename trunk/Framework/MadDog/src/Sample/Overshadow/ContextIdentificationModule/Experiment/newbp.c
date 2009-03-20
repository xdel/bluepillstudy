#include "newbp.h"

BOOLEAN StartRecording;
ULONG64 SyscallTimes;

ULONG32 OriginSysenterEIP[2];
ULONG32 OriginSysenterESP[2];
static KMUTEX g_Mutex;
ULONG32 SavedEax;
ULONG32 SavedEbx;
ULONG32 SavedEcx;
ULONG32 SavedEdx;

void NTAPI IncreaseCounter()
{
	//KeWaitForSingleObject (&g_Mutex, Executive, KernelMode, FALSE, NULL);
	SyscallTimes++;
	//KeReleaseMutex (&g_Mutex, FALSE);
}
void __declspec(naked) CcFakeSysenterTrap()
{
	__asm{
		mov SavedEax,eax
		mov SavedEbx,ebx
		mov SavedEcx,ecx
		mov SavedEdx,edx
	}
	//SyscallTimes++;
	IncreaseCounter();
	__asm{
		mov eax,SavedEax
		mov ebx,SavedEbx
		mov ecx,SavedEcx
		mov edx,SavedEdx
		mov esp,OriginSysenterEIP
		jmp OriginSysenterEIP
	}
}


void NTAPI CcSetupSysenterTrap(int cProcessorNumber)
{
	PVOID newSysenterESP;
	OriginSysenterEIP[cProcessorNumber] = MsrRead(MSR_IA32_SYSENTER_EIP);
	OriginSysenterESP[cProcessorNumber] = MsrRead(MSR_IA32_SYSENTER_ESP);
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, OriginSysenterEIP:%x\n",cProcessorNumber,OriginSysenterEIP));
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, OriginSysenterESP:%x\n",cProcessorNumber,OriginSysenterESP));
	newSysenterESP = ExAllocatePoolWithTag (NonPagedPool, 256 * PAGE_SIZE, L"ITL");
	MsrWrite(MSR_IA32_SYSENTER_EIP,&CcFakeSysenterTrap);
	MsrWrite(MSR_IA32_SYSENTER_ESP,newSysenterESP);
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, NewSysenterEntry:%x\n",cProcessorNumber,MsrRead(MSR_IA32_SYSENTER_EIP)));
}
void NTAPI CcDestroySysenterTrap(int cProcessorNumber)
{
	MsrWrite(MSR_IA32_SYSENTER_EIP,OriginSysenterEIP[cProcessorNumber]);
	MsrWrite(MSR_IA32_SYSENTER_ESP,OriginSysenterESP[cProcessorNumber]);
}

NTSTATUS DriverUnload (
    PDRIVER_OBJECT DriverObject
)
{
	CCHAR cProcessorNumber;
	cProcessorNumber = 0;
	for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) 
	{
		KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));

		CcDestroySysenterTrap(cProcessorNumber);

	}

	DbgPrint("Sysenter %d times\n",SyscallTimes);
    	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry (
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
   	 NTSTATUS Status;
   	 CCHAR cProcessorNumber;

    	__asm { int 3 }
	cProcessorNumber = 0;
	for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) 
	{
		KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));

		CcSetupSysenterTrap(cProcessorNumber);

	}

	KeRevertToUserAffinityThread ();

      	DriverObject->DriverUnload = DriverUnload;
    	return STATUS_SUCCESS;
}
