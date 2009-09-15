#include <inc/vmx/common.h>
#include <inc/vmx/hvm.h>


extern ZVMSTATUS HvmSubvertCpu(void * GuestRsp);
//+++++++++++++++Public Function+++++++++++++++++++++++++++++++++
/**
 * effects:To see if the indicated bit is set or not.
 * requires: 0<=bitNo<=63
 */
bool ZVMAPI CmIsBitSet (
  uint32_t v,
  uint8_t bitNo
)
{//Finished
  uint32_t mask = (uint32_t) 1 << bitNo;
  return (bool) ((v & mask) != 0);
}

/**
* effects:Raise the interruption level to dispatch level, then
* install VM Root hypervisor by call <CallbackProc>
*/
ZVMSTATUS ZVMAPI CmDeliverToProcessor (
										int8_t cProcessorNumber,
										PCALLBACK_PROC CallbackProc,
										void* CallbackParam,
										PZVMSTATUS pCallbackStatus
										)
{ //Finish
	ZVMSTATUS CallbackStatus;
	//KIRQL OldIrql;
	//EFI_TPL OldTpl;

	if (!CallbackProc)
		return ZVM_INVALID_PARAMETER;

	if (pCallbackStatus)
		*pCallbackStatus = ZVM_UNSUCCESSFUL;

	// AffinityThread not finished by zhumin
	//KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));

	//OldIrql = KeRaiseIrqlToDpcLevel ();
	//OldTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);
	CallbackStatus = CallbackProc (CallbackParam);
	

	//KeLowerIrql (OldIrql);
	//gBS->RestoreTPL(OldTpl);

	// AffinityThread not finished by zhumin
	//KeRevertToUserAffinityThread ();

	// save the status of the callback which has run on the current core
	if (pCallbackStatus)
		*pCallbackStatus = CallbackStatus;

	return ZVMSUCCESS;
}

ZVMSTATUS ZVMAPI CmInitializeSegmentSelector (
  SEGMENT_SELECTOR * SegmentSelector,
  uint16_t Selector,
  uint8_t * GdtBase
)
{
  PSEGMENT_DESCRIPTOR SegDesc;

  if (!SegmentSelector)
    return ZVM_INVALID_PARAMETER;

  if (Selector & 0x4) 
  {
    cprintf ("CmInitializeSegmentSelector(): Given selector (0x%X) points to LDT\n", Selector);
    return ZVM_INVALID_PARAMETER;
  }

  SegDesc = (PSEGMENT_DESCRIPTOR) ((uint8_t *) GdtBase + (Selector & ~0x7));

  SegmentSelector->sel = Selector;
  SegmentSelector->base = SegDesc->base0 | SegDesc->base1 << 16 | SegDesc->base2 << 24;
  SegmentSelector->limit = SegDesc->limit0 | (SegDesc->limit1attr1 & 0xf) << 16;
  SegmentSelector->attributes.UCHARs = SegDesc->attr0 | (SegDesc->limit1attr1 & 0xf0) << 4;

  //For Framework's Reason...
  //if (!(SegDesc->attr0 & LA_STANDARD)) {
    //uint64_t tmp;
    //// this is a TSS or callgate etc, save the base high part
    //tmp = (*(uint64_t *) ((uint8_t *) SegDesc + 8));
    //SegmentSelector->base = (SegmentSelector->base & 0xffffffff) | (tmp << 32);
  //}

  if (SegmentSelector->attributes.fields.g) {
    // 4096-bit granularity is enabled for this segment, scale the limit
    SegmentSelector->limit = (SegmentSelector->limit << 12) + 0xfff;
  }

  return ZVMSUCCESS;
}


//+++++++++++++++++++++++asm volatile Function++++++++++++++++++++++++++++++++
/**
//effect: call HvmSubvertCpu to setup VM
**/
ZVMSTATUS ZVMAPI CmSubvert (void *
)
{
	uint32_t esp =0 ;
	
	//CM_SAVE_ALL_NOSEGREGS
	asm volatile("pushl %edi");
	asm volatile("pushl %esi");
	asm volatile("pushl %ebp");
	asm volatile("pushl %ebp");
	asm volatile("pushl %ebx");
	asm volatile("pushl %edx");
	asm volatile("pushl %ecx");
	asm volatile("pushl %eax");
	
	//send args for HvmsubvertCpu
	asm volatile("movl %%esp,%0":"=r"(esp));
	
	HvmSubvertCpu((void *)esp);
	
	return ZVMSUCCESS;
}


void CmSlipIntoMatrix()
{
	asm volatile("popl %ebp");
		
	asm volatile("popl %eax");
	asm volatile("popl %ecx");
	asm volatile("popl %edx");
	asm volatile("popl %ebx");
	asm volatile("popl %ebp");
	asm volatile("popl %ebp");
	asm volatile("popl %esi");
	asm volatile("popl %edi");

    //cprintf("pop done!\n");

	asm volatile("addl $8,%esp");
}

void CmReloadGdtr(PSEGMENT_DESCRIPTOR gdtbase, uint8_t gdtlimit)
{
	unsigned char gdtr[6];
	char *gdtr_addr = (char *)&gdtr[0];
	cprintf("gdtr_addr = 0x%x\n", gdtr_addr);
	*(uint32_t *)&gdtr[2] = (uint32_t)gdtbase;
	*(uint16_t *)&gdtr[0] = gdtlimit;
	//asm volatile("ldgt %0"::"m"(gdtr));
	asm volatile("lgdt %0"::"m"(gdtr_addr));
}

void CmReloadIdtr(void* idtbase, uint8_t idtlimit)
{
	unsigned char idtr[6];
	char *idtr_addr = (char *)&idtr[0];
	cprintf("idtr_addr = 0x%x\n", idtr_addr);
	*(uint32_t *)&idtr[2] = (uint32_t)idtbase;
	*(uint16_t *)&idtr[0] = idtlimit;
	//asm volatile("lidt %0"::"m"(idtr));
	asm volatile("lidt %0"::"m"(idtr_addr));
}

/**
 * VmxRead ...
 **/
 uint32_t VmxRead(uint32_t field)
 {
	uint32_t ecx;

    asm volatile(".byte   0x0f, 0x78\n"
        ".byte   0xc1\n"
        : "=c" (ecx)
        : "a" (field)
        : "memory");
    return ecx;
 } 
 /**
  * VmxWrite for compile...
  **/ 
void VmxWrite(uint32_t field, uint32_t value)
{
	asm volatile(".byte	0x0f, 0x79\n"
	    ".byte	0xC1\n"
		:
		:"a"(field),"c"(value)
		:"memory");
}
/**
 * MsrRead for compile... this one is wrong ,should be modified
 **/
uint64_t MsrRead(uint64_t reg)
{
	uint64_t value = 0;
	uint32_t eax,edx;
	asm volatile("movl %0,%%ecx"::"r"(reg));
	asm volatile("rdmsr");
	asm volatile("movl %%edx,%0":"=r"(edx));
	asm volatile("movl %%eax,%0":"=r"(eax));
	value = edx;
	value = (value << 32) | eax;
	return value;
}

/**
 * MsrWrite for compile...
 **/ 
void MsrWrite(uint32_t field, uint32_t value)
{
	asm volatile("movl %0,%%eax"::"r"(value));
	asm volatile("movl %0,%%ecx"::"r"(field));
	asm volatile("wrmsr");
	///asm volatile("ret");
	
}

/**
 * GetCpuIdInfo for compile...
 **/
void GetCpuIdInfo(uint32_t fn,uint32_t* ret_eax,uint32_t* ret_ebx,uint32_t* ret_ecx,uint32_t* ret_edx)
 {
	 asm volatile("pushl %eax");
	 asm volatile("pushl %ebx");
	 asm volatile("pushl %ecx");
	 asm volatile("pushl %edx");
	 
	 asm volatile("movl %0,%%ecx"::"r"(ret_ecx));
	 asm volatile("movl (%eax),%ecx");
	 
	 asm volatile("movl %0,%%eax"::"r"(fn));
	 
	 asm volatile("cpuid");
	 
	 asm volatile("movl %0,%%esi"::"r"(ret_eax));
	 asm volatile("movl (%esi),%eax");
 } 
 
 
 void set_in_cr4(uint32_t mask)
 {
	 asm volatile("movl %0,%%eax"::"r"(mask));
	 asm volatile("movl %cr4,%ecx");
	 asm volatile("orl %eax,%ecx");
	 asm volatile("movl %ecx,%cr4"); 
 }
 
 void clear_in_cr4(uint32_t mask)
 {
	 asm volatile("movl %0,%%eax"::"r"(mask));
	 asm volatile("movl %cr4,%ecx");
	 asm volatile("not %eax");
	 asm volatile("andl %eax,%ecx");
	 asm volatile("movl %ecx,%cr4");
 }
 
 uint64_t get_cr4()
 {
	 uint64_t cr4;
	 asm volatile("movl %%cr4,%0":"=r"(cr4));
	 return cr4;
 }
 
 void VmxTurnOn(ZION_PHYSICAL_ADDRESS addr)
 {
	asm volatile("movl $8,%eax");
	asm volatile("addl %ebp,%eax");
	asm volatile(".byte 0xf3, 0x0f, 0xC7");
	asm volatile(".byte 0x30");
 }
 
 
 void VmxTurnOff()
 {
	 asm volatile(".byte	0x0f, 0x01, 0xC4");
 }
 
 void VmxClear(ZION_PHYSICAL_ADDRESS addr)
 {
	 asm  volatile(".byte	0x66, 0x0f, 0xC7\n"
	              ".byte 0x30\n"
				  :
				  :"a"(&addr)
				  :"memory");
 }
 
 void VmxPtrld(ZION_PHYSICAL_ADDRESS addr)
 {
	 asm  volatile(".byte	0x0f, 0xC7\n"
				  ".byte 0x30\n"
				  :
				  :"a"(&addr)
				  :"memory");
 }
 
 void VmxLaunch()
 {
	 asm volatile(".byte	0x0f, 0x01, 0xC2");
 }
 
 uint64_t RegGetRflags()
 {
	 uint64_t flag;
	 asm volatile("pushf");
	 asm volatile("popl %eax");
	 asm volatile("movl %%eax,%0":"=r"(flag));
	 return flag;
	 
 }
 
 uint64_t RegGetRsp()
 {
	 uint64_t value;
	 asm volatile("movl %esp,%eax");
	 asm volatile("addl $4,%eax");
	 asm volatile("movl %%eax,%0":"=r"(value));
	 return value;
 }
 
 uint64_t RegGetCr0()
 {
	 uint64_t cr0;
	 asm volatile("movl %%cr0,%0":"=r"(cr0));
	 return cr0;
 }
 
  uint64_t RegGetCr3()
 {
	 uint64_t cr3;
	 asm volatile("movl %%cr3,%0":"=r"(cr3));
	 return cr3;
 }
 
  uint64_t RegGetCr4()
 {
	 uint64_t cr4;
	 asm volatile("movl %%cr4,%0":"=r"(cr4));
	 return cr4;
 }
 
  uint64_t RegGetFs()
 {
	 uint64_t fs;
	 asm volatile("movl %fs,%eax");
	 asm volatile("movl %%eax,%0":"=r"(fs));
	 return fs;
 }
 
  uint64_t RegGetGs()
 {
	 uint64_t Gs;
	 asm volatile("movl %gs,%eax");
	 asm volatile("movl %%eax,%0":"=r"(Gs));
	 return Gs;
 }
 
  uint64_t RegGetEs()
 {
	 uint64_t Es;
	 asm volatile("movl %es,%eax");
	 asm volatile("movl %%eax,%0":"=r"(Es));
	 return Es;
 }
  
  uint64_t RegGetCs()
{
	 uint64_t Cs;
	 asm volatile("movl %cs,%eax");
	 asm volatile("movl %%eax,%0":"=r"(Cs));
	 return Cs;
}
 
  uint64_t RegGetSs()
 {
	 uint64_t Ss;
	 asm volatile("movl %ss,%eax");
	 asm volatile("movl %%eax,%0":"=r"(Ss));
	 return Ss;
 }
  
  uint64_t RegGetDs()
  {
	 uint64_t Ds;
	 asm volatile("movl %ds,%eax");
	 asm volatile("movl %%eax,%0":"=r"(Ds));
	 return Ds;
  }
  
  uint64_t GetTrSelector()
 {
	 uint64_t st;
	 asm volatile("str %eax");
	 asm volatile("movl %%eax,%0":"=r"(st));
	 return st;
	 
 }
 
 uint8_t GetGdtLimit()
 {
	 uint8_t limit = 0;
	 unsigned char gdtr[6];
	 asm volatile("sgdt %0":"=m"(gdtr));
	 limit = limit | gdtr[0];
	 limit = (limit << 4) | gdtr[1];
	 return limit;
 }
 
  uint8_t GetIdtLimit()
 {
	 uint8_t limit = 0;
	 unsigned char idtr[6];
	 asm volatile("sidt %0":"=m"(idtr));
	 limit = limit | idtr[0];
	 limit = (limit << 4) | idtr[1];
	 return limit;
 }
 
 uint32_t GetGdtBase()
 {
	 uint32_t base = 0;
	 unsigned char gdtr[6];
	 asm volatile("sgdt %0":"=m"(gdtr));
	 base = *(uint32_t *)&gdtr[2];
	 return base;
 }
 
 uint32_t GetLdtr()
 {
	 uint32_t value;
	 asm volatile("sldt %eax");
	 asm volatile("movl %%eax,%0":"=r"(value));
	 return value;
 }
 
 uint32_t GetIdtBase()
 {
	 uint32_t base =0;
	 unsigned char idtr[6];
	 asm volatile("sidt %0":"=m"(idtr));
	 base = *(uint32_t *)&idtr[2];
	 return base;
 }
 
//CmIOIn PROC StdCall _Port
	//mov edx,_Port
	//in  eax,dx
	//ret
//CmIOIn ENDP
 uint32_t CmIOIn(uint32_t port)
 {
	 uint32_t invalue;
	 asm volatile("movl %0, %%edx"::"r"(port));
	 asm volatile("inl %%dx,%0":"=a"(invalue));
	 return invalue;
 }


//CmIOOutB PROC StdCall _Port,_Data
	//mov  eax,_Data
	//mov  edx,_Port
	//out  dx,al
	//ret
//CmIOOutB ENDP
void CmIOOutB(uint32_t port, uint32_t data)
{
	asm volatile("movl %0,%%eax"::"r"(data));
	asm volatile("movl %0,%%edx"::"r"(port));
	asm volatile("outb %al,%dx");
}

//CmIOOutW PROC StdCall _Port,_Data
	//mov  eax,_Data
	//mov  edx,_Port
	//out  dx,ax
	//ret
//CmIOOutW ENDP

void CmIOOutW(uint32_t port, uint32_t data)
{
	asm volatile("movl %0,%%eax"::"r"(data));
	asm volatile("movl %0,%%edx"::"r"(port));
	asm volatile("outb %ax,%dx");
}

//CmIOOutD PROC StdCall _Port,_Data
	//mov  eax,_Data
	//mov  edx,_Port
	//out  dx,eax
	//ret
//CmIOOutD ENDP
 
void CmIOOutD(uint32_t port, uint32_t data)
{
	asm volatile("movl %0,%%eax"::"r"(data));
	asm volatile("movl %0,%%edx"::"r"(port));
	asm volatile("outb %eax,%dx");
} 
 
//+++++++++++++++++++++++DDK Function+++++++++++++++++++++++++++++++
void InitializeListHead(PZION_LIST_ENTRY ListHead)
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

void InsertTailList(PZION_LIST_ENTRY ListHead, PZION_LIST_ENTRY Entry)
{
	PZION_LIST_ENTRY temp;
	temp = ListHead->Blink;
	ListHead->Blink = Entry;
	Entry->Flink = ListHead;
	Entry->Blink = temp;
	temp->Flink = Entry;
}


