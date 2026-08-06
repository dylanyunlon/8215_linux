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
#include "types.h"
#include "base_regs.h"
#include "x_hal_ic.h"
#include "drv_common.h"
#include "imgresz_hal.h"
#include "imgresz_hal_if.h"
#include "imgresz_hal_errcode.h"
#include "x_hal_1176.h"
#include "chip_ver.h"
#include "x_rtos.h"
#include "x_printf.h"
#include "x_assert.h"
#include "imgresz_log.h"

#define IMGRESZ_HAL_SUBLINE_ADJUST 1
#define VA_TO_PA(x)  (x)


/*extern u32 u4SwapMode;*/

static u32 _au4IMGRZ_FILTERCOEF_H[FACTOR_MAX][18] = {
	/* 0~17*/
	/*FACTOR_0*/
	{
		0x40202020, 0x20202020, 0x20202020 , 0x20202020, 0x20202020, 0x20202020 ,
		0x20202020, 0x20202020, 0x20202020 , 0x20202020, 0x20202020, 0x20202020 ,
		0x20202020, 0x20202020, 0x20202020 , 0x20202020, 0x00000000, 0x00000000
	},
	/*FACTOR_0_0625*/
	{
		0x3f21201f, 0x2121201f, 0x2121201f , 0x2121201f, 0x2121201f, 0x2121201f ,
		0x2121201f, 0x2121201e, 0x2121201e , 0x2121201e, 0x2021201e, 0x2021201e ,
		0x21211f1e, 0x22201f1e, 0x22201f1e , 0x22201f1e, 0x00000000, 0x00000000
	},
	/*FACTOR_0_125*/
	{
		0x3c24211d, 0x2524211c, 0x2524211c , 0x2524201b, 0x2524201b, 0x2523201b ,
		0x25231f1a, 0x25231f1a, 0x24231f1a , 0x24231f19, 0x26221e19, 0x25221e18 ,
		0x24221e18, 0x25221d18, 0x24221d17 , 0x25211d17, 0x00000000, 0x00000000
	},
	/*FACTOR_0_25*/
	{
		0x3833220f, 0x3831210e, 0x3830200d , 0x382f1f0c, 0x382f1d0b, 0x372e1c0a ,
		0x372d1b09, 0x372c1a08, 0x362b1807 , 0x342a1706, 0x34291605, 0x34281504 ,
		0x33271403, 0x34251302, 0x34241102 , 0x34231001, 0x00000000, 0x00000000
	},
	/*FACTOR_0_5*/
	{
		0x845000ee, 0x844bfdef, 0x8345faf0 , 0x8340f7f1, 0x813af5f2, 0x7f35f3f3 ,
		0x7d2ff1f4, 0x7a29eff6, 0x7724eef7 , 0x741fedf8, 0x6e1aedfa, 0x6915edfb ,
		0x6610ecfc, 0x5f0cedfd, 0x5c07edfe , 0x5404eeff, 0x33333331, 0x33333333
	},
	/*FACTOR_1*/
	{
		0x00000000, 0xfef40300, 0xf9ea0500 , 0xf0e30700, 0xe5de0800, 0xd6db0800 ,
		0xc5da0800, 0xb1db0700, 0x9ddd0600 , 0x87e10500, 0x70e50400, 0x5aea0300 ,
		0x44ef0200, 0x31f40100, 0x20f80000 , 0x0ffc0000, 0x44444448, 0x44444444
	},
	/*FACTOR_RM*/
	{
		0x00000000, 0xf0000000, 0xe0000000 , 0xd0000000, 0xc0000000, 0xb0000000 ,
		0xa0000000, 0x90000000, 0x80000000 , 0x70000000, 0x60000000, 0x50000000 ,
		0x40000000, 0x30000000, 0x20000000 , 0x10000000, 0x00000008, 0x00000000
	}

};


static u32 _au4IMGRZ_FILTERCOEF_V[FACTOR_MAX][9] = {
	/* 0~8*/
	/*FACTOR_0*/
	{
		0x40408040, 0x40404040, 0x40404040 , 0x40404040, 0x40404040,
		0x40404040 , 0x40404040, 0x40404040, 0x00000000
	},
	/*FACTOR_0_0625*/
	{
		0x41408040, 0x41404140, 0x41404140 , 0x40404140, 0x413f4040,
		0x403f403f , 0x403f403f, 0x403f403f, 0x00000000
	},
	/*FACTOR_0_125*/
	{
		0x43417e41, 0x42404340, 0x423f4240 , 0x423e423f, 0x423e423e,
		0x423d423d , 0x423c413d, 0x403c413c, 0x00000000
	},
	/*FACTOR_0_25*/
	{
		0x4b427a43, 0x4a3f4b40, 0x493c4a3e , 0x483a493b, 0x47374838,
		0x47344636 , 0x45324533, 0x442f4431, 0x00000000
	},
	/*FACTOR_0_5*/
	{
		0x72417446, 0x6f36703b, 0x6a2c6c31 , 0x65226827, 0x6019621e,
		0x59115c15 , 0x510a560d, 0x4a034f06, 0x00000000
	},
	/*FACTOR_1*/
	{
		0x01f40000, 0xf8e2ffea, 0xdfd9eddd , 0xbad9ced8, 0x8edfa5db,
		0x5fe976e4 , 0x33f348ee, 0x0ffc1ff8, 0x00000005
	},
	/*FACTOR_RM*/
	{
		0xf0000000, 0xd000e000, 0xb000c000 , 0x9000a000, 0x70008000,
		0x50006000 , 0x30004000, 0x10002000, 0x00000001
	}
};


static u32 u4HwImgReszOffset[HW_IMGRESZ_NUM] = {0xf0004400,0xf0004600};


INLINE void vHwImgReszWrite32(u32 u4HwId, u32 addr, u32 value)
{
	*(volatile u32 *)(u4HwImgReszOffset[u4HwId] + addr) = value;
}

INLINE u32 u4HwImgReszRead32(u32 u4HwId, u32 addr)
{
	return *(volatile u32 *)(u4HwImgReszOffset[u4HwId] + addr);
}

INLINE u32 u4HwImgReszReadBim(u32 addr)
{
	return *(volatile u32 *)(0x70000000 + addr);
}

INLINE void vHwImgReszWriteBim(u32 addr, u32 value)
{
	*(volatile u32 *)(0x70000000 + addr) = value;
}

void vImgResz_HAL_Set_Interupt(BOOL Enable)
{
	u32 u4RegVal;

	if (Enable) {
		u4RegVal = u4HwImgReszReadBim(IMGRESZ_InterRupt_ENABLE_REG);
		u4RegVal |= IMGRESZ_InterRupt_ENABLE;
		vHwImgReszWriteBim(IMGRESZ_InterRupt_ENABLE_REG, u4RegVal);
	}
}

void vHwImgRezeDMAReset(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal |= IMG_RESZ_DMA_SW_RST;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
	/*u4RegVal &= ~IMG_RESZ_DMA_SW_RST;*/
	/* vHwImgReszWrite32(u4HwId,RW_IMG_RESZ_START,u4RegVal);*/
}

void vHwImgRezeSetWaitWRDone(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal |= IMG_RESZ_WAIT_WR_DONE;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}


void vHwImgRezeReset(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal |= IMG_RESZ_SW_RESET_ON;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
	u4RegVal |= IMG_RESZ_REGISTER_RESET_ON;/*add register reset by lu.sun*/
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
	u4RegVal &= ~IMG_RESZ_REGISTER_RESET_ON;/*add register reset by lu.sun*/
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
	u4RegVal &= ~IMG_RESZ_SW_RESET_ON;
	u4RegVal &= ~IMG_RESZ_DMA_SW_RST;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}

void vHwImgReszTempMMUSet(u32 u4HwId)
{
	vHwImgReszWrite32(0, RW_IMG_RESZ_MEM_IF_MODE, 0x80000000 | (u4HwImgReszRead32(0, RW_IMG_RESZ_MEM_IF_MODE)));
}

void vHwImgReszEnable(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal |= IMG_RESZ_ENABLE + IMG_RESZ_CHK_SUM_CLR;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}


void vHwImgReszDisable(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal &= ~IMG_RESZ_ENABLE;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}

void vHwImgReszEnableMMU(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal |= IMG_RESZ_MMU_ENABLE;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}
void vHwImgReszDisableMMU(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal &= ~IMG_RESZ_MMU_ENABLE;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}

void vHwImgIntResize(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal |= IMG_RESZ_INT_ON;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}
/*sun end*/

void vHwImgReszPrinterRegister(u32 u4HwId)
{
	u32 u4Base = 0x0000;
	u32 u4Temp = 0;

	for (u4Temp = 0; u4Temp < 128; u4Temp++) {
		if ((u4Temp % 4) == 0) {
			IMGR_LOG(IMGR_LOG_LVL_DBG, "[0x%8x]", 0xFD004400 + u4Base + u4Temp * 4);
		}

		IMGR_LOG(IMGR_LOG_LVL_DBG, "0x%8x   ", u4HwImgReszRead32(u4HwId, (u4Base + u4Temp * 4)));

		if ((u4Temp % 4) == 3) {
			IMGR_LOG(IMGR_LOG_LVL_DBG, "\n");
		}
	}
}
void vHwImgReszResize(u32 u4HwId)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal |= IMG_RESZ_ACTIVATE;
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}



BOOL fgHwImgReszResizeWorking(u32 u4HwId)
{
	/* No working does not mean finish.*/
	/* In picture mode, before finish, it will sometimes working and sometime rest.*/

	if ((u4HwImgReszRead32(u4HwId, RO_IMG_RESZ_INTERFACE_MONITOR_REG) & 0xFF) == 0) {
		return FALSE;
	} else {
		return TRUE;
	}
}


BOOL fgHwImgReszInterruptExist(u32 u4HwId)
{
	if (((u4HwImgReszRead32(u4HwId, RO_IMG_RESZ_INTERFACE_MONITOR_REG) >> 16) & 1) == 0) {
		return FALSE;
	} else {
		return TRUE;
	}
}


BOOL fgHwImgReszResizeFinish(u32 u4HwId)
{
	return (u4HwImgReszRead32(u4HwId, RO_IMG_RESZ_DONE) & 1);
}

BOOL fgHwImgReszDMARstFinish(u32 u4HwId)
{
	return (u4HwImgReszRead32(u4HwId, RO_IMG_RESZ_DONE) & 0x2);
}

s32 i4HwImgReszSetResizeMode(u32 u4HwId,
			       IMGRESZ_HAL_RESIZE_MODE_T eResizeMode,
			       IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TYPE);

	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		u4RegVal &= ~IMG_RESZ_JPEG_MODE;
		u4RegVal &= ~IMG_RESZ_OSD_PARTIAL_MODE;
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER:
		u4RegVal |= IMG_RESZ_JPEG_MODE;
		u4RegVal &= ~IMG_RESZ_OSD_PARTIAL_MODE;
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
		u4RegVal &= ~IMG_RESZ_JPEG_MODE;

		switch (eResizeMode) {
		case IMGRESZ_HAL_RESIZE_MODE_FRAME:
			u4RegVal &= ~IMG_RESZ_OSD_PARTIAL_MODE;
			break;

		case IMGRESZ_HAL_RESIZE_MODE_PARTIAL:
			u4RegVal |= IMG_RESZ_OSD_PARTIAL_MODE;
			break;
		}

		break;

	default:
		return E_IMGRESZ_HAL_FAIL;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TYPE, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetResampleMethod(u32 u4HwId,
				   IMGRESZ_HAL_RESAMPLE_METHOD_T eHResampleMethod,
				   IMGRESZ_HAL_RESAMPLE_METHOD_T eVResampleMethod)
{
	u32 u4RegVal;

	/* Set Horizontal resample method*/
	switch (eHResampleMethod) {
	case IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR:
		/* Both bilinear and 8-tap are always turn on.*/
		break;

	case IMGRESZ_HAL_RESAMPLE_METHOD_8_TAP:
		/* Both bilinear and 8-tap are always turn on.*/
		break;

	default:
		return E_IMGRESZ_HAL_FAIL;
	}

	/* Set Vertical resample method*/
	switch (eVResampleMethod) {
	case IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR:
		u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TYPE);
		u4RegVal &= ~IMG_RESZ_V_4_TAP_FILTER;
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TYPE, u4RegVal);
		break;

	case IMGRESZ_HAL_RESAMPLE_METHOD_4_TAP:
		u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TYPE);
		u4RegVal |= IMG_RESZ_V_4_TAP_FILTER;
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TYPE, u4RegVal);
		break;

	default:
		return E_IMGRESZ_HAL_FAIL;
	}

	return S_IMGRESZ_HAL_OK;
}


/*////////////////////////////////////////////////////////////////////////////////*/
/**/
/*     Source Buffer Setting*/
/**/
/*////////////////////////////////////////////////////////////////////////////////*/
s32 i4HwImgReszSetSrcBufFormat(u32 u4HwId, IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat)
{
	u32 u4ReszType;
	u32 u4OsdModeSetting;
	u32 u4AddrSwapSetting;
	u32 u4WTSetting;

	u4ReszType = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TYPE);
	u4OsdModeSetting = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING);
	u4AddrSwapSetting = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_MEM_IF_MODE);

	u4WTSetting = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_WT_SET);

	u4AddrSwapSetting &= ~(IMG_RESZ_IN_ADDR_SWAP_MODE_0 | IMG_RESZ_IN_ADDR_SWAP_MODE_1 |
			       IMG_RESZ_IN_ADDR_SWAP_MODE_2);

	u4AddrSwapSetting &= ~(IMG_RESZ_IN_ADDR_SWAP_MODE_0 | IMG_RESZ_IN_ADDR_SWAP_MODE_1 |
			       IMG_RESZ_IN_ADDR_SWAP_MODE_2 | IMG_RESZ_IN_ADDR_SWAP_MODE_4 |
			       IMG_RESZ_IN_ADDR_SWAP_MODE_5 | IMG_RESZ_IN_ADDR_SWAP_MODE_3 |
			       IMG_RESZ_IN_ADDR_SWAP_MODE_6 | IMG_RESZ_IN_ADDR_SWAP_MODE_7);

	u4ReszType &= ~IMG_RESZ_FIELD; /* If not clear, OSD mode will incorrect.*/

	u4WTSetting |= IMG_RESZ_WT_READ_BYPASS;

	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		/* Set source buffer main format*/
		u4ReszType &= ~IMG_RESZ_SEL_OSD_MODE;

		/* Set source buffer progressive/interlace format*/
		if (prSrcBufferFormat->fgProgressiveFrame) {
			u4ReszType &= ~IMG_RESZ_FIELD;
		} else {
			u4ReszType |= IMG_RESZ_FIELD;

			if (prSrcBufferFormat->fgTopField) {
				u4ReszType |= IMG_RESZ_INTERLACE_TOP_FIELD;
			} else {
				u4ReszType &= ~IMG_RESZ_INTERLACE_TOP_FIELD;
			}
		}

		/* Set source buffer Raster Scan/Block Mode format*/
		if (prSrcBufferFormat->fgBlockMode) {
			u4ReszType &= ~IMG_RESZ_RASTER_SCAN_IN;
		} else {
			u4ReszType |= IMG_RESZ_RASTER_SCAN_IN;
		}

#if 0

		if (prSrcBufferFormat->fgAddrSwap) {
			switch (u4SwapMode) {
			case 1:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_1;
				break;

			case 2:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_2;
				break;

			case 3:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_3;
				break;

			case 4:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_4;
				break;

			case 5:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_5;
				break;

			case 6:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_6;
				break;

			case 7:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_7;
				break;

			default:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_0;
				break;
			}
		}

#endif
#ifdef DRV_SUPPORT_ADDRESS_SWAP

		/* Set address swap*/
		if (prSrcBufferFormat->fgAddrSwap) {
			switch (DRV_ADDRESS_SWAP_MODE) {
			case ASM_5:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_6;
				break;

			default:
				u4AddrSwapSetting |= IMG_RESZ_IN_ADDR_SWAP_MODE_1;
				break;
			}
		}

#endif
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER:
		/* Set source buffer main format*/
		u4ReszType &= ~IMG_RESZ_SEL_OSD_MODE;

		/* Set Sample Factor*/
		{
			u32 u4Value;

			u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_JPG_MODE);
			u4Value &= 0xFFFFF000;
			u4Value |= (((prSrcBufferFormat->u4HSampleFactor[0] - 1) & 3) << 10);
			u4Value |= (((prSrcBufferFormat->u4VSampleFactor[0] - 1) & 3) << 8);
			u4Value |= (((prSrcBufferFormat->u4HSampleFactor[1] - 1) & 3) << 6);
			u4Value |= (((prSrcBufferFormat->u4VSampleFactor[1] - 1) & 3) << 4);
			u4Value |= (((prSrcBufferFormat->u4HSampleFactor[2] - 1) & 3) << 2);
			u4Value |= (((prSrcBufferFormat->u4VSampleFactor[2] - 1) & 3) << 0);
			vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_JPG_MODE, u4Value);
		}
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		/* Set source buffer main format*/
		u4ReszType |= IMG_RESZ_SEL_OSD_MODE;
		u4OsdModeSetting &= ~IMG_RESZ_OSD_DIRECT_MODE;

		/* Set index buffer format*/
		u4OsdModeSetting &= ~(IMG_RESZ_OSD_INDEX_2BPP | IMG_RESZ_OSD_INDEX_4BPP |
				      IMG_RESZ_OSD_INDEX_8BPP);

		switch (prSrcBufferFormat->eIndexBufferFormat) {
		case IMGRESZ_HAL_INDEX_BUFFER_FORMAT_2BPP:
			u4OsdModeSetting |= IMG_RESZ_OSD_INDEX_2BPP;
			break;

		case IMGRESZ_HAL_INDEX_BUFFER_FORMAT_4BPP:
			u4OsdModeSetting |= IMG_RESZ_OSD_INDEX_4BPP;
			break;

		case IMGRESZ_HAL_INDEX_BUFFER_FORMAT_8BPP:
			u4OsdModeSetting |= IMG_RESZ_OSD_INDEX_8BPP;
			break;

		default:
			return E_IMGRESZ_HAL_FAIL;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
		/* Set source buffer main format*/
		u4ReszType |= IMG_RESZ_SEL_OSD_MODE;
		u4OsdModeSetting |= IMG_RESZ_OSD_DIRECT_MODE;

		/* Set ARGB buffer format*/
		u4OsdModeSetting &= ~(IMG_RESZ_OSD_DIRECT_RGB565 | IMG_RESZ_OSD_DIRECT_ARGB1555 |
				      IMG_RESZ_OSD_DIRECT_ARGB4444 | IMG_RESZ_OSD_DIRECT_ARGB8888);

		switch (prSrcBufferFormat->eARGBBufferFormat) {
		case IMGRESZ_HAL_ARGB_BUFFER_FORMAT_0565:
			u4OsdModeSetting |= IMG_RESZ_OSD_DIRECT_RGB565;
			break;

		case IMGRESZ_HAL_ARGB_BUFFER_FORMAT_1555:
			u4OsdModeSetting |= IMG_RESZ_OSD_DIRECT_ARGB1555;
			break;

		case IMGRESZ_HAL_ARGB_BUFFER_FORMAT_4444:
			u4OsdModeSetting |= IMG_RESZ_OSD_DIRECT_ARGB4444;
			break;

		case IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888:
			u4OsdModeSetting |= IMG_RESZ_OSD_DIRECT_ARGB8888;
			break;

		default:
			return E_IMGRESZ_HAL_FAIL;
		}

		/* Set WT compression*/
		if (prSrcBufferFormat->fgWT) {
			u4WTSetting &= ~IMG_RESZ_WT_READ_BYPASS;

			switch (prSrcBufferFormat->eBufferMainFormat) {
			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
				u4WTSetting &= ~IMG_RESZ_WT_READ_FORMAT_AYUV;
				break;

			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
				u4WTSetting |= IMG_RESZ_WT_READ_FORMAT_AYUV;
				break;

			default:
				break;
			}
		} else {
			u4WTSetting |= IMG_RESZ_WT_READ_BYPASS;

			switch (prSrcBufferFormat->eBufferMainFormat) {
			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
				u4WTSetting &= ~IMG_RESZ_WT_READ_FORMAT_AYUV;
				break;

			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
				u4WTSetting |= IMG_RESZ_WT_READ_FORMAT_AYUV;
				break;

			default:
				break;
			}
		}

		break;

	default:
		return E_IMGRESZ_HAL_FAIL;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TYPE, u4ReszType);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING, u4OsdModeSetting);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_MEM_IF_MODE, u4AddrSwapSetting);

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_WT_SET, u4WTSetting);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetSrcBufAddr(u32 u4HwId, u32 u4BufSA1, u32 u4BufSA2, u32 u4BufSA3)
{
#if 0/*!IMGRESZ_IO_MMU_TEST*/
	u4BufSA1 = u4BufSA1 ? VA_TO_PA(u4BufSA1) : 0;
	u4BufSA2 = u4BufSA2 ? VA_TO_PA(u4BufSA2) : 0;

	if (u4BufSA3 != 0) {
		u4BufSA3 = u4BufSA3 ? VA_TO_PA(u4BufSA3) : 0;
	}

#endif
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_Y_ADDR_BASE1, (u4BufSA1 >> 4));
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_CB_ADDR_BASE1, (u4BufSA2 >> 4));
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_CR_ADDR_BASE1, (u4BufSA3 >> 4));

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetSrcPrevRowBufAddr(u32 u4HwId, u32 u4BufSA1, u32 u4BufSA2, u32 u4BufSA3)
{
#if 0/*!IMGRESZ_IO_MMU_TEST*/
	u4BufSA1 = u4BufSA1 ? VA_TO_PA(u4BufSA1) : 0;
	u4BufSA2 = u4BufSA2 ? VA_TO_PA(u4BufSA2) : 0;
	u4BufSA3 = u4BufSA3 ? VA_TO_PA(u4BufSA3) : 0;
#endif
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_Y_ADDR_BASE2, (u4BufSA1 >> 4));
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_CB_ADDR_BASE2, (u4BufSA2 >> 4));
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_CR_ADDR_BASE2, (u4BufSA3 >> 4));

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetSrcBufWidth(u32 u4HwId, u32 u4BufWidth, IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat)
{
	u32 u4YBufWidth;
	u32 u4CBufWidth;

	if (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER) {
		u4YBufWidth = u4BufWidth;

		if ((prSrcBufferFormat->u4HSampleFactor[1] < prSrcBufferFormat->u4HSampleFactor[0])
		    && (prSrcBufferFormat->u4HSampleFactor[2] < prSrcBufferFormat->u4HSampleFactor[0])) {
			if (prSrcBufferFormat->fgJpg) {
				u4CBufWidth = ((u4BufWidth / 1 + 15) / 16) * 16;
			} else {
				u4CBufWidth = ((u4BufWidth / 2 + 15) / 16) * 16;
			}
		} else {
			u4CBufWidth = u4BufWidth;
		}

		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_BUF_LEN,
				  (((u4CBufWidth >> 4) & 0x3FFF) << 12) | (u4YBufWidth >> 4));
	} else {
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_BUF_LEN,
				  (((u4BufWidth >> 4) & 0x3FFF) << 12) | (u4BufWidth >> 4));
	}

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetSrcRowBufHeight(u32 u4HwId, u32 u4RowBufHeight,
				    IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat, BOOL fgRPRRacingModeEnable)
{
	u32 u4Value;
	u32 u4RowBufHeightY = 0, u4RowBufHeightCb = 0, u4RowBufHeightCr = 0;

	if (u4RowBufHeight == 0) {
		u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_JPG_MODE);
		u4Value &= ~IMG_RESZ_LINES_ASSIGNED_DIRECTLY;
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_JPG_MODE, u4Value);
	} else {
		u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_JPG_MODE);
		u4Value |= IMG_RESZ_LINES_ASSIGNED_DIRECTLY;
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_JPG_MODE, u4Value);
		u4RowBufHeightY = u4RowBufHeight;


		if (fgRPRRacingModeEnable) {                /*sun new*/
			u4RowBufHeightCb = u4RowBufHeight / 2;
		}

		if (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER) {
			u4RowBufHeightCb = u4RowBufHeight * prSrcBufferFormat->u4VSampleFactor[1] /
					   prSrcBufferFormat->u4VSampleFactor[0];
			u4RowBufHeightCr = u4RowBufHeight * prSrcBufferFormat->u4VSampleFactor[2] /
					   prSrcBufferFormat->u4VSampleFactor[0];
		}

		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_LINE_NUM_IN_Y_COLOR_BUF, u4RowBufHeightY);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_LINE_NUM_IN_CB_COLOR_BUF, u4RowBufHeightCb);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_LINE_NUM_IN_CR_COLOR_BUF, u4RowBufHeightCr);
	}

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetSrcImageWidthHeight(u32 u4HwId, u32 u4Width, u32 u4Height,
					IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat)
{
	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_Y, ((u4Width & 0xFFFF) << 16) | (u4Height & 0xFFFF));

		switch (prSrcBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
			vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_CB,
					  (((u4Width / 2) & 0xFFFF) << 16) | ((u4Height / 2) & 0xFFFF));
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
			vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_CB,
					  (((u4Width / 2) & 0xFFFF) << 16) | (u4Height & 0xFFFF));
			break;

		default:
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER: {
		u32 u4SrcWidth, u4SrcHeight;

		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_Y, ((u4Width & 0xFFFF) << 16) | (u4Height & 0xFFFF));

		if ((prSrcBufferFormat->u4HSampleFactor[1] != 0) && (prSrcBufferFormat->u4VSampleFactor[1] != 0)) {
			u4SrcWidth = u4Width * prSrcBufferFormat->u4HSampleFactor[1] /
				     prSrcBufferFormat->u4HSampleFactor[0];
			u4SrcHeight = u4Height * prSrcBufferFormat->u4VSampleFactor[1] /
				      prSrcBufferFormat->u4VSampleFactor[0];

			/* For jpeg picture mode, prevent source height 401 come two
				interrupt (Y interrupt and C interrupt)*/
			if ((u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[0]) !=
			    (u4Height * prSrcBufferFormat->u4VSampleFactor[1])) {
				u4SrcHeight++;
			}

			vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_CB,
					  ((u4SrcWidth & 0xFFFF) << 16) | (u4SrcHeight & 0xFFFF));
		}

		if ((prSrcBufferFormat->u4HSampleFactor[2] != 0) && (prSrcBufferFormat->u4VSampleFactor[2] != 0)) {
			u4SrcWidth = u4Width * prSrcBufferFormat->u4HSampleFactor[2] /
				     prSrcBufferFormat->u4HSampleFactor[0];
			u4SrcHeight = u4Height * prSrcBufferFormat->u4VSampleFactor[2] /
				      prSrcBufferFormat->u4VSampleFactor[0];

			/* For jpeg picture mode, prevent source height 401 come two
				interrupt (Y interrupt and C interrupt)*/
			if ((u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[0]) !=
			    (u4Height * prSrcBufferFormat->u4VSampleFactor[2])) {
				u4SrcHeight++;
			}

			vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_CR,
					  ((u4SrcWidth & 0xFFFF) << 16) | (u4SrcHeight & 0xFFFF));
		}
	}
	break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_Y, ((u4Width & 0xFFFF) << 16) | (u4Height & 0xFFFF));
		break;
	}

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetSrcImageClipWidthHeight(u32 u4HwId, u32 u4YWidth, u32 u4CWidth,
					    u32 u4YHeight, u32 u4CHeight,
					    IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat)
{
	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_Y, ((u4YWidth & 0xFFFF) << 16) | (u4YHeight & 0xFFFF));
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_CB, ((u4CWidth & 0xFFFF) << 16) | (u4CHeight & 0xFFFF));
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_Y, ((u4YWidth & 0xFFFF) << 16) | (u4YHeight & 0xFFFF));
		break;

	default:
		break;
	}

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetSrcImageOffset(u32 u4HwId, u32 u4XOff, u32 u4YOff,
				   IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat,
				   u32 u4SrcHeight, u32 u4TgtHeight)
{
	u32 u4XOff1 = 0, u4YOff1 = 0;
	u32 u4XOff2 = 0, u4YOff2 = 0;
	u32 u4XOff3 = 0, u4YOff3 = 0;

	u4XOff1 = u4XOff;
	u4YOff1 = u4YOff;

	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prSrcBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
			u4XOff2 = u4XOff >> 1;
			u4YOff2 = u4YOff >> 1;
#if IMGRESZ_HAL_SUBLINE_ADJUST

			/* Adjust subline*/
			if ((!prSrcBufferFormat->fgProgressiveFrame) &&
			    (!prSrcBufferFormat->fgTopField) &&
			    (u4SrcHeight > u4TgtHeight)) {
				u32 u4Q;

				//ASSERT(u4TgtHeight);
				u4Q = (u4SrcHeight - u4TgtHeight) / (u4TgtHeight * 2);
				/*u32 u4R_norm =  ( ((u4SrcHeight - u4TgtHeight) - (u4Q * u4TgtHeight *2)) * 2048 ) /
						(2 * u4SrcHeight );*/

				u4YOff1 += u4Q * 2;
				u4YOff2 += u4Q * 2;
			}

#endif
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
			u4XOff2 = u4XOff >> 1;
			u4YOff2 = u4YOff;
			break;

		default:
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER: {
		u32 u4HSample, u4VSample;

		if ((prSrcBufferFormat->u4HSampleFactor[1] != 0) && (prSrcBufferFormat->u4VSampleFactor[1] != 0)) {
			u4HSample = prSrcBufferFormat->u4HSampleFactor[0] / prSrcBufferFormat->u4HSampleFactor[1];
			u4VSample = prSrcBufferFormat->u4VSampleFactor[0] / prSrcBufferFormat->u4VSampleFactor[1];
			u4XOff2 = u4XOff / u4HSample;
			u4YOff2 = u4YOff / u4VSample;
		} else {
			u4XOff2 = 0;
			u4YOff2 = 0;
		}

		if ((prSrcBufferFormat->u4HSampleFactor[2] != 0) && (prSrcBufferFormat->u4VSampleFactor[2] != 0)) {
			u4HSample = prSrcBufferFormat->u4HSampleFactor[0] / prSrcBufferFormat->u4HSampleFactor[2];
			u4VSample = prSrcBufferFormat->u4VSampleFactor[0] / prSrcBufferFormat->u4VSampleFactor[2];
			u4XOff3 = u4XOff / u4HSample;
			u4YOff3 = u4YOff / u4VSample;
		} else {
			u4XOff3 = 0;
			u4YOff3 = 0;
		}
	}
	break;

	default:
		break;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_Y, ((u4XOff1 & 0xFFF) << 12) | (u4YOff1 & 0xFFF));
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_CB, ((u4XOff2 & 0xFFF) << 12) | (u4YOff2 & 0xFFF));
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_CR, ((u4XOff3 & 0xFFF) << 12) | (u4YOff3 & 0xFFF));

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetRPR(u32 u4HwId, BOOL fgRPRMode, BOOL fgRPRRacingModeEnable)
{
	u32 u4Value;

	u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_RPR);

	if (fgRPRMode) {
		u4Value |= IMG_RESZ_RPR_FLAG_ON;
	} else {
		u4Value &= ~IMG_RESZ_RPR_FLAG_ON;
	}

	if (fgRPRRacingModeEnable) {
		u4Value &= ~(0x1 << 10);
		u4Value |= IMG_RESZ_TRC_VDEC_EN;
		u4Value |= IMG_RESZ_TRC_VDEC_INT;
	} else {
		u4Value &= ~IMG_RESZ_TRC_VDEC_EN;
		u4Value &= ~IMG_RESZ_TRC_VDEC_INT;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_RPR, u4Value);
	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetFLIP(u32 u4HwId, BOOL fgFlipModeEnable)
{
	u32 u4Value;

	u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_FLIP);

	if (fgFlipModeEnable) {
		u4Value |= IMG_RESZ_FLIP_EN;
	} else {
		u4Value &= ~IMG_RESZ_FLIP_EN;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_FLIP, u4Value);

	return S_IMGRESZ_HAL_OK;
}
/*IMG_RESZ_CBCR_SWAP*/
s32 i4HwImgReszSetSwap(u32 u4HwId, BOOL fgSwapModeEnable)
{
	u32 u4Value;

	u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TYPE);

	if (fgSwapModeEnable) {
		u4Value |= IMG_RESZ_CBCR_SWAP;
	} else {
		u4Value &= ~IMG_RESZ_CBCR_SWAP;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TYPE, u4Value);

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetSrcFirstRow(u32 u4HwId, BOOL fgFirstRow)
{
	u32 u4Value;

	u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_JPG_MODE);

	if (fgFirstRow) {
		u4Value |= IMG_RESZ_FIRST_BLOCK_LINE;
	} else {
		u4Value &= ~IMG_RESZ_FIRST_BLOCK_LINE;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_JPG_MODE, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetSrcLastRow(u32 u4HwId, BOOL fgLastRow)
{
	u32 u4Value;

	u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_JPG_MODE);

	if (fgLastRow) {
		u4Value |= IMG_RESZ_LAST_BLOCK_LINE;
	} else {
		u4Value &= ~IMG_RESZ_LAST_BLOCK_LINE;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_JPG_MODE, u4Value);

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszResetDestPartialBuf(u32 u4HwId, BOOL fgReset)
{
	u32 u4Value;

	u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_MEM_IF_MODE);

	if (fgReset) {
		u4Value = u4Value | IMG_RESZ_REST_ADDRESS;
	} else {
		u4Value = u4Value & ~IMG_RESZ_REST_ADDRESS;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_MEM_IF_MODE, u4Value);

	return S_IMGRESZ_HAL_OK;
}

/*////////////////////////////////////////////////////////////////////////////////*/
/**/
/*     Destination Buffer Setting*/
/**/
/*////////////////////////////////////////////////////////////////////////////////*/
s32 i4HwImgReszSetDestBufFormat(u32 u4HwId, IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat,
				  IMGRESZ_HAL_IMG_BUF_FORMAT_T *prDestBufferFormat)
{
	u32 u4RegVal;
	u32 u4OsdModeSetting;
	u32 u4AddrSwapSetting;
	u32 u4CscSetting;
	u32 u4WTSetting;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TYPE);
	u4OsdModeSetting = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING);
	u4AddrSwapSetting = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_MEM_IF_MODE);
	u4CscSetting = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_OSD_CSC_SETTING);
	u4WTSetting = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_WT_SET);
	u4AddrSwapSetting &= ~(IMG_RESZ_OUT_ADDR_SWAP_MODE_0 | IMG_RESZ_OUT_ADDR_SWAP_MODE_1 |
			       IMG_RESZ_OUT_ADDR_SWAP_MODE_2);

	u4AddrSwapSetting &= ~(IMG_RESZ_OUT_ADDR_SWAP_MODE_0 | IMG_RESZ_OUT_ADDR_SWAP_MODE_1 |
			       IMG_RESZ_OUT_ADDR_SWAP_MODE_2 | IMG_RESZ_OUT_ADDR_SWAP_MODE_3 |
			       IMG_RESZ_OUT_ADDR_SWAP_MODE_4 | IMG_RESZ_OUT_ADDR_SWAP_MODE_5 |
			       IMG_RESZ_OUT_ADDR_SWAP_MODE_6 | IMG_RESZ_OUT_ADDR_SWAP_MODE_7);

	u4RegVal &= ~IMG_RESZ_V2OSD; /* If not clear, OSD mode will incorrect.*/
	u4CscSetting &= ~IMG_RESZ_OSD_CSC_ENABLE;
	u4WTSetting |= IMG_RESZ_WT_WRITE_BYPASS;

	switch (prDestBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		/* Set destination buffer YUV format*/
		u4RegVal &= ~(IMG_RESZ_420_OUT | IMG_RESZ_422_OUT | IMG_RESZ_444_OUT);

		switch (prDestBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
			u4RegVal |= IMG_RESZ_420_OUT;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
			u4RegVal |= IMG_RESZ_422_OUT;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4RegVal |= IMG_RESZ_444_OUT;
			break;

		default:
			u4RegVal |= IMG_RESZ_420_OUT;
			break;
		}

		/* Set destination buffer Raster Scan/Block Mode format*/
		if (prDestBufferFormat->fgBlockMode) {
			u4RegVal &= ~IMG_RESZ_RASTER_SCAN_OUT;
		} else {
			u4RegVal |= IMG_RESZ_RASTER_SCAN_OUT;
		}

#if 0

		if (prDestBufferFormat->fgAddrSwap) {
			switch (u4SwapMode) {
			case 1:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_1;
				break;

			case 2:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_2;
				break;

			case 3:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_3;
				break;

			case 4:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_4;
				break;

			case 5:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_5;
				break;

			case 6:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_6;
				break;

			case 7:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_7;
				break;

			default:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_0;
				break;
			}
		}

#endif
#ifdef DRV_SUPPORT_ADDRESS_SWAP

		/* Set address swap*/
		if (prDestBufferFormat->fgAddrSwap) {
			switch (DRV_ADDRESS_SWAP_MODE) {
			case ASM_5:
			default:
				u4AddrSwapSetting |= IMG_RESZ_OUT_ADDR_SWAP_MODE_1;
				break;
			}
		}

#endif
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
		u4OsdModeSetting &= ~(IMG_RESZ_OSD_OUTPUT_RGB565 | IMG_RESZ_OSD_OUTPUT_ARGB1555 |
				      IMG_RESZ_OSD_OUTPUT_ARGB4444 | IMG_RESZ_OSD_OUTPUT_ARGB8888);

		switch (prDestBufferFormat->eARGBBufferFormat) {
		case IMGRESZ_HAL_ARGB_BUFFER_FORMAT_0565:
			u4OsdModeSetting |= IMG_RESZ_OSD_OUTPUT_RGB565;
			break;

		case IMGRESZ_HAL_ARGB_BUFFER_FORMAT_1555:
			u4OsdModeSetting |= IMG_RESZ_OSD_OUTPUT_ARGB1555;
			break;

		case IMGRESZ_HAL_ARGB_BUFFER_FORMAT_4444:
			u4OsdModeSetting |= IMG_RESZ_OSD_OUTPUT_ARGB4444;
			break;

		case IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888:
			u4OsdModeSetting |= IMG_RESZ_OSD_OUTPUT_ARGB8888;
			break;
		}

		if (prDestBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER) {
			switch (prSrcBufferFormat->eBufferMainFormat) {
			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER:
				u4RegVal |= IMG_RESZ_V2OSD;
				u4RegVal &= ~(IMG_RESZ_420_OUT | IMG_RESZ_422_OUT | IMG_RESZ_444_OUT);
				u4RegVal |= IMG_RESZ_444_OUT;
				break;

			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
				u4CscSetting |= IMG_RESZ_OSD_CSC_ENABLE;
#if 1
				/* Do not down-scale Y*/
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_YIN_D16;
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_CIN_D128;
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_YOUT_A16;
				u4CscSetting |= IMG_RESZ_OSD_CSC_COUT_A128;
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF11, 0x132);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF12, 0x259);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF13, 0x75);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF21, 0x1F50);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF22, 0x1EA5);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF23, 0x20B);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF31, 0x20B);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF32, 0x1E4A);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF33, 0x1FAB);
#else
				/* Down-scale Y*/
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_YIN_D16;
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_CIN_D128;
				u4CscSetting |= IMG_RESZ_OSD_CSC_YOUT_A16;
				u4CscSetting |= IMG_RESZ_OSD_CSC_COUT_A128;
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF11, 0x107);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF12, 0x204);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF13, 0x64);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF21, 0x1F68);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF22, 0x1ED6);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF23, 0x1C2);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF31, 0x1C2);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF32, 0x1E87);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF33, 0x1FB7);
#endif
				break;

			default:
				break;
			}
		} else if (prDestBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER) {
			switch (prSrcBufferFormat->eBufferMainFormat) {
			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
				u4CscSetting |= IMG_RESZ_OSD_CSC_ENABLE;
#if 1
				/* Do not up-scale Y*/
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_YIN_D16;
				u4CscSetting |= IMG_RESZ_OSD_CSC_CIN_D128;
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_YOUT_A16;
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_COUT_A128;
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF11, 0x400);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF12, 0x0);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF13, 0x57C);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF21, 0x400);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF22, 0x1EA8);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF23, 0x1D35);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF31, 0x400);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF32, 0x6EE);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF33, 0x0);
#else
				/* Up-scale Y*/
				u4CscSetting |= IMG_RESZ_OSD_CSC_YIN_D16;
				u4CscSetting |= IMG_RESZ_OSD_CSC_CIN_D128;
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_YOUT_A16;
				u4CscSetting &= ~IMG_RESZ_OSD_CSC_COUT_A128;
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF11, 0x4A8);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF12, 0x0);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF13, 0x662);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF21, 0x4A8);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF22, 0x1E70);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF23, 0x1CBF);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF31, 0x4A8);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF32, 0x812);
				vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_COEF33, 0x0);
#endif
				break;

			default:
				break;
			}
		}

		/* Set WT compression*/
		if (prDestBufferFormat->fgWT) {
			u4WTSetting &= ~IMG_RESZ_WT_WRITE_BYPASS;

			switch (prDestBufferFormat->eBufferMainFormat) {
			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
				u4WTSetting &= ~IMG_RESZ_WT_WRITE_FORMAT_AYUV;
				break;

			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
				u4WTSetting |= IMG_RESZ_WT_WRITE_FORMAT_AYUV;
				break;

			default:
				break;
			}
		} else {
			u4WTSetting |= IMG_RESZ_WT_WRITE_BYPASS;

			switch (prDestBufferFormat->eBufferMainFormat) {
			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
				u4WTSetting &= ~IMG_RESZ_WT_WRITE_FORMAT_AYUV;
				break;

			case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
				u4WTSetting |= IMG_RESZ_WT_WRITE_FORMAT_AYUV;
				break;

			default:
				break;
			}
		}

		break;

	default:
		return E_IMGRESZ_HAL_FAIL;

	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TYPE, u4RegVal);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING, u4OsdModeSetting);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_MEM_IF_MODE, u4AddrSwapSetting);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CSC_SETTING, u4CscSetting);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_WT_SET, u4WTSetting);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetDestBufAddr(u32 u4HwId, u32 u4BufSA1, u32 u4BufSA2)
{
#if 0/*!IMGRESZ_IO_MMU_TEST*/
	u4BufSA1 = u4BufSA1 ? VA_TO_PA(u4BufSA1) : 0;
	u4BufSA2 = u4BufSA2 ? VA_TO_PA(u4BufSA2) : 0;
#endif
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TGT_Y_ADDR_BASE, (u4BufSA1 >> 4));
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TGT_C_ADDR_BASE, (u4BufSA2 >> 4));

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetDestBufWidth(u32 u4HwId, u32 u4BufWidth)
{
	u32 u4Value;

	if (u4BufWidth == 0x4000) {
		u4BufWidth = 0x4000 - 16;
	}

	u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TGT_BUF_LEN);
	u4Value = (u4Value & 0xFF000000) | (((u4BufWidth >> 4) & 0x3FFF) << 12) | (u4BufWidth >> 4);

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TGT_BUF_LEN, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetDestImageWidthHeight(u32 u4HwId, u32 u4Width, u32 u4Height)
{
	if (u4Width == 0x1000) {
		u4Width = 0xFFF;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TGT_SIZE, ((u4Width & 0xFFFF) << 16) | (u4Height & 0xFFFF));

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetDestImageOffset(u32 u4HwId, u32 u4XOff, u32 u4YOff)
{
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TGT_OFFSET, ((u4XOff & 0xFFF) << 12) | (u4YOff & 0xFFF));

	return S_IMGRESZ_HAL_OK;
}

void vHwImgReszReadBurstLength(u32 u4HwId, BOOL fgEnable)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_START);
	u4RegVal &= ~(IMG_RESZ_RD_BURST_ON);

	if (fgEnable) {
		u4RegVal |= IMG_RESZ_RD_BURST_ON;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_START, u4RegVal);
}
/*////////////////////////////////////////////////////////////////////////////////*/
/**/
/*     Alpha Blending*/
/**/
/*////////////////////////////////////////////////////////////////////////////////*/
s32 i4HwImgReszSetAlphaBlendingLevel(u32 u4HwId, u32 u4Alpha)
{
	u32 u4RegVal;

	if (u4Alpha > 128) {
		u4Alpha = 128;
	}

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING);
	u4RegVal &= ~(((u32)0xFF) << IMG_RESZ_A_BLEND_SHIFT);
	u4RegVal |= u4Alpha << IMG_RESZ_A_BLEND_SHIFT;

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetPreloadBufAddr(u32 u4HwId, u32 u4BufSA1, u32 u4BufSA2)
{
	u4BufSA1 = u4BufSA1 ? VA_TO_PA(u4BufSA1) : 0;
	u4BufSA2 = u4BufSA2 ? VA_TO_PA(u4BufSA2) : 0;

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_Y_PRELOAD_OW_ADDR_BASE, u4BufSA1 >> 4);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_C_PRELOAD_OW_ADDR_BASE, u4BufSA2 >> 4);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetAlphaChangeScalingType(u32 u4HwId, u32 u4ScalingType)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING);
	u4RegVal &= ~(IMG_RESZ_OSD_ALPHA_SCALE_NORMAL | IMG_RESZ_OSD_ALPHA_SCALE_REF_LEFT |
		      IMG_RESZ_OSD_ALPHA_SCALE_REF_NEAREST);

	switch (u4ScalingType) {
	case 0:
		u4RegVal |= IMG_RESZ_OSD_ALPHA_SCALE_NORMAL;
		break;

	case 1:
		u4RegVal |= IMG_RESZ_OSD_ALPHA_SCALE_REF_LEFT;
		break;

	case 2:
		u4RegVal |= IMG_RESZ_OSD_ALPHA_SCALE_REF_NEAREST;
		break;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetAlphaChangeBilinearBoundary(u32 u4HwId, BOOL fgBilinearBoundary)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING);

	if (fgBilinearBoundary) {
		u4RegVal |= IMG_RESZ_OSD_ALPHA_BILINEAR_BOUNDARY;
	} else {
		u4RegVal &= ~IMG_RESZ_OSD_ALPHA_BILINEAR_BOUNDARY;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetOnlyDistinguishAlpha(u32 u4HwId, BOOL fgOnlyDistinguishAlpha)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING);

	if (fgOnlyDistinguishAlpha) {
		u4RegVal |= IMG_RESZ_OSD_ONLY_DISTINGUISH_ALPHA;
	} else {
		u4RegVal &= ~IMG_RESZ_OSD_ONLY_DISTINGUISH_ALPHA;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MODE_SETTING, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetLumaKeyEnable(u32 u4HwId, BOOL fgEnable)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_LUMA_KEY);

	if (fgEnable) {
		u4RegVal |= IMG_RESZ_LUMA_KEY_ENABLE;
	} else {
		u4RegVal &= ~IMG_RESZ_LUMA_KEY_ENABLE;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_LUMA_KEY, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetLumaKeyScalingType(u32 u4HwId, u32 u4ScalingType)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_LUMA_KEY);
	u4RegVal &= ~(IMG_RESZ_LUMA_KEY_SCALE_NORMAL | IMG_RESZ_LUMA_KEY_SCALE_REF_LEFT |
		      IMG_RESZ_LUMA_KEY_SCALE_REF_NEAREST);

	switch (u4ScalingType) {
	case 0:
		u4RegVal |= IMG_RESZ_LUMA_KEY_SCALE_NORMAL;
		break;

	case 1:
		u4RegVal |= IMG_RESZ_LUMA_KEY_SCALE_REF_LEFT;
		break;

	case 2:
		u4RegVal |= IMG_RESZ_LUMA_KEY_SCALE_REF_NEAREST;
		break;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_LUMA_KEY, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetLumaKey(u32 u4HwId, u8 u1LumaKey)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_LUMA_KEY);
	u4RegVal &= ~(0xFF << IMG_RESZ_LUMA_KEY_SHIFT);
	u4RegVal |= (u1LumaKey << IMG_RESZ_LUMA_KEY_SHIFT);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_LUMA_KEY, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetLumaKeyBilinearBoundary(u32 u4HwId, BOOL fgBilinearBoundary)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_LUMA_KEY);

	if (fgBilinearBoundary) {
		u4RegVal |= IMG_RESZ_LUMA_KEY_BILINEAR_BOUNDARY;
	} else {
		u4RegVal &= ~IMG_RESZ_LUMA_KEY_BILINEAR_BOUNDARY;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_LUMA_KEY, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetLumaKeyYUVMode(u32 u4HwId, BOOL fgYUVMode)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_LUMA_KEY);

	if (fgYUVMode) {
		u4RegVal |= IMG_RESZ_LUMA_KEY_YUV_MODE;
	} else {
		u4RegVal &= ~IMG_RESZ_LUMA_KEY_YUV_MODE;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_LUMA_KEY, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}

/*////////////////////////////////////////////////////////////////////////////////*/
/**/
/*     DRAM Related*/
/**/
/*////////////////////////////////////////////////////////////////////////////////*/
s32 i4HwImgReszSetDramReqBurstLimit(u32 u4HwId, u32 u4Limit)
{
	u32 u4RegVal;

	u4RegVal = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_MEM_IF_MODE);
	u4RegVal &= ~(IMG_RESZ_DRAM_BURST_LIMIT_1 | IMG_RESZ_DRAM_BURST_LIMIT_2 |
		      IMG_RESZ_DRAM_BURST_LIMIT_4 | IMG_RESZ_DRAM_BURST_LIMIT_8 |
		      IMG_RESZ_DRAM_BURST_LIMIT_16);

	/*u4RegVal |= ((u4Limit & 0x1F) << 8);*/
	switch (u4Limit) {
	case 1:
		u4RegVal |= IMG_RESZ_DRAM_BURST_LIMIT_1;
		break;

	case 2:
		u4RegVal |= IMG_RESZ_DRAM_BURST_LIMIT_2;
		break;

	case 4:
		u4RegVal |= IMG_RESZ_DRAM_BURST_LIMIT_4;
		break;

	case 8:
		u4RegVal |= IMG_RESZ_DRAM_BURST_LIMIT_8;
		break;

	case 16:
		u4RegVal |= IMG_RESZ_DRAM_BURST_LIMIT_16;
		break;

	default:
		u4RegVal |= IMG_RESZ_DRAM_BURST_LIMIT_16;
		break;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_MEM_IF_MODE, u4RegVal);

	return S_IMGRESZ_HAL_OK;
}


/*////////////////////////////////////////////////////////////////////////////////*/
/**/
/*     Temporary Buffer (For partial mode) Related*/
/**/
/*////////////////////////////////////////////////////////////////////////////////*/
s32 i4HwImgReszSetExtend16Store(u32 u4HwId, BOOL fgEnable)
{
	u32 u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TGT_BUF_LEN);

	if (fgEnable) {
		u4Value |= IMG_RESZ_BOUND_EXTEND_16_ON;
	} else {
		u4Value &= ~IMG_RESZ_BOUND_EXTEND_16_ON;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TGT_BUF_LEN, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetSRAMLineBufSize(u32 u4HwId, u32 u4BufLen)
{
	u32 u4Len = u4BufLen;
	u32 u4Mask = ~(0x1F << IMG_RESZ_LINE_BUFFER_LEN_SHIFT);
	u32 u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_TGT_BUF_LEN);

	/* parameter checking*/
	if (u4BufLen > 46) { /*CK ???*/
		return E_IMGRESZ_HAL_FAIL;
	}

	u4Value &= u4Mask;
	u4Value += (u4Len << IMG_RESZ_LINE_BUFFER_LEN_SHIFT);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TGT_BUF_LEN, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetTmpLineBufLen(u32 u4HwId, IMGRESZ_HAL_RESAMPLE_METHOD_T eVResampleMethod,
				  u32 u4SrcHeight, u32 u4DestHeight, u32 u4DestWidth,
				  BOOL fgRPRMode, BOOL fgLumaKeyEnable)
{
	BOOL fgUseExtend16 = 0;
	u32 u4LineBufWidth = 11;
	s32 i4Ret;

	switch (eVResampleMethod) {
	case IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR:
		if (u4SrcHeight < u4DestHeight) {
			if (fgLumaKeyEnable) {
				u4LineBufWidth = 0x1E;
			} else {
				u4LineBufWidth = 0x1F;
			}

			fgUseExtend16 = 0;
		} else {
			fgUseExtend16 = 1;
			u4LineBufWidth = 0x10;
		}

		/* always do these things,for BDP00119760*/
		{

			/* hardware constrain*/
			while ((u4DestWidth % (u4LineBufWidth << 5)) <= 8/*4*/) {
				u4LineBufWidth--;

				if (u4LineBufWidth == 0) {
					return E_IMGRESZ_HAL_FAIL;
				}
			}
		}
		break;

	case IMGRESZ_HAL_RESAMPLE_METHOD_4_TAP:
		fgUseExtend16 = 0;
		u4LineBufWidth = 0x10;
		break;

	default:
		return E_IMGRESZ_HAL_FAIL;
	}

	/* set extend 16*/
	i4HwImgReszSetExtend16Store(u4HwId, fgUseExtend16);
	/* set temp line buffer length*/

	if (fgRPRMode) {
		u4LineBufWidth = 0x10;
	}

	i4Ret = i4HwImgReszSetSRAMLineBufSize(u4HwId, u4LineBufWidth);

	if (i4Ret < 0) {
		return i4Ret;
	}

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetTempBufAddr(u32 u4HwId, u32 u4Addr)
{
	u4Addr = u4Addr ? VA_TO_PA(u4Addr) : 0;

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_TMP_ADDR_BASE, (u4Addr >> 4));

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetJpegPicMode(u32 u4HwId, BOOL fgPicMode)
{
	if (fgPicMode) {
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_INTERFACE_SWITCH, IMG_RESZ_TRACKING_WITH_JPG_HW);
	} else {
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_INTERFACE_SWITCH, IMG_RESZ_NOT_TRACKING_WITH_JPG_HW);
	}

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetJpegPreloadMode(u32 u4HwId, BOOL fgPreloadMode)
{
	u32 u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_JPG_MODE);

	if (fgPreloadMode) {
		u4Value |= IMG_RESZ_PRELOAD_DRAM_DATA;
	} else {
		u4Value &= ~IMG_RESZ_PRELOAD_DRAM_DATA;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_JPG_MODE, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetJpegComponent(u32 u4HwId, BOOL fgYExist, BOOL fgCbExist, BOOL fgCrExist)
{
	u32 u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_JPG_MODE);

	if (fgYExist) {
		u4Value |= IMG_RESZ_RECORD_Y;
	} else {
		u4Value &= ~IMG_RESZ_RECORD_Y;
	}

	if (fgCbExist) {
		u4Value |= IMG_RESZ_RECORD_CB;
	} else {
		u4Value &= ~IMG_RESZ_RECORD_CB;
	}

	if (fgCrExist) {
		u4Value |= IMG_RESZ_RECORD_CR;
	} else {
		u4Value &= ~IMG_RESZ_RECORD_CR;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_JPG_MODE, u4Value);

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetJpegComponentExt(u32 u4HwId, BOOL fgYExist, BOOL fgCbExist,
				     BOOL fgCrExist, BOOL fgRPRRacingModeEnable)
{
	u32 u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_JPG_MODE);

	if ((!(fgYExist && fgCbExist && fgCrExist)) || fgRPRRacingModeEnable) {
		u4Value &= 0xFFFFF000;
		u4Value |= 0x0 << 10;
		u4Value |= 0x0 & 3 << 8;
		u4Value |= 0x0 & 3 << 6;
		u4Value |= 0x0 & 3 << 4;
		u4Value |= 0x0 & 3 << 2;
		u4Value |= 0x0 & 3 << 0;

		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_JPG_MODE, u4Value);
	}

	return S_IMGRESZ_HAL_OK;
}



/*////////////////////////////////////////////////////////////////////////////////*/
/**/
/*     Index Buffer Related*/
/**/
/*////////////////////////////////////////////////////////////////////////////////*/
s32 i4HwImgReszSetIndexBufColorTranslation(u32 u4HwId)
{
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_COLOR_TRANSLATION0, 0x03020100); /* For 2BPP and 4BPP*/
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_COLOR_TRANSLATION1, 0x07060504); /* For 4BPP only*/
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_COLOR_TRANSLATION2, 0x0B0A0908); /* For 4BPP only*/
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_COLOR_TRANSLATION3, 0x0F0E0D0C); /* For 4BPP only*/

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetColorPalletTable(u32 u4HwId, u32 u4ColorPalletTableAddr)
{
	u32 u4i = 0;
	u8 *pu1GblColTbl = (u8 *)u4ColorPalletTableAddr;
	u32 u4GifScaleCptVal = 0;

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MD_CTRL, 0);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MD_CTRL, IMG_RESZ_OSD_CPT_ENABLE);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CPT_ADDR, 0);

	for (u4i = 0; u4i < 256; u4i++) {
		u4GifScaleCptVal = (pu1GblColTbl[u4i * 4] << 24);
		u4GifScaleCptVal = u4GifScaleCptVal + (pu1GblColTbl[u4i * 4 + 1] << 16);
		u4GifScaleCptVal = u4GifScaleCptVal + (pu1GblColTbl[u4i * 4 + 2] << 8);
		u4GifScaleCptVal = u4GifScaleCptVal + (pu1GblColTbl[u4i * 4 + 3]);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_CPT_DATA, u4GifScaleCptVal);
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MD_CTRL, 0);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MD_CTRL, IMG_RESZ_OSD_ED_CPT);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_OSD_MD_CTRL, IMG_RESZ_OSD_CPT_ENABLE + IMG_RESZ_OSD_ED_CPT);

	return S_IMGRESZ_HAL_OK;
}


/*////////////////////////////////////////////////////////////////////////////////*/
/**/
/*     Scale Factro Setting*/
/**/
/*////////////////////////////////////////////////////////////////////////////////*/
s32 i4HwImgReszSetH8TapsScale_Y(u32 u4HwId, u32 u4Offset, u32 u4Factor)
{
	u32 u4Value = u4Factor & 0x03FFFFFF;

	/* parameter checking*/
	if (0 != (u4Factor & 0xFC000000)) {
		return E_IMGRESZ_HAL_FAIL;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H8TAPS_SCL_Y, u4Value);

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H8TAP_OFSET_Y, u4Offset);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetH8TapsScale_Cb(u32 u4HwId, u32 u4Offset, u32 u4Factor)
{
	u32 u4Value = u4Factor & 0x03FFFFFF;

	/* parameter checking*/
	if (0 != (u4Factor & 0xFC000000)) {
		return E_IMGRESZ_HAL_FAIL;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H8TAPS_SCL_CB, u4Value);

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H8TAP_OFSET_CB, u4Offset);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetH8TapsScale_Cr(u32 u4HwId, u32 u4Offset, u32 u4Factor)
{
	u32 u4Value = u4Factor & 0x03FFFFFF;

	/* parameter checking*/
	if (0 != (u4Factor & 0xFC000000)) {
		return E_IMGRESZ_HAL_FAIL;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H8TAPS_SCL_CR, u4Value);

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H8TAP_OFSET_CR, u4Offset);

	return S_IMGRESZ_HAL_OK;

}

s32 i4HwImgReszSetH8tapsCoefficients(u32 u4HwId, IMGRZ_SCALE_FACTOR eFactor)
{
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF0, _au4IMGRZ_FILTERCOEF_H[eFactor][0]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF1, _au4IMGRZ_FILTERCOEF_H[eFactor][1]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF2, _au4IMGRZ_FILTERCOEF_H[eFactor][2]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF3, _au4IMGRZ_FILTERCOEF_H[eFactor][3]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF4, _au4IMGRZ_FILTERCOEF_H[eFactor][4]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF5, _au4IMGRZ_FILTERCOEF_H[eFactor][5]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF6, _au4IMGRZ_FILTERCOEF_H[eFactor][6]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF7, _au4IMGRZ_FILTERCOEF_H[eFactor][7]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF8, _au4IMGRZ_FILTERCOEF_H[eFactor][8]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF9, _au4IMGRZ_FILTERCOEF_H[eFactor][9]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF10, _au4IMGRZ_FILTERCOEF_H[eFactor][10]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF11, _au4IMGRZ_FILTERCOEF_H[eFactor][11]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF12, _au4IMGRZ_FILTERCOEF_H[eFactor][12]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF13, _au4IMGRZ_FILTERCOEF_H[eFactor][13]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF14, _au4IMGRZ_FILTERCOEF_H[eFactor][14]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF15, _au4IMGRZ_FILTERCOEF_H[eFactor][15]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF16, _au4IMGRZ_FILTERCOEF_H[eFactor][16]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H_COEF17, _au4IMGRZ_FILTERCOEF_H[eFactor][17]);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetV4tapsCoefficients(u32 u4HwId, IMGRZ_SCALE_FACTOR eFactor)
{
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF0, _au4IMGRZ_FILTERCOEF_V[eFactor][0]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF1, _au4IMGRZ_FILTERCOEF_V[eFactor][1]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF2, _au4IMGRZ_FILTERCOEF_V[eFactor][2]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF3, _au4IMGRZ_FILTERCOEF_V[eFactor][3]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF4, _au4IMGRZ_FILTERCOEF_V[eFactor][4]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF5, _au4IMGRZ_FILTERCOEF_V[eFactor][5]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF6, _au4IMGRZ_FILTERCOEF_V[eFactor][6]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF7, _au4IMGRZ_FILTERCOEF_V[eFactor][7]);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_COEF8, _au4IMGRZ_FILTERCOEF_V[eFactor][8]);

	return S_IMGRESZ_HAL_OK;
}


IMGRZ_SCALE_FACTOR eHwImgReszCalcScaleFactor(u32 u4SrcSize, u32 u4TgtSize)
{
	u32 u4Scale = u4TgtSize * 10000 / u4SrcSize;

	if (u4Scale >= 10000) {
		return FACTOR_1;
	} else if (u4Scale >= 5000) {
		return FACTOR_0_5;
	} else if (u4Scale >= 2500) {
		return FACTOR_0_25;
	} else if (u4Scale >= 1250) {
		return FACTOR_0_125;
	} else if (u4Scale >= 625) {
		return FACTOR_0_0625;
	} else {
		return FACTOR_0;
	}
}


s32 i4HwImgReszSetHecScale_Y(u32 u4HwId, u32 u4Offset, u32 u4Factor, BOOL fgScaleUp)
{
	u32 u4Value = u4Factor & 0x00000FFF;

	u4Value += ((u4Offset & 0x000007FF) << 12);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HSA_SCL_Y, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetHecScale_Cb(u32 u4HwId, u32 u4Offset, u32 u4Factor, BOOL fgScaleUp)
{
	u32 u4Value = u4Factor & 0x00000FFF;

	u4Value += ((u4Offset & 0x000007FF) << 12);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HSA_SCL_CB, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetHecScale_Cr(u32 u4HwId, u32 u4Offset, u32 u4Factor, BOOL fgScaleUp)
{
	u32 u4Value = u4Factor & 0x00000FFF;

	u4Value += ((u4Offset & 0x000007FF) << 12);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HSA_SCL_CR, u4Value);

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetV4TapsScale_Y(u32 u4HwId, u32 u4Offset, u32 u4Factor)
{
	u32 u4Value = u4Factor & 0x03FFFFFF;

	/* parameter checking*/
	if (0 != (u4Factor & 0xFC000000)) {
		return E_IMGRESZ_HAL_FAIL;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V4TAPS_SCL_Y, u4Value);

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V4TAP_OFSET_Y, u4Offset);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetV4TapsScale_Cb(u32 u4HwId, u32 u4Offset, u32 u4Factor)
{
	u32 u4Value = u4Factor & 0x03FFFFFF;

	/* parameter checking*/
	if (0 != (u4Factor & 0xF3000000)) {
		return E_IMGRESZ_HAL_FAIL;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V4TAPS_SCL_CB, u4Value);

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V4TAP_OFSET_C, u4Offset);

	return S_IMGRESZ_HAL_OK;
}

/* Maybe the following function is not needed for v4-Tap is not supported in JPEG mode.*/
s32 i4HwImgReszSetV4TapsScale_Cr(u32 u4HwId, u32 u4Offset, u32 u4Factor)
{
	u32 u4Value = u4Factor & 0x03FFFFFF;

	/* parameter checking*/
	if (0 != (u4Factor & 0xF3000000)) {
		return E_IMGRESZ_HAL_FAIL;
	}

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V4TAPS_SCL_CR, u4Value);

	/*/Maybe it is not needed.Check it.*/
	/*vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V4TAP_OFSET_C, u4Offset);*/

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetVScale_Y(u32 u4HwId, u32 u4Offset, u32 u4Factor, BOOL fgScaleUp)
{
	u32 u4Value = u4Factor & 0x000007FF;

	/* parameter checking*/
	if ((0 != (u4Factor & 0xFFFFF800)) || (0 != (u4Offset & 0xFFFFF000))) {
		return E_IMGRESZ_HAL_FAIL;
	}

	u4Value += ((u4Offset & 0x000007FF) << 12) + ((fgScaleUp & 0x1) << 11);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_SCL_Y, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetVScale_Cb(u32 u4HwId, u32 u4Offset, u32 u4Factor, BOOL fgScaleUp)
{
	u32 u4Value = u4Factor & 0x000007FF;

	/* parameter checking*/
	if ((0 != (u4Factor & 0xFFFFF800)) || (0 != (u4Offset & 0xFFFFF000))) {
		return E_IMGRESZ_HAL_FAIL;
	}

	u4Value += ((u4Offset & 0x000007FF) << 12) + ((fgScaleUp & 0x1) << 11);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_SCL_CB, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetVScale_Cr(u32 u4HwId, u32 u4Offset, u32 u4Factor, BOOL fgScaleUp)
{
	u32 u4Value = u4Factor & 0x000007FF;

	/* parameter checking*/
	if ((0 != (u4Factor & 0xFFFFF800)) || (0 != (u4Offset & 0xFFFFF000))) {
		return E_IMGRESZ_HAL_FAIL;
	}

	u4Value += ((u4Offset & 0x000007FF) << 12) + ((fgScaleUp & 0x1) << 11);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V_SCL_CR, u4Value);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetHScaleFactor(u32 u4HwId, u32 u4SrcWidth, u32 u4TgtWidth,
				 IMGRESZ_HAL_RESAMPLE_METHOD_T eResampleMethod,
				 IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat,
				 IMGRESZ_HAL_IMG_BUF_FORMAT_T *prDestBufferFormat)
{
	u32 u4Factor, u4Offset;
	u32 u4SrcWidthY = 0, u4SrcWidthCb = 0, u4SrcWidthCr = 0;
	u32 u4TgtWidthY = 0, u4TgtWidthCb = 0, u4TgtWidthCr = 0;

	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prSrcBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
			u4SrcWidthY = u4SrcWidth;
			u4SrcWidthCb = u4SrcWidth >> 1;
			u4SrcWidthCr = u4SrcWidth >> 1;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4SrcWidthY = u4SrcWidth;
			u4SrcWidthCb = u4SrcWidth;
			u4SrcWidthCr = u4SrcWidth;
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER:
		u4SrcWidthY = u4SrcWidth;
		u4SrcWidthCb = u4SrcWidth * prSrcBufferFormat->u4HSampleFactor[1] /
			       prSrcBufferFormat->u4HSampleFactor[0];
		u4SrcWidthCr = u4SrcWidth * prSrcBufferFormat->u4HSampleFactor[2] /
			       prSrcBufferFormat->u4HSampleFactor[0];
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		u4SrcWidthY = u4SrcWidth;
		break;
	}

	switch (prDestBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prDestBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
			u4TgtWidthY = u4TgtWidth;
			u4TgtWidthCb = u4TgtWidth >> 1;
			u4TgtWidthCr = u4TgtWidth >> 1;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4TgtWidthY = u4TgtWidth;
			u4TgtWidthCb = u4TgtWidth;
			u4TgtWidthCr = u4TgtWidth;
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		u4TgtWidthY = u4TgtWidth;
		u4TgtWidthCb = u4TgtWidth;
		u4TgtWidthCr = u4TgtWidth;
		break;

	default:
		break;
	}

	/* Y*/
	if ((eResampleMethod == IMGRESZ_HAL_RESAMPLE_METHOD_8_TAP) || (u4SrcWidthY <= u4TgtWidthY)) {
		if (u4SrcWidthY == u4TgtWidthY) {
			u4Factor = (0x40000 * u4SrcWidthY + (u4TgtWidthY >> 1)) / u4TgtWidthY;
		} else {
			u4Factor = (0x40000 * (u4SrcWidthY - 1) + ((u4TgtWidthY - 1) >> 1)) /
				   (u4TgtWidthY - 1);		  /* For centralize*/
		}

		u4Offset = 0;
		i4HwImgReszSetH8TapsScale_Y(u4HwId, u4Offset, u4Factor);
		i4HwImgReszSetH8tapsCoefficients(u4HwId, eHwImgReszCalcScaleFactor(u4SrcWidth, u4TgtWidth));
		i4HwImgReszSetHecScale_Y(u4HwId, 0, 0x800, TRUE);
	} else {
		u4Factor = (2048 * u4TgtWidthY + (u4SrcWidthY >> 1)) / u4SrcWidthY;
		u4Offset = 2048 - u4Factor;
		i4HwImgReszSetHecScale_Y(u4HwId, u4Offset, u4Factor, TRUE);
		i4HwImgReszSetH8TapsScale_Y(u4HwId, 0, 0x40000);
	}

	if ((prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER) ||
	    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER) ||
	    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER)) {
		return S_IMGRESZ_HAL_OK;
	}

	/* Cb*/
	if ((eResampleMethod == IMGRESZ_HAL_RESAMPLE_METHOD_8_TAP) || (u4SrcWidthCb <= u4TgtWidthCb)) {
		if ((u4SrcWidthCb == u4TgtWidthCb) || (u4TgtWidthCb == 1)) {
			u4Factor = (0x40000 * u4SrcWidthCb + (u4TgtWidthCb >> 1)) / u4TgtWidthCb;
		} else {
			u4Factor = (0x40000 * (u4SrcWidthCb - 1) + ((u4TgtWidthCb - 1) >> 1)) /
				   (u4TgtWidthCb - 1); /* For centralize*/
		}

		u4Offset = 0;
		i4HwImgReszSetH8TapsScale_Cb(u4HwId, u4Offset, u4Factor);
		i4HwImgReszSetHecScale_Cb(u4HwId, 0, 0x800, TRUE);
	} else {
		u4Factor = (2048 * u4TgtWidthCb + (u4SrcWidthCb >> 1)) / u4SrcWidthCb;
		u4Offset = 2048 - u4Factor;
		i4HwImgReszSetHecScale_Cb(u4HwId, u4Offset, u4Factor, TRUE);
		i4HwImgReszSetH8TapsScale_Cb(u4HwId, 0, 0x40000);
	}

	if (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER) {
		return S_IMGRESZ_HAL_OK;
	}

	/* Cr*/
	if ((eResampleMethod == IMGRESZ_HAL_RESAMPLE_METHOD_8_TAP) || (u4SrcWidthCr <= u4TgtWidthCr)) {
		if ((u4SrcWidthCr == u4TgtWidthCr) || (u4TgtWidthCb == 1)) {
			u4Factor = (0x40000 * u4SrcWidthCr + (u4TgtWidthCr >> 1)) / u4TgtWidthCr;
		} else {
			u4Factor = (0x40000 * (u4SrcWidthCr - 1) + ((u4TgtWidthCr - 1) >> 1)) /
				   (u4TgtWidthCr - 1); /* For centralize*/
		}

		u4Offset = 0;
		i4HwImgReszSetH8TapsScale_Cr(u4HwId, u4Offset, u4Factor);
		i4HwImgReszSetHecScale_Cr(u4HwId, 0, 0x800, TRUE);
	} else {
		u4Factor = (2048 * u4TgtWidthCr + (u4SrcWidthCr >> 1)) / u4SrcWidthCr;
		u4Offset = 2048 - u4Factor;
		i4HwImgReszSetHecScale_Cr(u4HwId, u4Offset, u4Factor, TRUE);
		i4HwImgReszSetH8TapsScale_Cr(u4HwId, 0, 0x40000);
	}

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetClipHScaleFactor(u32 u4HwId, u32 u4SrcWidth, u32 u4TgtWidth,
				     u32 u4TgtClipWidth, u32 u4XOff_Clip,
				     u32 *u4SrcClipYWidth,
				     u32 *u4SrcClipCWidth,
				     IMGRESZ_HAL_RESAMPLE_METHOD_T eResampleMethod,
				     IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat,
				     IMGRESZ_HAL_IMG_BUF_FORMAT_T *prDestBufferFormat)
{
	unsigned int u4Factor, u4Offset;
	unsigned int u4SrcWidthY = 0, u4SrcWidthCb = 0, u4SrcWidthCr = 0, u4XOff_Clip_Y = 0,
		     u4XOff_Clip_Cb = 0, u4XOff_Clip_Cr = 0;
	unsigned int u4TgtWidthY = 0, u4TgtWidthCb = 0, u4TgtWidthCr = 0;
	unsigned int u4Tmp_SrcOftX = 0, u4Tmp_SrcOftCb = 0, u4Tmp_SrcFilterOftX = 0,
		     u4Tmp_SrcFilterOftCb = 0;

	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prSrcBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
			u4SrcWidthY = u4SrcWidth;
			u4SrcWidthCb = u4SrcWidth >> 1;
			u4SrcWidthCr = u4SrcWidth >> 1;
			u4XOff_Clip_Y = u4XOff_Clip;
			u4XOff_Clip_Cb = u4XOff_Clip >> 1;
			u4XOff_Clip_Cr = u4XOff_Clip >> 1;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4SrcWidthY = u4SrcWidth;
			u4SrcWidthCb = u4SrcWidth;
			u4SrcWidthCr = u4SrcWidth;
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER:
		u4SrcWidthY = u4SrcWidth;
		u4SrcWidthCb = u4SrcWidth * prSrcBufferFormat->u4HSampleFactor[1] /
			       prSrcBufferFormat->u4HSampleFactor[0];
		u4SrcWidthCr = u4SrcWidth * prSrcBufferFormat->u4HSampleFactor[2] /
			       prSrcBufferFormat->u4HSampleFactor[0];
		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		u4SrcWidthY = u4SrcWidth;
		break;
	}

	switch (prDestBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prDestBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
			u4TgtWidthY = u4TgtWidth;
			u4TgtWidthCb = u4TgtWidth >> 1;
			u4TgtWidthCr = u4TgtWidth >> 1;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4TgtWidthY = u4TgtWidth;
			u4TgtWidthCb = u4TgtWidth;
			u4TgtWidthCr = u4TgtWidth;
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		u4TgtWidthY = u4TgtWidth;
		u4TgtWidthCb = u4TgtWidth;
		u4TgtWidthCr = u4TgtWidth;
		break;

	default:
		break;
	}

	/* Y*/
	if (eResampleMethod == IMGRESZ_HAL_RESAMPLE_METHOD_8_TAP) {
		if (u4SrcWidthY == u4TgtWidthY) {
			u4Factor = (0x40000 * u4SrcWidthY + (u4TgtWidthY >> 1)) / u4TgtWidthY;
		} else {
			u4Factor = (0x40000 * (u4SrcWidthY - 1) + ((u4TgtWidthY - 1) >> 1)) /
				   (u4TgtWidthY - 1); /* For centralize*/
			/*u4Factor = (0x40000 * u4SrcWidthY + (u4TgtWidthY >> 1)) / u4TgtWidthY;*/
		}

		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4XOff_Clip == %d\n", (int)u4XOff_Clip);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Factor == 0x%x\n",  u4Factor);

		u4Tmp_SrcOftX = u4XOff_Clip_Y * u4Factor;
		/*u4Tmp_SrcFilterOftX = (u4Tmp_SrcOftX&0x3ffff) +
			(((u4Tmp_SrcOftX>>18) >=3)?0x40000:(u4Tmp_SrcOftX >>18)*0x40000);*/
		u4Tmp_SrcFilterOftX = (u4Tmp_SrcOftX & 0x3ffff) +
				      (((u4Tmp_SrcOftX >> 18) >= 3) ? 0xC0000 : (u4Tmp_SrcOftX >> 18) * 0x40000);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Tmp_SrcFilterOftX == 0x%x \r\n",  u4Tmp_SrcFilterOftX);
		u4Tmp_SrcOftX = ((u4Tmp_SrcOftX >> 18) >= 3) ? ((u4Tmp_SrcOftX >> 18) - 3) : (u4Tmp_SrcOftX >> 18);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Tmp_SrcOftX == 0x%x \r\n",  u4Tmp_SrcOftX);
		/* *u4SrcClipYWidth = ((u4TgtClipWidth*u4Factor+u4Tmp_SrcFilterOftX) +0x3ffff)>>18; */
		*u4SrcClipYWidth  = u4SrcWidth  - u4Tmp_SrcOftX;

		/* IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Tmp_SrcOftX, *u4SrcClipYWidth ,u4Tmp_SrcFilterOftX == 0x%x,
			 0x%x== ,0x%x== \r\n", u4Tmp_SrcOftX, (unsigned int)*u4SrcClipYWidth, u4Tmp_SrcFilterOftX);*/

		if (0 == *u4SrcClipYWidth) {
			*u4SrcClipYWidth = 4;
			u4Tmp_SrcOftX -= 4;
		}

		/* u4Tmp_SrcOftX += u4SrcImgXOff */
		u4Offset = 0;

		/* vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_H8TAP_OFSET_Y, u4Tmp_SrcFilterOftX);//u4Tmp_SrcFilterOftX); */
		i4HwImgReszSetH8TapsScale_Y(u4HwId, u4Tmp_SrcFilterOftX, u4Factor);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_Y,
				  u4HwImgReszRead32(u4HwId , RW_IMG_RESZ_SRC_OFFSET_Y) & 0xFFF);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_Y,
				  u4HwImgReszRead32(u4HwId , RW_IMG_RESZ_SRC_OFFSET_Y) | (u4Tmp_SrcOftX << 12));
		i4HwImgReszSetH8tapsCoefficients(u4HwId, eHwImgReszCalcScaleFactor(u4SrcWidth, u4TgtWidth));
		i4HwImgReszSetHecScale_Y(u4HwId, 0, 0x800, TRUE);
	}

	if ((prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER) ||
	    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER) ||
	    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER)) {
		return S_IMGRESZ_HAL_OK;
	}

	/* Cb */
	if (eResampleMethod == IMGRESZ_HAL_RESAMPLE_METHOD_8_TAP) {
		if (u4SrcWidthCb == u4TgtWidthCb) {
			u4Factor = (0x40000 * u4SrcWidthCb + (u4TgtWidthCb >> 1)) / u4TgtWidthCb;
		} else {
			u4Factor = (0x40000 * (u4SrcWidthCb - 1) + ((u4TgtWidthCb - 1) >> 1)) /
				   (u4TgtWidthCb - 1); /* For centralize */
		}

		u4Tmp_SrcOftCb = u4XOff_Clip_Cb * u4Factor;
		u4Tmp_SrcFilterOftCb = (u4Tmp_SrcOftCb & 0x3ffff) +
				       (((u4Tmp_SrcOftCb >> 18) >= 3) ? 0xC0000 : (u4Tmp_SrcOftCb >> 18) * 0x40000);
		u4Tmp_SrcOftCb = ((u4Tmp_SrcOftCb >> 18) >= 3) ?
				 ((u4Tmp_SrcOftCb >> 18) - 3) : (u4Tmp_SrcOftCb >> 18);
		/*u4SrcClipCWidth = (((u4TgtClipWidth/2)*u4Factor+u4Tmp_SrcFilterOftX)+0x3ffff)>>18; */
		*u4SrcClipCWidth = u4SrcWidthCb - u4Tmp_SrcOftCb;
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Tmp_SrcOftX, *u4SrcClipCWidth == 0x%x, 0x%x== \r\n",
			 u4Tmp_SrcOftCb, (unsigned int)*u4SrcClipCWidth);

		if (0 == *u4SrcClipCWidth) {
			*u4SrcClipCWidth = 4;
			u4Tmp_SrcOftCb -= 4;
		}

		i4HwImgReszSetH8TapsScale_Cb(u4HwId, u4Tmp_SrcFilterOftCb, u4Factor);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_CB,
				  u4HwImgReszRead32(u4HwId , RW_IMG_RESZ_SRC_OFFSET_CB) & 0xFFF);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_CB,
				  u4HwImgReszRead32(u4HwId , RW_IMG_RESZ_SRC_OFFSET_CB) | (u4Tmp_SrcOftCb << 12));

		i4HwImgReszSetHecScale_Cb(u4HwId, 0, 0x800, TRUE);
	}

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetVScaleFactor(u32 u4HwId,
				 u32 u4SrcHeight, u32 u4TgtHeight,
				 IMGRESZ_HAL_RESAMPLE_METHOD_T eResampleMethod,
				 IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat,
				 IMGRESZ_HAL_IMG_BUF_FORMAT_T *prDestBufferFormat)
{
	u32 u4Factor, u4Offset;
	u32 u4SrcHeightY = 0, u4SrcHeightCb = 0, u4SrcHeightCr = 0;
	u32 u4TgtHeightY = 0, u4TgtHeightCb = 0, u4TgtHeightCr = 0;

	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prSrcBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
			u4SrcHeightY = u4SrcHeight;
			u4SrcHeightCb = u4SrcHeight >> 1;
			u4SrcHeightCr = u4SrcHeight >> 1;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4SrcHeightY = u4SrcHeight;
			u4SrcHeightCb = u4SrcHeight;
			u4SrcHeightCr = u4SrcHeight;
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER:
		u4SrcHeightY = u4SrcHeight;
		u4SrcHeightCb = u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[1] /
				prSrcBufferFormat->u4VSampleFactor[0];
		u4SrcHeightCr = u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[2] /
				prSrcBufferFormat->u4VSampleFactor[0];

		/* For jpeg picture mode, prevent source height 401 come two interrupt (Y interrupt and C interrupt) */
		if ((u4SrcHeightCb * prSrcBufferFormat->u4VSampleFactor[0]) !=
				(u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[1])) {
			u4SrcHeightCb++;
		}

		if ((u4SrcHeightCr * prSrcBufferFormat->u4VSampleFactor[0]) !=
				(u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[2])) {
			u4SrcHeightCr++;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		u4SrcHeightY = u4SrcHeight;
		break;
	}

	switch (prDestBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prDestBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
			u4TgtHeightY = u4TgtHeight;
			u4TgtHeightCb = u4TgtHeight >> 1;
			u4TgtHeightCr = u4TgtHeight >> 1;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4TgtHeightY = u4TgtHeight;
			u4TgtHeightCb = u4TgtHeight;
			u4TgtHeightCr = u4TgtHeight;
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		u4TgtHeightY = u4TgtHeight;
		u4TgtHeightCb = u4TgtHeight;
		u4TgtHeightCr = u4TgtHeight;
		break;

	default:
		break;
	}

	switch (eResampleMethod) {
	case IMGRESZ_HAL_RESAMPLE_METHOD_4_TAP:
		/* Y*/
		u4Factor = (0x40000 * u4SrcHeightY + (u4TgtHeightY >> 1)) / u4TgtHeightY;

		u4Offset = 0;
		i4HwImgReszSetV4TapsScale_Y(u4HwId, u4Offset, u4Factor);
		i4HwImgReszSetV4tapsCoefficients(u4HwId, eHwImgReszCalcScaleFactor(u4SrcHeight, u4TgtHeight));

		if ((prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER) ||
		    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER) ||
		    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER)) {
			break;
		}

		/* Cb*/
		u4Factor = (0x40000 * u4SrcHeightCb + (u4TgtHeightCb >> 1)) / u4TgtHeightCb;

		u4Offset = 0;
		i4HwImgReszSetV4TapsScale_Cb(u4HwId, u4Offset, u4Factor);

		if (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER) {
			break;
		}

		/* Cr*/
		u4Factor = (0x40000 * u4SrcHeightCr + (u4TgtHeightCr >> 1)) / u4TgtHeightCr;

		u4Offset = 0;
		i4HwImgReszSetV4TapsScale_Cr(u4HwId, u4Offset, u4Factor);
		i4HwImgReszSetV4tapsCoefficients(u4HwId, eHwImgReszCalcScaleFactor(u4SrcHeight, u4TgtHeight));
		break;

	case IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR:

		/* Y*/
		if (u4TgtHeightY < u4SrcHeightY) { /* scale dowm, source accumulator */
			u4Factor = (2048 * u4TgtHeightY + (u4SrcHeightY >> 1)) / u4SrcHeightY;
			u4Offset = 2048 - u4Factor;
#if IMGRESZ_HAL_SUBLINE_ADJUST

			/* Adjust subline. */
			if ((prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER) &&
			    (!prSrcBufferFormat->fgProgressiveFrame) &&
			    (!prSrcBufferFormat->fgTopField)) {
				switch (prSrcBufferFormat->eYUVFormat) {
				case IMGRESZ_HAL_IMG_YUV_FORMAT_420: {
					u32 u4Value, u4Q, u4R_norm;

					//ASSERT(u4TgtHeight);
					u4Q = (u4SrcHeight - u4TgtHeight) / (u4TgtHeight * 2);
					u4R_norm = (((u4SrcHeight - u4TgtHeight) - (u4Q * u4TgtHeight * 2)) * 2048) /
							(2 * u4SrcHeight);

					u4Offset += u4R_norm;

					u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_SRC_SIZE_Y);
					u4Value &= 0xFFFF0000;
					u4Value |= ((u4SrcHeightY - u4Q) & 0xFFFF);
					vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_Y, u4Value);
				}
				break;

				default:
					break;
				}
			}

#endif
			i4HwImgReszSetVScale_Y(u4HwId, u4Offset, u4Factor, FALSE);
		} else { /*scale up, bilinear */
			if (u4TgtHeightY == u4SrcHeightY) {
				u4Factor = 0;
				u4Offset = 0;
			} else {
				/*u4Factor = (2048 * u4SrcHeightY + (u4TgtHeightY >> 1)) / u4TgtHeightY;*/
				u4Factor = (2048 * (u4SrcHeightY - 1) + ((u4TgtHeightY - 1) >> 1)) /
				    (u4TgtHeightY - 1); /* For centralize*/
				u4Offset = 0;
			}

			i4HwImgReszSetVScale_Y(u4HwId, u4Offset, u4Factor, TRUE);
		}

		if ((prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER) ||
		    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER) ||
		    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER)) {
			break;
		}

		/* Cb*/
		if (u4TgtHeightCb < u4SrcHeightCb) { /*scale dowm, source accumulator*/
			u4Factor = (2048 * u4TgtHeightCb + (u4SrcHeightCb >> 1)) / u4SrcHeightCb;
			u4Offset = 2048 - u4Factor;
#if IMGRESZ_HAL_SUBLINE_ADJUST

			/* Adjust subline.*/
			if ((prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER) &&
			    (!prSrcBufferFormat->fgProgressiveFrame) &&
			    (!prSrcBufferFormat->fgTopField)) {
				switch (prSrcBufferFormat->eYUVFormat) {
				case IMGRESZ_HAL_IMG_YUV_FORMAT_420: {
					u32 u4Value, u4Q, u4R_norm;

					//ASSERT(u4TgtHeight);
					u4Q = (u4SrcHeight - u4TgtHeight) / (u4TgtHeight * 2);
					u4R_norm = (((u4SrcHeight - u4TgtHeight) - (u4Q * u4TgtHeight * 2)) * 2048) /
							(2 * u4SrcHeight);

					u4Offset += u4R_norm;

					u4Value = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_SRC_SIZE_CB);
					u4Value &= 0xFFFF0000;
					u4Value |= ((u4SrcHeightCb - u4Q) & 0xFFFF);
					vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_SIZE_CB, u4Value);
				}
				break;

				default:
					break;
				}
			}

#endif
			i4HwImgReszSetVScale_Cb(u4HwId, u4Offset, u4Factor, FALSE);
		} else { /*scale up, bilinear*/
			if (u4TgtHeightCb == u4SrcHeightCb) {
				u4Factor = 0;
				u4Offset = 0;
			} else {
				/*u4Factor = (2048 * u4SrcHeightCb + (u4TgtHeightCb >> 1)) / u4TgtHeightCb;*/
				u4Factor = (2048 * (u4SrcHeightCb - 1) + ((u4TgtHeightCb - 1) >> 1)) /
					(u4TgtHeightCb - 1); /* For centralize*/
				u4Offset = 0;
			}

			i4HwImgReszSetVScale_Cb(u4HwId, u4Offset, u4Factor, TRUE);
		}

		if (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER) {
			break;
		}

		/* Cr*/
		if (u4TgtHeightCr < u4SrcHeightCr) { /*scale dowm, source accumulator*/
			u4Factor = (2048 * u4TgtHeightCr + (u4SrcHeightCr >> 1)) / u4SrcHeightCr;
			u4Offset = 2048 - u4Factor;
			i4HwImgReszSetVScale_Cr(u4HwId, u4Offset, u4Factor, FALSE);
		} else { /*scale up, bilinear*/
			if (u4TgtHeightCr == u4SrcHeightCr) {
				u4Factor = 0;
				u4Offset = 0;
			} else {
				/*u4Factor = (2048 * u4SrcHeightCr + (u4TgtHeightCr >> 1)) / u4TgtHeightCr;*/
				u4Factor = (2048 * u4SrcHeightCr + (u4TgtHeightCr >> 1)) /
					u4TgtHeightCr; /* For centralize*/
				u4Offset = 0;
			}

			i4HwImgReszSetVScale_Cr(u4HwId, u4Offset, u4Factor, TRUE);
		}

		break;

	default:
		return E_IMGRESZ_HAL_FAIL;
	}

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetClipVScaleFactor(u32 u4HwId,
				     u32 u4SrcHeight, u32 u4TgtHeight, u32 u4TgtClipHeight, u32 u4YOff_Clip,
				     u32 *u4SrcClipYHeight,
				     u32 *u4SrcClipCHeight,
				     IMGRESZ_HAL_RESAMPLE_METHOD_T eResampleMethod,
				     IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat,
				     IMGRESZ_HAL_IMG_BUF_FORMAT_T *prDestBufferFormat)
{
	unsigned int u4Factor, u4Offset;
	unsigned int u4SrcHeightY = 0, u4SrcHeightCb = 0, u4SrcHeightCr = 0, u4YOff_Clip_Y = 0,
			u4YOff_Clip_Cb = 0, u4YOff_Clip_Cr = 0;
	unsigned int u4TgtHeightY = 0, u4TgtHeightCb = 0, u4TgtHeightCr = 0;
	unsigned int u4Tmp_SrcOftY = 0, u4Tmp_SrcOftCb = 0, u4Tmp_SrcFilterOftY = 0,
			u4Tmp_SrcFilterOftCb = 0;

	switch (prSrcBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prSrcBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
			u4SrcHeightY = u4SrcHeight;
			u4SrcHeightCb = u4SrcHeight >> 1;
			u4SrcHeightCr = u4SrcHeight >> 1;
			u4YOff_Clip_Y = u4YOff_Clip;
			u4YOff_Clip_Cb = u4YOff_Clip >> 1;
			u4YOff_Clip_Cr = u4YOff_Clip >> 1;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4SrcHeightY = u4SrcHeight;
			u4SrcHeightCb = u4SrcHeight;
			u4SrcHeightCr = u4SrcHeight;
			u4YOff_Clip_Y = u4YOff_Clip;
			u4YOff_Clip_Cb = u4YOff_Clip;
			u4YOff_Clip_Cr = u4YOff_Clip;
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER:
		u4SrcHeightY = u4SrcHeight;
		u4SrcHeightCb = u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[1] /
				prSrcBufferFormat->u4VSampleFactor[0];
		u4SrcHeightCr = u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[2] /
				prSrcBufferFormat->u4VSampleFactor[0];

		/* For jpeg picture mode, prevent source height 401 come two
			interrupt (Y interrupt and C interrupt)*/
		if ((u4SrcHeightCb * prSrcBufferFormat->u4VSampleFactor[0]) !=
				(u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[1])) {
			u4SrcHeightCb++;
		}

		if ((u4SrcHeightCr * prSrcBufferFormat->u4VSampleFactor[0]) !=
				(u4SrcHeight * prSrcBufferFormat->u4VSampleFactor[2])) {
			u4SrcHeightCr++;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		u4SrcHeightY = u4SrcHeight;
		break;
	}

	switch (prDestBufferFormat->eBufferMainFormat) {
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER:
		switch (prDestBufferFormat->eYUVFormat) {
		case IMGRESZ_HAL_IMG_YUV_FORMAT_420:
			u4TgtHeightY = u4TgtHeight;
			u4TgtHeightCb = u4TgtHeight >> 1;
			u4TgtHeightCr = u4TgtHeight >> 1;
			break;

		case IMGRESZ_HAL_IMG_YUV_FORMAT_422:
		case IMGRESZ_HAL_IMG_YUV_FORMAT_444:
			u4TgtHeightY = u4TgtHeight;
			u4TgtHeightCb = u4TgtHeight;
			u4TgtHeightCr = u4TgtHeight;
			break;
		}

		break;

	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER:
	case IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER:
		u4TgtHeightY = u4TgtHeight;
		u4TgtHeightCb = u4TgtHeight;
		u4TgtHeightCr = u4TgtHeight;
		break;

	default:
		break;
	}

	switch (eResampleMethod) {
	case IMGRESZ_HAL_RESAMPLE_METHOD_4_TAP:
		/* Y*/
		u4Factor = (0x40000 * (u4SrcHeightY - 1) + ((u4TgtHeightY - 1) >> 1)) / (u4TgtHeightY - 1);
		u4Offset = 0;

		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4YOff_Clip == %d \r\n", (unsigned int)u4YOff_Clip);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Factor == %d \r\n",  u4Factor);

		u4Tmp_SrcOftY = u4YOff_Clip_Y * u4Factor;
		/* *u4Tmp_SrcFilterOftY = (u4Tmp_SrcOftY&0x3ffff) + (((u4Tmp_SrcOftY>>18)>=1)?
			0x40000:(u4Tmp_SrcOftY>>18)*0x40000);*/
		u4Tmp_SrcFilterOftY = (u4Tmp_SrcOftY & 0x3ffff) +
			(((u4Tmp_SrcOftY >> 18) >= 1) ? 0x40000 : (u4Tmp_SrcOftY >> 18) * 0x40000);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Tmp_SrcFilterOftY == %x \r\n",  u4Tmp_SrcFilterOftY);

		u4Tmp_SrcOftY = ((u4Tmp_SrcOftY >> 18) >= 1) ? ((u4Tmp_SrcOftY >> 18) - 1) : (u4Tmp_SrcOftY >> 18);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Tmp_SrcOftY == %x \r\n",  u4Tmp_SrcOftY);

		*u4SrcClipYHeight = u4SrcHeight - u4Tmp_SrcOftY;
		/* *u4SrcClipYHeight = ((u4TgtClipHeight*u4Factor+u4Tmp_SrcFilterOftY)+0x3ffff)>>18; */
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Tmp_SrcOftY, *u4SrcClipYHeight == 0x%x, 0x%x== \r\n",
			u4Tmp_SrcOftY, (unsigned int)*u4SrcClipYHeight);

		if (0 == *u4SrcClipYHeight) {
			*u4SrcClipYHeight = 4;
			u4Tmp_SrcOftY -= 4;
		}

		i4HwImgReszSetV4TapsScale_Y(u4HwId, u4Tmp_SrcFilterOftY, u4Factor);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_Y,
				u4HwImgReszRead32(u4HwId , RW_IMG_RESZ_SRC_OFFSET_Y) & 0xFFF000);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_Y,
				u4HwImgReszRead32(u4HwId , RW_IMG_RESZ_SRC_OFFSET_Y) | u4Tmp_SrcOftY);
		i4HwImgReszSetV4tapsCoefficients(u4HwId, eHwImgReszCalcScaleFactor(u4SrcHeight, u4TgtHeight));

		if ((prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER) ||
		    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER) ||
		    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER)) {
			break;
		}

		/* Cb */
		u4Factor = (0x40000 * (u4SrcHeightCb - 1) + ((u4TgtHeightCb - 1) >> 1)) / (u4TgtHeightCb - 1);
		u4Tmp_SrcOftCb = u4YOff_Clip_Cb * u4Factor;
		u4Tmp_SrcFilterOftCb = (u4Tmp_SrcOftCb & 0x3ffff) +
				(((u4Tmp_SrcOftCb >> 18) >= 1) ? 0x40000 : (u4Tmp_SrcOftCb >> 18) * 0x40000);
		u4Tmp_SrcOftCb = ((u4Tmp_SrcOftCb >> 18) >= 1) ?
				((u4Tmp_SrcOftCb >> 18) - 1) : (u4Tmp_SrcOftCb >> 18);
		/* *u4SrcClipCHeight = (((u4TgtClipHeight/2)*u4Factor+u4Tmp_SrcFilterOftY)+0x3ffff)>>18; */
		*u4SrcClipCHeight = u4SrcHeightCb - u4Tmp_SrcOftCb;
		IMGR_LOG(IMGR_LOG_LVL_DBG, "u4Tmp_SrcOftY, u4SrcClipCHeight == 0x%x, 0x%x== \r\n",
				u4Tmp_SrcOftCb, (unsigned int)*u4SrcClipCHeight);

		if (0 == *u4SrcClipCHeight) {
			*u4SrcClipCHeight = 4;
			u4Tmp_SrcOftCb -= 4;
		}

		i4HwImgReszSetV4TapsScale_Cb(u4HwId, u4Tmp_SrcFilterOftCb, u4Factor);
		/* vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_V4TAP_OFSET_C, u4Tmp_SrcFilterOftY);*/
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_CB,
			u4HwImgReszRead32(u4HwId , RW_IMG_RESZ_SRC_OFFSET_CB) & 0xFFF000);
		vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_SRC_OFFSET_CB,
			u4HwImgReszRead32(u4HwId , RW_IMG_RESZ_SRC_OFFSET_CB) | u4Tmp_SrcOftCb);

		if (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER) {
			break;
		}

		/*i4HwImgReszSetV4tapsCoefficients(u4HwId, eHwImgReszCalcScaleFactor(u4SrcHeight, u4TgtHeight));*/
		break;

	default:
		return E_IMGRESZ_HAL_FAIL;
	}

	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetRPRHScaleFactor(u32 u4HwId, s32 i4SrcWidth, s32 i4TgtWidth)
{
	u32 u4Factor, u4Offset;
	/* u32 u4SrcWidthY = 0,u4SrcWidthCb = 0,u4SrcWidthCr = 0;*/
	/* u32 u4TgtWidthY = 0,u4TgtWidthCb = 0,u4TgtWidthCr = 0;*/

	s32 m = 0;
	s32 u4Temp;
	s32 Hprime;
	s32 D;
	s32   i4UxR;
	s32 u4Ax_Initial;
	s32 u4Ax_Increment;


	/* u4SrcWidthY = u4SrcWidth;
	u4SrcWidthCb = u4SrcWidth >> 1;
	u4TgtWidthY = u4TgtWidth;
	u4TgtWidthCb = u4TgtWidth >> 1;*/

	u4Temp = i4SrcWidth;

	while (u4Temp > 0) {
		m = m + 1;
		u4Temp = u4Temp >> 1;
	}

	/* check for case when u4SrcWidth is power of two */
	if (i4SrcWidth == (1 << (m - 1))) {
		m = m - 1;
	}

	Hprime = 1 << m;
	D = (64 * Hprime) / 16;


	/* iUxL and iUxR are independent of row, so compute once only */
	i4UxR = ((((i4SrcWidth - i4TgtWidth) << 1)) << (4 + m)); /* numerator part */
	/* complete iUxR init by dividing by H with rounding to nearest integer, */
	/* half-integers away from 0 */
	//ASSERT(i4TgtWidth);

	if (i4UxR >= 0) {
		i4UxR = (i4UxR + (i4TgtWidth >> 1)) / i4TgtWidth;
	} else {
		i4UxR = (i4UxR - (i4TgtWidth >> 1)) / i4TgtWidth;
	}



	/* initial x displacement and the x increment are independent of row */
	/* so compute once only */
	u4Ax_Initial = i4UxR + (D >> 1);
	u4Ax_Increment = (Hprime << 6) + (i4UxR << 1);


	u4Factor = u4Ax_Initial << (18 - (m + 6));
	u4Offset = u4Ax_Increment << (18 - (m + 6));

	i4HwImgReszSetH8TapsScale_Y(u4HwId, u4Factor, u4Offset);
	i4HwImgReszSetH8TapsScale_Cb(u4HwId, u4Factor, u4Offset);
	i4HwImgReszSetH8tapsCoefficients(u4HwId, FACTOR_RM);
	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetRPRVScaleFactor(u32 u4HwId, s32 i4SrcHeight, s32 i4TgtHeight)
{
	u32 u4Factor, u4Offset;
	/*u32 u4SrcHeightY = 0,u4SrcHeightCb = 0,u4SrcHeightCr = 0;*/
	/*u32 u4TgtHeightY = 0,u4TgtHeightCb = 0,u4TgtHeightCr = 0;*/

	s32 n = 0;
	s32 u4Temp;
	s32   i4UyLB;
	s32   i4UyL_inc;

	/* u4SrcHeightY = u4SrcHeight;*/
	/*u4SrcHeightCb = u4SrcHeight >> 1;*/
	/*u4TgtHeightY = u4TgtHeight;*/
	/*u4TgtHeightCb = u4TgtHeight >> 1;*/

	u4Temp = i4SrcHeight;

	while (u4Temp > 0) {
		n = n + 1;
		u4Temp = u4Temp >> 1;
	}

	/* check for case when uInHeight is power of two */
	if (i4SrcHeight == (1 << (n - 1))) {
		n = n - 1;
	}


	i4UyLB = ((i4SrcHeight - i4TgtHeight) << (n + 5)); /* numerator */
	/* complete iUyLB by dividing by V with rounding to nearest integer, */
	/* half-integers away from 0 */
	//ASSERT(i4TgtHeight);

	if (i4UyLB >= 0) {
		i4UyLB = (i4UyLB + (i4TgtHeight >> 1)) / i4TgtHeight;
	} else {
		i4UyLB = (i4UyLB - (i4TgtHeight >> 1)) / i4TgtHeight;
	}

	i4UyL_inc = (i4UyLB << 1);

	u4Factor = ((1 << (6 + n)) + i4UyL_inc) << (18 - (n + 6));
	u4Offset = (i4UyLB + (1 << (1 + n))) << (18 - (n + 6));

	i4HwImgReszSetV4TapsScale_Y(u4HwId, u4Offset, u4Factor);
	i4HwImgReszSetV4TapsScale_Cb(u4HwId, u4Offset, u4Factor);
	i4HwImgReszSetV4tapsCoefficients(u4HwId, FACTOR_RM);
	return S_IMGRESZ_HAL_OK;
}

s32 i4HwImgReszSetScale1to1Factor(u32 u4HwId,
				    IMGRESZ_HAL_IMG_BUF_FORMAT_T *prSrcBufferFormat)

{
	/* Y*/
	i4HwImgReszSetH8TapsScale_Y(u4HwId, 0, 0x40000);
	i4HwImgReszSetV4TapsScale_Y(u4HwId, 0, 0x40000);
	i4HwImgReszSetHecScale_Y(u4HwId, 0, 0x800, TRUE);
	i4HwImgReszSetVScale_Y(u4HwId, 0, 0x0, TRUE);

	if ((prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER) ||
	    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER) ||
	    (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER)) {
		return S_IMGRESZ_HAL_OK;
	}

	/* Cb*/
	i4HwImgReszSetH8TapsScale_Cb(u4HwId, 0, 0x40000);
	i4HwImgReszSetV4TapsScale_Cb(u4HwId, 0, 0x40000);
	i4HwImgReszSetHecScale_Cb(u4HwId, 0, 0x800, TRUE);
	i4HwImgReszSetVScale_Cb(u4HwId, 0, 0x0, TRUE);

	if (prSrcBufferFormat->eBufferMainFormat == IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER) {
		return S_IMGRESZ_HAL_OK;
	}

	/* Cr*/
	i4HwImgReszSetH8TapsScale_Cr(u4HwId, 0, 0x40000);
	i4HwImgReszSetV4TapsScale_Cr(u4HwId, 0, 0x40000);
	i4HwImgReszSetHecScale_Cr(u4HwId, 0, 0x800, TRUE);
	i4HwImgReszSetVScale_Cr(u4HwId, 0, 0x0, TRUE);
	return S_IMGRESZ_HAL_OK;
}

/*////////////////////////////////////////////////////////////////////////////////*/
/**/
/*  Hardware status related function*/
/**/
/*////////////////////////////////////////////////////////////////////////////////*/
s32 i4HwImgReszGetHwStatus(u32 u4HwId, IMGRESZ_HAL_HW_STATUS_T *prHwStatus)
{
	prHwStatus->u4SrcCntY = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_SRC_CNT_Y);
	prHwStatus->u4SrcCntCb = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_SRC_CNT_CB);
	prHwStatus->u4SrcCntCr = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_SRC_CNT_CR);
	prHwStatus->u4VOffsetY = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_OFFSET_Y);
	prHwStatus->u4VOffsetCb = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_OFFSET_CB);
	prHwStatus->u4VOffsetCr = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_OFFSET_CR);
	prHwStatus->u4VNextCY = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_NEXT_C_Y);
	prHwStatus->u4VNextCCb = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_NEXT_C_CB);
	prHwStatus->u4VNextCCr = u4HwImgReszRead32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_NEXT_C_CR);

	return S_IMGRESZ_HAL_OK;
}


s32 i4HwImgReszSetHwStatus(u32 u4HwId, IMGRESZ_HAL_HW_STATUS_T *prHwStatus)
{
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_SRC_CNT_Y, prHwStatus->u4SrcCntY);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_SRC_CNT_CB, prHwStatus->u4SrcCntCb);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_SRC_CNT_CR, prHwStatus->u4SrcCntCr);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_OFFSET_Y, prHwStatus->u4VOffsetY);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_OFFSET_CB, prHwStatus->u4VOffsetCb);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_OFFSET_CR, prHwStatus->u4VOffsetCr);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_NEXT_C_Y, prHwStatus->u4VNextCY);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_NEXT_C_CB, prHwStatus->u4VNextCCb);
	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_V_NEXT_C_CR, prHwStatus->u4VNextCCr);

	vHwImgReszWrite32(u4HwId, RW_IMG_RESZ_HW_STATUS_CPU_ASSIGN, 1); /* Trigger write status*/

	return S_IMGRESZ_HAL_OK;
}

#define IMGRESZ_WAIT_COUNT  10000
void vHwImgReszWaitCountReset(u32 u4HwId)
{
	u32 u4RegVal = 0;

	u4RegVal = u4HwImgReszRead32(u4HwId, RO_IMG_RESZ_FSM_MONITOR_REG);

	if ((u4RegVal & WORK_COUNTER) > 0) {
		u32 i = 0;

		while ((u4HwImgReszRead32(u4HwId, RO_IMG_RESZ_FSM_MONITOR_REG) & WORK_COUNTER) > 0) {
			i++;

			if (i > IMGRESZ_WAIT_COUNT) {
				/* IMGR_LOG(IMGR_LOG_LVL_ERR, "Last scale time out \r\n");*/
				break;
			}
		}
	}
}





