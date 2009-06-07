#include "Handlers.h"
extern PParameter AppParameter[2];
extern PBOOLEAN pRegState; //Hypervisor side software registration state.
NTSTATUS SNProtectorInitBeforeVMXON(
)
{
	ULONG i;
	
	pRegState = HvMmAllocatePages(BYTES_TO_PAGES(sizeof(Parameter)),NULL,LAB_TAG,NULL);//Waste much space here....

	for(i =0 ;i<2;i++)
	{
		AppParameter[i] = HvMmAllocatePages(BYTES_TO_PAGES(sizeof(Parameter)),NULL,LAB_TAG,NULL);
	}
	return STATUS_SUCCESS;
}
NTSTATUS HvmSetupVMControlBlock (
    PCPU Cpu,
    PVOID GuestEip,
    PVOID GuestEsp
)
{
	SEGMENT_SELECTOR SegmentSelector;
	PVOID GdtBase;
    ULONG32 Interceptions;
	HVSTATUS Status;

	/*16BIT Fields */

    /*16BIT Host-Statel Fields. */
    VmxWrite (HOST_ES_SELECTOR, RegGetEs () & 0xf8);
    VmxWrite (HOST_CS_SELECTOR, RegGetCs () & 0xf8);
    VmxWrite (HOST_SS_SELECTOR, RegGetSs () & 0xf8);
    VmxWrite (HOST_DS_SELECTOR, RegGetDs () & 0xf8);

    VmxWrite (HOST_FS_SELECTOR, (RegGetFs () & 0xf8));
    VmxWrite (HOST_GS_SELECTOR, (RegGetGs () & 0xf8));
    VmxWrite (HOST_TR_SELECTOR, (GetTrSelector () & 0xf8));

    ///*64BIT Control Fields. */
    //VmxWrite (IO_BITMAP_A, Cpu->Vmx.IOBitmapAPA.LowPart);
    //VmxWrite (IO_BITMAP_A_HIGH, Cpu->Vmx.IOBitmapBPA.HighPart);
    //VmxWrite (IO_BITMAP_B, Cpu->Vmx.IOBitmapBPA.LowPart);

    // FIXME???
    //*(((unsigned char*)(Cpu->Vmx.IOBitmapB))+((0xc880-0x8000)/8))=0xff;  //0xc880-0xc887  

    //VmxWrite (IO_BITMAP_B_HIGH, Cpu->Vmx.IOBitmapBPA.HighPart);

    VmxWrite (MSR_BITMAP, Cpu->Vmx.MSRBitmapPA.LowPart);
    VmxWrite (MSR_BITMAP_HIGH, Cpu->Vmx.MSRBitmapPA.HighPart);

    //VM_EXIT_MSR_STORE_ADDR          = 0x00002006,  //no init
    //VM_EXIT_MSR_STORE_ADDR_HIGH     = 0x00002007,  //no init
    //VM_EXIT_MSR_LOAD_ADDR           = 0x00002008,  //no init
    //VM_EXIT_MSR_LOAD_ADDR_HIGH      = 0x00002009,  //no init
    //VM_ENTRY_MSR_LOAD_ADDR          = 0x0000200a,  //no init
    //VM_ENTRY_MSR_LOAD_ADDR_HIGH     = 0x0000200b,  //no init

   /* VmxWrite (TSC_OFFSET, 0);
    VmxWrite (TSC_OFFSET_HIGH, 0);*/

    //VIRTUAL_APIC_PAGE_ADDR          = 0x00002012,   //no init
    //VIRTUAL_APIC_PAGE_ADDR_HIGH     = 0x00002013,   //no init

    /*64BIT Guest-State Fields. */
    VmxWrite (VMCS_LINK_POINTER, 0xffffffff);
    VmxWrite (VMCS_LINK_POINTER_HIGH, 0xffffffff);

    VmxWrite (GUEST_IA32_DEBUGCTL, MsrRead (MSR_IA32_DEBUGCTL) & 0xffffffff);
    VmxWrite (GUEST_IA32_DEBUGCTL_HIGH, MsrRead (MSR_IA32_DEBUGCTL) >> 32);

    /*32BIT Control Fields. */
    //disable Vmexit by Extern-interrupt,NMI and Virtual NMI
    // Pin-based VM-execution controls
    VmxWrite (PIN_BASED_VM_EXEC_CONTROL, PtVmxAdjustControls (0, MSR_IA32_VMX_PINBASED_CTLS));//<------------------5.1 Finished

    Interceptions = CPU_BASED_ACTIVATE_MSR_BITMAP | CPU_BASED_MOV_DR_EXITING;
    // Primary processor-based VM-execution controls
    VmxWrite (
      CPU_BASED_VM_EXEC_CONTROL, 
      PtVmxAdjustControls (Interceptions, MSR_IA32_VMX_PROCBASED_CTLS) );

    VmxWrite (EXCEPTION_BITMAP, (ULONG)1 << 14);

    //VmxWrite (PAGE_FAULT_ERROR_CODE_MASK, 2);   // W/R
    //VmxWrite (PAGE_FAULT_ERROR_CODE_MATCH, 2);  // write cause the fault
    VmxWrite (PAGE_FAULT_ERROR_CODE_MASK, 0);
    VmxWrite (PAGE_FAULT_ERROR_CODE_MATCH, 0xFFFFFFFF);

    // VM-exit controls
    // bit 15, Acknowledge interrupt on exit
    VmxWrite (VM_EXIT_CONTROLS, 
        PtVmxAdjustControls (VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS));
    // VM-entry controls
    VmxWrite (VM_ENTRY_CONTROLS, 
        PtVmxAdjustControls (0, MSR_IA32_VMX_ENTRY_CTLS));

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
    // CR0 guest/host mask
    //VmxWrite (CR0_GUEST_HOST_MASK, X86_CR0_PG);   //X86_CR0_WP
    VmxWrite (CR0_GUEST_HOST_MASK, 0);
    // CR0 read shadow
    //VmxWrite (CR0_READ_SHADOW, (RegGetCr4 () & X86_CR0_PG) | X86_CR0_PG);
    // if PG is clear, a vmexit will be caused
    VmxWrite (CR0_READ_SHADOW, 0);

    //VmxWrite(CR4_GUEST_HOST_MASK, X86_CR4_VMXE|X86_CR4_PAE|X86_CR4_PSE);
    // disable vmexit 0f mov to cr4 expect for X86_CR4_VMXE
    VmxWrite (CR4_GUEST_HOST_MASK, 0); 
    VmxWrite (CR4_READ_SHADOW, 0);

	Status = PtVmxCR3AccessInterception(
		Cpu,
		0,
		NULL,
		FALSE,
		PtVmxDispatchCR3Access
		);

    /* NATURAL Read-only State Fields:need not setup. */

    /* NATURAL GUEST State Fields. */

    VmxWrite (GUEST_CR0, RegGetCr0 ());
    VmxWrite (GUEST_CR3, HvMmGetOriginGuestCR3());
    VmxWrite (GUEST_CR4, RegGetCr4 ());

    GdtBase = (PVOID) GetGdtBase ();

    // Setup guest selectors
    VmxFillGuestSelectorData (GdtBase, ES, RegGetEs ());//<----------------------5.2 Finished
    VmxFillGuestSelectorData (GdtBase, CS, RegGetCs ());
    VmxFillGuestSelectorData (GdtBase, SS, RegGetSs ());
    VmxFillGuestSelectorData (GdtBase, DS, RegGetDs ());
    VmxFillGuestSelectorData (GdtBase, FS, RegGetFs ());
    VmxFillGuestSelectorData (GdtBase, GS, RegGetGs ());
    VmxFillGuestSelectorData (GdtBase, LDTR, GetLdtr ());
    VmxFillGuestSelectorData (GdtBase, TR, GetTrSelector ());

    // LDTR/TR bases have been set in VmxFillGuestSelectorData()
    VmxWrite (GUEST_GDTR_BASE, (ULONG) GdtBase);
    VmxWrite (GUEST_IDTR_BASE, GetIdtBase ());

    VmxWrite (GUEST_DR7, 0x400);
    VmxWrite (GUEST_RSP, (ULONG) GuestEsp);     //setup guest sp
    VmxWrite (GUEST_RIP, (ULONG) GuestEip);     //setup guest ip:CmSlipIntoMatrix
    VmxWrite (GUEST_RFLAGS, RegGetRflags ());
    //VmxWrite(GUEST_PENDING_DBG_EXCEPTIONS, 0);//no init
    VmxWrite (GUEST_SYSENTER_ESP, (ULONG)MsrRead (MSR_IA32_SYSENTER_ESP));
    VmxWrite (GUEST_SYSENTER_EIP, (ULONG)MsrRead (MSR_IA32_SYSENTER_EIP));

    /* HOST State Fields. */
    VmxWrite (HOST_CR0, RegGetCr0 ());

    // HOST cr3
    VmxWrite (HOST_CR3, HvMmGetHostCR3());
    VmxWrite (HOST_CR4, RegGetCr4 ());

    // unchecked
    //VmxWrite (HOST_FS_BASE, MsrRead (MSR_FS_BASE));
    //VmxWrite (HOST_GS_BASE, MsrRead (MSR_GS_BASE));
	//注意这里只处理FS和GS两个段寄存器。
    MadDog_InitializeSegmentSelector (&SegmentSelector, RegGetFs (), (PVOID) GetGdtBase ());//<----------------------5.3 Finish
    VmxWrite (HOST_FS_BASE, SegmentSelector.base);

    MadDog_InitializeSegmentSelector (&SegmentSelector, RegGetGs (), (PVOID) GetGdtBase ());
    VmxWrite (HOST_GS_BASE, SegmentSelector.base);

    // TODO: we must setup our own TSS
    // FIXME???

    MadDog_InitializeSegmentSelector (&SegmentSelector, GetTrSelector (), (PVOID) GetGdtBase ());
    VmxWrite (HOST_TR_BASE, SegmentSelector.base);

    // unchecked
    //VmxWrite (HOST_GDTR_BASE, (ULONG64) Cpu->GdtArea);
    //VmxWrite (HOST_IDTR_BASE, (ULONG64) Cpu->IdtArea);

    // FIXME???
    VmxWrite(HOST_GDTR_BASE, GetGdtBase());
    VmxWrite(HOST_IDTR_BASE, GetIdtBase());

    VmxWrite (HOST_IA32_SYSENTER_ESP, (ULONG)MsrRead (MSR_IA32_SYSENTER_ESP));
    VmxWrite (HOST_IA32_SYSENTER_EIP, (ULONG)MsrRead (MSR_IA32_SYSENTER_EIP));
	
	//Apply VMXTimer Service
	//Status = PtVmxSetTimerInterval(
	//	Cpu,
	//	2^16, //ticks
	//	TRUE, //Save the rest time-slice on #VMEXIT
	//	VmxDispatchTimerExpired	
	//);
	//return HVSTATUS_NTSTATUS(Status);
	return STATUS_SUCCESS;
}


/**
 * effects: 用于填充VMCB中Guest状态描述中的段选择器部分
 */
static NTSTATUS NTAPI VmxFillGuestSelectorData (
    PVOID GdtBase,
    ULONG Segreg,//SEGREGS枚举中的段选择符号，用于描述要Fill哪个段选择器
    USHORT Selector
)
{//Finished
    SEGMENT_SELECTOR SegmentSelector = {0};
    ULONG uAccessRights;
    MadDog_InitializeSegmentSelector (&SegmentSelector, Selector, GdtBase);//<--------------------6.1 Finished
    uAccessRights = 
        ((PUCHAR)&SegmentSelector.attributes)[0] + (((PUCHAR)&SegmentSelector.attributes)[1] << 12);

    if (!Selector)
        uAccessRights |= 0x10000;

    VmxWrite (GUEST_ES_SELECTOR + Segreg * 2, Selector);
    VmxWrite (GUEST_ES_LIMIT + Segreg * 2, SegmentSelector.limit);
    VmxWrite (GUEST_ES_AR_BYTES + Segreg * 2, uAccessRights);

    //if ((Segreg == LDTR) || (Segreg == TR))
    {
        // don't setup for FS/GS - their bases are stored in MSR values
        // for x64?
        VmxWrite (GUEST_ES_BASE + Segreg * 2, SegmentSelector.base);
    }

    return STATUS_SUCCESS;
}