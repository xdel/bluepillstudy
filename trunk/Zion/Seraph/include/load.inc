%include "include/bochs.inc"

; ----- // for boot.asm, boot2.asm // -----
MBR_offset 								equ 		0x7c00
BOOT2_offset 							equ 		0x7e00							; Memory offset where boot2 will be loaded.
BOOT_ZION_offset					equ 		0x7e00							; Memory offset where boot_Zion will be loaded.
StartSecOfMBR		 				equ 		1										; Start sector of MBR.
StartSecOfMBR_bak 				equ 		33									; Start sector of MBR backup.
StartSecOfBoot2 					equ 		34 									; Start sector of boot2.
StartSecOfBoot_Zion 				equ 		35 									; Start sector of boot_Zion.
SizeOfBoot_Zion 						equ 		8 										; boot_Zion size (with respect to sectors).

;------ // for boot_Zion.asm // -----
%ifdef __BOCHS_DEBUG__
StartSecOfKernalFile 				equ 		189									; kernle ELF 文件在硬盘中的起始扇区号
%else
StartSecOfKernalFile 				equ 		71682500						; kernle ELF 文件在硬盘中的起始扇区号
%endif

BaseOfLoader							equ	 	0x0									; loader 被加载到的位置 ----  段地址
OffsetOfLoader						equ	  	BOOT_ZION_offset			; loader 被加载到的位置 ---- 偏移地址
BaseOfLoader_PhyAddr			equ		BaseOfLoader * 0x10		; loader 被加载到的位置 ---- 物理地址

BaseOfKernelFile						equ	 	0x1000							; kernel 被加载到的位置 ----  段地址
OffsetOfKernelFile					equ		0x0000							; kernel 被加载到的位置 ---- 偏移地址
BaseOfKernelFile_PhyAddr		equ		BaseOfKernelFile * 0x10 	; kernel 被加载到的位置 ---- 物理地址

KernelEntryPoint_PhyAddr		equ		0x10000c 						; kernel 运行入口 ---- 物理地址

;----- for memory infomation -----
MEMCHKBUF_SIZE 					equ 		512 									; 内存布局存储缓冲区大小
MemSize_PhyAddr 					equ 		0x9000 							; 内存地址信息存放的位置 ---- 物理地址
MCRNumber_PhyAddr 			equ 		0x9008
MemInfo_PhyAddr 					equ 		0x9010

