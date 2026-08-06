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
 *Date: 2017-03-29
 */
#include "cp_if.h"
#include "cp_reg.h"
#include "cp_log.h"

#ifdef __ARM2__
unsigned int _u4CP_DBG_LVL = CP_LOG_LVL_HAL;
#else
unsigned int _u4CP_DBG_LVL = CP_LOG_LVL_DBG;
#endif
unsigned char *_pcCpLogLevel[] = {
	 "[CP] [ERR]",
	 "[CP] [WARN]",
	 "[CP] [INFO]",
	 "[CP] [HAL]",
	 "[CP] [DBG]",
	 "[CP] [IRQ]",
};

unsigned long _IO_BASE_ = 0x10000000;/*init for arm2*/


/*
 *             function : set hue for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
                              u4Hue, the value you want to set.
 *output parameter : void.
 *                return : void.
 */
void VcpSetHue(u32 u4VcpIdx, u32 u4Hue)
{
    if (u4Hue < 0 || u4Hue > 0x3f) {
        CP_LOG(CP_LOG_LVL_DBG, "VcpSetHue invalid hue value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
	    vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,
				 u4Hue << HUE_DEGREE_SHF, HUE_DEGREE);
    } else {

    }
    CP_LOG(CP_LOG_LVL_DBG, "VcpSetHue:Set SCE Global Hue = 0x%x\n", (unsigned int)u4Hue);    
}
EXPORT_SYMBOL(VcpSetHue);

/*
 *             function : set ygain for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
                              u4YGain, the value you want to set.
 *output parameter : void.
 *                return : void.
 */
void VcpSetYGain(u32 u4VcpIdx, u32 u4YGain)
{
    if (u4YGain < 0 || u4YGain > 0x1ff) {
        CP_LOG(CP_LOG_LVL_DBG, "VcpSetYGain invalid y gain value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
	    vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_Y, u4YGain << MLC_GAIN_Y_SHF,
                   MLC_GAIN_Y);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN,
                   MLC_GAIN_Y_EN);
    } else {

    }
    CP_LOG(CP_LOG_LVL_DBG, "VcpSetYGain:Set YGain = 0x%x\n", u4YGain);    
}
EXPORT_SYMBOL(VcpSetYGain);

/*
 *             function : set ugain for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
                              u4uGain, the value you want to set.
 *output parameter : void.
 *                return : void.
 */
void VcpSetUGain(u32 u4VcpIdx, u32 u4UGain)
{
    if (u4UGain < 0 || u4UGain > 0x1ff) {
        CP_LOG(CP_LOG_LVL_DBG, "VcpSetUGain invalid u gain value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, u4UGain << MLC_GAIN_U_SHF,
                   MLC_GAIN_U);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN, MLC_GAIN_U_EN);
    } else {

    }
    CP_LOG(CP_LOG_LVL_DBG, "VcpSetUGain:Set UGain = 0x%x\n", u4UGain);    
}
EXPORT_SYMBOL(VcpSetUGain);

/*
 *             function : set Vgain for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
                              u4VGain, the value you want to set.
 *output parameter : void.
 *                return : void.
 */
void VcpSetVGain(u32 u4VcpIdx, u32 u4VGain)
{
    if (u4VGain < 0 || u4VGain > 0x1ff) {
        CP_LOG(CP_LOG_LVL_DBG, "VcpSetVGain invalid v gain value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, u4VGain << MLC_GAIN_V_SHF,
                   MLC_GAIN_V);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN, MLC_GAIN_V_EN);
    } else {

    }
    CP_LOG(CP_LOG_LVL_DBG, "VcpSetVGain:Set VGain = 0x%x\n", u4VGain);    
}
EXPORT_SYMBOL(VcpSetVGain);

/*
 *             function : set contrast for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
                              u4Contrast, the value you want to set.
 *output parameter : void.
 *                return : void.
 */
void VcpSetContrast(u32 u4VcpIdx, u32 u4Contrast)
{
    if (u4Contrast < 0 || u4Contrast > 0xff) {
        CP_LOG(CP_LOG_LVL_DBG, "VcpSetContrast invalid contrast value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_BRIGHT_CONT,
                 (u4Contrast << CONTRAST_GAIN_SHF), CONTRAST_GAIN);
    } else {

    }
    CP_LOG(CP_LOG_LVL_DBG, "VcpSetContrast:set contrast = 0x%x\n", u4Contrast);    
}
EXPORT_SYMBOL(VcpSetContrast);

/*
 *             function : set brightness for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
                              u4Brightness, the value you want to set.
 *output parameter : void.
 *                return : void.
 */
void VcpSetBrightness(u32 u4VcpIdx, u32 u4Brightness)
{
    if (u4Brightness < 0 || u4Brightness > 0xff) {
        CP_LOG(CP_LOG_LVL_DBG, "VcpSetBrightness invalid brightness value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_BRIGHT_CONT,
                 (u4Brightness << BRIGHT_GAIN_SHF), BRIGHT_GAIN);
    } else {

    }
    CP_LOG(CP_LOG_LVL_DBG, "VcpSetBrightness:set Brightness = 0x%x\n", u4Brightness);    
}
EXPORT_SYMBOL(VcpSetBrightness);

/*
 *             function : set saturation for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
                              u4Saturation, the value you want to set.
 *output parameter : void.
 *                return : void.
 */
void VcpSetSaturation(u32 u4VcpIdx, u32 u4Saturation)
{
    if (u4Saturation < 0 || u4Saturation > 0xff) {
        CP_LOG(CP_LOG_LVL_DBG, "VcpSetSaturation invalid saturation value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_SATURATION, (u4Saturation << SAT_GAIN_SHF),
                 SAT_GAIN);
    } else {

    }
    CP_LOG(CP_LOG_LVL_DBG, "VcpSetSaturation:set saturation = 0x%x\n", u4Saturation);    
}
EXPORT_SYMBOL(VcpSetSaturation);

