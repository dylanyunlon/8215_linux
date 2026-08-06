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

#include "aud_oal.h"
#include "x_debug.h"
#include <mach/base_regs.h>

#include <media/atc/drv_av_d.h>
#include <media/atc/drv_aud.h>

#include "aud_extdac.h"

#include "aud_3360_reg_rw.h"

#include "aud_pwmdac.h"
#include "aud_clock.h"
#include "aud_debug.h"
#include "aud_if.h"


s32 Aud_SoutSourceSwitch(AUD_DAC_CLASS_T eDacType, AUD_OUT_TYPE_T eSource)
{
    s32 i4RetVal = AUD_OK;

    if(eDacType == AUD_FRONT_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_AOUT_CFG2,
                           AUD_FRNT_SRC_SEL_BIT_START,
                           AUD_FRNT_SRC_SEL_BIT_NUM,
                           eSource);
        if (eSource == AUD_AOUT1)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Front Ext DAC select Aout1. \n"));
            AUD_REG_BITS_WRITE(DVD_AUD_MCLK_SEL, 28, 1, 0); //Front DAC I2C clock use AOUT 1 Clock.
        }
        else if (eSource == AUD_AOUT2)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Front Ext DAC select Aout2. \n"));
            AUD_REG_BITS_WRITE(DVD_AUD_MCLK_SEL, 28, 1, 1); //Front DAC I2C clock use AOUT 2 Clock.
        }
        else if (eSource == AUD_DVD_OUT)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Front Ext DAC select DVD. \n"));
        }
    }
    else if (eDacType == AUD_REAR_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_AOUT_CFG2,
                           AUD_REAR_SRC_SEL_BIT_START,
                           AUD_REAR_SRC_SEL_BIT_NUM,
                           eSource);
        if (eSource == AUD_AOUT1)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Rear Ext DAC select Aout1. \n"));
            AUD_REG_BITS_WRITE(DVD_AUD_MCLK_SEL, 29, 1, 1); //Rear DAC I2C clock use AOUT 1 Clock.
        }
        else if (eSource == AUD_AOUT2)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Rear Ext DAC select Aout2. \n"));
            AUD_REG_BITS_WRITE(DVD_AUD_MCLK_SEL, 29, 1, 0); //Rear DAC I2C clock use AOUT 1 Clock.
        }
        else if (eSource == AUD_DVD_OUT)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Rear Ext DAC select DVD. \n"));
        }
    }
    else if (eDacType == AUD_GPS_DAC)
    {
        // GPS DAC no select option, it's connect fixed.
        i4RetVal = AUD_FAIL;
    }

    return (i4RetVal);
}


void Aud_RearMultiSel(AUD_REAR_GROUP_TYPE_E eType)
{
    // gpio4/gpio5 + gpio6/gpio7
    // i2s_out2_d_bck_sel, 0x54[17:15]=1, 
    // i2s_out2_m_lr_sel,  0x54[14:12]=1,
    // ts_d1 / ts_d2
    // i2s_out2_d_bck_sel, 0x54[17:15]=2, 
    // i2s_out2_m_lr_sel,  0x54[14:12]=2,
    // gpio53 / gpio54
    // i2s_out2_d_bck_sel, 0x54[17:15]=3, 
    // i2s_out2_m_lr_sel,  0x54[14:12]=3,
    // vin0 / vin1
    // i2s_out2_d_bck_sel, 0x54[17:15]=4, 
    // i2s_out2_m_lr_sel,  0x54[14:12]=4,
    // ain0_r / ain0_l
    // i2s_out2_d_bck_sel, 0x54[17:15]=5, 
    // i2s_out2_m_lr_sel,  0x54[14:12]=5,
    // gpio28 / gpio29
    // i2s_out2_d_bck_sel, 0x54[17:15]=6, 
    // i2s_out2_m_lr_sel,  0x54[14:12]=6,
    // ar4 / al4
    // i2s_out2_d_bck_sel, 0x54[17:15]=7, 
    // i2s_out2_m_lr_sel,  0x54[14:12]=7,
    if (eType == AUD_REAR_GROUP_4)
    {
        CKGEN_WRITE32_MASK(0x330, 0x3 << 8, 0x3 << 8);
        CKGEN_WRITE32_MASK(0x338, 0x3 << 8, 0x3 << 8);
    }
    CKGEN_WRITE32_MASK(0x24, 0x1 << 15, 0x1 << 15);
    CKGEN_WRITE32_MASK(0x1c, 0x17 << 24, 0xff << 24);
    CKGEN_WRITE32_MASK(0x18, 0x1 << 0, 0x3 << 0);
    CKGEN_WRITE32_MASK(0xa4, 0x1 << 28, 0x1 << 28);

    CKGEN_WRITE32_MASK(REG_PADMUX0, eType << 15, 0x7 << 15);
    CKGEN_WRITE32_MASK(REG_PADMUX0, eType << 12, 0x7 << 12);
    
}


static void Aud_GPSMultiSel(AUD_GPS_GROUP_TYPE_E eType)
{
    u32 u4M_LR;
    u32 u4D_BCK;
    
    switch (eType)
    {
    case AUD_GPS_GROUP_0:
        u4M_LR  = 1;
        u4D_BCK = 1;
        break;
    case AUD_GPS_GROUP_1:
        u4M_LR  = 2;
        u4D_BCK = 2;
        break;
    case AUD_GPS_GROUP_2:
        u4M_LR  = 3;
        u4D_BCK = 1;
        break;
    case AUD_GPS_GROUP_3:
        u4M_LR  = 3;
        u4D_BCK = 2;
        break;
    case AUD_GPS_GROUP_4:
        u4M_LR  = 3;
        u4D_BCK = 3;
        break;
    case AUD_GPS_GROUP_5:
        u4M_LR  = 3;
        u4D_BCK = 4;
        break;
    case AUD_GPS_GROUP_6:
        u4M_LR  = 4;
        u4D_BCK = 3;
        break;
    case AUD_GPS_GROUP_7:
        u4M_LR  = 5;
        u4D_BCK = 4;
        break;

    default:
        LOG(LOG_FAIL, TEXT("Error Parameter of GPS MultiFunction Select.\n"));
        return ;
    }

    CKGEN_WRITE32_MASK(0x330, 0x1 << 8, 0x1 << 8);
    CKGEN_WRITE32_MASK(0x338, 0x1 << 8, 0x1 << 8);

    CKGEN_WRITE32_MASK(REG_PADMUX0, 0x0 << 22, 0x1 << 22); // disable other multi func.
    CKGEN_WRITE32_MASK(REG_PADMUX0, 0x0 << 12, 0x7 << 12); // disable other multi func.

    CKGEN_WRITE32_MASK(REG_PADMUX6, u4M_LR << 3, 0x7 << 3);
    CKGEN_WRITE32_MASK(REG_PADMUX6, u4D_BCK << 6, 0x7 << 6);
}


s32 Aud_ExtDAC_MultiFuncSel(AUD_DAC_CLASS_T eDACCls, bool fgSel)
{
    if (fgSel)
    {
        if (eDACCls == AUD_FRONT_DAC)
        {
            // Multi function select: 1--(ar2/al2/ar1/al1/ar0/al0)
            CKGEN_SETBITS(REG_PADMUX2, 0x1 << 29);
        }
        else if (eDACCls == AUD_GPS_DAC)
        {
            Aud_GPSMultiSel(AUD_GPS_GROUP_6); // default setting for m1v1
        }
        else if (eDACCls == AUD_REAR_DAC)
        {
            Aud_RearMultiSel(AUD_REAR_GROUP_6); // default setting for m1v1
        }
    }
    else
    {
        if (eDACCls == AUD_FRONT_DAC)
        {
            // Multi function select: 1--(ar2/al2/ar1/al1/ar0/al0)
            CKGEN_CLRBITS(REG_PADMUX2, 0x1 << 29);
        }
    }

    return (AUD_OK);
}


s32 Aud_ExtDAC_SDATASelect(AUD_DAC_CLASS_T eDacCls, AUD_OUT_TYPE_T eSource)
{
    if (eDacCls == AUD_FRONT_DAC)
    {
        if (eSource == AUD_AOUT1)
        {
            AUD_REG_BITS_WRITE(0x50C8, 4, 3, AUD_CH_1_2);
            AUD_REG_BITS_WRITE(0x50C8, 7, 3, AUD_CH_3_4);
            AUD_REG_BITS_WRITE(0x50C8, 10, 3, AUD_CH_5_6);
            // Follow 2 lines code for Verify Test.
            //AUD_REG_BITS_WRITE(0x50C8, 7, 3, AUD_CH_1_2);
            //AUD_REG_BITS_WRITE(0x50C8, 10, 3, AUD_CH_1_2);
            LOG(LOG_DAC, TEXT("[AUD_CFG] Aout1 SDATA to Front. \n"));
        }
        else if (eSource == AUD_AOUT2)
        {
            AUD_REG_BITS_WRITE(0x50C8, 22, 2, AUD_CH_1_2);
            AUD_REG_BITS_WRITE(0x50C8, 24, 2, AUD_CH_1_2);
            AUD_REG_BITS_WRITE(0x50C8, 26, 2, AUD_CH_1_2);
            LOG(LOG_DAC, TEXT("[AUD_CFG] Aout2 SDATA to Front. \n"));
        }
    }
    else if (eDacCls == AUD_REAR_DAC)
    {
        if (eSource == AUD_AOUT1)
        {
            AUD_REG_BITS_WRITE(0x50C8, 16, 3, AUD_CH_1_2);
            LOG(LOG_DAC, TEXT("[AUD_CFG] Aout1 SDATA to Rear. \n"));
        }
        else if (eSource == AUD_AOUT2)
        {
            AUD_REG_BITS_WRITE(0x50C8, 28, 2, AUD_CH_1_2);
            LOG(LOG_DAC, TEXT("[AUD_CFG] Aout2 SDATA to Rear. \n"));
        }
    }   
    
    return (AUD_OK);
}

