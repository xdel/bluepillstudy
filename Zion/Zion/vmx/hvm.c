#include "hvm.h"

//static KMUTEX g_HvmMutex;		//未完成！
// static EFI_LOCK g_HvmLock;
extern PZion_Control g_HvmControl;
// 
uint32_t g_uSubvertedCPUs = 0;
uint32_t KeNumberProcessors = 1;
// PHVM_DEPENDENT Hvm;

// static EFI_STATUS HvmSetupGdt (
//     PCPU Cpu
// );
// 
// static EFI_STATUS HvmSetupIdt (
//     PCPU Cpu
// );
// 
// static EFI_STATUS EFIAPI HvmLiberateCpu (
//     PVOID Param
// );

static bool HvmIsImplemented(void);

//+++++++++++++++++++++Public Functions++++++++++++++++++++++++
/**
* effects: Uninstall and remove Zion VM
*/
ZVMSTATUS ZVMAPI HvmSpitOutBluepill ()
{//Finish

#ifndef ENABLE_HYPERCALLS

	return ZVM_NOT_SUPPORTED;

#else

	int8_t cProcessorNumber;
	ZVMSTATUS Status, CallbackStatus;

	cprintf("HelloWorld:HvmSpitOutBluepill(): Going to liberate %d processor%s\n",
	KeNumberProcessors, KeNumberProcessors == 1 ? "" : "s");

	// Not find the API in EFI Edk
	//KeWaitForSingleObject (&g_HvmLock, Executive, KernelMode, FALSE, NULL);

	for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) {

		cprintf("Zion Hypervisor:HvmSpitOutBluepill(): Liberating processor #%d\n", cProcessorNumber);

		Status = CmDeliverToProcessor (cProcessorNumber, HvmLiberateCpu, NULL, &CallbackStatus);//<-------------3.1  Finish by zhumin

		if (!ZVM_SUCCESS (Status)) {
			//KdPrint(("HelloWorld:HvmSpitOutBluepill(): CmDeliverToProcessor() failed with status 0x%08hX\n", Status));
		}

		if (!ZVM_SUCCESS (CallbackStatus)) {
			//Print(("HelloWorld:HvmSpitOutBluepill(): HvmLiberateCpu() failed with status 0x%08hX\n", CallbackStatus));
		}
	}

	//Print(("HelloWorld:HvmSpitOutBluepill(): Finished at irql %d\n", KeGetCurrentIrql ()));

	//KeReleaseMutex (&g_HvmLock, FALSE);
	EfiReleaseLock(&g_HvmLock);
	return ZVMSUCCESS;
#endif
}

/**
* effects: install our VM root hypervisor on the fly.
*/
ZVMSTATUS ZVMAPI HvmSwallowBluepill()
{//SAME
	int8_t cProcessorNumber;
	ZVMSTATUS Status, CallbackStatus;

	cprintf("Zion Hypervisor:HvmSwallowBluepill(): Going to subvert %d processor%s\n",
		KeNumberProcessors, KeNumberProcessors == 1 ? "" : "s");

	//KeWaitForSingleObject (&g_HvmMutex, Executive, KernelMode, FALSE, NULL);

	for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) 
	{
		cprintf("Zion Hypervisor:HvmSwallowBluepill():Installing HelloWorld VT Root Manager on processor #%d\n", cProcessorNumber);

		Status = CmDeliverToProcessor(cProcessorNumber, CmSubvert, NULL, &CallbackStatus,TRUE);//<----------------2.1 Finish

		if (!ZVM_SUCCESS (Status)) {
			cprintf("Zion Hypervisor:HvmSwallowBluepill(): CmDeliverToProcessor() failed with status 0x%08hX\n", Status);
			//KeReleaseMutex (&g_HvmMutex, FALSE);

			HvmSpitOutBluepill ();//<----------------2.2

			return Status;
		}

		if (!ZVM_SUCCESS (CallbackStatus)) {
			cprintf("Zion Hypervisor:HvmSwallowBluepill(): HvmSubvertCpu() failed with status 0x%08hX\n", CallbackStatus);
			//KeReleaseMutex (&g_HvmMutex, FALSE);

			HvmSpitOutBluepill ();

			return CallbackStatus;
		}
	}

	//KeReleaseMutex (&g_HvmMutex, FALSE);

	if (KeNumberProcessors != g_uSubvertedCPUs) {
		HvmSpitOutBluepill ();
		return ZVMUNSUCCESSFUL;
	}

	return ZVMSUCCESS;
}

/**
 * Check if this cpu supports Intel VT Technology.
 */
ZVMSTATUS ZVMAPI HvmInit()//Finished
{
	
	//KeInitializeMutex (&g_HvmLock, 0);
	//CmInitSpinLock(&g_HvmLock);
	//EfiInitializeLock(&g_HvmLock, 0);

	return ZVMSUCCESS;
}
/**
 * Check if this cpu supports Intel VT Technology.
 */
bool ZVMAPI HvmSupport()
{// Finished by zhumin
	bool ArchIsOK = FALSE;
	
// 	Hvm = &Vmx;
// 
//     ArchIsOK = Hvm->ArchIsHvmImplemented ();
	ArchIsOK = HvmIsImplemented();

	return ArchIsOK;
}

bool HvmIsImplemented(void)
{
	uint32_t eax, ebx, ecx, edx;
	cpuid(0, &eax, &ebx, &ecx, &edx);
	if (eax < 1) 
	{
		cprintf("HvmIsImplemented(): Extended CPUID functions not implemented\n");
		return FALSE;
	}
	if (!(ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69)) 
	{
		cprintf("HvmIsImplemented(): Not an INTEL processor\n");
		return FALSE;
	}

	// intel cpu use fun_0x1 to test VMX.
	// CPUID.1:ECX.VMX[bit 5] = 1
	cpuid(0x1, &eax, &ebx, &ecx, &edx);
	return (bool) (CmIsBitSet (ecx, 5));
}

// /**
//  * Intialize the CPU struct and start VM by invoking VmxVirtualize()
//  * requires: a valid <GuestRsp>
//  * 构建CPU结构，然后通过调用VmxVirtualize()函数开启VM
//  */
// 
// EFI_STATUS EFIAPI HvmSubvertCpu (
//     PVOID GuestRsp
// )
// { // UnFinished!!!!! The most important
//     PCPU Cpu;//It will be used as the hypervisor struct.
//     PVOID HostKernelStackBase;
//     EFI_STATUS Status;
//     EFI_PHYSICAL_ADDRESS HostStackPA;
// 
//     // allocate memory for host stack, 16 * 4k
//     HostKernelStackBase = MmAllocatePages(HOST_STACK_SIZE_IN_PAGES, &HostStackPA);
//     if (!HostKernelStackBase) 
//     {
//         //Print(("HvmSubvertCpu(): Failed to allocate %d pages for the host stack\n", HOST_STACK_SIZE_IN_PAGES));
//         //return STATUS_INSUFFICIENT_RESOURCES;
// 		return EFI_BUFFER_TOO_SMALL;
//     }
// 
//     // unchecked -8 or -4 ?
//     Cpu = (PCPU) ((PCHAR) HostKernelStackBase + HOST_STACK_SIZE_IN_PAGES * EFI_PAGE_SIZE - 4 - sizeof (CPU));
//     Cpu->HostStack = HostKernelStackBase;
// 
//     // for interrupt handlers which will address CPU through the FS
//     Cpu->SelfPointer = Cpu;
// 
//     //Cpu->ProcessorNumber = KeGetCurrentProcessorNumber();
// 	Cpu->ProcessorNumber = NumberOfProcessors;
// 
//    // Cpu->Nested = FALSE;
// 
//     InitializeListHead (&Cpu->GeneralTrapsList);
//     InitializeListHead (&Cpu->MsrTrapsList);
//    // InitializeListHead (&Cpu->IoTrapsList);
// 
//     Cpu->GdtArea = MmAllocatePages (EFI_SIZE_TO_PAGES (BP_GDT_LIMIT), NULL);//Currently we create our own GDT and IDT area
//     if (!Cpu->GdtArea) 
//     {
//         //Print(("HvmSubvertCpu(): Failed to allocate memory for GDT\n"));
//         //return STATUS_INSUFFICIENT_RESOURCES;
// 		return EFI_BUFFER_TOO_SMALL;
//     }
// 
//     Cpu->IdtArea = MmAllocatePages (EFI_SIZE_TO_PAGES (BP_IDT_LIMIT), NULL);
//     if (!Cpu->IdtArea) 
//     {
//         //Print(("HvmSubvertCpu(): Failed to allocate memory for IDT\n"));
//         //return STATUS_INSUFFICIENT_RESOURCES;
// 		return EFI_BUFFER_TOO_SMALL;
//     }
// 	
// 	//Status = Hvm->ArchRegisterTraps(Cpu);//<----------------3.1 Finish
// 	Status =  g_HvmControl->ApplyTraps(Cpu);
//     if (!EFISUCCESS (Status)) 
//     {
// 		//Print(("Helloworld:HvmSubvertCpu(): Failed to register NewBluePill traps, status 0x%08hX\n", Status));
//         //return EFI_LOAD_ERROR;
// 		return EFI_LOAD_ERROR;
//     }
// 
//     //Status = Hvm->ArchInitialize (Cpu, CmSlipIntoMatrix, GuestRsp);//<----------------3.2 Finish
// 	//EFI_BREAKPOINT();
// 	Status = Hvm->ArchInitialize (Cpu, (PVOID) (UINTN)CmSlipIntoMatrix, GuestRsp);   // Can't use CmSlipIntoMatrix by PVOID
// 	if (!EFISUCCESS (Status)) 
//     {
//         //Print(("Helloworld:HvmSubvertCpu(): ArchInitialize() failed with status 0x%08hX\n", Status));
//         return Status;
//     }
// 
// 	gST->ConOut->OutputString (gST->ConOut, L"\t\n\n----------3:		Helloworld: HvmSubvertCpu(): ArchInitialize() succeed succeed!----------\r\n");
//     InterlockedIncrement (&g_uSubvertedCPUs);
// 
//     // no API calls allowed below this point: we have overloaded GDTR and selectors
//     // unchecked
//     HvmSetupGdt (Cpu);//<----------------3.3 Finish
//     HvmSetupIdt (Cpu);//<----------------3.4 Finish
// 
//     Status = Hvm->ArchVirtualize(Cpu);//<----------------3.5 Finish
// 	gST->ConOut->OutputString (gST->ConOut, L"\t\n\n----------4:		Helloworld: HvmSubvertCpu(): ArchVirtualize and VmxLanch succeed succeed!----------\r\n");
//     // never reached
//     InterlockedDecrement (&g_uSubvertedCPUs);
//     return Status;
// }
// 
// EFI_STATUS EFIAPI HvmResumeGuest ()
// {
// 	/*Print(("HvmResumeGuest(): Processor #%d, irql %d in GUEST\n",
// 		KeGetCurrentProcessorNumber (), 
// 		KeGetCurrentIrql ()));*/
// 
// 	// irql will be lowered in the CmDeliverToProcessor()
// 	//CmSti();
// 	return EFI_SUCCESS;
// }
// 
// //+++++++++++++++++++++Static Functions++++++++++++++++++++++++
// //Must move to x86 architecture.
// // unchecked
// /**
// * effects:复制当前的GDT表到Cpu结构中存储。
// */
// static EFI_STATUS HvmSetupGdt (
// 							 PCPU Cpu
// 							 )
// {	//Finish
// 	//UINT64 GuestTssBase;
// 	//USHORT GuestTssLimit;
// 	//PSEGMENT_DESCRIPTOR GuestTssDescriptor;
// 
// 	if (!Cpu || !Cpu->GdtArea)
// 		return EFI_INVALID_PARAMETER;
// 
// 	EfiCopyMem (Cpu->GdtArea, (PVOID) GetGdtBase(), GetGdtLimit());
// 
// 	CmReloadGdtr(Cpu->GdtArea, GetGdtLimit());//加载Cpu->GdtArea中的GDT表
// 
// 	return EFI_SUCCESS;
// }
// 
// /**
// * effects:复制当前的IDT表到Cpu结构中存储。
// */
// static EFI_STATUS HvmSetupIdt (
// 							 PCPU Cpu
// 							 )
// {
// 	//UCHAR i;
// 
// 	if (!Cpu || !Cpu->IdtArea)
// 		return EFI_INVALID_PARAMETER;
// 
// 	EfiCopyMem (Cpu->IdtArea, (PVOID) GetIdtBase(), GetIdtLimit());
// 
// 	CmReloadIdtr(Cpu->IdtArea, GetIdtLimit());//加载Cpu->IdtArea中的IDT表
// 
// 	return EFI_SUCCESS;
// }
// 
/**
* effects:深入VMM ROOT 模式卸载HelloWorld VM
*/
static EFI_STATUS EFIAPI HvmLiberateCpu (
									  PVOID Param
									  )
{ // Finish by zhumin
	//EFI_STATUS Status;
	UINT64 Efer;
	//PCPU Cpu;

	// called at DPC level

	/*if (KeGetCurrentIrql () != DISPATCH_LEVEL)
		return EFI_LOAD_ERROR;*/

	Efer = MsrRead (MSR_EFER);

	//Print(("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on entry: 0x%X\n", Efer));
	RegSetCr3(MADDOG_EXIT_EAX);//<------------------!!!!!!!!卸载用!!!!!!!!!!

	Efer = MsrRead (MSR_EFER);
	//Print(("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on exit: 0x%X\n", Efer));

	return EFI_SUCCESS;
}
// 
// // this function is call when guest => host
// VOID EFIAPI HvmEventCallback (
// 							 PCPU Cpu,                   // cpu struct
// 							 PGUEST_REGS GuestRegs       // store guest's regs
// 							 )
// {//Finished
// 	//EFI_STATUS Status;
// 
// 	if (!Cpu || !GuestRegs)
// 		return;
// 
// 	GuestRegs->esp = VmxRead (GUEST_RSP);
// 
// 	// it's an original event
// 	Hvm->ArchDispatchEvent (Cpu, GuestRegs);
// 
// 	VmxWrite (GUEST_RSP, GuestRegs->esp);
// 
// 	return;
// }
static ZVMSTATUS ZVMAPI HvmLiberateCpu (
									  PVOID Param
									  )
{ //Finish
	ZVMSTATUS Status;
	uint64_t Efer;
	PCPU Cpu;

	// called at DPC level

	if (KeGetCurrentIrql () != DISPATCH_LEVEL)
		return STATUS_UNSUCCESSFUL;

	Efer = MsrRead (MSR_EFER);

	cprintf("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on entry: 0x%X\n", Efer);

	//if (!NT_SUCCESS (Status = HcMakeHypercall (NBP_HYPERCALL_UNLOAD, 0, NULL))) {
	//  _KdPrint (("HvmLiberateCpu(): HcMakeHypercall() failed on processor #%d, status 0x%08hX\n",
	//             KeGetCurrentProcessorNumber (), Status));

	//  return Status;
	//}
	// for vmx
	//VmxVmCall(0x1);
	// cause vmexit
	RegSetCr3(MADDOG_EXIT_EAX);//

	Efer = MsrRead (MSR_EFER);
	cprintf("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on exit: 0x%X\n", Efer);

	return STATUS_SUCCESS;
}

