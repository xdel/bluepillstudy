/* 
 * Copyright holder: Invisible Things Lab
 * 
 * This software is protected by domestic and International
 * copyright laws. Any use (including publishing and
 * distribution) of this software requires a valid license
 * from the copyright holder.
 *
 * This software is provided for the educational use only
 * during the Black Hat training. This software should not
 * be used on production systems.
 *
 */

#include "newbp.h"

NTSTATUS DriverUnload (
    PDRIVER_OBJECT DriverObject
)
{
    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry (
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
	MsrRead(MSR_IA32_VMX_PROCBASED_CTLS);
      DriverObject->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}
