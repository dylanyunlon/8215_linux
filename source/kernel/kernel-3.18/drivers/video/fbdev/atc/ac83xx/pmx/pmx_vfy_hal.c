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
#ifndef __ARM2__
#include <linux/mm.h>
#include <mach/pinmux.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_pinmux_table.h>
#include <media/atc/drv_av_d.h>
#include <media/atc/display_inc.h>
#include <media/atc/pmx_hal.h>
#include "x_stl_lib.h"
#include "x_debug.h"
#else
#include "x_types.h"
#include "ac83xx_gpio_pinmux.h"
#include "ac83xx_pinmux_table.h"
#include "x_ckgen_8317.h"
#include "drv_av_d.h"
#include "display_inc.h"
#include "pmx_hal.h"
#endif
#include "x_os.h"
#include "x_rtos.h"
#include "x_assert.h"
#include "x_util.h"
#include "x_printf.h"
#include "x_bim.h"

#include "pmx_vfy_drv.h"
#include "pmx_vfy_hal.h"
#include "tcon.h"
#include "log.h"

#if 0
#define WriteREG(arg, val) (*(volatile __u32*)(IO_BASE_VA + (arg)) = val)
#else
void WriteREG(__u32 arg, __u32 val)
{
	__u32 regaddr;

	regaddr = IO_BASE_VA + arg;
	*(volatile __u32 *)regaddr = val;
}
#endif
#define ReadREG(arg)       (*(volatile __u32 *)(IO_BASE_VA + (arg)))
#define WriteREGMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))

__s32 _i4VerifyRoutineBreakCnt = 0;

void vPmxVerifyHalSysInit(void)
{
	/*((unsigned long *)0xFD00005c)[0] |= (0xF<<6);*/
	if (_u4LCDType == LCD_TYPE_TTL) {
#ifdef __ARM2__
		GPIO_MultiFun_Set(PIN_32_DE, TTL_DE_SEL);

		GPIO_MultiFun_Set(PIN_194_HSYNC, TTL_SYNC_SEL);
		GPIO_MultiFun_Set(PIN_195_VSYNC, TTL_SYNC_SEL);

		GPIO_MultiFun_Set(PIN_159_VB0, TTL_8B_SEL);
		GPIO_MultiFun_Set(PIN_160_VB1, TTL_8B_SEL);
		GPIO_MultiFun_Set(PIN_168_VG0, TTL_8B_SEL);
		GPIO_MultiFun_Set(PIN_169_VG1, TTL_8B_SEL);
		GPIO_MultiFun_Set(PIN_186_VR0, TTL_8B_SEL);
		GPIO_MultiFun_Set(PIN_187_VR1, TTL_8B_SEL);

		GPIO_MultiFun_Set(PIN_161_VB2, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_163_VB3, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_164_VB4, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_165_VB5, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_166_VB6, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_167_VB7, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_170_VG2, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_171_VG3, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_172_VG4, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_173_VG5, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_174_VG6, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_175_VG7, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_188_VR2, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_189_VR3, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_190_VR4, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_191_VR5, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_192_VR6, TTL_6_8B_SEL);
		GPIO_MultiFun_Set(PIN_193_VR7, TTL_6_8B_SEL);
#else
		dwPmxHalSetPinctrl();
#endif
	}

	((unsigned long *)0xFD0000b4)[0] |= 0x030180E1; /*0x030180E1;*/
	((unsigned long *)0xFD0000d0)[0] |= 0x030180E1; /*0x030180E1;*/
	((unsigned long *)0xFD0000b0)[0] |= 1; /*LVDS Power Down*/
	((unsigned long *)0xFD0000cc)[0] |= 1; /*LVDS Reset*/


	vFpdTconInit();
}

void vPmxReOpenVOPLL(void)
{
	switch (_u4DispMode) {
	case RES_480P_800:
		/*800x480 panel init*/
		vPmxVerifyHalLoadSetting(_ArOutputInitSetting[0].pu4RegSetting + 16, 20);
		break;

	case RES_600P_800:
		/*800x600 panel init*/
		vPmxVerifyHalLoadSetting(_ArOutputInitSetting[1].pu4RegSetting + 16, 20);
		break;

	case RES_600P_1024:
		/*1024x600 panel init*/
		vPmxVerifyHalLoadSetting(_ArOutputInitSetting[2].pu4RegSetting + 16, 20);
		break;
	}
}

void vPmxVerifyHalReset(__u8 ucVdoId)
{
	__u32 u4VdoRegBase;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
	} else {
		u4VdoRegBase = 0x43100;
	}

	WriteREG((u4VdoRegBase + 0x3c), 0xff);
	WriteREG((u4VdoRegBase + 0x3c), 0x0);
}

void vPmxVerifyHalLoadSetting(__u32 pu4Array[], __u32 u4Size)
{
	__u32 u4Idx;
	__u32 u4RegAddr, u4RegVal;

	u4Size = u4Size / 2;

	for (u4Idx = 0; u4Idx < u4Size; u4Idx++) {
		u4RegAddr = pu4Array[u4Idx * 2];
		u4RegVal = pu4Array[u4Idx * 2 + 1];
		WriteREG(u4RegAddr, u4RegVal);
	}
}

void vPmxVerifyHalVdoPtr(__u8 ucVdoId, __u32 u4YBuf, __u32 u4CBuf)
{
	__u32 u4BankNo;
	__u32 u4VdoRegBase, u4RegVal;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
	} else {
		u4VdoRegBase = 0x43100;
	}

	u4BankNo = u4YBuf / (16 * 1024 * 1024);
	u4YBuf  %= (16 * 1024 * 1024);
	u4CBuf  %= (16 * 1024 * 1024);

	u4RegVal = (u4BankNo << 24) | (u4YBuf >> 2);
	WriteREG((u4VdoRegBase + 0x00), u4RegVal);
	WriteREG((u4VdoRegBase + 0x08), u4RegVal);
	WriteREG((u4VdoRegBase + 0x80), u4RegVal);
	WriteREG((u4VdoRegBase + 0x84), u4RegVal);
	WriteREG((u4VdoRegBase + 0xEC), u4RegVal);
	u4RegVal = u4CBuf >> 2;
	WriteREG((u4VdoRegBase + 0x04), u4RegVal);
	WriteREG((u4VdoRegBase + 0x0C), u4RegVal);
	WriteREG((u4VdoRegBase + 0xFC), u4RegVal);
}

void vPmxVerifyHalWrChVdoPtr(__u8 ucVdoId, __u32 u4YBuf, __u32 u4CBuf)
{
	__u32 u4VdoRegBase;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42300;
	} else {
		u4VdoRegBase = 0x43300;
	}

	WriteREG((u4VdoRegBase + 0x08), (u4YBuf >> 3));
	WriteREG((u4VdoRegBase + 0x10), (u4CBuf >> 3));
}

void vPmxVerifyHalWrChOn(__u8 ucVdoId, __u8 ucOn)
{
	__u32 u4VdoRegBase;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42300;
	} else {
		u4VdoRegBase = 0x43300;
	}

	WriteREGMsk((u4VdoRegBase + 0x0), ucOn, (1 << 0));
}

void vPmxVerifyHalInterlaceExtra(__u8 ucVdoId, __u8 ucPmxMode)
{
	__u32 u4DispfmtRegBase, u4VdoRegBase;

	if (ucVdoId == 0) {
		u4DispfmtRegBase = 0x42000;
		u4VdoRegBase = 0x42100;
	} else {
		u4DispfmtRegBase = 0x43000;
		u4VdoRegBase = 0x43100;
	}

	WriteREG((u4VdoRegBase + 0x20), 0x00010000);
	WriteREG((u4VdoRegBase + 0x24), 0x03ff03fe);
	WriteREG((u4VdoRegBase + 0x28), 0x80800000);
	WriteREG((u4VdoRegBase + 0x2c), 0x2060a0e0);
	WriteREG((u4VdoRegBase + 0x50), 0x03ff03ff);
	WriteREG((u4VdoRegBase + 0x54), 0x03ff03ff);
	WriteREGMsk((u4VdoRegBase + 0x1c), 0x00002000, 0x00002000);
	WriteREGMsk((u4DispfmtRegBase + 0xac), 0, 0x08000000);

	/*temp scl*/
	if (ucPmxMode == RES_480I) {
		WriteREGMsk(0xa4690, 0x00000027, 0x000000ff);
	} else {
		/*WriteREGMsk(0xa4690, 0x0000001a, 0x000000ff); //800x480*/
		/*WriteREGMsk(0xa46e8, 0x00000442, 0x00000fff);*/
		WriteREGMsk(0xa4690, 0x00000025, 0x000000ff);   /*800x600*/
		WriteREGMsk(0xa46e8, 0x00000442, 0x00000fff);
	}
}

void vPmxVerifyHalVdoFit(__u8 ucVdoId, __u32 u4In, __u32 u4Out)
{
	__u32 u4HFactor = 0, u4VFactor = 0;
	__u32 u4VdoRegBase = 0, u4FmtRegBase = 0;

	switch (u4Out) {
	case RES_480P:
	case RES_480I:
		if (u4In == RES_480P) {
			u4HFactor = (__u32)(720 * 256 / 720);
			u4VFactor = (__u32)(480 * 128 / 480);
		} else if (u4In == RES_576P) {
			u4HFactor = (__u32)(720 * 256 / 720);
			u4VFactor = (__u32)(576 * 128 / 480);
		} else if (u4In == RES_1080P60HZ) {
			u4HFactor = (__u32)(1920 * 256 / 720);
			u4VFactor = (__u32)(1080 * 128 / 480);
		} else if (u4In == RES_720P60HZ) {
			u4HFactor = (__u32)(1280 * 256 / 720);
			u4VFactor = (__u32)(720 * 128 / 480);
		} else if (u4In == RES_480P_800) {
			u4HFactor = (__u32)(800 * 256 / 720);
			u4VFactor = (__u32)(480 * 128 / 480);
		} else if (u4In == RES_600P_800) {
			u4HFactor = (__u32)(800 * 256 / 720);
			u4VFactor = (__u32)(600 * 128 / 480);
		} else if (u4In == RES_600P_1024) {
			u4HFactor = (__u32)(1024 * 256 / 720);
			u4VFactor = (__u32)(600 * 128 / 480);
		}

		break;

	case RES_576P:
	case RES_576I:
		if (u4In == RES_480P) {
			u4HFactor = (__u32)(720 * 256 / 720);
			u4VFactor = (__u32)(480 * 128 / 576);
			u4VFactor = u4VFactor + 1;/*add@jgao ,OK 480->576 path*/
		} else if (u4In == RES_576P) {
			u4HFactor = (__u32)(720 * 256 / 720);
			u4VFactor = (__u32)(576 * 128 / 576);
		} else if (u4In == RES_1080P60HZ) {
			u4HFactor = (__u32)(1920 * 256 / 720);
			u4VFactor = (__u32)(1080 * 128 / 576);
		} else if (u4In == RES_720P60HZ) {
			u4HFactor = (__u32)(1280 * 256 / 720);
			u4VFactor = (__u32)(720 * 128 / 576);
		} else if (u4In == RES_480P_800) {
			u4HFactor = (__u32)(800 * 256 / 720);
			u4VFactor = (__u32)(480 * 128 / 576);
		} else if (u4In == RES_600P_800) {
			u4HFactor = (__u32)(800 * 256 / 720);
			u4VFactor = (__u32)(600 * 128 / 576);
		} else if (u4In == RES_600P_1024) {
			u4HFactor = (__u32)(1024 * 256 / 720);
			u4VFactor = (__u32)(600 * 128 / 576);
		}

		break;

	case RES_480P_800:
		if (u4In == RES_480P) {
			u4HFactor = (__u32)(720 * 256 / 800);
			u4VFactor = (__u32)(480 * 128 / 480);
		} else if (u4In == RES_576P) {
			u4HFactor = (__u32)(720 * 256 / 800);
			u4VFactor = (__u32)(576 * 128 / 480);
		} else if (u4In == RES_1080P60HZ) {
			u4HFactor = (__u32)(1920 * 256 / 800);
			u4VFactor = (__u32)(1080 * 128 / 480);
		} else if (u4In == RES_720P60HZ) {
			u4HFactor = (__u32)(1280 * 256 / 800);
			u4VFactor = (__u32)(720 * 128 / 480);
		} else if (u4In == RES_480P_800) {
			u4HFactor = (__u32)(800 * 256 / 800);
			u4VFactor = (__u32)(480 * 128 / 480);
		} else if (u4In == RES_600P_800) {
			u4HFactor = (__u32)(800 * 256 / 800);
			u4VFactor = (__u32)(600 * 128 / 480);
		} else if (u4In == RES_600P_1024) {
			u4HFactor = (__u32)(1024 * 256 / 800);
			u4VFactor = (__u32)(600 * 128 / 480);
		}

		break;

	case RES_600P_800:
		if (u4In == RES_480P) {
			u4HFactor = (__u32)(720 * 256 / 800);
			u4VFactor = (__u32)(480 * 128 / 600);
		} else if (u4In == RES_576P) {
			u4HFactor = (__u32)(720 * 256 / 800);
			u4VFactor = (__u32)(576 * 128 / 600);
		} else if (u4In == RES_1080P60HZ) {
			u4HFactor = (__u32)(1920 * 256 / 800);
			u4VFactor = (__u32)(1080 * 128 / 600);
		} else if (u4In == RES_720P60HZ) {
			u4HFactor = (__u32)(1280 * 256 / 800);
			u4VFactor = (__u32)(720 * 128 / 600);
		} else if (u4In == RES_480P_800) {
			u4HFactor = (__u32)(800 * 256 / 800);
			u4VFactor = (__u32)(480 * 128 / 600);
		} else if (u4In == RES_600P_800) {
			u4HFactor = (__u32)(800 * 256 / 800);
			u4VFactor = (__u32)(600 * 128 / 600);
		} else if (u4In == RES_600P_1024) {
			u4HFactor = (__u32)(1024 * 256 / 800);
			u4VFactor = (__u32)(600 * 128 / 600);
		}

		break;

	case RES_600P_1024:
		if (u4In == RES_480P) {
			u4HFactor = (__u32)(720 * 256 / 1024);
			u4VFactor = (__u32)(480 * 128 / 600);
		} else if (u4In == RES_576P) {
			u4HFactor = (__u32)(720 * 256 / 1024);
			u4VFactor = (__u32)(576 * 128 / 600);
		} else if (u4In == RES_1080P60HZ) {
			u4HFactor = (__u32)(1920 * 256 / 1024);
			u4VFactor = (__u32)(1080 * 128 / 600);
		} else if (u4In == RES_720P60HZ) {
			u4HFactor = (__u32)(1280 * 256 / 1024);
			u4VFactor = (__u32)(720 * 128 / 600);
		} else if (u4In == RES_480P_800) {
			u4HFactor = (__u32)(800 * 256 / 1024);
			u4VFactor = (__u32)(480 * 128 / 600);
		} else if (u4In == RES_600P_800) {
			u4HFactor = (__u32)(800 * 256 / 1024);
			u4VFactor = (__u32)(600 * 128 / 600);
		} else if (u4In == RES_600P_1024) {
			u4HFactor = (__u32)(1024 * 256 / 1024);
			u4VFactor = (__u32)(600 * 128 / 600);
		}

		break;

	case (RES_720P60HZ): {
		if (u4In == RES_720P60HZ) {
			u4HFactor = (__u32)(256);
			u4VFactor = (__u32)(128);
		}

		break;
	}

	case (RES_800P_1280): {
		if (u4In == RES_800P_1280) {
			u4HFactor = (__u32)(256);
			u4VFactor = (__u32)(128);
		}

		break;
	}

	default:
		break;
	}

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
		u4FmtRegBase = 0x42000;
	} else {
		u4VdoRegBase = 0x43100;
		u4FmtRegBase = 0x43000;
	}

	if (u4In == RES_480P) {
		WriteREG((u4VdoRegBase + 0x10), 0x01e0b45a);
		WriteREGMsk((u4VdoRegBase + 0xe0), 0xb4, 0xFFF);
		WriteREG((u4FmtRegBase + 0x9c), 0x000002d0);
	} else if (u4In == RES_576P) {
		WriteREG((u4VdoRegBase + 0x10), 0x0240b45a);
		WriteREGMsk((u4VdoRegBase + 0xe0), 0xb4, 0xFFF);
		WriteREG((u4FmtRegBase + 0x9c), 0x000002d0);
	} else if (u4In == RES_720P60HZ) {
		WriteREG((u4VdoRegBase + 0x10), 0x02d0b4a0);
		WriteREGMsk((u4VdoRegBase + 0xe0), 0x140, 0xFFF);
		WriteREG((u4FmtRegBase + 0x9c), 0x00000500);
	} else if (u4In == RES_1080P60HZ) {
		WriteREG((u4VdoRegBase + 0x10), 0x0438b4f0);
		WriteREGMsk((u4VdoRegBase + 0xe0), 0x1E0, 0xFFF);
		WriteREG((u4FmtRegBase + 0x9c), 0x00000780);
	} else if (u4In == RES_480P_800) {
		WriteREG((u4VdoRegBase + 0x10), 0x01e0c864);
		WriteREGMsk((u4VdoRegBase + 0xe0), 0xc8, 0xFFF);
		WriteREG((u4FmtRegBase + 0x9c), 0x00000320);
	} else if (u4In == RES_600P_800) {
		WriteREG((u4VdoRegBase + 0x10), 0x0258c864);
		WriteREGMsk((u4VdoRegBase + 0xe0), 0xc8, 0xFFF);
		WriteREG((u4FmtRegBase + 0x9c), 0x00000320);
	} else if (u4In == RES_600P_1024) {
		WriteREG((u4VdoRegBase + 0x10), 0x02580080);
		WriteREGMsk((u4VdoRegBase + 0xe0), 0x100, 0xFFF);
		WriteREG((u4FmtRegBase + 0x9c), 0x00000400);
	}

	if (u4HFactor > 0x200) {
		WriteREGMsk((u4FmtRegBase + 0xB0), (u4HFactor << 15), 0xFFFF0000);
		WriteREGMsk((u4VdoRegBase + 0x1C), (0x1 << 11), (0x1 << 11));
	} else {
		WriteREGMsk((u4FmtRegBase + 0xB0), (u4HFactor << 16), 0xFFFF0000);
		WriteREGMsk((u4VdoRegBase + 0x1C), 0, (0x1 << 11));
	}

	if ((ReadREG((u4FmtRegBase + 0x94)) & 0x8000) == 0x8000) {
		/*progress mode*/
		WriteREGMsk((u4VdoRegBase + 0x14), u4VFactor, 0xFFFFFFFF);
	} else { /*interlace mode*/
		WriteREGMsk((u4VdoRegBase + 0x14), u4VFactor << 1, 0xFFFFFFFF);
	}
}

void vPmxVerifyHalEnablePMX(__u8 ucVdoId, __u8 ucOn)
{
	__u32 u4DispFmtRegBase;

	if (ucVdoId == 0) {
		u4DispFmtRegBase = 0x42000;
	} else {
		u4DispFmtRegBase = 0x43000;
	}

	if (ucOn) {
		WriteREGMsk((u4DispFmtRegBase + 0xAC), 0x1, 0x1);
	} else {
		WriteREGMsk((u4DispFmtRegBase + 0xAC), 0x0, 0x1);
	}
}

void vPmxVerifyHalVdoXSkip(__u8 ucVdoId, __u32 u4XSkip)
{
	__u32 u4VdoRegBase;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
	} else {
		u4VdoRegBase = 0x43100;
	}

	WriteREGMsk((u4VdoRegBase + 0x14), (u4XSkip << 16), 0xFFFF0000);
}

void vPmxVerifyHalZoom(__u8 ucVdoId, __u32 u4SrcW, __u32 u4SrcH
	, __u32 u4DstW, __u32 u4DstH, __u32 u4ZoomW, __u32 u4ZoomH)
{
	__u32 u4HFactor = 0, u4VFactor = 0, u4PixLen = 0, u4DWNeed = 0;
	__u32 u4VdoRegBase = 0, u4FmtRegBase = 0;
	__u32 u4HFit = 0, u4VFit = 0;
	/*__u32 u4i = 0, u4Size = 0;*/
	UINT64 u8Tmp = 0;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
		u4FmtRegBase = 0x42000;
	} else {
		u4VdoRegBase = 0x43100;
		u4FmtRegBase = 0x43000;
	}

	switch (u4SrcW) {
	case 720:
		if (u4DstW == 720) {
			u4HFit = 10000;
		} else if (u4DstW == 800) {
			u4HFit = 9000;
		} else if (u4DstW == 1024) {
			u4HFit = 7031;
		}

		break;

	case 800:
		if (u4DstW == 720) {
			u4HFit = 11111;
		} else if (u4DstW == 800) {
			u4HFit = 10000;
		} else if (u4DstW == 1024) {
			u4HFit = 7812;
		}

		break;

	case 1024:
		if (u4DstW == 720) {
			u4HFit = 14222;
		} else if (u4DstW == 800) {
			u4HFit = 12800;
		} else if (u4DstW == 1024) {
			u4HFit = 10000;
		}

	default:
		break;
	}

	switch (u4SrcH) {
	case 480:
		if (u4DstH == 480) {
			u4VFit = 10000;
		} else if (u4DstH == 576) {
			u4VFit = 8333;
		} else if (u4DstH == 600) {
			u4VFit = 8000;
		}

		break;

	case 576:
		if (u4DstH == 480) {
			u4VFit = 12000;
		} else if (u4DstH == 576) {
			u4VFit = 10000;
		} else if (u4DstH == 600) {
			u4VFit = 9600;
		}

		break;

	case 600:
		if (u4DstH == 480) {
			u4VFit = 12500;
		} else if (u4DstH == 576) {
			u4VFit = 10416;
		} else if (u4DstH == 600) {
			u4VFit = 10000;
		}

		break;

	default:
		break;
	}

	switch (u4ZoomW) {
	case PMX_VFY_H_ZOOM_A2X:
		u4PixLen = (__u32)(u4SrcW * 10000 / 14140);
		u4HFactor = (__u32)(256 * u4HFit / 14140);
		break;

	case PMX_VFY_H_ZOOM_A3X:
		u4PixLen = (__u32)(u4SrcW * 10000 / 17320);
		u4HFactor = (__u32)(256 * u4HFit / 17320);
		break;

	case PMX_VFY_H_ZOOM_A4X:
		u4PixLen = (__u32)(u4SrcW / 2);
		u4HFactor = (__u32)(256 * u4HFit / (2 * 10000));
		break;

	case PMX_VFY_H_ZOOM_1X:
		u4PixLen = u4SrcW;
		u4HFactor = 256 * u4HFit / 10000;
		break;

	case PMX_VFY_H_ZOOM_2X:
		u4PixLen = (__u32)(u4SrcW / 2);
		u4HFactor = (__u32)(256 * u4HFit / (2 * 10000));
		break;

	case PMX_VFY_H_ZOOM_3X:
		u4PixLen = (__u32)(u4SrcW / 3);
		u4HFactor = (__u32)(256 * u4HFit / (3 * 10000));
		break;

	case PMX_VFY_H_ZOOM_4X:
		u4PixLen = (__u32)(u4SrcW / 4);
		u4HFactor = (__u32)(256 * u4HFit / (4 * 10000));
		break;

	case PMX_VFY_H_ZOOM_1D2X:
		u4PixLen = u4SrcW;
		u4HFactor = (__u32)(256 * u4HFit * 2 / 10000);
		break;

	case PMX_VFY_H_ZOOM_1D3X:
		u4PixLen = u4SrcW;
		u4HFactor = (__u32)(256 * u4HFit * 3 / 10000);
		break;

	case PMX_VFY_H_ZOOM_1D4X:
		u4PixLen = u4SrcW;
		u4HFactor = (__u32)(256 * u4HFit * 4 / 10000);
		break;

	case PMX_VFY_H_ZOOM_A1D2X:
		u4PixLen = u4SrcW;
		u8Tmp = (UINT64)u4HFit;
		u8Tmp = (256 * 14140 * u8Tmp) / (10000 * 10000);
		u4HFactor = (__u32)u8Tmp;
		break;

	case PMX_VFY_H_ZOOM_A1D3X:
		u4PixLen = u4SrcW;
		u8Tmp = (UINT64)u4HFit;
		u8Tmp = (256 * 17320 * u8Tmp) / (10000 * 10000);
		u4HFactor = (__u32)u8Tmp;
		break;

	case PMX_VFY_H_ZOOM_A1D4X:
		u4PixLen = u4SrcW;
		u4HFactor = (__u32)(256 * u4HFit * 2 / 10000);
		break;

	default:
		FB_PRINT(FB_LOG_LVL_DBG, "FMT", "[Pmx] H Zoom Type Invalid\n");
		break;
	}

	switch (u4ZoomH) {
	case PMX_VFY_V_ZOOM_A2X:
		u4VFactor = (__u32)(128 * u4VFit / 14140);
		break;

	case PMX_VFY_V_ZOOM_A3X:
		u4VFactor = (__u32)(128 * u4VFit / 17320);
		break;

	case PMX_VFY_V_ZOOM_A4X:
		u4VFactor = (__u32)(128 * u4VFit / (2 * 10000));
		break;

	case PMX_VFY_V_ZOOM_1X:
		u4VFactor = 128 * u4VFit / 10000;
		break;

	case PMX_VFY_V_ZOOM_2X:
		u4VFactor = (__u32)(128 * u4VFit / (2 * 10000));
		break;

	case PMX_VFY_V_ZOOM_3X:
		u4VFactor = (__u32)(128 * u4VFit / (3 * 10000));
		break;

	case PMX_VFY_V_ZOOM_4X:
		u4VFactor = (__u32)(128 * u4VFit / (4 * 10000));
		break;

	case PMX_VFY_V_ZOOM_1D2X:
		u4VFactor = (__u32)(128 * u4VFit * 2 / 10000);
		break;

	case PMX_VFY_V_ZOOM_1D3X:
		u4VFactor = (__u32)(128 * u4VFit * 3 / 10000);
		break;

	case PMX_VFY_V_ZOOM_1D4X:
		u4VFactor = (__u32)(128 * u4VFit * 4 / 10000);
		break;

	case PMX_VFY_V_ZOOM_A1D2X:
		u8Tmp = (UINT64)u4VFit;
		u8Tmp = (128 * 14140 * u8Tmp) / (10000 * 10000);
		u4VFactor = (__u32)u8Tmp;
		break;

	case PMX_VFY_V_ZOOM_A1D3X:
		u8Tmp = (UINT64)u4VFit;
		u8Tmp = (128 * 17320 * u8Tmp) / (10000 * 10000);
		u4VFactor = (__u32)u8Tmp;
		break;

	case PMX_VFY_V_ZOOM_A1D4X:
		u4VFactor = (__u32)(128 * u4VFit * 2 / 10000);
		break;

	default:
		FB_PRINT(FB_LOG_LVL_DBG, "FMT", "[Pmx] V Zoom Type Invalid\n");
		break;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "FMT", "[Pmx] H Factor: 0x%x, V Factor: 0x%x\r\n", (unsigned int)u4HFactor
		, (unsigned int)u4VFactor);

	if (u4PixLen & 0xF) { /*u4PixLen must be multiple of 16*/
		u4PixLen = ((u4PixLen >> 4) + 1) << 4;
	}

	u4DWNeed = u4PixLen >> 2;

	WriteREGMsk((u4VdoRegBase + 0xe0), (u4DWNeed), 0x1FF);
	WriteREGMsk((u4VdoRegBase + 0x10), (u4DWNeed << 8), 0xFF00);
	WriteREG((u4FmtRegBase + 0x9C), u4PixLen);

	if (u4HFactor > 0x200) {
		WriteREGMsk((u4FmtRegBase + 0xB0), (u4HFactor << 15), 0x3FFF0000);
		WriteREGMsk((u4VdoRegBase + 0x1C), (0x1 << 11), (0x1 << 11));
	} else {
		WriteREGMsk((u4FmtRegBase + 0xB0), (u4HFactor << 16), 0x3FFF0000);
		WriteREGMsk((u4VdoRegBase + 0x1C), 0, (0x1 << 11));
	}

	if ((ReadREG((u4FmtRegBase + 0x94)) & 0x8000) == 0x8000) {
		/*progress mode*/
		WriteREGMsk((u4VdoRegBase + 0x14), u4VFactor, 0xFFFFFFFF);
	} else { /*interlace mode*/
		WriteREGMsk((u4VdoRegBase + 0x14), u4VFactor << 1, 0xFFFFFFFF);
	}

	WriteREGMsk((u4VdoRegBase + 0x1C), 0x3, 0x3);
}

void vPmxVerifyHalDeIntMode(__u8 ucVdoId, __u8 ucMode)
{
	__u32 u4VdoRegBase;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
	} else {
		u4VdoRegBase = 0x43100;
	}

	switch (ucMode) {
	case PMX_VFY_VDO_FLD:
		WriteREGMsk((u4VdoRegBase + 0x30), 0, 0x3);
		break;

	case PMX_VFY_VDO_FRM:
		WriteREGMsk((u4VdoRegBase + 0x30), 3, 0x3);
		break;

	case PMX_VFY_VDO_4MA:
		WriteREGMsk((u4VdoRegBase + 0x88), (1 << 24), (1 << 24));
		break;

	case PMX_VFY_VDO_REPEAT:
		WriteREGMsk((u4VdoRegBase + 0x30), 0, 0x3);
		WriteREGMsk((u4VdoRegBase + 0x1C), (3 << 6), (3 << 6));
		break;
	}
}

__u32 _u4PmxVfyOsdRegBase[5] = {0x20100, 0x20200, 0x20300, 0x20a00, 0x20b00};

void vPmxVerifyHalOSDMixRatio(__u8 ucOsdId, __u32 u4MixRatio)
{
	__u32 u4RegBase;
	__u32 u4RegVal;

	if ((ucOsdId > 5) || (ucOsdId < 1)) {
		return; /*error osd id*/
	}

	u4RegBase = _u4PmxVfyOsdRegBase[ucOsdId - 1];
	u4RegVal = ReadREG(u4RegBase + 0x08) & (~0xFF000000);
	u4RegVal |= ((u4MixRatio & 0xFF) << 24);
	WriteREG(u4RegBase + 0x08, u4RegVal);
}

void vPmxVerifyHalSrcFmt(__u8 ucVdoId, __u8 ucSrcFmt)
{
	__u32 u4VdoRegBase;
	__u32 u4Reg424E0, u4Reg42430;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
	} else if (ucVdoId == 1) {
		u4VdoRegBase = 0x43100;
	} else {
		return; /*error*/
	}

	u4Reg42430 = ReadREG(u4VdoRegBase + 0x30) & (~0x00200000);
	u4Reg424E0 = ReadREG(u4VdoRegBase + 0xE0) & (~0x00800000);

	switch (ucSrcFmt) {
	case PMX_VFY_VDO_SRC_420_MB: /*YCbCr 420 MB*/
		u4Reg42430 |= 0x0;
		u4Reg424E0 |= 0x0;
		break;

	case PMX_VFY_VDO_SRC_422_MB: /*YCbCr 422 MB*/
		u4Reg42430 |= 0x00200000;
		u4Reg424E0 |= 0x0;
		break;

	case PMX_VFY_VDO_SRC_420_SL: /*YCbCr 420 scan-line*/
		u4Reg42430 |= 0x0;
		u4Reg424E0 |= 0x00800000;
		break;

	case PMX_VFY_VDO_SRC_422_SL: /*YCbCr 422 scan-line*/
		u4Reg42430 |= 0x00200000;
		u4Reg424E0 |= 0x00800000;
		break;

	default:
		break;
	}

	WriteREG(u4VdoRegBase + 0x30, u4Reg42430);
	WriteREG(u4VdoRegBase + 0xe0, u4Reg424E0);
}

void vPmxVerifyHalLumaKey(__u8 ucVdoId, __u8 ucOn, __u32 u4LumaKeyVal)
{
	__u32 u4VdoRegBase, u4DispfmtRegBase, u4VdoutfmtRegBase;
	__u32 u4Reg;

	u4VdoutfmtRegBase = 0x3000;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
		u4DispfmtRegBase = 0x42000;
	} else if (ucVdoId == 1) {
		u4VdoRegBase = 0x43100;
		u4DispfmtRegBase = 0x43000;
	} else {
		return; /*error*/
	}

	u4Reg = ReadREG(u4VdoutfmtRegBase + 0xFC) & (~0xFF040000);
	u4Reg |= (ucOn << 18);
	u4Reg |= (u4LumaKeyVal << 24);
	WriteREG((u4VdoutfmtRegBase + 0xFC), u4Reg);

	u4Reg = (u4LumaKeyVal << 4);
	WriteREG((u4DispfmtRegBase + 0xFC), u4Reg);

	u4Reg = ReadREG(u4DispfmtRegBase + 0xE4) & (~0x00000008);

	if (ucOn) {
		u4Reg |= 0x00000008;
	}

	WriteREG((u4DispfmtRegBase + 0xE4), u4Reg);

	u4Reg = ReadREG(u4VdoRegBase + 0xC8) & (~0x020000FF);
	u4Reg |= (ucOn << 25);
	u4Reg |= (u4LumaKeyVal);
	WriteREG((u4VdoRegBase + 0xC8), u4Reg);
}

void vPmxVerifyHalShiftLine(__u8 ucVdoId, __u8 ucHD, __u8 ucLine)
{
	__u32 u4VdoRegBase = 0;

	if (ucVdoId == 0) {
		u4VdoRegBase = 0x42100;
	} else if (ucVdoId == 1) {
		u4VdoRegBase = 0x43100;
	}

	if (ucHD) {
		switch (ucLine) {
		case 149:
			WriteREG((u4VdoRegBase + 0x20), 0x008F008E);
			WriteREG((u4VdoRegBase + 0x24), 0x00450046);
			WriteREG((u4VdoRegBase + 0x28), 0xC0004080);
			WriteREG((u4VdoRegBase + 0x2C), 0x40A0C020);
			WriteREG((u4VdoRegBase + 0x50), 0x008E008F);
			WriteREG((u4VdoRegBase + 0x54), 0x00460045);
			WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
			break;

		case 150:
			WriteREG((u4VdoRegBase + 0x20), 0x008F0090);
			WriteREG((u4VdoRegBase + 0x24), 0x00490048);
			WriteREG((u4VdoRegBase + 0x28), 0x4080C000);
			WriteREG((u4VdoRegBase + 0x2C), 0x80E00060);
			WriteREG((u4VdoRegBase + 0x50), 0x0090008F);
			WriteREG((u4VdoRegBase + 0x54), 0x00480047);
			WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
			break;

		case 151:
			WriteREG((u4VdoRegBase + 0x20), 0x00910090);
			WriteREG((u4VdoRegBase + 0x24), 0x00490048);
			WriteREG((u4VdoRegBase + 0x28), 0xC0004080);
			WriteREG((u4VdoRegBase + 0x2C), 0xC02040A0);
			WriteREG((u4VdoRegBase + 0x50), 0x00900091);
			WriteREG((u4VdoRegBase + 0x54), 0x00480049);
			WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
			break;

		case 152:
			WriteREG((u4VdoRegBase + 0x20), 0x00910092);
			WriteREG((u4VdoRegBase + 0x24), 0x00490048);
			WriteREG((u4VdoRegBase + 0x28), 0x4080C000);
			WriteREG((u4VdoRegBase + 0x2C), 0xC06080E0);
			WriteREG((u4VdoRegBase + 0x50), 0x00920091);
			WriteREG((u4VdoRegBase + 0x54), 0x004A0049);
			WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
			break;

		default:
			break;
		}
	} else {
		switch (ucLine) {
		case 149:
			WriteREG((u4VdoRegBase + 0x20), 0x00910090);
			WriteREG((u4VdoRegBase + 0x24), 0x00490048);
			WriteREG((u4VdoRegBase + 0x28), 0xA9602A80);
			WriteREG((u4VdoRegBase + 0x2C), 0xB52035A0);
			WriteREG((u4VdoRegBase + 0x50), 0x00900091);
			WriteREG((u4VdoRegBase + 0x54), 0x00480049);
			WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
			break;

		case 150:
			WriteREG((u4VdoRegBase + 0x20), 0x00910092);
			WriteREG((u4VdoRegBase + 0x24), 0x00490048);
			WriteREG((u4VdoRegBase + 0x28), 0x2A80AA00);
			WriteREG((u4VdoRegBase + 0x2C), 0xF56075E0);
			WriteREG((u4VdoRegBase + 0x50), 0x00920091);
			WriteREG((u4VdoRegBase + 0x54), 0x00480049);
			WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
			break;

		case 151:
			WriteREG((u4VdoRegBase + 0x20), 0x00930092);
			WriteREG((u4VdoRegBase + 0x24), 0x00470048);
			WriteREG((u4VdoRegBase + 0x28), 0xAA002A80);
			WriteREG((u4VdoRegBase + 0x2C), 0x35A0B520);
			WriteREG((u4VdoRegBase + 0x50), 0x00920093);
			WriteREG((u4VdoRegBase + 0x54), 0x00480047);
			WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
			break;

		case 152:
			WriteREG((u4VdoRegBase + 0x20), 0x00930094);
			WriteREG((u4VdoRegBase + 0x24), 0x0049004A);
			WriteREG((u4VdoRegBase + 0x28), 0x2A80AA00);
			WriteREG((u4VdoRegBase + 0x2C), 0x75E0F560);
			WriteREG((u4VdoRegBase + 0x50), 0x00940093);
			WriteREG((u4VdoRegBase + 0x54), 0x004A0049);
			WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
			break;

		default:
			break;
		}
	}
}

void vPmxVerifyHalDispInitChkSum(__u8 ucVdoId)
{
	__u32 u4DispfmtRegBase;

	if (ucVdoId == 0) {
		u4DispfmtRegBase = 0x42000;
	} else if (ucVdoId == 1) {
		u4DispfmtRegBase = 0x43000;
	} else {
		return; /*error*/
	}

	WriteREG((u4DispfmtRegBase + 0x50), (0x1 << 1));
	WriteREG((u4DispfmtRegBase + 0x50), 0x0);
	WriteREG((u4DispfmtRegBase + 0x50), (0x1 << 0));
}

__u32 u4PmxVerifyHalDispGetChkSum(__u8 ucVdoId)
{
	__u32 u4DispfmtRegBase, u4ReturnVal;

	if (ucVdoId == 0) {
		u4DispfmtRegBase = 0x42000;
	} else if (ucVdoId == 1) {
		u4DispfmtRegBase = 0x43000;
	} else {
		return 0; /*error*/
	}

	u4ReturnVal = ReadREG(u4DispfmtRegBase + 0x50) >> 8;
	return u4ReturnVal;
}

void vPmxVerifyHalRoutineBreak(void)
{
	_i4VerifyRoutineBreakCnt++; /*please set ICE breakpoint here*/
}

void vPmxVerifyHalSetDataSource(__u8 ucOsdSel, __u8 ucFpdSel)
{
	WriteREGMsk(0x1f008, ucOsdSel, 1);
	WriteREGMsk(0x1f000, ucFpdSel, 1);
}



