.CODE

; MsrRead (ULONG32 reg (rcx));

MsrRead PROC
;	xor		rax, rax        //rax清零
	rdmsr				; MSR[ecx] --> edx:eax //Loads the contents of a 64-bit model-specific register (MSR) specified in the ECX register into
                                                               registers EDX:EAX. The EDX register receives the high-order 32 bits and the EAX register receives
                                                                 the low order bits.
	shl		rdx, 32                    //rdx左移32位
	or		rax, rdx                   //结果放在rax
	ret
MsrRead ENDP

; MsrWrite (ULONG32 reg (rcx), ULONG64 MsrValue (rdx));

MsrWrite PROC
	mov		rax, rdx
	shr		rdx, 32                    //rdx右移32位
	wrmsr                                    //This instruction writes the contents of the EDX:EAX register pair into a 64-bit model-specific register
                                                   specified in the ECX register.                                    
	ret
MsrWrite ENDP

; MsrSafeWrite (ULONG32 reg (rcx), ULONG32 eax (rdx), ULONG32 edx (r8));

MsrSafeWrite PROC
	mov		rax, rdx
	mov		rdx, r8
	xor		r8, r8
	wrmsr
	mov		rax, r8		; r8 will be set to STATUS_UNSUCCESSFUL if there is a fault
	ret
MsrSafeWrite ENDP

; MsrReadWithEax (PULONG32 reg (rcx), PULONG32 eax (rdx), PULONG32 edx (r8));

MsrReadWithEaxEdx PROC
	mov		r9, rdx
	mov		r10, rcx
	mov		eax, dword ptr [rdx]          //强制类型转化
	mov		ecx, dword ptr [rcx]
	mov		edx, dword ptr [r8]
	rdmsr				; MSR[ecx] --> edx:eax
	mov		[r8], edx	
	mov		[r9], eax
	mov		[r10], ecx
	ret
MsrReadWithEaxEdx ENDP


END
