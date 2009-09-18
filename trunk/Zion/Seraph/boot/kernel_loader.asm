%include	"include/load.inc"
%include	"include/hdd.inc"
%include	"include/elf.inc"
%include	"include/pm.inc"

org		OffsetOfLoader

	jmp		LABEL_START		; Start

; GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------
;															Base 			Limit 			Attribute
LABEL_GDT:						Descriptor 	0, 				0, 				0														; Null
LABEL_DESC_FLAT_C:		Descriptor 	0, 				0xFFFFFF,	DA_CR    | DA_32 | DA_LIMIT_4K		; Code, 0 ~ 4G
LABEL_DESC_FLAT_RW:	Descriptor 	0, 				0xFFFFFF,	DA_DRW | DA_32 | DA_LIMIT_4K		; Data, 0 ~ 4G
LABEL_DESC_VIDEO:		Descriptor 	0xb8000, 	0xffff, 			DA_DRW | DA_DPL3	 						; Video
; GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------

GdtLen		equ		$ - LABEL_GDT
GdtPtr		dw		GdtLen - 1												; Segment limit
				dd		BaseOfLoader_PhyAddr + LABEL_GDT	; Segment base

; GDT Selector ----------------------------------------------------------------------------------
SelectorFlatC		equ		LABEL_DESC_FLAT_C		- LABEL_GDT
SelectorFlatRW	equ		LABEL_DESC_FLAT_RW	- LABEL_GDT
SelectorVideo		equ		LABEL_DESC_VIDEO		- LABEL_GDT
; GDT Selector ----------------------------------------------------------------------------------



BaseOfStack	equ	OffsetOfLoader

; 16-bit 实模式代码 ----------------------------------------------------------
[SECTION .s16]
[BITS 16]
LABEL_START:
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, BaseOfStack

	call 		GetMemInfo 						; 获取内存布局
	
	call 		LoadKernelFile 					; 载入kernel ELF 文件

	; Get ready for protect-mode
	lgdt		[GdtPtr] 								; Load GDTR

	mov 	ax, 0x0003							; BIOS video mode
	int 		0x10
	
	mov 	ah, 0x02								; Set cursor position to
	mov 	bh, 0x0 								; page 0, 
	mov 	dx, 0x0000 							; row 0 and column 0
	int 		0x10 									; int 10h, ah=2

	; Turn on A20 Gate
	cli 													; Close interrupt
	; // 方法一：
	in			al, 0x92
	or			al, 00000010b
	out		0x92, al
	
	; // 方法二：
;	call 			Wait_8042
;	mov 		al , 0xd1
;	out 			0x64 , al
;	call 			Wait_8042
;	mov 		al , 0xdf
;	out 			0x60 , al
;	call 			Wait_8042
	
	mov			eax, cr0 				; Trun on protect-mode
	or				eax, 1
	mov			cr0, eax
	
	; Switich to protect-mode
	jmp		dword SelectorFlatC:(BaseOfLoader_PhyAddr+LABEL_PM_START)

;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



; 获取内存布局 ----------------------------------------------------------
GetMemInfo:
	mov		ebx, 0						; ebx = 后续值, 开始时需为 0
	mov		di, _MemChkBuf		; es:di 指向一个地址范围描述符结构（Address Range Descriptor Structure）
.MemChkLoop:
	mov		eax, 0xe820				; eax = 0000E820h
	mov		ecx, 20					; ecx = 地址范围描述符结构的大小
	mov		edx, 0x534D4150	; edx = 'SMAP'
	int		0x15						; int 15h
	jc			.MemChkFail
	add		di, 20
	inc		dword [_dwMCRNumber]	; dwMCRNumber = ARDS 的个数
	cmp		ebx, 0
	jne		.MemChkLoop
	jmp		.MemChkOK
.MemChkFail:
	mov		dword [_dwMCRNumber], 0
.MemChkOK:

	ret


	
; Load kernel file (origin ELF file) -------------------------------
; NOTE: 调用int 13h, ah=0x42，每次读入1 个（或8 个）扇区；填充内存0x10000~0x7FFFF。
LoadKernelFile:
	mov 		si, DAP_struct
	mov 		ecx, StartSecOfKernalFile		; Start from logic sector StartSecOfKernalFile
	mov 		dx, BaseOfKernelFile				; 起始内存段地址
	mov 		bx, OffsetOfKernelFile				; 起始内存偏移地址
.loop_load_kernel:
	push 		dx
	mov 		[DAP_SecStart], ecx
	mov 		[DAP_Mem_seg], dx
	mov 		[DAP_Mem_off], bx
;	mov 		al, 1
	mov 		al, 64
	mov 		[DAP_SecNum], al
	call 			ReadSector_LBA
	pop 		dx
	
;	inc 			ecx 										; 硬盘便宜1 个扇区
	add			ecx, 8 									; 硬盘偏移8 个扇区
;	add			bx, 0x200 							; 内存偏移地址递增512B
	add 			bx, 0x1000 							; 内存偏移地址递增4KB
	cmp 		bx, 0x0000
	jne 			.loop_load_kernel

	add 			dx, 0x1000							; 内存段地址递增
	cmp 		dx, LimitOfKernelFile				; 判断是否到达加载地址的段界限
	jne 			.loop_load_kernel

	ret



; LBA 模式读取硬盘扇区
ReadSector_LBA:
	mov 		dl, DrvNum 							; dl = 驱动器号
	mov 		ah, 0x42 								; ah = 42h
	int 			0x13									; int 13h
	jc 				ReadSector_LBA 					; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止
	ret



Wait_8042:
	in 			al , 0x64
	test 			al , 0x2
	jnz 			Wait_8042
	ret



; 32-bit 保护模式代码段 --------------------------------------------------
[SECTION .s32]
ALIGN	32
[BITS	32]

LABEL_PM_START:
	mov		ax, SelectorVideo
	mov		gs, ax
	mov		ax, SelectorFlatRW
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		ss, ax
	mov		esp, TopOfStack

;	push	szMemChkTitle							; 显示内存信息标题
;	call		DispStr
;	add		esp, 4

	call		DispMemInfo 								; 显示内存信息

	mov 	eax, [dwMemSize] 						; 将内存大小写入到指定地址
	mov 	[MemSize_PhyAddr], eax

	mov 	eax, [dwMCRNumber] 				; 将内存布局信息的条目数写入到指定地址
	mov 	[MCRNumber_PhyAddr], eax
	
	mov 		eax, MEMCHKBUF_SIZE 			; 将内存布局信息写入指定地址
	push		eax											; size
	mov			eax, MemChkBuf	
	push		eax											; src
	mov 		eax, MemInfo_PhyAddr			; dst
	push 		eax
	call			MemCpy
	add			esp, 12

	call			InitKernel 									; 解析kernel 的ELF 格式，将kernel 代码在内存中重定位

	; Switch to kernel.
	mov 		eax, [BaseOfKernelFile_PhyAddr + e_entry]
	and 			eax, PAddrMask						; Physical address mask
	jmp			eax											; 进入内核
	
;	jmp		SelectorFlatC:KernelEntryPoint_PhyAddr		; 进入内核

%include "include/asm_pm_lib.inc"


; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; ------------------------------------------------------------------------
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
MemCpy:
	push		ebp
	mov			ebp, esp
	push		esi
	push		edi
	push		ecx

	mov			edi, [ebp + 8]			; Destination
	mov			esi, [ebp + 12]			; Source
	mov			ecx, [ebp + 16]		; Counter
.1:
	cmp			ecx, 0						; 判断计数器
	jz				.2								; 计数器为零时跳出

	mov			al, [ds:esi]				; ┓
	inc			esi							; ┃
													; ┣ 逐字节移动
	mov			byte [es:edi], al		; ┃
	inc			edi							; ┛

	dec			ecx							; 计数器减一
	jmp			.1								; 循环
.2:
	mov			eax, [ebp + 8]			; 返回值

	pop			ecx
	pop			edi
	pop			esi
	mov			esp, ebp
	pop			ebp
	ret
; MemCpy 结束-------------------------------------------------------------



; 显示内存信息 --------------------------------------------------------------
DispMemInfo:
	push		esi
	push		edi
	push		ecx

	mov			esi, MemChkBuf
	mov			ecx, [dwMCRNumber]		;for(int i=0;i<[MCRNumber];i++) 
.loop:													;{// 每次得到一个ARDS(Address Range Descriptor Structure)结构
	mov			edx, 5								;	for(int j=0;j<5;j++)	// 每次得到一个ARDS中的成员，共5个成员
	mov			edi, ARDStruct					;	{	// 依次显示：BaseAddrLow，BaseAddrHigh，LengthLow，LengthHigh，Type
.1:														;
;	push		dword [esi]						;
;	call			DispInt								;		DispInt(MemChkBuf[j*4]); // 显示一个成员
;	pop			eax									;
	mov 		eax, [esi]							;		(注：不输出内存布局，以此句替代。)
	stosd												;		ARDStruct[j*4] = MemChkBuf[j*4];
	add			esi, 4								;
	dec			edx									;
	cmp			edx, 0								;
	jnz			.1										;	}
;	call			DispReturn						;	printf("\n");
	cmp			dword [dwType], 1			;	if(Type == AddressRangeMemory) // AddressRangeMemory : 1, AddressRangeReserved : 2
	jne			.2										;	{
	mov			eax, [dwBaseAddrLow]	;
	add			eax, [dwLengthLow]		;
	cmp			eax, [dwMemSize]			;		if(BaseAddrLow + LengthLow > MemSize)
	jb				.2										;
	mov			[dwMemSize], eax			;			MemSize = BaseAddrLow + LengthLow;
.2:														;	}
	loop			.loop								;}
															;
;	call			DispReturn						;printf("\n");
	push		szRAMSize						;
	call			DispStr								;printf("RAM size: ");
	add			esp, 4								;
															;
	push		dword [dwMemSize]		;
	call			DispInt								;DispInt(MemSize);
	add			esp, 4								;

	pop			ecx
	pop			edi
	pop			esi
	ret
; ---------------------------------------------------------------------------



; InitKernel ---------------------------------------------------------------------------------
; 将 kernel file 的内容经过ELF 格式解析后重定位
; --------------------------------------------------------------------------------------------
InitKernel:	
	; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
	xor			esi, esi
	mov			cx, word [BaseOfKernelFile_PhyAddr + e_phnum]	; ┓ ecx <- pELFHdr->e_phnum
	movzx		ecx, cx																		; ┛
	mov			esi, [BaseOfKernelFile_PhyAddr + e_phoff]				; esi <- pELFHdr->e_phoff
	add			esi, BaseOfKernelFile_PhyAddr								; esi <- OffsetOfKernel + pELFHdr->e_phoff
.LoadSegment_Begin:
	mov			eax, [esi + p_type]
	cmp			eax, 0																; PT_NULL
	jz				.LoadSegment_NoAction
	push		dword [esi + p_filesz]								; size	┓
	mov			eax, [esi + p_offset]										;	┃
	add			eax, BaseOfKernelFile_PhyAddr						;	┣ ::memcpy(	(void*)(pPHdr->p_vaddr),
	push		eax																; src	┃		uchCode + pPHdr->p_offset,
	mov 		eax, [esi + p_vaddr]									; dst	┃		pPHdr->p_filesz;
	and 			eax, PAddrMask										; Physical address mask
	push 		eax
	call			MemCpy															;	┃
	add			esp, 12															;	┛
.LoadSegment_NoAction:
	xor 			eax, eax 													  		; esi += pELFHdr->e_phentsize
	mov 		ax, word [BaseOfKernelFile_PhyAddr + e_phentsize]
	add			esi, eax 
	dec			ecx
	jnz			.LoadSegment_Begin
	

	; 遍历每一个 Section Header，将.bss 段清零
	xor 			edx, edx
	mov 		dx, word [BaseOfKernelFile_PhyAddr + e_shnum]
	xor 			esi, esi
	mov			esi, [BaseOfKernelFile_PhyAddr + e_shoff]	
	add			esi, BaseOfKernelFile_PhyAddr 
	xor 			bl, bl
.ConfigSection_Begin:
	mov			eax, dword [esi + sh_type]
	cmp			eax, SHT_NOBITS 
	jne			.ConfigSection_NoAction
	mov 		eax, dword [esi + sh_addr] 
	and 			eax, PAddrMask												; Physical address mask
	mov 		ecx, dword [esi + sh_size]
.ConfigSection_ClearLoop:
	mov 		byte [eax], bl
	inc 			eax
	loop 		.ConfigSection_ClearLoop
.ConfigSection_NoAction:
	xor 			eax, eax 													  		; esi += pELFHdr->e_phentsize
	mov 		ax, word [BaseOfKernelFile_PhyAddr + e_shentsize]
	add			esi, eax 
	dec			edx
	jnz			.ConfigSection_Begin

	ret
; InitKernel ^^^^^^^^^^^^^^^^^^^^^^^^^^^^



; 数据段
[SECTION .data]
ALIGN	32
LABEL_DATA:
; 实模式下使用这些符号
; 字符串
_szMemChkTitle:			db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_szRAMSize:					db	"RAM size: ", 0
_szReturn:					db	0Ah, 0
;; 变量
_dwMCRNumber:			dd	0	; Memory Check Result
_dwDispPos:				dd	(80 * 0 + 0) * 2	; 屏幕第 1 行, 第 0 列。
_dwMemSize:				dd	0
_ARDStruct:		; Address Range Descriptor Structure
	_dwBaseAddrLow:	dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:					dd	0
_MemChkBuf:				times	MEMCHKBUF_SIZE	db	0

DAP_struct:		; 被int 13h, ah=42h 调用的Disk Address Packet (DAP) 结构体
	DAP_Size: 				db 	16 		; size of DAP
	DAP_unused_1:		db 	0 			; unused, should be zero
	DAP_SecNum: 			db 	1 			; number of sectors to be read, 0..127
	DAP_unused_2: 		db 	0 			; unused, should be zero
	DAP_Mem_off: 			dw 	0 			; offset value of memory buffer to which sectors will be transferred
	DAP_Mem_seg: 		dw 	0 			; segment value of memory buffer to which sectors will be transferred
	DAP_SecStart: 			dd 	0, 0 		; absolute number of the start of the sectors to be read 
														; (1st sector of drive has number 0)
	
;
;; 保护模式下使用这些符号
szMemChkTitle				equ		BaseOfLoader_PhyAddr + _szMemChkTitle
szRAMSize					equ		BaseOfLoader_PhyAddr + _szRAMSize
szReturn						equ		BaseOfLoader_PhyAddr + _szReturn
dwDispPos					equ		BaseOfLoader_PhyAddr + _dwDispPos
dwMemSize					equ		BaseOfLoader_PhyAddr + _dwMemSize
dwMCRNumber			equ		BaseOfLoader_PhyAddr + _dwMCRNumber
ARDStruct					equ		BaseOfLoader_PhyAddr + _ARDStruct
	dwBaseAddrLow		equ		BaseOfLoader_PhyAddr + _dwBaseAddrLow
	dwBaseAddrHigh		equ		BaseOfLoader_PhyAddr + _dwBaseAddrHigh
	dwLengthLow			equ		BaseOfLoader_PhyAddr + _dwLengthLow
	dwLengthHigh			equ		BaseOfLoader_PhyAddr + _dwLengthHigh
	dwType					equ		BaseOfLoader_PhyAddr + _dwType
MemChkBuf					equ		BaseOfLoader_PhyAddr + _MemChkBuf


; 堆栈在数据段的末尾
StackSpace:					times	SizeOfStack		db	0
TopOfStack					equ		BaseOfLoader_PhyAddr + $	; 栈顶
