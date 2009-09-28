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
GdtPtr:		dw		GdtLen - 1												; Segment limit
				dd		BaseOfLoader_PhyAddr + LABEL_GDT	; Segment base

; GDT Selector ----------------------------------------------------------------------------------
SelectorFlatC		equ		LABEL_DESC_FLAT_C		- LABEL_GDT
SelectorFlatRW		equ		LABEL_DESC_FLAT_RW	- LABEL_GDT
SelectorVideo		equ		LABEL_DESC_VIDEO		- LABEL_GDT
; GDT Selector ----------------------------------------------------------------------------------



; 16-bit 实模式代码 ----------------------------------------------------------
[SECTION .s16]
[BITS 16]
LABEL_START:
	mov		ax, cs
	mov		ds, ax
	mov		ss, ax
	mov		sp, OffsetOfLoader
	
	call 		GetMemInfo 						; 获取内存布局

%ifndef __LOAD_KERNEL_IN_PM__
	call 		LoadKernelFile 						; 载入kernel ELF 文件
%endif

	; Get ready for protect-mode
	lgdt		[GdtPtr] 								; Load GDTR

	mov 	ax, 0x0003							; Set BIOS video mode
	int 		0x10
	
	; Turn on A20 Gate
	cli 													; Close interrupt
	in			al, 0x92
	or			al, 00000010b
	out		0x92, al
	
	TurnOnPM										; Turn on protect-mode
	
	; Switich to protect-mode
	jmp		dword SelectorFlatC:(BaseOfLoader_PhyAddr+LABEL_PM_START)

;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



; 获取内存布局 ----------------------------------------------------------
GetMemInfo:
	mov			ebx, 0							; ebx = 后续值, 开始时需为 0
	mov			di, _MemChkBuf			; es:di 指向一个地址范围描述符结构（Address Range Descriptor Structure）
.MemChkLoop:
	mov			eax, 0xe820					; eax = 0000E820h
	mov			ecx, 20						; ecx = 地址范围描述符结构的大小
	mov			edx, 0x534D4150		; edx = 'SMAP'
	int			0x15							; int 15h
	jc				.MemChkFail
	add			di, 20
	inc			dword [_dwMCRNumber]		; dwMCRNumber = ARDS 的个数
	cmp			ebx, 0
	jne			.MemChkLoop
	jmp			.MemChkOK
.MemChkFail:
	mov			dword [_dwMCRNumber], 0
.MemChkOK:

	ret
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


%ifndef __LOAD_KERNEL_IN_PM__
; Load kernel file (origin ELF file) -------------------------------
LoadKernelFile:
	mov 		si, DAP_struct
	mov 		ecx, StartSecOfKernalFile		; Start from logic sector StartSecOfKernalFile
	mov 		dx, BaseOfKernelFile				; 起始内存段地址
	mov 		bx, OffsetOfKernelFile			; 起始内存偏移地址
	mov 		al, SecNumOfReadOnce		; 每次中断读取的磁盘扇区数
	mov 		[DAP_SecNum], al

.loop_load_kernel:
	push 		dx
	mov 		[DAP_SecStart], ecx
	mov 		[DAP_Mem_seg], dx
	mov 		[DAP_Mem_off], bx
	call 			ReadSector_LBA
	pop 		dx
	
	add			ecx, SecNumOfReadOnce					; 硬盘偏移OnceReadSecNum 个扇区
	add 			bx, (SecNumOfReadOnce * 0x200)		; 内存偏移地址递增
	cmp 		bx, 0x0000
	jne 			.loop_load_kernel

	add 			dx, 0x1000							; 内存段地址递增
	cmp 		dx, LimitOfKernelFile				; 判断是否到达加载地址的段界限
	jne 			.loop_load_kernel

	ret
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


; LBA 模式读取硬盘扇区
ReadSector_LBA:
	mov 		dl, DrvNum 					; dl = 驱动器号
	mov 		ah, 0x42 						; ah = 42h
	int 			0x13							; int 13h
	jc 				ReadSector_LBA 			; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止
	ret
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
%endif



; 32-bit 保护模式代码段 --------------------------------------------------
[SECTION .s32]
ALIGN	32
[BITS	32]
LABEL_PM_START:
	mov			ax, SelectorVideo
	mov			gs, ax
	mov			ax, SelectorFlatRW
	mov			ds, ax
	mov			es, ax
	mov			fs, ax
	mov			ss, ax
	mov			esp, TopOfProtStack

	call			DispMemInfo 							; 显示内存信息

	mov 		eax, [dwMemSize] 					; 将内存大小写入到指定地址
	mov 		[MemSize_PhyAddr], eax

	mov 		eax, [dwMCRNumber] 				; 将内存布局信息的条目数写入到指定地址
	mov 		[MCRNumber_PhyAddr], eax
	
	mov 		eax, MEMCHKBUF_SIZE 			; 将内存布局信息写入指定地址
	push		eax											; size
	mov			eax, MemChkBuf	
	push		eax											; src
	mov 		eax, MemInfo_PhyAddr			; dst
	push 		eax
	call			MemCpy
	add			esp, 12

%ifdef __LOAD_KERNEL_IN_PM__
;// 在保护模式下load kernel ELF 文件
	mov 		eax, [_dwMemSize] 					; 计算kernel ELF 文件存放的起始地址（从内存高端偏移）
	sub 			eax, (OffsetFromMemoryEnd_MB * 0x100000)
	mov 		dword [KernelFile_PhyAddr], eax

	call 			LoadKernelFile_PM 					; Load kernel file from prot-mode.
%else
;// 在实模式下已经将kernel ELF 文件load 好，此处仅将文件地址写入变量
	mov 		eax, BaseOfKernelFile_PhyAddr
	mov 		dword [KernelFile_PhyAddr], eax
%endif

	call			InitKernel 									; 解析kernel 的ELF 格式，将kernel 代码在内存中重定位
	mov 		eax, [KernelFile_PhyAddr] 		; 从ELF文件中取入口地址
	add 			eax, e_entry
	mov 		eax, [eax]
	mov 		[KernelEntryPoint], eax

	; Switch to kernel.
	mov 		eax, [KernelEntryPoint]
	and 			eax, PAddrMask						; Physical address mask
	jmp			eax											; 进入内核

;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



; 显示内存信息 --------------------------------------------------------------
DispMemInfo:
;	push		szMemChkTitle					; 显示内存信息标题
;	call			DispStr
;	add			esp, 4

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
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



; InitKernel ---------------------------------------------------------------------------------
; 将 kernel file 的内容经过ELF 格式解析后重定位
; --------------------------------------------------------------------------------------------
InitKernel:	
	; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
	mov 		ecx, [KernelFile_PhyAddr]					; ┓ ecx <- pELFHdr->e_phnum
	add 			ecx, e_phnum
	mov 		cx, word [ecx]
	movzx		ecx, cx												; ┛
	mov 		esi, [KernelFile_PhyAddr] 					; esi <- pELFHdr->e_phoff
	add 			esi, e_phoff
	mov 		esi, [esi]
	add 			esi, [KernelFile_PhyAddr]					; esi <- OffsetOfKernel + pELFHdr->e_phoff
.LoadSegment_Begin:
	mov			eax, [esi + p_type]
	cmp			eax, 0												; PT_NULL
	jz				.LoadSegment_NoAction
	push		dword [esi + p_filesz]						; size	┓
	mov			eax, [esi + p_offset]									;	┃
	add			eax, [KernelFile_PhyAddr]							;	┣ ::memcpy(	(void*)(pPHdr->p_vaddr),
	push		eax														; src	┃		uchCode + pPHdr->p_offset,
	mov 		eax, [esi + p_vaddr]								; dst	┃		pPHdr->p_filesz;
	and 			eax, PAddrMask										; Physical address mask
	push 		eax
	call			MemCpy													;	┃
	add			esp, 12													;	┛
.LoadSegment_NoAction:
	mov 		eax, [KernelFile_PhyAddr]							; esi += pELFHdr->e_phentsize
	add 			eax, e_phentsize
	movzx		eax, word [eax]
	add			esi, eax
	dec			ecx
	jnz			.LoadSegment_Begin
	

	; 遍历每一个 Section Header，将.bss 段清零
	xor 			edx, edx
	mov 		edx, [KernelFile_PhyAddr]
	add 			edx, e_shnum
	movzx		edx, word [edx]
	mov 		esi, [KernelFile_PhyAddr]
	add 			esi, e_shoff
	mov 		esi, [esi]
	add 			esi, [KernelFile_PhyAddr]
.ConfigSection_Begin:
	mov			eax, dword [esi + sh_type]
	cmp			eax, SHT_NOBITS 
	jne			.ConfigSection_NoAction
	mov 		eax, dword [esi + sh_addr] 
	and 			eax, PAddrMask									; Physical address mask
	mov 		ecx, dword [esi + sh_size]
.ConfigSection_ClearLoop:
	mov 		byte [eax], 0
	inc 			eax
	loop 		.ConfigSection_ClearLoop
.ConfigSection_NoAction:

	mov 		eax, [KernelFile_PhyAddr]						; esi += pELFHdr->e_shentsize
	add 			eax, e_shentsize
	movzx		eax, word [eax]

	add			esi, eax 
	dec			edx
	jnz			.ConfigSection_Begin

	ret
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



%ifdef __LOAD_KERNEL_IN_PM__
; Load kernel file from prot-mode ----------------------------------------
; GDT ---------------------------------------------------------------------------------------------------
;												Base 	Limit 			Attribute
GDT_TEMP:			Descriptor 	0, 		0, 				0					; Null
DESC_CODE16: 	Descriptor 	0, 		0xffffff,			DA_C			; 16-bit code
; GDT ---------------------------------------------------------------------------------------------------

GdtTemp_Len		equ		$ - GDT_TEMP
GdtTemp_Ptr:		dw		GdtTemp_Len - 1										; Segment limit
							dd		BaseOfLoader_PhyAddr + GDT_TEMP		; Segment base

; GDT Selector ----------------------------------------------------------------------------------
SelectCode16_temp		equ		DESC_CODE16		- GDT_TEMP
; GDT Selector ----------------------------------------------------------------------------------

LoadKernelFile_PM:
	mov 	[SPValueInProtMode], esp 				; Save current ESP value
	sgdt 	[SaveGDTR] 									; Save current GDTR value
	lgdt 		[GdtTemp_Ptr] 									; Load temporary GDT for 16-bit code use
	jmp 		SelectCode16_temp:(BaseOfLoader_PhyAddr + BACK_TO_REAL)
	
[BITS 16]
BACK_TO_REAL:
	TurnOffPM 													; NOTE: 关闭保护模式必须在16位代码段进行
	jmp 		0: REAL_ENTRY 								; 将CS清零，跳转到实模式代码
	
; 实模式堆栈空间
RealStack: 			times 	SizeOfRealStack 		db 	0
TopOfRealStack: 	; 栈顶

REAL_ENTRY:
	mov 		ax, cs
	mov 		ds, ax
	mov 		es, ax
	mov 		ss, ax
	mov 		sp, TopOfRealStack

;--------------------------------------------
; Load kernel file (origin ELF file)
	mov 		si, DAP_struct 						; DAP 结构体地址
	mov 		ebx, StartSecOfKernalFile		; Start from logic sector StartSecOfKernalFile
	mov 		word [DAP_Mem_seg], BaseOfTempLoadAddr 	; 1MB 以下暂存位置 -- 段地址
	mov 		word [DAP_Mem_off], OffsetOfTempLoadAddr 	; 1MB 以下暂存位置 -- 偏移地址
	mov 		byte [DAP_SecNum], SecNumOfReadOnce 			; 单次中断读取的扇区数
	mov 		ecx, SizeOfKernelFile			; [用于循环计数]
	mov 		edi, [KernelFile_PhyAddr] 		; 内存高端放置ELF文件的起始地址
	mov 		ax, BaseOfTempLoadAddr 	; 1MB 以下暂存位置 -- 偏移地址
	mov 		es, ax
.loop_load_kernel:
	mov 		[DAP_SecStart], ebx			; 起始扇区号
	call 			ReadSector_LBA

	; 将1MB 以下临时存放的内容拷贝至目标位置（内存高端）
	push 		ecx
	push 		esi
	mov 		ecx, (SecNumOfReadOnce * 128)	; [用于循环计数]
	mov 		esi, OffsetOfTempLoadAddr
.loop_MemCopy:
	mov 		eax, dword [es:esi]				; 4-byte 拷贝
	add 			esi, 4
	mov 		dword [fs:edi], eax
	add 			edi, 4
	loop 		.loop_MemCopy
	pop 		esi
	pop 		ecx
		
	add			ebx, SecNumOfReadOnce			; 硬盘偏移SecNumOfReadOnce 个扇区
	loop 		.loop_load_kernel
;--------------------------------------------

	lgdt 			[_SaveGDTR] 								; Recover the previous GDTR

	TurnOnPM 													; Turn on prot-mode

	jmp 		dword SelectorFlatC:(BaseOfLoader_PhyAddr + BACK_TO_PROT)


; LBA 模式读取硬盘扇区
ReadSector_LBA:
	mov 		dl, DrvNum 							; dl = 驱动器号
	mov 		ah, 0x42 								; ah = 42h
	int 			0x13									; int 13h
	jc 				ReadSector_LBA 					; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止
	ret

[BITS 32]
BACK_TO_PROT:
	mov			ax, SelectorFlatRW
	mov			ss, ax
	mov 		ds, ax
	mov 		es, ax
	mov 		esp, [SPValueInProtMode]
	ret
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
%endif



; 数据段 -----------------------------------------------------------------------
[SECTION .data]
ALIGN	32
LABEL_DATA:
; 实模式下使用这些符号
; 字符串
;_szMemChkTitle:			db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
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
_SPValueInProtMode: 	dd 	0
_KernelFile_PhyAddr: 	dd 	0
_KernelEntryPoint: 		dd 	0
_SaveGDTR: 				dd 	0, 0

;; 保护模式下使用这些符号
;szMemChkTitle				equ		BaseOfLoader_PhyAddr + _szMemChkTitle
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
SPValueInProtMode 		equ 		BaseOfLoader_PhyAddr + _SPValueInProtMode
KernelFile_PhyAddr 		equ 		BaseOfLoader_PhyAddr + _KernelFile_PhyAddr
KernelEntryPoint 			equ 		BaseOfLoader_PhyAddr + _KernelEntryPoint
SaveGDTR 					equ 		BaseOfLoader_PhyAddr + _SaveGDTR

; 保护模式的栈空间 -----------------------------------------------------------------
ProtStack:					times	SizeOfProtStack		db	0
TopOfProtStack			equ		BaseOfLoader_PhyAddr + $	; 栈顶

