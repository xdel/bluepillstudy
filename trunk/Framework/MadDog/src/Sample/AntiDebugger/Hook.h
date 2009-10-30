#pragma once
#include <ntddk.h>
#include <windef.h>

VOID HookKiDispatchException ();
VOID UnHookKiDispatchException();