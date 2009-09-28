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

