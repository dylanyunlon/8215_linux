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
#ifndef _AUD_DSP_CFG_H_
#define _AUD_DSP_CFG_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include <media/atc/drv_aud.h>
#include "aud_drv_config.h"
#include "AsvDef.h"



extern u8 u1DspAGetState(void);

typedef struct _AUD_DSP_PTS_INF_
{
   u32 u4UpdatePtsStcH;
   u32 u4UpdatePtsStcL;
   u32 u4FirstAudPtsStcH;
   u32 u4FirstAudPtsStcL;
   u32 u4StartPtsStcH;
   u32 u4StartPtsStcL;
   u32 u4EndPtsStcH;
   u32 u4EndPtsStcL;
   u32 u4PrimaryPtsH;
   u32 u4PrimaryPtsL;
}AUD_DSP_PTS_INF;

// *********************************************************************
// Export API
// *********************************************************************


#endif //_AUD_DSP_CFG_H_

