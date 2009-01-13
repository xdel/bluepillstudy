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

  Print (("\n\n\n/*****16-bit Guest-State Fields*****/\n"));
  addr = GUEST_ES_SELECTOR;
  Print (("GUEST_ES_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_CS_SELECTOR;
  Print (("GUEST_CS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_SS_SELECTOR;
  Print (("GUEST_SS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_DS_SELECTOR;
  Print (("GUEST_DS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_FS_SELECTOR;
  Print (("GUEST_FS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_GS_SELECTOR;
  Print (("GUEST_GS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_LDTR_SELECTOR;
  Print (("GUEST_LDTR_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_TR_SELECTOR;
  Print (("GUEST_TR_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****16-bit Host-State Fields*****/\n"));
  addr = HOST_ES_SELECTOR;
  Print (("HOST_ES_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_CS_SELECTOR;
  Print (("HOST_CS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_SS_SELECTOR;
  Print (("HOST_SS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_DS_SELECTOR;
  Print (("HOST_DS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_FS_SELECTOR;
  Print (("HOST_FS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_GS_SELECTOR;
  Print (("HOST_GS_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_TR_SELECTOR;
  Print (("HOST_TR_SELECTOR 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****64-bit Control Fields*****/\n"));
  addr = IO_BITMAP_A;
  Print (("IO_BITMAP_A 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = IO_BITMAP_A_HIGH;
  Print (("IO_BITMAP_A_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = IO_BITMAP_B;
  Print (("IO_BITMAP_B 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = IO_BITMAP_B_HIGH;
  Print (("IO_BITMAP_B_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = MSR_BITMAP;
  Print (("MSR_BITMAP 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = MSR_BITMAP_HIGH;
  Print (("MSR_BITMAP_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_MSR_STORE_ADDR;
  Print (("VM_EXIT_MSR_STORE_ADDR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_MSR_STORE_ADDR_HIGH;
  Print (("VM_EXIT_MSR_STORE_ADDR_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_MSR_LOAD_ADDR;
  Print (("VM_EXIT_MSR_LOAD_ADDR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_MSR_LOAD_ADDR_HIGH;
  Print (("VM_EXIT_MSR_LOAD_ADDR_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_ENTRY_MSR_LOAD_ADDR;
  Print (("VM_ENTRY_MSR_LOAD_ADDR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_ENTRY_MSR_LOAD_ADDR_HIGH;
  Print (("VM_ENTRY_MSR_LOAD_ADDR_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = TSC_OFFSET;
  Print (("TSC_OFFSET 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = TSC_OFFSET_HIGH;
  Print (("TSC_OFFSET_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VIRTUAL_APIC_PAGE_ADDR;
  Print (("VIRTUAL_APIC_PAGE_ADDR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VIRTUAL_APIC_PAGE_ADDR_HIGH;
  Print (("VIRTUAL_APIC_PAGE_ADDR_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****64-bit Guest-State Fields*****/\n"));
  addr = VMCS_LINK_POINTER;
  Print (("VMCS_LINK_POINTER 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VMCS_LINK_POINTER_HIGH;
  Print (("VMCS_LINK_POINTER_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_IA32_DEBUGCTL;
  Print (("GUEST_IA32_DEBUGCTL 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_IA32_DEBUGCTL_HIGH;
  Print (("GUEST_IA32_DEBUGCTL_HIGH 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****32-bit Control Fields*****/\n"));
  addr = PIN_BASED_VM_EXEC_CONTROL;
  Print (("PIN_BASED_VM_EXEC_CONTROL 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CPU_BASED_VM_EXEC_CONTROL;
  Print (("CPU_BASED_VM_EXEC_CONTROL 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = EXCEPTION_BITMAP;
  Print (("EXCEPTION_BITMAP 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = PAGE_FAULT_ERROR_CODE_MASK;
  Print (("PAGE_FAULT_ERROR_CODE_MASK 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = PAGE_FAULT_ERROR_CODE_MATCH;
  Print (("PAGE_FAULT_ERROR_CODE_MATCH 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CR3_TARGET_COUNT;
  Print (("CR3_TARGET_COUNT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_CONTROLS;
  Print (("VM_EXIT_CONTROLS 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_MSR_STORE_COUNT;
  Print (("VM_EXIT_MSR_STORE_COUNT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_MSR_LOAD_COUNT;
  Print (("VM_EXIT_MSR_LOAD_COUNT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_ENTRY_CONTROLS;
  Print (("VM_ENTRY_CONTROLS 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_ENTRY_MSR_LOAD_COUNT;
  Print (("VM_ENTRY_MSR_LOAD_COUNT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_ENTRY_INTR_INFO_FIELD;
  Print (("VM_ENTRY_INTR_INFO_FIELD 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_ENTRY_EXCEPTION_ERROR_CODE;
  Print (("VM_ENTRY_EXCEPTION_ERROR_CODE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_ENTRY_INSTRUCTION_LEN;
  Print (("VM_ENTRY_INSTRUCTION_LEN 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = TPR_THRESHOLD;
  Print (("TPR_THRESHOLD 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = SECONDARY_VM_EXEC_CONTROL;
  Print (("SECONDARY_VM_EXEC_CONTROL 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****32-bit RO Data Fields*****/\n"));
  addr = VM_INSTRUCTION_ERROR;
  Print (("VM_INSTRUCTION_ERROR 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_REASON;
  Print (("VM_EXIT_REASON 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_INTR_INFO;
  Print (("VM_EXIT_INTR_INFO 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_INTR_ERROR_CODE;
  Print (("VM_EXIT_INTR_ERROR_CODE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = IDT_VECTORING_INFO_FIELD;
  Print (("IDT_VECTORING_INFO_FIELD 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = IDT_VECTORING_ERROR_CODE;
  Print (("IDT_VECTORING_ERROR_CODE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VM_EXIT_INSTRUCTION_LEN;
  Print (("VM_EXIT_INSTRUCTION_LEN 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = VMX_INSTRUCTION_INFO;
  Print (("VMX_INSTRUCTION_INFO 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****32-bit Guest-State Fields*****/\n"));
  addr = GUEST_ES_LIMIT;
  Print (("GUEST_ES_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_CS_LIMIT;
  Print (("GUEST_CS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_SS_LIMIT;
  Print (("GUEST_SS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_DS_LIMIT;
  Print (("GUEST_DS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_FS_LIMIT;
  Print (("GUEST_FS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_GS_LIMIT;
  Print (("GUEST_GS_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_LDTR_LIMIT;
  Print (("GUEST_LDTR_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_TR_LIMIT;
  Print (("GUEST_TR_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_GDTR_LIMIT;
  Print (("GUEST_GDTR_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_IDTR_LIMIT;
  Print (("GUEST_IDTR_LIMIT 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_ES_AR_BYTES;
  Print (("GUEST_ES_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_CS_AR_BYTES;
  Print (("GUEST_CS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_SS_AR_BYTES;
  Print (("GUEST_SS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_DS_AR_BYTES;
  Print (("GUEST_DS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_FS_AR_BYTES;
  Print (("GUEST_FS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_GS_AR_BYTES;
  Print (("GUEST_GS_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_LDTR_AR_BYTES;
  Print (("GUEST_LDTR_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_TR_AR_BYTES;
  Print (("GUEST_TR_AR_BYTES 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_INTERRUPTIBILITY_INFO;
  Print (("GUEST_INTERRUPTIBILITY_INFO 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_ACTIVITY_STATE;
  Print (("GUEST_ACTIVITY_STATE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_SM_BASE;
  Print (("GUEST_SM_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_SYSENTER_CS;
  Print (("GUEST_SYSENTER_CS 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****32-bit Host-State Fields*****/\n"));
  addr = HOST_IA32_SYSENTER_CS;
  Print (("HOST_IA32_SYSENTER_CS 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****Natural 64-bit Control Fields*****/\n"));
  addr = CR0_GUEST_HOST_MASK;
  Print (("CR0_GUEST_HOST_MASK 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CR4_GUEST_HOST_MASK;
  Print (("CR4_GUEST_HOST_MASK 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CR0_READ_SHADOW;
  Print (("CR0_READ_SHADOW 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CR4_READ_SHADOW;
  Print (("CR4_READ_SHADOW 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CR3_TARGET_VALUE0;
  Print (("CR3_TARGET_VALUE0 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CR3_TARGET_VALUE1;
  Print (("CR3_TARGET_VALUE1 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CR3_TARGET_VALUE2;
  Print (("CR3_TARGET_VALUE2 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = CR3_TARGET_VALUE3;
  Print (("CR3_TARGET_VALUE3 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****Natural 64-bit RO Data Fields*****/\n"));
  addr = EXIT_QUALIFICATION;
  Print (("EXIT_QUALIFICATION 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_LINEAR_ADDRESS;
  Print (("GUEST_LINEAR_ADDRESS 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****Natural 64-bit Guest-State Fields*****/\n"));
  addr = GUEST_CR0;
  Print (("GUEST_CR0 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_CR3;
  Print (("GUEST_CR3 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_CR4;
  Print (("GUEST_CR4 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_ES_BASE;
  Print (("GUEST_ES_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_CS_BASE;
  Print (("GUEST_CS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_SS_BASE;
  Print (("GUEST_SS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_DS_BASE;
  Print (("GUEST_DS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_FS_BASE;
  Print (("GUEST_FS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_GS_BASE;
  Print (("GUEST_GS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_LDTR_BASE;
  Print (("GUEST_LDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_TR_BASE;
  Print (("GUEST_TR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_GDTR_BASE;
  Print (("GUEST_GDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_IDTR_BASE;
  Print (("GUEST_IDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_DR7;
  Print (("GUEST_DR7 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_RSP;
  Print (("GUEST_RSP 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_RIP;
  Print (("GUEST_RIP 0x%X: 0x%llX\n", addr, VmxRead (addr)));
  addr = GUEST_RFLAGS;
  Print (("GUEST_RFLAGS 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_PENDING_DBG_EXCEPTIONS;
  Print (("GUEST_PENDING_DBG_EXCEPTIONS 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_SYSENTER_ESP;
  Print (("GUEST_SYSENTER_ESP 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = GUEST_SYSENTER_EIP;
  Print (("GUEST_SYSENTER_EIP 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  Print (("\n\n\n/*****Natural 64-bit Host-State Fields*****/\n"));
  addr = HOST_CR0;
  Print (("HOST_CR0 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_CR3;
  Print (("HOST_CR3 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_CR4;
  Print (("HOST_CR4 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_FS_BASE;
  Print (("HOST_FS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_GS_BASE;
  Print (("HOST_GS_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_TR_BASE;
  Print (("HOST_TR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_GDTR_BASE;
  Print (("HOST_GDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_IDTR_BASE;
  Print (("HOST_IDTR_BASE 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_IA32_SYSENTER_ESP;
  Print (("HOST_IA32_SYSENTER_ESP 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_IA32_SYSENTER_EIP;
  Print (("HOST_IA32_SYSENTER_EIP 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_RSP;
  Print (("HOST_RSP 0x%X: 0x%llx\n", addr, VmxRead (addr)));
  addr = HOST_RIP;
  Print (("HOST_RIP 0x%X: 0x%llx\n", addr, VmxRead (addr)));

  return;
}

static VOID DumpMemory (
  PUCHAR Addr,
  ULONG64 Len
)
{
  ULONG64 i;
  for (i = 0; i < Len; i++) {
    Print (("0x%x 0x%x\n", Addr + i, *(Addr + i)));
  }
}

VOID NTAPI VmxCrash (
  PCPU Cpu,
  PGUEST_REGS GuestRegs
)
{
  PHYSICAL_ADDRESS pa;
  NTSTATUS Status;
  Print (("!!!VMX CRASH!!!\n"));

#if DEBUG_LEVEL>1
  Print (("rax 0x%llX\n", GuestRegs->eax));
  Print (("rcx 0x%llX\n", GuestRegs->ecx));
  Print (("rdx 0x%llX\n", GuestRegs->edx));
  Print (("rbx 0x%llX\n", GuestRegs->ebx));
  Print (("rsp 0x%llX\n", GuestRegs->esp));
  Print (("rbp 0x%llX\n", GuestRegs->ebp));
  Print (("rsi 0x%llX\n", GuestRegs->esi));
  Print (("rdi 0x%llX\n", GuestRegs->edi));

  //Print (("r8 0x%llX\n", GuestRegs->r8));
  //Print (("r9 0x%llX\n", GuestRegs->r9));
  //Print (("r10 0x%llX\n", GuestRegs->r10));
  //Print (("r11 0x%llX\n", GuestRegs->r11));
  //Print (("r12 0x%llX\n", GuestRegs->r12));
  //Print (("r13 0x%llX\n", GuestRegs->r13));
  //Print (("r14 0x%llX\n", GuestRegs->r14));
  //Print (("r15 0x%llX\n", GuestRegs->r15));
  //Print (("Guest MSR_EFER Read 0x%llx \n", Cpu->Vmx.GuestEFER));
  //CmGetPagePaByPageVaCr3 (
  //    Cpu->SparePage, 
  //    Cpu->SparePagePTE,
  //    (ULONG)VmxRead (GUEST_CR3), 
  //    (ULONG)VmxRead (GUEST_RIP), 
  //    &pa);
  //Print (("VmxCrash() IOA: Failed to map PA 0x%p to VA 0x%p\n", pa.QuadPart, Cpu->SparePage));
#endif

#if DEBUG_LEVEL>2
  if (!NT_SUCCESS (Status = CmPatchPTEPhysicalAddress (Cpu->SparePagePTE, Cpu->SparePage, pa))) {
    Print (("VmxCrash() IOA: Failed to map PA 0x%p to VA 0x%p, status 0x%08hX\n", pa.QuadPart, Cpu->SparePage,
               Status));
  }
  DumpMemory ((PUCHAR)
              (((ULONG64) Cpu->SparePage) | ((VmxRead (GUEST_RIP) - 0x10) & 0xfff)), 0x50);
#endif
  while (1);
}
