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

#include "pp_hal.h"
#include "pp_log.h"
#include "pp_drv.h"

#ifdef __ARM2__
unsigned int _u4PP_DBG_LVL = PP_LOG_LVL_HAL;
#else
unsigned int _u4PP_DBG_LVL = PP_LOG_LVL_DBG;
#endif
unsigned char *_pcPpLogLevel[] = {
	"[PP] [ERR]",
	"[PP] [WARN]",
	"[PP] [INFO]",
	"[PP] [HAL]",
	"[PP] [DBG]",
	"[PP] [IRQ]",
};

void PpInit(unsigned char VideoPath, PP_DISPLAY_MODE_E eDisplayMode)
{
	_PpInit(VideoPath, eDisplayMode);
}

void PostFmt(unsigned char VideoPath, unsigned int PicSize)
{
	PP_LOG(PP_LOG_LVL_INFO, "VideoPath = %d, PicSize = %d\n", VideoPath ,PicSize);

	_PostFmt(VideoPath, PicSize);
}
EXPORT_SYMBOL(PostFmt);

void PostSharpness(unsigned char UiMin, unsigned char UiMax, unsigned char UiDft, unsigned char UiCur)
{
	_PostSharpness(UiMin, UiMax, UiDft, UiCur);

}
EXPORT_SYMBOL(PostSharpness);

void PostCTI(unsigned char UiMin, unsigned char UiMax, unsigned char UiDft, unsigned char UiCur)
{
	_PostCTI(UiMin, UiMax, UiDft, UiCur);

}
EXPORT_SYMBOL(PostCTI);

void PostSharpParaSet(POST_SHN_BAND_PARA eBandPara)
{
	vDrvPostSharpParaSet(eBandPara);

}
EXPORT_SYMBOL(PostSharpParaSet);

void PostSharpCtrlSet(POST_SHN_CTRL_PARA eCtrlPara)
{
    vDrvPostSharpCtrlSet(eCtrlPara);

}
EXPORT_SYMBOL(PostSharpCtrlSet);

void SetPostCTI(unsigned char u1Data)
{

	if((u1Data < 0) || (u1Data > 5))	
		PP_LOG(PP_LOG_LVL_INFO, "Arg data is not correct!\n");
	else
		i4PostVideoProc(POST_VIDEO_CTI, 0, 5, 0, u1Data);
}
EXPORT_SYMBOL(SetPostCTI);

void CTICtrlSet(POST_CTI_CTRL_PARA eCtrlPara)
{
    eDrvCTIRParamSet(eCtrlPara);
}
EXPORT_SYMBOL(CTICtrlSet);

void SetPostProcess(unsigned char VideoPath)
{
	_getPPaddr(VideoPath);
	PostSharpness(3, 3, 2, 2);
	PostCTI(3, 3, 2, 2);
}
EXPORT_SYMBOL(SetPostProcess);

