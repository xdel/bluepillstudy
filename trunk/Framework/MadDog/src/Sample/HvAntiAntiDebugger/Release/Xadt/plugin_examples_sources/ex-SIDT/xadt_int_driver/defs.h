#include        "ntifs.h"

typedef struct{
        unsigned Limit:16;
        unsigned IdtBaseLo:16;
        unsigned IdtBaseHi:16;
}IDT_BASE, *PIDT_BASE;

typedef struct{
        unsigned OffsetLow:16;
        unsigned SegmentSelector:16;
        unsigned Reserved:5;
        unsigned Reverved1:3;
        unsigned Type:3;
        unsigned Size:1;
        unsigned Reserved2:1;
        unsigned Dpl:2;
        unsigned Present:1;
        unsigned OffsetHigh:16;
}IDT_ENTRY, *PIDT_ENTRY;

__declspec(dllimport) NTSTATUS KeSetAffinityThread(IN PKTHREAD, IN KAFFINITY);

#define get_ints CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define get_drs  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
