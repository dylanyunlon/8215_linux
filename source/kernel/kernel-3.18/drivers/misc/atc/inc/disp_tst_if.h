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

/** @file disp_tst_if.h
 *  This header file declares exported APIs of Disp test.
 */

#ifndef DISP_TST_IF_H
#define DISP_TST_IF_H
//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include "drv_config.h"
#include "chip_ver.h"
//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------
extern void vDispEmuStart(void);
extern void vDispVfLoadSrcPic(UINT32 dwPicInfoIdx, UINT32 dwTgtBufNo);
extern void vDispVfLoadReg(UINT32 pdArrary [ ], UINT32 dwSize);
extern void vDispVfSetReso(UINT32 dwModule, UINT32 dwSrcReso, UINT32 dwOutputReso);
extern void vDispCLIZoom(UINT32 dwMvdoIdx, UINT32 dwHScaleUp, UINT32 dwHScaleDiv, UINT32 dwVScaleUp, UINT32 dwVScaleDiv);
extern void vDispVfSetPicMode(UINT32 dwMvdoIdx, UINT32 dwPicMode);
extern void vDispCLISetHDMI(UINT32 dwHdmiReso, UINT32 dwHdmiFreq);
extern void vDispVfSetSrcFmt(UINT32 dwMvdoIdx, UINT32 dwSrcFmt);
extern void vDispVfSetVer4TapFilter(UINT32 dwMvdoIdx, UINT32 u4On);
extern void vDispVfHDownScaler(UINT32 u4On, UINT32 u4MvdoIdx, UINT32 dwHScaleUp, UINT32 dwHScaleDiv);
extern void vDispVfLumaKey(UINT32 u4MvdoIdx, UINT32 u4On, UINT32 u4LumaKeyVal);
extern void vDispVfSetScalerHDSFilter(BOOL fgOn, UINT32 u4InReso, UINT32 u4OutReso, BOOL fgInterlaced);
extern void vDispVfSetOSDMixRatio(UINT32 u4OsdId, UINT32 u4MixRatio);
extern void vDispScalerSetTvType(UINT8 u1TvType);
extern void vDispVfSetMode(UINT32 dwSettingIdx);
extern void vDispVfSetFrmBufAddrs(UINT32 dwMvdoIdx, UINT32 dwYFrmIdx, UINT32 dwXFrmIdx, UINT32 dwWFrmIdx, UINT32 dwZFrmIdx, UINT32 dwAFrmIdx);
extern void vDispHalSetScaler709601(BOOL fgEnableCC, BOOL fg709to601);
extern void vDispHalScalerReset(void);
#endif //DISP_TST_IF_H
