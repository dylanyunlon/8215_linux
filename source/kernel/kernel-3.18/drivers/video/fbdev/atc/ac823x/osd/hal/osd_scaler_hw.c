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
#else
#include "drv_osd_if.h"
#endif
#include "osd_hw.h"

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

/*lint -save -e960 */
#define OSD_SC_SETGET_TMPL(NAME, FIELD) \
	__s32 _OSD_SC_Set##NAME(__u32 u4Scaler, __u32 u4Value) \
	{ \
		OSD_VERIFY_SCALER(u4Scaler); \
		_rOsdScalerReg[u4Scaler].rField.FIELD = u4Value; \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_SC_Get##NAME(__u32 u4Scaler, __u32 *pu4Value) \
	{ \
		OSD_VERIFY_SCALER(u4Scaler); \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = _rOsdScalerReg[u4Scaler].rField.FIELD; \
		return (__s32)OSD_RET_OK; \
	} \

/* above line is intendedly left blanc */
/*lint -restore */


/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

static OSD_SC_UNION_T _rOsdScalerReg[OSD_SCALER_MAX_NUM];
volatile OSD_SC_UNION_T *_prHwOsdScalerReg[OSD_CORE_REG_NUM];


/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_SC_GetReg(__u32 u4Scaler, __u32 *pOsdScalerReg)
{
	__s32 u4Idx = 0;

	OSD_VERIFY_SCALER(u4Scaler);

	if (pOsdScalerReg == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	for (; u4Idx < OSD_SC_REG_NUM; u4Idx++) {
		pOsdScalerReg[u4Idx] = _rOsdScalerReg[u4Scaler].au4Reg[u4Idx];
	}

	return (__s32)OSD_RET_OK;
}

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_SC_SetReg(__u32 u4Scaler, const __u32 *pOsdScalerReg)
{
	__s32 u4Idx = 0;

	OSD_VERIFY_SCALER(u4Scaler);

	if (pOsdScalerReg == NULL) {
		for (; u4Idx < OSD_SC_REG_NUM; u4Idx++) {
			_rOsdScalerReg[u4Scaler].au4Reg[u4Idx] =
				_prHwOsdScalerReg[u4Scaler]->au4Reg[u4Idx];
		}
	} else {
		for (; u4Idx < OSD_SC_REG_NUM; u4Idx++) {
			_rOsdScalerReg[u4Scaler].au4Reg[u4Idx] = pOsdScalerReg[u4Idx];
		}
	}

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_SC_UpdateHwReg(__u32 u4Scaler)
{
	__u32 u4Idx = 0;

	OSD_VERIFY_SCALER(u4Scaler);

	for (; u4Idx < OSD_SC_REG_NUM; u4Idx++) {
		_prHwOsdScalerReg[u4Scaler]->au4Reg[u4Idx] =
			_rOsdScalerReg[u4Scaler].au4Reg[u4Idx];
	}

	if (u4Scaler <= OSD_SCALER_5) {
		_OSD_UpdateReg();
	} else {
		_OSD_R_UpdateReg();
	}

	return (__s32)OSD_RET_OK;
}
OSD_SC_SETGET_TMPL(VuscColorEdgeOnly, fgVuscColorEdgeOnly)
OSD_SC_SETGET_TMPL(VuscAlphaEdgeEn, fgVuscAlphaEdgeEn)
OSD_SC_SETGET_TMPL(VdscColorEdgeOnly, fgVdscColorEdgeOnly)
OSD_SC_SETGET_TMPL(VdscAlphaEdgeEn, fgVdscAlphaEdgeEn)
OSD_SC_SETGET_TMPL(HuscColorEdgeOnly, fgHuscColorEdgeOnly)
OSD_SC_SETGET_TMPL(HuscAlphaEdgeEn, fgHuscAlphaEdgeEn)
OSD_SC_SETGET_TMPL(HdscColorEdgeOnly, fgHdscColorEdgeOnly)
OSD_SC_SETGET_TMPL(HdscAlphaEdgeEn, fgHdscAlphaEdgeEn)
OSD_SC_SETGET_TMPL(VuscEn, fgVuscEn)
OSD_SC_SETGET_TMPL(VdscEn, fgVdscEn)
OSD_SC_SETGET_TMPL(HuscEn, fgHuscEn)
OSD_SC_SETGET_TMPL(HdscEn, fgHdscEn)
OSD_SC_SETGET_TMPL(ScLpfEn, fgScLpfEn)
OSD_SC_SETGET_TMPL(ScEn, fgScEn)
OSD_SC_SETGET_TMPL(SrcVSize, u4SrcVSize)
OSD_SC_SETGET_TMPL(SrcHSize, u4SrcHSize)
OSD_SC_SETGET_TMPL(DstVSize, u4DstVSize)
OSD_SC_SETGET_TMPL(DstHSize, u4DstHSize)
OSD_SC_SETGET_TMPL(VscHSize, u4VscHSize)
OSD_SC_SETGET_TMPL(HdscStep, u4HdscStep)
OSD_SC_SETGET_TMPL(HdscOfst, u4HdscOfst)
OSD_SC_SETGET_TMPL(HuscStep, u4HuscStep)
OSD_SC_SETGET_TMPL(HuscOfst, u4HuscOfst)
OSD_SC_SETGET_TMPL(VscOfstTop, u4VscOfstTop)
OSD_SC_SETGET_TMPL(VscOfstBot, u4VscOfstBot)
OSD_SC_SETGET_TMPL(VscStep, u4VscStep)

OSD_SC_SETGET_TMPL(ScLpfC3, u4ScLpfC3)
OSD_SC_SETGET_TMPL(ScLpfC4, u4ScLpfC4)
OSD_SC_SETGET_TMPL(ScLpfC5, u4ScLpfC5)



