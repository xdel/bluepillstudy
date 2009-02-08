#pragma once
#include <ntddk.h>
#include "VTCore.h"
#include "VTUtilAPIs.h"
#include "Vmx/Vmcs.h"
#include "Vmx/Vmx.h"

NTSTATUS HvmSetupVMControlBlock (
    PCPU Cpu,
    PVOID GuestEip,
    PVOID GuestEsp
);

//+++++++++++++++++++++Inner Functions++++++++++++++++++++++++

// make the ctl code legal
static ULONG32 NTAPI VmxAdjustControls (
    ULONG32 Ctl,
    ULONG32 Msr
);

/**
 * effects: 用于填充VMCB中Guest状态描述中的段选择器部分
 */
static NTSTATUS NTAPI VmxFillGuestSelectorData (
    PVOID GdtBase,
    ULONG Segreg,//SEGREGS枚举中的段选择符号，用于描述要Fill哪个段选择器
    USHORT Selector
);