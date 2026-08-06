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

#ifndef __HW_IRT_DMA_H__
#define __HW_IRT_DMA_H__

#include "x_hal_ic.h"
#include "drv_config.h"
#include "chip_ver.h"
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <asm/page.h>
#include <linux/of_reserved_mem.h>
#include <io.h>
#include <linux/interrupt.h>





//#define IRT_DMA_BASE                                    0xfd046000//(IO_BASE + 0x46000)


// must use IO_READ32 or IO_WRITE32 to control IRT_DMA HW registers
#define IRT_DMA_REG_TIRG          (0x00)
#define IRT_DMA_REG_CTRL          (0x04)
#define IRT_DMA_REG_HVSIZE        (0x08)
#define IRT_DMA_REG_DRAM_Y_RD_STA   (0x0C)
#define IRT_DMA_REG_DRAM_Y_WR_STA   (0x10)
#define IRT_DMA_REG_DRAM_C_RD_STA   (0x14)
#define IRT_DMA_REG_DRAM_C_WR_STA   (0x18)

#define IRT_DMA_REG_WR_OFFSET     (0x1C)
#define IRT_DMA_REG_MON           (0x20)


//------------------------------------------------
//IRT_DMA HW registers' shift
//   [IRT_DMA_TIRG] - 0x27000
#define IRT_DMA_SHT_RG_5351_MODE_EN   24
#define IRT_DMA_SHT_RG_5351_MODE_SEL  20
#define IRT_DMA_SHT_RG_SCAN_LINE      16
//#define IRT_DMA_SHT_RG_ALIGN_MODE     13
#define IRT_DMA_SHT_RG_DONE_SEL       12
#define IRT_DMA_SHT_RG_DMA_MODE       4
#define IRT_DMA_SHT_RG_BURST_BLOCK_EN 1
#define IRT_DMA_SHT_RG_START          0

//   [IRT_DMA_CTRL] - 0x27004
#define IRT_DMA_SHT_RG_DRAM_CLK_EN    31
#define IRT_DMA_SHT_RG_MON_SEL        4
#define IRT_DMA_SHT_RG_RESET_B        0


//   [IRT_DMA_HVSIZE] - 0x27008
#define IRT_DMA_SHT_RG_VSIZE          16
#define IRT_DMA_SHT_RG_HSIZE          0

//   [IRT_DMA_DRAM_RD_STA] - 0x2700C
#define IRT_DMA_SHT_DRAM_Y_RD_STA       0

//   [IRT_DMA_DRAM_WR_STA] - 0x27010
#define IRT_DMA_SHT_DRAM_Y_WR_STA       0

//   [IRT_DMA_DRAM_RD_STA] - 0x27014
#define IRT_DMA_SHT_DRAM_C_RD_STA       0

//   [IRT_DMA_DRAM_WR_STA] - 0x27018
#define IRT_DMA_SHT_DRAM_C_WR_STA       0

//   [IRT_DMA_DRAM_WR_OFFSET] - 0x2701C
#define IRT_DMA_SHT_RG_WR_OFFSET      16
#define IRT_DMA_SHT_RG_RD_OFFSET      0


//   [IRT_DMA_DRAM_MON] - 0x27020
#define IRT_DMA_SHT_MON               0



//------------------------------------------------
// IRT_DMA HW registers' mask
//   [IRT_DMA_TIRG] - 0x27000
#define IRT_DMA_MSK_RG_5351_MODE_EN   ((UINT32)0x1 << IRT_DMA_SHT_RG_5351_MODE_EN)
#define IRT_DMA_MSK_RG_5351_MODE_SEL  ((UINT32)0x3 << IRT_DMA_SHT_RG_5351_MODE_SEL)
#define IRT_DMA_MSK_RG_SCAN_LINE      ((UINT32)0x1 << IRT_DMA_SHT_RG_SCAN_LINE)
//#define IRT_DMA_MSK_RG_ALIGN_MODE   ((UINT32)0x1 << IRT_DMA_SHT_RG_ALIGN_MODE)
#define IRT_DMA_MSK_RG_DONE_SEL     ((UINT32)0x1 << IRT_DMA_SHT_RG_DONE_SEL)
#define IRT_DMA_MSK_RG_DMA_MODE     ((UINT32)0xF << IRT_DMA_SHT_RG_DMA_MODE)
#define IRT_DMA_MSK_RG_START        ((UINT32)0x1 << IRT_DMA_SHT_RG_START)
#define IRT_DMA_MSK_RG_BURST_BLOCK_EN ((UINT32)0x1 <<IRT_DMA_SHT_RG_BURST_BLOCK_EN)

//   [IRT_DMA_CTRL] - 0x27004
#define IRT_DMA_MSK_RG_DRAM_CLK_EN  ((UINT32)0x1 << IRT_DMA_SHT_RG_DRAM_CLK_EN)
#define IRT_DMA_MSK_RG_MON_SEL      ((UINT32)0x7 << IRT_DMA_SHT_RG_MON_SEL)
#define IRT_DMA_MSK_RG_RESET_B      ((UINT32)0x1 << IRT_DMA_SHT_RG_RESET_B)

//   [IRT_DMA_HVSIZE] - 0x27008
#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)  //add@jgao 3363support 4k*2k
#define IRT_DMA_MSK_RG_VSIZE        ((UINT32)0x1FFF << IRT_DMA_SHT_RG_VSIZE)
#define IRT_DMA_MSK_RG_HSIZE        ((UINT32)0x1FFF << IRT_DMA_SHT_RG_HSIZE)
#else
#define IRT_DMA_MSK_RG_VSIZE        ((UINT32)0x7FF << IRT_DMA_SHT_RG_VSIZE)
#define IRT_DMA_MSK_RG_HSIZE        ((UINT32)0x7FF << IRT_DMA_SHT_RG_HSIZE)
#endif

//   [IRT_DMA_DRAM_RD_STA] - 0x2700C
#define IRT_DMA_MSK_DRAM_Y_RD_STA     ((UINT32)0xFFFFFFF << IRT_DMA_SHT_DRAM_Y_RD_STA)

//   [IRT_DMA_DRAM_WR_STA] - 0x27010
#define IRT_DMA_MSK_DRAM_Y_WR_STA     ((UINT32)0xFFFFFFF << IRT_DMA_SHT_DRAM_Y_WR_STA)

//   [IRT_DMA_DRAM_RD_STA] - 0x27014
#define IRT_DMA_MSK_DRAM_C_RD_STA     ((UINT32)0xFFFFFFF << IRT_DMA_SHT_DRAM_C_RD_STA)

//   [IRT_DMA_DRAM_WR_STA] - 0x27018
#define IRT_DMA_MSK_DRAM_C_WR_STA     ((UINT32)0xFFFFFFF << IRT_DMA_SHT_DRAM_C_WR_STA)


//   [IRT_DMA_DRAM_WR_OFFSET] - 0x2701C
#define IRT_DMA_MSK_RG_WR_OFFSET    ((UINT32)0x3FF << IRT_DMA_SHT_RG_WR_OFFSET)
#define IRT_DMA_MSK_RG_RD_OFFSET    ((UINT32)0x3FF << IRT_DMA_SHT_RG_RD_OFFSET)

//   [IRT_DMA_DRAM_MON] - 0x27020
#define IRT_DMA_MSK_MON             ((UINT32)0xFFFFFFFF << IRT_DMA_SHT_MON)



//------------------------------------------------
// IRT_DMA HW registers' fld
//   [IRT_DMA_TIRG] - 0x27000
#define IRT_DMA_FLD_RG_5351_MODE_EN(x)    (((UINT32)(x) << IRT_DMA_SHT_RG_5351_MODE_EN)&(IRT_DMA_MSK_RG_5351_MODE_EN))
#define IRT_DMA_FLD_RG_5351_MODE_SEL(x)   (((UINT32)(x) << IRT_DMA_SHT_RG_5351_MODE_SEL)&(IRT_DMA_MSK_RG_5351_MODE_SEL))
#define IRT_DMA_FLD_RG_SCAN_LINE(x)       (((UINT32)(x) << IRT_DMA_SHT_RG_SCAN_LINE)&(IRT_DMA_MSK_RG_SCAN_LINE))
//#define IRT_DMA_FLD_RG_ALIGN_MODE(x)   (((UINT32)(x) << IRT_DMA_SHT_RG_ALIGN_MODE)&(IRT_DMA_MSK_RG_ALIGN_MODE))
#define IRT_DMA_FLD_RG_DONE_SEL(x)   (((UINT32)(x) << IRT_DMA_SHT_RG_DONE_SEL)&(IRT_DMA_MSK_RG_DONE_SEL))
#define IRT_DMA_FLD_RG_DMA_MODE(x)   (((UINT32)(x) << IRT_DMA_SHT_RG_DMA_MODE)&(IRT_DMA_MSK_RG_DMA_MODE))
#define IRT_DMA_FLD_RG_BURST_READ(x)  (((UINT32)(x) << IRT_DMA_SHT_RG_BURST_BLOCK_EN)&(IRT_DMA_MSK_RG_BURST_BLOCK_EN)) //add@jgao
#define IRT_DMA_FLD_RG_START(x)      (((UINT32)(x) << IRT_DMA_SHT_RG_START)&(IRT_DMA_MSK_RG_START))

//   [IRT_DMA_CTRL] - 0x27004
#define IRT_DMA_FLD_RG_DRAM_CLK_EN(x)(((UINT32)(x) << IRT_DMA_SHT_RG_DRAM_CLK_EN)&(IRT_DMA_MSK_RG_DRAM_CLK_EN))
#define IRT_DMA_FLD_RG_MON_SEL(x)    (((UINT32)(x) << IRT_DMA_SHT_RG_MON_SEL)&(IRT_DMA_MSK_RG_MON_SEL))
#define IRT_DMA_FLD_RG_RESET_B(x)    (((UINT32)(x) << IRT_DMA_SHT_RG_RESET_B)&(IRT_DMA_MSK_RG_RESET_B))

//   [IRT_DMA_HVSIZE] - 0x27008
#define IRT_DMA_FLD_RG_VSIZE(x)      (((UINT32)(x) << IRT_DMA_SHT_RG_VSIZE)&(IRT_DMA_MSK_RG_VSIZE))
#define IRT_DMA_FLD_RG_HSIZE(x)      (((UINT32)(x) << IRT_DMA_SHT_RG_HSIZE)&(IRT_DMA_MSK_RG_HSIZE))

//   [IRT_DMA_DRAM_RD_STA] - 0x2700C
#define IRT_DMA_FLD_DRAM_Y_RD_STA(x)   (((UINT32)(x) << IRT_DMA_SHT_DRAM_Y_RD_STA)&(IRT_DMA_MSK_DRAM_Y_RD_STA))

//   [IRT_DMA_DRAM_WR_STA] - 0x27010
#define IRT_DMA_FLD_DRAM_Y_WR_STA(x)   (((UINT32)(x) << IRT_DMA_SHT_DRAM_Y_WR_STA)&(IRT_DMA_MSK_DRAM_Y_WR_STA))

//   [IRT_DMA_DRAM_RD_STA] - 0x27014
#define IRT_DMA_FLD_DRAM_C_RD_STA(x)   (((UINT32)(x) << IRT_DMA_SHT_DRAM_C_RD_STA)&(IRT_DMA_MSK_DRAM_C_RD_STA))

//   [IRT_DMA_DRAM_WR_STA] - 0x27018
#define IRT_DMA_FLD_DRAM_C_WR_STA(x)   (((UINT32)(x) << IRT_DMA_SHT_DRAM_C_WR_STA)&(IRT_DMA_MSK_DRAM_C_WR_STA))

//   [IRT_DMA_DRAM_WR_OFFSET] - 0x2701C
#define IRT_DMA_FLD_RG_WR_OFFSET(x)  (((UINT32)(x) << IRT_DMA_SHT_RG_WR_OFFSET)&(IRT_DMA_MSK_RG_WR_OFFSET))
#define IRT_DMA_FLD_RG_RD_OFFSET(x)  (((UINT32)(x) << IRT_DMA_SHT_RG_RD_OFFSET)&(IRT_DMA_MSK_RG_RD_OFFSET))

//   [IRT_DMA_DRAM_MON] - 0x27020
#define IRT_DMA_FLD_MON(x)           (((UINT32)(x) << IRT_DMA_SHT_MON)&(IRT_DMA_MSK_MON))

typedef enum
{
    IRT_DMA_OK = 0,
    IRT_DMA_INV_ARG,
    IRT_DMA_OUT_OF_MEM,
    IRT_DMA_UNINIT,
    IRT_DMA_UNDEF_ERR,
    IRT_DMA_WOULD_BLOCK
} IRT_DMA_HAL_ERR_CODE_T;

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------
#if ((CONFIG_DRV_VERIFY_SUPPORT == 1) || (CONFIG_DRV_FPGA_BOARD == 1))
void IRT_WriteREG(UINT32 arg, UINT32 val, const CHAR *szFunction, INT32 i4Line);
UINT32 IRT_ReadREG(UINT32 arg, const CHAR *szFunction, INT32 i4Line);
void IRT_WriteREGMsk(UINT32 arg, UINT32 val, UINT32 msk, const CHAR *szFunction, INT32 i4Line);

#define IRT_DMA_READ32(offset)            IRT_ReadREG(irt_dma_base+(offset), __FUNCTION__, __LINE__)
#define IRT_DMA_WRITE32(offset, value)    IRT_WriteREG(irt_dma_base+(offset), value, __FUNCTION__, __LINE__)
#define IRT_DMA_MSK_WRITE32(offset, value, mask)  IRT_WriteREGMsk(irt_dma_base+(offset), value, mask, __FUNCTION__, __LINE__)

extern INT32 UTIL_Printf(const CHAR *ps_format, ...);

#define IRT_Printf(level, format, ...)  \
        { \
            if (1) \
            { \
                UTIL_Printf("%s"format, "[irt] ", ##__VA_ARGS__);  \
            }  \
        }

#define IRT_PrintfEx(level, format, ...)  \
        { \
            if (1) \
            { \
                UTIL_Printf(format, ##__VA_ARGS__);  \
            }  \
        }

#define IRT_APIEntry(...)  \
        { \
            IRT_Printf(1, "-->%s, %d\n", __FUNCTION__, __LINE__); \
        }

#define IRT_APILeave()  \
        { \
            IRT_Printf(1, "<--%s, %d\n", __FUNCTION__, __LINE__); \
        }

#define IRT_APIEntryEx(format, ...)  \
        { \
            IRT_Printf(1, "-->%s("format"), %d\n", __FUNCTION__, ##__VA_ARGS__, __LINE__); \
        }

#define IRT_APILeaveEx(format, ...)  \
        { \
            IRT_Printf(1, "<--%s("format"), %d\n", __FUNCTION__, ##__VA_ARGS__, __LINE__); \
        }
#else
#define IRT_DMA_READ32(offset)          IO_READ32(irt_dma_base, (offset))
#define IRT_DMA_WRITE32(offset, value)  IO_WRITE32(irt_dma_base, (offset), (value))
#define IRT_DMA_MSK_WRITE32(offset, value, mask)  IRT_DMA_WRITE32(offset,(IRT_DMA_READ32(offset)& ~(mask))|(value))

#define IRT_WriteREG(offset, val, szFunction, i4Line)  IRT_DMA_WRITE32(offset, val)
#define IRT_ReadREG(offset, szFunction, i4Line)        IRT_DMA_READ32(offset)
#define IRT_WriteREGMsk(offset, val, msk, szFunction, i4Line) IRT_DMA_MSK_WRITE32(offset, val, msk)

#define IRT_Printf(level, format, ...)
#define IRT_APIEntry(...)
#define IRT_APILeave()
#define IRT_APIEntryEx(format, ...)
#define IRT_APILeaveEx(format, ...)
#endif


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------

extern INT32 IRT_DMA_HwInit(void);
extern INT32 IRT_DMA_HwUnInit(void);
//static void IRT_DMA_HwISR(UINT16 u2Vector);
extern INT32 IRT_DMA_HwReset(void);
extern INT32 IRT_DMA_HwSrcModeSet(UINT32 u4Rg5351ModeEn, UINT32 u4Rg5351ModeSel, UINT32 u4RgScanLine,UINT32 u4BlockBurstRead);
extern INT32 IRT_DMA_HwTrig(void);
extern INT32 IRT_DMA_HwHVSize(UINT32 u4Width, UINT32 u4Height);
extern INT32 IRT_DMA_HwYCAndRotMode(UINT32 u4Mode);
extern INT32 IRT_DMA_HwSrcSa(UINT32 *pu4SrcYAdd,UINT32 *pu4SrcCAdd);
extern INT32 IRT_DMA_HwDstSa(UINT32 *pu4DstYAdd,UINT32 *pu4DstCAdd);
extern INT32 IRT_DMA_HwWROffset(UINT32 u4Width, UINT32 u4Height, IRT_DMA_MODE_T eMode);

void IRT_FlushDCacheRange(UINT32 u4Start, UINT32 u4Len);
void IRT_CleanDCacheRange(UINT32 u4Start, UINT32 u4Len);
void IRT_InvDCacheRange(UINT32 u4Start, UINT32 u4Len);

#endif /* __HW_IRT_DMA_H__ */
