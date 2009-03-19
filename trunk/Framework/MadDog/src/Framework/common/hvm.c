
#include "hvm.h"
//#include "vmxtraps.h"

static KMUTEX g_HvmMutex;
extern PMadDog_Control g_HvmControl;

ULONG g_uSubvertedCPUs = 0;
PHVM_DEPENDENT Hvm;

static NTSTATUS HvmSetupGdt (
    PCPU Cpu
);

static NTSTATUS HvmSetupIdt (
    PCPU Cpu
);

static NTSTATUS NTAPI HvmLiberateCpu (
    PVOID Param
);

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++

/**
 * effects:卸载HelloWorld VM
 */
NTSTATUS NTAPI HvmSpitOutBluepill (
)
{//Finish

	#ifndef ENABLE_HYPERCALLS

		return STATUS_NOT_SUPPORTED;

	#else

	CCHAR cProcessorNumber;
	NTSTATUS Status, CallbackStatus;

	//g_bDisableComOutput = TRUE;

	Print(("HelloWorld:HvmSpitOutBluepill(): Going to liberate %d processor%s\n",
		 KeNumberProcessors, KeNumberProcessors == 1 ? "" : "s"));

	KeWaitForSingleObject (&g_HvmMutex, Executive, KernelMode, FALSE, NULL);

	for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) {

		Print(("HelloWorld:HvmSpitOutBluepill(): Liberating processor #%d\n", cProcessorNumber));

		Status = CmDeliverToProcessor (cProcessorNumber, HvmLiberateCpu, NULL, &CallbackStatus,TRUE);//<-------------3.1  Finish

		if (!NT_SUCCESS (Status)) {
			KdPrint(("HelloWorld:HvmSpitOutBluepill(): CmDeliverToProcessor() failed with status 0x%08hX\n", Status));
		}

		if (!NT_SUCCESS (CallbackStatus)) {
			Print(("HelloWorld:HvmSpitOutBluepill(): HvmLiberateCpu() failed with status 0x%08hX\n", CallbackStatus));
		}
	}

	Print(("HelloWorld:HvmSpitOutBluepill(): Finished at irql %d\n", KeGetCurrentIrql ()));

	KeReleaseMutex (&g_HvmMutex, FALSE);
	return STATUS_SUCCESS;
	#endif
}


/**
 * effects: install our VM root hypervisor on the fly.
 * 在操作系统运行时刻动态安装helloworld
 */
NTSTATUS NTAPI HvmSwallowBluepill()
{//SAME
	CCHAR cProcessorNumber;
	NTSTATUS Status, CallbackStatus;

	Print(("HelloWorld:HvmSwallowBluepill(): Going to subvert %d processor%s\n",
			 KeNumberProcessors, KeNumberProcessors == 1 ? "" : "s"));

	KeWaitForSingleObject (&g_HvmMutex, Executive, KernelMode, FALSE, NULL);

	for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) 
	{
		Print(("HelloWorld:HvmSwallowBluepill():Installing HelloWorld VT Root Manager on processor #%d\n", cProcessorNumber));

		Status = CmDeliverToProcessor(cProcessorNumber, CmSubvert, NULL, &CallbackStatus,TRUE);//<----------------2.1 Finish

		if (!NT_SUCCESS (Status)) {
			Print(("HelloWorld:HvmSwallowBluepill(): CmDeliverToProcessor() failed with status 0x%08hX\n", Status));
			KeReleaseMutex (&g_HvmMutex, FALSE);

			HvmSpitOutBluepill ();//<----------------2.2

			return Status;
		}

		if (!NT_SUCCESS (CallbackStatus)) {
			Print(("HelloWorld:HvmSwallowBluepill(): HvmSubvertCpu() failed with status 0x%08hX\n", CallbackStatus));
			KeReleaseMutex (&g_HvmMutex, FALSE);

			HvmSpitOutBluepill ();

			return CallbackStatus;
		}
	}

	KeReleaseMutex (&g_HvmMutex, FALSE);

	if (KeNumberProcessors != g_uSubvertedCPUs) {
		HvmSpitOutBluepill ();
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}

/**
 * Check if this cpu supports Intel VT Technology.
 */
NTSTATUS NTAPI HvmInit()//Finished
{
	//BOOLEAN ArchIsOK = FALSE;
	//
	//Hvm = &Vmx;

 //   ArchIsOK = Hvm->ArchIsHvmImplemented ();

	//if (!ArchIsOK) {
	//	Print(("HvmInit(): Your Intel CPU doesn't either support VT Technology or isn't an Intel CPU at all.\n"));
	//	return STATUS_NOT_SUPPORTED;
	//} else {
	//	Print(("HvmInit(): Your Intel CPU supports VT Technology.\n"));
	//}

	KeInitializeMutex (&g_HvmMutex, 0);

	return STATUS_SUCCESS;
}
/**
 * Check if this cpu supports Intel VT Technology.
 */
BOOLEAN NTAPI HvmSupport()
{
	BOOLEAN ArchIsOK = FALSE;
	
	Hvm = &Vmx;

    ArchIsOK = Hvm->ArchIsHvmImplemented ();

	return ArchIsOK;
}

/**
 * Intialize the CPU struct and start VM by invoking VmxVirtualize()
 * requires: a valid <GuestRsp>
 * 构建CPU结构，然后通过调用VmxVirtualize()函数开启VM
 */
NTSTATUS NTAPI HvmSubvertCpu (
    PVOID GuestRsp
)
{ //Finish
    PCPU Cpu;//It will be used as the hypervisor struct.
    PVOID HostKernelStackBase;
    NTSTATUS Status;
    PHYSICAL_ADDRESS HostStackPA;

    Print(("HvmSubvertCpu(): Running on processor #%d\n", KeGetCurrentProcessorNumber()));
	
	//区别1:删除了对VT技术再次确认，因为认为这个是多余的。
    //if (!Hvm->ArchIsHvmImplemented()) 
    //{
    //    Print (("HvmSubvertCpu(): HVM extensions not implemented on this processor\n"));
    //    return STATUS_NOT_SUPPORTED;
    //}

    // allocate memory for host stack, 16 * 4k
    HostKernelStackBase = MmAllocatePages(HOST_STACK_SIZE_IN_PAGES, &HostStackPA);
    if (!HostKernelStackBase) 
    {
        Print(("HvmSubvertCpu(): Failed to allocate %d pages for the host stack\n", HOST_STACK_SIZE_IN_PAGES));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // unchecked -8 or -4 ?
    Cpu = (PCPU) ((PCHAR) HostKernelStackBase + HOST_STACK_SIZE_IN_PAGES * PAGE_SIZE - 4 - sizeof (CPU));
    Cpu->HostStack = HostKernelStackBase;

    // for interrupt handlers which will address CPU through the FS
    Cpu->SelfPointer = Cpu;

    Cpu->ProcessorNumber = KeGetCurrentProcessorNumber();

   // Cpu->Nested = FALSE;

    InitializeListHead (&Cpu->GeneralTrapsList);
    InitializeListHead (&Cpu->MsrTrapsList);
   // InitializeListHead (&Cpu->IoTrapsList);

    Cpu->GdtArea = MmAllocatePages (BYTES_TO_PAGES (BP_GDT_LIMIT), NULL);//Currently we create our own GDT and IDT area
    if (!Cpu->GdtArea) 
    {
        Print(("HvmSubvertCpu(): Failed to allocate memory for GDT\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Cpu->IdtArea = MmAllocatePages (BYTES_TO_PAGES (BP_IDT_LIMIT), NULL);
    if (!Cpu->IdtArea) 
    {
        Print(("HvmSubvertCpu(): Failed to allocate memory for IDT\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
	
	//区别2:删除了SparePage
    // allocate a 4k page. Fail the init if we can't allocate such page
    // (e.g. all allocations reside on 2mb pages).

//    //Cpu->SparePage=MmAllocatePages(1,&Cpu->SparePagePA);
//    Cpu->SparePage = MmAllocateContiguousPagesSpecifyCache (1, &Cpu->SparePagePA, MmCached);
//    if (!Cpu->SparePage)
//    {
//        Print (("HvmSubvertCpu(): Failed to allocate 1 page for the dummy page (DPA_CONTIGUOUS)\n"));
//        return STATUS_UNSUCCESSFUL;
//    }
//    // this is valid only for host page tables, as this VA may point into 2mb page in the guest.
//    Cpu->SparePagePTE = (PULONG)GET_PTE_VADDRESS(Cpu->SparePage);
//
//#ifdef SVM_SPAREPAGE_NON_CACHED
//    *Cpu->SparePagePTE |= (1 << 4);       // set PCD (Cache Disable);
//#endif

    //Status = Hvm->ArchRegisterTraps(Cpu);//<----------------3.1 Finish
	Status =  g_HvmControl->ApplyTraps(Cpu);
    if (!NT_SUCCESS (Status)) 
    {
		Print(("Helloworld:HvmSubvertCpu(): Failed to register NewBluePill traps, status 0x%08hX\n", Status));
        return STATUS_UNSUCCESSFUL;
    }

    Status = Hvm->ArchInitialize (Cpu, CmSlipIntoMatrix, GuestRsp);//<----------------3.2 Finish
    if (!NT_SUCCESS (Status)) 
    {
        Print(("Helloworld:HvmSubvertCpu(): ArchInitialize() failed with status 0x%08hX\n", Status));
        return Status;
    }

    InterlockedIncrement (&g_uSubvertedCPUs);

    // no API calls allowed below this point: we have overloaded GDTR and selectors
    // unchecked
//#ifdef _X86_
    HvmSetupGdt (Cpu);//<----------------3.3 Finish
    HvmSetupIdt (Cpu);//<----------------3.4 Finish
//#else
//    HvmSetupGdt (Cpu);
//    HvmSetupIdt (Cpu);
//#endif

#if DEBUG_LEVEL > 1
    Print(("HvmSubvertCpu(): RFLAGS = %#x\n", RegGetRflags ()));
#endif

    Status = Hvm->ArchVirtualize(Cpu);//<----------------3.5 Finish

    // never reached
    InterlockedDecrement (&g_uSubvertedCPUs);
    return Status;
}

NTSTATUS NTAPI HvmResumeGuest (
)
{
    Print(("HvmResumeGuest(): Processor #%d, irql %d in GUEST\n",
        KeGetCurrentProcessorNumber (), 
        KeGetCurrentIrql ()));

    // irql will be lowered in the CmDeliverToProcessor()
    //CmSti();
    return STATUS_SUCCESS;
}

//+++++++++++++++++++++Static Functions++++++++++++++++++++++++

//Must move to x86 architecture.
// unchecked
/**
 * effects:复制当前的GDT表到Cpu结构中存储。
 */
static NTSTATUS HvmSetupGdt (
    PCPU Cpu
)
{	//Finish
    ULONG64 GuestTssBase;
    USHORT GuestTssLimit;
    PSEGMENT_DESCRIPTOR GuestTssDescriptor;

    if (!Cpu || !Cpu->GdtArea)
        return STATUS_INVALID_PARAMETER;

    memcpy (Cpu->GdtArea, (PVOID) GetGdtBase(), GetGdtLimit());

    CmReloadGdtr(Cpu->GdtArea, GetGdtLimit());//加载Cpu->GdtArea中的GDT表

    return STATUS_SUCCESS;

//#if DEBUG_LEVEL>2
//    CmDumpGdt ((PUCHAR)GetGdtBase(), 0x67);     //(USHORT)GetGdtLimit());
//#endif
//
//    // set code and stack selectors the same with NT to simplify our unloading
//    CmSetGdtEntry (Cpu->GdtArea,
//        BP_GDT_LIMIT,
//        BP_GDT64_CODE,
//        0, 0, LA_STANDARD | LA_DPL_0 | LA_CODE | LA_PRESENT | LA_READABLE | LA_ACCESSED, HA_LONG);
//
//    // we don't want to have a separate segment for DS and ES. They will be equal to SS.
//    CmSetGdtEntry (Cpu->GdtArea,
//        BP_GDT_LIMIT,
//        BP_GDT64_DATA,
//        0, 0xfffff, LA_STANDARD | LA_DPL_0 | LA_PRESENT | LA_WRITABLE | LA_ACCESSED, HA_GRANULARITY | HA_DB);
//
//    // fs
//    CmSetGdtEntry (Cpu->GdtArea,
//        BP_GDT_LIMIT,
//        KGDT64_R3_CMTEB, 0, 0x3c00, LA_STANDARD | LA_DPL_3 | LA_PRESENT | LA_WRITABLE | LA_ACCESSED, HA_DB);
//
//    // gs
//    CmSetGdtEntry (Cpu->GdtArea,
//        BP_GDT_LIMIT,
//        KGDT64_R3_DATA,
//        0, 0xfffff, LA_STANDARD | LA_DPL_3 | LA_PRESENT | LA_WRITABLE | LA_ACCESSED, HA_GRANULARITY | HA_DB);
//
//    GuestTssDescriptor = (PSEGMENT_DESCRIPTOR) (GetGdtBase () + GetTrSelector ());
//
//    GuestTssBase = GuestTssDescriptor->base0 | GuestTssDescriptor->base1 << 16 | GuestTssDescriptor->base2 << 24;
//    GuestTssLimit = GuestTssDescriptor->limit0 | (GuestTssDescriptor->limit1attr1 & 0xf) << 16;
//    if (GuestTssDescriptor->limit1attr1 & 0x80)
//        // 4096-bit granularity is enabled for this segment, scale the limit
//        GuestTssLimit <<= 12;
//
//    if (!(GuestTssDescriptor->attr0 & 0x10))
//    {
//        GuestTssBase = (*(PULONG64) ((PUCHAR) GuestTssDescriptor + 4)) & 0xffffffffff000000;
//        GuestTssBase |= (*(PULONG32) ((PUCHAR) GuestTssDescriptor + 2)) & 0x00ffffff;
//    }
//#if DEBUG_LEVEL>2
//    CmDumpTSS64 ((PTSS64) GuestTssBase, GuestTssLimit);
//#endif
//
//    MmMapGuestTSS64 ((PTSS64) GuestTssBase, GuestTssLimit);
//
//    // don't need to reload TR - we use 0x40, as in xp/vista.
//    CmSetGdtEntry (Cpu->GdtArea, BP_GDT_LIMIT, BP_GDT64_SYS_TSS, (PVOID) GuestTssBase, GuestTssLimit,     //BP_TSS_LIMIT,
//        LA_BTSS64 | LA_DPL_0 | LA_PRESENT | LA_ACCESSED, 0);
//
//    // so far, we have 5 GDT entries.
//    // 0x10: CODE64         cpl0                                            CS
//    // 0x18: DATA           dpl0                                            DS, ES, SS
//    // 0x28: DATA           dpl3                                            GS
//    // 0x40: Busy TSS64, base is equal to NT TSS    TR
//    // 0x50: DATA           dpl3                                            FS
//
//#if DEBUG_LEVEL>2
//    CmDumpGdt ((PUCHAR) Cpu->GdtArea, BP_GDT_LIMIT);
//#endif
//
//    CmReloadGdtr (Cpu->GdtArea, BP_GDT_LIMIT);
//
//    // set new DS and ES
//    CmSetBluepillESDS ();
//
//    // we will use GS as our PCR pointer; GS base will be set to the Cpu in HvmEventCallback
//    // FIXME: but it is not?
//
//    return STATUS_SUCCESS;
}

/**
 * effects:复制当前的IDT表到Cpu结构中存储。
 */
static NTSTATUS HvmSetupIdt (
    PCPU Cpu
)
{
    UCHAR i;

    if (!Cpu || !Cpu->IdtArea)
        return STATUS_INVALID_PARAMETER;

    memcpy (Cpu->IdtArea, (PVOID) GetIdtBase(), GetIdtLimit());

    // just use the system IDT?
    //for (i = 0; i < 255; i++)
    //{
    //    CmSetIdtEntry(
    //        Cpu->IdtArea, 
    //        BP_IDT_LIMIT, 
    //        0x0d,    // #GP
    //        BP_GDT64_CODE, 
    //        InGeneralProtection, 
    //        0, 
    //        LA_PRESENT | LA_DPL_0 | LA_INTGATE64);
    //}

    CmReloadIdtr(Cpu->IdtArea, GetIdtLimit());//加载Cpu->IdtArea中的IDT表

    return STATUS_SUCCESS;
}
/**
 * effects:深入VMM ROOT 模式卸载HelloWorld VM
 */
static NTSTATUS NTAPI HvmLiberateCpu (
    PVOID Param
)
{ //Finish
  NTSTATUS Status;
  ULONG64 Efer;
  PCPU Cpu;

  // called at DPC level

  if (KeGetCurrentIrql () != DISPATCH_LEVEL)
    return STATUS_UNSUCCESSFUL;

  Efer = MsrRead (MSR_EFER);

  Print(("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on entry: 0x%X\n", Efer));

  //if (!NT_SUCCESS (Status = HcMakeHypercall (NBP_HYPERCALL_UNLOAD, 0, NULL))) {
  //  _KdPrint (("HvmLiberateCpu(): HcMakeHypercall() failed on processor #%d, status 0x%08hX\n",
  //             KeGetCurrentProcessorNumber (), Status));

  //  return Status;
  //}
  // for vmx
  //VmxVmCall(0x1);
  // cause vmexit
  RegSetCr3(MADDOG_EXIT_EAX);//<------------------!!!!!!!!卸载用!!!!!!!!!!

  Efer = MsrRead (MSR_EFER);
  Print(("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on exit: 0x%X\n", Efer));

  return STATUS_SUCCESS;
}

// this function is call when guest => host
VOID NTAPI HvmEventCallback (
    PCPU Cpu,                   // cpu struct
    PGUEST_REGS GuestRegs       // store guest's regs
)
{//Finished
    NTSTATUS Status;

    if (!Cpu || !GuestRegs)
        return;

    GuestRegs->esp = VmxRead (GUEST_RSP);

    // it's an original event
    Hvm->ArchDispatchEvent (Cpu, GuestRegs);

    VmxWrite (GUEST_RSP, GuestRegs->esp);

    return;
}
