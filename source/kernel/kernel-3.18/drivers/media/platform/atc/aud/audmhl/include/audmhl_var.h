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
/*----------------------------------------------------------------------------*
 * $RCSfile: audmhl_var.h,v $
 * $Revision: #2 $
 * $Date: 2016/03/01 $
 * $Author: lingbo.liu $
 *
 * Description:
 *         This header file contains edid information for SCOM, specific
 *         definitions, which are exported.
 *---------------------------------------------------------------------------*/

#ifndef _AUDMHL_VAR_H_
#define _AUDMHL_VAR_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include <media/atc/x_audin.h>

#if CONFIG_DRV_HDMI_RX    

typedef enum
{
    AUDINPORT_INTERNAL_SPDIF_IN_OPTICAL,
    AUDINPORT_INTERNAL_SPDIF_IN_COAXIAL,
    AUDINPORT_INTERNAL_SPDIF_IN_ARC,
    AUDINPORT_EXTERNAL_SPDIF_IN_0,
    AUDINPORT_EXTERNAL_SPDIF_IN_1,
    AUDINPORT_EXTERNAL_SPDIF_IN_2,
    AUDINPORT_EXTERNAL_SPDIF_IN_3,
    AUDINPORT_EXTERNAL_SPDIF_IN_4,
    AUDINPORT_EXTERNAL_SPDIF_IN_5,
    AUDINPORT_EXTERNAL_SPDIF_IN_6,
    AUDINPORT_EXTERNAL_SPDIF_IN_7,
    AUDINPORT_LINE_IN_0,
    AUDINPORT_LINE_IN_1,
    AUDINPORT_LINE_IN_2,
    AUDINPORT_LINE_IN_3,
    AUDINPORT_LINE_IN_4,
    AUDINPORT_LINE_IN_5,
    AUDINPORT_LINE_IN_6,
    AUDINPORT_LINE_IN_7,
    AUDINPORT_INTERNAL_HDMI_RX,
    AUDIN_NONE
} AUDIO_IN_TYPE_T;

typedef enum
{
  AUDMHL_IN = 0x00,
  AUDMHL_NONE 
} AUDMHL_IN_TYPE;

typedef enum
{
   AUDMHL_IN_OFF = 0x00,
   AUDMHL_IN_ON  = 0x01
} AUDMHL_IN_ONOFF;

typedef struct _AUDMHL_SET_T
{
  AUDIO_IN_TYPE_T u1Input;  // Audio-In input selection
  AUDMHL_IN_ONOFF fgOnOff;  // Audio-In On/Off selection
  AUDMHL_IN_TYPE  eAudmhlInType;
  AUDIN_DIGITAL_DETECT eAudioDigiDet;  // Audio-in DIR input Detect
} AUDMHL_SET_T;

#define AUDIN_CMD_PRI_HIGH 1
#define AUDIN_CMD_PRI_LOW  2

#if 0
typedef enum _AUDIN_COMP_ID_T
{
    AUDIN_COMP_1,
    AUDIN_COMP_NS
}AUDIN_COMP_ID_T;
#endif

typedef struct _AIN_CFG_T
{
    u8 uFormat;
    u8 uBits;
    u8 uCycle;
    bool fgIsSPDIFin;
    bool fgLrckInv;
}AIN_CFG_T;


//audio 2/4/6/8 channels for line in module.
#define AINACK_CFG_CH_NUM_MASK  ((u32)0x3<< 0)
#define AINACK_CFG_CH_NUM_2     ((u32)0x0<< 0)    
#define AINACK_CFG_CH_NUM_4     ((u32)0x1<< 0)
#define AINACK_CFG_CH_NUM_6     ((u32)0x2<< 0)
#define AINACK_CFG_CH_NUM_8     ((u32)0x3<< 0)


#define AUDIO_IN_INT_PERIOD (64*4)     // Set 0x53EC[5:4]  for 32/64/128/256 double words ,256 bytes
#define INT_TIME_SLOT        6  // Number of interrupt period for one slot , 8k
//#define INT_TIME_SLOT     33   // Number of interrupt period for one slot , 8k
#define HDMI_INT_TIME_SLOT  48 // Number of interrupt period for one slot , 8k
//#define HDMI_INT_TIME_SLOT 66 // Number of interrupt period for one slot , 8k
#define HDMI_AUDIO_IN_SLOT_SIZE 0x1000

#define AIN_MUTLI_16BIT (0x0F<<8)   //16bits
#define AIN_MUTLI_24BIT (0x17<<8)   //24bits

#define AIN_MUTLI_FMT_RJ                (0 << 13)        // Right aligned with LRCK 
#define AIN_MUTLI_FMT_LJ                (1 << 13)        // Left aligned with LRCK
#define AIN_MUTLI_FMT_I2S               (3 << 13)        // I2S interface
#define AIN_MUTLI_LRCK_INV              (1 << 15)        // Invert LRCK for multiple line in
#define AIN_MUTLI_LRCK_CYC_16           (0 << 16)        // LRCK selection 16
#define AIN_MUTLI_LRCK_CYC_24           (1 << 16)        // LRCK selection 24
#define AIN_MUTLI_LRCK_CYC_32           (2 << 16)        // LRCK selection 32


#endif

#endif
