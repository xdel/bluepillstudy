;-----// ELF Header //-----
e_ident 				equ 			0x00 		; 16 bytes
e_type 				equ 			0x10		; 2 bytes
e_machine 			equ 			0x12 		; 2 bytes
e_version 			equ 			0x14 		; 4 bytes
e_entry 				equ 			0x18 		; 4 bytes
e_phoff 				equ 			0x1C 		; 4 bytes
e_shoff 				equ 			0x20 		; 4 bytes
e_flags 				equ 			0x24 		; 4 bytes
e_ehsize 				equ 			0x28 		; 4 bytes
e_phentsize 		equ 			0x2A 		; 2 bytes
e_phnum 			equ 			0x2C 		; 2 bytes
e_shentsize 			equ 			0x2E 		; 2 bytes
e_shnum 				equ 			0x30 		; 2 bytes
e_shstrndx 			equ 			0x32 		; 2 bytes


;-----// Program Header //-----
p_type 				equ 			0x00		; 4 bytes
p_offset 				equ 			0x04 		; 4 bytes
p_vaddr				equ 			0x08 		; 4 bytes
p_paddr				equ 			0x0C 		; 4 bytes
p_filesz 				equ 			0x10 		; 4 bytes
p_memsz 			equ 			0x14 		; 4 bytes
p_flags 				equ 			0x18		; 4 bytes
p_align 				equ 			0x1C 		; 4 bytes


;-----// Section Header //-----
sh_name 				equ 			0x00 		; 4 bytes
sh_type 				equ 			0x04 		; 4 bytes
sh_flags 				equ 			0x08 		; 4 bytes
sh_addr 				equ 			0x0C 		; 4 bytes
sh_offset 				equ 			0x10 		; 4 bytes
sh_size 				equ 			0x14 		; 4 bytes
sh_link 				equ 			0x18 		; 4 bytes
sh_info 				equ 			0x1C 		; 4 bytes
sh_addralign 		equ 			0x20 		; 4 bytes
sh_entsize 			equ 			0x24 		; 4 bytes

; Section types
SHT_NULL 					equ 		0x00
SHT_PROGBITS 			equ 		0x01
SHT_SYMTAB 				equ 		0x02
SHT_STRTAB 				equ 		0x03
SHT_RELA 					equ 		0x04
SHT_HASH 					equ 		0x05
SHT_DYNAMIC 			equ 		0x06
SHT_NOTE 					equ 		0x07
SHT_NOBITS 				equ 		0x08
SHT_REL 						equ 		0x09
SHT_SHLIB 					equ 		0x0A
SHT_DYNSYM 				equ 		0x0B
SHT_LOPROC 				equ 		0x70000000
SHT_HIPROC 				equ 		0x7FFFFFFF
SHT_LOUSER 				equ 		0x80000000
SHT_HIUSER 				equ 		0x8FFFFFFF
