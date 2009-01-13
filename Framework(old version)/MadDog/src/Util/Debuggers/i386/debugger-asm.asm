.686p
.model flat,StdCall
option casemap:none

.CODE

CmInitSpinLock PROC StdCall BpSpinLock
	mov	eax,BpSpinLock
	and	dword ptr [eax], 0
	ret
CmInitSpinLock ENDP


CmAcquireSpinLock PROC StdCall BpSpinLock
	mov	eax,BpSpinLock
loop_down:
	lock	bts dword ptr [eax], 0
	jb	loop_down
	ret
CmAcquireSpinLock ENDP


CmReleaseSpinLock PROC StdCall BpSpinLock
	mov	eax,BpSpinLock
	lock	btr dword ptr [eax], 0
	ret
CmReleaseSpinLock ENDP


END
