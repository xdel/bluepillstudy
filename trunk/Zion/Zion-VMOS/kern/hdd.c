//#define 	__HDD_DEBUG__

#include <inc/kern/hdd.h>
#include <inc/lib/stdio.h>
#include <inc/memlayout.h>
#include <inc/arch/port_op.h>

#define 	ADDR_OFFSET  	KERNBASE

static void print_identify_info(u16* hdinfo);
static void hd_cmd_out(struct hd_cmd* cmd);
static int waitfor(int mask, int val, uint timeout);
#ifdef __HDD_DEBUG__
static void TEST_output_hdbuf ( u16 *hdbuf, u32 size16_t );
#endif

/**************************************************
 * Function: 		Initialize hard disk.
 * Description: 	Check hard drive.
 * @param: 		<none>
 * @return: 		<none>
 * ************************************************/
void 
init_hd ( void )
{
	// Get the number of drives from the BIOS data area
	u8 	*pNrDrives = (u8*)(0x475 + ADDR_OFFSET);
	cprintf("\tNumber of hard disk drives: %d\n", *pNrDrives);
}//init_hd()



/**************************************************
 * Function: 		Identify hard disk.
 * Description: 	Get the hard disk information.
 * @param: 		drive - Driver number.
 * @return: 		<none>
 * ************************************************/
void 
hd_identify ( int drive )
{
	u8 hdbuf[HDBUF_SIZE];
	struct hd_cmd 	cmd = {0};

	cmd.device  = MAKE_DEVICE_REG(0, drive, 0);
	cmd.command = CMD_IDENTIFY;
	hd_cmd_out(&cmd);
	if ( !waitfor(STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT) ) {
		cprintf("Error: Wait for data request status timeout!\n");
		return;
	}//if
	
	((void (*)(u16, void*, int))port_read)(REG_DATA, hdbuf, HDBUF_SIZE);

	print_identify_info((u16*)hdbuf);
}//hd_identify()



/**************************************************
 * Function: 		Print hard disk identify information.
 * Description: 	Print the hard disk information retrieved via ATA_IDENTIFY command.
 * @param: 		hdinfo - The buffer read from the disk i/o port.
 * @return: 		<none>
 * ************************************************/
static void 
print_identify_info ( u16 *hdinfo )
{
 // Display hard disk serial number and model.
	uint 		i, k;
	char 		s[64];
	struct iden_info_ascii {
		uint 	idx;
		uint 	len;
		char const *description;
	} iinfo[] = {
		{10, 20, "HD SN"}, 		// Serial number in ASCII
		{27, 20, "HD Model"} 	// Model number in ASCII
		};

	for ( k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
		char * p = (char*)&hdinfo[iinfo[k].idx];
		for ( i = 0; i < iinfo[k].len/2; i++) {
			s[i*2+1] = *p++;
			s[i*2] = *p++;
		}//for(i)
		s[i*2] = 0;
		cprintf("\t%s: %s\n", iinfo[k].description, s);
	}//for(k)

#ifdef __HDD_DEBUG__
	TEST_output_hdbuf(hdinfo, HDBUF_SIZE/2);
#endif

	int capabilities = hdinfo[49];
	cprintf("\tLBA supported: %s\n", (capabilities & 0x0200) ? "Yes" : "No");

	int cmd_set_supported = hdinfo[83];
	cprintf("\tLBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "Yes" : "No");

	uint64_t sectors = ((uint64_t)hdinfo[103] << 48) + ((uint64_t)hdinfo[102] << 32) + ((uint64_t)hdinfo[101] << 16) + hdinfo[100];
	cprintf("\tHD size: %d MB\n", sectors * 512 / 0x100000);
}//print_identify_info()



void 
hd_rw ( u8 rw, u8 *hdbuf, u64 sect_nr, u16 sector_cnt )
{
	struct hd_cmd cmd = {0};
	
	cmd.features	= 0;
	cmd.sector_cnt_0_7 = sector_cnt & 0xFF;
	cmd.sector_cnt_8_15 = (sector_cnt >> 8) & 0xFF;
	cmd.lba_0_7 = sect_nr & 0xFF;
	cmd.lba_8_15 = (sect_nr >>  8) & 0xFF;
	cmd.lba_16_23 = (sect_nr >> 16) & 0xFF;
	cmd.lba_24_31 = (sect_nr >> 24) & 0xFF;
	cmd.lba_32_39 = (sect_nr >> 32) & 0xFF;
	cmd.lba_40_47 = (sect_nr >> 40) & 0xFF;
	cmd.device	= MAKE_DEVICE_REG(1, DEV_HD_MASTER, (sect_nr >> 24) & 0xF);
	cmd.command	= (rw == HD_READ) ? CMD_READ : CMD_WRITE;
	hd_cmd_out(&cmd);

	if ( rw == HD_READ ) { 	// For hard disk read.
		while ( sector_cnt ) {
			if ( !waitfor(STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT) ) {
				cprintf("\tError: Wait for data request status timeout!\n");
				return;
			}//if
			((void (*)(u16, void*, int))port_read)(REG_DATA, hdbuf, SECTOR_SIZE);
			hdbuf += SECTOR_SIZE;
			sector_cnt--;
		}//while
	} else {	// For hard disk write.
		cprintf("\tWarning: Hard disk write has not been implemented.\n");
		return;
	}//if...else

#ifdef __HDD_DEBUG__
	hdbuf -= SECTOR_SIZE;
	TEST_output_hdbuf((u16 *)hdbuf, HDBUF_SIZE/2);
#endif

}//hdd_rw()


/**************************************************
 * Function: 		Hard disk command output.
 * Description: 	Output a command to HD controller.
 * @param: 		cmd - The command struct pointer.
 * @return: 		<none>
 * ************************************************/
static void 
hd_cmd_out ( struct hd_cmd* cmd )
{
	/**
	 * For all commands, the host must first check if BSY=1,
	 * and should proceed no further unless and until BSY=0
	 */
	if ( !waitfor(STATUS_BSY, 0, HD_TIMEOUT) ) {
		cprintf("Error: Wait for hard disk busy status timeout!\n");
		return;
	}

	/* Activate the Interrupt Enable (nIEN) bit */
//	((void (*)(u16, u8 ))out_byte)(REG_DEV_CTRL, 0);

	/* Load required parameters in the Command Block Registers */
	((void (*)(u16, u8 ))out_byte)(REG_FEATURES, cmd->features);
	((void (*)(u16, u8 ))out_byte)(REG_NSECTOR,  cmd->sector_cnt_8_15);
	((void (*)(u16, u8 ))out_byte)(REG_NSECTOR,  cmd->sector_cnt_0_7);
	((void (*)(u16, u8 ))out_byte)(REG_LBA_HIGH, cmd->lba_40_47);
	((void (*)(u16, u8 ))out_byte)(REG_LBA_MID,  cmd->lba_32_39);
	((void (*)(u16, u8 ))out_byte)(REG_LBA_LOW,  cmd->lba_24_31);
	((void (*)(u16, u8 ))out_byte)(REG_LBA_HIGH, cmd->lba_16_23);
	((void (*)(u16, u8 ))out_byte)(REG_LBA_MID,  cmd->lba_8_15);
	((void (*)(u16, u8 ))out_byte)(REG_LBA_LOW,  cmd->lba_0_7);
	((void (*)(u16, u8 ))out_byte)(REG_DEVICE,   cmd->device);
	
	/* Write the command code to the Command Register */
	((void (*)(u16, u8 ))out_byte)(REG_CMD, cmd->command);
}//hd_cmd_out()



/**************************************************
 * Function: 		Wait.
 * Description: 	Wait for a certain status.
 * @param: 		mask - Status mask.
 * 					val - Required status.
 * 					timeout - Timeout value.
 * @return: 		1 if succeed, 0 if timeout.
 * ************************************************/
static int 
waitfor ( int mask, int val, uint timeout )
{
	u8 status = 0;
	uint		t = 0;

	while ( t++ < timeout ) {
		status = ((u8 (*)(u16))in_byte)(REG_STATUS);
#ifdef __HDD_DEBUG__
		cprintf("HD_Status=0x%02x, mask=0x%02x, result=0x%02x\n", status, mask, (status & mask));
#endif
		if ( (status & mask) == val ) {
			return 1;
		}//if
	}//while

	return 0;
}//waitfor()


#ifdef __HDD_DEBUG__
// TEST: Output hdbufer.
static void 
TEST_output_hdbuf ( u16 *hdbuf, u32 size16_t )
{
	for ( u32 i=0; i<size16_t ; i++ ) {
		if ( (i % 100) == 0) {
			getchar();
		}
		if ( (i % 10) == 0 ) {
			cprintf("\n[%3d] ", i);
		}//if
		cprintf("%04x ", hdbuf[i]);
	}//for
	cprintf("\n");
}//TEST_output_hdbuf
#endif



