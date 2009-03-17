#include "newbp.h"

BOOLEAN StartRecording;
ULONG32 SyscallTimes;

ULONG64 OriginSysenterEIP;
ULONG64 OriginSysenterCS;

void NTAPI CcFakeSysenterTrap()
{
	ULONG32 eaxMem;
	ULONG32 ebxMem;
	ULONG32 ecxMem;
	ULONG32 edxMem;
	__asm{
		mov eaxMem,eax;
		mov ebxMem,ebx;
		mov ecxMem,ecx;
		mov edxMem,edx;
	}
	SyscallTimes++;
	__asm{
		mov eax,eaxMem;
		mov ebx,ebxMem;
		mov ecx,ecxMem;
		mov edx,edxMem;
		call OriginSysenterEIP;
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

    __asm { int 3 }

CcSetupSysenterTrap();
      DriverObject->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}
