#pragma once

//for yupiwang's work
#define     GET_PDE_VADDRESS(va) ((((ULONG)(va) >> 22) << 2) + PDE_BASE)
#define     GET_PTE_VADDRESS(va) ((((ULONG)(va) >> 12) << 2) + PTE_BASE)

#define     GET_PDE_VOFFSET(va, base)   ((((ULONG)(va) >> 22) << 2) + base)
#define     GET_PTE_VOFFSET(va, base)   (((((ULONG)(va) >> 12) & 0x3ff) << 2) + base)

//#define     GET_PDE_VALUE(va, base)    *(PULONG)GET_PDE_VADDRESS(va, base)
//#define     GET_PTE_VALUE(va, base)    *(PULONG)GET_PTE_VADDRESS(va, base)

#define     GET_4KPAGE_PA(pte, va)      ((pte & 0xfffff000) + (va & 0xfff))
#define     GET_4MPAGE_PA(pde, va)      ((pde & 0xffc00000) + (va & 0x3fffff))

#define     IS_BIT_SET(value, bitno)    (BOOLEAN)(((ULONG64)value & (ULONG64)(1 << bitno)) != 0)

#define     IS_PAGE_PRESENT(x)  IS_BIT_SET(x, 0)
#define     IS_LARGE_PAGE(x)    IS_BIT_SET(x, 7)
//

#define     PTE_BASE        0xC0000000
#define     PDE_BASE        0xc0300000
#define     PTE_TOP_X86     0xC03FFFFF
#define     PDE_TOP_X86     0xC0300FFF

#define P_PRESENT			0x01
#define P_WRITABLE			0x02
#define P_USERMODE			0x04
#define P_WRITETHROUGH		0x08
#define P_CACHE_DISABLED	0x10
#define P_ACCESSED			0x20
#define P_DIRTY				0x40
#define P_LARGE				0x80
#define P_GLOBAL			0x100

//#define AP_PDP		8
//#define AP_PML4		16



