#include "newbp.h"

BOOLEAN StartRecording;
ULONG64 SyscallTimes;

ULONG64 OriginSysenterEIP;
ULONG64 OriginSysenterCS;

ULONG32 SavedEax;
ULONG32 SavedEbx;
ULONG32 SavedEcx;
ULONG32 SavedEdx;
void __declspec(naked) CcFakeSysenterTrap()
{
	//ULONG32 eaxMem;
	//ULONG32 ebxMem;
	//ULONG32 ecxMem;
	//ULONG32 edxMem;
	__asm{
		mov SavedEax,eax;
		mov SavedEbx,ebx;
		mov SavedEcx,ecx;
		mov SavedEdx,edx;
	}
	SyscallTimes++;
	__asm{
		mov eax,SavedEax;
		mov ebx,SavedEbx;
		mov ecx,SavedEcx;
		mov edx,SavedEdx;
		jmp OriginSysenterEIP;
	}
}


void NTAPI CcSetupSysenterTrap()
{
	OriginSysenterEIP = MsrRead(MSR_IA32_SYSENTER_EIP);
	HvmPrint(("In CcSetupSysenterTrap(): OriginSysenterAddr:%x\n",OriginSysenterEIP));
	HvmPrint(("In CcSetupSysenterTrap(): FakeSysenterTrap:%x\n",&CcFakeSysenterTrap));
	MsrWrite(MSR_IA32_SYSENTER_EIP,&CcFakeSysenterTrap);
	HvmPrint(("In CcSetupSysenterTrap(): NewSysenterEntry:%x\n",MsrRead(MSR_IA32_SYSENTER_EIP)));
}
 void NTAPI CcDestroySysenterTrap()
{
	MsrWrite(MSR_IA32_SYSENTER_EIP,OriginSysenterEIP);
}

NTSTATUS DriverUnload (
    PDRIVER_OBJECT DriverObject
)
{
CcDestroySysenterTrap();
	DbgPrint("Sysenter %d times\n",SyscallTimes);
    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry (
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
    NTSTATUS Status;
    //CmDebugBreak();
   // ULONG ulOldCR3;

    //__asm { int 3 }

CcSetupSysenterTrap();
      DriverObject->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}
