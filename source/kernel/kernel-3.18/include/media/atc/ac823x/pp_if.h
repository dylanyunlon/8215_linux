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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-02-27
 */
#ifndef _PP_IF_H_
#define _PP_IF_H_

#include "pp_hal.h"
#include "pp_drv.h"

void PostFmt(unsigned char VideoPath, unsigned int PicSize);
void PostSharpness(unsigned char UiMin, unsigned char UiMax, unsigned char UiDft, unsigned char UiCur);
void PostCTI(unsigned char UiMin, unsigned char UiMax, unsigned char UiDft, unsigned char UiCur);
void PostSharpParaSet(POST_SHN_BAND_PARA eBandPara);
void PostSharpCtrlSet(POST_SHN_CTRL_PARA eCtrlPara);
void SetPostCTI(unsigned char u1Data);
void CTICtrlSet(POST_CTI_CTRL_PARA eCtrlPara);
void SetPostProcess(unsigned char VideoPath);
void PpInit(unsigned char VideoPath, PP_DISPLAY_MODE_E eDisplayMode);

#endif
