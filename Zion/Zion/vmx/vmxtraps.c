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

#include "vmxtraps.h"
#include "scancode.h"
//#include "vmx.h"


//#ifdef VMX_ENABLE_PS2_KBD_SNIFFER
//# include "misc/scancode.h"
//#endif

//extern PHYSICAL_ADDRESS g_IdentityPageTableBasePhysicalAddress, g_IdentityPageTableBasePhysicalAddress_Legacy;

static bool ZVMAPI VmxDispatchVmxInstrDummy (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  bool WillBeAlsoHandledByGuestHv
)
{
  uint64_t inst_len;
  if (!Cpu || !GuestRegs)
    return TRUE;
  cprintf(("VmxDispatchVminstructionDummy(): Nested virtualization not supported in this build!\n"));

  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  Trap->General.RipDelta = inst_len;

  VmxWrite (GUEST_RFLAGS, (VmxRead (GUEST_RFLAGS) & (~0x8d5)) | 0x1 /* VMFailInvalid */ );
  return TRUE;
}

static bool ZVMAPI VmxDispatchException(
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  bool WillBeAlsoHandledByGuestHv
 )
 {
  uint64_t inst_len, uIntrInfo;
  uint32_t cr0=0x7fffffff;

  if (!Cpu || !GuestRegs)
    return TRUE;

  uIntrInfo = VmxRead (VM_EXIT_INTR_INFO);
  if ((uIntrInfo & 0xff) != 13)
    // we accept only #GP here
    return TRUE;
	
  cprintf("entrying here...\n");	
  asm volatile("movl %cr0,%eax");
  asm volatile("andl %0,%%eax"::"r"(cr0));
  asm volatile("movl %eax,%cr0");
  
  inst_len = VmxRead(VM_EXIT_INSTRUCTION_LEN);
		
  if (Trap->General.RipDelta == 0)
			Trap->General.RipDelta = inst_len;\
		
		
  return TRUE;
			
	
  	
 }

static bool ZVMAPI VmxDispatchVmxoff(
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  bool WillBeAlsoHandledByGuestHv
 )
 {
	 uint64_t inst_len;
	 
	 if(!Cpu || !GuestRegs)
		return TRUE;
		
     asm volatile(".byte	0x0f, 0x01, 0xC4");
	 
	 inst_len = VmxRead(VM_EXIT_INSTRUCTION_LEN);
		
		if (Trap->General.RipDelta == 0)
			Trap->General.RipDelta = inst_len;
			
	 return TRUE;
 }
 
 
static bool ZVMAPI VmxDispatchCpuid (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  bool WillBeAlsoHandledByGuestHv
)
{
  uint32_t fn, eax, ebx, ecx, edx;
  uint64_t inst_len;

  if (!Cpu || !GuestRegs)
    return TRUE;

  fn = GuestRegs ->eax;
  cprintf("fn is 0x%x\n",fn);
  GuestRegs ->eax = 0x123456;

  cprintf("Hello world!\n");

				
  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  
  if (Trap->General.RipDelta == 0)
    Trap->General.RipDelta = inst_len;
	
  
  return TRUE;
}

//static bool ZVMAPI VmxDispatchMsrRead (
  //PCPU Cpu,
  //PGUEST_REGS GuestRegs,
  //PNBP_TRAP Trap,
  //bool WillBeAlsoHandledByGuestHv
//)
//{
  //LARGE_INTEGER MsrValue;
  //uint32_t ecx;
  //uint64_t inst_len;

  //if (!Cpu || !GuestRegs)
    //return TRUE;

  //inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  //if (Trap->General.RipDelta == 0)
    //Trap->General.RipDelta = inst_len;

  //ecx = (uint32_t) GuestRegs->ecx;

  //switch (ecx) {
  //case MSR_IA32_SYSENTER_CS:
    //MsrValue.QuadPart = VmxRead (GUEST_SYSENTER_CS);
    //break;

  //case MSR_IA32_SYSENTER_ESP:
    //MsrValue.QuadPart = VmxRead (GUEST_SYSENTER_ESP);
    //break;
  //case MSR_IA32_SYSENTER_EIP:
    //MsrValue.QuadPart = VmxRead (GUEST_SYSENTER_EIP);
    //break;
  //case MSR_GS_BASE:
    //MsrValue.QuadPart = VmxRead (GUEST_GS_BASE);
    //break;
  //case MSR_FS_BASE:
    //MsrValue.QuadPart = VmxRead (GUEST_FS_BASE);
    //break;
  //case MSR_EFER:
    //MsrValue.QuadPart = Cpu->Vmx.GuestEFER;
    ////cprintf(("Guestip 0x%llx MSR_EFER Read 0x%llx 0x%llx \n",VmxRead(GUEST_RIP),ecx,MsrValue.QuadPart));
    //break;
  //default:
    //MsrValue.QuadPart = MsrRead (ecx);
  //}

  //GuestRegs->eax = MsrValue.LowPart;
  //GuestRegs->edx = MsrValue.HighPart;

  //return TRUE;
//}

//static bool ZVMAPI VmxDispatchMsrWrite (
  //PCPU Cpu,
  //PGUEST_REGS GuestRegs,
  //PNBP_TRAP Trap,
  //bool WillBeAlsoHandledByGuestHv
//)
//{
  //LARGE_INTEGER MsrValue;
  //uint32_t ecx;
  //uint64_t inst_len;

  //if (!Cpu || !GuestRegs)
    //return TRUE;

  //inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  //if (Trap->General.RipDelta == 0)
    //Trap->General.RipDelta = inst_len;

  //ecx = (uint32_t) GuestRegs->ecx;

  //MsrValue.LowPart = (uint32_t) GuestRegs->eax;
  //MsrValue.HighPart = (uint32_t) GuestRegs->edx;

  //switch (ecx) {
  //case MSR_IA32_SYSENTER_CS:
    //VmxWrite (GUEST_SYSENTER_CS, MsrValue.QuadPart);
    //break;
  //case MSR_IA32_SYSENTER_ESP:
    //VmxWrite (GUEST_SYSENTER_ESP, MsrValue.QuadPart);
    //break;
  //case MSR_IA32_SYSENTER_EIP:
    //VmxWrite (GUEST_SYSENTER_EIP, MsrValue.QuadPart);
    //break;
  //case MSR_GS_BASE:
    //VmxWrite (GUEST_GS_BASE, MsrValue.QuadPart);
    //break;
  //case MSR_FS_BASE:
    //VmxWrite (GUEST_FS_BASE, MsrValue.QuadPart);
    //break;
  //case MSR_EFER:
    ////cprintf(("Guestip 0x%llx MSR_EFER write 0x%llx 0x%llx\n",VmxRead(GUEST_RIP),ecx,MsrValue.QuadPart)); 
    //Cpu->Vmx.GuestEFER = MsrValue.QuadPart;
    //MsrWrite (MSR_EFER, (MsrValue.QuadPart) | EFER_LME);
    //break;
  //default:
    //MsrWrite (ecx, MsrValue.QuadPart);
  //}

  //return TRUE;
//}

//static void VmxUpdateGuestEfer (
  //PCPU Cpu
//)
//{
  //if (Cpu->Vmx.GuestEFER & EFER_LMA)
    //VmxWrite (VM_ENTRY_CONTROLS, VmxRead (VM_ENTRY_CONTROLS) | (VM_ENTRY_IA32E_MODE));
  //else
    //VmxWrite (VM_ENTRY_CONTROLS, VmxRead (VM_ENTRY_CONTROLS) & (~VM_ENTRY_IA32E_MODE));
//}

////TODO: this function needs to be cleaned up -- too much stuff is commented out
//static bool ZVMAPI VmxDispatchCrAccess (
  //PCPU Cpu,
  //PGUEST_REGS GuestRegs,
  //PNBP_TRAP Trap,
  //bool WillBeAlsoHandledByGuestHv
//)
//{
  //uint32_t exit_qualification;
  //uint32_t gp, cr;
  //uint64_t value;
  //uint64_t inst_len;

  //if (!Cpu || !GuestRegs)
    //return TRUE;

//#if DEBUG_LEVEL>2
  //cprintf (("VmxDispatchCrAccess()\n"));
//#endif

  //inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  //if (Trap->General.RipDelta == 0)
    //Trap->General.RipDelta = inst_len;

  //exit_qualification = (uint32_t) VmxRead (EXIT_QUALIFICATION);
  //gp = (exit_qualification & CONTROL_REG_ACCESS_REG) >> 8;
  //cr = exit_qualification & CONTROL_REG_ACCESS_NUM;

//#if DEBUG_LEVEL>2
  //cprintf (("VmxDispatchCrAccess(): gp: 0x%x cr: 0x%x exit_qualification: 0x%x\n", gp, cr, exit_qualification));
//#endif

  //switch (exit_qualification & CONTROL_REG_ACCESS_TYPE) {
  //case TYPE_MOV_TO_CR:
    //if (cr == 0) {
      //Cpu->Vmx.GuestCR0 = *(((Puint64_t) GuestRegs) + gp);
      //if ((*(((Puint64_t) GuestRegs) + gp)) & X86_CR0_PG)        //enable paging
      //{
        ////cprintf(("VmxDispatchCrAccess():paging\n"));
        //VmxWrite (GUEST_CR3, Cpu->Vmx.GuestCR3);
        //if (Cpu->Vmx.GuestEFER & EFER_LME)
          //Cpu->Vmx.GuestEFER |= EFER_LMA;
        //else
          //Cpu->Vmx.GuestEFER &= ~EFER_LMA;
      //} 
	  //else                    //disable paging
      //{
        ////cprintf(("VmxDispatchCrAccess():disable paging\n"));                         
        //Cpu->Vmx.GuestCR3 = VmxRead (GUEST_CR3);
        //VmxWrite (GUEST_CR3, g_IdentityPageTableBasePhysicalAddress_Legacy.QuadPart);
        ///*
           //if(Cpu->Vmx.GuestMode) //Long Mode
           //VmxWrite(GUEST_CR3,g_IdentityPageTableBasePhysicalAddress.QuadPart);         
           //else //Legacy Mode
           //VmxWrite(GUEST_CR3,g_IdentityPageTableBasePhysicalAddress_Legacy.QuadPart);                          
         //*/
        //Cpu->Vmx.GuestEFER &= ~EFER_LMA;
      //}
//#ifdef _X86_
      //VmxWrite (CR0_READ_SHADOW, (*(((Puint32_t) GuestRegs) + gp)) & X86_CR0_PG);
//#else
      //VmxWrite (CR0_READ_SHADOW, (*(((Puint64_t) GuestRegs) + gp)) & X86_CR0_PG);
//#endif
      //VmxUpdateGuestEfer (Cpu);
      //return FALSE;
    //}

    //if (cr == 3) {
      //Cpu->Vmx.GuestCR3 = *(((Puint64_t) GuestRegs) + gp);

      //if (Cpu->Vmx.GuestCR0 & X86_CR0_PG)       //enable paging
      //{
//#if DEBUG_LEVEL>2
        //cprintf (("VmxDispatchCrAccess(): TYPE_MOV_TO_CR cr3:0x%x\n", *(((Puint64_t) GuestRegs) + gp)));
//#endif
//#ifdef _X86_
        //VmxWrite (GUEST_CR3, *(((Puint32_t) GuestRegs) + gp));
//#else
        //VmxWrite (GUEST_CR3, *(((Puint64_t) GuestRegs) + gp));
//#endif
      //}
      //return TRUE;
    //}
    //if (cr == 4) {

      ////if(debugmode)
      ////cprintf(("VmxDispatchCrAccess(): TYPE_MOV_TO_CR Cpu->Vmx.GuestEFER:0x%x Cpu->Vmx.GuestCR0:0x%x cr4:0x%x\n",Cpu->Vmx.GuestEFER,Cpu->Vmx.GuestCR0,*(((Puint64_t)GuestRegs)+gp)));
      ////Nbp need enabele VMXE. so guest try to clear cr4_vmxe, it would be mask.
//#ifdef _X86_
      //VmxWrite (CR4_READ_SHADOW, (*(((Puint32_t) GuestRegs) + gp)) & (X86_CR4_VMXE | X86_CR4_PAE));
      //Cpu->Vmx.GuestCR4 = *(((Puint32_t) GuestRegs) + gp);
      //VmxWrite (GUEST_CR4, (*(((Puint32_t) GuestRegs) + gp)) | X86_CR4_VMXE);

//#else
      ////VmxWrite(CR4_READ_SHADOW, (*(((Puint64_t)GuestRegs)+gp)) & (X86_CR4_VMXE|X86_CR4_PAE|X86_CR4_PSE));
      //VmxWrite (CR4_READ_SHADOW, (*(((Puint64_t) GuestRegs) + gp)) & (X86_CR4_VMXE));

      //Cpu->Vmx.GuestCR4 = *(((Puint64_t) GuestRegs) + gp);
      //VmxWrite (GUEST_CR4, (*(((Puint64_t) GuestRegs) + gp)) | X86_CR4_VMXE);
//#endif

      //return FALSE;

    //}
    //break;
  //case TYPE_MOV_FROM_CR:
    //if (cr == 3) {
      //value = Cpu->Vmx.GuestCR3;
//#if DEBUG_LEVEL>2
      //cprintf (("VmxDispatchCrAccess(): TYPE_MOV_FROM_CR cr3:0x%x\n", value));
//#endif
//#ifdef _X86_
      //*(((Puint32_t) GuestRegs) + gp) = (uint32_t) value;
//#else
      //*(((Puint64_t) GuestRegs) + gp) = value;
//#endif
    //}
    //break;
  //case TYPE_CLTS:
    //break;
  //case TYPE_LMSW:
    //break;
  //}

  //return TRUE;
//}


static bool ZVMAPI VmxDispatchIoAccess (
  PCPU Cpu,
  PGUEST_REGS GuestRegs,
  PNBP_TRAP Trap,
  bool WillBeAlsoHandledByGuestHv
)
{
  uint32_t exit_qualification;
  uint32_t port, size;
  uint32_t dir, df, vm86;
  uint64_t inst_len;

  if (!Cpu || !GuestRegs)
    return TRUE;

  inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  if (Trap->General.RipDelta == 0)
    Trap->General.RipDelta = inst_len;

  exit_qualification = (uint32_t) VmxRead (EXIT_QUALIFICATION);
  init_scancode ();

  if (CmIsBitSet (exit_qualification, 6))
    port = (exit_qualification >> 16) & 0xFFFF;
  else
    port = ((uint32_t) (GuestRegs->edx)) & 0xffff;

  size = (exit_qualification & 7) + 1;
  dir = CmIsBitSet (exit_qualification, 3);     /* direction */
  if (dir) {                    //in

    GuestRegs->eax = CmIOIn (port);
    //if (port == 0x64) {
      //if (GuestRegs->eax & 0x20)
        //ps2mode = 0x1;          //mouse
      //else
        //ps2mode = 0;            //mouse
    //}ps2mode == 0x0 &&
    if (port == 0x60 && (GuestRegs->eax & 0xff) < 0x80) {
      cprintf ("IO 0x%x IN 0x%x \n", port, GuestRegs->eax); //scancode[GuestRegs->eax & 0xff]));
	  cprintf ("sancode is %c\n",scancode[GuestRegs->eax & 0xff]);

    }

  } else {                      //out

    if (size == 1)
      CmIOOutB (port, (uint32_t) GuestRegs->eax);
    if (size == 2)
      CmIOOutW (port, (uint32_t) GuestRegs->eax);
    if (size == 4)
      CmIOOutD (port, (uint32_t) GuestRegs->eax);

  }

  return TRUE;
}
//#endif

//#ifdef INTERCEPT_RDTSCs
//static BOOLEAN ZVMAPI VmxDispatchRdtsc (
  //PCPU Cpu,
  //PGUEST_REGS GuestRegs,
  //PNBP_TRAP Trap,
  //BOOLEAN WillBeAlsoHandledByGuestHv
//)
//{
  //uint64_t inst_len;

  //inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  //if (Trap->General.RipDelta == 0)
    //Trap->General.RipDelta = inst_len;

//# if DEBUG_LEVEL>2
  //cprintf (("VmxDispatchRdtsc(): RDTSC intercepted, RIP: 0x%p\n", VmxRead (GUEST_RIP)));
//# endif
  //if (Cpu->Tracing > 0) {
    //Cpu->Tsc = Cpu->EmulatedCycles + Cpu->LastTsc;
  //} else {
    //Cpu->Tsc = RegGetTSC ();
  //}

//# if DEBUG_LEVEL>2
  //cprintf ((" Tracing = %d, LastTsc = %p, EmulatedCycles = %p, Tsc = %p\n",
             //Cpu->Tracing, Cpu->LastTsc, Cpu->EmulatedCycles, Cpu->Tsc));
//# endif

  //Cpu->LastTsc = Cpu->Tsc;
  //Cpu->EmulatedCycles = 0;
  //Cpu->NoOfRecordedInstructions = 0;
  //Cpu->Tracing = INSTR_TRACE_MAX;

  //GuestRegs->rdx = (size_t) (Cpu->Tsc >> 32);
  //GuestRegs->rax = (size_t) (Cpu->Tsc & 0xffffffff);
  //VmxWrite (GUEST_RFLAGS, VmxRead (GUEST_RFLAGS) | 0x100);      // set TF

  //return TRUE;
//}

//// FIXME: This looks like it needs reviewing -- compare with the SvmDispatchDB
//static BOOLEAN ZVMAPI VmxDispatchException (
  //PCPU Cpu,
  //PGUEST_REGS GuestRegs,
  //PNBP_TRAP Trap,
  //BOOLEAN WillBeAlsoHandledByGuestHv
//)
//{
  //uint64_t inst_len, uIntrInfo;

  //if (!Cpu || !GuestRegs)
    //return TRUE;

  //uIntrInfo = VmxRead (VM_EXIT_INTR_INFO);
  //if ((uIntrInfo & 0xff) != 1)
    //// we accept only #DB here
    //return TRUE;

//# if DEBUG_LEVEL>2
  //cprintf (("VmxDispatchException(): DB intercepted, RIP: 0x%p, INTR_INFO 0x%p, flags 0x%p, II 0x%p, PD 0x%p\n",
             //VmxRead (GUEST_RIP), VmxRead (VM_EXIT_INTR_INFO), VmxRead (GUEST_RFLAGS),
             //VmxRead (GUEST_INTERRUPTIBILITY_INFO), VmxRead (GUEST_PENDING_DBG_EXCEPTIONS)));
//# endif

  //VmxWrite (GUEST_INTERRUPTIBILITY_INFO, 0);
  //// FIXME: why is this commented?
////      if (RegGetDr6() & 0x40) {

//# if DEBUG_LEVEL>2
  //cprintf (("VmxDispatchException(): DB intercepted, RIP: 0x%p\n", VmxRead (GUEST_RIP)));
//# endif

  //Cpu->EmulatedCycles += 6;     // TODO: replace with f(Opcode)
  //if (Cpu->Tracing-- <= 0)
    //VmxWrite (GUEST_RFLAGS, VmxRead (GUEST_RFLAGS) & ~0x100);   // disable TF

  //Cpu->NoOfRecordedInstructions++;
  ////TODO: add instruction opcode to Cpu->RecordedInstructions[]

////      }       

  //return TRUE;
//}
//#endif

//static bool ZVMAPI VmxDispatchINVD (
  //PCPU Cpu,
  //PGUEST_REGS GuestRegs,
  //PNBP_TRAP Trap,
  //bool WillBeAlsoHandledByGuestHv
//)
//{
  //uint64_t inst_len;

  //if (!Cpu || !GuestRegs)
    //return TRUE;

  //inst_len = VmxRead (VM_EXIT_INSTRUCTION_LEN);
  //if (Trap->General.RipDelta == 0)
    //Trap->General.RipDelta = inst_len;

  //return TRUE;
//}

//
// ------------------------------------------------------------------------------------
//

ZVMSTATUS ZVMAPI VmxRegisterTraps (
  PCPU Cpu
)
{
  ZVMSTATUS Status;
  PNBP_TRAP Trap;
#ifndef VMX_SUPPORT_NESTED_VIRTUALIZATION
  // used to set dummy handler for all VMX intercepts when we compile without nested support
  uint32_t i, TableOfVmxExits[] = {
    EXIT_REASON_VMCALL,
    EXIT_REASON_VMCALL,
    EXIT_REASON_VMLAUNCH,
    EXIT_REASON_VMRESUME,
    EXIT_REASON_VMPTRLD,
    EXIT_REASON_VMPTRST,
    EXIT_REASON_VMREAD,
    EXIT_REASON_VMWRITE,
    EXIT_REASON_VMXON,
    EXIT_REASON_VMXOFF
  };
#endif
   
  if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, EXIT_REASON_CPUID, 0, // length of the instruction, 0 means length need to be get from vmcs later. 
                                                     VmxDispatchCpuid, &Trap))) {
    cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchCpuid with status 0x%08hX\n"));
    return Status;
  }
  TrRegisterTrap (Cpu, Trap);
  
  if (!ZVM_SUCCESS(Status = TrInitializeGeneralTrap(Cpu, EXIT_REASON_VMXOFF, 0,VmxDispatchVmxoff, &Trap)))
  {
	  cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchVmxoff with status 0x%08hX\n"));
  }
  TrRegisterTrap (Cpu,Trap);
  
  if (!ZVM_SUCCESS(Status = TrInitializeGeneralTrap(Cpu, EXIT_REASON_EXCEPTION_NMI, 0,VmxDispatchException, &Trap)))
  {
	  cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchVmxoff with status 0x%08hX\n"));
  }
  TrRegisterTrap (Cpu,Trap); 
  
  if (!ZVM_SUCCESS(Status = TrInitializeGeneralTrap(Cpu, EXIT_REASON_IO_INSTRUCTION, 0,VmxDispatchIoAccess, &Trap)))
  {
	  cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchVmxoff with status 0x%08hX\n"));
  }
  TrRegisterTrap (Cpu,Trap); 

  //if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, EXIT_REASON_MSR_READ, 0,      // length of the instruction, 0 means length need to be get from vmcs later. 
                                                     //VmxDispatchMsrRead, &Trap))) {
    //cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchMsrRead with status 0x%08hX\n"));
    //return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, EXIT_REASON_MSR_WRITE, 0,     // length of the instruction, 0 means length need to be get from vmcs later. 
                                                     //VmxDispatchMsrWrite, &Trap))) {
    //cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchMsrWrite with status 0x%08hX\n"));
    //return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, EXIT_REASON_CR_ACCESS, 0,     // length of the instruction, 0 means length need to be get from vmcs later. 
                                                     //VmxDispatchCrAccess, &Trap))) {
    //cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchCrAccess with status 0x%08hX\n"));
    //return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, EXIT_REASON_INVD, 0,  // length of the instruction, 0 means length need to be get from vmcs later. 
                                                     //VmxDispatchINVD, &Trap))) {
    //cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchINVD with status 0x%08hX\n"));
    //return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  // set dummy handler for all VMX intercepts if we compile wihtout nested support
  ///for (i = 0; i < sizeof (TableOfVmxExits) / sizeof (uint32_t); i++) {
    ///if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, TableOfVmxExits[i], 0,      // length of the instruction, 0 means length need to be get from vmcs later. 
       ///                                                VmxDispatchVmxInstrDummy, &Trap))) {
      ///cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchVmon with status 0x%08hX\n"));
      ///return Status;
    ///}
    ///TrRegisterTrap (Cpu, Trap);
  ///}

//#ifdef INTERCEPT_RDTSCs
  //if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, EXIT_REASON_EXCEPTION_NMI, 0, VmxDispatchException, &Trap))) {
    //cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchException with status 0x%08hX\n"));
    //return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);

  //if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, EXIT_REASON_RDTSC, 0, VmxDispatchRdtsc, &Trap))) {
    //cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchRdtsc with status 0x%08hX\n"));
    //return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);
//#endif
//#ifdef VMX_ENABLE_PS2_KBD_SNIFFER
  //if (!ZVM_SUCCESS (Status = TrInitializeGeneralTrap (Cpu, EXIT_REASON_IO_INSTRUCTION, 0,        // length of the instruction, 0 means length need to be get from vmcs later. 
                                                     //VmxDispatchIoAccess, &Trap))) {
    //cprintf (("VmxRegisterTraps(): Failed to register VmxDispatchIoAccess with status 0x%08hX\n"));
    //return Status;
  //}
  //TrRegisterTrap (Cpu, Trap);
//#endif

  return ZVMSUCCESS;
}
