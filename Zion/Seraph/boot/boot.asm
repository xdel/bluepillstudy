;##################################################
;	File: 			boot.asm
;	Description:	Seraph MBR Loader. Make a branch
;					between Zion entry and normal
;					booting. The latter branch will
;					load boot-stage2 for further.
;##################################################

org		0x7c00

	jmp 	Boot_start

BOOT2_ADDR 		equ 	0x8000		; Memory address where boot2 will be loaded.
BOOT_ZION_ADDR	equ 	0x8200		; Memory address where boot_Zion will be loaded.

Boot_start:
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov 	ss, ax
	
	call 	Clear_screen			; Clear current screen.
	
	mov 	ax, Msg_Logo			; String: "@@@ Seraph MBR Loader @@@"
	mov 	cx, MsgLen_Logo
	mov 	dx, 0x0104				; Position: row 1, column 2.
	call 	Display_string
	
	mov 	ax, Msg_Branch			; String: "Enter Zion? [No] "
	mov 	cx, MsgLen_Branch
	mov 	dx, 0x0404				; Position: row 4, column 4.
	call 	Display_string
	
	call 	Wait_key
	and 	al, 0xdf				; Trun all char into uppercase.
	cmp 	al, 'Y'
	jne 	Normal_boot				; Branch if choose normal booting.
	
	mov 	ax, Msg_Yes				; String: "Yes"
	mov 	cx, MsgLen_Yes
	mov 	dx, 0x0415				; Position: row 4, column 21.
	call 	Display_string

	mov 	ax, Msg_ZionBoot		; String: "Now, entering Zion..."
	mov 	cx, MsgLen_ZionBoot
	mov 	dx, 0x0604				; Position: row 6, column 4.
	call	Display_string

	mov 	ax, Msg_LoadBootZion	; String: "Loading Zion boot loader..."
	mov 	cx, MsgLen_LoadBootZion	
	mov 	dx, 0x0804				; Position: row 8, column 4.
	call 	Display_string
	
	mov 	cx, 0x002b				; Start from HDD0 track 0, sector 43,
	mov 	al, 6 					; read 6 sectors
	mov 	dx, 0x0000				; to address 0000:8200 in memory.
	mov 	bx, BOOT_ZION_ADDR
	call 	Read_sector
	jc 		ERR_Read_sector_fail	; Branch if not succeed.
	
	mov 	ax, Msg_OK				; String: "OK!"
	mov 	cx, MsgLen_OK	
	mov 	dx, 0x0820				; Position: row 8, column 32.
	call 	Display_string

	jmp		BOOT_ZION_ADDR			; Jump to boot-Zion.

	jmp 	$						; Spin here forever.
									; FUTURE WORK: Some code for Zion should be added here, inclding:
									; 		(1) Load Zion image into memory.
									; 		(2) Jump to the entry point of Zion.

Normal_boot:
	mov 	ax, Msg_No				; String: "No"
	mov 	cx, MsgLen_No
	mov 	dx, 0x0415				; Position: row 4, column 21.
	call 	Display_string

	mov 	ax, Msg_NormalBoot		; String: "Normal booting..."
	mov 	cx, MsgLen_NormalBoot
	mov 	dx, 0x0604				; Position: row 6, column 4.
	call 	Display_string
	
	mov 	ax, Msg_LoadBoot2		; String: "Loading boot-stage2..."
	mov 	cx, MsgLen_LoadBoot2	
	mov 	dx, 0x0804				; Position: row 8, column 4.
	call 	Display_string
	
	mov 	cx, 0x002a				; Start from HDD0 track 0, sector 42,
	mov 	al, 1 					; read 1 sector
	mov 	dx, 0x0000				; to address 0000:8000 in memory.
	mov 	bx, BOOT2_ADDR
	call 	Read_sector
	jc 		ERR_Read_sector_fail	; Branch if not succeed.
	
	mov 	ax, Msg_OK				; String: "OK!"
	mov 	cx, MsgLen_OK	
	mov 	dx, 0x081b				; Position: row 8, column 27.
	call 	Display_string

	jmp		BOOT2_ADDR				; Jump to boot-stage2.

ERR_Read_sector_fail:				; Read sector error disposal.
	mov 	ax, Msg_FAIL			; String: "Fail!"
	mov 	cx, MsgLen_FAIL	
	mov 	dx, 0x081b				; Position: row 8, column 27.
	call 	Display_string

	jmp 	$ 						; Spin here forever.
	


;----------------------------------------
;	Function: 		Clear_screen
; 	Description:	Clear the current screen.
;----------------------------------------
Clear_screen:
	mov 	ax, 0x0600
	mov 	bx, 0x0700
	mov 	cx, 0
	mov 	dx, 0x184f
	int 	0x10
	ret



;----------------------------------------
;	Function: 		Display_string
; 	Description:	Display charactor string.
; 	NOTE:			String index stored in DH.
;----------------------------------------
Display_string:
	mov		bp, ax				; '.
	mov 	ax, ds				;  | ES:BP = string address. 
	mov 	es, ax				; /
	mov		ax, 0x1301			; AH = 13, AL = 01h
	mov		bx, 0x7				; Page 0 (BH = 0); backr: black, front: white (BL = 0x7).
	int		0x10				; INT 10H
	ret



;----------------------------------------
;	Function: 		Read_sector
; 	Description:	Read one sector from hard disk.
; 	NOTE:			Using BIOS interrupt service 13H, function 02H.
;		INT 13H，AH=02H 读扇区:
;		入口参数：
;			AH=02H 指明调用读扇区功能。
;			AL 置要读的扇区数目，不允许使用读磁道末端以外的数值，也不允许使该寄存器为0。
;			DL 需要进行读操作的驱动器号。
;			DH 所读磁盘的磁头号。
;			CH 磁道号的低8位数。
;			CL 低5位放入所读起始扇区号，位7-6表示磁道号的高2位。
;			ES:BX 读出数据的缓冲区地址。
;		返回参数：
;			如果CF=1，AX中存放出错状态。读出后的数据在ES:BX区域依次排列。
;----------------------------------------
Read_sector:
	mov 	es, dx
	mov 	ah, 0x02
	mov 	dx, 0x0080
	int 	0x13
	ret



;----------------------------------------
;	Function: 		Wait_key
; 	Description:	Wait for keyboard input.
; 	NOTE:			Using BIOS interrupt service 10H, function 16H.
;----------------------------------------
Wait_key:
	mov 	ah, 0x10
	int 	0x16
	ret



;----------------------------------------
;	Message strings
;----------------------------------------
MsgLen_Logo 			equ 	25
Msg_Logo: 				db 		"@@@ Seraph MBR Loader @@@"
MsgLen_Branch 			equ 	17
Msg_Branch:				db 		"Enter Zion? [No] "
MsgLen_ZionBoot			equ 	21
Msg_ZionBoot:			db		"Now, entering Zion..."
MsgLen_NormalBoot 		equ 	17
Msg_NormalBoot:			db 		"Normal Booting..."
MsgLen_Yes 				equ 	3
Msg_Yes: 				db 		"Yes"
MsgLen_No 				equ 	2
Msg_No:					db 		"No"
MsgLen_LoadBoot2 		equ 	22
Msg_LoadBoot2:			db 		"Loading boot-stage2..."
MsgLen_OK 				equ 	3
Msg_OK:					db 		"OK!"
MsgLen_FAIL 			equ 	5
Msg_FAIL:				db 		"Fail!"
MsgLen_LoadBootZion 	equ 	27
Msg_LoadBootZion: 		db 		"Loading Zion boot loader..."



	times 	510-($-$$)	db	0	; Fullfile the rest space, making binary file to be 512 bytes exactly.
	dw 		0xaa55				; MBR magic number.

