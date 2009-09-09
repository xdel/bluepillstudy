;##################################################
;	File: 			boot2.asm
;	Description:	Boot-stage2 for normal booting,
; 					loads normal booting MBR to 0x7c00
;					and jumps to it.
;##################################################
%include	"include/load.inc"

org		BOOT2_offset

	jmp 		Boot_start

Boot_start:
	mov			ax, cs
	mov			ds, ax
	mov			es, ax
	mov 		ss, ax
	
	mov 		ax, Msg_LoadMBR		
	mov 		cx, MsgLen_LoadMBR
	mov 		dx, 0x0a04							; Position: row 10, column 4.
	call 			Display_string
	
	mov 		ax, 0x0000							; Read MBR backup
	mov 		es, ax
	mov 		ax, 0x0201
	mov 		dx, 0x0080
	xor 			ch, ch
	mov 		cl, StartSecOfMBR_bak
	mov 		bx, MBR_offset
	int 			0x13 									; int 13h, ah=02
	jc 				ERROR 								; Branch if not succeed.

;	mov 		ax, 0x0000	 						; Write MBR secto
;	mov 		es, ax
;	mov 		ax, 0x0301
;	mov 		dx, 0x0080
;	xor 			ch, ch
;	mov 		cl, StartSecOfMBRr
;	mov 		bx, MBR_offset
;	int 			0x13 									; int 13h, ah=03
;	jc 				ERROR 								; Branch if not succeed.

	mov 		ax, Msg_OK							; String: "OK!"
	mov 		cx, MsgLen_OK
	mov 		dx, 0x0a23							; Position: row 10, column 35.
	call 			Display_string
	
	jmp			MBR_offset							; Jump to MBR.

ERROR:													; Read sector error disposal.
	mov 		ax, Msg_FAIL						; String: "Fail!"
	mov 		cx, MsgLen_FAIL
	mov 		dx, 0x0a23							; Position: row 10, column 35.
	call 			Display_string
	jmp			$											; Spin here forever.

	

;----------------------------------------
;	Function: 		Display_string
; 	Description:	Display charactor string.
; 	NOTE:			String index stored in DH.
;----------------------------------------
Display_string:
	mov			bp, ax					; '.
	mov 		ax, ds					;  | ES:BP = string address. 
	mov 		es, ax					; /
	mov			ax, 0x1301			; AH = 13,  AL = 01h
	mov			bx, 0x7				; Page 0 (BH = 0); back: black, front: white (BL = 0x7).
	int			0x10					; INT 10H
	ret


;----------------------------------------
;	Message strings
;----------------------------------------
MsgLen_LoadMBR 		equ 		29
Msg_LoadMBR:			db 		"Loading normal booting MBR..."
MsgLen_OK					equ 		3
Msg_OK:						db		"OK!"
MsgLen_FAIL         		equ     	5
Msg_FAIL:           			db      	"Fail!"


	times 	512-($-$$)	db	0	; Fullfile the rest space, making binary file to be 512 bytes exactly.
