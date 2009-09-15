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


#include <inc/vmx/vmx.h>
#include <inc/vmx/vmxtraps.h>
#include <inc/vmx/hvm.h>
#include <inc/lib/malloc.h>

extern uint32_t HostCr3,guestcr3;

HVM_DEPENDENT Vmx = {
  ARCH_VMX,
  //VmxIsImplemented,
  VmxInitialize,
  VmxVirtualize,
  //VmxShutdown,
  VmxIsNestedEvent,
  VmxDispatchNestedEvent,
  VmxDispatchEvent,
  VmxAdjustRip,
  VmxRegisterTraps,
  VmxIsTrapVaild
};

uint32_t g_HostStackBaseAddress; 


void VmxVmexitHandler()
{		
        PCPU Cpu;
		PGUEST_REGS GuestRegs;
		

	    //HVM_SAVE_ALL_NOSEGREGS
		

	    asm volatile("pushl %edi");
        asm volatile("pushl %esi");
        asm volatile("pushl %ebp");
        asm volatile("pushl %ebp");
        asm volatile("pushl %ebx");
        asm volatile("pushl %edx");
        asm volatile("pushl %ecx");
        asm volatile("pushl %eax");
		
		asm volatile("movl 0x2c(%%esp),%0":"=r"(Cpu));
		asm volatile("movl %%esp,%0":"=r"(GuestRegs));
		
		asm volatile("movl 0x28(%%esp),%0":"=r"(GuestRegs->ebp));
		
		
		
		HvmEventCallback(Cpu,GuestRegs);
		
	
		asm volatile("popl %eax");
		asm volatile("popl %ecx");
		asm volatile("popl %edx");
		asm volatile("popl %ebx");
		asm volatile("popl %ebp");
		asm volatile("popl %ebp");
		asm volatile("popl %esi");
		asm volatile("popl %edi");



        //vmx-resume
		asm volatile(".byte 0x0f, 0x01, 0xC3");
}		


static bool ZVMAPI VmxIsNestedEvent (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
)
{
  return FALSE;                 // DUMMY!!! This build doesn't support nested virtualization!!!
}

static void ZVMAPI VmxAdjustRip (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  uint64_t Delta
)
{
  VmxWrite (GUEST_RIP, VmxRead (GUEST_RIP) + Delta);
  return;
}

static void ZVMAPI VmxDispatchNestedEvent (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
)
{
  ZVMSTATUS Status;
  PNBP_TRAP Trap;
  bool bInterceptedByGuest;
  uint64_t Exitcode;

  if (!Cpu || !GuestRegs)
    return;

  cprintf ("VmxDispatchNestedEvent(): DUMMY!!! This build doesn't support nested virtualization!\n");

}

static void VmxHandleInterception (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  bool WillBeAlsoHandledByGuestHv
)
{
  ZVMSTATUS Status;
  uint64_t Exitcode;
  PNBP_TRAP Trap;
  
  uint32_t test,value;

  if (!Cpu || !GuestRegs)
    return;

  Exitcode = VmxRead (VM_EXIT_REASON);
  
  

  // search for a registered trap for this interception
  Status = TrFindRegisteredTrap (Cpu, GuestRegs, Exitcode, &Trap);
  if (!ZVM_SUCCESS (Status)) {
    cprintf ("VmxHandleInterception(): TrFindRegisteredTrap() failed for exitcode 0x%x\n",Exitcode);
	///while(1);
    return;
  }
  // we found a trap handler
  if (!ZVM_SUCCESS (Status = TrExecuteGeneralTrapHandler (Cpu, GuestRegs, Trap, WillBeAlsoHandledByGuestHv)))
  {
    cprintf  ("VmxHandleInterception(): HvmExecuteGeneralTrapHandler() failed with status 0x%08hX\n");
  }

}

static void ZVMAPI VmxDispatchEvent (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
)
{

  ///cprintf ("VmxDispatchEvent(): exitcode = %x\n",VmxRead (VM_EXIT_REASON));


  VmxHandleInterception (Cpu, GuestRegs, FALSE
                         /* this intercept will not be handled by guest hv */
    );

}

ZVMSTATUS ZVMAPI VmxEnable (
  void* VmxonVA,
  ZION_PHYSICAL_ADDRESS VmxonPA
)
{
  uint64_t cr4;
  uint64_t vmxmsr;
  uint64_t flags;

  set_in_cr4 (X86_CR4_VMXE);
  cr4 = get_cr4 ();
  if (!(cr4 & X86_CR4_VMXE))
  {
	cprintf("ZVM_NOT_SUPPORTED");
    return ZVM_NOT_SUPPORTED;
  }	
  vmxmsr = MsrRead (MSR_IA32_FEATURE_CONTROL);
  if (!(vmxmsr & 4)) {
    cprintf ("VmxEnable(): VMX is not supported: IA32_FEATURE_CONTROL is 0x%llx\n");
    return ZVM_NOT_SUPPORTED;
  }

  vmxmsr = MsrRead (MSR_IA32_VMX_BASIC);
  *((uint64_t *) VmxonVA) = (vmxmsr & 0xffffffff);       //set up vmcs_revision_id
  ///VmxonPA = MmGetPhysicalAddress (VmxonVA);
  //VmxTurnOn (MmGetPhysicalAddress (VmxonVA));
  ///flags = RegGetRflags ();
  VmxTurnOn(VmxonPA);
  ///flags = RegGetRflags ();
  return ZVMSUCCESS;
}

ZVMSTATUS ZVMAPI VmxDisable (
)
{
  uint64_t cr4;
  VmxTurnOff ();
  cr4 = get_cr4 ();
  clear_in_cr4 (X86_CR4_VMXE);
  cr4 = get_cr4 ();
  cprintf("VmxDisable(): CR4 after VmxDisable: 0x%llx\n");
  return ZVMSUCCESS;
}

static uint32_t ZVMAPI VmxAdjustControls (
  uint32_t Ctl,
  uint32_t Msr
)
{
  LARGE_INTEGER MsrValue;

  MsrValue.QuadPart = MsrRead (Msr);

  Ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
  Ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
  return Ctl;
}

ZVMSTATUS ZVMAPI VmxFillGuestSelectorData (
  void* GdtBase,
  uint32_t Segreg,
  uint16_t Selector
)
{
  SEGMENT_SELECTOR SegmentSelector = { 0 };
  uint32_t uAccessRights;

  CmInitializeSegmentSelector (&SegmentSelector, Selector, (uint8_t *)GdtBase);
  uAccessRights = ((uint8_t *) & SegmentSelector.attributes)[0] + (((uint8_t *) & SegmentSelector.attributes)[1] << 12);

  if (!Selector)
    uAccessRights |= 0x10000;

  VmxWrite (GUEST_ES_SELECTOR + Segreg * 2, Selector);
  VmxWrite (GUEST_ES_LIMIT + Segreg * 2, SegmentSelector.limit);
  VmxWrite (GUEST_ES_AR_BYTES + Segreg * 2, uAccessRights);

  if ((Segreg == LDTR) || (Segreg == TR))
    // don't setup for FS/GS - their bases are stored in MSR values
    VmxWrite (GUEST_ES_BASE + Segreg * 2, SegmentSelector.base);

  return ZVMSUCCESS;
}


static bool ZVMAPI VmxIsTrapVaild (
  uint32_t TrappedVmExit
)
{
  if (TrappedVmExit > VMX_MAX_GUEST_VMEXIT)
    return FALSE;
  return TRUE;
}

static ZVMSTATUS VmxSetupVMCS (
  PCPU Cpu,
  void* GuestRip,
  void* GuestRsp
)
{
  SEGMENT_SELECTOR SegmentSelector;
  ZION_PHYSICAL_ADDRESS VmcsToContinuePA;
  ZVMSTATUS Status;
  void* GdtBase;
  uint32_t Interceptions;

  if (!Cpu || !Cpu->Vmx.OriginalVmcs)
  {
	cprintf("!Cpu->Vmx.OriginalVmcs...\n");
    return ZVM_INVALID_PARAMETER;
  }
  VmcsToContinuePA = Cpu->Vmx.VmcsToContinuePA;
  VmxClear (VmcsToContinuePA);
  VmxPtrld (VmcsToContinuePA);

  /*16BIT Fields */

  /*16BIT Host-Statel Fields. */
//#ifdef _X86_
  VmxWrite (HOST_ES_SELECTOR, (RegGetEs () & 0xf8));
  VmxWrite (HOST_CS_SELECTOR, (RegGetCs () & 0xf8));
  VmxWrite (HOST_SS_SELECTOR, (RegGetSs () & 0xf8));
  VmxWrite (HOST_DS_SELECTOR, (RegGetDs () & 0xf8));
//#else
  //VmxWrite (HOST_ES_SELECTOR, BP_GDT64_DATA);
  //VmxWrite (HOST_CS_SELECTOR, BP_GDT64_CODE);
  //VmxWrite (HOST_SS_SELECTOR, BP_GDT64_DATA);
  //VmxWrite (HOST_DS_SELECTOR, BP_GDT64_DATA);
//#endif
  VmxWrite (HOST_FS_SELECTOR, (RegGetFs () & 0xf8));
  VmxWrite (HOST_GS_SELECTOR, (RegGetGs () & 0xf8));
  VmxWrite (HOST_TR_SELECTOR, (GetTrSelector()& 0xf8));

  /*64BIT Control Fields. */
  //VmxWrite (IO_BITMAP_A, Cpu->Vmx.IOBitmapAPA.LowPart);
//#ifdef VMX_ENABLE_PS2_KBD_SNIFFER
  //*(((unsigned char *) (Cpu->Vmx.IOBitmapA)) + (0x60 / 8)) = 0x11;      //0x60 0x64 PS keyboard mouse
//#endif
  //VmxWrite (IO_BITMAP_A_HIGH, Cpu->Vmx.IOBitmapBPA.HighPart);
  //VmxWrite (IO_BITMAP_B, Cpu->Vmx.IOBitmapBPA.LowPart);
  //// FIXME???
  ////*(((unsigned char*)(Cpu->Vmx.IOBitmapB))+((0xc880-0x8000)/8))=0xff;  //0xc880-0xc887  
  //VmxWrite (IO_BITMAP_B_HIGH, Cpu->Vmx.IOBitmapBPA.HighPart);

  VmxWrite (MSR_BITMAP, Cpu->Vmx.MSRBitmapPA.LowPart);
  VmxWrite (MSR_BITMAP_HIGH, Cpu->Vmx.MSRBitmapPA.HighPart);
  //VM_EXIT_MSR_STORE_ADDR          = 0x00002006,  //no init
  //VM_EXIT_MSR_STORE_ADDR_HIGH     = 0x00002007,  //no init
  //VM_EXIT_MSR_LOAD_ADDR           = 0x00002008,  //no init
  //VM_EXIT_MSR_LOAD_ADDR_HIGH      = 0x00002009,  //no init
  //VM_ENTRY_MSR_LOAD_ADDR          = 0x0000200a,  //no init
  //VM_ENTRY_MSR_LOAD_ADDR_HIGH     = 0x0000200b,  //no init
  //VmxWrite (TSC_OFFSET, 0);
  //VmxWrite (TSC_OFFSET_HIGH, 0);
  //VIRTUAL_APIC_PAGE_ADDR          = 0x00002012,   //no init
  //VIRTUAL_APIC_PAGE_ADDR_HIGH     = 0x00002013,   //no init


  /*64BIT Guest-Statel Fields. */
  VmxWrite (VMCS_LINK_POINTER, 0xffffffff);
  VmxWrite (VMCS_LINK_POINTER_HIGH, 0xffffffff);

  VmxWrite (GUEST_IA32_DEBUGCTL, MsrRead (MSR_IA32_DEBUGCTL) & 0xffffffff);
  VmxWrite (GUEST_IA32_DEBUGCTL_HIGH, MsrRead (MSR_IA32_DEBUGCTL) >> 32);

  /*32BIT Control Fields. */
  VmxWrite (PIN_BASED_VM_EXEC_CONTROL, VmxAdjustControls (1, MSR_IA32_VMX_PINBASED_CTLS));      //disable Vmexit by Extern-interrupt,NMI and Virtual NMI 

//#ifdef VMX_ENABLE_MSR_BITMAP
  Interceptions = CPU_BASED_ACTIVATE_MSR_BITMAP;
//#endif

//#ifdef VMX_ENABLE_PS2_KBD_SNIFFER
  //Interceptions |= CPU_BASED_ACTIVATE_IO_BITMAP;
//#endif

//#ifdef INTERCEPT_RDTSCs
  //Interceptions |= CPU_BASED_RDTSC_EXITING;
//#endif
  VmxWrite (CPU_BASED_VM_EXEC_CONTROL, (VmxAdjustControls (Interceptions, MSR_IA32_VMX_PROCBASED_CTLS) | (1<<24)));

//#ifdef INTERCEPT_RDTSCs
  ///VmxWrite (EXCEPTION_BITMAP, 1 << 13);  // intercept #DB
//#endif
  VmxWrite (PAGE_FAULT_ERROR_CODE_MASK, 0);
  VmxWrite (PAGE_FAULT_ERROR_CODE_MATCH, 0);
  VmxWrite (CR3_TARGET_COUNT, 0);

//#ifdef _X86_
  VmxWrite (VM_EXIT_CONTROLS, VmxAdjustControls (VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS));
  VmxWrite (VM_ENTRY_CONTROLS, VmxAdjustControls (0, MSR_IA32_VMX_ENTRY_CTLS));
//#else
  //VmxWrite (VM_EXIT_CONTROLS,
            //VmxAdjustControls (VM_EXIT_IA32E_MODE | VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS));
  //VmxWrite (VM_ENTRY_CONTROLS, VmxAdjustControls (VM_ENTRY_IA32E_MODE, MSR_IA32_VMX_ENTRY_CTLS));
//#endif

  VmxWrite (VM_EXIT_MSR_STORE_COUNT, 0);
  VmxWrite (VM_EXIT_MSR_LOAD_COUNT, 0);

  VmxWrite (VM_ENTRY_MSR_LOAD_COUNT, 0);
  VmxWrite (VM_ENTRY_INTR_INFO_FIELD, 0);

  //VM_ENTRY_EXCEPTION_ERROR_CODE   = 0x00004018,  //no init
  //VM_ENTRY_INSTRUCTION_LEN        = 0x0000401a,  //no init
  //TPR_THRESHOLD                   = 0x0000401c,  //no init

  /*32BIT Read-only Fields:need no setup */



  /*32BIT Guest-Statel Fields. */

  VmxWrite (GUEST_GDTR_LIMIT, GetGdtLimit ());
  VmxWrite (GUEST_IDTR_LIMIT, GetIdtLimit ());

  VmxWrite (GUEST_INTERRUPTIBILITY_INFO, 0);
  VmxWrite (GUEST_ACTIVITY_STATE, 0);   //Active state          
  //GUEST_SM_BASE          = 0x98000,   //no init
  VmxWrite (GUEST_SYSENTER_CS, MsrRead (MSR_IA32_SYSENTER_CS));



  /*32BIT Host-Statel Fields. */

  VmxWrite (HOST_IA32_SYSENTER_CS, MsrRead (MSR_IA32_SYSENTER_CS));     //no use

  /* NATURAL Control State Fields:need not setup. */
  ///VmxWrite (CR0_GUEST_HOST_MASK, X86_CR0_PG);
  VmxWrite(CR0_GUEST_HOST_MASK,0);
  //VmxWrite(CR4_GUEST_HOST_MASK, X86_CR4_VMXE|X86_CR4_PAE|X86_CR4_PSE);//disable vmexit 0f mov to cr4 expect for X86_CR4_VMXE
  VmxWrite(CR4_GUEST_HOST_MASK,0);
  ///VmxWrite (CR4_GUEST_HOST_MASK, X86_CR4_VMXE); //disable vmexit 0f mov to cr4 expect for X86_CR4_VMXE

  VmxWrite(CR0_READ_SHADOW,0);
  ///VmxWrite (CR0_READ_SHADOW, (RegGetCr4 () & X86_CR0_PG) | X86_CR0_PG);

  VmxWrite (CR4_READ_SHADOW, 0);
 
  VmxWrite (CR3_TARGET_VALUE0, 0);      //no use
  VmxWrite (CR3_TARGET_VALUE1, 0);      //no use                        
  VmxWrite (CR3_TARGET_VALUE2, 0);      //no use
  VmxWrite (CR3_TARGET_VALUE3, 0);      //no use

  /* NATURAL Read-only State Fields:need not setup. */

  /* NATURAL GUEST State Fields. */

  VmxWrite (GUEST_CR0, RegGetCr0 ());
  VmxWrite (GUEST_CR3, guestcr3);
  ///VmxWrite (GUEST_CR3, RegGetCr3 ());
  VmxWrite (GUEST_CR4, RegGetCr4 ());

  
  GdtBase = (void*) GetGdtBase ();
  // Setup guest selectors

  VmxFillGuestSelectorData (GdtBase, ES, RegGetEs ());
  VmxFillGuestSelectorData (GdtBase, CS, RegGetCs ());
  VmxFillGuestSelectorData (GdtBase, SS, RegGetSs ());
  VmxFillGuestSelectorData (GdtBase, DS, RegGetDs ());
  VmxFillGuestSelectorData (GdtBase, FS, RegGetFs ());
  VmxFillGuestSelectorData (GdtBase, GS, RegGetGs ());
  VmxFillGuestSelectorData (GdtBase, LDTR, GetLdtr ());
  VmxFillGuestSelectorData (GdtBase, TR, GetTrSelector ());

//#ifdef _X86_
  //CmInitializeSegmentSelector (&SegmentSelector, RegGetEs (), (uint8_t *) GetGdtBase ());
  //VmxWrite (GUEST_ES_BASE, SegmentSelector.base);

  //CmInitializeSegmentSelector (&SegmentSelector, RegGetCs (), (uint8_t *) GetGdtBase ());
  //VmxWrite (GUEST_CS_BASE, SegmentSelector.base);

  //CmInitializeSegmentSelector (&SegmentSelector, RegGetSs (), (uint8_t *) GetGdtBase ());
  //VmxWrite (GUEST_SS_BASE, SegmentSelector.base);

  //CmInitializeSegmentSelector (&SegmentSelector, RegGetDs (), (uint8_t *) GetGdtBase ());
  //VmxWrite (GUEST_DS_BASE, SegmentSelector.base);

  //CmInitializeSegmentSelector (&SegmentSelector, RegGetFs (), (uint8_t *) GetGdtBase ());
  //VmxWrite (GUEST_FS_BASE, SegmentSelector.base);

  //CmInitializeSegmentSelector (&SegmentSelector, RegGetGs (), (uint8_t *) GetGdtBase ());
  //VmxWrite (GUEST_GS_BASE, SegmentSelector.base);
//#else
  //VmxWrite (GUEST_ES_BASE, 0);
  //VmxWrite (GUEST_CS_BASE, 0);
  //VmxWrite (GUEST_SS_BASE, 0);
  //VmxWrite (GUEST_DS_BASE, 0);
  //VmxWrite (GUEST_FS_BASE, MsrRead (MSR_FS_BASE));
  //VmxWrite (GUEST_GS_BASE, MsrRead (MSR_GS_BASE));
//#endif

  // LDTR/TR bases have been set in VmxFillGuestSelectorData()
  VmxWrite (GUEST_GDTR_BASE, (uint64_t) GdtBase);
  VmxWrite (GUEST_IDTR_BASE, GetIdtBase ());

  VmxWrite (GUEST_DR7, 0x400);
  VmxWrite (GUEST_RSP, (uint64_t) GuestRsp);     //setup guest sp
  VmxWrite (GUEST_RIP, (uint64_t) GuestRip);     //setup guest ip:CmSlipIntoMatrix
  VmxWrite (GUEST_RFLAGS, RegGetRflags ());
  //VmxWrite(GUEST_PENDING_DBG_EXCEPTIONS, 0);//no init
  VmxWrite (GUEST_SYSENTER_ESP, (uint32_t)MsrRead (MSR_IA32_SYSENTER_ESP));
  VmxWrite (GUEST_SYSENTER_EIP, (uint32_t)MsrRead (MSR_IA32_SYSENTER_EIP));


  /* HOST State Fields. */
  VmxWrite (HOST_CR0, RegGetCr0 ());

//#ifdef VMX_USE_PRIVATE_CR3
  //// private cr3
  //VmxWrite (HOST_CR3, g_PageMapBasePhysicalAddress.QuadPart);
//#else
  VmxWrite(HOST_CR3,HostCr3);
  ///VmxWrite (HOST_CR3, RegGetCr3 ());
//#endif
  VmxWrite (HOST_CR4, RegGetCr4 ());


    CmInitializeSegmentSelector (&SegmentSelector, RegGetFs (), (uint8_t *) GetGdtBase ());
    VmxWrite (HOST_FS_BASE, SegmentSelector.base);
	
	CmInitializeSegmentSelector (&SegmentSelector, RegGetGs (), (uint8_t *) GetGdtBase ());
    VmxWrite (HOST_GS_BASE, SegmentSelector.base);
  ///VmxWrite (HOST_FS_BASE, MsrRead (MSR_FS_BASE)); for MSR_FS_BASE and MSR_GS_BASE is x86-64's
  ///VmxWrite (HOST_GS_BASE, MsrRead (MSR_GS_BASE));

  // TODO: we must setup our own TSS
  // FIXME???

  CmInitializeSegmentSelector (&SegmentSelector, GetTrSelector (), (uint8_t *) GetGdtBase ());
  VmxWrite (HOST_TR_BASE, SegmentSelector.base);

  ///VmxWrite (HOST_GDTR_BASE, (uint64_t) Cpu->GdtArea);
  ///VmxWrite (HOST_IDTR_BASE, (uint64_t) Cpu->IdtArea);

  // FIXME???
     VmxWrite(HOST_GDTR_BASE, (uint32_t)GetGdtBase());
	 VmxWrite(HOST_IDTR_BASE, (uint32_t)GetIdtBase());

  VmxWrite (HOST_IA32_SYSENTER_ESP, MsrRead (MSR_IA32_SYSENTER_ESP));
  VmxWrite (HOST_IA32_SYSENTER_EIP, MsrRead (MSR_IA32_SYSENTER_EIP));

//#ifdef _X86_
  VmxWrite (HOST_RSP, g_HostStackBaseAddress + 0x0C00); //setup host sp at vmxLaunch(...)
//#else
  //VmxWrite (HOST_RSP, (uint64_t) Cpu);   //setup host sp at vmxLaunch(...)
//#endif
  VmxWrite (HOST_RIP, (uint32_t) VmxVmexitHandler);      //setup host ip:CmSlipIntoMatrix


  return ZVMSUCCESS;
}


static ZVMSTATUS ZVMAPI VmxInitialize (
  PCPU Cpu,
  void* GuestRip,
  void* GuestRsp
)
{
  ZION_PHYSICAL_ADDRESS AlignedVmcsPA;
  uint64_t VaDelta;
  ZVMSTATUS Status;

#ifndef _X86_
  void *tmp,*tmp2,*tmp3;
  //tmp = MmAllocateContiguousPages (1, NULL);
  ///tmp2 = MmAllocPages(3,NULL);
  tmp = MmAllocPages(1,NULL);
  ///tmp3 = MmAllocPages(3,NULL);
  g_HostStackBaseAddress = (uint64_t) tmp;
#endif
  // do not deallocate anything here; MmShutdownManager will take care of that

  //Allocate VMXON region
  //Cpu->Vmx.OriginaVmxonR = MmAllocateContiguousPages (VMX_VMXONR_SIZE_IN_PAGES, &Cpu->Vmx.OriginalVmxonRPA);
  Cpu->Vmx.OriginaVmxonR = MmAllocPages(VMX_VMXONR_SIZE_IN_PAGES, (uint32_t *)&Cpu->Vmx.OriginalVmxonRPA);
  if (!Cpu->Vmx.OriginaVmxonR) {
    cprintf ("VmxInitialize(): Failed to allocate memory for original VMCS\n");
    //return STATUS_INSUFFICIENT_RESOURCES;
	return -1;
  }

  //cprintf ("VmxInitialize(): OriginaVmxonR VA: 0x%p\n", Cpu->Vmx.OriginaVmxonR);
  //cprintf ("VmxInitialize(): OriginaVmxonR PA: 0x%llx\n", Cpu->Vmx.OriginalVmxonRPA);

  //Allocate VMCS
  //Cpu->Vmx.OriginalVmcs = MmAllocateContiguousPages (VMX_VMCS_SIZE_IN_PAGES, &Cpu->Vmx.OriginalVmcsPA);
  Cpu->Vmx.OriginalVmcs = MmAllocPages(VMX_VMCS_SIZE_IN_PAGES, (uint32_t *)&Cpu->Vmx.OriginalVmcsPA);

  if (!Cpu->Vmx.OriginalVmcs) {
    cprintf ("VmxInitialize(): Failed to allocate memory for original VMCS\n");
    //return STATUS_INSUFFICIENT_RESOURCES;
	return -1;
  }

  //cprintf ("VmxInitialize(): Vmcs VA: 0x%x\n", Cpu->Vmx.OriginalVmcs);
  //cprintf ("VmxInitialize(): Vmcs PA: 0x%llx\n", Cpu->Vmx.OriginalVmcsPA);

  // these two PAs are equal if there're no nested VMs
  Cpu->Vmx.VmcsToContinuePA = Cpu->Vmx.OriginalVmcsPA;

  //init IOBitmap and MsrBitmap
  //Cpu->Vmx.IOBitmapA = MmAllocateContiguousPages (VMX_IOBitmap_SIZE_IN_PAGES, &Cpu->Vmx.IOBitmapAPA);
  Cpu->Vmx.IOBitmapA = MmAllocPages (VMX_IOBitmap_SIZE_IN_PAGES, (uint32_t *)&Cpu->Vmx.IOBitmapAPA);
  if (!Cpu->Vmx.IOBitmapA) {
    cprintf (("VmxInitialize(): Failed to allocate memory for IOBitmapA\n"));
    //return STATUS_INSUFFICIENT_RESOURCES;
	return -1;
  }
  //RtlZeroMemory (Cpu->Vmx.IOBitmapA, PAGE_SIZE);

  //cprintf ("VmxInitialize(): IOBitmapA VA: 0x%x\n", Cpu->Vmx.IOBitmapA);
  //cprintf ("VmxInitialize(): IOBitmapA PA: 0x%x\n", Cpu->Vmx.IOBitmapAPA);

  //Cpu->Vmx.IOBitmapB = MmAllocateContiguousPages (VMX_IOBitmap_SIZE_IN_PAGES, &Cpu->Vmx.IOBitmapBPA);
  Cpu->Vmx.IOBitmapB = MmAllocPages (VMX_IOBitmap_SIZE_IN_PAGES, (uint32_t *)&Cpu->Vmx.IOBitmapBPA);
  if (!Cpu->Vmx.IOBitmapB) {
    cprintf ("VmxInitialize(): Failed to allocate memory for IOBitmapB\n");
    //return STATUS_INSUFFICIENT_RESOURCES;
	return -1;
  }
  //RtlZeroMemory (Cpu->Vmx.IOBitmapB, PAGE_SIZE);

  //cprintf ("VmxInitialize(): IOBitmapB VA: 0x%x\n", Cpu->Vmx.IOBitmapB);
  //cprintf ("VmxInitialize(): IOBitmapB PA: 0x%x\n", Cpu->Vmx.IOBitmapBPA);

  //Cpu->Vmx.MSRBitmap = MmAllocateContiguousPages (VMX_MSRBitmap_SIZE_IN_PAGES, &Cpu->Vmx.MSRBitmapPA);
  Cpu->Vmx.MSRBitmap = MmAllocPages(VMX_MSRBitmap_SIZE_IN_PAGES, (uint32_t *)&Cpu->Vmx.MSRBitmapPA);
  
  if (!Cpu->Vmx.MSRBitmap) {
    cprintf ("VmxInitialize(): Failed to allocate memory for  MSRBitmap\n");
    //return STATUS_INSUFFICIENT_RESOURCES;
	return -1;
  }
  //RtlZeroMemory (Cpu->Vmx.MSRBitmap, PAGE_SIZE);
  //cprintf ("VmxInitialize(): MSRBitmap VA: 0x%x\n", Cpu->Vmx.MSRBitmap);
  //cprintf ("VmxInitialize(): MSRBitmap PA: 0x%x\n", Cpu->Vmx.MSRBitmapPA);

  if (!ZVM_SUCCESS (VmxEnable (Cpu->Vmx.OriginaVmxonR,Cpu->Vmx.OriginalVmxonRPA))) {
    cprintf ("VmxInitialize(): Failed to enable Vmx\n");
    return ZVM_UNSUCCESSFUL;
  }
   
  *((uint64_t *) (Cpu->Vmx.OriginalVmcs)) = (MsrRead (MSR_IA32_VMX_BASIC) & 0xffffffff); //set up vmcs_revision_id      
  
  if (!ZVM_SUCCESS (Status = VmxSetupVMCS (Cpu, GuestRip, GuestRsp))) {
    cprintf ("Vmx(): VmxSetupVMCS() failed with status 0x%08hX\n");
    VmxDisable ();
    return Status;
  }

  ///cprintf (("VmxInitialize(): Vmx enabled\n"));

  ///Cpu->Vmx.GuestEFER = MsrRead (MSR_EFER); MSR_EFER is x86-64's
  ///cprintf ("Guest MSR_EFER Read 0x%llx \n");

  Cpu->Vmx.GuestCR0 = RegGetCr0 ();
  Cpu->Vmx.GuestCR3 = RegGetCr3 ();
  Cpu->Vmx.GuestCR4 = RegGetCr4 ();

//#ifdef INTERCEPT_RDTSCs
  //Cpu->Tracing = 0;
//#endif
//#ifdef BLUE_CHICKEN
  //Cpu->ChickenQueueSize = 0;
  //Cpu->ChickenQueueHead = Cpu->ChickenQueueTail = 0;
//#endif
  //CmCli ();
  return ZVMSUCCESS;
}

static ZVMSTATUS ZVMAPI VmxVirtualize (
  PCPU Cpu
)
{
  uint64_t rsp;
  if (!Cpu)//ÑéÖ¤²ÎÊý
    return ZVM_INVALID_PARAMETER;

  ///cprintf ("VmxVirtualize(): VmxRead: 0x%x \n",VmxRead (VM_INSTRUCTION_ERROR));//VM_INSTRUCTION_ERROR=0x00004400
  ///cprintf ("VmxVirtualize(): RFlags before vmxLaunch: 0x%x \n", RegGetRflags ());
  cprintf ("VmxVirtualize(): PCPU: 0x%p \n", Cpu);
  ///rsp = RegGetRsp ();
  ///cprintf ("VmxVirtualize(): Rsp: 0x%x \n", rsp);

  ///cprintf ("GuestIp is 0x%x\n", VmxRead(GUEST_RIP));
  ///cprintf ("GuestSp is 0x%x\n", VmxRead(GUEST_RSP));

  *((uint32_t *) (g_HostStackBaseAddress + 0x0C00)) = (uint32_t) Cpu;
  
  ///cprintf("host rsp is 0x%x\n",g_HostStackBaseAddress + 0x0C00);

  ///cprintf("Launching...\n");
  VmxLaunch ();
  
  cprintf("If this sentence happened, it's wrong...\n");

  // never returns

  return ZVM_UNSUCCESSFUL;
}

