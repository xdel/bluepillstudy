#include        "defs.h"

ULONG   OldInt1 = 0;
ULONG   OldInt3 = 0;
ULONG   OldInt2d = 0;

ULONG   OldInts1[32];
ULONG   OldInts3[32];
ULONG   OldInts2d[32];


ULONG   int1_hit  = 0;
ULONG   int3_hit  = 0;
ULONG   int2d_hit = 0;

void update_drs(PVOID buffer){
        __asm{
                push    edi
                mov     edi, buffer
                cld
                mov     eax, dr0
                stosd
                mov     eax, dr1
                stosd
                mov     eax, dr2
                stosd
                mov     eax, dr3
                stosd
                mov     eax, dr6
                stosd
                mov     eax, dr7
                stosd
                pop     edi
        }
}
NTSTATUS service_handle(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp){
        PIO_STACK_LOCATION cur_sl = IoGetCurrentIrpStackLocation( pIrp );
        NTSTATUS status           = STATUS_NOT_IMPLEMENTED;
        ULONG   information       = 0;
        PULONG  sbuf;
        
        if (cur_sl->Parameters.DeviceIoControl.IoControlCode == get_ints){
                if (cur_sl->Parameters.DeviceIoControl.OutputBufferLength < 12)
                        status = STATUS_BUFFER_TOO_SMALL;
                else{
                        sbuf = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
                        sbuf[0] = int1_hit;
                        sbuf[1] = int3_hit;
                        sbuf[2] = int2d_hit;
                        information = 12;
                        status = STATUS_SUCCESS; 
                }        
        }
        
        if (cur_sl->Parameters.DeviceIoControl.IoControlCode == get_drs){
                if (cur_sl->Parameters.DeviceIoControl.OutputBufferLength < 6 * sizeof(ULONG))
                        status = STATUS_BUFFER_TOO_SMALL;
                else{
                        update_drs(pIrp->AssociatedIrp.SystemBuffer);
                        status = STATUS_SUCCESS;
                        information = 6*sizeof(ULONG);
                }
        }
        
        pIrp->IoStatus.Status = status;
        pIrp->IoStatus.Information = information;
        IofCompleteRequest(pIrp, IO_NO_INCREMENT);
        return status;
}

NTSTATUS cc_handle(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp){
        
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = 0;
        IofCompleteRequest(pIrp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
}

__declspec(naked) int1(){
        __asm{
                cli
                pushad
                push    es
                push    ds
                push    fs
                mov     eax, 30h
                mov     fs, ax
                mov     ax, 23h
                mov     es, ax
                mov     ds, ax
                lock inc int1_hit
                pop     fs
                pop     ds
                pop     es
                popad
                jmp cs:[OldInt1]
        }
}

__declspec(naked) int3(){
        __asm{
                cli
                pushad
                push    es
                push    ds
                push    fs
                mov     eax, 30h
                mov     fs, ax
                mov     ax, 23h
                mov     es, ax
                mov     ds, ax
                lock inc int3_hit
                pop     fs
                pop     ds
                pop     es
                popad
                jmp cs:[OldInt3]
        }
}

__declspec(naked) int2d(){
        __asm{
                cli
                pushad
                push    es
                push    ds
                push    fs
                mov     eax, 30h
                mov     fs, ax
                mov     ax, 23h
                mov     es, ax
                mov     ds, ax
                lock inc int2d_hit
                pop     fs
                pop     ds
                pop     es
                popad
                jmp cs:[OldInt2d]
        }
}

ULONG   HookInterupt(ULONG NewIntAddress, ULONG IdtVector, PULONG OldIntTable, PULONG OldInterupt){
        ULONG OldIntHandler;
        IDT_BASE idt_base;
        PIDT_ENTRY idt_entry;
        
        __asm   sidt  idt_base      
        idt_entry = (PIDT_ENTRY)((idt_base.IdtBaseHi << 16) + idt_base.IdtBaseLo);

        __asm   pushfd
        __asm   cli

        OldIntHandler = (idt_entry[ IdtVector ].OffsetHigh <<16) + idt_entry[ IdtVector ].OffsetLow;
        idt_entry[ IdtVector ].OffsetHigh = (USHORT)(NewIntAddress >> 16);
        idt_entry[ IdtVector ].OffsetLow  = (USHORT)(NewIntAddress & 0xFFFF);
        
        if (OldIntTable != NULL)
                *OldIntTable = OldIntHandler;
        if (OldInterupt != NULL)
                if (*OldInterupt == 0)
                        *OldInterupt = OldIntHandler;
        
        __asm   popfd                           //if interupts were disabled prior to hook then don't enable them...
        return 0;
}        
        
WCHAR *szDevice  = L"\\Device\\xadt_int";
WCHAR *szSymlink = L"\\DosDevices\\xadt_int";

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegPath){
        PKTHREAD cur_thread;
        UCHAR i;
        UNICODE_STRING us_device, us_symlink;
        PDEVICE_OBJECT pDeviceObject;
        
        RtlInitUnicodeString(&us_device, szDevice);
        RtlInitUnicodeString(&us_symlink, szSymlink);
        
        IoCreateDevice(pDriverObject,
                       0,
                       &us_device,
                       FILE_DEVICE_UNKNOWN,
                       0,
                       FALSE,
                       &pDeviceObject);
        
        IoCreateSymbolicLink(&us_symlink, &us_device);
                
        cur_thread = PsGetCurrentThread();
        
        for (i = 0; i < KeNumberProcessors; i++){
                KeSetAffinityThread(cur_thread, 1 << i);
                HookInterupt((ULONG)int1, 0x1, &OldInts1[i], &OldInt1);
                HookInterupt((ULONG)int3, 0x3, &OldInts3[i], &OldInt3);
                HookInterupt((ULONG)int2d, 0x2d, &OldInts2d[i], &OldInt2d);
        }
        
        pDriverObject->MajorFunction[IRP_MJ_CREATE] =
        pDriverObject->MajorFunction[IRP_MJ_CLOSE]  = cc_handle;
        pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = service_handle;
        
        return STATUS_SUCCESS;
}
               