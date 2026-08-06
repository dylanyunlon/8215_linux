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
#include "x_ioopt.h"
#else
#include "drv_osd_if.h"
#endif
#include "osd_hw.h"
/*#include "osd_if.h"*/


/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

/*lint -save -e960 */
#define OSD_BASE_SETGET_TMPL(NAME, FIELD) \
	__s32 _OSD_BASE_Set##NAME(__u32 u4Value) \
	{ \
		_rOsdBaseReg.rField.FIELD = u4Value; \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_BASE_Get##NAME(__u32 *pu4Value) \
	{ \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = _rOsdBaseReg.rField.FIELD; \
		return (__s32)OSD_RET_OK; \
	} \

#define OSD_BASE_SETGET_(NAME, FIELD) \
	__s32 _OSD_BASE_Set##NAME(__u32 u4Value) \
	{ \
		_prHwOsdBaseReg->rField.FIELD = u4Value; \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_BASE_Get##NAME(__u32 *pu4Value) \
	{ \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = _prHwOsdBaseReg->rField.FIELD; \
		return (__s32)OSD_RET_OK; \
	} \

#define OSD_R_BASE_SETGET_TMPL(NAME, FIELD) \
	__s32 _OSD_R_BASE_Set##NAME(__u32 u4Value) \
	{ \
		_rOsdRBaseReg.rField.FIELD = u4Value; \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_R_BASE_Get##NAME(__u32 *pu4Value) \
	{ \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = _rOsdRBaseReg.rField.FIELD; \
		return (__s32)OSD_RET_OK; \
	} \

#define OSD_R_BASE_SETGET_(NAME, FIELD) \
	__s32 _OSD_R_BASE_Set##NAME(__u32 u4Value) \
	{ \
		_prHwOsdRBaseReg->rField.FIELD = u4Value; \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_R_BASE_Get##NAME(__u32 *pu4Value) \
	{ \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = _prHwOsdRBaseReg->rField.FIELD; \
		return (__s32)OSD_RET_OK; \
	} \

/* above line is intendedly left blanc */
/*lint -restore */


/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

static OSD_BASE_UNION_T _rOsdBaseReg;
volatile OSD_BASE_UNION_T *_prHwOsdBaseReg = NULL;

static OSD_BASE_UNION_T _rOsdRBaseReg;
volatile OSD_BASE_UNION_T *_prHwOsdRBaseReg = NULL;

/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
void _OSD_AlwaysUpdateReg(bool fgEnable)
{
	__u32  u4Val;


	if (fgEnable) {
		u4Val = IO_READ32(osdf_reg, 0);
		u4Val |= 0x02;
		IO_WRITE32(osdf_reg, 0, u4Val);
	} else {
		u4Val = IO_READ32(osdf_reg, 0);
		u4Val &= (~0x02);
		IO_WRITE32(osdf_reg, 0, u4Val);
	}
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
void _OSD_UpdateReg(void)
{
	__u32  u4Val;

	u4Val = IO_READ32(osdf_reg, 0);
	u4Val |= 0x01;
	IO_WRITE32(osdf_reg, 0, u4Val);
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_BASE_GetReg(__u32 *pOsdBaseReg)
{
	__s32 u4Idx = OSD_BASE_SKIP;

	if (pOsdBaseReg == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	for (; u4Idx < OSD_BASE_REG_NUM; u4Idx++) {
		pOsdBaseReg[u4Idx] = _rOsdBaseReg.au4Reg[u4Idx];
	}

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
#ifdef __ARM2__
static void _msleep(unsigned int cnt)
{
	int i =0, i4result =0;

	for (i =0; i< (cnt * 0x10000); i++)
	{
	   i4result ++;
	}
}
#endif
__s32 _OSD_BASE_SetReg(const __u32 *pOsdBaseReg, bool fgHwReset)
{
	/*__u32 u4Idx = OSD_BASE_SKIP;*/
	__u32 u4Idx;


	if (pOsdBaseReg == NULL) {
		if (fgHwReset) {
			u4Idx = IO_READ32(osdf_reg, 4);


			u4Idx |= OSD_RESET_PLANE_MASK;
			IO_WRITE32(osdf_reg, 4, u4Idx);
#ifndef __ARM2__
			Sleep(5);
#else
			_msleep(5);
#endif
			u4Idx &= (~OSD_RESET_PLANE_MASK);
			u4Idx |= 0x01F00000;
			IO_WRITE32(osdf_reg, 4, u4Idx);


			IO_WRITE32(osdf_reg, 0x38, 0x80008000);
			IO_WRITE32(osdf_reg, 0x3c, 0x80008000);

		}

		/*_prHwOsdBaseReg->rField.fgResetAll = 3;*/
		for (u4Idx = 0; u4Idx < OSD_BASE_REG_NUM; u4Idx++) {
			_rOsdBaseReg.au4Reg[u4Idx] = _prHwOsdBaseReg->au4Reg[u4Idx];
		}

		_rOsdBaseReg.au4Reg[1] &= (~OSD_RESET_PLANE_MASK);
	} else {
		for (u4Idx = OSD_BASE_SKIP; u4Idx < OSD_BASE_REG_NUM; u4Idx++) {
			_rOsdBaseReg.au4Reg[u4Idx] = pOsdBaseReg[u4Idx];
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
__s32  _OSD_BASE_Update(__u32 u4Base)
{
	__u32 u4Idx = OSD_BASE_SKIP;

	if (u4Base == OSD_BASE_MAIN) {
		for (; u4Idx < OSD_BASE_REG_NUM; u4Idx++) {
			_prHwOsdBaseReg->au4Reg[u4Idx] = _rOsdBaseReg.au4Reg[u4Idx];
		}

		_OSD_UpdateReg();
	} else {
		for (; u4Idx < OSD_R_BASE_REG_NUM; u4Idx++) {
			_prHwOsdRBaseReg->au4Reg[u4Idx] = _rOsdRBaseReg.au4Reg[u4Idx];
		}

		_OSD_R_UpdateReg();
	}

	return (__s32)OSD_RET_OK;
}
__s32 _OSD_BASE_UpdateHwReg(void)
{
	__u32 u4Idx = OSD_BASE_SKIP;

	for (; u4Idx < OSD_BASE_REG_NUM; u4Idx++) {
		_prHwOsdBaseReg->au4Reg[u4Idx] = _rOsdBaseReg.au4Reg[u4Idx];
	}

	_OSD_UpdateReg();

	return (__s32)OSD_RET_OK;
}

/*OSD_BASE_FIELD_T * gpt_osd_base_filed;*/

OSD_BASE_SETGET_TMPL(Update, fgUpdate)
OSD_BASE_SETGET_TMPL(AlwaysUpdate, fgAlwaysUpdate)
OSD_BASE_SETGET_TMPL(ResetMainPath, fgRstMainFmt)
OSD_BASE_SETGET_TMPL(ResetAuxPath, fgRstAuxFmt)
OSD_BASE_SETGET_TMPL(ResetMegPath, fgRstMegFmt)
OSD_BASE_SETGET_TMPL(ResetOsd1, fgRstOsd1)
OSD_BASE_SETGET_TMPL(ResetOsd2, fgRstOsd2)
OSD_BASE_SETGET_TMPL(ResetOsd3, fgRstOsd3)
OSD_BASE_SETGET_TMPL(ResetOsd4, fgRstOsd4)
OSD_BASE_SETGET_TMPL(ResetOsd5, fgRstOsd5)
OSD_BASE_SETGET_TMPL(ResetCursor, fgRstCsr)

/* 08h OSD mode configuration register */
OSD_BASE_SETGET_TMPL(HsEdge, fgHsEdge)
OSD_BASE_SETGET_TMPL(VsEdge, fgVsEdge)
OSD_BASE_SETGET_TMPL(FldPol, fgFldPol)
OSD_BASE_SETGET_TMPL(Osd1Prgs, fgOsd1Prgs)
OSD_BASE_SETGET_TMPL(Osd2Prgs, fgOsd2Prgs)
OSD_BASE_SETGET_TMPL(Osd3Prgs, fgOsd3Prgs)
OSD_BASE_SETGET_TMPL(Osd4Prgs, fgOsd4Prgs)
OSD_BASE_SETGET_TMPL(Osd5Prgs, fgOsd5Prgs)
OSD_BASE_SETGET_TMPL(CsrPrgs, fgCsrPrgs)
OSD_BASE_SETGET_TMPL(Osd1Path, fgOsd1Aux)
OSD_BASE_SETGET_TMPL(Osd2Path, fgOsd2Aux)
OSD_BASE_SETGET_TMPL(Osd3Path, fgOsd3Aux)
OSD_BASE_SETGET_TMPL(Osd4Path, fgOsd4Aux)
OSD_BASE_SETGET_TMPL(Osd5Path, fgOsd5Aux)
OSD_BASE_SETGET_TMPL(CsrPath, fgCsrAux)
OSD_BASE_SETGET_TMPL(HsEdgeMeg, fgHsEdgeMeg)
OSD_BASE_SETGET_TMPL(VsEdgeMeg, fgVsEdgeMeg)
OSD_BASE_SETGET_TMPL(FldPolMeg, fgFldPolMeg)
OSD_BASE_SETGET_TMPL(Osd1Dotctl, u4Osd1Dotctl)
OSD_BASE_SETGET_TMPL(Osd2Dotctl, u4Osd2Dotctl)
OSD_BASE_SETGET_TMPL(Osd3Dotctl, u4Osd3Dotctl)
OSD_BASE_SETGET_TMPL(Osd4Dotctl, u4Osd4Dotctl)
OSD_BASE_SETGET_TMPL(Osd5Dotctl, u4Osd5Dotctl)
OSD_BASE_SETGET_TMPL(CsrDotctl, u4CsrDotctl)


/* 0Ch Main FMT Vsync Timing Configuration Register */
OSD_BASE_SETGET_TMPL(OvtMain, u4OvtMain)
OSD_BASE_SETGET_TMPL(VsWidthMain, u4VsWidthMain)
OSD_BASE_SETGET_TMPL(HsWidthMain, u4HsWidthMain)

/* 10h FMT H-Timing Configuration Register#1 */
OSD_BASE_SETGET_TMPL(ScrnHStartOsd2, u4ScrnHStartOsd2)
OSD_BASE_SETGET_TMPL(ScrnHStartOsd1, u4ScrnHStartOsd1)

/* 14h FMT H-Timing Configuration Register#2 */
OSD_BASE_SETGET_TMPL(ScrnHStartCsr, u4ScrnHStartCsr)
OSD_BASE_SETGET_TMPL(ScrnHStartOsd3, u4ScrnHStartOsd3)

/* 18h Main FMT V-Timing Configuration Register #1 */
OSD_BASE_SETGET_TMPL(ScrnVStartBotMain, u4ScrnVStartBotMain)
OSD_BASE_SETGET_TMPL(ScrnVStartTopMain, u4ScrnVStartTopMain)

/* 1Ch Main FMT Timing Configuration Register #2 */
OSD_BASE_SETGET_TMPL(ScrnVSizeMain, u4ScrnVSizeMain)
OSD_BASE_SETGET_TMPL(ScrnHSizeMain, u4ScrnHSizeMain)

/* 20h OSD1 Window Position Configuration Register */
OSD_BASE_SETGET_TMPL(Osd1VStart, u4Osd1VStart)
OSD_BASE_SETGET_TMPL(Osd1HStart, u4Osd1HStart)

/* 24h OSD2 Window Position Configuration Register  */
OSD_BASE_SETGET_TMPL(Osd2VStart, u4Osd2VStart)
OSD_BASE_SETGET_TMPL(Osd2HStart, u4Osd2HStart)

/* 28h OSD3 Window Position Configuration Register */
OSD_BASE_SETGET_TMPL(Osd3VStart, u4Osd3VStart)
OSD_BASE_SETGET_TMPL(Osd3HStart, u4Osd3HStart)

/* 2Ch OSD Misc. Control Register */
OSD_BASE_SETGET_TMPL(Osd12Ex, fgOsd12Ex)
OSD_BASE_SETGET_TMPL(Osd34Ex, fgOsd34Ex)


/* 40h FMT H-Timing Configuration Register #3 */
OSD_BASE_SETGET_TMPL(ScrnHStartOsd4, u4ScrnHStartOsd4)
OSD_BASE_SETGET_TMPL(ScrnHStartOsd5, u4ScrnHStartOsd5)

/* 44h OSD4 Window Position Configuration Register */
OSD_BASE_SETGET_TMPL(Osd4VStart, u4Osd4VStart)
OSD_BASE_SETGET_TMPL(Osd4HStart, u4Osd4HStart)

/* 48h OSD5 Window Position Configuration Register */
OSD_BASE_SETGET_TMPL(Osd5VStart, u4Osd5VStart)
OSD_BASE_SETGET_TMPL(Osd5HStart, u4Osd5HStart)

/* 4Ch Aux FMT Vsync Timing Configuration Register */
OSD_BASE_SETGET_TMPL(OvtAux, u4OvtAux)
OSD_BASE_SETGET_TMPL(VsWidthAux, u4VsWidthAux)
OSD_BASE_SETGET_TMPL(HsWidthAux, u4HsWidthAux)

/* 50h Aux FMT V-Timing Configuration Register #1  */
OSD_BASE_SETGET_TMPL(ScrnVStartBotAux, u4ScrnVStartBotAux)
OSD_BASE_SETGET_TMPL(ScrnVStartTopAux, u4ScrnVStartTopAux)

/* 54h Aux FMT Timing Configuration Register #2  */
OSD_BASE_SETGET_TMPL(ScrnVSizeAux, u4ScrnVSizeAux)
OSD_BASE_SETGET_TMPL(ScrnHSizeAux, u4ScrnHSizeAux)

/* 58h Message FMT Vsync Timing Configuration Register */
OSD_BASE_SETGET_TMPL(OvtMeg, u4OvtMeg)
OSD_BASE_SETGET_TMPL(VsWidthMeg, u4VsWidthMeg)
OSD_BASE_SETGET_TMPL(HsWidthMeg, u4HsWidthMeg)

/* 5Ch Message FMT V-Timing Configuration Register #1 */
OSD_BASE_SETGET_TMPL(ScrnVStartBotMeg, u4ScrnVStartBotMeg)
OSD_BASE_SETGET_TMPL(ScrnVStartTopMeg, u4ScrnVStartTopMeg)

/* 60h Message FMT Timing Configuration Register #2 */
OSD_BASE_SETGET_TMPL(ScrnVSizeMeg, u4ScrnVSizeMeg)
OSD_BASE_SETGET_TMPL(ScrnHSizeMeg, u4ScrnHSizeMeg)

/* DWORD - 064 OSD_FMT_64 */
OSD_BASE_SETGET_TMPL(OhtMain, u4OhtMain)
/* DWORD - 084 OSD_FMT_68 */
OSD_BASE_SETGET_TMPL(OhtAux, u4OhtAux)
/* DWORD - 084 OSD_FMT_6C */
OSD_BASE_SETGET_TMPL(OhtMeg, u4OhtMeg)
/* DWORD - 088 OSD_FMT_70 */
OSD_BASE_SETGET_TMPL(IntTGen, fgIntTGen)
OSD_BASE_SETGET_TMPL(CheckSumEn, fgCheckSumEn)
OSD_BASE_SETGET_TMPL(Sc1CheckSumSel, u4Sc1CheckSumSel)
OSD_BASE_SETGET_TMPL(Sc2CheckSumSel, u4Sc2CheckSumSel)
OSD_BASE_SETGET_TMPL(Sc3CheckSumSel, u4Sc3CheckSumSel)
OSD_BASE_SETGET_TMPL(Sc4CheckSumSel, u4Sc4CheckSumSel)
/* DWORD - 090 OSD_FMT_78 */
OSD_BASE_SETGET_(Osd1CheckSum, u4Osd1CheckSum)
/* DWORD - 094 OSD_FMT_7C */
OSD_BASE_SETGET_(Osd1ScCheckSum, u4Osd1ScCheckSum)
/* DWORD - 098 OSD_FMT_80 */
OSD_BASE_SETGET_(Osd2CheckSum, u4Osd2CheckSum)
/* DWORD - 09C OSD_FMT_84 */
OSD_BASE_SETGET_(Osd2ScCheckSum, u4Osd2ScCheckSum)
/* DWORD - 0A0 OSD_FMT_88 */
OSD_BASE_SETGET_(Osd3CheckSum, u4Osd3CheckSum)
/* DWORD - 0A4 OSD_FMT_8C */
OSD_BASE_SETGET_(Osd3ScCheckSum, u4Osd3ScCheckSum)
/* DWORD - 0C0 OSD_FMT_90 */
OSD_BASE_SETGET_(Osd4CheckSum, u4Osd4CheckSum)
/* DWORD - 0C4 OSD_FMT_94 */
/* DWORD - 0C8 OSD_FMT_98 */
OSD_BASE_SETGET_(Osd5CheckSum, u4Osd5CheckSum)
/* DWORD - 0CC OSD_FMT_9C */


/********************************************************************************************
 *    OSD_R BASE REGISTER
 ************************************************************************************************/

/* above line is intendedly left blanc */
/*lint -restore */




/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
void _OSD_R_AlwaysUpdateReg(bool fgEnable)
{
	__u32  u4Val;


	if (fgEnable) {
		u4Val = IO_READ32(osdr_reg, 0);
		u4Val |= 0x02;
		IO_WRITE32(osdr_reg, 0, u4Val);
	} else {
		u4Val = IO_READ32(osdr_reg, 0);
		u4Val &= (~0x02);
		IO_WRITE32(osdr_reg, 0, u4Val);
	}
}


#if 1
/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
void _OSD_R_UpdateReg(void)
{
	__u32  u4Val;

	u4Val = IO_READ32(osdr_reg, 0);
	u4Val |= 0x01;
	IO_WRITE32(osdr_reg, 0, u4Val);
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_R_BASE_GetReg(__u32 *pOsdBaseReg)
{
	__s32 u4Idx = OSD_BASE_SKIP;

	if (pOsdBaseReg == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	for (; u4Idx < OSD_R_BASE_REG_NUM; u4Idx++) {
		pOsdBaseReg[u4Idx] = _rOsdRBaseReg.au4Reg[u4Idx];
	}

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_R_BASE_SetReg(const __u32 *pOsdBaseReg)
{
	/*__u32 u4Idx = OSD_BASE_SKIP;*/
	__u32 u4Idx;


	if (pOsdBaseReg == NULL) {
		u4Idx = IO_READ32(osdr_reg, 4);
#if !CONFIG_DRV_FAST_LOGO
		u4Idx |= OSD_RESET_PLANE_MASK;
		IO_WRITE32(osdr_reg, 4, u4Idx);

		/*x_thread_delay(5);*/
#ifndef __ARM2__
		mdelay(5);
#else
		//msleep(5);
#endif
		u4Idx &= (~OSD_RESET_PLANE_MASK);
		u4Idx |= 0x01F00000;
		IO_WRITE32(osdr_reg, 4, u4Idx);


		IO_WRITE32(osdr_reg, 0x38, 0x80008000);
		IO_WRITE32(osdr_reg, 0x3c, 0x80008000);
#endif

		for (u4Idx = 0; u4Idx < OSD_R_BASE_REG_NUM; u4Idx++) {
			_rOsdRBaseReg.au4Reg[u4Idx] = _prHwOsdRBaseReg->au4Reg[u4Idx];
		}

		_rOsdRBaseReg.au4Reg[1] &= (~OSD_RESET_PLANE_MASK);
	} else {
		for (u4Idx = OSD_BASE_SKIP; u4Idx < OSD_R_BASE_REG_NUM; u4Idx++) {
			_rOsdRBaseReg.au4Reg[u4Idx] = pOsdBaseReg[u4Idx];

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
__s32 _OSD_R_BASE_UpdateHwReg(void)
{
	__u32 u4Idx = OSD_BASE_SKIP;

	for (; u4Idx < OSD_R_BASE_REG_NUM; u4Idx++) {
		_prHwOsdRBaseReg->au4Reg[u4Idx] = _rOsdRBaseReg.au4Reg[u4Idx];
	}

	_OSD_R_UpdateReg();

	return (__s32)OSD_RET_OK;
}
/*OSD_BASE_FIELD_T * gpt_osd_base_filed;*/

OSD_R_BASE_SETGET_TMPL(Update, fgUpdate)
OSD_R_BASE_SETGET_TMPL(AlwaysUpdate, fgAlwaysUpdate)
OSD_R_BASE_SETGET_TMPL(ResetMainPath, fgRstMainFmt)
OSD_R_BASE_SETGET_TMPL(ResetAuxPath, fgRstAuxFmt)
OSD_R_BASE_SETGET_TMPL(ResetMegPath, fgRstMegFmt)
OSD_R_BASE_SETGET_TMPL(ResetOsd6, fgRstOsd1)
OSD_R_BASE_SETGET_TMPL(ResetOsd7, fgRstOsd2)
OSD_R_BASE_SETGET_TMPL(ResetOsd8, fgRstOsd3)

/* 08h OSD mode configuration register */
OSD_R_BASE_SETGET_TMPL(HsEdge, fgHsEdge)
OSD_R_BASE_SETGET_TMPL(VsEdge, fgVsEdge)
OSD_R_BASE_SETGET_TMPL(FldPol, fgFldPol)
OSD_R_BASE_SETGET_TMPL(Osd6Prgs, fgOsd1Prgs)
OSD_R_BASE_SETGET_TMPL(Osd7Prgs, fgOsd2Prgs)
OSD_R_BASE_SETGET_TMPL(Osd8Prgs, fgOsd3Prgs)
OSD_R_BASE_SETGET_TMPL(CsrPrgs, fgCsrPrgs)
OSD_R_BASE_SETGET_TMPL(Osd6Path, fgOsd1Aux)
OSD_R_BASE_SETGET_TMPL(Osd7Path, fgOsd2Aux)
OSD_R_BASE_SETGET_TMPL(Osd8Path, fgOsd3Aux)
OSD_R_BASE_SETGET_TMPL(CsrPath, fgCsrAux)
OSD_R_BASE_SETGET_TMPL(HsEdgeMeg, fgHsEdgeMeg)
OSD_R_BASE_SETGET_TMPL(VsEdgeMeg, fgVsEdgeMeg)
OSD_R_BASE_SETGET_TMPL(FldPolMeg, fgFldPolMeg)
OSD_R_BASE_SETGET_TMPL(Osd6Dotctl, u4Osd1Dotctl)
OSD_R_BASE_SETGET_TMPL(Osd7Dotctl, u4Osd2Dotctl)
OSD_R_BASE_SETGET_TMPL(Osd8Dotctl, u4Osd3Dotctl)
OSD_R_BASE_SETGET_TMPL(CsrDotctl, u4CsrDotctl)


/* 0Ch Main FMT Vsync Timing Configuration Register */
OSD_R_BASE_SETGET_TMPL(OvtMain, u4OvtMain)
OSD_R_BASE_SETGET_TMPL(VsWidthMain, u4VsWidthMain)
OSD_R_BASE_SETGET_TMPL(HsWidthMain, u4HsWidthMain)

/* 10h FMT H-Timing Configuration Register#1 */
OSD_R_BASE_SETGET_TMPL(ScrnHStartOsd7, u4ScrnHStartOsd2)
OSD_R_BASE_SETGET_TMPL(ScrnHStartOsd6, u4ScrnHStartOsd1)

/* 14h FMT H-Timing Configuration Register#2 */
OSD_R_BASE_SETGET_TMPL(ScrnHStartCsr, u4ScrnHStartCsr)
OSD_R_BASE_SETGET_TMPL(ScrnHStartOsd8, u4ScrnHStartOsd3)

/* 18h Main FMT V-Timing Configuration Register #1 */
OSD_R_BASE_SETGET_TMPL(ScrnVStartBotMain, u4ScrnVStartBotMain)
OSD_R_BASE_SETGET_TMPL(ScrnVStartTopMain, u4ScrnVStartTopMain)

/* 1Ch Main FMT Timing Configuration Register #2 */
OSD_R_BASE_SETGET_TMPL(ScrnVSizeMain, u4ScrnVSizeMain)
OSD_R_BASE_SETGET_TMPL(ScrnHSizeMain, u4ScrnHSizeMain)

/* 20h OSD1 Window Position Configuration Register */
OSD_R_BASE_SETGET_TMPL(Osd6VStart, u4Osd1VStart)
OSD_R_BASE_SETGET_TMPL(Osd6HStart, u4Osd1HStart)

/* 24h OSD2 Window Position Configuration Register  */
OSD_R_BASE_SETGET_TMPL(Osd7VStart, u4Osd2VStart)
OSD_R_BASE_SETGET_TMPL(Osd7HStart, u4Osd2HStart)

/* 28h OSD3 Window Position Configuration Register */
OSD_R_BASE_SETGET_TMPL(Osd8VStart, u4Osd3VStart)
OSD_R_BASE_SETGET_TMPL(Osd8HStart, u4Osd3HStart)

/* 2Ch OSD Misc. Control Register */

/* 40h FMT H-Timing Configuration Register #3 */

/* 44h OSD4 Window Position Configuration Register */

/* 48h OSD5 Window Position Configuration Register */

/* 4Ch Aux FMT Vsync Timing Configuration Register */
OSD_R_BASE_SETGET_TMPL(OvtAux, u4OvtAux)
OSD_R_BASE_SETGET_TMPL(VsWidthAux, u4VsWidthAux)
OSD_R_BASE_SETGET_TMPL(HsWidthAux, u4HsWidthAux)

/* 50h Aux FMT V-Timing Configuration Register #1  */
OSD_R_BASE_SETGET_TMPL(ScrnVStartBotAux, u4ScrnVStartBotAux)
OSD_R_BASE_SETGET_TMPL(ScrnVStartTopAux, u4ScrnVStartTopAux)

/* 54h Aux FMT Timing Configuration Register #2  */
OSD_R_BASE_SETGET_TMPL(ScrnVSizeAux, u4ScrnVSizeAux)
OSD_R_BASE_SETGET_TMPL(ScrnHSizeAux, u4ScrnHSizeAux)

/* 58h Message FMT Vsync Timing Configuration Register */
OSD_R_BASE_SETGET_TMPL(OvtMeg, u4OvtMeg)
OSD_R_BASE_SETGET_TMPL(VsWidthMeg, u4VsWidthMeg)
OSD_R_BASE_SETGET_TMPL(HsWidthMeg, u4HsWidthMeg)

/* 5Ch Message FMT V-Timing Configuration Register #1 */
OSD_R_BASE_SETGET_TMPL(ScrnVStartBotMeg, u4ScrnVStartBotMeg)
OSD_R_BASE_SETGET_TMPL(ScrnVStartTopMeg, u4ScrnVStartTopMeg)

/* 60h Message FMT Timing Configuration Register #2 */
OSD_R_BASE_SETGET_TMPL(ScrnVSizeMeg, u4ScrnVSizeMeg)
OSD_R_BASE_SETGET_TMPL(ScrnHSizeMeg, u4ScrnHSizeMeg)

/* DWORD - 064 OSD_FMT_64 */
OSD_R_BASE_SETGET_TMPL(OhtMain, u4OhtMain)
/* DWORD - 084 OSD_FMT_68 */
OSD_R_BASE_SETGET_TMPL(OhtAux, u4OhtAux)
/* DWORD - 084 OSD_FMT_6C */
OSD_R_BASE_SETGET_TMPL(OhtMeg, u4OhtMeg)
/* DWORD - 088 OSD_FMT_70 */
OSD_R_BASE_SETGET_TMPL(IntTGen, fgIntTGen)
OSD_R_BASE_SETGET_TMPL(CheckSumEn, fgCheckSumEn)
/* DWORD - 090 OSD_FMT_78 */
OSD_R_BASE_SETGET_(Osd6CheckSum, u4Osd1CheckSum)
/* DWORD - 094 OSD_FMT_7C */
/* DWORD - 098 OSD_FMT_80 */
OSD_R_BASE_SETGET_(Osd7CheckSum, u4Osd2CheckSum)
/* DWORD - 09C OSD_FMT_84 */
/* DWORD - 0A0 OSD_FMT_88 */
OSD_R_BASE_SETGET_(Osd8CheckSum, u4Osd3CheckSum)
/* DWORD - 0A4 OSD_FMT_8C */
/* DWORD - 0C0 OSD_FMT_90 */
/* DWORD - 0C4 OSD_FMT_94 */
/* DWORD - 0C8 OSD_FMT_98 */
/* DWORD - 0CC OSD_FMT_9C */
#endif







