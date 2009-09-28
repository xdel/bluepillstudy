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

