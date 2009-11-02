#pragma once

#include <ntddk.h>
#include "HvCore.h"

//+++++++++++++++++++++Definitions+++++++++++++++++++++++++++
#define FN_ENTERKDE			0x1000
#define FN_EXITKDE			0x2000

#define INTR_ID_MASK		0xff
#define INTR_TYPE_MASK		( 0x7 << 8 )

#define     GET_PDE_VADDRESS(va) ((((ULONG)(va) >> 22) << 2) + PDE_BASE)
#define     GET_PTE_VADDRESS(va) ((((ULONG)(va) >> 12) << 2) + PTE_BASE)
#define     PTE_BASE        0xC0000000
#define     PDE_BASE        0xc0300000

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

typedef struct{
        unsigned Vector:8;
        unsigned InteruptionType:3;
        unsigned DeliverErrorCode:1;
        unsigned NMIUnblocking:1;
        unsigned Reserved:18;
        unsigned Valid:1;
}INTERUPTION_INFORMATION_FIELD, *PINTERUPTION_INFORMATION_FIELD;

typedef struct{
        unsigned Limit:16;
        unsigned BaseLo:16;
        unsigned BaseHi:16;
}IDT_BASE, *PIDT_BASE, GDT_BASE, *PGDT_BASE;

typedef struct{
        unsigned Selector:16;
        unsigned Reserved:14;
        unsigned SourceOfTaskSwitch:2;
}TASK_SWITCH_EQUALIFICATION, *PTASK_SWITCH_EQUALIFICATION;

typedef struct{
        unsigned SegmentType:4;
        unsigned DescriptorType:1;
        unsigned Dpl:2;
        unsigned Present:1;
        unsigned Reserved1:4;
        unsigned Available:1;
        unsigned Reserved2:1;           //used only for CS segment
        unsigned DefaultOperationSize:1;
        unsigned Granularity:1;
        unsigned SegmentUnusable:1;
        unsigned Reserved3:15;
}SEGMENT_ACCESS_RIGHTS, *PSEGMENT_ACCESS_RIGHTS;

typedef struct{
        ULONG   regEdi;
        ULONG   regEsi;
        ULONG   regEbp;
        ULONG   regEsp;
        ULONG   regEbx;
        ULONG   regEdx;
        ULONG   regEcx;
        ULONG   regEax;
}PUSHAD_REGS, *PPUSHAD_REGS;

typedef struct{
        unsigned SegmentLimitLo:16;
        unsigned BaseLow:16;
        unsigned BaseMid:8;
        unsigned Type:4;
        unsigned DescriptorType:1;
        unsigned Dpl:2;
        unsigned Present:1;
        unsigned SegmentLimitHi:4;
        unsigned Available:1;
        unsigned L:1;                   //same as reserved2 in SEGMENT_ACCESS_RIGHTS
        unsigned DefaultOperationSize:1;
        unsigned Granularity:1;
        unsigned BaseHi:8;
}GDT_ENTRY, *PGDT_ENTRY;

typedef struct _KiIoAccessMap{                                                                       
/*0x000*/     UINT8        DirectionMap[32];                                      
/*0x020*/     UINT8        IoMap[8196];                                           
}KiIoAccessMap, *PKiIoAccessMap;

typedef struct _KTSS{                                                                           
/*0x000*/      UINT16       Backlink;                                                  
/*0x002*/      UINT16       Reserved0;                                                 
/*0x004*/      ULONG32      Esp0;                                                      
/*0x008*/      UINT16       Ss0;                                                       
/*0x00A*/      UINT16       Reserved1;                                                 
/*0x00C*/      ULONG32      NotUsed1[4];                                               
/*0x01C*/      ULONG32      CR3;                                                       
/*0x020*/      ULONG32      Eip;                                                       
/*0x024*/      ULONG32      EFlags;                                                    
/*0x028*/      ULONG32      Eax;                                                       
/*0x02C*/      ULONG32      Ecx;                                                       
/*0x030*/      ULONG32      Edx;                                                       
/*0x034*/      ULONG32      Ebx;                                                       
/*0x038*/      ULONG32      Esp;                                                       
/*0x03C*/      ULONG32      Ebp;                                                       
/*0x040*/      ULONG32      Esi;                                                       
/*0x044*/      ULONG32      Edi;                                                       
/*0x048*/      UINT16       Es;                                                        
/*0x04A*/      UINT16       Reserved2;                                                 
/*0x04C*/      UINT16       Cs;                                                        
/*0x04E*/      UINT16       Reserved3;                                                 
/*0x050*/      UINT16       Ss;                                                        
/*0x052*/      UINT16       Reserved4;                                                 
/*0x054*/      UINT16       Ds;                                                        
/*0x056*/      UINT16       Reserved5;                                                 
/*0x058*/      UINT16       Fs;                                                        
/*0x05A*/      UINT16       Reserved6;                                                 
/*0x05C*/      UINT16       Gs;                                                        
/*0x05E*/      UINT16       Reserved7;                                                 
/*0x060*/      UINT16       LDT;                                                       
/*0x062*/      UINT16       Reserved8;                                                 
/*0x064*/      UINT16       Flags;                                                     
/*0x066*/      UINT16       IoMapBase;                                                 
/*0x068*/      struct _KiIoAccessMap IoMaps[1];                                        
/*0x208C*/     UINT8        IntDirectionMap[32];                                       
           }KTSS, *PKTSS;

/**
 * effects: Register traps in this function
 * requires: <Cpu> is valid
 */
NTSTATUS NTAPI VmxRegisterTraps (
  PCPU Cpu
);

