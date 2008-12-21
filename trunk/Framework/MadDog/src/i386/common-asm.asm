.686p
.model flat,StdCall
option casemap:none

EXTERN	 HvmSubvertCpu@4:PROC  
EXTERN	 HvmResumeGuest@0 : PROC



CM_SAVE_ALL_NOSEGREGS MACRO
        push edi
        push esi
        push ebp
        push ebp ;        push esp
        push ebx
        push edx
        push ecx
        push eax
ENDM

CM_RESTORE_ALL_NOSEGREGS MACRO
        pop eax
        pop ecx
        pop edx
        pop ebx
        pop ebp ;        pop esp
        pop ebp
        pop esi
        pop edi        
ENDM


.CODE

CmDebugBreak PROC
	int	3
	ret
CmDebugBreak ENDP


; CmReloadGdtr (PVOID GdtBase (rcx), ULONG GdtLimit (rdx) );

CmReloadGdtr PROC StdCall _GdtBase,_GdtLimit
	push	_GdtBase
	shl	_GdtLimit, 16
	push	_GdtLimit
	lgdt	fword ptr [esp+2]	; do not try to modify stack selector with this ;)
	pop	eax
	pop	eax
	ret
CmReloadGdtr ENDP

; CmReloadIdtr (PVOID IdtBase (rcx), ULONG IdtLimit (rdx) );

CmReloadIdtr PROC StdCall _IdtBase,_IdtLimit
	push	_IdtBase
	shl	_IdtLimit, 16
	push	_IdtLimit
	lidt	fword ptr [esp+2]	; do not try to modify stack selector with this ;)
	pop	eax
	pop	eax
	ret
CmReloadIdtr ENDP

; CmSubvert (PVOID  GuestRsp);
CmSubvert PROC StdCall _GuestRsp


	CM_SAVE_ALL_NOSEGREGS


	mov	eax,esp
	push	eax        ;setup esp to argv[0]
	call	HvmSubvertCpu@4
	ret

CmSubvert ENDP

; CmSlipIntoMatrix (PVOID  GuestRsp);
CmSlipIntoMatrix PROC StdCall _GuestRsp
	pop	ebp	
	call   HvmResumeGuest@0
	CM_RESTORE_ALL_NOSEGREGS	
	ret

CmSlipIntoMatrix ENDP

CmCli PROC
	cli
	ret
CmCli  ENDP

CmSti PROC
	sti
	ret
CmSti ENDP

END
