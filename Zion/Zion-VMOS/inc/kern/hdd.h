#ifndef __KERN_HDD_H
#define __KERN_HDD_H

#include <inc/types.h>
#include <inc/hardcoding_param.h>

#define 	HDBUF_SIZE 		512
#define 	SECTOR_SIZE		512


/* I/O Ports used by hard disk (primary) controllers. */
#define 		REG_DATA				(HD_IOPORT_BASE + 0)		// Data 	[I/O]
#define 		REG_FEATURES		(HD_IOPORT_BASE + 1)		// Features 		[O]
#define 		REG_NSECTOR			(HD_IOPORT_BASE + 2)		// Sector Count 		[I/O]
#define 		REG_LBA_LOW		(HD_IOPORT_BASE + 3)		// Sector Number / LBA Bits 0-7	[I/O]
#define 		REG_LBA_MID			(HD_IOPORT_BASE + 4)		// Cylinder Low / LBA Bits 8-15	[I/O]
#define 		REG_LBA_HIGH		(HD_IOPORT_BASE + 5)		// Cylinder High / LBA Bits 16-23	[I/O]
#define 		REG_DEVICE			(HD_IOPORT_BASE + 6)		// Drive | Head | LBA bits 24-27	[I/O]
#define 		REG_STATUS			(HD_IOPORT_BASE + 7)		// Status 		[I]
#define 		REG_ERROR				REG_FEATURES					// Error			[I]
#define 		REG_CMD					REG_STATUS						// Command 	[O]


#define		STATUS_BSY			0x80
#define		STATUS_DRDY			0x40
#define		STATUS_DFSE			0x20
#define		STATUS_DSC			0x10
#define		STATUS_DRQ			0x08
#define		STATUS_CORR			0x04
#define		STATUS_IDX				0x02
#define		STATUS_ERR			0x01



/* device numbers of hard disk */
#define		MINOR_hd1a			0x10
#define		MINOR_hd2a			0x20
#define		MINOR_hd2b			0x21
#define		MINOR_hd3a			0x30
#define		MINOR_hd4a			0x40

#define		ROOT_DEV		MAKE_DEV(DEV_HD, MINOR_BOOT)	/* 3, 0x21 */

#define		INVALID_INODE			0
#define		ROOT_INODE				1

#define		MAX_DRIVES				2
#define		NR_PART_PER_DRIVE	4
#define		NR_SUB_PER_PART		16
#define		NR_SUB_PER_DRIVE	(NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define		NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)


struct hd_cmd {
	u8	features;
	u8	sector_cnt_0_7;
	u8 	sector_cnt_8_15;
	u8	lba_0_7;
	u8	lba_8_15;
	u8	lba_16_23;
	u8	lba_24_31;
	u8	lba_32_39;
	u8	lba_40_47;
	u8	device;
	u8	command;
};


#define	HD_TIMEOUT		3000
#define 	CMD_IDENTIFY		0xEC
#define 	CMD_READ			0x20
#define 	CMD_WRITE			0x30

/* for DEVICE register. */
#define	MAKE_DEVICE_REG(lba,drv,lba_highest) (	\
						((lba) << 6) | 										\
						((drv) << 4) | 										\
						(lba_highest & 0xF) | 							\
						0xA0)

#define 	HD_READ 		0x1
#define 	HD_WRITE 		0x2

#define 	DEV_HD_MASTER 		0
#define 	DEV_HD_SLAVE	 		1

void init_hd ( void );
void hd_identify ( int drive );
void hd_rw ( u8 rw, u8 *hdbuf, u64 sect_nr, u16 sector_cnt );


#endif /* _HDD_H_ */
