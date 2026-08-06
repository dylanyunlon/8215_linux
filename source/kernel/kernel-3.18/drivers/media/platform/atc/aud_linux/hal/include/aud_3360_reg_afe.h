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



#ifndef _AUDIO_3360_REG_AFE_H_
#define _AUDIO_3360_REG_AFE_H_

/////////////////////////////////////////////////////////////////////////////
//                   AFE1 Register

/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_AFE1_BASE                      (0xA8200)

#define AUD_REG_AFE1_CFG0                      (AUD_REG_AFE1_BASE + 0x84)
#define AUD_REG_AFE1_CFG1                      (AUD_REG_AFE1_BASE + 0x88)
#define AUD_REG_AFE1_CFG2                      (AUD_REG_AFE1_BASE + 0x8C)
#define AUD_REG_AFE1_CFG3                      (AUD_REG_AFE1_BASE + 0x90)
#define AUD_REG_AFE1_CFG4                      (AUD_REG_AFE1_BASE + 0x94)
#define AUD_REG_AFE1_CFG5                      (AUD_REG_AFE1_BASE + 0x98)
#define AUD_REG_AFE1_CFG6                      (AUD_REG_AFE1_BASE + 0x9C)
#define AUD_REG_AFE1_CFG7                      (AUD_REG_AFE1_BASE + 0xA0)
#define AUD_REG_AFE1_CFG8                      (AUD_REG_AFE1_BASE + 0xA4)
#define AUD_REG_AFE1_CFG9                      (AUD_REG_AFE1_BASE + 0xA8)
#define AUD_REG_AFE1_CFG10                     (AUD_REG_AFE1_BASE + 0xAC)
#define AUD_REG_AFE1_CFG11                     (AUD_REG_AFE1_BASE + 0xB0)
#define AUD_REG_AFE1_CFG12                     (AUD_REG_AFE1_BASE + 0xB4)
#define AUD_REG_AFE1_CFG13                     (AUD_REG_AFE1_BASE + 0xB8)
#define AUD_REG_AFE1_CFG14                     (AUD_REG_AFE1_BASE + 0xBC)
#define AUD_REG_AFE1_CFG15                     (AUD_REG_AFE1_BASE + 0xC0)
#define AUD_REG_AFE1_CFG16                     (AUD_REG_AFE1_BASE + 0xC4)
#define AUD_REG_AFE1_CFG17                     (AUD_REG_AFE1_BASE + 0xC8)
#define AUD_REG_AFE1_CFG18                     (AUD_REG_AFE1_BASE + 0xCC)
#define AUD_REG_AFE1_CFG19                     (AUD_REG_AFE1_BASE + 0xD0)
#define AUD_REG_AFE1_CFG20                     (AUD_REG_AFE1_BASE + 0xD4)
#define AUD_REG_AFE1_CFG21                     (AUD_REG_AFE1_BASE + 0xD8)
#define AUD_REG_AFE1_CFG22                     (AUD_REG_AFE1_BASE + 0xDC)

/////////////////////////////////////////////////////////////////////////////
//                   AFE2 Register
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_AFE2_BASE                      (0xA8300)

#define AUD_REG_AFE2_CFG0                      (AUD_REG_AFE2_BASE + 0x04)
#define AUD_REG_AFE2_CFG1                      (AUD_REG_AFE2_BASE + 0x08)
#define AUD_REG_AFE2_CFG2                      (AUD_REG_AFE2_BASE + 0x0C)
#define AUD_REG_AFE2_CFG3                      (AUD_REG_AFE2_BASE + 0x10)
#define AUD_REG_AFE2_CFG4                      (AUD_REG_AFE2_BASE + 0x14)
#define AUD_REG_AFE2_CFG5                      (AUD_REG_AFE2_BASE + 0x18)
#define AUD_REG_AFE2_CFG6                      (AUD_REG_AFE2_BASE + 0x1C)
#define AUD_REG_AFE2_CFG7                      (AUD_REG_AFE2_BASE + 0x20)
#define AUD_REG_AFE2_CFG8                      (AUD_REG_AFE2_BASE + 0x24)
#define AUD_REG_AFE2_CFG9                      (AUD_REG_AFE2_BASE + 0x28)
#define AUD_REG_AFE2_CFG10                     (AUD_REG_AFE2_BASE + 0x2C)
#define AUD_REG_AFE2_CFG11                     (AUD_REG_AFE2_BASE + 0x30)
#define AUD_REG_AFE2_CFG12                     (AUD_REG_AFE2_BASE + 0x34)
#define AUD_REG_AFE2_CFG13                     (AUD_REG_AFE2_BASE + 0x38)
#define AUD_REG_AFE2_CFG14                     (AUD_REG_AFE2_BASE + 0x3C)
#define AUD_REG_AFE2_CFG15                     (AUD_REG_AFE2_BASE + 0x40)
#define AUD_REG_AFE2_CFG16                     (AUD_REG_AFE2_BASE + 0x44)
#define AUD_REG_AFE2_CFG17                     (AUD_REG_AFE2_BASE + 0x48)
#define AUD_REG_AFE2_CFG18                     (AUD_REG_AFE2_BASE + 0x4C)
#define AUD_REG_AFE2_CFG19                     (AUD_REG_AFE2_BASE + 0x50)
#define AUD_REG_AFE2_CFG20                     (AUD_REG_AFE2_BASE + 0x54)
#define AUD_REG_AFE2_CFG21                     (AUD_REG_AFE2_BASE + 0x58)
#define AUD_REG_AFE2_CFG22                     (AUD_REG_AFE2_BASE + 0x5C)

/////////////////////////////////////////////////////////////////////////////
//                   AUADC Register
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_AUADC_BASE                     (0x300)

#define AUD_REG_AADC_CFG0                      (AUD_REG_AUADC_BASE + 0x00)
#define AUD_REG_AADC_CFG1                      (AUD_REG_AUADC_BASE + 0x04)
#define AUD_REG_AADC_CFG2                      (AUD_REG_AUADC_BASE + 0x08)
#define AUD_REG_AADC_CFG3                      (AUD_REG_AUADC_BASE + 0x0C)
#define AUD_REG_AADC_CFG4                      (AUD_REG_AUADC_BASE + 0x10)
#define AUD_REG_AADC_CFG5                      (AUD_REG_AUADC_BASE + 0x14)
#define AUD_REG_AADC_CFG6                      (AUD_REG_AUADC_BASE + 0x18)
#define AUD_REG_AADC_CFG7                      (AUD_REG_AUADC_BASE + 0x1C)
#define AUD_REG_AADC_CFG8                      (AUD_REG_AUADC_BASE + 0x20)
#define AUD_REG_AADC_CFG9                      (AUD_REG_AUADC_BASE + 0x24)
#define AUD_REG_AADC_CFG10                     (AUD_REG_AUADC_BASE + 0x28)
#define AUD_REG_AADC_CFG11                     (AUD_REG_AUADC_BASE + 0x2C)
#define AUD_REG_AADC_CFG12                     (AUD_REG_AUADC_BASE + 0x30)
#define AUD_REG_AADC_CFG13                     (AUD_REG_AUADC_BASE + 0x34)
#define AUD_REG_AADC_CFG14                     (AUD_REG_AUADC_BASE + 0x38)
#define AUD_REG_AADC_CFG15                     (AUD_REG_AUADC_BASE + 0x3C)
#define AUD_REG_AADC_CFG16                     (AUD_REG_AUADC_BASE + 0x40)
#define AUD_REG_AADC_RO                        (AUD_REG_AUADC_BASE + 0x44)



#endif // #ifndef _AUDIO_3360_REG_AFE_H_


