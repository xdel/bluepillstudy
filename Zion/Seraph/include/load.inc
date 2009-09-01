BaseOfLoader							equ	 	0x0									; loader 被加载到的位置 ----  段地址
OffsetOfLoader						equ	  	0x8200							; loader 被加载到的位置 ---- 偏移地址
BaseOfLoader_PhyAddr			equ		BaseOfLoader * 0x10		; loader 被加载到的位置 ---- 物理地址

BaseOfKernelFile						equ	 	0x1000							; kernel 被加载到的位置 ----  段地址
OffsetOfKernelFile						equ		0x0									; kernel 被加载到的位置 ---- 偏移地址
BaseOfKernelFile_PhyAddr		equ		BaseOfKernelFile * 0x10 	; kernel 被加载到的位置 ---- 物理地址

KernelEntryPoint_PhyAddr		equ		0x10000c 						; kernel 运行入口 ---- 物理地址

MemSizeInfo_PhyAddr 			equ 		0x9000 							; 内存地址信息存放的位置 ---- 物理地址
