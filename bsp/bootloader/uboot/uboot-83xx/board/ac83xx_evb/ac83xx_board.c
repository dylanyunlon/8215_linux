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
#include <linux/mtd/atc_nand.h>

#include <chip_ver.h>
#include <BL_VERSION_83xx.h>
#include <display_uboot.h>
#include "backcar_cfg.h"

#include <mmc.h>
#include <partition.h>
#include <bootctrl.h>

#include <pinmux.h>
#include <ac83xx_pinmux_table.h>
#include <ac83xx_gpio_pinmux.h>
#include "configs/ac83xx_evb.h"

#include "reserved_memory.h"   //for send the reserved memory info from dts to arm2
#include <asm/arch/args_to_arm2.h>
#include "printf.h"
#include "metazone_inter.h"

extern UPG_STATUS_T r_upg_status;
ARGS_TO_UBOOT r_args_to_uboot;

extern unsigned long long g_miscPartitonAddr;
extern unsigned long long g_tzPartitonAddr;
extern unsigned long long g_tzPartitonSize;

extern unsigned long long g_dtbPartitonAddr;
extern unsigned long long g_dtbPartitonSize;

extern unsigned long long g_arm2PartitonAddr;
extern unsigned long long g_arm2PartitonSize;
extern unsigned long long g_logoPartitonAddr;
extern unsigned long long g_logoPartitonSize;
extern unsigned long long g_metazonePartitonAddr;
extern unsigned long long g_metazonePartitonSize;

extern unsigned int g_systemIndex;
extern unsigned int _sdagentflag;
extern unsigned int _logosize;
extern unsigned int _arm2size;
extern unsigned int _dtbsize;
extern unsigned int _bootmisc_size;

extern unsigned int g_dtbAddrOnSD;  //0x100000;
extern unsigned int g_logoAddrOnSD; //0x400000;
extern unsigned int g_arm2AddrOnSD; //0x800000;
extern unsigned int g_BootMiscAddrOnSD; //0xA00000?;

//Boot up img info struct
/*Header*/
typedef struct PartitionInfo_Header{
	char         szMagic[5];
	unsigned int PartitionNum;
	UINT64       u8MACStartAddr;
	UINT64       u8ImageStartAddr;
}PartitionInfo_Header;

typedef struct PartitionInfo{
	char szPN[48];   //partition name
	char szIN[48];   //image name
	UINT64 u8PSA;    //image start addr on SD
	UINT64 u8PS;     //image real size
}PartitionInfo;

struct PartitionInfo_Header *bootup_img_hdr;
unsigned int bootup_img_num;
struct PartitionInfo bootup_img_info[50];
unsigned int bootup_img_info_size_blk;


extern char g_bootcmd[300];
extern char g_bootrecovercmd[300];

extern unsigned int mmc_bootup_device;

DECLARE_GLOBAL_DATA_PTR;

extern BOOL ARM2Image_Read(void);
extern BOOL TrustZoneImage_Read(void);
extern int init_ti_chip(int ti_type);
extern UINT32 u4ARM2Start(void);
extern BOOL Logo_Read(void);

extern char *uitostr_hex(char *str,unsigned int u4);
/*
* get reserved memory block from dts and pass it to arm2/trustzone/dvp
* add by mtk68080 2015.12.25
*/


/*args to arm2 and its init flag*/
ARGS_TO_ARM2_P args_to_arm2;
unsigned int args_to_arm2_init_flag = 0;

unsigned long long g_tz_mem_addr;
unsigned long long g_fb_mem_addr;   //framebuffer reserve memory start addr
unsigned long long g_logo_mem_addr;
unsigned long long g_arm2_mem_addr;
unsigned long long g_logofb_mem_addr; 


/*
typedef  struct partition{

	char      szPartName[20];
	char      szType[20];
	unsigned int  u4PartitionStartAddr;
	unsigned long long  u8PartitionStartAddr;
	unsigned int   u4PartitionSize;
	char  szImageFileName[20];
	unsigned int u4OffsetData;
	unsigned int u4SizeImage;
	unsigned int u4OffsetNextImage;
	unsigned int u4SegmentSize;
	unsigned int u4RealDataSize;
}partition;


typedef  struct partitionread{

	char      szPartName[20];
	char      szType[20];
	unsigned int  u4PartitionStartAddr;
	unsigned long long  u8PartitionStartAddr;
	unsigned int   u4PartitionSize;
	char  szImageFileName[20];
	unsigned int u4OffsetData;
	unsigned int u4SizeImage;
	unsigned int u4OffsetNextImage;
	unsigned int u4SegmentSize;
	unsigned int u4RealDataSize;
	struct partitionread *nextpartition;
}partitionread;
*/

void __aeabi_unwind_cpp_pr0(void)
{

	printf("call __aeabi_unwind_cpp_pr0\r\n");
	return;
}
/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			"bne 1b":"=r" (loops):"0" (loops));
}

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
#define MEM_BUF_ARGS_TO_UBOOT		0xC0000400
#define AVM_OFFSET_LIMIT				0x0100000
int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	gd->bd->bi_arch_number = MACH_TYPE_AC83XX;
	gd->bd->bi_boot_params = 0x00000100;
	//gd->bd->bi_boot_params = 0x00200100;

	pinmux_init();

#if 0   //not used
#ifdef _NOT_USED_
	/* get args to uboot (from preloader) */
	if(((ARGS_TO_UBOOT*)MEM_BUF_ARGS_TO_UBOOT)->u4_head_sig == ARGS_TO_UBOOT_HEAD_SIG)
	{
		r_args_to_uboot = *((ARGS_TO_UBOOT*)MEM_BUF_ARGS_TO_UBOOT);
	}
#endif
#endif

	//	usbpllset();

	return 0;
}


#define CFG_BOOTARGS_ROOT "noinitrd root=/dev/mtdblock7 ro"
#define CFG_BOOGARGS_MTD  "mtdparts=ac83xx_nand:4M(boot_1),4M(boot_2),4M(part_info_1),4M(part_info_2),4M(kernel_1),4M(kernel_2),\
	128M(rootfs_1),128M(rootfs_2),4M(standby_1),4M(standby_2),4M(fe_bin_1),4M(fe_bin_2),4M(fe_test_data),\
4M(APDA),4M(CPS_manager),4M(key_block_1),4M(key_block_2),4M(fe_parameters),4M(fe_power_curve),4M(upg_status),4M(acfg),\
4M(misc_data),4M(fast_init_logo),4M(fast_init_parameters),4M(log),256M(BUDA),8M(browser),32M(cust_part_1)"
#define CFG_BOOTARGS_CONSOLE "console=ttyMT0 "
#define CFG_BOOTARGS_MEM	"mem=256M"
char _bootargs_buf[1024];
static unsigned int usb0_speed=0, usb1_speed=0, usb0_mode=0;


void print_args_to_uboot(void)
{
	if(r_args_to_uboot.u4_head_sig != ARGS_TO_UBOOT_HEAD_SIG)
	{
		puts("no r_args_to_uboot\n");
		return ;
	}

	printf("r_args_to_uboot:\n");
	printf("\thead sig\t: 0x%08x\n", r_args_to_uboot.u4_head_sig);
	printf("\tversion\t: %d\n", r_args_to_uboot.u4_version);
	printf("\tboot type\t: %d\n", r_args_to_uboot.u4_boot_type);
	printf("\tdram ch1\t: 0x%08x\n", r_args_to_uboot.u4_dram_size_ch1);
	printf("\tdram ch2\t: 0x%08x\n", r_args_to_uboot.u4_dram_size_ch2);
	printf("\tkern addr\t: 0x%08x\n", r_args_to_uboot.u4_kernel_addr);
	printf("\tinitrd addr\t: 0x%08x\n", r_args_to_uboot.u4_initrd_addr);
	printf("\tinitrd size\t: 0x%08x\n", r_args_to_uboot.u4_initrd_size);
}

#define SB_BOOT_TYPE_NORMAL		0
#define SB_BOOT_TYPE_RECOVERY	1
#define SB_BOOT_TYPE_UPG		2
#define SB_BOOT_TYPE_ERR		0xff
extern int atc_restore_nand_att(void);

void print_rsv(char *name, unsigned int start, unsigned int size)
{
	printf("%-20s  start: 0x%08x  size:  0x%08x \r\n", name, start, size);
}

#ifdef CONFIG_KERNEL_ZIMAGE

extern char *g_partitionstr;
extern char *g_mtdparts;
static void modify_loglevel(char *cmdline)
{
	char *valueops = NULL;
	char *endops = NULL;
	char *loglevel = '8';
#ifdef CONFIG_ATC_USER
	loglevel = '4';
#else
	loglevel = '8';
#endif
	if( NULL ==cmdline )
		return;
	char *loglevelops = strstr(cmdline, "loglevel=");
	if( loglevelops )
	{
		valueops = loglevelops + strlen("loglevel=");
		*valueops = loglevel;
	}
	return;
}
//we use zImage which not cat with dtb,so we need load dtb ourselves.
//it is ok with both Image and zImage,
static int fdt_set_cmdline_for_zImage()
{
	char *propvalue = NULL;
	char *propvaluetest;
	char *propvalue2 = NULL;
	char *timerpropvalue = NULL;
	char *mempropvalue = NULL;
	int   chosen_noffset;
	int   timer_noffset;
	int   mem_noffset;
	int   len;
	char tmp_buf[32] = {0};
#ifdef ATC_AB_PARTITION_SUPPORT
	slot_metadata_t slot_info[2];
	int ab_slot = 0;
	char suffix[20] = {0};
#endif

	propvaluetest = (char *)malloc(1024);
	memset(propvaluetest,0,1024);
	metazone_read(0x10050, &usb0_speed);
	metazone_read(0x10051, &usb1_speed);
	metazone_read(0x10052, &usb0_mode);

	chosen_noffset = fdt_path_offset ((void *)(FDT_LOAD_ADDR), "/chosen");
	propvalue = (char *)fdt_getprop ((void *)(FDT_LOAD_ADDR), chosen_noffset, "bootargs", &len);
	strcat(propvaluetest,propvalue);
	modify_loglevel(propvaluetest);

#ifdef ATC_AB_PARTITION_SUPPORT
	get_slotinfo_from_bcb(slot_info);
	//dump_slot_metadata(slot_info);
	const char* slot_suffix = get_suffix_slot(slot_info);
	if (slot_suffix != NULL) {
		printf("slot_suffix is %s\n", slot_suffix);
		sprintf(suffix, "slot_suffix=%s", slot_suffix);
		strcat((char *)propvaluetest, " ");
		strcat((char *)propvaluetest, suffix);
	}
#endif

#ifdef CONFIG_BOOT_MMC
	strcat((char *)propvaluetest, " ");
	if (g_systemIndex != 0) {
		sprintf(tmp_buf, "root=/dev/mmcblk0p%d", g_systemIndex);
		strcat((char *)propvaluetest, tmp_buf);
	}
	//emmc need "parts=..."  info
	strcat((char *)propvaluetest," ");
	propvaluetest = strcat((char *)propvaluetest, g_partitionstr);
#else
	strcat((char *)propvaluetest, " ");
	if (g_systemIndex != 0) {
		if (args_to_arm2->upgrade_mode != 2)  {
			sprintf(tmp_buf, "root=/dev/mtkd%d", g_systemIndex);
			strcat((char *)propvaluetest, tmp_buf);
		} else {
			printf("recovery mode no need to add ext root\n");
		}

	}
	//nand need "mtdparts=atcnand:..." info
	strcat((char *)propvaluetest," ");
	propvaluetest = strcat((char *)propvaluetest,g_mtdparts);
#endif

	if (_sdagentflag == 2) //force set usb0 to host mode for booting to special recovery upgrade mode
	{

		strcat((char *)propvaluetest, " usbo=high usbh=high otg=host");
	}
	else
	{
    /* set usb port0(otg) speed */
	strcat((char *)propvaluetest, " ");
	if(usb0_speed == 0x554f3131)
		strcat((char *)propvaluetest, "usbo=full");
	else if(usb0_speed == 0x554f3230)
		strcat((char *)propvaluetest, "usbo=high");
	else
		strcat((char *)propvaluetest, "usbo=null");

	/* set usb port1(host only) speed */
	strcat((char *)propvaluetest, " ");
	if(usb1_speed == 0x55483131)
		strcat((char *)propvaluetest, "usbh=full");
	else if(usb1_speed == 0x55483230)
		strcat((char *)propvaluetest, "usbh=high");
	else
		strcat((char *)propvaluetest, "usbh=null");

	/* set usb port0 mode */
	strcat((char *)propvaluetest, " ");
	if(usb0_mode == 0x55534244)
		strcat((char *)propvaluetest, "otg=dev");
	else if(usb0_mode == 0x55534248)
		strcat((char *)propvaluetest, "otg=host");
	else
		strcat((char *)propvaluetest, "otg=otg");
	}

	fdt_setprop_string((void *)(FDT_LOAD_ADDR),chosen_noffset,"bootargs",(char *)propvaluetest);

	free(propvaluetest);

}

// set cpu num, 2/4   default 4
static int fdt_set_cpu_core_num_for_zImage()
{
	int cpu_noffset;
	unsigned int cpu_core_num = 0;
	int ret;

	extern unsigned int num_cpu();
	cpu_core_num = num_cpu();
	printf("CPU CORE NUM: %d\r\n", cpu_core_num);
	if (cpu_core_num == 2)
	{
		cpu_noffset = fdt_path_offset ((void *)(FDT_LOAD_ADDR), "/cpus/cpu@2");
		ret = fdt_del_node((void *)(FDT_LOAD_ADDR), cpu_noffset);
		if (ret){
			printf("Remove dts /cpus/cpu@2 fail!\r\n");
		}
		cpu_noffset = fdt_path_offset ((void *)(FDT_LOAD_ADDR), "/cpus/cpu@3");
		ret = fdt_del_node((void *)(FDT_LOAD_ADDR), cpu_noffset);
		if (ret){
			printf("Remove dts /cpus/cpu@3 fail!\r\n");
		}
	}

	return 0;


}

// remove unused reserved memory node
static int fdt_mod_rsv_mem_node_for_zImage()
{
	/*dvp node is already remove from dts*/
#if 0
	int noffset;
	int ret = 0;

	noffset = fdt_path_offset ((void *)(FDT_LOAD_ADDR), "/reserved-memory/dvp");
	ret = fdt_del_node((void *)(FDT_LOAD_ADDR), noffset);
	if (ret)
	{
		printf("Remove dts /reserved-memory/dvp node failed!\r\n");
	}
	else
	{
		printf("Remove dts /reserved-memory/dvp node successfully!\r\n");
	}
	return ret;
#else
	return 0;
#endif


}

/*add by mtk68080 2016.01.05*/
//get reserved memory info by node name
/*
int fdt_get_reserve_memory_by_node_name(P_RESERVE_MEMORY_INFO_T p_rsv_mem)//char *node_name)
{
	//int rsv_mem_noffset;      //to store /reserved-memory info;
	int reserve_mem_noffset;  //to store /reserved-memory/node_name info
	int len;
	const void *nodep;
	const u32   *data_32;
	const u64   *data_64;
	char full_node_name[60]="/reserved-memory/";
	strcat(full_node_name, p_rsv_mem->name);

	reserve_mem_noffset = fdt_path_offset ((void *)(FDT_LOAD_ADDR), full_node_name);

	if (reserve_mem_noffset < 0)
		return 1;

	nodep = fdt_getprop((void *)(FDT_LOAD_ADDR), reserve_mem_noffset, "reg", &len);

	if (len == 0)
		return 1;

	if (len == 8)  //32bit * 2
	{
        	data_32 = nodep;
		p_rsv_mem->start_addr = fdt32_to_cpu(data_32[0]);
		p_rsv_mem->size = fdt32_to_cpu(data_32[1]);
		printf("%s len= %d data0= 0x%x%08x data1= 0x%x%08x\r\n", p_rsv_mem->name, len,
			          (u32)(p_rsv_mem->start_addr >> 32),
					  (u32)(p_rsv_mem->start_addr & 0xffffffff),
					  (u32)(p_rsv_mem->size >> 32),
					  (u32)(p_rsv_mem->size & 0xffffffff));
	}
	else if (len == 16) //64bit * 2
	{
        data_64 = nodep;
		p_rsv_mem->start_addr = fdt64_to_cpu(data_64[0]);
		p_rsv_mem->size = fdt64_to_cpu(data_64[1]);
		printf("%s len= %d data0= 0x%x%08x data1= 0x%x%08x\r\n", p_rsv_mem->name, len,
						  (u32)(p_rsv_mem->start_addr >> 32),
						  (u32)(p_rsv_mem->start_addr & 0xffffffff),
						  (u32)(p_rsv_mem->size >> 32),
						  (u32)(p_rsv_mem->size & 0xffffffff));
	}
	else
	{
		return 1;  //unknown format
	}
	return 0;


}
*/
void print_64(unsigned long long value)
{
	printf("\t0x%x%08x", (u32)(value >> 32), (u32)(value & 0xffffffff));
}

#endif


enum boot_type {normal, recovery};
static int fdt_set_cmdline_for_boot(enum boot_type type)
{
    char *propvalue = NULL;
	char *propvaluetest;
	int   chosen_noffset;
	int   len;

	propvaluetest = (char *)malloc(1024);
	memset(propvaluetest,0,1024);


	chosen_noffset = fdt_path_offset ((void *)(FDT_LOAD_ADDR), "/chosen");
	if (type == normal)
#ifdef CONFIG_BOOT_MMC
		propvalue = (char *)fdt_getprop ((void *)(FDT_LOAD_ADDR), chosen_noffset, "bootargs1", &len);
#else
		propvalue = (char *)fdt_getprop ((void *)(FDT_LOAD_ADDR), chosen_noffset, "bootargs3", &len);
#endif
	else if (type == recovery)
		propvalue = (char *)fdt_getprop ((void *)(FDT_LOAD_ADDR), chosen_noffset, "bootargs2", &len);

	strcat(propvaluetest,propvalue);

	fdt_setprop_string((void *)(FDT_LOAD_ADDR),chosen_noffset,"bootargs",(char *)propvaluetest);

	free(propvaluetest);
	return 0;

}

static int fdt_set_cmdline_for_boot_special_recovery(unsigned int image_addr, unsigned int mac_addr, unsigned int boot_device)
{
    char *propvalue = NULL;
	char *propvaluetest;
	char buf[100];
	int   chosen_noffset;
	int   len;

	propvaluetest = (char *)malloc(1024);
	memset(propvaluetest,0,1024);


	chosen_noffset = fdt_path_offset ((void *)(FDT_LOAD_ADDR), "/chosen");

	propvalue = (char *)fdt_getprop ((void *)(FDT_LOAD_ADDR), chosen_noffset, "bootargs", &len);


	strcat(propvaluetest,propvalue);
	memset(buf,0,100);
	sprintf(buf, " image_start_addr=0x%x mac_start_addr=0x%x boot_device=%d", image_addr, mac_addr, boot_device);
	strcat(propvaluetest,buf);


	fdt_setprop_string((void *)(FDT_LOAD_ADDR),chosen_noffset,"bootargs",(char *)propvaluetest);

	free(propvaluetest);
	return 0;

}

static struct bootloader_message  bcb;
extern int upg_rsd_check_ext_sdcard_available(int * pdev_num);
extern int upg_rsd_write_raw_image(char * filename, unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size);

#ifdef CONFIG_SECURITY_UPGRADE
unsigned int get_bootflag_from_bcb(){
	extern unsigned int DATAZONE_LOAD_ADDR_INT;
	struct bootloader_message * tmp = (struct bootloader_message *)DATAZONE_LOAD_ADDR_INT;
	return tmp->bootflag;
}
#endif

void get_slotinfo_from_bcb(slot_metadata_t *slot)
{
	extern unsigned int DATAZONE_LOAD_ADDR_INT;
	struct bootloader_message * tmp = (struct bootloader_message *)DATAZONE_LOAD_ADDR_INT;

	memcpy(slot, tmp->metadata.slot_info, sizeof(tmp->metadata.slot_info));
}

void setup_recovery_env(){
	//setenv("bootcmd", CONFIG_RECOVERY_BOOTCMD);
	setenv("bootcmd", g_bootrecovercmd);
	//setenv("bootcmd", CONFIG_RECOVERY_BOOTCMD);
	printf("setup_recovery_env hh: g_bootrecovercmd:%s\r\n",g_bootrecovercmd);
}

extern	int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);
static unsigned int boot_time_ms(void)
{
	volatile unsigned int time = 0;

	/***
	* Register F000814C, which was triggered by BootROM,
	* start with 0xFFFFFFFF, end with 0x00000000,
	* decrease with every 27M crystal oscillation.
	*/
	time = (0xFFFFFFFF - (*((volatile uint32_t*)(0xF000814C)))) / 27000;
	return time;
}

void do_upg_preloader(void)
{
    char matchfilename[256]= {0};
    char szShowString[60]={0};
    unsigned long long read_file_addr = BASE_ADDR;

    partitionread *ppartitionread,*p;

    ppartitionread = readpartitioninfofromflash();
    p = ppartitionread;

    if(upg_rsd_check_udisk_available() == 1)
	{
		printf("do_rsd_upgrade find USB device\n");
		while(p)
    	{
    		if (strcmp(p->szPartName, "preloader") == 0)
    		{
    			if (upg_rsd_read_raw_image("83XX_Preloader_realchip_sd.bin", "usb", 0, read_file_addr, p->u8PartitionSize) == 0)
    			{
    				upg_rsd_write_raw_image("83XX_Preloader_realchip_sd.bin", read_file_addr, p->u8PartitionStartAddr, p->u8PartitionSize);
                    break;
    			}
    		}
            p = p->nextpartition;
        }
	}
	else
	{
	    int dev_num = 0;
        if(upg_rsd_check_ext_sdcard_available(&dev_num) == 1)
		{
			if (dev_num != 0)
			{
			    while(p)
            	{
            		if (strcmp(p->szPartName, "preloader") == 0)
            		{
            			if (upg_rsd_read_raw_image("83XX_Preloader_realchip_sd.bin", "mmc", dev_num, read_file_addr, p->u8PartitionSize) == 0)
            			{
            				upg_rsd_write_raw_image("83XX_Preloader_realchip_sd.bin", read_file_addr, p->u8PartitionStartAddr, p->u8PartitionSize);
                            break;
            			}
            		}
                    p = p->nextpartition;
                }
			}
	    }
	}
    return;
}

void do_clear_bcb_recovery(void)
{

    int ret;
    // Use page-aligned buffer (NAND page size is typically 4KB)
    #define NAND_PAGE_SIZE 4096
    static unsigned char page_aligned_buffer[NAND_PAGE_SIZE] __attribute__((aligned(NAND_PAGE_SIZE)));
    struct bootloader_message *bcb = (struct bootloader_message *)page_aligned_buffer;
    printf("Starting BCB command and recovery clear...\n");
    // Clear page-aligned buffer
    memset(page_aligned_buffer, 0xFF, NAND_PAGE_SIZE);

    // Read BCB data from datazone partition using partition name
    printf("Reading BCB data from datazone partition...\n");
    char addr_str[16];
    sprintf(addr_str, "0x%lx", (unsigned long)page_aligned_buffer);
    char *argv_read[] = {"nand", "read", addr_str, "datazone", "0x1000"};
    ret = do_nand(NULL, 0, 6, argv_read);
    if(ret != 0){
        printf("Warning: Failed to read BCB info, using default BCB settings\n");
    } else {
        printf("Successfully read BCB info\n");
    }

    // Clear BCB command and recovery fields
    memset(bcb->command, 0, sizeof(bcb->command));
    memset(bcb->recovery, 0, sizeof(bcb->recovery));
#ifdef CONFIG_SECURITY_UPGRADE
    // If security upgrade is enabled, calculate and set checksum
    uint32_t chksum = calc_bcb_checksum(bcb);
    printf("New bcb_checksum: 0x%x\n", chksum);
    put_bcb_checksum(bcb, chksum);
#endif

    printf("Clearing BCB command and recovery fields\n");

    // Erase datazone partition
    printf("Erasing first 4KB of datazone partition...\n");
    char *argv_erase[] = {"nand", "erase", "datazone", "0x1000"};
    ret = do_nand(NULL, 0, 5, argv_erase);
    if (ret != 0) {
        printf("Error: Failed to erase datazone partition\n");
        return;
    }

    // Write modified BCB data to datazone partition
    printf("Writing modified BCB data...\n");
    char *argv_write[] = {"nand", "write", addr_str, "datazone", "0x1000"};
    ret = do_nand(NULL, 0, 6, argv_write);
    if (ret != 0) {
        printf("Error: Failed to write BCB data\n");
        return;
    }
    printf("BCB data written successfully, command and recovery fields cleared\n");
    return;
}



void check_recovery_mode(){
    // Check recovery mode
    int ret = -1;
    int partflag = -1;
    int b_recovery = 0;
    // Use page-aligned buffer for BCB data
    #define NAND_PAGE_SIZE 4096
    static unsigned char page_aligned_buffer[NAND_PAGE_SIZE] __attribute__((aligned(NAND_PAGE_SIZE)));
    struct bootloader_message *bcb = (struct bootloader_message *)page_aligned_buffer;

    printf("Checking recovery mode...\n");

    // Clear page-aligned buffer
    memset(page_aligned_buffer, 0, NAND_PAGE_SIZE);

    // Read BCB data from datazone partition
    char addr_str[16];
    sprintf(addr_str, "0x%lx", (unsigned long)page_aligned_buffer);
    char *argv_read[] = {"nand", "read", addr_str, "datazone", "0x1000"};
    ret = do_nand(NULL, 0, 5, argv_read);
    if(ret != 0){
        printf("Warning: Failed to read BCB info from datazone partition\n");
        // Continue with normal boot if BCB read fails
        setenv("bootcmd", g_bootcmd);
        printf("set bootcmd: g_bootcmd:%s\r\n", g_bootcmd);
        fdt_set_cmdline_for_boot(normal);
        return;
    }

    // Check if BCB command indicates recovery mode
    if(strncmp(bcb->command, "boot-recovery", 13) == 0) {
        printf("Recovery mode detected via BCB command: %s\n", bcb->command);
        // Set upgrade mode for ARM2
        SetUpgradeMode(2);
        printf("set upgrade flag for arm2 ");
        // Setup recovery environment
        setup_recovery_env();
        fdt_set_cmdline_for_boot(recovery);
    } else {
        // Normal boot mode
        setenv("bootcmd", g_bootcmd);
        printf("set bootcmd: g_bootcmd:%s\r\n", g_bootcmd);
        fdt_set_cmdline_for_boot(normal);
    }
}


static void parse_rsv_info(void)
{
    RSV_MEM_T *rsv;
    rsv = get_rsv_mem_by_name("arm2");
    if (NULL == rsv)
    {
       printf("arm2_rsv get failed ,Please check\r\n");
       return FALSE;
    }
    g_arm2_mem_addr = rsv->start_addr;
    args_to_arm2 = (void *)g_arm2_mem_addr;

   rsv = get_rsv_mem_by_name("animation");
   if (NULL == rsv)
   {
       printf("animation get failed ,Please check\r\n");
       return;
   }
   g_logofb_mem_addr = rsv->start_addr;
   g_logo_mem_addr = rsv->start_addr + rsv->size - MRF_LOAD_OFFSET;

   rsv = get_rsv_mem_by_name("framebuffer");
   if (NULL == rsv)
   {
       printf("framebuffer get failed ,Please check\r\n");
       return;
   }
   g_fb_mem_addr = rsv->start_addr;

}

int boot_to_special_recovery(void);

int misc_init_r (void)
{

	if (_sdagentflag == 2)
		boot_to_special_recovery();
	//UINT32 u4BootType;

	/* print dbg msg - args to uboot */
#if 0  //not used
	print_args_to_uboot();
#endif

#if 0
#ifndef CONFIG_BOOT_MMC
	atc_restore_nand_att();
#endif
#endif


/* the order must be DTB_READ->Logo_Read->ARM2Image_Read
  * for we need to adjust dtb/logo/arm2 location on SD card dynamicly
  * and need to get mem addr to load logo/arm2/trustzone from dts
  * add by mtk68080 2016.01.07*/
#ifdef CONFIG_KERNEL_ZIMAGE
	BOOL DTB_Read(void);
	//for zImage/Image without fdt at the end

	printf("[Uboot] dtb read start at [%d] ms\r\n", boot_time_ms());
	if (_sdagentflag == 0)
		DTB_Read();

//	dump_rsv_mem_info();
	parse_rsv_info();
//	get_reserve_mem_from_dts();  //get static reserved memory info and write it to prop location

	if (_sdagentflag == 0)
		fdt_mod_rsv_mem_node_for_zImage();

//	SetUpgradeMode(0);

	if (_sdagentflag == 0)
	{
#ifdef ENABLE_RECOVERY
		check_recovery_mode();
#endif
		// check recovery fun will choose the right bootargs to use.
		// modify dtb, to make sure parts info is same as xml

		//printf("[Uboot] Metazone_Read read start at [%d] ms\r\n", boot_time_ms());
		//if (!Metazone_Read())//to get usp value from metazone for bootargs
		//	printf("[Uboot] Metazone_init fail\r\n");
		metazone_init(1);
		fdt_set_cmdline_for_zImage();
		fdt_set_cpu_core_num_for_zImage(); //adjust core num in dts according to efuse
#ifdef ENABLE_RECOVERY
		do_clear_bcb_recovery();
#endif

	}

#endif

	if(_sdagentflag != 0)
	{
		printf("[Uboot] Logo read start at [%d] ms\r\n", boot_time_ms());
		Logo_Read();
		//printf("ubooot >>>>>\r\n");

		printf("[Uboot] ARM2Image read start at [%d] ms\r\n", boot_time_ms());
		ARM2Image_Read();
	}
#ifdef CONFIG_TRUSTZONE_SUPPORT
	//printf("[Uboot] TrustZoneImage_Read read start at [%d] ms\r\n", boot_time_ms());
	//if(_sdagentflag == 0)
	//	TrustZoneImage_Read();
#endif

	if(_sdagentflag != 0)
		u4ARM2Start();

	//Uboot_DisplayInit(0x5300000,800,480,0);


	return 0;	// ignore misc init for android by mtk40148

}

/**********************************************
 * Routine: ddr_size
 * Description: return 1, 512M
 return 0, 256M
 return -1, error
 **********************************************/
#define DDR_MEM_SIZE_TE0  0x8000000
#define DDR_MEM_SIZE_TE1  0x18000000

int ddr_size()
{
	unsigned long data;

	*((volatile unsigned long *)(DDR_MEM_SIZE_TE0)) = 0x5a5a5a5a;
	*((volatile unsigned long *)(DDR_MEM_SIZE_TE1)) = 0xa5a5a5a5;

	data = *((volatile unsigned long *)(DDR_MEM_SIZE_TE0));
	if (0x5a5a5a5a == data){
		return 1;
	}else if (0xa5a5a5a5 == data){
		return 0;
	}else{
		return -1;
	}
}
u32 get_ram_size_from_preloader()   //now we support < 4G ram size
{
	return (u32)((*(volatile unsigned long*)0xF0008104));
}
/**********************************************
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 **********************************************/
int dram_init (void)
{
	int ret;
	u32 btype;
	u32 ram_size = 0;
	btype = get_board_type();

	display_board_info(btype);

	/* get dram size data from preloader */
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	//gd->bd->bi_dram[0].size = 0xc700000;	// 199M
	//#ifdef CONFIG_DVP_RESERVED_MEM
	gd->bd->bi_dram[0].size = 0x10000000;//TOTAL_DRAM_SIZE  - PHYS_SDRAM_1 - RESV_MEM_IO_SIZE - 0x200000  ;  //0xb500000
	//printf("[wts]  dram init physcial sdram %x   size %x %x  \r\n",  PHYS_SDRAM_1,   RESV_MEM_IO_SIZE, gd->bd->bi_dram[0].size    );
	//printf("dram_init size 193 0x%08x\r\n", gd->bd->bi_dram[0].size);
	//#else
	//	gd->bd->bi_dram[0].size = 0xaf00000;  //0xbf00000
	//	printf("dram_init size 203 0x%08x\r\n", gd->bd->bi_dram[0].size);
	//#endif
	//ret = ddr_size();
	//if (1 == ret){
		//printf("DDR size: 512M \r\n");
	if (CONFIG_NR_DRAM_BANKS == 2) {
		gd->bd->bi_dram[1].start = 0x10000000;
		gd->bd->bi_dram[1].size = 0x10000000;	// 512M only use 512MB in uboot
	}
		//gd->bd->bi_dram[1].size = 0x30000000;	// total 1G
	//}else if(0 == ret){
		//printf("DDR size: 256M \r\n");
		//gd->bd->bi_dram[1].start = 0x10000000;
		//gd->bd->bi_dram[1].size = 0;	// 0
	//}

	ram_size = get_ram_size_from_preloader();
	if (ram_size)
		printf("DRAM SIZE: %dM\r\n", ram_size/1024/1024);
	else
	{
		//printf("wrong ram size from preloader,please make sure you use correct preloader!!!\r\n");
		//while(1);
		printf("no ram size arg from preloader, use default\r\n");
	}

	//copy slave core booting code

	//memcpy((void *)0x0, (void *)0xc0000000, 512);

	return 0;
}


#define ReadReg32(addr)			(*(volatile UINT32 *)(addr))
#define WriteReg32(addr,data)	((*(volatile UINT32 *)(addr))=(UINT32)(data))
//#define ARM_PHY_STARTADD ARM2_RESERVED_MEM_PA

extern char *uitostr_hex(char *str,unsigned int u4);

//#ifdef CONFIG_CMD_SDAGENT
//    unsigned int g_logoAddressOnSD = 0x400000;
//    unsigned int g_arm2AddressOnSD = 0x800000;
//#endif


BOOL DTB_Read(void)
{
    UINT32 *pMemAddr = (UINT32 *)FDT_LOAD_ADDR;

	char   szDTBPhysAddr[16];
	char   szDTBSize[16];
	char *szValAddr = (char *)malloc(17);
	char *szValSize = (char *)malloc(17);
	int ret = 0;
	memset(szValAddr,0,17);
	memset(szValSize,0,17);

	sprintf(szDTBPhysAddr, "0x%x", FDT_LOAD_ADDR);
	sprintf(szDTBSize, "0x%x", (u32)g_dtbPartitonSize);
#ifdef ATC_AB_PARTITION_SUPPORT
	slot_metadata_t slot_info[2];
#endif

	//printf("szDTBPhysAddr %s \r\n",szDTBPhysAddr);

#ifdef CONFIG_BOOT_MMC
	#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
		char *argv[6];
	    if(_sdagentflag == 0)
	    {
	        printf("DTB_Read: memory addr= %s , part addr= 0x%x , image size= 0x%x \r\n", szDTBPhysAddr, (unsigned int)g_dtbPartitonAddr, (unsigned int)g_dtbPartitonSize);
	        argv[0] = "mmc";
			argv[1] = "read";
			argv[2] = "0";
			argv[3] = szDTBPhysAddr;
			argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_dtbPartitonAddr/512));
			argv[5] = uitostr_hex(szValSize,(unsigned int)(g_dtbPartitonSize/512));

	    }
		else  //do upgrade
		{
			printf("Upgrade DTB_Read: memory addr = %s , g_logoAddressOnSD= 0x%x , logosize= 0x%x\r\n",szDTBPhysAddr, (unsigned int)g_dtbAddrOnSD, (unsigned int)_dtbsize);
		    argv[0] = "mmc";
			argv[1] = "read";
			if (mmc_bootup_device == 1)
				argv[2] = "1";
			else
				argv[2] = "2";
			argv[3] = szDTBPhysAddr;
			argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_dtbAddrOnSD/512));
			argv[5] = uitostr_hex(szValSize,(unsigned int)(_dtbsize/512));

		}
	#elif (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
		char *argv[6] = {"mmc","read","2","0x1FF01000","0x3000","0x1000"};
		argv[3] = szDTBPhysAddr;
		argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_dtbPartitonAddr/512));
		argv[5] = uitostr_hex(szValSize,(unsigned int)(g_dtbPartitonSize/512));
	#endif
#else
	char *argv[6] = {"nand", "read", "0x1FF01000", "dtb", "0xC0000",""};
#ifdef ATC_AB_PARTITION_SUPPORT
    get_slotinfo_from_bcb(slot_info);
	const char* slot_suffix = get_suffix_slot(slot_info);
	if (!strcmp(slot_suffix, BOOTCTRL_SUFFIX_A)) {
        argv[3] = "dtb_a";
	} else {
        argv[3] = "dtb_b";
	}
#endif
	if (_sdagentflag == 0)
	{
		printf("NandBoot DTB_Read: memory addr= %s , part addr= 0x%x , image size= 0x%x \r\n", szDTBPhysAddr, (unsigned int)g_dtbPartitonAddr, (unsigned int)g_dtbPartitonSize);
		argv[2] = szDTBPhysAddr;
		argv[4] = uitostr_hex(szValSize,(unsigned int)(g_dtbPartitonSize));
	}
	else  //nand upgrade
	{
		printf("NandUpgrade DTB_Read: memory addr = %s , g_logoAddressOnSD= 0x%x , logosize= 0x%x\r\n",szDTBPhysAddr, (unsigned int)g_dtbAddrOnSD, (unsigned int)_dtbsize);
	    argv[0] = "mmc";
		argv[1] = "read";
		if (mmc_bootup_device == 1)
			argv[2] = "1";
		else
			argv[2] = "2";
		//argv[3] = "0xC500000";
		argv[3] = szDTBPhysAddr;
		argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_dtbAddrOnSD/512));
		argv[5] = uitostr_hex(szValSize,(unsigned int)(_dtbsize/512));

	}
#endif

#ifdef CONFIG_BOOT_MMC
	ret = do_mmcops(NULL, 0, 6, argv);
#else
	if (_sdagentflag == 0)
		ret = do_nand(NULL, 0, 5, argv);
	else
		ret = do_mmcops(NULL, 0, 6, argv);
#endif

	if (0 != ret)
	{
		printf("ERROR: Unable to read DTB Image\r\n");
		return FALSE;
	}

	//printf("Read DTB Image Success\r\n");
	free(szValAddr);
	free(szValSize);


	return TRUE;

}

void SetUpgradeMode(UINT32 flag){
	args_to_arm2->upgrade_mode = flag;
	printf("args_to_arm2->upgrade_mode %08x = %d\n",&(args_to_arm2->upgrade_mode), args_to_arm2->upgrade_mode);
}


#define WAKEUP_KEY_REG (IO_BASE + 0x240D0)
#if CONFIG_COPY_UPGRADE_SUPPORT
UINT32 GetCopyUpgradeMode(){
	return (*((volatile unsigned long *)(WAKEUP_KEY_REG)) & 0x00000002);
}
#else
UINT32 GetCopyUpgradeMode(){
	return (0);
}
#endif

int get_mtz_upg_mode(void)
{
    u32 mode = 0;

    metazone_read(0x10066, &mode);
    printf("#### metazone_read 0x10066 value is %u ####\n", mode);
    if (mode == 0x11) {
        printf("check fastboot upgrade mode trigged!\n");
        metazone_write(0x10066, 0);
        metazone_flush(TRUE);
        return 1;
    } else if (mode == 0x12) {
        printf("check copy upgrade mode trigged!\n");
        metazone_write(0x10066, 0);
        metazone_flush(TRUE);
        return 2;
    } else {
        return 0;
    }
}

BOOL ARM2Image_Read(void)
{
    //UINT32 *pMemAddr = ARM_PHY_STARTADD + 0x1000;
	unsigned int *pagetable = (unsigned int*)CFG_PAGETABLE_ADDRESS;
	unsigned int start, end;
	u32 ram_size = 0;
	char   szArm2PhysAddr[16];
	//char szVal[17];
	char *szValAddr = (char *)malloc(17);
	char *szValSize = (char *)malloc(17);
	int ret = 0;
#ifdef ATC_AB_PARTITION_SUPPORT
	slot_metadata_t slot_info[2];
#endif

	memset(szValAddr,0,17);
	memset(szValSize,0,17);

    // sprintf(szArm2PhysAddr, "0x%x", ARM_PHY_STARTADD + 0x1000);
	sprintf(szArm2PhysAddr, "0x%x", (u32)g_arm2_mem_addr+ 0x40000);

	//printf("szArm2PhysAddr %s \r\n",szArm2PhysAddr);

#ifdef CONFIG_BOOT_MMC
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	//char *argv[6] = {"mmc","read","0","0x1FF01000","0x3000","0x1000"};
	char *argv[6];
/*
#ifndef	CONFIG_CMD_SDAGENT
	char *argv[6] = {"mmc","read","0","0x1FF01000",uitostr_hex(szValAddr,(unsigned int)(g_arm2PartitonAddr/512)),uitostr_hex(szValSize,(unsigned int)(g_arm2PartitonSize/512))};
#else
    char *argv[6] = {"mmc","read","2","0x1FF01000",uitostr_hex(szValAddr,(unsigned int)(g_arm2AddressOnSD/512)),"0x1000"};
#endif
*/

	//printf("ARM2Image_Read _sdagentflag =0x%x \r\n",_sdagentflag);
    if(_sdagentflag == 0)
    {
        printf("ARM2Image_Read: memory addr= %s , partition addr= 0x%x , image size= 0x%x \r\n", szArm2PhysAddr, (unsigned int)g_arm2PartitonAddr,(unsigned int)g_arm2PartitonSize);
        argv[0] = "mmc";
		argv[1] = "read";
		argv[2] = "0";
		argv[3] = "0x1FF01000";
		argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_arm2PartitonAddr/512));
		argv[5] = uitostr_hex(szValSize,(unsigned int)(g_arm2PartitonSize/512));

    }
	else
	{
	    {
	        g_arm2AddrOnSD = g_logoAddrOnSD + (unsigned int)_logosize;
	    }
		printf("ARM2Image_Read: memory addr= %s , g_arm2AddressOnSD= 0x%x , arm2size= 0x%x\r\n",szArm2PhysAddr, (unsigned int)g_arm2AddrOnSD, (unsigned int)_arm2size);
	    argv[0] = "mmc";
		argv[1] = "read";
		if (mmc_bootup_device == 1)
			argv[2] = "1";
		else
			argv[2] = "2";
		argv[3] = "0x1FF01000";
		argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_arm2AddrOnSD/512));
		argv[5] = uitostr_hex(szValSize,(unsigned int)(_arm2size/512));

		//printf("ARM2Image_Read argv[4]=%s argv[5]=%s\r\n",argv[4],argv[5]);
	}
#elif (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
	char *argv[6] = {"mmc","read","2","0x1FF01000","0x3000","0x1000"};
#endif
	argv[3] = szArm2PhysAddr;
#else
	char *argv[6] = {"nand", "read", "0x1FF01000", "arm2", "0xC0000",""};	//TODO need to change size from 0xc0000 to 0x400000
#ifdef ATC_AB_PARTITION_SUPPORT
	get_slotinfo_from_bcb(slot_info);
	const char* slot_suffix = get_suffix_slot(slot_info);
	if (!strcmp(slot_suffix, BOOTCTRL_SUFFIX_A)) {
        	argv[3] = "arm2_a";
	} else {
        	argv[3] = "arm2_b";
	}
#endif
	if (_sdagentflag == 0)
	{
		printf("ARM2Image_Read: memory addr= %s , partition addr= 0x%x , image size= 0x%x \r\n", szArm2PhysAddr, (unsigned int)g_arm2PartitonAddr,(unsigned int)g_arm2PartitonSize);
		argv[2] = szArm2PhysAddr;
		argv[4] = uitostr_hex(szValSize, (unsigned int)(g_arm2PartitonSize));
	}
	else
	{
	    {
	        g_arm2AddrOnSD = g_logoAddrOnSD + (unsigned int)_logosize;
	    }
	    printf("ARM2Image_Read: memory addr= %s ,  g_arm2AddressOnSD= 0x%x , arm2size= 0x%x\r\n",szArm2PhysAddr, (unsigned int)g_arm2AddrOnSD, (unsigned int)_arm2size);
	    argv[0] = "mmc";
		argv[1] = "read";
		if (mmc_bootup_device == 1)
			argv[2] = "1";
		else
			argv[2] = "2";
		//argv[3] = "0x1FF01000";
		argv[3] = szArm2PhysAddr;
		argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_arm2AddrOnSD/512));
		argv[5] = uitostr_hex(szValSize,(unsigned int)(_arm2size/512));

		//printf("ARM2Image_Read argv[4]=%s argv[5]=%s\r\n",argv[4],argv[5]);

	}
#endif

	//printf("+ARM2Image_Read\r\n");
#ifdef CONFIG_BOOT_MMC
	ret = do_mmcops(NULL, 0, 6, argv);
#else
	if(_sdagentflag == 0)
		ret = do_nand(NULL, 0, 5, argv);  //4
	else
		ret = do_mmcops(NULL, 0, 6, argv);
#endif

	if (0 != ret)
	{
		printf("ERROR: Unable to read ARM2 Image\r\n");
		return FALSE;
	}
	args_to_arm2->jump_instr = 0xea00fffe;       //arm2 addr 0,  ldr pc, [pc #0x1000]
	ram_size = get_ram_size_from_preloader();
	if (ram_size)
	{
		printf("DRAM SIZE: %dM\r\n", ram_size/1024/1024);
		args_to_arm2->dram_size = ram_size;
	}
	else
	{
		//printf("wrong ram size from preloader,please make sure you use correct preloader!!!\r\n");
		//while(1);
		printf("no ram size arg from preloader, use default 1G for arm2\r\n");
		args_to_arm2->dram_size = 0x40000000;   //memroy size 1G
	}
    printf("Upgrade mode: %d \n", _sdagentflag);


	//if do copy upgrade, pass upgrade mode flag

	if (GetCopyUpgradeMode() != 0 || (_sdagentflag == 1))
	{
		SetUpgradeMode(1);
	}
	else
	{
		SetUpgradeMode(0);
	}

	flush_cache(args_to_arm2, sizeof(*args_to_arm2));

	/* disable pagetable for arm2 region */
	start = CFG_ARM2_RESERVED_ADDR;
	end = start + CFG_ARM2_RESERVED_SIZE;
	start >>= 20;
	end >>= 20;
	for (; start < end; start++) {
		pagetable[start] = 0;
	}
	/* clean TLB */
	start = 0;
	__asm__ volatile("mcr p15, 0, %0, c8, c6, 0" : : "r" (start));  // Invalid Data TLB
	__asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (start)); // DSB
	__asm__ volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (start));  // ISB

	//printf("BYTE %X \r\n",*pMemAddr);

    //printf("Read ARM2Image Image Success use dynamic partition szValAddr:%s,szValSize=%s\r\n",szValAddr,szValSize);
	//printf("Read ARM2 Image Success use dynamic partition\r\n");
	free(szValAddr);
	free(szValSize);
	return TRUE;

}

BOOL TrustZoneImage_Read(void)
{
	//UINT32 *pMemAddr = TRUSTZONE_RESERVED_MEM_PA;
	int trustzone_size = 0;
	char   szTZPhysAddr[16];
	char   szTZSize[16];
	//char szVal[17];
	char *szValAddr = (char *)malloc(17);
	char *szValSize = (char *)malloc(17);
	int ret = 0;
	RSV_MEM_T *trustzone_rsv = NULL;
#ifdef ATC_AB_PARTITION_SUPPORT
	slot_metadata_t slot_info[2];
#endif

	memset(szValAddr,0,17);
	memset(szValSize,0,17);
	trustzone_rsv = get_rsv_mem_by_name("trustzone");
	if (NULL == trustzone_rsv)
	{
		printf("trustzone_rsv get failed ,Please check\r\n");
		return FALSE;
	}
	g_tz_mem_addr = trustzone_rsv->start_addr;
	trustzone_size = trustzone_rsv->size;

	//sprintf(szTZPhysAddr, "0x%x", TRUSTZONE_RESERVED_MEM_PA);
	//sprintf(szTZSize, "0x%x", TRUSTZONE_RESERVED_MEM_SIZE);
	sprintf(szTZPhysAddr, "0x%x", (u32)g_tz_mem_addr);
	sprintf(szTZSize, "0x%x", (u32)(trustzone_size));

	//printf("szTZPhysAddr %s \r\n",szTZPhysAddr);

#ifdef CONFIG_BOOT_MMC
	#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
		char *argv[6];
		//printf("TrustZoneImage_Read _sdagentflag =0x%x \r\n",_sdagentflag);
	    if(_sdagentflag == 0)
	    {
	        printf("TrustZoneImage_Read: memory addr= %s , parttition addr= 0x%x , image size= 0x%x \r\n", szTZPhysAddr, (unsigned int)g_tzPartitonAddr,(unsigned int)g_tzPartitonSize);
	        argv[0] = "mmc";
			argv[1] = "read";
			argv[2] = "0";
			argv[3] = szTZPhysAddr;
			argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_tzPartitonAddr/512));
			argv[5] = uitostr_hex(szValSize,(unsigned int)(g_tzPartitonSize/512));

	    }
	#elif (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
		char *argv[6] = {"mmc","read","2","0x1FF01000","0x3000","0x1000"};
		argv[3] = szTZPhysAddr;
	#endif
	//argv[3] = szTZPhysAddr;
#else
	char *argv[6] = {"nand", "read", "0x1FF01000", "trustzone", "0xC0000",""};
#ifdef ATC_AB_PARTITION_SUPPORT
    get_slotinfo_from_bcb(slot_info);
	const char* slot_suffix = get_suffix_slot(slot_info);
	if (!strcmp(slot_suffix, BOOTCTRL_SUFFIX_A)) {
        argv[3] = "trustzone_a";
	} else {
        argv[3] = "trustzone_b";
	}
#endif
	if (_sdagentflag == 0)
	{
		argv[2] = szTZPhysAddr;
		if ( g_tzPartitonSize > trustzone_size)//TRUSTZONE_RESERVED_MEM_SIZE)
			printf("WARNING: trustzone partition size is bigger than reserved memory size\r\n");
		argv[4] = uitostr_hex(szValSize,(unsigned int)(g_tzPartitonSize));;
		printf("TrustZoneImage_Read: memory addr= %s , parttition addr= 0x%x , image size= 0x%x \r\n", szTZPhysAddr, (unsigned int)g_tzPartitonAddr,(unsigned int)g_tzPartitonSize);
		//char *argv[5] = {"nand", "read", "0x1FF01000", "arm2", "0xC0000"};  //TODO need to change size from 0xc0000 to 0x400000
	}
#endif

	//printf("+TrustZoneImage_Read\r\n");

#ifdef CONFIG_BOOT_MMC
	ret = do_mmcops(NULL, 0, 6, argv);
#else
	if(_sdagentflag == 0)
		ret = do_nand(NULL, 0, 5, argv);  //4
#endif

	if (0 != ret)
	{
		printf("ERROR: Unable to read TrustZone Image\r\n");
		return FALSE;
	}

	//printf("Read TrustZone Image Success\r\n");
	free(szValAddr);
	free(szValSize);
	return TRUE;

}

BOOL Logo_Read(void)
{
	char   szMrfPhysAddr[16];
	char *szValAddr = (char *)malloc(17);
	char *szValSize = (char *)malloc(17);
	int ret = 0;
	memset(szValAddr,0,17);
	memset(szValSize,0,17);


	//sprintf(szMrfPhysAddr, "0x%x", MRF_BUFFER_PA_ADDR);
	sprintf(szMrfPhysAddr, "0x%x", (u32)g_logo_mem_addr);

#ifdef CONFIG_BOOT_MMC
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	//char *argv[6] = {"mmc", "read","0", "0xC500000", "0x4000", "0x2000"};
/*
#ifndef	CONFIG_CMD_SDAGENT
	char *argv[6] = {"mmc", "read","0", "0xC500000", uitostr_hex(szValAddr,(unsigned int)(g_logoPartitonAddr/512)), uitostr_hex(szValSize,(unsigned int)(g_logoPartitonSize/512))};
#else
    char *argv[6] = {"mmc", "read","2", "0xC500000", uitostr_hex(szValAddr,g_logoAddressOnSD/512), "0x2000"};
#endif
*/
    char *argv[6];
    if(_sdagentflag == 0)
    {

	//printf("Logo_Read _sdagentflag111 =0x%x,g_logoPartitonAddr=0x%x,g_logoPartitonSize=0x%x \r\n",_sdagentflag,g_logoPartitonAddr,g_logoPartitonSize);
	    printf("Logo_Read: memory addr= %s , partition addr= 0x%x , image size= 0x%x \r\n",
	    		szMrfPhysAddr, (unsigned int)g_logoPartitonAddr ,(unsigned int)g_logoPartitonSize);
		argv[0] = "mmc";
		argv[1] = "read";
		argv[2] = "0";
		argv[3] = "0xC500000";
		argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_logoPartitonAddr/512));
		argv[5] = uitostr_hex(szValSize,(unsigned int)(g_logoPartitonSize/512));
    }
	else
	{
		{
			g_logoAddrOnSD = g_dtbAddrOnSD + (unsigned int)_dtbsize;
		}
		printf("Logo_Read: memory addr = %s , g_logoAddressOnSD= 0x%x , logosize= 0x%x\r\n",szMrfPhysAddr, (unsigned int)g_logoAddrOnSD, (unsigned int)_logosize);
	    argv[0] = "mmc";
		argv[1] = "read";
		if (mmc_bootup_device == 1)
			argv[2] = "1";
		else
			argv[2] = "2";
		argv[3] = "0xC500000";
		argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_logoAddrOnSD/512));
		argv[5] = uitostr_hex(szValSize,(unsigned int)(_logosize/512));
	}
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
	char *argv[6] = {"mmc", "read","2", "0xC500000", "0x4000", "0x2000"};
#endif
	argv[3] = szMrfPhysAddr;
#else
	char *argv[6] = {"nand", "read", "0xC500000", "logo", "0x800000", ""};
	if (_sdagentflag == 0)
	{
		printf("Logo_Read: memory addr= %s , partition addr= 0x%x , image size= 0x%x \r\n",
				szMrfPhysAddr, (unsigned int)g_logoPartitonAddr ,(unsigned int)g_logoPartitonSize);
		argv[2] = szMrfPhysAddr;
		argv[4] = uitostr_hex(szValSize, (unsigned int)(g_logoPartitonSize));
	}
	else
	{
		{
			g_logoAddrOnSD = g_dtbAddrOnSD + (unsigned int)_dtbsize;
		}

		printf("Logo_Read: memory addr = %s , g_logoAddressOnSD= 0x%x , logosize= 0x%x\r\n",szMrfPhysAddr, (unsigned int)g_logoAddrOnSD, (unsigned int)_logosize);
	    argv[0] = "mmc";
		argv[1] = "read";
		if (mmc_bootup_device == 1)
			argv[2] = "1";
		else
			argv[2] = "2";
		//argv[3] = "0xC500000";
		argv[3] = szMrfPhysAddr;
		argv[4] = uitostr_hex(szValAddr,(unsigned int)(g_logoAddrOnSD/512));
		argv[5] = uitostr_hex(szValSize,(unsigned int)(_logosize/512));
	}
#endif

#ifdef CONFIG_BOOT_MMC
	ret = do_mmcops(NULL, 0, 6, argv);
#else
	if (_sdagentflag == 0)
		ret = do_nand(NULL, 0, 5, argv);
	else
		ret = do_mmcops(NULL, 0, 6, argv);
#endif

	if (0 != ret)
	{
		printf("ERROR: Unable to read logo Image\r\n");
		return FALSE;
	}


	//printf("Read logo Image Success use dynamic partition szValAddr:0x%s,szValSize=0x%s\r\n",szValAddr,szValSize);
	free(szValAddr);
	free(szValSize);
	return TRUE;

}

//#define ARM_PHY_STARTADD (0x0FF00000)

UINT32 u4ARM2Start(void)
{
	UINT32 tmp;
#if trace_debug
	tmp = ReadReg32(0xF0038088);
        tmp |= 0x1;
        WriteReg32(0xF0038088,tmp);
	tmp = ReadReg32(0xF0000058);
        tmp |= 0x2000000;
        WriteReg32(0xF0000058,tmp);
#endif
	printf("Init ARM2!\r\n");
	WriteReg32(0xF004501C,0x00000001);
	//WriteReg32(0xF0045020,ARM2_RESERVED_MEM_PA);
	WriteReg32(0xF0045020,(u32)g_arm2_mem_addr);
	WriteReg32(0xF00381B8,0x00000003);

	return 0;
}

void dvp_uart_init()
{
    //default for dvp debug
    const BOOL fgUart4DvpDebug = TRUE;

    if (fgUart4DvpDebug)
    {
        GPIO_MultiFun_Set(PIN_143_URXD2,DVD_RS232_SEL);
        GPIO_MultiFun_Set(PIN_155_UTXD2,DVD_RS232_SEL);

    }
    else
    {
        GPIO_MultiFun_Set(PIN_143_URXD2,UART2_SEL);
        GPIO_MultiFun_Set(PIN_155_UTXD2,UART2_SEL);
    }
}

/*
partitionread * readpartitioninfofromflash()
{
    struct mmc *emmc_dev = NULL;
	int emmc_dev_num = 0;
	int n;
	ulong size;
	int err = 0;
	char  buf[512];
	partitionread *ppartread = NULL;
	partitionread *partitionhead = NULL;
	partition *ppartition = NULL;
	unsigned int u4PartionAddress = 0;
	ppartread = (partitionread *)malloc(sizeof(partitionread));
	partitionhead = ppartread;
	memset(ppartread,0,sizeof(partition));
    u4PartionAddress = 0x400000 - 512;

	emmc_dev = find_mmc_device(emmc_dev_num);
	if ( NULL == emmc_dev){
		printf("can't find emmc device\n");
		return -1;
	}
	err = mmc_init(emmc_dev);
	if(err){
		printf("init emmc device fail\n");
		return -1;
	}

    while(u4PartionAddress !=0)
    {
        n = emmc_dev->block_dev.block_read(emmc_dev_num, u4PartionAddress/512, 1, (char *)buf);
		if (!n)
		{
			printf("readpartitioninfofromflash block_read fail result=%d\r\n",n);
			return -1;
		}
        ppartition = (partition *)buf;
		ppartread->u8PartitionStartAddr = ppartition->u8PartitionStartAddr;

		u4PartionAddress = ppartition->u4OffsetNextImage;

        printf("readpartitioninfofromflash partition start address=0x%X\r\n",(unsigned int)(ppartread->u8PartitionStartAddr));
		if(u4PartionAddress !=0)
		{
		    ppartread->nextpartition =  (partitionread *)malloc(sizeof(partitionread));
	        memset(ppartread,0,sizeof(partitionread));
			ppartread->nextpartition->nextpartition = NULL;
			ppartread = ppartread->nextpartition;
		}

    }



	return partitionhead;
}

*/
typedef struct upgrade_header{
	char szSignature[4];
	unsigned int bFormatFlash;
	unsigned int bEraseemmc;
#ifndef CONFIG_BOOT_MMC
	unsigned int bVerifyImage;
#endif
	unsigned int bModifyPartition;
	unsigned int bAdvanceMode;
	unsigned int nSegmentSize;
	unsigned long long u8UserdataPartitionAddress;
	unsigned int nWriteproSize;
	unsigned int u4Reserver[512 - 28];
}upgrade_header;

extern int emmc_write_raw_image(struct mmc * mmc, int dev_num, partitioninfo *part);

int boot_to_special_recovery(void)
{
	struct mmc *mmc;

	int dev_num = 0,err,flashAddr = 0, flashSize = 0;
	unsigned int mac_addr = 0, image_addr = 0;
	int n = 0;
	char  buf[512];

	upgrade_header  *pheader = NULL;
	int bFormatFlash = 0;
	int bEraseemmc = 0;
#ifndef CONFIG_BOOT_MMC
	int bVerifyImage = 0;
#endif
	int bModifyPartition = 0;
	int bYaffs2fs = 1;
	partitioninfo *part = NULL;
	unsigned long long u8UserdataPartitionAddress;

	partitionhead parthead;
	partitionread *pCurpart= NULL,*pPrepart=NULL;
	int partcnt = 0;
	char szShowString[60]={0};

	int emmc_dev_num = 0;
	struct mmc *emmc_dev = NULL;
	int error;
	unsigned start_time, end_time;

	long spendtime = 0L;
	spendtime = get_timer(0);

	if (_sdagentflag != 2)
	{
		printf("ERR: Not Special Recovery Upgrade Mode\r\n");
		return -1;
	}




#ifdef NEW_PARTITION_DESIGN
	/* Check if ATC Upgrade Tool Match the minimum request*/
	extern int _aut_version;
	if (CONFIG_AUT_VERSION > _aut_version)
	{
		printf("ERR: AUT Tool Version[%x] is too old, Please at least use Version[%x]\r\n", _aut_version, CONFIG_AUT_VERSION);
		while(1);
	}
#endif



	dev_num = mmc_bootup_device;
	mmc = find_mmc_device(mmc_bootup_device);

	if(mmc == NULL)
	{
		printf("ERR: can't find mmc device[%d]\r\n", mmc_bootup_device);
		return -1;
	}

	err = mmc_init(mmc);
	if (err)
	{
		printf("ERR: mmc init fail\r\n");
		return -1;
	}

#ifdef CONFIG_BOOT_MMC
    emmc_dev = find_mmc_device(emmc_dev_num);
	if ( NULL == emmc_dev)
	{
		printf("ERR: can't find emmc device\n");
		return -1;
	}

	err = mmc_init(emmc_dev);
	if(err)
	{
		printf("ERR: init emmc device fail\n");
		return -1;
	}
#endif

	start_time = boot_time_ms();

	flashAddr = CONFIG_BOOTUP_IMG_INFO_ON_SD_ADDR/512;
	n = mmc->block_dev.block_read(dev_num,flashAddr,1,buf);
	flush_cache((ulong)buf,512);
	bootup_img_hdr = (PartitionInfo_Header *)buf;
	printf("########Bootup image header##########\r\n");
	printf("Header Magic String:      %s\r\n", bootup_img_hdr->szMagic);
	printf("Bootup Image Num:         %d\r\n", bootup_img_hdr->PartitionNum);
	printf("MAC Start Addr:           0x%x%08x\r\n", bootup_img_hdr->u8MACStartAddr);
	printf("Upgrade Image Start Addr: 0x%x%08x\r\n", bootup_img_hdr->u8ImageStartAddr);

	bootup_img_num = bootup_img_hdr->PartitionNum;
	mac_addr = bootup_img_hdr->u8MACStartAddr;
	image_addr = bootup_img_hdr->u8ImageStartAddr;


	//phrase bootup image info
	flashAddr = (CONFIG_BOOTUP_IMG_INFO_ON_SD_ADDR + 512)/512;
	bootup_img_info_size_blk = (bootup_img_num * sizeof(struct PartitionInfo) + 512) / 512;
	n = mmc->block_dev.block_read(dev_num,flashAddr, bootup_img_info_size_blk, bootup_img_info);

	//phrase upgrade header
	flashAddr = image_addr/512;
	n = mmc->block_dev.block_read(dev_num,flashAddr-1,1,buf);
	flush_cache((ulong)buf,512);
	pheader  = (upgrade_header *)buf;
	bFormatFlash = pheader->bFormatFlash;   //flag for format "usrdata" partition
	bModifyPartition = pheader->bModifyPartition;
	u8UserdataPartitionAddress = pheader->u8UserdataPartitionAddress;
	//printf("write-protect-size: 0x%x\r\n", pheader->nWriteproSize);
	bEraseemmc = pheader->bEraseemmc;   //flag for erase emmc or nand
#ifndef CONFIG_BOOT_MMC
	bVerifyImage = pheader->bVerifyImage;
#endif

	end_time = boot_time_ms();
	printf("[BOOT TIME] Phrase Bootup Image Info Takes [%d] ms\r\n", end_time - start_time);

	printf("########Upgrade header##########\r\n");
	printf("Header Magic String:      %s\r\n", pheader->szSignature);
	printf("Format NAND:              %d\r\n", pheader->bFormatFlash);
	printf("Format eMMC:              %d\r\n", pheader->bEraseemmc);
#ifndef CONFIG_BOOT_MMC
	printf("Verify Image:             %d\r\n", pheader->bVerifyImage);
#endif
	printf("Upgrade All Partition:    %d\r\n", pheader->bModifyPartition);
	printf("Use Advanced Mode:        %d\r\n", pheader->bAdvanceMode);
	printf("Segment Size:             %d\r\n", pheader->nSegmentSize);
	printf("Internal SD Start Addr:   0x%x%08x\r\n", pheader->u8UserdataPartitionAddress);
#ifdef NEW_PARTITION_DESIGN
	printf("Write Protect Size:       %d\r\n", pheader->nWriteproSize);
#endif

	if (bModifyPartition != 1)//select all part in tool
	{
		printf("ERR: Special Recovery need upgrade all image\r\n");
		return -1;
	}

#if 1//#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	start_time = boot_time_ms();
	if(bModifyPartition == 1)
	{//should done before erase
	}
	end_time = boot_time_ms();
	printf("[BOOT TIME] Clear eMMC Protect Setting Takes [%d] ms\r\n", end_time - start_time);
#endif

#if 0  //special recovery will do this now, to cover cancel upgrade request
	if (bEraseemmc !=0)
	{
		printf("========Satrt erase eMMC========\r\n");
		start_time = boot_time_ms();
		mmc_erase(emmc_dev,8,0,1);
		end_time = boot_time_ms();
		printf("[BOOT TIME] Erase eMMC Takes [%d] ms\r\n", end_time - start_time);
		printf("========Finish erase eMMC========\r\n");
	}
#endif



	for(n = 0; n < bootup_img_num; n++)
	{
		printf("########Start##########\r\n");
		printf("Partition Name:    %s\r\n", bootup_img_info[n].szPN);
		printf("Start Addr OnSD:   0x%x%08x\r\n", bootup_img_info[n].u8PSA);
		printf("Image Size:        0x%x%08x\r\n", bootup_img_info[n].u8PS);
		printf("########End############\r\n");
	}

	//dtb
	start_time = boot_time_ms();
	flashAddr = bootup_img_info[0].u8PSA/512;
	flashSize = bootup_img_info[0].u8PS/512;
	printf("partition[%s]\r\n", bootup_img_info[0].szPN);
	n = mmc->block_dev.block_read(dev_num, flashAddr, flashSize, (u32)FDT_LOAD_ADDR);
//	get_reserve_mem_from_dts();  //get static reserved memory info and write it to prop location
	fdt_mod_rsv_mem_node_for_zImage();
	fdt_set_cmdline_for_boot(recovery);
	fdt_set_cmdline_for_zImage();
	fdt_set_cpu_core_num_for_zImage(); //adjust core num in dts according to efuse
	end_time = boot_time_ms();
	printf("[BOOT TIME] Read and Phrase dtb Image Takes [%d] ms\r\n", end_time - start_time);

	//logo
	start_time = boot_time_ms();
	flashAddr = bootup_img_info[1].u8PSA/512;
	flashSize = bootup_img_info[1].u8PS/512;
	printf("partition[%s]\r\n", bootup_img_info[1].szPN);
	n = mmc->block_dev.block_read(dev_num, flashAddr, flashSize, (u32)g_logo_mem_addr);
	end_time = boot_time_ms();
	printf("[BOOT TIME] Read logo Image Takes [%d] ms\r\n", end_time - start_time);


	//arm2
	start_time = boot_time_ms();
	flashAddr = bootup_img_info[2].u8PSA/512;
	flashSize = bootup_img_info[2].u8PS/512;
	printf("partition[%s]\r\n", bootup_img_info[2].szPN);
	n = mmc->block_dev.block_read(dev_num, flashAddr, flashSize, (u32)((u32)g_arm2_mem_addr+ 0x40000));

	UINT32 *pMemAddr = (UINT32 *)((u32)g_arm2_mem_addr+ 0x40000);
	u32 ram_size;
	args_to_arm2->jump_instr = 0xea00fffe;    //arm2 addr 0,  ldr pc, [pc #0x40000]
	ram_size = get_ram_size_from_preloader();
	if (ram_size)
	{
		printf("DRAM SIZE: %dM\r\n", ram_size/1024/1024);
		args_to_arm2->dram_size = ram_size;
	}
	else
	{
		//printf("wrong ram size from preloader,please make sure you use correct preloader!!!\r\n");
		//while(1);
		printf("no ram size arg from preloader, use default 1G for arm2\r\n");
		args_to_arm2->dram_size = 0x40000000;   //memroy size 1G
	}

	SetUpgradeMode(2);
	u4ARM2Start();

	//while(1);
	end_time = boot_time_ms();
	printf("[BOOT TIME] Read and Start arm2 Image Takes [%d] ms\r\n", end_time - start_time);

	//kernel
	start_time = boot_time_ms();
	flashAddr = bootup_img_info[3].u8PSA/512;
	flashSize = bootup_img_info[3].u8PS/512;
	printf("partition[%s]\r\n", bootup_img_info[3].szPN);
	n = mmc->block_dev.block_read(dev_num, flashAddr, flashSize, 0x1008000);
	end_time = boot_time_ms();
	printf("[BOOT TIME] Read kernel Image Takes [%d] ms\r\n", end_time - start_time);
	//trustzone
	start_time = boot_time_ms();
	flashAddr = bootup_img_info[4].u8PSA/512;
	flashSize = bootup_img_info[4].u8PS/512;
	printf("partition[%s]\r\n", bootup_img_info[4].szPN);
	n = mmc->block_dev.block_read(dev_num, flashAddr, flashSize, (u32)g_tz_mem_addr);
	end_time = boot_time_ms();
	printf("[BOOT TIME] Read trustzone Image Takes [%d] ms\r\n", end_time - start_time);
	//recovery
	start_time = boot_time_ms();
	flashAddr = bootup_img_info[5].u8PSA/512;
	flashSize = bootup_img_info[5].u8PS/512;
	printf("partition[%s]\r\n", bootup_img_info[5].szPN);
	n = mmc->block_dev.block_read(dev_num, flashAddr, flashSize, 0xc800000);

	end_time = boot_time_ms();
	printf("[BOOT TIME] Read recovery Image Takes [%d] ms\r\n", end_time - start_time);


#ifdef CONFIG_BOOT_MMC
	flashAddr = image_addr/512;
	n = mmc->block_dev.block_read(dev_num, flashAddr, 1, buf);
	flush_cache((ulong)buf, flashAddr * 512);
	part = (partitioninfo *)buf;
	dumppartitioninfo(part);

	start_time = boot_time_ms();
	if(0 == strcmp(part->szType,"raw"))
	{
		printf("Begin to upgrade partition[%s]\r\n", part->szPartName);

		n = emmc_write_raw_image(mmc,dev_num,part);

		if (n == -1)
		{
			printf("ERR: upgrade partition[%s] fail\r\n", part->szPartName);
			return -1;
		}
	}
	printf("upgrade partition[%s] success\r\n", part->szPartName);

	end_time = boot_time_ms();
	printf("[BOOT TIME] Flash preloader Image Takes [%d] ms\r\n", end_time - start_time);
#endif

	spendtime = get_timer(spendtime);
	printf("\nspendtime: %ld\r\n", spendtime);

	//config bootargs for special recovery
	fdt_set_cmdline_for_boot_special_recovery(image_addr, mac_addr, mmc_bootup_device);
	// start trustzone
	printf("[BOOT TIME] Ready to Jump to Trustzone [%d] ms\r\n", boot_time_ms());
	run_command("bootz 0x10008000",0);

	return 0;

}

