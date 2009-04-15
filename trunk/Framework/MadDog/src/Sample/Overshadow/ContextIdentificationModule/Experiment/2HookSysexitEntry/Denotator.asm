.386
.model stdcall,flat
option casemap:none
include kernel32.inc
includelib kernel32.lib
include user32.inc
includelib user32.lib
includelib ntdll.lib;这个LIB 默认是没的 要在DDK里找
include D:\RadASM\masm32\macros\ucmacros.asm

ZwDeleteFile PROTO :DWORD
UNICODE_STRING struct
_Length dw ?
MaxLength dw ?
Buffer dd ?
UNICODE_STRING ends
PUNICODE_STRING typedef ptr UNICODE_STRING

OBJECT_ATTRIBUTES struct
_Length dd ?
RootDirectory dd ?
ObjectName PUNICODE_STRING ?
Attributes dd ?
SecurityDescriptor dd ?
SecurityQualityOfService dd ?
OBJECT_ATTRIBUTES ends

POBJECT_ATTRIBUTES typedef ptr UNICODE_STRING

OBJ_CASE_INSENSITIVE    equ 00000040h

.data
stObjAtr OBJECT_ATTRIBUTES <0>
WSTR szFileName,"\??\c:\1.txt"
UnicodeName UNICODE_STRING <0>

.code
start:
;初始化结构
mov UnicodeName._Length, 24
mov UnicodeName.MaxLength,100
mov UnicodeName.Buffer ,offset szFileName

mov stObjAtr._Length,sizeof stObjAtr
mov stObjAtr.RootDirectory,0
mov stObjAtr.ObjectName,offset UnicodeName
mov stObjAtr.Attributes,OBJ_CASE_INSENSITIVE
mov stObjAtr.SecurityDescriptor,0
mov stObjAtr.SecurityQualityOfService,0


push offset stObjAtr
push @exit
push @exit
mov edx,esp
mov eax,3eh
db 0fh
db 34h
add esp,12
ret
@exit:
invoke ExitProcess,0