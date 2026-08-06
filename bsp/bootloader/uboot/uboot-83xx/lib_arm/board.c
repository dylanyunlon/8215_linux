/*
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * To match the U-Boot user interface on ARM platforms to the U-Boot
 * standard (as on PPC platforms), some messages with debug character
 * are removed from the default U-Boot build.
 *
 * Define DEBUG here if you want additional info as shown below
 * printed upon startup:
 *
 * U-Boot code: 00F00000 -> 00F3C774  BSS: -> 00FC3274
 * IRQ Stack: 00ebff7c
 * FIQ Stack: 00ebef7c
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <mmc.h>
#include "fat.h"
#if CONFIG_EXTERNAL_MCU
#include <ext_mcu.h>
#endif

/*ac83xx*/
#include <asm/arch/ac83xx_part_tbl.h>
#include <asm/arch/ac83xx_upg_status.h>
#include <asm/arch/drv_vfd_83xx.h>
#include <asm/arch/ac83xx_pdwnc.h>
#include <asm/arch-ac83xx/x_pdwnc.h>

/*print on /off */
#include <asm/arch/x_bim.h>

#ifdef CONFIG_DRIVER_SMC91111
#include "../drivers/net/smc91111.h"
#endif
#ifdef CONFIG_DRIVER_LAN91C96
#include "../drivers/net/lan91c96.h"
#endif

#ifdef CONFIG_AC83XX_INPUT
#include <asm/arch/ac83xx_keyadc.h>
#endif

#ifdef CONFIG_AC83XX_TOUCH
#include <asm/arch/ac83xx_rtouchadc.h>
#endif

#ifdef CONFIG_AC83XX_TP
#include <asm/arch/ac83xx_ctouchadc.h>
#include <asm/arch/ac83xx_i2c.h>
#endif

#include "x_pdwnc.h"

#include "x_ver.h"
#include <partition.h>

#include <pinmux.h>
#include <ac83xx_pinmux_table.h>
#include <ac83xx_gpio_pinmux.h>
#include "configs/ac83xx_evb.h"
#include "environment.h"

#include "ac83xx_gpio.h"
#include "gpio.h"


DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;
char *g_mtdparts;
char *g_partitionstr;
unsigned long long g_miscPartitonAddr;
unsigned long long g_arm2PartitonAddr;
unsigned long long g_arm2PartitonSize;

unsigned long long g_tzPartitonAddr;
unsigned long long g_tzPartitonSize;

unsigned long long g_dtbPartitonAddr;
unsigned long long g_dtbPartitonSize;

unsigned long long g_bootmiscPartitonAddr;
unsigned long long g_bootmiscPartitonSize;

unsigned long long g_logoPartitonAddr;
unsigned long long g_logoPartitonSize;

unsigned long long g_metazonePartitonAddr;
unsigned long long g_metazonePartitonSize;

unsigned long long g_dvpPartitonAddr;
unsigned long long g_dvpPartitonSize;

unsigned int g_systemIndex = 0;
unsigned int mmc_bootup_device = 10; // default value is invalid value


char g_bootcmd[300] ={0};
char g_bootrecovercmd[300]={0};
unsigned long long g_envpartitionoffset;


extern unsigned int _sdagentflag;
/* printf on/off */
unsigned long disable_print=0;

#ifdef CONFIG_HAS_DATAFLASH
extern int  AT91F_DataflashInit(void);
extern void dataflash_print_info(void);
#endif

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

UPG_STATUS_T r_upg_status;

const char version_string[64] =
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING;

#ifdef CONFIG_DRIVER_CS8900
extern void cs8900_get_enetaddr (void);
#endif

#ifdef CONFIG_DRIVER_RTL8019
extern void rtl8019_get_enetaddr (uchar * addr);
#endif

extern void FastBoot(void);
extern int do_rsd_upgrade(void);

#if defined(CONFIG_HARD_I2C) || \
    defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static ulong mem_malloc_start = 0;
static ulong mem_malloc_end = 0;
static ulong mem_malloc_brk = 0;

static
void mem_malloc_init (ulong dest_addr)
{
	mem_malloc_start = dest_addr;
	mem_malloc_end = dest_addr + CONFIG_SYS_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

//	memset ((void *) mem_malloc_start, 0,
//			mem_malloc_end - mem_malloc_start);
	memset((void *)mem_malloc_start, 0, CONFIG_SYS_MALLOC_LEN);
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;

	return ((void *) old);
}


/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
void inline __coloured_LED_init (void) {}
extern void inline __coloured_LED_init (void) __attribute__((used));
void coloured_LED_init (void) __attribute__((weak, alias("__coloured_LED_init")));
void inline __red_LED_on (void) {}
extern void inline __red_LED_on (void) __attribute__((used));
void red_LED_on (void) __attribute__((weak, alias("__red_LED_on")));
void inline __red_LED_off(void) {}
extern void inline __red_LED_off (void) __attribute__((used));
void red_LED_off(void)	     __attribute__((weak, alias("__red_LED_off")));
void inline __green_LED_on(void) {}
extern void inline __green_LED_on (void) __attribute__((used));
void green_LED_on(void) __attribute__((weak, alias("__green_LED_on")));
void inline __green_LED_off(void) {}
extern void inline __green_LED_off (void) __attribute__((used));
void green_LED_off(void)__attribute__((weak, alias("__green_LED_off")));
void inline __yellow_LED_on(void) {}
extern void inline __yellow_LED_on (void) __attribute__((used));
void yellow_LED_on(void)__attribute__((weak, alias("__yellow_LED_on")));
void inline __yellow_LED_off(void) {}
extern void inline __yellow_LED_off (void) __attribute__((used));
void yellow_LED_off(void)__attribute__((weak, alias("__yellow_LED_off")));
void inline __blue_LED_on(void) {}
extern void inline __blue_LED_on (void) __attribute__((used));
void blue_LED_on(void)__attribute__((weak, alias("__blue_LED_on")));
void inline __blue_LED_off(void) {}
extern void inline __blue_LED_off (void) __attribute__((used));
void blue_LED_off(void)__attribute__((weak, alias("__blue_LED_off")));

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

#if defined(CONFIG_ARM_DCC) && !defined(CONFIG_BAUDRATE)
#define CONFIG_BAUDRATE 115200
#endif
static int init_baudrate (void)
{
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_r ("baudrate", tmp, sizeof (tmp));
	gd->bd->bi_baudrate = gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;

	return (0);
}

static int display_banner (void)
{
	printf ("\n\n%s\n\n", version_string);
	debug ("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	       _armboot_start, _bss_start, _bss_end);
#ifdef CONFIG_MODEM_SUPPORT
	debug ("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug ("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug ("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return (0);
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
static int display_dram_config (void)
{
	int i;

#ifdef DEBUG
	puts ("RAM Configuration:\n");

	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		printf ("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size (gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}
	//puts("DRAM:  ");
	//print_size(size, "\n");
#endif

	return (0);
}

#ifndef CONFIG_SYS_NO_FLASH
static void display_flash_config (ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}
#endif /* CONFIG_SYS_NO_FLASH */

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
static int init_func_i2c (void)
{
	puts ("I2C:   ");
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	puts ("ready\n");
    #if CONFIG_EXTERNAL_MCU
    ext_mcu_init();
    #endif
	return (0);
}
#endif

#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
#include <pci.h>
static int arm_pci_init(void)
{
	pci_init();
	return 0;
}
#endif /* CONFIG_CMD_PCI || CONFIG_PCI */

/*
 * Breathe some life into the board...
 *
 * Initialize a serial port as console, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependent #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

int print_cpuinfo (void);

init_fnc_t *init_sequence[] = {
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
	board_init,		/* basic board dependent setup */
#if defined(CONFIG_USE_IRQ)
//	interrupt_init,		/* set up exceptions */
#endif
//	timer_init,		/* initialize timer */
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_banner,		/* say that we are here */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
	dram_init,		/* configure available RAM banks */
#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
	arm_pci_init,
#endif
	display_dram_config,
	NULL,
};
/*******************************************************
 * Routine: usbpllset
 * Description: set usb pll
 ******************************************************/
static  void usbpllset ()
{
	printf("usbpllset: start\r\n");

	*(volatile uint32_t *)0x700080ac &= ~(3<<0);
	*(volatile uint32_t *)0x700080ac |= (3<<0);
	udelay(1000);
	*(volatile uint32_t *)0x7000f800 = 0x0093086a;
	udelay(1000);
	*(volatile uint32_t *)0x7000f860 = 0x01000002;
	udelay(1000);

	printf("usbpllset: end\r\n");
}
extern void mtdparts_set();
extern void dvp_uart_init();



void _reset(char mode, const char *cmd)
{
	/*
	 * use powerdown watch dog to reset system
	 */
	uint32_t u4Test;

	//printk("MTK Reboot is working now.\n");

	PDWNC_WRITE32(0x164, 0x24000164);

	PDWNC_WRITE32(REG_RW_WDT, 0xff000000);

	for(u4Test = 0; u4Test < 10000; u4Test++)
	{

	}
	PDWNC_WRITE32(REG_RW_WDTSET, 1);
	while(1);

}


#if 1    //each partition has one copy, when something wrong, we can use the copy to recovery
static void mmc_ops_copy(char* strPartitionName, unsigned long long u8StartAddr, unsigned long long u8PartitionSize)
{
	partitionread *ppartitionread,*p;

	ppartitionread = readpartitioninfofromflash();
	p = ppartitionread;

	while(p)
	{
		if (strcmp(p->szPartName, strPartitionName) == 0)
		{
			unsigned long long u8startAddr1 = u8StartAddr;
			unsigned long long u8startAddr2 = p->u8PartitionStartAddr;

			char *szValAddr1 = (char *)malloc(17);
			char *szValAddr2 = (char *)malloc(17);

			char sztempPhysAddr[16] = {0};

			sprintf(sztempPhysAddr, "0x%x", 0x4000000);
			memset(szValAddr1,0,17);
			memset(szValAddr2,0,17);

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
			if(isEnable(WRITE_PROTECT_ENABLE, p->u4Flag)) {
				if(emmc_set_user_wp(WP_DISABLE, p->u8PartitionStartAddr/512, p->u8PartitionSize/512, 1))
					printf("[WP]clear write protect fail(part: %s)\n", p->szPartName);
				else
					printf("[WP]clear write protect success(part: %s)\n", p->szPartName);
			}
#endif

			while ((u8startAddr1 < u8StartAddr + u8PartitionSize) &&
				    (u8startAddr2 < p->u8PartitionStartAddr + p->u8PartitionSize))
			{
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
				char *argv_read[6] = {"mmc","read","0",sztempPhysAddr,uitostr_hex(szValAddr1,(unsigned int)(u8startAddr1/512)), "0x800"};
#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
				char *argv_read[6] = {"mmc","read","2",sztempPhysAddr,uitostr_hex(szValAddr1,(unsigned int)(u8startAddr1/512)), "0x800"};
#endif

				if(0 == do_mmcops(NULL, 0, 6, argv_read))
				{
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
					char *argv_write[6] = {"mmc","write","0", sztempPhysAddr, uitostr_hex(szValAddr2,(unsigned int)(u8startAddr2/512)), "0x800"};
#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
					char *argv_write[6] = {"mmc","write","2", sztempPhysAddr, uitostr_hex(szValAddr2,(unsigned int)(u8startAddr2/512)), "0x800"};
#endif
					if(0 != do_mmcops(NULL, 0, 6, argv_write))
					{
						printf("\nWrite Backup Partition Failed!\n");
					}
				}

					u8startAddr1 = u8startAddr1 + 0x100000;
					u8startAddr2 = u8startAddr2 + 0x100000;
			}

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
			if(isEnable(WRITE_PROTECT_ENABLE, p->u4Flag)) {
				if(emmc_set_user_wp(WP_ENABLE, p->u8PartitionStartAddr/512, p->u8PartitionSize/512, 1))
					printf("[WP]set write protect fail(part: %s)\n", p->szPartName);
				else
					printf("[WP]set write protect success(part: %s)\n", p->szPartName);
			}
#endif
		}

		p = p->nextpartition;
	}

}

int check_backup_partition()
{
	partitionread *ppartitionread,*p;
    char szShowString[60]={0};

	ppartitionread = readpartitioninfofromflash();
	p = ppartitionread;

	LCD_CleanScreen();
	LCD_MallocStringBuf();

    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
    strcat(szShowString,"   ¿ªÊ¼»Ö¸´·ÖÇø ");
#else
    strcat(szShowString,"   Begin to recovery partition ");
#endif
    LCD_WriteString_FixedCharNum80(szShowString);

	while(p)
	{
		printf("check_back_partition %s\r\n", strstr(p->szPartName, "bk"));

		if (strstr(p->szPartName, "bk") != NULL)
		{
			char szVal[20] = {0};
			strncpy(szVal, p->szPartName, strlen(p->szPartName)-2);
			printf("check_back_partition %s\r\n", szVal);

			if(strcmp(szVal, "kernel") == 0)
			{
			    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ÕýÔÚ»Ö¸´·ÖÇø: ");
#else
				strcat(szShowString,"   Recoverying partition: ");
#endif
				strcat(szShowString, p->szPartName);
				strcat(szShowString,"...\n");
				//LCD_CleanScreen_part();
				//LCD_WriteString(szShowString);
				LCD_WriteString_FixedCharNum80(szShowString);
				mmc_ops_copy("kernel", p->u8PartitionStartAddr, p->u8PartitionSize);
			}

			if(0 == strcmp(szVal, "system"))
			{
			    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ÕýÔÚ»Ö¸´·ÖÇø: ");
#else
				strcat(szShowString,"   Recoverying partition: ");
#endif
				strcat(szShowString, p->szPartName);
				strcat(szShowString,"...\n");
				//LCD_CleanScreen_part();
				//LCD_WriteString(szShowString);
				LCD_WriteString_FixedCharNum80(szShowString);
				mmc_ops_copy("system", p->u8PartitionStartAddr, p->u8PartitionSize);
			}

			if(0 == strcmp(szVal, "app"))
			{
			    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ÕýÔÚ»Ö¸´·ÖÇø: ");
#else
				strcat(szShowString,"   Recoverying partition: ");
#endif
				strcat(szShowString, p->szPartName);
				strcat(szShowString,"...\n");
				//LCD_CleanScreen_part();
				//LCD_WriteString(szShowString);
				LCD_WriteString_FixedCharNum80(szShowString);
				mmc_ops_copy("app", p->u8PartitionStartAddr, p->u8PartitionSize);
			}
		}

		p = p->nextpartition;

	}

    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
    strcat(szShowString,"   »Ö¸´·ÖÇøÍê³É ");
#else
    strcat(szShowString,"   Recoverying has done ");
#endif
    LCD_WriteString_FixedCharNum80(szShowString);
    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
    strcat(szShowString,"   ÇëÖØÆôÏµÍ³ ");
#else
    strcat(szShowString,"   Please reboot the system ");
#endif
    LCD_WriteString_FixedCharNum80(szShowString);

	return 0;
}

#endif

#ifdef CONFIG_CMD_USB
int check_rsd_upgrade()
{
	int b_usb_upgrade = 0;

#if 0	// now we enter copy upgrade mode by press wakeup key during boot
#ifdef CONFIG_AC83XX_TOUCH
	b_usb_upgrade = b_usb_upgrade | (check_rtouch_pressed());
#endif

#ifdef CONFIG_AC83XX_TP
	i2c_init(400);
	b_usb_upgrade = b_usb_upgrade | (check_ctouch_pressed());
#endif

	return b_usb_upgrade;
#endif

    return 1;
}
#endif


void get_mmc_bootup_device_id_from_preloader()
{
	mmc_bootup_device = ((*(volatile UINT32 *)0xF0024168) & 0xF0000000) >> 28;

	printf("=========>>>  MMC Bootup Device: SD%d  <<<=========\n", mmc_bootup_device);
}

#define UBOOT_VER_MAIN     01
#define UBOOT_VER_MINOR  00
#define UBOOT_VER_REV       00

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

#define INIT_TVD_IN_UBOOT
#define INIT_TVD_WITH_PAL 0U
#ifdef INIT_TVD_IN_UBOOT
#define TVD_BASE_PA                   0xF00A7000U
#define VDOUT_CLOCK_GATED             0x00b4U       /* vdout clock gated*/
#define VDOUT_SYNC_RESET              0x00d0U       /* vdout tvd sync reset*/
#define SNOW_MODE                     0x414U
#define REG_VSRC_07                   0x434U
#define REG_VFE_00                    0x440U
#define REG_VFE_01                    0x444U
#define REG_VFE_02                    0x448U
#define REG_DFE_0E                    0x4F8U
#define REG_CDET_00                   0x540U


#define CLK_PDN_TVD1                    (1U << 1)
#define CLK_RESET_TVD1                  (1U << 1)
#define SIGNAL_MODE                     (0x7U << 28)    /*30:28*/
#define RG_UPDN                         (1U << 16)    /* CVBS enable clamp on blank for CHA, 0: disable, 1: enable*/
#define RG_VAGSELA                      (1U << 17)
#define RG_PGABUFNA_PWD                 (1U << 28)
#define RG_SHIFTA_PWD                   (1U << 24)
#define RG_OFFCURA_PWD                  (1U << 27)
#define RG_VSRC_INV_AIDX                (1U << 8)
#define VPRES4PIC_MODE                  (1U << 4)
#define VPRES1_MASK                     (1U << 0)
#define MODE000                         (1U << 4)
#define DET443_SEL                      (1U << 15)
#define RG_CVBS_REV_2                   (1U << 2)
#define RG_AISEL                        (0xFU << 8)


#define tvd_read(offset)              (*((volatile UINT32 *)(TVD_BASE_PA + (offset))))
#define tvd_write(offset, value)      (*((volatile UINT32 *)(TVD_BASE_PA + (offset))) = (value))
#define tvd_set_bit(offset, bit)       tvd_write((offset), tvd_read((offset)) | (bit))
#define tvd_clr_bit(offset, bit)       tvd_write((offset), tvd_read((offset)) & (~(bit)))
#define tvd_write_mask(offset, value, mask)    (tvd_write((offset), (((value) & (mask)) | (tvd_read((offset)) & (~mask)))))

enum {
	CVBSIN_0P = 0,
	CVBSIN_1P,
	CVBSIN_2P,
	CVBSIN_3P,
	CVBSIN_4P,
	CVBSIN_5P,
	CVBSIN_NONE
};

void tvd_clk_on(void)
{
	UINT32 u4Tmp, u4Reset;

	u4Tmp = *((volatile UINT32 *)(0xF0000000 + VDOUT_CLOCK_GATED));
	u4Reset = *((volatile UINT32 *)(0xF0000000 + VDOUT_SYNC_RESET));

	u4Tmp = u4Tmp | (CLK_PDN_TVD1);
	u4Reset = u4Reset | (CLK_RESET_TVD1);

	*((volatile UINT32 *)(0xF0000000 + VDOUT_CLOCK_GATED)) = u4Tmp;
	*((volatile UINT32 *)(0xF0000000 + VDOUT_SYNC_RESET)) = u4Reset;
}

void tvd_comb_setting(void)
{

	tvd_write(0x640u, 0x22110A10u);

	tvd_write(0x644u, 0xF0000006u);

	tvd_write(0x648u, 0x10002000u);

	tvd_write(0x64Cu, 0x3E00408Au);

	tvd_write(0x650u, 0x40300888u);

	tvd_write(0x654u, 0x6C000000u);

	tvd_write(0x658u, 0x00000067u);

	tvd_write(0x65Cu, 0x03440030u);

	tvd_write(0x660u, 0x01234444u);

	tvd_write(0x664u, 0x45678888u);

	tvd_write(0x668u, 0xF0100A8Du);

	tvd_write(0x66Cu, 0x00000003u);

	tvd_write(0x670u, 0x100D2808u);

	tvd_write(0x674u, 0x00801010u);

	tvd_write(0x678u, 0x84100A14u);

	tvd_write(0x67Cu, 0x01234567u);

	tvd_write(0x680u, 0x100AF850u);

	tvd_write(0x684u, 0x00000000u);

	tvd_write(0x688u, 0x00061410u);

	tvd_write(0x68Cu, 0x64101114u);

	tvd_write(0x690u, 0x0A074A36u);

	tvd_write(0x694u, 0x0A074A36u);

	tvd_write(0x698u, 0x007F9C00u);

	tvd_write(0x69Cu, 0x06000000u);

	tvd_write(0x6A0u, 0x0C0490E8u);

	tvd_write(0x6A4u, 0x00110333u);

	tvd_write(0x6A8u, 0x1488C084u);

	tvd_write(0x6ACu, 0xE8013434u);

	tvd_write(0x6B0u, 0x11202823u);

	tvd_write(0x6B4u, 0x04202411u);

	tvd_write(0x6B8u, 0x11111111u);

	tvd_write(0x6BCu, 0x000D8284u);

	tvd_write(0x6C0u, 0x00400833u);

	tvd_write(0x6C4u, 0x38081808u);

	tvd_write(0x6C8u, 0x60967050u);

	tvd_write(0x6CCu, 0x78801010u);

	tvd_write(0x6D0u, 0x0A0B4145u);

	tvd_write(0x6D4u, 0x0120FF05u);

	tvd_write(0x6D8u, 0x1C00640Au);

	tvd_write(0x6DCu, 0x0C006000u);

	tvd_write(0x6E0u, 0x0010101Fu);

	tvd_write(0x6E4u, 0x0069A900u);

	tvd_write(0x6E8u, 0x8A045AF4u);

	tvd_write(0x6ECu, 0x27030203u);

	tvd_write(0x6F0u, 0xC800000Fu);

	tvd_write(0x6F4u, 0x2F038408u);

	tvd_write(0x6F8u, 0x70061E05u);

	tvd_write(0x6FCu, 0x12D50000u);

	tvd_write(0x700u, 0x00DFA1B8u);

	tvd_write(0x704u, 0x200A80AAu);

	tvd_write(0x708u, 0x80000820u);

	tvd_write(0x70Cu, 0x11110435u);

	tvd_write(0x710u, 0x12345678u);

	tvd_write(0x714u, 0x02345678u);

	tvd_write(0x718u, 0x02345678u);

	tvd_write(0x71Cu, 0x01234567u);

	tvd_write(0x720u, 0x00081014u);

	tvd_write(0x724u, 0x00020206u);

	tvd_write(0x728u, 0x00000000u);

	tvd_write(0x72Cu, 0x00002480u);

	tvd_write(0x730u, 0x82040102u);

	tvd_write(0x734u, 0x00000041u);

	tvd_write(0x738u, 0x02040127u);

	tvd_write(0x73Cu, 0x00080000u);

	tvd_write(0x740u, 0x00015C1Eu);

	tvd_write(0x744u, 0x00800A10u);

	tvd_write(0x748u, 0x00000000u);

	tvd_write(0x74Cu, 0x3E40C028u);

	tvd_write(0x750u, 0x00800000u);

	tvd_write(0x754u, 0x45198064u);
	tvd_write(0x758u, 0x00000003u);

	tvd_write(0x75Cu, 0x1E0F1904u);

	tvd_write(0x760u, 0x83001405u);

	tvd_write(0x764u, 0x60145030u);

	tvd_write(0x768u, 0x08050510u);

	tvd_write(0x76Cu, 0x02345678u);

	tvd_write(0x770u, 0x00006110u);

	tvd_write(0x774u, 0x80007010u);

	tvd_write(0x778u, 0x00103111u);

	tvd_write(0x77Cu, 0x10009111u);
}

void init_tvd_common_register(void)
{
	UINT32 value = 0;

	/*Reset Register*/
	tvd_set_bit(0x400, (0x1U << 0)); /* reset register  */
	tvd_set_bit(0x400, (0x1U << 2)); /* reset TVD3D_core  */

	/*because it is level trigger,so we need to recovery it*/
	tvd_clr_bit(0x400, (0x1U << 0)); /* reset register  */
	tvd_clr_bit(0x400, (0x1U << 2)); /* reset TVD3D_core  */


	/* Init Common Register */
	tvd_set_bit(0x56c, (0x1U << 8)); /* fix mode detect error  */
	tvd_clr_bit(SNOW_MODE, (0x1U << 8)); /* clear force output snow  */
	tvd_clr_bit(SNOW_MODE, (0x1U << 9)); /* do not auto output snow */


	/* top layer control register */
	tvd_clr_bit(REG_VFE_00, (0x1U << 30));   /*cvbs power on */
	tvd_clr_bit(REG_VFE_01, (1U << 29)); /* cvbs ADC power on */
	tvd_clr_bit(REG_VFE_00, (1U << 23));   /*  input clamp power on */
	tvd_clr_bit(REG_VFE_02, (1U << 14));    /*  ADC ref_clk select(0: Pll , 1 XTAL) */
	tvd_clr_bit(REG_VFE_00, (1U << 25));

	/* RG_GLB_PWD */
	/* HAL_WRITE32((IO_BASE_VA+0x6A0), 0x00240000); */
	/* RG_GLB_PWD only bit 17 we need to care */
	value = (*((volatile UINT32 *)(0xF00006a0)));
	(*((volatile UINT32 *)(0xF00006a0))) = (value & (~(1U << 17)));


	/*channel A enable */
	tvd_set_bit(REG_VFE_00, RG_UPDN); /*  cvbd enable clamp on  blank for CHA */
	tvd_set_bit(REG_VFE_00, RG_VAGSELA); /* cvbs channel A    PGA CM buffer input  0.5v */
	tvd_clr_bit(REG_VFE_00, RG_PGABUFNA_PWD); /* cvbs  channel A  input  BUFFER  power on */
	tvd_clr_bit(REG_VFE_00, RG_SHIFTA_PWD); /*  cvbs channel A shift power on */
	tvd_clr_bit(REG_VFE_00, RG_OFFCURA_PWD); /*  cvbs channel A offset current power on */


	tvd_set_bit(REG_VSRC_07, RG_VSRC_INV_AIDX);  /* select CHA to TVD */
	tvd_clr_bit(REG_DFE_0E, VPRES4PIC_MODE);
	tvd_clr_bit(REG_DFE_0E, VPRES1_MASK);

	tvd_write(0x5DC, 0x700C4A60);
	tvd_write_mask(0x570, (0 << 14), (1 << 14));

	tvd_set_bit(REG_CDET_00, MODE000);    /* init tvd mode is : PAL              A7F540[4]= 1 */
	tvd_clr_bit(REG_CDET_00, DET443_SEL);
#if INIT_TVD_WITH_PAL
	tvd_clr_bit(REG_CDET_00, (1U << 6));
	tvd_set_bit(REG_CDET_00, (1U << 7));
	tvd_set_bit(REG_CDET_00, (1U << 8));
#endif

	tvd_write(0x418, 0xFF001996);
	tvd_write(0x41C, 0x1EBD0010);
	tvd_write(0x420, 0x15710000);
	tvd_set_bit(0x424, 0x1<<31);
	tvd_write_mask(0x424, (0x0F0000 << 0), 0xFFFFFF);
#if INIT_TVD_WITH_PAL
	tvd_write_mask(0x540u, 0x30173F93u, 0xFFFFFFF0u);
#else
	tvd_write_mask(0x540u, 0x30173E63u, 0xFFFFFFF0u);
#endif
	tvd_comb_setting();

	tvd_write(0x4FC, 0x420C5564);
}

void tvd_init(void)
{
	unsigned int u4CHACvbsInxP = CVBSIN_1P;

	tvd_clk_on();
	init_tvd_common_register();
	switch (u4CHACvbsInxP) {
	case CVBSIN_1P:
		tvd_set_bit(REG_VFE_02, RG_CVBS_REV_2); /*  select CVBS0P */
		tvd_write_mask(REG_VFE_00, (0 << 8), RG_AISEL);
		break;

	case CVBSIN_2P:
		tvd_clr_bit(REG_VFE_02, RG_CVBS_REV_2); /*  no select CVBS0P to CHA */
		tvd_write_mask(REG_VFE_00, (1 << 8), RG_AISEL);     /* select CVBS1P to CHA */
		break;

	case CVBSIN_3P:
		tvd_clr_bit(REG_VFE_02, RG_CVBS_REV_2); /* no select CVBS0P to CHA */
		tvd_write_mask(REG_VFE_00, (2 << 8), RG_AISEL);     /* select CVBS2P to CHA */
		break;

	case CVBSIN_4P:
		tvd_clr_bit(REG_VFE_02, RG_CVBS_REV_2); /* no select CVBS0P to CHA */
		tvd_write_mask(REG_VFE_00, (4 << 8), RG_AISEL);     /* select CVBS3P to CHA */
		break;

	case CVBSIN_5P:
		tvd_clr_bit(REG_VFE_02, RG_CVBS_REV_2); /*  no select CVBS0P to CHA */
		tvd_write_mask(REG_VFE_00, (8 << 8), RG_AISEL);     /*  select CVBS4P to CHA */
		break;

	default:
		tvd_clr_bit(REG_VFE_02, RG_CVBS_REV_2);  /* no select CVBS0P to CHA */
		tvd_write_mask(REG_VFE_00, 0, RG_AISEL);      /*  select CVBSNONE to CHA */
		break;
	}
}

#endif
extern int get_mtz_upg_mode(void);
void start_armboot (void)
{
	init_fnc_t **init_fnc_ptr;
	char *s;
	int upg_mode = 0;
#if defined(CONFIG_VFD) || defined(CONFIG_LCD)
	unsigned long addr;
#endif
	int b_recovery_bakcup = 0;
	int ret = 0;
	unsigned int time_ms = 0;

#ifdef INIT_TVD_IN_UBOOT
	tvd_init();
#endif


	icache_enable();
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t*)(_armboot_start - CONFIG_SYS_MALLOC_LEN - sizeof(gd_t));
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset ((void*)gd, 0, sizeof (gd_t));
	gd->bd = (bd_t*)((char*)gd - sizeof(bd_t));
	memset (gd->bd, 0, sizeof (bd_t));

	gd->bd->bi_env = (env_t*)((char*)gd - sizeof(bd_t) - sizeof(env_t));
	memset (gd->bd->bi_env, 0, sizeof (env_t));

	gd->flags |= GD_FLG_RELOC;

	monitor_flash_len = _bss_start - _armboot_start;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}
	time_ms = boot_time_ms();
	printf("[VER][UBOOT] V%02d.%02d_%02d_%06d [%s] [Time] %s %s %s\r\n", UBOOT_VER_MAIN, UBOOT_VER_MINOR, UBOOT_VER_REV, P4_CHANGELIST, BRANCH_NAME, (__DATE__),(__TIME__), AUTO_BUILD);
	printf("[Uboot] start boot time: %10d\n", time_ms);
	//printf("start_armboot  1, s = 0x%x, gd  = 0x%x \r\n", s, gd);
	//printf("start_armboot _sdagentflag = 0x%x\r\n",_sdagentflag);

	get_mmc_bootup_device_id_from_preloader();

#ifdef CONFIG_BOOT_MMC
#if defined (BOOT_FROM_EMMC)
	printf("===============================>>> EMMC BOOT <<<===================================\r\n");
#elif defined (BOOT_FROM_SD2)
	printf("================================>>> SD2 BOOT <<<====================================\r\n");
#endif
#else
	printf("===============================>>> NAND BOOT <<<===================================\r\n");
	if (_sdagentflag == 1)//we need read arm2 and logo from bootup SD CARD when do nand upgrade, by mtk68080
	{
		printf("Do Nand Upgrade, now check BOOTUP  SD CARD:\r\n");
	}
#endif

	if (_sdagentflag == 2)
	{
		printf("Boot Special Recovery System From Ext SD Card\r\n");
	}



	/* armboot_start is defined in the board-specific linker script */
	mem_malloc_init (_armboot_start - CONFIG_SYS_MALLOC_LEN);

#ifndef CONFIG_SYS_NO_FLASH
	/* configure available FLASH banks */
	display_flash_config (flash_init ());
#endif /* CONFIG_SYS_NO_FLASH */

#ifdef CONFIG_VFD
#	ifndef PAGE_SIZE
#	  define PAGE_SIZE 4096
#	endif
	/*
	 * reserve memory for VFD display (always full pages)
	 */
	/* bss_end is defined in the board-specific linker script */
	addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
	vfd_setmem (addr);
	gd->fb_base = addr;
#endif /* CONFIG_VFD */

#ifdef CONFIG_LCD
	/* board init may have inited fb_base */
	if (!gd->fb_base) {
#		ifndef PAGE_SIZE
#		  define PAGE_SIZE 4096
#		endif
		/*
		 * reserve memory for LCD display (always full pages)
		 */
		/* bss_end is defined in the board-specific linker script */
		addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
		lcd_setmem (addr);
		gd->fb_base = addr;
	}
#endif /* CONFIG_LCD */

#ifndef CONFIG_MACH_AC8317FPGA
	usbpllset(); //bin yang remove
#endif

#ifdef CONFIG_CMD_NAND
#ifdef LOAD_ATC_NAND_DRV
	printf("NAND init start:  %d\n", boot_time_ms());
	nand_init();		/* go init the NAND */
	printf("NAND init end:  %d\n", boot_time_ms());
#endif
#endif



#ifdef _NOT_USED_
#if ACON_STANDBY
	puts("[AC83XX Boot] Standby on starts\n");
	/* Checking standby mode */
	v_ac83xx_standby();
	puts("[AC83XX Boot] Standby on ends\n");
#endif
#endif

#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif

#ifdef CONFIG_HAS_DATAFLASH
	AT91F_DataflashInit();
	dataflash_print_info();
#endif

#ifdef CONFIG_GENERIC_MMC
	//puts ("MMC:   ");
	mmc_initialize (gd->bd);
#endif

	/* initialize environment */
	printf("env_relocate start:  %d\n", boot_time_ms());
	env_relocate ();
	printf("env_relocate end:  %d\n", boot_time_ms());

#ifdef CONFIG_VFD
	/* must do this after the framebuffer is allocated */
	drv_vfd_init();
#endif /* CONFIG_VFD */

#ifdef CONFIG_AC83XX_VFD
	vStbyVFDInit();
#endif

#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif

	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

	stdio_init ();	/* get the devices list going. */

	jumptable_init ();

#if defined(CONFIG_API)
	/* Initialize API */
	api_init ();
#endif

	console_init_r ();	/* fully init console as a device */

	if (_sdagentflag == 0)
	{
#ifndef CONFIG_BOOT_MMC
		mtdparts_set();  //nand boot will need a default mtdparts before merge from partition table
#endif
		//g_partitionstr is parts info for kernel
		printf("check_partition start:  %d\n", boot_time_ms());
		g_partitionstr = check_partition();  //will update g_mtdparts and g_bootcmd at the same time
		printf("check_partition end:  %d\n", boot_time_ms());
	}



#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init ();
#endif


#ifndef CONFIG_BOOT_MMC
	if (_sdagentflag == 0) //upgrade
	{
		setenv("mtdparts", g_mtdparts);
	}
	else
	{
		mtdparts_set();
	}
#endif

#ifdef ATC_AB_PARTITION_SUPPORT
	if (_sdagentflag == 0) {
		printf("Normal boot need check ab slot\n");
		ab_boot_check();
	}
#endif

	if (_sdagentflag == 0) // above already use checkpartition() to make g_bootcmd acording to real kernel and rootfs position
		setenv("bootcmd", g_bootcmd);


#if defined(CONFIG_MISC_INIT_R)
	//#ifndef CONFIG_CMD_SDAGENT
#ifndef CONFIG_MSDC_ETT
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif
	//#endif
#endif
	/* enable exceptions */
	//enable_interrupts ();

	/* Perform network card initialisation if necessary */
#ifdef CONFIG_DRIVER_TI_EMAC
	/* XXX: this needs to be moved to board init */
	extern void davinci_eth_set_mac_addr (const u_int8_t *addr);
	if (getenv ("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		davinci_eth_set_mac_addr(enetaddr);
	}
#endif

#ifdef CONFIG_DRIVER_CS8900
	/* XXX: this needs to be moved to board init */
	cs8900_get_enetaddr ();
#endif

#if defined(CONFIG_DRIVER_SMC91111) || defined (CONFIG_DRIVER_LAN91C96)
	/* XXX: this needs to be moved to board init */
	if (getenv ("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		smc_set_mac_addr(enetaddr);
	}
#endif /* CONFIG_DRIVER_SMC91111 || CONFIG_DRIVER_LAN91C96 */

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif

#ifdef BOARD_LATE_INIT
	board_late_init ();
#endif

#if defined(CONFIG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	puts ("Net:   ");
#endif
	eth_initialize(gd->bd);
#if defined(CONFIG_RESET_PHY_R)
	debug ("Reset Ethernet PHY\n");
	reset_phy();
#endif
#endif

#ifdef ONEKEY_RECOVERY_ENABLE
	GPIO_MultiFun_Set(PIN_URXD5,PINMUX_LEVEL_GPIO_END_FLAG);
	gpio_direction_input(PIN_URXD5);
	GPIO_Pull_UpDown(PIN_URXD5, PULLDOWN); // Default pull down
	b_recovery_bakcup = gpio_get_value(PIN_URXD5);// Use  GPIO 32  = 1 (high)

	printf("\nDetect recovery backup partition request! b_recovery_bakcup=%d\n", b_recovery_bakcup);

	if (b_recovery_bakcup)
	{
		SetUpgradeMode(1);
		if (check_backup_partition() == 0)
		{
			while(1);
		}
	}
#endif

	if (_sdagentflag == 0) {
		upg_mode = get_mtz_upg_mode();
		printf("upgrade mode is %d\n", upg_mode);
		if (upg_mode == 1) {
			FastBoot();
			while(1);
		} else if (upg_mode == 2) {
			ret = do_rsd_upgrade();
			if (!ret) {
				printf("copyupgrade ok, reboot platform now\n");
				_reset(0, NULL);
			} else {
				printf("copyupgrade fail, need reboot manual\n");
				while(1);
			}
		}
	}

#if 0
	// when wakeup key press down, we will check usb/sd card copy upgrade
	if (GetCopyUpgradeMode() != 0)
	{
		printf("Wakeup key press down, check USB/SD card copy upgrade!!\r\n");
		//#if 0//def CONFIG_CMD_USB
		//if (check_rsd_upgrade() != 0)
		//{
		if (do_rsd_upgrade() == 0)
		{
			_reset(0, NULL);
			//while(1);
		}else{
			printf("please reset system!!\r\n");
			while(1);
		}
		//}
		//#endif
	}
	else
	{
		printf("Wakeup key not press down, no copy upgrade!!\r\n");
		printf("We will boot normal!!\r\n");
	}
#endif

#ifdef CONFIG_MSDC_ETT
#define FORCE_MSDC_ETT		(1) // Max Xia Add, Force build MSDC ETT bin, this flag make MSDC ETT uboot is run in SD2.
#else
#define FORCE_MSDC_ETT		(0)
#endif

#if (FORCE_MSDC_ETT == 0)
#ifndef  CONFIG_NAND_DEBUG
	if (_sdagentflag == 1)
	{
		ret = run_command("sdagent", 0);
		if (ret == -1)
		{
			printf(".................................................\r\n");
			printf("......... Upgrade Linux Image fail!!! ........\r\n");
			printf(".................................................\r\n\r\n");
			printf("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
		}
		else
		{
			printf(".................................................\r\n");
			printf("......... Upgrade Linux Image Success ........\r\n");
			printf(".................................................\r\n\r\n");
			printf("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
		}

		//while(1);
		printf("Press Enter key to enter Debug Mode\r\n");
		while(1)
		{
			if (getc() == '\r')
				break;
		}

		setenv("bootcmd", "");

		while(1)
		{
			main_loop();
		}
	}
#endif // CONFIG_NAND_DEBUG_VERSION

#endif

	//#define UBOOT_POWER_ON_OFF_TEST
#ifdef UBOOT_POWER_ON_OFF_TEST
	{
		//use gpio 136 for test
		//output hight when boot is ok
		// add by mtk68080
		unsigned int time = 0;
		time = boot_time_ms();
		//set gpio136 as output
		ac83xx_gpio_inout_sel_reg(136, 1);
		//gpio 136 output high
		ac83xx_gpio_set_value_reg(136, 1);
		printf("[Uboot] Power on time: %dms\r\n", time);
		printf("[Uboot] GPIO136 output high now.\r\n");
	}
#else
	printf("[Uboot] Power ON_OFF Test Not Enable.\r\n");
#endif


#ifndef CONFIG_MSDC_ETT
#ifdef CONFIG_DVP_SUPPORT
	printf("Config DVP Feature\r\n");
	dvp_uart_init();
	//printf("ready to DVDInit\n");
	DVDInit();
#endif

#ifdef CONFIG_ANDROID_RECOVERY
	//check_recovery_mode();
#endif
#endif	// CONFIG_MSDC_ETT



#if 0
	printf("...TRANSFER DVP RS232 TO UART3 START...\r\n");
	*(volatile UINT32 *)0xF0000058 = ((*(volatile UINT32 *)0xF0000058) & (~(3<<15)));
	*(volatile UINT32 *)0xF000006C = ((*(volatile UINT32 *)0xF000006C)| (1<<16));
	printf("...TRANSFER DVP RS232 TO UART3 END...0x%x,0x%x\r\n",*(volatile UINT32 *)0xF0000058,*(volatile UINT32 *)0xF000006C);
#endif
	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
