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

#include "paging.h"

#define DbgPrint(...) {}

static LIST_ENTRY g_PageTableList;
//g_PageTableList 是一个对应自定义页的全局页表，其组成部分是挂在LIST_ENTRY上的一个一个ALLOCATED_PAGE结构（挂钩是其中的le项）
//这个页表可以做最基本的映射（好像他也就有意做一级映射，没看到多级）记录了分配方式（PAGE_ALLOCATION_TYPE），
//页数（分配时貌似要物理也对齐，虽然在windows中没这个要求）
static KSPIN_LOCK g_PageTableListLock;

PHYSICAL_ADDRESS g_PageMapBasePhysicalAddress;
PHYSICAL_ADDRESS g_IdentityPageTableBasePhysicalAddress, g_IdentityPageTableBasePhysicalAddress_Legacy;

static PUCHAR g_PageTableBases[4] = {
  (PUCHAR) PT_BASE,
  (PUCHAR) PD_BASE,
  (PUCHAR) PDP_BASE,
  (PUCHAR) PML4_BASE
};
//这个函数的作用就是保存了所有自定义页表和页信息到g_PageTableList中（这个自定义页可以有不定长个物理页）
//PhysicalAddress和GuestAddress在windows的页表中存在映射关系。具体他们三者的关系可参考"发现.doc"
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

  AllocatedPage = ExAllocatePoolWithTag (NonPagedPool, sizeof (ALLOCATED_PAGE), ITL_TAG);
  if (!AllocatedPage)
    return STATUS_INSUFFICIENT_RESOURCES;
  RtlZeroMemory (AllocatedPage, sizeof (ALLOCATED_PAGE));

  PhysicalAddress.QuadPart = PhysicalAddress.QuadPart & 0x000ffffffffff000;//取中间的40位
  HostAddress = (PVOID) ((ULONG64) HostAddress & 0xfffffffffffff000);//取高52位

  AllocatedPage->AllocationType = AllocationType;
  AllocatedPage->PhysicalAddress = PhysicalAddress;
  AllocatedPage->HostAddress = HostAddress;
  AllocatedPage->GuestAddress = GuestAddress;
  AllocatedPage->uNumberOfPages = uNumberOfPages;
  AllocatedPage->Flags = Flags;

  ExInterlockedInsertTailList (&g_PageTableList, &AllocatedPage->le, &g_PageTableListLock);//把一个entry原子性的插入到g_PageTableList（双向链表）的末尾

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
//通过PhysicalAddress在自定义全局页表（g_PageTableList）中寻找某个PageTable或页表项
static NTSTATUS NTAPI MmFindPageByPA (
  PHYSICAL_ADDRESS PhysicalAddress,//（in）
  PALLOCATED_PAGE * pAllocatedPage//返回找到的自定义页表项的指针（out）
)
{
  PALLOCATED_PAGE AllocatedPage;
  KIRQL OldIrql;

  if (!pAllocatedPage)
    return STATUS_INVALID_PARAMETER;

  KeAcquireSpinLock (&g_PageTableListLock, &OldIrql);//进入临界区

  PhysicalAddress.QuadPart = PhysicalAddress.QuadPart & 0x000ffffffffff000;//更新PhysicalAddress.QuadPart只要其中间40位去查找

  AllocatedPage = (PALLOCATED_PAGE) g_PageTableList.Flink;
  while (AllocatedPage != (PALLOCATED_PAGE) & g_PageTableList) {//如果g_PageTableList非空
    AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);
	//AllocatedPage被换成了AllocatedPage（此时AllocatedPage指向g_PageTableList中第一元素）的基地址和le在AllocatedPage中的偏移地址

    if (AllocatedPage->PhysicalAddress.QuadPart == PhysicalAddress.QuadPart) {
      *pAllocatedPage = AllocatedPage;
      KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
      return STATUS_SUCCESS;
    }

    AllocatedPage = (PALLOCATED_PAGE) AllocatedPage->le.Flink;//遍历下一项
  }

  KeReleaseSpinLock (&g_PageTableListLock, OldIrql);//退出临界区
  return STATUS_UNSUCCESSFUL;
}

//通过HostAddress在自定义全局页表（g_PageTableList）中寻找某个PageTable或页表项
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

  HostAddress = (PVOID) ((ULONG64) HostAddress & 0xfffffffffffff000);

  AllocatedPage = (PALLOCATED_PAGE) g_PageTableList.Flink;
  while (AllocatedPage != (PALLOCATED_PAGE) & g_PageTableList) {
    AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);

    if (AllocatedPage->HostAddress == HostAddress) {
      *pAllocatedPage = AllocatedPage;
      KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
      return STATUS_SUCCESS;
    }

    AllocatedPage = (PALLOCATED_PAGE) AllocatedPage->le.Flink;
  }

  KeReleaseSpinLock (&g_PageTableListLock, OldIrql);
  return STATUS_UNSUCCESSFUL;
}
//构建整个PageTable的关键方法。与MmCreateTable配合使用，主要负责页表设定，填充
static NTSTATUS NTAPI MmUpdatePageTable (
  PVOID PageTable,
  UCHAR PageTableLevel,//要创建的子级页表的等级
  PVOID VirtualAddress,
  PHYSICAL_ADDRESS PhysicalAddress,
  BOOLEAN bLargePage//P_LARGE标志只会出现在次低级页表项上
)
{
  ULONG64 PageTableOffset, GlobalOffset;
  ULONG64 GlobalOffset1, GlobalOffset2, GlobalOffset3, GlobalOffset4;
  PVOID LowerPageTableHostVA, LowerPageTableGuestVA;
  PALLOCATED_PAGE LowerPageTable;
  PHYSICAL_ADDRESS LowerPageTablePA;
  NTSTATUS Status;
  PHYSICAL_ADDRESS PagePA, l1, l2, l3;

  PALLOCATED_PAGE Pml4e, Pdpe, Pde, Pte;

  // get the offset in the specified page table level
  PageTableOffset = (((ULONG64) VirtualAddress & (((ULONG64) 1) << (12 + PageTableLevel * 9)) 
                      - 1) >> (12 + ((ULONG64) PageTableLevel - 1) * 9));//(VirtualAddress &( 1<<48)-1)>>39//最后12位未知，每个PageTable占9位，PageTableOffset也有9位

  if ((PageTableLevel == 1) || (bLargePage && (PageTableLevel == 2))) //低级或次低级bLargePage
  {
    // patch PTE/PDE
/*
		GlobalOffset=(((ULONG64)VirtualAddress & (((ULONG64)1)<<(12+4*9))-1)>>12);

		GlobalOffset4=(((ULONG64)VirtualAddress & (((ULONG64)1)<<(12+4*9))-1)>>(12+(3)*9));
		GlobalOffset3=(((ULONG64)VirtualAddress & (((ULONG64)1)<<(12+4*9))-1)>>(12+(2)*9));
		GlobalOffset2=(((ULONG64)VirtualAddress & (((ULONG64)1)<<(12+4*9))-1)>>(12+(1)*9));
		GlobalOffset1=(((ULONG64)VirtualAddress & (((ULONG64)1)<<(12+4*9))-1)>>(12+(0)*9));

		MmFindPageByHostVA(GlobalOffset4*8+g_PageTableBases[3],&Pml4e);
		MmFindPageByHostVA(GlobalOffset3*8+g_PageTableBases[2],&Pdpe);
		MmFindPageByHostVA(GlobalOffset2*8+g_PageTableBases[1],&Pde);
		MmFindPageByHostVA(GlobalOffset1*8+g_PageTableBases[0],&Pte);

		DbgPrint("MmUpdatePageTable(): VA 0x%p: PML4E 0x%p, PDPE 0x%p, PDE 0x%p, PTE 0x%p\n",
			VirtualAddress,
			GlobalOffset4*8+g_PageTableBases[3],
			GlobalOffset3*8+g_PageTableBases[2],
			GlobalOffset2*8+g_PageTableBases[1],
			GlobalOffset1*8+g_PageTableBases[0]);
		DbgPrint("MmUpdatePageTable(): Guest: PML4E 0x%p, PDPE 0x%p, PDE 0x%p, PTE 0x%p\n",
			(GlobalOffset4*8 & 0xfff) + (PUCHAR)Pml4e->GuestAddress,
			(GlobalOffset3*8 & 0xfff) + (PUCHAR)Pdpe->GuestAddress,
			(GlobalOffset2*8 & 0xfff) + (PUCHAR)Pde->GuestAddress,
			(GlobalOffset1*8 & 0xfff) + (PUCHAR)Pte->GuestAddress);

		DbgPrint("MmUpdatePageTable(): VA 0x%p, HPTE 0x%p, GPTE 0x%p, PA 0x%p\n",
			VirtualAddress,
			GlobalOffset*8+g_PageTableBases[0],
			(PUCHAR)PageTable+8*PageTableOffset,
			PhysicalAddress.QuadPart);
*/

#ifdef SET_PCD_BIT//是否禁用Cache的设置
    ((PULONG64) PageTable)[PageTableOffset] = PhysicalAddress.QuadPart |        /*P_GLOBAL | */
		//64位是8个字节，所以这里的话虽然PageTableOffset只有9位，但是一共有2^9个8字节，因此仍然填充满了4K(PAGE_SIZE)大小的空间(也就是Pml4Page->GuestAddress
		//所指的在MmInitManager中Pml4Page所指向的自定义页表那块内存)
      P_WRITABLE | P_PRESENT | P_CACHE_DISABLED;//可写，当前在内存，禁用Cache，这里的这些是用来描述页的
		//对于PhysicalAddress.QuadPart，实际上后12位也确实是为了1.页对齐就足够了。2.自定义的一些标志可以使用这些位（这些标志在common.h的240-248行）
#else
    ((PULONG64) PageTable)[PageTableOffset] = PhysicalAddress.QuadPart | /*P_GLOBAL | */ P_WRITABLE | P_PRESENT;//可写，当前在内存
#endif
    if (bLargePage)
      ((PULONG64) PageTable)[PageTableOffset] |= P_LARGE;//标记为P_LARGE
    return STATUS_SUCCESS;
  }

  GlobalOffset =
	    (((ULONG64) VirtualAddress & (((ULONG64) 1) << (12 + 4 * 9)) - 1) >> (12 + ((ULONG64) PageTableLevel - 2) * 9));//(VirtualAddress &(1<<48)-1)>>(12+(PageTableLevel - 2) * 9)
  //GlobalOffset有效位可能有36，27，18，9位，对应页表(PageTableLevel)级数为2，3，4，5级(5是不可能被用到的)
  LowerPageTablePA.QuadPart = ((PULONG64) PageTable)[PageTableOffset] & 0x000ffffffffff000;
  LowerPageTableHostVA = GlobalOffset * 8 + g_PageTableBases[PageTableLevel - 2];//LowerPageTableHostVA的低3位肯定是0，这个地址一会儿会被用来寻找Page

  if (!LowerPageTablePA.QuadPart) //如果LowerPageTablePA.QuadPart无效
  {
    Status = MmFindPageByHostVA (LowerPageTableHostVA, &LowerPageTable);
    if (!NT_SUCCESS (Status)) {
		//如果未找到LowerPageTableHostVA所对应子PageTable,则要新建一个LowerPageTableHostVA对应的子PageTable（新建子级PageTable）
      LowerPageTableGuestVA = ExAllocatePoolWithTag (NonPagedPool, PAGE_SIZE, ITL_TAG);
      if (!LowerPageTableGuestVA)
        return STATUS_INSUFFICIENT_RESOURCES;
      RtlZeroMemory (LowerPageTableGuestVA, PAGE_SIZE);

      LowerPageTablePA = MmGetPhysicalAddress (LowerPageTableGuestVA);

      Status =
        MmSavePage (LowerPageTablePA, LowerPageTableHostVA,
                    LowerPageTableGuestVA, PAT_POOL, 1, AP_PAGETABLE | (1 << (PageTableLevel - 1)));
      if (!NT_SUCCESS (Status)) {//如果创建子级PageTable未成功则输出信息
        DbgPrint
          ("MmUpdatePageTable(): Failed to store page table level %d, MmSavePage() returned status 0x%08X\n",
           PageTableLevel - 1, Status);
        return Status;
      }
    } else {
      LowerPageTablePA.QuadPart = LowerPageTable->PhysicalAddress.QuadPart;//实际上是相对应的物理地址
      LowerPageTableGuestVA = LowerPageTable->GuestAddress;//实际上是OS中这段内存的地址
    }

#ifdef SET_PCD_BIT
    ((PULONG64) PageTable)[PageTableOffset] = LowerPageTablePA.QuadPart |       /*P_GLOBAL | */
      P_WRITABLE | P_PRESENT | P_CACHE_DISABLED;//这里的这些是用来描述子页表的
#else
    ((PULONG64) PageTable)[PageTableOffset] = LowerPageTablePA.QuadPart | /*P_GLOBAL | */ P_WRITABLE | P_PRESENT;
#endif

    Status = MmCreateMapping (LowerPageTablePA, LowerPageTableHostVA, FALSE);//创建子级PageTable映射关系，通过调用MmCreateMapping产生间接递归
    if (!NT_SUCCESS (Status)) {
      DbgPrint
        ("MmUpdatePageTable(): MmCreateMapping() failed to map PA 0x%p with status 0x%08X\n",
         LowerPageTablePA.QuadPart, Status);
      return Status;
    }

  }else //如果LowerPageTablePA.QuadPart有效
  {
	Status = MmFindPageByPA (LowerPageTablePA, &LowerPageTable);//用LowerPageTablePA寻址（是个PhysicalAddress）
		if (!NT_SUCCESS (Status)) 
		{//如果没找到，那么说明这个地址不代表页表，因为如果代表页表，则在创建时是根据LowerPageTableHostVA创建再设置LowerPageTablePA.QuadPart的，
			//因此LowerPageTablePA.QuadPart必无效而不会跳到这里
			LowerPageTablePA.QuadPart = ((PULONG64) PageTable)[PageTableOffset];
			if ((PageTableLevel == 2) && (LowerPageTablePA.QuadPart & P_LARGE)) 
			{
				DbgPrint ("MmUpdatePageTable(): Found large PDE, data 0x%p\n", LowerPageTablePA.QuadPart);
				 return STATUS_SUCCESS;
			}else 
			{
				DbgPrint
				("MmUpdatePageTable(): Failed to find lower page table (pl%d) guest VA, data 0x%p, status 0x%08X\n",
				PageTableLevel - 1, LowerPageTablePA.QuadPart, Status);//因为找到的都是具体Page内容而不会是指向一个页表了
				return Status;
			}
		}

    LowerPageTableGuestVA = LowerPageTable->GuestAddress;
  }

  return MmUpdatePageTable (LowerPageTableGuestVA, PageTableLevel - 1, VirtualAddress, PhysicalAddress, bLargePage);
}
//分配连续页，并把页信息保存到g_PageTableList中且正确创建映射。
PVOID NTAPI MmAllocatePages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA//输出属性，会被更新为第一个页的物理地址
)
{
  PVOID PageVA, FirstPage;
  PHYSICAL_ADDRESS PagePA;
  NTSTATUS Status;
  ULONG i;

  if (!uNumberOfPages)
    return NULL;

  FirstPage = PageVA = ExAllocatePoolWithTag (NonPagedPool, uNumberOfPages * PAGE_SIZE, ITL_TAG);//此时PageVA和FirstPage都是Windows操作系统的虚拟地址
  //Nonpaged pool, which is nonpageable system memory. Nonpaged pool can be accessed from any IRQL, but it is a scarce resource and drivers should allocate it only when necessary. 
  //The system can only allocate buffers larger than PAGE_SIZE from nonpaged pool in multiples of PAGE_SIZE. Requests for buffers larger than PAGE_SIZE, but not a PAGE_SIZE multiple, waste nonpageable memory. 
  if (!PageVA)
    return NULL;
  RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);//清零

  if (pFirstPagePA)
    *pFirstPagePA = MmGetPhysicalAddress (PageVA);

  for (i = 0; i < uNumberOfPages; i++) {

    // map to the same addresses in the host pagetables as they are in guest's
    PagePA = MmGetPhysicalAddress (PageVA);
    Status = MmSavePage (PagePA, PageVA, PageVA, !i ? PAT_POOL : PAT_DONT_FREE, uNumberOfPages, 0);//保存页描述到g_PageTableList
	//PageVA是连续的，PagePA可能是不连续的，对于每个PageVA，都要存uNumberOfPages
	//猜想这个问题的答案会出现在!i ? PAT_POOL : PAT_DONT_FREE中，为首的是PAT_POOL类型，后面的都是PAT_DONT_FREE类型，其实对于后者来说，可能uNumberOfPages就没什么用了
    if (!NT_SUCCESS (Status)) {
      DbgPrint ("MmAllocatePages(): MmSavePage() failed with status 0x%08X\n", Status);
      return NULL;
    }

    Status = MmCreateMapping (PagePA, PageVA, FALSE);//创建页映射，完成页表填写
    if (!NT_SUCCESS (Status)) {
      DbgPrint
        ("MmAllocatePages(): MmCreateMapping() failed to map PA 0x%p with status 0x%08X\n", PagePA.QuadPart, Status);
      return NULL;
    }

    PageVA = (PUCHAR) PageVA + PAGE_SIZE;
  }

  return FirstPage;//返回第一个页的虚拟地址
}
//分配连续页 ，这里的连续(Contiguous)应当是指PagePA(物理内存)的地址连续
PVOID NTAPI MmAllocateContiguousPages (
  ULONG uNumberOfPages,
  PPHYSICAL_ADDRESS pFirstPagePA//输出属性，会被更新为第一个页的物理地址
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
  l3.QuadPart = 0x200000;

  FirstPage = PageVA = MmAllocateContiguousMemorySpecifyCache (uNumberOfPages * PAGE_SIZE, l1, l2, l3, MmCached);//The MmAllocateContiguousMemorySpecifyCache routine allocates a range of physically contiguous, cache-aligned memory from nonpaged pool.
  //MmCached :The processor should cache the requested memory. 
  if (!PageVA)
    return NULL;

  RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);

  PagePA = MmGetPhysicalAddress (PageVA);
  if (pFirstPagePA)
    *pFirstPagePA = PagePA;

  for (i = 0; i < uNumberOfPages; i++) {//这之间的逻辑可参考MmAllocatePages

    // map to the same addresses in the host pagetables as they are in guest's

    Status = MmSavePage (PagePA, PageVA, PageVA, !i ? PAT_CONTIGUOUS : PAT_DONT_FREE, uNumberOfPages, 0);
    if (!NT_SUCCESS (Status)) {
      DbgPrint ("MmAllocateContiguousPages(): MmSavePage() failed with status 0x%08X\n", Status);
      return NULL;
    }

    Status = MmCreateMapping (PagePA, PageVA, FALSE);
    if (!NT_SUCCESS (Status)) {
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

PVOID NTAPI MmAllocateContiguousPagesSpecifyCache (
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
  l3.QuadPart = 0x10000;

  FirstPage = PageVA = MmAllocateContiguousMemorySpecifyCache (uNumberOfPages * PAGE_SIZE, l1, l2, l3, CacheType);
  if (!PageVA)
    return NULL;

  RtlZeroMemory (PageVA, uNumberOfPages * PAGE_SIZE);

  PagePA = MmGetPhysicalAddress (PageVA);
  if (pFirstPagePA)
    *pFirstPagePA = PagePA;

  for (i = 0; i < uNumberOfPages; i++) {

    // map to the same addresses in the host pagetables as they are in guest's

    Status = MmSavePage (PagePA, PageVA, PageVA, !i ? PAT_CONTIGUOUS : PAT_DONT_FREE, uNumberOfPages, 0);
    if (!NT_SUCCESS (Status)) {
      DbgPrint ("MmAllocateContiguousPages(): MmSavePage() failed with status 0x%08X\n", Status);
      return NULL;
    }

    Status = MmCreateMapping (PagePA, PageVA, FALSE);
    if (!NT_SUCCESS (Status)) {
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
//创建PhysicalAddress和VirtualAddress之间的映射，实际上构造完的结果使得可以用VirtualAddress去寻址PhysicalAddress
NTSTATUS NTAPI MmCreateMapping (
  PHYSICAL_ADDRESS PhysicalAddress,
  PVOID VirtualAddress,
  BOOLEAN bLargePage//对于LargePage的优化是通过把它放在次低级优先级上实现的
)
{
  PALLOCATED_PAGE Pml4Page;
  NTSTATUS Status;
  //如果是从MmInitManager执行这个方法，那么g_PageTableList中的唯一项是PhysicalAddress为g_PageMapBasePhysicalAddress的根级页表(PML4)
  Status = MmFindPageByPA (g_PageMapBasePhysicalAddress, &Pml4Page);//检查g_PageMapBasePhysicalAddress是否已对应上某个自定义页（检查PhysicalAddress为g_PageMapBasePhysicalAddress的自定义页表项是否已存在）

  if (!NT_SUCCESS (Status)) {//未找到（在MmInitManager未成功配置）
    return STATUS_UNSUCCESSFUL;
  }

  PhysicalAddress.QuadPart = PhysicalAddress.QuadPart & 0x000ffffffffff000;//把 PhysicalAddress.QuadPart更新为只要中间的40位
  VirtualAddress = (PVOID) ((ULONG64) VirtualAddress & 0xfffffffffffff000);//虚地址要高52位

  return MmUpdatePageTable (Pml4Page->GuestAddress, 4, VirtualAddress, PhysicalAddress, bLargePage);//通过MmCreateMapping与MmUpdatePageTable这两个方法的不断间接递归而构造整个从最高级到最低级的PageTable的Tree
  //这个4是指下一级页表的等级
}

NTSTATUS NTAPI MmMapGuestPages (
  PVOID FirstPage,
  ULONG uNumberOfPages
)
{
  PHYSICAL_ADDRESS PhysicalAddress;
  NTSTATUS Status;

  FirstPage = (PVOID) ((ULONG64) FirstPage & 0xfffffffffffff000);

  // Everything is made present, writable, executable, 4kb and cpl0 only.
  // Mapping is done to the same virtual addresses in the host.
  while (uNumberOfPages--) {

    // Guest memory may not be contiguous   
    PhysicalAddress = MmGetPhysicalAddress (FirstPage);

    if (!NT_SUCCESS (Status = MmCreateMapping (PhysicalAddress, FirstPage, FALSE))) {
      DbgPrint ("MmMapGuestPages(): MmCreateMapping() failed with status 0x%08X\n", Status);
      return Status;
    }

    FirstPage = (PVOID) ((PUCHAR) FirstPage + PAGE_SIZE);
  }

  return STATUS_SUCCESS;
}

NTSTATUS NTAPI MmWalkGuestPageTable (
  PULONG64 PageTable,
  UCHAR bLevel
)
{
  ULONG64 i;
  PVOID VirtualAddress;
  PUCHAR ShortPageVA;
  PHYSICAL_ADDRESS PhysicalAddress;
  PULONG64 LowerPageTable;

  if (!MmIsAddressValid (PageTable))
    return STATUS_SUCCESS;

  for (i = 0; i < 0x200; i++)

    if (PageTable[i] & P_PRESENT) {

      if (((bLevel == 2) && (PageTable[i] & P_LARGE)) || (bLevel == 1)) {

        if (bLevel == 1)
          VirtualAddress = (PVOID) (((LONGLONG) (&PageTable[i]) - PT_BASE) << 9);
        else
          VirtualAddress = (PVOID) (((LONGLONG) (&PageTable[i]) - PD_BASE) << 18);

        if ((LONGLONG) VirtualAddress & 0x0000800000000000)
          VirtualAddress = (PVOID) ((LONGLONG) VirtualAddress | 0xffff000000000000);

        PhysicalAddress.QuadPart = PageTable[i] & 0x000ffffffffff000;

        if ((ULONGLONG) VirtualAddress >= PT_BASE && (ULONGLONG) VirtualAddress < PML4_BASE + 0x1000)
          // guest pagetable stuff here - so don't map it
          continue;

        DbgPrint
          ("MmWalkGuestPageTable(): %sValid pl%d at 0x%p, index 0x%x, VA 0x%p, PA 0x%p %s\n",
           bLevel == 3 ? "   " : bLevel == 2 ? "      " : bLevel ==
           1 ? "         " : "", bLevel, &PageTable[i], i, VirtualAddress, PhysicalAddress.QuadPart, ((bLevel == 2)
                                                                                                      && (PageTable[i] &
                                                                                                          P_LARGE)) ?
           "LARGE" : "");

        if (bLevel == 2) {
          for (ShortPageVA = (PUCHAR) VirtualAddress + 0x0 * PAGE_SIZE;
               ShortPageVA < (PUCHAR) VirtualAddress + 0x200 * PAGE_SIZE;
               ShortPageVA += PAGE_SIZE, PhysicalAddress.QuadPart += PAGE_SIZE)
            MmCreateMapping (PhysicalAddress, ShortPageVA, FALSE);
        } else
          MmCreateMapping (PhysicalAddress, VirtualAddress, FALSE);
      }

      if ((bLevel > 1) && !((bLevel == 2) && (PageTable[i] & P_LARGE))) {
        LowerPageTable = (PULONG64) (g_PageTableBases[bLevel - 2] + 8 * (i << (9 * (5 - bLevel))));
        MmWalkGuestPageTable (LowerPageTable, bLevel - 1);
      }
    }

  return STATUS_SUCCESS;
}

NTSTATUS NTAPI MmMapGuestKernelPages (
)
{
  PULONG64 Pml4e = (PULONG64) PML4_BASE;
  PULONG64 Pdpe;
  PULONG64 Pde;
  ULONG uPml4eIndex, uPdpeIndex, uPdeIndex;

  for (uPml4eIndex = 0x100; uPml4eIndex < 0x200; uPml4eIndex++)// 4//256 Times
    if (Pml4e[uPml4eIndex] & P_PRESENT) {//这里非常诡异，所有的Pml4e[uPml4eIndex]都是0

      Pdpe = (PULONG64) PDP_BASE + (uPml4eIndex << 9);
      MmWalkGuestPageTable (Pdpe, 3);
    }

  return STATUS_SUCCESS;
}

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

NTSTATUS NTAPI MmInitManager (
)
{
  PVOID Pml4Page;
  NTSTATUS Status;
  PHYSICAL_ADDRESS l1, l2, l3;

  InitializeListHead (&g_PageTableList);//初始化一个全局的页表的列表
  KeInitializeSpinLock (&g_PageTableListLock);//进入临界区

  Pml4Page = ExAllocatePoolWithTag (NonPagedPool, PAGE_SIZE, ITL_TAG);//ExAllocatePoolWithTag作用是分配某种类型的Pool Memory。在这里类型是非分页的，一页大小，自己声明了个Tag是LTI（应该是个私有的Tag）
  if (!Pml4Page)//分配不成功
    return STATUS_INSUFFICIENT_RESOURCES;
  RtlZeroMemory (Pml4Page, PAGE_SIZE);//把一块内存里面的内容清零
  //分配了一个自定义页（只有一个物理页大小）
  //Pml4Page所指向的内存非常重要，因为它被用作一个自定义页表！！！
  g_PageMapBasePhysicalAddress = MmGetPhysicalAddress (Pml4Page);//获取这段内存开始处的物理地址（Pml4Page对应的物理地址）
  //把新建的Pml4Page对应页表注册到全局页表列表里
  if (!NT_SUCCESS
      (Status =
       MmSavePage (g_PageMapBasePhysicalAddress, (PVOID) PML4_BASE, Pml4Page, PAT_POOL, 1, AP_PAGETABLE | AP_PML4)))//PML4_BASE(HostAddress):0xFFFFF6FB7DBED000 ，并且这个是顶级页表邋（AP_PML4）
  {
	  //分配不成功
    DbgPrint ("MmInitManager(): MmSavePage() failed to save PML4 page, status 0x%08X\n", Status);
    return Status;
  }
	//构建页表
  if (!NT_SUCCESS (Status = MmCreateMapping (g_PageMapBasePhysicalAddress, (PVOID) PML4_BASE, FALSE))) {
    DbgPrint ("MmInitManager(): MmCreateMapping() failed to map PML4 page, status 0x%08X\n", Status);
    return Status;
  }

  return STATUS_SUCCESS;
}
//关闭MemoryMapping管理任务
//最主要的任务是释放g_PageTableList所存储的所有页表，所有页的信息，因此也释放了所有页所占的Windows内存资源
NTSTATUS NTAPI MmShutdownManager (
)
{
  PALLOCATED_PAGE AllocatedPage;
  ULONG i;
  PULONG64 Entry;

  while (AllocatedPage = (PALLOCATED_PAGE) ExInterlockedRemoveHeadList (&g_PageTableList, &g_PageTableListLock)) {
	//CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le)返回ALLOCATED_PAGE结构体的基虚拟地址
    AllocatedPage = CONTAINING_RECORD (AllocatedPage, ALLOCATED_PAGE, le);

    if (AllocatedPage->Flags & AP_PAGETABLE) {//如果是页表
      for (i = 0, Entry = AllocatedPage->GuestAddress; i < 0x200; i++) {
        if (Entry[i] & P_ACCESSED)
          DbgPrint
            ("MmShutdownManager(): HPT 0x%p: index %d accessed, entry 0x%p, FL 0x%X\n",
             AllocatedPage->HostAddress, i, Entry[i], AllocatedPage->Flags);
      }
    }

    switch (AllocatedPage->AllocationType) {
    case PAT_POOL:
      ExFreePool (AllocatedPage->GuestAddress);//从Windows操作系统中释放内存池
      break;
    case PAT_CONTIGUOUS:
      MmFreeContiguousMemorySpecifyCache (AllocatedPage->GuestAddress,
                                          AllocatedPage->uNumberOfPages * PAGE_SIZE, MmCached);//从Windows操作系统中释放连续分配内存空间
      break;
    case PAT_DONT_FREE:
      // this is not the first page in the allocation
	  // 这句话验证了在317行关于PAT_DONT_FREE类型的猜想
      break;
    }

    ExFreePool (AllocatedPage);
  }

  return STATUS_SUCCESS;
}

NTSTATUS NTAPI MmInitIdentityPageTable (
)
{
  PHYSICAL_ADDRESS FirstPdePA, FirstPdptePA, l1, l2, l3;
  PULONG64 FirstPdeVA, FirstPdpteVA, FirstPml4eVA;
  PULONG64 FirstPdeVa_Legacy;
  ULONG64 i, j;
  l1.QuadPart = 0;
  l2.QuadPart = -1;
  l3.QuadPart = 0x200000;

  //Long Mode

  //64*512 Pde
  FirstPdeVA = (PULONG64) MmAllocateContiguousMemorySpecifyCache (64 * PAGE_SIZE, l1, l2, l3, MmCached);
  if (!FirstPdeVA)
    return STATUS_INSUFFICIENT_RESOURCES;

  RtlZeroMemory (FirstPdeVA, 64 * PAGE_SIZE);

  FirstPdePA = MmGetPhysicalAddress (FirstPdeVA);

  _KdPrint (("MmInitIdentityPageTable: FirstPdeVA 0x%p FirstPdePA 0x%llX\n", FirstPdeVA, FirstPdePA.QuadPart));
  for (i = 0; i < 64; i++) {
    for (j = 0; j < 512; j++) {
      *FirstPdeVA = ((i * 0x40000000) + j * 0x200000) | P_WRITABLE | P_PRESENT | P_CACHE_DISABLED | P_LARGE;
      FirstPdeVA++;
    }
  }

  //64 Pdpte
  FirstPdpteVA = (PULONG64) MmAllocateContiguousMemorySpecifyCache (PAGE_SIZE, l1, l2, l3, MmCached);

  if (!FirstPdpteVA)
    return STATUS_INSUFFICIENT_RESOURCES;

  RtlZeroMemory (FirstPdpteVA, PAGE_SIZE);

  FirstPdptePA = MmGetPhysicalAddress (FirstPdpteVA);

  _KdPrint (("MmInitIdentityPageTable: FirstPdpteVA 0x%p FirstPdptePA 0x%llX\n", FirstPdpteVA, FirstPdptePA.QuadPart));
  for (i = 0; i < 64; i++) {
    {
      *FirstPdpteVA = (i * 0x1000 + FirstPdePA.QuadPart) | P_WRITABLE | P_PRESENT | P_CACHE_DISABLED;
      FirstPdpteVA++;
    }
  }

  //Pml4e
  FirstPml4eVA = (PULONG64) MmAllocateContiguousMemorySpecifyCache (PAGE_SIZE, l1, l2, l3, MmCached);

  if (!FirstPml4eVA)
    return STATUS_INSUFFICIENT_RESOURCES;

  RtlZeroMemory (FirstPml4eVA, PAGE_SIZE);

  g_IdentityPageTableBasePhysicalAddress = MmGetPhysicalAddress (FirstPml4eVA);

  _KdPrint (("MmInitIdentityPageTable: FirstPml4eVA 0x%p g_IdentityPageTableBasePhysicalAddress 0x%llX\n", FirstPdeVA,
             g_IdentityPageTableBasePhysicalAddress.QuadPart));
  *FirstPml4eVA = (FirstPdptePA.QuadPart) | P_WRITABLE | P_PRESENT | P_CACHE_DISABLED;

  //Legacy Mode
  FirstPdeVa_Legacy = (PULONG64) MmAllocateContiguousMemorySpecifyCache (PAGE_SIZE, l1, l2, l3, MmCached);

  if (!FirstPml4eVA)
    return STATUS_INSUFFICIENT_RESOURCES;

  RtlZeroMemory (FirstPdeVa_Legacy, PAGE_SIZE);

  g_IdentityPageTableBasePhysicalAddress_Legacy = MmGetPhysicalAddress (FirstPdeVa_Legacy);
  for (j = 0; j < 4; j++) {
    *FirstPdeVa_Legacy = (j * 0x1000 + FirstPdePA.QuadPart) | P_PRESENT | P_CACHE_DISABLED;
    FirstPdeVa_Legacy++;
  }
  _KdPrint (("MmInitIdentityPageTable: FirstPdeVa_Legacy 0x%p g_IdentityPageTableBasePhysicalAddress_Legacy 0x%llX\n",
             FirstPdeVa_Legacy, g_IdentityPageTableBasePhysicalAddress_Legacy.QuadPart));

  return STATUS_SUCCESS;
}
