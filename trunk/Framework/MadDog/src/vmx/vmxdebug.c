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

#include "vmx.h"

VOID NTAPI VmxDumpVmcs (
)
{

  ULONG32 addr;

  DbgPrint("\n\n\n/*****16-bit Guest-State Fields*****/\n");
  addr = GUEST_ES_SELECTOR;
  DbgPrint("GUEST_ES_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_CS_SELECTOR;
  DbgPrint("GUEST_CS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_SS_SELECTOR;
  DbgPrint("GUEST_SS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_DS_SELECTOR;
  DbgPrint("GUEST_DS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_FS_SELECTOR;
  DbgPrint("GUEST_FS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_GS_SELECTOR;
  DbgPrint("GUEST_GS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_LDTR_SELECTOR;
  DbgPrint("GUEST_LDTR_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_TR_SELECTOR;
  DbgPrint("GUEST_TR_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****16-bit Host-State Fields*****/\n");
  addr = HOST_ES_SELECTOR;
  DbgPrint("HOST_ES_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_CS_SELECTOR;
  DbgPrint("HOST_CS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_SS_SELECTOR;
  DbgPrint("HOST_SS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_DS_SELECTOR;
  DbgPrint("HOST_DS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_FS_SELECTOR;
  DbgPrint("HOST_FS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_GS_SELECTOR;
  DbgPrint("HOST_GS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_TR_SELECTOR;
  DbgPrint("HOST_TR_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****64-bit Control Fields*****/\n");
  addr = IO_BITMAP_A;
  DbgPrint("IO_BITMAP_A 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = IO_BITMAP_A_HIGH;
  DbgPrint("IO_BITMAP_A_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = IO_BITMAP_B;
  DbgPrint("IO_BITMAP_B 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = IO_BITMAP_B_HIGH;
  DbgPrint("IO_BITMAP_B_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = MSR_BITMAP;
  DbgPrint("MSR_BITMAP 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = MSR_BITMAP_HIGH;
  DbgPrint("MSR_BITMAP_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_MSR_STORE_ADDR;
  DbgPrint("VM_EXIT_MSR_STORE_ADDR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_MSR_STORE_ADDR_HIGH;
  DbgPrint("VM_EXIT_MSR_STORE_ADDR_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_MSR_LOAD_ADDR;
  DbgPrint("VM_EXIT_MSR_LOAD_ADDR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_MSR_LOAD_ADDR_HIGH;
  DbgPrint("VM_EXIT_MSR_LOAD_ADDR_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_ENTRY_MSR_LOAD_ADDR;
  DbgPrint("VM_ENTRY_MSR_LOAD_ADDR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_ENTRY_MSR_LOAD_ADDR_HIGH;
  DbgPrint("VM_ENTRY_MSR_LOAD_ADDR_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = TSC_OFFSET;
  DbgPrint("TSC_OFFSET 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = TSC_OFFSET_HIGH;
  DbgPrint("TSC_OFFSET_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VIRTUAL_APIC_PAGE_ADDR;
  DbgPrint("VIRTUAL_APIC_PAGE_ADDR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VIRTUAL_APIC_PAGE_ADDR_HIGH;
  DbgPrint("VIRTUAL_APIC_PAGE_ADDR_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****64-bit Guest-State Fields*****/\n");
  addr = VMCS_LINK_POINTER;
  DbgPrint("VMCS_LINK_POINTER 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VMCS_LINK_POINTER_HIGH;
  DbgPrint("VMCS_LINK_POINTER_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_IA32_DEBUGCTL;
  DbgPrint("GUEST_IA32_DEBUGCTL 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_IA32_DEBUGCTL_HIGH;
  DbgPrint("GUEST_IA32_DEBUGCTL_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****32-bit Control Fields*****/\n");
  addr = PIN_BASED_VM_EXEC_CONTROL;
  DbgPrint("PIN_BASED_VM_EXEC_CONTROL 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CPU_BASED_VM_EXEC_CONTROL;
  DbgPrint("CPU_BASED_VM_EXEC_CONTROL 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = EXCEPTION_BITMAP;
  DbgPrint("EXCEPTION_BITMAP 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = PAGE_FAULT_ERROR_CODE_MASK;
  DbgPrint("PAGE_FAULT_ERROR_CODE_MASK 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = PAGE_FAULT_ERROR_CODE_MATCH;
  DbgPrint("PAGE_FAULT_ERROR_CODE_MATCH 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CR3_TARGET_COUNT;
  DbgPrint("CR3_TARGET_COUNT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_CONTROLS;
  DbgPrint("VM_EXIT_CONTROLS 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_MSR_STORE_COUNT;
  DbgPrint("VM_EXIT_MSR_STORE_COUNT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_MSR_LOAD_COUNT;
  DbgPrint("VM_EXIT_MSR_LOAD_COUNT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_ENTRY_CONTROLS;
  DbgPrint("VM_ENTRY_CONTROLS 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_ENTRY_MSR_LOAD_COUNT;
  DbgPrint("VM_ENTRY_MSR_LOAD_COUNT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_ENTRY_INTR_INFO_FIELD;
  DbgPrint("VM_ENTRY_INTR_INFO_FIELD 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_ENTRY_EXCEPTION_ERROR_CODE;
  DbgPrint("VM_ENTRY_EXCEPTION_ERROR_CODE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_ENTRY_INSTRUCTION_LEN;
  DbgPrint("VM_ENTRY_INSTRUCTION_LEN 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = TPR_THRESHOLD;
  DbgPrint("TPR_THRESHOLD 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = SECONDARY_VM_EXEC_CONTROL;
  DbgPrint("SECONDARY_VM_EXEC_CONTROL 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****32-bit RO Data Fields*****/\n");
  addr = VM_INSTRUCTION_ERROR;
  DbgPrint("VM_INSTRUCTION_ERROR 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_REASON;
  DbgPrint("VM_EXIT_REASON 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_INTR_INFO;
  DbgPrint("VM_EXIT_INTR_INFO 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_INTR_ERROR_CODE;
  DbgPrint("VM_EXIT_INTR_ERROR_CODE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = IDT_VECTORING_INFO_FIELD;
  DbgPrint("IDT_VECTORING_INFO_FIELD 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = IDT_VECTORING_ERROR_CODE;
  DbgPrint("IDT_VECTORING_ERROR_CODE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VM_EXIT_INSTRUCTION_LEN;
  DbgPrint("VM_EXIT_INSTRUCTION_LEN 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = VMX_INSTRUCTION_INFO;
  DbgPrint("VMX_INSTRUCTION_INFO 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****32-bit Guest-State Fields*****/\n");
  addr = GUEST_ES_LIMIT;
  DbgPrint("GUEST_ES_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_CS_LIMIT;
  DbgPrint("GUEST_CS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_SS_LIMIT;
  DbgPrint("GUEST_SS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_DS_LIMIT;
  DbgPrint("GUEST_DS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_FS_LIMIT;
  DbgPrint("GUEST_FS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_GS_LIMIT;
  DbgPrint("GUEST_GS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_LDTR_LIMIT;
  DbgPrint("GUEST_LDTR_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_TR_LIMIT;
  DbgPrint("GUEST_TR_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_GDTR_LIMIT;
  DbgPrint("GUEST_GDTR_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_IDTR_LIMIT;
  DbgPrint("GUEST_IDTR_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_ES_AR_BYTES;
  DbgPrint("GUEST_ES_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_CS_AR_BYTES;
  DbgPrint("GUEST_CS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_SS_AR_BYTES;
  DbgPrint("GUEST_SS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_DS_AR_BYTES;
  DbgPrint("GUEST_DS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_FS_AR_BYTES;
  DbgPrint("GUEST_FS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_GS_AR_BYTES;
  DbgPrint("GUEST_GS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_LDTR_AR_BYTES;
  DbgPrint("GUEST_LDTR_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_TR_AR_BYTES;
  DbgPrint("GUEST_TR_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_INTERRUPTIBILITY_INFO;
  DbgPrint("GUEST_INTERRUPTIBILITY_INFO 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_ACTIVITY_STATE;
  DbgPrint("GUEST_ACTIVITY_STATE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_SM_BASE;
  DbgPrint("GUEST_SM_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_SYSENTER_CS;
  DbgPrint("GUEST_SYSENTER_CS 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****32-bit Host-State Fields*****/\n");
  addr = HOST_IA32_SYSENTER_CS;
  DbgPrint("HOST_IA32_SYSENTER_CS 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****Natural 64-bit Control Fields*****/\n");
  addr = CR0_GUEST_HOST_MASK;
  DbgPrint("CR0_GUEST_HOST_MASK 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CR4_GUEST_HOST_MASK;
  DbgPrint("CR4_GUEST_HOST_MASK 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CR0_READ_SHADOW;
  DbgPrint("CR0_READ_SHADOW 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CR4_READ_SHADOW;
  DbgPrint("CR4_READ_SHADOW 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CR3_TARGET_VALUE0;
  DbgPrint("CR3_TARGET_VALUE0 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CR3_TARGET_VALUE1;
  DbgPrint("CR3_TARGET_VALUE1 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CR3_TARGET_VALUE2;
  DbgPrint("CR3_TARGET_VALUE2 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = CR3_TARGET_VALUE3;
  DbgPrint("CR3_TARGET_VALUE3 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****Natural 64-bit RO Data Fields*****/\n");
  addr = EXIT_QUALIFICATION;
  DbgPrint("EXIT_QUALIFICATION 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_LINEAR_ADDRESS;
  DbgPrint("GUEST_LINEAR_ADDRESS 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****Natural 64-bit Guest-State Fields*****/\n");
  addr = GUEST_CR0;
  DbgPrint("GUEST_CR0 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_CR3;
  DbgPrint("GUEST_CR3 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_CR4;
  DbgPrint("GUEST_CR4 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_ES_BASE;
  DbgPrint("GUEST_ES_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_CS_BASE;
  DbgPrint("GUEST_CS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_SS_BASE;
  DbgPrint("GUEST_SS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_DS_BASE;
  DbgPrint("GUEST_DS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_FS_BASE;
  DbgPrint("GUEST_FS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_GS_BASE;
  DbgPrint("GUEST_GS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_LDTR_BASE;
  DbgPrint("GUEST_LDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_TR_BASE;
  DbgPrint("GUEST_TR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_GDTR_BASE;
  DbgPrint("GUEST_GDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_IDTR_BASE;
  DbgPrint("GUEST_IDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_DR7;
  DbgPrint("GUEST_DR7 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_RSP;
  DbgPrint("GUEST_RSP 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_RIP;
  DbgPrint("GUEST_RIP 0x%X: 0x%llX\n", addr, VmxRead (addr));
  addr = GUEST_RFLAGS;
  DbgPrint("GUEST_RFLAGS 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_PENDING_DBG_EXCEPTIONS;
  DbgPrint("GUEST_PENDING_DBG_EXCEPTIONS 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_SYSENTER_ESP;
  DbgPrint("GUEST_SYSENTER_ESP 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = GUEST_SYSENTER_EIP;
  DbgPrint("GUEST_SYSENTER_EIP 0x%X: 0x%llx\n", addr, VmxRead (addr));

  DbgPrint("\n\n\n/*****Natural 64-bit Host-State Fields*****/\n");
  addr = HOST_CR0;
  DbgPrint("HOST_CR0 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_CR3;
  DbgPrint("HOST_CR3 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_CR4;
  DbgPrint("HOST_CR4 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_FS_BASE;
  DbgPrint("HOST_FS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_GS_BASE;
  DbgPrint("HOST_GS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_TR_BASE;
  DbgPrint("HOST_TR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_GDTR_BASE;
  DbgPrint("HOST_GDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_IDTR_BASE;
  DbgPrint("HOST_IDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_IA32_SYSENTER_ESP;
  DbgPrint("HOST_IA32_SYSENTER_ESP 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_IA32_SYSENTER_EIP;
  DbgPrint("HOST_IA32_SYSENTER_EIP 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_RSP;
  DbgPrint("HOST_RSP 0x%X: 0x%llx\n", addr, VmxRead (addr));
  addr = HOST_RIP;
  DbgPrint("HOST_RIP 0x%X: 0x%llx\n", addr, VmxRead (addr));

  return;
}

static VOID DumpMemory (
  PUCHAR Addr,
  ULONG64 Len
)
{
  ULONG64 i;
  for (i = 0; i < Len; i++) {
    DbgPrint("0x%x 0x%x\n", Addr + i, *(Addr + i));
  }
}

VOID NTAPI VmxCrash (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
)
{
  PHYSICAL_ADDRESS pa;
  NTSTATUS Status;
  DbgPrint("!!!VMX CRASH!!!\n");

#if DEBUG_LEVEL>1
  DbgPrint("rax 0x%llX\n", GuestRegs->eax);
  DbgPrint("rcx 0x%llX\n", GuestRegs->ecx);
  DbgPrint("rdx 0x%llX\n", GuestRegs->edx);
  DbgPrint("rbx 0x%llX\n", GuestRegs->ebx);
  DbgPrint("rsp 0x%llX\n", GuestRegs->esp);
  DbgPrint("rbp 0x%llX\n", GuestRegs->ebp);
  DbgPrint("rsi 0x%llX\n", GuestRegs->esi);
  DbgPrint("rdi 0x%llX\n", GuestRegs->edi);

  //DbgPrint("r8 0x%llX\n", GuestRegs->r8);
  //DbgPrint("r9 0x%llX\n", GuestRegs->r9);
  //DbgPrint("r10 0x%llX\n", GuestRegs->r10);
  //DbgPrint("r11 0x%llX\n", GuestRegs->r11);
  //DbgPrint("r12 0x%llX\n", GuestRegs->r12);
  //DbgPrint("r13 0x%llX\n", GuestRegs->r13);
  //DbgPrint("r14 0x%llX\n", GuestRegs->r14);
  //DbgPrint("r15 0x%llX\n", GuestRegs->r15);
  //DbgPrint("Guest MSR_EFER Read 0x%llx \n", Cpu->Vmx.GuestEFER);
  //CmGetPagePaByPageVaCr3 (
  //    Cpu->SparePage, 
  //    Cpu->SparePagePTE,
  //    (ULONG)VmxRead (GUEST_CR3), 
  //    (ULONG)VmxRead (GUEST_RIP), 
  //    &pa);
  //DbgPrint("VmxCrash() IOA: Failed to map PA 0x%p to VA 0x%p\n", pa.QuadPart, Cpu->SparePage);
#endif

#if DEBUG_LEVEL>2
  if (!NT_SUCCESS (Status = CmPatchPTEPhysicalAddress (Cpu->SparePagePTE, Cpu->SparePage, pa))) {
    DbgPrint("VmxCrash() IOA: Failed to map PA 0x%p to VA 0x%p, status 0x%08hX\n", pa.QuadPart, Cpu->SparePage,
               Status);
  }
  DumpMemory ((PUCHAR)
              (((ULONG64) Cpu->SparePage) | ((VmxRead (GUEST_RIP) - 0x10) & 0xfff)), 0x50);
#endif
  while (1);
}
