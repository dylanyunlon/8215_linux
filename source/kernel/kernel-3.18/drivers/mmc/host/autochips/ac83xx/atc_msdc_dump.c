#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>  /* for mdely */
#include <linux/irqflags.h>  /* for mdely */
#include <asm/io.h>        /* __raw_readl */

#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/card.h>

#include "atc_msdc.h"
#include "atc_msdc_dump.h"
#include "atc_msdc_dbg.h"
#include <mach/board.h>
#include <linux/mmc/sd_misc.h>


#ifdef MTK_MSDC_USE_CACHE
unsigned int g_power_reset;
#endif

#define MTK_MMC_DUMP_DBG 0

#define SIMP_SUCCESS          (1)
#define SIMP_FAILED           (0)
#define MAX_POLLING_STATUS (50000)

/* the base address of sd card slot */
#define BOOT_STORAGE_ID        (0)
#define EXTEND_STORAGE_ID     (1)
#define MSDC_CLKSRC      (MSDC_CLKSRC_196MHZ)
static u32 clks[] = {
200000000, 196000000, 189000000, 162000000,
147000000, 135000000, 100000000, 27000000
};

#define BLK_LEN            (512)
#define MAX_SCLK           (52000000)
#define NORMAL_SCLK        (25000000)
#define MIN_SCLK           (260000)
#define MAX_DMA_CNT (64 * 1024 - 512)

#ifndef CONFIG_OF
static void __iomem* u_msdc_base[HOST_MAX_NUM] = {MSDC0_BASE, MSDC1_BASE, MSDC2_BASE};
#endif
static struct msdc_hw *p_msdc_hw[HOST_MAX_NUM] = {NULL, NULL, NULL};

static sector_t lp_start_sect = (sector_t)(-1);
static sector_t lp_nr_sects = (sector_t)(-1);

extern struct msdc_host *atc_msdc_host[];

extern struct msdc_hw msdc0_hw;
extern struct msdc_hw msdc1_hw;
extern struct msdc_hw msdc2_hw;

static struct simp_msdc_host g_msdc_host[2];
static struct simp_msdc_card g_msdc_card[2];
static struct simp_msdc_host *pmsdc_boot_host = &g_msdc_host[BOOT_STORAGE_ID];
static struct simp_msdc_host *pmsdc_extend_host = &g_msdc_host[EXTEND_STORAGE_ID];
static struct simp_mmc_host g_mmc_host[2];
static struct simp_mmc_card g_mmc_card[2];
static struct simp_mmc_host *pmmc_boot_host = &g_mmc_host[BOOT_STORAGE_ID];
static struct simp_mmc_host *pmmc_extend_host = &g_mmc_host[EXTEND_STORAGE_ID];
/* @partition_ready_flag,
 *  = 0: partition init not ready
 *  = 1: partition init is done and succeed
 *  = -1: there is no expdb partition
 */
static int partition_ready_flag = 0;

#define msdc_retry(expr, retry, cnt,id) \
    do { \
        int backup = cnt; \
        while (retry) { \
            if (!(expr)) break; \
            if (cnt-- == 0) { \
                retry--; mdelay(1); cnt = backup; \
            } \
        } \
        if (retry == 0) { \
        } \
        WARN_ON(retry == 0); \
    } while(0)

#define msdc_reset(id) \
    do { \
        int retry = 3, cnt = 1000; \
        MSDC_SET_BITS(MSDC_CFG, MSDC_CFG_RST); \
        msdc_retry(MSDC_READ32(MSDC_CFG) & MSDC_CFG_RST, retry, cnt, id); \
    } while(0)

#define msdc_clr_int() \
	do { \
		volatile u32 val = MSDC_READ32(MSDC_INT); \
		MSDC_SET_BITS(MSDC_INT, val); \
	} while(0)

#define msdc_clr_fifo(id) \
	do { \
		int retry = 3, cnt = 1000; \
		MSDC_SET_BITS(MSDC_FIFOCS, MSDC_FIFOCS_CLR); \
		msdc_retry(MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_CLR, retry, cnt, id); \
	} while(0)

#define msdc_reset_hw(id) \
	do { \
		msdc_reset(id); \
		msdc_clr_fifo(id); \
		msdc_clr_int(); \
	} while(0)


#ifdef ATC_LINUX_PLATFORM
int msdc_get_expdb_offset(void)
{
	unsigned int i;
	struct mmc_part_info *part_info;

	/* find the expdb partition in emmc */
	part_info = mmc_find_part_by_name("expdb");
	if(!part_info) {
		MSDC_LOG_NORMAL(pr_err, "get partition expdb info fail");
		return -1;
	}
	lp_start_sect = part_info->sector_offset;
	lp_nr_sects = part_info->sector_size;
	MSDC_LOG_NORMAL(pr_err,"expdb partition sect offset 0x%08x, size 0x%08x", lp_start_sect, lp_nr_sects);

	return 0;

}
#else
int msdc_get_expdb_offset(void)
{
	unsigned int i;
	struct hd_struct *hd_info;

	/* find the expdb partition in emmc */
	hd_info = mmc_find_part_by_name("expdb");
	if(!hd_info) {
		MSDC_LOG_NORMAL(pr_err, "get partition expdb info fail");
		return -1;
	}
	lp_start_sect = hd_info->start_sect;
	lp_nr_sects = hd_info->nr_sects;
	MSDC_LOG_NORMAL(pr_err,"expdb partition sect offset 0x%08x, size 0x%08x", lp_start_sect, lp_nr_sects);

	return 0;
}
#endif
static void get_emmc_dump_info(struct work_struct *work)
{
	if(partition_ready_flag == 0) {
		if(msdc_get_expdb_offset()) {
			partition_ready_flag = -1;
			pr_err( "Get expdb info fail\n");
		} else
			partition_ready_flag = 1;
	}
}

static void msdc_mdelay(u32 time)
{
   mdelay(time);
   return;
}

/* #define PERI_MSDC_SRCSEL   (0xF100000c) */
/* #define PDN_REG            (0xF1000010) */
static void simp_msdc_config_clksrc(struct simp_msdc_host *host, int clksrc)
{
    host->clksrc = clksrc;
    host->clk  = clks[clksrc];
}

static void simp_msdc_config_clock(struct simp_msdc_host *host, unsigned int hz)  /* no ddr */
{
    // struct msdc_hw *hw = host->priv;
    void __iomem *base = host->base;
    u32 mode;  /* use divisor or not */
    u32 div = 0;
    u32 sclk;
    u32 hclk = host->clk;
    u32 orig_clksrc = host->clksrc;

    //pr_debug("clk=%u\n, clksrc=%u\n", hclk, orig_clksrc);

    if (hz >= hclk) {
        mode = 0x1; /* no divisor */
        sclk = hclk;
    } else {
        mode = 0x0; /* use divisor */
        if (hz >= (hclk >> 1)) {
            div  = 0;         /* mean div = 1/2 */
            sclk = hclk >> 1; /* sclk = clk / 2 */
        } else {
            div  = (hclk + ((hz << 2) - 1)) / (hz << 2);
            sclk = (hclk >> 2) / div;
        }
    }
    host->sclk  = sclk;
    //pr_notice("clock<%d>\n",sclk);

    /* set clock mode and divisor */
    //simp_msdc_config_clksrc(host, MSDC_CLKSRC_NONE);

    /* designer said: best way is wait clk stable while modify clk config bit */
    MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD|MSDC_CFG_CKDIV,(mode << 8)|(div % 0xff));

    simp_msdc_config_clksrc(host, orig_clksrc);

    /* wait clock stable */
    while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));
}

static void msdc_set_timeout(struct simp_msdc_host *host, u32 ns, u32 clks)
{
    void __iomem *base = host->base;
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout >> 20; /* in 2^20 sclk cycle unit */
    timeout = timeout > 1 ? timeout - 1 : 0;
    timeout = timeout > 255 ? 255 : timeout;

    MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, timeout);
}

#if 0
extern S32 pwrap_read_nochk(U32  adr, U32 *rdata);
extern S32 pwrap_write_nochk(U32  adr, U32  wdata);
#define msdc_power_set_field(reg,field,val) \
    do {    \
        volatile unsigned int tv;  \
        pwrap_read_nochk(reg, &tv);    \
        tv &= ~(field); \
        tv |= ((val) << (uffs((unsigned int)field) - 1)); \
        pwrap_write_nochk(reg,tv); \
    } while(0)
#define msdc_power_get_field(reg,field,val) \
    do {    \
        volatile unsigned int tv;  \
        pwrap_read_nochk(reg, &tv);    \
        val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
    } while(0)
#endif

static unsigned int simp_mmc_power_up(struct simp_mmc_host *host,bool on)
{
#if 0
	switch(host->mtk_host->id){
		case 0:
			simp_msdc_ldo_power(on, MT6325_POWER_LDO_VEMC33, VOL_3000);
			break;
		case 1:
			simp_msdc_ldo_power(on, MT6325_POWER_LDO_VMC, VOL_3000);
			simp_msdc_ldo_power(on, MT6325_POWER_LDO_VMCH, VOL_3000);
			break;
		default:
			break;
	}
#endif
	return SIMP_SUCCESS;
}

/* do not change to 1.8v, so cmd11 not used */
static unsigned int simp_mmc_set_signal_voltage(struct simp_mmc_host *host, int volt, bool cmd11)
{
    /* set mmc card voltage */

    return SIMP_SUCCESS;
}

#define clk_readl(addr) \
    DRV_Reg32(addr)

#define clk_setl(addr, val) \
        mt_reg_sync_writel(clk_readl(addr) | (val), addr)

#define clk_clrl(addr, val) \
        mt_reg_sync_writel(clk_readl(addr) & ~(val), addr)

static unsigned int simp_mmc_enable_clk(struct simp_mmc_host *host)
{
        return SIMP_SUCCESS;
}


static unsigned int simp_mmc_hw_reset_for_init(struct simp_mmc_host *host)
{
    void __iomem *base;

    base = host->mtk_host->base;
    if (0 == host->mtk_host->id){
        /* check emmc card support HW Rst_n yes or not is the good way.
         * but if the card not support it , here just failed.
         *     if the card support it, Rst_n function enable under DA driver,
         *     pls see SDMMC_Download_BL_PostProcess_Internal() */
        /* 1ms pluse to trigger emmc enter pre-idle state */
        MSDC_SET_BITS(EMMC_IOCON, EMMC_IOCON_BOOTRST);
        msdc_mdelay(1);
        MSDC_CLR_BITS(EMMC_IOCON, EMMC_IOCON_BOOTRST);

        /* clock is need after Rst_n pull high, and the card need
         * clock to calculate time for tRSCA, tRSTH */
       //MSDC_SET_BITS(MSDC_CFG, MSDC_CFG_CKPDN);
        msdc_mdelay(1);

    }

    return SIMP_SUCCESS;
}

static int msdc_rsp[] = {
    0,  /* RESP_NONE */
    1,  /* RESP_R1 */
    2,  /* RESP_R2 */
    3,  /* RESP_R3 */
    4,  /* RESP_R4 */
    1,  /* RESP_R5 */
    1,  /* RESP_R6 */
    1,  /* RESP_R7 */
    7,  /* RESP_R1b */
};

static unsigned int simp_msdc_init(struct simp_mmc_host *mmc_host)
{
    unsigned int ret = 0;
    void __iomem *base;
    struct simp_msdc_host *host = mmc_host->mtk_host;

    simp_msdc_init_hw(atc_msdc_host[host->id]);
    simp_msdc_config_clock(host, MIN_SCLK);

    return ret;
}


static void simp_mmc_set_bus_mode(struct simp_mmc_host *host, unsigned int mode)
{
    /* mtk: msdc not support to modify bus mode */

}

/* =======================something for msdc cmd/data */
#define CMD_WAIT_RETRY  (0x8FFFFFFF)
#define sdc_is_busy()          (MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY)
#define sdc_is_cmd_busy()      (MSDC_READ32(SDC_STS) & SDC_STS_CMDBUSY)

#define sdc_send_cmd(cmd,arg) \
    do { \
        MSDC_WRITE32(SDC_ARG, (arg)); \
        MSDC_WRITE32(SDC_CMD, (cmd)); \
    } while(0)

int simp_offset = 0;
u8 simp_ext_csd[512];
static int simp_msdc_cmd(struct simp_msdc_host *host, unsigned int cmd, unsigned int raw,
                                         unsigned int arg, int rsptyp, unsigned int *resp)
{
    int retry = 5000; //CMD_WAIT_RETRY;
    void __iomem *base = host->base;
    unsigned int error = 0 ;
    unsigned int intsts = 0;
    unsigned int cmdsts = MSDC_INT_CMDRDY | MSDC_INT_CMDTMO | MSDC_INT_RSPCRCERR;

    /* wait before send command */
    if (cmd == CMD13) {
        while (retry--) {
            if (!sdc_is_cmd_busy())
                break;
            msdc_mdelay(1);
        }
        if (retry == 0) {
            error = 1;
            goto end;
        }
    } else {
        while (retry--) {
            if (!sdc_is_busy())
                break;
            msdc_mdelay(1);
        }
        if (retry == 0) {
            error = 2;
            goto end;
        }
    }

    if ((CMD17 == cmd || CMD18 == cmd ||
        CMD24 == cmd || CMD25 == cmd) && (host->card->type == MMC_TYPE_MMC))
        arg += simp_offset;

    sdc_send_cmd(raw, arg);
#if MTK_MMC_DUMP_DBG
    printk("cmd=0x%x, arg=0x%x\n", raw, arg);
#endif
    /* polling to check the interrupt */
    retry = 5000; //CMD_WAIT_RETRY;
    while( (intsts & cmdsts) == 0) {
        intsts = MSDC_READ32(MSDC_INT);
        retry--;
#if MTK_MMC_DUMP_DBG
        if(retry %1000 == 0){
            printk("int cmd=0x%x, arg=0x%x, retry=0x%x, intsts=0x%x\n", raw, arg, retry, intsts);
        }
#endif
        if (retry == 0) {
            error = 3;
            goto end;
        }
        msdc_mdelay(1);
    }

    intsts &= cmdsts ;
    MSDC_WRITE32(MSDC_INT, intsts); /* clear interrupts */

    if (intsts & MSDC_INT_CMDRDY) {
        /* get the response */
        switch (rsptyp) {
        case RESP_NONE:
            break;
        case RESP_R2:
            *resp++ = MSDC_READ32(SDC_RESP3);
            *resp++ = MSDC_READ32(SDC_RESP2);
            *resp++ = MSDC_READ32(SDC_RESP1);
            *resp   = MSDC_READ32(SDC_RESP0);
            break;
        default: /* Response types 1, 3, 4, 5, 6, 7(1b) */
            *resp = MSDC_READ32(SDC_RESP0);
        }
#if MTK_MMC_DUMP_DBG
        pr_debug("msdc cmd<%d> arg<0x%x> resp<0x%x>Ready \r\n", cmd, arg, *resp);
#endif
    } else {
        error = 4;
        goto end;
    }

    if (rsptyp == RESP_R1B) {
        retry = 9999;
        while ((MSDC_READ32(MSDC_PS) & MSDC_PS_DAT0) != MSDC_PS_DAT0){
            retry--;
            if(retry %5000 == 0){
                pr_debug("int cmd=0x%x, arg=0x%x, retry=0x%x, intsts=0x%x\n",
                    raw, arg, retry, intsts);
            }
            if (retry == 0) {
                error = 5;
                goto end;
            }
            msdc_mdelay(1);
        }
#if MTK_MMC_DUMP_DBG
        pr_debug("msdc cmd<%d> done \r\n", cmd);
#endif
    }

end:
    if(error){
       pr_err( "cmd:%d,arg:0x%x,error=%d,intsts=0x%x\n", cmd, arg, error, intsts);
    }
    return error;
}

/* ======================= */

static int simp_mmc_go_idle(struct simp_mmc_host *host)
{
    int err = 0;
    unsigned int resp = 0;
    struct simp_msdc_host *phost = host->mtk_host;

    err = simp_msdc_cmd(phost, CMD0, CMD0_RAW, CMD0_ARG, RESP_NONE, &resp);

    return err;
}

static unsigned int simp_mmc_get_status(struct simp_mmc_host *host, unsigned int* status)
{
    unsigned int resp  = 0;
    unsigned int err  = 0;
    struct simp_msdc_host *phost = host->mtk_host;
    unsigned int rca = 0;
#ifdef MTK_MSDC_USE_CACHE

    if(g_power_reset)
        rca = phost->card->rca << 16;
    else
        rca = atc_msdc_host[host->index]->mmc->card->rca << 16;
#else
    rca = phost->card->rca << 16;
#endif
    /* pr_debug("rca=0x%x, atc_msdc_host[%d]->mmc->card->rca=0x%x,
     phost->card->rca=0x%x\n", rca, host->index,
      atc_msdc_host[host->index]->mmc->card->rca, phost->card->rca); */
    err = simp_msdc_cmd(phost, CMD13, CMD13_RAW, rca, RESP_R1,  &resp);

    *status = resp;

    return err;
}

static unsigned int simp_mmc_send_stop(struct simp_mmc_host *host)
{
    unsigned int resp  = 0;
    unsigned int err  = 0;
    struct simp_msdc_host *phost = host->mtk_host;

    /* send command */
    err = simp_msdc_cmd(phost, CMD12, CMD12_RAW, 0, RESP_R1B,  &resp);

    return err;
}

static int simp_mmc_send_op_cond(struct simp_mmc_host *host, unsigned int ocr, unsigned int *rocr)
{
    int err = 0, i;
    unsigned int resp = 0;
    struct simp_msdc_host *phost = host->mtk_host;

    for (i = 500; i; i--) {
        err = simp_msdc_cmd(phost, CMD1, CMD1_RAW, ocr, RESP_R3, &resp);
        if (err){
            break;
        }

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (resp & MMC_CARD_BUSY){
            break;
         }

        err = -ETIMEDOUT;

        msdc_mdelay(10);
    }

    if (rocr)
        *rocr = resp;

    if(i <= 400)
        pr_err( "cmd1: resp(0x%x), i=%d\n", resp, i);

    return err;
}

static int simp_mmc_all_send_cid(struct simp_mmc_host *host, unsigned int *cid)
{
    int err = 0;
    unsigned int resp[4] = {0};
    struct simp_msdc_host *phost = host->mtk_host;

    err = simp_msdc_cmd(phost, CMD2, CMD2_RAW, 0, RESP_R2, resp);

#if MTK_MMC_DUMP_DBG
    pr_debug("resp: 0x%x 0x%x 0x%x 0x%x\n", resp[0], resp[1], resp[2], resp[3]);
#endif
    memcpy(cid, resp, sizeof(u32) * 4);

    return 0;
}

static int simp_mmc_set_relative_addr(struct simp_mmc_card *card)
{
    int err;
    unsigned int resp;
    struct simp_msdc_host *phost = card->host->mtk_host;

    err = simp_msdc_cmd(phost, CMD3, CMD3_RAW, card->rca << 16, RESP_R1, &resp);

    return err;
}

static int simp_mmc_send_csd(struct simp_mmc_card *card, unsigned int *csd)
{
    int err;
    unsigned int resp[4] = {0};
    struct simp_msdc_host *phost = card->host->mtk_host;

    err = simp_msdc_cmd(phost, CMD9, CMD9_RAW, card->rca << 16, RESP_R2, resp);

    memcpy(csd, resp, sizeof(u32) * 4);

    return err;
}

static const unsigned int tran_exp[] = {
    10000,        100000,        1000000,    10000000,
    0,        0,        0,        0
};

static const unsigned char tran_mant[] = {
    0,    10,    12,    13,    15,    20,    25,    30,
    35,    40,    45,    50,    55,    60,    70,    80,
};

static const unsigned int tacc_exp[] = {
        1,    10, 100,    1000,    10000,    100000, 1000000, 10000000,
    };

static const unsigned int tacc_mant[] = {
            0,    10, 12, 13, 15, 20, 25, 30,
            35, 40, 45, 50, 55, 60, 70, 80,
    };

#define UNSTUFF_BITS(resp,start,size)                    \
    ({                                \
        const int __size = size;                \
        const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;    \
        const int __off = 3 - ((start) / 32);            \
        const int __shft = (start) & 31;            \
        u32 __res;                        \
        __res = resp[__off] >> __shft;                \
        if (__size + __shft > 32)                \
            __res |= resp[__off-1] << ((32 - __shft) % 32);    \
        __res & __mask;                        \
    })


static int simp_mmc_decode_csd(struct simp_mmc_card *card)
{
    struct mmc_csd *csd = &card->csd;
    unsigned int e, m, a, b;
    u32 *resp = card->raw_csd;

    /*
     * We only understand CSD structure v1.1 and v1.2.
     * v1.2 has extra information in bits 15, 11 and 10.
     * We also support eMMC v4.4 & v4.41.
     */
    csd->structure = UNSTUFF_BITS(resp, 126, 2);
    if (csd->structure == 0) {
        pr_err("unrecognised CSD structure version %d\n", csd->structure);
        return -EINVAL;
    }

    csd->mmca_vsn     = UNSTUFF_BITS(resp, 122, 4);
    m = UNSTUFF_BITS(resp, 115, 4);
    e = UNSTUFF_BITS(resp, 112, 3);
    csd->tacc_ns     = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
    csd->tacc_clks     = UNSTUFF_BITS(resp, 104, 8) * 100;

    m = UNSTUFF_BITS(resp, 99, 4);
    e = UNSTUFF_BITS(resp, 96, 3);
    csd->max_dtr      = tran_exp[e] * tran_mant[m];
    csd->cmdclass      = UNSTUFF_BITS(resp, 84, 12);

    e = UNSTUFF_BITS(resp, 47, 3);
    m = UNSTUFF_BITS(resp, 62, 12);
    csd->capacity      = (1 + m) << (e + 2);

    csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
    csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
    csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
    csd->read_misalign = UNSTUFF_BITS(resp, 77, 1);
    csd->r2w_factor = UNSTUFF_BITS(resp, 26, 3);
    csd->write_blkbits = UNSTUFF_BITS(resp, 22, 4);
    csd->write_partial = UNSTUFF_BITS(resp, 21, 1);

    if (csd->write_blkbits >= 9) {
        a = UNSTUFF_BITS(resp, 42, 5);
        b = UNSTUFF_BITS(resp, 37, 5);
        csd->erase_size = (a + 1) * (b + 1);
        csd->erase_size <<= csd->write_blkbits - 9;
    }

    return 0;
}

static int simp_mmc_select_card(struct simp_mmc_host *host, struct simp_mmc_card *card)
{
    int err;
    unsigned int resp;
    struct simp_msdc_host *phost = host->mtk_host;

    err = simp_msdc_cmd(phost, CMD7, CMD7_RAW, card->rca << 16, RESP_R1, &resp);

    return 0;
}

/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
static unsigned int simp_mmc_select_voltage(struct simp_mmc_host *host, unsigned int ocr)
{
#if 0
    int bit;

    ocr &= host->ocr_avail;

    bit = ffs(ocr);
    if (bit) {
        bit -= 1;
        ocr &= 3 << bit;
        mmc_host_clk_hold(host);
        host->ios.vdd = bit;
        mmc_set_ios(host);
        mmc_host_clk_release(host);
    } else {
        pr_warning("%s: host doesn't support card's voltages\n",
                mmc_hostname(host));
        ocr = 0;
    }
#endif

    return ocr;
}

#define EXT_CSD_SEC_CNT                    212
#ifdef MTK_MSDC_USE_CACHE
#define EXT_CSD_FLUSH_CACHE             32      /* W */
//#define EXT_CSD_CACHE_CTRL              33      /* R/W */
#endif

#define CAPACITY_2G                        (2 * 1024 * 1024 * 1024ULL)

static void simp_emmc_cal_offset(struct simp_mmc_card *card)
{
    simp_offset = 0;
}
static int simp_msdc_pio_read(struct simp_msdc_host *host, unsigned int *ptr, unsigned int size);
static void simp_msdc_set_blknum(struct simp_msdc_host *host, unsigned int blknum);

static int simp_mmc_read_ext_csd(struct simp_mmc_host *host, struct simp_mmc_card *card)
{
    int err = 0;
    unsigned int resp;
    struct simp_msdc_host *phost = host->mtk_host;
    void __iomem *base = phost->base;
    memset(simp_ext_csd, 0, 512);
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) {
        pr_err( "MSDC MMCA_VSN: %d. Skip EXT_CSD\n", card->csd.mmca_vsn);
        return 0;
    }
    msdc_clr_fifo(host->mtk_host->id);
    simp_msdc_set_blknum(phost, 1);
    msdc_set_timeout(phost, 100000000, 0);

    err = simp_msdc_cmd(phost, CMD8, CMD8_RAW_EMMC, 0, RESP_R1, &resp);
    if (err){
        goto out;
    }

    err = simp_msdc_pio_read(phost, (unsigned int *)(simp_ext_csd), 512);
    if (err){
        pr_err( "pio read ext csd error(0x%d)\n", err);
        goto out;
    }
out:
    return err;
}

static void simp_mmc_decode_ext_csd(struct simp_mmc_card *card)
{
    card->ext_csd.sectors =
           simp_ext_csd[EXT_CSD_SEC_CNT + 0] << 0 |
        simp_ext_csd[EXT_CSD_SEC_CNT + 1] << 8 |
        simp_ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
        simp_ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
    return;
}

static int simp_emmc_switch_bus(struct simp_mmc_host *host,struct simp_mmc_card *card)
{
    struct simp_msdc_host *phost = host->mtk_host;
    unsigned int resp;
    return simp_msdc_cmd(phost, ACMD6, ACMD6_RAW_EMMC, ACMD6_ARG_EMMC, RESP_R1B, &resp);
}
static int simp_mmc_init_card(struct simp_mmc_host *host, unsigned int ocr,
    struct simp_mmc_card *oldcard)
{
    int err = 0;
    unsigned int rocr;
    unsigned int cid[4];
    void __iomem *base;
    struct simp_mmc_card* card = host->card;
    base = host->mtk_host->base;

    /* Set correct bus mode for MMC before attempting init */
    simp_mmc_set_bus_mode(host, MMC_BUSMODE_OPENDRAIN);  // NULL func now

    /* Initialization should be done at 3.3 V I/O voltage. */
    simp_mmc_set_signal_voltage(host, MMC_SIGNAL_VOLTAGE_330, 0); // NULL func now

    /*
     * Since we're changing the OCR value, we seem to
     * need to tell some cards to go back to the idle
     * state.  We wait 1ms to give cards time to
     * respond.
     * mmc_go_idle is needed for eMMC that are asleep
     */
    simp_mmc_go_idle(host);

    /* The extra bit indicates that we support high capacity */
    err = simp_mmc_send_op_cond(host, ocr | (1 << 30), &rocr);
    if (err)
        goto err;

    err = simp_mmc_all_send_cid(host, cid);
    if (err)
        goto err;

    card->type = MMC_TYPE_MMC;
    card->rca = 1;
    host->mtk_host->card->rca = 1;
    memcpy(card->raw_cid, cid, sizeof(card->raw_cid));

    /*
     * For native busses:  set card RCA and quit open drain mode.
     */
    err = simp_mmc_set_relative_addr(card);
    if (err)
        goto err;

    simp_mmc_set_bus_mode(host, MMC_BUSMODE_PUSHPULL);

    /*
     * Fetch CSD from card.
     */
    err = simp_mmc_send_csd(card, card->raw_csd);
    if (err)
        goto err;

    err = simp_mmc_decode_csd(card);
    if (err)
        goto err;

#if 0
    err = mmc_decode_csd(card);
    if (err)
        goto err;
    err = mmc_decode_cid(card);
    if (err)
        goto err;
#endif

    err = simp_mmc_select_card(host, card);
    if (err)
        goto err;
    err = simp_mmc_read_ext_csd(host,card);
    if (err)
            goto err;

    simp_mmc_decode_ext_csd(card);
    simp_emmc_cal_offset(card);
    if(simp_offset < 0)
        goto err;
    err = simp_emmc_switch_bus(host,card);
    MSDC_SET_FIELD(SDC_CFG, SDC_CFG_BUSWIDTH, 1);  /* 1: 4 bits mode */
    simp_msdc_config_clock(host->mtk_host, NORMAL_SCLK);
    return SIMP_SUCCESS;

err:
    return SIMP_FAILED;
}

#define ACMD41_RETRY   (20)
static int simp_mmc_sd_init(struct simp_mmc_host *host)
{
    struct simp_msdc_host *phost = host->mtk_host;
    u32 ACMD41_ARG = 0;
    u8  retry;
    void __iomem *base;
    unsigned int resp = 0;
    int bRet = 0;

    base = phost->base;
    if (simp_msdc_cmd(phost, CMD0, CMD0_RAW, CMD0_ARG, RESP_NONE, &resp)) goto EXIT;

    if (simp_msdc_cmd(phost, CMD8, CMD8_RAW, CMD8_ARG, RESP_R7,   &resp)){
        // SD v1.0 will not repsonse to CMD8, then clr HCS bit
        //printk("SD v1.0, clr HCS bit\n");
        ACMD41_ARG = ACMD41_ARG_10;
    } else if (resp == CMD8_ARG) {
        //printk("SD v2.0, set HCS bit\n");
        ACMD41_ARG = ACMD41_ARG_20;
    }


    retry = ACMD41_RETRY;
    while (retry--) {
        if (simp_msdc_cmd(phost,  CMD55,  CMD55_RAW,  CMD55_ARG << 16,  RESP_R1, &resp)) goto EXIT;
        if (simp_msdc_cmd(phost, ACMD41, ACMD41_RAW, ACMD41_ARG,  RESP_R3, &resp)) goto EXIT;
        if (resp & R3_OCR_POWER_UP_BIT) {
            phost->card->card_cap = ((resp & R3_OCR_CARD_CAPACITY_BIT) ? high_capacity : standard_capacity);
            if(phost->card->card_cap == standard_capacity){
                //printk("just standard_capacity card!!\r\n");
            }
            break;
        }
        msdc_mdelay(1000 / ACMD41_RETRY);
    }

    if (simp_msdc_cmd(phost,  CMD2,    CMD2_RAW,   CMD2_ARG, RESP_R2, &resp)) goto EXIT;

    if (simp_msdc_cmd(phost,  CMD3,    CMD3_RAW,   CMD3_ARG, RESP_R6, &resp)) goto EXIT;

    /* save the rca */
    phost->card->rca = (resp & 0xffff0000) >> 16;     /* RCA[31:16]*/

    if (simp_msdc_cmd(phost,  CMD9,    CMD9_RAW,   CMD9_ARG << 16, RESP_R2, &resp)) goto EXIT;

    if (simp_msdc_cmd(phost,  CMD13,  CMD13_RAW,  CMD13_ARG << 16, RESP_R1, &resp)) goto EXIT;

    if (simp_msdc_cmd(phost,  CMD7,    CMD7_RAW,   CMD7_ARG << 16, RESP_R1, &resp)) goto EXIT;

    msdc_mdelay(10);

    if (simp_msdc_cmd(phost,  CMD55,  CMD55_RAW,  CMD55_ARG << 16, RESP_R1, &resp)) goto EXIT;

    if (simp_msdc_cmd(phost, ACMD42, ACMD42_RAW, ACMD42_ARG, RESP_R1, &resp)) goto EXIT;

    if (simp_msdc_cmd(phost,  CMD55,  CMD55_RAW,  CMD55_ARG << 16, RESP_R1, &resp)) goto EXIT;

    if (simp_msdc_cmd(phost, ACMD6,   ACMD6_RAW,  ACMD6_ARG, RESP_R1, &resp)) goto EXIT;

    /* set host bus width to 4 */
    MSDC_SET_FIELD(SDC_CFG, SDC_CFG_BUSWIDTH, 1);  /* 1: 4 bits mode */
    simp_msdc_config_clock(phost, NORMAL_SCLK);

#if MTK_MMC_DUMP_DBG
    pr_err( "sd card inited\n");
#endif
    bRet = 1;

EXIT:
    return bRet;
}

#ifdef MTK_MSDC_USE_CACHE
static int mmc_disable_cache(struct simp_mmc_host *host)
{
    int err = 0;
    unsigned int resp;
    unsigned int status = 0;
    int polling = msdc_clr_fifoTATUS;
    struct simp_msdc_host *phost = host->mtk_host;

    do{
        err = simp_mmc_get_status(host, &status);
        if(err) {
            return -1;
        }

        if(R1_CURRENT_STATE(status) == 5 || R1_CURRENT_STATE(status) == 6){
            simp_mmc_send_stop(host);
        }
        //msdc_mdelay(1);
    }while(R1_CURRENT_STATE(status) == 7 && polling--);

    if(R1_CURRENT_STATE(status) == 7)
        return -2;

    err = simp_msdc_cmd(phost, ACMD6, ACMD6_RAW_EMMC, ACMD6_ARG_DISABLE_CACHE, RESP_R1B, &resp);

    if(!err){
        polling = MAX_POLLING_STATUS;
        do{
            err = simp_mmc_get_status(host, &status);
            if(err)
                return -3;
            //msdc_mdelay(1);
        }while(R1_CURRENT_STATE(status) == 7 && polling--);

        if (status & 0xFDFFA000)
            printk("msdc unexpected status 0x%x after switch", status);
        if (status & R1_SWITCH_ERROR)
            return -4;
    }

    return err;
}
#endif

static unsigned int simple_mmc_attach_sd(struct simp_mmc_host *host)
{
    //int err = SIMP_FAILED;

    /* power up host */
    simp_mmc_power_up(host,0);
    msdc_mdelay(20);
    simp_mmc_power_up(host,1);

    /* enable clock */
    simp_mmc_enable_clk(host);

    /*
     * Some eMMCs (with VCCQ always on) may not be reset after power up, so
     * do a hardware reset if possible.
     */
    simp_mmc_hw_reset_for_init(host);

    /* power up card: Initialization should be done at 3.3 V I/O voltage. */
    simp_mmc_set_signal_voltage(host, MMC_SIGNAL_VOLTAGE_330, 0);

    /* init msdc host */
    simp_msdc_init(host);

    simp_mmc_sd_init(host);

    return SIMP_SUCCESS;
}

/* make clk & power always on */
static unsigned int simple_mmc_attach_mmc(struct simp_mmc_host *host)
{
    int err = 0;
    unsigned int ocr;

#ifdef MTK_MSDC_USE_CACHE
    g_power_reset = 0;
    /* turn off cache will trigger flushing of the cache data to non-volatile storage */
    if(!atc_msdc_host[host->index] ||
         !atc_msdc_host[host->index]->mmc ||
         !atc_msdc_host[host->index]->mmc->card){
        printk("[%s]: host/mmc/card is not existed\n", __func__);
    } else if(atc_msdc_host[host->index]->mmc->card->ext_csd.cache_ctrl & 0x1){
        /* enable clock */
        simp_mmc_enable_clk(host);

        /* init msdc host */
        simp_msdc_init(host);

        err = mmc_disable_cache(host);
        if(err){
            printk("[%s]: failed to disable cache ops, err = %d\n", __func__, err); ;
            err = 0;
        } else {
#if MTK_MMC_DUMP_DBG
            printk("[%s]: successfully disabled cache ops.\n", __func__ );
#endif
        }
    } else {
#if MTK_MMC_DUMP_DBG
        printk("[%s]: cache is not enabled, no need to disable it\n", __func__);
#endif
    }
#endif

    /* power up host */
    simp_mmc_power_up(host,1);
    msdc_mdelay(10);
#ifdef MTK_MSDC_USE_CACHE
    g_power_reset = 1;
#endif
    /*
     * Some eMMCs (with VCCQ always on) may not be reset after power up, so
     * do a hardware reset if possible.
     */
    simp_mmc_hw_reset_for_init(host);

    /* power up card: Initialization should be done at 3.3 V I/O voltage. */
    simp_mmc_set_signal_voltage(host, MMC_SIGNAL_VOLTAGE_330, 0);

    /* enable clock */
    simp_mmc_enable_clk(host);

    /* init msdc host */
    simp_msdc_init(host);

    /*=================== begin to init emmc card =======================*/

    /* Set correct bus mode for MMC before attempting attach */
    simp_mmc_set_bus_mode(host, MMC_BUSMODE_OPENDRAIN);

    simp_mmc_go_idle(host);

    err = simp_mmc_send_op_cond(host, 0, &ocr);


    /*
     * Sanity check the voltages that the card claims to
     * support.
     */
    if (ocr & 0x7F) {
#if MTK_MMC_DUMP_DBG
        pr_err( "msdc0: card claims to support voltages "
               "below the defined range. These will be ignored.\n");
#endif
        ocr &= ~0x7F;
    }

    host->ocr = simp_mmc_select_voltage(host, ocr);

    /*
     * Can we support the voltage of the card?
     */
    if (!host->ocr) {
        pr_err("msdc0: card voltage not support\n");
        err = -EINVAL;
        goto err;
    }

    /*
     * Detect and init the card.
     */
    err = simp_mmc_init_card(host, host->ocr, NULL);
    if (err == SIMP_FAILED){
        pr_err("init eMMC failed\n");
        goto err;
    }
#if MTK_MMC_DUMP_DBG
    pr_debug("init eMMC success\n");
#endif
    pr_err("init eMMC success\n");

    /*=================== end mmc card init =============================*/
    return SIMP_SUCCESS;
err:
    return SIMP_FAILED;
}
static const unsigned g_freqs[] = {300000, 260000, 200000, 100000};
#define HOST_MIN_MCLK (260000)

static int emmc_init = 0;
static int sd_init = 0;
/* not use freq para */
static unsigned int simp_mmc_rescan_try_freq(struct simp_mmc_host *host, unsigned freq)
{
    int err = SIMP_FAILED;

    /* sd/emmc will support */
    if (host->mtk_host->card->type == MMC_TYPE_MMC){
#if MTK_MMC_DUMP_DBG
        pr_debug("init emmc for ipanic dump\n");
#endif
        err = simple_mmc_attach_mmc(host);
    } else if(host->mtk_host->card->type == MMC_TYPE_SD){
#if MTK_MMC_DUMP_DBG
        pr_debug("init sd card\n");
#endif
        err = simple_mmc_attach_sd(host);
    }

    return err;
}
static unsigned int simp_init_emmc(void){
	int i = 0;
	int ret = 0;

	for (i = 0; i < ARRAY_SIZE(g_freqs); i++) {
		if (SIMP_SUCCESS == simp_mmc_rescan_try_freq(pmmc_boot_host, (unsigned)max(g_freqs[i], (unsigned)HOST_MIN_MCLK))) {
			break;
		}
		if (g_freqs[i] <= HOST_MIN_MCLK){
			pr_err("failed to init eMMC, line:%d\n", __LINE__);
			ret = 1;
		}
	}
	if(0 == ret)
		emmc_init = 1;
	return ret;
}

static unsigned int simp_init_sd(void){
    int i = 0;
    int ret = 0;

    for (i = 0; i < ARRAY_SIZE(g_freqs); i++) {
        if (SIMP_SUCCESS == simp_mmc_rescan_try_freq(pmmc_extend_host, (unsigned)max(g_freqs[i], (unsigned)HOST_MIN_MCLK))) {
            break;
        }
        if (g_freqs[i] <= HOST_MIN_MCLK){
            pr_err("failed to init eMMC, line:%d\n", __LINE__);
            ret = 1;
        }
    }
    if(0 == ret)
        sd_init = 1;
    return ret;
}
unsigned int reset_boot_up_device(int type){
    int ret = 0;

    if(type == MMC_TYPE_MMC)
        ret = simp_init_emmc();
    else if(type == MMC_TYPE_SD)
        ret = simp_init_sd();
    else{
        pr_err("invalide card type: %d\n", type);
        ret = 1;
    }

    return ret;
}
EXPORT_SYMBOL(reset_boot_up_device);

#define MSDC_FIFO_SZ            (128)
#define MSDC_FIFO_THD           (64)    /* (128) */
#define msdc_txfifocnt()   ((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_TXCNT) >> 16)
#define msdc_rxfifocnt()   ((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_RXCNT) >> 0)
#define msdc_fifo_write32(v)   MSDC_WRITE32(MSDC_TXDATA, (v))
#define msdc_fifo_write8(v)    MSDC_WRITE8(MSDC_TXDATA, (v))
#define msdc_fifo_read32()     MSDC_READ32(MSDC_RXDATA)
#define msdc_fifo_read8()      MSDC_READ8(MSDC_RXDATA)
static int simp_msdc_pio_write(struct simp_msdc_host *host, unsigned int *ptr, unsigned int size)
{
    void __iomem *base = host->base;
    unsigned int  left = size;
    unsigned int  status = 0;
    unsigned char *u8ptr;
    int l_count = 0;
    int err = 0;
    int print_count = 2;

    while(1){
            status = MSDC_READ32(MSDC_INT);
            MSDC_WRITE32(MSDC_INT,status);
        if (status & MSDC_INT_DATCRCERR) {
            pr_err("[MSDC%d] DAT CRC error (0x%x), Left DAT: %d bytes\n",
                host->id, status, left);
            err = -5;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            pr_err("[MSDC%d] DAT TMO error (0x%x), Left DAT: %d bytes\n",
                host->id, status, left);
            err = -110;
            break;
        } else if(status & MSDC_INT_XFER_COMPL){
            break;
        }
        if(left == 0)
            continue;
        if ((left >= MSDC_FIFO_SZ) && (msdc_txfifocnt() == 0)) {
            int count = MSDC_FIFO_SZ >> 2;
            do {
                msdc_fifo_write32(*ptr);
                ptr++;
            } while (--count);
            left -= MSDC_FIFO_SZ;
        } else if (left < MSDC_FIFO_SZ && msdc_txfifocnt() == 0) {
            while (left > 3) {
                msdc_fifo_write32(*ptr); ptr++;
                left -= 4;
            }

            u8ptr = (u8*)ptr;
            while(left){
                msdc_fifo_write8(*u8ptr);
                u8ptr++;
                left--;
            }
        } else {
                status = MSDC_READ32(MSDC_INT);

                if ((status & MSDC_INT_DATCRCERR) || (status & MSDC_INT_DATTMO)) {

                    if (status & MSDC_INT_DATCRCERR){
                        pr_err("[MSDC%d] DAT CRC error (0x%x), Left DAT: %d bytes\n",
                                host->id, status, left);
                        err = -5;
                    }
                    if (status & MSDC_INT_DATTMO){
                        pr_err("[MSDC%d] DAT TMO error (0x%x), Left DAT: %d bytes\n",
                                host->id, status, left);
                        err = -110;
                    }

                    MSDC_WRITE32(MSDC_INT, status);
                    msdc_reset_hw(host->id);
                    return err;
                }
        }

        l_count++;
        if (l_count > 500) {
            l_count=0;
            if(print_count > 0){
#if MTK_MMC_DUMP_DBG
                pr_debug("size= %d, left= %d.\r\n", size, left);
#endif
                print_count--;
            }
        }
    }

    return err;
}

static int simp_msdc_pio_read(struct simp_msdc_host *host, unsigned int *ptr, unsigned int size)
{
    void __iomem *base = host->base;
    unsigned int  left = size;
    unsigned int  status = 0;
    unsigned char *u8ptr;
    int l_count = 0;
    int err = 0;
    int print_count = 2;
    int done = 0;

    while(1){
            status = MSDC_READ32(MSDC_INT);
            MSDC_WRITE32(MSDC_INT,status);
        if (status & MSDC_INT_DATCRCERR) {
            pr_err("[MSDC%d] DAT CRC error (0x%x), Left DAT: %d bytes\n",
                host->id, status, left);
            err = -5;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            pr_err("[MSDC%d] DAT TMO error (0x%x), Left DAT: %d bytes\n",
                host->id, status, left);
            err = -110;
            break;
        } else if(status & MSDC_INT_XFER_COMPL){
            done = 1;
        }
        if (done && (left == 0))
            break;

        while (left) {
            /* pr_err("left(%d)/FIFO(%d)\n", left,msdc_rxfifocnt()); */
            if ((left >=  MSDC_FIFO_THD) && (msdc_rxfifocnt() >= MSDC_FIFO_THD)) {
                int count = MSDC_FIFO_THD >> 2;
                do {
                    *ptr++ = msdc_fifo_read32();
                } while (--count);
                left -= MSDC_FIFO_THD;
            } else if ((left < MSDC_FIFO_THD) && msdc_rxfifocnt() >= left) {
                while (left > 3) {
                    *ptr++ = msdc_fifo_read32();
                    left -= 4;
                }

                u8ptr = (u8 *)ptr;
                while(left) {
                    *u8ptr++ = msdc_fifo_read8();
                    left--;
                }
            } else {
                status = MSDC_READ32(MSDC_INT);

                if ((status & MSDC_INT_DATCRCERR) || (status & MSDC_INT_DATTMO)) {

                    if (status & MSDC_INT_DATCRCERR){
                        pr_err("[MSDC%d] DAT CRC error (0x%x), Left DAT: %d bytes\n",
                                host->id, status, left);
                        err = -5;
                    }
                    if (status & MSDC_INT_DATTMO){
                        pr_err("[MSDC%d] DAT TMO error (0x%x), Left DAT: %d bytes\n",
                                host->id, status, left);
                        err = -110;
                    }

                    MSDC_WRITE32(MSDC_INT, status);
                    msdc_reset_hw(host->id);
                    return err;
                }
            }

            /* timeout monitor*/
            l_count++;
            if (l_count > 50000) {
                l_count = 0;
                if(print_count > 0){
                    pr_err("size= %d, left= %d, done=%d. \r\n", size, left, done);
                    print_count--;
                }
            }
        }
    }

    return err;
}

static void simp_msdc_set_blknum(struct simp_msdc_host *host, unsigned int blknum)
{
    void __iomem *base = host->base;
    MSDC_WRITE32(SDC_BLK_NUM, blknum);
}

static  int simp_mmc_single_write(struct simp_mmc_host *host, unsigned int addr, void* buf, unsigned int size)
{
    unsigned int resp  = 0;
    unsigned int err = 0;
    /* unsigned int intsts = 0; */
    struct simp_msdc_host *phost = host->mtk_host;
    void __iomem *base = phost->base;

    if (size != 512){
        pr_err("emmc: write para error!\n");
        return -1;
    }

    simp_msdc_set_blknum(phost, 1);

    /* send command */
    err = simp_msdc_cmd(phost, CMD24, CMD24_RAW, addr, RESP_R1,  &resp);

    /* write the data to FIFO */
    err = simp_msdc_pio_write(phost, (unsigned int *)buf, 512);
    if (err){
        pr_err("write data: error(%d)\n", err);
    }

    /* make sure contents in fifo flushed to device */
    BUG_ON(msdc_txfifocnt());

    /* check and clear interrupt */
    /* while ((intsts & MSDC_INT_XFER_COMPL) == 0 ) { */
    /* intsts = MSDC_READ32(MSDC_INT); */
    /* } */
    /* sdr_set_bits(MSDC_INT, MSDC_INT_XFER_COMPL); */

    return err;
}

static  int simp_mmc_single_read(struct simp_mmc_host *host, unsigned int addr, void* buf, unsigned int size)
{
    unsigned int resp  = 0;
    unsigned int err  = 0;
    struct simp_msdc_host *phost = host->mtk_host;

    if (size != 512){
        pr_err( "emmc: read para error!\n");
        return -1;
    }

    simp_msdc_set_blknum(phost, 1);

    /* send command */
    err = simp_msdc_cmd(phost, CMD17, CMD17_RAW, addr, RESP_R1,  &resp);

    /* read the data out*/
    err = simp_msdc_pio_read(phost, (unsigned int *)buf, 512);
    if (err){
            pr_err( "read data: error(%d)\n", err);
    }

    return err;
}

static  int simp_mmc_multi_write(struct simp_mmc_host *host, unsigned int addr, void* buf, unsigned int nblk)
{
    unsigned int resp  = 0;
    unsigned int err = 0;
    //unsigned int intsts = 0;
    struct simp_msdc_host *phost = host->mtk_host;
    void __iomem *base = phost->base;

    simp_msdc_set_blknum(phost, nblk);

    /* send command */
    err = simp_msdc_cmd(phost, CMD25, CMD25_RAW, addr, RESP_R1,  &resp);

    /* write the data to FIFO */
    err = simp_msdc_pio_write(phost, (unsigned int *)buf, 512*nblk);
    if (err){
        pr_err( "write data: error(%d)\n", err);
    }

    /* make sure contents in fifo flushed to device */
   BUG_ON(msdc_txfifocnt());

    /* check and clear interrupt */

    simp_mmc_send_stop(host);

    return err;
}
static  int simp_mmc_multi_read(struct simp_mmc_host *host, unsigned int addr, void* buf, unsigned int nblk)
{
    unsigned int resp  = 0;
    unsigned int err  = 0;
    //unsigned int intsts = 0;
    struct simp_msdc_host *phost = host->mtk_host;
    //void __iomem *base = phost->base;


    simp_msdc_set_blknum(phost, nblk);

    /* send command */
    err = simp_msdc_cmd(phost, CMD18, CMD18_RAW, addr, RESP_R1,  &resp);

    /* read the data out*/
    err = simp_msdc_pio_read(phost, (unsigned int *)buf, 512*nblk);
    if (err){
        pr_err( "read data: error(%d)\n", err);
    }

    simp_mmc_send_stop(host);
    return err;
}


/* card_type tell to use which host, will support PANIC dump info to emmc card
 * and KDUMP info to sd card */
int msdc_init_panic(int dev)
{
    return 1;
}
static int simp_mmc_get_host(int card_type,bool boot)
{
    int index = 0;
    for(;index < HOST_MAX_NUM;++index){
        if(p_msdc_hw[index]){
            if((card_type == p_msdc_hw[index]->host_function) && (boot == p_msdc_hw[index]->boot))
                return index;
        }
    }
    return HOST_MAX_NUM;

}
static int simp_mmc_init(int card_type,bool boot)
{
    struct simp_mmc_host *host;

    if(boot){
            /* init some struct */
            pmmc_boot_host->mtk_host = pmsdc_boot_host;
            pmmc_boot_host->card = &g_mmc_card[BOOT_STORAGE_ID];
            pmmc_boot_host->card->host = pmmc_boot_host;

            host = pmmc_boot_host;

            memset(pmmc_boot_host->mtk_host, 0, sizeof(struct simp_msdc_host));
            pmmc_boot_host->mtk_host->id       = simp_mmc_get_host(card_type,boot);
            if(pmmc_boot_host->mtk_host->id >= HOST_MAX_NUM)
                return -1;
        #ifdef CONFIG_OF
            pmmc_boot_host->mtk_host->base = (atc_msdc_host[pmmc_boot_host->mtk_host->id])->base;
            pr_notice("msdc @ 0x%p, id:%d\n", pmmc_boot_host->mtk_host->base, pmmc_boot_host->mtk_host->id);
        #else
            pmmc_boot_host->mtk_host->base     = u_msdc_base[pmmc_boot_host->mtk_host->id];
        #endif
            pmmc_boot_host->mtk_host->clksrc   = MSDC_CLKSRC;
            pmmc_boot_host->mtk_host->clk      = clks[MSDC_CLKSRC];
            pmmc_boot_host->mtk_host->card     = &g_msdc_card[BOOT_STORAGE_ID];

            /* not use now, may be delete */
            memset(&g_msdc_card[BOOT_STORAGE_ID], 0, sizeof(struct simp_msdc_card));
            g_msdc_card[BOOT_STORAGE_ID].type        = MMC_TYPE_MMC;
            g_msdc_card[BOOT_STORAGE_ID].file_system = _RAW_;

            /* init host & card */

    }
    else   {
            pmmc_extend_host->mtk_host = pmsdc_extend_host;
            pmmc_extend_host->card = &g_mmc_card[EXTEND_STORAGE_ID];
            pmmc_extend_host->card->host = pmmc_extend_host;

            host = pmmc_extend_host;

            memset(pmmc_extend_host->mtk_host, 0, sizeof(struct simp_msdc_host));
            pmmc_extend_host->mtk_host->id       = 2;//simp_mmc_get_host(card_type,boot);
            if(pmmc_extend_host->mtk_host->id >= HOST_MAX_NUM)
                return -1;
        #ifdef CONFIG_OF
            pmmc_extend_host->mtk_host->base = (atc_msdc_host[pmmc_extend_host->mtk_host->id])->base;
            pr_notice("msdc @ 0x%p, id:%d\n", pmmc_extend_host->mtk_host->base, pmmc_extend_host->mtk_host->id);
        #else
            pmmc_extend_host->mtk_host->base     = u_msdc_base[pmmc_extend_host->mtk_host->id];
        #endif
            pmmc_extend_host->mtk_host->clksrc   = MSDC_CLKSRC;
            pmmc_extend_host->mtk_host->clk      = clks[MSDC_CLKSRC];
            pmmc_extend_host->mtk_host->card     = &g_msdc_card[EXTEND_STORAGE_ID];

            /* not use now, may be delete */
            memset(&g_msdc_card[EXTEND_STORAGE_ID], 0, sizeof(struct simp_msdc_card));
            g_msdc_card[EXTEND_STORAGE_ID].type        = MMC_TYPE_SD;
            g_msdc_card[EXTEND_STORAGE_ID].file_system = FAT32;

            //pr_notice("g_msdc_card[SD_MSDC_ID] addr is 0x%x\n", &g_msdc_card[EXTEND_STORAGE_ID]);
            //pr_notice("g_msdc_card +1 addr is 0x%x\n", g_msdc_card + 1);
            //pr_notice("pmsdc_sd_host->card addr is 0x%x\n", pmsdc_extend_host->card);
            pmsdc_extend_host->card->card_cap = 1;
            }
    return 0;
}


/*--------------------------------------------------------------------------*/
/* porting for panic dump interface                                         */
/*--------------------------------------------------------------------------*/
static void dump_buff(char *buff)
{
	int i;
	for(i=0; i<512; i+=8)
	{
		pr_err("[%d~%d] %x %x %x %x %x %x %x %x\n", i, i+8,
				buff[i], buff[i+1],buff[i+2],buff[i+3],buff[i+4], buff[i+5],buff[i+6],buff[i+7]);
	}
}

static int simp_emmc_dump_write(unsigned char* buf, unsigned int len, unsigned int offset,unsigned int dev)
{
    /* maybe delete in furture */
    unsigned int i;
    unsigned int status = 0;
    int polling = MAX_POLLING_STATUS;
    unsigned long long l_start_offset;
    unsigned int l_addr;
    unsigned char *l_buf;
    unsigned int ret = 1;  /* != 0 means error occur */
    int err = 0;

    if (0 != len % 512) {
        /* emmc always in slot0 */
        pr_err("debug: parameter error!\n");
        return ret;
    }

    /* find the offset in emmc */
    if (lp_start_sect == (sector_t)(-1) || lp_nr_sects == (sector_t)(-1)){
        pr_err("not find in scatter file error!\n");
        return ret;
    }

    if (lp_nr_sects < (len >> 9)){
        pr_err("write operation oversize!\n");
        return ret;
    }

    if (lp_nr_sects < (offset >> 9)){
        pr_err("write operation oversize!\n");
        return ret;
    }

    if (lp_nr_sects < ((len + offset) >> 9)){
        pr_err("write operation oversize!\n");
        return ret;
    }

#if MTK_MMC_DUMP_DBG
    pr_debug("write start sector = %llu, part size = %llu\n", (u64)lp_start_sect, (u64)lp_nr_sects);
#endif

    l_start_offset = (u64)offset + (u64)(lp_start_sect << 9);

#if MTK_MMC_DUMP_DBG
    pr_debug("write start address = %llu\n", (u64)l_start_offset);
#endif


    if (emmc_init == 0) {
        if(simp_init_emmc() != 0)
            return ret;
    }

    for (i = 0; i < (len/512); i++) {
        /* code */
        l_addr = (l_start_offset >> 9) + i; /*blk address*/
        l_buf  = (buf + i * 512);

#if MTK_MMC_DUMP_DBG
        pr_debug("l_start_offset =0x%x\n", l_addr);
#endif
        /* add address check over expdb for each block */
        if (l_addr >= (lp_start_sect + lp_nr_sects)) {
            pr_err("write 512 Bytes address over boundary at 0x%x\n", l_addr);
            return ret;
        }

        err = simp_mmc_single_write(pmmc_boot_host, l_addr, l_buf, 512);
        if (err) {
            pr_err("write 512 Bytes fail at 0x%x\n", l_addr);
            return ret;
        }

        do{
            simp_mmc_get_status(pmmc_boot_host, &status);
        }while(R1_CURRENT_STATE(status) == 7 && polling--);
    }
    if(err == 0)
        return 0;
    else
        return ret;
}

static int simp_sd_dump_write(unsigned char* buf, unsigned int len, unsigned int offset,unsigned int dev)
{
    unsigned int l_addr;
    unsigned char *l_buf;
    int polling = MAX_POLLING_STATUS;
    unsigned int status = 0;
    int ret = -1;
    int err = 0;

    if (0 != len % 512) {
        /* emmc always in slot0 */
        printk("debug: parameter error!\n");
        return ret;
    }
#if 0
    printk("write data:");
    for (i = 0; i < 32; i++) {
        printk("0x%x", buf[i]);
        if (0 == (i+1)%32)
            printk("\n");
    }
#endif
    if (sd_init == 0) {
        if(simp_init_sd() != 0)
            return ret;
    }

    l_buf  = buf;
    if(pmsdc_extend_host->card->card_cap == standard_capacity) {
        l_addr = offset << 9;
    } else {
        l_addr = offset;
    }

#if MTK_MMC_DUMP_DBG
    printk("l_start_offset = 0x%x len = %d buf<%p>\n", l_addr, len, l_buf);
#endif

    if(len == 512)
        err = simp_mmc_single_write(pmmc_extend_host, l_addr, l_buf, 512);
    else
        err = simp_mmc_multi_write(pmmc_extend_host, l_addr, l_buf, len/512);
    do{
        simp_mmc_get_status(pmmc_extend_host, &status);
    }while(R1_CURRENT_STATE(status) == 7 && polling--);

    if(err == 0)
        ret = 0;

#if MTK_MMC_DUMP_DBG
    printk("%s: dump write ret=0x%x\n", __func__, ret);
#endif
    return ret;
}

static int sd_dump_read(unsigned char* buf, unsigned int len, unsigned int offset)
{
    /* unsigned int i; */
    unsigned int l_addr;
    unsigned char *l_buf;
    unsigned int ret = SIMP_FAILED;
    int err = 0;

    if (0 != len % 512) {
        pr_err("debug: parameter error!\n");
        return ret;
    }

    if (sd_init == 0) {
        if(simp_init_sd() != 0)
            return ret;
    }
    l_buf  = buf;

#if MTK_MMC_DUMP_DBG
    pr_debug("l_start_offset = 0x%x len = %d\n", offset, len);
#endif
    if(pmsdc_extend_host->card->card_cap == standard_capacity) {
        l_addr = offset << 9;
    } else {
        l_addr = offset;
    }
    if(len == 512)
        err = simp_mmc_single_read(pmmc_extend_host, l_addr, l_buf, 512);
    else
        err = simp_mmc_multi_read(pmmc_extend_host, l_addr, l_buf, len/512);
#if 0
    pr_debug("read data:");
    for (i = 0; i < 32; i++) {
        pr_debug("0x%x", buf[i]);
        if (0 == (i+1)%32)
            pr_debug("\n");
    }
#endif
    if(err == 0)
        ret = SIMP_SUCCESS;
    return ret;
}

static int simp_emmc_dump_read(unsigned char* buf, unsigned int len, unsigned int offset,unsigned int dev)
{
     /* maybe delete in furture */
    unsigned int i;
    unsigned int status = 0;
    int polling = MAX_POLLING_STATUS;
    unsigned long long l_start_offset;
    unsigned int l_addr;
    unsigned char *l_buf;
    unsigned int ret = 1;  /* != 0 means error occur */
    int err = 0;

    if (0 != len % 512) {
        /* emmc always in slot0 */
        pr_err("debug: parameter error!\n");
        return ret;
    }

    /* find the offset in emmc */
    if (lp_start_sect == (sector_t)(-1) || lp_nr_sects == (sector_t)(-1)){
        pr_err("not find in scatter file error!\n");
        return ret;
    }

    if (lp_nr_sects < (len >> 9)){
        pr_err("write operation oversize!\n");
        return ret;
    }

    if (lp_nr_sects < (offset >> 9)){
        pr_err("write operation oversize!\n");
        return ret;
    }

    if (lp_nr_sects < ((len + offset) >> 9)){
        pr_err("write operation oversize!\n");
        return ret;
    }

#if MTK_MMC_DUMP_DBG
    pr_debug("write start sector = %llu, part size = %llu\n", (u64)lp_start_sect, (u64)lp_nr_sects);
#endif

    l_start_offset = (u64)offset + (u64)(lp_start_sect << 9);

#if MTK_MMC_DUMP_DBG
    pr_debug("write start address = %llu\n", (u64)l_start_offset);
#endif


    if (emmc_init == 0) {
        if(simp_init_emmc() != 0)
            return ret;
    }

    for (i = 0; i < (len/512); i++) {
        /* code */
        l_addr = (l_start_offset >> 9) + i; /*blk address*/
        l_buf  = (buf + i * 512);

#if MTK_MMC_DUMP_DBG
        pr_debug("l_start_offset =0x%x\n", l_addr);
#endif

        /* add address check over expdb for each block */
        if (l_addr >= (lp_start_sect + lp_nr_sects)) {
            pr_err("read 512 Bytes address over boundary at 0x%x\n", l_addr);
            return ret;
        }

        err = simp_mmc_single_read(pmmc_boot_host, l_addr, l_buf, 512);
        if (err) {
            pr_err("read 512 Bytes fail at 0x%x\n", l_addr);
            return ret;
        }
    }
    if(err == 0)
        return 0;
    else
        return ret;

}

int card_dump_func_write(unsigned char* buf, unsigned int len, unsigned long long offset, int dev)
{
    int ret = SIMP_FAILED;
    unsigned int sec_offset = 0;
#if MTK_MMC_DUMP_DBG
    pr_debug("card_dump_func_write len<%d> addr<%lld> type<%d>\n", len, offset, dev);
#endif
    if(offset % 512){
        pr_err("Address isn't 512 alignment!\n");
        return SIMP_FAILED;
    }
    sec_offset = offset/512;
#if 0
    if(partition_ready_flag != 1) {
	    return ret;
    }
#endif

    switch (dev){
        case DUMP_INTO_BOOT_CARD_IPANIC:
            ret = simp_emmc_dump_write(buf, len, (unsigned int)offset, dev);
            break;
        case DUMP_INTO_BOOT_CARD_KDUMP:
            break;
        case DUMP_INTO_EXTERN_CARD:
            ret = simp_sd_dump_write(buf, len, sec_offset, dev);
            break;
        default:
        pr_err("unknown card type, error!\n");
            break;
    }

    return ret;
}
EXPORT_SYMBOL(card_dump_func_write);

extern int simple_sd_ioctl_rw(struct msdc_ioctl* msdc_ctl);
#define SD_FALSE             (-1)
#define SD_TRUE              (0)
#define DEBUG_MMC_IOCTL      (0)

static int emmc_dump_read(unsigned char *buf, unsigned int len, unsigned int offset,unsigned int slot)
{
    /* maybe delete in furture */
    struct msdc_ioctl msdc_ctl;
    unsigned int i;
    unsigned long long l_start_offset = 0;
    unsigned int ret = SD_FALSE;

    if ((0 != slot) || (0 != offset % 512) || (0 != len % 512)) {
        /* emmc always in slot0 */
        pr_err("debug: slot is not use for emmc!\n");
        return ret;
    }


    if (0 != len % 512) {
        /* emmc always in slot0 */
        pr_err("debug: parameter error!\n");
        return ret;
    }
    /* find the offset in emmc */
    if (lp_start_sect == (sector_t)(-1) || lp_nr_sects == (sector_t)(-1)){
        pr_err("not find in scatter file error!\n");
        return ret;
    }
    if (lp_nr_sects < ((len + offset) >> 9)){
        pr_err("write operation oversize!\n");
        return ret;
    }

#if MTK_MMC_DUMP_DBG
    pr_debug("read start sector = %llu, part size = %llu\n", (u64)lp_start_sect, (u64)lp_nr_sects);
#endif

    l_start_offset = (u64)offset + (u64)(lp_start_sect << 9);
#if MTK_MMC_DUMP_DBG
    pr_debug("read start address = %llu\n", (u64)l_start_offset);
#endif

    msdc_ctl.partition = EMMC_PART_USER;
    msdc_ctl.iswrite = 0;
    msdc_ctl.host_num = slot;
    msdc_ctl.opcode = MSDC_CARD_DUNM_FUNC;
    msdc_ctl.total_size = MAX_DMA_CNT;
    msdc_ctl.trans_type = 0;
    msdc_ctl.address = l_start_offset >> 9;
    msdc_ctl.buffer = (u32 *) buf;
    for (i = 0; i < (len/MAX_DMA_CNT); i++) {
#if DEBUG_MMC_IOCTL
        pr_debug("l_start_offset = 0x%x\n", msdc_ctl.address);
#endif
        msdc_ctl.result = simple_sd_ioctl_rw(&msdc_ctl);
        msdc_ctl.address += MAX_DMA_CNT >> 9;
        msdc_ctl.buffer = (u32 *) (buf + MAX_DMA_CNT * (i+1));
}
    msdc_ctl.total_size = len % MAX_DMA_CNT;
    if(0 != msdc_ctl.total_size){

#if DEBUG_MMC_IOCTL
        pr_debug("l_start_offset =0x%x\n", msdc_ctl.address);
#endif
        msdc_ctl.result = simple_sd_ioctl_rw(&msdc_ctl);
    }

#if DEBUG_MMC_IOCTL
    pr_debug("read data:");
    dump_buff(buf);
#endif

    return SD_TRUE;
}

int card_dump_func_read(unsigned char* buf, unsigned int len, unsigned long long offset, int dev)
{

    unsigned int ret = SIMP_FAILED;
    unsigned int sec_offset = 0;
#if MTK_MMC_DUMP_DBG
    pr_err( "card_dump_func_read len<%d> addr<%lld> type<%d>\n",len,offset,dev);
#endif
    if(offset % 512){
        printk("Address isn't 512 alignment!\n");
        return SIMP_FAILED;
    }
    sec_offset = offset/512;
    if(partition_ready_flag != 1) {
	    return ret;
    }

    switch (dev){
        case DUMP_INTO_BOOT_CARD_IPANIC:
            ret = emmc_dump_read(buf, len, (unsigned int)offset, dev);
            //ret = simp_emmc_dump_read(buf, len, (unsigned int)offset, dev);
            break;
        case DUMP_INTO_BOOT_CARD_KDUMP:
            break;
        case DUMP_INTO_EXTERN_CARD:
            ret = sd_dump_read(buf, len, sec_offset);
            break;
        default:
            printk("unknown card type, error!\n");
            break;
    }
    return ret;

}
EXPORT_SYMBOL(card_dump_func_read);


/*--------------------------------------------------------------------------*/
/* porting for kdump interface                                              */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* module init/exit                                                         */
/*--------------------------------------------------------------------------*/

static void simp_msdc_hw_init(void)
{
	p_msdc_hw[0] = &msdc0_hw;
	p_msdc_hw[1] = &msdc1_hw;
	p_msdc_hw[2] = &msdc2_hw;
}
static int __init emmc_dump_init(void)
{

    simp_msdc_hw_init();
    #ifndef CONFIG_MTD_NAND_ATC
    simp_mmc_init(MSDC_EMMC,1);
    #endif
    simp_mmc_init(MSDC_SD,0);

    return 0;
}

static void __exit emmc_dump_exit(void)
{
}

static struct delayed_work get_dump_info;
static int __init init_get_dump_work(void)
{
	#ifndef CONFIG_MTD_NAND_ATC
	INIT_DELAYED_WORK(&get_dump_info, get_emmc_dump_info);
	schedule_delayed_work(&get_dump_info, 5000);
	#endif
	return 0;
}

module_init(emmc_dump_init);
module_exit(emmc_dump_exit);
late_initcall_sync(init_get_dump_work);
MODULE_LICENSE("GPL");
