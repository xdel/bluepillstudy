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

#ifndef _DBGCLIENT_IOCTL_
# define _DBGCLIENT_IOCTL_

# define DBGCLIENT_DEVICE	FILE_DEVICE_UNKNOWN
//Use the system-supplied CTL_CODE macro, which is defined in wdm.h and ntddk.h, to define new I/O control codes
//返回I/O control code
# define IOCTL_REGISTER_WINDOW CTL_CODE(DBGCLIENT_DEVICE, 0x1, METHOD_BUFFERED, FILE_WRITE_ACCESS)
# define IOCTL_UNREGISTER_WINDOW CTL_CODE(DBGCLIENT_DEVICE, 0x2, METHOD_BUFFERED, FILE_WRITE_ACCESS)

# define DEBUG_WINDOW_IN_PAGES	5

typedef struct _DEBUG_WINDOW
{
  UCHAR bBpId;//存储该Debug Window所属的BluePill 的ID号
  PVOID pWindowVA;//存储Debug Window所在的HostAddress
  ULONG uWindowSize;//存储Debug Window占用的Windows虚拟地址的Size
} DEBUG_WINDOW,
 *PDEBUG_WINDOW;

#endif // _DBGCLIENT_IOCTL_
