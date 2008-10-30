#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
//typedef unsigned int ULONG32;

ULONG32 __declspec(naked) NBPCall (ULONG32 knock) {//自定义出入栈顺序
	__asm { 
	push 	ebp
	mov	ebp, esp
	push	ebx
	push	ecx
	push	edx
	cpuid    ;看来是要用cpuid触发异常陷入VM
	pop	edx
	pop	ecx
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret
	}
}
//真正的客户端入口程序处
int __cdecl main(int argc, char **argv) {
	ULONG32 knock;
	if (argc != 2) {
		printf ("bpknock <magic knock>\n");
		return 0;
	}
	knock = strtoul (argv[1], 0, 0);//把输入的字符串转换成数字。 参数一 字符串的起始地址。参数二 返回字符串有效数字的结尾地址。参数三 转化基数。

	__try {
		printf ("knock answer: %#x\n", NBPCall (knock));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	
	return 0;

}
