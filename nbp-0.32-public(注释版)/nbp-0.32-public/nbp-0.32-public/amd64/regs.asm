; only some boring stuff here...


.CODE

RegGetTSC PROC
;	rdtscp                       //Read Time-Stamp Counter and Processor ID,is used to read the modelspecific
                                    ;   time-stamp counter (TSC) register.
                                   ;    Loads the value of the processor’s 64-bit time-stamp counter into registers EDX:EAX, and loads the
                                   ;   value of TSC_AUX into ECX. This instruction ignores operand size.
	rdtsc                        ;Read Time-Stamp Counter,Loads the value of the processor’s 64-bit time-stamp counter into registers EDX:EAX
	shl		rdx, 32      ;rdx左移32位
	or		rax, rdx     ;最后rax保存的就是EDX:EAX
	ret
RegGetTSC ENDP

RegGetRax PROC
	mov		rax, rax  //Loads a subset of processor state from the VMCB specified by the physical address in the rAX register.
	ret
RegGetRax ENDP


RegGetRbx PROC
	mov		rax, rbx
	ret
RegGetRbx ENDP


RegGetCs PROC
	mov		rax, cs            //cs,ds es,ss,fs,gs:segment resigters
	ret
RegGetCs ENDP

RegGetDs PROC
	mov		rax, ds
	ret
RegGetDs ENDP

RegGetEs PROC
	mov		rax, es
	ret
RegGetEs ENDP

RegGetSs PROC
	mov		rax, ss
	ret
RegGetSs ENDP

RegGetFs PROC
	mov		rax, fs
	ret
RegGetFs ENDP

RegGetGs PROC
	mov		rax, gs
	ret
RegGetGs ENDP

RegGetCr0 PROC
	mov		rax, cr0   //cr0,cr2,cr3,cr4,cr8:control registers
	ret
RegGetCr0 ENDP

RegGetCr2 PROC
	mov		rax, cr2
	ret
RegGetCr2 ENDP

RegGetCr3 PROC
	mov		rax, cr3
	ret
RegGetCr3 ENDP

RegSetCr3 PROC
	mov		cr3, rcx
	ret
RegSetCr3 ENDP

RegGetCr4 PROC
	mov		rax, cr4
	ret
RegGetCr4 ENDP

RegGetCr8 PROC
	mov		rax, cr8
	ret
RegGetCr8 ENDP

RegSetCr8 PROC
	mov		cr8, rcx
	ret
RegSetCr8 ENDP

RegGetDr6 PROC
	mov		rax, dr6
	ret
RegGetDr6 ENDP

RegGetDr0 PROC
	mov		rax, dr0                // debug registers, DR0, DR1, DR2, and DR3
	ret
RegGetDr0 ENDP

RegGetDr1 PROC
	mov		rax, dr1              
	ret
RegGetDr1 ENDP

RegGetDr2 PROC
	mov		rax, dr2
	ret
RegGetDr2 ENDP

RegGetDr3 PROC
	mov		rax, dr3
	ret
RegGetDr3 ENDP

RegSetDr0 PROC
	mov		dr0, rcx
	ret
RegSetDr0 ENDP

RegSetDr1 PROC
	mov		dr1, rcx
	ret
RegSetDr1 ENDP

RegSetDr2 PROC
	mov		dr2, rcx
	ret
RegSetDr2 ENDP

RegSetDr3 PROC
	mov		dr3, rcx
	ret
RegSetDr3 ENDP

RegGetRflags PROC
	pushfq                      //Push the RFLAGS quadword(The 64-bit) onto stack
	pop		rax
	ret
RegGetRflags ENDP

RegGetRsp PROC
	mov		rax, rsp      //rsp:64位的堆栈指针
	add		rax, 8        //因为是64位
	ret
RegGetRsp ENDP

GetIdtBase PROC
	LOCAL	idtr[10]:BYTE
	
	sidt	idtr                        //sidt:Store Interrupt Descriptor Table Register--idtr
	mov		rax, QWORD PTR idtr[2]   //QWORD PTR：将所给地址处的数据作双字处理 
                                                               即取8个字节
	ret
GetIdtBase ENDP

GetIdtLimit PROC
	LOCAL	idtr[10]:BYTE
	
	sidt	idtr
	mov		ax, WORD PTR idtr[0]  //WORD PTR：将所给地址处的数据作单字处理 
                                                               即取2个字节
	ret
GetIdtLimit ENDP

GetGdtBase PROC
	LOCAL	gdtr[10]:BYTE

	sgdt	gdtr
	mov		rax, QWORD PTR gdtr[2]
	ret
GetGdtBase ENDP

GetGdtLimit PROC
	LOCAL	gdtr[10]:BYTE

	sgdt	gdtr
	mov		ax, WORD PTR gdtr[0]
	ret
GetGdtLimit ENDP

;add by cini
GetLdtr PROC
	sldt	rax                    
	ret
GetLdtr ENDP

;add end

GetTrSelector PROC
	str	rax
	ret
GetTrSelector ENDP




