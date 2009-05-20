.686p
.model flat,StdCall
option casemap:none

.CODE

MmInvalidatePage PROC StdCall _PageVA
	invlpg	[_PageVA]
	ret
MmInvalidatePage ENDP

END
