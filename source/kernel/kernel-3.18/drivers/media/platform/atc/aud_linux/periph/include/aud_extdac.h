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
//#include "x_typedef.h"
#include <linux/types.h>

#include "aud_pwmdac.h"

#ifndef __AUD_EXTDAC_H_
#define __AUD_EXTDAC_H_

/*
From CS4341A Spec :
In the I2C mode, data is clocked into and out of the bi-directional serial control data line, SDA, by
the serial control port clock, SCL (see Figure 6 for the clock to data relationship). There is no CS
pin. Pin AD0 enables the user to alter the chip address (001000[AD0][R/W]) and should be tied to
VA or GND as required, before powering up the device. If the device ever detects a high to low
transition on the AD0/CS pin after power-up, SPI mode will be selected.
*/
#define REAR_CS4341_DEVID               0x10
#define GPS_CS4341_DEVID                0x11


// Rear External DAC Multi-Function Select
typedef enum AUD_REAR_GROUP_TYPE{
    AUD_REAR_GROUP_0 = 1,   // 1 + 1 // mclk:gpio4, sdata:gpio6, lrck:gpio5, bck:gpio7
    AUD_REAR_GROUP_1,   // 2 + 2        //  mclk:ts_d1, sdata:ts_d3, lrck:ts_d2, bck:ts_d4
    AUD_REAR_GROUP_2,   // 3 + 3        // mclk:gpio53, sdata:gpio55, lrck:gpio54, bck:gpio56
    AUD_REAR_GROUP_3,   // 4 + 4        // mclk:vin0, sdata:vin2, lrck:vin1, bck:vin3
    AUD_REAR_GROUP_4,   // 5 + 5        // mclk:ain0_r, sdata:ain1_r, lrck:ain0_1, bck:ain1_1
    AUD_REAR_GROUP_5,   // 6 + 6        // mclk:gpio28, sdata:gpio30, lrck:gpio29, bck:gpio31
    AUD_REAR_GROUP_6,   // 7 + 7        // mclk:ar4, sdata:ar3, lrck:al4, bck:al3
}AUD_REAR_GROUP_TYPE_E;


// GPS External DAC Multi-Function Select
typedef enum AUD_GPS_GROUP_TYPE{
    AUD_GPS_GROUP_0,   // 1 + 1
    AUD_GPS_GROUP_1,   // 2 + 2
    AUD_GPS_GROUP_2,   // 3 + 1
    AUD_GPS_GROUP_3,   // 3 + 2
    AUD_GPS_GROUP_4,   // 3 + 3
    AUD_GPS_GROUP_5,   // 3 + 4
    AUD_GPS_GROUP_6,   // 4 + 3
    AUD_GPS_GROUP_7,   // 5 + 4
}AUD_GPS_GROUP_TYPE_E;

extern s32 Aud_SoutSourceSwitch(AUD_DAC_CLASS_T eDacType, AUD_OUT_TYPE_T eSource);
extern s32 Aud_ExtDAC_MultiFuncSel(AUD_DAC_CLASS_T eDACCls, bool fgSel);
extern s32 Aud_ExtDAC_SDATASelect(AUD_DAC_CLASS_T eDacCls, AUD_OUT_TYPE_T eSource);
extern void Aud_RearMultiSel(AUD_REAR_GROUP_TYPE_E eType);

#endif // __AUD_EXTDAC_H_

