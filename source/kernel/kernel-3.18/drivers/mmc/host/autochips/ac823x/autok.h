/*
 * Copyright (C) 2017 AutoChips Inc.
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

#ifndef _AUTOK_H_
#define _AUTOK_H_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/completion.h>
#include <linux/scatterlist.h>
#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include "mt_sd.h"

#define AUTOK_DEBUG_TIME  (1)
#define E_RESULT_PASS     (0)
#define E_RESULT_CMD_TMO  (1<<0)
#define E_RESULT_RSP_CRC  (1<<1)
#define E_RESULT_DAT_CRC  (1<<2)
#define E_RESULT_DAT_TMO  (1<<3)
#define E_RESULT_W_CRC    (1<<4)
#define E_RESULT_ERR      (1<<5)
#define E_RESULT_START    (1<<6)
#define E_RESULT_PW_SMALL (1<<7)
#define E_RESULT_KEEP_OLD (1<<8)
#define E_RESULT_CMP_ERR  (1<<9)
#define E_RESULT_FATAL_ERR  (1<<10)

#define E_RESULT_MAX

#ifndef NULL
#define NULL                0
#endif
#ifndef TRUE
#define TRUE                (0 == 0)
#endif
#ifndef FALSE
#define FALSE               (0 != 0)
#endif

#define AUTOK_DBG_OFF                             0
#define AUTOK_DBG_ERROR                           1
#define AUTOK_DBG_RES                             2
#define AUTOK_DBG_WARN                            3
#define AUTOK_DBG_TRACE                           4
#define AUTOK_DBG_LOUD                            5

extern unsigned int autok_debug_level;

enum TUNE_TYPE {
	TUNE_CMD = 0,
	TUNE_DATA,
	TUNE_LATCH_CK,
};
#define AUTOK_TAG 	"[MSDC]"

#define UTIL_Printf(_fmt ,args...)           \
		do {								\
			pr_info(AUTOK_TAG""_fmt, ##args); 		\
		} while(0)

enum ERROR_TYPE {
	CMD_ERROR = 0,
	DATA_ERROR,
	CRC_STATUS_ERROR,
};

enum MODE_TYPE {
	HS200_TUNE = 0,
	HS400_TUNE,
	SDR104_TUNE,
};

enum AUTOK_PARAM {
	/* command response sample selection (MSDC_SMPL_RISING, MSDC_SMPL_FALLING) */
	CMD_EDGE,

	/* read data sample selection (MSDC_SMPL_RISING, MSDC_SMPL_FALLING) */
	RDATA_EDGE,

	/* write data sample selection (MSDC_SMPL_RISING, MSDC_SMPL_FALLING) */
	WDATA_EDGE,

	/* read data async fifo out edge select */
	RD_FIFO_EDGE,

	/* write data crc status async fifo out edge select */
	WD_FIFO_EDGE,

	/* [Data Tune]CMD Pad RX Delay Line1 Control. This register is used to
	   fine-tune CMD pad macro respose latch timing. Total 32 stages[Data Tune] */
	CMD_RD_D_DLY1,

	/* [Data Tune]CMD Pad RX Delay Line1 Sel-> delay cell1 enable */
	CMD_RD_D_DLY1_SEL,

	/* [Data Tune]CMD Pad RX Delay Line2 Control. This register is used to
	   fine-tune CMD pad macro respose latch timing. Total 32 stages[Data Tune] */
	CMD_RD_D_DLY2,

	/* [Data Tune]CMD Pad RX Delay Line1 Sel-> delay cell2 enable */
	CMD_RD_D_DLY2_SEL,

	/* [Data Tune]DAT Pad RX Delay Line1 Control (for MSDC RD), Total 32 stages [Data Tune] */
	DAT_RD_D_DLY1,

	/* [Data Tune]DAT Pad RX Delay Line1 Sel-> delay cell1 enable */
	DAT_RD_D_DLY1_SEL,

	/* [Data Tune]DAT Pad RX Delay Line2 Control (for MSDC RD), Total 32 stages [Data Tune] */
	DAT_RD_D_DLY2,

	/* [Data Tune]DAT Pad RX Delay Line2 Sel-> delay cell2 enable */
	DAT_RD_D_DLY2_SEL,

	/* Internal MSDC clock phase selection. Total 8 stages, each stage can delay 1 clock period of msdc_src_ck */
	INT_DAT_LATCH_CK,

	/* DS Pad Z clk delay count, range: 0~63, Z dly1(0~31)+Z dly2(0~31) */
	EMMC50_DS_Z_DLY1,

	/* DS Pad Z clk del sel: [dly2_sel:dly1_sel] -> [0,1]:
	 * dly1 enable [1,2]:dl2 & dly1 enable ,else :no dly enable, */
	EMMC50_DS_Z_DLY1_SEL,

	/* DS Pad Z clk delay count, range: 0~63, Z dly1(0~31)+Z dly2(0~31) */
	EMMC50_DS_Z_DLY2,

	/* DS Pad Z clk del sel: [dly2_sel:dly1_sel] -> [0,1]:
	 * dly1 enable [1,2]:dl2 & dly1 enable ,else :no dly enable, */
	EMMC50_DS_Z_DLY2_SEL,

	/* DS Pad Z_DLY clk delay count, range: 0~31 */
	EMMC50_DS_ZDLY_DLY,
	TUNING_PARAM_COUNT,

	/* CMD scan result */
	CMD_SCAN_R0,
	CMD_SCAN_R1,
	CMD_SCAN_R2,
	CMD_SCAN_R3,
	CMD_SCAN_R4,
	CMD_SCAN_R5,
	CMD_SCAN_R6,
	CMD_SCAN_R7,

	CMD_SCAN_F0,
	CMD_SCAN_F1,
	CMD_SCAN_F2,
	CMD_SCAN_F3,
	CMD_SCAN_F4,
	CMD_SCAN_F5,
	CMD_SCAN_F6,
	CMD_SCAN_F7,

	/* DATA scan result */
	DAT_SCAN_R0,
	DAT_SCAN_R1,
	DAT_SCAN_R2,
	DAT_SCAN_R3,
	DAT_SCAN_R4,
	DAT_SCAN_R5,
	DAT_SCAN_R6,
	DAT_SCAN_R7,

	DAT_SCAN_F0,
	DAT_SCAN_F1,
	DAT_SCAN_F2,
	DAT_SCAN_F3,
	DAT_SCAN_F4,
	DAT_SCAN_F5,
	DAT_SCAN_F6,
	DAT_SCAN_F7,

	/* DS scan result */
	DS_SCAN_0,
	DS_SCAN_1,
	DS_SCAN_2,
	DS_SCAN_3,
	DS_SCAN_4,
	DS_SCAN_5,
	DS_SCAN_6,
	DS_SCAN_7,

	/* Device CMD RX result */
	D_CMD_SCAN_0,
	D_CMD_SCAN_1,
	D_CMD_SCAN_2,
	D_CMD_SCAN_3,

	/* Device DATA RX result */
	D_DATA_SCAN_0,
	D_DATA_SCAN_1,
	D_DATA_SCAN_2,
	D_DATA_SCAN_3,

	/* Host CMD TX result */
	H_CMD_SCAN_0,
	H_CMD_SCAN_1,
	H_CMD_SCAN_2,
	H_CMD_SCAN_3,

	/* Host DATA TX result */
	H_DATA_SCAN_0,
	H_DATA_SCAN_1,
	H_DATA_SCAN_2,
	H_DATA_SCAN_3,

	/* AUTOK version */
	AUTOK_VER0,
	AUTOK_VER1,
	AUTOK_VER2,
	AUTOK_VER3,

	TUNING_PARA_SCAN_COUNT,

	/* Data line rising/falling latch  fine tune selection in read transaction.
	   1'b0: All data line share one value indicated by MSDC_IOCON.R_D_SMPL.
	   1'b1: Each data line has its own  selection value indicated by Data line (x): MSDC_IOCON.R_D(x)_SMPL */
	READ_DATA_SMPL_SEL,

	/* Data line rising/falling latch  fine tune selection in write transaction.
	   1'b0: All data line share one value indicated by MSDC_IOCON.W_D_SMPL.
	   1'b1: Each data line has its own selection value indicated by Data line (x): MSDC_IOCON.W_D(x)_SMPL */
	WRITE_DATA_SMPL_SEL,

	/* Data line delay line fine tune selection. 1'b0: All data line share one delay
	   selection value indicated by PAD_TUNE.PAD_DAT_RD_RXDLY. 1'b1: Each data line has its
	   own delay selection value indicated by Data line (x): DAT_RD_DLY(x).DAT0_RD_DLY */
	DATA_DLYLINE_SEL,

	/* [Data Tune]CMD & DATA Pin tune Data Selection[Data Tune Sel] */
	MSDC_DAT_TUNE_SEL,

	/* [Async_FIFO Mode Sel For Write Path] */
	MSDC_WCRC_ASYNC_FIFO_SEL,

	/* [Async_FIFO Mode Sel For CMD Path] */
	MSDC_RESP_ASYNC_FIFO_SEL,

	/* Write Path Mux for emmc50 function & emmc45 function , Only emmc50 design valid,[1-eMMC50, 0-eMMC45] */
	EMMC50_WDATA_MUX_EN,

	/* CMD Path Mux for emmc50 function & emmc45 function , Only emmc50 design valid,[1-eMMC50, 0-eMMC45] */
	EMMC50_CMD_MUX_EN,

	/* write data crc status async fifo output edge select */
	EMMC50_WDATA_EDGE,

	/* CKBUF in CKGEN Delay Selection. Total 32 stages */
	CKGEN_MSDC_DLY_SEL,

	/* CMD response turn around period. The turn around cycle = CMD_RSP_TA_CNTR + 2,
	   Only for USH104 mode, this register should be set to 0 in non-UHS104 mode */
	CMD_RSP_TA_CNTR,

	/* Write data and CRC status turn around period. The turn around cycle = WRDAT_CRCS_TA_CNTR + 2,
	   Only for USH104 mode,  this register should be set to 0 in non-UHS104 mode */
	WRDAT_CRCS_TA_CNTR,

	/* CLK Pad TX Delay Control. This register is used to add delay to CLK phase. Total 32 stages */
	PAD_CLK_TXDLY,
	TOTAL_PARAM_COUNT
};


struct autok_params{
	char cmd_edge;
	char rdata_edge;
	char wdata_edge;
	char rd_fifo_edge;
	char wd_fifo_edge;
	char cmd_rd_d_dly1;
	char cmd_rd_d_dly1_sel;
	char cmd_rd_d_dly2;
	char cmd_rd_d_dly2_sel;
	char dat_rd_d_dly1;
	char dat_rd_d_dly1_sel;
	char dat_rd_d_dly2;
	char dat_rd_d_dly2_sel;
	char int_dat_latch_ck;
	char emmc50_ds_z_dly1;
	char emmc50_ds_z_dly1_sel;
	char emmc50_ds_z_dly2;
	char emmc50_ds_z_dly2_sel;
	char emmc50_ds_zdly_dly;
};

struct autok_in_metazone{
	char host_id;
	char mode;
	char clk;
	char mtz_idx;
	char valid;
	char update_times;
	struct autok_params  result;
};

#define KL_EMMC_MTZ		(0x10000)
#define KL_SDIO_MTZ		(0x10001)
#define LK_EMMC_MTZ		(0x10002)
#define MMC_TIMING_LEGACY	0
#define AUTOK_MMC_UHS_SDR50	1
#define AUTOK_MMC_UHS_SDR104 2
#define AUTOK_MMC_UHS_DDR50	3
#define AUTOK_MMC_MMC_DDR52	4
#define AUTOK_MMC_MMC_HS200	5
#define AUTOK_MMC_MMC_HS400	6

#if 1
#define SD_CMD_SEND_TUNING_BLOCK        (19 | SD_CMD_BIT)
#define SD_CMD_BIT                      (1 << 7)
#endif
//#define USE_INTERNAL_DLY
/**********************************************************
* Feature  Control Defination                             *
**********************************************************/
#define AUTOK_OFFLINE_TUNE_TX_ENABLE  0
#define AUTOK_OFFLINE_TUNE_LATCH_EN_ENABLE  0
#define AUTOK_OFFLINE_TUNE_CLK_TX_ENABLE  0
#define AUTOK_OFFLINE_CMD_D_RX_ENABLE 0
#define AUTOK_OFFLINE_DAT_D_RX_ENABLE 0
#define AUTOK_OFFLINE_TUNE_DEVICE_RX_ENABLE 0
#define AUTOK_PARAM_DUMP_ENABLE   0
#define AUTOK_DEBUG_LOG  0
/* #define CHIP_DENALI_3_DAT_TUNE */
#define NEW_PATH  0
#define DDR50_TUNE_SINGLE_EDGE  1
#define USE_HS200_DATA_RES      0

#define MSDC_SDIO_CLK_TX_TUNE 555
#define MSDC_SD_CLK_TX_TUNE 666

#if (AUTOK_OFFLINE_TUNE_CLK_TX_ENABLE||AUTOK_OFFLINE_TUNE_DEVICE_RX_ENABLE)
#define AUTOK_MAX_ERR    1000
#else
#define AUTOK_MAX_ERR    120
#endif
//#define AUTOK_UPDATE_DISABLE  
/**********************************************************
* Function Declaration                                    *
**********************************************************/
extern int autok_path_sel(struct msdc_host *host);
extern int autok_init_sdr104(struct msdc_host *host);
extern int autok_init_ddr(struct msdc_host *host);
extern int autok_init_hs200(struct msdc_host *host);
extern int autok_init_hs400(struct msdc_host *host);
extern int autok_offline_tuning_TX(struct msdc_host *host);
extern int autok_offline_tuning_clk_TX(struct msdc_host *host, unsigned int opcode);
extern int autok_offline_tuning_device_RX(struct msdc_host *host, u8 *res);
extern int autok_offline_tuning_cmd_latch_en(struct msdc_host *host, enum MODE_TYPE mode_type, unsigned int opcode);
extern int autok_offline_tuning_cts_latch_en(struct msdc_host *host, enum MODE_TYPE mode_type, unsigned int opcode);
extern void autok_msdc_tx_setting(struct msdc_host *host,struct mmc_ios *ios);
extern void autok_low_speed_switch_edge(struct msdc_host *host,struct mmc_ios *ios, enum ERROR_TYPE error_type);
extern void autok_tuning_parameter_init(struct msdc_host *host, unsigned char *res);
extern int ddr50_execute_tuning(struct msdc_host *host, unsigned char *res);
extern int autok_execute_tuning(struct msdc_host *host, unsigned char *res);
extern int autok_execute_tuning_1bit(struct msdc_host *host, unsigned char *res);
extern int hs200_execute_tuning(struct msdc_host *host, unsigned char *res);
extern int hs400_execute_tuning(struct msdc_host *host, unsigned char *res);
extern void  autok_save_result(struct msdc_host * host);
extern void autok_restore_result(struct msdc_host *host,char type);
extern void autok_print_result(struct msdc_host *host);

#endif  /* _AUTOK_H_ */

