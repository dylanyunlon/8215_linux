/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 *  Copyright(C) 2006 NXP BV, All rights reserved.
 *  by Jean-Paul Saman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ac83xx_basic.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <i2c.h>
#include <asm/mach-types.h>

#include <asm/arch/x_typedef.h>
#include <asm/arch/x_bim.h>
//#include <usb.h>
#include <ac83xx_gpio.h>

#include <asm/arch/ac83xx_part_tbl.h>
#include <asm/arch/ac83xx_upg_status.h>
#include <stdio_dev.h>
#include "drv_config_mem.h"
#include <asm/arch/args_to_uboot.h>
#include <upg_config.h>

#include <common.h>
#include <command.h>
#include <s_record.h>
#include <net.h>
#include <ata.h>
#include <part.h>
#include <fat.h>

#include <command.h>
#include <mmc.h>

/*
 *
 * New NAND support
 *
 */
#include <linux/mtd/mtd.h>

#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <asm/arch/x_typedef.h>
#define SD_READ32(_base, _offset) *((volatile uint32_t *)((_base + IO_BASE)+ (_offset)))

#define SD_WRITE32(_base, _offset, _data) \
(*((volatile uint32_t *)(((uint32_t)_base + IO_BASE) + (_offset))) = (_data))
#define SD_SETBIT(_base, offset, dBit) \
SD_WRITE32(_base, offset, SD_READ32(_base, offset) | (dBit))
#define SD_CLRBIT(_base, offset, dBit) \
SD_WRITE32(_base, offset, SD_READ32(_base, offset) & (~(dBit)))
int image_in_sd = 0;
//#define KERNEL_DIRECTORY "/upgrade"
#define KERNEL_DIRECTORY "/upgrade"
#define KERNEL_NAME "uimage"
#define ROOTFS_NAME "ramdisk.gz"
#define UBIDATA_NAME "userdata.ubifs"
#define UBISYSTEM_NAME "system.ubifs"

#define USRDATA_NAME "saveuserdata"


#define ARM2_NAME "arm2.nb0"
#define BACKCAR_NAME "logo.mrf"

#define KERNEL_OFFSET  0x200000
#define KERNEL_SIZE    0x200000
#define KERNEL_SD_INDEX 1
extern int do_mtdparts(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

static int is_upgradeimage = 0;

void mtdparts_set()
{
	int argc = 2;
	char *argv[] = {"mtdparts", "default"};
	do_mtdparts(NULL, 0, argc, argv);
}

static inline char hextostring(int x)
{
	if ( x <0 || x > 15){
		printf("hex reverse error\r\n");
		return '\0';
	}

	if ( x>=0 && x <=9)
		return x+'0';

	switch(x){
		case 10:
			return 'a';
		case 11:
			return 'b';
		case 12:
			return 'c';
		case 13:
			return 'd';
		case 14:
			return 'e';
		case 15:
			return 'f';
	}
}
static inline int ultohex(char *p, ulong num)
{

	int i = 0;
	ulong tmp = num;
	p[8] = '\0';
	while((tmp>>4) != 0){	
		p[7-i] = hextostring(tmp - (tmp>>4) * 16);			
		tmp = tmp >> 4;		
		i++;
	}
	
	p[7-i] = hextostring(tmp);
	return i;
}


static inline int str2long(char *p, ulong *num)
{
	char *endptr;

	*num = simple_strtoul(p, &endptr, 16);
	return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}

extern int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);

int nand_format_usrdata_sd (char *partitionName)
{
	char *argv[4] = {"nand", "format"};
	
	argv[2] = partitionName;
	return do_nand(NULL, 0, 3, argv);
}

int write_nand_sd (u32 base_addr, char *partitionName, unsigned size)
{
	char buf1[10] = {0};
	char buf2[10] = {0};
	char *argv[6] = {"nand", "write"};

	sprintf(buf1, "%x", base_addr);
	argv[2] = buf1;
	argv[3] = partitionName;
	sprintf(buf2, "%x", size);
	argv[4] = buf2;
	return do_nand(NULL, 0, 5, argv);
}



void nand_erase_image(const char* name)
{
	int argc = 3;
	char *argv[] = {"nand", "erase", name};
	do_nand(NULL, 0, argc, argv);

}

void nand_write_image(const char* name, const char* offset)
{
	int argc = 4;
	char *argv[] = {"nand", "write", offset, name};
	do_nand(NULL, 0, argc, argv);
}


/*****************************************
 * Routine: read_kernel
 * Description: read kernel from sd.
 *****************************************/

int read_image(const char* name, unsigned long offset)
{	
	char *dirname = KERNEL_DIRECTORY;
	int ret;
	char filename[100];
	int dev=KERNEL_SD_INDEX;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;
	

	image_in_sd = 0;
	dev_desc=get_dev("mmc", dev);
	if (dev_desc==NULL) {
		printf ("\n** Invalid boot device **\n");
		return ;
	}
	if (fat_register_device(dev_desc,part)!=0) {
		printf ("\n** Unable to use %s %d:%d for fatls **\n","mmc",dev,part);
		return ;
	}
	ret = do_fat_read_update_kerenl(dirname, NULL, 0, LS_YES, name);

	// no uImage ,return
	if(-2 != ret){
		image_in_sd= 0;
		printf("No %s in sd card\n", name);
		return;
	}
	image_in_sd = 1;

	sprintf(filename, "%s/%s", dirname, name);
	return  file_fat_read (filename, (unsigned char *)(offset), 0);
}
int mmc_init_upgrade()
{
	struct mmc *mmc;
	int dev_num;

	mmc = find_mmc_device(KERNEL_SD_INDEX);

	if (mmc)
		return mmc_init(mmc);		
}

/*****************************************
 * Routine: kernel_update
 * Description: kernel update auto.
 *****************************************/
void kernel_update()
{	
	int ret;
	/* init the sd card */

	/* read the kernel from the card*/
	ret = read_image(KERNEL_NAME, 0x6000000);
	if (-1 == ret){
		printf("Read %s error! \r\n", KERNEL_NAME);
		return;
	}
	if (0 == image_in_sd)
		return;

	is_upgradeimage = 1;

	nand_erase_image("kernel");
	nand_write_image("kernel", "0x6000000");

	printf("/***********************************************/\r\n");
	printf("/*            Upgrade kernel successed!        */\r\n");
	printf("/***********************************************/\r\n");
	return;	
}



/*****************************************
 * Routine: arm2_update
 * Description: arm2 update auto.
 *****************************************/
void arm2_update()
{	
	int ret;
	/* init the sd card */

	/* read the arm2_update from the card*/
	ret = read_image(ARM2_NAME, 0x6000000);
	if (-1 == ret){
		printf("Read %s error! \r\n", ARM2_NAME);
		return;
	}
	if (0 == image_in_sd)
		return;

	is_upgradeimage = 1;

	nand_erase_image("arm2");
	nand_write_image("arm2", "0x6000000");

	printf("/***********************************************/\r\n");
	printf("/*            Upgrade arm2 successed!        */\r\n");
	printf("/***********************************************/\r\n");
	return;	
}


/*****************************************
 * Routine: logo_update
 * Description: logo_update auto.
 *****************************************/
void logo_update()
{	
	int ret;
	/* init the sd card */

	/* read the logo_update from the card*/
	ret = read_image(BACKCAR_NAME, 0x6000000);
	if (-1 == ret){
		printf("Read %s error! \r\n", BACKCAR_NAME);
		return;
	}
	if (0 == image_in_sd)
		return;

	is_upgradeimage = 1;

	nand_erase_image("logo");
	nand_write_image("logo", "0x6000000");

	printf("/***********************************************/\r\n");
	printf("/*            Upgrade logo successed!        */\r\n");
	printf("/***********************************************/\r\n");
	return;	

}

/*****************************************
 * Routine: rootfs_update
 * Description: kernel update auto.
 *****************************************/
void rootfs_update()
{	
	ulong ret;
	/* init the sd card */

	/* read the kernel from the card*/
	ret = read_image(ROOTFS_NAME, 0x6000000);
	if (-1 == ret){
		printf("Read %s error! \r\n", ROOTFS_NAME);
		return;
	}
	
	if (0 == image_in_sd)
		return;

	is_upgradeimage = 1;

	nand_erase_image("rootfs");
	nand_write_image("rootfs", "0x6000000");
	printf("/***********************************************/\r\n");
	printf("/*            Upgrade rootfs successed!        */\r\n");
	printf("/***********************************************/\r\n");
	return;	
}

extern int do_ubi(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);
/*****************************************
 * Routine: ubi_data_update
 * Description: data update auto.
 *       
 *****************************************/
void ubi_data_update()
{	
	unsigned ret;
	unsigned num;
	/* init the sd card */

	/* read the kernel from the card*/
	ret = read_image(UBIDATA_NAME, 0x8000000);
	if (-1 == ret){
		printf("Read %s error! \r\n", ROOTFS_NAME);
		return;
	}
	
	if (0 == image_in_sd)
		return;

	is_upgradeimage = 1;

	nand_erase_image("data");
#if 1
	{
		int argc = 3;
		char *argv[] = {"ubi", "part", "data"};
		do_ubi(NULL, 0, argc, argv);
	}

	{
		int argc = 4;
		char *argv[] = {"ubi", "create", "data", "0x11500000"};
		do_ubi(NULL, 0, argc, argv);
	}
	
	{	
		int argc = 5;
		char data_size[9];
		num = ultohex(data_size, ret);
		printf("ret = %d, data_size = %s\r\n", ret, &data_size[7 - num]);

		char *argv[] = {"ubi", "write", "0x8000000", "data", (&data_size[7 - num])};
		do_ubi(NULL, 0, argc, argv);
	}
#endif
#if 0
    char data_size[9];
	//num = ultohex(data_size, ret);
	
    write_nand_sd(0x8000000,"data",ret);
#endif		
	printf("/***********************************************/\r\n");
	printf("/*            Upgrade data successed!      */\r\n");
	printf("/***********************************************/\r\n");
	return;	
}

/*****************************************
 * Routine: ubi_system_update
 * Description: system update auto.
 *       
 *****************************************/
void ubi_system_update()
{	
	unsigned ret;
	unsigned num;
	/* init the sd card */

	/* read the kernel from the card*/
	ret = read_image(UBISYSTEM_NAME, 0x7000000);
	if (-1 == ret){
		printf("Read %s error! \r\n", ROOTFS_NAME);
		return;
	}
	
	if (0 == image_in_sd)
		return;

	is_upgradeimage = 1;

	nand_erase_image("system");
#if 1
	{
		int argc = 3;
		char *argv[] = {"ubi", "part", "system"};
		do_ubi(NULL, 0, argc, argv);
	}

	{
		int argc = 4;
		char *argv[] = {"ubi", "create", "system", "0xba00000"};
		do_ubi(NULL, 0, argc, argv);
	}
	
	{	
		int argc = 5;
		char data_size[9];
		num = ultohex(data_size, ret);
		printf("ret = %d, data_size = %s\r\n", ret, &data_size[7 - num]);

		char *argv[] = {"ubi", "write", "0x7000000", "system", (&data_size[7 - num])};
		do_ubi(NULL, 0, argc, argv);
	}
#endif
#if 0
    char data_size[9];
	num = ultohex(data_size, ret);
    write_nand_sd(0x7000000,"system",ret);
#endif		
	printf("/***********************************************/\r\n");
	printf("/*            Upgrade system successed!    */\r\n");
	printf("/***********************************************/\r\n");
	return;	
}

void ubi_usrdata_update()
{	
	unsigned ret;
	unsigned num;
	/* init the sd card */
	ret = read_image(USRDATA_NAME, 0x7000000);

	if (0 != image_in_sd)
	{
	    printf("/***********************************************/\r\n");
	    printf("/*      you want to save userdata in flash!    */\r\n");
	    printf("/***********************************************/\r\n");
		return;
	}
		
		
	nand_erase_image("usrdata");
#if 1	
	{
		int argc = 3;
		char *argv[] = {"ubi", "part", "usrdata"};
		do_ubi(NULL, 0, argc, argv);
	}
	{
		int argc = 4;
		char *argv[] = {"ubi", "create", "usrdata", "0xD0000000"};
		do_ubi(NULL, 0, argc, argv);
	}	
#endif

#if 0
	nand_format_usrdata_sd("usrdata");
#endif	
	printf("/***********************************************/\r\n");
	printf("/*            Upgrade usrdata successed!    */\r\n");
	printf("/***********************************************/\r\n");
	return;	
}


/*****************************************
 * Routine: Image_update
 * Description: Image update auto.
 *       include:uImage,ramdisk,ubifs
 *****************************************/
void Image_update()
{
#if 1
	int ret;
	SD_SETBIT(0x0000000, 0x404, 0x1c000000);
	ret = mmc_init_upgrade();
	if (ret){
		return;
	}
	kernel_update();
	rootfs_update();
	ubi_data_update();
	ubi_system_update();
	ubi_usrdata_update();	
    
	arm2_update();
	logo_update();
	extern char * env_name_spec;
	
	printf ("\Image_update ndo_saveenv: Saving Environment to %s...\n", env_name_spec);
	saveenv();


	if (1 == is_upgradeimage)
	{
	    printf("/**************************************************************************/\r\n");
	    printf("/*    Upgrade image successed! Please pull out sd card and reset board    */\r\n");
	    printf("/**************************************************************************/\r\n");
		while(1);
	}
#endif
}
