[SECTION .text]
ALIGN 32
[BITS 32]
global 		LoadKernelFile

HDD_IOPORT_BASE equ 		0x18D0 		; for ATA IDE: 0x1F0
;HDD_IOPORT_BASE equ 		0x1F0 		; for ATA IDE: 0x1F0

REG_DATA				equ 		(HDD_IOPORT_BASE + 0)		;	Data				I/O		
REG_FEATURES		equ 		(HDD_IOPORT_BASE + 1)		;	Features			O		
REG_NSECTOR			equ 		(HDD_IOPORT_BASE + 2)		;	Sector Count			I/O		
REG_LBA_LOW		equ 		(HDD_IOPORT_BASE + 3)		;	Sector Number / LBA Bits 0-7	I/O		
REG_LBA_MID			equ 		(HDD_IOPORT_BASE + 4)		;	Cylinder Low / LBA Bits 8-15	I/O		
REG_LBA_HIGH		equ 		(HDD_IOPORT_BASE + 5)		;	Cylinder High / LBA Bits 16-23	I/O		
REG_DEVICE			equ 		(HDD_IOPORT_BASE + 6)		;	Drive | Head | LBA bits 24-27	I/O		
REG_STATUS			equ 		(HDD_IOPORT_BASE + 7)		;	Status				I		
REG_ERROR				equ 		REG_FEATURES						;	Error				I		
REG_CMD					equ 		REG_STATUS							;	Command				O		

STATUS_BSY				equ 		0x80
STATUS_DRDY			equ 		0x40
STATUS_DFSE			equ 		0x20
STATUS_DSC			equ 		0x10
STATUS_DRQ			equ 		0x08
STATUS_CORR			equ 		0x04
STATUS_IDX				equ 		0x02
STATUS_ERR			equ 		0x01

CMD_READ 				equ 		0x20
CMD_WRITE 			equ 		0x30

LoadKernelFile:
	push		ebp
	mov			ebp, esp

testbusy:    
;	mov			dx, REG_CMD 		; 测试busy 位，当设备不忙时继续   
;	in				al, dx        
;	test			al, STATUS_BSY
;	jz				testbusy
      
	mov			al, 1 						; 读多少扇区，如果写入00表示读256扇区
	mov			dx, REG_NSECTOR
	out			dx, al    

	mov			al, 0
	mov			dx, REG_LBA_LOW
	out			dx, al
   
	mov			al, 0
	mov			dx, REG_LBA_MID
	out			dx, al
   
	mov			al, 0
	mov			dx, REG_LBA_HIGH
	out			dx, al

	mov			al, 33 					;从哪个扇区开始读,我们这里读0磁头0柱面1扇区，即MBR
	mov			dx, REG_LBA_LOW
	out			dx, al
   
	mov			al, 0
	mov			dx, REG_LBA_MID
	out			dx, al
   
	mov			al, 0
	mov			dx, REG_LBA_HIGH
	out			dx, al

	mov			al, 0xa0                                                  ;设备选择和LBA 23 to 27
	mov			dx, REG_DEVICE                                 
	out			dx, al
	mov			al, CMD_READ                                                ;在所有参数都写入后，就下读命令
	mov			dx, REG_CMD
	out			dx, al

testDRQ:                                                        ;检测数据是否准备好
	mov			dx, REG_STATUS
	in				al, dx
	and			al, STATUS_DRQ
	jz				testDRQ

	mov 		ecx, 2048
.wait:
	nop
	loop 		.wait

	mov			edi, 0xc0001000                                              ;读取数据
	mov			cx, 512
	mov			dx, REG_DATA
readdata:
	cld
	rep			insw

	mov			esp, ebp
	pop			ebp
	ret
