#include "newbp.h"

BOOLEAN StartRecording;
ULONG64 SyscallTimes;
ULONG32 pSyscallTimes;

ULONG32 OriginSysenterEIP[2];
ULONG32 OriginSysenterESP[2];
static KMUTEX g_Mutex;
static ULONG lock;
static ULONG plock;
void NTAPI IncreaseCounter()
{
	//KeWaitForSingleObject (&g_Mutex, Executive, KernelMode, FALSE, NULL);
	//SyscallTimes++;
	__asm{
		//inc dword ptr [pSyscallTimes]
	}
	//KeReleaseMutex (&g_Mutex, FALSE);
}
void __declspec(naked) CcFakeSysenterTrap()
{
	//ULONG32 currentProcessor;
	//ULONG32 targetESP;
	//ULONG32 targetEIP;
	
	__asm{
	loop_down:
		lock	bts dword ptr [plock], 0
		jb	loop_down; Acquire A Spin Lock
	}

	__asm{
		//mov SavedEax,eax
		//mov SavedEbx,ebx
		//mov SavedEcx,ecx
		//mov SavedEdx,edx
		push eax
		push ebx
		push ecx
		push edx
	}
	
	//currentProcessor = KeGetCurrentProcessorNumber();
	//targetESP=OriginSysenterESP[currentProcessor];
	//targetEIP=OriginSysenterEIP[currentProcessor];

	SyscallTimes++;
	//IncreaseCounter();
	__asm{
		pop edx
		pop ecx
		pop ebx
		pop eax
		lock	btr dword ptr [plock], 0; Release the Spin Lock
		//mov esp,targetESP
		//jmp targetEIP
		jmp OriginSysenterEIP[0]
	}
}


void NTAPI CcSetupSysenterTrap(int cProcessorNumber)
{
	PVOID newSysenterESP;
	OriginSysenterEIP[cProcessorNumber] = MsrRead(MSR_IA32_SYSENTER_EIP);
	OriginSysenterESP[cProcessorNumber] = MsrRead(MSR_IA32_SYSENTER_ESP);
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, OriginSysenterEIP:%x\n",cProcessorNumber,OriginSysenterEIP[cProcessorNumber]));
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, OriginSysenterESP:%x\n",cProcessorNumber,OriginSysenterESP[cProcessorNumber]));
	newSysenterESP = ExAllocatePoolWithTag (NonPagedPool, 4 * PAGE_SIZE, 'ITL');
	MsrWrite(MSR_IA32_SYSENTER_EIP,&CcFakeSysenterTrap);
	MsrWrite(MSR_IA32_SYSENTER_ESP,newSysenterESP);
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, NewSysenterEntry:%x\n",cProcessorNumber,MsrRead(MSR_IA32_SYSENTER_EIP)));
	
	pSyscallTimes =&SyscallTimes;
	plock=&lock;
	__asm{
		and	dword ptr [plock], 0
	}
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

    	//__asm { int 3 }
	for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) 
	{
		KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));

		CcSetupSysenterTrap(cProcessorNumber);

	}

	KeRevertToUserAffinityThread ();

      	DriverObject->DriverUnload = DriverUnload;
    	return STATUS_SUCCESS;
}
