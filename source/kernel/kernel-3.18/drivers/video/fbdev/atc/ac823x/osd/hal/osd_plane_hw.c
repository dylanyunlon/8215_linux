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
/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

#ifndef __ARM2__
#include <media/atc/drv_osd_if.h>
#include "windows.h"
#else
#include "drv_osd_if.h"
#endif
/*#include "osd_if.h"*/
#include "osd_hw.h"
#include "chip_ver.h"
#include "osd_map.h"


/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/
/*lint -save -e960 */
#define OSD_PLA_SETGET_TMPL(NAME, FIELD) \
	__s32 _OSD_PLA_Set##NAME(__u32 u4Plane, __u32 u4Value) \
	{ \
		OSD_VERIFY_PLANE(u4Plane); \
		_rOsdPlaneCoreReg[u4Plane].rField.FIELD = u4Value; \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_PLA_Get##NAME(__u32 u4Plane, __u32 *pu4Value) \
	{ \
		OSD_VERIFY_PLANE(u4Plane); \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.FIELD; \
		return (__s32)OSD_RET_OK; \
	} \

/* above line is intendedly left blanc */
/*lint -restore */

#ifndef __ARM2__
/*lint -save -e960 */
#define OSD_PLA_SETGET_ADDRESS_TMPL(NAME, FIELD) \
	__s32 _OSD_PLA_Set##NAME(__u32 u4Plane, __u32 u4Value) \
	{ \
		OSD_VERIFY_PLANE(u4Plane); \
		_rOsdPlaneCoreReg[u4Plane].rField.FIELD = (u4Value == 0) ? 0 : VA_TO_PA(u4Value) >> 4; \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_PLA_Get##NAME(__u32 u4Plane, __u32 *pu4Value) \
	{ \
		OSD_VERIFY_PLANE(u4Plane); \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = (__u32)__va(_rOsdPlaneCoreReg[u4Plane].rField.FIELD << 4); \
		return (__s32)OSD_RET_OK; \
	}
/* above line is intendedly left blanc */
/*lint -restore */
#else
#define OSD_PLA_SETGET_ADDRESS_TMPL(NAME, FIELD) \
	__s32 _OSD_PLA_Set##NAME(__u32 u4Plane, __u32 u4Value) \
	{ \
		OSD_VERIFY_PLANE(u4Plane); \
		_rOsdPlaneCoreReg[u4Plane].rField.FIELD = (u4Value == 0) ? 0 : VA_TO_PA(u4Value) >> 4; \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_PLA_Get##NAME(__u32 u4Plane, __u32 *pu4Value) \
	{ \
		OSD_VERIFY_PLANE(u4Plane); \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = (__u32)PA_TO_VA(_rOsdPlaneCoreReg[u4Plane].rField.FIELD << 4); \
		return (__s32)OSD_RET_OK; \
	}
#endif


/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

/* software register */
static OSD_PLA_CORE_UNION_T _rOsdPlaneCoreReg[OSD_PLANE_MAX_NUM];
/* hardware register map */
volatile OSD_PLA_CORE_UNION_T *_prHwOsdPlaneCoreReg[OSD_CORE_REG_NUM];

/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_PLA_GetReg(__u32 u4Plane, __u32 *pOsdPlaneReg)
{
	__u32 u4Idx = 0;

	OSD_VERIFY_PLANE(u4Plane);

	if (pOsdPlaneReg == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	for (; u4Idx < OSD_CORE_REG_NUM; u4Idx++) {
		pOsdPlaneReg[u4Idx] = _rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx];
	}

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_PLA_SetReg(__u32 u4Plane, const __u32 *pOsdPlaneReg)
{
	__u32 u4Idx = 0;

	OSD_VERIFY_PLANE(u4Plane);

	if (pOsdPlaneReg == NULL) {
		/* a null pointer means reset OSD */
		/* get a copy of reset value from hw */
		for (; u4Idx < OSD_CORE_REG_NUM; u4Idx++) {
			_rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx] =
				_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx];
		}

		return (__s32)OSD_RET_OK;
	}
	for (; u4Idx < OSD_CORE_REG_NUM; u4Idx++) {
		_rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx] = pOsdPlaneReg[u4Idx];
	}

	return (__s32)OSD_RET_OK;
}


/*! \fn __s32 _OSD_PLA_UpdateHwReg(__u32 u4Plane)
      \brief Update the value of software register to hardware register
      \param u4Plane plane index
      \retval __s32  if returner is OSD_RET_OK, the function works successful
*/
__s32 _OSD_PLA_UpdateHwRegInVsync(__u32 u4Plane)
{
	return (__s32)OSD_RET_OK;
}

void _OSD_PLA_SetDestColorKey(bool fgEnable, __u32 u4ColorKey)
{


	OSD_PLA_CORE_UNION_T *prHWReg;

	prHWReg = (OSD_PLA_CORE_UNION_T *)_prHwOsdPlaneCoreReg[0];

	if (fgEnable) {

		/*prHWReg->rField.u4DstKeyLow, prHWReg->rField.u4DstKeyUp));*/

		prHWReg->rField.u4DstKeyEnable = 1;
		prHWReg->rField.u4DstKeySel      = 1;
		/*prHWReg->rField.u4Revert             = 0;*/

		prHWReg->rField.u4DstKeyLow = u4ColorKey;
		prHWReg->rField.u4DstKeyUp    = u4ColorKey;
	} else {

		prHWReg->rField.u4DstKeyEnable = 0;


	}

	/*prHWReg->rField.u4DstKeyLow, prHWReg->rField.u4DstKeyUp));*/


	_OSD_UpdateReg();


	/*prHWReg->rField.u4DstKeyLow, prHWReg->rField.u4DstKeyUp));*/


}
__s32 _OSD_PLA_Enable(__u32 u4Plane)
{
	OSD_VERIFY_PLANE(u4Plane);

	{
		_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0] =
			_rOsdPlaneCoreReg[u4Plane].au4Reg[0];
	}

	if (u4Plane <= OSD_PLANE_5) {
		_OSD_UpdateReg();  /* set shadowed register upate flag */
	} else {
		_OSD_R_UpdateReg();
	}

	return (__s32)OSD_RET_OK;
}
__s32 _OSD_PLA_UpdateHwReg(__u32 u4Plane)
{
	__u32 u4Idx = 0;

	OSD_VERIFY_PLANE(u4Plane);

	for (; u4Idx < OSD_CORE_REG_NUM; u4Idx++) {
		_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx] =
			_rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx];
	}

	if (u4Plane <= OSD_PLANE_5) {
		_OSD_UpdateReg();  /* set shadowed register upate flag */
	} else {
		_OSD_R_UpdateReg();
	}

	return (__s32)OSD_RET_OK;
}

OSD_PLA_SETGET_TMPL(Enable, fgOsdEn)
OSD_PLA_SETGET_TMPL(FakeHdr, fgFakeHdr)
OSD_PLA_SETGET_TMPL(PrngEn, fgPrngEn)
OSD_PLA_SETGET_TMPL(AlphaZeroBlack, fgAlphaZeroBlack)
OSD_PLA_SETGET_TMPL(OutRngColorMode, fgOutRngColorMode)
OSD_PLA_SETGET_ADDRESS_TMPL(HeaderAddr, u4HeaderAddr)
OSD_PLA_SETGET_TMPL(Blending, u4GobalBlending)
OSD_PLA_SETGET_TMPL(Fading, u4FadingRatio)
OSD_PLA_SETGET_TMPL(HFilter, fgHFilter)
OSD_PLA_SETGET_TMPL(ColorExpSel, fgColorExpSel)
OSD_PLA_SETGET_TMPL(AlphaRatioEn, fgAlphaRatioEn)

OSD_PLA_SETGET_TMPL(ContReqLmt, u4ContReqLmt)
OSD_PLA_SETGET_TMPL(FifoSize, u4FifoSize)
OSD_PLA_SETGET_TMPL(PauseCnt, u4PauseCnt)
OSD_PLA_SETGET_TMPL(ContReqLmt0, u4ContReqLmt0)
OSD_PLA_SETGET_TMPL(BurstDis, fgBurstDis)
OSD_PLA_SETGET_TMPL(RgbMode, fgRgbMode)
OSD_PLA_SETGET_TMPL(VacancyThr, u4VacancyThr)
OSD_PLA_SETGET_TMPL(HMirrorEn, fgHMirrorEn)
OSD_PLA_SETGET_TMPL(VFlipEn, fgVFlipEn)
OSD_PLA_SETGET_TMPL(Rgb2YcbrbEn, fgRgb2YcbrbEn)
OSD_PLA_SETGET_TMPL(XVYCCEn, fgXVYCCEn)
OSD_PLA_SETGET_TMPL(YCbCr709En, fgYCbCr709En)
OSD_PLA_SETGET_TMPL(Osd5FifoWrapEn, fgOsd5FifoWrapEn)
OSD_PLA_SETGET_TMPL(Osd1ArbRgnEn, fgOsd1ArbRgnEn)
/*
8560 OSD5 Only suport 4bpp
*/




