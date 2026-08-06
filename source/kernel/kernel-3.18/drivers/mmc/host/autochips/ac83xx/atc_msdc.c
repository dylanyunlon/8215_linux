#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/dma-mapping.h>
#include <linux/irq.h>
#include <linux/suspend.h>
#include <linux/clk.h>
#include <linux/clk-private.h>

#include <mach/dma.h>
#include <mach/board.h>
#include <mach/irqs.h>
#include <mach/power_loss_emmc_test.h>
#include <linux/mmc/atc_storage_partition.h>

#define ATC_GPIO_KS		(0)
#if ATC_GPIO_KS
#include <linux/gpio/consumer.h>
#else
#include <linux/gpio.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/pinmux.h>
#endif

#include "atc_msdc.h"
#include "atc_msdc_dbg.h"
#include "atc_msdc_ett.h"

#include <linux/proc_fs.h>
#include "../card/queue.h"
#include "../../misc/atc/inc/x_ver.h" 
#include <mach/memory.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <../../atc_modules/connectivity/atc_combo/atc_combo.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
#include <../drivers/soc/autochips/connectivity/atc_combo/atc_combo.h>
#else // 3.18
#include <../drivers/net/wireless/connectivity/atc_combo/atc_combo.h>
#endif

#define DRV_NAME            "atc-msdc"
#define MSDC_USE_CCF        (1)

#define CONV2UINT(x)		((unsigned int)(x))

#define MSDC_EMMC_POWER_OFF_VCC			(1)	/* Shut down entrie eMMC power when suspend/resume */

#if 0//MTK_EMMC_SUPPORT
#define CAPACITY_2G			(2 * 1024 * 1024 * 1024ULL)
u32 g_emmc_mode_switch = 0;
#endif

#if (MSDC_DATA1_INT == 1)
static u16 u_sdio_irq_counter;
static u16 u_msdc_irq_counter;
static int int_sdio_irq_enable;
#endif
unsigned char gpiono;
unsigned char trigger;
int protect_level;
int protect_gpio=0;
/* For GPIO Polling CD method */
int run_card_detect = 0;

struct mmc_blk_data {
	spinlock_t lock;
	struct gendisk *disk;
	struct mmc_queue queue;
	unsigned int usage;
	unsigned int read_only;
};

struct msdc_host *atc_msdc_host[HOST_MAX_NUM] = { NULL, NULL, NULL };
int g_dma_debug[HOST_MAX_NUM] = { 0, 0, 0 };
u32 latest_int_status[HOST_MAX_NUM] = { 0, 0, 0 };

/* 0 for PIO; 1 for DMA; 2 for nothing */
transfer_mode msdc_latest_transfer_mode[HOST_MAX_NUM] = {
	TRAN_MOD_NUM,
	TRAN_MOD_NUM,
	TRAN_MOD_NUM,
};

/* 0 for read; 1 for write; 2 for nothing */
operation_type msdc_latest_operation_type[HOST_MAX_NUM] = {
	OPER_TYPE_NUM,
	OPER_TYPE_NUM,
	OPER_TYPE_NUM,
};

struct dma_addr msdc_latest_dma_address[MAX_BD_PER_GPD];

static int msdc_rsp[] = {
	0,			/* RESP_NONE */
	1,			/* RESP_R1 */
	2,			/* RESP_R2 */
	3,			/* RESP_R3 */
	4,			/* RESP_R4 */
	1,			/* RESP_R5 */
	1,			/* RESP_R6 */
	1,			/* RESP_R7 */
	7,			/* RESP_R1b */
};

/* clock source for host: global */
static u32 hclksrc[] = {
	200000000, 196000000, 189000000, 162000000, 147000000, 135000000, 100000000, 27000000
};

extern int ett_tune_flag;
static u8 emmc_id;
static bool sdio_resume_back = false;
static int  sdio_init_status=0;

/****************************************************************************/
/* For C build error,  function declaration at first */
/****************************************************************************/
static int msdc_pad_init(struct msdc_host *host, u32 nand_boot);
static int card_detect_gpio(struct msdc_host *host);
extern void ac83xx_mask_ack_bim_irq(uint32_t irq);
extern int sd_reinit(struct mmc_host *host, u32 ocr,struct mmc_card *oldcard);
void msdc_force_reinit(struct msdc_host *host);
static int msdc_lower_onlyfreq(struct msdc_host *host);

#ifdef CONFIG_MMC_ATC_SW_WP
#define MAX_WP_REGION 5
struct wp_region_t {
u32 sect_start;
u32 sect_end;
};
struct wp_region_info {
	struct wp_region_t region[MAX_WP_REGION];
	int region_num;
};
struct wp_region_info wp_rg_info = { 0 };

int emmc_get_write_protect_region(struct wp_region_info *wpinfo)
{
	uint8_t i;
	bool found = false;
	u64 ro_start, ro_end;

	memset(wpinfo, 0, sizeof(struct wp_region_info));

	for(i = 0; i < PART_NUM; i++) {
		if(strncmp(PartInfo[i].rw_type, "RO", 2) == 0) {
			if(found == false) {
				ro_start = PartInfo[i].part_offset;
				ro_end = PartInfo[i].part_offset + PartInfo[i].part_size;
				found = true;
			} else {
				ro_end += PartInfo[i].part_size;
			}

		} else {
			if(found == true) {
				found = false;
				wpinfo->region[wpinfo->region_num].sect_start = ro_start >> 9;
				wpinfo->region[wpinfo->region_num].sect_end = ro_end >> 9;
				wpinfo->region_num += 1;
				if(wpinfo->region_num > MAX_WP_REGION)
					goto fail;
			}

		}
	}
	if(found == true) {
		wpinfo->region[wpinfo->region_num].sect_start = ro_start >> 9;
		wpinfo->region[wpinfo->region_num].sect_end = ro_end >> 9;
		wpinfo->region_num += 1;
		if(wpinfo->region_num > MAX_WP_REGION)
			goto fail;
	}
	for (i = 0; i < wpinfo->region_num; i++) {
		MSDC_LOG_NORMAL(pr_info, "write protect region[%d]: 0x%x ~ 0x%x\n", i,
			wpinfo->region[i].sect_start, wpinfo->region[i].sect_end);
	}
	return wpinfo->region_num;
fail:
	MSDC_LOG_NORMAL(pr_err, "to many ro regions\n");
	return -1;
}

bool is_in_wp_area(u32 sect_addr) {
	int i;
	if(wp_rg_info.region_num > MAX_WP_REGION)
		wp_rg_info.region_num = MAX_WP_REGION;

	for(i = 0; i < wp_rg_info.region_num; i++) {
		if((sect_addr < wp_rg_info.region[i].sect_start) || (sect_addr >= wp_rg_info.region[i].sect_end))
			continue;
		else
			return true;
	}

	return false;
}
#endif

/****************************************************************************/
/* Debug Function                                                                                                              */
/****************************************************************************/
/*static void msdc_dump_tuning_register(struct msdc_host *host)
{
	u32 base = host->base;
	MSDC_LOG(DUMP, "[TMP] MSDC_IOCON     = 0x%.8x", MSDC_READ32(base + 0x04U));
	MSDC_LOG(DUMP, "[TMP] PATCH_BIT0     = 0x%.8x", MSDC_READ32(base + 0xB0U));
	MSDC_LOG(DUMP, "[TMP] PATCH_BIT1     = 0x%.8x", MSDC_READ32(base + 0xB4U));
	MSDC_LOG(DUMP, "[TMP] PAD_TUNE       = 0x%.8x", MSDC_READ32(base + 0xECU));
}*/

static void msdc_dump_register(struct msdc_host *host)
{
	u32 base = host->base;

	MSDC_LOG(DUMP, "Reg[00] MSDC_CFG       = 0x%.8x", MSDC_READ32(base + 0x00U));
	MSDC_LOG(DUMP, "Reg[04] MSDC_IOCON     = 0x%.8x", MSDC_READ32(base + 0x04U));
	MSDC_LOG(DUMP, "Reg[08] MSDC_PS        = 0x%.8x", MSDC_READ32(base + 0x08U));
	MSDC_LOG(DUMP, "Reg[0C] MSDC_INT       = 0x%.8x", MSDC_READ32(base + 0x0CU));
	MSDC_LOG(DUMP, "Reg[10] MSDC_INTEN     = 0x%.8x", MSDC_READ32(base + 0x10U));
	MSDC_LOG(DUMP, "Reg[14] MSDC_FIFOCS    = 0x%.8x", MSDC_READ32(base + 0x14U));
	MSDC_LOG(DUMP, "Reg[30] SDC_CFG        = 0x%.8x", MSDC_READ32(base + 0x30U));
	MSDC_LOG(DUMP, "Reg[34] SDC_CMD        = 0x%.8x", MSDC_READ32(base + 0x34U));
	MSDC_LOG(DUMP, "Reg[38] SDC_ARG        = 0x%.8x", MSDC_READ32(base + 0x38U));
	MSDC_LOG(DUMP, "Reg[3C] SDC_STS        = 0x%.8x", MSDC_READ32(base + 0x3CU));
	MSDC_LOG(DUMP, "Reg[40] SDC_RESP0      = 0x%.8x", MSDC_READ32(base + 0x40U));
	MSDC_LOG(DUMP, "Reg[44] SDC_RESP1      = 0x%.8x", MSDC_READ32(base + 0x44U));
	MSDC_LOG(DUMP, "Reg[48] SDC_RESP2      = 0x%.8x", MSDC_READ32(base + 0x48U));
	MSDC_LOG(DUMP, "Reg[4C] SDC_RESP3      = 0x%.8x", MSDC_READ32(base + 0x4CU));
	MSDC_LOG(DUMP, "Reg[50] SDC_BLK_NUM    = 0x%.8x", MSDC_READ32(base + 0x50U));
	MSDC_LOG(DUMP, "Reg[58] SDC_CSTS       = 0x%.8x", MSDC_READ32(base + 0x58U));
	MSDC_LOG(DUMP, "Reg[5C] SDC_CSTS_EN    = 0x%.8x", MSDC_READ32(base + 0x5CU));
	MSDC_LOG(DUMP, "Reg[60] SDC_DATCRC_STS = 0x%.8x", MSDC_READ32(base + 0x60U));
	MSDC_LOG(DUMP, "Reg[70] EMMC_CFG0      = 0x%.8x", MSDC_READ32(base + 0x70U));
	MSDC_LOG(DUMP, "Reg[74] EMMC_CFG1      = 0x%.8x", MSDC_READ32(base + 0x74U));
	MSDC_LOG(DUMP, "Reg[78] EMMC_STS       = 0x%.8x", MSDC_READ32(base + 0x78U));
	MSDC_LOG(DUMP, "Reg[7C] EMMC_IOCON     = 0x%.8x", MSDC_READ32(base + 0x7CU));
	MSDC_LOG(DUMP, "Reg[80] SD_ACMD_RESP   = 0x%.8x", MSDC_READ32(base + 0x80U));
	MSDC_LOG(DUMP, "Reg[84] SD_ACMD19_TRG  = 0x%.8x", MSDC_READ32(base + 0x84U));
	MSDC_LOG(DUMP, "Reg[88] SD_ACMD19_STS  = 0x%.8x", MSDC_READ32(base + 0x88U));
	MSDC_LOG(DUMP, "Reg[90] DMA_SA         = 0x%.8x", MSDC_READ32(base + 0x90U));
	MSDC_LOG(DUMP, "Reg[94] DMA_CA         = 0x%.8x", MSDC_READ32(base + 0x94U));
	MSDC_LOG(DUMP, "Reg[98] DMA_CTRL       = 0x%.8x", MSDC_READ32(base + 0x98U));
	MSDC_LOG(DUMP, "Reg[9C] DMA_CFG        = 0x%.8x", MSDC_READ32(base + 0x9CU));
	MSDC_LOG(DUMP, "Reg[A0] SW_DBG_SEL     = 0x%.8x", MSDC_READ32(base + 0xA0U));
	MSDC_LOG(DUMP, "Reg[A4] SW_DBG_OUT     = 0x%.8x", MSDC_READ32(base + 0xA4U));
	MSDC_LOG(DUMP, "Reg[A8] DMA_LENGTH     = 0x%.8x", MSDC_READ32(base + 0xA8U));
	MSDC_LOG(DUMP, "Reg[B0] PATCH_BIT0     = 0x%.8x", MSDC_READ32(base + 0xB0U));
	MSDC_LOG(DUMP, "Reg[B4] PATCH_BIT1     = 0x%.8x", MSDC_READ32(base + 0xB4U));
	MSDC_LOG(DUMP, "Reg[EC] PAD_TUNE       = 0x%.8x", MSDC_READ32(base + 0xECU));
	MSDC_LOG(DUMP, "Reg[F0] DAT_RD_DLY0    = 0x%.8x", MSDC_READ32(base + 0xF0U));
	MSDC_LOG(DUMP, "Reg[F4] DAT_RD_DLY1    = 0x%.8x", MSDC_READ32(base + 0xF4U));
	MSDC_LOG(DUMP, "Reg[F8] HW_DBG_SEL     = 0x%.8x", MSDC_READ32(base + 0xF8U));
	MSDC_LOG(DUMP, "Rg[100] MAIN_VER       = 0x%.8x", MSDC_READ32(base + 0x100U));
	MSDC_LOG(DUMP, "Rg[104] ECO_VER        = 0x%.8x", MSDC_READ32(base + 0x104U));
}

static void msdc_debug_reg(struct msdc_host *host)
{
	u32 base = host->base;
	u32 i;

	for (i = 0; i < 26; i++) {
		MSDC_WRITE32((base + 0xA0U), i);
		MSDC_LOG(DUMP, "SW_DBG_SEL: write reg[%x] to 0x%x", (base + 0xA0U), i);
		MSDC_LOG(DUMP, "SW_DBG_OUT: read  reg[%x] to 0x%x", (base + 0xA4U), MSDC_READ32(base + 0xA4U));
	}

	MSDC_WRITE32((base + 0xA0U), 0);
}

#if 0
static void msdc_dump_card_status(struct msdc_host *host, u32 status)
{
	static const char *state[] const = {
		"Idle",		/* 0 */
		"Ready",	/* 1 */
		"Ident",	/* 2 */
		"Stby",		/* 3 */
		"Tran",		/* 4 */
		"Data",		/* 5 */
		"Rcv",		/* 6 */
		"Prg",		/* 7 */
		"Dis",		/* 8 */
		"Reserved",	/* 9 */
		"Reserved",	/* 10 */
		"Reserved",	/* 11 */
		"Reserved",	/* 12 */
		"Reserved",	/* 13 */
		"Reserved",	/* 14 */
		"I/O mode",	/* 15 */
	};

	if (status & R1_OUT_OF_RANGE) {
		MSDC_LOG(RSP, "[CARD_STATUS] Out of Range");
	}

	if (status & R1_ADDRESS_ERROR) {
		MSDC_LOG(RSP, "[CARD_STATUS] Address Error");
	}

	if (status & R1_BLOCK_LEN_ERROR) {
		MSDC_LOG(RSP, "[CARD_STATUS] Block Len Error");
	}

	if (status & R1_ERASE_SEQ_ERROR) {
		MSDC_LOG(RSP, "[CARD_STATUS] Erase Seq Error");
	}

	if (status & R1_ERASE_PARAM) {
		MSDC_LOG(RSP, "[CARD_STATUS] Erase Param");
	}

	if (status & R1_WP_VIOLATION) {
		MSDC_LOG(RSP, "[CARD_STATUS] WP Violation");
	}

	if (status & R1_CARD_IS_LOCKED) {
		MSDC_LOG(RSP, "[CARD_STATUS] Card is Locked");
	}

	if (status & R1_LOCK_UNLOCK_FAILED) {
		MSDC_LOG(RSP, "[CARD_STATUS] Lock/Unlock Failed");
	}

	if (status & R1_COM_CRC_ERROR) {
		MSDC_LOG(RSP, "[CARD_STATUS] Command CRC Error");
	}

	if (status & R1_ILLEGAL_COMMAND) {
		MSDC_LOG(RSP, "[CARD_STATUS] Illegal Command");
	}

	if (status & R1_CARD_ECC_FAILED) {
		MSDC_LOG(RSP, "[CARD_STATUS] Card ECC Failed");
	}

	if (status & R1_CC_ERROR) {
		MSDC_LOG(RSP, "[CARD_STATUS] CC Error");
	}

	if (status & R1_ERROR) {
		MSDC_LOG(RSP, "[CARD_STATUS] Error");
	}

	if (status & R1_UNDERRUN) {
		MSDC_LOG(RSP, "[CARD_STATUS] Underrun");
	}

	if (status & R1_OVERRUN) {
		MSDC_LOG(RSP, "[CARD_STATUS] Overrun");
	}

	if (status & R1_CID_CSD_OVERWRITE) {
		MSDC_LOG(RSP, "[CARD_STATUS] CID/CSD Overwrite");
	}

	if (status & R1_WP_ERASE_SKIP) {
		MSDC_LOG(RSP, "[CARD_STATUS] WP Eraser Skip");
	}

	if (status & R1_CARD_ECC_DISABLED) {
		MSDC_LOG(RSP, "[CARD_STATUS] Card ECC Disabled");
	}

	if (status & R1_ERASE_RESET) {
		MSDC_LOG(RSP, "[CARD_STATUS] Erase Reset");
	}

	if ((status & R1_READY_FOR_DATA) == 0) {
		MSDC_LOG(RSP, "[CARD_STATUS] Not Ready for Data");
	}

	if (status & R1_SWITCH_ERROR) {
		MSDC_LOG(RSP, "[CARD_STATUS] Switch error");
	}

	if (status & R1_APP_CMD) {
		MSDC_LOG(RSP, "[CARD_STATUS] App Command");
	}

	MSDC_LOG(RSP, "[CARD_STATUS] '%s' State", state[R1_CURRENT_STATE(status)]);
}
#endif




/****************************************************************************/
/* Debug Function                                                                                                              */
/****************************************************************************/

static void msdc_dump_clock_sts(struct msdc_host *host)
{

}

static void msdc_dump_info(u32 id)
{
	struct msdc_host *host = atc_msdc_host[id];
	u32 base;
	u32 temp;

	if (host == NULL) {
		MSDC_LOG(ERR, "msdc host<%d> null", id);
		return;
	}

	base = host->base;

	/* 1: dump msdc hw register */
	msdc_dump_register(host);
	MSDC_LOG(DUMP, "latest_INT_status<0x%.8x>", latest_int_status[id]);
	/* 2: check msdc clock gate and clock source */
#if 0
	msdc_dump_clock_sts(host);

	/* 3: For designer */
	msdc_debug_reg(host);

	/* 4: check the register read_write */
	temp = MSDC_READ32(base + 0xB0);
	MSDC_LOG(DUMP, "patch reg[%x] = 0x%.8x", (base + 0xB0), temp);

	temp = (~temp);
	MSDC_WRITE32(base + 0xB0, temp);
	temp = MSDC_READ32(base + 0xB0);
	MSDC_LOG(DUMP, "patch reg[%x] = 0x%.8x second time", (base + 0xB0), temp);

	temp = (~temp);
	MSDC_WRITE32(base + 0xB0, temp);
	temp = MSDC_READ32(base + 0xB0);
	MSDC_LOG(DUMP, "patch reg[%x] = 0x%.8x Third time", (base + 0xB0), temp);
#endif
}

int msdc_check_dram_bus_status(struct msdc_host *host)
{
	int i = 10;

	while (i > 0) {
		/* Check bit7 and bit11 of below two registers. */
		MSDC_LOG(ERR, "Reg[0xFD042344] = 0x%08x, Reg[0xFD042544] = 0x%08x",
			 MSDC_READ32(0xFD042344), MSDC_READ32(0xFD042544));
		i--;
	}

	return 0;
}

#if 0
static int msdc_clk_stable(struct msdc_host *host, u32 mode, u32 div)
{
	u32 base = host->base;
	int retry = 0;
	int cnt = 1000;
	int retry_cnt = 1;

	do {
		retry = 3;
		MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD | MSDC_CFG_CKDIV,
			       (mode << 8) | ((div + retry_cnt) % 0xff));
		/* MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, mode); */
		MSDC_RETRY(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB), retry, cnt, host->id);

		if (retry == 0) {
			MSDC_LOG(ERR, "msdc%d on clock failed ===> retry twice", host->id);
			msdc_dump_info(host->id);
		}

		retry = 3;
		MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, div);
		MSDC_RETRY(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB), retry, cnt, host->id);

		if (retry == 0) {
			msdc_dump_info(host->id);
		}

		MSDC_RESET_HW(host->id);

		if (retry_cnt == 2) {
			break;
		}

		retry_cnt += 1;
	} while (!retry);

	return 0;
}

#else

static int msdc_clk_stable(struct msdc_host *host, u32 mode, u32 div)
{
	u32 base = host->base;
	int retry = 5;
	int cnt = 1000;

	MSDC_RESET_HW(host->id);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, mode);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, div);
	MSDC_RETRY(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB), retry, cnt, host->id);

	if (retry == 0) {
		msdc_dump_info(host->id);
		return 1; /* clock is still not stable */
	}

	return 0;
}

#endif

void msdc_set_sr(struct msdc_host *host, int clk, int cmd, int dat)
{
	return;			/* Max Xia, Prepare to remove this function */
}

void msdc_set_rdtdsel_dbg(struct msdc_host *host, bool rdsel, u32 value)
{
	return;			/* Max Xia, Prepare to remove this function */
}

void msdc_set_rdtdsel(struct msdc_host *host, bool sd_18)
{
	return;			/* Max Xia, Prepare to remove this function */
}

/* For change driving strength after switch to 1.8V */
void msdc_set_driving(struct msdc_host *host, struct msdc_hw *hw, bool sd_18)
{
	msdc_pad_init(host, 0);
}

static void msdc_sd_power_switch(struct msdc_host *host, u32 on)
{
	#if ATC_GPIO_KS
	struct gpio_desc * msdc_gpio_desc;
	#endif
	
	switch (host->id) {
	case 1:
		/* MSDC_SET_FIELD(MSDC_SW_GPIO_OUTPUT_VALUE, SD_V33_18_SW1_VALUE, on); */
		#if ATC_GPIO_KS
		msdc_gpio_desc = gpio_to_desc(host->vol_sw_gpio);
		gpiod_direction_output(msdc_gpio_desc, on);
		#else
		gpio_request(PIN_0_GPIO0, "MSDC1_SW");	/* Register GPIO for 33V_18V Switch */
		gpio_direction_output(PIN_0_GPIO0, on);
		#endif

		msdc_set_rdtdsel(host, 1);
		msdc_set_driving(host, host->hw, 1);
		break;

	case 2:
		/* MSDC_SET_FIELD(MSDC_SW_GPIO_OUTPUT_VALUE, SD_V33_18_SW2_VALUE, on); */
		#if ATC_GPIO_KS
		msdc_gpio_desc = gpio_to_desc(host->vol_sw_gpio);
		gpiod_direction_output(msdc_gpio_desc, on);
		#else
		gpio_request(PIN_116_SD_V33_18_SW2, "MSDC2_SW");	/* Register GPIO for 33V_18V Switch */
		#endif

		msdc_set_rdtdsel(host, 1);
		msdc_set_driving(host, host->hw, 1);
		break;

	default:
		break;
	}
}

static void msdc_set_timeout(struct msdc_host *host, u32 ns, u32 clks)
{
	u32 base = host->base;
	u32 timeout, clk_ns;

	host->timeout_ns = ns;
	host->timeout_clks = clks;

	clk_ns = 1000000000UL / host->sclk;
	timeout = ns / clk_ns + clks;
	timeout = timeout >> 20;	/* in 1048576 sclk cycle unit (83/85) */
	timeout = timeout > 1 ? timeout - 1 : 0;
	timeout = timeout > 255 ? 255 : timeout;

	MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, timeout);

	MSDC_LOG(OPS, "Set read data timeout: %dns %dclks -> %d x 1048576  cycles", ns, clks,
		 timeout + 1);
}

/* msdc_eirq_sdio() will be called when EIRQ(for WIFI) */
static void msdc_eirq_sdio(void *data)
{
	struct msdc_host *host = (struct msdc_host *)data;

	MSDC_LOG(INT, "SDIO EINT");

	mmc_signal_sdio_irq(host->mmc);
}

/* msdc_eirq_cd is used!  We are using EINT for card detection. */
static irqreturn_t msdc_eirq_cd(int irq, void *data)
{
	struct msdc_host *host = (struct msdc_host *)data;

	/* MSDC_LOG(ERR, "CD EINT"); */
	host->init_retry_times = 0;

	tasklet_hi_schedule(&host->card_tasklet);
	ac83xx_mask_ack_bim_irq(irq);
	return IRQ_HANDLED;
}

#ifdef MSDC_POWER_FAIL_WP
static irqreturn_t msdc_eirq_emmc_wp(int irq, void *data)
{
    struct msdc_host *host = (struct msdc_host *)data;

    MSDC_LOG(ERR, "EMMC WP EINT");

    tasklet_hi_schedule(&host->emmc_protected);

	ac83xx_mask_ack_bim_irq(irq);
    return IRQ_HANDLED;
}
#endif

static int card_detect_gpio(struct msdc_host *host)
{
	int ret;

	#if ATC_GPIO_KS
	struct gpio_desc * msdc_gpio_desc;
	#endif

	/* simulate sd insert */
	ret = sd_simulate_detect_gpio(host);
	if(ret >= 0)
		return ret;

	#if ATC_GPIO_KS
	msdc_gpio_desc = gpio_to_desc(host->cd_gpio);
	return !(gpiod_get_value(msdc_gpio_desc));
	#else
	return !(gpio_get_value(host->cd_gpio));
	#endif
}

/* detect cd interrupt */
static void msdc_tasklet_card(unsigned long arg)
{
	struct msdc_host *host = (struct msdc_host *)arg;
	struct msdc_hw *hw = host->hw;
	u32 inserted;
	unsigned long irq_flags;

	/* MSDC_LOG(ERR, "----------->>> msdc_tasklet_card <<<------------"); */

	/* un-removable card, return directly, such as eMMC */
	if (!(hw->flags & MSDC_REMOVABLE)) {
		host->card_inserted = 1;
		MSDC_LOG(ERR, "-------->>>un_removable card <<<--------");
		return;
	}

	if (hw->cd_level) {
		inserted = (host->sd_cd_polarity == 0) ? 1 : 0;
	} else {
		inserted = (host->sd_cd_polarity == 0) ? 0 : 1;
	}

	/* Get CD status from GPIO value */
	inserted = card_detect_gpio(host);
	MSDC_LOG_NORMAL(pr_info, "[%d]: Card <%s>, Max_Clock <%dMHz>", host->id, inserted ? "inserted" : "removed", hw->clk_max);

	host->card_inserted = inserted;

	/* Max Xia, Reset clock and tuning parameters when found a new card was inserted. TODO. */
	if (inserted) {
		host->mmc->f_max = hw->clk_max * MSDC_CLK_1MHZ;
	}

	if ((hw->flags & MSDC_CD_PIN_EN) && inserted) {
		host->power_cycle = 0;
		host->power_cycle_enable = 1;
	}
	/*reset sd_version_retry*/
	host->sd_version_retry=0;

	/* [Fix me] if card remove during a request */
	/* spin_unlock_irqrestore(&host->lock, flags); */
	if (host->suspend)
		MSDC_LOG(ERR, "ERR: card detect in suspend state, host->suspend(%d)",
			 host->suspend);

	if ((host->suspend == 0) && host->sd_cd_insert_work) {
		spin_lock_irqsave(&host->detect_queue_lock, irq_flags);

		if (host->queue_len == 0) {
			host->queue_len = 1;
			spin_unlock_irqrestore(&host->detect_queue_lock, irq_flags);
			mmc_detect_change(host->mmc, msecs_to_jiffies(200));
		} else {
			spin_unlock_irqrestore(&host->detect_queue_lock, irq_flags);
		}
	}

	/* MSDC_LOG(ERR, "insert_tasklet(%d)", host->sd_cd_insert_work); */
}

static void msdc_apply_ett_settings(struct msdc_host *host)
{
	unsigned int i = 0;
	u32 base = host->base;
	struct msdc_ett_settings *ett = NULL, *ett_item = NULL;
	unsigned int ett_count = 0;
	if(host->hw->host_function == MSDC_SDIO){
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, host->hw->cmdrtactr_sdr104);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, host->hw->wdatcrctactr_sdr104);
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,host->hw->intdatlatcksel_sdr104);
	}
#ifdef MTK_EMMC_ETT_TO_DRIVER
	if((host->hw->host_function == MSDC_EMMC)){
		switch (emmc_id) {
#ifdef MSDC_SUPPORT_SANDISK_COMBO_ETT
		case SANDISK_EMMC_CHIP:
			MSDC_LOG_NORMAL(pr_info, "[%d]apply sandisk emmc ett settings", host->id);
			ett = msdc0_ett_hs200_settings_for_sandisk;
			ett_count=sizeof(msdc0_ett_hs200_settings_for_sandisk)/sizeof(struct msdc_ett_settings);
			MSDC_LOG_NORMAL(pr_info, "[%d]hs200 ett, ett_count=%d", host->id, ett_count);
			break;
#endif
		default:
			MSDC_LOG_NORMAL(pr_info, "[%d]apply default emmc ett settings", host->id);
			break;
		}
 
		for (i = 0; i < ett_count; i++) {
			ett_item = (struct msdc_ett_settings *)(ett + i);
			MSDC_SET_FIELD((base + ett_item->reg_addr),
					ett_item->reg_offset, ett_item->value);
			MSDC_LOG_NORMAL(pr_info, "[%d]reg[0x%x],offset[0x%x],val[0x%x],readback[0x%x]", host->id, ett_item->reg_addr,
				ett_item->reg_offset, ett_item->value, MSDC_READ32(base + ett_item->reg_addr));
		}
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, host->hw->cmd_rsp_ta_cntr);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, host->hw->wrdat_crcs_ta_cntr);
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, host->hw->read_dat_latch_ck_sel);
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, host->hw->read_ckgen_delay_sel);

		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, host->hw->cmd_edge);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, host->hw->read_sample_edge);
		MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, host->hw->read_pad_delay);		
		MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, host->hw->write_internal_delay);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, host->hw->write_sample_edge);
	}
#endif
}

#ifdef MSDC_POWER_FAIL_WP
static void msdc_emmc_protect(unsigned long arg)
{
    struct msdc_host *host = (struct msdc_host *)arg;
    u32 protected;	

	// Get status from GPIO value
    //protected = card_detect_gpio(host);
    protected =gpio_get_value(host->protect_gpio);
	host->protected = protected;
	MSDC_LOG(IRQ,"gpio_value:%d,host->protect_init:%d,host->protect_gpio:%d",
		protected,host->protect_init,host->protect_gpio);
	MSDC_LOG(IRQ, "emmc protected <%s>", protected == host->protect_init? "enable" : "disable");  
}
#endif

void msdc_set_mclk(struct msdc_host *host, int ddr, u32 hz)
{
	u32 base = host->base;
	u32 mode;
	u32 flags;
	u32 div;
	u32 sclk;
	u32 hclk = host->hclk;

	if (!hz) {		/* set mmc system clock to 0 */
		/* MSDC_LOG(ERR, "!!!set mclk to 0");  // fix me: need to set to 0 */
		if (host->hw->flags & MSDC_SDIO_IRQ) {
			host->saved_para.hz = hz;
		}

		host->mclk = 0;
		/* MSDC_RESET_HW(host->id); */
		return;
	}

	/* Special Setting for HS200 */
	 if((host->hw->host_function == MSDC_EMMC)&&(hz > MSDC_CLK_100MHZ) && (hz <= MSDC_CLK_200MHZ))
	{
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, 2);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, 2);
	}

	msdc_irq_save(flags);

	if (ddr) {		/* may need to modify later */
		mode = 0x2;	/* ddr mode and use divisor */

		if (hz >= (hclk >> 2)) {
			div = 0;	/* mean div = 1/4 */
			sclk = hclk >> 2;	/* sclk = clk / 4 */
		} else {
			div = (hclk + ((hz << 2) - 1)) / (hz << 2);
			sclk = (hclk >> 2) / div;
			div = (div >> 1);
		}
	} else if (hz >= hclk) {
		mode = 0x1;	/* no divisor */
		div = 0;
		sclk = hclk;
	} else {
		mode = 0x0;	/* use divisor */

		if (hz >= (hclk >> 1)) {
			div = 0;	/* mean div = 1/2 */
			sclk = hclk >> 1;	/* sclk = clk / 2 */
		} else {
			div = (hclk + ((hz << 2) - 1)) / (hz << 2);
			sclk = (hclk >> 2) / div;
		}
	}

	/* Change clock and wait it stable */
	msdc_clk_stable(host, mode, div);

	host->sclk = sclk;
	host->mclk = hz;
	host->ddr = ddr;

	msdc_set_timeout(host, host->timeout_ns, host->timeout_clks);	/* need because clk changed. */

	MSDC_LOG_NORMAL(pr_info, "[%d]%s: Set<%dKHz> Source<%dKHz> -> sclk<%dKHz> DDR<%d> mode<%d> div<%d>", host->id, __FUNCTION__,
		 hz / 1000, hclk / 1000, sclk / 1000, ddr, mode, div);
	
	if((host->hw->host_function == MSDC_SDIO)&&(host->sclk >MSDC_CLK_100MHZ)){
		msdc_apply_ett_settings(host);
   	}
	msdc_irq_restore(flags);
}


/* 0 means pass */
static u32 msdc_power_tuning(struct msdc_host *host)
{
	struct mmc_host *mmc = host->mmc;
	struct mmc_card *card;
	struct mmc_request *mrq;
	u32 power_cycle = 0;
	int read_timeout_tune = 0;
	int write_timeout_tune = 0;
	u32 rwcmd_timeout_tune = 0;
	u32 read_timeout_tune_uhs104 = 0;
	u32 write_timeout_tune_uhs104 = 0;
	u32 sw_timeout = 0;
	u32 ret = 1;
	u32 host_err = 0;
	u32 base = host->base;

	if (!mmc) {
		return 1;
	}

	card = mmc->card;

	if (card == NULL) {
		MSDC_LOG(ERR, "mmc->card is NULL");
		return 1;
	}

#if 0//eMMC first

	if (mmc_card_mmc(card) && (host->hw->host_function == MSDC_EMMC)) {
		/* Fixme: */
		return 1;
	}

#endif

	if ((host->sd_30_busy > 0) && (host->sd_30_busy <= MSDC_MAX_POWER_CYCLE)) {
		host->power_cycle_enable = 1;
	}

	if (mmc_card_sd(card) && (host->hw->host_function == MSDC_SD) &&
	    (host->power_cycle < MSDC_MAX_POWER_CYCLE) && (host->power_cycle_enable)) {
		/* power cycle */
		MSDC_LOG_NORMAL(pr_info, "[%d]Power cycle start", host->id);
		spin_unlock(&host->lock);

		mdelay(10);

		spin_lock(&host->lock);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, host->hw->ddlsel);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, host->hw->cmd_edge);	/* save the para */
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, host->hw->rdata_edge);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, host->hw->wdata_edge);
		host->saved_para.pad_tune = MSDC_READ32(MSDC_PAD_TUNE);
		host->saved_para.ddly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
		host->saved_para.ddly1 = MSDC_READ32(MSDC_DAT_RDDLY1);
		MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP,
			       host->saved_para.cmd_resp_ta_cntr);
		MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS,
			       host->saved_para.wrdat_crc_ta_cntr);

		if ((host->sclk > MSDC_CLK_100MHZ) && (host->power_cycle >= 1)) {
			mmc->caps &= ~MMC_CAP_UHS_SDR104;
		}

		if (((host->sclk <= MSDC_CLK_100MHZ) && ((host->sclk > MSDC_CLK_50MHZ) || (host->ddr))) &&
			(host->power_cycle >= 1)) {
			mmc->caps &= ~(MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104 | MMC_CAP_UHS_DDR50);
		}

		msdc_host_mode[host->id] = mmc->caps;
		msdc_host_mode2[host->id] = mmc->caps2;

		/* clock should set to 260K */
		mmc->ios.clock = HOST_MIN_MCLK;
		mmc->ios.bus_width = MMC_BUS_WIDTH_1;
		mmc->ios.timing = MMC_TIMING_LEGACY;
		msdc_set_mclk(host, 0, HOST_MIN_MCLK);

		/* re-init the card! */
		mrq = host->mrq;
		host->mrq = NULL;
		power_cycle = host->power_cycle;
		host->power_cycle = MSDC_MAX_POWER_CYCLE;
		read_timeout_tune = host->read_time_tune;
		write_timeout_tune = host->write_time_tune;
		rwcmd_timeout_tune = host->rwcmd_time_tune;
		read_timeout_tune_uhs104 = host->read_timeout_uhs104;
		write_timeout_tune_uhs104 = host->write_timeout_uhs104;
		sw_timeout = host->sw_timeout;
		host_err = host->error;
		spin_unlock(&host->lock);
		ret = sd_reinit(mmc, mmc->card->ocr, card);
		spin_lock(&host->lock);
		host->mrq = mrq;
		host->power_cycle = power_cycle;
		host->read_time_tune = read_timeout_tune;
		host->write_time_tune = write_timeout_tune;
		host->rwcmd_time_tune = rwcmd_timeout_tune;

		if (host->sclk > MSDC_CLK_100MHZ) {
			host->write_timeout_uhs104 = write_timeout_tune_uhs104;
		} else {
			host->read_timeout_uhs104 = 0;
			host->write_timeout_uhs104 = 0;
		}

		host->sw_timeout = sw_timeout;
		host->error = host_err;

		if (!ret) {
			host->power_cycle_enable = 0;
		}

		MSDC_LOG_NORMAL(pr_info, "[%d]Power cycle host->error(0x%x)", host->id, host->error);
		(host->power_cycle)++;
		MSDC_LOG_NORMAL(pr_info, "[%d]Power cycle Done", host->id);

		/* cxl: because in power cycle, tune status will be clear, which will cause logical error in msdc_irq(), so we need to set it again */
		SET_TUNE_STS(host, TUNE_STS_IN_TUNE);

	}

	return ret;
}


static void msdc_send_stop(struct msdc_host *host)
{
	struct mmc_command stop = { 0 };
	struct mmc_request mrq = { 0 };
	u32 err = -1;

	stop.opcode = MMC_STOP_TRANSMISSION;
	stop.arg = 0;
	stop.flags = MMC_RSP_R1B | MMC_CMD_AC;

	mrq.cmd = &stop;
	stop.mrq = &mrq;
	stop.data = NULL;

	err = msdc_do_command(host, &stop, 0, CMD_TIMEOUT);
}

static void dump_cmd_tuning_success_params(struct msdc_host *host)
{
	u32 base = host->base;
	u32 rsmpl, rrdly, cmdrtc, dl_cksel;
	u32 ck_sel, ckgen, internal_delay, pad_delay, sample_edge;

	if (host->hw->tuning_method == TUNE_EACH_DAT_LINE) {
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, rsmpl);
		MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, rrdly);
		MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, cmdrtc);
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, dl_cksel);
		MSDC_LOG_NORMAL(pr_info,
			 "[%d]%s:TUNE_CMD: rsmpl<%u> rrdly<%u> cmdrtc<%u> dl_cksel<%u> sfreq<%u> method<%u>", host->id, __FUNCTION__,
			 rsmpl, rrdly, cmdrtc, dl_cksel, host->sclk, host->hw->tuning_method);
	} else {
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);
		MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, internal_delay);
		MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, pad_delay);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, sample_edge);
		MSDC_LOG_NORMAL(pr_info,
			 "[%d]%s:TUNE_CMD: ck_sel<%u> ckgen<%u> internal_delay<%u> pad_delay<%u> sample_edge<%u> sfreq<%u> method<%u>", host->id,  __FUNCTION__,
			 ck_sel, ckgen, internal_delay, pad_delay, sample_edge, host->sclk,
			 host->hw->tuning_method);
	}
}


static void dump_read_tuning_success_params(struct msdc_host *host)
{
	u32 base = host->base;
	u32 dsel, dl_cksel, dsmpl, rxdly0, rxdly1;
	u32 ck_sel, ckgen, pad_delay, sample_edge;

	if (host->hw->tuning_method == TUNE_EACH_DAT_LINE) {
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, dsel);
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, dl_cksel);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, dsmpl);
		rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
		rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);
		MSDC_LOG_NORMAL(pr_info,
			 "[%d]%s:TUNE_READ: dsmpl<%u> rxdly0<0x%x> rxdly1<0x%x> dsel<%u> dl_cksel<%u> sfreq<%u> method<%u>", host->id, __FUNCTION__,
			 dsmpl, rxdly0, rxdly1, dsel, dl_cksel, host->sclk,
			 host->hw->tuning_method);
	} else {
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);
		MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, pad_delay);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, sample_edge);
		MSDC_LOG_NORMAL(pr_info,
			 "[%d]%s:TUNE_READ: ck_sel<%u> ckgen<%u> pad_delay<%u> sample_edge<%u> sfreq<%u> method<%u>", host->id, __FUNCTION__,
			 ck_sel, ckgen, pad_delay, sample_edge, host->sclk,
			 host->hw->tuning_method);
	}
}


static void dump_write_tuning_success_params(struct msdc_host *host)
{
	u32 base = host->base;
	u32 dsmpl, d_cntr, rxdly0;
	u32 ck_sel, ckgen, pad_delay, internal_delay, sample_edge;

	if (host->hw->tuning_method == TUNE_EACH_DAT_LINE) {
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, dsmpl);
		MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, d_cntr);
		rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
		MSDC_LOG_NORMAL(pr_info,
			 "[%d]%s:TUNE_WRITE: dsmpl<%u> rxdly0<0x%x> d_cntr<%u> sfreq<%u> method<%u>", host->id, __FUNCTION__,
			 dsmpl, rxdly0, d_cntr, host->sclk, host->hw->tuning_method);
	} else {
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);
		/* only tune data0 */
		MSDC_GET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0, pad_delay);
		MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, internal_delay);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, sample_edge);
		MSDC_LOG_NORMAL(pr_info,
			 "[%d]%s:TUNE_WRITE: ck_sel<%u> ckgen<%u> pad_delay<%u> internal_delay<%u> sample_edge<%u> sfreq<%u> method<%u>", host->id, __FUNCTION__,
			 ck_sel, ckgen, pad_delay, internal_delay, sample_edge, host->sclk,
			 host->hw->tuning_method);
	}

}

static void msdc_reset_tune_counter(struct msdc_host *host, TUNE_COUNTER index)
{
	if ((index >= 0) && (index <= all_counter)) {
		switch (index) {
		case cmd_counter:
			if (host->t_counter.time_cmd != 0) {
				dump_cmd_tuning_success_params(host);
				MSDC_LOG_NORMAL(pr_info,  "[%d]TUNE CMD Times(%d)", host->id, host->t_counter.time_cmd);
				host->t_counter.time_cmd = 0;
			}

			break;

		case read_counter:
			if (host->t_counter.time_read != 0) {
				dump_read_tuning_success_params(host);
				MSDC_LOG_NORMAL(pr_info, "[%d]TUNE READ Times(%d)", host->id, host->t_counter.time_read);
				host->t_counter.time_read = 0;
			}

			break;

		case write_counter:
			if (host->t_counter.time_write != 0) {
				dump_write_tuning_success_params(host);
				MSDC_LOG_NORMAL(pr_info, "[%d]TUNE WRITE Times(%d)", host->id, host->t_counter.time_write);
				host->t_counter.time_write = 0;
			}

			break;

		case all_counter:
			if (host->t_counter.time_cmd != 0) {
				dump_cmd_tuning_success_params(host);
				MSDC_LOG_NORMAL(pr_info, "[%d]TUNE CMD Times(%d)", host->id, host->t_counter.time_cmd);
			}

			if (host->t_counter.time_read != 0) {
				dump_read_tuning_success_params(host);
				host->last_dat_err_cmd_opcode = 0;
				host->last_dat_err_cmd_arg = 0;
				host->last_dat_err_intr = 0;
				MSDC_LOG_NORMAL(pr_info, "[%d]TUNE READ Times(%d)", host->id, host->t_counter.time_read);
			}

			if (host->t_counter.time_write != 0) {
				dump_write_tuning_success_params(host);
				host->last_dat_err_cmd_opcode = 0;
				host->last_dat_err_cmd_arg = 0;
				host->last_dat_err_intr = 0;
				MSDC_LOG_NORMAL(pr_info, "[%d]TUNE WRITE Times(%d)", host->id, host->t_counter.time_write);
			}

			host->t_counter.time_cmd = 0;
			host->t_counter.time_read = 0;
			host->t_counter.time_write = 0;
			break;

		default:
			break;
		}
	} else {
		MSDC_LOG(ERR, "msdc%d ==> reset counter index(%d) error!", host->id, index);
	}
}


/* Fix me. when need to abort */
static u32 msdc_abort_data(struct msdc_host *host)
{
	struct mmc_host *mmc = host->mmc;
	u32 base = host->base;
	u32 status = 0;
	u32 state = 0;
	u32 err = 0;
	unsigned long tmo = jiffies + POLLING_BUSY;
	u32 try_cnt = 0;

	/* SDIO card does not support CMD13 */
	if (host && host->mmc && host->mmc->card && mmc_card_sdio(host->mmc->card)) {
		MSDC_RESET_HW(host->id);
		return 0;
	}

	while ((state != 4) && (host->card_inserted)) {	/* until status to "tran" */
		MSDC_RESET_HW(host->id);

		while ((err = msdc_get_card_status(mmc, host, &status))) {
			MSDC_LOG(ERR, "CMD13 ERR<%d>", err);

			if (err != CONV2UINT(-EIO)) {
				try_cnt++;

				if (try_cnt > 1) {
					if (msdc_lower_onlyfreq(host)) {
						return msdc_power_tuning(host);
					}
				}
			} else if (msdc_tune_cmdrsp(host)) {
				MSDC_LOG(ERR, "update cmd para failed");
				return 1;
			}
			if(!host->card_inserted)
				goto out;
		}

		state = R1_CURRENT_STATE(status);

		/* MSDC_LOG(ERR, "check card state<%d>", state); */
		if ((state == 5) || (state == 6)) {
			MSDC_LOG(ERR, "state<%d> need cmd12 to stop", state);
			msdc_send_stop(host);	/* don't tuning */
		} else if (state == 7) {	/* busy in programing */
			MSDC_LOG(ERR, "state<%d> card is busy in programing status", state);
			spin_unlock(&host->lock);
			msleep(100);
			spin_lock(&host->lock);
		} else if (state != 4) {
			MSDC_LOG(ERR, "state<%d> ??? ", state);
			return msdc_power_tuning(host);
		}

		if (time_after(jiffies, tmo)) {
			MSDC_LOG(ERR, "abort timeout. Do power cycle");

			if ((host->hw->host_function == MSDC_SD)
			    && ((host->sclk >= MSDC_CLK_100MHZ) || host->ddr)) {
				host->sd_30_busy++;
			}

			return msdc_power_tuning(host);
		}
	}

out:
	MSDC_RESET_HW(host->id);
	return 0;
}

static u32 msdc_polling_idle(struct msdc_host *host)
{
	struct mmc_host *mmc = host->mmc;
	u32 status = 0;
	u32 state = 0;
	u32 err = 0;
	unsigned long tmo = jiffies + POLLING_BUSY;

	while (state != 4) {	/* until status to "tran" */
		while ((err = msdc_get_card_status(mmc, host, &status))) {
			MSDC_LOG(ERR, "CMD13 ERR<%d>", err);

			if (err != CONV2UINT(-EIO)) {
				return msdc_power_tuning(host);
			} else if (msdc_tune_cmdrsp(host)) {
				MSDC_LOG(ERR, "update cmd para failed");
				return 1;
			}
		}

		state = R1_CURRENT_STATE(status);

		/* MSDC_LOG(ERR, "check card state<%d>", state); */
		if ((state == 5) || (state == 6)) {
			MSDC_LOG(ERR, "state<%d> need cmd12 to stop", state);
			msdc_send_stop(host);	/* don't tuning */
		} else if (state == 7) {	/* busy in programing */
			MSDC_LOG(ERR, "state<%d> card is busy", state);
			spin_unlock(&host->lock);
			msleep(100);
			spin_lock(&host->lock);
		} else if (state != 4) {
			MSDC_LOG(ERR, "state<%d> ??? ", state);
			return msdc_power_tuning(host);
		}

		if (time_after(jiffies, tmo)) {
			MSDC_LOG(ERR, "abort timeout. Do power cycle");
			return msdc_power_tuning(host);
		}
	}

	return 0;
}


/*
 *   Save/Restore msdc tuning parameters for Suspend/Resume
 */
static void msdc_save_setting(struct mmc_host *mmc)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 base = host->base;

	MSDC_LOG_NORMAL(pr_info, "[%d]Save msdc Setting @mclk=%d\n", host->id, host->mclk/1000);

	host->saved_para.ddr = host->ddr;
	host->saved_para.hz = host->mclk;
	host->saved_para.sdc_cfg = MSDC_READ32(SDC_CFG);
	host->saved_para.iocon = MSDC_READ32(MSDC_IOCON);
	host->saved_para.pad_tune = MSDC_READ32(MSDC_PAD_TUNE);
	host->saved_para.ddly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
	host->saved_para.ddly1 = MSDC_READ32(MSDC_DAT_RDDLY1);
	host->saved_para.patch_bit0 = MSDC_READ32(MSDC_PATCH_BIT0);
	host->saved_para.patch_bit1 = MSDC_READ32(MSDC_PATCH_BIT1);
}

static void msdc_restore_setting(struct mmc_host *mmc, int force)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 base = host->base;

	if(force || ((host->mclk == host->saved_para.hz) && (host->ddr == host->saved_para.ddr))) {
		MSDC_LOG_NORMAL(pr_info, "[%d]Restore msdc Setting(%s)", host->id, force?"force":"not force");

		if(force)
			msdc_set_mclk(host, host->saved_para.ddr, host->saved_para.hz);

		MSDC_WRITE32(SDC_CFG, host->saved_para.sdc_cfg);
		MSDC_WRITE32(MSDC_IOCON, host->saved_para.iocon);
		MSDC_WRITE32(MSDC_PAD_TUNE, host->saved_para.pad_tune);
		MSDC_WRITE32(MSDC_DAT_RDDLY0, host->saved_para.ddly0);
		MSDC_WRITE32(MSDC_DAT_RDDLY1, host->saved_para.ddly1);
		MSDC_WRITE32(MSDC_PATCH_BIT0, host->saved_para.patch_bit0);
		MSDC_WRITE32(MSDC_PATCH_BIT1, host->saved_para.patch_bit1);
	}
}

static u32 msdc_pm(pm_message_t state, void *data)
{
	u32 ret = 0;
	struct msdc_host *host = (struct msdc_host *)data;

	int evt = state.event;
	u32 base = host->base;

	/* =========================== Power Suspend Flow ========================== */
	if ((evt == PM_EVENT_SUSPEND) || (evt == PM_EVENT_USER_SUSPEND)) {
		if (host->suspend)
			/* already suspend */
			/* default 0 */
		{
			return ret;
		}

		/* for memory card. power_mode will be set to OFF in core layer suspend */
		if ((evt == PM_EVENT_SUSPEND) && (host->power_mode != MMC_POWER_OFF)) {
			return ret;
		}

		host->suspend = 1;
		host->pm_state = state;	/* default PMSG_RESUME */

		/* MSDC_LOG(ERR, "%s Suspend", evt == PM_EVENT_SUSPEND ? "PM" : "USR"); */

		if (host->hw->flags & MSDC_SYS_SUSPEND) {	/* set for card */

			if ((host->hw->host_function == MSDC_EMMC) && host->mmc->card
			    && mmc_card_mmc(host->mmc->card))
#if MSDC_EMMC_POWER_OFF_VCC
				/* Notify core layer, we shut down both VCCQ and VCC */
				host->mmc->pm_flags &= (~MMC_PM_KEEP_POWER);

#else
				host->mmc->pm_flags |= MMC_PM_KEEP_POWER;
#endif
			host->mmc->pm_flags |= MMC_PM_IGNORE_PM_NOTIFY;
			MSDC_LOG_NORMAL(pr_info, "[%d]Suspend pm_flags = 0x%08X", host->id, host->mmc->pm_flags);

			if ((host->hw->host_function == MSDC_EMMC) && host->mmc->card
			    && mmc_card_mmc(host->mmc->card)) {
				if (host->power_control) {
					host->power_control(host, 0);
				}
			}

			if(host->hw->host_function == MSDC_SDIO) {
				sdio_resume_back = true;
			}

		} else {
			host->mmc->pm_flags |= MMC_PM_IGNORE_PM_NOTIFY;	/* just for double confirm */
			/* MSDC_LOG(ERR, "Invoke mmc_remove_host()"); */
			mmc_remove_host(host->mmc);
		}
	}
	/* =========================== Power Resume Flow ========================== */
	else if ((evt == PM_EVENT_RESUME) || (evt == PM_EVENT_USER_RESUME)) {
		if (!host->suspend) {
			/* MSDC_LOG(ERR, "warning: already resume"); */
			return ret;
		}

		/* No PM resume when USR suspend */
		if ((evt == PM_EVENT_RESUME) && (host->pm_state.event == PM_EVENT_USER_SUSPEND)) {
			MSDC_LOG(ERR, "PM Resume when in USR Suspend");	/* won't happen. */
			return ret;
		}

		host->suspend = 0;
		host->pm_state = state;

		/* MSDC_LOG(ERR, "%s Resume", evt == PM_EVENT_RESUME ? "PM" : "USR"); */

		if (host->hw->flags & MSDC_SYS_SUSPEND) {	/* will not set for WIFI */

			if ((host->hw->host_function == MSDC_EMMC) && host->mmc->card
			    && mmc_card_mmc(host->mmc->card)) {
				MSDC_RESET_HW(host->id);

				if (host->power_control) {
					host->power_control(host, 1);
					mdelay(10);
				}
			}

			MSDC_LOG_NORMAL(pr_info, "[%d]Resume pm_flags = 0x%08X", host->id, host->mmc->pm_flags);
		} else {
			host->mmc->pm_flags |= MMC_PM_IGNORE_PM_NOTIFY;
			ret = mmc_add_host(host->mmc);
		}
	}

	return ret;
}

#ifdef ATC_EMMC_SUPPORT
u8 emmc_ext_csd[512];
u8 emmc_partition_access = 0;

static int msdc_get_data(u8 *dst, struct mmc_data *data)
{
	int left;
	u8 *ptr;
	struct scatterlist *sg = data->sg;
	int num = data->sg_len;

	while (num) {
		left = sg_dma_len(sg);
		ptr = (u8 *) sg_virt(sg);
		memcpy(dst, ptr, left);
		sg = sg_next(sg);
		dst += left;
		num--;
	}

	return 0;
}

/* Max Xia, kernel 3.18 */
#if 0
/* Get Capacity from EXT_CSD. All partitions sum.. */
static u32 msdc_get_other_capacity(void)
{
	u32 device_other_capacity = 0;

	device_other_capacity = emmc_ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024
				+ emmc_ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024
				+ emmc_ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024
				+ emmc_ext_csd[EXT_CSD_GP1_SIZE_MULT + 2] * 256 * 256
				+ emmc_ext_csd[EXT_CSD_GP1_SIZE_MULT + 1] * 256
				+ emmc_ext_csd[EXT_CSD_GP1_SIZE_MULT + 0]
				+ emmc_ext_csd[EXT_CSD_GP2_SIZE_MULT + 2] * 256 * 256
				+ emmc_ext_csd[EXT_CSD_GP2_SIZE_MULT + 1] * 256
				+ emmc_ext_csd[EXT_CSD_GP2_SIZE_MULT + 0]
				+ emmc_ext_csd[EXT_CSD_GP3_SIZE_MULT + 2] * 256 * 256
				+ emmc_ext_csd[EXT_CSD_GP3_SIZE_MULT + 1] * 256
				+ emmc_ext_csd[EXT_CSD_GP3_SIZE_MULT + 0]
				+ emmc_ext_csd[EXT_CSD_GP4_SIZE_MULT + 2] * 256 * 256
				+ emmc_ext_csd[EXT_CSD_GP4_SIZE_MULT + 1] * 256
				+ emmc_ext_csd[EXT_CSD_GP4_SIZE_MULT + 0];
	return device_other_capacity;
}
#endif

u32 erase_start = 0;
u32 erase_end = 0;
/* extern int mmc_erase_group_aligned(struct mmc_card *card, unsigned int from, unsigned int nr); */
#endif

/*--------------------------------------------------------------------------*/
/* mmc_host_ops members                                                      */
/*--------------------------------------------------------------------------*/
static u32 wints_cmd = MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR | MSDC_INT_CMDTMO |
		       MSDC_INT_ACMDRDY | MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO;
static unsigned int msdc_command_start(struct msdc_host *host, struct mmc_command *cmd, int tune,	/* not used */
				       unsigned long timeout)
{
	u32 base = host->base;
	u32 opcode = cmd->opcode;
	u32 rawcmd;
	u32 rawarg;
	u32 resp;
	unsigned long tmo;
#ifdef MTK_MSDC_USE_CMD23
	struct mmc_command *sbc = NULL;
	if(host->data && host->data->mrq && host->data->mrq->sbc)
		sbc = host->data->mrq->sbc;
#endif

	/* Protocol layer does not provide response type, but our hardware needs
	 * to know exact type, not just size!
	 */
	if ((opcode == MMC_SEND_OP_COND) || (opcode == SD_APP_OP_COND)) {
		resp = RESP_R3;
	} else if ((opcode == MMC_SET_RELATIVE_ADDR) || (opcode == SD_SEND_RELATIVE_ADDR)) {
		resp = (mmc_cmd_type(cmd) == MMC_CMD_BCR) ? RESP_R6 : RESP_R1;
	} else if (opcode == MMC_FAST_IO) {
		resp = RESP_R4;
	} else if (opcode == MMC_GO_IRQ_STATE) {
		resp = RESP_R5;
	} else if (opcode == MMC_SELECT_CARD) {
		resp = (cmd->arg != 0) ? RESP_R1B : RESP_NONE;
		host->app_cmd_arg = cmd->arg;
		/* MSDC_LOG(WRN, "select card<0x%.8x>", cmd->arg);  // select and de-select */
	} else if ((opcode == SD_IO_RW_DIRECT) || (opcode == SD_IO_RW_EXTENDED)) {
		resp = RESP_R1;        /* SDIO workaround. */
	} else if ((opcode == SD_SEND_IF_COND) && (mmc_cmd_type(cmd) == MMC_CMD_BCR)) {
		resp = RESP_R1;
	} else {
		switch (mmc_resp_type(cmd)) {
		case MMC_RSP_R1:
			resp = RESP_R1;
			break;

		case MMC_RSP_R1B:
			resp = RESP_R1B;
			break;

		case MMC_RSP_R2:
			resp = RESP_R2;
			break;

		case MMC_RSP_R3:
			resp = RESP_R3;
			break;

		case MMC_RSP_NONE:
		default:
			resp = RESP_NONE;
			break;
		}
	}

	cmd->error = 0;
	/* rawcmd :
	 * vol_swt << 30 | auto_cmd << 28 | blklen << 16 | go_irq << 15 |
	 * stop << 14 | rw << 13 | dtype << 11 | rsptyp << 7 | brk << 6 | opcode
	 */
	rawcmd = opcode | msdc_rsp[resp] << 7 | host->blksz << 16;

	if (opcode == MMC_READ_MULTIPLE_BLOCK) {
		if (host->autocmd & MSDC_AUTOCMD12)
			rawcmd |= (1 << 28);
#ifdef MTK_MSDC_USE_CMD23
		else if ((host->autocmd & MSDC_AUTOCMD23)) {
			rawcmd |= (1 << 29);
			if (sbc) {
				/* if the block number is bigger than 0xFFFF,
				 * then CMD23 arg will be failed to set it
				 */
				if (MSDC_READ32(SDC_BLK_NUM) != (sbc->arg & 0xFFFF))
					MSDC_LOG_NORMAL(pr_err, "[%d]acmd23 arg(0x%x) != read blocks(0x%x),SDC_BLK_NUM(0x%x)", host->id, sbc->arg, 
						host->mrq->cmd->data->blocks, MSDC_READ32(SDC_BLK_NUM));
				else
					MSDC_WRITE32(SDC_BLK_NUM, sbc->arg);
			}
		}
#endif
	} else if (opcode == MMC_WRITE_MULTIPLE_BLOCK) {
		if (host->autocmd & MSDC_AUTOCMD12)
			rawcmd |= (1 << 28);
#ifdef MTK_MSDC_USE_CMD23
		else if ((host->autocmd & MSDC_AUTOCMD23)) {
			rawcmd |= (1 << 29);
			if (sbc) {
				if (MSDC_READ32(SDC_BLK_NUM) != (sbc->arg & 0xFFFF))
					MSDC_LOG_NORMAL(pr_err, "[%d]acmd23 arg(0x%x) != write blocks(0x%x),SDC_BLK_NUM(0x%x)", host->id, sbc->arg,
						host->mrq->cmd->data->blocks, MSDC_READ32(SDC_BLK_NUM));
				else
					MSDC_WRITE32(SDC_BLK_NUM, sbc->arg);
			}
		}
#endif
	} else if ((opcode == SD_IO_RW_DIRECT) && cmd->flags == CONV2UINT(-1)) {
		rawcmd |= (1 << 14);
	} else if (opcode == SD_SWITCH_VOLTAGE) {
		rawcmd |= (1 << 30);
	} else if (opcode == MMC_STOP_TRANSMISSION) {
		rawcmd |= (1 << 14);
		rawcmd &= ~(0x0FFF << 16);
	}

	if(cmd->data) {
		if (cmd->data->flags & MMC_DATA_WRITE) {
			rawcmd |= (1 << 13);
		}

		if (cmd->data->blocks > 1) {
			rawcmd |= (2 << 11);
		} else {
			rawcmd |= (1 << 11);
		}
	}

	MSDC_LOG(CMD, "CMD<%d><0x%.8x> Arg<0x%.8x>", opcode, rawcmd, cmd->arg);

	tmo = jiffies + timeout;

	if (opcode == MMC_SEND_STATUS) {
		for (;;) {
			if (!SDC_IS_CMD_BUSY()) {
				break;
			}

			if (time_after(jiffies, tmo)) {
				MSDC_LOG(ERR, "XXX cmd_busy timeout: before CMD<%d>", opcode);
				cmd->error = CONV2UINT(-ETIMEDOUT);
				MSDC_RESET_HW(host->id);
				return cmd->error;	/* Fix me: error handling */
			}
		}
	} else {
		for (;;) {
			if (!SDC_IS_BUSY()) {
				break;
			}

			if (time_after(jiffies, tmo)) {
				MSDC_LOG(ERR, "XXX sdc_busy timeout: before CMD<%d>", opcode);
				cmd->error = CONV2UINT(-ETIMEDOUT);
				MSDC_RESET_HW(host->id);
				return cmd->error;
			}
		}
	}

	/* BUG_ON(in_interrupt()); */
	host->cmd = cmd;
	host->cmd_rsp = resp;

	/* use polling way */
	MSDC_CLR_BITS(MSDC_INTEN, wints_cmd);
	rawarg = cmd->arg;

#ifdef ATC_EMMC_SUPPORT		/* Max Xia, Important!!!! */

	if ((host->hw->host_function == MSDC_EMMC) && (host->hw->boot == MSDC_BOOT_EN)) {
		if (cmd->opcode == MMC_ERASE_GROUP_START) {
			erase_start = rawarg;
		}

		if (cmd->opcode == MMC_ERASE_GROUP_END) {
			erase_end = rawarg;
		}
	}

	if ((cmd->opcode == MMC_ERASE) &&
		((cmd->arg == MMC_SECURE_ERASE_ARG) || (cmd->arg == MMC_ERASE_ARG)) &&
		host->mmc->card && (host->hw->host_function == MSDC_EMMC) &&
		(host->hw->boot == MSDC_BOOT_EN) &&
		(!mmc_erase_group_aligned(host->mmc->card, erase_start, erase_end))) {
		if ((cmd->arg == MMC_SECURE_ERASE_ARG) && mmc_can_secure_erase_trim(host->mmc->card)) {
			rawarg = MMC_SECURE_TRIM1_ARG;
		} else if ((cmd->arg == MMC_ERASE_ARG) ||
		((cmd->arg == MMC_SECURE_ERASE_ARG) && !mmc_can_secure_erase_trim(host->mmc->card))) {
			rawarg = MMC_TRIM_ARG;
		}
	}

#endif
	SDC_SEND_CMD(rawcmd, rawarg);

	/* irq too fast, then cmd->error has value, and don't call msdc_command_resp, don't tune. */
	return 0;
}

static unsigned int msdc_command_resp_polling(struct msdc_host *host,
					      struct mmc_command *cmd,
					      int tune, unsigned long timeout)
{
	u32 base = host->base;
	u32 intsts;
	u32 resp;
	unsigned long tmo;

	u32 cmdsts = MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR | MSDC_INT_CMDTMO;
#ifdef MTK_MSDC_USE_CMD23
		struct mmc_command *sbc = NULL;

		if (host->autocmd & MSDC_AUTOCMD23) {
			if (host->data && host->data->mrq && host->data->mrq->sbc)
				sbc = host->data->mrq->sbc;
	
			/* autocmd interrupt disabled, used polling way */
			cmdsts |= MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO;
		}
#endif
	resp = host->cmd_rsp;

	/*polling */
	tmo = jiffies + timeout;

	while (1) {
		if (((intsts = MSDC_READ32(MSDC_INT)) & cmdsts) != 0){
			/* clear all int flag */
#ifdef MTK_MSDC_USE_CMD23
			/* need clear autocmd23 command ready interrupt */
			intsts &= (cmdsts | MSDC_INT_ACMDRDY);
#else
			intsts &= cmdsts;
#endif
			MSDC_WRITE32(MSDC_INT, intsts);
			break;
		}

		if (time_after(jiffies, tmo)) {
			MSDC_LOG(ERR, "CMD<%d> polling_for_completion timeout ARG<0x%.8x>", cmd->opcode, cmd->arg);
			cmd->error = CONV2UINT(-ETIMEDOUT);
			host->sw_timeout++;
			msdc_dump_info(host->id);
			MSDC_RESET_HW(host->id);
			goto out;
		}
	}

	/* command interrupts */
	if (intsts & cmdsts) {
#ifdef MTK_MSDC_USE_CMD23
		if ((intsts & MSDC_INT_CMDRDY) || (intsts & MSDC_INT_ACMD19_DONE)) {
#else
		if ((intsts & MSDC_INT_CMDRDY) || (intsts & MSDC_INT_ACMDRDY)
			|| (intsts & MSDC_INT_ACMD19_DONE)) {
#endif
			u32 *rsp = NULL;

			rsp = &cmd->resp[0];

			switch (host->cmd_rsp) {
			case RESP_NONE:
				break;

			case RESP_R2:
				*rsp = MSDC_READ32(SDC_RESP3);
				rsp++;
				*rsp = MSDC_READ32(SDC_RESP2);
				rsp++;
				*rsp = MSDC_READ32(SDC_RESP1);
				rsp++;
				*rsp = MSDC_READ32(SDC_RESP0);
				rsp++;
				break;

			default:	/* Response types 1, 3, 4, 5, 6, 7(1b) */
				*rsp = MSDC_READ32(SDC_RESP0);
				break;
			}
		} else if (intsts & MSDC_INT_RSPCRCERR) {
			cmd->error = CONV2UINT(-EIO);
			//MSDC_LOG(IRQ, "CMD<%d> MSDC_INT_RSPCRCERR Arg<0x%.8x>", cmd->opcode, cmd->arg);
			MSDC_RESET_HW(host->id);
		} else if (intsts & MSDC_INT_CMDTMO) {
			cmd->error = CONV2UINT(-ETIMEDOUT);

			/* Not in init process, init process will try to recongnize it as SD/eMMC/SDIO, */
			/* send serveral type commands, it is not real error. */
			//if (host->sclk > MSDC_CLK_400KHZ)
				//MSDC_LOG(IRQ, "CMD<%d> MSDC_INT_CMDTMO Arg<0x%.8x> CLK<%u>",cmd->opcode,cmd->arg,host->sclk);
			MSDC_RESET_HW(host->id);
		}
#ifdef MTK_MSDC_USE_CMD23
		if ((sbc != NULL) && (host->autocmd & MSDC_AUTOCMD23)) {
			if (intsts & MSDC_INT_ACMDRDY) {
				u32 *arsp = &sbc->resp[0];
				*arsp = MSDC_READ32(SDC_ACMD_RESP);
				
			} else if (intsts & MSDC_INT_ACMDCRCERR) {
				//MSDC_LOG_NORMAL(pr_err, "[%d]autocmd23 crc error", host->id);
				sbc->error = (unsigned int)-EIO;
				cmd->error = (unsigned int)-EIO;
				/* host->error |= REQ_CMD23_EIO; */
				MSDC_RESET_HW(host->id);
			} else if (intsts & MSDC_INT_ACMDTMO) {
				//MSDC_LOG_NORMAL(pr_err, "[%d]autocmd23 tmo error", host->id);
				sbc->error = (unsigned int)-ETIMEDOUT;
				cmd->error = (unsigned int)-ETIMEDOUT;
				msdc_dump_info(host->id);
				/* host->error |= REQ_CMD23_TMO; */
				MSDC_RESET_HW(host->id);
			}
		}
#endif 
    }
out:
	if((host->last_cmd_err_cmd_opcode != cmd->opcode) || (host->last_cmd_err_cmd_arg != cmd->arg) || (host->last_cmd_err_intr != cmd->error)) {
#ifdef CONFIG_MSDC_NEW_TUNING_SUPPORT
		if((host->last_cmd_err_intr == (unsigned int)-EIO)
				&& (host->last_cmd_err_cmd_opcode != MMC_SEND_TUNING_BLOCK)
				&& (host->last_cmd_err_cmd_opcode != MMC_SEND_TUNING_BLOCK_HS200))
#else
		if(host->last_cmd_err_intr == (unsigned int)-EIO)
#endif
			MSDC_LOG(ERR, "CMD<%d>, MSDC_INT_RSPCRCERR, Arg<0x%.8x> CLK<%u>, fail_cnt:%d",
					host->last_cmd_err_cmd_opcode, host->last_cmd_err_cmd_arg, host->sclk, host->last_cmd_err_retry_cnt);

#ifdef CONFIG_MSDC_NEW_TUNING_SUPPORT
		else if((host->last_cmd_err_intr == (unsigned int)-ETIMEDOUT) && (host->sclk > MSDC_CLK_400KHZ)
				&& (host->last_cmd_err_cmd_opcode != MMC_SEND_TUNING_BLOCK)
				&& (host->last_cmd_err_cmd_opcode != MMC_SEND_TUNING_BLOCK_HS200))
#else
		else if((host->last_cmd_err_intr == (unsigned int)-ETIMEDOUT) && (host->sclk > MSDC_CLK_400KHZ))

#endif
			MSDC_LOG(ERR, "CMD<%d>, MSDC_INT_CMDTMO, Arg<0x%.8x> CLK<%u>, fail_cnt:%d",
					host->last_cmd_err_cmd_opcode, host->last_cmd_err_cmd_arg, host->sclk, host->last_cmd_err_retry_cnt);

		host->last_cmd_err_cmd_opcode = cmd->opcode;
		host->last_cmd_err_cmd_arg = cmd->arg;
		host->last_cmd_err_intr = cmd->error;
		host->last_cmd_err_retry_cnt = 0;
	} else {
		host->last_cmd_err_retry_cnt ++;
	}
	host->cmd = NULL;

	return cmd->error;
}

#if 0
static unsigned int msdc_command_resp(struct msdc_host *host,
				      struct mmc_command *cmd, int tune, unsigned long timeout)
{
	u32 base = host->base;
	u32 opcode = cmd->opcode;
	/* u32 resp = host->cmd_rsp; */
	/* u32 tmo; */
	/* u32 intsts; */

	spin_unlock(&host->lock);

	if (!wait_for_completion_timeout(&host->cmd_done, 10 * timeout)) {
		MSDC_LOG(ERR, "CMD<%d> wait_for_completion timeout ARG<0x%.8x>", opcode,
			 cmd->arg);
		host->sw_timeout++;
		msdc_dump_info(host->id);
		cmd->error = CONV2UINT(-ETIMEDOUT);
		MSDC_RESET_HW(host->id);
	}

	spin_lock(&host->lock);

	MSDC_CLR_BITS(MSDC_INTEN, wints_cmd);
	host->cmd = NULL;
	/* if (resp == RESP_R1B) {
	   while ((MSDC_READ32(MSDC_PS) & 0x10000) != 0x10000);
	   } */

	return cmd->error;
}
#endif


unsigned int msdc_do_command(struct msdc_host *host,
			     struct mmc_command *cmd, int tune, unsigned long timeout)
{
	MVG_EMMC_DECLARE_INT32(delay_ns);
	MVG_EMMC_DECLARE_INT32(delay_us);
	MVG_EMMC_DECLARE_INT32(delay_ms);

	if ((cmd->opcode == MMC_GO_IDLE_STATE) && (host->hw->host_function == MSDC_SD)) {
		mdelay(10);
	}

	MVG_EMMC_ERASE_MATCH(host, (u64)cmd->arg, delay_ms, delay_us, delay_ns, cmd->opcode);

	if (msdc_command_start(host, cmd, tune, timeout)) {
		goto end;
	}

	MVG_EMMC_ERASE_RESET(delay_ms, delay_us, cmd->opcode);

	if (msdc_command_resp_polling(host, cmd, tune, timeout)) {
		goto end;
	}

end:

	MSDC_LOG(CMD, "        return<%d> resp<0x%.8x>", cmd->error, cmd->resp[0]);
	return cmd->error;
}

/* The abort condition when PIO read/write
   tmo:
*/
static int msdc_pio_abort_warning(struct msdc_host *host, struct mmc_data *data, unsigned long tmo)
{
	int ret = 0;
	/* u32  base = host->base; */

	if (atomic_read(&host->abort)) {
		ret = 1;
	}

	if (time_after(jiffies, tmo)) {
		data->error = CONV2UINT(-ETIMEDOUT);
		MSDC_LOG(ERR, "PIO Data Timeout: CMD<%d>", host->mrq->cmd->opcode);
		msdc_dump_info(host->id);
		ret = 1;
	}

	return ret;
}

/* The abort condition when PIO read/write
   tmo:
*/
static int msdc_pio_abort(struct msdc_host *host, struct mmc_data *data, unsigned long tmo)
{
	int ret = 0;
	u32 base = host->base;

	if (atomic_read(&host->abort)) {
		ret = 1;
	}

	if (time_after(jiffies, tmo)) {
		data->error = CONV2UINT(-ETIMEDOUT);
#ifdef CONFIG_MSDC_NEW_TUNING_SUPPORT
		if((host->mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK) && (host->mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK_HS200))
#endif
			MSDC_LOG(ERR, "PIO Data SW Timeout: CMD<%d>", host->mrq->cmd->opcode);
		//msdc_dump_info(host->id);
		ret = 1;
	}

	if (ret) {
		MSDC_RESET_HW(host->id);
#ifdef CONFIG_MSDC_NEW_TUNING_SUPPORT
		if((host->mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK) && (host->mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK_HS200))
#endif
			MSDC_LOG(ERR, "msdc pio find abort");
	}

	return ret;
}

/*
   Need to add a timeout, or WDT timeout, system reboot.
*/
/* pio mode data read/write */
static int msdc_pio_read(struct msdc_host *host, struct mmc_data *data)
{
	struct scatterlist *sg = data->sg;
	u32 base = host->base;
	u32 num = data->sg_len;
	u32 *ptr;
	u8 *u8ptr;
	u32 left = 0;
	u32 count, size = 0;
	u32 wints = MSDC_INTEN_DATTMO | MSDC_INTEN_DATCRCERR | MSDC_INTEN_XFER_COMPL;
	u32 ints = 0;
	bool get_xfer_done = 0;
	
	unsigned long tmo = jiffies + DAT_TIMEOUT;

#ifdef CONFIG_MSDC_ETT_SUPPORT
	if (ett_tune_flag&&(host->hw->host_function == MSDC_SDIO))
		tmo = jiffies + (HZ/10);
#endif

#ifdef CONFIG_MSDC_NEW_TUNING_SUPPORT
	if((host->mrq->cmd->opcode == MMC_SEND_TUNING_BLOCK)
			|| (host->mrq->cmd->opcode == MMC_SEND_TUNING_BLOCK_HS200))
		tmo = jiffies + (HZ/10);
#endif

	/* MSDC_CLR_BITS(MSDC_INTEN, wints); */
	/* set to 4bytes access mode */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, MSDC_IOCON_RISCSZ_4BYTE);

	while (1) {
		if (!get_xfer_done) {
			ints = MSDC_READ32(MSDC_INT);
			ints &= wints;
			MSDC_WRITE32(MSDC_INT, ints);
		}

		if (ints & MSDC_INT_DATTMO) {
			data->error = CONV2UINT(-ETIMEDOUT);
			MSDC_RESET_HW(host->id);
			break;
		} else if (ints & MSDC_INT_DATCRCERR) {
			data->error = CONV2UINT(-EIO);
			MSDC_RESET_HW(host->id);
			break;
		} else if (ints & MSDC_INT_XFER_COMPL) {
			get_xfer_done = 1;

			if ((num == 0) && (left == 0)) {
				break;
			}
		}

		if (msdc_pio_abort_warning(host, data, tmo)) {
			if ((num == 0) && (left == 0)) {
				break;
			}

			MSDC_RESET_HW(host->id);
			MSDC_LOG(ERR, "msdc pio find abort");
			goto end;
		}

		if ((num == 0) && (left == 0)) {
			continue;
		}

		left = sg_dma_len(sg);
		ptr = sg_virt(sg);

		while (left) {
			if ((left >= MSDC_FIFO_THD) && (MSDC_RXFIFO_CNT() >= MSDC_FIFO_THD)) {
				count = MSDC_FIFO_THD >> 2;

				do {
					*ptr = MSDC_FIFO_READ32();
					ptr++;
				} while (--count);

				left -= MSDC_FIFO_THD;
			} else if ((left < MSDC_FIFO_THD) && MSDC_RXFIFO_CNT() >= left) {

				while (left > 3) {
					*ptr = MSDC_FIFO_READ32();
					ptr++;
					left -= 4;
				}

				if (left > 0) {
					MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, 0);
					u8ptr = (u8 *) ptr;

					while (left) {
						*u8ptr = MSDC_FIFO_READ8();
						u8ptr++;
						left--;
					}

					MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, 2);
				}
			}

			if (msdc_pio_abort(host, data, tmo)) {
				goto end;
			}
		}

		size += sg_dma_len(sg);
		sg = sg_next(sg);
		num--;
	}

end:
	data->bytes_xfered += size;
	MSDC_LOG(FIO, "        PIO Read<%d>bytes", size);

	/* MSDC_CLR_BITS(MSDC_INTEN, wints); */
#ifdef CONFIG_MSDC_NEW_TUNING_SUPPORT
	if ((data->error) &&
		((host->mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK) && (host->mrq->cmd->opcode != MMC_SEND_TUNING_BLOCK_HS200)))
#else
	if (data->error)
#endif
		MSDC_LOG(ERR, "read pio data->error<%d> left<%d> size<%d>", data->error, left, size);

	return data->error;
}

/* please make sure won't using PIO when size >= 512
   which means, memory card block read/write won't using pio
   then don't need to handle the CMD12 when data error.
*/
static int msdc_pio_write(struct msdc_host *host, struct mmc_data *data)
{
	u32 base = host->base;
	struct scatterlist *sg = data->sg;
	u32 num = data->sg_len;
	u32 *ptr;
	u8 *u8ptr;
	u32 left = 0;
	u32 count, size = 0;
	u32 wints = MSDC_INTEN_DATTMO | MSDC_INTEN_DATCRCERR | MSDC_INTEN_XFER_COMPL;
	bool get_xfer_done = 0;
	unsigned long tmo = jiffies + DAT_TIMEOUT;
	u32 ints = 0;
#ifdef CONFIG_MSDC_ETT_SUPPORT
	if (ett_tune_flag&&(host->hw->host_function == MSDC_SDIO))
				tmo = jiffies + (HZ/10);
#endif
	/* MSDC_CLR_BITS(MSDC_INTEN, wints); */
	/* set to 4bytes access mode */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, MSDC_IOCON_RISCSZ_4BYTE);

	while (1) {
		if (!get_xfer_done) {
			ints = MSDC_READ32(MSDC_INT);
			ints &= wints;
			MSDC_WRITE32(MSDC_INT, ints);
		}

		if (ints & MSDC_INT_DATTMO) {
			data->error = CONV2UINT(-ETIMEDOUT);
			MSDC_RESET_HW(host->id);
			break;
		} else if (ints & MSDC_INT_DATCRCERR) {
			data->error = CONV2UINT(-EIO);
			MSDC_RESET_HW(host->id);
			break;
		} else if (ints & MSDC_INT_XFER_COMPL) {
			get_xfer_done = 1;

			if ((num == 0) && (left == 0)) {
				break;
			}
		}

		if (msdc_pio_abort(host, data, tmo)) {
			goto end;
		}

		if ((num == 0) && (left == 0)) {
			continue;
		}

		left = sg_dma_len(sg);
		ptr = sg_virt(sg);

		while (left) {
			if ((left >= MSDC_FIFO_SZ) && (MSDC_TXFIFO_CNT() == 0)) {
				count = MSDC_FIFO_SZ >> 2;

				do {
					MSDC_FIFO_WRITE32(*ptr);
					ptr++;
				} while (--count);

				left -= MSDC_FIFO_SZ;
			} else if ((left < MSDC_FIFO_SZ) && (MSDC_TXFIFO_CNT() == 0)) {
				while (left > 3) {
					MSDC_FIFO_WRITE32(*ptr);
					ptr++;
					left -= 4;
				}

				if (left > 0) {
					u8ptr = (u8 *) ptr;
					MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, 0);

					while (left) {
						MSDC_FIFO_WRITE8(*u8ptr);
						u8ptr++;
						left--;
					}

					MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, 2);
				}
			}

			if (msdc_pio_abort(host, data, tmo)) {
				goto end;
			}

		}

		size += sg_dma_len(sg);
		sg = sg_next(sg);
		num--;
	}

end:
	data->bytes_xfered += size;
	MSDC_LOG(FIO, "        PIO Write<%d>bytes", size);

	if (data->error) {
		MSDC_LOG(ERR, "write pio data->error<%d>", data->error);
	}

	/* MSDC_CLR_BITS(MSDC_INTEN, wints); */
	return data->error;
}


static void msdc_dma_start(struct msdc_host *host)
{
	u32 base = host->base;
	u32 wints = MSDC_INTEN_XFER_COMPL | MSDC_INTEN_DATTMO | MSDC_INTEN_DATCRCERR;

	if (host->autocmd == MSDC_AUTOCMD12) {
		wints |= MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO | MSDC_INT_ACMDRDY;
	}

	MSDC_SET_BITS(MSDC_INTEN, wints);
	mb();
	MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_START, 1);

	/* MSDC_LOG(DMA, "DMA start"); */
}

static void msdc_dma_stop(struct msdc_host *host)
{
	u32 base = host->base;
	int retry = 30;
	int count = 1000;
	/* u32 retries=500; */
	u32 wints = MSDC_INTEN_XFER_COMPL | MSDC_INTEN_DATTMO | MSDC_INTEN_DATCRCERR;

	if (host->autocmd == MSDC_AUTOCMD12) {
		wints |= MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO | MSDC_INT_ACMDRDY;
	}

	/* MSDC_LOG(DMA, "DMA status: 0x%.8x", MSDC_READ32(MSDC_DMA_CFG)); */
	/* while (MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS); */

	MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_STOP, 1);
	/* while (MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS); */
	MSDC_RETRY((MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS), retry, count, host->id);
	/* if(retry == 0){ */
	/* MSDC_LOG(ERR, "!!ASSERT!!"); */
	/* BUG(); */
	/* } */
	mb();
	MSDC_CLR_BITS(MSDC_INTEN, wints);	/* Not just xfer_comp */

	/* MSDC_LOG(DMA, "DMA stop"); */
}

#if 0
/* dump a gpd list */
static void msdc_dma_dump(struct msdc_host *host, struct msdc_dma *dma)
{
	gpd_t *gpd = dma->gpd;
	bd_t *bd = dma->bd;
	bd_t *ptr;
	int i = 0;
	int p_to_v;

	if (dma->mode != MSDC_MODE_DMA_DESC) {
		return;
	}

	MSDC_LOG(ERR, "try to dump gpd and bd");

	/* dump gpd */
	MSDC_LOG(ERR, ".gpd<0x%.8x> gpd_phy<0x%.8x>", (int)gpd, (int)dma->gpd_addr);
	MSDC_LOG(ERR, "...hwo   <%d>", gpd->hwo);
	MSDC_LOG(ERR, "...bdp   <%d>", gpd->bdp);
	MSDC_LOG(ERR, "...chksum<0x%.8x>", gpd->chksum);
	/* MSDC_LOG(ERR, "...intr  <0x%.8x>", gpd->intr ); */
	MSDC_LOG(ERR, "...next  <0x%.8x>", (int)gpd->next);
	MSDC_LOG(ERR, "...ptr   <0x%.8x>", (int)gpd->ptr);
	MSDC_LOG(ERR, "...buflen<0x%.8x>", gpd->buflen);
	/* MSDC_LOG(ERR, "...extlen<0x%.8x>", gpd->extlen ); */
	/* MSDC_LOG(ERR, "...arg   <0x%.8x>", gpd->arg ); */
	/* MSDC_LOG(ERR, "...blknum<0x%.8x>", gpd->blknum ); */
	/* MSDC_LOG(ERR, "...cmd   <0x%.8x>", gpd->cmd ); */

	/* dump bd */
	MSDC_LOG(ERR, ".bd<0x%.8x> bd_phy<0x%.8x> gpd_ptr<0x%.8x>", (int)bd, (int)dma->bd_addr,
		 (int)gpd->ptr);
	ptr = bd;
	p_to_v = ((u32) bd - (u32) dma->bd_addr);

	while (1) {
		MSDC_LOG(ERR, ".bd[%d]", i);
		i++;
		MSDC_LOG(ERR, "...eol   <%d>", ptr->eol);
		MSDC_LOG(ERR, "...chksum<0x%.8x>", ptr->chksum);
		/* MSDC_LOG(ERR, "...blkpad<0x%.8x>", ptr->blkpad ); */
		/* MSDC_LOG(ERR, "...dwpad <0x%.8x>", ptr->dwpad ); */
		MSDC_LOG(ERR, "...next  <0x%.8x>", (int)ptr->next);
		MSDC_LOG(ERR, "...ptr   <0x%.8x>", (int)ptr->ptr);
		MSDC_LOG(ERR, "...buflen<0x%.8x>", (int)ptr->buflen);

		if (ptr->eol == 1) {
			break;
		}

		/* find the next bd, virtual address of ptr->next */
		/* don't need to enable when use malloc */
		/* BUG_ON( (ptr->next + p_to_v)!=(ptr+1) ); */
		/* MSDC_LOG(ERR, ".next bd<0x%.8x><0x%.8x>", (ptr->next + p_to_v), (ptr+1)); */
		ptr++;
	}

	MSDC_LOG(ERR, "dump gpd and bd finished");
}
#endif

/* calc checksum */
static u8 msdc_dma_calcs_chksum(u8 *buf, u32 len)
{
	u32 i, sum = 0;

	for (i = 0; i < len; i++) {
		sum += buf[i];
	}

	return 0xFF - (u8) sum;
}

#ifdef MSDC_DMA_BOUNDARY_LIMITAION

#define MSDC_DMA_BOUNDARY_SIZE                       (2048)
#define MSDC_DMA_START_ADDRESS_ALIGNMENT             (0)
#define MSDC_DMA_NOT_EXCEED_BOUNDARY                 (0)
#define MSDC_DMA_ACROSS_BOUNDARY                     (1)

u32 msdc_dma_address_check(u32 start, u32 len)
{
	/* mask bit0-1 */
	start &= (~0x3);

	/* two type of violation 2k boundary will be checked */
	if (0 == start % 64) {
		return MSDC_DMA_START_ADDRESS_ALIGNMENT;
	}

	if (len > (MSDC_DMA_BOUNDARY_SIZE - (start & 0x7FF)))
		/* cross boundary */
	{
		return MSDC_DMA_ACROSS_BOUNDARY;
	}

	return MSDC_DMA_NOT_EXCEED_BOUNDARY;
}

#endif

/* gpd bd setup + dma registers */
static int msdc_dma_config(struct msdc_host *host, struct msdc_dma *dma)
{
	u32 base = host->base;
	u32 sglen = dma->sglen;
	/* u32 i, j, num, bdlen, arg, xfersz; */
	u32 j, num, bdlen;
	u32 dma_address, dma_len;
	u8 blkpad, dwpad, chksum;
	struct scatterlist *sg = dma->sg;
	gpd_t *gpd;
	bd_t *bd;
#ifdef MSDC_DMA_BOUNDARY_LIMITAION
	u32 ret;
	u32 dma_address_tmp, dma_len_tmp;
#endif

	switch (dma->mode) {
	case MSDC_MODE_DMA_BASIC:
		BUG_ON(dma->xfersz > 65535);
		BUG_ON(dma->sglen != 1);
		dma_address = sg_dma_address(sg);
		dma_len = sg_dma_len(sg);
		MSDC_WRITE32(MSDC_DMA_SA, dma_address);

		MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_LASTBUF, 1);
		MSDC_WRITE32(MSDC_DMA_LEN, dma_len);
		MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, dma->burstsz);
		MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 0);
		break;

	case MSDC_MODE_DMA_DESC:
		blkpad = (dma->flags & DMA_FLAG_PAD_BLOCK) ? 1 : 0;
		dwpad = (dma->flags & DMA_FLAG_PAD_DWORD) ? 1 : 0;
		chksum = (dma->flags & DMA_FLAG_EN_CHKSUM) ? 1 : 0;

		/* calculate the required number of gpd */
		num = (sglen + MAX_BD_PER_GPD - 1) / MAX_BD_PER_GPD;
		BUG_ON(num != 1);

		gpd = dma->gpd;
		bd = dma->bd;
		bdlen = sglen;

		/* modify gpd */
		/* gpd->intr = 0; */
		gpd->hwo = 1;	/* hw will clear it */
		gpd->bdp = 1;
		gpd->chksum = 0;	/* need to clear first. */
		gpd->chksum = (chksum ? msdc_dma_calcs_chksum((u8 *) gpd, 16) : 0);

		/* modify bd */
		for (j = 0; j < bdlen; j++) {
#ifdef MSDC_DMA_VIOLATION_DEBUG

			if (g_dma_debug[host->id]
			    && (msdc_latest_operation_type[host->id] == OPER_TYPE_READ)) {
				MSDC_LOG(ERR, "msdc%d do write 0x10000", host->id);
				dma_address = 0x10000;
			} else {
				dma_address = sg_dma_address(sg);
			}

#else
			dma_address = sg_dma_address(sg);
#endif

			dma_len = sg_dma_len(sg);

#ifdef MSDC_DMA_BOUNDARY_LIMITAION
			/* just for MT6582 */
			ret = msdc_dma_address_check(dma_address, dma_len);

			if (ret == MSDC_DMA_ACROSS_BOUNDARY) {
				/* add debug info */
				MSDC_LOG(DMA, "across 2k boundary; start = 0x%x, len = %d",
					 dma_address, dma_len);

				/* need spilt the BD to BD_1, BD_2,
				 * rule: make sure the start address of BD_2 is 2K boundary */
				dma_len_tmp = MSDC_DMA_BOUNDARY_SIZE - (dma_address & 0x7FF);
				MSDC_INIT_BD(&bd[j], blkpad, dwpad, dma_address, dma_len_tmp);

				MSDC_LOG(DMA, "BD_1: start = 0x%x, len = %d", dma_address,
					 dma_len_tmp);

				bd[j].eol = 0;
				bd[j].chksum = 0;	/* checksume need to clear first */
				bd[j].chksum =
					(chksum ? msdc_dma_calcs_chksum((u8 *)(&bd[j]), 16) : 0);

				j += 1;
				bdlen += 1;

				dma_address_tmp = dma_address + dma_len_tmp;
				dma_len_tmp = dma_len - dma_len_tmp;
				MSDC_INIT_BD(&bd[j], blkpad, dwpad, dma_address_tmp, dma_len_tmp);

				MSDC_LOG(DMA, "BD_2: start = 0x%x, len = %d", dma_address_tmp,
					 dma_len_tmp);

				if (j == bdlen - 1) {
					bd[j].eol = 1;        /* the last bd */
				} else {
					bd[j].eol = 0;
				}

				bd[j].chksum = 0;	/* checksume need to clear first */
				bd[j].chksum =
					(chksum ? msdc_dma_calcs_chksum((u8 *)(&bd[j]), 16) : 0);
			} else {
				/* if no need change, take the old way */
				MSDC_INIT_BD(&bd[j], blkpad, dwpad, dma_address, dma_len);

				if (j == bdlen - 1) {
					bd[j].eol = 1;        /* the last bd */
				} else {
					bd[j].eol = 0;
				}

				bd[j].chksum = 0;	/* checksume need to clear first */
				bd[j].chksum =
					(chksum ? msdc_dma_calcs_chksum((u8 *)(&bd[j]), 16) : 0);
			}

#else
			MSDC_INIT_BD(&bd[j], blkpad, dwpad, dma_address, dma_len);

			if (j == bdlen - 1) {
				bd[j].eol = 1;        /* the last bd */
			} else {
				bd[j].eol = 0;
			}

			bd[j].chksum = 0;	/* checksume need to clear first */
			bd[j].chksum = (chksum ? msdc_dma_calcs_chksum((u8 *)(&bd[j]), 16) : 0);
#endif

			sg++;
		}

#ifdef MSDC_DMA_VIOLATION_DEBUG

		if (g_dma_debug[host->id]
		    && (msdc_latest_operation_type[host->id] == OPER_TYPE_READ)) {
			g_dma_debug[host->id] = 0;
		}

#endif

		dma->used_gpd += 2;
		dma->used_bd += bdlen;

		MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
		MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, dma->burstsz);
		MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);

		MSDC_WRITE32(MSDC_DMA_SA, (u32) dma->gpd_addr);
		break;

	default:
		break;
	}

	/* MSDC_LOG(DAM, "DMA_CTRL = 0x%.8x", MSDC_READ32(MSDC_DMA_CTRL)); */
	/* MSDC_LOG(DMA, "DMA_CFG  = 0x%.8x", MSDC_READ32(MSDC_DMA_CFG)); */
	/* MSDC_LOG(DMA, "DMA_SA   = 0x%.8x", MSDC_READ32(MSDC_DMA_SA)); */

	return 0;
}

#ifdef FORCE_DESCRIPTOR_DMA

static void msdc_dma_setup(struct msdc_host *host, struct msdc_dma *dma,
			   struct scatterlist *sg, unsigned int sglen)
{
	BUG_ON(sglen > MAX_BD_NUM);	/* not support currently */

	dma->sg = sg;
	dma->flags = DMA_FLAG_EN_CHKSUM;
	dma->sglen = sglen;
	dma->xfersz = host->xfer_size;
	dma->burstsz = MSDC_BRUST_64B;

	dma->mode = MSDC_MODE_DMA_DESC;

	/* MSDC_LOG(DMA, "DMA mode<%d> sglen<%d> xfersz<%d>", dma->mode, dma->sglen, dma->xfersz); */

	msdc_dma_config(host, dma);
}

#else

static void msdc_dma_setup(struct msdc_host *host, struct msdc_dma *dma,
			   struct scatterlist *sg, unsigned int sglen)
{
#ifdef MSDC_DMA_BOUNDARY_LIMITAION
	u32 dma_address, dma_len, ret;
#endif
	BUG_ON(sglen > MAX_BD_NUM);	/* not support currently */

	dma->sg = sg;
	dma->flags = DMA_FLAG_EN_CHKSUM;
	/* dma->flags = DMA_FLAG_NONE; *//* CHECKME */
	dma->sglen = sglen;
	dma->xfersz = host->xfer_size;
	dma->burstsz = MSDC_BRUST_64B;

	if ((sglen == 1) && (sg_dma_len(sg) <= MAX_DMA_CNT)) {
#ifdef MSDC_DMA_BOUNDARY_LIMITAION
		dma_len = sg_dma_len(sg);
		dma_address = sg_dma_address(sg);
		ret = msdc_dma_address_check(dma_address, dma_len);

		if (ret == MSDC_DMA_ACROSS_BOUNDARY) {
			dma->mode = MSDC_MODE_DMA_BASIC;
		} else
#endif
			dma->mode = MSDC_MODE_DMA_BASIC;
	} else {
		dma->mode = MSDC_MODE_DMA_DESC;
	}

	/* MSDC_LOG(DMA, "DMA mode<%d> sglen<%d> xfersz<%d>", dma->mode, dma->sglen, dma->xfersz); */

	msdc_dma_config(host, dma);
}
#endif

/* set block number before send command */
static void msdc_set_blknum(struct msdc_host *host, u32 blknum)
{
	u32 base = host->base;

	MSDC_WRITE32(SDC_BLK_NUM, blknum);
}

/*  */
/* This Func is for SDIO3.0 */
/*  */
static void msdc_restore_info(struct msdc_host *host)
{
	u32 base = host->base;
	int retry = 3;

	mb();
	MSDC_RESET_HW(host->id);
	host->saved_para.msdc_cfg = host->saved_para.msdc_cfg & 0xFFFFFFDF;	/* force bit5(BV18SDT) to 0 */
	MSDC_WRITE32(MSDC_CFG, host->saved_para.msdc_cfg);

	while (retry--) {
		msdc_set_mclk(host, host->saved_para.ddr, host->saved_para.hz);

		if (MSDC_READ32(MSDC_CFG) != host->saved_para.msdc_cfg) {
			MSDC_LOG(ERR,
				 "msdc set_mclk is unstable (cur_cfg=%x, save_cfg=%x, cur_hz=%d, save_hz=%d).",
				 MSDC_READ32(MSDC_CFG), host->saved_para.msdc_cfg, host->mclk,
				 host->saved_para.hz);
		} else {
			break;
		}
	}

	/* -----------  for SDIO 3.0  -----------*/
	MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, host->saved_para.int_dat_latch_ck_sel);
	MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, host->saved_para.ckgen_msdc_dly_sel);
	MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, host->saved_para.cmd_resp_ta_cntr);
	MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, host->saved_para.wrdat_crc_ta_cntr);
	/* --------- End for SDIO 3.0 ---------*/
	MSDC_WRITE32(MSDC_DAT_RDDLY0, host->saved_para.ddly0);
	MSDC_WRITE32(MSDC_PAD_TUNE, host->saved_para.pad_tune);
	MSDC_WRITE32(SDC_CFG, host->saved_para.sdc_cfg);
	/* get INTEN status for SDIO */
	MSDC_SET_FIELD(MSDC_INTEN, MSDC_INT_SDIOIRQ, host->saved_para.inten_sdio_irq);
	MSDC_WRITE32(MSDC_IOCON, host->saved_para.iocon);
}

static int msdc_hs_read_pre(struct msdc_host *host)
{
	u32 base = host->base;
	/* If in tuning process, we not change tune parameters, return directly */
	if ((host->hw->read_pre_setting_en) && NOT_IN_TUNE_PROCESS(host)) {
		if (host->hw->tuning_method == TUNE_ALL_DAT_LINE) {
			if (!host->ddr) {
				/* set all data line use same delay cycle */
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 0);
				/* set all data line use same sample edge */
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
					       host->hw->read_dat_latch_ck_sel);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
					       host->hw->read_ckgen_delay_sel);
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL,
					       host->hw->read_sample_edge);
				MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
					       host->hw->read_pad_delay);
			} else {
				/* set all data line use same delay cycle */
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 0);
				/* set all data line use same sample edge */
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
					       host->hw->ddr_read_dat_latch_ck_sel);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
					       host->hw->ddr_read_ckgen_delay_sel);
			}

			/* MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS,
						host->hw->read_patch_bit1_wrdat_crcs); */
			/* MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP,
						host->hw->read_patch_bit1_cmdrsp_ta); */
		} else {		/* (TUNE_EACH_DAT_LINE) */
			/* set each data line has its own delay selection */
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
				       host->hw->read_ckgen_delay_sel);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
				       host->hw->read_dat_latch_ck_sel);
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, host->hw->read_sample_edge);
			MSDC_WRITE32(MSDC_DAT_RDDLY0, host->hw->read_rxdly0);
			MSDC_WRITE32(MSDC_DAT_RDDLY1, host->hw->read_rxdly1);
		}
	}
	return 0;
}

static int msdc_hs_write_pre(struct msdc_host *host)
{
	u32 base = host->base;
	/* If in tuning process, we not change tune parameters, return directly */
	if ((host->hw->write_pre_setting_en) && NOT_IN_TUNE_PROCESS(host)) {
		if (host->hw->tuning_method == TUNE_ALL_DAT_LINE) {
			if (!host->ddr) {
				/* different data line use different sample edge */
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 1);

				/* different data line use different delay cycle */
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
					       host->hw->write_dat_latch_ck_sel);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
					       host->hw->write_ckgen_delay_sel);
				MSDC_SET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0,
					       host->hw->write_pad_delay);
				MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY,
					       host->hw->write_internal_delay);
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL,
					       host->hw->write_sample_edge);
			} else {
				/* =====  Write OPS only tuning DAT0 ===== */

				/* different data line use different sample edge */
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 1);

				/* different data line use different delay cycle */
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
					       host->hw->ddr_write_dat_latch_ck_sel);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
					       host->hw->ddr_write_ckgen_delay_sel);
			}

			/* MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS,
						host->hw->write_patch_bit1_wrdat_crcs); */
			/* MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP,
						host->hw->write_patch_bit1_cmdrsp_ta); */
		
		} else {		/* (TUNE_EACH_DAT_LINE) */
		/* set each data line has its own delay */
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, host->hw->write_sample_edge);
		//MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, host->hw->write_dat_latch_ck_sel);
		//MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, host->hw->write_ckgen_delay_sel);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, host->hw->write_patch_bit1_wrdat_crcs);
		MSDC_WRITE32(MSDC_DAT_RDDLY0, host->hw->write_rxdly0);
		}
	}
	return 0;
}

static int msdc_save_read_pre(struct msdc_host *host)
{
	u32 base = host->base;

	if (host->hw->tuning_method == TUNE_ALL_DAT_LINE) {
		if (!host->ddr) {
			MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
				       host->hw->read_dat_latch_ck_sel);
			MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
				       host->hw->read_ckgen_delay_sel);
			MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, host->hw->read_sample_edge);
			MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
				       host->hw->read_pad_delay);
		} else {
			MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
				       host->hw->ddr_read_dat_latch_ck_sel);
			MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
				       host->hw->ddr_read_ckgen_delay_sel);
		}

		/* MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, host->hw->read_patch_bit1_wrdat_crcs); */
		/* MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, host->hw->read_patch_bit1_cmdrsp_ta); */

		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, 2);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, 2);
	} else {
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
			       host->hw->read_ckgen_delay_sel);
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
			       host->hw->read_dat_latch_ck_sel);
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, host->hw->read_sample_edge);
		host->hw->read_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
		host->hw->read_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);
	}

	return 0;
}

static int msdc_save_write_pre(struct msdc_host *host)
{
	u32 base = host->base;

	if (host->hw->tuning_method == TUNE_ALL_DAT_LINE) {
		if (!host->ddr) {
			MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
				       host->hw->write_dat_latch_ck_sel);
			MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
				       host->hw->write_ckgen_delay_sel);
			MSDC_GET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0,
				       host->hw->write_pad_delay);
			MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY,
				       host->hw->write_internal_delay);
			MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL,
				       host->hw->write_sample_edge);
		} else {
			MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
				       host->hw->ddr_write_dat_latch_ck_sel);
			MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
				       host->hw->ddr_write_ckgen_delay_sel);
		}

		/* MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS,
					host->hw->write_patch_bit1_wrdat_crcs); */
		/* MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP,
					host->hw->write_patch_bit1_cmdrsp_ta); */

		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, 2);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, 2);
	} else {
		MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, host->hw->write_sample_edge);
		MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS,
			       host->hw->write_patch_bit1_wrdat_crcs);
		host->hw->write_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
	}

	return 0;
}

static int msdc_do_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_command *cmd;
	struct mmc_data *data;

	u32 l_autocmd23_is_set = 0;
#ifdef MTK_MSDC_USE_CMD23
	u32 l_card_no_cmd23 = 0;
#endif

	u32 base = host->base;
	unsigned int left = 0;
	int dma = 0, read = 1, dir = DMA_FROM_DEVICE;
	u32 map_sg = 0;		/* Fix the bug of dma_map_sg and dma_unmap_sg not match issue */
	int nents = 0;		/* number of buffers to map */
	unsigned long pio_tmo;

	/* SDIO */
	if (host->hw->flags & MSDC_SDIO_IRQ) {
		mb();

		if (host->saved_para.hz) {
			if (host->saved_para.suspend_flag) {
				MSDC_LOG_NORMAL(pr_info,
					 "msdc%d resume[s] cur_cfg=%x, save_cfg=%x, cur_hz=%d, save_hz=%d", host->id,
					 MSDC_READ32(MSDC_CFG), host->saved_para.msdc_cfg,
					 host->mclk, host->saved_para.hz);
				host->saved_para.suspend_flag = 0;
				msdc_restore_info(host);
			} else if ((host->saved_para.msdc_cfg != 0)
				   && (MSDC_READ32(MSDC_CFG) != host->saved_para.msdc_cfg)) {
				MSDC_LOG_NORMAL(pr_info,
					 "msdc%d resume[ns] cur_cfg=%x, save_cfg=%x, cur_hz=%d, save_hz=%d", host->id,
					 MSDC_READ32(MSDC_CFG), host->saved_para.msdc_cfg,
					 host->mclk, host->saved_para.hz);
				msdc_restore_info(host);
			}
		}
	}

#if (MSDC_DATA1_INT == 1)

	if (host->hw->flags & MSDC_SDIO_IRQ) {
		if ((u_sdio_irq_counter > 0) && ((u_sdio_irq_counter % 800) == 0))
			MSDC_LOG(ERR,
				 "Ahsin sdio_irq=%d, msdc_irq=%d SDC_CFG=%x MSDC_INTEN=%x MSDC_INT=%x",
				 u_sdio_irq_counter, u_msdc_irq_counter, MSDC_READ32(SDC_CFG),
				 MSDC_READ32(MSDC_INTEN), MSDC_READ32(MSDC_INT));
	}

#endif

	BUG_ON(mmc == NULL);
	BUG_ON(mrq == NULL);

	host->error = 0;
	atomic_set(&host->abort, 0);

	cmd = mrq->cmd;
	data = mrq->cmd->data;

	/* check msdc is work ok. rule is RX/TX fifocnt must be zero after last request
	 * if find abnormal, try to reset msdc first
	 */
	if (MSDC_TXFIFO_CNT() || MSDC_RXFIFO_CNT()) {
		MSDC_LOG(ERR, "register abnormal,please check! rxfifo = %u, txfifo = %u",
			 MSDC_RXFIFO_CNT(), MSDC_TXFIFO_CNT());
		/* msdc_dump_info(host->id); */
		MSDC_RESET_HW(host->id);
	}

	if (!data) {		/* *********************  CMD Only ********************* */

		if (msdc_do_command(host, cmd, 0, CMD_TIMEOUT) != 0) {
			goto done;
		}

		if ((host->hw->host_function == MSDC_EMMC) &&
			(cmd->opcode == MMC_ALL_SEND_CID))
			emmc_id = UNSTUFF_BITS(cmd->resp, 120, 8);

#ifdef ATC_EMMC_SUPPORT
		if ((host->hw->host_function == MSDC_EMMC) &&
		    (host->hw->boot == MSDC_BOOT_EN) &&
		    (cmd->opcode == MMC_SWITCH) && (((cmd->arg >> 16) & 0xFF) == EXT_CSD_PART_CONFIG)) {
			emmc_partition_access = (u8)((cmd->arg >> 8) & 0x07);
			MSDC_LOG(ERR, "Switch eMMC Access Partition: 0x%02X",
				 emmc_partition_access);
		}

#endif
	} else {		/* *********************  CMD With DAT  ********************* */
		BUG_ON(data->blksz > HOST_MAX_BLKSZ);

		data->error = 0;
		read = data->flags & MMC_DATA_READ ? 1 : 0;
		msdc_latest_operation_type[host->id] = read ? OPER_TYPE_READ : OPER_TYPE_WRITE;
		host->data = data;
		host->xfer_size = data->blocks * data->blksz;
		host->blksz = data->blksz;

		/* deside the transfer mode */
		if (drv_mode[host->id] == MODE_PIO) {
			host->dma_xfer = dma = 0;
			msdc_latest_transfer_mode[host->id] = TRAN_MOD_PIO;
		} else if (drv_mode[host->id] == MODE_DMA) {
			host->dma_xfer = dma = 1;
			msdc_latest_transfer_mode[host->id] = TRAN_MOD_DMA;
		} else if (drv_mode[host->id] == MODE_SIZE_DEP) {
			host->dma_xfer = dma = ((host->xfer_size >= dma_size[host->id]) ? 1 : 0);
			msdc_latest_transfer_mode[host->id] = dma ? TRAN_MOD_DMA : TRAN_MOD_PIO;
		}

		if (read) {
			if ((host->timeout_ns != data->timeout_ns) ||
			    (host->timeout_clks != data->timeout_clks)) {
				msdc_set_timeout(host, data->timeout_ns, data->timeout_clks);
			}

			msdc_hs_read_pre(host);
		} else {
			msdc_hs_write_pre(host);
		}


		msdc_set_blknum(host, data->blocks);
        //MSDC_CLR_FIFO();  /* no need */
#ifdef MTK_MSDC_USE_CMD23
		if (0 == (host->autocmd & MSDC_AUTOCMD23)) {
			/* start the cmd23 first, mrq->sbc is NULL with single r/w */
			if (mrq->sbc) {
				host->autocmd &= ~MSDC_AUTOCMD12;
				if (msdc_command_start(host, mrq->sbc, 0, CMD_TIMEOUT) != 0)
					goto done;

				/* then wait command done */
				if (msdc_command_resp_polling(host, mrq->sbc, 0, CMD_TIMEOUT))
					goto stop;
			} else {
				/* some sd card may not support cmd23,
				 * some emmc card have problem with cmd23, so use cmd12 here */
				if (host->hw->host_function != MSDC_SDIO)
					host->autocmd |= MSDC_AUTOCMD12;
			}
		} else {
			/* enable auto cmd23 */
			if (mrq->sbc) {
				host->autocmd &= ~MSDC_AUTOCMD12;
			} else {
				/* some sd card may not support cmd23,
				 * some emmc card have problem with cmd23, so use cmd12 here */
				if (host->hw->host_function != MSDC_SDIO) {
					host->autocmd &= ~MSDC_AUTOCMD23;
					host->autocmd |= MSDC_AUTOCMD12;
					l_card_no_cmd23 = 1;
				}
			}
		}
#endif    
		if (dma) {	/* =================== DMA RW ======================= */
			MSDC_DMA_ON();	/* enable DMA mode first!! */
			init_completion(&host->xfer_done);

#ifndef MTK_MSDC_USE_CMD23
			/* start the command first */
			if (host->hw->host_function != MSDC_SDIO)
				host->autocmd |= MSDC_AUTOCMD12;
#endif

			if (msdc_command_start(host, cmd, 0, CMD_TIMEOUT) != 0) {
				goto done;
			}

			dir = read ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
			nents = dma_map_sg(mmc_dev(mmc), data->sg, data->sg_len, dir);

			if (nents == 0) {
				MSDC_LOG(ERR, "invoke dma_map_sg failed, dma buffer error.");
			}

			map_sg = 1;

			/* then wait command done */
			if (msdc_command_resp_polling(host, cmd, 0, CMD_TIMEOUT) != 0) {	/* not tuning */
				goto stop;
			}

			/* for read, the data coming too fast, then CRC error, start DMA no business with CRC. */
			msdc_dma_setup(host, &host->dma, data->sg, data->sg_len);
			msdc_dma_start(host);

			spin_unlock(&host->lock);

			if (!wait_for_completion_timeout(&host->xfer_done, DAT_TIMEOUT)) {
				MSDC_LOG(ERR, "XXX CMD<%d> ARG<0x%x> wait xfer_done<%d> timeout!!",
					 cmd->opcode, cmd->arg, data->blocks * data->blksz);
				host->sw_timeout++;

				msdc_dump_info(host->id);
				data->error = CONV2UINT(-ETIMEDOUT);

				MSDC_RESET_HW(host->id);
			}

			spin_lock(&host->lock);
			msdc_dma_stop(host);

			if ((mrq->data && mrq->data->error)
				||(host->autocmd & MSDC_AUTOCMD12 && mrq->stop && mrq->stop->error)
				||(mrq->sbc && (mrq->sbc->error != 0)
				&& (host->autocmd & MSDC_AUTOCMD23))){
				MSDC_CLR_FIFO(host->id);
				MSDC_CLR_INT();
			}
		} else {	/* =================== PIO RW ======================= */
			/* Firstly: send command 
			 * need ask the designer, how about autocmd12
			 * or autocmd23 with pio mode
			 */
			host->autocmd &= ~MSDC_AUTOCMD12;

			l_autocmd23_is_set = 0;
			if (host->autocmd & MSDC_AUTOCMD23) {
				l_autocmd23_is_set = 1;
				host->autocmd &= ~MSDC_AUTOCMD23;
			}

			host->dma_xfer = 0;

			if (msdc_do_command(host, cmd, 0, CMD_TIMEOUT) != 0) {
				goto stop;
			}

			/* Secondly: pio data phase */
			if (read) {
				if (msdc_pio_read(host, data)) {
					goto stop;        /* need cmd12. */
				}
			} else {
				if (msdc_pio_write(host, data)) {
					goto stop;
				}
			}

			/* For write case: make sure contents in fifo flushed to device */
			if (!read) {
				pio_tmo = jiffies + DAT_TIMEOUT;

				while (1) {
					left = MSDC_TXFIFO_CNT();

					if (left == 0) {
						break;
					}

					if (msdc_pio_abort(host, data, pio_tmo)) {
						break;
						/* Fix me: what about if data error, when stop ? how to? */
					}
				}
			} else {
				/* Fix me: read case: need to check CRC error */
			}

			/* For write case: SDCBUSY and Xfer_Comp will assert when DAT0 not busy.
			   For read case : SDCBUSY and Xfer_Comp will assert when last byte read out from FIFO.
			 */

			/* try not to wait xfer_comp interrupt. the next command will check SDC_BUSY.
			   SDC_BUSY means xfer_comp assert */

		}		/* PIO mode */

stop:
		/* pio mode will disable autocmd23 */
		if (l_autocmd23_is_set == 1) {
			l_autocmd23_is_set = 0;
			host->autocmd |= MSDC_AUTOCMD23;
		}
#ifndef MTK_MSDC_USE_CMD23
				/* Last: stop transfer */
				if (data && data->stop) {
					if (!((cmd->error == 0) && (data->error == 0)
						  && (host->autocmd & MSDC_AUTOCMD12)
						  && (cmd->opcode == MMC_READ_MULTIPLE_BLOCK
						  || cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))) {
						if (msdc_do_command(host, data->stop, 0, CMD_TIMEOUT))
							goto done;
					}
				}
#else
				if (host->hw->host_function == MSDC_EMMC) {
					if (data && data->stop) {
						/* multi r/w with no cmd23 and no autocmd12,need send cmd12
						 * manual if PIO mode and autocmd23 enable, cmd12 need send,
						 * because autocmd23 is disable under PIO
						 */
						if ((((mrq->sbc == NULL) && !(host->autocmd & MSDC_AUTOCMD12))
							|| (!dma && mrq->sbc && (host->autocmd & MSDC_AUTOCMD23)))
							&& (cmd->opcode == MMC_READ_MULTIPLE_BLOCK
							 || cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK)) {
							if (msdc_do_command(host, data->stop, 0, CMD_TIMEOUT))
								goto done;
						}
					}
		
				} else {
					/* for non emmc card, use old flow */
					if (data && data->stop) {
						if (!((cmd->error == 0) && (data->error == 0)
							  && (host->autocmd & MSDC_AUTOCMD12)
							  && (cmd->opcode == MMC_READ_MULTIPLE_BLOCK
							  || cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))) {
							if (msdc_do_command(host, data->stop, 0, CMD_TIMEOUT))
								goto done;
						}
					}
				}
#endif
	}

done:
#ifdef MTK_MSDC_USE_CMD23
		/* for msdc use cmd23, but card not supported(sbc is NULL),
		 * need enable autocmd23 for next request
		 */
		if (1 == l_card_no_cmd23) {
			if (host->hw->host_function != MSDC_SDIO) {
				host->autocmd |= MSDC_AUTOCMD23;
				host->autocmd &= ~MSDC_AUTOCMD12;
				l_card_no_cmd23 = 0;
			}
		}
#endif

	if (data != NULL) {
		host->data = NULL;
		host->dma_xfer = 0;

		if (dma != 0) {
			MSDC_DMA_OFF();
			host->dma.used_bd = 0;
			host->dma.used_gpd = 0;

			if (map_sg == 1) {
				/*if(data->error == 0){
				   int retry = 3;
				   int count = 1000;
				   MSDC_RETRY(host->dma.gpd->hwo,retry,count,host->id);
				   } */
				dma_unmap_sg(mmc_dev(mmc), data->sg, data->sg_len, dir);
			}
		}

#ifdef ATC_EMMC_SUPPORT

		/* Save EXT_CSD Copy in driver */
		if ((cmd->opcode == MMC_SEND_EXT_CSD) && (cmd->error == 0) && (data->error == 0)) {
			msdc_get_data(emmc_ext_csd, data);
			/* MSDC_LOG(ERR, "Save eMMC EXT_CSD"); */
		}

#endif

		host->blksz = 0;

		MSDC_LOG(OPS, "CMD<%d> data<%s %s> blksz<%d> block<%d> error<%d>", cmd->opcode,
			 (dma ? "dma" : "pio"), (read ? "read " : "write"), data->blksz,
			 data->blocks, data->error);

		if (!(IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ))) {
			if ((cmd->opcode != 17) && (cmd->opcode != 18) && (cmd->opcode != 24) &&
				(cmd->opcode != 25)) {
				MSDC_LOG(NRW, "CMD<%2d> arg<0x%08x> Resp<0x%08x> data<%s> size<%d>",
					 cmd->opcode, cmd->arg, cmd->resp[0],
					 (read ? "read " : "write"), data->blksz * data->blocks);
			} else {
				MSDC_LOG(RW, "CMD<%2d> arg<0x%08x> Resp<0x%08x> block<%d>",
					 cmd->opcode, cmd->arg, cmd->resp[0], data->blocks);
			}
		}
	} else {
		if (!(IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ))) {
			if (cmd->opcode != 13) {	/* by pass CMD13 */
				MSDC_LOG(NRW,
					 "CMD<%2d> arg<0x%08X> resp<0x%08x 0x%08x 0x%08x 0x%08x>",
					 cmd->opcode, cmd->arg, cmd->resp[0], cmd->resp[1],
					 cmd->resp[2], cmd->resp[3]);
			}
		}
	}

	if (mrq->cmd->error == CONV2UINT(-EIO)) {
		host->error |= REQ_CMD_EIO;
		sdio_tune_flag |= 0x1;

		if (mrq->cmd->opcode == SD_IO_RW_EXTENDED) {
			sdio_tune_flag |= 0x1;
		}
	}

	if (mrq->cmd->error == CONV2UINT(-ETIMEDOUT)) {
		host->error |= REQ_CMD_TMO;
	}

	if (mrq->data && mrq->data->error) {
		host->error |= REQ_DAT_ERR;

		sdio_tune_flag |= 0x10;

		if (mrq->data->flags & MMC_DATA_READ) {
			sdio_tune_flag |= 0x80;
		} else {
			sdio_tune_flag |= 0x40;
		}
	}
#ifdef MTK_MSDC_USE_CMD23
	if (mrq->sbc && (mrq->sbc->error == (unsigned int)-EIO))
		host->error |= REQ_CMD_EIO;
	if (mrq->sbc && (mrq->sbc->error == (unsigned int)-ETIMEDOUT)) 
		host->error |= REQ_CMD_TMO;	
#endif
	if (mrq->stop && (mrq->stop->error == CONV2UINT(-EIO))) {
		host->error |= REQ_STOP_EIO;
	}

	if (mrq->stop && (mrq->stop->error == CONV2UINT(-ETIMEDOUT))) {
		host->error |= REQ_STOP_TMO;
	}

	/* if (host->error) MSDC_LOG(ERR, "host->error<%d>", host->error); */

	return host->error;
}


static int msdc_tune_rw_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_command *cmd;
	struct mmc_data *data;
#ifdef MTK_MSDC_USE_CMD23
	u32 l_autocmd23_is_set = 0;
#endif

	u32 base = host->base;
	int read = 1, dma = 1;

	BUG_ON(mmc == NULL);
	BUG_ON(mrq == NULL);

	/* host->error = 0; */
	atomic_set(&host->abort, 0);

	cmd = mrq->cmd;
	data = mrq->cmd->data;

	/* check msdc is work ok. rule is RX/TX fifocnt must be zero after last request
	 * if find abnormal, try to reset msdc first
	 */
	if (MSDC_TXFIFO_CNT() || MSDC_RXFIFO_CNT()) {
		MSDC_LOG(ERR, "register abnormal,please check!");
		MSDC_RESET_HW(host->id);
	}

	BUG_ON(data->blksz > HOST_MAX_BLKSZ);

	data->error = 0;
	read = data->flags & MMC_DATA_READ ? 1 : 0;
	msdc_latest_operation_type[host->id] = read ? OPER_TYPE_READ : OPER_TYPE_WRITE;
	host->data = data;
	host->xfer_size = data->blocks * data->blksz;
	host->blksz = data->blksz;
	host->dma_xfer = 1;

	if (read) {
		if ((host->timeout_ns != data->timeout_ns) ||
		    (host->timeout_clks != data->timeout_clks)) {
			msdc_set_timeout(host, data->timeout_ns, data->timeout_clks);
		}
	}

	msdc_set_blknum(host, data->blocks);
	MSDC_DMA_ON();		/* enable DMA mode first!! */
	init_completion(&host->xfer_done);

	/* start the command first */
#ifndef MTK_MSDC_USE_CMD23
		if (host->hw->host_function != MSDC_SDIO)
			host->autocmd |= MSDC_AUTOCMD12;
#else
		if (host->hw->host_function != MSDC_SDIO) {
			host->autocmd |= MSDC_AUTOCMD12;
	
			/* disable autocmd23 in error tuning flow */
			l_autocmd23_is_set = 0;
			if (host->autocmd & MSDC_AUTOCMD23) {
				l_autocmd23_is_set = 1;
				host->autocmd &= ~MSDC_AUTOCMD23;
			}
		}
#endif

	if (msdc_command_start(host, cmd, 0, CMD_TIMEOUT) != 0) {
		goto done;
	}

	/* then wait command done */
	if (msdc_command_resp_polling(host, cmd, 0, CMD_TIMEOUT) != 0) {	/* not tuning */
		goto stop;
	}

	/* for read, the data coming too fast, then CRC error
	   start DMA no business with CRC. */
	msdc_dma_setup(host, &host->dma, data->sg, data->sg_len);
	msdc_dma_start(host);
	/* MSDC_LOG(ERR, "1.Power cycle enable(%d)",host->power_cycle_enable); */

	spin_unlock(&host->lock);

	if (!wait_for_completion_timeout(&host->xfer_done, DAT_TIMEOUT)) {
		MSDC_LOG(ERR, "XXX CMD<%d> ARG<0x%x> wait xfer_done<%d> timeout!!", cmd->opcode,
			 cmd->arg, data->blocks * data->blksz);
		host->sw_timeout++;

		/* Check dram bus status */
		//msdc_check_dram_bus_status(host);
		msdc_dump_info(host->id);
		data->error = CONV2UINT(-ETIMEDOUT);
		MSDC_RESET_HW(host->id);
	}

	spin_lock(&host->lock);
	/* MSDC_LOG(ERR, "2.Power cycle enable(%d)",host->power_cycle_enable); */
	msdc_dma_stop(host);

		if ((mrq->data && mrq->data->error)
	    || (host->autocmd & MSDC_AUTOCMD12 && mrq->stop && mrq->stop->error)
	    || (mrq->sbc && (mrq->sbc->error != 0)
	    && (host->autocmd & MSDC_AUTOCMD23))) {
			MSDC_CLR_FIFO(host->id); 
			MSDC_CLR_INT(); 
		}
        	

stop:
	/* Last: stop transfer */

        if (data->stop){ 
			if(!((cmd->error == 0) && (data->error == 0) && (host->autocmd & MSDC_AUTOCMD12) && (cmd->opcode == MMC_READ_MULTIPLE_BLOCK || cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))){
            	if (msdc_do_command(host, data->stop, 0, CMD_TIMEOUT) != 0) {
                	goto done; 
            	}
			}
        } 

done:
	host->data = NULL;
	host->dma_xfer = 0;
	MSDC_DMA_OFF();
	host->dma.used_bd = 0;
	host->dma.used_gpd = 0;
	host->blksz = 0;

	MSDC_LOG(OPS, "CMD<%d> data<%s %s> blksz<%d> block<%d> error<%d>", cmd->opcode,
		 (dma ? "dma" : "pio"), (read ? "read " : "write"), data->blksz, data->blocks,
		 data->error);

	if (!(IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ))) {
		if ((cmd->opcode != 17) && (cmd->opcode != 18) && (cmd->opcode != 24)
		    && (cmd->opcode != 25)) {
			MSDC_LOG(NRW, "CMD<%3d> arg<0x%8x> Resp<0x%8x> data<%s> size<%d>",
				 cmd->opcode, cmd->arg, cmd->resp[0], (read ? "read " : "write"),
				 data->blksz * data->blocks);
		} else {
			MSDC_LOG(RW, "CMD<%3d> arg<0x%8x> Resp<0x%8x> block<%d>", cmd->opcode,
				 cmd->arg, cmd->resp[0], data->blocks);
		}
	} else {
		if (!(IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ))) {
			if (cmd->opcode != 13) {	/* by pass CMD13 */
				MSDC_LOG(NRW, "CMD<%3d> arg<0x%8x> resp<%8x %8x %8x %8x>",
					 cmd->opcode, cmd->arg, cmd->resp[0], cmd->resp[1],
					 cmd->resp[2], cmd->resp[3]);
			}
		}
	}

	host->error = 0;

	if (mrq->cmd->error == CONV2UINT(-EIO)) {
		host->error |= REQ_CMD_EIO;
	}

	if (mrq->cmd->error == CONV2UINT(-ETIMEDOUT)) {
		host->error |= REQ_CMD_TMO;
	}

	if (mrq->data && (mrq->data->error)) {
		host->error |= REQ_DAT_ERR;
	}

	if (mrq->stop && (mrq->stop->error == CONV2UINT(-EIO))) {
		host->error |= REQ_STOP_EIO;
	}

	if (mrq->stop && (mrq->stop->error == CONV2UINT(-ETIMEDOUT))) {
		host->error |= REQ_STOP_TMO;
	}
#ifdef MTK_MSDC_USE_CMD23
	if (l_autocmd23_is_set == 1) {
		/* restore the value */
		host->autocmd |= MSDC_AUTOCMD23;
	}
#endif

	return host->error;
}

static void msdc_pre_req(struct mmc_host *mmc, struct mmc_request *mrq, bool is_first_req)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_data *data;
	struct mmc_command *cmd = mrq->cmd;
	int read = 1, dir = DMA_FROM_DEVICE;
	int nents = 0;		/* number of buffers to map */

	BUG_ON(!cmd);
	data = mrq->data;

	if (data) {
		data->host_cookie = 0;
	}

	if (data
	    && ((cmd->opcode == MMC_READ_SINGLE_BLOCK) || (cmd->opcode == MMC_READ_MULTIPLE_BLOCK)
		|| (cmd->opcode == MMC_WRITE_BLOCK) || (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))) {
		host->xfer_size = data->blocks * data->blksz;
		read = data->flags & MMC_DATA_READ ? 1 : 0;

		/* Select transfer mode */
		if (drv_mode[host->id] == MODE_PIO) {
			data->host_cookie = 0;
			msdc_latest_transfer_mode[host->id] = TRAN_MOD_PIO;
		} else if (drv_mode[host->id] == MODE_DMA) {
			data->host_cookie = 1;
			msdc_latest_transfer_mode[host->id] = TRAN_MOD_DMA;
		} else if (drv_mode[host->id] == MODE_SIZE_DEP) {
			data->host_cookie = ((host->xfer_size >= dma_size[host->id]) ? 1 : 0);
			msdc_latest_transfer_mode[host->id] =
				data->host_cookie ? TRAN_MOD_DMA : TRAN_MOD_PIO;
		}

		if (data->host_cookie) {
			dir = read ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
			nents = dma_map_sg(mmc_dev(mmc), data->sg, data->sg_len, dir);

			if (nents == 0) {
				MSDC_LOG(ERR, "invoke dma_map_sg failed, dma buffer error.");
			}
		}

		MSDC_LOG(OPS, "CMD<%d> ARG<0x%x>data<%s %s> blksz<%d> block<%d> error<%d>",
			 mrq->cmd->opcode, mrq->cmd->arg, (data->host_cookie ? "dma" : "pio"),
			 (read ? "read " : "write"), data->blksz, data->blocks, data->error);
	}
}


static void msdc_dma_clear(struct msdc_host *host)
{
	u32 base = host->base;

	host->data = NULL;
	host->mrq = NULL;
	host->dma_xfer = 0;
	MSDC_DMA_OFF();
	host->dma.used_bd = 0;
	host->dma.used_gpd = 0;
	host->blksz = 0;
}

static void msdc_post_req(struct mmc_host *mmc, struct mmc_request *mrq, int err)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_data *data;
	/* struct mmc_command *cmd = mrq->cmd; */
	int read = 1, dir = DMA_FROM_DEVICE;

	data = mrq->data;

	if (data && data->host_cookie) {
		host->xfer_size = data->blocks * data->blksz;
		read = data->flags & MMC_DATA_READ ? 1 : 0;
		dir = read ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
		dma_unmap_sg(mmc_dev(mmc), data->sg, data->sg_len, dir);
		data->host_cookie = 0;
		MSDC_LOG(OPS, "CMD<%d> ARG<0x%x> blksz<%d> block<%d> error<%d>", mrq->cmd->opcode,
			 mrq->cmd->arg, data->blksz, data->blocks, data->error);
	}

	return;

}


static int msdc_do_request_async(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_command *cmd;
	struct mmc_data *data;
	u32 base = host->base;
#ifdef MTK_MSDC_USE_CMD23
		u32 l_card_no_cmd23 = 0;
#endif

	int dma = 0, read = 1;

	MVG_EMMC_DECLARE_INT32(delay_ns);
	MVG_EMMC_DECLARE_INT32(delay_us);
	MVG_EMMC_DECLARE_INT32(delay_ms);

	BUG_ON(mmc == NULL);
	BUG_ON(mrq == NULL);

#ifdef MSDC_POWER_FAIL_WP
	if (!IS_CARD_PRESENT(host) || host->power_mode == MMC_POWER_OFF || 
			(host->protected == host->protect_init && host->id == 0 && (mrq->cmd->opcode == 24 || mrq->cmd->opcode == 25))) {
		MSDC_LOG_NORMAL(pr_info, "[%d]cmd<%d> arg<0x%x> card<%d> power<%d> emmc protected!", host->id, mrq->cmd->opcode,mrq->cmd->arg, IS_CARD_PRESENT(host), host->power_mode);
#else
	if (!IS_CARD_PRESENT(host) || host->power_mode == MMC_POWER_OFF){
		MSDC_LOG_NORMAL(pr_info, "[%d]cmd<%d> arg<0x%x> card<%d> power<%d> ", host->id, mrq->cmd->opcode,mrq->cmd->arg, IS_CARD_PRESENT(host), host->power_mode);
#endif
		mrq->cmd->error = (unsigned int)-ENOMEDIUM;
		if(mrq->done)
			mrq->done(mrq);         // call done directly.
		return 0;
	}

	SET_TUNE_STS(host, TUNE_STS_NOT_IN_TUNE);
	host->error = 0;
	atomic_set(&host->abort, 0);

	spin_lock(&host->lock);
	cmd = mrq->cmd;
	data = mrq->cmd->data;
	host->mrq = mrq;

	/* check msdc is work ok. rule is RX/TX fifocnt must be zero after last request
	 * if find abnormal, try to reset msdc first
	 */
	if (MSDC_TXFIFO_CNT() || MSDC_RXFIFO_CNT()) {
		MSDC_LOG(ERR, "register abnormal,please check!");
		MSDC_RESET_HW(host->id);
	}

	BUG_ON(data == NULL);
	BUG_ON(data->blksz > HOST_MAX_BLKSZ);

	data->error = 0;
	read = data->flags & MMC_DATA_READ ? 1 : 0;
	msdc_latest_operation_type[host->id] = read ? OPER_TYPE_READ : OPER_TYPE_WRITE;
	host->data = data;
	host->xfer_size = data->blocks * data->blksz;
	host->blksz = data->blksz;
	host->dma_xfer = 1;

	if (read) {
		if ((host->timeout_ns != data->timeout_ns) ||
		    (host->timeout_clks != data->timeout_clks)) {
			msdc_set_timeout(host, data->timeout_ns, data->timeout_clks);
		}

		msdc_hs_read_pre(host);
	} else {
		msdc_hs_write_pre(host);
	}

	msdc_set_blknum(host, data->blocks);
	MSDC_DMA_ON();		/* enable DMA mode first!! */
#ifdef MTK_MSDC_USE_CMD23
		/* if tuning flow run here, no problem?? need check!!!!!!! */
		if (0 == (host->autocmd & MSDC_AUTOCMD23)) {
			/* start the cmd23 first */
			if (mrq->sbc) {
				host->autocmd &= ~MSDC_AUTOCMD12;
	
				if (msdc_command_start(host, mrq->sbc, 0, CMD_TIMEOUT) != 0)
					goto done;
	
				/* then wait command done */
				if (msdc_command_resp_polling(host, mrq->sbc, 0, CMD_TIMEOUT) != 0)
					goto stop;
			} else {
				/* some sd card may not support cmd23,
				 * some emmc card have problem with cmd23, so use cmd12 here */
				if (host->hw->host_function != MSDC_SDIO)
					host->autocmd |= MSDC_AUTOCMD12;
			}
		} else {
			if (mrq->sbc) {
				host->autocmd &= ~MSDC_AUTOCMD12;

			} else {
				/* some sd card may not support cmd23,
				 * some emmc card have problem with cmd23, so use cmd12 here */
				if (host->hw->host_function != MSDC_SDIO) {
					host->autocmd &= ~MSDC_AUTOCMD23;
					host->autocmd |= MSDC_AUTOCMD12;
					l_card_no_cmd23 = 1;
				}
			}
		}
	
#else
		/* start the command first */
		if (host->hw->host_function != MSDC_SDIO)
			host->autocmd |= MSDC_AUTOCMD12;
#endif	

	if (msdc_command_start(host, cmd, 0, CMD_TIMEOUT) != 0) {
		goto done;
	}

	/* then wait command done */
	if (msdc_command_resp_polling(host, cmd, 0, CMD_TIMEOUT) != 0) {	/* not tuning. */
		goto stop;
	}

	/* for read, the data coming too fast, then CRC error, start DMA no business with CRC. */
	msdc_dma_setup(host, &host->dma, data->sg, data->sg_len);
	msdc_dma_start(host);
	/* MSDC_LOG(ERR, "0.Power cycle enable(%d)",host->power_cycle_enable); */
	MVG_EMMC_WRITE_MATCH(host, (u64)cmd->arg, delay_ms, delay_us, delay_ns, cmd->opcode, host->xfer_size);
	spin_unlock(&host->lock);
#ifdef MTK_MSDC_USE_CMD23
	/* for msdc use cmd23, but card not supported(sbc is NULL),
	 * need enable autocmd23 for next request.
	 */
	if (1 == l_card_no_cmd23) {
		if (host->hw->host_function != MSDC_SDIO) {
			host->autocmd |= MSDC_AUTOCMD23;
			host->autocmd &= ~MSDC_AUTOCMD12;
			l_card_no_cmd23 = 0;
		}
	}
#endif
	return 0;


stop:
#ifndef MTK_MSDC_USE_CMD23
		/* Last: stop transfer */
		if (data && data->stop) {
			if (!((cmd->error == 0) && (data->error == 0)
				&& (host->autocmd & MSDC_AUTOCMD12)
				&& (cmd->opcode == MMC_READ_MULTIPLE_BLOCK
				|| cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))) {
				if (msdc_do_command(host, data->stop, 0, CMD_TIMEOUT) != 0)
					goto done;
			}
		}
#else
	
		if (host->hw->host_function == MSDC_EMMC) {
			/* error handle will do msdc_abort_data() */
		} else {
			if (data && data->stop) {
				if (!((cmd->error == 0) && (data->error == 0)
					  && (host->autocmd & MSDC_AUTOCMD12)
					  && (cmd->opcode == MMC_READ_MULTIPLE_BLOCK
					  || cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))) {
					if (msdc_do_command(host, data->stop, 0, CMD_TIMEOUT) != 0)
						goto done;
				}
			}
		}
#endif
done:
#ifdef MTK_MSDC_USE_CMD23
		/* for msdc use cmd23, but card not supported(sbc is NULL),
		 * need enable autocmd23 for next request
		 */
		if (1 == l_card_no_cmd23) {
			if (host->hw->host_function != MSDC_SDIO) {
				host->autocmd |= MSDC_AUTOCMD23;
				host->autocmd &= ~MSDC_AUTOCMD12;
				l_card_no_cmd23 = 0;
			}
		}
#endif

	msdc_dma_clear(host);

	MSDC_LOG(OPS, "CMD<%d> data<%s %s> blksz<%d> block<%d> error<%d>", cmd->opcode,
		 (dma ? "dma" : "pio"), (read ? "read " : "write"), data->blksz, data->blocks,
		 data->error);

	if (!(IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ))) {
		if ((cmd->opcode != 17) && (cmd->opcode != 18) && (cmd->opcode != 24)
		    && (cmd->opcode != 25)) {
			MSDC_LOG(NRW, "CMD<%3d> arg<0x%8x> Resp<0x%8x> data<%s> size<%d>",
				 cmd->opcode, cmd->arg, cmd->resp[0], (read ? "read " : "write"),
				 data->blksz * data->blocks);
		} else
			MSDC_LOG(RW, "CMD<%3d> arg<0x%8x> Resp<0x%8x> block<%d>", cmd->opcode,
				 cmd->arg, cmd->resp[0], data->blocks);
	}

#ifdef MTK_MSDC_USE_CMD23
	if (mrq->sbc && (mrq->sbc->error == (unsigned int)-EIO))
		host->error |= REQ_CMD_EIO;
	if (mrq->sbc && (mrq->sbc->error == (unsigned int)-ETIMEDOUT)) 
		host->error |= REQ_CMD_TMO;
#endif

	if (mrq->cmd->error == CONV2UINT(-EIO)) {
		host->error |= REQ_CMD_EIO;
	}

	if (mrq->cmd->error == CONV2UINT(-ETIMEDOUT)) {
		host->error |= REQ_CMD_TMO;
	}

	if (mrq->stop && (mrq->stop->error == CONV2UINT(-EIO))) {
		host->error |= REQ_STOP_EIO;
	}

	if (mrq->stop && (mrq->stop->error == CONV2UINT(-ETIMEDOUT))) {
		host->error |= REQ_STOP_TMO;
	}

	if (mrq->done) {
		mrq->done(mrq);
	}

	spin_unlock(&host->lock);

	return host->error;
}

static int msdc_app_cmd(struct mmc_host *mmc, struct msdc_host *host)
{
	struct mmc_command cmd = { 0 };
	struct mmc_request mrq = { 0 };
	u32 err = -1;

	cmd.opcode = MMC_APP_CMD;
	cmd.arg = host->app_cmd_arg;	/* meet mmc->card is null when ACMD6 */
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;

	mrq.cmd = &cmd;
	cmd.mrq = &mrq;
	cmd.data = NULL;

	err = msdc_do_command(host, &cmd, 0, CMD_TIMEOUT);
	return err;
}

static int msdc_lower_onlyfreq(struct msdc_host *host)
{
	u32 div, mode;
	u32 base = host->base;

	MSDC_LOG(ERR, "------------------>>> need to lower only freq <<<---------------------");

	msdc_reset_tune_counter(host, all_counter);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, mode);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, div);

	if (div >= MSDC_MAX_FREQ_DIV) {
		MSDC_LOG(ERR, "but, div<%d> power tuning", div);
		return 1;
	} else if (mode == 1) {
		mode = 0;
		msdc_clk_stable(host, mode, div);
		host->sclk = (div == 0) ? hclksrc[host->hw->clk_src] / 2 :
			     hclksrc[host->hw->clk_src] / (4 * div);

		MSDC_LOG(ERR, "new div<%d>, mode<%d> new freq.<%dKHz>", div, mode,
			 host->sclk / 1000);
		return 0;
	}

	msdc_clk_stable(host, mode, div + 1);
	host->sclk =
		(mode == 2) ? hclksrc[host->hw->clk_src] / (2 * 4 * (div + 1)) :
		hclksrc[host->hw->clk_src] / (4 * (div + 1));
	MSDC_LOG(ERR, "new div<%d>, mode<%d> new freq.<%dKHz>", div + 1, mode,
		 host->sclk / 1000);
	return 0;
}

static int msdc_lower_freq(struct msdc_host *host)
{
	u32 div, mode;
	u32 base = host->base;

	MSDC_LOG(ERR, "----------->>> need to lower freq <<<-------------");

	msdc_reset_tune_counter(host, all_counter);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, mode);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, div);

	if (div >= MSDC_MAX_FREQ_DIV) {
		MSDC_LOG(ERR, "but, div<%d> power tuning", div);
		return msdc_power_tuning(host);
	} else if (mode == 1) {
		mode = 0;
		msdc_clk_stable(host, mode, div);
		host->sclk =
			(div ==
			 0) ? hclksrc[host->hw->clk_src] / 2 : hclksrc[host->hw->clk_src] / (4 * div);

		MSDC_LOG(ERR, "new div<%d>, mode<%d> new freq.<%dKHz>", div, mode,
			 host->sclk / 1000);
		return 0;
	}

	msdc_clk_stable(host, mode, div + 1);
	host->sclk =
		(mode == 2) ? hclksrc[host->hw->clk_src] / (2 * 4 * (div + 1)) :
		hclksrc[host->hw->clk_src] / (4 * (div + 1));
	MSDC_LOG(ERR, "new div<%d>, mode<%d> new freq.<%dKHz>", div + 1, mode,
		 host->sclk / 1000);
	return 0;
}


int msdc_tune_cmdrsp_each_dat_line(struct msdc_host *host)
{
	int result = 0;
	u32 base = host->base;
	u32 sel = 0;
	u32 cur_rsmpl = 0, orig_rsmpl;
	u32 cur_rrdly = 0, orig_rrdly;
	u32 cur_cntr = 0, orig_cmdrtc;
	u32 cur_dl_cksel = 0, orig_dl_cksel;


	/*********************************/
#if 0
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, orig_rrdly);
	MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, orig_cmdrtc);
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
	MSDC_LOG(TUNE, "TUNE_CMD: rsmpl<%d> rrdly<%d> cmdrtc<%d> dl_cksel<%d> sfreq.<%d>",
		 orig_rsmpl, orig_rrdly, orig_cmdrtc, orig_dl_cksel, host->sclk);
#endif
	/*********************************/

	/* get current command respone sample edge */
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);

	/* get current command pad delay (0-31) */
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, orig_rrdly);

	/* get current command response turn around period */
	MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, orig_cmdrtc);

	/* get current internal clock delay */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

	if (host->mclk >= MSDC_CLK_100MHZ) {	/* 100MHz */
		sel = 1;
	} else {
		/* set command response turn around period, only useful for UHS104, so set it to default value (1) */
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, 1);
		/* set clock latch to 0. Max Xia Why ??? */
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, 0);
	}

	/* change command response sample edge */
	cur_rsmpl = (orig_rsmpl + 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cur_rsmpl % 2);

	/* In SD/eMMC init flow, fix rising edge for latching cmd response */
	if (host->mclk <= MSDC_CLK_400KHZ) {	/* 400KHz */
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
		cur_rsmpl = 2;
	}

	/* after try to send command both rising edge and falling edge, change delay */
	if (cur_rsmpl >= 2) {
		/* change command pad RX delay */
		cur_rrdly = (orig_rrdly + 1);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, cur_rrdly % 32);
	}

	/* command response pad delay reach to MAX and clock higher than 100MHz */
	if ((cur_rrdly >= 32) && (sel)) {
		/* change command response turn around period */
		cur_cntr = (orig_cmdrtc + 1);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, cur_cntr % 8);
	}

	/* command response turn around period reach to MAX and clock higher than 100MHz */
	if ((cur_cntr >= 8) && (sel)) {
		/* change command response internal latch clock delay */
		cur_dl_cksel = (orig_dl_cksel + 1);
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel % 8);
	}

	/* increase command tune counter */
	++(host->t_counter.time_cmd);

	/* clock higher than 100MHz, tune time counter reach to MAX */
	if ((sel && (host->t_counter.time_cmd == CMD_TUNE_UHS_MAX_TIME)) ||
	    /* clock lower than 100MHz, turn time counter reach to MAX */
	    ((sel == 0) && (host->t_counter.time_cmd == CMD_TUNE_HS_MAX_TIME))) {
#if MSDC_LOWER_FREQ
		result = msdc_lower_freq(host);
#else
		result = 1;
#endif
		host->t_counter.time_cmd = 0;
	}

#if TUNE_PARAMS_DETAILS
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, orig_rrdly);
	MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, orig_cmdrtc);
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
	MSDC_LOG(TUNE, "TUNE_CMD: rsmpl<%d> rrdly<%d> cmdrtc<%d> dl_cksel<%d> sfreq.<%d>",
		 orig_rsmpl, orig_rrdly, orig_cmdrtc, orig_dl_cksel, host->sclk);
#endif

	return result;
}

static int msdc_tune_read_each_dat_line(struct msdc_host *host)
{
	u32 base = host->base;
	u32 sel = 0;
	u32 ddr = 0;
	u32 dcrc;
	u32 clkmode = 0;
	u32 cur_rxdly0, cur_rxdly1;
	u32 cur_dsmpl = 0, orig_dsmpl;
	u32 cur_dsel = 0, orig_dsel;
	u32 cur_dl_cksel = 0, orig_dl_cksel;
	u32 cur_dat0 = 0, cur_dat1 = 0, cur_dat2 = 0, cur_dat3 = 0,
	    cur_dat4 = 0, cur_dat5 = 0, cur_dat6 = 0, cur_dat7 = 0;
	u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3, orig_dat4, orig_dat5, orig_dat6, orig_dat7;
	int result = 0;

	if (host->mclk >= MSDC_CLK_100MHZ) {	/* 100MHz */
		sel = 1;
	} else {
		/* set CKBUF in CKGEN Delay to 0 */
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, 0);
	}

	/* get CKMOD: 00->divider mode, 01->no divider mode, 02 -> DDR mode */
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, clkmode);
	ddr = (clkmode == 2) ? 1 : 0;

	/* get current CKBUF in CKGEN Delay, 32 stages */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orig_dsel);

	/* get current Data Latch Delay, 8 stages */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

	/* get current Read Data Sample Edge, rising edge or falling edge */
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);

	/* set each data line has its own delay selection */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);


	/* change Read Data Sample Edge */
	cur_dsmpl = (orig_dsmpl + 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_dsmpl % 2);

	/* after try to read data both rising edge and falling edge, change delay */
	if (cur_dsmpl >= 2) {
		/* get value of Data CRC Status */
		MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS | SDC_DCRC_STS_NEG, dcrc);

		/* if it is not DDR mode, mask unconcerned bits */
		if (!ddr) {
			dcrc &= ~SDC_DCRC_STS_NEG;
		}

		/* get current data line delay */
		cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
		cur_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);

		orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
		orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
		orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
		orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;
		orig_dat4 = (cur_rxdly1 >> 24) & 0x1F;
		orig_dat5 = (cur_rxdly1 >> 16) & 0x1F;
		orig_dat6 = (cur_rxdly1 >> 8) & 0x1F;
		orig_dat7 = (cur_rxdly1 >> 0) & 0x1F;

		if (ddr) {
			/* DDR mode, if data line occur CRC error, delay = delay + 1; */
			cur_dat0 = (dcrc & (1 << 0)
				    || dcrc & (1 << 8)) ? (orig_dat0 + 1) : orig_dat0;
			cur_dat1 = (dcrc & (1 << 1)
				    || dcrc & (1 << 9)) ? (orig_dat1 + 1) : orig_dat1;
			cur_dat2 = (dcrc & (1 << 2)
				    || dcrc & (1 << 10)) ? (orig_dat2 + 1) : orig_dat2;
			cur_dat3 = (dcrc & (1 << 3)
				    || dcrc & (1 << 11)) ? (orig_dat3 + 1) : orig_dat3;
			cur_dat4 = (dcrc & (1 << 4)
				    || dcrc & (1 << 12)) ? (orig_dat4 + 1) : orig_dat4;
			cur_dat5 = (dcrc & (1 << 5)
				    || dcrc & (1 << 13)) ? (orig_dat5 + 1) : orig_dat5;
			cur_dat6 = (dcrc & (1 << 6)
				    || dcrc & (1 << 14)) ? (orig_dat6 + 1) : orig_dat6;
			cur_dat7 = (dcrc & (1 << 7)
				    || dcrc & (1 << 15)) ? (orig_dat7 + 1) : orig_dat7;
		} else {
			/* SDR mode, if data line occur CRC error, delay = delay + 1; */
			cur_dat0 = (dcrc & (1 << 0)) ? (orig_dat0 + 1) : orig_dat0;
			cur_dat1 = (dcrc & (1 << 1)) ? (orig_dat1 + 1) : orig_dat1;
			cur_dat2 = (dcrc & (1 << 2)) ? (orig_dat2 + 1) : orig_dat2;
			cur_dat3 = (dcrc & (1 << 3)) ? (orig_dat3 + 1) : orig_dat3;
			cur_dat4 = (dcrc & (1 << 4)) ? (orig_dat4 + 1) : orig_dat4;
			cur_dat5 = (dcrc & (1 << 5)) ? (orig_dat5 + 1) : orig_dat5;
			cur_dat6 = (dcrc & (1 << 6)) ? (orig_dat6 + 1) : orig_dat6;
			cur_dat7 = (dcrc & (1 << 7)) ? (orig_dat7 + 1) : orig_dat7;
		}

		/* write back new delay setting */
		cur_rxdly0 = ((cur_dat0 & 0x1F) << 24) | ((cur_dat1 & 0x1F) << 16) |
			     ((cur_dat2 & 0x1F) << 8) | ((cur_dat3 & 0x1F) << 0);
		cur_rxdly1 = ((cur_dat4 & 0x1F) << 24) | ((cur_dat5 & 0x1F) << 16) |
			     ((cur_dat6 & 0x1F) << 8) | ((cur_dat7 & 0x1F) << 0);

		MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
		MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1);

	}

	/* if one data line delay is reach to MAX delay */
	if ((cur_dat0 >= 32) || (cur_dat1 >= 32) || (cur_dat2 >= 32) || (cur_dat3 >= 32) ||
	    (cur_dat4 >= 32) || (cur_dat5 >= 32) || (cur_dat6 >= 32) || (cur_dat7 >= 32)) {
		if (sel) {	/* clock higher than 100MHz */
			/* reset data line delay to 0 */
			MSDC_WRITE32(MSDC_DAT_RDDLY0, 0);
			MSDC_WRITE32(MSDC_DAT_RDDLY1, 0);

			/* change CKBUF in CKGEN Delay */
			cur_dsel = (orig_dsel + 1);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, cur_dsel % 32);
		}
	}

	/* CKBUF in CKGEN Delay reach MAX delay */
	if (cur_dsel >= 32) {
		/* if use divider mode and clock high than 100MHz */
		if ((clkmode == 1) && sel) {
			cur_dl_cksel = (orig_dl_cksel + 1);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
				       cur_dl_cksel % 8);
		}
	}

	/* increase the tune counter */
	++(host->t_counter.time_read);

	/* clock higher than 100MHz, SDR Direct Mode, tune time counter reach to MAX */
	if (((sel == 1) && (clkmode == 1) && (host->t_counter.time_read == READ_TUNE_UHS_CLKMOD1_MAX_TIME)) ||
	    /* clock higher than 100MHz, SDR Div Mode or DDR mode, tune time counter reach to MAX */
	    ((sel == 1) && ((clkmode == 0) || (clkmode == 2)) && (host->t_counter.time_read == READ_TUNE_UHS_MAX_TIME)) ||
	    /* clock lower than 100MHz, SDR Div Mode or DDR mode, tune time counter reach to MAX */
	    ((sel == 0) && ((clkmode == 0) || (clkmode == 2)) && (host->t_counter.time_read == READ_TUNE_HS_MAX_TIME))) {
#if MSDC_LOWER_FREQ
		result = msdc_lower_freq(host);
#else
		result = 1;
#endif
		host->t_counter.time_read = 0;
	}

#if TUNE_PARAMS_DETAILS
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orig_dsel);
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);
	cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
	cur_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);
	MSDC_LOG(TUNE,
		 "TUNE_READ: dsmpl<%d> rxdly0<0x%x> rxdly1<0x%x> dsel<%d> dl_cksel<%d> sfreq.<%d>",
		 orig_dsmpl, cur_rxdly0, cur_rxdly1, orig_dsel, orig_dl_cksel, host->sclk);
#endif

	return result;
}

static int msdc_tune_write_each_dat_line(struct msdc_host *host)
{
	u32 base = host->base;

	/* u32 cur_wrrdly = 0, orig_wrrdly; */
	u32 cur_dsmpl = 0, orig_dsmpl;
	u32 cur_rxdly0 = 0;
	u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3;
	u32 cur_dat0 = 0, cur_dat1 = 0, cur_dat2 = 0, cur_dat3 = 0;
	u32 cur_d_cntr = 0, orig_d_cntr;
	int result = 0;

	int sel = 0;
	int clkmode = 0;
	/* MSDC_IOCON_DDR50 CKD need to check. [Fix me] */

	if (host->mclk >= MSDC_CLK_100MHZ) {	/* 100MHz */
		sel = 1;
	} else {
		/* set write data turn around period, only useful for UHS104, so set it to default value (1) */
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, 1);
	}

	/* get CKMOD: 00->divider mode, 01->no divider mode, 02 -> DDR mode */
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, clkmode);

	/* MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, orig_wrrdly); */

	/* get current write data sample edge,  rising edge or falling edge */
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, orig_dsmpl);

	/* get current write data turn around period */
	MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, orig_d_cntr);

	/* set each data line has its own delay */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);


	/* change write data sample edge */
	cur_dsmpl = (orig_dsmpl + 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, cur_dsmpl % 2);

	/* after try to write data both rising edge and falling edge, change delay */
	if (cur_dsmpl >= 2) {
		cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);

		orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
		orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
		orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
		orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;

		cur_dat0 = (orig_dat0 + 1);	/* only adjust bit-1 for crc */
		cur_dat1 = orig_dat1;
		cur_dat2 = orig_dat2;
		cur_dat3 = orig_dat3;

		cur_rxdly0 = ((cur_dat0 & 0x1F) << 24) | ((cur_dat1 & 0x1F) << 16) |
			     ((cur_dat2 & 0x1F) << 8) | ((cur_dat3 & 0x1F) << 0);

		MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
	}

	/* data0 line delay reach to MAX and clock higher than 100MHz */
	if ((cur_dat0 >= 32) && (sel)) {
		/* increase write data turn around period */
		cur_d_cntr = (orig_d_cntr + 1);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, cur_d_cntr % 8);
	}

	/* increase write tune counter */
	++(host->t_counter.time_write);

	/* it means clock lower than 100MHz and tune write times is max */
	if (((sel == 0) && (host->t_counter.time_write == WRITE_TUNE_HS_MAX_TIME)) ||
	    /* it means clock higher than 100MHz and tune write times is max */
	    (sel && (host->t_counter.time_write == WRITE_TUNE_UHS_MAX_TIME))) {
#if MSDC_LOWER_FREQ
		result = msdc_lower_freq(host);
#else
		result = 1;
#endif
		host->t_counter.time_write = 0;
	}

#if TUNE_PARAMS_DETAILS
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, orig_dsmpl);
	MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, orig_d_cntr);
	cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
	MSDC_LOG(TUNE, "TUNE_WRITE: dsmpl<%d> rxdly0<0x%x> d_cntr<%d> sfreq.<%d>", orig_dsmpl,
		 cur_rxdly0, orig_d_cntr, host->sclk);
#endif

	return result;
}


int msdc_tune_cmdrsp_all_dat_lines(struct msdc_host *host)
{
	int result = 0;
	u32 cur_ck_sel = 0, ck_sel = 0;
	u32 cur_ckgen = 0, ckgen = 0;
	u32 cur_internal_delay = 0, internal_delay = 0;
	u32 cur_pad_delay = 0, pad_delay = 0;
	u32 cur_sample_edge = 0, sample_edge = 0;
	u32 base = host->base;
	u32 max_count;

	if(host->hw->host_function == MSDC_SD)
		max_count = CMD_TUNE_SD_MAX_TIME;
	else
		max_count = CMD_TUNE_MAX_TIME;

	/* Step1 - Get current tuning parameters values */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_ck_sel);
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, cur_ckgen);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, cur_internal_delay);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, cur_pad_delay);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cur_sample_edge);

	/* Step2 - Tuning sample edge */
	sample_edge = (cur_sample_edge + 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, sample_edge % 2);

	/* In SD/eMMC init flow, fix rising edge for latching cmd response */
	if (host->mclk <= MSDC_CLK_400KHZ) {	/* 400KHz */
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
		sample_edge = 2;
	}

	/* Step3 - Tuning pad delay, change command pad RX delay */
	if (sample_edge >= 2) {
		if(host->hw->host_function == MSDC_SD)
			pad_delay = (cur_pad_delay + 3);
		else
			pad_delay = (cur_pad_delay + 1);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, pad_delay % 32);
	}

	/* Step4 - Tuning internal delay */
	if (pad_delay >= 32) {
		if(host->hw->host_function == MSDC_SD)
			internal_delay = (internal_delay + 2);
		else
			internal_delay = (internal_delay + 1);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, internal_delay % 32);
	}

	/* Step5 - Tuning ckgen */
	if (internal_delay >= 32) {
		ckgen = (cur_ckgen + 1);
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen % 32);
	}

	/* Step6 - Tuning ckgen sel */
	if (ckgen >= 32) {
		ck_sel = (cur_ck_sel + 1);
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel % 8);
	}

	/* increase command tune counter */
	++(host->t_counter.time_cmd);

	if (host->t_counter.time_cmd == max_count) {	/* tune time counter reach to MAX */
#if MSDC_LOWER_FREQ
		result = msdc_lower_freq(host);
#else
		result = 1;
#endif
		host->t_counter.time_cmd = 0;
	}

	return result;
}

static int msdc_tune_read_all_dat_lines(struct msdc_host *host)
{
	int result = 0;
	u32 cur_ck_sel = 0, ck_sel = 0;
	u32 cur_ckgen = 0, ckgen = 0;
	u32 cur_pad_delay = 0, pad_delay = 0;
	u32 cur_sample_edge = 0, sample_edge = 0;

	u32 base = host->base;

	/* Step1 - Get current tuning parameters values */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_ck_sel);
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, cur_ckgen);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, cur_pad_delay);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_sample_edge);

	/* Step2 - Set tuning method, all data line use same delay */
	/* set all data line use same delay cycle */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 0);

	/* set all data line use same sample edge */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

	/* ========================   DDR  ============================ */
	if (host->ddr) {
		/* Step3 - DDR, tuning ckgen, 32 stages */
		ckgen = cur_ckgen + 1;
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen % 32);

		/* Step4 - DDR, tuning ck_sel, 8 stages */
		if (ckgen >= 32) {
			ck_sel = cur_ck_sel + 1;
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel % 8);
		}

		/* ========================   SDR  ============================ */
	} else {

		/* Step3 - Tuning sample edge */
		sample_edge = (cur_sample_edge + 1);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, sample_edge % 2);

		/* Step4 - Tuning Pad Delay */
		if (sample_edge >= 2) {
			pad_delay = cur_pad_delay + 1;
			MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, pad_delay % 32);
		}

		/* Step5 - Tuning tuning ckgen, 32 stages */
		if (pad_delay >= 32) {
			ckgen = cur_ckgen + 1;
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen % 32);
		}

		/* Step6 - Tuning ck_sel, 8 stages */
		if (ckgen >= 32) {
			ck_sel = cur_ck_sel + 1;
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel % 8);
		}
	}

	/* increase the tune counter */
	++(host->t_counter.time_read);

	/* DDR Mode, tune time counter reach to MAX */
	if ((host->ddr && (host->t_counter.time_read == READ_TUNE_DDR_MAX_TIME)) ||
	    /* SDR Mode, tune time counter reach to MAX */
	    ((host->ddr == 0) && (host->t_counter.time_read == READ_TUNE_MAX_TIME))) {
#if MSDC_LOWER_FREQ
		result = msdc_lower_freq(host);
#else
		result = 1;
#endif
		host->t_counter.time_read = 0;
	}

	return result;
}

static int msdc_tune_write_all_dat_lines(struct msdc_host *host)
{
	int result = 0;
	u32 cur_ck_sel = 0, ck_sel = 0;
	u32 cur_ckgen = 0, ckgen = 0;
	u32 cur_internal_delay = 0, internal_delay = 0;
	u32 cur_pad_delay = 0, pad_delay = 0;
	u32 cur_sample_edge = 0, sample_edge = 0;

	u32 base = host->base;

	/* Step1 - Get current tuning parameters values, obtain orginal values */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_ck_sel);
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, cur_ckgen);
	/* only tune data0 */
	MSDC_GET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0, cur_pad_delay);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, cur_internal_delay);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, cur_sample_edge);

	/* Step2 - Set tuning method, all data line use different delay and sample edge */
	/* different data line use different sample edge */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 1);

	/* different data line use different delay cycle */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

	/* ========================   DDR  ============================ */
	if (host->ddr) {
		/* Step3 - DDR, tuning ckgen, 32 stages */
		ckgen = cur_ckgen + 1;
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen % 32);

		/* Step4 - DDR, tuning ck_sel, 8 stages */
		if (ckgen >= 32) {
			ck_sel = cur_ck_sel + 1;
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel % 8);
		}

		/* ========================   SDR  ============================ */
	} else {

		/* Step3 - Tuning sample edge */
		sample_edge = (cur_sample_edge + 1);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, sample_edge % 2);

		/* Step4 - Tuning Internal Delay */
		if (sample_edge >= 2) {
			internal_delay = cur_internal_delay + 1;
			MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, internal_delay % 32);
		}

		/* Step5 - Tuning Pad Delay */
		if (internal_delay >= 32) {
			pad_delay = cur_pad_delay + 1;
			MSDC_SET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0, pad_delay % 32);
		}

		/* Step6 - Tuning tuning ckgen, 32 stages */
		if (pad_delay >= 32) {
			ckgen = cur_ckgen + 1;
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen % 32);
		}

		/* Step7 - Tuning ck_sel, 8 stages */
		if (ckgen >= 32) {
			ck_sel = cur_ck_sel + 1;
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel % 8);
		}
	}

	/* increase write tune counter */
	++(host->t_counter.time_write);

	/* DDR mode and tune write times is max */
	if ((host->ddr && (host->t_counter.time_write == WRITE_TUNE_DDR_MAX_TIME)) ||
	    /* SDR mode and tune write times is max */
	    ((host->ddr == 0) && (host->t_counter.time_write == WRITE_TUNE_MAX_TIME))) {
#if MSDC_LOWER_FREQ
		result = msdc_lower_freq(host);
#else
		result = 1;
#endif
		host->t_counter.time_write = 0;
	}

	return result;
}


int msdc_tune_cmdrsp(struct msdc_host *host)
{
	if (host && host->hw && (host->hw->tuning_method == TUNE_EACH_DAT_LINE)) {
		return msdc_tune_cmdrsp_each_dat_line(host);
	} else {
		return msdc_tune_cmdrsp_all_dat_lines(host);
	}
}

static int msdc_tune_read(struct msdc_host *host)
{
	if (host && host->hw && (host->hw->tuning_method == TUNE_EACH_DAT_LINE)) {
		return msdc_tune_read_each_dat_line(host);
	} else {
		return msdc_tune_read_all_dat_lines(host);
	}
}

static int msdc_tune_write(struct msdc_host *host)
{
	if (host && host->hw && (host->hw->tuning_method == TUNE_EACH_DAT_LINE)) {
		return msdc_tune_write_each_dat_line(host);
	} else {
		return msdc_tune_write_all_dat_lines(host);
	}
}


int msdc_get_card_status(struct mmc_host *mmc, struct msdc_host *host, u32 *status)
{
	struct mmc_command cmd;
	struct mmc_request mrq;
	u32 err;

	memset(&cmd, 0, sizeof(struct mmc_command));
	cmd.opcode = MMC_SEND_STATUS;	/* CMD13 */
	cmd.arg = host->app_cmd_arg;
	cmd.flags = MMC_RSP_SPI_R2 | MMC_RSP_R1 | MMC_CMD_AC;

	memset(&mrq, 0, sizeof(struct mmc_request));
	mrq.cmd = &cmd;
	cmd.mrq = &mrq;
	cmd.data = NULL;

	err = msdc_do_command(host, &cmd, 0, CMD_TIMEOUT);	/* tune until CMD13 pass. */

	if (status) {
		*status = cmd.resp[0];
	}

	return err;
}

/* #define TUNE_FLOW_TEST */
#ifdef TUNE_FLOW_TEST
static void msdc_reset_para(struct msdc_host *host)
{
	u32 base = host->base;
	u32 dsmpl, rsmpl;

	/* because we have a card, which must work at dsmpl<0> and rsmpl<0> */

	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, dsmpl);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, rsmpl);

	if (dsmpl == 0) {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 1);
		MSDC_LOG(ERR, "set dspl<0>");
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, 0);
	}

	if (rsmpl == 0) {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 1);
		MSDC_LOG(ERR, "set rspl<0>");
		MSDC_WRITE32(MSDC_DAT_RDDLY0, 0);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, 0);
	}
}
#endif

static void msdc_dump_trans_error(struct msdc_host *host,
				  struct mmc_command *cmd,
				  struct mmc_data *data, struct mmc_command *stop)
{
	/* u32 base = host->base; */

	if ((cmd->opcode == 52U) && (cmd->arg == 0xC00U)) {
		return;
	}

	if ((cmd->opcode == 52U) && (cmd->arg == 0x80000C08U)) {
		return;
	}

	/* by pass the SDIO CMD TO for SD/eMMC */
	if (!(IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ))) {
		if ((host->hw->host_function == MSDC_SD) && (cmd->opcode == 5U)) {
			return;
		}
	} else {
		if (cmd->opcode == 8U) {
			return;
		}
	}

	if(cmd->error)
		MSDC_LOG(ERR, "XXX CMD<%d><0x%x> Error<%d> Resp<0x%x>", cmd->opcode, cmd->arg, cmd->error, cmd->resp[0]);

	if(data && data->error) {
		MSDC_LOG(IRQ, "XXX DAT block<%d> Error<%d>", data->blocks, data->error);
	}

	if(stop && stop->error) {
		MSDC_LOG(ERR, "XXX STOP<%d><0x%x> Error<%d> Resp<0x%x>", stop->opcode, stop->arg, stop->error, stop->resp[0]);
	}
	/* Max Xia Marked, Autocmd & when DAT CRC ERR, AutoCmd12 will not be sent */

	if ((host->hw->host_function == MSDC_SD) &&
	    (host->sclk > MSDC_CLK_100MHZ) && (data) && (data->error != CONV2UINT(-ETIMEDOUT))) {
		if ((data->flags & MMC_DATA_WRITE) && (host->write_timeout_uhs104)) {
			host->write_timeout_uhs104 = 0;
		}

		if ((data->flags & MMC_DATA_READ) && (host->read_timeout_uhs104)) {
			host->read_timeout_uhs104 = 0;
		}
	}

	if ((host->hw->host_function == MSDC_EMMC) &&
	    (data) && (data->error != CONV2UINT(-ETIMEDOUT))) {
		if ((data->flags & MMC_DATA_WRITE) && (host->write_timeout_emmc)) {
			host->write_timeout_emmc = 0;
		}

		if ((data->flags & MMC_DATA_READ) && (host->read_timeout_emmc)) {
			host->read_timeout_emmc = 0;
		}
	}
}

void msdc_remove_bad_sd(struct mmc_host *mmc, bool mark_bad)
{
/* it will rescan after removing sd if mark_bad is false */
	struct msdc_host *host = mmc_priv(mmc);
	unsigned long flags;
	int got_polarity = 0;

	host->card_inserted = 0;
	if (host->mmc->card) {
		spin_lock_irqsave(&host->remove_bad_card, flags);
		got_polarity = host->sd_cd_polarity;
		if(mark_bad)
			host->block_bad_card = 1;
		mmc_card_set_removed(host->mmc->card);
		spin_unlock_irqrestore(&host->remove_bad_card, flags);

		if (((host->hw->flags & MSDC_CD_PIN_EN) &&
					(got_polarity ^ host->hw->cd_level)) ||
				(!(host->hw->flags & MSDC_CD_PIN_EN))) {
			tasklet_hi_schedule(&host->card_tasklet);
		}
	}

}

/* ops.request */
static void msdc_do_request_legacy(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_command *cmd;
	struct mmc_data *data;
	struct mmc_command *stop = NULL;
	struct mmc_command *sbc = NULL;
	int data_abort = 0;
	int need_save_pre_setting = 0;
	unsigned int legacy_req_tune_times = 0;

	/* === for sdio profile === */
	u32 old_H32 = 0, old_L32 = 0, new_H32 = 0, new_L32 = 0;
	u32 ticks = 0, opcode = 0, sizes = 0, bRx = 0;

	msdc_reset_tune_counter(host, all_counter);

	if (host->mrq) {
		MSDC_LOG(ERR, "XXX host->mrq<0x%.8x> cmd<%d>arg<0x%x>", (int)host->mrq,
			 host->mrq->cmd->opcode, host->mrq->cmd->arg);
		BUG();
	}

	/* Check card's present and power mode */
#ifdef MSDC_POWER_FAIL_WP
	if (!IS_CARD_PRESENT(host) || host->power_mode == MMC_POWER_OFF || 
			(host->protected== host->protect_init && host->id == 0 && (mrq->cmd->opcode == 24 || mrq->cmd->opcode == 25))) {
        	MSDC_LOG_NORMAL(pr_info, "[%d]cmd<%d> arg<0x%x> card<%d> power<%d> emmc protected!", host->id, mrq->cmd->opcode,mrq->cmd->arg, IS_CARD_PRESENT(host), host->power_mode);
#else
	if (!IS_CARD_PRESENT(host) || host->power_mode == MMC_POWER_OFF ) {
        	MSDC_LOG_NORMAL(pr_info, "[%d]cmd<%d> arg<0x%x> card<%d> power<%d> ", host->id, mrq->cmd->opcode,mrq->cmd->arg, IS_CARD_PRESENT(host), host->power_mode);

#endif
		mrq->cmd->error = (unsigned int)-ENOMEDIUM; 

		if (mrq->done) {
			mrq->cmd->retries = 0;	/* please don't retry. */
			mrq->done(mrq);	/* call done directly. */
		}
		return;
	}

	/* start to process */
	spin_lock(&host->lock);
	host->power_cycle_enable = 1;

	cmd = mrq->cmd;
	data = mrq->cmd->data;

	if (data) {
		stop = data->stop;
	}
#ifdef MTK_MSDC_USE_CMD23
	if (data)
		sbc = mrq->sbc;
#endif

	host->mrq = mrq;
	SET_TUNE_STS(host, TUNE_STS_NOT_IN_TUNE);

	while (msdc_do_request(mmc, mrq)) {	/* there is some error */
#ifdef CONFIG_MSDC_NEW_TUNING_SUPPORT
		if((cmd->opcode == MMC_SEND_TUNING_BLOCK) || (cmd->opcode == MMC_SEND_TUNING_BLOCK_HS200))
			goto out;
#endif

		/* becasue ISR executing time will be monitor, try to dump the info here. */
		if(legacy_req_tune_times < 3)
			msdc_dump_trans_error(host, cmd, data, stop);
		legacy_req_tune_times ++;
		SET_TUNE_STS(host, TUNE_STS_IN_TUNE);
		need_save_pre_setting = 1;
		data_abort = 0;
#ifdef CONFIG_MSDC_ETT_SUPPORT
		if (ett_tune_flag&&(host->hw->host_function == MSDC_SDIO))
			goto out;
#else
		/* SDIO: don't tuning */
		if((host->hw->host_function == MSDC_SDIO) && (mmc->card)) {
			goto out;
		}
#endif

#ifdef MTK_MSDC_USE_CMD23
		if ((sbc != NULL) && (sbc->error == (unsigned int)-ETIMEDOUT)) {
			if (cmd->opcode == MMC_READ_MULTIPLE_BLOCK
				|| cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK) {
				/* not tuning, go out directly */
				MSDC_LOG(ERR,"==cmd23 timeout==\n");
				goto out;
			}
		}
#endif

#ifdef MTK_MSDC_USE_CMD23
		/* cmd->error also set when autocmd23 crc error */
		if ((cmd->error == (unsigned int)-EIO)
		    || (stop && (stop->error == (unsigned int)-EIO))
		    || (sbc && (sbc->error == (unsigned int)-EIO))) {
#else
		if ((cmd->error == (unsigned int)-EIO)
		    || (stop && (stop->error == (unsigned int)-EIO))) {
#endif
			if (msdc_tune_cmdrsp(host)){
				MSDC_LOG(ERR, "failed to update cmd para");
				goto out;
			}
		}

		/* DAT CRC ERR */
		if (data && (data->error == CONV2UINT(-EIO))) {
			if (data->flags & MMC_DATA_READ) {	/* read */
				if (msdc_tune_read(host)) {
					MSDC_LOG(ERR, "failed to update read para");
					goto out;
				}
			} else {
				if (msdc_tune_write(host)) {
					MSDC_LOG(ERR, "failed to update write para");
					goto out;
				}
			}
		}

		if((host->hw->host_function == MSDC_SD) && data && (data->error == CONV2UINT(-ETIMEDOUT))) {
			if(data->flags & MMC_DATA_READ) {
				MSDC_LOG(ERR, "sd legacy read tmo, try reinit");
				msdc_save_setting(host->mmc);
				msdc_force_reinit(host);
				msdc_restore_setting(host->mmc, 1);
				goto out;
			}
		}

		/* bring the card to "tran" state */
		if (data) {
			if (msdc_abort_data(host)) {
				MSDC_LOG(ERR, "abort failed");
				data_abort = 1;

				if (host->hw->host_function == MSDC_SD) {
					msdc_remove_bad_sd(mmc, false);
					goto out;
				}
			}
		}

		/* CMD TO ->  */
		if (cmd->error == CONV2UINT(-ETIMEDOUT)) {
			if((host->id == 0) && (cmd->opcode == MMC_SEND_EXT_CSD) && (host->mclk <= mmc->f_init)) {
				MSDC_LOG(ERR, "cmd<%d> cmd timeout while initializing", cmd->opcode);
				msdc_force_reinit(host);
			} else if ((cmd->opcode == MMC_READ_SINGLE_BLOCK)
			    || (cmd->opcode == MMC_READ_MULTIPLE_BLOCK)
			    || (cmd->opcode == MMC_WRITE_BLOCK)
			    || (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK)) {
				if (data_abort) {
					if (msdc_power_tuning(host)) {
						goto out;
					}
				}
			} else if((host->id == 0) && ((cmd->opcode == MMC_SWITCH) || (cmd->opcode == MMC_SEND_STATUS))) {
				if (msdc_tune_cmdrsp(host)){
					MSDC_LOG(ERR, "failed to update cmd%d para", cmd->opcode);
					if(host->mclk <= mmc->f_init) {
						MSDC_LOG_NORMAL(pr_err, "try force reinit");
						msdc_force_reinit(host);
					}
					goto out;
				}
			} else if((host->id == 0) && (cmd->opcode == MMC_ALL_SEND_CID)) {
				/*cxl: Don't retry emmc cmd2 directly, it will fail with TIMEOUT, should do cmd0->cmd1->cmd2 flow to reovery*/
				msdc_tune_cmdrsp(host);
				MSDC_LOG_NORMAL(pr_info, "emmc cmd2 fail: just tuning, skip retry");
				goto out;
			} else if((host->hw->host_function == MSDC_SDIO) && (mmc->card)) {
				if(msdc_tune_cmdrsp(host)){
					MSDC_LOG(ERR, "failed to update cmd%d para", cmd->opcode);
					msdc_save_setting(host->mmc);
					msdc_force_reinit(host);
					msdc_restore_setting(host->mmc, 1);
					goto out;
				}
			} else {
				if ((host->hw->host_function == MSDC_SD) && (host->sclk >= 12000000U)) {
					if(cmd->opcode == MMC_SEND_STATUS) {
						if (msdc_tune_cmdrsp(host)){
							MSDC_LOG(ERR, "failed to update cmd12 para");
							goto out;
						}
					} else {
						MSDC_LOG(ERR, "CTO : %d", cmd->opcode);
						msdc_abort_data(host);
					}
				}
				goto out;
			}
		}

		/* Data timeout issue. // Max Xia : Need consider max tuning times */
		if (data && (data->error == CONV2UINT(-ETIMEDOUT))) {
			if (data->flags & MMC_DATA_READ) {
				if((host->id == 0) && (cmd->opcode == MMC_SEND_EXT_CSD) && (host->mclk <= mmc->f_init)) {
					MSDC_LOG(ERR, "cmd<%d> data timeout while initializing", cmd->opcode);
					msdc_force_reinit(host);
				} else if (!(host->sw_timeout) &&
				    (host->hw->host_function == MSDC_SD) &&
				    (host->sclk > MSDC_CLK_100MHZ) &&
				    (host->read_timeout_uhs104 < MSDC_MAX_R_TIMEOUT_TUNE)) {
					if (host->t_counter.time_read) {
						host->t_counter.time_read--;
					}

					host->read_timeout_uhs104++;
					msdc_tune_read(host);
				} else if ((host->sw_timeout)
					   || (host->read_timeout_uhs104 >= MSDC_MAX_R_TIMEOUT_TUNE)
					   || ((++(host->read_time_tune)) > MSDC_MAX_TIMEOUT_RETRY)) {
					MSDC_LOG(ERR,
						 "msdc%d exceed max read timeout retry times(%d) or SW timeout(%d) or read timeout tuning times(%d),Power cycle",
						 host->id, host->read_time_tune, host->sw_timeout,
						 host->read_timeout_uhs104);

					if (msdc_power_tuning(host)) {
						goto out;
					}
				}
			} else if (data->flags & MMC_DATA_WRITE) {
				if ((!(host->sw_timeout)) &&
				    (host->hw->host_function == MSDC_SD) &&
				    (host->sclk > MSDC_CLK_100MHZ) &&
				    (host->write_timeout_uhs104 < MSDC_MAX_W_TIMEOUT_TUNE)) {
					if (host->t_counter.time_write) {
						host->t_counter.time_write--;
					}

					host->write_timeout_uhs104++;
					msdc_tune_write(host);
				} else if (!(host->sw_timeout) &&
					   (host->hw->host_function == MSDC_EMMC) &&
					   (host->write_timeout_emmc <
					    MSDC_MAX_W_TIMEOUT_TUNE_EMMC)) {
					if (host->t_counter.time_write) {
						host->t_counter.time_write--;
					}

					host->write_timeout_emmc++;
					msdc_tune_write(host);
				} else if ((host->hw->host_function == MSDC_SD) &&
					   ((host->sw_timeout)
					    || (host->write_timeout_uhs104 >= MSDC_MAX_W_TIMEOUT_TUNE)
					    || ((++(host->write_time_tune)) > MSDC_MAX_TIMEOUT_RETRY))) {
					MSDC_LOG(ERR,
						 "msdc%d exceed max write timeout retry times(%d) or SW timeout(%d) or write timeout tuning time(%d),Power cycle",
						 host->id, host->write_time_tune, host->sw_timeout,
						 host->write_timeout_uhs104);

					if (!(host->sd_30_busy) && msdc_power_tuning(host)) {
						goto out;
					}
				} else if ((host->hw->host_function == MSDC_EMMC) &&
					   ((host->sw_timeout)
					    || ((++(host->write_time_tune)) > MSDC_MAX_TIMEOUT_RETRY_EMMC))) {
					MSDC_LOG(ERR,
						 "msdc%d exceed max write timeout retry times(%d) or SW timeout(%d) or write timeout tuning time(%d),Power cycle",
						 host->id, host->write_time_tune, host->sw_timeout,
						 host->write_timeout_emmc);
					host->write_timeout_emmc = 0;
					goto out;
				}

			}
		}

		/* clear the error condition. */
		cmd->error = 0;

		if (data) {
			data->error = 0;
		}

		if (stop) {
			stop->error = 0;
		}

#ifdef MTK_MSDC_USE_CMD23
		if (sbc)
			sbc->error = 0;
#endif

		/* check if an app commmand. */
		if (host->app_cmd) {
			while (msdc_app_cmd(host->mmc, host)) {
				if (msdc_tune_cmdrsp(host)) {
					MSDC_LOG(ERR, "failed to update cmd para for app");
					goto out;
				}
			}
		}

		if (!IS_CARD_PRESENT(host)) {
			goto out;
		}
	} /* while(msdc_do_request(mmc,mrq)) end */
       if(legacy_req_tune_times) {
               MSDC_LOG_NORMAL(pr_info, "cmd%d legacy total tune: %d times\n", cmd->opcode, legacy_req_tune_times);
       }

	/* Start --------------------------------------------------------------- */
	if ((host->read_time_tune)
	    && ((cmd->opcode == MMC_READ_SINGLE_BLOCK) || (cmd->opcode == MMC_READ_MULTIPLE_BLOCK))) {
		host->read_time_tune = 0;
		MSDC_LOG_NORMAL(pr_info, "[%d]Read recover", host->id);
		msdc_dump_trans_error(host, cmd, data, stop);
	}

	if ((host->write_time_tune)
	    && ((cmd->opcode == MMC_WRITE_BLOCK) || (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))) {
		host->write_time_tune = 0;
		MSDC_LOG_NORMAL(pr_info, "[%d]Write recover", host->id);
		msdc_dump_trans_error(host, cmd, data, stop);
	}

	host->sw_timeout = 0;
	/* End --------------------------------------------------------------- */

out:
	SET_TUNE_STS(host, TUNE_STS_NOT_IN_TUNE);

	/* Max Xia: need consider tuning result and save new pre_setting parameters here */
	if (need_save_pre_setting) {
		if ((cmd->opcode == MMC_READ_SINGLE_BLOCK) || (cmd->opcode == MMC_READ_MULTIPLE_BLOCK)) {
			msdc_save_read_pre(host);
		} else if ((cmd->opcode == MMC_WRITE_BLOCK) || (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK)) {
			msdc_save_write_pre(host);
		}
	}

	msdc_reset_tune_counter(host, all_counter);

#ifdef TUNE_FLOW_TEST

	if (!IS_CARD_SDIO(host)) {
		msdc_reset_para(host);
	}

#endif

	/* ==== when request done, check if app_cmd ==== */
	if (mrq->cmd->opcode == MMC_APP_CMD) {
		host->app_cmd = 1;
		host->app_cmd_arg = mrq->cmd->arg;	/* save the RCA */
	} else {
		host->app_cmd = 0;
		/* host->app_cmd_arg = 0; */
	}

	host->mrq = NULL;

	/* === for sdio profile === */
	if (sdio_pro_enable) {
		if ((mrq->cmd->opcode == 52) || (mrq->cmd->opcode == 53)) {
			ticks = msdc_time_calc(old_L32, old_H32, new_L32, new_H32);

			opcode = mrq->cmd->opcode;

			if (mrq->cmd->data) {
				sizes = mrq->cmd->data->blocks * mrq->cmd->data->blksz;
				bRx = mrq->cmd->data->flags & MMC_DATA_READ ? 1 : 0;
			} else {
				bRx = mrq->cmd->arg & 0x80000000 ? 1 : 0;
			}

			if (!mrq->cmd->error) {
				msdc_performance(opcode, sizes, bRx, ticks);
			}
		}
	}

	spin_unlock(&host->lock);

	mmc_request_done(mmc, mrq);
}

static void msdc_tune_async_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_command *cmd;
	struct mmc_data *data;
	struct mmc_command *stop = NULL;
#ifdef MTK_MSDC_USE_CMD23
	struct mmc_command *sbc = NULL;
#endif
	int data_abort = 0;
	int need_save_pre_setting = 0;
	unsigned int async_req_tune_times = 0;

	if (host->mrq) {
		WARN_ON(host->mrq);

		if(host->mrq->cmd)
			MSDC_LOG(ERR, "host->mrq<0x%.8x> cmd<%d> arg<0x%x>", (int)host->mrq, host->mrq->cmd->opcode, host->mrq->cmd->arg);

		if (host->mrq->data) {
			MSDC_LOG(ERR, "request data size<%d>", host->mrq->data->blocks * host->mrq->data->blksz);
			MSDC_LOG(ERR, "request attach to host force data timeout and retry");
			host->mrq->data->error = CONV2UINT(-ETIMEDOUT);
		} else {
			MSDC_LOG(ERR, "request attach to host force cmd timeout and retry");
			if(host->mrq->cmd)
				host->mrq->cmd->error = CONV2UINT(-ETIMEDOUT);
		}

		MSDC_LOG(ERR, "current request <0x%.8x> cmd<%d>arg<0x%x>", (int)mrq, mrq->cmd->opcode, mrq->cmd->arg);
		if (mrq->data)
			MSDC_LOG(ERR, "current request data size<%d>", mrq->data->blocks * mrq->data->blksz);
	}

	if (!IS_CARD_PRESENT(host) || (host->power_mode == MMC_POWER_OFF)) {
		MSDC_LOG(ERR, "cmd<%d> arg<0x%x> card<%d> power<%d>", mrq->cmd->opcode,
			 mrq->cmd->arg, IS_CARD_PRESENT(host), host->power_mode);
		mrq->cmd->error = CONV2UINT(-ENOMEDIUM);
		return;
	}

	cmd = mrq->cmd;
	data = mrq->cmd->data;

	if (data) {
		stop = data->stop;
	}

#ifdef MTK_MSDC_USE_CMD23
	if (data)
		sbc = mrq->sbc;
#endif

#ifdef MTK_MSDC_USE_CMD23
	/* if requset was executed successfully, reset tune count and return. */
	if ((cmd->error == 0) && (data && (data->error == 0)) && (!stop || (stop->error == 0))&&(!sbc || (sbc->error == 0))) {
#else
	if ((cmd->error == 0) && (data && (data->error == 0)) && (!stop || (stop->error == 0))) {
#endif
		if ((cmd->opcode == MMC_READ_SINGLE_BLOCK) || (cmd->opcode == MMC_READ_MULTIPLE_BLOCK)) {
			host->read_time_tune = 0;
		}

		if ((cmd->opcode == MMC_WRITE_BLOCK) || (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK)) {
			host->write_time_tune = 0;
		}

		host->rwcmd_time_tune = 0;
		host->sw_timeout = 0;
		host->power_cycle_enable = 1;
		return;
	}

	/********   start to process the request that it was executed failed  ***********/
	spin_lock(&host->lock);

	SET_TUNE_STS(host, TUNE_STS_IN_TUNE);
	host->mrq = mrq;

	do {
		/* becasue ISR executing time will be monitor, try to dump the info here. */
		if(async_req_tune_times < 2)
			msdc_dump_trans_error(host, cmd, data, stop);
		async_req_tune_times ++;
		need_save_pre_setting = 1;

#ifdef MTK_MSDC_USE_CMD23
		if ((sbc != NULL) && (sbc->error == (unsigned int)-ETIMEDOUT)) {
			if (cmd->opcode == MMC_READ_MULTIPLE_BLOCK
					|| cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK) {
				/* not tuning, go out directly */
				MSDC_LOG(ERR, "==cmd23 timeout==");
				goto out;
			}
		}
#endif
		/* CMD CRC ERR */
#ifdef MTK_MSDC_USE_CMD23
		/* cmd->error also set when autocmd23 crc error */
		if ((cmd->error == (unsigned int)-EIO)
		    || (stop && (stop->error == (unsigned int)-EIO))
		    || (sbc && (sbc->error == (unsigned int)-EIO))) {
#else
		if ((cmd->error == (unsigned int)-EIO)
		    || (stop && (stop->error == (unsigned int)-EIO))) {
#endif
			if (msdc_tune_cmdrsp(host)){
				MSDC_LOG(ERR, "failed to update cmd para");
				goto out;
			}
		}

		if(cmd->error == -EREMOTEIO) {
			MSDC_LOG(ERR, "###write protect area###");
			goto out;
		}

		/* DAT CRC ERR */
		if (data && (data->error == CONV2UINT(-EIO))) {
			if (data->flags & MMC_DATA_READ) {	/* read */
				if (msdc_tune_read(host)) {
					MSDC_LOG(ERR, "failed to update read para");
					goto out;
				}
			} else {
				if (msdc_tune_write(host)) {
					MSDC_LOG(ERR, "failed to update write para");
					goto out;
				}
			}
		}

		/* bring the card to "tran" state */
		if (data) {
			if (msdc_abort_data(host)) {
				MSDC_LOG(ERR, "abort failed");
				data_abort = 1;

				/* Only SD !!?? */
				if (host->hw->host_function == MSDC_SD) {
					msdc_remove_bad_sd(mmc, false);
					goto out;
				}
			}
		}

		/* CMD TO -> not tuning */
		if (cmd->error == CONV2UINT(-ETIMEDOUT)) {
			if ((cmd->opcode == MMC_READ_SINGLE_BLOCK)
					|| (cmd->opcode == MMC_READ_MULTIPLE_BLOCK)
					|| (cmd->opcode == MMC_WRITE_BLOCK)
					|| (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK)) {

				if(host->hw->host_function == MSDC_EMMC) {
					if((host->sw_timeout < MSDC_MAX_TIMEOUT_RETRY_EMMC)
							&& (host->rwcmd_timeout_emmc < MSDC_MAX_TIMEOUT_RETRY_EMMC)) {
						if(host->t_counter.time_cmd)
							host->t_counter.time_cmd--;

						host->rwcmd_timeout_emmc++;
						msdc_tune_cmdrsp(host);
					} else if((host->sw_timeout >= MSDC_MAX_TIMEOUT_RETRY_EMMC)
							|| (host->rwcmd_timeout_emmc >= MSDC_MAX_TIMEOUT_RETRY_EMMC)) {
						MSDC_LOG(ERR, "msdc%d exceed max rwcmd timeout retry times(%d) "
								"or SW timeout(%d) or rwcmd timeout tuning time(%d), "
								"low freq",
								host->id, host->rwcmd_time_tune, host->sw_timeout,
								host->rwcmd_timeout_emmc);
						host->rwcmd_timeout_emmc = 0;
						if (msdc_lower_freq(host))
							goto out;
					}
				} else if(host->hw->host_function == MSDC_SD) {
					if(host->sw_timeout || (++(host->rwcmd_time_tune) > MSDC_MAX_TIMEOUT_RETRY)) {
						MSDC_LOG(ERR, "msdc%d exceed max r/w cmd timeout tune times(%d) "
								"or SW timeout(%d), Power cycle",
								host->id, host->rwcmd_time_tune, host->sw_timeout);
						if (!(host->sd_30_busy) && msdc_power_tuning(host))
							goto out;
					}
				}

				/* Max Xia add for SDIO RW */
			} else if ((cmd->opcode == SD_IO_RW_DIRECT) || (cmd->opcode == SD_IO_RW_EXTENDED)) {
				MSDC_LOG(ERR, "SDIO RW CMD%d Timeout", cmd->opcode);
				msdc_abort_data(host);
				goto out;
			} else {
				goto out;
			}
		}

		/* No card was inserted or the card was removed */
		if (cmd->error == CONV2UINT(-ENOMEDIUM)) {
			goto out;
		}

		/* Data timeout issue. */
		if (data && (data->error == CONV2UINT(-ETIMEDOUT))) {
			if (data->flags & MMC_DATA_READ) {
				if (host->hw->host_function == MSDC_SD) {//sd read
					if(!(host->sw_timeout) && (host->sclk > MSDC_CLK_100MHZ)
							&& (host->read_timeout_uhs104 < MSDC_MAX_R_TIMEOUT_TUNE)) {
						if (host->t_counter.time_read) {
							host->t_counter.time_read--;
						}
						host->read_timeout_uhs104++;
						msdc_tune_read(host);
					} else if((host->sw_timeout) || (host->read_timeout_uhs104 >= MSDC_MAX_R_TIMEOUT_TUNE)
							|| (++(host->read_time_tune) > MSDC_MAX_TIMEOUT_RETRY)) {
						MSDC_LOG(ERR, "msdc%d exceed max read timeout retry times(%d) or "
								"SW timeout(%d) or read timeout tuning times(%d), low freq",
								host->id, host->read_time_tune, host->sw_timeout,
								host->read_timeout_uhs104);
						if (!(host->sd_30_busy) && msdc_lower_freq(host))
							goto out;
					}

				} else if (host->hw->host_function == MSDC_EMMC) {//emmc read
					if((host->sw_timeout < MSDC_MAX_TIMEOUT_RETRY_EMMC) && (host->read_timeout_emmc < MSDC_MAX_R_TIMEOUT_TUNE_EMMC)) {
						if(host->t_counter.time_read)
							host->t_counter.time_read --;
						host->read_timeout_emmc++;
						msdc_tune_read(host);
					} else if((host->sw_timeout >= MSDC_MAX_TIMEOUT_RETRY_EMMC)
							|| (host->read_timeout_emmc >= MSDC_MAX_R_TIMEOUT_TUNE_EMMC)
							|| (++(host->read_time_tune) > MSDC_MAX_TIMEOUT_RETRY_EMMC)){
						MSDC_LOG(ERR, "msdc%d exceed max read timeout retry times(%d) or "
								"SW timeout(%d) or read timeout tuning time(%d), low freq",
								host->id, host->read_time_tune, host->sw_timeout,
								host->read_timeout_emmc);
						host->read_timeout_emmc = 0;
						if (msdc_lower_freq(host))
							goto out;
					}
				}
			} else if (data->flags & MMC_DATA_WRITE) {
				if (host->hw->host_function == MSDC_SD) {//sd write
					if(!(host->sw_timeout) && (host->sclk > MSDC_CLK_100MHZ)
							&& (host->write_timeout_uhs104 < MSDC_MAX_W_TIMEOUT_TUNE)) {
						if (host->t_counter.time_write) {
							host->t_counter.time_write--;
						}
						host->write_timeout_uhs104++;
						msdc_tune_write(host);
					} else if(host->sw_timeout || (host->write_timeout_uhs104 >= MSDC_MAX_W_TIMEOUT_TUNE)
							|| (++(host->write_time_tune) > MSDC_MAX_TIMEOUT_RETRY)) {
						MSDC_LOG(ERR, "msdc%d exceed max write timeout retry times(%d) or "
								"SW timeout(%d) or write timeout tuning time(%d),Power cycle",
								host->id, host->write_time_tune, host->sw_timeout,
								host->write_timeout_uhs104);

						if (!(host->sd_30_busy) && msdc_power_tuning(host)) {
							goto out;
						}
					}
				} else if (host->hw->host_function == MSDC_EMMC) {//emmc write
					if((host->sw_timeout < MSDC_MAX_TIMEOUT_RETRY_EMMC) && (host->write_timeout_emmc < MSDC_MAX_W_TIMEOUT_TUNE_EMMC)) {
						if (host->t_counter.time_write) {
							host->t_counter.time_write--;
						}
						host->write_timeout_emmc++;
						msdc_tune_write(host);
					} else if((host->sw_timeout >= MSDC_MAX_TIMEOUT_RETRY_EMMC)
							|| (host->write_timeout_emmc >= MSDC_MAX_W_TIMEOUT_TUNE_EMMC)
							|| (++(host->write_time_tune) > MSDC_MAX_TIMEOUT_RETRY_EMMC)) {
						MSDC_LOG(ERR, "msdc%d exceed max write timeout retry times(%d) or "
								"SW timeout(%d) or write timeout tuning time(%d), low freq",
								host->id, host->write_time_tune, host->sw_timeout,
								host->write_timeout_emmc);
						host->write_timeout_emmc = 0;
						if (msdc_lower_freq(host))
							goto out;

					}
				}
			}
		}

		/* clear the error condition. */
		cmd->error = 0;

		if (data) {
			data->error = 0;
		}

		if (stop) {
			stop->error = 0;
		}

#ifdef MTK_MSDC_USE_CMD23
		if (sbc)
			sbc->error = 0;
#endif
		if (host->hw->host_function != MSDC_EMMC)
			host->sw_timeout = 0;

		if (!IS_CARD_PRESENT(host)) {
			goto out;
		}
	} while (msdc_tune_rw_request(mmc, mrq));

	if(async_req_tune_times) {
		MSDC_LOG_NORMAL(pr_info, "cmd%d async total tune: %d times\n", cmd->opcode, async_req_tune_times);
	}

	/* Max Xia: below code has not been executed */
	/* Start --------------------------------------------------------------- */
	if ((host->rwcmd_time_tune)
	    && ((cmd->opcode == MMC_READ_SINGLE_BLOCK) || (cmd->opcode == MMC_READ_MULTIPLE_BLOCK)
		|| (cmd->opcode == MMC_WRITE_BLOCK) || (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))) {
		host->rwcmd_time_tune = 0;
		host->rwcmd_timeout_emmc = 0;
		MSDC_LOG(ERR, "RW cmd recover");
		msdc_dump_trans_error(host, cmd, data, stop);
	}

	if ((host->read_time_tune)
	    && (cmd->opcode == MMC_READ_SINGLE_BLOCK || cmd->opcode == MMC_READ_MULTIPLE_BLOCK)) {
		host->read_time_tune = 0;
		host->read_timeout_uhs104 = 0;
		host->read_timeout_emmc = 0;
		MSDC_LOG(ERR, "Read recover");
		msdc_dump_trans_error(host, cmd, data, stop);
	}

	if ((host->write_time_tune)
	    && ((cmd->opcode == MMC_WRITE_BLOCK) || (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK))) {
		host->write_time_tune = 0;
		host->write_timeout_uhs104 = 0;
		host->write_timeout_emmc = 0;
		MSDC_LOG(ERR, "Write recover");
		msdc_dump_trans_error(host, cmd, data, stop);
	}

	host->power_cycle_enable = 1;
	host->sw_timeout = 0;
	/* End --------------------------------------------------------------- */

out:

	if ((host->sclk <= MSDC_CLK_50MHZ) && (!host->ddr)) {
		host->sd_30_busy = 0;
	}

	if (need_save_pre_setting) {
		if ((cmd->opcode == MMC_READ_SINGLE_BLOCK) || (cmd->opcode == MMC_READ_MULTIPLE_BLOCK)) {
			msdc_save_read_pre(host);
		} else if ((cmd->opcode == MMC_WRITE_BLOCK) || (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK)) {
			msdc_save_write_pre(host);
		}
	}

	msdc_reset_tune_counter(host, all_counter);
	host->mrq = NULL;
	SET_TUNE_STS(host, TUNE_STS_NOT_IN_TUNE);
	spin_unlock(&host->lock);
}

/*  */
/* host ops interface function, used by core layer after register host. */
/*  */
static void msdc_ops_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct mmc_data *data;
	struct msdc_host *host = NULL;
	int async = 0;

	BUG_ON(mmc == NULL);
	BUG_ON(mrq == NULL);
	data = mrq->data;

	if (data) {
		async = data->host_cookie;
	}
#ifdef CONFIG_MMC_ATC_SW_WP
	if(mmc->sw_wp_enable && data && (data->flags & MMC_DATA_WRITE)) {
		if(is_in_wp_area(mrq->cmd->arg) || is_in_wp_area(mrq->cmd->arg + data->blocks)) {
			MSDC_LOG(ERR, "try to write wp area(sect 0x%x, cnt %d), force fail\n", mrq->cmd->arg, data->blocks);

			mrq->cmd->error = CONV2UINT(-ECANCELED);
			if(mrq->done)
				mrq->done(mrq);
			return;
		}
	}
#endif

	if (async) {
		msdc_do_request_async(mmc, mrq);
	} else {
		msdc_do_request_legacy(mmc, mrq);
	}
}

/*  */
/* called by ops.set_ios */
/*  */
static void msdc_set_buswidth(struct msdc_host *host, u32 width)
{
	/* static char *bus_width_desc[] = { "1", "err", "4", "8" }; */

	u32 base = host->base;
	u32 val = MSDC_READ32(SDC_CFG);

	val &= ~SDC_CFG_BUSWIDTH;

	switch (width) {
	case MMC_BUS_WIDTH_1:
		val |= (MSDC_BUS_1BITS << 16);
		break;

	case MMC_BUS_WIDTH_4:
		val |= (MSDC_BUS_4BITS << 16);
		break;

	case MMC_BUS_WIDTH_8:
		val |= (MSDC_BUS_8BITS << 16);
		break;

	default:
		MSDC_LOG(ERR, "Error Bus Width = %d", width);
		return;
	}

	MSDC_WRITE32(SDC_CFG, val);

	/* MSDC_LOG(CFG, "Bus Width = %d", width); */
	/* MSDC_LOG(ERR, "Select Bus Width: %sbit mode", bus_width_desc[width]); */
}


/*  */
/* ops.set_ios */
/*  */
static void msdc_ops_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct msdc_hw *hw = host->hw;
	u32 base = host->base;
	u32 ddr = 0;
	u32 cur_rxdly0 = 0, cur_rxdly1 = 0;

#ifdef ATC_MSDC_DEBUG
	static const char *vdd[] const = {
		"1.50v", "1.55v", "1.60v", "1.65v", "1.70v", "1.80v", "1.90v",
		"2.00v", "2.10v", "2.20v", "2.30v", "2.40v", "2.50v", "2.60v",
		"2.70v", "2.80v", "2.90v", "3.00v", "3.10v", "3.20v", "3.30v",
		"3.40v", "3.50v", "3.60v"
	};
	static const char *power_mode[] const = {
		"OFF", "UP", "ON"
	};
	static const char *bus_mode[] const = {
		"UNKNOWN", "OPENDRAIN", "PUSHPULL"
	};
	static const char *timing[] const = {
		"LEGACY", "MMC_HS", "SD_HS"
	};

	MSDC_LOG(CFG, "SET_IOS: CLK(%dkHz), BUS(%s), BW(%u), PWR(%s), VDD(%s), TIMING(%s)",
		 ios->clock / 1000, bus_mode[ios->bus_mode],
		 (ios->bus_width == MMC_BUS_WIDTH_4) ? 4 : 1,
		 power_mode[ios->power_mode], vdd[ios->vdd], timing[ios->timing]);
#endif

	spin_lock(&host->lock);

	if ((ios->timing == MMC_TIMING_UHS_DDR50)||
		(ios->timing== MMC_TIMING_MMC_DDR52)){
		ddr = 1;
	}

	msdc_set_buswidth(host, ios->bus_width);

	/* Power control ??? */
	switch (ios->power_mode) {
	case MMC_POWER_OFF:
	case MMC_POWER_UP:
		msdc_set_driving(host, hw, 0);
		host->power_mode = ios->power_mode;
		break;

	case MMC_POWER_ON:
		host->power_mode = MMC_POWER_ON;
		break;

	default:
		break;
	}
	if (host->timing != ios->timing) {
		if ((host->id == 0)&&(ios->timing == MMC_TIMING_MMC_HS200))
			msdc_apply_ett_settings(host);

		host->timing = ios->timing;
	}

	if ((msdc_host_mode[host->id] != mmc->caps) || (msdc_host_mode2[host->id] != mmc->caps2)) {
		MSDC_LOG(ERR, " !!!Caps is not match ");
		mmc->caps = msdc_host_mode[host->id];
		mmc->caps2 = msdc_host_mode2[host->id];

		MSDC_WRITE32(MSDC_PAD_TUNE, 0x00000000);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, host->hw->datwrddly);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, host->hw->cmdrrddly);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, host->hw->cmdrddly);
		MSDC_WRITE32(MSDC_IOCON, 0x00000000);

		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);
		cur_rxdly0 =
			((host->hw->dat0rddly & 0x1F) << 24) | ((host->hw->dat1rddly & 0x1F) << 16) |
			((host->hw->dat2rddly & 0x1F) << 8) | ((host->hw->dat3rddly & 0x1F) << 0);
		cur_rxdly1 =
			((host->hw->dat4rddly & 0x1F) << 24) | ((host->hw->dat5rddly & 0x1F) << 16) |
			((host->hw->dat6rddly & 0x1F) << 8) | ((host->hw->dat7rddly & 0x1F) << 0);
		MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
		MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1);

		if ((host->hw->host_function == MSDC_EMMC) || (host->hw->host_function == MSDC_SD)) {
			MSDC_WRITE32(MSDC_PATCH_BIT1, 0xFFFF00C9);
		} else {
			MSDC_WRITE32(MSDC_PATCH_BIT1, 0xFFFF0009);
		}

		/* internal clock: latch read data, not apply to sdio */
		if (!(IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ))) {
			host->hw->cmd_edge = 0;	/* tuning from 0 */
			host->hw->rdata_edge = 0;
			host->hw->wdata_edge = 0;
		}
	}

	/* not change when clock Freq. not changed ddr need set clock */
	if ((host->mclk != ios->clock) || (host->ddr != ddr)) {
		if (ios->clock >= MSDC_CLK_25MHZ) {

			if (ios->clock > MSDC_CLK_100MHZ)
				;	/* Set SD 18 Driving Strength here!! */

			if (IS_CARD_SDIO(host) && sdio_enable_tune) {

				sdio_tune_flag = 0;
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);
				/* Latch edge */
				host->hw->cmd_edge = sdio_iocon_rspl;
				host->hw->rdata_edge = sdio_iocon_dspl;
				host->hw->wdata_edge = sdio_iocon_w_dspl;

				/* CMD and DATA delay */
				MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY,
					       sdio_pad_tune_rdly);
				MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY,
					       sdio_pad_tune_rrdly);
				MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY,
					       sdio_pad_tune_wrdly);
				MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
					       sdio_dat_rd_dly0_0);
				cur_rxdly0 =
					(sdio_dat_rd_dly0_0 << 24) | (sdio_dat_rd_dly0_1 << 16) |
					(sdio_dat_rd_dly0_2 << 8) | (sdio_dat_rd_dly0_3 << 0);

				host->saved_para.pad_tune = MSDC_READ32(MSDC_PAD_TUNE);
				host->saved_para.ddly0 = cur_rxdly0;
			}

		}

#if 0				/* Max Xia, If enable below code, it will cause tuning after resume */

		if (ios->clock == 0) {
			MSDC_LOG(ERR
				 "---------------->>> Clock be Set to 0 <<<-----------------");

			if (ios->power_mode == MMC_POWER_OFF) {
				MSDC_LOG(ERR,
					 "------------------->>> Clock be Set to 0, MMC_POWER_OFF <<<--------------------");
				MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, hw->cmd_edge);	/* save the para */
				MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, hw->rdata_edge);
				MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, hw->wdata_edge);
				host->saved_para.pad_tune = MSDC_READ32(MSDC_PAD_TUNE);
				host->saved_para.ddly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
				host->saved_para.ddly1 = MSDC_READ32(MSDC_DAT_RDDLY1);
				MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP,
					       host->saved_para.cmd_resp_ta_cntr);
				MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS,
					       host->saved_para.wrdat_crc_ta_cntr);
			}

			/* reset to default value */
			MSDC_WRITE32(MSDC_IOCON, 0x00000000);
			MSDC_WRITE32(MSDC_DAT_RDDLY0, 0x00000000);
			MSDC_WRITE32(MSDC_DAT_RDDLY1, 0x00000000);
			MSDC_WRITE32(MSDC_PAD_TUNE, 0x00000000);
			MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, 1);
			MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, 1);
		}

#endif

		if (host && host->mmc && host->mmc->card && mmc_card_sdio(host->mmc->card))

			/* if (host->hw->host_function == MSDC_SDIO && ios->clock == MSDC_CLK_25MHZ) */
			if (ios->clock == MSDC_CLK_50MHZ) {
				MSDC_LOG(ERR,
					 "---------------->>> Set MSDC_IOCON_RSPL at %dMHz <<<-----------------",
					 ios->clock);
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, host->hw->cmd_edge);
			}

		/* Change clock and save */
		msdc_set_mclk(host, ddr, ios->clock);

		/*because sdio not call suspend/resume, so we restore settings here*/
		if(sdio_resume_back && (host->hw->host_function == MSDC_SDIO) && (ios->timing >= MMC_TIMING_UHS_SDR104)) {
			MSDC_LOG_NORMAL(pr_debug, "restore sdio settings, mclk=%dKHz\n", host->mclk/1000);
			msdc_restore_setting(host->mmc, 0);
			sdio_resume_back = false;
		}
	}

	spin_unlock(&host->lock);
}

/* ops.get_ro */
static int msdc_ops_get_ro(struct mmc_host *mmc)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 base = host->base;
	unsigned long flags;
	int ro = 0;

	spin_lock_irqsave(&host->lock, flags);

	if (host->hw->flags & MSDC_WP_PIN_EN) {	/* set for card */
		ro = (MSDC_READ32(MSDC_PS) >> 31);
	}

	spin_unlock_irqrestore(&host->lock, flags);
	return ro;
}

/* ops.get_cd */
static int msdc_ops_get_cd(struct mmc_host *mmc)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 base;
	unsigned long flags;

	base = host->base;
	spin_lock_irqsave(&host->lock, flags);

	/* for sdio card */
#if 1
	if (host->hw->host_function == MSDC_SDIO) {
		MSDC_LOG_NORMAL(pr_info, "wifi slot card %s", host->card_inserted?"on":"off");
		goto end;
	}
#else
	/* for sdio, depends on USER_RESUME */
	if (IS_CARD_SDIO(host)) {
		if (!(host->hw->flags & MSDC_SDIO_IRQ)) {
			host->card_inserted =
				(host->pm_state.event == PM_EVENT_USER_RESUME) ? 1 : 0;
			MSDC_LOG(INIT, "sdio ops_get_cd<%d>", host->card_inserted);
			goto end;
		}
	}
#endif

	/* for emmc card, MSDC_REMOVABLE not set, always return 1 */
	if (!(host->hw->flags & MSDC_REMOVABLE)) {
		host->card_inserted = 1;
		goto end;
	}

	/* for sd card (MSDC_CD_PIN_EN not set) */
	if (host->hw->flags & MSDC_CD_PIN_EN) {
		if (host->hw->cd_level) {
			host->card_inserted = (host->sd_cd_polarity == 0) ? 1 : 0;
		} else {
			host->card_inserted = (host->sd_cd_polarity == 0) ? 0 : 1;
		}
	} else {
		/* for debounce purpose, we need check again */
		host->card_inserted = card_detect_gpio(host);
	}

	if(host->hw->host_function == MSDC_SD && host->block_bad_card)
		host->card_inserted = 0;

	MSDC_LOG_NORMAL(pr_info, "Card insert<%d> Block bad card<%d>",
		host->card_inserted, host->block_bad_card);

	/* now clear bad card flag */
	if(host->block_bad_card)
		host->block_bad_card  = 0;

end:
	spin_unlock_irqrestore(&host->lock, flags);
	return host->card_inserted;
}

static int msdc_ops_get_rescan(struct mmc_host *mmc)
{
	struct msdc_host *host = mmc_priv(mmc);
	unsigned long irq_flags;

#if 1
	/* SDIO and it has been set to disabled */
	if ((host->hw->host_function == MSDC_SDIO) && (!host->card_inserted)) {
		if (host->mmc->card) {
			mmc_card_set_removed(host->mmc->card);
		}
	}

#endif

	if ((host->hw->host_function == MSDC_SD) && (!host->card_inserted)) {
		if (host->mmc->card) {
			mmc_card_set_removed(host->mmc->card);
		}
	}

	spin_lock_irqsave(&host->detect_queue_lock, irq_flags);
	host->queue_len = 0;
	spin_unlock_irqrestore(&host->detect_queue_lock, irq_flags);

	return 1;
}

/* ops.enable_sdio_irq */
static void msdc_ops_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct msdc_hw *hw = host->hw;
	u32 base = host->base;
	u32 tmp;

	if (hw->flags & MSDC_EXT_SDIO_IRQ) {	/* yes for sdio */
		if (enable) {
			hw->enable_sdio_eirq();        /* combo_sdio_enable_eirq */
		} else {
			hw->disable_sdio_eirq();        /* combo_sdio_disable_eirq */
		}
	} else {
		MSDC_LOG(ERR, "XXX ");	/* so never enter here */
#if (MSDC_DATA1_INT == 1)

		if (host->hw->flags & MSDC_SDIO_IRQ) {
			int_sdio_irq_enable = enable;

			if (!u_sdio_irq_counter) {
				MSDC_LOG(ERR, "msdc2 u_sdio_irq_counter=1");
			}

			if (u_sdio_irq_counter < 0xFFFF) {
				u_sdio_irq_counter = u_sdio_irq_counter + 1;
			} else {
				u_sdio_irq_counter = 1;
			}

			if (u_sdio_irq_counter < 7) {
				MSDC_LOG(ERR, "msdc2 sdio_irq enable: %d", int_sdio_irq_enable);
			}

			/* MSDC_LOG(ERR, "Ahsin int_sdio_irq_enable=%d  u_sdio_irq_counter=%d",
			   int_sdio_irq_enable,u_sdio_irq_counter); */
		}

#endif
		tmp = MSDC_READ32(SDC_CFG);

		/* FIXME. Need to interrupt gap detection */
		if (enable) {
			tmp |= (SDC_CFG_SDIOIDE | SDC_CFG_SDIOINTWKUP);
#if (MSDC_DATA1_INT == 1)
			MSDC_SET_BITS(MSDC_INTEN, MSDC_INT_SDIOIRQ);
#endif
		} else {
			/* tmp &= ~(SDC_CFG_SDIOIDE | SDC_CFG_SDIOINTWKUP); */
#if (MSDC_DATA1_INT == 1)
			MSDC_CLR_BITS(MSDC_INTEN, MSDC_INT_SDIOIRQ);
#endif
		}

		MSDC_WRITE32(SDC_CFG, tmp);
	}
}

static int msdc_ops_switch_volt(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 base = host->base;
	int err = 0;
	u32 timeout = 100;
	u32 retry = 10;
	u32 status;
	u32 sclk = host->sclk;

	if ((host->hw->host_function != MSDC_EMMC) && (ios->signal_voltage != MMC_SIGNAL_VOLTAGE_330)) {
		MSDC_LOG_NORMAL(pr_info, "[%d]%s: power switch volt to 1.8V.", host->id, __FUNCTION__);
		/* make sure SDC is not busy (TBC) */
		/* WAIT_COND(!SDC_IS_BUSY(), timeout, timeout); */
		err = CONV2UINT(-EIO);
		MSDC_RETRY(SDC_IS_BUSY(), retry, timeout, host->id);

		if ((timeout == 0) && (retry == 0)) {
			err = CONV2UINT(-ETIMEDOUT);
			goto out;
		}

		/* pull up disabled in CMD and DAT[3:0] to allow card drives them to low */
		/* check if CMD/DATA lines both 0 */
		if ((MSDC_READ32(MSDC_PS) & ((1U << 24) | (0xFU << 16))) == 0) {

			/* change signal from 3.3v to 1.8v for FPGA this can not work */
			if (ios->signal_voltage == MMC_SIGNAL_VOLTAGE_180) {
				if (host->power_switch) {
					host->power_switch(host, 1);
				} else {
					MSDC_LOG(ERR,
						 "No power switch callback. Please check host_function<%u>",
						 host->hw->host_function);
				}
			}

			/* wait at least 5ms for 1.8v signal switching in card */
			mdelay(10);

			/* config clock to 10~12MHz mode for volt switch detection by host. */
			msdc_set_mclk(host, 0, 12000000);	/*For FPGA 13MHz clock,this not work */

			mdelay(105);

			/* start to detect volt change by providing 1.8v signal to card */
			MSDC_SET_BITS(MSDC_CFG, MSDC_CFG_BV18SDT);

			/* wait at max. 1ms */
			mdelay(1);
			/* MSDC_LOG(ERR, "before read status"); */

			while ((status = MSDC_READ32(MSDC_CFG)) & MSDC_CFG_BV18SDT)
				;

			if (status & MSDC_CFG_BV18PSS) {
				err = 0;
			}

			/* MSDC_LOG(ERR, "msdc V1800 status (0x%x),err(%d)",status,err); */
			/* config clock back to init clk freq. */
			msdc_set_mclk(host, 0, sclk);
		}
	}

out:

	return err;
}

static void msdc_ops_stop(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 err = -1;

	if(host->hw->host_function != MSDC_SDIO){
		if(!mrq->stop)
			return;
#ifdef MTK_MSDC_USE_CMD23
		if(!(host->error & REQ_DAT_ERR))
			return;
#endif
  		if((host->autocmd & MSDC_AUTOCMD12) && (!(host->error & REQ_DAT_ERR)))
			return;
		MSDC_LOG(OPS, "MSDC Stop for non-autocmd12 host->error(%d)host->autocmd(%d)",
			 host->error, host->autocmd);
		err = msdc_do_command(host, mrq->stop, 0, CMD_TIMEOUT);

		if (err) {
			if (mrq->stop->error == CONV2UINT(-EIO)) {
				host->error |= REQ_STOP_EIO;
			}

			if (mrq->stop->error == CONV2UINT(-ETIMEDOUT)) {
				host->error |= REQ_STOP_TMO;
			}
		}
	}
}


static bool msdc_check_written_data(struct mmc_host *mmc, struct mmc_request *mrq)
{
	u32 result = 0;
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_card *card;

	if (!IS_CARD_PRESENT(host) || (host->power_mode == MMC_POWER_OFF)) {
		MSDC_LOG(ERR, "cmd<%d> arg<0x%x> card<%d> power<%d>", mrq->cmd->opcode,
			 mrq->cmd->arg, IS_CARD_PRESENT(host), host->power_mode);
		mrq->cmd->error = CONV2UINT(-ENOMEDIUM);
		return 0;
	}

	if (mmc->card) {
		card = mmc->card;
	} else {
		return 0;
	}

	/* Only Check SD Write */
	if ((host->hw->host_function == MSDC_SD)
	    && (host->sclk > MSDC_CLK_100MHZ)
	    && mmc_card_sd(card)
	    && (mrq->data)
	    && (mrq->data->flags & MMC_DATA_WRITE)
	    && (host->error == 0)) {
		spin_lock(&host->lock);

		if (msdc_polling_idle(host)) {
			spin_unlock(&host->lock);
			return 0;
		}

		spin_unlock(&host->lock);
		result = __mmc_sd_num_wr_blocks(card);

		if ((result != mrq->data->blocks) && (IS_CARD_PRESENT(host))
		    && (host->power_mode == MMC_POWER_ON)) {
			mrq->data->error = CONV2UINT(-EIO);
			host->error |= REQ_DAT_ERR;
			MSDC_LOG(ERR,
				 "written data<%d> blocks isn't equal to request data blocks<%d>",
				 result, mrq->data->blocks);
			return 1;
		}
	}

	return 0;
}

static void msdc_dma_error_reset(struct mmc_host *mmc)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 base = host->base;
	struct mmc_data *data = host->data;

	if (data && host->dma_xfer && (data->host_cookie) && NOT_IN_TUNE_PROCESS(host)) {
		host->sw_timeout++;
		host->error |= REQ_DAT_ERR;
		msdc_dump_info(host->id);
		MSDC_RESET_HW(host->id);
		msdc_dma_stop(host);
		MSDC_CLR_FIFO(host->id);
		MSDC_CLR_INT();
		msdc_dma_clear(host);
	}
}

/* .card_event will be called when rescan card */
static void msdc_ops_card_event(struct mmc_host *mmc)
{
	struct msdc_host *host = mmc_priv(mmc);

	if (host->block_bad_card) {
		host->block_bad_card = 0;

		if (host->mmc->card) {
			mmc_card_set_removed(host->mmc->card);
			MSDC_LOG(ERR, "remove bad SD card");
		}
	}
}

static void msdc_init_card_status(struct mmc_host *mmc, int status)
{
	struct msdc_host *host = mmc_priv(mmc);
	struct mmc_card *card=mmc->card;
	host->init_card_status = status;

	if (host->init_card_status != MMC_INIT_CARD_STATUS_SUCCESS) {
		MSDC_LOG_NORMAL(pr_info, "[%d]mmc core notify init card status <%d> times<%d>", host->id, status, host->init_retry_times);
	}
	else
		host->init_retry_times=0;
	
	if (host->hw->host_function == MSDC_SDIO) {
		if(status == MMC_INIT_CARD_STATUS_SUCCESS)
			sdio_init_status=SDIO_INIT_SUCCESS;
		else if(status == MMC_INIT_CARD_STATUS_FAILED)
			sdio_init_status=SDIO_INIT_FAIL;
	}
	else if ((status == MMC_INIT_CARD_STATUS_FAILED) && (host->init_retry_times < MSDC_INIT_RETRY_TIMES)){
		host->init_retry_times++;
		if(host->hw->cd_method & MSDC_CD_METHOD_INT_POL){
			tasklet_hi_schedule(&host->card_tasklet);
		}
	}
	if(host->init_retry_times >= MSDC_INIT_RETRY_TIMES){
		host->block_bad_card = 1;
		host->init_retry_times=0;
	}

	/*try to reinit 1.0 version sd card*/
	if ((status == MMC_INIT_CARD_STATUS_SUCCESS)&&(host->hw->host_function == MSDC_SD)&&(host->sd_version_retry<SD_VERSION_RETRY_TIMES)){
		if(card&&(card->scr.sda_vsn==0)){
			MSDC_LOG(ERR, "try to reinit 1.0 version card \n");
			host->sd_version_retry++;
			mmc_release_host(mmc);
			mmc_remove_card(card);
			mmc->card = NULL;
			mmc_claim_host(mmc);
			mmc_detach_bus(mmc);
			mmc_power_off(mmc);
			mmc_detect_change(mmc, msecs_to_jiffies(200));
		}
	}

}

/*
 * hs200, sdr104 tuning method
 */
#ifdef	CONFIG_MSDC_NEW_TUNING_SUPPORT
struct msdc_delay_phase {
	u8 maxlen;
	u8 start;
	u8 final_phase;
};
#define PAD_DELAY_MAX (32)
static u32 test_delay_bit(u32 delay, u32 bit)
{
	bit %= PAD_DELAY_MAX;
	return delay & (1 << bit);
}
static int get_delay_len(u32 delay, u32 start_bit)
{
	int i;

	for (i = 0; i < (PAD_DELAY_MAX - start_bit); i++) {
		if (test_delay_bit(delay, start_bit + i) == 0)
			return i;
	}
	return PAD_DELAY_MAX - start_bit;
}

static struct msdc_delay_phase get_best_delay(struct msdc_host *host, u32 delay)
{
	int start = 0, len = 0;
	int start_final = 0, len_final = 0;
	u8 final_phase = 0xff;
	struct msdc_delay_phase delay_phase = { 0, };

	if (delay == 0) {
		MSDC_LOG_NORMAL(pr_err, "msdc%d phase error: [map:%x]\n", host->id, delay);
		delay_phase.final_phase = final_phase;
		return delay_phase;
	}

	while (start < PAD_DELAY_MAX) {
		len = get_delay_len(delay, start);
		if (len_final < len) {
			start_final = start;
			len_final = len;
		}
		start += len ? len : 1;
		if (len >= 12 && start_final < 4)
			break;
	}

	/* The rule is that to find the smallest delay cell */
	if (start_final == 0)
		final_phase = (start_final + len_final / 3) % PAD_DELAY_MAX;
	else
		final_phase = (start_final + len_final / 2) % PAD_DELAY_MAX;
	MSDC_LOG_NORMAL(pr_info, "msdc%d: phase: [map:%x] [maxlen:%d] [final:%d]\n", host->id,
			delay, len_final, final_phase);

	delay_phase.maxlen = len_final;
	delay_phase.start = start_final;
	delay_phase.final_phase = final_phase;
	return delay_phase;
}

static int msdc_send_tuning(struct mmc_host *host, u32 opcode, int *cmd_error)
{
	struct mmc_request mrq = {NULL};
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;
	struct mmc_ios *ios = &host->ios;
	int size, err = 0;
	u8 *data_buf;

	if (ios->bus_width == MMC_BUS_WIDTH_8)
		size = 128;
	else if (ios->bus_width == MMC_BUS_WIDTH_4)
		size = 64;
	else
		return -EINVAL;

	data_buf = kzalloc(size, GFP_KERNEL);
	if (!data_buf)
		return -ENOMEM;
	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = opcode;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;

	data.blksz = size;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	/*
	 * According to the tuning specs, Tuning process
	 * is normally shorter 40 executions of CMD19,
	 * and timeout value should be shorter than 150 ms
	 */
	data.timeout_ns = 150 * NSEC_PER_MSEC;

	data.sg = &sg;
	data.sg_len = 1;
	sg_init_one(&sg, data_buf, size);

	mmc_wait_for_req(host, &mrq);

	if (cmd_error)
		*cmd_error = cmd.error;

	if (cmd.error) {
		err = cmd.error;
		goto out;
	}

	if (data.error) {
		err = data.error;
		goto out;
	}

out:
	kfree(data_buf);
	return err;
}

static int msdc_tune_response(struct mmc_host *mmc, u32 opcode)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 rise_delay = 0, fall_delay = 0;
	struct msdc_delay_phase final_rise_delay, final_fall_delay = { 0,};
	struct msdc_delay_phase internal_delay_phase;
	u8 final_delay, final_maxlen;
	u32 internal_delay = 0;
	int cmd_err;
	int i;
	u32 base = host->base;

	if (mmc->ios.timing == MMC_TIMING_MMC_HS200 ||
	    mmc->ios.timing == MMC_TIMING_UHS_SDR104)
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, host->hs200_cmd_int_delay);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
	for(i=0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, i);
		msdc_send_tuning(mmc, opcode, &cmd_err);
		if(!cmd_err)
			rise_delay |= (1 << i);
	}
	final_rise_delay = get_best_delay(host, rise_delay);
	if (final_rise_delay.maxlen >= 12 || (final_rise_delay.start == 0 && final_rise_delay.maxlen >= 4))
		goto skip_fall;
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 1);
	for (i = 0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, i);
		msdc_send_tuning(mmc, opcode, &cmd_err);
		if (!cmd_err)
			fall_delay |= (1 << i);
	}
	final_fall_delay = get_best_delay(host, fall_delay);

skip_fall:
	final_maxlen = max(final_rise_delay.maxlen, final_fall_delay.maxlen);
	if (final_maxlen == final_rise_delay.maxlen) {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, final_rise_delay.final_phase);
		final_delay = final_rise_delay.final_phase;
	} else {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 1);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, final_fall_delay.final_phase);
		final_delay = final_fall_delay.final_phase;
	}

	for (i = 0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, i);
		msdc_send_tuning(mmc, opcode, &cmd_err);
		if (!cmd_err)
			internal_delay |= (1 << i);
	}
	MSDC_LOG_NORMAL(pr_info, "[%d]: Final internal delay: 0x%x\n", host->id, internal_delay);
	internal_delay_phase = get_best_delay(host, internal_delay);
	MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, internal_delay_phase.final_phase);
done:
	MSDC_LOG_NORMAL(pr_info, "[%d]: Final cmd pad delay: %x\n", host->id, final_delay);
	return final_delay == 0xff ? -EIO : 0;
}

static int msdc_tune_data(struct mmc_host *mmc, u32 opcode)
{
	struct msdc_host *host = mmc_priv(mmc);
	u32 rise_delay = 0, fall_delay = 0;
	struct msdc_delay_phase final_rise_delay, final_fall_delay = { 0, };
	u8 final_delay, final_maxlen;
	int i, ret;
	u32 base = host->base;

	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 0);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, 0);
	for (i = 0 ; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, i);
		ret = msdc_send_tuning(mmc, opcode, NULL);
		if (!ret)
			rise_delay |= (1 << i);
	}
	final_rise_delay = get_best_delay(host, rise_delay);
	if (final_rise_delay.maxlen >= 12 ||
	    (final_rise_delay.start == 0 && final_rise_delay.maxlen >= 4))
		goto skip_fall;

	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, 1);
	for (i = 0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, i);
		ret = msdc_send_tuning(mmc, opcode, NULL);
		if (!ret)
			fall_delay |= (1 << i);
	}
	final_fall_delay = get_best_delay(host, fall_delay);

skip_fall:
	final_maxlen = max(final_rise_delay.maxlen, final_fall_delay.maxlen);
	if (final_maxlen == final_rise_delay.maxlen) {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 0);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, 0);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
			      final_rise_delay.final_phase);
		final_delay = final_rise_delay.final_phase;
	} else {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 1);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, 1);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
			      final_fall_delay.final_phase);
		final_delay = final_fall_delay.final_phase;
	}

	if (mmc->ios.timing == MMC_TIMING_MMC_HS200 ||
	    mmc->ios.timing == MMC_TIMING_UHS_SDR104)
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY,
			      host->hs200_write_int_delay);
	MSDC_LOG_NORMAL(pr_info, "[%d] Final data pad delay: %x\n", host->id, final_delay);
	return final_delay == 0xff ? -EIO : 0;
}

static int msdc_execute_tuning(struct mmc_host *mmc, u32 opcode)
{
	struct msdc_host *host = mmc_priv(mmc);
	int ret;
	unsigned long long time1, time2;

	if(host->hw->host_function != MSDC_SDIO)
		return 0;

	time1 = sched_clock();

	ret = msdc_tune_response(mmc, opcode);
	if (ret == -EIO) {
		MSDC_LOG_NORMAL(pr_err, "[%d] tune response fail!\n", host->id);
		goto out;
	}

	ret = msdc_tune_data(mmc, opcode);
	if (ret == -EIO)
		MSDC_LOG_NORMAL(pr_err, "[%d] tune data fail!\n", host->id);

	time2 = sched_clock();
	MSDC_LOG_NORMAL(pr_info,"[%d] tune takes %lld ms\n", host->id, (time2-time1)/1000/1000);

out:
	return ret;
}
#endif

static struct mmc_host_ops atc_msdc_ops = {
	.post_req = msdc_post_req,
	.pre_req = msdc_pre_req,
	.request = msdc_ops_request,
	.req_tuning = msdc_tune_async_request,
	.set_ios = msdc_ops_set_ios,
	.get_ro = msdc_ops_get_ro,
	.get_cd = msdc_ops_get_cd,
	.get_rescan = msdc_ops_get_rescan,
	.enable_sdio_irq = msdc_ops_enable_sdio_irq,
	.start_signal_voltage_switch = msdc_ops_switch_volt,
	.send_stop = msdc_ops_stop,
	.dma_error_reset = msdc_dma_error_reset,
	.check_written_data = msdc_check_written_data,
	.card_event = msdc_ops_card_event,
	.init_card_status=msdc_init_card_status,
#ifdef	CONFIG_MSDC_NEW_TUNING_SUPPORT
	.execute_tuning = msdc_execute_tuning,
#endif
};

/*--------------------------------------------------------------------------*/
/* interrupt handler                                                    */
/*--------------------------------------------------------------------------*/
static irqreturn_t msdc_irq(int irq, void *dev_id)
{
	struct msdc_host *host = (struct msdc_host *)dev_id;
	struct mmc_data *data = host->data;
	struct mmc_command *cmd = host->cmd;
	struct mmc_command *stop = NULL;
	struct mmc_request *mrq = NULL;
	u32 base = host->base;

	u32 cmdsts = MSDC_INT_RSPCRCERR | MSDC_INT_CMDTMO | MSDC_INT_CMDRDY |
		     MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO | MSDC_INT_ACMDRDY | MSDC_INT_ACMD19_DONE;
	u32 datsts = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO;
	u32 intsts, inten;

	/* Read Interrupt Status */
	intsts = MSDC_READ32(MSDC_INT);

	latest_int_status[host->id] = intsts;
	inten = MSDC_READ32(MSDC_INTEN);

	/* MSDC_LOG(ERR,  "-----> intsts = 0x%08X, inten = 0x%08X <-----", intsts, inten); */

#if (MSDC_DATA1_INT == 1)

	if (host->hw->flags & MSDC_SDIO_IRQ) {
		intsts &= inten;
	} else
#endif
		inten &= intsts;

	MSDC_WRITE32(MSDC_INT, intsts);	/* clear interrupts */

	/* sdio interrupt */
	if (intsts & MSDC_INT_SDIOIRQ) {
		MSDC_LOG(IRQ, "XXX MSDC_INT_SDIOIRQ");	/* seems not sdio irq */
#if (MSDC_DATA1_INT == 1)

		if (host->hw->flags & MSDC_SDIO_IRQ) {
			if (!u_msdc_irq_counter) {
				MSDC_LOG(ERR, "msdc2 u_msdc_irq_counter=1");
			}

			if (u_msdc_irq_counter < 0xFFFF) {
				u_msdc_irq_counter = u_msdc_irq_counter + 1;
			} else {
				u_msdc_irq_counter = 1;
			}

			mmc_signal_sdio_irq(host->mmc);
		}

#endif
	}

	/* transfer complete interrupt */
	if (data != NULL) {
		stop = data->stop;

#if (MSDC_DATA1_INT == 1)

		if (host->hw->flags & MSDC_SDIO_IRQ) {
			if (intsts & MSDC_INT_XFER_COMPL) {
				data->bytes_xfered = host->dma.xfersz;

				if ((data->host_cookie) && NOT_IN_TUNE_PROCESS(host)) {
					msdc_dma_stop(host);
					mrq = host->mrq;
					msdc_dma_clear(host);

					if (mrq&&mrq->done) {
						mrq->done(mrq);
					}

					host->error &= ~REQ_DAT_ERR;
				} else {
					complete(&host->xfer_done);
				}
			}
		} else
#endif
		{
			if (intsts & MSDC_INT_XFER_COMPL) {
				data->bytes_xfered = host->dma.xfersz;

				if ((data->host_cookie) && NOT_IN_TUNE_PROCESS(host)) {
					msdc_dma_stop(host);
					mrq = host->mrq;
					msdc_dma_clear(host);

					if (mrq&&mrq->done) {
						mrq->done(mrq);
					}

					host->error &= ~REQ_DAT_ERR;
				} else {
					complete(&host->xfer_done);
				}
			}
		}

		/* Check data transfer status, DATTO or CRC ERR */
		if (intsts & datsts) {
			/* do basic reset, or stop command will sdc_busy */
			MSDC_RESET_HW(host->id);
			atomic_set(&host->abort, 1);	/* For PIO mode exit */

			if (intsts & MSDC_INT_DATTMO) {
				if (host->card_inserted) {
					data->error = CONV2UINT(-ETIMEDOUT);
				} else {
					data->error = CONV2UINT(-ENOMEDIUM);
				}

				if(host->mrq)
					MSDC_LOG(IRQ, "CMD<%d> Arg<0x%.8x> MSDC_INT_DATTMO", host->mrq->cmd->opcode, host->mrq->cmd->arg);
				else
					MSDC_LOG(ERR, "[TMP] TIMEOUT, host->mrq is null, please check\n");

			} else if (intsts & MSDC_INT_DATCRCERR) {
				if (host->card_inserted) {
					data->error = CONV2UINT(-EIO);
				} else {
					data->error = CONV2UINT(-ENOMEDIUM);
				}

#if 1

				if ((host->last_dat_err_cmd_opcode != host->mrq->cmd->opcode) ||
				    (host->last_dat_err_cmd_arg != host->mrq->cmd->arg) ||
				    (host->last_dat_err_intr != MSDC_INT_DATCRCERR)) {
					host->last_dat_err_cmd_opcode = host->mrq->cmd->opcode;
					host->last_dat_err_cmd_arg = host->mrq->cmd->arg;
					host->last_dat_err_intr = MSDC_INT_DATCRCERR;
					if(host->mrq)
						MSDC_LOG(IRQ,  "CMD<%d> Arg<0x%.8x> MSDC_INT_DATCRCERR, SDC_DCRC_STS<0x%x>",
								host->mrq->cmd->opcode, host->mrq->cmd->arg, MSDC_READ32(SDC_DCRC_STS));
					else
						MSDC_LOG(ERR, "[TMP] CRC fail, host->mrq is null, please check\n");
				}

#else
				MSDC_LOG(IRQ,
					 "XXX CMD<%d> Arg<0x%.8x> MSDC_INT_DATCRCERR, SDC_DCRC_STS<0x%x>",
					 host->mrq->cmd->opcode, host->mrq->cmd->arg,
					 MSDC_READ32(SDC_DCRC_STS));
#endif
			}

			/* if(MSDC_READ32(MSDC_INTEN) & MSDC_INT_XFER_COMPL) { */
			if (host->dma_xfer) {
				if ((data->host_cookie) && NOT_IN_TUNE_PROCESS(host)) {
					msdc_dma_stop(host);
					MSDC_CLR_FIFO(host->id);
					MSDC_CLR_INT();
					mrq = host->mrq;
					msdc_dma_clear(host);

					if (mrq&&mrq->done) {
						mrq->done(mrq);
					}

					host->error |= REQ_DAT_ERR;
				} else {
					/* Read CRC come fast, XFER_COMPL not enabled */
					complete(&host->xfer_done);
				}
			}	/* PIO mode can't do complete, because not init */
		}

		/* Check stop command status */
		if ((stop != NULL) && (host->autocmd == MSDC_AUTOCMD12) && (intsts & cmdsts)) {
			if (intsts & MSDC_INT_ACMDRDY) {
				u32 *arsp = &stop->resp[0];
				*arsp = MSDC_READ32(SDC_ACMD_RESP);
			} else if (intsts & MSDC_INT_ACMDCRCERR) {
				stop->error = CONV2UINT(-EIO);
				host->error |= REQ_STOP_EIO;
				MSDC_RESET_HW(host->id);
			} else if (intsts & MSDC_INT_ACMDTMO) {
				stop->error = CONV2UINT(-ETIMEDOUT);
				host->error |= REQ_STOP_TMO;
				MSDC_RESET_HW(host->id);
			}

			if ((intsts & MSDC_INT_ACMDCRCERR) || (intsts & MSDC_INT_ACMDTMO)) {
				if (host->dma_xfer) {
					if ((data->host_cookie) && NOT_IN_TUNE_PROCESS(host)) {
						msdc_dma_stop(host);
						MSDC_CLR_FIFO(host->id);
						MSDC_CLR_INT();
						mrq = host->mrq;
						msdc_dma_clear(host);

						if (mrq&&mrq->done) {
							mrq->done(mrq);
						}
					} else
						/* Autocmd12 issued but error occur, the data transfer done INT */
						/* will not issue, so cmplete is need here */
					{
						complete(&host->xfer_done);
					}
				} /* PIO mode can't do complete, because not init */
			}
		}
	}

	/* command interrupts */
	if ((cmd != NULL) && (intsts & cmdsts)) {
		if (intsts & MSDC_INT_CMDRDY) {
			u32 *rsp = NULL;

			rsp = &cmd->resp[0];

			switch (host->cmd_rsp) {
			case RESP_NONE:
				break;

			case RESP_R2:
				*rsp = MSDC_READ32(SDC_RESP3);
				rsp++;
				*rsp = MSDC_READ32(SDC_RESP2);
				rsp++;
				*rsp = MSDC_READ32(SDC_RESP1);
				rsp++;
				*rsp = MSDC_READ32(SDC_RESP0);
				rsp++;
				break;

			default:	/* Response types 1, 3, 4, 5, 6, 7(1b) */
				*rsp = MSDC_READ32(SDC_RESP0);
				break;
			}
		} else if (intsts & MSDC_INT_RSPCRCERR) {
			cmd->error = CONV2UINT(-EIO);
			MSDC_LOG(IRQ, "CMD<%d> MSDC_INT_RSPCRCERR Arg<0x%.8x>", cmd->opcode, cmd->arg);
			MSDC_RESET_HW(host->id);
		} else if (intsts & MSDC_INT_CMDTMO) {
			cmd->error = CONV2UINT(-ETIMEDOUT);
			MSDC_LOG(IRQ, "CMD<%d> MSDC_INT_CMDTMO Arg<0x%.8x>", cmd->opcode, cmd->arg);
			MSDC_RESET_HW(host->id);
		}

		if (intsts & (MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR | MSDC_INT_CMDTMO)) {
			complete(&host->cmd_done);
		}
	}

	/* mmc irq interrupts */
	if (intsts & MSDC_INT_MMCIRQ) {
		; /* MSDC_LOG(ERR, " MMCIRQ: SDC_CSTS=0x%.8x", MSDC_READ32(SDC_CSTS)); */
	}
	
	latest_int_status[host->id] = 0;
	return IRQ_HANDLED;
}

/*--------------------------------------------------------------------------*/
/* platform_driver members                                                      */
/*--------------------------------------------------------------------------*/

#if 0 /* Below functions are not invoked by other functions, Mark temporarily */
static int msdc_power_down_reset(struct msdc_host *host, u32 disable)
{
	if (!host) {
		return 1;
	}

	MSDC_MODULE_HW_RESET(host);

	return 0;
}

static int msdc_sw_reset(struct msdc_host *host, u32 disable)
{
	if (!host) {
		return 1;
	}

	MSDC_MODULE_SW_RESET(host);

	return 0;
}

#endif /* Below functions are not invoked by other functions, Mark temporarily */

static int msdc_pad_multi_func(struct msdc_host *host, u32 nand_boot)
{
	#if ATC_GPIO_KS
	struct gpio_desc * msdc_gpio_desc;
	#endif

	if (!host) {
		return 1;
	}

	#if ATC_GPIO_KS
	msdc_gpio_desc = gpio_to_desc(host->vol_sw_gpio);
	#endif
	
	if (host->id == 0) {
		/* Set RST pin as GPIO, for swtich voltage between 3.3V and 1.8V, default 3.3V. (EVB) */
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_8BIT_RST_GPIO_CTL, 0);
		/* MSDC_SET_FIELD(MSDC_SW_GPIO_ENABLE_OUTPUT, SD_V33_18_SW0_ENABLE, 1); */
		/* MSDC_SET_FIELD(MSDC_SW_GPIO_OUTPUT_VALUE, SD_V33_18_SW0_VALUE, 1); */
		#if ATC_GPIO_KS
		gpiod_direction_output(msdc_gpio_desc, 1);
		#else
		gpio_request(PIN_114_SD_V33_18_SW0, "MSDC0_RST");
		gpio_direction_output(PIN_114_SD_V33_18_SW0, 1);
		#endif

		/* ================= eMMC boot, for SD0 Setting ================ */
		if (!nand_boot) {
			/* Set SD0 pins work as MSDC mode */
			MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_8BIT_GPIO_CTL, 0);
		}
		/* ================== Nand boot, for SD0 Setting ================ */
		else {
			MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_RST_GPIO_CTL, 1);
			/* Make SD0 work as SD */
			MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_GPIO_CTL, 0);
			/* Disable emmc 8bit */
			MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_8BIT_GPIO_CTL, 0x3FF);
		}
	} else if (host->id == 1) {
		/* Port1's pins was used for port1 */
		MSDC_SET_FIELD(MSDC_PAD_RST_RXDLY, SD1_DATA_PINS_AS_SD0_HIGH_4DATA, 0);
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD1_GPIO_CTL, 0);

		/* Set RST pin as GPIO, for swtich voltage between 3.3V and 1.8V, default 3.3V. (EVB) */
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD1_RST_GPIO_CTL, 1);
		/* MSDC_SET_FIELD(MSDC_SW_GPIO_ENABLE_OUTPUT, SD_V33_18_SW1_ENABLE, 1); */
		/* MSDC_SET_FIELD(MSDC_SW_GPIO_OUTPUT_VALUE, SD_V33_18_SW1_VALUE, 0); */
		#if ATC_GPIO_KS
		gpiod_direction_output(msdc_gpio_desc, 0);
		#else
		gpio_request(PIN_115_SD_V33_18_SW1, "MSDC1_RST");
		gpio_direction_output(PIN_115_SD_V33_18_SW1, 0);
		#endif

	} else if (host->id == 2) {
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD2_GPIO_CTL, 0);
		/* Set RST pin as GPIO, for swtich voltage between 3.3V and 1.8V, default 3.3V. (EVB) */
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD2_RST_GPIO_CTL, 1);

		/* MSDC_SET_FIELD(MSDC_SW_GPIO_ENABLE_OUTPUT, SD_V33_18_SW2_ENABLE, 1); */
		/* MSDC_SET_FIELD(MSDC_SW_GPIO_OUTPUT_VALUE, SD_V33_18_SW2_VALUE, 0); */
		#if ATC_GPIO_KS
		gpiod_direction_output(msdc_gpio_desc, 0);
		#else
		gpio_request(PIN_116_SD_V33_18_SW2, "MSDC2_RST");
		gpio_direction_output(PIN_116_SD_V33_18_SW2, 0);
		#endif
	} else {
		MSDC_LOG(ERR, "Wrong Host ID (%d)", host->id); /* Wrong!!! */
	}
	return 0;
}

/* When trap pin is selected to nand boot, SD0 is working in 4 bit mode for SD card, not emmc. */
static int msdc_pad_init(struct msdc_host *host, u32 nand_boot)
{
	if (!host) {
		return 1;
	}

	/*
	MSDC_LOG(ERR, "clk_drv<%u>, cmd_drv<%u>, dat_drv<%u>, slew_rate<%u>", host->hw->clk_drv, host->hw->cmd_drv,
		host->hw->dat_drv, host->hw->slew_rate);
	*/
	
	if (host->id == 0) {
		/* ================= eMMC boot, for SD0 Setting ================ */
		/* if (!nand_boot) */
		{
			/* CLK Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_RESISTOR_MASK,
				       host->hw->clk_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_PUPD_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_DRV_MASK, host->hw->clk_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* CMD Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_RESISTOR_MASK,
				       host->hw->cmd_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_DRV_MASK, host->hw->cmd_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT0 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT1 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT2 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT3 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT4 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT5 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT6 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT7 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* RST Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_PUPD_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_SR_MASK, host->hw->slew_rate);
		}
		/* ================= Nand boot, for SD0 Setting ================ */
		/* else */
		{
			/* CLK Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_RESISTOR_MASK,
				       host->hw->clk_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_PUPD_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_DRV_MASK, host->hw->clk_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* CMD Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_RESISTOR_MASK,
				       host->hw->cmd_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_DRV_MASK, host->hw->cmd_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT0 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT1 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT2 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* DAT3 Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_SR_MASK, host->hw->slew_rate);

			/* RST Pad */
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_RESISTOR_MASK,
				       host->hw->dat_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_PUPD_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_DRV_MASK, host->hw->dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_SR_MASK, host->hw->slew_rate);
		}
	} else if (host->id == 1) {
		/* CLK Pad */
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_RESISTOR_MASK, host->hw->clk_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_PUPD_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_DRV_MASK, host->hw->clk_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* CMD Pad */
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_RESISTOR_MASK, host->hw->cmd_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_DRV_MASK, host->hw->cmd_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* DAT0 Pad */
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* DAT1 Pad */
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* DAT2 Pad */
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* DAT3 Pad */
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* RST Pad */
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_PUPD_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_SR_MASK, host->hw->slew_rate);
	} else if (host->id == 2) {
		/* CLK Pad */
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_RESISTOR_MASK, host->hw->clk_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_PUPD_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_DRV_MASK, host->hw->clk_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* CMD Pad */
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_RESISTOR_MASK, host->hw->cmd_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_DRV_MASK, host->hw->cmd_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* DAT0 Pad */
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* DAT1 Pad */
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* DAT2 Pad */
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* DAT3 Pad */
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_SR_MASK, host->hw->slew_rate);

		/* RST Pad */
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_RESISTOR_MASK, host->hw->dat_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_PUPD_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_DRV_MASK, host->hw->dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_SR_MASK, host->hw->slew_rate);
	} else {
		/* Wrong!!! */
	}

	return 0;
}

static int msdc_select_clock_source(struct msdc_host *host)
{
#if (MSDC_USE_CCF==0)
	switch (host->hw->clk_src) {
	case MSDC_CLKSRC_200MHZ:
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_ARMPLL2_D2);	/* 202MHz */
		/* MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_MSDCPLL_D2); // 189MHz */
		break;

	case MSDC_CLKSRC_196MHZ:
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_MSDCPLL_D2);	/* 196MHz */
		break;

	case MSDC_CLKSRC_189MHZ:
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_DMPLL_D2);	/* 189MHz */
		break;

	case MSDC_CLKSRC_162MHZ:
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_SYSPLL_D4);
		break;

	case MSDC_CLKSRC_147MHZ:
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_APLL2_D2);
		break;

	case MSDC_CLKSRC_135MHZ:
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_MSDCPLL_D3);
		break;

	case MSDC_CLKSRC_108MHZ:
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_SYSPLL_D6);
		break;

	case MSDC_CLKSRC_27MHZ:
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_27MHZ);
		break;

	default:
		MSDC_LOG(ERR, "error source clock value: %d", host->hw->clk_src);
	}
#endif
	return 0;
}
#if MSDC_USE_CCF
#if 1
static const char *msdc_hclk_parents[] = {
	"clk27m_ck",
	"apll2_d3",
	"usbpll_d6",
	"syspll_d9",
	"usbpll_d8",
	"syspll_d12",
	"usbpll_d10",
	"syspll_d18"
};

static const char *msdc_clksrc_parents[]= {
	"armpll2_d2",   //202MHz
	"msdcpll_d2",	//196MHZ
	"dmpll_d2",		//189MHZ
	"syspll_d4",	//162MHZ
	"apll2_d2",		//147MHZ
	"msdcpll_d3",	//135MHZ
	"syspll_d6",	//108MHZ
	"clk27m_ck",	//27MHZ
};
#else
static const char *msdc_hclk_parents[] = {
	"clk27m_ck",
	"apll2_d3",
	"usbpll_d6",
	"syspll_d9",
	"usbpll_d8",
	"syspll_d12",
	"usbpll_d10",
	"syspll_d18"
};

static const char *msdc_clksrc_parents[]= {
	"clk27m_ck",
	"msdcpll_d2",
	"armpll2_d2",
	"syspll_d4",
	"usbpll_d4",
	"syspll_d6",
	"syspll_d12",
	"usbpll_d10",
	"dmpll_d2",
	"apll2_d2",
	"apll2_d3",
	"apll1_d2",
	"msdcpll_d3",
	"msdcpll_d4"
};
#endif
static int msdc_ccf_select_hclk(struct msdc_host *host,u32 hclk)
{
	int ret;
	struct clk *msdc_clk_parent;

	msdc_clk_parent=clk_get(NULL,msdc_hclk_parents[hclk]);
	ret=clk_set_parent(host->h_clk,msdc_clk_parent);
	if(ret){
		MSDC_LOG(ERR,"set hclk parent for msdc %d fail",host->id);
		return 1;
	}
	return 0;
}

static int msdc_ccf_select_clksrc(struct msdc_host *host)
{
	int ret;
	struct clk *msdc_clk_parent;
	
	msdc_clk_parent=clk_get(NULL,msdc_clksrc_parents[host->hw->clk_src]);
	ret=clk_set_parent(host->clk_source,msdc_clk_parent);
	if(ret){
		MSDC_LOG(ERR,"set clk source parent for msdc %d fail",host->id);
		return 1;
	}
	MSDC_LOG_NORMAL(pr_debug, "[%d]set clk source through ccf", host->id);
	return 0;
}

static int msdc_ccf_clkgate(struct msdc_host *host,u32 on)
{
	if(on){
		/*if (clk_is_enabled(host->clk_gate))
			clk_disable(host->clk_gate);
		if(__clk_is_prepared(host->gate))
			clk_unprepare(host->clk_gate);*/
		if(clk_prepare(host->clk_gate))
			MSDC_LOG_NORMAL(pr_err, "[%d]clk prepare fail", host->id);
	
		if(clk_enable(host->clk_gate))
			MSDC_LOG_NORMAL(pr_err, "[%d] clk enable fail", host->id);
		MSDC_LOG_NORMAL(pr_debug, "[%d]enable clk gate through ccf", host->id);
	}
	else{
		clk_disable(host->clk_gate);
		clk_unprepare(host->clk_gate);

		MSDC_LOG_NORMAL(pr_debug, "[%d]disable clk gate through ccf", host->id);
	}
	return 0;
}
#endif

/* called by msdc_drv_probe */
static void msdc_init_hw(struct msdc_host *host)
{
	u32 base = host->base;
	struct msdc_hw *hw = host->hw;
	#if 0
	u32 cur_rxdly0, cur_rxdly1;
	#endif

	MSDC_MODULE_SW_RESET(host); //reset all registers

#if MSDC_USE_CCF
	msdc_ccf_select_hclk(host,MSDC_HCLK_SEL_SYSPLL_D9);
	msdc_ccf_select_clksrc(host);
	msdc_ccf_clkgate(host,GATE_ENABLE_CLOCK);
#else
/* All host controller use same internal clock(hclock) for internal DMA */
	MSDC_SELECT_HCLK_SRC(MSDC_HCLK_SEL_SYSPLL_D9);
	/* MSDC_SELECT_HCLK_SRC(MSDC_HCLK_SEL_APLL2_D3); */
	MSDC_CLOCK_GATE(host, GATE_ENABLE_CLOCK);
	/* Set clock source, each slot has different clock source */
	msdc_select_clock_source(host);
#endif
	/* Pad init */
	msdc_pad_init(host, 0);
	msdc_pad_multi_func(host, 0);

	/* Clock Free Runing, Max Xia Add */
	/* MSDC_SET_BITS(MSDC_CFG, MSDC_CFG_CKPDN); */
	MSDC_CLR_BITS(MSDC_CFG, MSDC_CFG_CKPDN);

	/* Configure to MMC/SD mode */
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_MODE, MSDC_SDMMC);

	/* Reset */
	MSDC_RESET_HW(host->id);

	/* Disable and clear all interrupts */
	MSDC_CLR_BITS(MSDC_INTEN, MSDC_READ32(MSDC_INTEN));
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));

	/* Disable internal card detect, because AC8317 does not support this function */
	MSDC_CLR_BITS(MSDC_PS, MSDC_PS_CDEN);
	MSDC_CLR_BITS(MSDC_INTEN, MSDC_INTEN_CDSC);
	MSDC_CLR_BITS(SDC_CFG, SDC_CFG_INSWKUP);

	/* Set RISC to 4bytes access mode */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, MSDC_IOCON_RISCSZ_4BYTE);

	/* Init tuning params from config file for tuning each data line */
	if (hw->tuning_method == TUNE_EACH_DAT_LINE) {
		#if 0
		cur_rxdly0 = ((hw->dat0rddly & 0x1F) << 24) | ((hw->dat1rddly & 0x1F) << 16) |
			     ((hw->dat2rddly & 0x1F) << 8) | ((hw->dat3rddly & 0x1F) << 0);
		cur_rxdly1 = ((hw->dat4rddly & 0x1F) << 24) | ((hw->dat5rddly & 0x1F) << 16) |
			     ((hw->dat6rddly & 0x1F) << 8) | ((hw->dat7rddly & 0x1F) << 0);
		MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
		MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1);
		#else
		MSDC_WRITE32(MSDC_DAT_RDDLY0, hw->write_rxdly0);
		#endif
	}

	if ((host->hw->host_function == MSDC_EMMC) || (host->hw->host_function == MSDC_SD)) {
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_GET_BUSY_MARGIN, 1);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_GET_CRC_MARGIN, 1);
	}

	/* for safety, should clear SDC_CFG.SDIO_INT_DET_EN & set SDC_CFG.SDIO in
	   pre-loader,uboot,kernel drivers. and SDC_CFG.SDIO_INT_DET_EN will be only
	   set when kernel driver wants to use SDIO bus interrupt */
	/* Configure to enable SDIO mode. it's must otherwise sdio cmd5 failed */
	MSDC_SET_BITS(SDC_CFG, SDC_CFG_SDIO);

	/* disable detect SDIO device interupt function */
	MSDC_CLR_BITS(SDC_CFG, SDC_CFG_SDIOIDE);

	/* write crc timeout detection */
	MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 30, 1);

	/* Configure to default data timeout */
	MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, DEFAULT_DTOC);

	msdc_set_buswidth(host, MMC_BUS_WIDTH_1);

	MSDC_LOG(FUC, "init hardware done!");
}


/* called by msdc_drv_remove */
static void msdc_deinit_hw(struct msdc_host *host)
{
	u32 base = host->base;

	/* Disable and clear all interrupts */
	MSDC_CLR_BITS(MSDC_INTEN, MSDC_READ32(MSDC_INTEN));
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
}

void msdc_force_reinit(struct msdc_host *host)
{
    MSDC_LOG(ERR, "");
#if MSDC_USE_CCF
    msdc_ccf_clkgate(host,GATE_DISABLE_CLOCK);
#else
    MSDC_CLOCK_GATE(host, GATE_DISABLE_CLOCK);
#endif
    MSDC_MODULE_SW_RESET(host);
    msdc_init_hw(host);
}

/* init gpd and bd list in msdc_drv_probe */
static void msdc_init_gpd_bd(struct msdc_host *host, struct msdc_dma *dma)
{
	gpd_t *gpd = dma->gpd;
	bd_t *bd = dma->bd;
	bd_t *ptr, *prev;

	/* we just support one gpd */
	int bdlen = MAX_BD_PER_GPD;

	/* init the 2 gpd */
	memset(gpd, 0, sizeof(gpd_t) * 2);
	/* pointer to a null gpd, bug! kmalloc <-> virt_to_phys */
	/* gpd->next = (void *)virt_to_phys(gpd + 1); */
	/* bug */
	/* gpd->next = (dma->gpd_addr + 1);  */
	gpd->next = (void *)((u32) dma->gpd_addr + sizeof(gpd_t));

	/* gpd->intr = 0; */
	gpd->bdp = 1;		/* hwo, cs, bd pointer */
	/* gpd->ptr  = (void*)virt_to_phys(bd); */
	gpd->ptr = (void *)dma->bd_addr;	/* physical address */

	memset(bd, 0, sizeof(bd_t) * bdlen);
	ptr = bd + bdlen - 1;
	/* 0 or 1 [Fix me] */
	/* ptr->eol  = 1;   */
	/* ptr->next = 0; */

	while (ptr != bd) {
		prev = ptr - 1;
		prev->next = (void *)(dma->bd_addr + sizeof(bd_t) * (ptr - bd));
		ptr = prev;
	}
}

void msdc_init_dma_latest_address(void)
{
	struct dma_addr *ptr, *prev;
	int bdlen = MAX_BD_PER_GPD;

	memset(msdc_latest_dma_address, 0, sizeof(struct dma_addr) * bdlen);
	ptr = msdc_latest_dma_address + bdlen - 1;

	while (ptr != msdc_latest_dma_address) {
		prev = ptr - 1;
		prev->next =
			(void *)(msdc_latest_dma_address +
				 sizeof(struct dma_addr) * (ptr - msdc_latest_dma_address));
		ptr = prev;
	}

}

struct msdc_host *msdc_get_host(int host_function, bool boot, bool secondary)
{
	int host_index = 0;
	struct msdc_host *host = NULL;

	for (; host_index < HOST_MAX_NUM; ++host_index) {
		if (!atc_msdc_host[host_index]) {
			continue;
		}

		if ((host_function == atc_msdc_host[host_index]->hw->host_function)
		    && (boot == atc_msdc_host[host_index]->hw->boot)) {
			host = atc_msdc_host[host_index];
			break;
		}
	}

	if (secondary && (host_function == MSDC_SD)) {
		host = atc_msdc_host[2];
	}

	if (host == NULL) {
		MSDC_LOG_NORMAL(pr_err, "This host(<host_function:%d> <boot:%d><secondary:%d>) isn't in MSDC host config list",
			 host_function, boot, secondary);
	}

	return host;
}
EXPORT_SYMBOL(msdc_get_host);

struct gendisk *mmc_get_disk(struct mmc_card *card)
{
	struct mmc_blk_data *md;

	BUG_ON(!card);
	md = mmc_get_drvdata(card);
	BUG_ON(!md);
	BUG_ON(!md->disk);

	return md->disk;
}

/*******************************************************************************************/

/* New idea for card detect: */
/* We maybe use GPIO status for retry init card. when mmc init new card failed, it can 'callback' to host driver, */
/* so, we create a new monitor thread for detect GPIO status for whether new card is inserted, */
/* if the new card is inserted and mmc core init it failed, we triger mmc core to re-init new card. */
/* if (host->card_status && host->card_inserted && !host->init_status) reinit-card; */

/*****************************************************************************************/

static int msdc_cd_polling_thread_func(void *arg)
{
	int insertcard = -1;
	int id;
	struct msdc_host *host = NULL;

	/* daemonize("msdc_cd_polling_thread"); */

	do {
		for (id = 0; id < HOST_MAX_NUM; id++) {
			host = atc_msdc_host[id];

			if (!host) {
				continue;
			}

			if (!(host->hw->flags & MSDC_REMOVABLE)) {
				continue;
			}

			if (card_detect_gpio(host) > 1) {
				continue;
			}

			host->card_status = card_detect_gpio(host);

			/* status has been changed */
			if (host->card_status != host->old_status) {
				if (host->card_status)
					/* msdc_init_hw(host); */
				{
					insertcard = 1;
				} else {
					insertcard = 0;
				}

				if (host->block_bad_card) {
					host->block_bad_card = 0;
				}

				host->old_status = host->card_status;
				MSDC_LOG_NORMAL(pr_info, "[%d]@@@@ card detect = %d", host->id, insertcard);
				tasklet_hi_schedule(&host->card_tasklet);
			}
		}

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ / 50);
	} while (run_card_detect);

	return 0;
}


void card_detect_gpio_config(struct msdc_host *host)
{
	#if ATC_GPIO_KS
	struct gpio_desc * msdc_gpio_desc;
	#endif

	/* set card detect gpio pull-up */
	if ((host->id != 0) && (host->id != 1)) {
		MSDC_SET_FIELD(0xFD000400, 1 << (18 + host->id), 1);
		MSDC_SET_FIELD(0xFD000418, 1 << (18 + host->id), 0);
	} else {
		MSDC_SET_FIELD(0xFD000400, 1 << (18 + host->id), 0);
		MSDC_SET_FIELD(0xFD000418, 1 << (18 + host->id), 1);
	}

	/* set eint clock */
	MSDC_SET_FIELD(0xFD000018, 0x30000000, 0x0);
	MSDC_SET_FIELD(0xFD008740, 0x1FFF, 0x10FF);

	switch (host->id) {
	case 0:
		//MSDC_SET_FIELD(0xFD008754, 0x3FF, 0x108);//Not use now, use for GPIO70(RTS).
		/* EINT4 function to msdc0 card detect pin */
		//MSDC_CONFIG_CD_EINT(host, MSDC_EINT_EN_ENABLE, MSDC_EINT_TYPE_DUAL_EDGE);
		#if ATC_GPIO_KS
		msdc_gpio_desc = gpio_to_desc(host->cd_gpio);
		gpiod_direction_input(msdc_gpio_desc);
		#else
		//bsp_pinset(EINT4_SEL, PINMUX_FUNCTION3);
		#endif
		break;

	case 1:
		MSDC_SET_FIELD(0xFD008758, 0x3FF, 0x108);
		/* EINT5 function to msdc1 card detect pin */
		MSDC_CONFIG_CD_EINT(host, MSDC_EINT_EN_ENABLE, MSDC_EINT_TYPE_DUAL_EDGE);
		#if ATC_GPIO_KS
		msdc_gpio_desc = gpio_to_desc(host->cd_gpio);
		gpiod_direction_input(msdc_gpio_desc);
		#else
		bsp_pinset(EINT5_SEL, PINMUX_FUNCTION3);
		#endif
		break;

	case 2:
		MSDC_SET_FIELD(0xFD00875C, 0x3FF, 0x108);
		/* EINT6 function to msdc2 card detect pin */
		MSDC_CONFIG_CD_EINT(host, MSDC_EINT_EN_ENABLE, MSDC_EINT_TYPE_DUAL_EDGE);
		#if ATC_GPIO_KS
		msdc_gpio_desc = gpio_to_desc(host->cd_gpio);
		gpiod_direction_input(msdc_gpio_desc);
		#else
		bsp_pinset(EINT6_SEL, PINMUX_FUNCTION3);
		#endif
		break;
	}
}

static int msdc_auto_select_clk(unsigned int clk_max)
{
	int i, j;
	int min = 200000000, num = 0, tmp;

	for (i = 0; i < sizeof(hclksrc) / sizeof(u32); i++) {
		j = 1;

		while (1) {
			tmp = hclksrc[i] / j;

			if (tmp <= clk_max) {
				if (min > clk_max - tmp) {
					min = clk_max - tmp;
					num = i;
				}

				break;
			}

			j++;
		}

		if (min == 0) {
			break;
		}
	}

	return num;
}

/* This function is used for removable wifi sub-board */
/* return value: 0-no sido wifi, other SD slot number of sdio wifi */
/* type: 0-on-board wifi, 1-removable wifi (wifi sub-board) */
#define WIFI_TYPE_ON_BOARD_SD1		(0)
#define WIFI_TYPE_REMOVEABLE		(1)

u32 msdc_get_removable_wifi_slot(void)
{
	u32 i = 0;
	struct msdc_host *host = 0;

	/* 1. Scan already be inserted card */
	for (i = 1; i < HOST_MAX_NUM; i++) {	/* SD Slot 0 has been used for eMMC */
		host = atc_msdc_host[i];

		if (host && host->mmc && host->mmc->card) {
			if (mmc_card_sdio(host->mmc->card)) {
				return i;
			}
		}
	}

	/* 2. Scan Last time SDIO WIFI Slot */
	/* SD Slot 0 has been used for eMMC */
	for (i = 1; i < HOST_MAX_NUM; i++) {
		host = atc_msdc_host[i];

		if (host && host->hw && (host->hw->host_function == MSDC_SDIO)) {
			return i;
		}
	}

	return 0;
}
EXPORT_SYMBOL(msdc_get_removable_wifi_slot);

#ifdef CONFIG_SDIO_CLK_SWITCH
int msdc_change_sdio_clock(unsigned int slot,unsigned char clk)
{
	struct msdc_host *host = 0;
	
	u32 base;
	
	if(slot>=HOST_MAX_NUM){
		MSDC_LOG(ERR, " the slot<%d> num is err",slot);
		return -1;
	}

	if(clk>MSDC_CLKSRC_AUTO){
		MSDC_LOG(ERR, " the clk<%d> value is err",clk);
		return -1;
	}
	host = atc_msdc_host[slot];

	if (!host){
		MSDC_LOG(ERR, "host of msdc%d is null pointer", slot);
		return -1;
	}
	
	if (host->hw->clk_src == MSDC_CLKSRC_AUTO) {
        host->hw->clk_src = msdc_auto_select_clk(host->hw->clk_max * MSDC_CLK_1MHZ);
    }
	else
		host->hw->clk_src=clk;

    /*Set clock source for msdc host*/
	msdc_select_clock_source(host);
	MSDC_LOG_NORMAL(pr_info, "msdc%d change clk from %d  to  %d",slot,host->hclk,hclksrc[clk]);
	host->hclk= hclksrc[clk];
	mdelay(5);
	return 0;
}
#endif

#ifdef CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE
static void msdc_detect_pre_setting(u32 slot)
{
	static int prev_chip_type = ATC_WIFI_CHIP_TYPE_UNKNOWN;
    struct msdc_host *host = NULL;
    struct msdc_hw *hw = NULL;
	struct mmc_host *mmc = NULL;
	int chip_type = atc_combo_get_chip_type();

	if (chip_type == prev_chip_type) {
		return;
	}
	host = atc_msdc_host[slot];
	hw = host->hw;
	mmc = host->mmc;
	MSDC_LOG_NORMAL(pr_info, "[%d] %s for chip_type %d -> %d",
			slot, __func__, prev_chip_type, chip_type);
	prev_chip_type = chip_type;

	if (chip_type == ATC_WIFI_CHIP_TYPE_AIC8800_SDIO) {
		hw->clk_src = MSDC_CLKSRC_108MHZ;
		host->hclk = hclksrc[hw->clk_src];
		hw->clk_max = 50;
		mmc->f_max = hw->clk_max * MSDC_CLK_1MHZ;
		hw->cmd_drv_sd_18 = 0x2;
		hw->dat_drv_sd_18 = 0x2;
		hw->clk_drv_sd_18 = 0x2;
	} else {
		hw->clk_src = MSDC_CLKSRC_200MHZ;
		host->hclk = hclksrc[hw->clk_src];
		hw->clk_max = 200;
		mmc->f_max = hw->clk_max * MSDC_CLK_1MHZ;
		hw->cmd_drv_sd_18 = 0x2F;
		hw->dat_drv_sd_18 = 0x2F;
		hw->clk_drv_sd_18 = 0x2F;
	}
	msdc_force_reinit(host);
}
#endif

#ifdef CONFIG_SDIO_CLK_SWITCH
int msdc_detect_change(unsigned int slot, u32 enable, u32 type,u32 clksrc)
#else
int msdc_detect_change(unsigned int slot, u32 enable, u32 type)
#endif
{
	struct msdc_host *host = 0;
	u8 prev_host_function;
	sdio_init_status=0;
	int i;
	/* u32 aleady_inserted = 0; */

	if (slot >= HOST_MAX_NUM) {
		MSDC_LOG(ERR, "msdc_detect_change function paramer <%d> err", slot);
		return 1;
	}

	if (type == WIFI_TYPE_REMOVEABLE) {
		slot = msdc_get_removable_wifi_slot();

		if (slot == 0) {	/* can't found the wifi sub-board */
			MSDC_LOG(ERR, "Can not find removable SDIO WIFI in SD1 & SD2");
			return 1;
		}

		MSDC_LOG(ERR, "Found removable SDIO WIFI in SD Slot%d", slot);
	}

	host = atc_msdc_host[slot];

	if (!host) {
		MSDC_LOG(ERR, "host of msdc%d is null pointer", slot);
		return 1;
	}

#ifdef CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE
	if (enable) {
		msdc_detect_pre_setting(slot);
	}
#endif

	prev_host_function = host->hw->host_function;

	/* Bypass SD Card has been inserted */
	if (host->mmc && host->mmc->card && mmc_card_sd(host->mmc->card)) {
		MSDC_LOG(ERR, "ERROR: Slot%d has been inserted a SD card.", slot);

		/* Reset SD Attributes */
		if (host->hw->host_function == MSDC_SDIO) {
			host->hw->flags |= MSDC_REMOVABLE;
			host->hw->host_function = MSDC_SD;
			enable_irq(161 + slot);	/* Enable eint for Card Detect */
		}

		return 1;
	}

	if (enable) {
		host->hw->flags &= ~MSDC_REMOVABLE;
		host->hw->host_function = MSDC_SDIO;
		disable_irq(161 + slot);	/* Disable eint for Card Detect */
	}
	else {
		host->hw->flags |= MSDC_REMOVABLE;
		host->hw->host_function = MSDC_SDIO;

		if (type == WIFI_TYPE_REMOVEABLE) {
			enable_irq(161 + slot);        /* Enable eint for Card Detect */
		}
	}

	if(host->hw->flags & MSDC_UHS1){
		host->mmc->caps |= MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
				MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104; 
		msdc_host_mode[host->id] = host->mmc->caps;
	}
	MSDC_LOG_NORMAL(pr_info, "WIFI SDIO: Slot%d %s", slot, enable ? "enable" : "disable");
	host->card_inserted = enable;
#ifdef CONFIG_SDIO_CLK_SWITCH
	if(enable&&(host->hw->host_function == MSDC_SDIO))
		msdc_change_sdio_clock(slot,clksrc);
#endif
	if((prev_host_function == MSDC_SD) && (host->hw->host_function == MSDC_SDIO)) {
		MSDC_LOG_NORMAL(pr_info, "[%d]change to sdio, call unregister_pm_notifier", host->id);
		unregister_pm_notifier(&host->mmc->pm_notify);
	}
	if(!enable && host->hw->host_function == MSDC_SDIO) {
		msdc_save_setting(host->mmc);
	}

	/* Triger to rescan for init card */
	if (host && host->mmc && enable && (type == WIFI_TYPE_REMOVEABLE)) {
		if (!host->mmc->card) {	/* Not init, rescan and init it */
			mmc_detect_change(host->mmc, 0);
		}
	} else {
		mmc_detect_change(host->mmc, 0);
	}

	if(enable&&(!host->mmc->card)){
		for(i=0;i<100;i++){
			if(sdio_init_status==SDIO_INIT_SUCCESS)
				return 0;
			else if(sdio_init_status==SDIO_INIT_FAIL)
				return 1;
			mdelay(10);
		}
		return 1;
	}
	else
		return 0;
	MSDC_LOG(INFO, "msdc_detect_change success");
}
EXPORT_SYMBOL(msdc_detect_change);



/* SIDO WIFI VID and PID definitions */
#define MT5931_VENDER_ID	(0x0000037A)
#define MT5931_DEVICE_ID	(0x00005931)
#define MT6630_VENDER_ID	(0x0000037A)
#define MT6630_DEVICE_ID	(0x00006630)

/* return value: 0-no sido wifi, 1-MT5931, 2-MT6630 */
int msdc_get_sdio_wifi(u32 slot)
{
	int i = 0;
	struct msdc_host *host = 0;

	if (slot >= HOST_MAX_NUM) {
		MSDC_LOG(ERR, "msdc_get_sdio_wifi function paramer <%d> err", slot);
		return 0;
	}

	for (i = 1; i < HOST_MAX_NUM; i++) {	/* SD Slot 0 has been used for eMMC */
		host = atc_msdc_host[i];

		if (host && host->mmc && host->mmc->card) {
			if (mmc_card_sdio(host->mmc->card)) {
				MSDC_LOG(ERR, "WIFI SDIO (Slot%d): VID=0x%08X, PID=0x%08X", i,
					 host->mmc->card->cis.vendor, host->mmc->card->cis.device);

				if ((host->mmc->card->cis.vendor == MT5931_VENDER_ID) &&
					(host->mmc->card->cis.device == MT5931_DEVICE_ID)) {
					return 1;
				} else if ((host->mmc->card->cis.vendor == MT6630_VENDER_ID) &&
				(host->mmc->card->cis.device == MT6630_DEVICE_ID)) {
					return 2;
				}
			}

			return 1;
		}
	}

	return 0;
}
EXPORT_SYMBOL(msdc_get_sdio_wifi);


/* define thread flags for card detect thread */
#define CLONE_KERNEL	(CLONE_FS | CLONE_FILES | CLONE_SIGHAND)

#if MSDC_USE_CCF
static int msdc_get_ccf_clk_pointer(struct platform_device *pdev,
				struct msdc_host *host)
{

	if (host->id == 0) {
		host->clk_source= devm_clk_get(&pdev->dev, "MSDC0-CLKSRC");
		host->h_clk = devm_clk_get(&pdev->dev, "MSDC0-HCLK");
		host->clk_gate= devm_clk_get(&pdev->dev, "MSDC0-CLKGATE");
	} else if (host->id == 1) {
		host->clk_source= devm_clk_get(&pdev->dev, "MSDC1-CLKSRC");
		host->h_clk = devm_clk_get(&pdev->dev, "MSDC1-HCLK");
		host->clk_gate= devm_clk_get(&pdev->dev, "MSDC1-CLKGATE");
	} else if (host->id == 2) {
		host->clk_source= devm_clk_get(&pdev->dev, "MSDC2-CLKSRC");
		host->h_clk = devm_clk_get(&pdev->dev, "MSDC2-HCLK");
		host->clk_gate= devm_clk_get(&pdev->dev, "MSDC2-CLKGATE");
	} 
	
	if (IS_ERR(host->clk_source)) {
		MSDC_LOG(ERR,"can not get msdc%d clksrc\n", host->id);
		return 1;
	} else if(IS_ERR(host->h_clk)) {
		MSDC_LOG(ERR,"can not get msdc%d hclk\n", host->id);
		return 1;
	}else if(IS_ERR(host->clk_gate)){
		MSDC_LOG(ERR,"can not get msdc%d clkgata\n", host->id);
		return 1;
	}
	
	return 0;
}
#endif
static int msdc_get_pinctl_settings(struct msdc_host *host)
{
	struct mmc_host *mmc = host->mmc;
	struct device_node *np = mmc->parent->of_node;
	struct device_node *pinctl_node;
	struct device_node *pins_cmd_node;
	struct device_node *pins_dat_node;
	struct device_node *pins_clk_node;
	struct device_node *pinctl_sdr104_node;
	struct device_node *pinctl_sdr50_node;
	struct device_node *pinctl_ddr50_node;

	/* parse pinctl settings */
	pinctl_node = of_parse_phandle(np, "pinctl", 0);
	pins_cmd_node = of_get_child_by_name(pinctl_node, "pins_cmd");
	of_property_read_u8(pins_cmd_node, "drive-strength", &host->hw->cmd_drv);

	pins_dat_node = of_get_child_by_name(pinctl_node, "pins_dat");
	of_property_read_u8(pins_dat_node, "drive-strength", &host->hw->dat_drv);

	pins_clk_node = of_get_child_by_name(pinctl_node, "pins_clk");
	of_property_read_u8(pins_clk_node, "drive-strength", &host->hw->clk_drv);

/********************************************************************************************************/
	pinctl_sdr104_node = of_parse_phandle(np, "pinctl_sdr104", 0);
	pins_cmd_node = of_get_child_by_name(pinctl_sdr104_node, "pins_cmd");
	of_property_read_u8(pins_cmd_node, "drive-strength", &host->hw->cmd_drv_sd_18);

	pins_dat_node = of_get_child_by_name(pinctl_sdr104_node, "pins_dat");
	of_property_read_u8(pins_dat_node, "drive-strength", &host->hw->dat_drv_sd_18);

	pins_clk_node = of_get_child_by_name(pinctl_sdr104_node, "pins_clk");
	of_property_read_u8(pins_clk_node, "drive-strength", &host->hw->clk_drv_sd_18);

/********************************************************************************************************/
	pinctl_sdr50_node = of_parse_phandle(np, "pinctl_sdr50", 0);
	pins_cmd_node = of_get_child_by_name(pinctl_sdr50_node, "pins_cmd");
	of_property_read_u8(pins_cmd_node, "drive-strength", &host->hw->cmd_drv_sd_18_sdr50);

	pins_dat_node = of_get_child_by_name(pinctl_sdr50_node, "pins_dat");
	of_property_read_u8(pins_dat_node, "drive-strength", &host->hw->dat_drv_sd_18_sdr50);

	pins_clk_node = of_get_child_by_name(pinctl_sdr50_node, "pins_clk");
	of_property_read_u8(pins_clk_node, "drive-strength", &host->hw->clk_drv_sd_18_sdr50);

/********************************************************************************************************/
	pinctl_ddr50_node = of_parse_phandle(np, "pinctl_ddr50", 0);
	pins_cmd_node = of_get_child_by_name(pinctl_ddr50_node, "pins_cmd");
	of_property_read_u8(pins_cmd_node, "drive-strength", &host->hw->cmd_drv_sd_18_ddr50);

	pins_dat_node = of_get_child_by_name(pinctl_ddr50_node, "pins_dat");
	of_property_read_u8(pins_dat_node, "drive-strength", &host->hw->dat_drv_sd_18_ddr50);

	pins_clk_node = of_get_child_by_name(pinctl_ddr50_node, "pins_clk");
	of_property_read_u8(pins_clk_node, "drive-strength", &host->hw->clk_drv_sd_18_ddr50);

	return 0;
}

static int msdc_get_rigister_settings(struct msdc_host *host)
{
	struct mmc_host *mmc = host->mmc;
	struct device_node *np = mmc->parent->of_node;
	struct device_node *register_setting_node = NULL;

	/*parse hw property settings*/
	register_setting_node = of_parse_phandle(np, "register_setting", 0);
	if (register_setting_node) {
		of_property_read_u8(register_setting_node, "dat0rddly", &host->hw->dat0rddly);
		of_property_read_u8(register_setting_node, "dat1rddly", &host->hw->dat1rddly);
		of_property_read_u8(register_setting_node, "dat2rddly", &host->hw->dat2rddly);
		of_property_read_u8(register_setting_node, "dat3rddly", &host->hw->dat3rddly);
		of_property_read_u8(register_setting_node, "dat4rddly", &host->hw->dat4rddly);
		of_property_read_u8(register_setting_node, "dat5rddly", &host->hw->dat5rddly);
		of_property_read_u8(register_setting_node, "dat6rddly", &host->hw->dat6rddly);
		of_property_read_u8(register_setting_node, "dat7rddly", &host->hw->dat7rddly);

		of_property_read_u8(register_setting_node, "datwrddly", &host->hw->datwrddly);
		of_property_read_u8(register_setting_node, "cmdrrddly", &host->hw->cmdrrddly);
		of_property_read_u8(register_setting_node, "cmdrddly", &host->hw->cmdrddly);

		of_property_read_u8(register_setting_node, "cmd_edge", &host->hw->cmd_edge);
		of_property_read_u8(register_setting_node, "rdata_edge", &host->hw->rdata_edge);
		of_property_read_u8(register_setting_node, "wdata_edge", &host->hw->wdata_edge);
	} else {
		MSDC_LOG_NORMAL(pr_err, "[%d]register_setting is not found in DT", host->id);
		return 1;
	}

	return 0;
}


/**
 *	msdc_of_parse() - parse host's device-tree node
 *	@host: host whose node should be parsed.
 *
 */
int msdc_of_parse(struct mmc_host *mmc)
{
	struct device_node *np;
	struct msdc_host *host = mmc_priv(mmc);
	int ret = 0;
	int len = 0;

	if ((!mmc->parent) || (!mmc->parent->of_node))
		return 1;

	np = mmc->parent->of_node;
	host->mmc = mmc;  /* msdc_check_init_done() need */
	host->hw = kzalloc(sizeof(struct msdc_hw), GFP_KERNEL);

	/*basic settings*/
	if (0 == strcmp(np->name, "MSDC0")) {
		host->id = 0;
		//mmc->index = 0;
	}
	else if (0 == strcmp(np->name, "MSDC1")) {
		host->id = 1;
		//mmc->index = 1;
	}
	else if (0 == strcmp(np->name, "MSDC2")) {
		host->id = 2;
		//mmc->index = 2;
	}
	else {
		MSDC_LOG_NORMAL(pr_err, "[%d]parse DT for host index error. np-name: %s", host->id, np->name);
		return 1;
	}

	MSDC_LOG_NORMAL(pr_debug, "[%d]of msdc DT probe %s!", host->id, np->name);

	/* iomap register */
	host->base = (u32)of_iomap(np, 0);
	if (!host->base) {
		MSDC_LOG_NORMAL(pr_err, "[%d]can't of_iomap for msdc!!", host->id);
		return -ENOMEM;
	} else {
		MSDC_LOG_NORMAL(pr_debug, "[%d]of_iomap for msdc @ 0x%08X", host->id, host->base);
	}
	
	/* get irq #  */
	host->irq = irq_of_parse_and_map(np, 0);
	MSDC_LOG_NORMAL(pr_debug, "[%d]get irq # %d", host->id, host->irq);
	BUG_ON(host->irq < 0);

	/* get clk_src */
	if (of_property_read_u8(np, "clk_src", &host->hw->clk_src)) {
		MSDC_LOG_NORMAL(pr_err, "[%d]error: clk_src isn't found in DT.", host->id);
		goto out;
	}

	/*Returns 0 on success, -EINVAL if the prope_mrty does not exist,
	* -ENODATA if property does not have a value, and -EOVERFLOW if the
	* property data isn't large enough.*/

	if (of_property_read_u8(np, "host_function", &host->hw->host_function)) {
		MSDC_LOG_NORMAL(pr_err, "[%d]host_function isn't found in DT", host->id);
	}

	if (of_property_read_u8(np, "cd_method", &host->hw->cd_method)) {
		MSDC_LOG_NORMAL(pr_err, "[%d]cd_method isn't found in DT", host->id);
	}

	if (of_property_read_u8(np, "tuning_method", &host->hw->tuning_method)) {
		MSDC_LOG_NORMAL(pr_err, "[%d]tuning_method isn't found in DT", host->id);
	}

	if (of_property_read_u8(np, "slew_rate", &host->hw->slew_rate)) {
		MSDC_LOG_NORMAL(pr_err, "[%d]slew_rate isn't found in DT", host->id);
	}

	if (of_property_read_u8(np, "clk_resistor", &host->hw->clk_resistor)) {
		MSDC_LOG_NORMAL(pr_err, "[%d]clk_resistor isn't found in DT", host->id);
	}

	if (of_property_read_u8(np, "dat_resistor", &host->hw->dat_resistor)) {
		MSDC_LOG_NORMAL(pr_err, "[%d]dat_resistor isn't found in DT", host->id);
	}

	if (of_property_read_u8(np, "cmd_resistor", &host->hw->cmd_resistor)) {
		MSDC_LOG_NORMAL(pr_err, "[%d]cmd_resistor isn't found in DT", host->id);
	}

	/* Tuning Parameters */
	of_property_read_u32(np, "read_pre_setting_en", &host->hw->read_pre_setting_en);
	of_property_read_u32(np, "read_dat_latch_ck_sel", &host->hw->read_dat_latch_ck_sel);
	of_property_read_u32(np, "read_ckgen_delay_sel", &host->hw->read_ckgen_delay_sel);
	of_property_read_u32(np, "read_sample_edge", &host->hw->read_sample_edge);
	of_property_read_u32(np, "read_pad_delay", &host->hw->read_pad_delay);
	of_property_read_u32(np, "ddr_read_dat_latch_ck_sel", &host->hw->ddr_read_dat_latch_ck_sel);
	of_property_read_u32(np, "ddr_read_ckgen_delay_sel", &host->hw->ddr_read_ckgen_delay_sel);
	of_property_read_u32(np, "read_patch_bit1_wrdat_crcs", &host->hw->read_patch_bit1_wrdat_crcs);
	of_property_read_u32(np, "read_patch_bit1_cmdrsp_ta", &host->hw->read_patch_bit1_cmdrsp_ta);
	of_property_read_u32(np, "read_rxdly0", &host->hw->read_rxdly0);
	of_property_read_u32(np, "read_rxdly1", &host->hw->read_rxdly1);
	of_property_read_u32(np, "write_pre_setting_en", &host->hw->write_pre_setting_en);
	of_property_read_u32(np, "write_dat_latch_ck_sel", &host->hw->write_dat_latch_ck_sel);
	of_property_read_u32(np, "write_ckgen_delay_sel", &host->hw->write_ckgen_delay_sel);
	of_property_read_u32(np, "write_sample_edge", &host->hw->write_sample_edge);
	of_property_read_u32(np, "write_pad_delay", &host->hw->write_pad_delay);
	of_property_read_u32(np, "write_internal_delay", &host->hw->write_internal_delay);
	of_property_read_u32(np, "ddr_read_ckgen_delay_sel", &host->hw->ddr_read_ckgen_delay_sel);
	of_property_read_u32(np, "ddr_write_dat_latch_ck_sel", &host->hw->ddr_write_dat_latch_ck_sel);
	of_property_read_u32(np, "ddr_write_ckgen_delay_sel", &host->hw->ddr_write_ckgen_delay_sel);
	of_property_read_u32(np, "write_patch_bit1_wrdat_crcs", &host->hw->write_patch_bit1_wrdat_crcs);
	of_property_read_u32(np, "write_patch_bit1_cmdrsp_ta", &host->hw->write_patch_bit1_cmdrsp_ta);
	of_property_read_u32(np, "write_rxdly0", &host->hw->write_rxdly0);
	
	of_property_read_u32(np, "cmdrtactr_sdr104", &host->hw->cmdrtactr_sdr104);
	of_property_read_u32(np, "wdatcrctactr_sdr104", &host->hw->wdatcrctactr_sdr104);
	of_property_read_u32(np, "intdatlatcksel_sdr104", &host->hw->intdatlatcksel_sdr104);
	
	of_property_read_u32(np, "flags", &host->hw->flags);
	of_property_read_u32(np, "clk_max", &host->hw->clk_max);
	of_property_read_u32(np, "cd_gpio", &host->cd_gpio);

	if (of_find_property(np, "bootable", &len)) {
		host->hw->boot = 1;
	}

	/*get cd_level*/
	of_property_read_u8(np, "cd_level", &host->hw->cd_level);

	/* get msdc flag(caps)*/
	if (of_find_property(np, "msdc-sys-suspend", &len)) {
		host->hw->flags |= MSDC_SYS_SUSPEND;
	}

	ret = msdc_get_rigister_settings(host);
	ret = msdc_get_pinctl_settings(host);

	return 0;

out:
	return ret;
}

const char *atc_msdc_sw_gpio_fct[] = {"msdc0sw", "msdc1sw", "msdc2sw"};
const char *atc_msdc_cd_gpio_fct[] = {"msdc0cd", "msdc1cd", "msdc2cd"};

int msdc_drv_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct msdc_host *host;
	struct msdc_hw *hw;
	u32 base;
	int ret = 0;

	#if ATC_GPIO_KS
	unsigned int cd_irq = 0;
	struct gpio_desc *msdc_gpio_desc;
	#endif

	/* Allocate MMC host for this device */
	mmc = mmc_alloc_host(sizeof(struct msdc_host), &pdev->dev);
	if (!mmc) {
		return -ENOMEM;
	}

	ret = mmc_of_parse(mmc);
	if (ret) {
		MSDC_LOG_NORMAL(pr_err, "mmc_of_parse DT happens error!!");
		mmc_free_host(mmc);
		return 1;
	}

	ret = msdc_of_parse(mmc);
	if (ret) {
		MSDC_LOG_NORMAL(pr_err, "msdc_of_parse DT happens error!!");
		mmc_free_host(mmc);
		return 1;
	}

	host = mmc_priv(mmc);
	base = host->base;
	hw = host->hw;

	if(host->id == 0) {
		/* Version information output */
		MOD_VERSION_INFO("MSDC", 1, 0, 1);
	}

	/* Set host parameters to mmc */
	mmc->ops = &atc_msdc_ops;
	mmc->f_min = HOST_MIN_MCLK;
	mmc->ocr_avail = MSDC_OCR_AVAIL;
	mmc->ios.bus_width = 0;	/* Default is 1bit mode */

	mmc->caps |= MMC_CAP_ERASE | MMC_CAP_WAIT_WHILE_BUSY;

	/* Max Xia Add, for eMMC is rescaned and identifed as eMMC only. */
	if (hw->host_function == MSDC_EMMC) {
		mmc->caps2 |= MMC_CAP2_FUNC_EMMC;
	}
	else {
		mmc->caps2 |= MMC_CAP2_NO_PRESCAN_POWERUP;
	}

	/* MMC core transfer sizes tunable parameters */
	mmc->max_segs = MAX_HW_SGMTS;
	mmc->max_seg_size = MAX_SGMT_SZ;
	mmc->max_blk_size = HOST_MAX_BLKSZ;
	mmc->max_req_size = MAX_REQ_SZ;
	mmc->max_blk_count = mmc->max_req_size;

	mmc->remove_bad_sd = msdc_remove_bad_sd;

	host = mmc_priv(mmc);
	host->hw = hw;
	host->mmc = mmc;
	host->error = 0;
	host->timing = 0;
	host->mclk = 0;		/* mclk: the request clock of mmc sub-system */
	host->sclk = 0;		/* sclk: the really clock after divition */
	host->pm_state = PMSG_RESUME;
	host->suspend = 0;
	host->power_mode = MMC_POWER_OFF;
	host->power_control = NULL;
	host->power_switch = NULL;
	host->sdio_wifi_type = -1;

	host->last_dat_err_cmd_opcode = 0;
	host->last_dat_err_cmd_arg = 0;
	host->last_dat_err_intr = 0;

	host->last_cmd_err_cmd_opcode = 0;
	host->last_cmd_err_cmd_arg = 0;
	host->last_cmd_err_intr = 0;

	#if ATC_GPIO_KS
	/* Parse Vol SW GPIO */
	msdc_gpio_desc = __gpiod_get(&(pdev->dev), atc_msdc_sw_gpio_fct[host->id], GPIOD_ASIS);
	if (IS_ERR(msdc_gpio_desc)) {
		MSDC_LOG(ERR, "Can't get MSDC%d SW GPIO from DTS.", host->id);
	} else {
		host->vol_sw_gpio = desc_to_gpio(msdc_gpio_desc);
		MSDC_LOG(ERR, "Get MSDC%d SW GPIO from DTS: %d", host->id, host->vol_sw_gpio);
	}

	msdc_gpio_desc = __gpiod_get(&(pdev->dev), atc_msdc_cd_gpio_fct[host->id], GPIOD_ASIS);
	if (IS_ERR(msdc_gpio_desc)) {
		MSDC_LOG(ERR, "Can't get MSDC%d CD GPIO from DTS.", host->id);
	} else {
		host->cd_gpio = desc_to_gpio(msdc_gpio_desc);
		MSDC_LOG(ERR, "Get MSDC%d CD GPIO from DTS: %d", host->id, host->cd_gpio);
	}
	#endif
	
	/* Limit Max and Min Clock */
	if ((hw->clk_max < 4) || (hw->clk_max > 200)) {
		MSDC_LOG(ERR, "Config Max Clock Error, Max Clock Range: 4MHz -- 200MHz");
		/* Set to SD Max default clock */
		hw->clk_max = 50;
	}

	if (hw->clk_src == MSDC_CLKSRC_AUTO) {
		hw->clk_src = msdc_auto_select_clk(hw->clk_max * MSDC_CLK_1MHZ);
	}
	host->hclk = hclksrc[hw->clk_src];	/* hclk: clock of clock source to msdc controller */
	
	/* Change Max Clock to Host Clock Source */
	host->mmc->f_max = hw->clk_max * MSDC_CLK_1MHZ;

	/* !!!We can limit eMMC work Clock here!!! */
#if MSDC_FORCE_REDUCE_EMMC_CLOCK

	if (host->hw->host_function == MSDC_EMMC) {
		MSDC_LOG_NORMAL(pr_err, "!!! eMMC Max Clock is set to %d!", MSDC_EMMC_WORK_CLOCK);
		host->mmc->f_max = MSDC_EMMC_WORK_CLOCK;
	}

#endif /* MSDC_FORCE_REDUCE_EMMC_CLOCK */

	/* SD3.0 Set switch vol func pointer. */
	if (hw->flags & MSDC_UHS1) {
		host->power_switch = msdc_sd_power_switch;
	}

	host->timeout_ns = 0;
	host->timeout_clks = DEFAULT_DTOC * 1048576;

#ifdef MTK_MSDC_USE_CMD23
		if (host->hw->host_function == MSDC_EMMC)
			mmc->caps |= MMC_CAP_ERASE | MMC_CAP_CMD23;
		else
			mmc->caps |= MMC_CAP_ERASE;
#else
		mmc->caps |= MMC_CAP_ERASE;
#endif
	/*
	// Enable AutoCMD12
	if(host->hw->host_function != MSDC_SDIO){
		host->autocmd = MSDC_AUTOCMD12;
	}
	else{
		host->autocmd = 0;
	}*/
#ifndef MTK_MSDC_USE_CMD23
		if (host->hw->host_function != MSDC_SDIO)
			host->autocmd |= MSDC_AUTOCMD12;
		else
			host->autocmd &= ~MSDC_AUTOCMD12;
#else
		if (host->hw->host_function == MSDC_EMMC) {
			host->autocmd &= ~MSDC_AUTOCMD12;
	
#if (1 == MSDC_USE_AUTO_CMD23)
			host->autocmd |= MSDC_AUTOCMD23;
#endif
	
		} else if (host->hw->host_function == MSDC_SD) {
			host->autocmd |= MSDC_AUTOCMD12;
		} else {
			host->autocmd &= ~MSDC_AUTOCMD12;
		}
#endif				/* end of MTK_MSDC_USE_CMD23 */

	host->mrq = NULL;

	host->dma.used_gpd = 0;
	host->dma.used_bd = 0;

	/* using dma_alloc_coherent *//* todo: using 1, for all slots */
	host->dma.gpd =
		dma_alloc_coherent(NULL, MAX_GPD_NUM * sizeof(gpd_t), &host->dma.gpd_addr, GFP_KERNEL);
	host->dma.bd =
		dma_alloc_coherent(NULL, MAX_BD_NUM * sizeof(bd_t), &host->dma.bd_addr, GFP_KERNEL);
	BUG_ON((!host->dma.gpd) || (!host->dma.bd));
	msdc_init_gpd_bd(host, &host->dma);

	msdc_clock_src[host->id] = hw->clk_src;
	msdc_host_mode[host->id] = mmc->caps;
	msdc_host_mode2[host->id] = mmc->caps2;
	atc_msdc_host[host->id] = host;
	host->read_time_tune = 0;
	host->write_time_tune = 0;
	host->rwcmd_time_tune = 0;
	host->rwcmd_timeout_emmc = 0;
	host->write_timeout_uhs104 = 0;
	host->write_timeout_emmc = 0;
	host->read_timeout_uhs104 = 0;
	host->read_timeout_emmc = 0;
	host->power_cycle = 0;
	host->power_cycle_enable = 1;
	host->sw_timeout = 0;
	host->tune = 0;
	host->ddr = 0;
	host->sd_cd_insert_work = 0;
	host->block_bad_card = 0;
	host->sd_30_busy = 0;
	host->init_retry_times=0;

	if (IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ)) {
		host->saved_para.suspend_flag = 0;
		host->saved_para.msdc_cfg = 0;
		host->saved_para.mode = 0;
		host->saved_para.div = 0;
		host->saved_para.sdc_cfg = 0;
		host->saved_para.iocon = 0;
		host->saved_para.ddr = 0;
		host->saved_para.hz = 0;
		host->saved_para.cmd_resp_ta_cntr = 0;	/* for SDIO 3.0 */
		host->saved_para.wrdat_crc_ta_cntr = 0;	/* for SDIO 3.0 */
		host->saved_para.int_dat_latch_ck_sel = 0;	/* for SDIO 3.0 */
		host->saved_para.ckgen_msdc_dly_sel = 0;	/* for SDIO 3.0 */
		host->saved_para.inten_sdio_irq = 0;	/* default disable */
	}

	if(host->hw->host_function == MSDC_EMMC) {
		mmc->before_suspend = msdc_save_setting;
		mmc->after_resume = msdc_restore_setting;
	}

	spin_lock_init(&host->lock);
	spin_lock_init(&host->remove_bad_card);
	spin_lock_init(&host->detect_queue_lock);

#if MSDC_USE_CCF
	if (msdc_get_ccf_clk_pointer(pdev, host))
		return 1;
#endif

	msdc_init_hw(host);

	/* Init card detect tasklet */
	tasklet_init(&host->card_tasklet, msdc_tasklet_card, (ulong) host);

	/* Register MSDC Interrupt */
	ret = request_irq((unsigned int)host->irq, msdc_irq, IRQF_TRIGGER_HIGH, DRV_NAME, host);

	if (ret) {
		MSDC_LOG(ERR, " !!!register irq failed");
		goto release;
	}

	MVG_EMMC_SETUP(host);

	if (!(hw->flags & MSDC_REMOVABLE)) {
		host->card_inserted = 1;
	}
	/* else if (hw->host_function == MSDC_SDIO) */
	/* host->card_inserted = 0; */
	else {
		card_detect_gpio_config(host);

		if ((hw->cd_method & MSDC_CD_METHOD_INTERRUPT)||
			(hw->cd_method & MSDC_CD_METHOD_INT_POL)){
			switch (host->id) {
			case 0:
				/* 0 - mean IRQF_TRIGGER_NONE, leave it alone, for Hardware or Firmware config it */
				#if ATC_GPIO_KS
				msdc_gpio_desc = gpio_to_desc(host->cd_gpio);
				cd_irq = gpiod_to_irq(msdc_gpio_desc);
				ret = request_irq(cd_irq, msdc_eirq_cd, 0, "MSDC0_EINT4", host);
				#else
				ret = 0;//Not use now, use for GPIO70(RTS).
				#endif
				if (ret) {
					MSDC_LOG(ERR, "!!!eint4 failed");
				}
				break;

			case 1:
				#if ATC_GPIO_KS
				msdc_gpio_desc = gpio_to_desc(host->cd_gpio);
				cd_irq = gpiod_to_irq(msdc_gpio_desc);
				ret = request_irq(cd_irq, msdc_eirq_cd, 0, "MSDC1_EINT5", host);
				#else
				ret = 0;//SD card uses EINT6 but not EINT5, EINT5 will be use for UART CTS.
				#endif
				if (ret) {
					MSDC_LOG(ERR, "!!!eint5 failed");
				}
				break;

			case 2:
				#if ATC_GPIO_KS
				msdc_gpio_desc = gpio_to_desc(host->cd_gpio);
				cd_irq = gpiod_to_irq(msdc_gpio_desc);
				ret = request_irq(cd_irq, msdc_eirq_cd, 0, "MSDC2_EINT6", host);
				#else
				ret = request_irq((unsigned int)163, msdc_eirq_cd, 0, "eint6", host);
				#endif
				if (ret) {
					MSDC_LOG(ERR, "!!!eint6 failed");
				}
				break;
			}
		}

		/* For card was inserted before CD interrupt is enabled. */
		host->card_inserted = card_detect_gpio(host);
	}

	host->queue_len = 0;
	
#ifdef MSDC_POWER_FAIL_WP	
if(host->id == 0){
    MSDC_LOG_NORMAL(pr_info,"going to request_emmc_wp_eirq ");
	tasklet_init(&host->emmc_protected, msdc_emmc_protect, (ulong)host);
	card_detect_gpio_config(host);
	
    ret = request_irq((unsigned int)161, msdc_eirq_emmc_wp, 0, "emmc_wp", host);	
    if(ret) {
			MSDC_LOG(ERR, " request emmc_wp fail \n");
		}else{							
			host->protect_init=0;           //set the protect write voltage level :0 or 1
			host->protect_gpio =70;          //gpio number
			if(host->protect_init)
			   host->protected = 0;
			else 
			   host->protected = 1;

			MSDC_LOG_NORMAL(pr_info,"request_emmc_wp_eirq is not null,host->protect_init is:%d,host->protect_gpio :%d\n",host->protect_init,host->protect_gpio );
			}
	}
#endif

	if (hw->flags & MSDC_SDIO_EINT) {
		//if (hw->request_sdio_eirq == NULL) {
			MSDC_LOG(ERR, "SDIO EINT Callback Enable");
			hw->request_sdio_eirq = &mtk_wcn_cmb_sdio_request_eirq;
		//}
		
		if (hw->request_sdio_eirq) {	/* set to combo_sdio_request_eirq() for WIFI */
			hw->request_sdio_eirq(msdc_eirq_sdio, (void *)host); /* msdc_eirq_sdio() will be called when EIRQ */
		} else {
			MSDC_LOG(ERR, "request_sdio_eirq is null");
		}
	}
	//else {
	//	MSDC_LOG(ERR, "hw->flags = %d", hw->flags);
	//}

	platform_set_drvdata(pdev, mmc);

	ret = mmc_add_host(mmc);

	if (ret) {
		goto free_irq;
	}

	if (host->hw->flags & MSDC_SDIO_IRQ) {
		MSDC_SET_BITS(SDC_CFG, SDC_CFG_SDIOIDE);        /* enable sdio detection */
	}

	host->sd_cd_insert_work = 1;

	/* If use GPIO Polling for CD, Create thread for it. */
	run_card_detect++;

	if (host->hw->cd_method & MSDC_CD_METHOD_POLLING) {
		host->card_status = 0;
		host->old_status = 0;

		if (run_card_detect == HOST_MAX_NUM) {
			MSDC_LOG_NORMAL(pr_info, "@@@ create msdc card detect polling thread.");
			kernel_thread(msdc_cd_polling_thread_func, host, CLONE_KERNEL);
		}
	}
#ifdef CONFIG_MMC_ATC_SW_WP
	if(host->id == 0) {
		emmc_get_write_protect_region(&wp_rg_info);
	}
#endif

	return 0;

free_irq:
	free_irq(host->irq, host);
release:
	platform_set_drvdata(pdev, NULL);
	msdc_deinit_hw(host);
	tasklet_kill(&host->card_tasklet);
	mmc_free_host(mmc);

	return ret;
}

/* 4 device share one driver, using "drvdata" to show difference */
int msdc_drv_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct msdc_host *host;
	struct resource *mem;

	mmc = platform_get_drvdata(pdev);
	BUG_ON(!mmc);

	host = mmc_priv(mmc);
	BUG_ON(!host);

	MSDC_LOG(ERR, "removed !!!");

	/* Set value for CD deamon thread exit. */
	if (host->hw->cd_method & MSDC_CD_METHOD_POLLING) {
		run_card_detect = 0;
	}

	platform_set_drvdata(pdev, NULL);
	mmc_remove_host(host->mmc);
	msdc_deinit_hw(host);

	tasklet_kill(&host->card_tasklet);

	free_irq(host->irq, host);

	dma_free_coherent(NULL, MAX_GPD_NUM * sizeof(gpd_t), host->dma.gpd, host->dma.gpd_addr);
	dma_free_coherent(NULL, MAX_BD_NUM * sizeof(bd_t), host->dma.bd, host->dma.bd_addr);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (mem) {
		release_mem_region(mem->start, mem->end - mem->start + 1);
	}

	mmc_free_host(host->mmc);

	return 0;
}

/* Fix me: Power Flow */
#ifdef CONFIG_PM
int msdc_drv_suspend(struct device *dev)
{
	int ret = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct msdc_host *host = mmc_priv(mmc);
	u32 base = host->base;
	struct pm_message state;

	MSDC_LOG_NORMAL(pr_info, "[%d]%s+++", host->id, __FUNCTION__);

	state.event = PM_EVENT_SUSPEND;
	if (host->hw->flags & MSDC_REMOVABLE) {
		disable_irq(161 + host->id);        /* Disable eint for Card Detect */
	}

	/* will set for card */
	if (mmc && (host->hw->flags & MSDC_SYS_SUSPEND)) {
		msdc_pm(state, (void *)host);
	}

	if (IS_CARD_SDIO(host) || (host->hw->flags & MSDC_SDIO_IRQ)) {
		if (host->saved_para.suspend_flag == 0) {
			host->saved_para.hz = host->mclk;
			if (host->saved_para.hz) {
				host->saved_para.suspend_flag = 1;
				mb();
				MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, host->saved_para.mode);
				MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, host->saved_para.div);
				/* For SDIO 3.0 */
				MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL,
					       host->saved_para.int_dat_latch_ck_sel);

				MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL,
					       host->saved_para.ckgen_msdc_dly_sel);

				MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP,
					       host->saved_para.cmd_resp_ta_cntr);

				MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS,
					       host->saved_para.wrdat_crc_ta_cntr);

				/* get INTEN status for SDIO */
				MSDC_GET_FIELD(MSDC_INTEN, MSDC_INT_SDIOIRQ, host->saved_para.inten_sdio_irq);
				host->saved_para.msdc_cfg = MSDC_READ32(MSDC_CFG);
				host->saved_para.ddly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
				host->saved_para.pad_tune = MSDC_READ32(MSDC_PAD_TUNE);
				host->saved_para.sdc_cfg = MSDC_READ32(SDC_CFG);
				host->saved_para.iocon = MSDC_READ32(MSDC_IOCON);
				host->saved_para.ddr = host->ddr;
			}
			MSDC_LOG(ERR, "msdc suspend cur_cfg=%x, save_cfg=%x, cur_hz=%d, save_hz=%d",
				 MSDC_READ32(MSDC_CFG), host->saved_para.msdc_cfg, host->mclk,
				 host->saved_para.hz);
		}

	}
	/* When VCC was shut down, all register values will be earsed, It's better to do sw reset*/
	MSDC_MODULE_SW_RESET(host);
	msdc_ccf_clkgate(host,GATE_DISABLE_CLOCK);
	MSDC_LOG_NORMAL(pr_info, "[%d]%s---", host->id, __FUNCTION__);

	return ret;
}

int msdc_drv_resume(struct device *dev)
{
	int ret = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct msdc_host *host = mmc_priv(mmc);
	struct pm_message state;

	MSDC_LOG_NORMAL(pr_info, "[%d]%s+++", host->id, __FUNCTION__);

	/* Reinit MSDC for power down operation will clear all setting */
	msdc_init_hw(host);

	/* Reinit EINT and re-detect card */
	if (host->hw->flags & MSDC_REMOVABLE) {
		card_detect_gpio_config(host);
		host->card_inserted = card_detect_gpio(host);
	}

	state.event = PM_EVENT_RESUME;

	if (mmc && (host->hw->flags & MSDC_SYS_SUSPEND)) { /* will set for card */
		msdc_pm(state, (void *)host);
	}

	if (host->hw->flags & MSDC_REMOVABLE) {
		enable_irq(161 + host->id);        /* Enable eint for Card Detect, Romvable */
	}

	/* This mean WIFI not controller by PM */
	MSDC_LOG_NORMAL(pr_info, "[%d]%s---", host->id, __FUNCTION__);
	return ret;
}

void msdc_drv_shutdown(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct msdc_host *host = mmc_priv(mmc);
	struct resource *mem;

	if (host->id == 1) {
		MSDC_LOG_NORMAL(pr_info, "[%d] sdio no need to shutdown", host->id);
		return;
	}

	// Set value for CD deamon thread exit.
	if (host->hw->cd_method & MSDC_CD_METHOD_POLLING) {
		run_card_detect = 0;
	}
	else if (host->hw->cd_method & MSDC_CD_METHOD_INTERRUPT) {
		switch (host->id)
		{
		case 0:
			free_irq((unsigned int)161, host);
			break;
		case 1:
			free_irq((unsigned int)162, host);
			break;
		case 2:
			free_irq((unsigned int)163, host);
			break;
		}
	}

	platform_set_drvdata(pdev, NULL);
	mmc_remove_host(host->mmc);
	msdc_deinit_hw(host);

	tasklet_kill(&host->card_tasklet);

	free_irq(host->irq, host);

	dma_free_coherent(NULL, MAX_GPD_NUM * sizeof(gpd_t), host->dma.gpd, host->dma.gpd_addr);
	dma_free_coherent(NULL, MAX_BD_NUM	* sizeof(bd_t),  host->dma.bd,	host->dma.bd_addr);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem)
		release_mem_region(mem->start, mem->end - mem->start + 1);

	mmc_free_host(host->mmc);

	MSDC_LOG_NORMAL(pr_info, "[%d]------------------->>>msdc_drv_shutdown<<<-----------------", host->id);
	//msleep(100);
}
EXPORT_SYMBOL(msdc_drv_shutdown);
#endif

#ifdef ATC_LINUX_PLATFORM
struct mmc_part_info *mmc_find_part_by_name(char *name)
{
	struct mmc_part_info *part_info;

	part_info = linux_mmc_find_partinfo_by_name(name);
	return part_info;
}
#else
struct hd_struct *mmc_find_part_by_name(char *name)
{
	struct disk_part_iter piter;
	struct hd_struct *part;
	struct msdc_host *host;
	struct gendisk *disk;

	int len = strlen(name);
	if(len == 0 || len > PARTITION_META_INFO_VOLNAMELTH)
		goto out;

	/* emmc always in slot0 */
	host = msdc_get_host(MSDC_EMMC,MSDC_BOOT_EN,0);
	BUG_ON(!host);
	BUG_ON(!host->mmc);
	BUG_ON(!host->mmc->card);
	disk = mmc_get_disk(host->mmc->card);

	disk_part_iter_init(&piter, disk, 0);
	while ((part = disk_part_iter_next(&piter))) {
		if((part->info) &&
				(0 == strncmp(part->info->volname, name, PARTITION_META_INFO_VOLNAMELTH))) {
			MSDC_LOG_NORMAL(pr_debug, "got partition %s, sect_start:0x%x, sect_cnt:0x%x", name,
					part->start_sect, part->nr_sects);
			break;
		}
	}
	disk_part_iter_exit(&piter);

out:
	return part;
}
#endif
void simp_msdc_init_hw(struct msdc_host *host)
{
	u32 base = host->base;
	struct msdc_hw *hw = host->hw;

	MSDC_MODULE_SW_RESET(host); //reset all registers

#if MSDC_USE_CCF
	msdc_ccf_select_hclk(host,MSDC_HCLK_SEL_SYSPLL_D9);
	msdc_ccf_select_clksrc(host);
	msdc_ccf_clkgate(host,GATE_ENABLE_CLOCK);
#else
/* All host controller use same internal clock(hclock) for internal DMA */
	MSDC_SELECT_HCLK_SRC(MSDC_HCLK_SEL_SYSPLL_D9);
	/* MSDC_SELECT_HCLK_SRC(MSDC_HCLK_SEL_APLL2_D3); */
	MSDC_CLOCK_GATE(host, GATE_ENABLE_CLOCK);
	/* Set clock source, each slot has different clock source */
	msdc_select_clock_source(host);
#endif
	/* Pad init */
	msdc_pad_init(host, 0);
	msdc_pad_multi_func(host, 0);

	MSDC_CLR_BITS(MSDC_CFG, MSDC_CFG_CKPDN);

	/* Configure to MMC/SD mode */
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_MODE, MSDC_SDMMC);

	/* Reset */
	MSDC_RESET_HW(host->id);

	/* Disable and clear all interrupts */
	MSDC_CLR_BITS(MSDC_INTEN, MSDC_READ32(MSDC_INTEN));
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));

	/* Disable internal card detect, because AC8317 does not support this function */
	MSDC_CLR_BITS(MSDC_PS, MSDC_PS_CDEN);
	MSDC_CLR_BITS(MSDC_INTEN, MSDC_INTEN_CDSC);
	MSDC_CLR_BITS(SDC_CFG, SDC_CFG_INSWKUP);

	/* Set RISC to 4bytes access mode */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, MSDC_IOCON_RISCSZ_4BYTE);

	MSDC_WRITE32(MSDC_DAT_RDDLY0, 0x0);
	MSDC_WRITE32(MSDC_DAT_RDDLY1, 0x0);
	MSDC_WRITE32(MSDC_PAD_TUNE, 0x0);

	if (host->id == 0) {
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_GET_BUSY_MARGIN, 1);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_GET_CRC_MARGIN, 1);
	}

	MSDC_SET_BITS(SDC_CFG, SDC_CFG_SDIO);

	/* disable detect SDIO device interupt function */
	MSDC_CLR_BITS(SDC_CFG, SDC_CFG_SDIOIDE);

	/* write crc timeout detection */
	MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 30, 1);

	/* Configure to default data timeout */
	MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, DEFAULT_DTOC);

	/* use pio mode */
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_PIO, 1);

	msdc_set_buswidth(host, MMC_BUS_WIDTH_1);

}

static inline int partition_proc_info(struct seq_file *m, struct hd_struct *this)
{
    char *no_partition_name = "n/a";

    return seq_printf(m, "mmcblk0p%d: %8.8x %8.8x  %s\n", this->partno,
               (unsigned int)this->start_sect,
               (unsigned int)this->nr_sects,
                ((this->info) ? (char *)(this->info->volname) : no_partition_name));
}

static int proc_partition_show(struct seq_file *m, void *v)
{
    struct disk_part_iter piter;
    struct hd_struct *part;
    struct msdc_host *host;
    struct gendisk *disk;

    /* emmc always in slot0 */
    host = msdc_get_host(MSDC_EMMC,MSDC_BOOT_EN,0);
    BUG_ON(!host);
    BUG_ON(!host->mmc);
    BUG_ON(!host->mmc->card);
    disk = mmc_get_disk(host->mmc->card);

    seq_printf(m, "partno:    start_sect   nr_sects  partition_name\n");
    disk_part_iter_init(&piter, disk, 0);
    while ((part = disk_part_iter_next(&piter))){
        partition_proc_info(m, part);
    }
    disk_part_iter_exit(&piter);

    return 0;
}

static int proc_partition_show_linux_platform(struct seq_file *m, void *v)
{
   /* boot_pinfo is defined in mmc/card/block.c */
   extern struct boot_partition_info boot_pinfo[32];
   int i = 0;
   struct boot_partition_info *info = boot_pinfo;

    seq_printf(m, "partno:    start_sect   nr_sects  partition_name\n");
    while(i < 32) {
	if(strlen(info->part_name))
		seq_printf(m, "mmcblk0p%d: %8.8x %8.8x  %s\n", i, info->part_offset,
			info->part_size, info->part_name);
	else
		break;
	i++;
	info++;
    }

    return 0;
}

static int proc_partition_open(struct inode *inode, struct file *file)
{
#ifdef ATC_LINUX_PLATFORM
    return single_open(file, proc_partition_show_linux_platform, NULL);
#else
    return single_open(file, proc_partition_show, NULL);
#endif
}

static const struct file_operations proc_partition_fops = {
    .open = proc_partition_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static const struct of_device_id msdc_of_ids[] = {
	{.compatible = "autochips,MSDC0",},
	{.compatible = "autochips,MSDC1",},
	{.compatible = "autochips,MSDC2",},
	{},
};

#ifdef CONFIG_PM
static struct dev_pm_ops atc_msdc_pm_ops = {
	.suspend = msdc_drv_suspend,
	.resume = msdc_drv_resume,
};
#endif

static struct platform_driver atc_msdc_driver = {
	.probe = msdc_drv_probe,
	.remove = msdc_drv_remove,
#ifdef CONFIG_PM
	.shutdown = msdc_drv_shutdown,
#endif
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = msdc_of_ids,
#ifdef CONFIG_PM
		.pm = &atc_msdc_pm_ops,
#endif
	},
};

/*--------------------------------------------------------------------------*/
/* module init/exit                                                      */
/*--------------------------------------------------------------------------*/
static struct proc_dir_entry *proc_partition;
static int __init atc_msdc_init(void)
{
	int ret;

	proc_partition = proc_create("partitioninfo", 0, NULL, &proc_partition_fops);

	ret = platform_driver_register(&atc_msdc_driver);

	if (ret) {
		MSDC_LOG_NORMAL(pr_err, "%s: Can't register driver\n", DRV_NAME);
		return ret;
	}

	msdc_debug_proc_init();
	msdc_init_dma_latest_address();
	return 0;
}

static void __exit atc_msdc_exit(void)
{
	platform_driver_unregister(&atc_msdc_driver);
	if(proc_partition)
		proc_remove(proc_partition);
}
module_init(atc_msdc_init);
module_exit(atc_msdc_exit);
MODULE_DESCRIPTION("Autochips MMC/SD Card Interface driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Qingqi Xia <qingqi.xia@autochips.com>");
