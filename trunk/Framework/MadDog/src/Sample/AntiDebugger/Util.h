#include <ntddk.h>
#include <windef.h>

PBYTE Memsearch( const PBYTE target, const PBYTE search, ULONG32 tlen, ULONG32 slen);
VOID Memcpy( const PBYTE dest, const PBYTE src, ULONG32 slen);
VOID InitSpinLock();
VOID AcquireSpinLock();
VOID ReleaseSpinLock();

VOID WPON();
VOID WPOFF();