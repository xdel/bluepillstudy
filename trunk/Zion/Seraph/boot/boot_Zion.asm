%include	"include/asm_prot_mod.inc"		; Constance and macros

org		0x8200
	jmp	LABEL_BEGIN

MEM_INFO_LEN 	equ 	512


[SECTION .gdt] 		; GDT segment
;----------<<< GDT >>>----------
; 										Base, 		Limit, 				Attribute
LABEL_GDT:				Descriptor 		0, 			0, 					0					; 空描述符
LABEL_DESC_CODE32:		Descriptor 		0, 			SegCode32Len - 1, 	DA_C + DA_32		; 非一致代码段, 32位
LABEL_DESC_DATA:		Descriptor 		0, 			DataLen - 1, 		DA_DRW				; Data
LABEL_DESC_STACK:		Descriptor 		0, 			TopOfStack, 		DA_DRWA + DA_32		; Stack, 32位
LABEL_DESC_VIDEO:		Descriptor 		0xb8000, 	0xffff, 			DA_DRW				; 显存
;-------------------------------

GdtLen 		equ		$ - LABEL_GDT	; GDT长度
GdtPtr		dw 		GdtLen - 1		; GDT界限
			dd		0				; GDT基地址

;----------<<< GDT Selector >>>----------
SelectorCode32		equ	LABEL_DESC_CODE32	- LABEL_GDT
SelectorData		equ	LABEL_DESC_DATA		- LABEL_GDT
SelectorStack		equ	LABEL_DESC_STACK	- LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO	- LABEL_GDT
;----------------------------------------



[SECTION .data] 	; Data segment
ALIGN	32
[BITS	32]
LABEL_DATA:
;----- Symbols in real mode -----
; Strings
_szPMMessage:			db	"In Protect Mode now.", 0x0a, 0x0a, 0	
_szMemChkTitle:			db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0x0a, 0
_szRAMSize				db	"RAM size:", 0
_szReturn				db	0x0a, 0

; Variables
_dwMCRNumber:			dd	0					; Memory Check Result
_dwDispPos:				dd	(80 * 6 + 0) * 2	; Screen row 6, column 0
_dwMemSize:				dd	0
_ARDStruct:		; Address Range Descriptor Structure
	_dwBaseAddrLow:		dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:			dd	0
_MemChkBuf:				times MEM_INFO_LEN	db	0

;----- Symbols in protected mode -----
szPMMessage				equ		_szPMMessage	- $$
szMemChkTitle			equ		_szMemChkTitle	- $$
szRAMSize				equ		_szRAMSize		- $$
szReturn				equ		_szReturn		- $$
dwDispPos				equ		_dwDispPos		- $$
dwMemSize				equ		_dwMemSize		- $$
dwMCRNumber				equ		_dwMCRNumber	- $$
ARDStruct				equ		_ARDStruct		- $$
	dwBaseAddrLow		equ		_dwBaseAddrLow	- $$
	dwBaseAddrHigh		equ		_dwBaseAddrHigh	- $$
	dwLengthLow			equ		_dwLengthLow	- $$
	dwLengthHigh		equ		_dwLengthHigh	- $$
	dwType				equ		_dwType			- $$
MemChkBuf				equ		_MemChkBuf		- $$

DataLen					equ		$ - LABEL_DATA



[SECTION .gs] 		; Global stack segment
ALIGN	32
[BITS	32]
LABEL_STACK:
	times 	512 	db 0

TopOfStack			equ		$ - LABEL_STACK - 1



[SECTION .s16] 		; 16-bit code segment
[BITS	16]
LABEL_BEGIN:
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, 0x8000

	; Get memory information.
	mov		ebx, 0
	mov		di, _MemChkBuf
.loop:
	mov		eax, 0xe820
	mov		ecx, 20
	mov		edx, 0x534d4150 		; "SMAP"
	int		0x15
	jc		LABEL_MEM_CHK_FAIL
	add		di, 20
	inc		dword [_dwMCRNumber]
	cmp		ebx, 0
	jne		.loop
	jmp		LABEL_MEM_CHK_OK

LABEL_MEM_CHK_FAIL:
	mov		dword [_dwMCRNumber], 0

LABEL_MEM_CHK_OK:

	; Initialize 32-bit code descriptor.
	xor		eax, eax
	mov		ax, cs
	shl		eax, 4
	add		eax, LABEL_SEG_CODE32
	mov		word [LABEL_DESC_CODE32 + 2], ax
	shr		eax, 16
	mov		byte [LABEL_DESC_CODE32 + 4], al
	mov		byte [LABEL_DESC_CODE32 + 7], ah

	; Initialize data segment descriptor.
	xor		eax, eax
	mov		ax, ds
	shl		eax, 4
	add		eax, LABEL_DATA
	mov		word [LABEL_DESC_DATA + 2], ax
	shr		eax, 16
	mov		byte [LABEL_DESC_DATA + 4], al
	mov		byte [LABEL_DESC_DATA + 7], ah

	; Initialize stack segment descriptor.
	xor		eax, eax
	mov		ax, ds
	shl		eax, 4
	add		eax, LABEL_STACK
	mov		word [LABEL_DESC_STACK + 2], ax
	shr		eax, 16
	mov		byte [LABEL_DESC_STACK + 4], al
	mov		byte [LABEL_DESC_STACK + 7], ah

	; Prepare for loading GDTR.
	xor		eax, eax
	mov		ax, ds
	shl		eax, 4
	add		eax, LABEL_GDT						; eax <- gdt 基地址
	mov		dword [GdtPtr + 2], eax				; [GdtPtr + 2] <- gdt 基地址

	; Load GDTR
	lgdt	[GdtPtr]

	cli 						; Close interrupt.

	; Open A20 gate.
	in		al, 0x92
	or		al, 00000010b
	out		0x92, al

	; Prepare to switch to protected mode.
	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax

	; Jump to protected mode.
	jmp		dword SelectorCode32:0			; Load SelectorCode32 into cs, 
											; jump to Code32Selector:0




[SECTION .s32] 		; 32-bit code segment
[BITS	32]
LABEL_SEG_CODE32:
	mov		ax, SelectorData
	mov		ds, ax					; Data segment selector.
	mov		ax, SelectorData
	mov		es, ax
	mov		ax, SelectorVideo
	mov		gs, ax					; Video segment selector.

	mov		ax, SelectorStack
	mov		ss, ax					; Stack segment selector.

	mov		esp, TopOfStack


	; Show message indicating protected mode.
	push	szPMMessage
	call	DispStr
	add		esp, 4

	; Table title of memory layout
	push	szMemChkTitle
	call	DispStr
	add		esp, 4

	call	DispMemSize				; Display memory layout infomation.

	jmp 	$ 						; Spin forever.



DispMemSize:
	push	esi
	push	edi
	push	ecx

	mov		esi, MemChkBuf
	mov		ecx, [dwMCRNumber] 		; for(int i=0;i<[MCRNumber];i++)//每次得到一个ARDS
.loop:				  				; {
	mov		edx, 5		  			; 	for(int j=0;j<5;j++) //每次得到一个ARDS中的成员
	mov		edi, ARDStruct	  		; 	{//依次显示BaseAddrLow,BaseAddrHigh,LengthLow,
.1:				  					; 		LengthHigh,Type
	push	dword [esi]	  			;
	call	DispInt		  			;    DispInt(MemChkBuf[j*4]); //显示一个成员
	pop		eax		  				;
	stosd			  				;    ARDStruct[j*4] = MemChkBuf[j*4];
	add		esi, 4		  			;
	dec		edx		  				;
	cmp		edx, 0		  			;
	jnz		.1		  				;  }
	call	DispReturn	  			;  printf("\n");
	cmp		dword [dwType], 1 		;  if(Type == AddressRangeMemory)
	jne		.2		  				;  {
	mov		eax, [dwBaseAddrLow]	;
	add		eax, [dwLengthLow]		;
	cmp		eax, [dwMemSize]  		;    if(BaseAddrLow + LengthLow > MemSize)
	jb		.2		  				;
	mov		[dwMemSize], eax  		;    MemSize = BaseAddrLow + LengthLow;
.2:				  					;  }
	loop	.loop		  			; }
				  					;
	call	DispReturn	  			; printf("\n");
	push	szRAMSize	  			;
	call	DispStr		  			; printf("RAM size:");
	add	esp, 4		  				;
				  					;
	push	dword [dwMemSize] 		;
	call	DispInt		  			; DispInt(MemSize);
	add		esp, 4		  			;

	pop		ecx
	pop		edi
	pop		esi
	ret

%include	"include/asm_lib.inc"

SegCode32Len	equ		$ - LABEL_SEG_CODE32


