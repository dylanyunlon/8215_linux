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
#include <linux/module.h>
#include <media/atc/drv_osd_if.h>
#include "x_debug.h"
#else
#include "assert.h"
#include "drv_osd_if.h"
#endif
#include "osd_hw.h"
/*#include "x_ckgen.h"*/
#define DEFINE_IS_LOG OSD_IsLog
#include "log.h"
/*#include "hw_scpos.h"*/
/*#include "scpos_reg.h"*/

/* tmp*/
#define bReadSCPOS(x)         0
/*#define bReadSCPOSMsk(x, y)   0*/


/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/

typedef struct _OSD_SCALER_INFO {
	__u32 u4Enable;
	__u32 u4SrcWidth;
	__u32 u4SrcHeight;
	__u32 u4DstWidth;
	__u32 u4DstHeight;
} OSD_SCALER_INFO;
__s32 OSD_SC_Scale_DAL(__u32 u4Scaler, __u32 u4Enable, __u32 u4SrcWidth,
		   __u32 u4SrcHeight, __u32 u4DstWidth, __u32 u4DstHeight);


/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

static OSD_SCALER_INFO _arScalerInfo[OSD_SCALER_MAX_NUM];


/*-----------------------------------------------------------------------------*/
/* Public functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief: when HStart(Vstart) change, call this function to restore scaler
 *  destination size which were truncated before
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
void OSD_SC_UpdateDstSize(__u32 u4Scaler)
{
	OSD_SCALER_INFO *prInfo = &_arScalerInfo[u4Scaler];

	IGNORE_RET(OSD_SC_Scale_DAL(u4Scaler, prInfo->u4Enable,
				prInfo->u4SrcWidth, prInfo->u4SrcHeight,
				prInfo->u4DstWidth, prInfo->u4DstHeight));
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_CheckCapability(__u32 u4SrcW, __u32 u4SrcH, __u32 u4DstW, __u32 u4DstH)
{
	if (u4DstH >= u4SrcH) {
		if (u4DstW >= u4SrcW) {
			return (__s32)OSD_RET_OK;
		} else if (u4DstW * 192 >= u4SrcW * 72) {
			return (__s32)OSD_RET_OK;
		}
	}

	if (u4DstW >= u4SrcW) {
		if (u4DstH * 108 >= u4SrcH * 48) {
			return (__s32)OSD_RET_OK;
		}
	}

	/*if ((u4DstW * 192 >= u4SrcW * 72) && (u4DstH * 108 >= u4SrcH * 48))*/
	if ((u4DstW * 192 >= u4SrcW * 72) && (u4DstH * 144 >= u4SrcH * 48)) {
		return (__s32)OSD_RET_OK;
	}

	return (__s32)OSD_RET_INV_ARG;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_Scale_DAL(__u32 u4Scaler, __u32 u4Enable, __u32 u4SrcWidth,
		   __u32 u4SrcHeight, __u32 u4DstWidth, __u32 u4DstHeight)
{
	__u32 u4Tmp, u4MaxWidth, u4MaxHeight, fgPrgs;


	OSD_VERIFY_SCALER(u4Scaler);

	IGNORE_RET(_OSD_BASE_GetScrnHSizeMain(&u4MaxWidth));
	IGNORE_RET(_OSD_BASE_GetScrnVSizeMain(&u4MaxHeight));

	/*IGNORE_RET(_OSD_BASE_GetOsd1Prgs(&fgPrgs));*/
	if ((u4MaxWidth == 0) || (u4MaxHeight) == 0) {
		u4MaxWidth = 0x780;
		u4MaxHeight = 0x438;
		IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(u4MaxWidth));
		IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(u4MaxHeight));
		FB_PRINT(FB_LOG_LVL_ERR, "OSD", "[osd_scaler_if]fastlogo didn't run first.\n");
	}

	ASSERT((u4MaxWidth > 0) && (u4MaxHeight > 0));

	/* protect if hstart+dst_w > max_w or vstart+dst_h > max_h*/
	switch (u4Scaler) {
	case (__u32)OSD_SCALER_1:
		IGNORE_RET(_OSD_BASE_GetOsd1Prgs(&fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd1HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd1VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_2:
		IGNORE_RET(_OSD_BASE_GetOsd2Prgs(&fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd2HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd2VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_3:
		IGNORE_RET(_OSD_BASE_GetOsd3Prgs(&fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd3HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd3VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_4:
		IGNORE_RET(_OSD_BASE_GetOsd4Prgs(&fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd4HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd4VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_5:
	case (__u32)OSD_SCALER_6:
		break;

	case (__u32)OSD_SCALER_7:
		IGNORE_RET(_OSD_R_BASE_GetOsd7Prgs(&fgPrgs));
		IGNORE_RET(_OSD_R_BASE_GetOsd7HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_R_BASE_GetOsd7VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_8:
		IGNORE_RET(_OSD_R_BASE_GetOsd8Prgs(&fgPrgs));
		IGNORE_RET(_OSD_R_BASE_GetOsd8HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_R_BASE_GetOsd8VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	default:
		ASSERT(0);
	}

	/* copy osd info*/
	_arScalerInfo[u4Scaler].u4Enable = u4Enable;
	_arScalerInfo[u4Scaler].u4SrcWidth = u4SrcWidth;
	_arScalerInfo[u4Scaler].u4SrcHeight = u4SrcHeight;
	_arScalerInfo[u4Scaler].u4DstWidth = u4DstWidth;
	_arScalerInfo[u4Scaler].u4DstHeight = u4DstHeight;

	if (u4SrcWidth == 0) {
		u4SrcWidth = u4MaxWidth;
	}

	if (u4DstWidth == 0) {
		u4DstWidth = u4MaxWidth;
	}

	if (u4SrcHeight == 0) {
		u4SrcHeight = (fgPrgs) ? u4MaxHeight : (u4MaxHeight << 1);
	}

	if (u4DstHeight == 0) {
		u4DstHeight = (fgPrgs) ? u4MaxHeight : (u4MaxHeight << 1);
	}

	/* modify for MW's API require*/
	IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, u4Enable));

	/* to cut non-necessary src input*/
	if (u4DstWidth > u4MaxWidth) {
		u4SrcWidth = (u4MaxWidth * u4SrcWidth) / u4DstWidth;
		u4DstWidth = u4MaxWidth;
		/* boundry condition protection*/
		u4SrcWidth = MAX(u4SrcWidth, 1);
		u4DstWidth = MAX(u4DstWidth, 2);
	}

	if (u4SrcWidth == u4DstWidth) {
		IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
		IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4SrcWidth));
		IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4SrcWidth));
		/* clear*/
		IGNORE_RET(_OSD_SC_SetHuscOfst(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHuscStep(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscOfst(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscStep(u4Scaler, 0));
	} else {
		if (u4SrcWidth < u4DstWidth) {
			/* horizontal scaling up*/
			IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 0));
			IGNORE_RET(_OSD_SC_SetHuscOfst(u4Scaler, 0));

			u4Tmp = ((u4SrcWidth - 1) << OSD_SC_STEP_BIT) / (u4DstWidth - 1);
			IGNORE_RET(_OSD_SC_SetHuscStep(u4Scaler, u4Tmp));

			IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth));
		} else {
#if 1
			/* horizontal scaling down*/
			IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 0));
			u4Tmp = ((u4DstWidth << OSD_SC_STEP_BIT) / u4SrcWidth) +
				(((u4DstWidth << OSD_SC_STEP_BIT) % u4SrcWidth) ? 1 : 0);
			IGNORE_RET(_OSD_SC_SetHdscOfst(u4Scaler, u4Tmp));
			IGNORE_RET(_OSD_SC_SetHdscStep(u4Scaler, u4Tmp));

			IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4DstWidth));
			IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth));
#else
			/* enable both scale up and scale down, for FPGA verification*/
			_OSD_SC_SetHdscEn(u4Scaler, 1);
			_OSD_SC_SetHuscEn(u4Scaler, 1);
			u4Tmp = ((u4DstWidth << (OSD_SC_STEP_BIT - 1)) / u4SrcWidth) +
				(((u4DstWidth << (OSD_SC_STEP_BIT - 1)) % u4SrcWidth) ? 1 : 0);
			_OSD_SC_SetHdscOfst(u4Scaler, u4Tmp);
			_OSD_SC_SetHdscStep(u4Scaler, u4Tmp);

			_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth);
			_OSD_SC_SetVscHSize(u4Scaler, u4DstWidth >> 1);

			_OSD_SC_SetHuscOfst(u4Scaler, 0);
			u4Tmp = (((u4DstWidth >> 1) - 1) << OSD_SC_STEP_BIT) /
				(u4DstWidth - 1);
			_OSD_SC_SetHuscStep(u4Scaler, u4Tmp);
			_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth);
#endif
		}
	}

	if (u4SrcHeight == u4DstHeight) {
		IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 0));

		if (fgPrgs) {
			if (u4DstHeight > u4MaxHeight) {
				u4DstHeight = u4MaxHeight;
			}

			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4DstHeight));
			IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));
		} else {
			u4DstHeight >>= 1;

			if (u4DstHeight > u4MaxHeight) {
				u4DstHeight = u4MaxHeight;
			}

			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4DstHeight));
			IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));
		}

		/* clear*/
		IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, 0));

		/* to choose osd clock as output clock*/
		/*OSD_BASE_SetClock(OSD_CK_OCLK);*/
		/* ???*/
		/*IGNORE_RET(OSD_BASE_SetClock(OSD_CK_SYS));*/
	} else {
		if (!fgPrgs) { /* interlaced mode*/
			u4DstHeight = u4DstHeight >> 1;
		}

		if (u4DstHeight > u4MaxHeight) {
			u4SrcHeight = (u4MaxHeight * u4SrcHeight) / u4DstHeight;
			u4DstHeight = u4MaxHeight;
			/* boundry condition protection*/
			u4SrcHeight = MAX(u4SrcHeight, 1);
			u4DstHeight = MAX(u4DstHeight, 2);

		}

		/* "=" only happen when src=2*dst in interlaced mode*/
		if (u4SrcHeight <= u4DstHeight) {
			/* vertical scaling up*/
			IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 0));
			IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 1));

			if (fgPrgs) {
				u4Tmp = ((u4SrcHeight - 1) << OSD_SC_STEP_BIT) /
					(u4DstHeight - 1);

				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, 0));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, 0));
			} else {
				u4Tmp = ((u4SrcHeight - 1) << OSD_SC_STEP_BIT) / (u4DstHeight);

				if ((u4Tmp % (1 << OSD_SC_STEP_BIT)) == 0) {
					u4Tmp--;
				}

				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Tmp >> 2));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, ((3 * u4Tmp) >> 2) &
								 0x3ffc));
			}

			IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, u4Tmp));
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4SrcHeight));

			if (fgPrgs) {
				IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));
			} else {
				IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight << 1));
			}

			/* to choose osd clock as output clock*/
			/*OSD_BASE_SetClock(OSD_CK_OCLK);*/
			/* ???*/
			/*IGNORE_RET(OSD_BASE_SetClock(OSD_CK_SYS));*/
		} else {
			if (OSD_SC_CheckCapability(u4SrcWidth, u4SrcHeight, u4DstWidth, u4DstHeight) !=
			    (__s32)OSD_RET_OK) {
				FB_PRINT(FB_LOG_LVL_ERR, "OSD", "OSD_SC_CheckCapability: fail\n");
			}

			/* vertical scaling down*/
			IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 0));

			if (fgPrgs) {
				u4Tmp = ((u4DstHeight << OSD_SC_STEP_BIT) / u4SrcHeight) +
					(((u4DstHeight << OSD_SC_STEP_BIT) % u4SrcHeight) ? 1 : 0);
				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Tmp));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4Tmp));
			} else {
				if (u4DstHeight < (u4SrcHeight >> 1)) {
					/*step<=0x2000*/
					u4Tmp = (((u4DstHeight + 1) << OSD_SC_STEP_BIT) /
						 u4SrcHeight) +
						((((u4DstHeight + 1) << OSD_SC_STEP_BIT) %
						  u4SrcHeight) ? 1 : 0);

					if (u4Tmp == (1 << (OSD_SC_STEP_BIT - 1))) {
						u4Tmp--;
					}

					IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4Tmp));
					IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Tmp +
									 (1 << (OSD_SC_STEP_BIT - 1))));
				} else {
					/*step>0x2000*/
					u4Tmp = (((u4DstHeight + 1) << OSD_SC_STEP_BIT) /
						 u4SrcHeight) +
						((((u4DstHeight + 1) << OSD_SC_STEP_BIT) %
						  u4SrcHeight) ? 1 : 0);

					if ((u4Tmp % (1 << OSD_SC_STEP_BIT)) == 0) {
						u4Tmp--;
					}

					IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4Tmp -
									 (1 << (OSD_SC_STEP_BIT - 1))));
					IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Tmp));
				}
			}

			IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, u4Tmp));
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4SrcHeight));

			if (fgPrgs) {
				IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));
			} else {
				IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight << 1));
			}

			/* to choose osd clock as syspll_d2_ck*/
			/* ???*/
			/*IGNORE_RET(OSD_BASE_SetClock(OSD_CK_SYS));*/
		}
	}

	IGNORE_RET(_OSD_SC_UpdateHwReg(u4Scaler));
	return (__s32)OSD_RET_OK;
}
EXPORT_SYMBOL(OSD_SC_Scale_DAL);

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_HDown(__u32 u4Scaler, __u32 u4SrcWidth, __u32 u4Step)
{
	__u32 u4DstWidth;

	OSD_VERIFY_SCALER(u4Scaler);

	IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, 1));
	IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 1));
	IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 0));
	IGNORE_RET(_OSD_SC_SetHdscOfst(u4Scaler, u4Step));
	IGNORE_RET(_OSD_SC_SetHdscStep(u4Scaler, u4Step));

	u4DstWidth = (u4SrcWidth * u4Step) >> OSD_SC_STEP_BIT;
	IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4DstWidth));
	IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth));

	IGNORE_RET(_OSD_SC_UpdateHwReg(u4Scaler));

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_HUp(__u32 u4Scaler, __u32 u4SrcWidth, __u32 u4Step)
{
	__u32 u4DstWidth, u4MaxWidth;

	OSD_VERIFY_SCALER(u4Scaler);

	IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, 1));
	IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 0));
	IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 1));
	IGNORE_RET(_OSD_SC_SetHuscOfst(u4Scaler, 0));
	IGNORE_RET(_OSD_SC_SetHuscStep(u4Scaler, u4Step));

	u4DstWidth = (((u4SrcWidth - 1) * OSD_SC_STEP_BASE) / u4Step) + 1;
	IGNORE_RET(_OSD_BASE_GetScrnHSizeMain(&u4MaxWidth));

	if (u4DstWidth > u4MaxWidth) {
		u4DstWidth = u4MaxWidth;
		u4SrcWidth = (((u4DstWidth - 1) * u4Step) >> OSD_SC_STEP_BIT) +
			     ((((u4DstWidth - 1) * u4Step) % OSD_SC_STEP_BASE) ? 1 : 0) + 1;
		IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4SrcWidth));
		IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
	} else {
		IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4SrcWidth));
	}

	IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth));

	IGNORE_RET(_OSD_SC_UpdateHwReg(u4Scaler));

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  only support progressive mode
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_VDown(__u32 u4Scaler, __u32 u4SrcHeight, __u32 u4Step)
{
	__u32 u4DstHeight = 0, fgPrgs;

	OSD_VERIFY_SCALER(u4Scaler);
	IGNORE_RET(OSD_BASE_GetPrgs(u4Scaler, &fgPrgs));

	IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, 1));
	IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 1));
	IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 0));

	if (fgPrgs) {
		IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Step));
		IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4Step));
		IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, u4Step));
		u4DstHeight = (u4SrcHeight * u4Step) >> OSD_SC_STEP_BIT;
	}

	IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));

	IGNORE_RET(_OSD_SC_UpdateHwReg(u4Scaler));

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  only support progressive mode
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_VUp(__u32 u4Scaler, __u32 u4SrcHeight, __u32 u4Step)
{
	__u32 u4DstHeight = 0, u4MaxHeight, fgPrgs;

	OSD_VERIFY_SCALER(u4Scaler);
	IGNORE_RET(OSD_BASE_GetPrgs(u4Scaler, &fgPrgs));

	IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, 1));
	IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 0));
	IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 1));

	if (fgPrgs) {
		IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, u4Step));

		u4DstHeight = (((u4SrcHeight - 1) * OSD_SC_STEP_BASE) / u4Step) + 1;
		IGNORE_RET(_OSD_BASE_GetScrnVSizeMain(&u4MaxHeight));

		if (u4DstHeight > u4MaxHeight) {
			u4DstHeight = u4MaxHeight;
			u4SrcHeight = (((u4DstHeight - 1) * u4Step) >> OSD_SC_STEP_BIT) +
				      ((((u4DstHeight - 1) * u4Step) % OSD_SC_STEP_BASE) ?
				       1 : 0) + 1;
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4SrcHeight));
		}
	}

	IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));

	IGNORE_RET(_OSD_SC_UpdateHwReg(u4Scaler));

	return (__s32)OSD_RET_OK;
}



/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_GetScalerInfo(__u32 u4Scaler, __u32 *pu4Enable,
			   __u32 *pu4SrcW, __u32 *pu4SrcH,
			   __u32 *pu4DstW, __u32 *pu4DstH,
			   __u32 *pu4Is16Bpp)
{
	OSD_VERIFY_SCALER(u4Scaler);

	IGNORE_RET(_OSD_SC_GetScEn(u4Scaler, pu4Enable));
	IGNORE_RET(_OSD_SC_GetSrcHSize(u4Scaler, pu4SrcW));
	IGNORE_RET(_OSD_SC_GetSrcVSize(u4Scaler, pu4SrcH));
	IGNORE_RET(_OSD_SC_GetDstHSize(u4Scaler, pu4DstW));
	IGNORE_RET(_OSD_SC_GetDstVSize(u4Scaler, pu4DstH));

	*pu4Is16Bpp = 0;
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_SetLpfInfo(__u32 u4Scaler, __u32 u4Enable, INT16 i2C1,
			INT16 i2C2, INT16 i2C3, INT16 i2C4, INT16 i2C5)
{
	__u32 tmp;

	OSD_VERIFY_SCALER(u4Scaler);
	IGNORE_RET(_OSD_SC_SetScLpfEn(u4Scaler, u4Enable));

	/*#ifdef CC_MT5381*/

	OSD_SET_LPF_SIGN_NUM(tmp, 32, i2C3);
	IGNORE_RET(_OSD_SC_SetScLpfC3(u4Scaler, tmp));

	OSD_SET_LPF_SIGN_NUM(tmp, 64, i2C4);
	IGNORE_RET(_OSD_SC_SetScLpfC4(u4Scaler, tmp));

	if ((i2C5 > 127) || (i2C5 < 0)) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	tmp = (UINT16)i2C5;
	IGNORE_RET(_OSD_SC_SetScLpfC5(u4Scaler, tmp));

	if (u4Enable) {
		IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, TRUE));
	}

	IGNORE_RET(_OSD_SC_UpdateHwReg(u4Scaler));

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_SetLpf(__u32 u4Scaler, __u32 u4Enable)
{
	return OSD_SC_SetLpfInfo(u4Scaler, u4Enable, OSD_DEFAULT_LPF_C1,
				 OSD_DEFAULT_LPF_C2, OSD_DEFAULT_LPF_C3,
				 OSD_DEFAULT_LPF_C4, OSD_DEFAULT_LPF_C5);
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_GetLpfInfo(__u32 u4Scaler, __u32 *pu4Enable, INT16 *pi2C1,
			INT16 *pi2C2, INT16 *pi2C3, INT16 *pi2C4, INT16 *pi2C5)
{
	__u32 tmp = 0;

	OSD_CHECK_NULL(pu4Enable);
	OSD_CHECK_NULL(pi2C1);
	OSD_CHECK_NULL(pi2C2);
	OSD_CHECK_NULL(pi2C3);
	OSD_CHECK_NULL(pi2C4);
	OSD_CHECK_NULL(pi2C5);

	OSD_VERIFY_SCALER(u4Scaler);
	IGNORE_RET(_OSD_SC_GetScLpfEn(u4Scaler, pu4Enable));


	IGNORE_RET(_OSD_SC_GetScLpfC3(u4Scaler, &tmp));
	OSD_GET_LPF_SIGN_NUM(tmp, 32, *pi2C3);

	IGNORE_RET(_OSD_SC_GetScLpfC4(u4Scaler, &tmp));
	OSD_GET_LPF_SIGN_NUM(tmp, 64, *pi2C4);

	IGNORE_RET(_OSD_SC_GetScLpfC5(u4Scaler, &tmp));
	*pi2C5 = (INT16)tmp;

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_SetFormat16Bpp(__u32 u4Scaler)
{
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_SetFormat32Bpp(__u32 u4Scaler)
{

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_SetSramConfiguration1(__u32 u4Mode)
{

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_GetSramConfiguration1(void)
{
	__u32 u4Mode = 0;

	return (__s32)u4Mode;
}

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_SetSramConfiguration2(__u32 u4Mode)
{

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_GetSramConfiguration2(void)
{
	__u32 u4Mode = 0;

	return (__s32)u4Mode;
}

#if (CONFIG_DRV_VERIFY_SUPPORT == 1)

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_SC_Interlace_Scale(__u32 u4Scaler, __u32 u4Enable, __u32 u4SrcWidth,
			     __u32 u4SrcHeight, __u32 u4DstWidth, __u32 u4DstHeight)
{
	__u32 u4Tmp, u4MaxWidth, u4MaxHeight, fgPrgs;


	OSD_VERIFY_SCALER(u4Scaler);

	IGNORE_RET(_OSD_BASE_GetScrnHSizeMain(&u4MaxWidth));
	IGNORE_RET(_OSD_BASE_GetScrnVSizeMain(&u4MaxHeight));
	/*IGNORE_RET(_OSD_BASE_GetOsd1Prgs(&fgPrgs));*/
	ASSERT((u4MaxWidth > 0) && (u4MaxHeight > 0));

	/* protect if hstart+dst_w > max_w or vstart+dst_h > max_h*/
	switch (u4Scaler) {
	case (__u32)OSD_SCALER_1:
		IGNORE_RET(_OSD_BASE_GetOsd1Prgs(&fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd1HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd1VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_2:
		IGNORE_RET(_OSD_BASE_GetOsd2Prgs(&fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd2HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd2VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_3:
		IGNORE_RET(_OSD_BASE_GetOsd3Prgs(&fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd3HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd3VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_4:
		IGNORE_RET(_OSD_BASE_GetOsd4Prgs(&fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd4HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd4VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_5:
	case (__u32)OSD_SCALER_6:
		break;

	case (__u32)OSD_SCALER_7:
		IGNORE_RET(_OSD_R_BASE_GetOsd7Prgs(&fgPrgs));
		IGNORE_RET(_OSD_R_BASE_GetOsd7HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_R_BASE_GetOsd7VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	case (__u32)OSD_SCALER_8:
		IGNORE_RET(_OSD_R_BASE_GetOsd8Prgs(&fgPrgs));
		IGNORE_RET(_OSD_R_BASE_GetOsd8HStart(&u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_R_BASE_GetOsd8VStart(&u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;

	default:
		ASSERT(0);
	}

	/* copy osd info*/
	_arScalerInfo[u4Scaler].u4Enable = u4Enable;
	_arScalerInfo[u4Scaler].u4SrcWidth = u4SrcWidth;
	_arScalerInfo[u4Scaler].u4SrcHeight = u4SrcHeight;
	_arScalerInfo[u4Scaler].u4DstWidth = u4DstWidth;
	_arScalerInfo[u4Scaler].u4DstHeight = u4DstHeight;


	if (u4Enable) {         /*add*/
		u4SrcWidth = u4SrcWidth * 2; /**/
	} else {                /**/
		u4SrcWidth = u4SrcWidth; /**/
	}

	if (u4SrcWidth == 0) {
		u4SrcWidth = u4MaxWidth;
	}

	if (u4DstWidth == 0) {
		u4DstWidth = u4MaxWidth;
	}

	if (u4SrcHeight == 0) {
		u4SrcHeight = (fgPrgs) ? u4MaxHeight : (u4MaxHeight << 1);
	}

	if (u4DstHeight == 0) {
		u4DstHeight = (fgPrgs) ? u4MaxHeight : (u4MaxHeight << 1);
	}

	/* modify for MW's API require*/
	IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, u4Enable));

	/* to cut non-necessary src input*/
	if (u4DstWidth > u4MaxWidth) {
		u4SrcWidth = (u4MaxWidth * u4SrcWidth) / u4DstWidth;
		u4DstWidth = u4MaxWidth;
		/* boundry condition protection*/
		u4SrcWidth = MAX(u4SrcWidth, 1);
		u4DstWidth = MAX(u4DstWidth, 2);
	}

	if (u4SrcWidth == u4DstWidth) {
		IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
		IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4SrcWidth));
		IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4SrcWidth));
		/* clear*/
		IGNORE_RET(_OSD_SC_SetHuscOfst(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHuscStep(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscOfst(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscStep(u4Scaler, 0));
	} else {
		if (u4SrcWidth < u4DstWidth) {
			/* horizontal scaling up*/
			IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 0));
			IGNORE_RET(_OSD_SC_SetHuscOfst(u4Scaler, 0));

			u4Tmp = ((u4SrcWidth - 1) << OSD_SC_STEP_BIT) / (u4DstWidth - 1);
			IGNORE_RET(_OSD_SC_SetHuscStep(u4Scaler, u4Tmp));

			IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth));

		} else {
			/* horizontal scaling down*/
			IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 0));
			u4Tmp = ((u4DstWidth << OSD_SC_STEP_BIT) / u4SrcWidth) +
				(((u4DstWidth << OSD_SC_STEP_BIT) % u4SrcWidth) ? 1 : 0);
			IGNORE_RET(_OSD_SC_SetHdscOfst(u4Scaler, u4Tmp));
			IGNORE_RET(_OSD_SC_SetHdscStep(u4Scaler, u4Tmp));

			IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4DstWidth));
			IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth));
		}
	}

	{
		if (!fgPrgs) { /* interlaced mode*/
			u4DstHeight = u4DstHeight >> 1;
		}

		if (u4DstHeight > u4MaxHeight) {
			u4SrcHeight = (u4MaxHeight * u4SrcHeight) / u4DstHeight;
			u4DstHeight = u4MaxHeight;
			/* boundry condition protection*/
			u4SrcHeight = MAX(u4SrcHeight, 1);
			u4DstHeight = MAX(u4DstHeight, 2);

		}

		/* "=" only happen when src=2*dst in interlaced mode*/
		if (u4SrcHeight <= u4DstHeight) {
			/* vertical scaling up*/
			IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 0));
			IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 1));

			if (fgPrgs) {
				u4Tmp = ((u4SrcHeight - 1) << OSD_SC_STEP_BIT) /
					(u4DstHeight - 1);

				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, 0));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, 0));
			} else {
				u4Tmp = ((u4SrcHeight - 1) << OSD_SC_STEP_BIT) / (u4DstHeight);

				if ((u4Tmp % (1 << OSD_SC_STEP_BIT)) == 0) {
					u4Tmp--;
				}

				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Tmp >> 2));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, ((3 * u4Tmp) >> 2) &
								 0x3ffc));
			}

			IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, u4Tmp));
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4SrcHeight));

			if (fgPrgs) {
				IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));
			} else {
				IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight << 1));
			}

			/* to choose osd clock as output clock*/
			/*OSD_BASE_SetClock(OSD_CK_OCLK);*/
			/* ???*/
			/*IGNORE_RET(OSD_BASE_SetClock(OSD_CK_SYS));*/
		} else {
			if (OSD_SC_CheckCapability(u4SrcWidth, u4SrcHeight, u4DstWidth, u4DstHeight) !=
			    (__s32)OSD_RET_OK) {
				FB_PRINT(FB_LOG_LVL_ERR, "OSD", "OSD_SC_CheckCapability: fail\n");
			}

			/* vertical scaling down*/
			IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 0));


			if (fgPrgs) {
				u4Tmp = ((u4DstHeight << OSD_SC_STEP_BIT) / u4SrcHeight) +
					(((u4DstHeight << OSD_SC_STEP_BIT) % u4SrcHeight) ? 1 : 0);
				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Tmp));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4Tmp));
			} else {
				if (u4DstHeight < (u4SrcHeight >> 1)) {
					/*step<=0x2000*/
					u4Tmp = (((u4DstHeight + 1) << OSD_SC_STEP_BIT) /
						 u4SrcHeight) +
						((((u4DstHeight + 1) << OSD_SC_STEP_BIT) %
						  u4SrcHeight) ? 1 : 0);

					if (u4Tmp == (1 << (OSD_SC_STEP_BIT - 1))) {
						u4Tmp--;
					}

					IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4Tmp));
					IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Tmp +
									 (1 << (OSD_SC_STEP_BIT - 1))));
				} else {
					/*step>0x2000*/
					u4Tmp = (((u4DstHeight + 1) << OSD_SC_STEP_BIT) /
						 u4SrcHeight) +
						((((u4DstHeight + 1) << OSD_SC_STEP_BIT) %
						  u4SrcHeight) ? 1 : 0);

					if ((u4Tmp % (1 << OSD_SC_STEP_BIT)) == 0) {
						u4Tmp--;
					}

					IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4Tmp -
									 (1 << (OSD_SC_STEP_BIT - 1))));
					IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4Tmp));
				}
			}

			IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, u4Tmp));
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4SrcHeight));

			if (fgPrgs) {
				IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));
			} else {
				IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight << 1));
			}

			/* to choose osd clock as syspll_d2_ck*/
			/* ???*/
			/*IGNORE_RET(OSD_BASE_SetClock(OSD_CK_SYS));*/
		}
	}

	IGNORE_RET(_OSD_SC_UpdateHwReg(u4Scaler));
	return (__s32)OSD_RET_OK;
}
#endif



