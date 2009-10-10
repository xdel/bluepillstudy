;##################################################
;	File: 			boot.asm
;	Description:	Seraph MBR Loader. Make a branch
;					between kernel entry and normal
;					booting. The latter branch will
;					load boot-stage2 for further.
;##################################################
%include "include/load.inc"
%include "include/hdd.inc"	

org		MBR_offset

	jmp 			Boot_start

Boot_start:
	mov			ax, cs
	mov			ds, ax
	mov			es, ax
	mov 		ss, ax
	
	call 			Clear_screen						; Clear current screen.
	
	mov 		ax, Msg_Logo						; String: "@@@ Seraph MBR/Boot Loader @@@"
	mov 		cx, MsgLen_Logo
	mov 		dx, 0x0104							; Position: row 1, column 2.
	call 			Display_string
	
	mov 		ax, Msg_Branch					; String: "Enter kernel? [No] "
	mov 		cx, MsgLen_Branch
	mov 		dx, 0x0404							; Position: row 4, column 4.
	call 			Display_string
	
	call 			Wait_key
	and 			al, 0xdf								; Trun all char into uppercase.
	cmp 		al, 'Y'
	jne 			Normal_boot						; Branch if choose normal booting.

	mov 		ax, Msg_Yes							; String: "Yes"
	mov 		cx, MsgLen_Yes
	mov 		dx, 0x0417							; Position: row 4, column 23.
	call 			Display_string

	mov 		ax, Msg_LoadKernelLoader			; String: "Loading kernel-loader..."
	mov 		cx, MsgLen_LoadKernelLoader	
	mov 		dx, 0x0604									; Position: row 6, column 4.
	call 			Display_string
	
	mov 		ax, StartSecOfKernelLoader		; Start sector,
	mov 		cl, SizeOfKernelLoader  			; read SizeOfKernelLoader sectors
	mov 		dx, 0x0000								; to address 0000:KERNEL_LOADER_offset in memory.
	mov 		bx, KERNEL_LOADER_offset
	call 			ReadSector
	jc 				ERR_Read_sector_fail				; Branch if not succeed.

	mov 		ax, Msg_OK							; String: "OK!"
	mov 		cx, MsgLen_OK	
	mov 		dx, 0x061d							; Position: row 6, column 29.
	call 			Display_string
	
	call 			Clear_screen						; Clear current screen.

	mov 		ah, 0x02								; Set cursor position to
	mov 		bh, 0x0 								; page 0, 
	mov 		dx, 0x0000 							; row 0 and column 0
	int 			0x10 									; int 10h, ah=2

	jmp			KERNEL_LOADER_offset		; Jump to kernel loader.



Normal_boot:
	mov 		ax, Msg_No							; String: "No"
	mov 		cx, MsgLen_No
	mov 		dx, 0x0417							; Position: row 4, column 23.
	call 			Display_string

	mov 		ax, Msg_NormalBoot			; String: "Normal booting..."
	mov 		cx, MsgLen_NormalBoot
	mov 		dx, 0x0604							; Position: row 6, column 4.
	call 			Display_string
	
	mov 		ax, Msg_LoadBoot2				; String: "Loading boot-stage2..."
	mov 		cx, MsgLen_LoadBoot2	
	mov 		dx, 0x0804							; Position: row 8, column 4.
	call 			Display_string
	
	mov 		ax, StartSecOfBoot2 			; Start sector 42,
	mov 		cl, 1 										; read 1 sector
	mov 		dx, 0x0000							; to address 0000:BOOT2_offset in memory.
	mov 		bx, BOOT2_offset
	call 			ReadSector
	jc 				ERR_Read_sector_fail			; Branch if not succeed.
	
	mov 		ax, Msg_OK							; String: "OK!"
	mov 		cx, MsgLen_OK	
	mov 		dx, 0x081b							; Position: row 8, column 27.
	call 			Display_string

	jmp			BOOT2_offset						; Jump to boot-stage2.

ERR_Read_sector_fail:								; Read sector error disposal.
	mov 		ax, Msg_FAIL						; String: "Fail!"
	mov 		cx, MsgLen_FAIL	
	mov 		dx, 0x081b							; Position: row 8, column 27.
	call 			Display_string
	jmp 			$ 											; Spin here forever.
	


;----------------------------------------
;	Function: 		Clear_screen
; 	Description:	Clear the current screen.
;----------------------------------------
Clear_screen:
	mov 		ax, 0x0600
	mov 		bh, 0x07
	mov 		cx, 0x0000
	mov 		dx, 0x184f
	int 			0x10									; int 10h, ah=6
	ret



;----------------------------------------
;	Function: 		Display_string
; 	Description:	Display charactor string.
; 	NOTE:			String index stored in DH.
;----------------------------------------
Display_string:
	mov			bp, ax					; '.
	mov 		ax, ds					;  | ES:BP = string address. 
	mov 		es, ax					; /
	mov			ax, 0x1301			; AH = 13, AL = 01h
	mov			bx, 0x7				; Page 0 (BH = 0); backr: black, front: white (BL = 0x7).
	int			0x10					; INT 10H
	ret



;----------------------------------------
;	Function: 		ReadSector
; 	Description:	Read sectors from hard disk.
; 	NOTE:			Using BIOS interrupt service 13H, function 02H.
;		INT 13H，AH=02H 读扇区:
;		入口参数：
;			AH=02H 指明调用读扇区功能。
;			AL 置要读的扇区数目，不允许使用读磁道末端以外的数值，也不允许使该寄存器为0。
;			DL 需要进行读操作的驱动器号。
;			DH 所读磁盘的磁头号。
;			CH 磁道号的低8位数。
;			CL 低6位放入所读起始扇区号，位7-6表示磁道号的高2位。
;			ES:BX 读出数据的缓冲区地址。
;		返回参数：
;			如果CF=1，AX中存放出错状态。读出后的数据在ES:BX区域依次排列。
;
; -----------------------------------------------------------------------
; 怎样由扇区号求扇区在磁盘中的位置 (扇区号 -> 柱面号, 起始扇区, 磁头号)
; -----------------------------------------------------------------------
; 设扇区号为 x
;                          				┌ 柱面号 = y >> 1
;       x           		┌ 商 y ┤
; -------------- => ┤     		 └ 磁头号 = y & 1
;  每磁道扇区数     │
;                   		└ 余 z => 起始扇区号 = z
; 注：有效扇区号为1～63。
;----------------------------------------
ReadSector:
	mov 		es, dx					; es的值存放在dx中

	push		bp
	mov			bp, sp
	sub			esp, 2					; 辟出两个字节的堆栈区域保存要读的扇区数: byte [bp-2]

	mov			byte [bp-2], cl
	push		bx						; 保存 bx
	mov			bl, SecPerTrk		; bl: 除数
	div			bl							; y 在 al 中, z 在 ah 中
	mov			cl, ah					; cl <- 起始扇区号
	xor 			ah, ah
	mov 		bl, HeadsNum		; 磁头数
	div 			bl 						; y/HeadsNum
	mov 		ch, al 					; ch <- 柱面号
	mov 		dh, ah 					; dh <- 磁头号
	pop			bx						; 恢复 bx
	; 至此, "柱面号, 起始扇区, 磁头号" 全部得到 ^^^^^^^^^^^^^^^^^^^^^^^^
	mov			dl, DrvNum			; 驱动器号
.GoOnReading:
	mov			ah, 0x02				; 读
	mov			al, byte [bp-2]		; 读 al 个扇区
	int			0x13
	jc				.GoOnReading		; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止

	add			esp, 2
	pop			bp
	ret



;----------------------------------------
;	Function: 		Wait_key
; 	Description:	Wait for keyboard input.
; 	NOTE:			Using BIOS interrupt service 10H, function 16H.
;----------------------------------------
Wait_key:
	mov 		ah, 0x00
	int 			0x16
	ret



;----------------------------------------
;	Message strings
;----------------------------------------
MsgLen_Logo 						equ 	30
Msg_Logo: 							db 		"@@@ Seraph MBR/Boot Loader @@@"
MsgLen_Branch 					equ 	19
Msg_Branch:						db 		"Enter kernel? [No] "
MsgLen_NormalBoot 			equ 	17
Msg_NormalBoot:				db 		"Normal Booting..."
MsgLen_LoadKernelLoader 	equ 	24
Msg_LoadKernelLoader:		db 		"Loading kernel-loader..."
MsgLen_LoadBoot2 			equ 	22
Msg_LoadBoot2:					db 		"Loading boot-stage2..."
MsgLen_Yes 						equ 	3
Msg_Yes: 								db 		"Yes"
MsgLen_No 							equ 	2
Msg_No:								db 		"No"
MsgLen_OK 							equ 	3
Msg_OK:								db 		"OK!"
MsgLen_FAIL 						equ 	5
Msg_FAIL:							db 		"Fail!"


	times 	510-($-$$)			db	0		; Fullfile the rest space, making binary file to be 512 bytes exactly.
	dw 		0xaa55								; MBR magic number.
