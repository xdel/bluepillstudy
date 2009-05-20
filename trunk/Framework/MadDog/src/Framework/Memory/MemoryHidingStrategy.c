#include "HvCore.h"
#include "MemoryHidingStrategyDef.h"

#ifdef USE_MEMORY_MEMORYHIDING_STRATEGY 

//++++++++++Inner Functions++++++++++++++
VOID NTAPI MmInvalidatePage (
  PVOID Page
);

static NTSTATUS NTAPI MmProtectKernelMemory();

static NTSTATUS NTAPI MmCreateMapping (
  PHYSICAL_ADDRESS PhysicalAddress,
  PVOID VirtualAddress,
  BOOLEAN bLargePage
);

static NTSTATUS NTAPI MmMapGuestPages (
  PVOID FirstPage,
  ULONG uNumberOfPages
);

static NTSTATUS NTAPI MmMapGuestKernelPages (
);

/*NTSTATUS NTAPI MmMapGuestTSS64 (
  PVOID Tss64,
  USHORT Tss64Limit
);
*/

static NTSTATUS NTAPI MmInitIdentityPageTable (
);

void NTAPI MmCoverHostVA(
 PVOID Source,
 PHYSICAL_ADDRESS dest
);

//++++++++++Global Variables++++++++++++++
static LIST_ENTRY g_PageTableList;
static KSPIN_LOCK g_PageTableListLock;

PHYSICAL_ADDRESS g_PageMapBasePhysicalAddress;
PHYSICAL_ADDRESS g_IdentityPageTableBasePhysicalAddress, g_IdentityPageTableBasePhysicalAddress_Legacy;
PHYSICAL_ADDRESS SparePagePA; // original PA of the SparePage

//static PUCHAR g_PageTableBases[2] = {
//  (PUCHAR) PTE_BASE,
//  (PUCHAR) PDE_BASE
//};


NTSTATUS CmPatchPTEPhysicalAddress (
    PULONG pPde,                            // pde's address
    PULONG pPte,                            // pte's address
    PVOID PageVA,                           // va to be patched
    PHYSICAL_ADDRESS NewPhysicalAddress     // new pa
)
{
    ULONG Pte,Pde;
    
    if (!pPde || !pPte || !PageVA)
        return STATUS_INVALID_PARAMETER;
    /*
    Pde = *pPde;
    if(Pde&0x80 != 0) //if this is a large page
	{
		Pde &= 0x1fff;
		Pde |= (NewPhysicalAddress.QuadPart & 0xfffffffffe000);
		*pPde = Pde;
		
    	}
    if(Pde&0x80 == 0)
    	{
    */
    Pte = *pPte;
    // set new pa
    Pte &= 0xfff;
    //Pte = 0;
    Pte |= (NewPhysicalAddress.QuadPart & 0xffffffffff000);
    *pPte = Pte;
    	
    // flush the tlb cache
    MmInvalidatePage (PageVA);

    //return STATUS_SUCCESS;
};

void NTAPI MmCoverHostVA(
	PVOID Source,
	PHYSICAL_ADDRESS dest
)
{
    PULONG Pde = 0, Pte = 0, PA = 0;
    PHYSICAL_ADDRESS Destaddr = dest;
    ULONG uSource = (ULONG)Source;

    //让SparePage 对应的PA变成页目录页
    Pde = (PULONG)GET_PDE_VADDRESS(uSource);
    Pte  = (PULONG)GET_PTE_VADDRESS(uSource);
    CmPatchPTEPhysicalAddress(Pde,Pte,uSource,Destaddr);
}

static NTSTATUS NTAPI MmProtectKernelMemory()
{
    PULONG pPde = (PULONG) PDE_BASE;
    PULONG pPte;
    ULONG uPdeIndex;

    // just walk kernel space, va >= 0x80000000
    for (uPdeIndex = 0x200; uPdeIndex < 0x400; uPdeIndex++)
    {
        if (!(pPde[uPdeIndex] & P_PRESENT)) 
        {
            continue;
        }

        // set it not writable
        pPde[uPdeIndex] &= ~((ULONG)P_WRITABLE);
    }

    return STATUS_SUCCESS;
}

static NTSTATUS NTAPI MmSavePage (
  PHYSICAL_ADDRESS PhysicalAddress,
  PVOID HostAddress,
  PVOID GuestAddress,
  PAGE_ALLOCATION_TYPE AllocationType,
  ULONG uNumberOfPages,
  ULONG Flags
)
{
  PALLOCATED_PAGE AllocatedPage;

  if (!GuestAddress)
    return STATUS_INVALID_PARAMETER;

  AllocatedPage = ExAllocatePoolWithTag (NonPagedPool, sizeof (ALLOCATED_PAGE), LAB_TAG);
  if (!AllocatedPage)
    return STATUS_INSUFFICIENT_RESOURCES;
  RtlZeroMemory (AllocatedPage, sizeof (ALLOCATED_PAGE));

  PhysicalAddress.QuadPart = PhysicalAddress.QuadPart & ALIGN_4KPAGE_MASK;
  HostAddress = (PVOID) ((ULONG) HostAddress & ALIGN_4KPAGE_MASK);

  AllocatedPage->AllocationType = AllocationType;
  AllocatedPage->PhysicalAddress = PhysicalAddress;
  AllocatedPage->HostAddress = HostAddress;
  AllocatedPage->GuestAddress = GuestAddress;
  AllocatedPage->uNumberOfPages = uNumberOfPages;
  AllocatedPage->Flags = Flags;

  ExInterlockedInsertTailList (&g_PageTableList, &AllocatedPage->le, &g_PageTableListLock);

  /*
     DbgPrint("MmSavePage(): PA 0x%X, HostVA 0x%p, GuestVA 0x%p, AT %d, FL 0x%X\n",
     PhysicalAddress.QuadPart,
     HostAddress,
     GuestAddress,
     AllocationType,
     Flags);
   */
  return STATUS_SUCCESS;
}

static NTSTATUS NTAPI MmFindPageByPA (
  PHYSICAL_ADDRESS PhysicalAddress,
  PALLOCATED_PAGE * pAllocatedPage
)
{
  PALLOCATED_PAGE AllocatedPage;
  KIRQL OldIrql;

  if (!pAllocatedPage)
    return STATUS_INVALID_PARAMETER;

  KeAcquireSpinLock (&g_PageTableListLock, &OldIrql);

  PhysicalAddress.QuadPart = PhysicalAddress.QuadPart & ALIGN_4KPAGE_MASK;

  AllocatedPage = (PALLOCATED_PAGE) g_PageTableList.Flink;
  while (AllocatedPage != (PALLOCATED_PAGE) & g_PageTableList) 
  {
    AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);

    if (AllocatedPage->PhysicalAddress.QuadPart == PhysicalAddress.QuadPart) 
    {
      *pAllocatedPage = AllocatedPage;
      KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
      return STATUS_SUCCESS;
    }

    AllocatedPage = (PALLOCATED_PAGE) AllocatedPage->le.Flink;
  }

  KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
  return STATUS_UNSUCCESSFUL;
}

static NTSTATUS NTAPI MmFindPageByHostVA (
  PVOID HostAddress,
  PALLOCATED_PAGE * pAllocatedPage
)
{
  PALLOCATED_PAGE AllocatedPage;
  KIRQL OldIrql;

  if (!pAllocatedPage)
    return STATUS_INVALID_PARAMETER;

  KeAcquireSpinLock (&g_PageTableListLock, &OldIrql);

  HostAddress = (PVOID) ((ULONG) HostAddress & ALIGN_4KPAGE_MASK);

  AllocatedPage = (PALLOCATED_PAGE) g_PageTableList.Flink;
  while (AllocatedPage != (PALLOCATED_PAGE) & g_PageTableList) 
  {
    AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);

    if (AllocatedPage->HostAddress == HostAddress) 
    {
      *pAllocatedPage = AllocatedPage;
      KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
      return STATUS_SUCCESS;
    }

    AllocatedPage = (PALLOCATED_PAGE) AllocatedPage->le.Flink;
  }

  KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
  return STATUS_UNSUCCESSFUL;
}

static NTSTATUS NTAPI MmUpdatePageTable (
  PVOID PageTable,
  UCHAR PageTableLevel,
  PVOID VirtualAddress,
  PHYSICAL_ADDRESS PhysicalAddress,
  BOOLEAN bLargePage
)
{
    ULONG PageTableOffset;
    PVOID LowerPageTableHostVA, LowerPageTableGuestVA;
    PALLOCATED_PAGE LowerPageTable;
    PHYSICAL_ADDRESS LowerPageTablePA;
    NTSTATUS Status;
    PHYSICAL_ADDRESS PagePA;

    // get the offset in the specified page table level
    switch (PageTableLevel)
    {
    case 1:
        PageTableOffset = ((ULONG)VirtualAddress >> 12) & 0x3ff;
        break;
    case 2:
        PageTableOffset = (ULONG)VirtualAddress >> 22;
        break;
    default:
        return STATUS_INVALID_PARAMETER;
    }

    if ((PageTableLevel == 1) || (bLargePage && (PageTableLevel == 2))) 
    {
        // patch PTE/PDE
#ifdef SET_PCD_BIT
        ((PULONG64) PageTable)[PageTableOffset] = PhysicalAddress.QuadPart |        /*P_GLOBAL | */
            P_WRITABLE | P_PRESENT | P_CACHE_DISABLED;
#else
        ((PULONG) PageTable)[PageTableOffset] = 
            (ULONG)(PhysicalAddress.QuadPart | /*P_GLOBAL | */ P_WRITABLE | P_PRESENT);
#endif
        if (bLargePage)
            ((PULONG) PageTable)[PageTableOffset] |= P_LARGE;

        // ok
        //DbgPrint(
        //    "MmUpdatePageTable(): mapped VA:0x%x to PA:0x%x\n", 
        //    VirtualAddress,
        //    PhysicalAddress.QuadPart);
        return STATUS_SUCCESS;
    }

    // here, must be pde, PageTableLevel == 2
    LowerPageTablePA.QuadPart = 
        ((PULONG)PageTable)[PageTableOffset] & ALIGN_4KPAGE_MASK;
    // get the host pagetable va
    LowerPageTableHostVA = 
        ((((ULONG)VirtualAddress & 0xffc00000) >> 12) << 2) + PTE_BASE;

    if (!LowerPageTablePA.QuadPart) 
    {
        // the next level page is not in the memory
        Status = MmFindPageByHostVA (LowerPageTableHostVA, &LowerPageTable);
        if (!NT_SUCCESS (Status) || VirtualAddress == LowerPageTableHostVA) 
        {
            // fail to find the page, then allocate it
            LowerPageTableGuestVA = ExAllocatePoolWithTag (NonPagedPool, PAGE_SIZE, LAB_TAG);
            if (!LowerPageTableGuestVA)
                return STATUS_INSUFFICIENT_RESOURCES;
            RtlZeroMemory (LowerPageTableGuestVA, PAGE_SIZE);

            LowerPageTablePA = MmGetPhysicalAddress (LowerPageTableGuestVA);

            Status = MmSavePage (
                LowerPageTablePA, 
                LowerPageTableHostVA,
                LowerPageTableGuestVA, 
                PAT_POOL, 
                1, 
                AP_PAGETABLE | AP_PDE);
            if (!NT_SUCCESS (Status)) 
            {
                DbgPrint(
                    "MmUpdatePageTable(): Failed to store page table level %d, MmSavePage() returned status 0x%08X\n",
                    PageTableLevel - 1, 
                    Status);
                return Status;
            }
        } 
        else 
        {
            // find the page
            LowerPageTablePA.QuadPart = LowerPageTable->PhysicalAddress.QuadPart;
            LowerPageTableGuestVA = LowerPageTable->GuestAddress;
        }

#ifdef SET_PCD_BIT
        ((PULONG64) PageTable)[PageTableOffset] = LowerPageTablePA.QuadPart |       /*P_GLOBAL | */
            P_WRITABLE | P_PRESENT | P_CACHE_DISABLED;
#else
        ((PULONG) PageTable)[PageTableOffset] = 
            (ULONG)(LowerPageTablePA.QuadPart | /*P_GLOBAL | */ P_WRITABLE | P_PRESENT);
#endif

        // create mapping
        Status = MmCreateMapping (LowerPageTablePA, LowerPageTableHostVA, FALSE);
        if (!NT_SUCCESS (Status)) 
        {
            DbgPrint(
                "MmUpdatePageTable(): MmCreateMapping() failed to map PA 0x%p with status 0x%08X\n",
                LowerPageTablePA.QuadPart, 
                Status);
            return Status;
        }

    } 
    else 
    {
        // LowerPageTablePA.QuadPart is not NULL
        Status = MmFindPageByPA (LowerPageTablePA, &LowerPageTable);
        if (!NT_SUCCESS (Status)) 
        {
            LowerPageTablePA.QuadPart = ((PULONG) PageTable)[PageTableOffset];
            if ((PageTableLevel == 2) && (LowerPageTablePA.QuadPart & P_LARGE)) 
            {
                // maybe a large page
                DbgPrint (
                    "MmUpdatePageTable(): Found large PDE, data 0x%p\n", 
                    LowerPageTablePA.QuadPart);
                return STATUS_SUCCESS;

            } 
            else 
            {
                DbgPrint(
                    "MmUpdatePageTable(): Failed to find lower page table (pl%d) guest VA, data 0x%p, status 0x%08X\n",
                    PageTableLevel - 1, 
                    LowerPageTablePA.QuadPart, 
                    Status);
                return Status;
            }
        }

        LowerPageTableGuestVA = LowerPageTable->GuestAddress;
    }

    return MmUpdatePageTable (
        LowerPageTableGuestVA, 
        PageTableLevel - 1, 
        VirtualAddress, 
        PhysicalAddress, 
        bLargePage);
}

/**
 * effects: Allocate <uNumberOfPages> pages from memory.
 */
PVOID NTAPI HvMmAllocatePages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA
)
{
  PVOID PageVA, FirstPage;
  PHYSICAL_ADDRESS PagePA;
  NTSTATUS Status;
  ULONG i;

  if (!uNumberOfPages)
    return NULL;

  FirstPage = PageVA = ExAllocatePoolWithTag (NonPagedPool, uNumberOfPages * PAGE_SIZE, LAB_TAG);
  if (!PageVA)
    return NULL;
  RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);

  if (pFirstPagePA)
    *pFirstPagePA = MmGetPhysicalAddress (PageVA);

  for (i = 0; i < uNumberOfPages; i++) 
  {
    // save pages
    PagePA = MmGetPhysicalAddress (PageVA);
    Status = MmSavePage (
        PagePA, 
        PageVA, 
        PageVA, 
        i == 0 ? PAT_POOL : PAT_DONT_FREE, 
        uNumberOfPages, 
        0);
    if (!NT_SUCCESS (Status)) 
    {
      DbgPrint ("MmAllocatePages(): MmSavePage() failed with status 0x%08X\n", Status);
      return NULL;
    }

    // map to the same addresses in the host pagetables as they are in guest's
    Status = MmCreateMapping (PagePA, PageVA, FALSE);
    if (!NT_SUCCESS (Status)) 
    {
      DbgPrint
        ("MmAllocatePages(): MmCreateMapping() failed to map PA 0x%p with status 0x%08X\n", PagePA.QuadPart, Status);
      return NULL;
    }
    MmCoverHostVA(PageVA,SparePagePA);
    PageVA = (PUCHAR) PageVA + PAGE_SIZE;
  }

  return FirstPage;
}

/**
 * effects: Allocate Contiguous Pages from memory.
 */
PVOID NTAPI HvMmAllocateContiguousPages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA
)
{
    return HvMmAllocateContiguousPagesSpecifyCache(
        uNumberOfPages,
        pFirstPagePA,
        MmCached);
}

/**
 * effects: Allocate Contiguous Pages from memory with the indicated cache strategy.
 */
PVOID NTAPI HvMmAllocateContiguousPagesSpecifyCache (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA,
  ULONG CacheType
)
{
  PVOID PageVA, FirstPage;
  PHYSICAL_ADDRESS PagePA, l1, l2, l3;
  NTSTATUS Status;
  ULONG i;

  if (!uNumberOfPages)
    return NULL;

  l1.QuadPart = 0;
  l2.QuadPart = -1;
  l3.QuadPart = 0x200000;    // 0x10000 ?

  FirstPage = PageVA = MmAllocateContiguousMemorySpecifyCache (
      uNumberOfPages * PAGE_SIZE, 
      l1, 
      l2, 
      l3, 
      CacheType);
  if (!PageVA)
    return NULL;

  RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);

  PagePA = MmGetPhysicalAddress (PageVA);
  if (pFirstPagePA)
    *pFirstPagePA = PagePA;

  for (i = 0; i < uNumberOfPages; i++) 
  {
    // save page
    Status = MmSavePage (
        PagePA, 
        PageVA, 
        PageVA, 
        i == 0 ? PAT_CONTIGUOUS : PAT_DONT_FREE, 
        uNumberOfPages, 
        0);
    if (!NT_SUCCESS (Status)) 
    {
      DbgPrint ("MmAllocateContiguousPages(): MmSavePage() failed with status 0x%08X\n", Status);
      return NULL;
    }

    // map to the same addresses in the host pagetables as they are in guest's
    Status = MmCreateMapping (PagePA, PageVA, FALSE);
    if (!NT_SUCCESS (Status)) 
    {
      DbgPrint
        ("MmAllocateContiguousPages(): MmCreateMapping() failed to map PA 0x%p with status 0x%08X\n",
         PagePA.QuadPart, Status);
      return NULL;
    }

    PageVA = (PUCHAR) PageVA + PAGE_SIZE;
    PagePA.QuadPart += PAGE_SIZE;
  }

  return FirstPage;
}

NTSTATUS NTAPI MmCreateMapping (
  PHYSICAL_ADDRESS PhysicalAddress,
  PVOID VirtualAddress,
  BOOLEAN bLargePage
)
{
  PALLOCATED_PAGE pPdePage;
  NTSTATUS Status;

  Status = MmFindPageByPA (g_PageMapBasePhysicalAddress, &pPdePage);
  if (!NT_SUCCESS (Status)) 
  {
    return STATUS_UNSUCCESSFUL;
  }

  PhysicalAddress.QuadPart = PhysicalAddress.QuadPart & ALIGN_4KPAGE_MASK;
  VirtualAddress = (PVOID) ((ULONG) VirtualAddress & ALIGN_4KPAGE_MASK);

  //DbgPrint(
  //    "MmCreateMapping(): ready to map VA:0x%x to PA:0x%x\n", 
  //    VirtualAddress,
  //    PhysicalAddress.QuadPart);

  return MmUpdatePageTable(
      pPdePage->GuestAddress, 
      2, 
      VirtualAddress, 
      PhysicalAddress, 
      bLargePage);
}

// add guset pages to host pagetable
NTSTATUS NTAPI MmMapGuestPages (
  PVOID FirstPage,
  ULONG uNumberOfPages
)
{
  PHYSICAL_ADDRESS PhysicalAddress;
  NTSTATUS Status;

  FirstPage = (PVOID) ((ULONG) FirstPage & ALIGN_4KPAGE_MASK);

  // Everything is made present, writable, executable, 4kb and cpl0 only.
  // Mapping is done to the same virtual addresses in the host.
  while (uNumberOfPages--) 
  {
    // Guest memory may not be contiguous   
    PhysicalAddress = MmGetPhysicalAddress (FirstPage);

    Status = MmCreateMapping (PhysicalAddress, FirstPage, FALSE);
    if (!NT_SUCCESS (Status)) 
    {
      DbgPrint ("MmMapGuestPages(): MmCreateMapping() failed with status 0x%08X\n", Status);
      return Status;
    }

    FirstPage = (PVOID) ((PUCHAR) FirstPage + PAGE_SIZE);
  }

  return STATUS_SUCCESS;
}

NTSTATUS NTAPI MmWalkGuestPageTable (
  PULONG PageTable,
  UCHAR bLevel
)
{
    ULONG i;
    PVOID VirtualAddress;
    PHYSICAL_ADDRESS PhysicalAddress;
    PULONG LowerPageTable;

    if (!MmIsAddressValid (PageTable))
        return STATUS_SUCCESS;

    if (bLevel != 1)
        return STATUS_UNSUCCESSFUL;

    // must be level 1
    // 10 bits
    for (i = 0; i < 0x400; i++)
    {
        if (PageTable[i] & P_PRESENT) 
        {
            // get VirtualAddress to map
            VirtualAddress = (PVOID) (((LONG)(&PageTable[i]) - PTE_BASE) << 10);

            //if ((LONG) VirtualAddress & 0x0000800000000000)
            //    VirtualAddress = (PVOID) ((LONGLONG) VirtualAddress | 0xffff000000000000);

            PhysicalAddress.QuadPart = PageTable[i] & ALIGN_4KPAGE_MASK;

            if ((ULONG) VirtualAddress >= PTE_BASE 
                && (ULONG) VirtualAddress <= PTE_TOP_X86)
            {
                // guest pagetable stuff here - so don't map it
                continue;
            }

            //DbgPrint(
            //    "MmWalkGuestPageTable(): %sValid pl%d at 0x%p, index 0x%x, VA 0x%p, PA 0x%p %s\n",
            //    bLevel == 2 ? "      " : bLevel ==
            //        1 ? "         " : "", 
            //    bLevel, 
            //    &PageTable[i], 
            //    i, 
            //    VirtualAddress, 
            //    PhysicalAddress.QuadPart, 
            //    ((bLevel == 2) && (PageTable[i] & P_LARGE)) ? "LARGE" : "");

            // blevel == 1, just map it in host pagetable
            MmCreateMapping (PhysicalAddress, VirtualAddress, FALSE);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI MmMapGuestKernelPages (
)
{
    PULONG pPde = (PULONG) PDE_BASE;
    PULONG pPte;
    ULONG uPdeIndex;
    PVOID VirtualAddress;
    PUCHAR ShortPageVA;
    PHYSICAL_ADDRESS PhysicalAddress;

    // just walk kernel space, va >= 0x80000000
    for (uPdeIndex = 0x200; uPdeIndex < 0x400; uPdeIndex++)
    {
        if (!(pPde[uPdeIndex] & P_PRESENT)) 
        {
            continue;
        }

        if (pPde[uPdeIndex] & P_LARGE)
        {
            // 4M page
            VirtualAddress = (PVOID)(uPdeIndex << 22);
            PhysicalAddress.QuadPart = pPde[uPdeIndex] & ALIGN_4KPAGE_MASK;

            if ((ULONG) VirtualAddress >= PTE_BASE 
                && (ULONG) VirtualAddress <= PTE_TOP_X86)
            {
                // guest pagetable stuff here - so don't map it
                continue;
            }

            // make 4M page into 4k pages in host
            for (ShortPageVA = (PUCHAR) VirtualAddress + 0x0 * PAGE_SIZE;
                ShortPageVA < (PUCHAR) VirtualAddress + 0x400 * PAGE_SIZE;
                ShortPageVA += PAGE_SIZE, PhysicalAddress.QuadPart += PAGE_SIZE)
            {
                MmCreateMapping (PhysicalAddress, ShortPageVA, FALSE);
            }
        }
        else
        {
            // 4k page
            pPte = (PULONG)((PUCHAR) PTE_BASE + (uPdeIndex << 10) * 4);
            MmWalkGuestPageTable (pPte, 1);
        }
    }

    return STATUS_SUCCESS;
}

// unchecked
/*
NTSTATUS MmMapGuestTSS64 (
  PTSS64 Tss64,
  USHORT Tss64Limit
)
{
  if (!Tss64)
    return STATUS_INVALID_PARAMETER;

  DbgPrint ("MmMapGuestTSS64(): Mapping TSS64 at 0x%p, limit %d\n", Tss64, Tss64Limit);
  MmMapGuestPages (Tss64, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64, Tss64Limit));

  if (Tss64->RSP0) {
    DbgPrint ("MmMapGuestTSS64(): Mapping RSP0 at 0x%p\n", Tss64->RSP0);
    MmMapGuestPages (Tss64->RSP0, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->RSP0, PAGE_SIZE));
  }
  if (Tss64->RSP1) {
    DbgPrint ("MmMapGuestTSS64(): Mapping RSP1 at 0x%p\n", Tss64->RSP1);
    MmMapGuestPages (Tss64->RSP1, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->RSP1, PAGE_SIZE));
  }
  if (Tss64->RSP2) {
    DbgPrint ("MmMapGuestTSS64(): Mapping RSP2 at 0x%p\n", Tss64->RSP2);
    MmMapGuestPages (Tss64->RSP2, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->RSP2, PAGE_SIZE));
  }

  if (Tss64->IST1) {
    DbgPrint ("MmMapGuestTSS64(): Mapping IST1 at 0x%p\n", Tss64->IST1);
    MmMapGuestPages (Tss64->IST1, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->IST1, PAGE_SIZE));
  }
  if (Tss64->IST2) {
    DbgPrint ("MmMapGuestTSS64(): Mapping IST2 at 0x%p\n", Tss64->IST2);
    MmMapGuestPages (Tss64->IST2, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->IST2, PAGE_SIZE));
  }
  if (Tss64->IST3) {
    DbgPrint ("MmMapGuestTSS64(): Mapping IST3 at 0x%p\n", Tss64->IST3);
    MmMapGuestPages (Tss64->IST3, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->IST3, PAGE_SIZE));
  }
  if (Tss64->IST4) {
    DbgPrint ("MmMapGuestTSS64(): Mapping IST4 at 0x%p\n", Tss64->IST4);
    MmMapGuestPages (Tss64->IST4, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->IST4, PAGE_SIZE));
  }
  if (Tss64->IST5) {
    DbgPrint ("MmMapGuestTSS64(): Mapping IST5 at 0x%p\n", Tss64->IST5);
    MmMapGuestPages (Tss64->IST5, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->IST5, PAGE_SIZE));
  }
  if (Tss64->IST6) {
    DbgPrint ("MmMapGuestTSS64(): Mapping IST6 at 0x%p\n", Tss64->IST6);
    MmMapGuestPages (Tss64->IST6, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->IST6, PAGE_SIZE));
  }
  if (Tss64->IST7) {
    DbgPrint ("MmMapGuestTSS64(): Mapping IST7 at 0x%p\n", Tss64->IST7);
    MmMapGuestPages (Tss64->IST7, ADDRESS_AND_SIZE_TO_SPAN_PAGES (Tss64->IST7, PAGE_SIZE));
  }

  return STATUS_SUCCESS;
}
*/

/**
 * effects: Initialize the Memory Manager.
 */
NTSTATUS NTAPI HvMmInitManager (
)
{
	PVOID pPdePage,SparePage;
	NTSTATUS Status;
	PHYSICAL_ADDRESS l1, l2, l3;
	
	//Step 1. Allocate memory for Cpu->SparePage First
	SparePage = ExAllocatePoolWithTag (NonPagedPool, PAGE_SIZE, LAB_TAG);
    SparePagePA = MmGetPhysicalAddress (SparePage);
	
	//Step 2.
	InitializeListHead (&g_PageTableList);
	KeInitializeSpinLock (&g_PageTableListLock);

	pPdePage = ExAllocatePoolWithTag (NonPagedPool, PAGE_SIZE, LAB_TAG);
	if (!pPdePage)
	return STATUS_INSUFFICIENT_RESOURCES;
	RtlZeroMemory (pPdePage, PAGE_SIZE);

	// store the page root
	g_PageMapBasePhysicalAddress = MmGetPhysicalAddress (pPdePage);
	Status = MmSavePage (
		g_PageMapBasePhysicalAddress, 
		(PVOID) PDE_BASE, 
		pPdePage, 
		PAT_POOL, 
		1, 
		AP_PAGETABLE | AP_PDE);
	if (!NT_SUCCESS(Status))
	{
		DbgPrint ("MmInitManager(): MmSavePage() failed to save PML4 page, status 0x%08X\n", Status);
		return Status;
	}

	// map it
	Status = MmCreateMapping (
		g_PageMapBasePhysicalAddress, 
		(PVOID) PDE_BASE, 
		FALSE);
	if (!NT_SUCCESS (Status)) 
	{
		DbgPrint ("MmInitManager(): MmCreateMapping() failed to map PML4 page, status 0x%08X\n", Status);
		return Status;
	}

	//Step 4. Copy kernel pages
	Status = MmMapGuestKernelPages();
	
	return Status;
}

/**
 * effects: Shutdown the Memory Manager.
 */
NTSTATUS NTAPI HvMmShutdownManager (
)
{
    PALLOCATED_PAGE AllocatedPage;
    ULONG i;
    PULONG Entry;

    while (AllocatedPage = (PALLOCATED_PAGE) ExInterlockedRemoveHeadList (&g_PageTableList, &g_PageTableListLock)) 
    {
        AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);

        if (AllocatedPage->Flags & AP_PAGETABLE) 
        {
            for (i = 0, Entry = AllocatedPage->GuestAddress; i < 0x400; i++)
            {
                if (Entry[i] & P_ACCESSED)
                {
                    DbgPrint
                        ("MmShutdownManager(): HPT 0x%p: index %d accessed, entry 0x%p, FL 0x%X\n",
                        AllocatedPage->HostAddress, i, Entry[i], AllocatedPage->Flags);
                }
            }
        }

        switch (AllocatedPage->AllocationType) 
        {
        case PAT_POOL:
            ExFreePool (AllocatedPage->GuestAddress);
            break;
        case PAT_CONTIGUOUS:
            MmFreeContiguousMemorySpecifyCache (
                AllocatedPage->GuestAddress,
                AllocatedPage->uNumberOfPages * PAGE_SIZE,
                MmCached);
            break;
        case PAT_DONT_FREE:
            // this is not the first page in the allocation
            break;
        }

        ExFreePool (AllocatedPage);
    }

    return STATUS_SUCCESS;
}

// unchecked
NTSTATUS NTAPI MmInitIdentityPageTable (
)
{
  //PHYSICAL_ADDRESS FirstPdePA, FirstPdptePA, l1, l2, l3;
  //PULONG64 FirstPdeVA, FirstPdpteVA, FirstPml4eVA;
  //PULONG64 FirstPdeVa_Legacy;
  //ULONG64 i, j;
  //l1.QuadPart = 0;
  //l2.QuadPart = -1;
  //l3.QuadPart = 0x200000;

  ////Long Mode

  ////64*512 Pde
  //FirstPdeVA = (PULONG64) MmAllocateContiguousMemorySpecifyCache (64 * PAGE_SIZE, l1, l2, l3, MmCached);
  //if (!FirstPdeVA)
  //  return STATUS_INSUFFICIENT_RESOURCES;

  //RtlZeroMemory (FirstPdeVA, 64 * PAGE_SIZE);

  //FirstPdePA = MmGetPhysicalAddress (FirstPdeVA);

  //_KdPrint (("MmInitIdentityPageTable: FirstPdeVA 0x%p FirstPdePA 0x%llX\n", FirstPdeVA, FirstPdePA.QuadPart));
  //for (i = 0; i < 64; i++) {
  //  for (j = 0; j < 512; j++) {
  //    *FirstPdeVA = ((i * 0x40000000) + j * 0x200000) | P_WRITABLE | P_PRESENT | P_CACHE_DISABLED | P_LARGE;
  //    FirstPdeVA++;
  //  }
  //}

  ////64 Pdpte
  //FirstPdpteVA = (PULONG64) MmAllocateContiguousMemorySpecifyCache (PAGE_SIZE, l1, l2, l3, MmCached);

  //if (!FirstPdpteVA)
  //  return STATUS_INSUFFICIENT_RESOURCES;

  //RtlZeroMemory (FirstPdpteVA, PAGE_SIZE);

  //FirstPdptePA = MmGetPhysicalAddress (FirstPdpteVA);

  //_KdPrint (("MmInitIdentityPageTable: FirstPdpteVA 0x%p FirstPdptePA 0x%llX\n", FirstPdpteVA, FirstPdptePA.QuadPart));
  //for (i = 0; i < 64; i++) {
  //  {
  //    *FirstPdpteVA = (i * 0x1000 + FirstPdePA.QuadPart) | P_WRITABLE | P_PRESENT | P_CACHE_DISABLED;
  //    FirstPdpteVA++;
  //  }
  //}

  ////Pml4e
  //FirstPml4eVA = (PULONG64) MmAllocateContiguousMemorySpecifyCache (PAGE_SIZE, l1, l2, l3, MmCached);

  //if (!FirstPml4eVA)
  //  return STATUS_INSUFFICIENT_RESOURCES;

  //RtlZeroMemory (FirstPml4eVA, PAGE_SIZE);

  //g_IdentityPageTableBasePhysicalAddress = MmGetPhysicalAddress (FirstPml4eVA);

  //_KdPrint (("MmInitIdentityPageTable: FirstPml4eVA 0x%p g_IdentityPageTableBasePhysicalAddress 0x%llX\n", FirstPdeVA,
  //           g_IdentityPageTableBasePhysicalAddress.QuadPart));
  //*FirstPml4eVA = (FirstPdptePA.QuadPart) | P_WRITABLE | P_PRESENT | P_CACHE_DISABLED;

  ////Legacy Mode
  //FirstPdeVa_Legacy = (PULONG64) MmAllocateContiguousMemorySpecifyCache (PAGE_SIZE, l1, l2, l3, MmCached);

  //if (!FirstPml4eVA)
  //  return STATUS_INSUFFICIENT_RESOURCES;

  //RtlZeroMemory (FirstPdeVa_Legacy, PAGE_SIZE);

  //g_IdentityPageTableBasePhysicalAddress_Legacy = MmGetPhysicalAddress (FirstPdeVa_Legacy);
  //for (j = 0; j < 4; j++) {
  //  *FirstPdeVa_Legacy = (j * 0x1000 + FirstPdePA.QuadPart) | P_PRESENT | P_CACHE_DISABLED;
  //  FirstPdeVa_Legacy++;
  //}
  //_KdPrint (("MmInitIdentityPageTable: FirstPdeVa_Legacy 0x%p g_IdentityPageTableBasePhysicalAddress_Legacy 0x%llX\n",
  //           FirstPdeVa_Legacy, g_IdentityPageTableBasePhysicalAddress_Legacy.QuadPart));

  return STATUS_SUCCESS;
}

#endif