/*
 * Defines x86 CPU feature bits
 */

#ifndef __I386_CPUFEATURE_H
#define __I386_CPUFEATURE_H


/* Intel-defined CPU features, CPUID level 0x00000001 (ecx) */
#define X86_FEATURE_SSE3		0 	/* Streaming SIMD Extensions-3 */
#define X86_FEATURE_DTES64		2 	/* 64-bit Debug Store */
#define X86_FEATURE_MWAIT		3 	/* Monitor/Mwait support */
#define X86_FEATURE_DSCPL		4 	/* CPL Qualified Debug Store */
#define X86_FEATURE_VMX			5 	/* Virtual Machine Extensions */
#define X86_FEATURE_SMX			6 	/* Safer Mode Extensions */
#define X86_FEATURE_EST			7 	/* Enhanced SpeedStep */
#define X86_FEATURE_TM2			8 	/* Thermal Monitor 2 */
#define X86_FEATURE_SSSE3		9 	/* Supplemental Streaming SIMD Extensions-3 */
#define X86_FEATURE_CID			10 	/* Context ID */
#define X86_FEATURE_CX16    	13 	/* CMPXCHG16B */
#define X86_FEATURE_XTPR		14 	/* Send Task Priority Messages */
#define X86_FEATURE_PDCM		15 	/* Perf/Debug Capability MSR */
#define X86_FEATURE_DCA			18 	/* Direct Cache Access */
#define X86_FEATURE_SSE4_1		19 	/* Streaming SIMD Extensions 4.1 */
#define X86_FEATURE_SSE4_2		20 	/* Streaming SIMD Extensions 4.2 */
#define X86_FEATURE_X2APIC		21 	/* Extended xAPIC */
#define X86_FEATURE_POPCNT		23 	/* POPCNT instruction */
#define X86_FEATURE_XSAVE		26 	/* XSAVE/XRSTOR/XSETBV/XGETBV */
#define X86_FEATURE_HYPERVISOR	31 	/* Running under some hypervisor */


/* Intel-defined CPU features, CPUID level 0x00000001 (edx) */
#define X86_FEATURE_FPU			0 	/* Onboard FPU */
#define X86_FEATURE_VME			1 	/* Virtual Mode Extensions */
#define X86_FEATURE_DE			2 	/* Debugging Extensions */
#define X86_FEATURE_PSE 		3 	/* Page Size Extensions */
#define X86_FEATURE_TSC			4 	/* Time Stamp Counter */
#define X86_FEATURE_MSR			5 	/* Model-Specific Registers, RDMSR, WRMSR */
#define X86_FEATURE_PAE			6 	/* Physical Address Extensions */
#define X86_FEATURE_MCE			7 	/* Machine Check Architecture */
#define X86_FEATURE_CX8			8	/* CMPXCHG8 instruction */
#define X86_FEATURE_APIC		9	/* Onboard APIC */
#define X86_FEATURE_SEP			11 	/* SYSENTER/SYSEXIT */
#define X86_FEATURE_MTRR		12 	/* Memory Type Range Registers */
#define X86_FEATURE_PGE			13 	/* Page Global Enable */
#define X86_FEATURE_MCA			14 	/* Machine Check Architecture */
#define X86_FEATURE_CMOV		15 	/* CMOV instruction (FCMOVCC and FCOMI too if FPU present) */
#define X86_FEATURE_PAT			16 	/* Page Attribute Table */
#define X86_FEATURE_PSE36		17 	/* 36-bit PSEs */
#define X86_FEATURE_PN			18 	/* Processor serial number */
#define X86_FEATURE_CLFLSH		19 	/* Supports the CLFLUSH instruction */
#define X86_FEATURE_DS			21 	/* Debug Store */
#define X86_FEATURE_ACPI		22 	/* ACPI via MSR */
#define X86_FEATURE_MMX			23 	/* Multimedia Extensions */
#define X86_FEATURE_FXSR		24 	/* FXSAVE and FXRSTOR instructions (fast save and restore */
									/* of FPU context), and CR4.OSFXSR available */
#define X86_FEATURE_XMM			25 	/* Streaming SIMD Extensions */
#define X86_FEATURE_XMM2		26 	/* Streaming SIMD Extensions-2 */
#define X86_FEATURE_SELFSNOOP	27 	/* CPU self snoop */
#define X86_FEATURE_HT			28 	/* Hyper-Threading */
#define X86_FEATURE_ACC			29 	/* Automatic clock control */
#define X86_FEATURE_IA64		30 	/* IA-64 processor */
#define X86_FEATURE_PBE			31 	/* Pending Break Enable */


#endif /* __I386_CPUFEATURE_H */

