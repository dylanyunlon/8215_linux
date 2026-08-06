/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Integrator AP board.
 *.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARCH_AC83XX 1
#define CONFIG_AC83XX 1
#define CONFIG_MACH_AC8317FPGA

#define CONFIG_OF_LIBFDT 1
#define CONFIG_FIT 1
//#define CONFIG_OF_BOARD_SETUP 1
#define CONFIG_SYS_BOOTMAPSZ (200 << 20)

#include <chip_ver.h>
#include <asm/arch/ac83xx_basic.h>       /* get chip and board defs */
//#define CONFIG_CM1176JZ_S 1 /* CPU core is ARM1176JZ-S */

#define CONFIG_FAST_BOOT 0
#define CONFIG_SYS_NO_FLASH                    1
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x10000000
#define CONFIG_SYS_HZ		   1000		// ticks every 1ms
//#define CFG_HZ_CLOCK		101250000	/* Timer 1 is clocked at 101.24Mhz */
#define CFG_HZ_CLOCK		27000000	/* Timer 1 is clocked at 101.24Mhz */
#define CFG_CLOCK_PER_TICKS	(CFG_HZ_CLOCK / CONFIG_SYS_HZ)
#define CFG_HZ_PER_USEC	    (CFG_HZ_CLOCK / 1000000)

//#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs  */
//#define CONFIG_INITRD_TAG	1
//#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r during start up */


#undef CONFIG_INIT_CRITICAL
#define CONFIG_CM_INIT		1
#define CONFIG_CM_REMAP		1
#undef CONFIG_CM_SPD_DETECT
#define CONFIG_USE_IRQ		0

/*
 * ac83xx high level configuration options
 */
//#define CONFIG_BIM_TWO_WAY_WRITE

/*-----------------------------------------------------------------------
 * Serial Configuration
 */
#define CFG_AC83XX_SERIAL
#define CONFIG_CONS_INDEX	0
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * MMC Configuration
 */
#define CONFIG_MMC     			(1)

#ifdef CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

#define CONFIG_MTK_SD
// OR
//#define CONFIG_ATC_MSDC

#define MSDC_SLOT0				(0)
#define MSDC_SLOT1				(1)
#define MSDC_SLOT2				(2)
#define CONFIG_BOOT_SD_SLOT		(MSDC_SLOT0)

#endif
/************************************************************************
 *USB Fastboot configuration
 *
************************************************************************/
#define   CONFIG_MTK_USBFASTBOOT
#define   CONFIG_CMD_FASTBOOT

/*-----------------------------------------------------------------------
 * FILE SYSTEMS WHICH HAS BEEN SUPPORTED TO FLASH INTO NAND
 * they are mutual exclusion
 */
#define SUPPORT_UBI		0
#define SUPPORT_VFAT	0
#define SUPPORT_YAFFS2	0
#define SUPPORT_EXT3	0
#define SUPPORT_EXT4    1

/*-----------------------------------------------------------------------
 * Commands
 * refer to config_cmd_all.h nad config_cmd_default.h
 */
#define CONFIG_AC83XX_MTDPARTS
#define CONFIG_CMD_AC83XX_BOOT
#define CONFIG_ANDROID_RECOVERY
#define CONFIG_CMD_UPG
#define CONFIG_CMD_BDI		/* bdinfo			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MTDPARTS	/* mtd parts support		*/
#define CONFIG_CMD_MTDPARTS_SPREAD
#define CONFIG_CMD_MTDPARTS_SHOW_NET_SIZES

#define CONFIG_CMD_NAND		/* NAND support			*/
//#define CONFIG_CMD_USB		/* USB Support			*/
#define CONFIG_CMD_NET             /*  NET support      */
#define CONFIG_CMD_PING		/* ping support			*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/


#ifdef CONFIG_MMC_BOOT
#define CONFIG_ENV_IS_IN_SD  
#else
#define CONFIG_ENV_IS_IN_NAND
#endif

#define CONFIG_ENV_SD_OFFSET   (8192)
#define CONFIG_ENV_PAGE_NUM    (16)
///#define CONFIG_AC83XX_VFD
#define MTDIDS_DEFAULT	"nand0=nand0"
//#define MTDPARTS_DEFAULT	"mtdparts=nand0:0x400000@0x0(boot),0x200000@0x400000(env),0x200000@0x600000(kernel),0x200000@0x800000(rootfs),200M@0xa00000(system),300M@0xD200000(data),-(usrdata)"
#define MTDPARTS_DEFAULT	"mtdparts=nand0:0x400000@0x0(boot),0x200000@0x400000(env),0x200000@0x600000(arm2),0x400000@0x800000(logo),0x400000@0xc00000(kernel),0x200000@0x1000000(rootfs),300M@0x1200000(system),300M@0x13E00000(data),100M@0x26A00000(cache),300M@0x2CE00000(swap),0x400000@0x3FA00000(recovery),0x200000@0x3FE00000(misc),0x3200000@0x40000000(backup),0x400000@0x43200000(metazone),0x400000@0x43600000(dvp),300M@0x43A00000(data4write),-(usrdata)"
#define MTD_ACTIVE_PART		"nand0,2"

#define CONFIG_BOOTDELAY	0
//#define CONFIG_BOOTARGS		"root=/dev/mtdblock0 mem=32M console=ttyAM0 console=tty"
//#define CONFIG_BOOTARGS		"root=/dev/ram0 rw initrd=0x02800000,16M console=ttyMT0 mem=384M"
//#define CONFIG_BOOTARGS		"noinitrd root=/dev/mtdblock7 ro "
//#define CONFIG_BOOTARGS		"initrd=0x07000000,0x210000 root=/dev/ram rw console=ttyMT0,115200 mem=199M init=/init mtdparts=nand0:0x400000@0x0(boot),0x200000@0x400000(env),0x200000@0x600000(arm2),0x200000@0x800000(logo),0x200000@0xc00000(kernel),0x200000@0xe00000(rootfs),200M@0x1000000(system),300M@0xD800000(data),-(usrdata) ubi.mtd=system ubi.mtd=data ubi.mtd=usrdata"
//#define CONFIG_BOOTARGS		"initrd=0x07000000,0x210000 root=/dev/ram rw console=ttyMT0,115200 mem=455M init=/init mtdparts=nand0:0x400000@0x0(boot),0x200000@0x400000(env),0x200000@0x600000(arm2),0x200000@0x800000(logo),0x400000@0xa00000(kernel),0x200000@0xe00000(rootfs),200M@0x1000000(system),300M@0xD800000(data),-(usrdata) ubi.mtd=system ubi.mtd=data ubi.mtd=usrdata"

#define PART_MOUNT		"1"
#define PART_NO_MOUNT	"0"
#define MMCPARTS_DEFAULT	"parts=0x400000@0x0(boot)0,0x200000@0x400000(env)0,0x200000@0x600000(arm2)0,0x400000@0x800000(logo)0,0x400000@0xc00000(kernel)0,0x200000@0x1000000(rootfs)0,300M@0x1200000(system)1,300M@0x13E00000(data)1,100M@0x26A00000(cache)1,300M@0x2CE00000(swap)0,0x400000@0x3FA00000(recovery)0,0x200000@0x3FE00000(misc)0,0x3200000@0x40000000(backup)0,0x400000@0x43200000(metazone)0,0x400000@0x43600000(dvp)0,300M@0x43a00000(data4write)1,-(usrdata)1"

#define CONFIG_BOOTARGS		"initrd=0x07000000,0x210000 root=/dev/ram console=ttyMT0,115200 init=/init "MMCPARTS_DEFAULT""

#ifdef CONFIG_CMD_AC83XX_BOOT
#define CONFIG_BOOTCOMMAND	"nand read 0x6000000 kernel\;nand read 0x7000000 rootfs\;bootm 0x6000000"
#define CONFIG_RECOVERY_BOOTCMD "nand read 0x6000000 kernel\;nand read 0x7000000 recovery\;bootm 0x6000000"
#else
#define CONFIG_BOOTCOMMAND	"cp.b 0x80500000 0x2800000 0x800000;bootm 0x80300000"
#endif

#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
#define CONFIG_MMC_BOOTCMD     "mmc read 0 0x6000000 0x6000 0x2000\;mmc read 0 0x7000000 0x8000 0x1000\;bootm 0x6000000"
#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
#define CONFIG_MMC_BOOTCMD     "mmc read 2 0x6000000 0x6000 0x2000\;mmc read 2 0x7000000 0x8000 0x1000\;bootm 0x6000000"
#endif

#define PRELOADER_SIZE   (0x7000)

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check keypress when bootdelay = 0 */



/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP	/* undef to save memory	    */
#define CONFIG_SYS_PROMPT	"ac83xx # "	/* Monitor Command Prompt   */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#define CONFIG_SYS_LOAD_ADDR	0x7fc0	/* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

#define CFG_PAGETABLE_ADDRESS   0x1000000

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */


#define CONFIG_NR_DRAM_BANKS	1	/* we have 1 bank of DRAM */

#define PHYS_SDRAM_1		0x00000000	/* SDRAM Bank #1 */
//#define PHYS_SDRAM_1_SIZE	(128*1024*1024)
//#define CONFIG_SETUP_MEMORY_TAGS 1

#define CFG_FLASH_BASE		0x28000000



//#define CONFIG_ENV_IS_IN_NAND	1	/* use NAND for environment vars	*/


#define CFG_NAND_U_BOOT_OFFS	(36 << 10)	/* Offset to RAM U-Boot image	*/
#define CFG_NAND_U_BOOT_SIZE	(512 << 10)	/* Size of RAM U-Boot image	*/

/*
 * Now the NAND chip has to be defined (no autodetection used!)
 */
#define CFG_NAND_PAGE_SIZE	(8192)		/* NAND chip page size		*/
#define CFG_NAND_BLOCK_SIZE	(0x200000)	/* NAND chip block size		*/
#define CFG_NAND_PAGE_COUNT	(256)		/* NAND chip page count		*/
#define CFG_NAND_BAD_BLOCK_POS	(1)		/* Location of bad block marker	*/
#undef CFG_NAND_4_ADDR_CYCLE			/* No fourth addr used (<=32MB)	*/
#define CFG_NAND_YAFFS1_NEW_OOB_LAYOUT
#define CFG_NAND_YAFFS_WRITE

#define CONFIG_CMD_UBI
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_RBTREE

#ifdef CONFIG_ENV_IS_IN_NAND
/*
 * For NAND booting the environment is embedded in the U-Boot image. Please take
 * look at the file board/amcc/sequoia/u-boot-nand.lds for details.
 */
#define CONFIG_ENV_SIZE		CFG_NAND_PAGE_SIZE
#define CONFIG_ENV_OFFSET		(0x400000)
#define CFG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#endif

#define CONFIG_MTD_NAND_AC83XX


#ifdef CONFIG_ENV_IS_IN_SD

#define CONFIG_ENV_SIZE   (CONFIG_ENV_PAGE_NUM*512)
#define CONFIG_ENV_OFFSET               (0x400000)
#define CFG_ENV_OFFSET_REDUND   (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#endif

//#define PHYS_FLASH_SIZE		0x01000000	/* 16MB */
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define PHYS_FLASH_1		(CFG_FLASH_BASE)

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ)	/* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ)	/* Timeout for Flash Write */
#define CONFIG_SYS_MAX_FLASH_SECT	    256

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN  (0x0800000) /* 8M */
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*-----------------------------------------------------------------------
 * Network setting
 */
#define CONFIG_ENV_OVERWRITE
/*-----------------------------------------------------------------------
 * There are various dependencies on the core module (CM) fitted
 * Users should refer to their CM user guide
 * - when porting adjust u-boot/Makefile accordingly
 *   to define the necessary CONFIG_ s for the CM involved
 * see e.g. integratorcp_CM926EJ-S_config
 */
#include "armcoremodule.h"


#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

/* USB */
#define CONFIG_USB_STORAGE
#define CONFIG_USB_MTKHCD
#define LITTLEENDIAN            1       /* used by usb.h  */


/*
 *  Board NAND Info.
 */
 
#define CONFIG_SYS_NAND_BASE 0x7001E400
#define CFG_NAND_ADDR 0x04000000  /* physical address to access nand at CS0*/

#define CONFIG_SYS_MAX_NAND_DEVICE 1	/* Max number of NAND devices */
#define CONFIG_SYS_NAND_MAX_CHIPS 2

#define LARGE_NAND_PAGESIZE
#ifdef LARGE_NAND_PAGESIZE
    #define SECTORSIZE          2048
    #define ADDR_ID             1
    #define ADDR_COLUMN         2
    #define ADDR_PAGE           2
    #define ADDR_COLUMN_PAGE    4
    #define SCANBLOCK_POS       62
    #define BADBLOCK_POS        0
    #define BLOCK_SCAN_FLAG     0x5A
#else
    #define SECTORSIZE          512
    #define ADDR_ID             1
    #define ADDR_COLUMN         1
    #define ADDR_PAGE           3  
    #define ADDR_COLUMN_PAGE    4  
    #define BLOCK_SCAN_POS      14
    #define BADBLOCK_POS        5/*15*/ 
    #define BLOCK_SCAN_FLAG     0x5A
#endif

#define NAND_ChipID_UNKNOWN 0x00
#define NAND_MAX_FLOORS     1
#define NAND_MAX_CHIPS      1

#define NAND_OP_NONE                0
#define NAND_OP_READID              1
#define NAND_OP_READSTATUS          2
#define NAND_OP_READPAGE            3
#define NAND_OP_WRITEPAGE           4
#define NAND_OP_READSPARE           5
#define NAND_OP_WRITESPARE          6

#define LARGE_NAND_BOOT


#define CONFIG_AC83XX_GPIO

#define CONFIG_DVP_RESERVED_MEM 1


#endif	/* __CONFIG_H */

