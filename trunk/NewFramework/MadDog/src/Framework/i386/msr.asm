.686p
.model flat,StdCall
option casemap:none
.CODE

; MsrRead (ULONG32 reg );

MsrRead PROC StdCall _reg
	mov		ecx, _reg
	rdmsr				; MSR[ecx] --> edx:eax
	ret
MsrRead ENDP

; MsrWrite (ULONG32 reg , ULONG64 MsrValue );

MsrWrite PROC StdCall _reg, _MsrValue_low,_MsrValue_high
	mov		eax, _MsrValue_low
	mov		edx, _MsrValue_high
	mov		ecx, _reg
	wrmsr
	ret
MsrWrite ENDP

; MsrSafeWrite (ULONG32 reg , ULONG32 eax , ULONG32 edx );
MsrSafeWrite PROC StdCall _reg, _MsrValue_low,_MsrValue_high
	mov		eax, _MsrValue_low
	mov		edx, _MsrValue_high
	mov		ecx, _reg
	wrmsr
	ret
MsrSafeWrite ENDP

; MsrReadWithEax (PULONG32 reg (rcx), PULONG32 eax (rdx), PULONG32 edx (r8));

MsrReadWithEaxEdx PROC StdCall _reg,_eax,_edx
	mov		eax, dword ptr [_eax]
	mov		ecx, dword ptr [_reg]
	mov		edx, dword ptr [_edx]

	rdmsr				; MSR[ecx] --> edx:eax
	
	mov		[_edx], edx	
	mov		[_eax], eax
	mov		[_reg], ecx

	ret
MsrReadWithEaxEdx ENDP


END
