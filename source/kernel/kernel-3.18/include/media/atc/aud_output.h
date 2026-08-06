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

#include "x_common.h"
#include "x_typedef.h"
#include "drv_av_d.h"

#ifndef __AUD_OUTPUT_H_
#define __AUD_OUTPUT_H_


typedef enum{
    AUD_FRONT,
    AUD_REAR,
    AUD_GPS,
    AUD_FRONT_REAR,
    AUD_SPDIF,
    AUD_OUT_MAX
} AUD_CFG_ID;

typedef enum{
    AUD_AOUT1,
    AUD_AOUT2,
    AUD_DVD_OUT,
    AUD_GPS_OUT,
    AUD_UNDEF_OUT
} AUD_OUT_TYPE_T;

typedef enum {
    AUD_DAC_PWM,
    AUD_DAC_EXT,
} AUD_DAC_TYPE_T;


typedef struct _AUD_OUTPUT_PATH_T
{
    AUD_CFG_ID          eOut;           /* Front, GPS, Rear */
    AUD_OUT_TYPE_T      eSrc;           /* Aout1, Aout2, DVP Aout, GPS Aout */
} AUD_OUTPUT_PATH_T;

typedef struct _AUD_DAC_TYPE_SEL_T
{
    AUD_CFG_ID          eOut;           /* 0:Front, 1:Rear, 2:GPS */
    AUD_DAC_TYPE_T      eDacType;       /* 0:PWM DAC, 1:External DAC */
} AUD_DAC_TYPE_SEL_T;


#endif // __AUD_OUTPUT_H_

