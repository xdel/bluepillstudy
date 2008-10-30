#include "dbgclient.h"
#include "dbgclient_ioctl.h"


LIST_ENTRY	g_DebugWindowsList;
KMUTEX	g_DebugWindowsListMutex;
KEVENT	g_ShutdownEvent;
PETHREAD	g_pScanWindowsThread=NULL;

PUCHAR	g_pDebugString=NULL;


static VOID PrintData()
{
	PDEBUG_WINDOW_ENTRY	pRegisteredWindow;
	PUCHAR	pData,pString;
	ULONG	i,uWindowSize;


	KeWaitForSingleObject(&g_DebugWindowsListMutex,Executive,KernelMode,FALSE,NULL);

	pRegisteredWindow=(PDEBUG_WINDOW_ENTRY)g_DebugWindowsList.Flink;
	while (pRegisteredWindow!=(PDEBUG_WINDOW_ENTRY)&g_DebugWindowsList) {
		pRegisteredWindow=CONTAINING_RECORD(pRegisteredWindow,DEBUG_WINDOW_ENTRY,le);

		pData=pRegisteredWindow->DebugWindow.pWindowVA;
			
		if (pData[0]) {

			uWindowSize=min(pRegisteredWindow->DebugWindow.uWindowSize-1,DEBUG_WINDOW_IN_PAGES*PAGE_SIZE);
			RtlCopyMemory(g_pDebugString,&pData[1],uWindowSize);
			pData[0]=0;

			pString=g_pDebugString;
			for (i=0;i<uWindowSize,g_pDebugString[i];i++) {
				if (g_pDebugString[i]==0x0a) {
					g_pDebugString[i]=0;
					DbgPrint("<%02X>:  %s\n",pRegisteredWindow->DebugWindow.bBpId,pString);
					pString=&g_pDebugString[i+1];
				}
			}
			if (*pString)
				DbgPrint("<%02X>:  %s\n",pRegisteredWindow->DebugWindow.bBpId,&pString);

			RtlZeroMemory(g_pDebugString,DEBUG_WINDOW_IN_PAGES*PAGE_SIZE);
		}

		pRegisteredWindow=(PDEBUG_WINDOW_ENTRY)pRegisteredWindow->le.Flink;
	}

	KeReleaseMutex(&g_DebugWindowsListMutex,FALSE);	
}

static VOID NTAPI ScanWindowsThread(PVOID Param)
{
	LARGE_INTEGER	Interval;


	Interval.QuadPart=RELATIVE(MILLISECONDS(10));

	while (STATUS_TIMEOUT==KeWaitForSingleObject(
							&g_ShutdownEvent,
							Executive,
							KernelMode,
							FALSE,
							&Interval)) {

		PrintData();
	}


	DbgPrint("ScanWindowsThread(): Shutting down\n");

	PsTerminateSystemThread(STATUS_SUCCESS);
}


NTSTATUS DeviceControl(
			IN PFILE_OBJECT pFileObject,
			IN PVOID pInputBuffer,
			IN ULONG uInputBufferLength,
			OUT PVOID pOutputBuffer,
			IN ULONG uOutputBufferLength,
			IN ULONG uIoControlCode,
			OUT PIO_STATUS_BLOCK pIoStatusBlock,
			IN PDEVICE_OBJECT pDeviceObject)
{
	PDEBUG_WINDOW	pDebugWindow=pInputBuffer;
	PDEBUG_WINDOW_ENTRY	pDwe,pRegisteredWindow;
	BOOLEAN	bFound;
	NTSTATUS	Status=STATUS_SUCCESS;


	switch (uIoControlCode) {
		case IOCTL_REGISTER_WINDOW:

			if (!pInputBuffer || uInputBufferLength!=sizeof(DEBUG_WINDOW) || pDebugWindow->pWindowVA<MM_SYSTEM_RANGE_START) {
				pIoStatusBlock->Status=STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			pDwe=ExAllocatePool(PagedPool,sizeof(DEBUG_WINDOW_ENTRY));
			if (!pDwe) {
				pIoStatusBlock->Status=STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			pDwe->DebugWindow=*pDebugWindow;

			KeWaitForSingleObject(&g_DebugWindowsListMutex,Executive,KernelMode,FALSE,NULL);

			bFound=FALSE;
			pRegisteredWindow=(PDEBUG_WINDOW_ENTRY)g_DebugWindowsList.Flink;
			while (pRegisteredWindow!=(PDEBUG_WINDOW_ENTRY)&g_DebugWindowsList) {
				pRegisteredWindow=CONTAINING_RECORD(pRegisteredWindow,DEBUG_WINDOW_ENTRY,le);

				if (pRegisteredWindow->DebugWindow.pWindowVA==pDebugWindow->pWindowVA) {
					bFound=TRUE;
					break;
				}

				pRegisteredWindow=(PDEBUG_WINDOW_ENTRY)pRegisteredWindow->le.Flink;
			}

			if (!bFound) {
				pDwe->pWindowMdl=IoAllocateMdl(pDwe->DebugWindow.pWindowVA,pDwe->DebugWindow.uWindowSize,FALSE,FALSE,NULL);
				if (!pDwe) {
					ExFreePool(pDwe);
					Status=STATUS_INSUFFICIENT_RESOURCES;
				} else {
					try {
						MmProbeAndLockPages(pDwe->pWindowMdl,KernelMode,IoReadAccess);
						InsertTailList(&g_DebugWindowsList,&pDwe->le);

						DbgPrint("dbgclient: NBP <%02X> registered, window at 0x%p, size: 0x%X\n",
							pDwe->DebugWindow.bBpId,
							pDwe->DebugWindow.pWindowVA,
							pDwe->DebugWindow.uWindowSize);

					} except(EXCEPTION_EXECUTE_HANDLER) {
						Status=STATUS_UNSUCCESSFUL;
						ExFreePool(pDwe);
					}
				}
			} else
				ExFreePool(pDwe);

			KeReleaseMutex(&g_DebugWindowsListMutex,FALSE);

			pIoStatusBlock->Information=0;
			pIoStatusBlock->Status=Status;
			break;

		case IOCTL_UNREGISTER_WINDOW:


			if (!pInputBuffer || uInputBufferLength!=sizeof(DEBUG_WINDOW)) {
				pIoStatusBlock->Status=STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			PrintData();

			KeWaitForSingleObject(&g_DebugWindowsListMutex,Executive,KernelMode,FALSE,NULL);

			bFound=FALSE;
			pRegisteredWindow=(PDEBUG_WINDOW_ENTRY)g_DebugWindowsList.Flink;
			while (pRegisteredWindow!=(PDEBUG_WINDOW_ENTRY)&g_DebugWindowsList) {
				pRegisteredWindow=CONTAINING_RECORD(pRegisteredWindow,DEBUG_WINDOW_ENTRY,le);

				if (pRegisteredWindow->DebugWindow.pWindowVA==pDebugWindow->pWindowVA) {
					RemoveEntryList(&pRegisteredWindow->le);

					MmUnlockPages(pRegisteredWindow->pWindowMdl);
					IoFreeMdl(pRegisteredWindow->pWindowMdl);

					ExFreePool(pRegisteredWindow);
					bFound=TRUE;

					DbgPrint("dbgclient: NBP <%02X> unregistered\n",pRegisteredWindow->DebugWindow.bBpId);
					break;
				}

				pRegisteredWindow=(PDEBUG_WINDOW_ENTRY)pRegisteredWindow->le.Flink;
			}
			KeReleaseMutex(&g_DebugWindowsListMutex,FALSE);

			pIoStatusBlock->Information=0;

			if (!bFound) {
				pIoStatusBlock->Status=STATUS_UNSUCCESSFUL;
			} else {
				pIoStatusBlock->Status=STATUS_SUCCESS;
			}
			break;
		default:
			pIoStatusBlock->Status=STATUS_INVALID_DEVICE_REQUEST;
	}

	return pIoStatusBlock->Status;
}

NTSTATUS DriverDispatcher(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	PIO_STACK_LOCATION	pIrpStack;
	PVOID	pInputBuffer,pOutputBuffer;
	ULONG	uInputBufferLength,uOutputBufferLength,uIoControlCode;
	NTSTATUS	Status;


	Status=pIrp->IoStatus.Status=STATUS_SUCCESS;
	pIrp->IoStatus.Information=0;

	pIrpStack=IoGetCurrentIrpStackLocation(pIrp);

	pInputBuffer             = pIrp->AssociatedIrp.SystemBuffer;
	uInputBufferLength       = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	pOutputBuffer            = pIrp->AssociatedIrp.SystemBuffer;
	uOutputBufferLength      = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	uIoControlCode           = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

	switch (pIrpStack->MajorFunction) {
		case IRP_MJ_DEVICE_CONTROL:
			Status=DeviceControl(
					pIrpStack->FileObject,
					pInputBuffer,
					uInputBufferLength,
					pOutputBuffer,
					uOutputBufferLength,
					uIoControlCode,
					&pIrp->IoStatus,
					pDeviceObject);
		break;
	}

	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	return Status;
}


VOID NTAPI DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING	DeviceLink;
	PDEBUG_WINDOW_ENTRY	pRegisteredWindow;



	KeSetEvent(&g_ShutdownEvent,0,FALSE);

	if (g_pScanWindowsThread) {
		KeWaitForSingleObject(g_pScanWindowsThread,Executive,KernelMode,FALSE,NULL);
		ObDereferenceObject(g_pScanWindowsThread);
	}

	RtlInitUnicodeString(&DeviceLink,L"\\DosDevices\\itldbgclient");
	IoDeleteSymbolicLink(&DeviceLink);

	if (DriverObject->DeviceObject)
		IoDeleteDevice(DriverObject->DeviceObject);

	PrintData();

	KeWaitForSingleObject(&g_DebugWindowsListMutex,Executive,KernelMode,FALSE,NULL);
	while (!IsListEmpty(&g_DebugWindowsList)) {
		pRegisteredWindow=(PDEBUG_WINDOW_ENTRY)RemoveHeadList(&g_DebugWindowsList);
		pRegisteredWindow=CONTAINING_RECORD(pRegisteredWindow,DEBUG_WINDOW_ENTRY,le);

		MmUnlockPages(pRegisteredWindow->pWindowMdl);
		IoFreeMdl(pRegisteredWindow->pWindowMdl);

		ExFreePool(pRegisteredWindow);
	}
	KeReleaseMutex(&g_DebugWindowsListMutex,FALSE);

	if (g_pDebugString)
		ExFreePool(g_pDebugString);

	DbgPrint("dbgclient: Shut down\n");
}



//BluePill Debug信息的消费者，以注册驱动的方式出现，Debug信息的生产者会把信息写到\\Device\\itldbgclient的共享内存空间上
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	NTSTATUS	Status;
	UNICODE_STRING	DeviceLink,DeviceName;
	PDEVICE_OBJECT	pDeviceObject;
	HANDLE	hThread;


	DriverObject->DriverUnload=DriverUnload;

	RtlInitUnicodeString(&DeviceName,L"\\Device\\itldbgclient");//声明了两个设备（这个设备在common/dbgclient.c中也有，因为那个是有对这个设备的发送debug信息操作）
	RtlInitUnicodeString(&DeviceLink,L"\\DosDevices\\itldbgclient");


	g_pDebugString=ExAllocatePool(PagedPool,DEBUG_WINDOW_IN_PAGES*PAGE_SIZE);
	if (!g_pDebugString) {
		DbgPrint("dbgclient: Failed to allocate %d bytes for debug window buffer\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	// The IoCreateDevice routine creates a device object for use by a driver.
	Status=IoCreateDevice(DriverObject,0,&DeviceName,DBGCLIENT_DEVICE,0,FALSE,&pDeviceObject);//创建出来的设备放在了pDeviceObject指针上
	if (!NT_SUCCESS(Status)) {
		DbgPrint("dbgclient: IoCreateDevice() failed with status 0x%08X\n",Status);
		return Status;
	}

	Status=IoCreateSymbolicLink(&DeviceLink,&DeviceName);//创建了符号连接，其中DeviceLink是对于该设备的用户可见名，后者是device object name
	if (!NT_SUCCESS(Status)) {
		IoDeleteDevice(DriverObject->DeviceObject);
		DbgPrint("dbgclient: IoCreateSymbolicLink() failed with status 0x%08X\n",Status);
		return Status;
	}

	InitializeListHead(&g_DebugWindowsList);
	KeInitializeMutex(&g_DebugWindowsListMutex,0);
	//The KeInitializeEvent routine initializes an event object as a synchronization (single waiter) or notification type event and sets it to a signaled or not signaled state.
	//在这里是自定义了一个Event,而且是NotificationEvent:Drivers typically use a NotificationEvent to wait for an I/O operation to complete
	//不过是unsignaled.
	KeInitializeEvent(&g_ShutdownEvent,NotificationEvent,FALSE);
	//The PsCreateSystemThread routine creates a system thread that executes in kernel mode and returns a handle for the thread.	
	//不过创建方式没懂
	if (!NT_SUCCESS(Status=PsCreateSystemThread(&hThread,
							(ACCESS_MASK)0L,
							NULL,
							0,
							NULL,
							ScanWindowsThread,
							NULL))) {

			DbgPrint("dbgclient: Failed to start ScanWindowsThread, status 0x%08X\n",Status);
			IoDeleteDevice(DriverObject->DeviceObject);
			IoDeleteSymbolicLink(&DeviceLink);
			return Status;
		}

	if (!NT_SUCCESS(Status=ObReferenceObjectByHandle(
							hThread,
							THREAD_ALL_ACCESS,
							NULL,
							KernelMode,
							&g_pScanWindowsThread,
							NULL))) {

		DbgPrint("HelloWorldDriver: Failed to get thread object of the ScanWindowsThread, status 0x%08X\n",Status);
		ZwClose(hThread);
		IoDeleteDevice(DriverObject->DeviceObject);
		IoDeleteSymbolicLink(&DeviceLink);
		return Status;
	}

	ZwClose(hThread);


	DriverObject->MajorFunction[IRP_MJ_CREATE]          =
	DriverObject->MajorFunction[IRP_MJ_CLOSE]           =
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = DriverDispatcher;

	DbgPrint("dbgclient: Initialized\n");

	return STATUS_SUCCESS;
}

