#include <inc/stdio.h>
#include <arch/cpu.h>
#include <arch/x86.h>
#include <arch/bitops.h>
#include <arch/cpufeature.h>

/*
 * Print CPU feature informations.
 */
int 
cpuinfo( void )
{
	uint32_t 	ecx, edx;

	/* Get CPU feature information from ECX and EDX registers. */
	cpuid(1, 0, 0, &ecx, &edx); 

	/* Detect CPU information from ECX */
	// Streaming SIMD Extensions 3.
	cprintf("SSE3: ");
	if ( !test_bit(X86_FEATURE_SSE3, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// MONITOR/MWAIT.
	cprintf("MONITOR: ");
	if ( !test_bit(X86_FEATURE_MWAIT, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// CPL Qualified Debug Store.
	cprintf("DS-CPL: ");
	if ( !test_bit(X86_FEATURE_DSCPL, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\n");

	// Virtual Machine Extensions.
	cprintf("VMX: ");
	if ( !test_bit(X86_FEATURE_VMX, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// Safer Mode Extensions.
	cprintf("SMX: ");
	if ( !test_bit(X86_FEATURE_SMX, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// Enhanced Intel SpeedStep technology.
	cprintf("EST: ");
	if ( !test_bit(X86_FEATURE_EST, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\n");

	// Thermal Monitor 2.
	cprintf("TM2: ");
	if ( !test_bit(X86_FEATURE_TM2, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// Supplemental Streaming SIMD Extensions 3.
	cprintf("SSSE3: ");
	if ( !test_bit(X86_FEATURE_SSSE3, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// L1 Context ID.
	cprintf("CNXT-ID: ");
	if ( !test_bit(X86_FEATURE_CID, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\n");

	// CMPXCHEG16B.
	cprintf("CMPXCHG16B: ");
	if ( !test_bit(X86_FEATURE_CX16, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t");

	// xTPR Update Control.
	cprintf("xTPR: ");
	if ( !test_bit(X86_FEATURE_XTPR, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// Perfmon and Debug Capability.
	cprintf("PDCM: ");
	if ( !test_bit(X86_FEATURE_PDCM, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\n");

	// DCA.
	cprintf("DCA: ");
	if ( !test_bit(X86_FEATURE_DCA, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// SSE4.1.
	cprintf("SSE4.1: ");
	if ( !test_bit(X86_FEATURE_SSE4_1, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\t\t");

	// SSE4.2.
	cprintf("SSE4.2: ");
	if ( !test_bit(X86_FEATURE_SSE4_2, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\n");

	// POPCNT.
	cprintf("POPCNT: ");
	if ( !test_bit(X86_FEATURE_POPCNT, &ecx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
	cprintf("\n");

	/* Detect CPU information from EDX */
/*	// .
	cprintf(": ");
	if ( !test_bit(X86_FEATURE_, &edx) ) {
		cprintf("No.");
	} else {
		cprintf("Yes.");
	}//if...else
*/
	return 0;
}//cpuinfo()
