%define 	__LOAD_KERNEL_IN_PM__ 			; 宏标记：在保护模式下从硬盘读取kernel ELF 文件

%include "include/bochs.inc"

; ----- // for boot.asm, boot2.asm // -----
MBR_offset 								equ 		0x7c00
BOOT2_offset 							equ 		0x7e00							; Memory offset where boot2 will be loaded.
KERNEL_LOADER_offset			equ 		0x0600							; Memory offset where kernel loader will be loaded.
StartSecOfMBR		 				equ 		1										; Start sector of MBR.
StartSecOfMBR_bak 				equ 		33									; Start sector of MBR backup.
StartSecOfBoot2 						equ 		34 									; Start sector of boot2.
StartSecOfKernelLoader 			equ 		35 									; Start sector of kernel loader.
SizeOfKernelLoader 					equ 		5 										; size of kernel loader (with respect to sectors).

;------ // for boot_Zion.asm // -----
%ifdef __BOCHS_DEBUG__
StartSecOfKernalFile 				equ 		189									; kernle ELF 文件在硬盘中的起始扇区号
%else
StartSecOfKernalFile 				equ 		71682500						; kernle ELF 文件在硬盘中的起始扇区号
%endif

SizeOfProtStack						equ 		128 									; Size of protecte-mode stack
SizeOfRealStack 						equ 		256									; Size of real-mode stack

BaseOfLoader							equ	 	0x0									; loader 被加载到的位置 ----  段地址
OffsetOfLoader						equ	  	KERNEL_LOADER_offset	; loader 被加载到的位置 ---- 偏移地址
BaseOfLoader_PhyAddr			equ		(BaseOfLoader * 0x10)	; loader 被加载到的位置 ---- 物理地址

%ifndef __LOAD_KERNEL_IN_PM__
; // for func: LoadKernelFile -- Load kernel ELF file in original real-mode.
BaseOfKernelFile						equ	 	0x0000							; kernel 被加载到的位置 ----  段地址
LimitOfKernelFile						equ		0x4000							; kernel 被加载到的界限 ---- 段地址
OffsetOfKernelFile						equ		0x1000							; kernel 被加载到的位置 ---- 偏移地址
BaseOfKernelFile_PhyAddr		equ		(BaseOfKernelFile * 0x10 + OffsetOfKernelFile)	; kernel 被加载到的位置 ---- 物理地址
SecNumOfReadOnce 				equ 		8 										; 每次中断读取的磁盘扇区数
%else
; // for func: LoadKernelFile_PM -- Load kernel ELF file from prot-mode.
SizeOfKernelFile 						equ 		10 									; kernel ELF 文件大小（以32KB为单位）
BaseOfTempLoadAddr 			equ 		0x8000 							; 1MB 以下临时存放从硬盘读出数据的位置 ---- 段地址
OffsetOfTempLoadAddr 			equ 		0x0000 							; 1MB 以下临时存放从硬盘读出数据的位置 ---- 偏移地址
OffsetFromMemoryEnd_MB		equ 		128 									; 距离内存高端的偏移量（单位：MB）
SecNumOfReadOnce 				equ 		64 									; 每次中断读取的磁盘扇区数
%endif

PAddrMask 								equ 		0xFFFFFFFF						; 物理地址掩码

;----- for memory infomation -----
MEMCHKBUF_SIZE 					equ 		512 									; 内存布局存储缓冲区大小
MemSize_PhyAddr 					equ 		0x9000 							; 内存地址信息存放的位置 ---- 物理地址
MCRNumber_PhyAddr 			equ 		0x9008
MemInfo_PhyAddr 					equ 		0x9010
