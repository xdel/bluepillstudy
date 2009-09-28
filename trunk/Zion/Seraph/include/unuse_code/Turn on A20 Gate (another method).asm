	; Turn on A20 Gate
	cli 													; Close interrupt
	call 			Wait_8042
	mov 		al , 0xd1
	out 			0x64 , al
	call 			Wait_8042
	mov 		al , 0xdf
	out 			0x60 , al
	call 			Wait_8042

Wait_8042:
	in 			al , 0x64
	test 			al , 0x2
	jnz 			Wait_8042
	ret

