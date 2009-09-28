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
; 设扇区号为 x （起始扇区号为0）
;                                         ┌ 柱面号 = y / 磁头数
;       x                      ┌ 商 y ┤
; ------------------- => ┤        └ 磁头号 = y / 磁头数 的余数
;  每磁道扇区数     		│
;                               └ 余 z => 起始扇区号 = z + 1
;----------------------------------------
ReadSector:
	mov 	es, dx				; es的值存放在dx中

	push	bx					; 保存 bx
	mov		bl, SecPerTrk	; bl: 除数
	div		bl						; y 在 al 中, z 在 ah 中
	inc 		ah
	mov		cl, ah				; cl <- 起始扇区号
	xor 		ah, ah
	mov 	bl, HeadsNum	; 磁头数
	div 		bl 					; y/HeadsNum
	mov 	ch, al 				; ch <- 柱面号
	mov 	dh, ah 				; dh <- 磁头号
	pop		bx					; 恢复 bx
	; 至此, "柱面号, 起始扇区, 磁头号" 全部得到
	mov		dl, DrvNum		; 驱动器号
.GoOnReading:
	mov		ah, 0x02			; 读
	mov		al, 1					; 读 1 个扇区
	int		0x13				; int 13h
	jc			.GoOnReading	; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止

	ret

;---------------- 调用范例 ----------------
; 目标：将硬盘中指定扇区的512k读进内存。
;	push 	es
;	mov 	ax, 189					; Start from sector 190,
;	mov 	dx, 0x1000				; to memory.
;	mov 	bx, 0x0
;.loop_ReadSec:
;	push 	ax
;	push 	dx
;	call 		ReadSector
;	pop 		dx
;	pop 		ax
;	
;	inc 		ax
;	add 		bx, 0x200 				; 内存地址偏移512字节
;	cmp 	bx, 0x0000
;	jne 		.loop_ReadSec
;
;	add 		dx, 0x1000				; 段地址递增
;	cmp 	dx, 0x8000
;	jne 		.loop_ReadSec
;
;	pop 		es
;---------------- 调用范例 (end) ----------------
