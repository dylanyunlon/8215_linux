/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

/*!
 * @file dmx_pvr.h
 *
 * @par Project
 *		MT3360
 *
 * @par Description
 *
 *
 * @par Author_Name
 *		Shuhui Zhang
 *
 */

#ifndef DMX_PVR_H_FILE
#define DMX_PVR_H_FILE

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/
#include "x_os.h"
#include "drv_config.h"
#include "windows.h"
#ifdef __linux__
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/pinctrl/consumer.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <mach/base_regs.h>
#include <media/atc/dmx_define.h>
#include <media/atc/drv_esm_if.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "drv_esm_if.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "x_hal_ic.h"
#include "dmx_def.h"
#include "dmx_pvr_if.h"
#include "x_ioopt.h"

/*-----------------------------------------------------------------------------*/
/* PVR LOG definitions*/
/*-----------------------------------------------------------------------------*/
#if 1
#define PVR_LOG_TRACE(arg, ...)		DMXLOG_TRACE(arg, ##__VA_ARGS__)
#define PVR_LOG_ERR(arg, ...)		DMXLOG_ERROR(arg, ##__VA_ARGS__)
#define PVR_LOG_INFO(arg, ...)		DMXLOG_TRACE(arg, ##__VA_ARGS__)
#define PVR_LOG_DBG(arg, ...)
#define PVR_LOG_DUMP(arg, ...)
#else
#define PVR_LOG_TRACE(arg, ...)		DMXLOG_TRACE(arg, ##__VA_ARGS__)
#define PVR_LOG_ERR(arg, ...)		DMXLOG_ERROR(arg, ##__VA_ARGS__)
#define PVR_LOG_INFO(arg, ...)		DMXLOG_TRACE(arg, ##__VA_ARGS__)
#define PVR_LOG_DBG(arg, ...)		DMXLOG_TRACE(arg, ##__VA_ARGS__)
#define PVR_LOG_DUMP(arg, ...)
#endif

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/
/* Clock*/
#define DMX_CLOCK					(324)	/*MHz */
#define DMX_PVRPLAY_TIMESTAMP_DIV_BASE			(DMX_CLOCK / 27)

struct dmx_dev_info {
	/* TODO */
	struct miscdevice cdev;	/* Char device structure */
	struct device *dev;

	int ftup_irq;
	int ddi_irq;

	struct clk *dmx_ts0_in_clk_parent;
	struct clk *dmx_ts0_in_clk;
	int ts0_in_sel;
	struct clk *dmx_ts1_in_clk_parent;
	struct clk *dmx_ts1_in_clk;
	int ts1_in_sel;
	struct clk *dmx_top_clk_parent;
	struct clk *dmx_top_clk;
	struct clk *dmx_ts0_gateclk;
	struct clk *dmx_ts1_gateclk;
	struct clk *dmx_27m_gateclk;
	struct clk *dmx_gateclk;

	struct pinctrl *pinctrl_demuxer;
	struct pinctrl_state *pins_tsclk_sel_set;
	struct pinctrl_state *pins_tsden_sel_set;
	struct pinctrl_state *pins_tssync_sel_set;
	struct pinctrl_state *pins_tsd0_sel_set;
	struct pinctrl_state *pins_tsd1_sel_set;
	struct pinctrl_state *pins_tsd2_sel_set;
	struct pinctrl_state *pins_tsd3_sel_set;
	struct pinctrl_state *pins_tsd4_sel_set;
	struct pinctrl_state *pins_tsd5_sel_set;
	struct pinctrl_state *pins_tsd6_sel_set;
	struct pinctrl_state *pins_tsd7_sel_set;

	void __iomem *dmx_base0_regs;
	void __iomem *dmx_base1_regs;
	void __iomem *dmx_base2_regs;
	void __iomem *dmx_base8_regs;
	void __iomem *dmx_ddi_regs;
};

extern struct dmx_dev_info *g_dmxdevinfo;

/* Demux register access commands*/
#define DMXCMD_READ8(offset)		IO_READ8(g_dmxdevinfo->dmx_base0_regs, (offset))
#define DMXCMD_READ16(offset)		IO_READ16(g_dmxdevinfo->dmx_base0_regs, (offset))
#define DMXCMD_READ32(offset)		IO_READ32(g_dmxdevinfo->dmx_base0_regs, ((offset) * 4))
#define DMXCMD_WRITE8(offset, value)	do {\
	IO_WRITE8(g_dmxdevinfo->dmx_base0_regs, (offset), (value)); mb();\
} while (0)
#define DMXCMD_WRITE16(offset, value)	do {\
	IO_WRITE16(g_dmxdevinfo->dmx_base0_regs, (offset), (value)); mb();\
} while (0)
#define DMXCMD_WRITE32(offset, value)	do {\
	IO_WRITE32(g_dmxdevinfo->dmx_base0_regs, ((offset) * 4), (value)); mb();\
} while (0)

/*****************************************************************************/
/* Playback PID index table (0x12C00)*/
/*****************************************************************************/
#define PID_INDEX_TABLE(pidx)	((volatile u32*)(g_dmxdevinfo->dmx_base0_regs + 0xC00))[(pidx)]

/*/ Read/Write DMEM commands*/

#define PVR_DMEM_BASE			((volatile u32*)(g_dmxdevinfo->dmx_base2_regs))
#define PVR_DMEM_MAX_LEN		(0x4000)

/*****************************************************************************/
/* Playback PID data structure access commands*/
/*****************************************************************************/
#define PVR_DMEM_ENTRY_LEN		30	/*120byte Playback PID data structure */
#define PVR_PID_DATA_STRUCT_SIZE	120
#define PID_W(word)				((volatile u32*)(g_dmxdevinfo->dmx_base2_regs))[(word)]
#define PID_S(pidx)				PID_W(PVR_DMEM_ENTRY_LEN * (pidx))
#define PID_S_W(pidx, word)		PID_W((PVR_DMEM_ENTRY_LEN * (pidx)) + (word))

/*****************************************************************************/
/* Start Code Patten-Mask*/
/*****************************************************************************/

/* Start Code Patten-Mask 0 Base Address*/
#define PVR_PATTERN_MATCH_BASE0			(g_dmxdevinfo->dmx_base2_regs + 0x2500)

/* Start Code Patten-Mask 1 Base Address*/
#define PVR_PATTERN_MATCH_BASE1			(g_dmxdevinfo->dmx_base2_regs + 0x25A8)

/*****************************************************************************/
/* Playback Global Region*/
/*****************************************************************************/
#define PLAYBACK_GBL_BASE		((volatile void *)(g_dmxdevinfo->dmx_base2_regs  + 0x2650))
#define PLAYBACK_GBL_SIZE		(9)	/* 36 bytes */
#define CT_SETTING			(*(volatile u32*)(g_dmxdevinfo->dmx_base2_regs + 0x2650))
#define MICROCODE_WATCHDOG		(*(volatile u32*)(g_dmxdevinfo->dmx_base2_regs + 0x2654))
#define CT_TSIDX_SETTING		(*(volatile u32*)(g_dmxdevinfo->dmx_base2_regs + 0x2658))
#define SECTIONFILTER_SETTING		(*(volatile u32*)(g_dmxdevinfo->dmx_base2_regs + 0x265C))
#define B_FRAME_DROP_ENABLE		(*(volatile u32*)(g_dmxdevinfo->dmx_base2_regs + 0x2660))

/*****************************************************************************/
/* One-Byte Section Filter*/
/*****************************************************************************/

/* one-byte section filter base address*/
#define	PVR_ONEBYTE_FILTER_BASE		(g_dmxdevinfo->dmx_base2_regs + 0x2674)

/*****************************************************************************/
/* Record Global Region 0*/
/*****************************************************************************/
#define	RECORD_GBL_REGION0_BASE		(g_dmxdevinfo->dmx_base2_regs + 0x26F4)

/*****************************************************************************/
/* Record Global Region 1*/
/*****************************************************************************/
#define RECORD_GBL_REGION1_BASE		(g_dmxdevinfo->dmx_base2_regs + 0x2730)

/*****************************************************************************/
/* Record Common Region*/
/*****************************************************************************/
#define RECORD_COMMON_REGION_BASE		(g_dmxdevinfo->dmx_base2_regs + 0x276C)

/*****************************************************************************/
/* HDCP Residual Byte Store Region*/
/*****************************************************************************/
#define HDCP_RESIDUAL_STORE_REGION_BASE		(g_dmxdevinfo->dmx_base2_regs + 0x27B8)

/*****************************************************************************/
/* CA*/
/*****************************************************************************/
#define PVR_DMEM_CA_GROUP_CNT					(16)
#define PVR_DMEM_CA_KEY_SIZE					(96)
#define PVR_DMEM_CA_KEY_BASE					(g_dmxdevinfo->dmx_base2_regs + 0x2BB8)
#define PVR_DMEM_CA_CTRL_BASE					(g_dmxdevinfo->dmx_base2_regs + 0x31B8)


/*****************************************************************************/
/* DivX/WMDRM-ND Keys 0 and DivX/WMDRM-ND Keys 0 (Copy)*/
/*****************************************************************************/

#define PVR_DMEM_MM_KEY_FIELD_SIZE				(14)	/*56 byte */
#define PVR_DMEM_MM_KEY_BASE					(g_dmxdevinfo->dmx_base2_regs + 0x3378)
#define PVR_DMEM_MM_IV_BASE					(g_dmxdevinfo->dmx_base2_regs + 0x3398)
#define PVR_DMEM_MM_KEY_LEN					(32)	/* bytes */
#define PVR_DMEM_MM_IV_LEN					(16)	/* bytes */

/*****************************************************************************/
/* DivX/WMDRM-ND Keys 1 and DivX/WMDRM-ND Keys 1 (Copy)*/
/*****************************************************************************/

#define PVR_DMEM_MM_KEY_BASE1					(g_dmxdevinfo->dmx_base2_regs + 0x33B0)	/*6aE80 */
#define PVR_DMEM_MM_KEY_SIZE1					(18)	/*72 byte */
#define PVR_DMEM_MM_IV_BASE1					(g_dmxdevinfo->dmx_base2_regs + 0x33D0)	/*6aEb8 */

/*****************************************************************************/
/* Hardware Section Filter*/
/*****************************************************************************/

/* Hardware Section Filter Bank*/
#define PVR_SECTION_PATTERN						(g_dmxdevinfo->dmx_base8_regs + 0x0)
#define PVR_SECTION_MASK						(g_dmxdevinfo->dmx_base8_regs + 0x400)
#define PVR_SECTION_POSNEG						(g_dmxdevinfo->dmx_base8_regs + 0x800)
#define PVR_SECTION_CONTROL						(g_dmxdevinfo->dmx_base8_regs + 0xC00)

/* Entry Count in one Hardware Section Filter Bank*/
#define PVR_SECTION_PER_UNIT					(80)


/* CA Key access Commands*/
#define PVR_DMEM_CA_CTRL(word)			((volatile u32*)(PVR_DMEM_CA_CTRL_BASE))[(word)]
#define PVR_DMEM_CA_EVEN_KEY(idx, word)	\
	((volatile u32*)(PVR_DMEM_CA_KEY_BASE + (PVR_DMEM_CA_KEY_SIZE * (idx))))[(word)]
#define PVR_DMEM_CA_EVEN_IV(idx, word)	\
	((volatile u32*)(PVR_DMEM_CA_KEY_BASE + (32 + (PVR_DMEM_CA_KEY_SIZE * (idx)))))[(word)]
#define PVR_DMEM_CA_ODD_KEY(idx, word)	\
	((volatile u32*)(PVR_DMEM_CA_KEY_BASE + (48 + (PVR_DMEM_CA_KEY_SIZE * (idx)))))[(word)]
#define PVR_DMEM_CA_ODD_IV(idx, word)	\
	((volatile u32*)(PVR_DMEM_CA_KEY_BASE + (80 + (PVR_DMEM_CA_KEY_SIZE * (idx)))))[(word)]

/*/CA Key base*/
#define PVR_DMEM_CA_KEY(idx, word)	\
	((volatile u32*)(PVR_DMEM_CA_KEY_BASE + (PVR_DMEM_CA_KEY_SIZE * (idx))))[(word)]


/* Playback Start Code Set Read/Write access commands*/
#define PVR_VCODE_W(word)				((volatile u32*)(PVR_PATTERN_MATCH_BASE0))[(word)]
#define PVR_VCODE_S_W(idx, word)		PVR_VCODE_W((2 + (4 * (idx))) + (word))

//Mini PVR Playback Start Code Set Read/Write access commands
#define MINI_PVR_VCODE_W(word)               ((volatile u32*)(PVR_PATTERN_MATCH_BASE1))[(word)]
#define MINI_PVR_VCODE_S_W(idx, word)        MINI_PVR_VCODE_W((2 + (4 * (idx))) + (word))


/* Multimedia DRM*/
#define PVR_DMEM_MM_KEY(word)			((volatile u32*)(PVR_DMEM_MM_KEY_BASE))[(word)]
#define PVR_DMEM_MM_IV(word)			((volatile u32*)(PVR_DMEM_MM_IV_BASE))[(word)]


/*/ HW Section Filter access commands*/
#define SECTION_FILTER_BASE_CTRL(base, Filteridx)			\
	((volatile u32*)(base))[(Filteridx)]
#define SECTION_FILTER_BASE(base, Filteridx, word)		\
	((volatile u32*)(base + ((Filteridx) * 8)))[(word)]
#define SECTION_FILTER_PAT(Filteridx, word)	\
	SECTION_FILTER_BASE(PVR_SECTION_PATTERN, Filteridx, word)
#define SECTION_FILTER_MASK(Filteridx, word)	 \
	SECTION_FILTER_BASE(PVR_SECTION_MASK , Filteridx, word)
#define SECTION_FILTER_POSNEG(Filteridx, word) \
	SECTION_FILTER_BASE(PVR_SECTION_POSNEG, Filteridx, word)
#define SECTION_FILTER_CTRL(Filteridx) \
	SECTION_FILTER_BASE_CTRL(PVR_SECTION_CONTROL, Filteridx)

/*/ One-Byte Section Filter access commands*/
#define ONEBYTE_SECTION_FILTER_BASE	PVR_ONEBYTE_FILTER_BASE
#define ONEBYTE_SECTION_FILTER_PAT(FilterIdx)  \
	(u8)((((volatile u32*)(PVR_ONEBYTE_FILTER_BASE))\
	[(FilterIdx) >> 1]) >> (((FilterIdx) % 2 == 1) ? 16 : 0))
#define ONEBYTE_SECTION_FILTER_MASK(FilterIdx) \
	(u8)((((volatile u32*)(PVR_ONEBYTE_FILTER_BASE))\
	[(FilterIdx) >> 1]) >> (((FilterIdx) % 2 == 1) ? 24 : 8))
#define ONEBYTE_SECTION_FILTER_W(FilterIdx)	 \
	((volatile u32*)(PVR_ONEBYTE_FILTER_BASE))[(FilterIdx) >> 1]


 /*FVR*/
/* Base addresses*/
#define FVR_GBL_SIZE						(15)	/* 64 bytes */
#define FVR_GBL_COMMON_SIZE					(19)	/* 76 bytes */
/* 28 bype Record PID data structure,total 128share*/
#define FVR_PID_SIZE						(7)
/*******************************************************************************/
/* Record PID Index Table*/
/********************************************************************************/
#define FVR_PID_TABLE_BASE					(g_dmxdevinfo->dmx_base0_regs + 0xE00)	/* 0x12E00 */
/********************************************************************************/
/* Record PID Data structure*/
/********************************************************************************/
#define FVR_PER_PID_OFFSET					(0x1E00)
#define FVR_PER_PID_BASE					(g_dmxdevinfo->dmx_base2_regs + FVR_PER_PID_OFFSET)
/********************************************************************************/
/* Record Global Region 0*/
/********************************************************************************/
#define FVR_GBL_PID_BASE0					(g_dmxdevinfo->dmx_base2_regs + 0x26F4)	/* 0x166F4 */
/********************************************************************************/
/* Record Global Region 1*/
/********************************************************************************/
#define FVR_GBL_PID_BASE1					(g_dmxdevinfo->dmx_base2_regs + 0x2730)	/* 0x16730 */
/********************************************************************************/
/* Record Common Region*/
/********************************************************************************/
#define FVR_GBL_COMMON_REGION				(g_dmxdevinfo->dmx_base2_regs + 0x276C)	/* 0x16730 */
/********************************************************************************/
/* Record Key*/
/********************************************************************************/
#define FVR_KEY_BASE						(g_dmxdevinfo->dmx_base2_regs + 0x31F8)
/* Interrupt*/
#define FVR_INT_ERR_MASK					0xFFFF
#define FVR_INT_ERR_REC_DBM					(1 << 0)
#define FVR_INT_ERR_uP						(1 << 2)
#define FVR_INT_STATUS_MASK					0xFFFF0000
#define FVR_INT_STATUS_REC_DBM					(1 << 16)
#define FVR_INT_STATUS_uP					(1 << 18)
/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/
/* FVR PID data structure access commands*/
/********************************************************************************/
/* Record Common Region*/
/********************************************************************************/
#define FVR_GBL_BASE_W(base, word)			 ((volatile u32*)(base))[(word)]
#define FVR_GBL_ARY_W(idx, word)		 \
			FVR_GBL_BASE_W(((idx == 0) ? FVR_GBL_PID_BASE0 : FVR_GBL_PID_BASE1), word)
#define FVR_GBL_TIMEINFO				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 0)
#define FVR_GBL_TIMEINFO_UPDATE				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 1)
#define FVR_GBL_SERIALNUM_ADDR				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 2)
#define FVR_GBL_DMEMENDADDR_TS0				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 3)
#define FVR_GBL_PKTCOUNTER_TS0				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 4)
#define FVR_GBL_DMEMENDADDR_TS1				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 5)
#define FVR_GBL_PKTCOUNTER_TS1				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 6)
#define FVR_GBL_DMEMENDADDR_TS2				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 7)
#define FVR_GBL_PKTCOUNTER_TS2				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 8)
#define FVR_GBL_DMEMENDADDR_TS3				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 9)
#define FVR_GBL_PKTCOUNTER_TS3				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 10)
#define FVR_GBL_DMEMENDADDR_CI14			FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 11)
#define FVR_GBL_PKTCOUNTER_CI14				FVR_GBL_BASE_W(FVR_GBL_COMMON_REGION, 12)
#define FVR_PER_PID_W(word)				((volatile u32*)(FVR_PER_PID_BASE))[(word)]
#define FVR_PER_PID_S(pidx)				FVR_PER_PID_W(FVR_PID_SIZE * (pidx))
#define FVR_PER_PID_S_W(pidx, word)			FVR_PER_PID_W((FVR_PID_SIZE * (pidx)) + (word))
/* PID index table access command*/
#define FVR_PID_INDEX_TABLE(pidx)			((volatile u32*)(FVR_PID_TABLE_BASE))[(pidx)]
#define FVR_KEY_EVEN_KEY(idx, word)			((volatile u32*)(FVR_KEY_BASE + (idx * 96)))[(word)]
#define FVR_KEY_EVEN_IV(idx, word)			((volatile u32*)(FVR_KEY_BASE + ((idx * 96) + 32)))[(word)]
#define FVR_KEY_ODD_KEY(idx, word)			((volatile u32*)(FVR_KEY_BASE + ((idx * 96) + 48)))[(word)]
#define FVR_KEY_ODD_IV(idx, word)			((volatile u32*)(FVR_KEY_BASE + ((idx * 96) + 80)))[(word)]
/* Base addresses*/
/* Misc */
#define PVR_PID_IDX_VIDEO				0x0	/* PVR Video use PID0 */
#define PVR_PID_IDX_AUDIO				0x1	/* PVR Audio use PID1 */
#define PVR_PID_IDX_SP					0x2	/* PVR SP use PID2 */
#define PVR_PID_IDX_SECTION				0x3	/* PVR Section use PID3 */

#define MINI_PVR_PID_IDX_VIDEO          0x19   //PVR Video use PID25
#define MINI_PVR_PID_IDX_AUDIO          0x1A   //PVR Audio use PID26
#define MINI_PVR_PID_IDX_SP             0x1B   //PVR SP use PID27
#define MINI_PVR_PID_IDX_SECTION        0x1C   //PVR Section use PID28

#define PVR_PID_IDX_MAX					PVR_PID_IDX_SECTION
/* Command Queue Info Store Memory (PID) Start Address in DMEM */
#define PVR_MM_COM_Q_PID_INDEX			10
#define PVR_MAX_MM_COM_Q_ITEM_NUM		200
#define PVR_MM_CMDQ_ENTRY_BASE			(&(PID_S(PVR_MM_COM_Q_PID_INDEX)))
#define PVR_MAX_VTRANS_STARTCODE_CNT	200
/* 820Bytes, when header buffer EA-WP <= this value, FTUP will stop transfer data*/
#define PVR_FTUP_VID_DMA_THRESHOLD		(PVR_MAX_VTRANS_STARTCODE_CNT * sizeof(u32) + 20)
#define PVR_FTUP_OTH_DMA_THRESHOLD		(32)	/* 32Bytes */
#define PVR_INT_QUEUE_DEPTH						64
#define PVR_ES_FIFO_ALIGNMENT					64
#define PVR_HEADER_FIFO_ALIGNMENT				32
#define PVR_DMEM_PATTERN_MATCH_LEN				42	/*168byte Pattern match len */
#define PVR_NULL_PIDX							0xff
#define PVR_NULL_CHANNEL_ID						0xff
#define PVR_PSI_BUF_TIMES						1

//path ID
#define DDI_PVR_DMA_PATH_ID                     0
#define MINI_PVR_DMA_PATH_ID                    1

#define PVR_VNULL							(120)
/* In CI+ verification, there is a high bit-rate stream test (stream replacement). The bitrate is much*/
/*higher than original speed.In MT5389, we found system could hang up if the total number of picture*/
/*interrupt (Partial PES) is bigger than 150 per second. To avoid high frequency interrupts, we start */
/*dropping B frames if picture interrupt is bigger than the threshold. For two playback scenario on 60fps*/
/* stream, the total number of picture interrupt could be bigger than 150 per second. So, to the threshold*/
/*is evaluated for each pidx*/
#define PVR_PICTURE_INT_THRESHOLD				(120)
#define DMX_HW_RETRY_CNT						5
/* Interrupt masks*/
#define PVR_INT_ERR_MASK						0xffff
#define PVR_INT_ERR_DBM							(1 << 0)
#define PVR_INT_ERR_DESCRAMBLER						(1 << 1)
#define PVR_INT_ERR_PCR							(1 << 2)
#define PVR_INT_ERR_RESERVED						(1 << 3)
#define PVR_INT_ERR_STERRING						(1 << 4)
#define PVR_INT_ERR_PES_FILTER						(1 << 5)
#define PVR_INT_ERR_FTUP						(1 << 6)
#define PVR_INT_ERR_MEM_ACCESS						(1 << 7)
#define PVR_INT_ERR_PCR2						(1 << 8)
#define PVR_INT_ERR_PVR							(1 << 9)
#define PVR_INT_ERR_PLAYBACK_STERRING					(1 << 11)
#define PVR_INT_STATUS_MASK						0xffff0000
#define PVR_INT_STATUS_DBM						(1 << 16)
#define PVR_INT_STATUS_DESCRAMBLER					(1 << 17)
#define PVR_INT_STATUS_PCR						(1 << 18)
#define PVR_INT_STATUS_PCR2						(1 << 19)
#define PVR_INT_STATUS_STERRING						(1 << 20)
#define PVR_INT_STATUS_PES_FILTER					(1 << 21)
#define PVR_INT_STATUS_FTUP						(1 << 22)
#define PVR_INT_STATUS_MEM_ACCESS					(1 << 23)
#define PVR_INT_STATUS_RESERVED						(1 << 24)
#define PVR_INT_STATUS_PVR						(1 << 25)
#define PVR_INT_PCR_MASK						(PVR_INT_ERR_PCR | \
									PVR_INT_ERR_PCR2 | \
									PVR_INT_STATUS_PCR | \
									PVR_INT_STATUS_PCR2)
/* | PVR_INT_STATUS_RESERVED)*/
#define PVR_REG_M2_CFG							33	/* 0x84 */
#define PVR_REG_M2_SYS_KEY0						54	/* 0xD8 */
/* Demux registers*/
#define PVR_REG_CONTROL							0	/* 0x0 */
#define PVR_REG_CONFIG1							1	/* 0x4 */
#define PVR_REG_CONFIG2							2	/* 0x8 */
#define PVR_REG_CONFIG3							581	/* 0x914 */
#define PVR_REG_CONFIG4							584	/* 0x920 */
#define PVR_REG_CONFIG5							587	/* 0x92c */
#define PVR_REG_MEM_CMD							3	/* 0xC */
#define PVR_REG_MEM_DATA						4	/* 0x10 */
#define PVR_FTI_SECURITY_ACCESS						5	/*0x14 */
#define PVR_REG_REC_CONFIG1						6	/* 0x18 */
#define PVR_REG_REC_CONFIG2						7	/* 0x1C */
#define PVR_REG_FRAMER_CONTROL						8	/* 0x20 */
#define PVR_REG_FRAMER_ERROR_HANDLE					9	/* 0x24 */
#define PVR_REG_DBM_CONTROL						10	/* 0x28 */
#define PVR_REG_DBM_BYPASS_PID						11	/* 0x2C */
#define PVR_REG_DBM_BYPASS_PID_2					12	/* 0x30 */
#define PVR_REG_DBM_BYPASS_PID_3					13	/* 0x34 */
#define PVR_REG_DBM_BYPASS_PID_4					14	/* 0x38 */
#define PVR_REG_DBM_BUF_CTRL						15	/* 0x3C */
#define PVR_REG_CA_CTRL							16	/* 0x40 */
#define PVR_REG_CA_IN_BUF_THRESHOLD					17	/* 0x44 */
#define PVR_REG_CA_IN_BUF_START						19	/* 0x4C */
#define PVR_REG_CA_IN_BUF_END						20	/* 0x50 */
#define PVR_REG_CA_IN_BUF_RP						21	/* 0x54 */
#define PVR_REG_CA_IN_BUF_WP						22	/* 0x58 */
#define PVR_REG_CA_OUT_BUF_THRESHOLD					18	/* 0x48 */
#define PVR_REG_CA_OUT_BUF_START					23	/* 0x5C */
#define PVR_REG_CA_OUT_BUF_END						24	/* 0x60 */
#define PVR_REG_CA_OUT_BUF_RP						25	/* 0x64 */
#define PVR_REG_CA_OUT_BUF_WP						26	/* 0x68 */
#define PVR_REG_DBM_MULTI_STREAM_MODE					31	/* 0x7C */
#define PVR_REG_PES_CONTRL						38	/* 0x98 */
#define PVR_REG_FTUP_CONTROL						48	/* 0xC0 */
#define PVR_REG_FTUP_TSPROC_RATE_1					49	/* 0xC4 */
#define PVR_REG_FTUP_TSPROC_RATE_2					51	/* 0xCC */
#define PVR_REG_PES_DBM_STEER_CTRL					50	/* 0xC8 */
#define PVR_REG_FTUP_FULL_STATUS					52	/* 0xD0 */
#define PVR_REG_PID_STRUCT_OFFSET					53	/* 0xD4 */
#define PVR_REG_INT_STAUS						64	/* 0x100 */
#define PVR_REG_INT_MASK						65	/* 0x104 */
#define PVR_REG_DBM_ERROR_STATUS_REG					80	/* 0x140 */
#define PVR_REG_DBM_NONERR_STATUS_REG					81	/* 0x144 */
#define PVR_REG_DESCRAMBLER_ERROR_STATUS_REG				88	/* 0x160 */
#define PVR_REG_DESCRAMBLER_NONERR_STATUS_REG				89	/* 0x164 */
#define PVR_REG_STEER_ERROR_STATUS_REG					96	/* 0x180 */
#define PVR_REG_STEER_NONERR_STATUS_REG					97	/* 0x184 */
#define PVR_REG_FTUP_ERROR_STATUS_REG					112	/* 0x1C0 */
#define PVR_REG_FTUP_NONERR_STATUS_REG1					113	/* 0x1C4 */
#define PVR_REG_FTUP_NONERR_STATUS_REG2					114	/* 0x1C8 */
#define PVR_REG_DES_IV_LO_ODD						118	/* 0x1d8 */
#define PVR_REG_DES_IV_HI_ODD						119	/* 0x1dc */
#define PVR_REG_AES_CTRL						120
#define PVR_REG_AES_IV_0_EVEN						121
#define PVR_REG_AES_IV_1_EVEN						122
#define PVR_REG_AES_IV_2_EVEN						123
#define PVR_REG_AES_IV_3_EVEN						124
#define PVR_REG_AES_IV_0_ODD						125
#define PVR_REG_AES_IV_1_ODD						126
#define PVR_REG_AES_IV_2_ODD						127
#define PVR_REG_AES_IV_3_ODD						128
#define PVR_REG_PCR_NUM_CONTROL						129
#define PVR_REG_PCR_CTRL						130
#define PVR_REG_PCR_ERR_THRESHOLD					131
#define PVR_REG_PCR_FIR_FILTER_COEFF0					132
#define PVR_REG_PCR_FIR_FILTER_COEFF1					133
#define PVR_REG_PCR_FIR_FILTER_COEFF2					134
#define PVR_REG_PCR_FIR_FILTER_COEFF3					135
#define PVR_REG_PCR_EXTENSION						136
#define PVR_REG_PCR_BASE						137
#define PVR_REG_STC_SNAP_EXTENSION					138
#define PVR_REG_STC_SNAP_BASE						139
#define PVR_REG_PCR_ERR_TERM_LOWER					140
#define PVR_REG_PCR_ERR_TERM_UPPER					141
#define	PVR_REG_STC_EXTENSION						142	/* 0x238 */
#define	PVR_REG_STC_BASE						143	/* 0x23c */
#define	PVR_REG_PCR_IIR_XN1						144	/* 0x240 */
#define PVR_REG_PCR_IIR_YN1						145
#define PVR_REG_PCR_IIR_YN2						146
#define PVR_REG_PCR_DV_CTRL						147
#define PVR_REG_PCRSPEED_CTRL0						582	/*0x918 */
#define PVR_REG_PCR2_CTRL						150
#define PVR_REG_PCR2_ERR_THRESHOLD					151
#define PVR_REG_PCR2_FIR_FILTER_COEFF0					152
#define PVR_REG_PCR2_FIR_FILTER_COEFF1					153
#define PVR_REG_PCR2_FIR_FILTER_COEFF2					154
#define PVR_REG_PCR2_FIR_FILTER_COEFF3					155
#define PVR_REG_PCR2_EXTENSION						156
#define PVR_REG_PCR2_BASE						157
#define	PVR_REG_STC2_SNAP_EXTENSION					158
#define PVR_REG_STC2_SNAP_BASE						159
#define	PVR_REG_PCR2_ERR_TERM_LOWER					160
#define PVR_REG_PCR2_ERR_TERM_UPPER					161
#define PVR_REG_STC2_EXTENSION						162	/* 0x288 */
#define PVR_REG_STC2_BASE						163	/* 0x28C */
#define PVR_REG_PCR2_IIR_XN1						164
#define PVR_REG_PCR2_IIR_YN1						165
#define PVR_REG_PCR2_IIR_YN2						166
#define PVR_REG_PCR_ERROR_STATUS_REG				180
#define PVR_REG_PCR_NONERR_STATUS_REG1				181
#define PVR_REG_PCR_NONERR_STATUS_REG2				182
#define PVR_REG_PCR2_ERROR_STATUS_REG				190
#define PVR_REG_PCR2_NONERR_STATUS_REG1				191
#define PVR_REG_PCR2_NONERR_STATUS_REG2				192
#define PVR_REG_DV_CLOCK_RECOVERY				147
#define PVR_REG_PCRSPEED_CTRL1					583	/*0x91C */
#define PVR_REG_PVR_CONTROL					210	/* 0x348 */
#define PVR_REG_PVR_PLAY_BUF_THRESHOLD				211	/* 0x34C */
#define PVR_REG_PVR_RECORD_BUF_THRESHOLD			212	/* 0x350 */
#define PVR_REG_PVR_PLAY_BUF_START				213	/* 0x354 */
#define PVR_REG_PVR_PLAY_BUF_END				214	/* 0x358 */
#define PVR_REG_PVR_PLAY_BUF_RP					215	/* 0x35C */
#define PVR_REG_PVR_PLAY_BUF_WP					216	/* 0x360 */
#define PVR_REG_PVR_TIMESTAMP_THRESHOLD				223	/* 0x37C */
#define PVR_REG_PVR_READ_BUFFER_STATUS				224	/* 0X380 */
#define PVR_REG_PVR_ERROR_STATUS_REG				230	/* 0x398 */
#define PVR_REG_PVR_NONERR_STATUS_REG				231	/* 0x39C */
#define PVR_REG_FTUP_TS_PACKET_FIFO_STATUS			259	/* 0x40C */
#define PVR_REG_DBM_STATUS_REG_3				260	/* 0x410 */
#define PVR_REG_FRAMER0_FSM					261	/* 0x414 */
#define PVR_REG_FRAMER1_FSM					262	/* 0x418 */
#define	PVR_REG_FTUP_PROGRAM_COUNTER				265	/* 0x424 */
#define PVR_REG_CMD_BUS_STATUS					268     //0x430

#define	PVR_REG_FRAMER0_STATUS					269	/*0x434 */
#define	PVR_REG_FRAMER1_STATUS					270	/*0x438 */

#define PVR_DMX_STATUS                          285     //0x474

#define	PVR_REG_REG_FILE_ADDR_REG				512	/* 0x800 */
#define	PVR_REG_LOCAL_ARBITOR_CTRL1				513	/* 0x804 */
#define	PVR_REG_LOCAL_ARBITOR_CTRL2				514	/* 0x808 */
#define PVR_REG_LOCAL_ARBITOR_MONITOR				515	/* 0x80C */
#define	PVR_REG_FRAMER_PREBYTE_CTRL				576	/* 0x900 */
#define	PVR_REG_FRAMER_CTRL1					577	/* 0x904 */
#define PVR_REG_FRAMER_CTRL2					578	/* 0x908 */
#define PVR_REG_PLAYBACK_FRAMER_ERROR_STATUS			579	/*0x90c */
#define PVR_REG_FRAMER_CTRL3					581	/* 0x914 */
#define PVR_REG_FRAMER_UNLOCK_CONTROL				588	/*0X930 */
#define PVR_REG_SECURE_DRAM_START_ADDR				590	/*0X938 */
#define PVR_REG_SECURE_DRAM_END_ADDR				591	/*0X93c */
#define PVR_REG_SECURE_PLAYBACK_PIDINDEX0			592	/*0X940 */
#define PVR_REG_SECURE_PLAYBACK_PIDINDEX1			593	/*0X944 */
#define PVR_REG_SECURE_PLAYBACK_PATTERN				94	/*0X948 */
#define PVR_FMR0						(0)
#define PVR_FMR1						(1)
#define PVR_FMR2						(2)
#define PVR_FMR3						(3)
#define PVR_FMR_DISABLE						(0 << 0)
#define PVR_FMR_ENABLE						(1 << 0)
#define PVR_FMR_SERIAL_INPUT					(0 << 1)
#define PVR_FMR_PARALLEL_INPUT					(1 << 1)
#define PVR_FMR_INTERNAL_SYNC					(0 << 2)
#define PVR_FMR_EXTERNAL_SYNC					(1 << 2)
#define PVR_FMR_NEGATIVE_EDGE					(0 << 3)
#define PVR_FMR_POSITIVE_EDGE					(1 << 3)
/* Demux message queue attributes*/
#define PVR_MSG_PRIORITY_HIGH					0
#define PVR_MSG_PRIORITY_NORMAL					128
#define PVR_MSG_PRIORITY_LOW					255
#define DDI_TS_INDEX_FOR_NO_FRAMER				3
/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/
/* DMX input mode*/
typedef enum {
	PVR_PUSH,
	PVR_HALF_PUSH,
	PVR_PULL
} PVR_INPUT_MODE_T;


/*Ckgen registers*/

/*************************************************************************************/
/*TODO: Need Modified (MTK40144)	-- BEGIN*/
/*************************************************************************************/

typedef enum {
	PVR_CKGEN_EXT_0 = 0,
	PVR_CKGEN_EXT_1 = 1,
	PVR_CKGEN_NUM
} PVR_EXT_CKGEN_T;

typedef enum {
	PVR_PINMUX_EXT_S = 0x10,	/* Serial */
	PVR_PINMUX_EXT_P = 0x11,	/* Parallel */
} PVR_PINMUX_SEL_T;

/*************************************************************************************/
/* TODO: Need Modified (MTK40144)  -- END*/
/*************************************************************************************/

typedef struct {
	u8 u1PIDIdx;
	u32 u4Status;
	u32 u4Status2;
} PVR_FTUP_INT_STATUS_INFO_T;

typedef struct {
	u8 u1PicType;
	uintptr_t ptrPicStartAddr;
} DMX_PIC_INFO_T;

typedef struct {
	PFN_PVR_NOTIFY pfnNotify;
	void *pvPrivData;
} DMX_PID_NTIFY_INFO_T;

/* Demux Hal callback function prototype*/
typedef u32(*DMX_HAL_FUNC_CB) (void *pvData, void *pvUserPrivate);

typedef struct {
	DMX_HAL_FUNC_CB pfnCB;
	void *pvPrivData;
} DMX_HAL_FUNC_INFO_T;

typedef struct {
	u32 u4Pattern0;
	u32 u4Pattern1;
	u32 u4Mask0;
	u32 u4Mask1;
} PVR_STARTCODE_T;

/* PID HW Control Parameter*/
typedef struct _PVR_PIDCFG_INFO_T_ {
	u32 u4HWPIDIndex;	/*/< HW PID Index */

	uintptr_t ptrDstFifoAddr;	/*/< ES Data Output FIFO Address */
	u32 u4DstFifoSize;	/*/< ES Data Output FIFO Size */
	uintptr_t ptrDstFifoWPtr;	/*/< ES Data Output FIFO Write Ptr */
	uintptr_t ptrDstFifoRPtr;	/*/< ES Data Output FIFO Read Ptr */
	uintptr_t ptrDstFifoSPtr;	/*/< ES Data Output FIFO Starting Ptr */
} PVR_PIDCFG_INFO_T;

typedef struct {
	u32 u4SkipLen;
	u32 u4PayloadLen;
} PVR_CMDQ_ENTRY_T;

typedef struct _PTX_CMDQ_INFO_T_ {
	u8 u1CmdNum;
	PVR_CMDQ_ENTRY_T *prEntry;
} PTX_CMDQ_INFO_T;

/* PID structure*/
typedef struct {
	/* PID attributes */
	bool fgEnable;		/* Valid */
	bool fgAllocateBuffer;	/* Allocate buffer or not */
	bool fgPrimary; 	/* Primary PID */
	u8 u1TsIndex;	/* TS index */
	u8 u1DeviceId;	/* Device ID */
	u8 u1KeyIndex;	/* Key index */
	u8 u1SteerMode; /* Steering mode */
	u16 u2Pid;		/* PID */

	/* Buffer info */
	u32 u4BufStart;	/* Buffer start */
	u32 u4BufEnd;	/* Buffer end */
	u32 u4BufLen;	/* Buffer length */
	u32 u4Rp;		/* Read pointer */
	u32 u4SectionRp;	/* Section read pointer */
	u32 u4PesRp;		/* PES read pointer */
	u32 u4Wp;		/* Write pointer */
	u32 u4PeakBufFull;	/* Peak Buffer Fullness, to estimate required */
	/* ES FIFO size */
	u32 u4HeaderBufAddr;	/* Header buffer start */

	
	PVR_PCR_MODE_T ePcrMode;	/* PCR mode */
	PVR_PID_TYPE_T ePidType;	/* PID type */
	PVR_DESC_MODE_T eDescMode;	/* Descramble mode */
	PFN_PVR_NOTIFY pfnNotify;	/* Demux callback function */
	void *pvNotifyTag;	/* Tag value of callback function */
	PFN_PVR_NOTIFY pfnScramble;	/* Scramble state callback function */
	void *pvScrambleTag;	/* Tag value of scramble callback */
} PVR_PID_STRUCT_T;

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/
/*/ Get byte in a word*/
#define PVR_GET_BYTE(arg, n)			((u8 *)&(arg))[n]

/*/ Function entry log*/
#define FUNC_ENTRY				PVR_LOG_DBG(TEXT("[PVR] %s line %d enter\r\n"),\
								DMX_FUNC_NAME, DMX_LINE_NO)
/*/ Function exit log*/
#define FUNC_EXIT				PVR_LOG_DBG(TEXT("[PVR] %s line %d exit\r\n"),\
								DMX_FUNC_NAME, DMX_LINE_NO)

/*/ address increment in ring buffer*/
#define ADDR_INCR_IN_RING(addr, incr, ringstart, ringend)		 \
	((((addr) + (incr)) < (ringend)) ? ((addr) + (incr)) : (((addr) + (incr)) - ((ringend) - (ringstart))))

/*-----------------------------------------------------------------------------*/
/* Prototype	of inter-file functions*/
/*-----------------------------------------------------------------------------*/

/* Initialization*/

EXTERN void _PVR_CkgenInit(bool fgOn, PVR_INPUT_TYPE_T eInputType);

EXTERN MRESULT _PVR_Init(PVR_INPUT_TYPE_T eInputType);

EXTERN void _PVR_Uninit(void);

EXTERN void _PVR_SetIgnorePESLen(bool fgEnable);

EXTERN void _PVR_SetScrambleSchemeEx(PVR_SCRAMBLE_TYPE_T eType, u8 u1Flag);

EXTERN bool _PVR_GetScrambleSchemeEx(PVR_SCRAMBLE_TYPE_T *peType, u8 *pu1Flag);


EXTERN void _PVR_EnablePower(bool fgEnable, PVR_INPUT_TYPE_T eInputType);

EXTERN bool _PVR_Reset(void);

EXTERN void _PVR_SetFramerMode(u8 u1Framer, PVR_FRAMER_MODE_T eMode,
			       bool fgExtSync, bool fgPosEdge);
EXTERN bool _PVR_Framer_130byteEnable(u8 u1Framer, bool fgDemod130byteTs,
				      bool fgInputEnable, u8 u1InputPktSize, bool fgOutputEnable,
				      u8 u1OutputPktSize);
EXTERN void _PVR_BypassErrorHandlingTable_Enable(u8 u1Framer, bool fgEnable);
EXTERN void _PVR_SetFramer(u8 u1FramerIdx, bool fgEnable, bool fgPareallel, bool fgExtSync,
			   bool fgPosEdge);
EXTERN bool _PVR_IsFramerEnabled(u8 u1Framer);

EXTERN void _PVR_EnableFramer(u8 u1FramerIdx, bool fgEnable);
EXTERN bool _PVR_LoadIMem(bool fgForceWrite, const u32 *pu4IData, u32 u4Len);

EXTERN bool _PVR_SetFrontEndEx(u8 u1TsIdx, PVR_FRONTEND_T eFrontEnd);

EXTERN bool _PVR_SetDbmChannel4(bool fg_playback_enable, bool fg_record_enable);
EXTERN bool _PVR_GetDbmChannel4(bool *fg_playback_enable, bool *fg_record_enable);

EXTERN PVR_INPUT_TYPE_T _PVR_GetInputType(void);

EXTERN PVR_FRONTEND_T _PVR_GetFrontEnd(u8 u1TsIdx);

EXTERN void _PVR_ResetFramer(u8 u1Framer);

EXTERN void _PVR_SetFramerEnabled(u8 u1Framer, bool fgEnable);

EXTERN u8 _PVR_GetFramerIndex(void);

EXTERN bool _PVR_ActivateDbmReset(void);

EXTERN bool _PVR_ReleaseDbmReset(void);

EXTERN bool _PVR_ResetDbmSafely(void);

EXTERN void _PVR_ClearInterruptQueue(void);

EXTERN VOID _PVR_DisableDmxInterrupt(VOID);

EXTERN VOID _PVR_EnableDmxInterrupt(VOID);

EXTERN void _PVR_ResetFTuP(void);

EXTERN bool _PVR_EnableFTI(bool fgEnable);

EXTERN MRESULT _PVR_SetInputType(PVR_INPUT_TYPE_T rInputType);

EXTERN bool _PVR_Start(void);

EXTERN bool _PVR_Stop(void);

EXTERN void _PVR_SetInputMode(PVR_INPUT_MODE_T rMode);

EXTERN void _PVR_GetVersion(void);

EXTERN bool _PVR_IsMicroProcessorStopped(void);

EXTERN void _PVR_SetDbm_InputSource(u8 u1TsIdx, PVR_DBM_INPUT_SOURCE_T eSource);


/* Descrambler*/

EXTERN void _PVR_DMEM_CA_Init(void);

EXTERN bool _PVR_SetDmemAesKey(const u16 u2KeyLen, const u8 au1Keys[PVR_DMEM_MM_KEY_LEN]);

EXTERN bool _PVR_SetDmemAesIV(const u16 u2KeyLen, const u8 au1Ivs[PVR_DMEM_MM_IV_LEN]);


/* ISR-related*/

EXTERN MRESULT _PVR_MMIntrHandler(void *pvArg);

EXTERN void _DmxIrqHandler(u16 u2Vector);

EXTERN bool _PVR_InitISR(void);

EXTERN bool _PVR_DeInitISR(void);

/* Handlers*/

/* Video start code*/

EXTERN bool _PVR_VCodeInit(void);

EXTERN bool _PVR_SetVideoType(u8 u1DevID, PVR_VIDEO_TYPE_T eVideoType);

EXTERN PVR_VIDEO_TYPE_T _PVR_GetVideoType(u8 u1DevID);

EXTERN s8 _PVR_GetVCode_Offset(PVR_VIDEO_TYPE_T eVideoType, u8 u1Idx);

/* Utilities*/

EXTERN u8 _PVR_GetByte(u8 **ppu1StartAddr, u32 u4EndAddr,
			  u32 u4BufLen, u32 u4Offset, u8 u1Forward);

EXTERN u32 _PVR_Align(u32 u4Addr, u32 u4Alignment);

EXTERN bool _PVR_IsAligned(u32 u4Addr, u32 u4Alignment);

EXTERN u32 _PVR_CopyRingBuffer(u32 u4Dst, u32 u4DstStart,
				  u32 u4DstEnd, u32 u4Src, u32 u4SrcStart,
				  u32 u4SrcEnd, u32 u4Size);

EXTERN bool _PVR_CopyDestRingBuffer(u32 u4Dst, u32 u4BufStart,
				    u32 u4BufEnd, u32 u4Src, u32 u4Size);



/* Helpers*/

EXTERN void _PVR_Lock(void);
EXTERN void _PVR_Unlock(void);

EXTERN void _PVR_LockApi(void);

EXTERN void _PVR_UnlockApi(void);

EXTERN u32 _PVR_AdvanceAddr(u32 u4Addr, s32 i4Increment, u32 u4Wp,
			       u32 u4FifoStart, u32 u4FifoEnd);

EXTERN u32 _PVR_Align_Dec(u32 u4Addr, u32 u4Alignment, u8 *pu1SkipBytes);

EXTERN u32 _PVR_CopyRingBuffer(u32 u4Dst, u32 u4DstStart,
				  u32 u4DstEnd, u32 u4Src, u32 u4SrcStart,
				  u32 u4SrcEnd, u32 u4Size);

EXTERN void _PVR_DumpDramKeyRegs(bool fgFirstClear);

EXTERN void _PVR_DumpDramLocalArbiter(u32 u4CurStatus);

EXTERN bool _PVR_IsPidEnabled(u8 u1Pidx);

EXTERN MRESULT _PVR_SetGlobalCBFunc(DMX_HAL_FUNC_CB pfnCB, void *pvUserPrivate);

EXTERN MRESULT _PVR_SetPIDFilterNtyFunc(u32 u4PIDIndex, PFN_PVR_NOTIFY pfnNotify,
					void *pvNotifyTag);

EXTERN bool _PVR_Set_PIDChunkSize(u32 u4PIDIdx, u32 u4ChunkSize);

EXTERN MRESULT _PVR_SetTxDstFIFO(PVR_PIDCFG_INFO_T *prPIDCfgInfo);

EXTERN bool _PVR_Set_MM_InsertBytes(u32 u4PIDIdx, PVR_INST_BYTES_INFO_T *prInstBytesInfo);

EXTERN bool _PVR_Set_MM_PIDDRMMode(u32 u4PIDIdx, PVR_DRM_PARAM_T *prDRMParam);

EXTERN bool _PVR_Set_PIDTriggerFlag(u32 u4PIDIdx, bool bEnable);

EXTERN bool _PVR_Set_DefaultPIDDataStruct(void);

EXTERN bool _PVR_Set_BypassPIDIdx(u8 u1TsIndex, u32 u4PIDIdx, bool fgToFTUP);

EXTERN void _PVR_Set_FTuPDMAThreshold(u32 u4DMAThreshold);

EXTERN bool _PVR_SetFramerPacketErrorHandling(u8 u1TsIndex, bool fgEnable, u32 u4Value);

EXTERN bool _PVR_CTInit(void);

EXTERN bool _PVR_LoaduCode(void);

EXTERN PVR_PID_STRUCT_T *_PVR_GetPidStruct(u32 u4PidIndex);

EXTERN DMX_PIC_INFO_T *_PVR_GetPicturesInfo(void);

EXTERN DMX_HAL_FUNC_INFO_T *_PVR_GetGlobalCbInfo(void);

EXTERN PVR_FTUP_INT_STATUS_INFO_T *_PVR_GetFtupIntStatus(void);

EXTERN void _PVR_Enable_CommandQueue(u32 u4PIDIdx);

EXTERN void _PVR_Disable_CommandQueue(u32 u4PIDIdx);

EXTERN void _PVR_Set_CommandQueue_Pointers(u32 u4PIDIdx, u32 u4ItemCount);

EXTERN bool _PVR_Fill_CommandQueue_Items(u32 u4ItemCount, PVR_CMDQ_ENTRY_T *prComQItem);

EXTERN bool _PVR_SetBypassMode(u8 u1TsIndex, u8 u1PacketSize,
			       bool fgSteerToFTuP, bool fgReset);

EXTERN void _PVR_DumpBuffer(const u8 au1Buf[], u32 u4Size, u32 u4BytesPerLine);

EXTERN bool _PVR_DumpPIDDataStruct(u8 u1PidIdx);

EXTERN void _PVR_DumpDMem(u32 u4StartAddr, u32 u4WordCnt);

EXTERN void _PVR_DumpMemory(u32 u4BufSa, u32 u4BufEa, u32 u4Addr, u32 u4BytesCnt);

EXTERN bool _PVR_DumpStartCodePattern_Ex(void);

EXTERN bool _PVR_DumpCmdQInfo(void);

EXTERN void _PVR_DumpRegisters(u32 u41stRegAddr, u32 u4RegsCnt);

EXTERN void _PVR_DumpPESHdrInfo(u8 u1PIDIdx);

EXTERN MRESULT _PVR_PowerOn(PVR_INPUT_TYPE_T eInputType);
EXTERN MRESULT _PVR_PowerDown(void);

EXTERN bool _PVR_SetPowerState(DMX_PM_STATE ePowerState, PVR_INPUT_TYPE_T eInputType);
EXTERN DMX_PM_STATE _PVR_GetPowerState(void);


#endif				/* _PVR_PVR_H_ */
