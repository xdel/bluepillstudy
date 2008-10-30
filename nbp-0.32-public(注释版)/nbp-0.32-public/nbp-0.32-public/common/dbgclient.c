/* 
 * Copyright holder: Invisible Things Lab
 * 
 * This software is protected by domestic and International
 * copyright laws. Any use (including publishing and
 * distribution) of this software requires a valid license
 * from the copyright holder.
 *
 * This software is provided for the educational use only
 * during the Black Hat training. This software should not
 * be used on production systems.
 *
 */

#include "dbgclient.h"

static PVOID g_DbgWindow = NULL;

static DbgSendCommand (
  ULONG uIoctlNumber,//要执行的I/O control code
  PVOID pData,//DebugWindow局部变量的地址
  ULONG uDataSize//传入的DebugWindow的struct的大小
)
{
  NTSTATUS Status;
  HANDLE hDbgClient;
  IO_STATUS_BLOCK Iosb;
  UNICODE_STRING DeviceName;
  OBJECT_ATTRIBUTES ObjectAttributes;

  RtlInitUnicodeString (&DeviceName, L"\\Device\\itldbgclient");//RtlInitUnicodeString初始化一个计数的Unicode String,保存到DeviceName中。L表示是Unicode类型
  //The InitializeObjectAttributes macro initializes the opaque OBJECT_ATTRIBUTES structure, which specifies the properties of an object handle to routines that open handles.
  //OBJ_CASE_INSENSITIVE:If this flag is specified, a case-insensitive comparison is used when matching the ObjectName parameter against the names of existing objects. Otherwise, object names are compared using the default system settings. 
  InitializeObjectAttributes (&ObjectAttributes, &DeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL);

  hDbgClient = 0;//DbgClient handle
  Status = ZwOpenFile (&hDbgClient, FILE_READ_ACCESS | FILE_WRITE_ACCESS, &ObjectAttributes, &Iosb, FILE_SHARE_READ, 0);//打开设备L"\\Device\\itldbgclient"
  if (!NT_SUCCESS (Status)) {
    DbgPrint ("DbgSendCommand(): ZwOpenFile() failed with status 0x%08X\n", Status);
    return Status;
  }
  //向该设备发送Command
  Status = NtDeviceIoControlFile (hDbgClient,
                                  NULL, NULL, NULL, &Iosb, uIoctlNumber, pData, uDataSize, pData, uDataSize);
  if (!NT_SUCCESS (Status)) {
    DbgPrint ("DbgSendCommand(): NtDeviceIoControlFile() failed with status 0x%08X\n", Status);
    ZwClose (hDbgClient);//关闭DbgClient handle
    return Status;
  }

  ZwClose (hDbgClient);
  return STATUS_SUCCESS;
}
//注册了一个Debug窗口，感觉上应该是通过把这个窗口注册为一个设备(FILE_DEVICE_UNKNOWN，设备名"\\Device\\itldbgclient")，且这个设备就是一段内存空间
//然后往这个内存空间中利用DbgPrintString方法写字符串
NTSTATUS NTAPI DbgRegisterWindow (
  UCHAR bBpId
)
{
  NTSTATUS Status;
  DEBUG_WINDOW DebugWindow;
  //在一个DbgRegisterWindow和一个DbgUnregisterWindow 间只可能是一个g_DbgWindow全局变量被赋值
  //猜想在一个BluePill驱动执行期间只能存在一个DebugWindow
  g_DbgWindow = MmAllocatePages (DEBUG_WINDOW_IN_PAGES, NULL);//为DbgWindow分配需要的空间，但丢弃第一个页的物理地址
  //g_DbgWindow 是Debug窗口的首分配页的虚拟地址
  if (!g_DbgWindow) {//分配未成功
    _KdPrint (("DbgRegisterWindow(): Failed to allocate %d pages for the debug messages window\n",
               DEBUG_WINDOW_IN_PAGES));
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  DebugWindow.bBpId = bBpId;
  DebugWindow.pWindowVA = g_DbgWindow;//实际上对于页来说，hostAddress和virturalAddress获得了同义,不过还是以NewBluePill自定义的虚拟地址去说吧
  DebugWindow.uWindowSize = DEBUG_WINDOW_IN_PAGES * PAGE_SIZE;

  // memory will be freed on memory manager shutdown in case of error
  return DbgSendCommand (IOCTL_REGISTER_WINDOW, &DebugWindow, sizeof (DebugWindow));
}

NTSTATUS NTAPI DbgUnregisterWindow (
)
{
  DEBUG_WINDOW DebugWindow;

  DebugWindow.pWindowVA = g_DbgWindow;
  DebugWindow.uWindowSize = DEBUG_WINDOW_IN_PAGES * PAGE_SIZE;

  return DbgSendCommand (IOCTL_UNREGISTER_WINDOW, &DebugWindow, sizeof (DebugWindow));
}
//向打印窗口打印一段消息(pString)
VOID NTAPI DbgPrintString (
  PUCHAR pString//要打印的字符串
)
{
  if (g_DbgWindow) {

    if (!*(PUCHAR) g_DbgWindow) {//如果原来有内容则不清空
      RtlZeroMemory (g_DbgWindow, DEBUG_WINDOW_IN_PAGES * PAGE_SIZE);
    }

    if (strlen ((PUCHAR) g_DbgWindow + 1) + strlen (pString) >= DEBUG_WINDOW_IN_PAGES * PAGE_SIZE)
      return;

    strcat ((PUCHAR) g_DbgWindow + 1, pString);
    *(PUCHAR) g_DbgWindow = 1;//标识有内容
  }
}
