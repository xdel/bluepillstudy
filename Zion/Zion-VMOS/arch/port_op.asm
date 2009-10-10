global	out_byte
global	in_byte
global	port_read
global	port_write

WAIT_LOOP_CNT 	equ 		64

; ================================
; 		void out_byte(u16 port, u8 value);
; ================================
out_byte:
	push 		eax
	push 		edx
	push		ebp
	mov			ebp, esp

	mov			edx, [esp + 16]		; port
	mov			al, [esp + 20]	; value
	out			dx, al
	
	mov			esp, ebp
	pop			ebp
	pop 		edx
	pop 		eax
	ret



; ================================
; 		u8 in_byte(u16 port);
; ================================
in_byte:
	push 		edx
	push		ebp
	mov			ebp, esp

	mov			edx, [esp + 12]		; port
	xor			eax, eax
	in				al, dx
	
	mov			esp, ebp
	pop			ebp
	pop 		edx
	ret



; ================================
;		void port_read(u16 port, void* buf, int n);
; ================================
port_read:
	push 		ecx
	push 		edx
	push 		edi
	push		ebp
	mov			ebp, esp
	
	mov			edx, [esp + 20]		; port
	mov			edi, [esp + 24]			; buf
	mov			ecx, [esp + 28]		; n
	shr			ecx, 1
	cld
	rep			insw
	
	mov			esp, ebp
	pop			ebp
	pop 		edi
	pop 		edx
	pop 		ecx
	ret



; ================================
;		void port_write(u16 port, void* buf, int n);
; ================================
port_write:
	push 		ecx
	push 		edx
	push 		esi
	push		ebp
	mov			ebp, esp

	mov			edx, [esp + 20]			; port
	mov			esi, [esp + 24]			; buf
	mov			ecx, [esp + 28]		; n
	shr			ecx, 1
	cld
	rep			outsw

	mov			esp, ebp
	pop			ebp
	pop 		esi
	pop 		edx
	pop 		ecx
	ret

