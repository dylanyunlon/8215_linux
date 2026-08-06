/*
 * Copyright (C) 2017 AutoChips  Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "autok.h"
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include "mt_sd.h"
#include "dbg.h"
//extern unsigned int ckgen[4];
unsigned int ckgen[4] = {0, 0, 0, 0};

extern void __iomem *topckgen_reg_base;
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sdio.h>

#define AUTOK_VERSION                   (0x17052617)
#define  AUTOK_TMO_DVT			0
#if AUTOK_TMO_DVT
#define AUTOK_CMD_TIMEOUT               (100) /* 100ms */
#define AUTOK_DAT_TIMEOUT               (30000000) /* 300ms */
#else
#define AUTOK_CMD_TIMEOUT               (HZ / 10) /* 100ms */
#define AUTOK_DAT_TIMEOUT               (HZ*3) /* 3s */
#endif
#define MSDC_FIFO_THD_1K                (1024)
#define TUNE_TX_CNT                     (20)
#define CHECK_QSR                       (0x800D)
/* #define TUNE_DATA_TX_ADDR (0x358000) */
/* Use negative value to represent address from end of device,
 * 33 blocks used by SGPT at end of device,
 * 32768 blocks used by flashinfo immediate before SGPT */
#define TUNE_DATA_TX_ADDR 0x35800//(-33-1)

#define CMDQ
#define AUTOK_LATCH_CK_EMMC_TUNE_TIMES  (10) /* 5.0IP eMMC 1KB fifo ZIZE */
#define AUTOK_LATCH_CK_EMMC_TUNE_TIMES_4BIT  (20) /* 5.0IP eMMC 1KB fifo ZIZE */
#define AUTOK_LATCH_CK_SDIO_TUNE_TIMES  (20) /* 4.5IP 1KB fifo CMD19 need send 20 times  */
#define AUTOK_LATCH_CK_SD_TUNE_TIMES    (20) /* 4.5IP 1KB fifo CMD19 need send 20 times  */
#define AUTOK_CMD_TIMES                 (10)
#define AUTOK_TUNING_INACCURACY         (3) /* scan result may find xxxxooxxx */
#define AUTOK_MARGIN_THOLD              (5)
#define AUTOK_BD_WIDTH_REF              (3)
#define AUTOK_BD_WIDTH_DIF              (15)
#define AUTOK_BD_MAX_WIDTH              (15)
#define AUTOK_BD_PASS_MIN               (10)
#define AUTOK_BD_FILTER                 (4)
#define AUTOK_BD_CNT_DIF                (2)
#define AUTOK_BD_POSITION_DIF           (5)
#define AUTOK_BD_CNT_MAX                (3)

#define AUTOK_READ                      0
#define AUTOK_WRITE                     1

#define AUTOK_FINAL_CKGEN_SEL           (0)
#define SCALE_TA_CNTR                   (8)
#define SCALE_CMD_RSP_TA_CNTR           (8)
#define SCALE_WDAT_CRC_TA_CNTR          (8)
#define SCALE_INT_DAT_LATCH_CK_SEL      (8)
#define SCALE_INTERNAL_DLY_CNTR         (32)
#define SCALE_PAD_DAT_DLY_CNTR          (32)

#define TUNING_INACCURACY (2)

/* autok platform specific setting */
#define AUTOK_CKGEN_VALUE                       (0)
#define AUTOK_CMD_LATCH_EN_HS400_PORT0_VALUE    (2)
#define AUTOK_CRC_LATCH_EN_HS400_PORT0_VALUE    (2)
#define AUTOK_CMD_LATCH_EN_HS200_PORT0_VALUE    (2)
#define AUTOK_CRC_LATCH_EN_HS200_PORT0_VALUE    (2)
#define AUTOK_CMD_LATCH_EN_SDR104_PORT1_VALUE   (3)
#define AUTOK_CRC_LATCH_EN_SDR104_PORT1_VALUE   (3)
#define AUTOK_CMD_LATCH_EN_HS_VALUE             (2)
#define AUTOK_CRC_LATCH_EN_HS_VALUE             (2)
#define AUTOK_CMD_TA_VALUE                      (0)
#define AUTOK_CRC_TA_VALUE                      (0)
#define AUTOK_BUSY_MA_VALUE                     (1)

/* autok msdc TX init setting */
#define AUTOK_MSDC0_HS400_CLKTXDLY            0
#define AUTOK_MSDC0_HS400_CMDTXDLY            0
#define AUTOK_MSDC0_HS400_DAT0TXDLY           0
#define AUTOK_MSDC0_HS400_DAT1TXDLY           0
#define AUTOK_MSDC0_HS400_DAT2TXDLY           0
#define AUTOK_MSDC0_HS400_DAT3TXDLY           0
#define AUTOK_MSDC0_HS400_DAT4TXDLY           0
#define AUTOK_MSDC0_HS400_DAT5TXDLY           0
#define AUTOK_MSDC0_HS400_DAT6TXDLY           0
#define AUTOK_MSDC0_HS400_DAT7TXDLY           0
#define AUTOK_MSDC0_HS400_TXSKEW              0

#define AUTOK_MSDC0_DDR50_DDRCKD              1
#define AUTOK_MSDC_DDRCKD                     0

#define AUTOK_MSDC0_CLKTXDLY                  0
#define AUTOK_MSDC0_CMDTXDLY                  0
#define AUTOK_MSDC0_DAT0TXDLY                 0
#define AUTOK_MSDC0_DAT1TXDLY                 0
#define AUTOK_MSDC0_DAT2TXDLY                 0
#define AUTOK_MSDC0_DAT3TXDLY                 0
#define AUTOK_MSDC0_DAT4TXDLY                 0
#define AUTOK_MSDC0_DAT5TXDLY                 0
#define AUTOK_MSDC0_DAT6TXDLY                 0
#define AUTOK_MSDC0_DAT7TXDLY                 0

#define AUTOK_MSDC0_TXSKEW                    0

#define AUTOK_MSDC1_CLK_TX_VALUE              10
#define AUTOK_MSDC1_CLK_SDR104_TX_VALUE       10


#ifndef AUTOK_UPDATE_DISABLE
#define AUTOK_MSDC2_CLK_TX_VALUE              10
#define AUTOK_MSDC3_CLK_TX_VALUE              10
#else
#define AUTOK_MSDC2_CLK_TX_VALUE              0
#define AUTOK_MSDC3_CLK_TX_VALUE              0
#endif

#define PORT0_PB0_RD_DAT_SEL_VALID
#define PORT1_PB0_RD_DAT_SEL_VALID
#define PORT2_PB0_RD_DAT_SEL_VALID
#define PORT3_PB0_RD_DAT_SEL_VALID

enum TUNE_TX_TYPE {
	TX_CMD = 0,
	TX_DATA,
};

enum EXD_RW_FLAG {
	EXT_READ = 0,
	EXT_WRITE,
};

#define autok_msdc_retry(expr, retry, cnt) \
	do { \
		int backup = cnt; \
		while (retry) { \
			if (!(expr)) \
				break; \
			if (cnt-- == 0) { \
				retry--; cnt = backup; \
			} \
		} \
	WARN_ON(retry == 0); \
} while (0)

#define autok_msdc_reset() \
	do { \
		int retry = 3, cnt = 1000; \
		MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_RST); \
		/* ensure reset operation be sequential  */ \
		mb(); \
		autok_msdc_retry(MSDC_READ32(MSDC_CFG) & MSDC_CFG_RST, retry, cnt); \
	} while (0)

#define msdc_rxfifocnt() \
	((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_RXCNT) >> 0)
#define msdc_txfifocnt() \
	((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_TXCNT) >> 16)
#if AUTOK_TMO_DVT
#define wait_cond_tmo(cond, tmo) \
	do { \
		unsigned long timeout = tmo*1000; \
		while (1) { \
			if ((cond) || (timeout == 0)) \
				break; \
			udelay(1);\
			timeout--; \
		} \
	} while (0)
#else
#define wait_cond_tmo(cond, tmo) \
	do { \
		unsigned long timeout = tmo+jiffies; \
		while (1) { \
			if ((cond) || (tmo == 0)) \
				break; \
			if (time_after(jiffies,timeout) && (!cond))\
				tmo=0; \
		} \
	} while (0)

#endif
#define msdc_clear_fifo() \
	do { \
		int retry = 5, cnt = 1000; \
		MSDC_SET_BIT32(MSDC_FIFOCS, MSDC_FIFOCS_CLR); \
		/* ensure fifo clear operation be sequential  */ \
		mb(); \
		autok_msdc_retry(MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_CLR, retry, cnt); \
	} while (0)

struct AUTOK_PARAM_RANGE {
	unsigned int start;
	unsigned int end;
};

struct AUTOK_PARAM_INFO {
	struct AUTOK_PARAM_RANGE range;
	char *param_name;
};

struct BOUND_INFO_NEW {
	unsigned char Bound_Start;
	unsigned char Bound_End;
};

#define	BD_NO_ERR (0)
#define	BD_OVERLAP_ERR (1 << 0)
#define	BD_WIDER_ERR (1 << 1)
#define	BD_WIDER_MAX_ERR (1 << 2)
#define	BD_CNT_ERR (1 << 3)
#define	BD_PASS_WIN_NARROW_ERR (1 << 4)
#define	BD_ALTERNATE_ERR (1 << 5)
#define	BD_CNT_DIF_ERR (1 << 6)
#define	BD_RELA_LOCA_ERR (1 << 7)
#define	BD_ALL_FAIL_ERR (1 << 8)

#define BD_MAX_CNT_NEW 32	/* Max Allowed Boundary Number */
struct AUTOK_SCAN_RES_NEW {
	/* Bound info record, currently only allow max to 32 fail bounds exist */
	struct BOUND_INFO_NEW fail_bd_info[BD_MAX_CNT_NEW];
	struct BOUND_INFO_NEW pass_bd_info[BD_MAX_CNT_NEW];
	/* Bound cnt record */
	unsigned char fail_bd_cnt;
	unsigned char pass_bd_cnt;
};
struct AUTOK_REF_INFO_NEW {
	/* inf[0] - rising edge res, inf[1] - falling edge res */
	struct AUTOK_SCAN_RES_NEW scan_info[2];
	/* optimised sample edge select */
	unsigned int opt_edge_sel;
	/* optimised dly cnt sel */
	unsigned int opt_dly_cnt;
	/* 1clk cycle equal how many delay cell cnt, if cycle_cnt is 0,
	   that is cannot calc cycle_cnt by current Boundary info */
	unsigned int cycle_cnt;
};

enum AUTOK_TX_SCAN_STA_E {
	START_POSITION = 0,
	PASS_POSITION,
	FAIL_POSITION,
};

enum AUTOK_SCAN_WIN {
	CMD_RISE,
	CMD_FALL,
	DAT_RISE,
	DAT_FALL,
	DS_WIN,
	D_CMD_RX,
	D_DATA_RX,
	H_CMD_TX,
	H_DATA_TX,
};

unsigned int g_autok_total_sectors;
unsigned int autok_debug_level = AUTOK_DBG_RES;
 
const struct AUTOK_PARAM_INFO autok_param_info[] = {
	{{0, 1}, "CMD_EDGE"},
	{{0, 1}, "RDATA_EDGE"},         /* async fifo mode Pad dat edge must fix to 0 */
	{{0, 1}, "RD_FIFO_EDGE"},
	{{0, 1}, "WDATA_EDGE"},
	{{0, 1}, "WD_FIFO_EDGE"},

	{{0, 31}, "CMD_RD_D_DLY1"},     /* Cmd Pad Tune Data Phase */
	{{0, 1}, "CMD_RD_D_DLY1_SEL"},
	{{0, 31}, "CMD_RD_D_DLY2"},
	{{0, 1}, "CMD_RD_D_DLY2_SEL"},

	{{0, 31}, "DAT_RD_D_DLY1"},     /* Data Pad Tune Data Phase */
	{{0, 1}, "DAT_RD_D_DLY1_SEL"},
	{{0, 31}, "DAT_RD_D_DLY2"},
	{{0, 1}, "DAT_RD_D_DLY2_SEL"},

	{{0, 7}, "INT_DAT_LATCH_CK"},   /* Latch CK Delay for data read when clock stop */

	{{0, 31}, "EMMC50_DS_Z_DLY1"},	/* eMMC50 Related tuning param */
	{{0, 1}, "EMMC50_DS_Z_DLY1_SEL"},
	{{0, 31}, "EMMC50_DS_Z_DLY2"},
	{{0, 1}, "EMMC50_DS_Z_DLY2_SEL"},
	{{0, 31}, "EMMC50_DS_ZDLY_DLY"},

	/* ================================================= */
	/* Timming Related Mux & Common Setting Config */
	{{0, 1}, "READ_DATA_SMPL_SEL"},         /* all data line path share sample edge */
	{{0, 1}, "WRITE_DATA_SMPL_SEL"},
	{{0, 1}, "DATA_DLYLINE_SEL"},           /* clK tune all data Line share dly */
	{{0, 1}, "MSDC_WCRC_ASYNC_FIFO_SEL"},   /* data tune mode select */
	{{0, 1}, "MSDC_RESP_ASYNC_FIFO_SEL"},   /* data tune mode select */
	/* eMMC50 Function Mux */
	{{0, 1}, "EMMC50_WDATA_MUX_EN"},        /* write path switch to emmc45 */
	{{0, 1}, "EMMC50_CMD_MUX_EN"},          /* response path switch to emmc45 */
	{{0, 1}, "EMMC50_WDATA_EDGE"},
	/* Common Setting Config */
	{{0, 31}, "CKGEN_MSDC_DLY_SEL"},
	{{1, 7}, "CMD_RSP_TA_CNTR"},
	{{1, 7}, "WRDAT_CRCS_TA_CNTR"},
	{{0, 31}, "PAD_CLK_TXDLY"},             /* tx clk dly fix to 0 for HQA res */
};

/*used to store autok result*/
unsigned char autok_tune_res0[TUNING_PARAM_COUNT];
unsigned char autok_tune_res1[TUNING_PARAM_COUNT];
unsigned char autok_tune_res2[TUNING_PARAM_COUNT];
unsigned char autok_tune_res3[TUNING_PARAM_COUNT];

/*Autok Timeout error count*/

int autok_cmd_tmo_count[]={0,0,0,0};
int autok_data_tmo_count[]={0,0,0,0};

int autok_save_to_metazone(void *arg);
/**********************************************************
* AutoK Basic Interface Implenment                        *
**********************************************************/

static void autok_reset_err_counter(struct msdc_host *host)
{
	int id=host->id;
	if(autok_cmd_tmo_count[id])
		MSDC_LOG(ERR,"[AUTOK]CMD TMO ERR counter %d", autok_cmd_tmo_count[id]);

	if(autok_data_tmo_count[id])
		MSDC_LOG(ERR,"[AUTOK]DATA TMO ERR counter %d",autok_data_tmo_count[id]);

	autok_cmd_tmo_count[id]=0;
	autok_data_tmo_count[id]=0;
}
static int autok_sdio_device_rx_set(struct msdc_host *host, unsigned int func_num, unsigned int base_addr,
unsigned int *reg_value, unsigned int r_w_dirc, unsigned int blk_len, unsigned int opcode)
{
	unsigned long base = host->base;
	unsigned int rawcmd = 0;
	unsigned int arg = 0;
	unsigned int sts = 0;
	unsigned int wints = 0;
	unsigned long tmo = 0;
	unsigned long write_tmo = 0;
	int ret = E_RESULT_PASS;
	unsigned int i = 0;
#if 0
	int retry = 3, cnt = 1000;
	unsigned int clk_mode, clk_div;

	if (opcode == SD_IO_RW_DIRECT) {
		MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, clk_mode);
		MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, clk_div);
		MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, 0);
		MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, 5);
		autok_msdc_retry(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB), retry, cnt);
	}
#endif

	switch (opcode) {
	case SD_IO_RW_DIRECT:
		rawcmd = (1 << 7) | (52);
		if (r_w_dirc) {
		arg = (r_w_dirc << 31) | (func_num << 28)
			| (base_addr << 9) | (*reg_value)
			| ((r_w_dirc) ? 0x08000000 : 0x00000000);
		} else {
			arg = (r_w_dirc << 31) | (func_num << 28)
				| (base_addr << 9)
				| ((r_w_dirc) ? 0x08000000 : 0x00000000);
		}
		MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case SD_IO_RW_EXTENDED:
		rawcmd = (blk_len << 16) | (r_w_dirc << 13) | (1 << 11) | (1 << 7) | (53);
		arg = (r_w_dirc << 31) | (func_num << 28)
			| (base_addr << 9) | (0 << 26) | (0 << 27) | (blk_len);
		MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	}
	tmo = AUTOK_DAT_TIMEOUT;
	wait_cond_tmo(!(MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY), tmo);
	if (tmo == 0) {
		MSDC_LOG(ERR,"[AUTOK]DRS MSDC busy tmo1 goto end...");
		ret = E_RESULT_FATAL_ERR;
		goto end;
	}

	/* clear fifo */
	autok_msdc_reset();
	msdc_clear_fifo();
	MSDC_WRITE32(MSDC_INT, 0xffffffff);

	/* start command */
	MSDC_WRITE32(SDC_ARG, arg);
	MSDC_WRITE32(SDC_CMD, rawcmd);

	/* wait interrupt status */
	wints = MSDC_INT_CMDTMO | MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR;
	tmo = AUTOK_CMD_TIMEOUT;
	wait_cond_tmo(((sts = MSDC_READ32(MSDC_INT)) & wints), tmo);
	if (tmo == 0) {
		MSDC_LOG(ERR,"[AUTOK]DRS wait int tmo");
		ret |= E_RESULT_CMD_TMO;
		goto end;
	}

	MSDC_WRITE32(MSDC_INT, (sts & wints));
	if (sts == 0) {
		ret |= E_RESULT_CMD_TMO;
		goto end;
	}

	if (sts & MSDC_INT_CMDRDY)
		ret |= E_RESULT_PASS;
	else if (sts & MSDC_INT_RSPCRCERR) {
		ret |= E_RESULT_RSP_CRC;
		goto end;
	} else if (sts & MSDC_INT_CMDTMO) {
		MSDC_LOG(ERR,"[AUTOK]DRS HW tmo");
		ret |= E_RESULT_CMD_TMO;
		goto end;
	}

	tmo = jiffies + AUTOK_DAT_TIMEOUT;
	while ((MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY) && (tmo != 0)) {
#if	AUTOK_TMO_DVT
		tmo --;
#else
		if(time_after(jiffies,tmo))
			tmo=0;
#endif
		if ((r_w_dirc == EXT_WRITE) && (opcode == SD_IO_RW_EXTENDED)) {
			if(blk_len<=4)
				MSDC_WRITE32(MSDC_TXDATA, *reg_value);
			else{
				for (i = 0; i < blk_len/8; i++) {
					MSDC_WRITE32(MSDC_TXDATA, 0x12345678);
					MSDC_WRITE32(MSDC_TXDATA, 0x33cc33cc);
				}
			}
#if 0
			UTIL_Printf("[AUTOK]DRS write reg %x dat %x ...\n",
				base_addr, *reg_value);
#endif
			write_tmo = AUTOK_DAT_TIMEOUT;
			wait_cond_tmo(!(MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY), write_tmo);
			if (write_tmo == 0) {
				MSDC_LOG(ERR,"[AUTOK]DRS MSDC busy tmo2 while write...");
				ret |= E_RESULT_FATAL_ERR;
				goto end;
			}
		}
	}
	if ((tmo == 0) && (MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY)) {
		MSDC_LOG(ERR,"[AUTOK]DRS MSDC busy tmo3...");
		ret |= E_RESULT_FATAL_ERR;
		goto end;
	}

	sts = MSDC_READ32(MSDC_INT);
	wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATCRCERR | MSDC_INT_DATTMO;
	if (sts) {
		/* clear status */
		MSDC_WRITE32(MSDC_INT, (sts & wints));
		if (sts & MSDC_INT_XFER_COMPL) {
			if ((r_w_dirc == EXT_READ) && (opcode == SD_IO_RW_EXTENDED)) {
				*reg_value = MSDC_READ32(MSDC_RXDATA);
#if 0
				UTIL_Printf("[AUTOK]DRS read reg %x dat %x ...\n",
					base_addr, *reg_value);
#endif
			}
			ret |= E_RESULT_PASS;
		}
		if (MSDC_INT_DATCRCERR & sts) {
			ret |= E_RESULT_DAT_CRC;
#if 0
			UTIL_Printf("[AUTOK]DRS dat crc...\n");
#endif
		}
		if (MSDC_INT_DATTMO & sts) {
			ret |= E_RESULT_DAT_TMO;
			MSDC_LOG(ERR,"[AUTOK]DRS dat tmo...");
		}
	}
end:
#if 0
	if (opcode == SD_IO_RW_DIRECT) {
		MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, clk_mode);
		MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, clk_div);
		autok_msdc_retry(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB), retry, cnt);
	}
#endif
	return ret;
}
static int autok_send_tune_cmd(struct msdc_host *host, unsigned int opcode, enum TUNE_TYPE tune_type_value)
{
	void __iomem *base = host->base;
	unsigned int value;
	unsigned int rawcmd = 0;
	unsigned int arg = 0;
	unsigned int sts = 0;
	unsigned int wints = 0;
	unsigned long tmo = 0;
	unsigned long write_tmo = 0;
	unsigned int left = 0;
	unsigned int fifo_have = 0;
	unsigned int fifo_1k_cnt = 0;
	unsigned int i = 0;
	unsigned int bus_width = 0;
	int ret = E_RESULT_PASS;
#ifdef MSDC_MON_DBG
	unsigned int dat_dly1, dat_dly2;
#endif
	unsigned int clk_tx;
	struct timeval t_start, t_end;

	switch (opcode) {
	case MMC_SEND_EXT_CSD:
		rawcmd =  (512 << 16) | (0 << 13) | (1 << 11) | (1 << 7) | (8);
		arg = 0;
		if (tune_type_value == TUNE_LATCH_CK)
			MSDC_WRITE32(SDC_BLK_NUM, host->tune_latch_ck_cnt);
		else
			MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case MMC_STOP_TRANSMISSION:
		rawcmd = (1 << 14)  | (7 << 7) | (12);
		arg = 0;
		break;
	case MMC_SEND_STATUS:
		rawcmd = (1 << 7) | (13);
		arg = (1 << 16);
		break;
	case MMC_READ_SINGLE_BLOCK:
		left = 512;
		rawcmd =  (512 << 16) | (0 << 13) | (1 << 11) | (1 << 7) | (17);
		arg = 0;
		if (tune_type_value == TUNE_LATCH_CK)
			MSDC_WRITE32(SDC_BLK_NUM, host->tune_latch_ck_cnt);
		else
			MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case SD_CMD_SEND_TUNING_BLOCK:
		left = 64;
		rawcmd =  (64 << 16) | (0 << 13) | (1 << 11) | (1 << 7) | (19);
		arg = 0;
		if (tune_type_value == TUNE_LATCH_CK)
			MSDC_WRITE32(SDC_BLK_NUM, host->tune_latch_ck_cnt);
		else
			MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case MMC_SEND_TUNING_BLOCK_HS200:
		MSDC_GET_FIELD(SDC_CFG, SDC_CFG_BUSWIDTH, bus_width);
		if (bus_width == 2) {
			left = 128;
			rawcmd =  (128 << 16) | (0 << 13) | (1 << 11) | (1 << 7) | (21);
			arg = 0;
		} else if (bus_width == 1) {
			left = 64;
			rawcmd =  (64 << 16) | (0 << 13) | (1 << 11) | (1 << 7) | (21);
			arg = 0;
		}
		if (tune_type_value == TUNE_LATCH_CK)
			MSDC_WRITE32(SDC_BLK_NUM, host->tune_latch_ck_cnt);
		else
			MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case MSDC_SDIO_CLK_TX_TUNE:
		/* get clk tx dly */
		MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, clk_tx);
		rawcmd =  (64 << 16) | (1 << 13) | (1 << 11) | (1 << 7) | (53);
		arg = (0x80000000) | (2 << 28) | (0xB0 << 9) | (0 << 26) | (0 << 27) | (64);
		MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case MSDC_SD_CLK_TX_TUNE:
		/* get clk tx dly */
		MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, clk_tx);
		rawcmd =  (512 << 16) | (1 << 13) | (1 << 11) | (1 << 7) | (24);
		if (TUNE_DATA_TX_ADDR >= 0)
			arg = TUNE_DATA_TX_ADDR;
		else
			arg = g_autok_total_sectors + TUNE_DATA_TX_ADDR;
			MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case MMC_WRITE_BLOCK:
		rawcmd =  (512 << 16) | (1 << 13) | (1 << 11) | (1 << 7) | (24);
		if (TUNE_DATA_TX_ADDR >= 0)
			arg = TUNE_DATA_TX_ADDR;
		else
			arg = g_autok_total_sectors + TUNE_DATA_TX_ADDR;
		MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case SD_IO_RW_DIRECT:
		rawcmd = (1 << 7) | (52);
		arg = (0x80000000) | (0 << 28) | (SDIO_CCCR_ABORT << 9) | (0);
		MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case SD_IO_RW_EXTENDED:
#if 0
		rawcmd =  (4 << 16) | (1 << 13) | (1 << 11) | (1 << 7) | (53);
		arg = (0x80000000) | (0 << 28) | (0xE0 << 9) | (0 << 26) | (0 << 27) | (4);
#endif
		rawcmd =  (4 << 16) | (0 << 13) | (1 << 11) | (1 << 7) | (53);
		arg = (0x00000000) | (0 << 28) | (0xB0 << 9) | (0 << 26) | (0 << 27) | (4);
		MSDC_WRITE32(SDC_BLK_NUM, 1);
		break;
	case 59:
		left = 64;
		rawcmd =  (64 << 16) | (0 << 13) | (1 << 11) | (1 << 7) | (59);
		arg = 0;
		break;
	}

	tmo = AUTOK_DAT_TIMEOUT;
	wait_cond_tmo(!(MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY), tmo);
	if (tmo == 0) {
		MSDC_LOG(ERR,"[AUTOK]MSDC busy tmo1 cmd%d goto end...", opcode&0x7F);
		ret |= E_RESULT_FATAL_ERR;
		goto end;
	}

	/* clear fifo */
	if ((tune_type_value == TUNE_CMD) || (tune_type_value == TUNE_DATA)) {
		autok_msdc_reset();
		msdc_clear_fifo();
		MSDC_WRITE32(MSDC_INT, 0xffffffff);
	}

	/* start command */
	MSDC_WRITE32(SDC_ARG, arg);
	MSDC_WRITE32(SDC_CMD, rawcmd);

	/* wait interrupt status */
	wints = MSDC_INT_CMDTMO | MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR;
	tmo = AUTOK_CMD_TIMEOUT;
	wait_cond_tmo(((sts = MSDC_READ32(MSDC_INT)) & wints), tmo);
	if (tmo == 0) {
		//MSDC_LOG(ERR,"[AUTOK]CMD%d wait int tmo", opcode&0x7F);
		ret |= E_RESULT_CMD_TMO;
		autok_cmd_tmo_count[host->id]++;
		goto end;
	}

	MSDC_WRITE32(MSDC_INT, (sts & wints));
	if (sts == 0) {
		ret |= E_RESULT_CMD_TMO;
		goto end;
	}

	if (sts & MSDC_INT_CMDRDY) {
		ret |= E_RESULT_PASS;
		if (tune_type_value == TUNE_CMD)
			goto end;
	} else if (sts & MSDC_INT_RSPCRCERR) {
		ret |= E_RESULT_RSP_CRC;
		if ((tune_type_value == TUNE_CMD) || (tune_type_value == TUNE_LATCH_CK))
			goto end;
	} else if (sts & MSDC_INT_CMDTMO) {
		//MSDC_LOG(ERR,"[AUTOK]CMD%d HW tmo", opcode&0x7F);
		ret |= E_RESULT_CMD_TMO;
		autok_cmd_tmo_count[host->id]++;
		if ((tune_type_value == TUNE_CMD) || (tune_type_value == TUNE_LATCH_CK))
			goto end;
	}

	if ((tune_type_value != TUNE_LATCH_CK) && (tune_type_value != TUNE_DATA))
		goto skip_tune_latch_ck_and_tune_data;
	tmo = jiffies+AUTOK_DAT_TIMEOUT;
	while ((MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY) && (tmo != 0)) {
#if	AUTOK_TMO_DVT
		tmo --;
#else
		if(time_after(jiffies,tmo))
			tmo=0;
#endif
		if (tune_type_value == TUNE_LATCH_CK) {
			fifo_have = msdc_rxfifocnt();
			if ((opcode == MMC_SEND_TUNING_BLOCK_HS200) || (opcode == MMC_READ_SINGLE_BLOCK)
				|| (opcode == MMC_SEND_EXT_CSD) || (opcode == SD_CMD_SEND_TUNING_BLOCK)
				|| (opcode == SD_IO_RW_EXTENDED)) {
				MSDC_SET_FIELD(MSDC_DBG_SEL, 0xffff << 0, 0x0b);
				MSDC_GET_FIELD(MSDC_DBG_OUT, 0x7ff << 0, fifo_1k_cnt);
				if ((fifo_1k_cnt >= MSDC_FIFO_THD_1K) && (fifo_have >= MSDC_FIFO_SZ)) {
					value = MSDC_READ32(MSDC_RXDATA);
					value = MSDC_READ32(MSDC_RXDATA);
					value = MSDC_READ32(MSDC_RXDATA);
					value = MSDC_READ32(MSDC_RXDATA);
				}
			}
		} else if ((tune_type_value == TUNE_DATA) && (opcode == MMC_WRITE_BLOCK)) {
			for (i = 0; i < 64; i++) {
#if AUTOK_OFFLINE_TUNE_LATCH_EN_ENABLE
				MSDC_WRITE32(MSDC_TXDATA, 0);
				MSDC_WRITE32(MSDC_TXDATA, 0);
#else
				MSDC_WRITE32(MSDC_TXDATA, 0xff00ff00);
				MSDC_WRITE32(MSDC_TXDATA, 0xff00ff00);
#endif
			}

			write_tmo = AUTOK_DAT_TIMEOUT;
			wait_cond_tmo(!(MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY), write_tmo);
			if (write_tmo == 0) {
				MSDC_LOG(ERR,"[AUTOK]MSDC busy tmo2 while write cmd%d goto end...", opcode&0x7F);
				ret |= E_RESULT_FATAL_ERR;
				goto end;
			}
		} else if ((tune_type_value == TUNE_DATA) && (opcode == MSDC_SD_CLK_TX_TUNE)) {
			MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, host->clk_tx);
			for (i = 0; i < 32; i++) {
				MSDC_WRITE32(MSDC_TXDATA, 0x33cc33cc);
				MSDC_WRITE32(MSDC_TXDATA, 0x33cc33cc);
			}
			/* restore clk tx brefore half data transfer */
			MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, clk_tx);
			do_gettimeofday(&t_start);
			do_gettimeofday(&t_end);
			while ((t_end.tv_usec - t_start.tv_usec) <= 10)
				do_gettimeofday(&t_end);
			for (i = 0; i < 32; i++) {
				MSDC_WRITE32(MSDC_TXDATA, 0x33cc33cc);
				MSDC_WRITE32(MSDC_TXDATA, 0x33cc33cc);
			}

			write_tmo = AUTOK_DAT_TIMEOUT;
			wait_cond_tmo(!(MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY), write_tmo);
			if (write_tmo == 0) {
				UTIL_Printf("[AUTOK]MSDC busy tmo2 while write cmd%d goto end...\r\n", opcode);
				ret |= E_RESULT_FATAL_ERR;
				goto end;
			}
		} else if ((tune_type_value == TUNE_DATA) && (opcode == MSDC_SDIO_CLK_TX_TUNE)) {
			MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, host->clk_tx);
			for (i = 0; i < 4; i++) {
				MSDC_WRITE32(MSDC_TXDATA, 0x12345678);
				MSDC_WRITE32(MSDC_TXDATA, 0x33cc33cc);
			}
			/* restore clk tx brefore half data transfer */
			MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, clk_tx);
			do_gettimeofday(&t_start);
			do_gettimeofday(&t_end);
			while ((t_end.tv_usec - t_start.tv_usec) <= 10)
				do_gettimeofday(&t_end);
			for (i = 0; i < 4; i++) {
				MSDC_WRITE32(MSDC_TXDATA, 0x87654321);
				MSDC_WRITE32(MSDC_TXDATA, 0xcc33cc33);
			}

			write_tmo = AUTOK_DAT_TIMEOUT;
			wait_cond_tmo(!(MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY), write_tmo);
			if (write_tmo == 0) {
				UTIL_Printf("[AUTOK]MSDC busy tmo2 while write cmd%d goto end...\r\n", opcode);
				ret |= E_RESULT_FATAL_ERR;
				goto end;
			}
		}
	}
	if ((tmo == 0) && (MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY)) {
		MSDC_LOG(ERR,"[AUTOK]MSDC busy tmo3 cmd%d goto end...", opcode&0x7F);
		ret |= E_RESULT_FATAL_ERR;
		goto end;
	}

	sts = MSDC_READ32(MSDC_INT);
	wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATCRCERR | MSDC_INT_DATTMO;
	if (sts) {
		/* clear status */
		MSDC_WRITE32(MSDC_INT, (sts & wints));
		if (sts & MSDC_INT_XFER_COMPL)
			ret |= E_RESULT_PASS;
		if (MSDC_INT_DATCRCERR & sts)
			ret |= E_RESULT_DAT_CRC;
 		if (MSDC_INT_DATTMO & sts) {
			ret |= E_RESULT_DAT_TMO;
			autok_data_tmo_count[host->id]++;
			//MSDC_LOG(ERR,"[AUTOK]MSDC cmd%d data tmo", opcode&0x7F);
		}
	}

skip_tune_latch_ck_and_tune_data:
	tmo = AUTOK_DAT_TIMEOUT;
	wait_cond_tmo(!(MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY), tmo);
	if (tmo == 0) {
		MSDC_LOG(ERR,"[AUTOK]MSDC busy tmo4 cmd%d goto end...", opcode&0x7F);
		ret |= E_RESULT_FATAL_ERR;
		goto end;
	}
	if ((tune_type_value == TUNE_CMD) || (tune_type_value == TUNE_DATA))
		msdc_clear_fifo();

end:
	if (opcode == MMC_STOP_TRANSMISSION) {
		tmo = AUTOK_DAT_TIMEOUT;
		wait_cond_tmo(((MSDC_READ32(MSDC_PS) & 0x10000) == 0x10000), tmo);
		if (tmo == 0) {
			MSDC_LOG(ERR,"[AUTOK]DTA0 busy tmo cmd%d goto end...", opcode&0x7F);
			ret |= E_RESULT_FATAL_ERR;
		}
	}

	if((autok_cmd_tmo_count[host->id]+autok_data_tmo_count[host->id])>AUTOK_MAX_ERR)
		ret = E_RESULT_FATAL_ERR;
	
	return ret;
}

static int autok_simple_score64(char *res_str64, unsigned long result64)
{
	unsigned int bit = 0;
	unsigned int num = 0;
	unsigned int old = 0;

	if (result64 == 0) {
		/* maybe result is 0 */
		strcpy(res_str64, "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO");
		return 64;
	}
	if (result64 == 0xFFFFFFFFFFFFFFFF) {
		strcpy(res_str64,
		       "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
		return 0;
	}

	/* calc continue zero number */
	while (bit < 64) {
		if (result64 & ((unsigned long) (1LL << bit))) {
			res_str64[bit] = 'X';
			bit++;
			if (old < num)
				old = num;
			num = 0;
			continue;
		}
		res_str64[bit] = 'O';
		bit++;
		num++;
	}
	if (num > old)
		old = num;

	res_str64[64] = '\0';
	return old;
}

enum {
	RD_SCAN_NONE,
	RD_SCAN_PAD_BOUND_S,
	RD_SCAN_PAD_BOUND_E,
	RD_SCAN_PAD_MARGIN,
};

/*get fail_bd_info ,pass_bd_info from rawdat , and updata it arcorrding bd_filter*/
static int autok_check_scan_res64_new(u64 rawdat, struct AUTOK_SCAN_RES_NEW *scan_res, unsigned int bd_filter)
{
	unsigned int bit;
	int i, j;
	unsigned char fail_bd_info_cnt = 0;
	unsigned char pass_bd_info_cnt = 0;
	enum AUTOK_TX_SCAN_STA_E RawScanSta = START_POSITION;

	/* check scan window boundary */
	for (bit = 0; bit < 64; bit++) {
		if (rawdat & (1LL << bit)) {
			switch (RawScanSta) {
			case START_POSITION:
				RawScanSta = FAIL_POSITION;
				scan_res->fail_bd_info[fail_bd_info_cnt++].Bound_Start = bit;
				scan_res->fail_bd_cnt++;
				break;
			case PASS_POSITION:
				RawScanSta = FAIL_POSITION;
				if (bit == 63) {
					scan_res->fail_bd_info[fail_bd_info_cnt++].Bound_Start = bit;
					scan_res->fail_bd_info[fail_bd_info_cnt - 1].Bound_End = bit;
				} else
					scan_res->fail_bd_info[fail_bd_info_cnt++].Bound_Start = bit;
				scan_res->pass_bd_info[pass_bd_info_cnt - 1].Bound_End = bit - 1;
				scan_res->fail_bd_cnt++;
				break;
			case FAIL_POSITION:
				RawScanSta = FAIL_POSITION;
				if (bit == 63)
					scan_res->fail_bd_info[fail_bd_info_cnt - 1].Bound_End = bit;
				break;
			default:
				break;
			}
		} else {
			switch (RawScanSta) {
			case START_POSITION:
				RawScanSta = PASS_POSITION;
				scan_res->pass_bd_info[pass_bd_info_cnt++].Bound_Start = bit;
				scan_res->pass_bd_cnt++;
				break;
			case PASS_POSITION:
				RawScanSta = PASS_POSITION;
				if (bit == 63)
					scan_res->pass_bd_info[pass_bd_info_cnt - 1].Bound_End = bit;
				break;
			case FAIL_POSITION:
				RawScanSta = PASS_POSITION;
				if (bit == 63) {
					scan_res->pass_bd_info[pass_bd_info_cnt++].Bound_Start = bit;
					scan_res->pass_bd_info[pass_bd_info_cnt-1].Bound_End = bit;
				} else
					scan_res->pass_bd_info[pass_bd_info_cnt++].Bound_Start = bit;
				scan_res->fail_bd_info[fail_bd_info_cnt - 1].Bound_End = bit - 1;
				scan_res->pass_bd_cnt++;
				break;
			default:
				break;
			}
		}
	}
#if AUTOK_DEBUG_LOG
	for (i = 0; i < scan_res->fail_bd_cnt; i++) {
		UTIL_Printf("[AUTOK]pre fail bd: S-%d E-%d\r\n",
			scan_res->fail_bd_info[i].Bound_Start
			, scan_res->fail_bd_info[i].Bound_End);
	}
#endif
	for (i = scan_res->fail_bd_cnt; i >= 0; i--) {
		if (i > scan_res->fail_bd_cnt)
			break;
		if ((i >= 1) && ((scan_res->fail_bd_info[i].Bound_Start
			- scan_res->fail_bd_info[i - 1].Bound_End - 1) < bd_filter)) {
			scan_res->fail_bd_info[i - 1].Bound_End = scan_res->fail_bd_info[i].Bound_End;
			scan_res->fail_bd_info[i].Bound_Start = 0;
			scan_res->fail_bd_info[i].Bound_End = 0;
			for (j = i; j < (scan_res->fail_bd_cnt - 1); j++) {
				scan_res->fail_bd_info[j].Bound_Start = scan_res->fail_bd_info[j + 1].Bound_Start;
				scan_res->fail_bd_info[j].Bound_End = scan_res->fail_bd_info[j + 1].Bound_End;
			}
			scan_res->fail_bd_info[scan_res->fail_bd_cnt - 1].Bound_Start = 0;
			scan_res->fail_bd_info[scan_res->fail_bd_cnt - 1].Bound_End = 0;
			scan_res->fail_bd_cnt--;
		}
	}
#if AUTOK_DEBUG_LOG
	for (i = 0; i < scan_res->fail_bd_cnt; i++) {
		UTIL_Printf("[AUTOK]cur fail bd: S-%d E-%d\r\n",
			scan_res->fail_bd_info[i].Bound_Start
			, scan_res->fail_bd_info[i].Bound_End);
	}
#endif
	return 0;
}

static unsigned int autok_check_scan_effective(struct AUTOK_REF_INFO_NEW *scan_info)
{
	unsigned char i, j;
	unsigned int bd_err = BD_NO_ERR;
	/* check if there is no pass boundary that may occur read broken hole */
	if ((scan_info->scan_info[0].pass_bd_cnt == 0)
		&& (scan_info->scan_info[1].pass_bd_cnt == 0))
		bd_err |= BD_ALL_FAIL_ERR;
	/* check if there is boundary overlap between rising and falling */
	for (i = 0; i < scan_info->scan_info[0].fail_bd_cnt; i++)
		for (j = 0; j < scan_info->scan_info[1].fail_bd_cnt; j++)
			if (((scan_info->scan_info[0].fail_bd_info[i].Bound_Start
				>= scan_info->scan_info[1].fail_bd_info[j].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[i].Bound_Start
				<= scan_info->scan_info[1].fail_bd_info[j].Bound_End))
				|| ((scan_info->scan_info[0].fail_bd_info[i].Bound_End
				<= scan_info->scan_info[1].fail_bd_info[j].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[i].Bound_End
				>= scan_info->scan_info[1].fail_bd_info[j].Bound_Start))
				|| ((scan_info->scan_info[1].fail_bd_info[j].Bound_Start
				>= scan_info->scan_info[0].fail_bd_info[i].Bound_Start)
				&& (scan_info->scan_info[1].fail_bd_info[j].Bound_Start
				<= scan_info->scan_info[0].fail_bd_info[i].Bound_End)))
				bd_err |= BD_OVERLAP_ERR;
	/* check boundary width  */
	for (i = 0; i < scan_info->scan_info[0].fail_bd_cnt; i++)
		if ((scan_info->scan_info[0].fail_bd_info[i].Bound_End
			- scan_info->scan_info[0].fail_bd_info[i].Bound_Start + 1) >= AUTOK_BD_MAX_WIDTH)
			bd_err |= BD_WIDER_MAX_ERR;
	for (i = 0; i < scan_info->scan_info[1].fail_bd_cnt; i++)
		if ((scan_info->scan_info[1].fail_bd_info[i].Bound_End
			- scan_info->scan_info[1].fail_bd_info[i].Bound_Start + 1) >= AUTOK_BD_MAX_WIDTH)
			bd_err |= BD_WIDER_MAX_ERR;
	/* check full boundary width difference */
	for (i = 0; i < scan_info->scan_info[0].fail_bd_cnt; i++)
		for (j = 0; j < scan_info->scan_info[1].fail_bd_cnt; j++) {
			if (((scan_info->scan_info[0].fail_bd_info[i].Bound_Start == 0)
				|| (scan_info->scan_info[0].fail_bd_info[i].Bound_End == 63))
				&& ((scan_info->scan_info[0].fail_bd_info[i].Bound_End
				- scan_info->scan_info[0].fail_bd_info[i].Bound_Start + 1)
				>= (scan_info->scan_info[1].fail_bd_info[j].Bound_End
				- scan_info->scan_info[1].fail_bd_info[j].Bound_Start + 1 + AUTOK_BD_WIDTH_DIF)))
				bd_err |= BD_WIDER_ERR;
			if (((scan_info->scan_info[1].fail_bd_info[j].Bound_Start == 0)
				|| (scan_info->scan_info[1].fail_bd_info[j].Bound_End == 63))
				&& ((scan_info->scan_info[0].fail_bd_info[i].Bound_End
				- scan_info->scan_info[0].fail_bd_info[i].Bound_Start + 1 + AUTOK_BD_WIDTH_DIF)
				<= (scan_info->scan_info[1].fail_bd_info[j].Bound_End
				- scan_info->scan_info[1].fail_bd_info[j].Bound_Start + 1)))
				bd_err |= BD_WIDER_ERR;
			if (((scan_info->scan_info[1].fail_bd_info[j].Bound_Start != 0)
				&& (scan_info->scan_info[1].fail_bd_info[j].Bound_End != 63))
				&& ((scan_info->scan_info[0].fail_bd_info[i].Bound_Start != 0)
				&& (scan_info->scan_info[0].fail_bd_info[i].Bound_End != 63))
				&& (((scan_info->scan_info[0].fail_bd_info[i].Bound_End
				- scan_info->scan_info[0].fail_bd_info[i].Bound_Start + 1 + AUTOK_BD_WIDTH_DIF)
				<= (scan_info->scan_info[1].fail_bd_info[j].Bound_End
				- scan_info->scan_info[1].fail_bd_info[j].Bound_Start + 1))
				|| ((scan_info->scan_info[1].fail_bd_info[j].Bound_End
				- scan_info->scan_info[1].fail_bd_info[j].Bound_Start + 1 + AUTOK_BD_WIDTH_DIF)
				<= (scan_info->scan_info[0].fail_bd_info[i].Bound_End
				- scan_info->scan_info[0].fail_bd_info[i].Bound_Start + 1))))
				bd_err |= BD_WIDER_ERR;
		}
	/* check boundary count abnormal */
	if (((scan_info->scan_info[0].fail_bd_cnt + AUTOK_BD_CNT_DIF)
		<= scan_info->scan_info[1].fail_bd_cnt)
		|| ((scan_info->scan_info[1].fail_bd_cnt + AUTOK_BD_CNT_DIF)
		<= scan_info->scan_info[0].fail_bd_cnt))
		bd_err |= BD_CNT_DIF_ERR;
	/* check boundary count */
	if ((scan_info->scan_info[0].fail_bd_cnt > AUTOK_BD_CNT_MAX)
		|| (scan_info->scan_info[1].fail_bd_cnt > AUTOK_BD_CNT_MAX))
		bd_err |= BD_CNT_ERR;
	/* check pass window width and relation boundary location @ single edge */
	switch (scan_info->scan_info[0].fail_bd_cnt) {
	case 3:
		if (((scan_info->scan_info[0].fail_bd_info[1].Bound_Start
			- scan_info->scan_info[0].fail_bd_info[0].Bound_End - 1) < AUTOK_BD_PASS_MIN)
			|| ((scan_info->scan_info[0].fail_bd_info[2].Bound_Start
			- scan_info->scan_info[0].fail_bd_info[1].Bound_End - 1) < AUTOK_BD_PASS_MIN)
			|| ((scan_info->scan_info[0].fail_bd_info[1].Bound_Start
			- scan_info->scan_info[0].fail_bd_info[0].Bound_End - 1)
			> ((scan_info->scan_info[0].fail_bd_info[2].Bound_Start
			- scan_info->scan_info[0].fail_bd_info[1].Bound_End - 1)
			+ AUTOK_BD_PASS_MIN / 2))
			|| ((scan_info->scan_info[0].fail_bd_info[2].Bound_Start
			- scan_info->scan_info[0].fail_bd_info[1].Bound_End - 1)
			> ((scan_info->scan_info[0].fail_bd_info[1].Bound_Start
			- scan_info->scan_info[0].fail_bd_info[0].Bound_End - 1)
			+ AUTOK_BD_PASS_MIN / 2)))
			bd_err |= BD_PASS_WIN_NARROW_ERR;
		break;
	case 2:
		if ((scan_info->scan_info[0].fail_bd_info[1].Bound_Start
			- scan_info->scan_info[0].fail_bd_info[0].Bound_End - 1) < AUTOK_BD_PASS_MIN)
			bd_err |= BD_PASS_WIN_NARROW_ERR;
		break;
	default:
		break;
	}
	switch (scan_info->scan_info[1].fail_bd_cnt) {
	case 3:
		if (((scan_info->scan_info[1].fail_bd_info[1].Bound_Start
			- scan_info->scan_info[1].fail_bd_info[0].Bound_End - 1) < AUTOK_BD_PASS_MIN)
			|| ((scan_info->scan_info[1].fail_bd_info[2].Bound_Start
			- scan_info->scan_info[1].fail_bd_info[1].Bound_End - 1) < AUTOK_BD_PASS_MIN)
			|| ((scan_info->scan_info[1].fail_bd_info[1].Bound_Start
			- scan_info->scan_info[1].fail_bd_info[0].Bound_End - 1)
			> ((scan_info->scan_info[1].fail_bd_info[2].Bound_Start
			- scan_info->scan_info[1].fail_bd_info[1].Bound_End - 1)
			+ AUTOK_BD_PASS_MIN / 2))
			|| ((scan_info->scan_info[1].fail_bd_info[2].Bound_Start
			- scan_info->scan_info[1].fail_bd_info[1].Bound_End - 1)
			> ((scan_info->scan_info[1].fail_bd_info[1].Bound_Start
			- scan_info->scan_info[1].fail_bd_info[0].Bound_End - 1)
			+ AUTOK_BD_PASS_MIN / 2)))
			bd_err |= BD_PASS_WIN_NARROW_ERR;
		break;
	case 2:
		if ((scan_info->scan_info[1].fail_bd_info[1].Bound_Start
			- scan_info->scan_info[1].fail_bd_info[0].Bound_End - 1) < AUTOK_BD_PASS_MIN)
			bd_err |= BD_PASS_WIN_NARROW_ERR;
		break;
	default:
		break;
	}
	/* check boundary alternate */
	switch (scan_info->scan_info[0].fail_bd_cnt) {
	case 3:
		switch (scan_info->scan_info[1].fail_bd_cnt) {
		case 3:
			/*
			** ooxxooooxxooooxxoooo   ooooxxooooxxooooxxoooo
			** oooooxxooooxxooooxxo   oxxooooxxooooxxooooooo
			*/
			if (!(((scan_info->scan_info[0].fail_bd_info[0].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[0].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[0].Bound_End
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[1].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_End
				< scan_info->scan_info[1].fail_bd_info[2].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[2].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[2].Bound_End))
				|| ((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_End
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[0].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[2].Bound_End
				< scan_info->scan_info[1].fail_bd_info[2].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[2].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[1].Bound_End))))
				bd_err |= BD_ALTERNATE_ERR;
			break;
		case 2:
			/*
			** ooxxooooxxooooxxoooo
			** oooooxxooooxxooooooo
			*/
			if (!((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_End
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[0].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[2].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[1].Bound_End)))
				bd_err |= BD_ALTERNATE_ERR;
			break;
		default:
			break;
		}
		break;
	case 2:
		switch (scan_info->scan_info[1].fail_bd_cnt) {
		case 3:
			/*
			** oooooxxooooxxooooooo
			** ooxxooooxxooooxxoooo
			*/
			if (!((scan_info->scan_info[0].fail_bd_info[0].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[0].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[0].Bound_End
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[1].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_End
				< scan_info->scan_info[1].fail_bd_info[2].Bound_Start)))
				bd_err |= BD_ALTERNATE_ERR;
			break;
		case 2:
			/*
			** ooxxooooxxoooooooooo   ooooooooooxxooooxxoooo
			** oooooxxooooxxooooooo   oooooooxxooooxxooooooo
			*/
			if (!(((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_End
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[0].Bound_End))
				|| ((scan_info->scan_info[0].fail_bd_info[0].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[0].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[0].Bound_End
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[1].Bound_End))))
				bd_err |= BD_ALTERNATE_ERR;
			break;
		case 1:
			/*
			** oooooxxooooxxooooooo
			** ooooooooxxoooooooooo
			*/
			if (!((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				&& (scan_info->scan_info[0].fail_bd_info[1].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[0].Bound_End)))
				bd_err |= BD_ALTERNATE_ERR;
			break;
		default:
			break;
		}
		break;
	case 1:
		switch (scan_info->scan_info[1].fail_bd_cnt) {
		case 2:
			/*
			** ooooooooxxoooooooooo
			** oooooxxooooxxooooooo
			*/
			if (!((scan_info->scan_info[0].fail_bd_info[0].Bound_Start
				> scan_info->scan_info[1].fail_bd_info[0].Bound_End)
				&& (scan_info->scan_info[0].fail_bd_info[0].Bound_End
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)))
				bd_err |= BD_ALTERNATE_ERR;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	if (bd_err & BD_ALTERNATE_ERR)
		return bd_err;
	/* check relative boundary location abnormal, compare pass window */
	switch (scan_info->scan_info[0].fail_bd_cnt) {
	case 3:
		switch (scan_info->scan_info[1].fail_bd_cnt) {
		case 3:
			if (!((((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[2].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[2].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[2].Bound_Start))
				|| (((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[0].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[2].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[2].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[2].Bound_Start))))
				bd_err |= BD_RELA_LOCA_ERR;
			break;
		case 2:
			if (!(((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[2].Bound_Start)))
				bd_err |= BD_RELA_LOCA_ERR;
			break;
		default:
			break;
		}
		break;
	case 2:
		switch (scan_info->scan_info[1].fail_bd_cnt) {
		case 3:
			if (!(((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[0].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[2].Bound_Start)))
				bd_err |= BD_RELA_LOCA_ERR;
			break;
		case 2:
			if (!((((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start))
				|| (((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[0].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[1].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[1].Bound_Start))))
				bd_err |= BD_RELA_LOCA_ERR;
			break;
		case 1:
			if (!(((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				&& ((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[1].Bound_Start)))
				bd_err |= BD_RELA_LOCA_ERR;
			break;
		default:
			break;
		}
		break;
	case 1:
		switch (scan_info->scan_info[1].fail_bd_cnt) {
		case 2:
			if (!(((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[0].Bound_Start)
				&& ((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[1].Bound_Start)))
				bd_err |= BD_RELA_LOCA_ERR;
			break;
		case 1:
			if (!(((scan_info->scan_info[0].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[1].fail_bd_info[0].Bound_Start)
				|| ((scan_info->scan_info[1].fail_bd_info[0].Bound_End
				+ AUTOK_BD_PASS_MIN / 2)
				< scan_info->scan_info[0].fail_bd_info[0].Bound_Start)))
				bd_err |= BD_RELA_LOCA_ERR;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return bd_err;
}

static void autok_scan_abnormal_select(struct AUTOK_REF_INFO_NEW *pInfo, unsigned int bd_err)
{
	unsigned int max_pass_win;
	unsigned char max_pass_win_position;
	unsigned char max_pass_win_edge;
	unsigned char i;

	if ((bd_err & (BD_CNT_DIF_ERR | BD_ALTERNATE_ERR | BD_CNT_ERR))
		&& !(bd_err & (BD_OVERLAP_ERR | BD_WIDER_ERR | BD_WIDER_MAX_ERR | BD_PASS_WIN_NARROW_ERR))) {
		if (pInfo->scan_info[0].fail_bd_cnt == 0) {
			pInfo->opt_edge_sel = 0;
			pInfo->opt_dly_cnt =
				(pInfo->scan_info[1].fail_bd_info[0].Bound_Start
				+ pInfo->scan_info[1].fail_bd_info[0].Bound_End) / 2;
		} else if (pInfo->scan_info[1].fail_bd_cnt == 0) {
			pInfo->opt_edge_sel = 1;
			pInfo->opt_dly_cnt =
				(pInfo->scan_info[0].fail_bd_info[0].Bound_Start
				+ pInfo->scan_info[0].fail_bd_info[0].Bound_End) / 2;
		} else {
			if (pInfo->scan_info[0].fail_bd_info[0].Bound_Start
				> pInfo->scan_info[1].fail_bd_info[0].Bound_End) {
				pInfo->opt_edge_sel = 0;
				pInfo->opt_dly_cnt =
					(pInfo->scan_info[1].fail_bd_info[0].Bound_Start
					+ pInfo->scan_info[1].fail_bd_info[0].Bound_End) / 2;
			} else {
				pInfo->opt_edge_sel = 1;
				pInfo->opt_dly_cnt =
					(pInfo->scan_info[0].fail_bd_info[0].Bound_Start
					+ pInfo->scan_info[0].fail_bd_info[0].Bound_End) / 2;
			}
		}
	} else {
		max_pass_win_position = 0;
		if (pInfo->scan_info[0].pass_bd_cnt != 0) {
			max_pass_win_edge = 0;
			max_pass_win = pInfo->scan_info[0].pass_bd_info[0].Bound_End
				- pInfo->scan_info[0].pass_bd_info[0].Bound_Start;
		} else if (pInfo->scan_info[1].pass_bd_cnt != 0) {
			max_pass_win_edge = 1;
			max_pass_win = pInfo->scan_info[1].pass_bd_info[0].Bound_End
				- pInfo->scan_info[1].pass_bd_info[0].Bound_Start;
		} else {
			max_pass_win_edge = 0;
			max_pass_win = pInfo->scan_info[0].pass_bd_info[0].Bound_End
				- pInfo->scan_info[0].pass_bd_info[0].Bound_Start;
		}
		for (i = 0; i < pInfo->scan_info[0].pass_bd_cnt; i++)
			if ((pInfo->scan_info[0].pass_bd_info[i].Bound_End
				- pInfo->scan_info[0].pass_bd_info[i].Bound_Start) > max_pass_win) {
				max_pass_win = pInfo->scan_info[0].pass_bd_info[i].Bound_End
					- pInfo->scan_info[0].pass_bd_info[i].Bound_Start;
				max_pass_win_position = i;
				max_pass_win_edge = 0;
			}
		for (i = 0; i < pInfo->scan_info[1].pass_bd_cnt; i++)
			if ((pInfo->scan_info[1].pass_bd_info[i].Bound_End
				- pInfo->scan_info[1].pass_bd_info[i].Bound_Start) > max_pass_win) {
				max_pass_win = pInfo->scan_info[1].pass_bd_info[i].Bound_End
					- pInfo->scan_info[1].pass_bd_info[i].Bound_Start;
				max_pass_win_position = i;
				max_pass_win_edge = 1;
			}
		if (max_pass_win_edge == 1) {
			pInfo->opt_edge_sel = 1;
			pInfo->opt_dly_cnt =
				(pInfo->scan_info[1].pass_bd_info[max_pass_win_position].Bound_Start
				+ pInfo->scan_info[1].pass_bd_info[max_pass_win_position].Bound_End) / 2;
		} else {
			pInfo->opt_edge_sel = 0;
			pInfo->opt_dly_cnt =
				(pInfo->scan_info[0].pass_bd_info[max_pass_win_position].Bound_Start
				+ pInfo->scan_info[0].pass_bd_info[max_pass_win_position].Bound_End) / 2;
		}
	}
}

/*get the opt_edge_sel/opt_dly_cnt/cycle_cnt according the filtering algorithm*/
static int autok_pad_dly_sel_new(struct AUTOK_REF_INFO_NEW *pInfo)
{
	unsigned int edge_sel;
	unsigned int bd_err;
#if AUTOK_DEBUG_LOG
	unsigned char i;
#endif
	/* check boundary effective */
	bd_err = autok_check_scan_effective(pInfo);
#if AUTOK_DEBUG_LOG
	for (i = 0; i < pInfo->scan_info[0].fail_bd_cnt; i++) {
		UTIL_Printf("[AUTOK]edge 0 fail bd: S-%d E-%d\r\n",
			pInfo->scan_info[0].fail_bd_info[i].Bound_Start
			, pInfo->scan_info[0].fail_bd_info[i].Bound_End);
	}
	for (i = 0; i < pInfo->scan_info[1].fail_bd_cnt; i++) {
		UTIL_Printf("[AUTOK]edge 1 fail bd: S-%d E-%d\r\n",
			pInfo->scan_info[1].fail_bd_info[i].Bound_Start
			, pInfo->scan_info[1].fail_bd_info[i].Bound_End);
	}
#endif
	if (bd_err != BD_NO_ERR) {
#if AUTOK_DEBUG_LOG
		if (bd_err & BD_OVERLAP_ERR)
			UTIL_Printf("[AUTOK]Warning: boundary overlap err!\r\n");
		if (bd_err & BD_ALTERNATE_ERR)
			UTIL_Printf("[AUTOK]Warning: boundary alternate position err!\r\n");
		if (bd_err & BD_CNT_DIF_ERR)
			UTIL_Printf("[AUTOK]Warning: boundary cnt dif err vs fall and rise!\r\n");
		if (bd_err & BD_CNT_ERR)
			UTIL_Printf("[AUTOK]Warning: boundary max cnt >3 err!\r\n");
		if (bd_err & BD_PASS_WIN_NARROW_ERR)
			UTIL_Printf("[AUTOK]Warning: pass window too narrow <10 err!\r\n");
		if (bd_err & BD_RELA_LOCA_ERR)
			UTIL_Printf("[AUTOK]Warning: boundary relation dif <5 err!\r\n");
		if (bd_err & BD_WIDER_ERR)
			UTIL_Printf("[AUTOK]Warning: boundary width dif too larger >10 err!\r\n");
		if (bd_err & BD_WIDER_MAX_ERR)
			UTIL_Printf("[AUTOK]Warning: boundary max width too wider >15 err!\r\n");
		if (bd_err & BD_ALL_FAIL_ERR)
			UTIL_Printf("[AUTOK]Warning: boundary all fail err!\r\n");
#endif
		/* select the middle of max pass window */
		autok_scan_abnormal_select(pInfo, bd_err);
		return bd_err;
	}
	/* boundary normal */
	switch (pInfo->scan_info[0].fail_bd_cnt) {
	case 3:
	case 2:
	case 1:
		if (pInfo->scan_info[1].fail_bd_cnt == 0) {
			if ((pInfo->scan_info[0].fail_bd_info[0].Bound_End
				+ pInfo->scan_info[0].fail_bd_info[0].Bound_Start) / 2 < 31) {
				pInfo->cycle_cnt = (64 - (pInfo->scan_info[0].fail_bd_info[0].Bound_End
					+ pInfo->scan_info[0].fail_bd_info[0].Bound_Start) / 2) << 1;
			} else {
				pInfo->cycle_cnt = ((pInfo->scan_info[0].fail_bd_info[0].Bound_End
					+ pInfo->scan_info[0].fail_bd_info[0].Bound_Start) / 2) << 1;
			}
			pInfo->opt_edge_sel = 1;
			pInfo->opt_dly_cnt = (pInfo->scan_info[0].fail_bd_info[0].Bound_End
				+ pInfo->scan_info[0].fail_bd_info[0].Bound_Start) / 2;
		} else {
			if (pInfo->scan_info[0].fail_bd_info[0].Bound_Start
				< pInfo->scan_info[1].fail_bd_info[0].Bound_Start)
				edge_sel = 0;
			else
				edge_sel = 1;
			if (pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_Start == 0) {
				pInfo->cycle_cnt = (pInfo->scan_info[edge_sel ^ 0x1].fail_bd_info[0].Bound_End
					- pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_End) << 1;
			} else {
				pInfo->cycle_cnt = ((pInfo->scan_info[edge_sel ^ 0x1].fail_bd_info[0].Bound_End
					+ pInfo->scan_info[edge_sel ^ 0x1].fail_bd_info[0].Bound_Start) / 2
					- (pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_End
					+ pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_Start) / 2) << 1;
			}
			if (pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_Start == 0) {
				pInfo->opt_edge_sel = edge_sel ^ 0x1;
				pInfo->opt_dly_cnt = 0;
			} else {
				if ((pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_End
					+ pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_Start) / 2
					>= pInfo->cycle_cnt / 2) {
					pInfo->opt_edge_sel = edge_sel;
					pInfo->opt_dly_cnt = (pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_End
					+ pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_Start) / 2
					- pInfo->cycle_cnt / 2;
				} else {
					pInfo->opt_edge_sel = edge_sel ^ 0x1;
					pInfo->opt_dly_cnt = (pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_End
						+ pInfo->scan_info[edge_sel].fail_bd_info[0].Bound_Start) / 2;
				}
			}
		}
		break;
	case 0:
		if (pInfo->scan_info[1].fail_bd_cnt == 0) {
			pInfo->cycle_cnt = 128;
			pInfo->opt_edge_sel = 0;
			pInfo->opt_dly_cnt = 31;
		} else {
			if ((pInfo->scan_info[1].fail_bd_info[0].Bound_End
				+ pInfo->scan_info[1].fail_bd_info[0].Bound_Start) / 2 < 31) {
				pInfo->cycle_cnt = (64 - (pInfo->scan_info[1].fail_bd_info[0].Bound_End
					+ pInfo->scan_info[1].fail_bd_info[0].Bound_Start) / 2) << 1;
			} else {
				pInfo->cycle_cnt = ((pInfo->scan_info[1].fail_bd_info[0].Bound_End
					+ pInfo->scan_info[1].fail_bd_info[0].Bound_Start) / 2) << 1;
			}
			pInfo->opt_edge_sel = 0;
			pInfo->opt_dly_cnt = (pInfo->scan_info[1].fail_bd_info[0].Bound_End
				+ pInfo->scan_info[1].fail_bd_info[0].Bound_Start) / 2;
		}
		break;
	default:
		break;
	}
	#if AUTOK_DEBUG_LOG
	UTIL_Printf("[AUTOK]1T = %d\r\n", pInfo->cycle_cnt);
	#endif
	return 0;
}

static int autok_ds_dly_sel(struct AUTOK_SCAN_RES_NEW *pInfo, unsigned int *pDlySel)
{
	unsigned int ret = 0;
	int uDlySel = 0;
	unsigned int max_pass_win;
	unsigned char max_pass_win_position;
	unsigned char i;

	max_pass_win_position = 0;
	max_pass_win = pInfo->pass_bd_info[0].Bound_End
		- pInfo->pass_bd_info[0].Bound_Start;

	for (i = 0; i < pInfo->pass_bd_cnt; i++)
		if ((pInfo->pass_bd_info[i].Bound_End
			- pInfo->pass_bd_info[i].Bound_Start) > max_pass_win) {
			max_pass_win = pInfo->pass_bd_info[i].Bound_End
				- pInfo->pass_bd_info[i].Bound_Start;
			max_pass_win_position = i;
		}
	uDlySel =
		(pInfo->pass_bd_info[max_pass_win_position].Bound_Start
		+ pInfo->pass_bd_info[max_pass_win_position].Bound_End) / 2;
	*pDlySel = uDlySel;

	return ret;
}

/*************************************************************************
* FUNCTION
*  msdc_autok_adjust_param
*
* DESCRIPTION
*  This function for auto-K, adjust msdc parameter
*
* PARAMETERS
*    host: msdc host manipulator pointer
*    param: enum of msdc parameter
*    value: value of msdc parameter
*    rw: AUTOK_READ/AUTOK_WRITE
*
* RETURN VALUES
*    error code: 0 success,
*               -1 parameter input error
*               -2 read/write fail
*               -3 else error
*************************************************************************/
static int msdc_autok_adjust_param(struct msdc_host *host, enum AUTOK_PARAM param, unsigned int *value,
				   int rw)
{
	void __iomem *base = host->base;
	unsigned int *reg;
	unsigned int field = 0;

	switch (param) {
	case READ_DATA_SMPL_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for READ_DATA_SMPL_SEL is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}

		reg = (unsigned int *) MSDC_IOCON;
		field = (u32) (MSDC_IOCON_R_D_SMPL_SEL);
		break;
	case WRITE_DATA_SMPL_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for WRITE_DATA_SMPL_SEL is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}

		reg = (unsigned int *) MSDC_IOCON;
		field = (u32) (MSDC_IOCON_W_D_SMPL_SEL);
		break;
	case DATA_DLYLINE_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for DATA_DLYLINE_SEL is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}

		reg = (unsigned int *) MSDC_IOCON;
		field = (unsigned int) (MSDC_IOCON_DDLSEL);
		break;
	case MSDC_DAT_TUNE_SEL:	/* 0-Dat tune 1-CLk tune ; */
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for DATA_DLYLINE_SEL is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE0;
		field = (u32) (MSDC_PAD_TUNE0_RXDLYSEL);
		break;
	case MSDC_WCRC_ASYNC_FIFO_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for DATA_DLYLINE_SEL is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PATCH_BIT2;
		field = (u32) (MSDC_PB2_CFGCRCSTS);
		break;
	case MSDC_RESP_ASYNC_FIFO_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for DATA_DLYLINE_SEL is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PATCH_BIT2;
		field = (u32) (MSDC_PB2_CFGRESP);
		break;
	case CMD_EDGE:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for CMD_EDGE is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_IOCON;
		field = (unsigned int) (MSDC_IOCON_RSPL);
		break;
	case RDATA_EDGE:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for RDATA_EDGE is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_IOCON;
		field = (u32) (MSDC_IOCON_R_D_SMPL);
		break;
	case WDATA_EDGE:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for WDATA_EDGE is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_IOCON;
		field = (u32) (MSDC_IOCON_W_D_SMPL);
		break;
	case RD_FIFO_EDGE:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for RDATA_EDGE is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PATCH_BIT0;
		field = (u32) (MSDC_PB0_RD_DAT_SEL);
		break;
	case WD_FIFO_EDGE:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for WDATA_EDGE is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PATCH_BIT2;
		field = (u32) (MSDC_PB2_CFGCRCSTSEDGE);
		break;
	case CMD_RD_D_DLY1:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for CMD_RD_DLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE0;
		field = (u32) (MSDC_PAD_TUNE0_CMDRDLY);
		break;
	case CMD_RD_D_DLY1_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for CMD_RD_DLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE0;
		field = (u32) (MSDC_PAD_TUNE0_CMDRRDLYSEL);
		break;
	case CMD_RD_D_DLY2:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for CMD_RD_DLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE1;
		field = (u32) (MSDC_PAD_TUNE1_CMDRDLY2);
		break;
	case CMD_RD_D_DLY2_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for CMD_RD_DLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE1;
		field = (u32) (MSDC_PAD_TUNE1_CMDRRDLY2SEL);
		break;
	case DAT_RD_D_DLY1:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for DAT0_RD_DLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE0;
		field = (u32) (MSDC_PAD_TUNE0_DATRRDLY);
		break;
	case DAT_RD_D_DLY1_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for CMD_RD_DLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE0;
		field = (u32) (MSDC_PAD_TUNE0_DATRRDLYSEL);
		break;
	case DAT_RD_D_DLY2:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for DAT1_RD_DLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE1;
		field = (u32) (MSDC_PAD_TUNE1_DATRRDLY2);
		break;
	case DAT_RD_D_DLY2_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for CMD_RD_DLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE1;
		field = (u32) (MSDC_PAD_TUNE1_DATRRDLY2SEL);
		break;
	case INT_DAT_LATCH_CK:
		if ((rw == AUTOK_WRITE) && (*value > 7)) {
			pr_debug
			    ("[%s] Input value(%d) for INT_DAT_LATCH_CK is out of range, it should be [0~7]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PATCH_BIT0;
		field = (u32) (MSDC_PB0_INT_DAT_LATCH_CK_SEL);
		break;
	case CKGEN_MSDC_DLY_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for CKGEN_MSDC_DLY_SEL is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PATCH_BIT0;
		field = (u32) (MSDC_PB0_CKGEN_MSDC_DLY_SEL);
		break;
	case CMD_RSP_TA_CNTR:
		if ((rw == AUTOK_WRITE) && (*value > 7)) {
			pr_debug
			    ("[%s] Input value(%d) for CMD_RSP_TA_CNTR is out of range, it should be [0~7]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PATCH_BIT1;
		field = (u32) (MSDC_PB1_CMD_RSP_TA_CNTR);
		break;
	case WRDAT_CRCS_TA_CNTR:
		if ((rw == AUTOK_WRITE) && (*value > 7)) {
			pr_debug
			    ("[%s] Input value(%d) for WRDAT_CRCS_TA_CNTR is out of range, it should be [0~7]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PATCH_BIT1;
		field = (u32) (MSDC_PB1_WRDAT_CRCS_TA_CNTR);
		break;
	case PAD_CLK_TXDLY:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for PAD_CLK_TXDLY is out of range, it should be [0~31]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) MSDC_PAD_TUNE0;
		field = (u32) (MSDC_PAD_TUNE0_CLKTXDLY);
		break;
	case EMMC50_WDATA_MUX_EN:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for EMMC50_WDATA_MUX_EN is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) EMMC50_CFG0;
		field = (u32) (MSDC_EMMC50_CFG_CRC_STS_SEL);
		break;
	case EMMC50_CMD_MUX_EN:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for EMMC50_CMD_MUX_EN is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) EMMC50_CFG0;
		field = (u32) (MSDC_EMMC50_CFG_CMD_RESP_SEL);
		break;
	case EMMC50_WDATA_EDGE:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for EMMC50_WDATA_EDGE is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) EMMC50_CFG0;
		field = (u32) (MSDC_EMMC50_CFG_CRC_STS_EDGE);
		break;
	case EMMC50_DS_Z_DLY1:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for EMMC50_DS_Z_DLY1 is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) EMMC50_PAD_DS_TUNE;
		field = (u32) (MSDC_EMMC50_PAD_DS_TUNE_DLY1);
		break;
	case EMMC50_DS_Z_DLY1_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for EMMC50_DS_Z_DLY1_SEL is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) EMMC50_PAD_DS_TUNE;
		field = (u32) (MSDC_EMMC50_PAD_DS_TUNE_DLYSEL);
		break;
	case EMMC50_DS_Z_DLY2:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for EMMC50_DS_Z_DLY2 is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) EMMC50_PAD_DS_TUNE;
		field = (u32) (MSDC_EMMC50_PAD_DS_TUNE_DLY2);
		break;
	case EMMC50_DS_Z_DLY2_SEL:
		if ((rw == AUTOK_WRITE) && (*value > 1)) {
			pr_debug
			    ("[%s] Input value(%d) for EMMC50_DS_Z_DLY1_SEL is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) EMMC50_PAD_DS_TUNE;
		field = (u32) (MSDC_EMMC50_PAD_DS_TUNE_DLY2SEL);
		break;
	case EMMC50_DS_ZDLY_DLY:
		if ((rw == AUTOK_WRITE) && (*value > 31)) {
			pr_debug
			    ("[%s] Input value(%d) for EMMC50_DS_Z_DLY2 is out of range, it should be [0~1]\n",
			     __func__, *value);
			return -1;
		}
		reg = (unsigned int *) EMMC50_PAD_DS_TUNE;
		field = (u32) (MSDC_EMMC50_PAD_DS_TUNE_DLY3);
		break;
	default:
		pr_debug("[%s] Value of [enum AUTOK_PARAM param] is wrong\n", __func__);
		return -1;
	}

	if (rw == AUTOK_READ)
		MSDC_GET_FIELD(reg, field, *value);
	else if (rw == AUTOK_WRITE) {
		MSDC_SET_FIELD(reg, field, *value);
	} else {
		pr_debug("[%s] Value of [int rw] is wrong\n", __func__);
		return -1;
	}

	return 0;
}

static int autok_param_update(enum AUTOK_PARAM param_id, unsigned int result, unsigned char *autok_tune_res)
{
	if (param_id < TUNING_PARAM_COUNT) {
		if ((result > autok_param_info[param_id].range.end) ||
		    (result < autok_param_info[param_id].range.start)) {
			UTIL_Printf("[AUTOK]param outof range : %d not in [%d,%d]\r\n",
				       result, autok_param_info[param_id].range.start,
				       autok_param_info[param_id].range.end);
			return -1;
		}
		autok_tune_res[param_id] = (unsigned char) result;
		return 0;
	}
	UTIL_Printf("[AUTOK]param not found\r\n");

	return -1;
}

static int autok_param_apply(struct msdc_host *host, unsigned char *autok_tune_res)
{
	unsigned int i = 0;
	unsigned int value = 0;

	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		value = (unsigned char) autok_tune_res[i];
		msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
	}

	return 0;
}

static int autok_result_dump(struct msdc_host *host, unsigned char *autok_tune_res)
{

	UTIL_Printf("[AUTOK]CMD [EDGE:%d DLY1:%d DLY2:%d]\r\n",
		autok_tune_res[0], autok_tune_res[5], autok_tune_res[7]);
	UTIL_Printf("[AUTOK]DAT [RDAT_EDGE:%d WDAT_EDGE:%d RD_FIFO_EDGE:%d WD_FIFO_EDGE:%d]\r\n",
		autok_tune_res[1], autok_tune_res[2], autok_tune_res[3], autok_tune_res[4]);
	UTIL_Printf("[AUTOK]DAT [LATCH_CK:%d DLY1:%d DLY2:%d]\r\n",
		autok_tune_res[13], autok_tune_res[9], autok_tune_res[11]);
	UTIL_Printf("[AUTOK]DS  [DLY1:%d DLY2:%d DLY3:%d]\r\n",
		autok_tune_res[14], autok_tune_res[16], autok_tune_res[18]);

	return 0;
}

#if AUTOK_PARAM_DUMP_ENABLE
static int autok_register_dump(struct msdc_host *host)
{
	unsigned int i = 0;
	unsigned int value = 0;
	unsigned char autok_tune_res[TUNING_PARAM_COUNT];

	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		msdc_autok_adjust_param(host, i, &value, AUTOK_READ);
		autok_tune_res[i] = value;
	}
	UTIL_Printf("[AUTOK]CMD [EDGE:%d DLY1:%d DLY2:%d]\r\n",
		autok_tune_res[0], autok_tune_res[5], autok_tune_res[7]);
	UTIL_Printf("[AUTOK]DAT [RDAT_EDGE:%d WDAT_EDGE:%d RD_FIFO_EDGE:%d WD_FIFO_EDGE:%d]\r\n",
		autok_tune_res[1], autok_tune_res[2], autok_tune_res[3], autok_tune_res[4]);
	UTIL_Printf("[AUTOK]DAT [LATCH_CK:%d DLY1:%d DLY2:%d]\r\n",
		autok_tune_res[13], autok_tune_res[9], autok_tune_res[11]);
	UTIL_Printf("[AUTOK]DS  [DLY1:%d DLY2:%d DLY3:%d]\r\n",
		autok_tune_res[14], autok_tune_res[16], autok_tune_res[18]);

	return 0;
}
#endif

void autok_tuning_parameter_init(struct msdc_host *host, unsigned char *res)
{
	unsigned int ret = 0;
	/* void __iomem *base = host->base; */

	/* MSDC_SET_FIELD(MSDC_PATCH_BIT2, 7<<29, 2); */
	/* MSDC_SET_FIELD(MSDC_PATCH_BIT2, 7<<16, 4); */

	ret = autok_param_apply(host, res);
}

/*******************************************************
* Function: msdc_autok_adjust_paddly                   *
* Param : value - delay cnt from 0 to 63               *
*         pad_sel - 0 for cmd pad and 1 for data pad   *
*******************************************************/
#define CMD_PAD_RDLY 0
#define DAT_PAD_RDLY 1
#define DS_PAD_RDLY 2
static void msdc_autok_adjust_paddly(struct msdc_host *host, unsigned int *value,
				     unsigned int pad_sel)
{
	unsigned int uCfgL = 0;
	unsigned int uCfgLSel = 0;
	unsigned int uCfgH = 0;
	unsigned int uCfgHSel = 0;
	unsigned int dly_cnt = *value;

	uCfgL = (dly_cnt > 31) ? (31) : dly_cnt;
	uCfgH = (dly_cnt > 31) ? (dly_cnt - 32) : 0;

	uCfgLSel = (uCfgL > 0) ? 1 : 0;
	uCfgHSel = (uCfgH > 0) ? 1 : 0;
	switch (pad_sel) {
	case CMD_PAD_RDLY:
		msdc_autok_adjust_param(host, CMD_RD_D_DLY1, &uCfgL, AUTOK_WRITE);
		msdc_autok_adjust_param(host, CMD_RD_D_DLY2, &uCfgH, AUTOK_WRITE);

		msdc_autok_adjust_param(host, CMD_RD_D_DLY1_SEL, &uCfgLSel, AUTOK_WRITE);
		msdc_autok_adjust_param(host, CMD_RD_D_DLY2_SEL, &uCfgHSel, AUTOK_WRITE);
		break;
	case DAT_PAD_RDLY:
		msdc_autok_adjust_param(host, DAT_RD_D_DLY1, &uCfgL, AUTOK_WRITE);
		msdc_autok_adjust_param(host, DAT_RD_D_DLY2, &uCfgH, AUTOK_WRITE);

		msdc_autok_adjust_param(host, DAT_RD_D_DLY1_SEL, &uCfgLSel, AUTOK_WRITE);
		msdc_autok_adjust_param(host, DAT_RD_D_DLY2_SEL, &uCfgHSel, AUTOK_WRITE);
		break;
	case DS_PAD_RDLY:
		msdc_autok_adjust_param(host, EMMC50_DS_Z_DLY1, &uCfgL, AUTOK_WRITE);
		msdc_autok_adjust_param(host, EMMC50_DS_Z_DLY2, &uCfgH, AUTOK_WRITE);

		msdc_autok_adjust_param(host, EMMC50_DS_Z_DLY1_SEL, &uCfgLSel, AUTOK_WRITE);
		msdc_autok_adjust_param(host, EMMC50_DS_Z_DLY2_SEL, &uCfgHSel, AUTOK_WRITE);
		break;
	}
}

static void autok_paddly_update(unsigned int pad_sel, unsigned int dly_cnt, unsigned char *autok_tune_res)
{
	unsigned int uCfgL = 0;
	unsigned int uCfgLSel = 0;
	unsigned int uCfgH = 0;
	unsigned int uCfgHSel = 0;

	uCfgL = (dly_cnt > 31) ? (31) : dly_cnt;
	uCfgH = (dly_cnt > 31) ? (dly_cnt - 32) : 0;

	uCfgLSel = (uCfgL > 0) ? 1 : 0;
	uCfgHSel = (uCfgH > 0) ? 1 : 0;
	switch (pad_sel) {
	case CMD_PAD_RDLY:
		autok_param_update(CMD_RD_D_DLY1, uCfgL, autok_tune_res);
		autok_param_update(CMD_RD_D_DLY2, uCfgH, autok_tune_res);

		autok_param_update(CMD_RD_D_DLY1_SEL, uCfgLSel, autok_tune_res);
		autok_param_update(CMD_RD_D_DLY2_SEL, uCfgHSel, autok_tune_res);
		break;
	case DAT_PAD_RDLY:
		autok_param_update(DAT_RD_D_DLY1, uCfgL, autok_tune_res);
		autok_param_update(DAT_RD_D_DLY2, uCfgH, autok_tune_res);

		autok_param_update(DAT_RD_D_DLY1_SEL, uCfgLSel, autok_tune_res);
		autok_param_update(DAT_RD_D_DLY2_SEL, uCfgHSel, autok_tune_res);
		break;
	case DS_PAD_RDLY:
		autok_param_update(EMMC50_DS_Z_DLY1, uCfgL, autok_tune_res);
		autok_param_update(EMMC50_DS_Z_DLY2, uCfgH, autok_tune_res);

		autok_param_update(EMMC50_DS_Z_DLY1_SEL, uCfgLSel, autok_tune_res);
		autok_param_update(EMMC50_DS_Z_DLY2_SEL, uCfgHSel, autok_tune_res);
		break;
	}
}

static void msdc_autok_window_apply(enum AUTOK_SCAN_WIN scan_win, u64 sacn_window, unsigned char *autok_tune_res)
{
	switch (scan_win) {
	case CMD_RISE:
		autok_tune_res[CMD_SCAN_R0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[CMD_SCAN_R1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[CMD_SCAN_R2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[CMD_SCAN_R3] = (sacn_window >> 24) & 0xff;
		autok_tune_res[CMD_SCAN_R4] = (sacn_window >> 32) & 0xff;
		autok_tune_res[CMD_SCAN_R5] = (sacn_window >> 40) & 0xff;
		autok_tune_res[CMD_SCAN_R6] = (sacn_window >> 48) & 0xff;
		autok_tune_res[CMD_SCAN_R7] = (sacn_window >> 56) & 0xff;
		break;
	case CMD_FALL:
		autok_tune_res[CMD_SCAN_F0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[CMD_SCAN_F1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[CMD_SCAN_F2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[CMD_SCAN_F3] = (sacn_window >> 24) & 0xff;
		autok_tune_res[CMD_SCAN_F4] = (sacn_window >> 32) & 0xff;
		autok_tune_res[CMD_SCAN_F5] = (sacn_window >> 40) & 0xff;
		autok_tune_res[CMD_SCAN_F6] = (sacn_window >> 48) & 0xff;
		autok_tune_res[CMD_SCAN_F7] = (sacn_window >> 56) & 0xff;
		break;
	case DAT_RISE:
		autok_tune_res[DAT_SCAN_R0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[DAT_SCAN_R1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[DAT_SCAN_R2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[DAT_SCAN_R3] = (sacn_window >> 24) & 0xff;
		autok_tune_res[DAT_SCAN_R4] = (sacn_window >> 32) & 0xff;
		autok_tune_res[DAT_SCAN_R5] = (sacn_window >> 40) & 0xff;
		autok_tune_res[DAT_SCAN_R6] = (sacn_window >> 48) & 0xff;
		autok_tune_res[DAT_SCAN_R7] = (sacn_window >> 56) & 0xff;
		break;
	case DAT_FALL:
		autok_tune_res[DAT_SCAN_F0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[DAT_SCAN_F1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[DAT_SCAN_F2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[DAT_SCAN_F3] = (sacn_window >> 24) & 0xff;
		autok_tune_res[DAT_SCAN_F4] = (sacn_window >> 32) & 0xff;
		autok_tune_res[DAT_SCAN_F5] = (sacn_window >> 40) & 0xff;
		autok_tune_res[DAT_SCAN_F6] = (sacn_window >> 48) & 0xff;
		autok_tune_res[DAT_SCAN_F7] = (sacn_window >> 56) & 0xff;
		break;
	case DS_WIN:
		autok_tune_res[DS_SCAN_0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[DS_SCAN_1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[DS_SCAN_2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[DS_SCAN_3] = (sacn_window >> 24) & 0xff;
		autok_tune_res[DS_SCAN_4] = (sacn_window >> 32) & 0xff;
		autok_tune_res[DS_SCAN_5] = (sacn_window >> 40) & 0xff;
		autok_tune_res[DS_SCAN_6] = (sacn_window >> 48) & 0xff;
		autok_tune_res[DS_SCAN_7] = (sacn_window >> 56) & 0xff;
		break;
	case D_CMD_RX:
		autok_tune_res[D_CMD_SCAN_0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[D_CMD_SCAN_1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[D_CMD_SCAN_2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[D_CMD_SCAN_3] = (sacn_window >> 24) & 0xff;
		break;
	case D_DATA_RX:
		autok_tune_res[D_DATA_SCAN_0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[D_DATA_SCAN_1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[D_DATA_SCAN_2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[D_DATA_SCAN_3] = (sacn_window >> 24) & 0xff;
		break;
	case H_CMD_TX:
		autok_tune_res[H_CMD_SCAN_0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[H_CMD_SCAN_1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[H_CMD_SCAN_2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[H_CMD_SCAN_3] = (sacn_window >> 24) & 0xff;
		break;
	case H_DATA_TX:
		autok_tune_res[H_DATA_SCAN_0] = (sacn_window >> 0) & 0xff;
		autok_tune_res[H_DATA_SCAN_1] = (sacn_window >> 8) & 0xff;
		autok_tune_res[H_DATA_SCAN_2] = (sacn_window >> 16) & 0xff;
		autok_tune_res[H_DATA_SCAN_3] = (sacn_window >> 24) & 0xff;
		break;
	}
}

static void msdc_autok_version_apply(unsigned char *autok_tune_res)
{
	autok_tune_res[AUTOK_VER0] = (AUTOK_VERSION >> 0) & 0xff;
	autok_tune_res[AUTOK_VER1] = (AUTOK_VERSION >> 8) & 0xff;
	autok_tune_res[AUTOK_VER2] = (AUTOK_VERSION >> 16) & 0xff;
	autok_tune_res[AUTOK_VER3] = (AUTOK_VERSION >> 24) & 0xff;
}

/*******************************************************
* Exectue tuning IF Implenment                         *
*******************************************************/
static int autok_write_param(struct msdc_host *host, enum AUTOK_PARAM param, unsigned int value)
{
	msdc_autok_adjust_param(host, param, &value, AUTOK_WRITE);

	return 0;
}

int autok_path_sel(struct msdc_host *host)
{
	void __iomem *base = host->base;

	autok_write_param(host, READ_DATA_SMPL_SEL, 0);
	autok_write_param(host, WRITE_DATA_SMPL_SEL, 0);

	/* clK tune all data Line share dly */
	autok_write_param(host, DATA_DLYLINE_SEL, 0);
	/* data tune mode select */
#if defined(CHIP_DENALI_3_DAT_TUNE)
	autok_write_param(host, MSDC_DAT_TUNE_SEL, 1);
#else
	autok_write_param(host, MSDC_DAT_TUNE_SEL, 0);
#endif
	autok_write_param(host, MSDC_DAT_TUNE_SEL, 0);	

	autok_write_param(host, MSDC_WCRC_ASYNC_FIFO_SEL, 1);
	autok_write_param(host, MSDC_RESP_ASYNC_FIFO_SEL, 0);
	
	/* eMMC50 Function Mux */
	/* write path switch to emmc45 */
	autok_write_param(host, EMMC50_WDATA_MUX_EN, 0);

	/* response path switch to emmc45 */
	autok_write_param(host, EMMC50_CMD_MUX_EN, 0);
	autok_write_param(host, EMMC50_WDATA_EDGE, 0);

	/* Common Setting Config */
	autok_write_param(host, CKGEN_MSDC_DLY_SEL, AUTOK_CKGEN_VALUE);
	autok_write_param(host, CMD_RSP_TA_CNTR, AUTOK_CMD_TA_VALUE);
	autok_write_param(host, WRDAT_CRCS_TA_CNTR, AUTOK_CRC_TA_VALUE);

	MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PB1_GET_BUSY_MA, AUTOK_BUSY_MA_VALUE);

	/* LATCH_TA_EN Config for WCRC Path HS FS mode */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CRCSTSENSEL, AUTOK_CRC_LATCH_EN_HS_VALUE);
	/* LATCH_TA_EN Config for CMD Path HS FS mode */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_RESPSTENSEL, AUTOK_CMD_LATCH_EN_HS_VALUE);

	/* DDR50 byte swap issue design fix feature enable */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, 1 << 19, 1);
	return 0;
}
EXPORT_SYMBOL(autok_path_sel);

int autok_init_sdr104(struct msdc_host *host)
{
	void __iomem *base = host->base;

	/* driver may miss data tune path setting in the interim */
	autok_path_sel(host);

	/* if any specific config need modify add here */
	/* LATCH_TA_EN Config for WCRC Path SDR104 mode */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CRCSTSENSEL, AUTOK_CRC_LATCH_EN_SDR104_PORT1_VALUE);
	/* LATCH_TA_EN Config for CMD Path SDR104 mode */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_RESPSTENSEL, AUTOK_CMD_LATCH_EN_SDR104_PORT1_VALUE);
	/* enable dvfs feature */
	/* if (host->hw->host_function == MSDC_SDIO) */
	/*	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_DVFS_EN, 1); */
#if !NEW_PATH
	MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PB1_BIAS_TUNE_28NM, 3);
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_POPENCNT, 8);
	MSDC_SET_FIELD(SDC_FIFO_CFG, WR_VALID_SEL, 1);
	MSDC_SET_FIELD(SDC_FIFO_CFG, RD_VALID_SEL, 1);
#else
	MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PB1_BIAS_TUNE_28NM, 6);
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_POPENCNT, 0);
	MSDC_SET_FIELD(SDC_FIFO_CFG, WR_VALID_SEL, 0);
	MSDC_SET_FIELD(SDC_FIFO_CFG, RD_VALID_SEL, 0);
#endif
	autok_write_param(host, CKGEN_MSDC_DLY_SEL, ckgen[host->id]);

	return 0;
}

int autok_init_ddr(struct msdc_host *host)
{
	unsigned long base = host->base;

	/* driver may miss data tune path setting in the interim */
	autok_path_sel(host);

	/* if any specific config need modify add here */
	/* LATCH_TA_EN Config for WCRC Path non_HS400 */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CRCSTSENSEL, AUTOK_CRC_LATCH_EN_HS_VALUE);
	/* LATCH_TA_EN Config for CMD Path non_HS400 */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_RESPSTENSEL, AUTOK_CMD_LATCH_EN_HS_VALUE);
#if !NEW_PATH
	MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PB1_BIAS_TUNE_28NM, 3);
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_POPENCNT, 8);
	MSDC_SET_FIELD(SDC_FIFO_CFG, WR_VALID_SEL, 1);
	MSDC_SET_FIELD(SDC_FIFO_CFG, RD_VALID_SEL, 1);
#else
	/* DDR mode water can not set to 0 */
	//MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_STOP_DLY_SEL, 6);
	//MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_POP_EN_CNT, 0);
	MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PB1_BIAS_TUNE_28NM, 3);
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_POPENCNT, 8);
	MSDC_SET_FIELD(SDC_FIFO_CFG, WR_VALID_SEL, 0);
	MSDC_SET_FIELD(SDC_FIFO_CFG, RD_VALID_SEL, 0);
#endif

#ifdef USE_INTERNAL_DLY
	autok_write_param(host, MSDC_DAT_TUNE_SEL, 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 0);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_DAT_RD_RXDLY, 0);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_CMD_RESP_RXDLY, 10);
	//MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_DAT_WR_RXDLY, 5);
	autok_write_param(host, MSDC_WCRC_ASYNC_FIFO_SEL, 0);
	autok_write_param(host, MSDC_RESP_ASYNC_FIFO_SEL, 1);
#endif
	return 0;
}

int autok_init_hs200(struct msdc_host *host)
{
	void __iomem *base = host->base;

	/* driver may miss data tune path setting in the interim */
	autok_path_sel(host);

	/* if any specific config need modify add here */
	/* LATCH_TA_EN Config for WCRC Path non_HS200 */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CRCSTSENSEL, AUTOK_CRC_LATCH_EN_HS200_PORT0_VALUE);
	/* LATCH_TA_EN Config for CMD Path non_HS200 */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_RESPSTENSEL, AUTOK_CMD_LATCH_EN_HS200_PORT0_VALUE);
#if !NEW_PATH
	MSDC_SET_FIELD(MSDC_PATCH_BIT1,  MSDC_PB1_BIAS_TUNE_28NM, 3);
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_POPENCNT, 8);
	MSDC_SET_FIELD(SDC_FIFO_CFG, WR_VALID_SEL, 1);
	MSDC_SET_FIELD(SDC_FIFO_CFG, RD_VALID_SEL, 1);
#else
	MSDC_SET_FIELD(MSDC_PATCH_BIT1,  MSDC_PB1_BIAS_TUNE_28NM, 6);
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_POPENCNT, 0);
	MSDC_SET_FIELD(SDC_FIFO_CFG, WR_VALID_SEL, 0);
	MSDC_SET_FIELD(SDC_FIFO_CFG, RD_VALID_SEL, 0);
#endif

#ifdef USE_INTERNAL_DLY
		autok_write_param(host, MSDC_DAT_TUNE_SEL, 1);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_R_D_SMPL, 0);
		MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_DAT_RD_RXDLY, 0);
		MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_CMD_RESP_RXDLY, 10);
		//MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_DAT_WR_RXDLY, 5);
		autok_write_param(host, MSDC_WCRC_ASYNC_FIFO_SEL, 0);
		autok_write_param(host, MSDC_RESP_ASYNC_FIFO_SEL, 1);
#endif
	autok_write_param(host, CKGEN_MSDC_DLY_SEL, ckgen[host->id]);
	return 0;
}

int autok_init_hs400(struct msdc_host *host)
{
	void __iomem *base = host->base;
	/* driver may miss data tune path setting in the interim */
	autok_path_sel(host);

	/* if any specific config need modify add here */
	/* LATCH_TA_EN Config for WCRC Path HS400 */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CRCSTSENSEL, AUTOK_CRC_LATCH_EN_HS400_PORT0_VALUE);
	/* LATCH_TA_EN Config for CMD Path HS400 */
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_RESPSTENSEL, AUTOK_CMD_LATCH_EN_HS400_PORT0_VALUE);
	/* write path switch to emmc50 */
	autok_write_param(host, EMMC50_WDATA_MUX_EN, 1);
	/* Specifical for HS400 Path Sel */
	autok_write_param(host, MSDC_WCRC_ASYNC_FIFO_SEL, 0);

#ifdef USE_INTERNAL_DLY
	autok_write_param(host, MSDC_DAT_TUNE_SEL, 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_R_D_SMPL, 0);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_DAT_RD_RXDLY, 0);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_CMD_RESP_RXDLY, 10);
	//MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_DAT_WR_RXDLY, 5);
	autok_write_param(host, MSDC_WCRC_ASYNC_FIFO_SEL, 0);
	autok_write_param(host, MSDC_RESP_ASYNC_FIFO_SEL, 1);

	autok_write_param(host, EMMC50_WDATA_MUX_EN, 1);
	autok_write_param(host, EMMC50_DS_Z_DLY1, 2);
	autok_write_param(host, EMMC50_DS_Z_DLY1_SEL, 1);
	autok_write_param(host, EMMC50_DS_Z_DLY2, 0);
	autok_write_param(host, EMMC50_DS_Z_DLY2_SEL, 0);
	autok_write_param(host, EMMC50_DS_ZDLY_DLY, 20);
#endif
	return 0;
}

int execute_online_tuning_hs400(struct msdc_host *host, unsigned char *res)
{
     void __iomem *base = host->base;
	unsigned int ret = 0;
	unsigned int response;
	unsigned int uCmdEdge = 0;
	unsigned long RawData64 = 0LL;
	unsigned int score = 0;
	unsigned int j, k;
	struct AUTOK_REF_INFO_NEW uCmdDatInfo;
	struct AUTOK_SCAN_RES_NEW *pBdInfo;
	char tune_result_str64[65];
	unsigned char p_autok_tune_res[TUNING_PARA_SCAN_COUNT];
	unsigned int opcode = MMC_SEND_STATUS;
	unsigned int uDatDly = 0;

	autok_init_hs400(host);
	memset((void *)p_autok_tune_res, 0, sizeof(p_autok_tune_res) / sizeof(unsigned char));
#if 0
	/* Step1 : Get Cmd setting from HS200 tuning result */
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL,
	p_autok_tune_res[CMD_EDGE]);
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CMDRDLY,
		p_autok_tune_res[CMD_RD_D_DLY1]);
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CMDRRDLYSEL,
		p_autok_tune_res[CMD_RD_D_DLY1_SEL]);
	MSDC_GET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_CMDRDLY2,
		p_autok_tune_res[CMD_RD_D_DLY2]);
	MSDC_GET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_CMDRRDLY2SEL,
		p_autok_tune_res[CMD_RD_D_DLY2_SEL]);
#endif
#if USE_HS200_DATA_RES
	/* Step1 : Get Cmd setting from HS200 data tuning result */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_PB0_RD_DAT_SEL,
	p_autok_tune_res[CMD_EDGE]);
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_DATRRDLY,
		p_autok_tune_res[CMD_RD_D_DLY1]);
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_DATRRDLYSEL,
		p_autok_tune_res[CMD_RD_D_DLY1_SEL]);
	MSDC_GET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_DATRRDLY2,
		p_autok_tune_res[CMD_RD_D_DLY2]);
	MSDC_GET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_DATRRDLY2SEL,
		p_autok_tune_res[CMD_RD_D_DLY2_SEL]);
#endif
	/* Step1 : Tuning Cmd Path */
	autok_tuning_parameter_init(host, p_autok_tune_res);
	memset(&uCmdDatInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));

	uCmdEdge = 0;
	do {
		pBdInfo = (struct AUTOK_SCAN_RES_NEW *)&(uCmdDatInfo.scan_info[uCmdEdge]);
		msdc_autok_adjust_param(host, CMD_EDGE, &uCmdEdge, AUTOK_WRITE);
		RawData64 = 0LL;
		for (j = 0; j < 64; j++) {
			msdc_autok_adjust_paddly(host, &j, CMD_PAD_RDLY);
			for (k = 0; k < AUTOK_CMD_TIMES / 2; k++) {
				ret = autok_send_tune_cmd(host, opcode, TUNE_CMD);
				if ((ret & (E_RESULT_CMD_TMO | E_RESULT_RSP_CRC)) != 0) {
					RawData64 |= (unsigned long)(1LL << j);
					break;
				} else if ((ret & E_RESULT_FATAL_ERR) != 0)
					return -1;
			}
		}
		score = autok_simple_score64(tune_result_str64, RawData64);
		UTIL_Printf("[AUTOK]CMD %d \t %d \t %s\r\n", uCmdEdge, score, tune_result_str64);
		if (uCmdEdge)
			msdc_autok_window_apply(CMD_FALL, RawData64, p_autok_tune_res);
		else
			msdc_autok_window_apply(CMD_RISE, RawData64, p_autok_tune_res);
		if (autok_check_scan_res64_new(RawData64, pBdInfo, 1) != 0)
			return -1;

		uCmdEdge ^= 0x1;
	} while (uCmdEdge);
#if !(USE_HS200_DATA_RES)
	if (autok_pad_dly_sel_new(&uCmdDatInfo) != 0) {
		UTIL_Printf("[AUTOK][Error]=============CMD Analysis Failed!!=======================\r\n");
	}
	autok_param_update(CMD_EDGE, uCmdDatInfo.opt_edge_sel, p_autok_tune_res);
	autok_paddly_update(CMD_PAD_RDLY, uCmdDatInfo.opt_dly_cnt, p_autok_tune_res);
#endif
	/* DLY3 keep default value 20 */
	p_autok_tune_res[EMMC50_DS_ZDLY_DLY] = 20;
	/* Step2 : Tuning DS Clk Path-ZCLK only tune DLY1 */
#ifdef CMDQ
	opcode = MMC_SEND_EXT_CSD; /* can also use MMC_READ_SINGLE_BLOCK */
#else
	opcode = MMC_READ_SINGLE_BLOCK;
#endif
	autok_tuning_parameter_init(host, p_autok_tune_res);
	/* check device status */
	ret = autok_send_tune_cmd(host, 13, TUNE_CMD);
	if (ret == E_RESULT_PASS) {
		response = MSDC_READ32(SDC_RESP0);
		#if AUTOK_DEBUG_LOG
		UTIL_Printf("[AUTOK]current device status 0x%08x\r\n", response);
		#endif
	} else
		UTIL_Printf("[AUTOK]CMD error while check device status\r\n");
	/* tune data pad delay , find data pad boundary */
	for (j = 0; j < 32; j++) {
		msdc_autok_adjust_paddly(host, &j, DAT_PAD_RDLY);
		for (k = 0; k < AUTOK_CMD_TIMES / 4; k++) {
			ret = autok_send_tune_cmd(host, opcode, TUNE_DATA);
			if ((ret & (E_RESULT_CMD_TMO | E_RESULT_RSP_CRC)) != 0) {
				UTIL_Printf("[AUTOK]Error Autok CMD Failed while tune DATA PAD Delay\r\n");
				return -1;
			} else if ((ret & (E_RESULT_DAT_CRC | E_RESULT_DAT_TMO)) != 0)
				break;
			else if ((ret & E_RESULT_FATAL_ERR) != 0)
				return -1;
		}
		if ((ret & (E_RESULT_DAT_CRC | E_RESULT_DAT_TMO)) != 0) {
			p_autok_tune_res[DAT_RD_D_DLY1] = j;
			if (j)
				p_autok_tune_res[DAT_RD_D_DLY1_SEL] = 1;
			break;
		}
	}
	autok_tuning_parameter_init(host, p_autok_tune_res);
	memset(&uCmdDatInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));
	pBdInfo = (struct AUTOK_SCAN_RES_NEW *)&(uCmdDatInfo.scan_info[0]);
	RawData64 = 0LL;
	/* tune DS delay , base on data pad boundary */
	for (j = 0; j < 32; j++) {
		msdc_autok_adjust_paddly(host, &j, DS_PAD_RDLY);
		for (k = 0; k < AUTOK_CMD_TIMES / 4; k++) {
			ret = autok_send_tune_cmd(host, opcode, TUNE_DATA);
			if ((ret & (E_RESULT_CMD_TMO | E_RESULT_RSP_CRC)) != 0) {
				UTIL_Printf("[AUTOK]Error Autok CMD Failed while tune DS Delay\r\n");
				return -1;
			} else if ((ret & (E_RESULT_DAT_CRC | E_RESULT_DAT_TMO)) != 0) {
				RawData64 |= (unsigned long) (1LL << j);
				break;
			} else if ((ret & E_RESULT_FATAL_ERR) != 0)
				return -1;
		}
	}
	RawData64 |= 0xffffffff00000000;
	score = autok_simple_score64(tune_result_str64, RawData64);
	UTIL_Printf("[AUTOK] DLY1/2 %d \t %d \t %s\r\n", uCmdEdge, score,
		       tune_result_str64);
	msdc_autok_window_apply(DS_WIN, RawData64, p_autok_tune_res);
	if (autok_check_scan_res64_new(RawData64, pBdInfo, 0) != 0)
		return -1;
	if (autok_ds_dly_sel(pBdInfo, &uDatDly) == 0) {
		autok_paddly_update(DS_PAD_RDLY, uDatDly, p_autok_tune_res);
	} else {
		UTIL_Printf("[AUTOK][Error]=============Analysis Failed!!=======================\r\n");
	}

	autok_tuning_parameter_init(host, p_autok_tune_res);

	autok_result_dump(host, p_autok_tune_res);
#if AUTOK_PARAM_DUMP_ENABLE
	autok_register_dump(host);
#endif
	msdc_autok_version_apply(p_autok_tune_res);
	if (res != NULL) {
		memcpy((void *)res, (void *)p_autok_tune_res,
		       sizeof(p_autok_tune_res) / sizeof(unsigned char));
	}

	return 0;
}

/* online tuning for latch ck */
int autok_execute_tuning_latch_ck(struct msdc_host *host, unsigned int opcode,
	unsigned int latch_ck_initail_value)
{
	unsigned int ret = 0;
	unsigned int j, k;
	void __iomem *base = host->base;
	unsigned int tune_time;
	unsigned int bus_width;

	MSDC_GET_FIELD(SDC_CFG, SDC_CFG_BUSWIDTH, bus_width);
	if (opcode == SD_IO_RW_EXTENDED)
		return 1;
	MSDC_WRITE32(MSDC_INT, 0xffffffff);
	switch (host->id) {
	case 0:
		if (opcode == MMC_SEND_TUNING_BLOCK_HS200) {
			if (bus_width == 2)
				tune_time = AUTOK_LATCH_CK_EMMC_TUNE_TIMES;
			else if (bus_width == 1)
				tune_time = AUTOK_LATCH_CK_EMMC_TUNE_TIMES_4BIT;
		} else if (opcode == MMC_READ_SINGLE_BLOCK) {
			if (bus_width == 2)
				tune_time = 3;
			else if (bus_width == 1)
				tune_time = 6;
		}
		break;
	case 1:
		tune_time = AUTOK_LATCH_CK_SD_TUNE_TIMES;
		break;
	case 2:
		tune_time = AUTOK_LATCH_CK_SDIO_TUNE_TIMES;
		break;
	case 3:
		tune_time = AUTOK_LATCH_CK_SDIO_TUNE_TIMES;
		break;
	default:
		tune_time = AUTOK_LATCH_CK_SDIO_TUNE_TIMES;
		break;
	}
	for (j = latch_ck_initail_value; j < 8; j += (host->hclk / host->sclk)) {
		host->tune_latch_ck_cnt = 0;
		msdc_clear_fifo();
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_PB0_INT_DAT_LATCH_CK_SEL, j);
		for (k = 0; k < tune_time; k++) {
			if ((opcode == MMC_SEND_TUNING_BLOCK_HS200) && (bus_width == 2)) {
				switch (k) {
				case 0:
					host->tune_latch_ck_cnt = 1;
					break;
				default:
					host->tune_latch_ck_cnt = k;
					break;
				}
			} else if ((opcode == SD_CMD_SEND_TUNING_BLOCK)
			|| ((opcode == MMC_SEND_TUNING_BLOCK_HS200) && (bus_width == 1))) {
				switch (k) {
				case 0:
				case 1:
				case 2:
					host->tune_latch_ck_cnt = 1;
					break;
				default:
					host->tune_latch_ck_cnt = k - 1;
					break;
				}
			} else if (opcode == MMC_SEND_EXT_CSD) {
				host->tune_latch_ck_cnt = k + 1;
			} else
				host->tune_latch_ck_cnt++;
			if(!msdc_get_cd(host))
						return -1;
			ret = autok_send_tune_cmd(host, opcode, TUNE_LATCH_CK);
			if ((ret & (E_RESULT_CMD_TMO | E_RESULT_RSP_CRC)) != 0) {
			#if AUTOK_DEBUG_LOG
				UTIL_Printf("[AUTOK]Error Autok CMD Failed while tune LATCH CK\r\n");
			#endif
				break;
			} else if ((ret & (E_RESULT_DAT_CRC | E_RESULT_DAT_TMO)) != 0) {
			#if AUTOK_DEBUG_LOG
				UTIL_Printf("[AUTOK]Error Autok  tune LATCH_CK error %d\r\n", j);
			#endif
				break;
			}
		}
		if (ret == 0) {
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_PB0_INT_DAT_LATCH_CK_SEL, j);
			break;
		}
	}
	host->tune_latch_ck_cnt = 0;

	return (j >= 8) ? 7 : j;

}

/* online tuning for SDIO/SD /eMMC4.5(hs200)  */
int execute_online_tuning(struct msdc_host *host, unsigned char *res, unsigned int opcode)
{
	void __iomem *base = host->base;
	unsigned int ret = 0;
	unsigned int uEdge = 0;
	unsigned long RawData64 = 0LL;
	unsigned long RawCmd64 = 0LL;
	unsigned int score = 0;
	unsigned int cmd_result = 0;
	unsigned int data_result = 0;
	unsigned int j, k ,l;
	unsigned int bd_err = BD_NO_ERR;
	struct AUTOK_REF_INFO_NEW uCmdInfo;
	struct AUTOK_REF_INFO_NEW uDatInfo;
	struct AUTOK_SCAN_RES_NEW *pBdCmdInfo;
	struct AUTOK_SCAN_RES_NEW *pBdDatInfo;
	char tune_result_str64[65];
	unsigned char p_autok_tune_res[TUNING_PARA_SCAN_COUNT];
 	if (opcode == MMC_SEND_TUNING_BLOCK_HS200)
		autok_init_hs200(host);
	else if ((opcode == SD_CMD_SEND_TUNING_BLOCK)
		|| (opcode == SD_IO_RW_EXTENDED))
		autok_init_sdr104(host);
	memset((void *)p_autok_tune_res, 0, sizeof(p_autok_tune_res) / sizeof(unsigned char));

	/* Step1 : Tuning Cmd Path and Data Path (Only Rising Edge Used)  */
	autok_tuning_parameter_init(host, p_autok_tune_res);
	for (l = ckgen[host->id]; l < 8; l = l + 2) {
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_PB0_CKGEN_MSDC_DLY_SEL, l);
		#if AUTOK_DEBUG_LOG
		UTIL_Printf("[AUTOK]adjust CKGEN = %d\r\n", l);
		#endif
	memset(&uCmdInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));
	memset(&uDatInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));

	uEdge = 0;
	do {
		pBdCmdInfo = (struct AUTOK_SCAN_RES_NEW *)&(uCmdInfo.scan_info[uEdge]);
		pBdDatInfo = (struct AUTOK_SCAN_RES_NEW *)&(uDatInfo.scan_info[uEdge]);
		msdc_autok_adjust_param(host, CMD_EDGE, &uEdge, AUTOK_WRITE);
		msdc_autok_adjust_param(host, RD_FIFO_EDGE, &uEdge, AUTOK_WRITE);
		RawCmd64 = 0LL;
		RawData64 = 0LL;
			for (j = 0; j < 64; j++) {
				msdc_autok_adjust_paddly(host, &j, CMD_PAD_RDLY);
				msdc_autok_adjust_paddly(host, &j, DAT_PAD_RDLY);
				for (k = 0; k < AUTOK_CMD_TIMES / 2; k++) {
					if(!msdc_get_cd(host))
						return -1;
	  				ret = autok_send_tune_cmd(host, opcode, TUNE_DATA);
					if ((ret & E_RESULT_RSP_CRC) != 0)
						RawCmd64 |= (unsigned long) (1LL << j);
					else if ((ret & E_RESULT_CMD_TMO) != 0) {
						autok_msdc_reset();
						msdc_clear_fifo();
						MSDC_WRITE32(MSDC_INT, 0xffffffff);
						RawCmd64 |= (unsigned long) (1LL << j);
					} else if ((ret & E_RESULT_FATAL_ERR) != 0)
						return -1;
					if ((ret & (E_RESULT_DAT_CRC | E_RESULT_DAT_TMO)) != 0)
						RawData64 |= (unsigned long) (1LL << j);
					else if ((ret & E_RESULT_FATAL_ERR) != 0)
						return -1;
				}
			}
			score = autok_simple_score64(tune_result_str64, RawCmd64);
			UTIL_Printf("[AUTOK]CMD %d \t %d \t %s\r\n", uEdge, score,
				       tune_result_str64);
			score = autok_simple_score64(tune_result_str64, RawData64);
			UTIL_Printf("[AUTOK]DAT %d \t %d \t %s\r\n", uEdge, score,
				       tune_result_str64);
			if (uEdge) {
				msdc_autok_window_apply(CMD_FALL, RawCmd64, p_autok_tune_res);
				msdc_autok_window_apply(DAT_FALL, RawData64, p_autok_tune_res);
			} else {
				msdc_autok_window_apply(CMD_RISE, RawCmd64, p_autok_tune_res);
				msdc_autok_window_apply(DAT_RISE, RawData64, p_autok_tune_res);
			}
			if (autok_check_scan_res64_new(RawCmd64, pBdCmdInfo, AUTOK_BD_FILTER) != 0) {
				host->autok_error = -1;
				return -1;
			}
			if (autok_check_scan_res64_new(RawData64, pBdDatInfo, AUTOK_BD_FILTER) != 0) {
				host->autok_error = -1;
				return -1;
			}

			uEdge ^= 0x1;
		} while (uEdge);
		if (autok_pad_dly_sel_new(&uCmdInfo) != 0) {
			cmd_result = 1;
			UTIL_Printf("[AUTOK][Error]=============CMD Analysis Failed!!=======================\r\n");
		}
		autok_param_update(CMD_EDGE, uCmdInfo.opt_edge_sel, p_autok_tune_res);
		autok_paddly_update(CMD_PAD_RDLY, uCmdInfo.opt_dly_cnt, p_autok_tune_res);
		bd_err = autok_pad_dly_sel_new(&uDatInfo);
		if (bd_err != 0) {
			UTIL_Printf("[AUTOK][Error]=============DATA Analysis Failed 0x%x!!=======================\r\n", bd_err);
			/*
			* only two edge bd tune fail ,use CKEGN retune.
			* tune CKGEN may not useful
			*/
			if (bd_err & BD_ALL_FAIL_ERR)
				data_result = 1;
			else {
				data_result = 0;
				break;
			}
		} else {
			data_result = 0;
			break;
		}
	}
	autok_param_update(RD_FIFO_EDGE, uDatInfo.opt_edge_sel, p_autok_tune_res);
	autok_paddly_update(DAT_PAD_RDLY, uDatInfo.opt_dly_cnt, p_autok_tune_res);
	autok_param_update(WD_FIFO_EDGE, uDatInfo.opt_edge_sel, p_autok_tune_res);
	if (cmd_result == 1) {
		autok_param_update(CMD_EDGE, uDatInfo.opt_edge_sel, p_autok_tune_res);
		autok_paddly_update(CMD_PAD_RDLY, uDatInfo.opt_dly_cnt, p_autok_tune_res);
	}
 	autok_tuning_parameter_init(host, p_autok_tune_res);
#if !NEW_PATH
	if (data_result == 0) {
		/* if data scan fail,do not tune latch ck */
		/* Step2 : Tuning LATCH CK  */
		p_autok_tune_res[INT_DAT_LATCH_CK] = autok_execute_tuning_latch_ck(host, opcode,
			p_autok_tune_res[INT_DAT_LATCH_CK]);
	}
#endif
	autok_tuning_parameter_init(host, p_autok_tune_res);
	
	autok_result_dump(host, p_autok_tune_res);

#if AUTOK_PARAM_DUMP_ENABLE
	autok_register_dump(host);
#endif
	msdc_autok_version_apply(p_autok_tune_res);
	if (res != NULL) {
		memcpy((void *)res, (void *)p_autok_tune_res,
		       sizeof(p_autok_tune_res) / sizeof(unsigned char));
	}

	return 0;
}

int execute_ddr50_online_tuning(struct msdc_host *host, unsigned char *res, unsigned int opcode)
{
	unsigned long base = host->base;
	unsigned int ret = 0;
	unsigned int uEdge = 0;
	unsigned long RawData64 = 0LL;
	unsigned long RawCmd64 = 0LL;
	unsigned int score = 0;
	unsigned int cmd_result = 0;
	unsigned int j, k ;
#if DDR50_TUNE_SINGLE_EDGE
	unsigned int ckgen;
#endif

	unsigned int check_cnt = 0;
	unsigned int iorx = 0;
	unsigned int base_addr = 0;
	unsigned int func_num = 0;
	unsigned int reg_value = 0;
	unsigned int r_w_dirc = 0;
	unsigned int uDatDly = 0;

	struct AUTOK_REF_INFO_NEW uCmdInfo;
	struct AUTOK_REF_INFO_NEW uDatInfo;
	struct AUTOK_SCAN_RES_NEW *pBdCmdInfo;
	struct AUTOK_SCAN_RES_NEW *pBdDatInfo;
	char tune_result_str64[65];
	unsigned char p_autok_tune_res[TUNING_PARA_SCAN_COUNT];

 	if (opcode == MMC_READ_SINGLE_BLOCK)
		autok_init_ddr(host);
	else if (opcode == SD_IO_RW_EXTENDED) {
		autok_init_ddr(host);
		UTIL_Printf("[AUTOK]SDIO device function enable\r\n");
		/* read previous device setting */
		base_addr = 0x02;
		func_num = 0x0;
		reg_value = 0;
		r_w_dirc = EXT_READ;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (reg_value & 0x02)
			goto tune_rx;
		/* function has not enabled, enable device function1 */
		base_addr = 0x02;
		func_num = 0x0;
		reg_value |= 0x02;
		r_w_dirc = EXT_WRITE;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]IOEx reg 0x%x set fail\r\n", base_addr);
		UTIL_Printf("[AUTOK]SDIO device function enable ready check\r\n");
		while ((!(iorx & 0x02)) && (check_cnt < 10)) {
			check_cnt++;
			base_addr = 0x03;
			func_num = 0x0;
			reg_value = 0x00;
			r_w_dirc = EXT_READ;
			ret = autok_sdio_device_rx_set(host, func_num, base_addr,
				&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
			if (ret != E_RESULT_PASS)
				UTIL_Printf("[AUTOK]IOEx reg 0x%x set fail\r\n", base_addr);
			iorx = MSDC_READ32(SDC_RESP0) & 0xff;
			UTIL_Printf("[AUTOK]iorx 0x%x\r\n", iorx);
		}
	}
tune_rx:
	memset((void *)p_autok_tune_res, 0, sizeof(p_autok_tune_res) / sizeof(unsigned char));

	/* Step1 : Tuning Cmd Path and Data Path (Only Rising Edge Used)  */
	autok_tuning_parameter_init(host, p_autok_tune_res);
	memset(&uCmdInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));
	memset(&uDatInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));

#if DDR50_TUNE_SINGLE_EDGE
	uEdge = 0;
	pBdCmdInfo = (struct AUTOK_SCAN_RES_NEW *)&(uCmdInfo.scan_info[uEdge]);
	pBdDatInfo = (struct AUTOK_SCAN_RES_NEW *)&(uDatInfo.scan_info[uEdge]);
	msdc_autok_adjust_param(host, CMD_EDGE, &uEdge, AUTOK_WRITE);
	msdc_autok_adjust_param(host, RDATA_EDGE, &uEdge, AUTOK_WRITE);
	for(ckgen = 0; ckgen < 32; ckgen++) {
		autok_write_param(host, CKGEN_MSDC_DLY_SEL, ckgen);
		RawCmd64 = 0LL;
		RawData64 = 0LL;
		for (j = 0; j < 64; j++) {
			msdc_autok_adjust_paddly(host, &j, CMD_PAD_RDLY);
			msdc_autok_adjust_paddly(host, &j, DAT_PAD_RDLY);
			for (k = 0; k < AUTOK_CMD_TIMES / 2; k++) {
				if (host->id == 3) {
					base_addr = 0x11c;
					func_num = 0x1;
					reg_value = 0;
					r_w_dirc = EXT_READ;
					ret = autok_sdio_device_rx_set(host, func_num, base_addr, 
						&reg_value, r_w_dirc, 4, SD_IO_RW_EXTENDED);
				} else
					ret = autok_send_tune_cmd(host, opcode, TUNE_DATA);
				if ((ret & E_RESULT_RSP_CRC) != 0)
					RawCmd64 |= (unsigned long) (1LL << j);
				else if ((ret & E_RESULT_CMD_TMO) != 0) {
					autok_msdc_reset();
					msdc_clear_fifo();
					MSDC_WRITE32(MSDC_INT, 0xffffffff);
					RawCmd64 |= (unsigned long) (1LL << j);
				} else if ((ret & E_RESULT_FATAL_ERR) != 0)
					return -1;
				if ((ret & (E_RESULT_DAT_CRC | E_RESULT_DAT_TMO)) != 0)
					RawData64 |= (unsigned long) (1LL << j);
				else if ((ret & E_RESULT_FATAL_ERR) != 0)
					return -1;
			}
		}
		score = autok_simple_score64(tune_result_str64, RawCmd64);
		UTIL_Printf("[AUTOK]CMD %d \t %d \t %s\r\n", uEdge, score,
			       tune_result_str64);
		score = autok_simple_score64(tune_result_str64, RawData64);
		UTIL_Printf("[AUTOK]DAT %d \t %d \t %s\r\n", uEdge, score,
			       tune_result_str64);
		if (score >= 20)
			break;
	}
	msdc_autok_window_apply(CMD_RISE, RawCmd64, p_autok_tune_res);
	msdc_autok_window_apply(DAT_RISE, RawData64, p_autok_tune_res);
	if (autok_check_scan_res64_new(RawCmd64, pBdCmdInfo, AUTOK_BD_FILTER) != 0) {
		host->autok_error = -1;
		return -1;
	}
	if (autok_check_scan_res64_new(RawData64, pBdDatInfo, AUTOK_BD_FILTER) != 0) {
		host->autok_error = -1;
		return -1;
	}

	if (autok_ds_dly_sel(pBdCmdInfo, &uDatDly) != 0) {
		cmd_result = 1;
		UTIL_Printf("[AUTOK][Error]=============CMD Analysis Failed!!=======================\r\n");
	}
	autok_paddly_update(CMD_PAD_RDLY, uDatDly, p_autok_tune_res);
	if (autok_ds_dly_sel(pBdDatInfo, &uDatDly) != 0) {
		UTIL_Printf("[AUTOK][Error]=============DATA Analysis Failed!!=======================\r\n");
	}
	autok_paddly_update(DAT_PAD_RDLY, uDatDly, p_autok_tune_res);

	if (cmd_result == 1) {
		autok_paddly_update(CMD_PAD_RDLY, uDatDly, p_autok_tune_res);
	}
#else
	uEdge = 0;
	do {
		pBdCmdInfo = (struct AUTOK_SCAN_RES_NEW *)&(uCmdInfo.scan_info[uEdge]);
		pBdDatInfo = (struct AUTOK_SCAN_RES_NEW *)&(uDatInfo.scan_info[uEdge]);
		msdc_autok_adjust_param(host, CMD_EDGE, &uEdge, AUTOK_WRITE);
		msdc_autok_adjust_param(host, RDATA_EDGE, &uEdge, AUTOK_WRITE);
		RawCmd64 = 0LL;
		RawData64 = 0LL;
		for (j = 0; j < 64; j++) {
			msdc_autok_adjust_paddly(host, &j, CMD_PAD_RDLY);
			msdc_autok_adjust_paddly(host, &j, DAT_PAD_RDLY);
			for (k = 0; k < AUTOK_CMD_TIMES / 2; k++) {
				if (host->id == 3) {
					base_addr = 0x11c;
					func_num = 0x1;
					reg_value = 0;
					r_w_dirc = EXT_READ;
					ret = autok_sdio_device_rx_set(host, func_num, base_addr, 
						&reg_value, r_w_dirc, 4, SD_IO_RW_EXTENDED);
				} else
					ret = autok_send_tune_cmd(host, opcode, TUNE_DATA);
				if ((ret & E_RESULT_RSP_CRC) != 0)
					RawCmd64 |= (unsigned long) (1LL << j);
				else if ((ret & E_RESULT_CMD_TMO) != 0) {
					autok_msdc_reset();
					msdc_clear_fifo();
					MSDC_WRITE32(MSDC_INT, 0xffffffff);
					RawCmd64 |= (unsigned long) (1LL << j);
				} else if ((ret & E_RESULT_FATAL_ERR) != 0)
					return -1;
				if ((ret & (E_RESULT_DAT_CRC | E_RESULT_DAT_TMO)) != 0)
					RawData64 |= (unsigned long) (1LL << j);
				else if ((ret & E_RESULT_FATAL_ERR) != 0)
					return -1;
			}
		}
		score = autok_simple_score64(tune_result_str64, RawCmd64);
		UTIL_Printf("[AUTOK]CMD %d \t %d \t %s\r\n", uEdge, score,
			       tune_result_str64);
		score = autok_simple_score64(tune_result_str64, RawData64);
		UTIL_Printf("[AUTOK]DAT %d \t %d \t %s\r\n", uEdge, score,
			       tune_result_str64);
		if (uEdge) {
			msdc_autok_window_apply(CMD_FALL, RawCmd64, p_autok_tune_res);
			msdc_autok_window_apply(DAT_FALL, RawData64, p_autok_tune_res);
		} else {
			msdc_autok_window_apply(CMD_RISE, RawCmd64, p_autok_tune_res);
			msdc_autok_window_apply(DAT_RISE, RawData64, p_autok_tune_res);
		}
		if (autok_check_scan_res64_new(RawCmd64, pBdCmdInfo, AUTOK_BD_FILTER) != 0) {
			host->autok_error = -1;
			return -1;
		}
		if (autok_check_scan_res64_new(RawData64, pBdDatInfo, AUTOK_BD_FILTER) != 0) {
			host->autok_error = -1;
			return -1;
		}

		uEdge ^= 0x1;
	} while (uEdge);

	if (autok_pad_dly_sel_new(&uCmdInfo) != 0) {
		cmd_result = 1;
		UTIL_Printf("[AUTOK][Error]=============CMD Analysis Failed!!=======================\r\n");
	}
	autok_param_update(CMD_EDGE, uCmdInfo.opt_edge_sel, p_autok_tune_res);
	autok_paddly_update(CMD_PAD_RDLY, uCmdInfo.opt_dly_cnt, p_autok_tune_res);
	if (autok_pad_dly_sel_new(&uDatInfo) != 0) {
		UTIL_Printf("[AUTOK][Error]=============DATA Analysis Failed!!=======================\r\n");
	}
	autok_param_update(RDATA_EDGE, uDatInfo.opt_edge_sel, p_autok_tune_res);
	autok_paddly_update(DAT_PAD_RDLY, uDatInfo.opt_dly_cnt, p_autok_tune_res);
	autok_param_update(WDATA_EDGE, uDatInfo.opt_edge_sel, p_autok_tune_res);

	autok_param_update(RDATA_EDGE, 0, p_autok_tune_res);
	autok_paddly_update(DAT_PAD_RDLY, 3, p_autok_tune_res);
	autok_param_update(WDATA_EDGE, 0, p_autok_tune_res);
	if (cmd_result == 1) {
		autok_param_update(CMD_EDGE, uDatInfo.opt_edge_sel, p_autok_tune_res);
		autok_paddly_update(CMD_PAD_RDLY, uDatInfo.opt_dly_cnt, p_autok_tune_res);
	}
#endif
 	autok_tuning_parameter_init(host, p_autok_tune_res);
#if 0
	/* Step2 : Tuning LATCH CK  */
	p_autok_tune_res[INT_DAT_LATCH_CK] = autok_execute_tuning_latch_ck(host, opcode,
		p_autok_tune_res[INT_DAT_LATCH_CK]);
#endif
	autok_tuning_parameter_init(host, p_autok_tune_res);
	autok_result_dump(host, p_autok_tune_res);

#if AUTOK_PARAM_DUMP_ENABLE
	autok_register_dump(host);
#endif
	msdc_autok_version_apply(p_autok_tune_res);
	if (res != NULL) {
		memcpy((void *)res, (void *)p_autok_tune_res,
		       sizeof(p_autok_tune_res) / sizeof(unsigned char));
	}

	return 0;
}

void autok_msdc_tx_setting(struct msdc_host *host,struct mmc_ios *ios)
{
	void __iomem *base = host->base;
	unsigned int clock_mode = 0;
	unsigned int timing_mode = 0;
    
	MSDC_GET_FIELD(MSDC_CFG,MSDC_CFG_CKMOD, clock_mode);

	if ((host->id == 0) && (clock_mode == 2))
		timing_mode = AUTOK_MMC_MMC_DDR52;
	else if (clock_mode == 2)
		timing_mode = AUTOK_MMC_UHS_DDR50;
	else if ((host->id == 0) && (clock_mode == 3))
		timing_mode = AUTOK_MMC_MMC_HS400;
	else if (clock_mode == 1)
		timing_mode = AUTOK_MMC_UHS_SDR104;

	if (host->id == 0) {
		if (ios->timing == MMC_TIMING_MMC_HS400) {
			MSDC_SET_FIELD(EMMC50_CFG0,
				MSDC_EMMC50_CFG_TXSKEW_SEL,
				AUTOK_MSDC0_HS400_TXSKEW);
			MSDC_SET_FIELD(MSDC_PAD_TUNE0,
				MSDC_PAD_TUNE0_CLKTXDLY,
				host->clk_tx);
			MSDC_SET_FIELD(EMMC50_PAD_CMD_TUNE,
				MSDC_EMMC50_PAD_CMD_TUNE_TXDLY,
				AUTOK_MSDC0_HS400_CMDTXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT01_TUNE,
				MSDC_EMMC50_PAD_DAT0_TXDLY,
				AUTOK_MSDC0_HS400_DAT0TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT01_TUNE,
				MSDC_EMMC50_PAD_DAT1_TXDLY,
				AUTOK_MSDC0_HS400_DAT1TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT23_TUNE,
				MSDC_EMMC50_PAD_DAT2_TXDLY,
				AUTOK_MSDC0_HS400_DAT2TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT23_TUNE,
				MSDC_EMMC50_PAD_DAT3_TXDLY,
				AUTOK_MSDC0_HS400_DAT3TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT45_TUNE,
				MSDC_EMMC50_PAD_DAT4_TXDLY,
				AUTOK_MSDC0_HS400_DAT4TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT45_TUNE,
				MSDC_EMMC50_PAD_DAT5_TXDLY,
				AUTOK_MSDC0_HS400_DAT5TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT67_TUNE,
				MSDC_EMMC50_PAD_DAT6_TXDLY,
				AUTOK_MSDC0_HS400_DAT6TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT67_TUNE,
				MSDC_EMMC50_PAD_DAT7_TXDLY,
				AUTOK_MSDC0_HS400_DAT7TXDLY);
		} else {
			if (ios->timing == MMC_TIMING_UHS_DDR50) {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_DDR50CKD,
					AUTOK_MSDC0_DDR50_DDRCKD);
			} else {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_DDR50CKD, 0);
			}
			MSDC_SET_FIELD(MSDC_PAD_TUNE0,
				MSDC_PAD_TUNE0_CLKTXDLY,
				host->clk_tx);
			MSDC_SET_FIELD(EMMC50_PAD_CMD_TUNE,
				MSDC_EMMC50_PAD_CMD_TUNE_TXDLY,
				AUTOK_MSDC0_CMDTXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT01_TUNE,
				MSDC_EMMC50_PAD_DAT0_TXDLY,
				AUTOK_MSDC0_DAT0TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT01_TUNE,
				MSDC_EMMC50_PAD_DAT1_TXDLY,
				AUTOK_MSDC0_DAT1TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT23_TUNE,
				MSDC_EMMC50_PAD_DAT2_TXDLY,
				AUTOK_MSDC0_DAT2TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT23_TUNE,
				MSDC_EMMC50_PAD_DAT3_TXDLY,
				AUTOK_MSDC0_DAT3TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT45_TUNE,
				MSDC_EMMC50_PAD_DAT4_TXDLY,
				AUTOK_MSDC0_DAT4TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT45_TUNE,
				MSDC_EMMC50_PAD_DAT5_TXDLY,
				AUTOK_MSDC0_DAT5TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT67_TUNE,
				MSDC_EMMC50_PAD_DAT6_TXDLY,
				AUTOK_MSDC0_DAT6TXDLY);
			MSDC_SET_FIELD(EMMC50_PAD_DAT67_TUNE,
				MSDC_EMMC50_PAD_DAT7_TXDLY,
				AUTOK_MSDC0_DAT7TXDLY);
		}
	} else if (host->id == 1) {
		MSDC_SET_FIELD(MSDC_IOCON,
			MSDC_IOCON_DDR50CKD, AUTOK_MSDC_DDRCKD);
		if (ios->timing == MMC_TIMING_UHS_SDR104) {
			MSDC_SET_FIELD(MSDC_PAD_TUNE0,
				MSDC_PAD_TUNE0_CLKTXDLY,
				host->clk_tx);
		} else {
			MSDC_SET_FIELD(MSDC_PAD_TUNE0,
				MSDC_PAD_TUNE0_CLKTXDLY,
				host->clk_tx);
		}
	} else if (host->id == 2) {
		MSDC_SET_FIELD(MSDC_IOCON,
			MSDC_IOCON_DDR50CKD, AUTOK_MSDC_DDRCKD);
		MSDC_SET_FIELD(MSDC_PAD_TUNE0,
			MSDC_PAD_TUNE0_CLKTXDLY,
			host->clk_tx);
	} else if (host->id == 3) {
		MSDC_SET_FIELD(MSDC_IOCON,
			MSDC_IOCON_DDR50CKD, AUTOK_MSDC_DDRCKD);
		MSDC_SET_FIELD(MSDC_PAD_TUNE0,
			MSDC_PAD_TUNE0_CLKTXDLY,
			host->clk_tx);
	}
}
EXPORT_SYMBOL(autok_msdc_tx_setting);

void autok_low_speed_switch_edge(struct msdc_host *host,struct mmc_ios *ios, enum ERROR_TYPE error_type)
{
	unsigned long base = host->base;
	unsigned int orig_resp_edge, orig_crc_fifo_edge, orig_read_edge, orig_read_fifo_edge;
	unsigned int cur_resp_edge, cur_crc_fifo_edge, cur_read_edge, cur_read_fifo_edge;
    unsigned int clock_mode = 0;
    unsigned int timing_mode = 0;

	MSDC_GET_FIELD(MSDC_CFG,MSDC_CFG_CKMOD, clock_mode);

	if ((host->id == 0) && (clock_mode == 2))
		timing_mode = AUTOK_MMC_MMC_DDR52;
	else if (clock_mode == 2)
		timing_mode = AUTOK_MMC_UHS_DDR50;

	//UTIL_Printf("[AUTOK][low speed switch edge]=========start========\r\n");
	if (host->id == 0) {
		switch (error_type) {
		case CMD_ERROR:
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, orig_resp_edge);
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, orig_resp_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, cur_resp_edge);
			UTIL_Printf("[AUTOK][CMD err]pre_edge = %d cur_edge = %d\r\n"
				, orig_resp_edge, cur_resp_edge);
			break;
		case DATA_ERROR:
#ifdef PORT0_PB0_RD_DAT_SEL_VALID
			if (ios->timing == MMC_TIMING_UHS_DDR50) {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge);
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge ^ 0x1);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , 0);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, cur_read_edge);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , cur_read_fifo_edge);
				UTIL_Printf("[AUTOK][read err]PB0_BIT3_VALID DDR pre_edge = %d",
					orig_read_edge);
				UTIL_Printf("cur_edge = %d cur_fifo_edge = %d\r\n",
					cur_read_edge, cur_read_fifo_edge);
			} else {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, 0);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , orig_read_fifo_edge);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , orig_read_fifo_edge ^ 0x1);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, cur_read_edge);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , cur_read_fifo_edge);
				UTIL_Printf("[AUTOK][read err]PB0[3]_VALID orig_fifo_edge = %d",
					orig_read_fifo_edge);
				UTIL_Printf("cur_edge = %d cur_fifo_edge = %d\r\n",
					cur_read_edge, cur_read_fifo_edge);
			}
#else
			MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
			MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , 0);
			MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, cur_read_edge);
			UTIL_Printf("[AUTOK][read err]PB0[3]_INVALID pre_edge = %d cur_edge = %d\r\n"
				, orig_read_edge, cur_read_edge);
#endif
			break;
		case CRC_STATUS_ERROR:
			MSDC_GET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, orig_crc_fifo_edge);
			MSDC_SET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, orig_crc_fifo_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, cur_crc_fifo_edge);
			UTIL_Printf("[AUTOK][write err]orig_fifo_edge = %d cur_fifo_edge = %d\r\n"
				, orig_crc_fifo_edge, cur_crc_fifo_edge);
			break;
		}
	} else if (host->id == 1) {
		switch (error_type) {
		case CMD_ERROR:
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, orig_resp_edge);
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, orig_resp_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, cur_resp_edge);
			UTIL_Printf("[AUTOK][CMD err]pre_edge = %d cur_edge = %d\r\n"
					, orig_resp_edge, cur_resp_edge);
			break;
		case DATA_ERROR:
#ifdef PORT1_PB0_RD_DAT_SEL_VALID
			if (ios->timing == MMC_TIMING_UHS_DDR50) {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge);
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge ^ 0x1);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , 0);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, cur_read_edge);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , cur_read_fifo_edge);
				UTIL_Printf("[AUTOK][read err]PB0[3]_VALID DDR pre_edge = %d",
					orig_read_edge);
				UTIL_Printf(" cur_edge = %d cur_fifo_edge = %d\r\n",
					cur_read_edge, cur_read_fifo_edge);
			} else {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, 0);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , orig_read_fifo_edge);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , orig_read_fifo_edge ^ 0x1);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, cur_read_edge);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , cur_read_fifo_edge);
				UTIL_Printf("[AUTOK][read err]PB0[3]_VALID orig_fifo_edge = %d",
					orig_read_fifo_edge);
				UTIL_Printf(" cur_edge = %d cur_fifo_edge = %d\r\n",
					cur_read_edge, cur_read_fifo_edge);
			}
#else
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL_SEL, 0);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, orig_read_edge);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , 0);
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, orig_read_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, cur_read_edge);
			UTIL_Printf("[AUTOK][read err]PB0[3]_INVALID pre_edge = %d cur_edge = %d\r\n"
				, orig_read_edge, cur_read_edge);
#endif
			break;
		case CRC_STATUS_ERROR:
			MSDC_GET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, orig_crc_fifo_edge);
			MSDC_SET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, orig_crc_fifo_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, cur_crc_fifo_edge);
			UTIL_Printf("[AUTOK][write err]orig_fifo_edge = %d cur_fifo_edge = %d\r\n"
				, orig_crc_fifo_edge, cur_crc_fifo_edge);
			break;
		}
	} else if (host->id == 2) {
		switch (error_type) {
		case CMD_ERROR:
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, orig_resp_edge);
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, orig_resp_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, cur_resp_edge);
			UTIL_Printf("[AUTOK][CMD err]pre_edge = %d cur_edge = %d\r\n"
				, orig_resp_edge, cur_resp_edge);
			break;
		case DATA_ERROR:
#ifdef PORT2_PB0_RD_DAT_SEL_VALID
			if (ios->timing == MMC_TIMING_UHS_DDR50) {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge);
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge ^ 0x1);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , 0);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, cur_read_edge);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , cur_read_fifo_edge);
				UTIL_Printf("[AUTOK][read err]PB0[3]_VALID DDR pre_edge = %d",
					orig_read_edge);
				UTIL_Printf(" cur_edge = %d cur_fifo_edge = %d\r\n",
					cur_read_edge, cur_read_fifo_edge);
			} else {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, 0);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , orig_read_fifo_edge);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , orig_read_fifo_edge ^ 0x1);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, cur_read_edge);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , cur_read_fifo_edge);
				UTIL_Printf("[AUTOK][read err]PB0[3]_VALID orig_fifo_edge = %d",
					orig_read_fifo_edge);
				UTIL_Printf(" cur_edge = %d cur_fifo_edge = %d\r\n",
					cur_read_edge, cur_read_fifo_edge);
			}
#else
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL_SEL, 0);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, orig_read_edge);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , 0);
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, orig_read_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, cur_read_edge);
			UTIL_Printf("[AUTOK][read err]PB0[3]_INVALID pre_edge = %d cur_edge = %d\r\n"
				, orig_read_edge, cur_read_edge);
#endif
			break;
		case CRC_STATUS_ERROR:
			MSDC_GET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, orig_crc_fifo_edge);
			MSDC_SET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, orig_crc_fifo_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, cur_crc_fifo_edge);
			UTIL_Printf("[AUTOK][write err]orig_fifo_edge = %d cur_fifo_edge = %d\r\n"
				, orig_crc_fifo_edge, cur_crc_fifo_edge);
			break;
		}
	} else if (host->id == 3) {
		switch (error_type) {
		case CMD_ERROR:
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, orig_resp_edge);
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, orig_resp_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_RSPL, cur_resp_edge);
			UTIL_Printf("[AUTOK][CMD err]pre_edge = %d cur_edge = %d\r\n"
				, orig_resp_edge, cur_resp_edge);
			break;
		case DATA_ERROR:
#ifdef PORT3_PB0_RD_DAT_SEL_VALID
			if (ios->timing == MMC_TIMING_UHS_DDR50) {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge);
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, orig_read_edge ^ 0x1);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , 0);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, cur_read_edge);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , cur_read_fifo_edge);
				UTIL_Printf("[AUTOK][read err]PB0[3]_VALID DDR pre_edge = %d",
					orig_read_edge);
				UTIL_Printf(" cur_edge = %d cur_fifo_edge = %d\r\n",
					cur_read_edge, cur_read_fifo_edge);
			} else {
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL_SEL, 0);
				MSDC_SET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, 0);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , orig_read_fifo_edge);
				MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , orig_read_fifo_edge ^ 0x1);
				MSDC_GET_FIELD(MSDC_IOCON,
					MSDC_IOCON_R_D_SMPL, cur_read_edge);
				MSDC_GET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , cur_read_fifo_edge);
				UTIL_Printf("[AUTOK][read err]PB0[3]_VALID orig_fifo_edge = %d",
					orig_read_fifo_edge);
				UTIL_Printf(" cur_edge = %d cur_fifo_edge = %d\r\n",
					cur_read_edge, cur_read_fifo_edge);
			}
#else
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL_SEL, 0);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, orig_read_edge);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0,
					MSDC_PB0_RD_DAT_SEL , 0);
			MSDC_SET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, orig_read_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_IOCON,
				MSDC_IOCON_R_D_SMPL, cur_read_edge);
			UTIL_Printf("[AUTOK][read err]PB0[3]_INVALID pre_edge = %d cur_edge = %d\r\n"
				, orig_read_edge, cur_read_edge);
#endif
			break;
		case CRC_STATUS_ERROR:
			MSDC_GET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, orig_crc_fifo_edge);
			MSDC_SET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, orig_crc_fifo_edge ^ 0x1);
			MSDC_GET_FIELD(MSDC_PATCH_BIT2,
				MSDC_PB2_CFGCRCSTSEDGE, cur_crc_fifo_edge);
			UTIL_Printf("[AUTOK][write err]orig_fifo_edge = %d cur_fifo_edge = %d\r\n"
				, orig_crc_fifo_edge, cur_crc_fifo_edge);
			break;
		}
	}
	//UTIL_Printf("[AUTOK][low speed switch edge]=========end========\r\n");
}
EXPORT_SYMBOL(autok_low_speed_switch_edge);

int autok_offline_tuning_TX(struct msdc_host *host)
{
	int ret = 0;
	void __iomem *base = host->base;
	unsigned int response;
	unsigned int tune_tx_value;
	unsigned char tune_cnt;
	unsigned char i;
	unsigned char tune_crc_cnt[32];
	unsigned char tune_pass_cnt[32];
	unsigned char tune_tmo_cnt[32];
	char tune_result[33];
	unsigned int cmd_tx;
	unsigned int dat0_tx;
	unsigned int dat1_tx;
	unsigned int dat2_tx;
	unsigned int dat3_tx;
	unsigned int dat4_tx;
	unsigned int dat5_tx;
	unsigned int dat6_tx;
	unsigned int dat7_tx;

	UTIL_Printf("[AUTOK][tune cmd TX]=========start========\r\n");
	/* store tx setting */
	MSDC_GET_FIELD(EMMC50_PAD_CMD_TUNE, MSDC_EMMC50_PAD_CMD_TUNE_TXDLY, cmd_tx);
	MSDC_GET_FIELD(EMMC50_PAD_DAT01_TUNE, MSDC_EMMC50_PAD_DAT0_TXDLY, dat0_tx);
	MSDC_GET_FIELD(EMMC50_PAD_DAT01_TUNE, MSDC_EMMC50_PAD_DAT1_TXDLY, dat1_tx);
	MSDC_GET_FIELD(EMMC50_PAD_DAT23_TUNE, MSDC_EMMC50_PAD_DAT2_TXDLY, dat2_tx);
	MSDC_GET_FIELD(EMMC50_PAD_DAT23_TUNE, MSDC_EMMC50_PAD_DAT3_TXDLY, dat3_tx);
	MSDC_GET_FIELD(EMMC50_PAD_DAT45_TUNE, MSDC_EMMC50_PAD_DAT4_TXDLY, dat4_tx);
	MSDC_GET_FIELD(EMMC50_PAD_DAT45_TUNE, MSDC_EMMC50_PAD_DAT5_TXDLY, dat5_tx);
	MSDC_GET_FIELD(EMMC50_PAD_DAT67_TUNE, MSDC_EMMC50_PAD_DAT6_TXDLY, dat6_tx);
	MSDC_GET_FIELD(EMMC50_PAD_DAT67_TUNE, MSDC_EMMC50_PAD_DAT7_TXDLY, dat7_tx);

	/* Step1 : Tuning Cmd TX */
	for (tune_tx_value = 0; tune_tx_value < 32; tune_tx_value++) {
		tune_tmo_cnt[tune_tx_value] = 0;
		tune_crc_cnt[tune_tx_value] = 0;
		tune_pass_cnt[tune_tx_value] = 0;
		MSDC_SET_FIELD(EMMC50_PAD_CMD_TUNE, MSDC_EMMC50_PAD_CMD_TUNE_TXDLY, tune_tx_value);
		for (tune_cnt = 0; tune_cnt < TUNE_TX_CNT; tune_cnt++) {
			ret = autok_send_tune_cmd(host, MMC_SEND_STATUS, TUNE_CMD);
			if ((ret & E_RESULT_CMD_TMO) != 0)
				tune_tmo_cnt[tune_tx_value]++;
			else if ((ret&(E_RESULT_RSP_CRC)) != 0)
				tune_crc_cnt[tune_tx_value]++;
			else if ((ret & (E_RESULT_PASS)) == 0)
				tune_pass_cnt[tune_tx_value]++;
		}

		/* UTIL_Printf("[AUTOK]tune_cmd_TX cmd_tx_value = %d tmo_cnt = %d crc_cnt = %d pass_cnt = %d\n",
			tune_tx_value, tune_tmo_cnt[tune_tx_value],tune_crc_cnt[tune_tx_value],
			tune_pass_cnt[tune_tx_value]); */
	}

	/* print result */
	for (i = 0; i < 32; i++) {
		if ((tune_tmo_cnt[i] != 0) || (tune_crc_cnt[i] != 0))
			tune_result[i] = 'X';
		else if (tune_pass_cnt[i] == TUNE_TX_CNT)
			tune_result[i] = 'O';
	}
	tune_result[32] = '\0';
	UTIL_Printf("[AUTOK]tune_cmd_TX 0 - 31      %s\r\n", tune_result);
	UTIL_Printf("[AUTOK][tune cmd TX]=========end========\r\n");

	/* restore cmd tx setting */
	MSDC_SET_FIELD(EMMC50_PAD_CMD_TUNE, MSDC_EMMC50_PAD_CMD_TUNE_TXDLY, cmd_tx);
	UTIL_Printf("[AUTOK][tune data TX]=========start========\r\n");

	/* Step2 : Tuning Data TX */
	for (tune_tx_value = 0; tune_tx_value < 32; tune_tx_value++) {
		tune_tmo_cnt[tune_tx_value] = 0;
		tune_crc_cnt[tune_tx_value] = 0;
		tune_pass_cnt[tune_tx_value] = 0;
		MSDC_SET_FIELD(EMMC50_PAD_DAT01_TUNE, MSDC_EMMC50_PAD_DAT0_TXDLY, tune_tx_value);
		MSDC_SET_FIELD(EMMC50_PAD_DAT01_TUNE, MSDC_EMMC50_PAD_DAT1_TXDLY, tune_tx_value);
		MSDC_SET_FIELD(EMMC50_PAD_DAT23_TUNE, MSDC_EMMC50_PAD_DAT2_TXDLY, tune_tx_value);
		MSDC_SET_FIELD(EMMC50_PAD_DAT23_TUNE, MSDC_EMMC50_PAD_DAT3_TXDLY, tune_tx_value);
		MSDC_SET_FIELD(EMMC50_PAD_DAT45_TUNE, MSDC_EMMC50_PAD_DAT4_TXDLY, tune_tx_value);
		MSDC_SET_FIELD(EMMC50_PAD_DAT45_TUNE, MSDC_EMMC50_PAD_DAT5_TXDLY, tune_tx_value);
		MSDC_SET_FIELD(EMMC50_PAD_DAT67_TUNE, MSDC_EMMC50_PAD_DAT6_TXDLY, tune_tx_value);
		MSDC_SET_FIELD(EMMC50_PAD_DAT67_TUNE, MSDC_EMMC50_PAD_DAT7_TXDLY, tune_tx_value);

		for (tune_cnt = 0; tune_cnt < TUNE_TX_CNT / 5; tune_cnt++) {
			/* check device status */
			response = 0;
			while (((response >> 9) & 0xF) != 4) {
				ret = autok_send_tune_cmd(host, MMC_SEND_STATUS, TUNE_CMD);
				if ((ret & (E_RESULT_RSP_CRC | E_RESULT_CMD_TMO)) != 0) {
					UTIL_Printf("[AUTOK]------while tune data TX cmd13 occur error------\r\n");
					UTIL_Printf("[AUTOK]------tune data TX fail------\r\n");
					goto end;
				}
				response = MSDC_READ32(SDC_RESP0);
				if ((((response >> 9) & 0xF) == 5) || (((response >> 9) & 0xF) == 6))
					ret = autok_send_tune_cmd(host, MMC_STOP_TRANSMISSION, TUNE_CMD);
			}

			/* send cmd24 write one block data */
			ret = autok_send_tune_cmd(host, MMC_WRITE_BLOCK, TUNE_DATA);
			response = MSDC_READ32(SDC_RESP0);
			if ((ret & (E_RESULT_RSP_CRC | E_RESULT_CMD_TMO)) != 0) {
				UTIL_Printf("[AUTOK]------while tune data TX cmd%d occur error------\n",
					MMC_WRITE_BLOCK);
				UTIL_Printf("[AUTOK]------tune data TX fail------\n");
				goto end;
			}
			if ((ret & E_RESULT_DAT_TMO) != 0)
				tune_tmo_cnt[tune_tx_value]++;
			else if ((ret & (E_RESULT_DAT_CRC)) != 0)
				tune_crc_cnt[tune_tx_value]++;
			else if ((ret & (E_RESULT_PASS)) == 0)
				tune_pass_cnt[tune_tx_value]++;
		}

		/* UTIL_Printf("[AUTOK]tune_data_TX data_tx_value = %d tmo_cnt = %d crc_cnt = %d pass_cnt = %d\n",
			tune_tx_value, tune_tmo_cnt[tune_tx_value],tune_crc_cnt[tune_tx_value],
			tune_pass_cnt[tune_tx_value]); */
	}

	/* print result */
	for (i = 0; i < 32; i++) {
		if ((tune_tmo_cnt[i] != 0) || (tune_crc_cnt[i] != 0))
			tune_result[i] = 'X';
		else if (tune_pass_cnt[i] == (TUNE_TX_CNT / 5))
			tune_result[i] = 'O';
	}
	tune_result[32] = '\0';
	UTIL_Printf("[AUTOK]tune_data_TX 0 - 31      %s\r\n", tune_result);

	/* restore data tx setting */
	MSDC_SET_FIELD(EMMC50_PAD_DAT01_TUNE, MSDC_EMMC50_PAD_DAT0_TXDLY, dat0_tx);
	MSDC_SET_FIELD(EMMC50_PAD_DAT01_TUNE, MSDC_EMMC50_PAD_DAT1_TXDLY, dat1_tx);
	MSDC_SET_FIELD(EMMC50_PAD_DAT23_TUNE, MSDC_EMMC50_PAD_DAT2_TXDLY, dat2_tx);
	MSDC_SET_FIELD(EMMC50_PAD_DAT23_TUNE, MSDC_EMMC50_PAD_DAT3_TXDLY, dat3_tx);
	MSDC_SET_FIELD(EMMC50_PAD_DAT45_TUNE, MSDC_EMMC50_PAD_DAT4_TXDLY, dat4_tx);
	MSDC_SET_FIELD(EMMC50_PAD_DAT45_TUNE, MSDC_EMMC50_PAD_DAT5_TXDLY, dat5_tx);
	MSDC_SET_FIELD(EMMC50_PAD_DAT67_TUNE, MSDC_EMMC50_PAD_DAT6_TXDLY, dat6_tx);
	MSDC_SET_FIELD(EMMC50_PAD_DAT67_TUNE, MSDC_EMMC50_PAD_DAT7_TXDLY, dat7_tx);

	UTIL_Printf("[AUTOK][tune data TX]=========end========\r\n");
end:
	return ret;
}

int autok_offline_tuning_clk_TX(struct msdc_host *host, unsigned int opcode)
{
	int ret = 0, result = 0;
	void __iomem *base = host->base;
	unsigned int response;
	unsigned int data_pin_status = 0xff;
	unsigned int tune_tx_value;
	unsigned char tune_cnt;
	unsigned char i;
	unsigned char tune_crc_cnt[32];
	unsigned char tune_pass_cnt[32];
	unsigned char tune_tmo_cnt[32];
	char tune_result[33];
	unsigned int clk_tx;
	struct timeval t_start, t_end;
	unsigned int host_tx_sel;
	u64 Tx64 = 0LL;
	struct AUTOK_REF_INFO_NEW uTxInfo;
	struct AUTOK_SCAN_RES_NEW *pInfo;

	memset(&uTxInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));
	pInfo = (struct AUTOK_SCAN_RES_NEW *)&(uTxInfo.scan_info[0]);
	Tx64 = 0LL;

	UTIL_Printf("[AUTOK][tune clk TX]=========start========\r\n");
	/* store tx setting */
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, clk_tx);

	/* Step1 : Tuning Clk TX */
	for (tune_tx_value = 0; tune_tx_value < 32; tune_tx_value++) {
		tune_tmo_cnt[tune_tx_value] = 0;
		tune_crc_cnt[tune_tx_value] = 0;
		tune_pass_cnt[tune_tx_value] = 0;
		MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, tune_tx_value);
		for (tune_cnt = 0; tune_cnt < TUNE_TX_CNT; tune_cnt++) {
			if (host->id != 0) {
				ret = autok_send_tune_cmd(host, SD_CMD_SEND_TUNING_BLOCK, TUNE_CMD);
				do_gettimeofday(&t_start);
				do_gettimeofday(&t_end);
				while ((t_end.tv_usec - t_start.tv_usec) <= 10)
					do_gettimeofday(&t_end);
			}
			if ((ret & E_RESULT_CMD_TMO) != 0) {
				autok_msdc_reset();
				msdc_clear_fifo();
				MSDC_WRITE32(MSDC_INT, 0xffffffff);
				tune_tmo_cnt[tune_tx_value]++;
			} else if ((ret&(E_RESULT_RSP_CRC)) != 0)
				tune_crc_cnt[tune_tx_value]++;
			else if ((ret & (E_RESULT_PASS)) == 0)
				tune_pass_cnt[tune_tx_value]++;
		}
#if AUTOK_DEBUG_LOG
		UTIL_Printf("[AUTOK]tune_cmd_TX cmd_tx_value = %d tmo_cnt = %d crc_cnt = %d pass_cnt = %d\r\n",
			tune_tx_value, tune_tmo_cnt[tune_tx_value],tune_crc_cnt[tune_tx_value],
			tune_pass_cnt[tune_tx_value]);
#endif
	}

	/* print result */
	for (i = 0; i < 32; i++) {
		if (tune_tmo_cnt[i] != 0)
			tune_result[i] = 'X';
		else if (tune_crc_cnt[i] != 0)
			tune_result[i] = 'R';
		else if (tune_pass_cnt[i] == TUNE_TX_CNT)
			tune_result[i] = 'O';
	}
	tune_result[32] = '\0';
	UTIL_Printf("[AUTOK]tune_clk_TX 0 - 31   %s\r\n", tune_result);
	UTIL_Printf("[AUTOK][tune clk TX]=========end========\r\n");

	/* restore clk tx setting */
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, clk_tx);
	UTIL_Printf("[AUTOK][tune clk TX @write data]=========start========\r\n");

	/* Step2 : Tuning clk TX @ write data */
	for (tune_tx_value = 0; tune_tx_value < 32; tune_tx_value++) {
		tune_tmo_cnt[tune_tx_value] = 0;
		tune_crc_cnt[tune_tx_value] = 0;
		tune_pass_cnt[tune_tx_value] = 0;
		host->clk_tx = tune_tx_value;
		for (tune_cnt = 0; tune_cnt < TUNE_TX_CNT / 5; tune_cnt++) {
			/* check device status */
			if (opcode == MSDC_SD_CLK_TX_TUNE) {
				MSDC_GET_FIELD(MSDC_PS, MSDC_PS_DAT, data_pin_status);
				if (!(data_pin_status & 0x1)) {
					autok_send_tune_cmd(host, MMC_STOP_TRANSMISSION, TUNE_CMD);
					while (!(data_pin_status & 0x1)) {
						MSDC_GET_FIELD(MSDC_PS, MSDC_PS_DAT, data_pin_status);
					}
				}
			} else if (opcode == MSDC_SDIO_CLK_TX_TUNE) {
				;
			}
			/* send cmd24 write one block data */
			ret = autok_send_tune_cmd(host, opcode, TUNE_DATA);
			response = MSDC_READ32(SDC_RESP0);
			if ((ret & (E_RESULT_RSP_CRC | E_RESULT_CMD_TMO)) != 0) {
				autok_msdc_reset();
				msdc_clear_fifo();
				MSDC_WRITE32(MSDC_INT, 0xffffffff);
				UTIL_Printf("[AUTOK]------while tune clk TX cmd%d occur error------\n",
					MMC_WRITE_BLOCK);
				UTIL_Printf("[AUTOK]------tune clk TX fail------\n");
				result = -1;
				goto end;
			}
			if ((ret & E_RESULT_DAT_TMO) != 0) {
				tune_tmo_cnt[tune_tx_value]++;
				Tx64 |= (u64) (1LL << tune_tx_value);
			} else if ((ret & (E_RESULT_DAT_CRC)) != 0) {
				tune_crc_cnt[tune_tx_value]++;
				Tx64 |= (u64) (1LL << tune_tx_value);
			} else if ((ret & (E_RESULT_PASS)) == 0)
				tune_pass_cnt[tune_tx_value]++;
		}
#if AUTOK_DEBUG_LOG
		UTIL_Printf("[AUTOK]tune_data_TX data_tx_value = %d tmo_cnt = %d crc_cnt = %d pass_cnt = %d\r\n",
			tune_tx_value, tune_tmo_cnt[tune_tx_value],tune_crc_cnt[tune_tx_value],
			tune_pass_cnt[tune_tx_value]);
#endif
	}

	/* print result */
	for (i = 0; i < 32; i++) {
		if ((tune_tmo_cnt[i] != 0) || (tune_crc_cnt[i] != 0))
			tune_result[i] = 'X';
		else if (tune_pass_cnt[i] == (TUNE_TX_CNT / 5))
			tune_result[i] = 'O';
	}
	tune_result[32] = '\0';
	UTIL_Printf("[AUTOK]tune_clk_TX 0 - 31 	 %s\r\n", tune_result);
	/* select a best dat rx setting */
	Tx64 |= 0xffffffff00000000;
	autok_check_scan_res64_new(Tx64, pInfo, 0);
	autok_ds_dly_sel(pInfo, &host_tx_sel);
	UTIL_Printf("[AUTOK]tune device data RX sel:%d\r\n", host_tx_sel);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, host_tx_sel);
	goto success;
end:
	/* restore clk tx setting */
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, clk_tx);
success:
	UTIL_Printf("[AUTOK][tune clk TX @write data]=========end========\r\n");
	return result;
}

void autok_msdc_device_rx_set(struct msdc_host *host, unsigned int
	cmd_tx, unsigned int data_p_tx, unsigned int data_n_tx)
{
	void __iomem *base = host->base;
	unsigned int ret = E_RESULT_PASS;
	unsigned int base_addr = 0;
	unsigned int func_num = 0;
	unsigned int reg_value = 0;
	unsigned int r_w_dirc = 0;
	unsigned int i;

#if 0
	UTIL_Printf("[AUTOK]DRS device RX set\r\n");
#endif
	/* write rx setting */
	base_addr = 0x11c;
	func_num = 0x1;
	if (cmd_tx == 0)
		reg_value = 0;
	else
		reg_value = (1 << 7) + cmd_tx;
	r_w_dirc = EXT_WRITE;
	ret = autok_sdio_device_rx_set(host, func_num, base_addr,
		&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
	if (ret != E_RESULT_PASS)
		UTIL_Printf("[AUTOK]DRS reg 0x%x set fail\r\n", base_addr);

	for (i = 0; i < 4; i++) {
		base_addr = 0x124 + i;
		func_num = 0x1;
		if (data_p_tx == 0)
			reg_value = 0;
		else
			reg_value = data_p_tx + (1 << 7);
		r_w_dirc = EXT_WRITE;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]DRS reg 0x%x set fail\r\n", base_addr);
	}

	for (i = 0; i < 4; i++) {
		base_addr = 0x128 + i;
		func_num = 0x1;
		if (data_n_tx == 0)
			reg_value = 0;
		else
			reg_value = data_n_tx + (1 << 7);
		r_w_dirc = EXT_WRITE;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]DRS reg 0x%x set fail\r\n", base_addr);
	}
	/* read back setting check */
	base_addr = 0x11c;
	func_num = 0x1;
	r_w_dirc = EXT_READ;
	ret = autok_sdio_device_rx_set(host, func_num, base_addr,
		&cmd_tx, r_w_dirc, 4, SD_IO_RW_DIRECT);
	cmd_tx = MSDC_READ32(SDC_RESP0) & 0xFF;
	if (ret != E_RESULT_PASS)
		UTIL_Printf("[AUTOK]DRS reg 0x%x read fail\r\n", base_addr);
	for (i = 0; i < 4; i++) {
		base_addr = 0x124 + i;
		func_num = 0x1;
		r_w_dirc = EXT_READ;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&data_p_tx, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]DRS reg 0x%x set fail\r\n", base_addr);
		else
			data_p_tx = (data_p_tx) | ((MSDC_READ32(SDC_RESP0) & 0xFF) << (i * 8));
	}
	for (i = 0; i < 4; i++) {
		base_addr = 0x128 + i;
		func_num = 0x1;
		r_w_dirc = EXT_READ;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&data_n_tx, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]DRS reg 0x%x set fail\r\n", base_addr);
		else
			data_n_tx = (data_n_tx) | ((MSDC_READ32(SDC_RESP0) & 0xFF) << (i * 8));
	}
}

void autok_msdc_device_rx_get(struct msdc_host *host, unsigned int *cmd_tx,
	unsigned int *data_p_tx, unsigned int *data_n_tx)
{
	void __iomem *base = host->base;
	unsigned int ret = E_RESULT_PASS;
	unsigned int base_addr = 0;
	unsigned int func_num = 0;
	unsigned int r_w_dirc = 0;
	unsigned int i;

	/* read back setting check */
	base_addr = 0x11c;
	func_num = 0x1;
	r_w_dirc = EXT_READ;
	ret = autok_sdio_device_rx_set(host, func_num, base_addr,
		cmd_tx, r_w_dirc, 4, SD_IO_RW_DIRECT);
	if (ret != E_RESULT_PASS)
		UTIL_Printf("[AUTOK]DRS reg 0x%x read fail\r\n", base_addr);
	else
		*cmd_tx = MSDC_READ32(SDC_RESP0) & 0xFF;
	for (i = 0; i < 4; i++) {
		base_addr = 0x124 + i;
		func_num = 0x1;
		r_w_dirc = EXT_READ;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			data_p_tx, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]DRS reg 0x%x set fail\r\n", base_addr);
		else
			*data_p_tx = (*data_p_tx) | ((MSDC_READ32(SDC_RESP0) & 0xFF) << (i * 8));
	}
	for (i = 0; i < 4; i++) {
		base_addr = 0x128 + i;
		func_num = 0x1;
		r_w_dirc = EXT_READ;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			data_n_tx, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]DRS reg 0x%x set fail\r\n", base_addr);
		else
			*data_n_tx = (*data_n_tx) | ((MSDC_READ32(SDC_RESP0) & 0xFF) << (i * 8));
	}
}

int autok_offline_tuning_device_RX(struct msdc_host *host, u8 *res)
{
	int result = 0;
#if AUTOK_OFFLINE_DAT_D_RX_ENABLE
	unsigned int dat_rx_sel = 0;
#endif
#if AUTOK_OFFLINE_CMD_D_RX_ENABLE
	unsigned int cmd_rx_sel = 0;
#endif

#if (AUTOK_OFFLINE_CMD_D_RX_ENABLE || AUTOK_OFFLINE_DAT_D_RX_ENABLE)
	int ret = 0;
	void __iomem *base = host->base;
	unsigned int tune_rx_value;
	unsigned char tune_cnt;
	unsigned char i;
	unsigned char tune_crc_cnt[32];
	unsigned char tune_pass_cnt[32];
	unsigned char tune_tmo_cnt[32];
	char tune_result[33];

	unsigned int check_cnt = 0;
	unsigned int iorx = 0;
	unsigned int base_addr = 0;
	unsigned int func_num = 0;
	unsigned int reg_value = 0;
	unsigned int r_w_dirc = 0;
	unsigned int cmd_rx = 0;
	unsigned int data_p_rx = 0;
	unsigned int data_n_rx = 0;
	u64 Rx64 = 0LL;
	struct AUTOK_REF_INFO_NEW uRxInfo;
	struct AUTOK_SCAN_RES_NEW *pInfo;

	memset(&uRxInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));
	pInfo = (struct AUTOK_SCAN_RES_NEW *)&(uRxInfo.scan_info[0]);
	Rx64 = 0LL;

	UTIL_Printf("[AUTOK]SDIO device function enable\r\n");
	/* read previous device setting */
	base_addr = 0x02;
	func_num = 0x0;
	reg_value = 0;
	r_w_dirc = EXT_READ;
	ret = autok_sdio_device_rx_set(host, func_num, base_addr,
		&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
	if (reg_value & 0x02)
		goto tune_device_rx;
	/* function has not enabled, enable device function1 */
	base_addr = 0x02;
	func_num = 0x0;
	reg_value |= 0x02;
	r_w_dirc = EXT_WRITE;
	ret = autok_sdio_device_rx_set(host, func_num, base_addr,
		&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
	if (ret != E_RESULT_PASS)
		UTIL_Printf("[AUTOK]IOEx reg 0x%x set fail\r\n", base_addr);
	UTIL_Printf("[AUTOK]SDIO device function enable ready check\r\n");
	while ((!(iorx & 0x02)) && (check_cnt < 10)) {
		check_cnt++;
		base_addr = 0x03;
		func_num = 0x0;
		reg_value = 0x00;
		r_w_dirc = EXT_READ;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]IOEx reg 0x%x set fail\r\n", base_addr);
		iorx = MSDC_READ32(SDC_RESP0) & 0xff;
		UTIL_Printf("[AUTOK]iorx 0x%x\r\n", iorx);
	}
tune_device_rx:
	/* store tx setting */
	autok_msdc_device_rx_get(host, &cmd_rx, &data_p_rx, &data_n_rx);
	UTIL_Printf("[AUTOK]pre SDIO cmd rx %x data rx = %x %x\r\n",
		cmd_rx, data_p_rx, data_n_rx);
#endif
#if AUTOK_OFFLINE_CMD_D_RX_ENABLE
	/*  Tuning Cmd TX */
	UTIL_Printf("[AUTOK][tune device cmd RX]=========start========\r\n");
	/* Step1 : Tuning Cmd TX */
	for (tune_rx_value = 0; tune_rx_value < 32; tune_rx_value++) {
		tune_tmo_cnt[tune_rx_value] = 0;
		tune_crc_cnt[tune_rx_value] = 0;
		tune_pass_cnt[tune_rx_value] = 0;
		autok_msdc_device_rx_set(host, tune_rx_value, data_p_rx & 0x1f, data_n_rx & 0x1f);
		for (tune_cnt = 0; tune_cnt < TUNE_TX_CNT; tune_cnt++) {
			ret = autok_send_tune_cmd(host, SD_CMD_SEND_TUNING_BLOCK, TUNE_DATA);
			if ((ret & E_RESULT_CMD_TMO) != 0) {
				tune_tmo_cnt[tune_rx_value]++;
				Rx64 |= (u64) (1LL << tune_rx_value);
			} else if ((ret&(E_RESULT_RSP_CRC)) != 0) {
				tune_crc_cnt[tune_rx_value]++;
				Rx64 |= (u64) (1LL << tune_rx_value);
			} else if ((ret & (E_RESULT_PASS)) == 0)
				tune_pass_cnt[tune_rx_value]++;
		}
#if 0
		UTIL_Printf("[AUTOK]tune_cmd_RX cmd_rx_value = %d tmo_cnt = %d crc_cnt = %d pass_cnt = %d\n",
			tune_rx_value, tune_tmo_cnt[tune_rx_value], tune_crc_cnt[tune_rx_value],
			tune_pass_cnt[tune_rx_value]);
#endif
	}

	/* print result */
	for (i = 0; i < 32; i++) {
		if ((tune_tmo_cnt[i] != 0) || (tune_crc_cnt[i] != 0))
			tune_result[i] = 'X';
		else if (tune_pass_cnt[i] == TUNE_TX_CNT)
			tune_result[i] = 'O';
	}
	tune_result[32] = '\0';
	UTIL_Printf("[AUTOK]tune_device_cmd_RX 0 - 31      %s\r\n", tune_result);
	/* select a best cmd rx setting, default setting may can not work */
	Rx64 |= 0xffffffff00000000;
	autok_check_scan_res64_new(Rx64, pInfo, 0);
	autok_ds_dly_sel(pInfo, &cmd_rx_sel);
	UTIL_Printf("[AUTOK]tune device cmd RX sel:%d\r\n", cmd_rx_sel);
	autok_msdc_device_rx_set(host, cmd_rx_sel, 0, 0);
	UTIL_Printf("[AUTOK][tune device cmd RX]=========end========\r\n");
	if (res != NULL)
		msdc_autok_window_apply(D_CMD_RX, Rx64, res);
#endif
#if AUTOK_OFFLINE_DAT_D_RX_ENABLE
	UTIL_Printf("[AUTOK][tune device data RX]=========start========\r\n");
	memset(&uRxInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));
	pInfo = (struct AUTOK_SCAN_RES_NEW *)&(uRxInfo.scan_info[0]);
	Rx64 = 0LL;
	/*  Tuning Data TX */
	for (tune_rx_value = 0; tune_rx_value < 32; tune_rx_value++) {
		tune_tmo_cnt[tune_rx_value] = 0;
		tune_crc_cnt[tune_rx_value] = 0;
		tune_pass_cnt[tune_rx_value] = 0;

		autok_msdc_device_rx_set(host, cmd_rx & 0x1f, tune_rx_value, tune_rx_value);
		for (tune_cnt = 0; tune_cnt < TUNE_TX_CNT; tune_cnt++) {
			/* send cmd53 write data */
			base_addr = 0xB0;
			func_num = 0x1;
			reg_value = 0x00;
			r_w_dirc = EXT_WRITE;
			ret = autok_sdio_device_rx_set(host, func_num, base_addr,
				&reg_value, r_w_dirc, 64, SD_IO_RW_EXTENDED);
			if ((ret & (E_RESULT_RSP_CRC | E_RESULT_CMD_TMO)) != 0) {
				UTIL_Printf("[AUTOK]tune data RX cmd%d occur error\n",
					SD_IO_RW_EXTENDED);
				UTIL_Printf("[AUTOK]tune data RX fail\n");
				result = -1;
				goto end;
			}
			if ((ret & E_RESULT_DAT_TMO) != 0) {
				tune_tmo_cnt[tune_rx_value]++;
				Rx64 |= (u64) (1LL << tune_rx_value);
				/* send CMD52 abort command */
				autok_send_tune_cmd(host, SD_IO_RW_DIRECT, TUNE_CMD);
			} else if ((ret & (E_RESULT_DAT_CRC)) != 0) {
				tune_crc_cnt[tune_rx_value]++;
				Rx64 |= (u64) (1LL << tune_rx_value);
				/* send CMD52 abort command */
				autok_send_tune_cmd(host, SD_IO_RW_DIRECT, TUNE_CMD);
			} else if ((ret & (E_RESULT_PASS)) == 0)
				tune_pass_cnt[tune_rx_value]++;
		}
#if 0
		UTIL_Printf("[AUTOK]tune_data_RX data_rx_value = %d tmo_cnt = %d crc_cnt = %d pass_cnt = %d\n",
			tune_rx_value, tune_tmo_cnt[tune_rx_value], tune_crc_cnt[tune_rx_value],
			tune_pass_cnt[tune_rx_value]);
#endif
	}

	/* print result */
	for (i = 0; i < 32; i++) {
		if ((tune_tmo_cnt[i] != 0) || (tune_crc_cnt[i] != 0))
			tune_result[i] = 'X';
		else if (tune_pass_cnt[i] == TUNE_TX_CNT)
			tune_result[i] = 'O';
	}
	tune_result[32] = '\0';
	UTIL_Printf("[AUTOK]tune_device_data_RX 0 - 31      %s\r\n", tune_result);
	/* select a best dat rx setting */
	Rx64 |= 0xffffffff00000000;
	autok_check_scan_res64_new(Rx64, pInfo, 0);
	autok_ds_dly_sel(pInfo, &dat_rx_sel);
	UTIL_Printf("[AUTOK]tune device data RX sel:%d\r\n", dat_rx_sel);
	/* restore data rx setting */
	//dat_rx_sel = 0;
	dat_rx_sel = (dat_rx_sel <= 16) ? (dat_rx_sel << 1) : 31;
	UTIL_Printf("[AUTOK]tune device data RX final sel:%d\r\n", dat_rx_sel);
	autok_msdc_device_rx_set(host, cmd_rx & 0x1f, dat_rx_sel & 0x1f, dat_rx_sel & 0x1f);
	//autok_msdc_device_rx_set(host, cmd_rx & 0x1f, data_p_rx & 0x1f, data_n_rx & 0x1f);
	UTIL_Printf("[AUTOK][tune device data RX]=========end========\r\n");
	if (res != NULL)
		msdc_autok_window_apply(D_DATA_RX, Rx64, res);
end:
#endif
	return result;
}

int autok_offline_tuning_cmd_latch_en(struct msdc_host *host, enum MODE_TYPE mode_type, unsigned int opcode)
{
	void __iomem *base = host->base;
	unsigned int ret = 0;
	unsigned int uEdge = 0;
	unsigned long RawCmd64 = 0LL;
	unsigned int score = 0;
	unsigned int i, j, k ;
	unsigned int cmd_latch_en;
	unsigned int cmd_edge;
	unsigned int cmd_en1, cmd_en2;
	unsigned int cmd_dly1, cmd_dly2;
	struct AUTOK_REF_INFO_NEW uCmdInfo;
	struct AUTOK_SCAN_RES_NEW *pBdCmdInfo;
	char tune_result_str64[65];

	UTIL_Printf("[AUTOK]tune cmd latch en start\r\n");
	MSDC_GET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_RESPSTENSEL, cmd_latch_en);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cmd_edge);
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CMDRDLY, cmd_dly1);
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CMDRRDLYSEL, cmd_en1);
	MSDC_GET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_CMDRDLY2, cmd_dly2);
	MSDC_GET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_CMDRRDLY2SEL, cmd_en2);

	if (mode_type == HS200_TUNE)
		autok_init_hs200(host);
	else if (mode_type == HS400_TUNE)
		autok_init_hs400(host);
	else if (mode_type == SDR104_TUNE)
		autok_init_sdr104(host);

	memset(&uCmdInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));
	for (i = 0; i < 8; i++) {
		MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_RESPSTENSEL, i);
		uEdge = 0;
		do {
			pBdCmdInfo = (struct AUTOK_SCAN_RES_NEW *)&(uCmdInfo.scan_info[uEdge]);
			msdc_autok_adjust_param(host, CMD_EDGE, &uEdge, AUTOK_WRITE);
			RawCmd64 = 0LL;
			for (j = 0; j < 64; j++) {
				msdc_autok_adjust_paddly(host, &j, CMD_PAD_RDLY);
				for (k = 0; k < AUTOK_CMD_TIMES / 2; k++) {
					ret = autok_send_tune_cmd(host, opcode, TUNE_CMD);
					if ((ret & E_RESULT_RSP_CRC) != 0)
						RawCmd64 |= (unsigned long) (1LL << j);
					else if ((ret & E_RESULT_CMD_TMO) != 0) {
						autok_msdc_reset();
						msdc_clear_fifo();
						MSDC_WRITE32(MSDC_INT, 0xffffffff);
						RawCmd64 |= (unsigned long) (1LL << j);
					} else if ((ret & E_RESULT_FATAL_ERR) != 0)
						return -1;
				}
			}
			score = autok_simple_score64(tune_result_str64, RawCmd64);
			UTIL_Printf("[AUTOK]CMD %d \t %d \t %s\r\n", uEdge, score,
					   tune_result_str64);

			uEdge ^= 0x1;
		} while (uEdge);
	}

	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_RESPSTENSEL, cmd_latch_en);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cmd_edge);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CMDRDLY, cmd_dly1);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CMDRRDLYSEL, cmd_en1);
	MSDC_SET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_CMDRDLY2, cmd_dly2);
	MSDC_SET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_CMDRRDLY2SEL, cmd_en2);

	UTIL_Printf("[AUTOK]tune cmd latch en end\r\n");

	return 0;
}

int autok_offline_tuning_cts_latch_en(struct msdc_host *host, enum MODE_TYPE mode_type, unsigned int opcode)
{
	void __iomem *base = host->base;
	unsigned int ret = 0;
	unsigned int uEdge = 0;
	unsigned long RawData64 = 0LL;
	unsigned int score = 0;
	unsigned int i, j, k ;
	unsigned int crc_latch_en;
	unsigned int wr_edge;
	unsigned int dat_en1, dat_en2;
	unsigned int dat_dly1, dat_dly2;
	struct AUTOK_REF_INFO_NEW uDatInfo;
	struct AUTOK_SCAN_RES_NEW *pBdDatInfo;
	char tune_result_str64[65];
	unsigned int response;
	unsigned int data_pin_status = 0xff;

	unsigned int base_addr = 0;
	unsigned int func_num = 0;
	unsigned int reg_value = 0;
	unsigned int r_w_dirc = 0;

	UTIL_Printf("[AUTOK]tune crc status latch en start\r\n");

	MSDC_GET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CRCSTSENSEL, crc_latch_en);
	MSDC_GET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CFGCRCSTSEDGE, wr_edge);
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_DATRRDLY, dat_dly1);
	MSDC_GET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_DATRRDLYSEL, dat_en1);
	MSDC_GET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_DATRRDLY2, dat_dly2);
	MSDC_GET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_DATRRDLY2SEL, dat_en2);

	if (mode_type == HS200_TUNE)
		autok_init_hs200(host);
	else if (mode_type == HS400_TUNE) {
		autok_init_hs400(host);
		/* write path switch to emmc4.5 */
		//autok_write_param(host, EMMC50_WDATA_MUX_EN, 0);
		/* Specifical for HS400 Path Sel */
		//autok_write_param(host, MSDC_WCRC_ASYNC_FIFO_SEL, 1);
	} else if (mode_type == SDR104_TUNE)
		autok_init_sdr104(host);

	memset(&uDatInfo, 0, sizeof(struct AUTOK_REF_INFO_NEW));
	for (i = 0; i < 8; i++) {
		MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CRCSTSENSEL, i);
		uEdge = 0;
		do {
			pBdDatInfo = (struct AUTOK_SCAN_RES_NEW *)&(uDatInfo.scan_info[uEdge]);
			msdc_autok_adjust_param(host, WD_FIFO_EDGE, &uEdge, AUTOK_WRITE);
			RawData64 = 0LL;
			for (j = 0; j < 64; j++) {
				msdc_autok_adjust_paddly(host, &j, DAT_PAD_RDLY);
				for (k = 0; k < AUTOK_CMD_TIMES / 5; k++) {
					/* check device status */
					if (mode_type == SDR104_TUNE) {
						if (opcode == MMC_WRITE_BLOCK) {
							autok_send_tune_cmd(host, MMC_STOP_TRANSMISSION, TUNE_CMD);
							while (!(data_pin_status | 0x1)) {
								MSDC_GET_FIELD(MSDC_PS, MSDC_PS_DAT, data_pin_status);
							}
						} else if (opcode == SD_IO_RW_EXTENDED) {
							;
						}
					} else {
						response = 0;
						while (((response >> 9) & 0xF) != 4) {
							ret = autok_send_tune_cmd(host, MMC_SEND_STATUS, TUNE_CMD);
							if ((ret & (E_RESULT_RSP_CRC | E_RESULT_CMD_TMO)) != 0) {
								UTIL_Printf("[AUTOK]------while tune data TX cmd13 occur error------\r\n");
								UTIL_Printf("[AUTOK]------tune data TX fail------\r\n");
								goto end;
							}
							response = MSDC_READ32(SDC_RESP0);
							if ((((response >> 9) & 0xF) == 5) || (((response >> 9) & 0xF) == 6))
								ret = autok_send_tune_cmd(host, MMC_STOP_TRANSMISSION, TUNE_CMD);
						}
					}
					if (host->id == 3) {
						base_addr = 0xB8;
						func_num = 0x2;
						reg_value = 0x00;
						r_w_dirc = EXT_WRITE;
						ret = autok_sdio_device_rx_set(host, func_num, base_addr,
							&reg_value, r_w_dirc, 64, SD_IO_RW_EXTENDED);
					} else
						ret = autok_send_tune_cmd(host, opcode, TUNE_DATA);
					if ((ret & (E_RESULT_RSP_CRC | E_RESULT_CMD_TMO)) != 0) {
						autok_msdc_reset();
						msdc_clear_fifo();
						MSDC_WRITE32(MSDC_INT, 0xffffffff);
						UTIL_Printf("[AUTOK]Error Autok CMD Failed while tune DAT Delay\r\n");
						goto end;
					} else if ((ret & E_RESULT_FATAL_ERR) != 0)
						goto end;
					if ((ret & (E_RESULT_DAT_CRC | E_RESULT_DAT_TMO)) != 0)
						RawData64 |= (unsigned long) (1LL << j);
					else if ((ret & E_RESULT_FATAL_ERR) != 0)
						goto end;
				}
			}
			score = autok_simple_score64(tune_result_str64, RawData64);
			UTIL_Printf("[AUTOK]DAT %d \t %d \t %s\r\n", uEdge, score,
					   tune_result_str64);

			uEdge ^= 0x1;
		} while (uEdge);
	}
end:
	if (mode_type == HS400_TUNE) {
		/* write path switch to emmc50 */
		//autok_write_param(host, EMMC50_WDATA_MUX_EN, 1);
		/* Specifical for HS400 Path Sel */
		//autok_write_param(host, MSDC_WCRC_ASYNC_FIFO_SEL, 0);
	}
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CRCSTSENSEL, crc_latch_en);
	MSDC_SET_FIELD(MSDC_PATCH_BIT2, MSDC_PB2_CFGCRCSTSEDGE, wr_edge);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_DATRRDLY, dat_dly1);
	MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_DATRRDLYSEL, dat_en1);
	MSDC_SET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_DATRRDLY2, dat_dly2);
	MSDC_SET_FIELD(MSDC_PAD_TUNE1, MSDC_PAD_TUNE1_DATRRDLY2SEL, dat_en2);
	UTIL_Printf("[AUTOK]tune crc status latch en end\r\n");

	return 0;
}
int ddr50_execute_tuning(struct msdc_host *host, unsigned char *res)
{
	int ret = 0;
	unsigned int clk_pwdn = 0;
	unsigned int int_en = 0;
	unsigned long base = host->base;
	unsigned char autok_tune_res[TUNING_PARAM_COUNT];
	unsigned int i = 0;
	unsigned int value = 0;
#if AUTOK_DEBUG_TIME
	struct timeval t_start, t_end;
	do_gettimeofday(&t_start);
#endif

	int_en = MSDC_READ32(MSDC_INTEN);
	MSDC_WRITE32(MSDC_INTEN, 0);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, 1);
	autok_reset_err_counter(host);

	/* store pre autok parameter */
	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		msdc_autok_adjust_param(host, i, &value, AUTOK_READ);
		autok_tune_res[i] = value;
	}
	if (host->id == 3)
		ret = execute_ddr50_online_tuning(host, res, SD_IO_RW_EXTENDED);
	else
		ret = execute_ddr50_online_tuning(host, res, MMC_READ_SINGLE_BLOCK);

	if (ret != 0) {
		UTIL_Printf("[AUTOK] ========Error: Autok Failed========\r\n");
		UTIL_Printf("[AUTOK] ========restore pre autok parameters========\r\n");
		/* restore pre autok parameter */
		for (i = 0; i < TUNING_PARAM_COUNT; i++) {
			value = (unsigned char) autok_tune_res[i];
			msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
		}
	}
	else
		autok_save_result(host);
	
	autok_reset_err_counter(host);
	autok_msdc_reset();
	msdc_clear_fifo();
	MSDC_WRITE32(MSDC_INT, 0xffffffff);
	MSDC_WRITE32(MSDC_INTEN, int_en);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);
#if AUTOK_DEBUG_TIME
	do_gettimeofday(&t_end);
	if((t_end.tv_usec<t_start.tv_usec)&&(t_end.tv_sec>t_start.tv_sec))
		UTIL_Printf("[DDR50/52 AUTOK]=========Time Cost:%d us========\n",
			1000000+t_end.tv_usec-t_start.tv_usec);
	else
		UTIL_Printf("[DDR50/52 AUTOK]=========Time Cost:%d us========\n",
			t_end.tv_usec-t_start.tv_usec);
#endif

	return ret;
}

int autok_execute_tuning(struct msdc_host *host, unsigned char *res)
{
	int ret = 0;
	unsigned int clk_pwdn = 0;
	unsigned int int_en = 0;
	void __iomem *base = host->base;
	unsigned char autok_tune_res[TUNING_PARAM_COUNT];
	unsigned int i = 0;
	unsigned int value = 0;
	unsigned int check_cnt = 0;
	unsigned int iorx = 0;
	unsigned int base_addr = 0;
	unsigned int func_num = 0;
	unsigned int reg_value = 0;
	unsigned int r_w_dirc = 0;
	unsigned int cmd_rx = 0;
	unsigned int data_p_rx = 0;
	unsigned int data_n_rx = 0;
	unsigned int tune_tx_rx_done = 0;
	unsigned int clk_mode;
	unsigned int rx_re_k = 0;

#if AUTOK_DEBUG_TIME
	struct timeval t_start, t_end;
	do_gettimeofday(&t_start);
#endif
	int_en = MSDC_READ32(MSDC_INTEN);
	MSDC_WRITE32(MSDC_INTEN, 0);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, 1);
	autok_reset_err_counter(host);

	/* store pre autok parameter */
	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		msdc_autok_adjust_param(host, i, &value, AUTOK_READ);
		autok_tune_res[i] = value;
	}

	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, clk_mode);
	if ((clk_mode == 1) && (host->id != 3)) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, host->clk_tx);
	} else if ((clk_mode == 1) && (host->id == 3)) {
#if (AUTOK_OFFLINE_TUNE_DEVICE_RX_ENABLE && AUTOK_OFFLINE_TUNE_CLK_TX_ENABLE)
		MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, 0);
#else
#ifndef AUTOK_UPDATE_DISABLE
		MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, host->clk_tx);
#endif
		if(!res){
				ret = execute_online_tuning(host, res, SD_CMD_SEND_TUNING_BLOCK);
				if (ret != 0) {
					UTIL_Printf("[AUTOK] ========Error: Autok Failed========\r\n");
					UTIL_Printf("[AUTOK] ========restore pre autok parameters========\r\n");
					/* restore pre autok parameter */
					for (i = 0; i < TUNING_PARAM_COUNT; i++) {
						value = (unsigned char) autok_tune_res[i];
						msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
						}
						goto end;
					}
				else{
					autok_save_result(host);
					#if MSDC_AUTOK_TO_MTZ
					if(host->hw->host_function==MSDC_SDIO)
						kthread_run(autok_save_to_metazone, (void *)host,"sdr104_autok");
					#endif
					}
					
			}
		else
			autok_restore_result(host,AUTOK_SDR104_PARAM);
		rx_re_k = 1;
#endif
	} else {
		MSDC_SET_FIELD(MSDC_PAD_TUNE0, MSDC_PAD_TUNE0_CLKTXDLY, 0);
	}
#ifdef AUTOK_UPDATE_DISABLE
	goto end;
#endif
	if ((clk_mode == 1) && (host->id == 3)) {
		UTIL_Printf("[AUTOK]SDIO device function enable\r\n");
		/* read previous device setting */
		base_addr = 0x02;
		func_num = 0x0;
		reg_value = 0;
		r_w_dirc = EXT_READ;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (reg_value & 0x02)
			goto device_rx_set;
		/* function has not enabled, enable device function1 */
		base_addr = 0x02;
		func_num = 0x0;
		reg_value |= 0x02;
		r_w_dirc = EXT_WRITE;
		ret = autok_sdio_device_rx_set(host, func_num, base_addr,
			&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
		if (ret != E_RESULT_PASS)
			UTIL_Printf("[AUTOK]IOEx reg 0x%x set fail\r\n", base_addr);
		UTIL_Printf("[AUTOK]SDIO device function enable ready check\r\n");
		while ((!(iorx & 0x02)) && (check_cnt < 10)) {
			check_cnt++;
			base_addr = 0x03;
			func_num = 0x0;
			reg_value = 0x00;
			r_w_dirc = EXT_READ;
			ret = autok_sdio_device_rx_set(host, func_num, base_addr,
				&reg_value, r_w_dirc, 4, SD_IO_RW_DIRECT);
			if (ret != E_RESULT_PASS)
				UTIL_Printf("[AUTOK]IOEx reg 0x%x set fail\r\n", base_addr);
			iorx = MSDC_READ32(SDC_RESP0) & 0xff;
			UTIL_Printf("[AUTOK]iorx 0x%x\r\n", iorx);
		}
	}

	if ((clk_mode == 1) && (host->id == 3)) {
device_rx_set:
#if (AUTOK_OFFLINE_TUNE_DEVICE_RX_ENABLE && AUTOK_OFFLINE_TUNE_CLK_TX_ENABLE)
		autok_msdc_device_rx_set(host, 0, 0 & 0x1f, 0 & 0x1f);
#else
		autok_msdc_device_rx_set(host,6 & 0x1f, 6 & 0x1f, 6 & 0x1f);
#endif
		autok_msdc_device_rx_get(host, &cmd_rx, &data_p_rx, &data_n_rx);
		UTIL_Printf("[AUTOK]SDIO cmd rx %x data rx = %x %x\r\n",
			cmd_rx, data_p_rx, data_n_rx);
	}

#if AUTOK_OFFLINE_TUNE_LATCH_EN_ENABLE
	if (host->id == 3)
		autok_offline_tuning_cmd_latch_en(host, SDR104_TUNE, SD_CMD_SEND_TUNING_BLOCK);
	else
		autok_offline_tuning_cmd_latch_en(host, SDR104_TUNE, MMC_READ_SINGLE_BLOCK);
#endif
#if (AUTOK_OFFLINE_TUNE_DEVICE_RX_ENABLE && AUTOK_OFFLINE_TUNE_CLK_TX_ENABLE)
re_tune_rx:
#endif
	if (!rx_re_k) {
		ret = execute_online_tuning(host, res, SD_CMD_SEND_TUNING_BLOCK);
		if (ret != 0) {
			UTIL_Printf("[AUTOK] ========Error: Autok Failed========\r\n");
			UTIL_Printf("[AUTOK] ========restore pre autok parameters========\r\n");
			/* restore pre autok parameter */
			for (i = 0; i < TUNING_PARAM_COUNT; i++) {
				value = (unsigned char) autok_tune_res[i];
				msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
			}
		} else
			autok_save_result(host);
	}

#if AUTOK_OFFLINE_TUNE_LATCH_EN_ENABLE
		if (host->id == 3)
			autok_offline_tuning_cts_latch_en(host, SDR104_TUNE, SD_IO_RW_EXTENDED);
		else
			autok_offline_tuning_cts_latch_en(host, SDR104_TUNE, MMC_WRITE_BLOCK);
#endif
	
	if (tune_tx_rx_done == 0) {
		tune_tx_rx_done = 1;
#if AUTOK_OFFLINE_TUNE_DEVICE_RX_ENABLE
		if (host->id == 3) {
			ret = autok_offline_tuning_device_RX(host, res);
			if (ret != 0)
				goto end;
		}
#endif
	
#if AUTOK_OFFLINE_TUNE_CLK_TX_ENABLE
		if (host->id == 3)
			autok_offline_tuning_clk_TX(host, MSDC_SDIO_CLK_TX_TUNE);
		else
			autok_offline_tuning_clk_TX(host, MSDC_SD_CLK_TX_TUNE);
		if (ret != 0)
				goto end; 
#endif
#if (AUTOK_OFFLINE_TUNE_DEVICE_RX_ENABLE && AUTOK_OFFLINE_TUNE_CLK_TX_ENABLE)
		if (host->id == 3)
			goto re_tune_rx;
#endif
	}
end:
	autok_reset_err_counter(host);
	autok_msdc_reset();
	msdc_clear_fifo();
	MSDC_WRITE32(MSDC_INT, 0xffffffff);
	MSDC_WRITE32(MSDC_INTEN, int_en);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);

#if AUTOK_DEBUG_TIME
	do_gettimeofday(&t_end);
	if((t_end.tv_usec<t_start.tv_usec)&&(t_end.tv_sec>t_start.tv_sec))
		UTIL_Printf("[SDR AUTOK]=========Time Cost:%d us========\n",
			1000000+t_end.tv_usec-t_start.tv_usec);
	else
		UTIL_Printf("[SDR AUTOK]=========Time Cost:%d us========\n",
			t_end.tv_usec-t_start.tv_usec);
#endif
	return ret;
}
int autok_execute_tuning_1bit(struct msdc_host *host, unsigned char *res)
{
	int ret = 0;
	unsigned int clk_pwdn = 0;
	unsigned int int_en = 0;
	unsigned long base = host->base;
	unsigned char autok_tune_res[TUNING_PARAM_COUNT];
	unsigned int i = 0;
	unsigned int value = 0;
#if AUTOK_DEBUG_TIME
	struct timeval t_start, t_end;
	do_gettimeofday(&t_start);
#endif
	int_en = MSDC_READ32(MSDC_INTEN);
	MSDC_WRITE32(MSDC_INTEN, 0);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, 1);
	autok_reset_err_counter(host);

	/* store pre autok parameter */
	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		msdc_autok_adjust_param(host, i, &value, AUTOK_READ);
		autok_tune_res[i] = value;
	}

	ret = execute_online_tuning(host, res, SD_IO_RW_EXTENDED);
	if (ret != 0) {
		UTIL_Printf("[AUTOK] ========Error: Autok Failed========\r\n");
		UTIL_Printf("[AUTOK] ========restore pre autok parameters========\r\n");
		/* restore pre autok parameter */
		for (i = 0; i < TUNING_PARAM_COUNT; i++) {
			value = (unsigned char) autok_tune_res[i];
			msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
		}
	}
	else
		autok_save_result(host);

	autok_reset_err_counter(host);
	autok_msdc_reset();
	msdc_clear_fifo();
	MSDC_WRITE32(MSDC_INT, 0xffffffff);
	MSDC_WRITE32(MSDC_INTEN, int_en);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);
#if AUTOK_DEBUG_TIME
	do_gettimeofday(&t_end);
	if((t_end.tv_usec<t_start.tv_usec)&&(t_end.tv_sec>t_start.tv_sec))
		UTIL_Printf("[DDR50/52 AUTOK]=========Time Cost:%d us========\n",
			1000000+t_end.tv_usec-t_start.tv_usec);
	else
		UTIL_Printf("[DDR50/52 AUTOK]=========Time Cost:%d us========\n",
			t_end.tv_usec-t_start.tv_usec);
#endif

	return ret;
}

int hs400_execute_tuning(struct msdc_host *host, unsigned char *res)
{
	int ret = 0;
	unsigned int clk_pwdn = 0;
	unsigned int int_en = 0;
	void __iomem *base = host->base;
	unsigned char autok_tune_res[TUNING_PARA_SCAN_COUNT];
	unsigned int i = 0;
	unsigned int value = 0;
	
#if AUTOK_DEBUG_TIME
	struct timeval t_start, t_end;
	do_gettimeofday(&t_start);
#endif

	int_en = MSDC_READ32(MSDC_INTEN);
	MSDC_WRITE32(MSDC_INTEN, 0);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, 1);
	autok_reset_err_counter(host);

	/* store pre autok parameter */
	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		msdc_autok_adjust_param(host, i, &value, AUTOK_READ);
		autok_tune_res[i] = value;
	}

#if AUTOK_OFFLINE_TUNE_LATCH_EN_ENABLE
	autok_offline_tuning_cmd_latch_en(host, HS400_TUNE, MMC_READ_SINGLE_BLOCK);
#endif
	ret = execute_online_tuning_hs400(host, res);
	if (ret != 0) {
		UTIL_Printf("[AUTOK] ========Error: Autok HS400 Failed========\r\n");
		UTIL_Printf("[AUTOK] ========restore pre autok parameters========\r\n");
		/* restore pre autok parameter */
		for (i = 0; i < TUNING_PARAM_COUNT; i++) {
			value = (unsigned char) autok_tune_res[i];
			msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
		}
	}
	else{
		autok_save_result(host);
	#if MSDC_AUTOK_TO_MTZ
		kthread_run(autok_save_to_metazone, (void *)host,"hs400_autok");
	#endif
	}
#if AUTOK_OFFLINE_TUNE_TX_ENABLE
	autok_offline_tuning_TX(host);
#endif

	autok_reset_err_counter(host);
	autok_msdc_reset();
	msdc_clear_fifo();
	MSDC_WRITE32(MSDC_INT, 0xffffffff);
	MSDC_WRITE32(MSDC_INTEN, int_en);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);

#if AUTOK_DEBUG_TIME
	do_gettimeofday(&t_end);
	if((t_end.tv_usec<t_start.tv_usec)&&(t_end.tv_sec>t_start.tv_sec))
		UTIL_Printf("[HS400 AUTOK]=========Time Cost:%d us========\n",
			1000000+t_end.tv_usec-t_start.tv_usec);
	else
		UTIL_Printf("[HS400 AUTOK]=========Time Cost:%d us========\n",
			t_end.tv_usec-t_start.tv_usec);
#endif

	return ret;
}
EXPORT_SYMBOL(hs400_execute_tuning);

int hs200_execute_tuning(struct msdc_host *host, unsigned char *res)
{
	int ret = 0;
	unsigned int clk_pwdn = 0;
	unsigned int int_en = 0;
	void __iomem *base = host->base;
	unsigned char autok_tune_res[TUNING_PARA_SCAN_COUNT];
	unsigned int i = 0;
	unsigned int value = 0;
	
#if AUTOK_DEBUG_TIME
	struct timeval t_start, t_end;
	do_gettimeofday(&t_start);
#endif
	int_en = MSDC_READ32(MSDC_INTEN);
	MSDC_WRITE32(MSDC_INTEN, 0);
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, 1);
	autok_reset_err_counter(host);

	/* store pre autok parameter */
	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		msdc_autok_adjust_param(host, i, &value, AUTOK_READ);
		autok_tune_res[i] = value;
	}

	MSDC_WRITE32(MSDC_INT, 0xffffffff);
#if AUTOK_OFFLINE_TUNE_LATCH_EN_ENABLE
	autok_offline_tuning_cmd_latch_en(host, HS200_TUNE, MMC_READ_SINGLE_BLOCK);
#endif
	ret = execute_online_tuning(host, res, MMC_SEND_TUNING_BLOCK_HS200);
	if (ret != 0) {
		UTIL_Printf("[AUTOK] ========Error: Autok HS200 Failed========\r\n");
		UTIL_Printf("[AUTOK] ========restore pre autok parameters========\r\n");
		/* restore pre autok parameter */
		for (i = 0; i < TUNING_PARAM_COUNT; i++) {
			value = (unsigned char) autok_tune_res[i];
			msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
		}
	}
	else
		autok_save_result(host);
#if AUTOK_OFFLINE_TUNE_LATCH_EN_ENABLE
	autok_offline_tuning_cts_latch_en(host, HS200_TUNE, MMC_WRITE_BLOCK);
#endif
	
	autok_reset_err_counter(host);
	autok_msdc_reset();
	msdc_clear_fifo();
	MSDC_WRITE32(MSDC_INT, 0xffffffff);
	MSDC_WRITE32(MSDC_INTEN, int_en);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKPDN, clk_pwdn);
	
	#if AUTOK_DEBUG_TIME
	do_gettimeofday(&t_end);
	if((t_end.tv_usec<t_start.tv_usec)&&(t_end.tv_sec>t_start.tv_sec))
		UTIL_Printf("[HS200 AUTOK]=========Time Cost:%d us========\n",
			1000000+t_end.tv_usec-t_start.tv_usec);
	else
		UTIL_Printf("[HS200 AUTOK]=========Time Cost:%d us========\n",
			t_end.tv_usec-t_start.tv_usec);
	#endif
	return ret;
}

void autok_save_result(struct msdc_host *host)
{
	unsigned char *autok_tune_res;
	unsigned int  value = 0;
	int i;
	switch(host->id){
		case 0:autok_tune_res=autok_tune_res0;
				break;
		case 1:autok_tune_res=autok_tune_res1;
				break;
		case 2:autok_tune_res=autok_tune_res2;
				break;
		case 3:autok_tune_res=autok_tune_res3;
				break;
		default:pr_err("error port %d \n",host->id);
				break;
	}

	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		msdc_autok_adjust_param(host, i, &value, AUTOK_READ);
		autok_tune_res[i] = value;
	}

}

void autok_restore_result(struct msdc_host *host,char type)
{
	unsigned char *autok_tune_res;
	unsigned int  value = 0;
	int i;
	switch(type){
		case AUTOK_HS200_PARAM:
				autok_init_hs200(host);
				break;
		case AUTOK_HS400_PARAM:
				autok_init_hs400(host);
				break;
		case AUTOK_SDR104_PARAM:
				autok_init_sdr104(host);
				break;
		default:pr_err("error type %d \n",type);
				break;
	}
	switch(host->id){
		case 0:autok_tune_res=autok_tune_res0;
				break;
		case 1:autok_tune_res=autok_tune_res1;
				break;
		case 2:autok_tune_res=autok_tune_res2;
				break;
		case 3:autok_tune_res=autok_tune_res3;
				break;
		default:pr_err("error port %d \n",host->id);
				break;
	}
	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		value = (unsigned char) autok_tune_res[i];
		msdc_autok_adjust_param(host, i, &value, AUTOK_WRITE);
	}

}
void autok_print_result(struct msdc_host *host)
{
	unsigned int i = 0;
	unsigned int value = 0;
	unsigned char autok_tune_res[TUNING_PARAM_COUNT];

	for (i = 0; i < TUNING_PARAM_COUNT; i++) {
		msdc_autok_adjust_param(host, i, &value, AUTOK_READ);
		autok_tune_res[i] = value;
	}
	UTIL_Printf("[AUTOK]CMD [EDGE:%d DLY1:%d DLY1_SEL:%d DLY2:%d DLY2_SEL:%d]\r\n",
		autok_tune_res[0], autok_tune_res[5],autok_tune_res[6], 
		autok_tune_res[7],autok_tune_res[8]);
	UTIL_Printf("[AUTOK]DAT [RDAT_EDGE:%d WDAT_EDGE:%d RD_FIFO_EDGE:%d WD_FIFO_EDGE:%d]\r\n",
		autok_tune_res[1], autok_tune_res[2], autok_tune_res[3], autok_tune_res[4]);
	UTIL_Printf("[AUTOK]DAT [LATCH_CK:%d DLY1:%d DLY1_SEL:%d DLY2:%d DLY2_SEL:%d]\r\n",
		autok_tune_res[13], autok_tune_res[9],autok_tune_res[10],
		autok_tune_res[11],autok_tune_res[12]);
	UTIL_Printf("[AUTOK]DS  [DLY1:%d DLY1_SEL:%d DLY2:%d DLY2_SEL:%d DLY3:%d]\r\n",
		autok_tune_res[14],autok_tune_res[15], autok_tune_res[16], 
		autok_tune_res[17],autok_tune_res[18]);

	return 0;
}

extern unsigned int MetaZone_WriteBinary(unsigned int u4Idx, const char *pbData, unsigned int u4Size);
extern u32 MetaZone_Flush(int fgSync);

void print_autok_metazone(struct autok_in_metazone params)
{
	pr_info("=====host id %d 		=====\n",params.host_id);
	pr_info("=====mode %d 			=====\n",params.mode);
	pr_info("=====clk %d 			=====\n",params.clk);
	pr_info("=====mtz_idx %d 		=====\n",params.mtz_idx);
	pr_info("=====valid %d 			=====\n",params.valid);
	pr_info("=====update_times %d 	=====\n",params.update_times);
	pr_info("=====cmd_edge %d 				=====\n",params.result.cmd_edge);
	pr_info("=====rd_fifo_edge %d 			=====\n",params.result.rdata_edge);
	pr_info("=====wdata_edge %d 				=====\n",params.result.wdata_edge);
	pr_info("=====rd_fifo_edge %d 			=====\n",params.result.rd_fifo_edge);
	pr_info("=====wd_fifo_edge %d 			=====\n",params.result.wd_fifo_edge);
	pr_info("=====cmd_rd_d_dly1 %d 			=====\n",params.result.cmd_rd_d_dly1);
	pr_info("=====cmd_rd_d_dly1_sel %d		=====\n",params.result.cmd_rd_d_dly1_sel);
	pr_info("=====cmd_rd_d_dly2 %d 			=====\n",params.result.cmd_rd_d_dly2);
	pr_info("=====cmd_rd_d_dly2_sel %d 		=====\n",params.result.cmd_rd_d_dly2_sel);
	pr_info("=====dat_rd_d_dly1 %d 			=====\n",params.result.dat_rd_d_dly1);
	pr_info("=====dat_rd_d_dly1_sel %d 		=====\n",params.result.dat_rd_d_dly1_sel);
	pr_info("=====dat_rd_d_dly2 %d 			=====\n",params.result.dat_rd_d_dly2);
	pr_info("=====dat_rd_d_dly2_sel %d 		=====\n",params.result.dat_rd_d_dly2_sel);
	pr_info("=====int_dat_latch_ck %d 		=====\n",params.result.int_dat_latch_ck);
	pr_info("=====emmc50_ds_z_dly1 %d 		=====\n",params.result.emmc50_ds_z_dly1);
	pr_info("=====emmc50_ds_z_dly1_sel %d 	=====\n",params.result.emmc50_ds_z_dly1_sel);
	pr_info("=====emmc50_ds_z_dly2 %d 		=====\n",params.result.emmc50_ds_z_dly2);
	pr_info("=====emmc50_ds_z_dly2_sel %d 	=====\n",params.result.emmc50_ds_z_dly2_sel);
	pr_info("=====emmc50_ds_zdly_dly %d 		=====\n",params.result.emmc50_ds_zdly_dly);
}

static int hs400_update_times=0;
static int sdr104_update_times=0;

int autok_save_to_metazone(void *arg)
{
	struct msdc_host *host=(struct msdc_host *)arg;
	struct autok_in_metazone params;
	int i;
	int mtz_index;
	params.host_id=host->id;
	params.clk=host->sclk/1000000;
	params.valid=1;
	
	if(host->hw->host_function==MSDC_EMMC){
		params.mode=HS400_TUNE;
		params.mtz_idx=0;
		mtz_index=KL_EMMC_MTZ;
		params.update_times=hs400_update_times++;
		memcpy(&(params.result),autok_tune_res0,TUNING_PARAM_COUNT);
	}
	else if(host->hw->host_function==MSDC_SDIO){
		params.mode=SDR104_TUNE;
		params.mtz_idx=1;
		mtz_index=KL_SDIO_MTZ;
		params.update_times=sdr104_update_times++;
		memcpy(&(params.result),autok_tune_res3,TUNING_PARAM_COUNT);
	}
	
	for(i=0;i<100;i++){
		if((!MetaZone_WriteBinary(mtz_index,&params,sizeof(struct autok_in_metazone)))
			&&(!MetaZone_Flush(1)))
			break;
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(2*HZ);
	}
	
	if(i==100)
		pr_err("save autok params to metazone fail \n");
	//print_autok_metazone(params);

	return 0;
}
