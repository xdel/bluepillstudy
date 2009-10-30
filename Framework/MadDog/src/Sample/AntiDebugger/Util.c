#include "Util.h"

static ULONG lock;
static PULONG plock;
ULONG g_uCr0 = 0;


PBYTE Memsearch( const PBYTE target, const PBYTE search, ULONG32 tlen, ULONG32 slen)
{
        PBYTE p= NULL;
        ULONG32 i= 0;

        for( p = target; p< ( target+ tlen); p++)
        {
                if( *p == search[0])
                {
                        for( i = 0; (i< slen) && (*p == search[i]); i++)
                        {
                                p++;
                                if( i == (slen-1) )
                                        return p - slen;
                        }
                }
        }
        return NULL;

}

VOID Memcpy( const PBYTE dest, const PBYTE src, ULONG32 slen)
{
    ULONG32 p;
    for( p = 0; p< slen; p++)
    {
		dest[p]=src[p];
    }
}

VOID WPON()
{

	_asm
	{
		sti
		push eax;
		mov eax, g_uCr0; //恢復原有 CR0 屬性
		mov cr0, eax;
		pop eax;
	};

}

VOID WPOFF()
{

ULONG uAttr;

_asm
{
push eax;
mov eax, cr0;
mov uAttr, eax;
and eax, 0FFFEFFFFh; // CR0 16 BIT = 0
mov cr0, eax;
pop eax;
cli
};

g_uCr0 = uAttr; //保存原有的 CRO 屬性

}

VOID InitSpinLock()
{
	plock = &lock;
	__asm{
		and	dword ptr [plock], 0
	}
}
VOID AcquireSpinLock()
{
	__asm{
	loop_down:
		lock	bts dword ptr [plock], 0
		jb	loop_down; Acquire A Spin Lock
	}
}
VOID ReleaseSpinLock()
{
	__asm{
		lock	btr dword ptr [plock], 0; Release the Spin Lock
	}
}