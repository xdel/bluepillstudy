;##################################################
;	File: 			boot2.asm
;	Description:	Boot-stage2 for normal booting,
; 					loads normal booting MBR to 0x7c00
;					and jumps to it.
;##################################################

org		0x8000

	jmp 	Boot_start

Boot_start:
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov 	ss, ax
	
	mov 	ax, Msg_LoadMBR		
	mov 	cx, MsgLen_LoadMBR
	mov 	dx, 0x0a04			; Position: row 10, column 4.
	call 	Display_string
	
	mov 	cx, 0x0029			; Read HDD0 track 0, sector 41
	mov 	ax, 0x0000			; to address 0000:8000 in memory.
	mov 	bx, 0x7c00
	call 	Read_one_sector
	jc 		ERR_Load_MBR		; Branch if not succeed.

	mov 	ax, Msg_OK			; String: "OK!"
	mov 	cx, MsgLen_OK
	mov 	dx, 0x0a23			; Position: row 10, column 35.
	call 	Display_string
	
	jmp		0x7c00				; Jump to MBR.

ERR_Load_MBR:					; Read sector error disposal.
	mov 	ax, Msg_FAIL		; String: "Fail!"
	mov 	cx, MsgLen_FAIL
	mov 	dx, 0x0a23			; Position: row 10, column 35.
	call 	Display_string
	
	jmp		$					; Spin here forever.

	

;----------------------------------------
;	Function: 		Display_string
; 	Description:	Display charactor string.
; 	NOTE:			String index stored in DH.
;----------------------------------------
Display_string:
	mov		bp, ax				; '.
	mov 	ax, ds				;  | ES:BP = string address. 
	mov 	es, ax				; /
	mov		ax, 0x1301			; AH = 13,  AL = 01h
	mov		bx, 0x7				; Page 0 (BH = 0); back: black, front: white (BL = 0x7).
	int		0x10				; INT 10H
	ret



;----------------------------------------
;	Function: 		Read_one_sector
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
Read_one_sector:
	mov 	es, ax
	mov 	ax, 0x0201
	mov 	dx, 0x0080
	int 	0x13
	ret



;----------------------------------------
;	Message strings
;----------------------------------------
MsgLen_LoadMBR 		equ 	29
Msg_LoadMBR:		db 		"Loading normal booting MBR..."
MsgLen_OK			equ 	3
Msg_OK:				db		"OK!"
MsgLen_FAIL         equ     5
Msg_FAIL:           db      "Fail!"
 


	times 	512-($-$$)	db	0	; Fullfile the rest space, making binary file to be 512 bytes exactly.

