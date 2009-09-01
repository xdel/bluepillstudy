BaseOfLoaderPhyAddr 		equ 		0x0

BaseOfKernelFile				equ	 	0x1000	; KERNEL 被加载到的位置 ----  段地址
OffsetOfKernelFile				equ		0x0			; KERNEL 被加载到的位置 ---- 偏移地址

BaseOfKernelFilePhyAddr	equ		BaseOfKernelFile * 10h
KernelEntryPointPhyAddr	equ		0x10000C
;KernelEntryPointPhyAddr	equ		0x030400

