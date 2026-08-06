/*
 * atc_nand.c - NAND Flash Driver for Autochips SoCs
 *
 * Copyright © Autochips.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/mmc/atc_storage_partition.h>

#include "atc_nand.h"
#include "atc_nfi.h"
#include "atc_ecc.h"
#include <mach/cache_operation.h>
#include <mach/ac83xx_irqs_vector.h>
#include <linux/proc_fs.h>

#include <linux/crc32.h>

extern int nand_markbad_block(char *name, int offset);

#define atc_nand_suspend NULL
#define atc_nand_resume NULL
/*#define USECACHE  1 do not use cache:0   otherwise:1*/
#ifdef USECACHE
#define BSP_FLUSH_IMPORT 1
#endif

#define SQUASHFS_PATCH  0
#define DRV_NAME	"atc-nand"
#define DRV_VERSION	"1.0"
#define DRV_DESC	"ATC on-chip NAND Flash Controller Driver"
#define to_atc_nand(m) container_of(m, struct atc_nand_info, mtd)

/*this value should be not changed until NAND HW changed, IC Designer will notify it.
the address input into NFI_STRADDR = allocate memory addr + NAND_HW_OFFSET*/
#define NAND_HW_OFFSET  0xC0000000

volatile u16 *NFI_BASE;
volatile u16 *NFI_CNFG;
volatile u16 *NFI_PAGEFMT;
volatile u16 *NFI_CON;
volatile u32 *NFI_ACCCON;
volatile u16 *NFI_INTR_EN;
volatile u16 *NFI_INTR;
volatile u16 *NFI_CMD;
volatile u16 *NFI_ADDRNOB;
volatile u32 *NFI_COLADDR;
volatile u32 *NFI_ROWADDR;
volatile u16 *NFI_STRDATA;
volatile u32 *NFI_DATAW;
volatile u32 *NFI_DATAR;
volatile u32 *NFI_STA;
volatile u16 *NFI_FIFOSTA;
volatile u16 *NFI_ADDRCNTR;
volatile u32 *NFI_STRADDR;
volatile u16 *NFI_BYTELEN;
volatile u32 *NFI_FDM0L;
volatile u32 *NFI_FDM0M;
volatile u16 *NFI_CSEL;
#ifdef USE_60BIT
volatile u32 *NFI_RANDOM_CFG;
#endif
volatile u32 *NFI_CLK_SEL;

volatile u32 *NFIECC_BASE;
volatile u16 *NFIECC_ENCCON;
volatile u32 *NFIECC_ENCCNFG;
volatile u32 *NFIECC_ENCDIADDR;
volatile u16 *NFIECC_ENCIDLE;
volatile u16 *NFIECC_DECCON;
volatile u32 *NFIECC_DECCNFG;
volatile u32 *NFIECC_DECDIADDR;
volatile u16 *NFIECC_DECIDLE;
volatile u16 *NFIECC_DECFER;
volatile u32 *NFIECC_DECENUM;
volatile u32 *NFIECC_DECENUM2;
volatile u16 *NFIECC_DECDONE;
volatile u32 *NFIECC_DECEL0;
volatile u16 *NFIECC_DECIRQEN;
volatile u16 *NFIECC_DECIRQSTA;
volatile u32 *NFIECC_FDMADDR;
volatile u32 *NFIECC_DECNFIDI;
/*===================================================================*/
/*===================================================================*/

/*unsigned int SECTOR_BYTES;*/
bool _fgUsingDMA = true;
bool _fgAUTO_FMT = true;
bool _fgECCSWCorrect = false;
bool _fgInitialization = false;
bool _fgDataInvert = false;
bool _fgUsingISR = false;

#define FDM_ECC_ERROR (0xFF)

#include <mach/nand.h>

/*get partition from cmdline*/
static const char *part_probe_types[]
= { "cmdlinepart", NULL };


static uint64_t g_nand_size = 0;

u8 fdm_data_err[FDM_BYTES];
u8 fdm_data_cor[FDM_BYTES];

u32 req_count_total = 0;
u32 req_count_fail = 0;

int atc_nand_remove(struct platform_device *pdev)
{
	struct atc_nand_info *info = platform_get_drvdata(pdev);
	struct mtd_info *mtd = NULL;

	dma_free_coherent(&pdev->dev,
			  (((NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE) +
			    sizeof(struct nand_buffers)) / 32 + 1) * 32 + 32, info->dmabuf,
			  info->dmaaddr);
	mtd = &info->mtd;

	if (mtd) {
		nand_release(mtd);
		kfree(mtd);
	}

	return 0;
}

static void atc_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct atc_nand_info *atcnand = to_atc_nand(mtd);

	memcpy(atcnand->dmabuf + atcnand->datalen, buf, len);
	atcnand->datalen += len;
}


static void atc_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct atc_nand_info *atcnand = to_atc_nand(mtd);

	memcpy(buf, atcnand->dmabuf + atcnand->datalen, len);
	atcnand->datalen += len;

}

static struct nand_ecclayout atc_oobinfo_4096 = {
	.eccbytes = 78,
	.eccpos = {
		   50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
		   60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
		   72, 73, 74, 75, 76, 77, 78, 79,
		   80, 81, 82, 83, 84, 85, 86, 87,
		   88, 89, 90, 91, 92, 93, 94, 95,
		   96, 97, 98, 99, 100, 101, 102, 103,
		   104, 105, 106, 107, 108, 109, 110, 111,
		   112, 113, 114, 115, 116, 117, 118, 119,
		   120, 121, 122, 123, 124, 125, 126, 127},
	.oobfree = {
		    {
		     .offset = 2,
		     .length = 48}
		    }
};

static struct nand_ecclayout atc_oobinfo_8192 = {
	.eccbytes = 78,
	.eccpos = {
		   50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
		   60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
		   72, 73, 74, 75, 76, 77, 78, 79,
		   80, 81, 82, 83, 84, 85, 86, 87,
		   88, 89, 90, 91, 92, 93, 94, 95,
		   96, 97, 98, 99, 100, 101, 102, 103,
		   104, 105, 106, 107, 108, 109, 110, 111,
		   112, 113, 114, 115, 116, 117, 118, 119,
		   120, 121, 122, 123, 124, 125, 126, 127},
	.oobfree = {
		    {
		     .offset = 2,
		     .length = 48}
		    }
};


static struct nand_ecclayout atc_oobinfo_2048 = {
	.eccbytes = 40,
	.eccpos = {
	    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	     46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {{2, 22} }
};


static struct nand_ecclayout atc_oobinfo_512 = {
	.eccbytes = 10,
	.eccpos = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	.oobfree = {{0, 6} }
};

static uint8_t bbt_pattern[] = { 'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = { '1', 't', 'b', 'B' };

static struct nand_bbt_descr atc_bbt_main_descr_512 = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
	    | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs = 0,
	.len = 4,
	.veroffs = 4,
	.maxblocks = 4,
	.pattern = bbt_pattern
};

static struct nand_bbt_descr atc_bbt_mirror_descr_512 = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
	    | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs = 0,
	.len = 4,
	.veroffs = 4,
	.maxblocks = 4,
	.pattern = mirror_pattern
};

static int atc_nand_device_ready(struct mtd_info *mtd)
{
	int result = !(*NFI_STA & STATUS_BUSY);

	return result;
}

static uint8_t atc_nand_read_byte(struct mtd_info *mtd)
{
	uint8_t d;

	atc_nand_read_buf(mtd, &d, 1);

	return d;
}

static bool atc_nand_status_ready(u32 u4Status)
{
        u32 timeout = 0xFFFF;

        while ((*NFI_STA & u4Status) != 0) {
                timeout--;
                if (0 == timeout)
                        return false;
        }
        return true;
}

static bool atc_nand_RFIFOValidSize(u16 size)
{
        u32 timeout = 0xFFFF;

        while (FIFO_RD_REMAIN(*NFI_FIFOSTA) > size) {
                timeout--;
                if (0 == timeout)
                        return false;
        }
        return true;
}

static bool atc_nand_WFIFOValidSize(u16 size)
{
        u32 timeout = 0xFFFF;

        while (FIFO_WR_REMAIN(*NFI_FIFOSTA) > size) {
                timeout--;
                if (0 == timeout)
                        return false;
        }
        return true;
}

static bool atc_nand_reset(void)
{
	*NFI_CON = NFI_RST | FIFO_FLUSH;

	return atc_nand_status_ready(STA_NFI_FSM_MASK | STA_NAND_FSM_MASK | STATUS_BUSY) && atc_nand_RFIFOValidSize(0)
                && atc_nand_WFIFOValidSize(0);
}

static inline void atc_ecc_decode_start(void)
{
	while((*NFIECC_DECIDLE & DEC_IDLE) == 0);
	*NFIECC_DECCON = 0;
	*NFIECC_DECCON = DEC_EN;
}

static inline void atc_ecc_decode_end(void)
{
	while((*NFIECC_DECIDLE & DEC_IDLE) == 0);
	*NFIECC_DECCON = 0;
}

static inline void atc_ecc_encode_start(void)
{
	while((*NFIECC_ENCIDLE & ENC_IDLE) == 0);
	*NFIECC_ENCCON = 0;
	*NFIECC_ENCCON = ENC_EN;
}

static inline void atc_ecc_encode_end(void)
{
	while((*NFIECC_ENCIDLE & ENC_IDLE) == 0);
	*NFIECC_ENCCON = 0;
}
/*------------------------------------------------------------------------------*/
/* Reset Device Callback Function*/
/*------------------------------------------------------------------------------*/
STATUS_E NAND_COMMON_Reset(const struct mtd_info *mtd, const u32 c_timeout)
{
	u32 timeout = c_timeout;
	STATUS_E ret = S_UNKNOWN_ERR;
	s32 i4Val;
	struct atc_nand_info *atcnand = to_atc_nand(mtd);

	/* reset the NFI core state machine, data FIFO and flushing FIFO */
	if(!atc_nand_reset()) {
		pr_err("[nand]%s fail\n", __FUNCTION__);
	}

	*NFI_CNFG = OP_RESET;

	/* enable interrupt */
	*NFI_INTR_EN = RESET_DONE_EN;

	/* reset cmd */
	*NFI_CMD = NAND_CMD_RESET;

	/* wait til CMD is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD complete");


	if (_fgUsingISR) {
		wait_for_completion((struct completion *)(&atcnand->reset_completion));
	} else {
		WAIT_FOR_STATUS_FLAG((*NFI_INTR & RESET_DONE), ATC_NAND_TIMEOUT,
				     "Wait NFI RESET_DONE complete", timeout);
		i4Val = *NFI_INTR;
#ifdef INT_WR_CLR
		*NFI_INTR = i4Val;
#endif

		if (0 != timeout) {
			ret = S_TIMEOUT;
			goto end;
		}
	}

	ret = S_DONE;

end:
	/* disable interrupt */
	*NFI_INTR_EN = 0;
	if(S_DONE != ret)
		pr_err("[nand]%s timeout\n", __FUNCTION__);
	return ret;
}

static void NAND_COMMON_ReadID(struct mtd_info *mtd, unsigned command)
{
	struct atc_nand_info *atcnand = to_atc_nand(mtd);
	uint nfi_pagefmt;
	u32 *p4Data;

	/* reset the NFI core state machine, data FIFO and flushing FIFO */
	if(!atc_nand_reset()) {
		pr_err("[nand]%s fail\n", __FUNCTION__);
	}

	*NFI_CNFG = OP_READ_ID_ST;

	/* always use 8bits I/O interface to read device id */
#ifdef USE_60BIT
	u16 nfi_cnfg;

	nfi_cnfg = *NFI_CNFG;
	*NFI_CNFG = nfi_cnfg & (~DBYTE_EN);
	*NFI_RANDOM_CFG = 0;
#else
	nfi_pagefmt = *NFI_PAGEFMT;
	*NFI_PAGEFMT = (nfi_pagefmt & (~PAGEFMT_16BITS)) | PAGEFMT_8BITS;
#endif

	/* read id cmd */
	*NFI_CMD = NAND_CMD_READID;
	/* wait til CMD is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD complete");

	/* issue addr */
	*NFI_COLADDR = 0;
	*NFI_ROWADDR = 0;
	*NFI_ADDRNOB = 1;
	/* wait til ADDR is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_ADDR), ATC_NAND_TIMEOUT, "Wait STATUS ADDR complete");

	/* set single read, read 8 bytes */
	*NFI_CON = SINGLE_RD | NOB_BYTE_8;
	/* wait til DATA_READ is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_DATAR), ATC_NAND_TIMEOUT, "Wait STATUS ADDR complete");

	p4Data = (u32 *) atcnand->dmabuf;
#if SQUASHFS_PATCH
	p4Data[atcnand->datalen / 4] = *NFI_DATAR;
#else
	/* read full id: 8 byte */
	p4Data[0] = *NFI_DATAR;
	p4Data[1] = *NFI_DATAR;
#endif

#ifdef  USE_60BIT
	*NFI_CNFG = nfi_cnfg;
#else
	*NFI_PAGEFMT = nfi_pagefmt;
#endif

}

/*------------------------------------------------------------------------------*/
/* ECC Error Software Correct*/
/* MUST be 32bits alignment addr */
/*------------------------------------------------------------------------------*/
STATUS_E NAND_COMMON_ECCErrCorrect(const u32 c_timeout, const u32 u4SectIdx, const u32 *p_data32)
{
	u32 timeout;
	u32 i;
	u32 u4ErrNum;
	u32 u4ErrVal;
	u16 u2ErrLoc;
	STATUS_E ret = S_UNKNOWN_ERR;
	u32 u4ErrLoc;

	/* wait for all block decode done */
	WAIT_FOR_STATUS_FLAG((*NFIECC_DECDONE & (1 << u4SectIdx)), ATC_NAND_TIMEOUT,
			     "Wait NFI NFIECC_DECDONE complete", timeout);

	if (0 != timeout) {
		ret = S_TIMEOUT;
		goto end;
	}

	if (u4SectIdx < 4) {
		u4ErrNum = (*NFIECC_DECENUM & (DECENUM_MASK << (u4SectIdx * 8))) >> (u4SectIdx * 8);
	} else {
		u4ErrNum =
		    (*NFIECC_DECENUM2 & (DECENUM_MASK << ((u4SectIdx - 4) * 8))) >> ((u4SectIdx - 4)
										     * 8);
	}

	if (DECENUM_MASK == u4ErrNum) {
		ret = S_ECC_UNCORRECT_ERR;
		goto end;
	} else if (0x0 == u4ErrNum) {
		ret = S_DONE;
		goto end;
	} else {
		for (i = 0; i < u4ErrNum; i++) {
			u2ErrLoc = *(UINT16 *) ((UINT32) NFIECC_DECEL0 + i * 2);

			if ((u2ErrLoc / 8) < SECTOR_BYTES) {
				u4ErrLoc =
				    (UINT32) p_data32 + u4SectIdx * SECTOR_BYTES + u2ErrLoc / 8;
				u4ErrVal = *(UINT8 *) u4ErrLoc;
				u4ErrVal = u4ErrVal & (1 << (u2ErrLoc % 8));

				if (u4ErrVal) {
					*(UINT8 *) u4ErrLoc &= (~u4ErrVal);
				} else {
					*(UINT8 *) u4ErrLoc |= (1 << (u2ErrLoc % 8));
				}
			} else {
				u4ErrLoc =
				    (UINT32) NFI_FDM0L + u4SectIdx * 8 +
				    (((u2ErrLoc / 8) - SECTOR_BYTES) / 4) * 4;
				u4ErrVal = *(UINT32 *) u4ErrLoc;
				u4ErrVal = u4ErrVal & (1 << ((u2ErrLoc - SECTOR_BYTES * 8) % 32));

				if (u4ErrVal) {
					*(UINT32 *) u4ErrLoc &= (~u4ErrVal);
				} else {
					*(UINT32 *) u4ErrLoc |=
					    (1 << ((u2ErrLoc - SECTOR_BYTES * 8) % 32));
				}
			}
		}

		ret = S_ECC_CORRECTABLE_ERR;
	}

end:
	return ret;
}

int atc_nand_ecc_fdm_check(struct mtd_info *mtd, u32 u4SectIdx)
{
	u16 u2ErrLoc;
	u32 page = *NFI_ROWADDR;
	u32 i;
	u32 u4ErrNum;
	u32 err_sect = 0;
	u32 Idx = 0;

	struct atc_nand_info *atcnand = to_atc_nand(mtd);

	for (Idx = 0 ; Idx < u4SectIdx; Idx++)
	{
		if (Idx < 4) {
			u4ErrNum = (*NFIECC_DECENUM & (DECENUM_MASK << (Idx * 8))) >> (Idx * 8);
		} else {
			u4ErrNum =
				(*NFIECC_DECENUM2 & (DECENUM_MASK << ((Idx - 4) * 8))) >> ((Idx - 4)
												 * 8);
		}

		for (i = 0 ; i < u4ErrNum ; i++)
		{
			u2ErrLoc = *(UINT16 *) ((UINT32) NFIECC_DECEL0 + i * 2);
			if (( (u2ErrLoc/ 8) >= SECTOR_BYTES) && ((u2ErrLoc / 8) < (SECTOR_BYTES + atcnand->fdm_ecc_size)))
			{
				if(printk_ratelimit())
					pr_warning("Page 0x%x FDM ECC correct %x off\n",page, (u2ErrLoc / 8));
				return 1;
			}
		}

		if(u4ErrNum)
			err_sect++;
	}

	if (err_sect > 1)
	{
		if(printk_ratelimit())
			pr_warning("Page 0x%x err_sect=%d\n", page, err_sect);
		return 1;
	}
	return 0;
}
/*------------------------------------------------------------------------------*/
/* ECC Error Detect*/
/*------------------------------------------------------------------------------*/
int atc_nand_ecc_errdetect(struct mtd_info *mtd, bool read_oob)
{
	u32 i;
	u32 val_decenum;
	unsigned int errbit_cnt_sector = 0;
	unsigned int max_bitflips = 0;

	if(*NFIECC_DECFER != 0) {
		val_decenum = *NFIECC_DECENUM;/* error number */

		for(i = 0; i < (((*NFI_CON) >> 12) & 0x0F); i++) {
			if(i == 4)
				val_decenum = *NFIECC_DECENUM2;/* error number 2 */

			errbit_cnt_sector = val_decenum & DECENUM_MASK;
			if(DECENUM_MASK == errbit_cnt_sector) {
				if(printk_ratelimit())
					pr_err("Page 0x%x Sector %d with ECC un-correctable Error\n", *NFI_ROWADDR, i);

				mtd->ecc_stats.failed++;
				return -1;//un-correctable

			} else if (errbit_cnt_sector && (read_oob == false)) {//read oob should not inscrease ecc_stats.corrected
				mtd->ecc_stats.corrected += errbit_cnt_sector;
				max_bitflips = max_t(unsigned int, max_bitflips, errbit_cnt_sector);
				//if(printk_ratelimit())
					//pr_warning("Page 0x%x Sector %d with errbit %d\n", *NFI_ROWADDR, i, errbit_cnt_sector);

			}
			val_decenum >>= 8;
		}

		if (atc_nand_ecc_fdm_check(mtd, (((*NFI_CON) >> 12) & 0x0F)))
			max_bitflips = FDM_ECC_ERROR;

	}

	return max_bitflips;
}


/*
 * DMA functions for buffer writing and reading
 */
static irqreturn_t atc_nand_irq(int irq, void *dev_id)
{
	struct atc_nand_info *atcnand = dev_id;

	if (AHB_DONE & *NFI_INTR) {
		*NFI_INTR_EN ^= AHB_DONE_EN;
#ifdef INT_WR_CLR
		*NFI_INTR = AHB_DONE;
#endif
		complete(&atcnand->dma_completion);
	}

	if (ERASE_DONE & *NFI_INTR) {
		*NFI_INTR_EN ^= ERASE_DONE_EN;
#ifdef INT_WR_CLR
		*NFI_INTR = ERASE_DONE;
#endif
		complete(&atcnand->erase_completion);
	}


	if (WR_DONE & *NFI_INTR) {
		*NFI_INTR_EN ^= WR_DONE_EN;
#ifdef INT_WR_CLR
		*NFI_INTR = WR_DONE;
#endif
		complete(&atcnand->program_completion);
	}


	if (RESET_DONE & *NFI_INTR) {
		*NFI_INTR_EN ^= RESET_DONE_EN;
#ifdef INT_WR_CLR
		*NFI_INTR = RESET_DONE;
#endif
		complete(&atcnand->reset_completion);
	}

	return IRQ_HANDLED;
}


/*------------------------------------------------------------------------------*/
/* Read From NFI FIFO*/
/* MUST be 32bits alignment addr */
/*------------------------------------------------------------------------------*/
STATUS_E NAND_COMMON_FIFO_Read(const struct atc_nand_info *atcnand, const u32 c_timeout,
	const bool bUsingDMA, u32 *p_data32, const u32 data_len)
{
	u32 timeout = c_timeout;
	u32 i;

	if (bUsingDMA) {
		/* read page data with DMA */
		/* wait for DMA transmission complete */

#ifdef LINUX_ISR_ENABLE
		if (_fgUsingISR) {
			wait_for_completion((struct completion *)(&atcnand->dma_completion));
		} else
#endif
		{
			/*--------------*/
			/* Trigger DMA action */
			/**NFI_CNFG |= AHB_MODE;*/
			WAIT_FOR_STATUS_FLAG((AHB_DONE == (*NFI_INTR & AHB_DONE)), ATC_NAND_TIMEOUT,
					     "Wait NFI AHB_DONE", timeout);
			*NFI_INTR_EN &= ~AHB_DONE_EN;	/* disable INT first */
			i = *NFI_INTR;	/*read clear again */
#ifdef INT_WR_CLR
			*NFI_INTR = i;
#endif

			if (0 != timeout) {
				return S_TIMEOUT;
			}
		}
	} else {
		/* read page data */
		for (i = 0; i < data_len; i += 4) {
			/* wait for data ready */
			/* when RD_EMPTY_MASK flag is poll-down, it means data is ready in FIFO at least 4 bytes. */
			WAIT_FOR_ZERO_FLAG((*NFI_FIFOSTA & RD_EMPTY_MASK), ATC_NAND_TIMEOUT,
					   "Wait NFI FIFO complete", timeout);

			if (0 != timeout) {
				return S_TIMEOUT;
			}

			*(u32 *) ((u32) p_data32 + i) = *NFI_DATAR;

			if (!((i + 4) % SECTOR_BYTES)) {
				if (_fgECCSWCorrect) {
					STATUS_E ret = S_UNKNOWN_ERR;
					/* AUTO_FMT must be enabled */
					ret = NAND_COMMON_ECCErrCorrect(c_timeout, ((i + 4) / SECTOR_BYTES) - 1,
						p_data32);
					if ((S_DONE != ret) && (S_ECC_CORRECTABLE_ERR != ret)) {
						return ret;
					}
				}
			}
		}
	}

	return S_DONE;

}

/*------------------------------------------------------------------------------*/
/* Read Status Callback Function*/
/*------------------------------------------------------------------------------*/
STATUS_E NAND_COMMON_ReadStatus(const struct mtd_info *mtd, const u32 c_timeout)
{
	struct atc_nand_info *atcnand = to_atc_nand(mtd);
	u32 *p4Data = (u32 *) atcnand->dmabuf;

	if(!atc_nand_reset()) {
		pr_err("[nand]%s fail\n", __FUNCTION__);
	}

	*NFI_CNFG = OP_READ_ID_ST;
#ifdef USE_60BIT
	*NFI_RANDOM_CFG = 0;
#endif

	/* read status cmd */
	*NFI_CMD = NAND_CMD_STATUS;
	/* wait til CMD is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD complete");

	/* set single read by DWORD */
	*NFI_CON = SINGLE_RD | NOB_DWORD;
	/* wait til DATA_READ is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_DATAR), ATC_NAND_TIMEOUT, "Wait STATUS_DATAR complete");

	/* single read doesn't need to polling FIFO */
#if SQUASHFS_PATCH
	p4Data[atcnand->datalen / 4] = *NFI_DATAR;
#else
	p4Data[0] = *NFI_DATAR;
#endif
	return S_DONE;
}

/*------------------------------------------------------------------------------*/
/* Block Erase Related Callback Function*/
/*------------------------------------------------------------------------------*/
STATUS_E NAND_COMMON_BlockErase(const struct mtd_info *mtd, const u32 page_addr)
{

	s32 i4Val;
	u32 timeout;
	u32 addr_cycle, column_addr_bytes, row_addr_bytes;
	STATUS_E ret = S_UNKNOWN_ERR;

	if (mtd->writesize > 512) {
		addr_cycle = 5;
		column_addr_bytes = 2;
	} else {
		addr_cycle = 3;
		column_addr_bytes = 1;
	}

	row_addr_bytes = addr_cycle - column_addr_bytes;


	/* reset the NFI core state machine, data FIFO and flushing FIFO */
	if(!atc_nand_reset()) {
		pr_err("[nand]%s fail\n", __FUNCTION__);
	}

	*NFI_CNFG = OP_ERASE;
	*NFI_INTR_EN = ERASE_DONE_EN;

	/* block erase cmd */
	*NFI_CMD = NAND_CMD_ERASE1;
	/* wait til CMD is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD ERASE1 complete");

	/* fill 1~4 cycle addr, erase command only fill row address, so column bits shift is unnecessary */
	*NFI_COLADDR = 0;
	*NFI_ROWADDR = page_addr;
	/* no. of addr cycle */
	*NFI_ADDRNOB = ROW_ADDR_NOB(row_addr_bytes);
	/* wait til ADDR is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_ADDR), ATC_NAND_TIMEOUT, "Wait STATUS_ADDR complete");

	/* block erase confirm */
	*NFI_CMD = NAND_CMD_ERASE2;
	/* wait til CMD is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD ERASE2 complete");


#ifdef LINUX_ISR_ENABLE

	if (_fgUsingISR) {
		struct atc_nand_info *atcnand = to_atc_nand(mtd);

		wait_for_completion((struct completion *)&atcnand->erase_completion);
	} else
#endif
	{
		WAIT_FOR_STATUS_FLAG((ERASE_DONE == (*NFI_INTR & ERASE_DONE)), ATC_NAND_TIMEOUT,
				     "Wait NFI erase done", timeout);

		*NFI_INTR_EN &= ~ERASE_DONE_EN;	/* disable INT first */
		i4Val = *NFI_INTR;	/* read clear */
#ifdef INT_WR_CLR
		*NFI_INTR = i4Val;
#endif

		if (0 != timeout) {
			ret = S_TIMEOUT;
			goto end;
		}
	}

	ret = S_DONE;

end:

	if (S_DONE != ret) {
		pr_err("\n[nand]%s failed at page 0x%x, ret = 0x%x\n", __FUNCTION__, page_addr, ret);
	}

	*NFI_CON = 0;
	atc_ecc_encode_end();


	return ret;
}

/*------------------------------------------------------------------------------*/
/* Write To NFI FIFO*/
/* MUST be 32bits alignment addr */
/*------------------------------------------------------------------------------*/
STATUS_E NAND_COMMON_FIFO_Write(const struct atc_nand_info *atcnand, const u32 c_timeout, const bool bUsingDMA,
	const u32 *p_data32, const u32 data_len)
{
	u32 timeout = c_timeout;
	u32 i;

	if (bUsingDMA) {
		/* program page data with DMA */

		/* wait for DMA transmission complete */
#ifdef LINUX_ISR_ENABLE
		if (_fgUsingISR) {
			wait_for_completion((struct completion *)&atcnand->dma_completion);
		} else
#endif
		{
			WAIT_FOR_STATUS_FLAG((AHB_DONE == (*NFI_INTR & AHB_DONE)), ATC_NAND_TIMEOUT,
					     "Wait NFI FIFO Write done", timeout);

			*NFI_INTR_EN &= ~AHB_DONE;	/* disable INT */
			i = *NFI_INTR;	/*read clear again */
#ifdef INT_WR_CLR
			*NFI_INTR = i;
#endif

			if (0 != timeout) {
				pr_err("[nand]%s timeout\n", __FUNCTION__);
				return S_TIMEOUT;
			}
		}
	} else {
		/* program page data */
		for (i = 0; i < data_len; i += 4, p_data32++) {
			/* wait for FIFO has space to enqueue */
			/* when WR_FULL_MASK flag is poll-down,*/
			/*it means there are at least 4 bytes free space in FIFO. */
			WAIT_FOR_ZERO_FLAG(*NFI_FIFOSTA & WR_FULL_MASK, ATC_NAND_TIMEOUT,
					   "Wait NFI FIFO Write done", timeout);

			if (0 != timeout) {
				pr_err("[nand]%s timeout\n", __FUNCTION__);
				return S_TIMEOUT;
			}

			*NFI_DATAW = *p_data32;
		}
	}

	return S_DONE;

}

static void atc_nand_cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
	struct atc_nand_info *atcnand = to_atc_nand(mtd);

	/*    u32    page_size = mtd->writesize; */
	/*printk("\nCommand received cmd code=0x%x page address= 0x%x", command, page_addr); */
	if (command == NAND_CMD_ERASE2 || command == NAND_CMD_PAGEPROG) {
		/* Second half of a command we already calculated */
		goto do_command;
	}

	/* Emulate NAND_CMD_READOOB on large-page chips */
	if (mtd->writesize > 512 && command == NAND_CMD_READOOB) {
		/*column += mtd->writesize; */
		command = NAND_CMD_READ0;
	}


	atcnand->data_pos = atcnand->datalen = 8192;
	/* reset the NFI core state machine, data FIFO and flushing FIFO */
	/*writel( NFI_RST|FIFO_FLUSH, atcnand->nfi_mmio + NFI_CON); */

	switch (command) {
	case NAND_CMD_READID:
		NAND_COMMON_ReadID(mtd, command);
		break;

	case NAND_CMD_READ0:
		break;

	case NAND_CMD_STATUS:
		NAND_COMMON_ReadStatus(mtd, MTD_NAND_DEFAULT_TIMEOUT);
		break;

	case NAND_CMD_ERASE1:
		NAND_COMMON_BlockErase(mtd, page_addr);
		break;

	case NAND_CMD_SEQIN:
		break;

	case NAND_CMD_RESET:
		NAND_COMMON_Reset(mtd, MTD_NAND_DEFAULT_TIMEOUT);
		break;

	default:
		BUG();

	}

do_command:
#if SQUASHFS_PATCH
	atcnand->datalen = 8192;
#else
	atcnand->datalen = 0;
#endif


}

static void atc_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	/* Mask the appropriate bit into the stored value of ctl1
	   which will be used by cafe_nand_cmdfunc() */
	if(chipnr)
		*NFI_CSEL = chipnr;
	else
		*NFI_CSEL = chipnr & 0x03;
}

static void atc_nand_bug(struct mtd_info *mtd)
{
	BUG();
}

static int atc_nand_read_page_internal(struct mtd_info *mtd, struct nand_chip *chip, int page, bool read_oob)
{
	struct atc_nand_info *atcnand = to_atc_nand(mtd);
	u32 page_size = mtd->writesize;
	u32 spare_size = mtd->oobsize;
	u32 column_addr_bytes, row_addr_bytes, addr_cycle;
	u32 *p_data32 = (u32 *) atcnand->dmabuf;
	u32 i;
#if SQUASHFS_PATCH
	u32 j;
#endif
	u32 timeout, dec_mask;
	u32 c_timeout = MTD_NAND_DEFAULT_TIMEOUT;
	int ret = 0;
	volatile u32 *p4TmpSpare = (u32 *) (chip->oob_poi);

#ifdef USECACHE
	if (!_fgDirectIO) {
#if BSP_FLUSH_IMPORT
	BSP_InvDCacheRange((UINT32) p_data32, spare_size);
#else
	flush_cache_all();
#endif
	}
#endif

#if SQUASHFS_PATCH
	p_data32[(mtd->writesize / 4) - 1] = 0x00908908;
#endif

	memset(chip->oob_poi, 0xFF, mtd->oobsize);

	if (mtd->writesize > 512) {
		addr_cycle = 5;
		column_addr_bytes = 2;
	} else {
		addr_cycle = 3;
		column_addr_bytes = 1;
	}
	row_addr_bytes = addr_cycle - column_addr_bytes;

	/* reset the NFI core state machine, data FIFO and flushing FIFO */
	if(!atc_nand_reset()) {
		pr_err("[nand]%s fail\n", __FUNCTION__);
	}

	*NFI_CNFG = OP_READ | READ_MODE | HW_ECC_EN;

	if (_fgAUTO_FMT) {
		*NFI_CNFG |= AUTO_FMT_EN;
		*NFIECC_FDMADDR = (u32)(NFI_FDM0L);
	}

	if (page_size == 512) {
		*NFI_CNFG |= SEL_SEC_512BYTE;
	}

	if (CS1 == *NFI_CSEL) {	/* mtk40184 add. RB_CS1 from NFI+0000[7] change to NFI2+0090[4] */
		*NFI_CSEL |= RB_CS1;
	}

	*NFI_CON = SEC_NUM(page_size >> atcnand->nand_sec_shift);
	*NFIECC_DECCNFG = atcnand->para_decode_config;
	atc_ecc_decode_start();

	if (_fgUsingDMA) {
		*NFI_CNFG |= AHB_MODE;
		*NFI_STRADDR = atcnand->dmaaddr + NAND_HW_OFFSET;
	}

	/* read cmd */
	*NFI_CMD = NAND_CMD_READ0;
	/* wait til CMD is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD complete");

	/* fill 1~4 cycle addr */
	*NFI_COLADDR = 0;
	*NFI_ROWADDR = page;
	/* no. of addr cycle */
	*NFI_ADDRNOB = COL_ADDR_NOB(column_addr_bytes) | ROW_ADDR_NOB(row_addr_bytes);

	/* wait til ADDR is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_ADDR), ATC_NAND_TIMEOUT, "Wait NFI STATUS_ADDR complete");

	/* read confirm */
	if (mtd->writesize > 512) {
		*NFI_CMD = NAND_CMD_READSTART;
		WAIT_FOR_ZERO((*NFI_STA & STATUS_BUSY), ATC_NAND_TIMEOUT,
			      "Wait NFI STATUS_BUSY complete");
	}

	if (_fgUsingDMA) {
		*NFI_INTR_EN |= AHB_DONE_EN;
	}

	i = *NFI_INTR;
#ifdef INT_WR_CLR
	*NFI_INTR = i;
#endif

	/* set burst read by DWORD */
	*NFI_CON |= BURST_RD | NOB_DWORD;

	/* read page data */
	ret = NAND_COMMON_FIFO_Read(atcnand, c_timeout, _fgUsingDMA, p_data32, page_size);
	if (ret != S_DONE) {
		if(ret == S_ECC_UNCORRECT_ERR)
			ret = -1;
		if(ret == S_TIMEOUT)
			ret = -EIO;
		pr_err("[MTD][%u]read fail %d \n",__LINE__,ret);
		goto end;
	}

	dec_mask = (1 << (page_size >> atcnand->nand_sec_shift)) - 1;
	/* wait for all block decode done */
	WAIT_FOR_ZERO_FLAG(((*NFIECC_DECDONE & dec_mask) != dec_mask), ATC_NAND_TIMEOUT, "Wait NFI decode done",
			   timeout);

	if (0 != timeout) {
		pr_err("[MTD][%s: %u]NFI_Wait Timeout\n", __func__, __LINE__);
		ret = -EIO;
		goto end;
	}
	/* read spare data */
	if (AUTO_FMT_EN & (*NFI_CNFG)) {
		volatile u32 *pFDMAddr = NFI_FDM0L;
		volatile u32 p4TmpSpare2[4];

		WAIT_FOR_ZERO_FLAG(((*NFI_ADDRCNTR & 0xF000) >> 12) != (page_size >> atcnand->nand_sec_shift),
				   ATC_NAND_TIMEOUT, "Wait NFI spare data ready", timeout);

		if (0 != timeout) {
			ret = -EIO;
			goto end;
		}

		/* read spare data */
		for (i = 0; i < (page_size >> atcnand->nand_sec_shift); i++) {
			p4TmpSpare2[0] = *pFDMAddr++;
			p4TmpSpare2[1] = *pFDMAddr++;
			p4TmpSpare2[2] = *pFDMAddr++;
			p4TmpSpare2[3] = *pFDMAddr++;
			memcpy((void *)p4TmpSpare, (void *)p4TmpSpare2, atcnand->fdm_size);
			p4TmpSpare = (u32 *) ((u32) p4TmpSpare + atcnand->fdm_size);
		}

	} else if (!(*NFI_CNFG & (AHB_MODE | AUTO_FMT_EN))) {
		for (i = 0; i < spare_size; i += 4, p4TmpSpare++) {
			/* when RD_EMPTY_MASK flag is poll-down, it means data is ready in FIFO at least 4 bytes. */
			WAIT_FOR_ZERO_FLAG((*NFI_FIFOSTA & RD_EMPTY_MASK), ATC_NAND_TIMEOUT,
					   "Wait NFI data ready", timeout);

			if (0 != timeout) {
				ret = -EIO;
				goto end;
			}

			*p4TmpSpare = *NFI_DATAR;
		}
	}

	if (*NFI_CNFG & HW_ECC_EN) {
		ret = atc_nand_ecc_errdetect(mtd, read_oob);
		goto end;
	}

end:
	/* disable burst read */
	*NFI_CON = 0x0;
	atc_ecc_decode_end();
	return ret;

}
static void fdm_buff_check(int page, u8 *fdm1, u8 *fdm2, u8 len)
{
	int i = 0;

	for(i=0; i < len ; i++) {
		if(fdm1[i] != fdm2[i])
			pr_info("page=%d fdm[%d] %02x to %02x \n",page, i, fdm1[i], fdm2[i]);
	}
}
static int atc_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{

	int ret = 0;
	u32 crc1,crc2;

	struct atc_nand_info *atcnand = to_atc_nand(mtd);

	ret = atc_nand_read_page_internal(mtd, chip, page, true);
	req_count_total++;
	if (ret == FDM_ECC_ERROR) {
		req_count_fail++;
		crc1 = crc32(0, atcnand->dmabuf, mtd->writesize);
		memcpy(fdm_data_err, (u8 *)(chip->oob_poi), FDM_BYTES);
		_fgUsingDMA = false;
		_fgECCSWCorrect = true;
		atcnand->para_decode_config = DEC_EMPTY_EN | DEC_CON(ECC_DEC_LOCATE) | DEC_NFI_MODE;
		atcnand->para_decode_config |= DEC_TNUM(atcnand->ecc_level) | DEC_CS(((SECTOR_BYTES + atcnand->fdm_ecc_size) << 3)
				+ atcnand->ecc_level * 14) | DEC_NFI_MODE;
		ret = atc_nand_read_page_internal(mtd, chip, page, true);
		crc2 = crc32(0, atcnand->dmabuf, mtd->writesize);

		_fgUsingDMA = true;
		_fgECCSWCorrect = false;
		atcnand->para_decode_config = DEC_EMPTY_EN | DEC_CON(ECC_DEC_CORRECT) | DEC_NFI_MODE;
		atcnand->para_decode_config |= DEC_TNUM(atcnand->ecc_level) | DEC_CS(((SECTOR_BYTES + atcnand->fdm_ecc_size) << 3)
				+ atcnand->ecc_level * 14) | DEC_NFI_MODE;
		memcpy(fdm_data_cor, (u8 *)(chip->oob_poi), FDM_BYTES);
		fdm_buff_check(page, fdm_data_err, fdm_data_cor, FDM_BYTES);
		pr_warning("oob page=%x correct fdm bitflip crc1=0x%x crc2=0x%x \n",page, crc1, crc2);
		if (crc1 == crc2)
			ret = 0;
		else
			WARN_ON(1);
	}

	return ret;
}

BOOL _fgDirectIO = false;

static int atc_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip,
			      uint8_t *buf, int oob_required, int page)
{
	int ret = 0;
	u32 crc1,crc2;
	struct atc_nand_info *atcnand = to_atc_nand(mtd);
	u8 *fmd_data;
	ret = atc_nand_read_page_internal(mtd, chip, page, false);
	req_count_total++;
#if 0
	if (_fgDirectIO) {
		if ((u32) buf < PAGE_OFFSET) {
			if (-S_RAM_ERROR != ret) {
				if (nr_pages == 1) {
					set_page_dirty(pages[0]);
					page_cache_release(pages[0]);
				} else {
					memcpy(buf, atcnand->dmabuf, mtd->writesize);
				}
			}
		}
	} else {
		memcpy(buf, atcnand->dmabuf, mtd->writesize);

	}
#else
	if (ret == FDM_ECC_ERROR) {
		req_count_fail++;
		crc1 = crc32(0, atcnand->dmabuf, mtd->writesize);
		memcpy(fdm_data_err, (u8 *)(chip->oob_poi), FDM_BYTES);
		_fgUsingDMA = false;
		_fgECCSWCorrect = true;
		atcnand->para_decode_config = DEC_EMPTY_EN | DEC_CON(ECC_DEC_LOCATE) | DEC_NFI_MODE;
		atcnand->para_decode_config |= DEC_TNUM(atcnand->ecc_level) | DEC_CS(((SECTOR_BYTES + atcnand->fdm_ecc_size) << 3)
				+ atcnand->ecc_level * 14) | DEC_NFI_MODE;
		ret = atc_nand_read_page_internal(mtd, chip, page, false);
		crc2 = crc32(0, atcnand->dmabuf, mtd->writesize);

		_fgUsingDMA = true;
		_fgECCSWCorrect = false;
		atcnand->para_decode_config = DEC_EMPTY_EN | DEC_CON(ECC_DEC_CORRECT) | DEC_NFI_MODE;
		atcnand->para_decode_config |= DEC_TNUM(atcnand->ecc_level) | DEC_CS(((SECTOR_BYTES + atcnand->fdm_ecc_size) << 3)
				+ atcnand->ecc_level * 14) | DEC_NFI_MODE;
		memcpy(fdm_data_cor, (u8 *)(chip->oob_poi), FDM_BYTES);
		fdm_buff_check(page, fdm_data_err, fdm_data_cor, FDM_BYTES);
		if(printk_ratelimit())
			pr_warning("page=%x correct fdm bitflip crc1=0x%x crc2=0x%x total=%d retry=%d\n",page, crc1, crc2,req_count_total,req_count_fail);
		if (crc1 == crc2)
			ret = 0;
		else
			WARN_ON(1);
	}

	memcpy(buf, atcnand->dmabuf, mtd->writesize);
#endif
	return ret;

}

static int atc_nand_write_page_internal(struct mtd_info *mtd, struct nand_chip *chip,
					const uint8_t *buf, int page, int cached, int raw)
{
	struct atc_nand_info *atcnand = to_atc_nand(mtd);
	u32 page_size, spare_size;
	u32 column_addr_bytes, row_addr_bytes;
	u32 addr_cycle;
	u32 timeout;
	u32 c_timeout = MTD_NAND_DEFAULT_TIMEOUT;
	STATUS_E ret = S_UNKNOWN_ERR;
	u32 i;
	u32 *p_data32 = (u32 *) atcnand->dmabuf;

	page_size = mtd->writesize;
	spare_size = mtd->oobsize;

	if (mtd->writesize > 512) {
		addr_cycle = 5;
		column_addr_bytes = 2;
	} else {
		addr_cycle = 3;
		column_addr_bytes = 1;
	}

	row_addr_bytes = addr_cycle - column_addr_bytes;

	if (!_fgDirectIO) {
		memcpy(atcnand->dmabuf, buf, page_size);
#ifdef USECACHE
#if BSP_FLUSH_IMPORT
		BSP_FlushDCacheRange((UINT32) p_data32, page_size);
#else
		flush_cache_all();
#endif
#endif
	}

	/* reset the NFI core state machine, data FIFO and flushing FIFO */
	if(!atc_nand_reset()) {
		pr_err("[nand]%s fail\n", __FUNCTION__);
	}

	*NFI_CNFG = OP_PROGRAM;

	if (_fgAUTO_FMT) {
		*NFI_CNFG |= AUTO_FMT_EN | HW_ECC_EN;
	}

	if (page_size == 512) {
		*NFI_CNFG |= SEL_SEC_512BYTE;
	}

	*NFIECC_ENCCNFG = atcnand->para_encode_config;

	if (_fgUsingDMA) {
		*NFI_CNFG |= AHB_MODE;
		*NFI_STRADDR = (u32) (atcnand->dmaaddr + NAND_HW_OFFSET);
	}

	atc_ecc_encode_start();

	/* in most 512 page size NAND flash, you have to setup destination pointer to 1st half area */
	if (page_size <= 512) {
		*NFI_CMD = NAND_CMD_READ0;
		/* wait til CMD is completely issued */
		WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD complete");
		*NFI_CON = NFI_RST;
		WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD complete");
	}

	*NFI_CON = SEC_NUM(page_size >> atcnand->nand_sec_shift);
	/* program cmd */
	*NFI_CMD = NAND_CMD_SEQIN;
	/* wait til CMD is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD complete");

	/* fill 1~4 cycle addr */
	*NFI_COLADDR = 0;
	*NFI_ROWADDR = page;
	/* no. of addr cycle */
	/**NFI_ADDRNOB = addr_cycle;*/
	*NFI_ADDRNOB = COL_ADDR_NOB(column_addr_bytes) | ROW_ADDR_NOB(row_addr_bytes);
	/* wait til ADDR is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_ADDR), ATC_NAND_TIMEOUT, "Wait NFI STATUS_ADDR complete");

	/* prepare FDM data */
	if (AUTO_FMT_EN & (*NFI_CNFG)) {
		volatile u32 *pFDMAddr = NFI_FDM0L;
		volatile u32 *pTmpAddr = (u32 *) (chip->oob_poi);
		u32 p4TmpBuf[4];

		memset(p4TmpBuf, 0xff, sizeof(p4TmpBuf));
		for (i = 0; i < (page_size >> atcnand->nand_sec_shift); i++) {
			memcpy((void *)p4TmpBuf, (void *)pTmpAddr, atcnand->fdm_size);
			*pFDMAddr++ = p4TmpBuf[0];
			*pFDMAddr++ = p4TmpBuf[1];
			*pFDMAddr++ = p4TmpBuf[2];
			*pFDMAddr++ = p4TmpBuf[3];
			pTmpAddr = (u32 *) ((u32) pTmpAddr + atcnand->fdm_size);
		}
	}

	if (_fgUsingDMA) {
		*NFI_INTR_EN = AHB_DONE_EN;
	}

	i = *NFI_INTR;		/* read clear */
#ifdef INT_WR_CLR
	*NFI_INTR = i;
#endif

	/* set burst program by DWORD */
	*NFI_CON |= BURST_WR | NOB_DWORD;
	/* wait til DATA_WRITE is completely issued */
	/*while( *NFI_STA  & STATUS_DATAW ); */

	/* program page data */
	ret = NAND_COMMON_FIFO_Write(mtd->priv, c_timeout, _fgUsingDMA, (u32 *) p_data32, page_size);
	if (ret != S_DONE) {
		goto end;
	}

	if (!(*NFI_CNFG & (AUTO_FMT_EN | AHB_MODE))) {
		/* NFI will automatically fetch data for spare under AHB_MODE, and NFI will */
		/* also fetch data from FDM register under AUTO_FMT mode */
		volatile u32 *p_spare32 = (u32 *) (buf + mtd->writesize);
		u32 i;

		for (i = 0; i < spare_size; i += 4, p_spare32++) {
			/* wait for FIFO has space to enqueue */
			timeout = c_timeout;
			WAIT_FOR_ZERO_FLAG((*NFI_FIFOSTA & WR_FULL_MASK), ATC_NAND_TIMEOUT,
					   "Wait NFI read FIFO ", timeout);

			if (0 != timeout) {
				ret = S_TIMEOUT;
				goto end;
			}

			*NFI_DATAW = *p_spare32;
		}
	}

	/* <<<<  WARNING!! >>>> */
	/* 1. You MUST read parity registers before issue program confirm (0x10) command. */
	/*    Since the parity registers will be clean by NFI after issue program confirm. */
	/* 2. You MUST wait until the NFI FIFO is empty! */
	/*    It means all data in the FIFO had been written to NAND flash, and then you can */
	/*    start to read ECC parity registers. */
	/*while(!(*NFI_FIFOSTA & WR_EMPTY_MASK)); */
	/*while ( *NFI_ADDRCNTR & 0x3FF); */
	WAIT_FOR_ZERO_FLAG((((*NFI_ADDRCNTR & 0xF000) >> 12) != (page_size >> atcnand->nand_sec_shift)),
			   ATC_NAND_TIMEOUT, "Wait NFI FIFO empty ", timeout);

	if (0 != timeout) {
		ret = S_TIMEOUT;
		goto end;
	}


	*NFI_INTR_EN |= WR_DONE_EN;
	/* program confirm */
	*NFI_CMD = NAND_CMD_PAGEPROG;
	/* wait til CMD is completely issued */
	WAIT_FOR_ZERO((*NFI_STA & STATUS_CMD), ATC_NAND_TIMEOUT, "Wait NFI CMD complete");

#ifdef LINUX_ISR_ENABLE

	if (_fgUsingISR) {
		struct atc_nand_info *atcnand = to_atc_nand(mtd);

		wait_for_completion((struct completion *)&atcnand->program_completion);
	} else
#endif
	{
		WAIT_FOR_STATUS_FLAG((WR_DONE == (*NFI_INTR & WR_DONE)), ATC_NAND_TIMEOUT,
				     "Wait NFI write page done", timeout);
		*NFI_INTR_EN &= ~WR_DONE;	/* disable INT first */

		if (0 != timeout) {
			ret = S_TIMEOUT;
			goto end;
		}

		i =  *NFI_INTR; //clear
#ifdef INT_WR_CLR
		*NFI_INTR = i;
#endif
	}

	ret = S_DONE;
end:
	if (ret != S_DONE) {
		pr_err("\n[nand]%s timeout\n", __FUNCTION__);
	}

	*NFI_CON = 0;
	atc_ecc_encode_end();
	atcnand->nand.cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	ret = (atcnand->nand.read_byte(mtd) & NAND_STATUS_FAIL) ? S_PGM_FAILED : S_DONE;
	if (ret != S_DONE) {
		pr_err("\n[nand]%s fail: 0x%x\n", __FUNCTION__, ret);
		return -1;
	}

	return 0;
}

static int atc_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			       uint32_t offset, int data_len, const uint8_t *buf,
			       int oob_required, int page, int cached, int raw)
{

	return atc_nand_write_page_internal(mtd, chip, buf, page, cached, raw);
}


static int atc_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	struct atc_nand_info *atcnand = to_atc_nand(mtd);
	int ret = 0;

	ret = atc_nand_write_page_internal(mtd, chip, atcnand->dmabuf, page, 0, 0);
	return ret;
}

static void atc_ioremap(void)
{
	NFI_CNFG = ((volatile u16 *)((u32) NFI_BASE + 0x0000));
	NFI_PAGEFMT = ((volatile u16 *)((u32) NFI_BASE + 0x0004));
	NFI_CON = ((volatile u16 *)((u32) NFI_BASE + 0x0008));
	NFI_ACCCON = ((volatile u32 *)((u32) NFI_BASE + 0x000C));
	NFI_INTR_EN = ((volatile u16 *)((u32) NFI_BASE + 0x0010));
	NFI_INTR = ((volatile u16 *)((u32) NFI_BASE + 0x0014));
	NFI_CMD = ((volatile u16 *)((u32) NFI_BASE + 0x0020));
	NFI_ADDRNOB = ((volatile u16 *)((u32) NFI_BASE + 0x0030));
	NFI_COLADDR = ((volatile u32 *)((u32) NFI_BASE + 0x0034));
	NFI_ROWADDR = ((volatile u32 *)((u32) NFI_BASE + 0x0038));
	NFI_STRDATA = ((volatile u16 *)((u32) NFI_BASE + 0x0040));
	NFI_DATAW = ((volatile u32 *)((u32) NFI_BASE + 0x0050));
	NFI_DATAR = ((volatile u32 *)((u32) NFI_BASE + 0x0054));
	NFI_STA = ((volatile u16 *)((u32) NFI_BASE + 0x0060));
	NFI_FIFOSTA = ((volatile u16 *)((u32) NFI_BASE + 0x0064));
	NFI_ADDRCNTR = ((volatile u16 *)((u32) NFI_BASE + 0x0070));
	NFI_STRADDR = ((volatile u32 *)((u32) NFI_BASE + 0x0080));
	NFI_BYTELEN = ((volatile u16 *)((u32) NFI_BASE + 0x0084));

	NFI_FDM0L = ((volatile u32 *)((u32) NFI_BASE + 0x200));
	NFI_FDM0M = ((volatile u32 *)((u32) NFI_BASE + 0x204));

	NFI_CSEL = ((volatile u16 *)((u32) NFI_BASE + 0x090));

	/*nfiecc */
	NFIECC_ENCCON = ((volatile u16 *)((u32) NFIECC_BASE + 0x0000));
	NFIECC_ENCCNFG = ((volatile u32 *)((u32) NFIECC_BASE + 0x0004));
	NFIECC_ENCDIADDR = ((volatile u32 *)((u32) NFIECC_BASE + 0x0008));
	NFIECC_ENCIDLE = ((volatile u16 *)((u32) NFIECC_BASE + 0x000C));
	NFIECC_DECCON = ((volatile u16 *)((u32) NFIECC_BASE + 0x0100));
	NFIECC_DECCNFG = ((volatile u32 *)((u32) NFIECC_BASE + 0x0104));
	NFIECC_DECDIADDR = ((volatile u32 *)((u32) NFIECC_BASE + 0x0108));
	NFIECC_DECIDLE = ((volatile u16 *)((u32) NFIECC_BASE + 0x010C));
	NFIECC_DECFER = ((volatile u16 *)((u32) NFIECC_BASE + 0x0110));
	NFIECC_DECENUM = ((volatile u32 *)((u32) NFIECC_BASE + 0x0150));
	NFIECC_DECENUM2 = ((volatile u32 *)((u32) NFIECC_BASE + 0x0154));
	NFIECC_DECDONE = ((volatile u16 *)((u32) NFIECC_BASE + 0x0118));
	NFIECC_DECEL0 = ((volatile u32 *)((u32) NFIECC_BASE + 0x160));
	NFIECC_DECIRQEN = ((volatile u16 *)((u32) NFIECC_BASE + 0x0134));
	NFIECC_DECIRQSTA = ((volatile u16 *)((u32) NFIECC_BASE + 0x0138));
	NFIECC_FDMADDR = ((volatile u32 *)((u32) NFIECC_BASE + 0x013C));
	NFIECC_DECNFIDI = ((volatile u32 *)((u32) NFIECC_BASE + 0x0148));
}

struct mtd_info *g_mtd;

static struct atc_nand_pdata *nand_atc_get_pdata(struct platform_device *pdev)
{
	if (!dev_get_platdata(&pdev->dev) && pdev->dev.of_node) {
		struct atc_nand_pdata *pdata;

		pdata = devm_kzalloc(&pdev->dev, sizeof(struct atc_nand_pdata), GFP_KERNEL);
		pdev->dev.platform_data = pdata;

		if (!pdata) {
			return ERR_PTR(-ENOMEM);
		}

		pdev->id = 0;
	}

	return dev_get_platdata(&pdev->dev);
}

static int atc_config_nand(struct atc_nand_info *atcnand)
{
	struct mtd_info *mtd = &atcnand->mtd;
	u32 spare_per_sector;
	u32 nand_sec_size = 0;

	if(mtd->writesize == 512) {
		atcnand->fdm_size = 8;
		atcnand->fdm_ecc_size = 8;
	} else {
		atcnand->fdm_size = FDM_BYTES;
		atcnand->fdm_ecc_size = FDM_ECC_BYTES;
	}
	*NFI_PAGEFMT = FDM_ECC_NUM(atcnand->fdm_ecc_size) | FDM_NUM(atcnand->fdm_size);
	*NFI_PAGEFMT &= ~PAGEFMT_16BITS;//we always use 8-bit interface

	switch (mtd->writesize) {
	case 512:
		nand_sec_size = 512;
		*NFI_CNFG |= SEL_SEC_512BYTE;
		*NFI_PAGEFMT |= PAGEFMT_2K_512;
		break;
	case 2048:
		nand_sec_size = 1024;
		*NFI_CNFG &= ~SEL_SEC_512BYTE;
		*NFI_PAGEFMT |= PAGEFMT_2K_512;
		break;
	case 4096:
		nand_sec_size = 1024;
		*NFI_CNFG &= ~SEL_SEC_512BYTE;
		*NFI_PAGEFMT |= PAGEFMT_4K_2K;
		break;
	case 8192:
		nand_sec_size = 1024;
		*NFI_CNFG &= ~SEL_SEC_512BYTE;
		*NFI_PAGEFMT |= PAGEFMT_8K_4K;
		break;
	default:
		pr_err("[nand]Un-supported page size %d\n", mtd->writesize);
		return  -1;
	}
	
	spare_per_sector = mtd->oobsize / (mtd->writesize/nand_sec_size);
	if(spare_per_sector == 56) {//for same to preloader setting
		spare_per_sector = 52;
	}
	
	if(nand_sec_size == 1024) {
		atcnand->nand_sec_shift = 10;
		switch(spare_per_sector){
		case 32:
			*NFI_PAGEFMT |= SPARE_32_16;
			atcnand->ecc_level = ECC_12_BITS;
			break;
		case 52:
			*NFI_PAGEFMT |= SPARE_52_26;
			atcnand->ecc_level = ECC_24_BITS;
			break;
		case 54:
			*NFI_PAGEFMT |= SPARE_54_27;
			atcnand->ecc_level = ECC_24_BITS;
			break;
		case 56:
			*NFI_PAGEFMT |= SPARE_56_28;
			atcnand->ecc_level = ECC_24_BITS;
			break;
		default:
			pr_err("[nand]spare size(%d) is not configured\n", spare_per_sector);
			return -1;
		}
	} else {//512Byte sector
		atcnand->nand_sec_shift = 9;
		atcnand->ecc_level = ECC_4_BITS;
		switch(spare_per_sector){
		case 16:
			*NFI_PAGEFMT |= SPARE_32_16;
			break;
		case 26:
			*NFI_PAGEFMT |= SPARE_52_26;
			break;
		case 27:
			*NFI_PAGEFMT |= SPARE_54_27;
			break;
		case 28:
			*NFI_PAGEFMT |= SPARE_56_28;
			break;
		default:
			pr_err("[nand]spare size(%d) is not configured\n", spare_per_sector);
			return -1;
		}
	}

	printk("[nand]Select %d bits ECC\n", atcnand->ecc_level);

	/* config ENC and DEC registers*/
	atcnand->para_encode_config = ENC_TNUM(atcnand->ecc_level) | ENC_MS((SECTOR_BYTES + atcnand->fdm_ecc_size) << 3) | ENC_NFI_MODE;
	if (_fgECCSWCorrect) {
		atcnand->para_decode_config = DEC_EMPTY_EN | DEC_CON(ECC_DEC_LOCATE) | DEC_NFI_MODE;
	} else {
		atcnand->para_decode_config = DEC_EMPTY_EN | DEC_CON(ECC_DEC_CORRECT) | DEC_NFI_MODE;
	}

	atcnand->para_decode_config |= DEC_TNUM(atcnand->ecc_level) | DEC_CS(((SECTOR_BYTES + atcnand->fdm_ecc_size) << 3)
				+ atcnand->ecc_level * 14) | DEC_NFI_MODE;


	return 0;

}

#ifdef CONFIG_NAND_ATC_SW_WP
extern int nand_set_wp(char *name,int enable);
extern void nand_wp_info(void);
extern void nand_clear_all_wp(void);
extern void nand_restore_default_wp(void);

static int nand_debug_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "\n=============================\nUsage:\n=============================\n");
	seq_puts(m, "set_wp partition_name [1/0]  \n\t==>set partition wp enable(1)/disable(0) \n\n");
	seq_puts(m, "wp_info   \n\t==> show each partition write protection status \n\n");
	seq_puts(m, "clear_all_wp  \n\t==> clear all partition write protection\n\n");
	seq_puts(m, "restore_def_wp  \n\t==>  restore defalut wp acorrding to partition excel\n\n");
	return 0;
}
static int nand_debug_proc_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	int ret = 0;
	char *cmd_buff = NULL;
	int arg1=0, arg2=0, arg3=0;
	char cmd[CMD_MAX_SIZE];
	char *arg;
	char name[32];
	
	if(size == 0) {
		ret = -EINVAL;
		goto fail_malloc;
	}

	if(size > CMD_BUFF_SIZE-1)
		size = CMD_BUFF_SIZE-1;

	cmd_buff = kzalloc(size, GFP_KERNEL);
	if (!cmd_buff) {
		ret = -ENOMEM;
		pr_err("alloc buffer fail");
		goto fail_malloc;
	}
	memset(cmd, 0, CMD_MAX_SIZE);

	if(copy_from_user(cmd_buff, buf, size) < 0) {
		pr_err("copy_from_user fail");
		ret = -EFAULT;
		goto fail;
	}

	if(sscanf(cmd_buff, "%s", &cmd[0]) < 1) {
		pr_err("parse cmd fail");
		ret = -EINVAL;
		goto fail;
	}

	arg = cmd_buff + strlen(cmd);

	pr_info("cmd: %s, arg: %s\n", cmd, arg);

	if(0 == strncmp(cmd, "set_wp", 6)) {
		sscanf(arg, "%s %d", name,&arg1);
		nand_set_wp(name,arg1);
	} else if(0 == strncmp(cmd, "wp_info", 11)) {
		nand_wp_info();
	} else if(0 == strncmp(cmd, "clear_all_wp", 11)) {
		nand_clear_all_wp();
	} else if(0 == strncmp(cmd, "restore_def_wp", 14)) {
		nand_restore_default_wp();
	} else if(0 == strncmp(cmd, "mark_badblock", 13)) {
		sscanf(arg, "%s %x", name, &arg1);
		pr_info("name: %s, arg: 0x%x\n", name, arg1);
		nand_markbad_block(name, arg1);
	} else if(0 == strncmp(cmd, "req_count", 9)) {
		pr_info("req_count_total=%d, req_count_fail=%d\n", req_count_total, req_count_fail);
	}

	ret = size;
fail:
	kfree(cmd_buff);
fail_malloc:
	return ret;
}


static int nand_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, nand_debug_proc_show, inode->i_private);
}

static const struct file_operations nand_proc_fops = {
	.open = nand_proc_open,
	.write = nand_debug_proc_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int nand_debug_proc_init(void)
{
	struct proc_dir_entry *prEntry;

	/* proc/msdc_debug node */
	prEntry = proc_create("nand_wp", 0660, NULL, &nand_proc_fops);

	if (!prEntry) {
		 pr_err("failed to create /proc/nand_wp");
	}

	return 0;
}
#endif

static int nand_size_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%llu\n", (unsigned long long)g_nand_size);

	return 0;
}

static int nand_size_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, nand_size_proc_show, inode->i_private);
}

static const struct file_operations nand_size_proc_fops = {
	.open = nand_size_proc_open,
	.write = seq_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int nand_size_proc_init(void)
{
	struct proc_dir_entry *prEntry;

	/* proc/nand_size node */
	prEntry = proc_create("nand_size", 0660, NULL, &nand_size_proc_fops);

	if (!prEntry) {
		 pr_err("failed to create /proc/nand_size");
	}

	return 0;
}

static int atc_nand_probe(struct platform_device *pdev)
{
	struct atc_nand_pdata *pdata = NULL;
	struct atc_nand_info *atcnand = NULL;
	struct resource *res0;
	struct resource *res1;
	struct clk *nand_parent_clk = NULL;
	int err = 0;
	int ret = 0;

	pdata = nand_atc_get_pdata(pdev);
	if (IS_ERR(pdata)) {
		return PTR_ERR(pdata);
	}
	if (pdata == NULL) {
		return -ENODEV;
	}

	atcnand = devm_kzalloc(&pdev->dev, sizeof(*atcnand), GFP_KERNEL);
	if (!atcnand) {
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, atcnand);

	res0 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	res1 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res0 || !res1) {
		dev_err(&pdev->dev, "resource missing\n");
		return -EINVAL;
	}

	/*get reg bank 0 */
	atcnand->nfi_regs = devm_ioremap_resource(&pdev->dev, res0);
	if (IS_ERR(atcnand->nfi_regs)) {
		return PTR_ERR(atcnand->nfi_regs);
	}

	atcnand->ecc_regs = devm_ioremap_resource(&pdev->dev, res1);
	if (IS_ERR(atcnand->nfi_regs)) {
		return PTR_ERR(atcnand->nfi_regs);
	}

	NFI_BASE = atcnand->nfi_regs;
	NFIECC_BASE = atcnand->ecc_regs;
	atc_ioremap();

#ifdef HARDWARE_INIT
	atcnand->nand_clk = devm_clk_get(&pdev->dev, "nandflash-device");
	if (atcnand->nand_clk == NULL) {
		pr_err("nand flash get clk failed\n");
		err = -ENODEV;
		goto out_free_mtd;
	}

	nand_parent_clk = clk_get(NULL, "armpll_d4");
	if (nand_parent_clk == NULL) {
		pr_err("get nand fland clock failed\n");
		err = -ENODEV;
		goto out_free_mtd;
	}

	ret = clk_set_parent(atcnand->nand_clk, nand_parent_clk);
	if (ret < 0) {
		err = -ENODEV;
		goto out_free_mtd;
	}

	atcnand->clk_onoff = devm_clk_get(&pdev->dev, "nandflash-clk-onoff");
	if (atcnand->clk_onoff == NULL) {
		err = -ENODEV;
		goto out_free_mtd;
	}

	err = clk_prepare_enable(atcnand->clk_onoff);
	if (err < 0) {
		pr_err("nand flash clk enable failed\n");
		goto out_free_mtd;
	}

	*NFI_ACCCON = 0xFFFFFFFF;
	*NFI_ACCCON = 0x288;
#endif
	pr_info(" NFI ACCON = %x \n", *NFI_ACCCON);
	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	atcnand->dmabuf = dma_alloc_coherent(&pdev->dev,
		(((NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE) + sizeof(struct nand_buffers)) / 32 + 1) * 32 + 32,
		&atcnand->dmaaddr, GFP_KERNEL);

	if (!atcnand->dmabuf) {
		err = -ENOMEM;
		goto out_free_mtd;
	}

	atcnand->mtd.priv = &atcnand->nand;
	atcnand->mtd.name = "atcnand";
	atcnand->nand.cmdfunc = atc_nand_cmdfunc;
	atcnand->nand.dev_ready = atc_nand_device_ready;
	atcnand->nand.read_byte = atc_nand_read_byte;
	atcnand->nand.read_buf = atc_nand_read_buf;
	atcnand->nand.write_buf = atc_nand_write_buf;
	atcnand->nand.select_chip = atc_nand_select_chip;
	atcnand->nand.chip_delay = 0;
	/* Enable the following for a flash based bad block table */
	atcnand->nand.options = NAND_NO_SUBPAGE_WRITE;
	init_completion(&atcnand->dma_completion);	/* add by mtk40148 */
	init_completion(&atcnand->erase_completion);
	init_completion(&atcnand->program_completion);
	init_completion(&atcnand->reset_completion);
#ifdef LINUX_ISR_ENABLE
	err = request_irq(VECTOR_NFI, atc_nand_irq, IRQF_SHARED, "ATC NFI driver", (void *)atcnand);

	if (err < 0) {
		pr_err("request irq error\n");
		goto out_free_mtd;
	}
#endif

	/* Scan to find existence of the device */
	if (nand_scan_ident(&atcnand->mtd, 1, NULL)) {
		err = -ENXIO;
		goto out_free_mtd;
	}

	err = atc_config_nand(atcnand);
	if (err < 0) {
		pr_err("config nand fail\n");
		goto out_free_mtd;
	}

	atcnand->nand.bbt_td = &atc_bbt_main_descr_512;
	atcnand->nand.bbt_md = &atc_bbt_mirror_descr_512;

	/* Set up ECC according to the type of chip we found */
	if (atcnand->mtd.writesize == 2048) {
		atcnand->nand.ecc.layout = &atc_oobinfo_2048;
		atcnand->nand.ecc.bytes = 40;
	} else if (atcnand->mtd.writesize == 512) {
		atcnand->nand.ecc.layout = &atc_oobinfo_512;
		atcnand->nand.ecc.bytes = 10;
	} else if (atcnand->mtd.writesize == 4096) {
		atcnand->nand.ecc.layout = &atc_oobinfo_4096;
		atcnand->nand.ecc.bytes = 78;
	} else if (atcnand->mtd.writesize == 8192) {
		atcnand->nand.ecc.layout = &atc_oobinfo_8192;
		atcnand->nand.ecc.bytes = 78;
	} else {
		pr_err("Unexpected NAND flash writesize %d. Aborting\n",
		       atcnand->mtd.writesize);
		goto out_free_mtd;
	}

	atcnand->nand.ecc.mode = NAND_ECC_HW;
	atcnand->nand.bbt_options |= NAND_BBT_USE_FLASH;
	atcnand->nand.ecc.strength = atcnand->ecc_level - 2;//set default bitfilp_threshold value
	atcnand->nand.ecc.size = atcnand->mtd.writesize;
	atcnand->nand.ecc.hwctl = (void *)atc_nand_bug;
	atcnand->nand.ecc.calculate = (void *)atc_nand_bug;
	atcnand->nand.ecc.correct = (void *)atc_nand_bug;
	atcnand->nand.write_page = atc_nand_write_page;
	atcnand->nand.ecc.write_page = (void *)atc_nand_bug;
	atcnand->nand.ecc.write_page_raw = (void *)atc_nand_bug;
	atcnand->nand.ecc.write_oob = atc_nand_write_oob;
	atcnand->nand.ecc.read_page = atc_nand_read_page;
	atcnand->nand.ecc.read_oob = atc_nand_read_oob;
	atcnand->nand.ecc.read_page_raw = atc_nand_read_page;

	err = nand_scan_tail(&atcnand->mtd);
	if (err < 0) {
		pr_err("add tail error\n");
		goto out_free_mtd;
	}
	
	if (is_power_of_2(atcnand->mtd.erasesize))
		atcnand->mtd.erasesize_shift = ffs(atcnand->mtd.erasesize) - 1;
	else
		atcnand->mtd.erasesize_shift = 0;
	
	if (is_power_of_2(atcnand->mtd.writesize))
		atcnand->mtd.writesize_shift = ffs(atcnand->mtd.writesize) - 1;
	else
		atcnand->mtd.writesize_shift = 0;
	
	atcnand->mtd.erasesize_mask = (1 << atcnand->mtd.erasesize_shift) - 1;
	atcnand->mtd.writesize_mask = (1 << atcnand->mtd.writesize_shift) - 1;
	/* We register the whole device first, separate from the partitions */
	if (pdata->partitions)
		ret = mtd_device_parse_register(&atcnand->mtd, NULL, NULL,
						pdata->partitions, pdata->nr_partitions);
	else {
		struct mtd_part_parser_data ppdata;

		ppdata.of_node = pdev->dev.of_node;
		ret = mtd_device_parse_register(&atcnand->mtd, part_probe_types, &ppdata, NULL, 0);
		if (ret < 0) {
			pr_err("mtd add error\n");
		}
	}
	g_nand_size = atcnand->nand.chipsize;
#ifdef CONFIG_NAND_ATC_SW_WP	
	nand_debug_proc_init();
#endif
	nand_size_proc_init();
	goto out;

out_free_mtd:
	/*   kfree(atcnand->dmabuf); */
out:
	return err;
}

int atc_nand_init_att(struct nand_chip *chip, struct nand_atc_bbt *atcbbt)
{
	u32 logblk;
	u16 phyblk;
	u16 bbidx = 0;

	struct nand_log2phy_tbl *att = (struct nand_log2phy_tbl *)chip->priv;

	if (atcbbt->realnum) {
		u16 firstrsvb = att->blocknum;

		if (atcbbt->rsvbn) {
			firstrsvb = (u16) atcbbt->phyidx[0];
		}

		phyblk = 0;

		for (logblk = 0; phyblk < firstrsvb; logblk++, phyblk++) {
			/* Find a goog physical block for current logical block */
			while ((atcbbt->realnum > bbidx) && (atcbbt->badblocks[bbidx] == phyblk)) {
				phyblk++;
				bbidx++;
			}

			att->log2phytbl[logblk] = phyblk;
		}

		for (; logblk < att->blocknum; logblk++) {
			att->log2phytbl[logblk] = 0xFFFF;
		}
	}

	for (bbidx = 0; bbidx < atcbbt->rsvbn; bbidx++) {
		if (atcbbt->logidx[bbidx] < att->blocknum) {
			att->log2phytbl[atcbbt->logidx[bbidx]] = atcbbt->phyidx[bbidx];
		}
	}

	logblk = atcbbt->realnum + atcbbt->rsvbn;
	logblk *= g_mtd->erasesize;
	g_mtd->size -= logblk;
	pr_info("ATT Bad Block(%d) Reserved Blocks(%d) MTD Size(0x%llx)\r\n", atcbbt->realnum,
	       atcbbt->rsvbn, g_mtd->size);

	return 0;
}


void atc_nand_shutdown(struct platform_device *pdev)
{
	
	struct atc_nand_info *info = platform_get_drvdata(pdev);
	struct mtd_info *mtd = NULL;

	mtd = &info->mtd;

	if (mtd) {
		nand_release(mtd);
		kfree(mtd);
	}
	pr_info("=== atc nand shutdown end === \n");
}


static const struct of_device_id atc_nand_of_match[] = {
	{.compatible = "Autochips,ac83xx-nandflash",},
	{},
};

MODULE_DEVICE_TABLE(of, atc_nand_of_match);

static struct platform_driver atc_nand_driver = {
	.probe = atc_nand_probe,
	.remove = atc_nand_remove,
	.suspend = atc_nand_suspend,
	.resume = atc_nand_resume,
	#ifdef CONFIG_PM
	.shutdown = atc_nand_shutdown,
	#endif
	.driver = {
		   .name = "atc_nand",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(atc_nand_of_match),
		   },
};

MODULE_ALIAS("platform:atc_nand");
module_platform_driver(atc_nand_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("AUTOCHIPS");
MODULE_DESCRIPTION("ATC NAND flash driver");
