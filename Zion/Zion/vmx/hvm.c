#include "hvm.h"
//#include "vmxapi.h"

//static KMUTEX g_HvmMutex;		//未完成！
// static EFI_LOCK g_HvmLock;
// 
uint32_t g_uSubvertedCPUs = 0;
uint32_t KeNumberProcessors = 1;
PHVM_DEPENDENT Hvm;

static bool HvmIsImplemented(void);

////+++++++++++++++++++++Public Functions++++++++++++++++++++++++
//**
//* effects: Uninstall and remove Zion VM
//*/
//ZVMSTATUS ZVMAPI HvmSpitOutBluepill ()
//{//Finish

//#ifndef ENABLE_HYPERCALLS

	//return ZVM_NOT_SUPPORTED;

//#else

	//int8_t cProcessorNumber;
	//ZVMSTATUS Status, CallbackStatus;

	//cprintf("HelloWorld:HvmSpitOutBluepill(): Going to liberate %d processor%s\n",
	//KeNumberProcessors, KeNumberProcessors == 1 ? "" : "s");

	//// Not find the API in EFI Edk
	////KeWaitForSingleObject (&g_HvmLock, Executive, KernelMode, FALSE, NULL);

	//for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) {

		//cprintf("Zion Hypervisor:HvmSpitOutBluepill(): Liberating processor #%d\n", cProcessorNumber);

		//Status = CmDeliverToProcessor (cProcessorNumber, HvmLiberateCpu, NULL, &CallbackStatus);//<-------------3.1  Finish by zhumin

		//if (!ZVM_SUCCESS (Status)) {
			//cprintf(("HelloWorld:HvmSpitOutBluepill(): CmDeliverToProcessor() failed with status 0x%08hX\n", Status));
		//}

		//if (!ZVM_SUCCESS (CallbackStatus)) {
			//cprintf(("HelloWorld:HvmSpitOutBluepill(): HvmLiberateCpu() failed with status 0x%08hX\n", CallbackStatus));
		//}
	//}
	//return ZVMSUCCESS;
//#endif
//}


static ZVMSTATUS HvmSetupGdt (
  PCPU Cpu
)
{
  uint64_t GuestTssBase;
  uint8_t GuestTssLimit;
  PSEGMENT_DESCRIPTOR GuestTssDescriptor;

  if (!Cpu || !Cpu->GdtArea)
    return ZVM_INVALID_PARAMETER;


	memcpy (Cpu->GdtArea, (void *) GetGdtBase(), GetGdtLimit());

    CmReloadGdtr(Cpu->GdtArea, GetGdtLimit());//加载Cpu->GdtArea中的GDT表
 
    return ZVMSUCCESS;
}

/**
//  * Intialize the CPU struct and start VM by invoking VmxVirtualize()
//  * requires: a valid <GuestRsp>
//  */

 ZVMSTATUS ZVMAPI HvmSubvertCpu (
     void * GuestRsp
 )
 { // UnFinished!!!!! The most important
     PCPU Cpu;//It will be used as the hypervisor struct.
     void * HostKernelStackBase;
     ZVMSTATUS Status;
     ZION_PHYSICAL_ADDRESS HostStackPA;
 
     // allocate memory for host stack, 16 * 4k
     HostKernelStackBase = MmAllocPages(HOST_STACK_SIZE_IN_PAGES, (uint32_t *)&HostStackPA);
	 
	 
     if (!HostKernelStackBase) 
     {
         cprintf("HvmSubvertCpu(): Failed to allocate %d pages for the host stack\n",HOST_STACK_SIZE_IN_PAGES);
         //return STATUS_INSUFFICIENT_RESOURCES;
 		 return -1;
     }
 
     // unchecked -8 or -4 ?
     Cpu = (PCPU) ((char *) HostKernelStackBase + HOST_STACK_SIZE_IN_PAGES * Zion_PageSize - 4 - sizeof (CPU));
     Cpu->HostStack = HostKernelStackBase;
      
     // for interrupt handlers which will address CPU through the FS
	 Cpu->SelfPointer = Cpu;

     Cpu->ProcessorNumber = 0;
	 //Cpu->ProcessorNumber = KeGetCurrentProcessorNumber();
     //Cpu->ProcessorNumber = NumberOfProcessors;
 
     //Cpu->Nested = FALSE;
 

     InitializeListHead (&Cpu->GeneralTrapsList);
     InitializeListHead (&Cpu->MsrTrapsList);
//    // InitializeListHead (&Cpu->IoTrapsList);


     Cpu->GdtArea = (PSEGMENT_DESCRIPTOR)MmAllocPages(BYTES_TO_PAGES(BP_GDT_LIMIT),NULL);//Currently we create our own GDT and IDT area
     if (!Cpu->GdtArea) 
     {
         cprintf(("HvmSubvertCpu(): Failed to allocate memory for GDT\n"));
         //return STATUS_INSUFFICIENT_RESOURCES;
 		 return -1;
     }
// 
     Cpu->IdtArea = (PSEGMENT_DESCRIPTOR)MmAllocPages(BYTES_TO_PAGES(BP_IDT_LIMIT),NULL);
     if (!Cpu->IdtArea) 
     {
         cprintf(("HvmSubvertCpu(): Failed to allocate memory for IDT\n"));
         //return STATUS_INSUFFICIENT_RESOURCES;
		 return -1;
     }
// 	

 	Status = Hvm->ArchRegisterTraps(Cpu);//<----------------3.1 Finish
 	///Status =  g_HvmControl->ApplyTraps(Cpu);
     if (!ZVM_SUCCESS(Status)) 
     {
 		cprintf("Helloworld:HvmSubvertCpu(): Failed to register NewBluePill traps, status 0x%08hX\n");
         //return EFI_LOAD_ERROR;
 		return -1;
     }
// 

     Status = Hvm->ArchInitialize (Cpu, (void *)&CmSlipIntoMatrix, GuestRsp);//<----------------3.2 Finish

// 	Status = Hvm->ArchInitialize (Cpu, (PVOID) (UINTN)CmSlipIntoMatrix, GuestRsp);   // Can't use CmSlipIntoMatrix by PVOID
 	if (!ZVM_SUCCESS (Status)) 
     {
         cprintf("Helloworld:HvmSubvertCpu(): ArchInitialize() failed with status 0x%08hX\n");
         return Status;
     }
     


//     // no API calls allowed below this point: we have overloaded GDTR and selectors
//     // unchecked


      HvmSetupGdt (Cpu);//<----------------3.3 Finish

	  HvmSetupIdt (Cpu);//<----------------3.4 Finish
    
// 
     Status = Hvm->ArchVirtualize(Cpu);//<----------------3.5 Finish

     cprintf("Wrong again...\n");
     return Status;
}


/**
* effects: install our VM root hypervisor on the fly.
*/
ZVMSTATUS ZVMAPI HvmSwallowBluepill()
{
	uint8_t cProcessorNumber;
	ZVMSTATUS Status, CallbackStatus;

	
	for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) 
	{
		///cprintf("Zion Hypervisor:HvmSwallowBluepill():Installing HelloWorld VT Root Manager on processor #%d\n", cProcessorNumber);


			Status = CmDeliverToProcessor (cProcessorNumber, CmSubvert, NULL, &CallbackStatus);
			
			///cprintf("SubvertCpu was done!\n");

	}


	return ZVMSUCCESS;
}

/**
 * Check if this cpu supports Intel VT Technology.
 */
ZVMSTATUS ZVMAPI HvmInit()//Finished
{
	Hvm = &Vmx;
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

ZVMSTATUS ZVMAPI HvmResumeGuest (
)
{
  cprintf ("While come to here, hypervisor was on ^-^\n");

  // irql will be lowered in the CmDeliverToProcessor()
  //CmSti();
  return ZVMSUCCESS;
}

void ZVMAPI HvmEventCallback (
  PCPU Cpu,
  PGUEST_REGS GuestRegs 
)
{
  ZVMSTATUS Status;

  if (!Cpu || !GuestRegs)
  {
	cprintf("error in event callback...\n");
	return;
   }
   

   GuestRegs->esp = VmxRead(GUEST_RSP);
  // it's an original event
  Hvm->ArchDispatchEvent (Cpu, GuestRegs);
  
  //if (Hvm->Architecture == ARCH_VMX)
    //VmxWrite (GUEST_RSP, GuestRegs->esp);
  return;
}




static ZVMSTATUS HvmSetupIdt (
  PCPU Cpu
)
{
  //uint8_t i;

  if (!Cpu || !Cpu->IdtArea)
    return ZVM_INVALID_PARAMETER;

  memcpy (Cpu->IdtArea, (void *) GetIdtBase (),GetIdtLimit());

  CmReloadIdtr (Cpu->IdtArea, GetIdtLimit());

  return ZVMSUCCESS;
}



 
/**
* effects:深入VMM ROOT 模式卸载HelloWorld VM
*/
//static ZVMSTATUS ZVMAPI HvmLiberateCpu (
									  //void * Param
									  //)
//{ // Finish by zhumin
	////EFI_STATUS Status;
	////UINT64 Efer;
	////PCPU Cpu;

	//// called at DPC level

	////if (KeGetCurrentIrql () != DISPATCH_LEVEL)
		//return EFI_LOAD_ERROR;*/

	////Efer = MsrRead (MSR_EFER);

	////Print(("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on entry: 0x%X\n", Efer));
	////RegSetCr3(MADDOG_EXIT_EAX);//<------------------!!!!!!!!卸载用!!!!!!!!!!

	////Efer = MsrRead (MSR_EFER);
	////Print(("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on exit: 0x%X\n", Efer));

	//return ZVMSUCCESS;
//}
//// 
//// // this function is call when guest => host
//// VOID EFIAPI HvmEventCallback (
//// 							 PCPU Cpu,                   // cpu struct
//// 							 PGUEST_REGS GuestRegs       // store guest's regs
//// 							 )
//// {//Finished
//// 	//EFI_STATUS Status;
//// 
//// 	if (!Cpu || !GuestRegs)
//// 		return;
//// 
//// 	GuestRegs->esp = VmxRead (GUEST_RSP);
//// 
//// 	// it's an original event
//// 	Hvm->ArchDispatchEvent (Cpu, GuestRegs);
//// 
//// 	VmxWrite (GUEST_RSP, GuestRegs->esp);
//// 
//// 	return;
//// }
//static ZVMSTATUS ZVMAPI HvmLiberateCpu (
									  //PVOID Param
									  //)
//{ //Finish
	//ZVMSTATUS Status;
	//uint64_t Efer;
	//PCPU Cpu;

	//// called at DPC level

	//if (KeGetCurrentIrql () != DISPATCH_LEVEL)
		//return STATUS_UNSUCCESSFUL;

	//Efer = MsrRead (MSR_EFER);

	//cprintf("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on entry: 0x%X\n", Efer);

	////if (!NT_SUCCESS (Status = HcMakeHypercall (NBP_HYPERCALL_UNLOAD, 0, NULL))) {
	////  _KdPrint (("HvmLiberateCpu(): HcMakeHypercall() failed on processor #%d, status 0x%08hX\n",
	////             KeGetCurrentProcessorNumber (), Status));

	////  return Status;
	////}
	//// for vmx
	////VmxVmCall(0x1);
	//// cause vmexit
	//RegSetCr3(MADDOG_EXIT_EAX);//

	//Efer = MsrRead (MSR_EFER);
	//cprintf("HelloWorld:HvmLiberateCpu(): Reading MSR_EFER on exit: 0x%X\n", Efer);

	//return STATUS_SUCCESS;
//}

