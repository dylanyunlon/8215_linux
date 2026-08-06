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
/**
* Copyright (C) 2013 AUTOCHIPS Incorporation. All Rights Reserved.
* @file:			cp.c
* Description:	 color process HW Setting
* Others:
* @author:		  atc_sd_sd2
* History:    [2015.11.3]
*/
#ifndef VCP_FOR_ANDROID

#ifndef __ARM2__
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <media/atc/cp.h>
#include <media/atc/display.h>
#include "x_debug.h"
#include "x_os.h"
#include "x_stl_lib.h"
#else
/*
#include "aud_comm.h"
*/
#include "x_types.h"
#include "cp.h"
#include "display.h"
#endif
#include "x_ckgen.h"
#include "x_assert.h"
#include "x_typedef.h"
#include "x_lint.h"
#include "cp_reg.h"
#include "cp_def.h"
#include "x_printf.h"

#define DEFINE_IS_LOG	CLI_IsLog

u32 _u4VCP_DBG_LVL = (u32)VCP_LOG_LVL_HAL;
u8 *_pcVcpLogLevel[] = {
	"OFF",
	"ERR",
	"WARN",
	"CLI",
	"INFO",
	"HAL",
	"IRQ",
	"TRACE",
	"DBG",
	"REGRW",
};

#if 0
#ifdef __ARM2__
enum {
	false	= 0,
	true	= 1
};
#endif
#endif
#define FRONT   0   /* refer to front row display */
#define REAR    1   /* refer to rear row display */

#define vColorProcessEnable(dVal)  vWriteReg(0x1F080, dVal)

#ifndef __ARM2__
void vWriteCP(u32 dAddr, u32 dVal)
{
    *(volatile u32*)(vcp_sysreg_base + dAddr) = dVal;
}
u32 dReadCP(u32 dAddr)
{
    return *(volatile u32*)(vcp_sysreg_base + dAddr);
}
void vWriteCPMsk(u32 dAddr, u32 dVal, u32 dMsk)
{
    vWriteCP(dAddr, (dReadCP(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)));
}
#else
#define FRONT_CP_REG_ADDR   ( IO_BASE_ADDRESS + 0x42600 )
#define vWriteCP(dAddr, dVal)  *(volatile u32 *)(FRONT_CP_REG_ADDR + dAddr) = dVal
#define dReadCP(dAddr)         *(volatile u32 *)(FRONT_CP_REG_ADDR + dAddr)
#define vWriteCPMsk(dAddr, dVal, dMsk) vWriteCP((dAddr), (dReadCP(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))
#endif

/*
 *             function : set cp module to be on or off.
 *  input parameter : u4VcpIdx, refer to front or rear display;
                              fgOnOrOff, when true means on /when false means off .
 *output parameter : void.
 *                return : true means ok,false for failed.
 */
bool VcpOnOff(u32 u4VcpIdx, bool fgOnOrOff)
{
    if (FRONT == u4VcpIdx) {
        if (true == fgOnOrOff) {
	        VCP_LOG(VCP_LOG_LVL_DBG, "Color Process on:\n");
        } else {
            VCP_LOG(VCP_LOG_LVL_DBG, "Color Process off:\n");
        }
        vColorProcessEnable((unsigned long)fgOnOrOff);
    } else if (REAR == u4VcpIdx) {
        if (true == fgOnOrOff) {
	        VCP_LOG(VCP_LOG_LVL_DBG, "Color Process on:\n");
        } else {
            VCP_LOG(VCP_LOG_LVL_DBG, "Color Process off:\n");
        }
    } else {
        VCP_LOG(VCP_LOG_LVL_ERR, "wrong display ID:%d \n", (int)u4VcpIdx);
        return false;
    }

    return true;
}
EXPORT_SYMBOL(VcpOnOff);

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
        VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetHue invalid hue value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
	    vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,
				 u4Hue << HUE_DEGREE_SHF, HUE_DEGREE);	/* turn on gamma */
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetHue:Set SCE Global Hue = 0x%x\n", (unsigned int)u4Hue);    
}
EXPORT_SYMBOL(VcpSetHue);

/*
 *             function : get hue from cp.
 *  input parameter : u4VcpIdx, refer to front or rear display; 
 *output parameter : void.
 *                return : hue's value.
 */
u32 VcpGetHue(u32 u4VcpIdx)
{
	u32 u4Hue = 0;

    if (FRONT == u4VcpIdx) {
        u4Hue = dReadCP((unsigned int)RW_PCLRP_HUE_SCECTRL);
        u4Hue = ((u4Hue & HUE_DEGREE) >> HUE_DEGREE_SHF);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpGetHue:Get SCE Global Hue = 0x%x\n", u4Hue);

    return u4Hue;
}
EXPORT_SYMBOL(VcpGetHue);

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
        VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetYGain invalid y gain value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
	    vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_Y, u4YGain << MLC_GAIN_Y_SHF,
                   MLC_GAIN_Y);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN,
                   MLC_GAIN_Y_EN);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetYGain:Set YGain = 0x%x\n", u4YGain);    
}
EXPORT_SYMBOL(VcpSetYGain);

/*
 *             function : get ygain for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : return value is ygain.
 */
u32 VcpGetYGain(u32 u4VcpIdx)
{
    u32 u4ygain = 0;

    if (FRONT == u4VcpIdx) {
        u4ygain = dReadCP(RW_PCLRP_GAIN_Y);
        u4ygain = ((u4ygain & MLC_GAIN_Y) >> MLC_GAIN_Y_SHF);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpGetYGain:Get YGain = 0x%x\n", u4ygain);

    return u4ygain;
}
EXPORT_SYMBOL(VcpGetYGain);

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
        VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetUGain invalid u gain value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, u4UGain << MLC_GAIN_U_SHF,
                   MLC_GAIN_U);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN, MLC_GAIN_U_EN);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetUGain:Set UGain = 0x%x\n", u4UGain);    
}
EXPORT_SYMBOL(VcpSetUGain);

/*
 *             function : get ugain for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : return value is ugain.
 */
u32 VcpGetUGain(u32 u4VcpIdx)
{
    u32 u4ugain = 0;

    if (FRONT == u4VcpIdx) {
        u4ugain = dReadCP(RW_PCLRP_GAIN_UV);
        u4ugain = ((u4ugain & MLC_GAIN_U) >> MLC_GAIN_U_SHF);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpGetUGain:Get UGain = 0x%x\n", u4ugain);

    return u4ugain;
}
EXPORT_SYMBOL(VcpGetUGain);

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
        VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetVGain invalid v gain value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, u4VGain << MLC_GAIN_V_SHF,
                   MLC_GAIN_V);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN, MLC_GAIN_V_EN);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetVGain:Set VGain = 0x%x\n", u4VGain);    
}
EXPORT_SYMBOL(VcpSetVGain);

/*
 *             function : get vgain for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : return value is vgain.
 */
u32 VcpGetVGain(u32 u4VcpIdx)
{
    u32 u4vgain = 0;

    if (FRONT == u4VcpIdx) {
        u4vgain = dReadCP(RW_PCLRP_GAIN_UV);
        u4vgain = ((u4vgain & MLC_GAIN_V) >> MLC_GAIN_V_SHF);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpGetVGain:Get VGain = 0x%x\n", u4vgain);

    return u4vgain;
}
EXPORT_SYMBOL(VcpGetVGain);

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
        VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetContrast invalid contrast value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_BRIGHT_CONT,
                 (u4Contrast << CONTRAST_GAIN_SHF), CONTRAST_GAIN);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetContrast:set contrast = 0x%x\n", u4Contrast);    
}
EXPORT_SYMBOL(VcpSetContrast);

/*
 *             function : get contrast for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : return value is contrast.
 */
u32 VcpGetContrast(u32 u4VcpIdx)
{
    u32 u4contrast = 0;

    if (FRONT == u4VcpIdx) {
         u4contrast =
            ((dReadCP((unsigned int)RW_PCLRP_BRIGHT_CONT) & CONTRAST_GAIN) >>
             CONTRAST_GAIN_SHF);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpGetContrast:get contrast = 0x%x\n", u4contrast);

    return u4contrast;
}
EXPORT_SYMBOL(VcpGetContrast);

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
        VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetBrightness invalid brightness value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_BRIGHT_CONT,
                 (u4Brightness << BRIGHT_GAIN_SHF), BRIGHT_GAIN);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetBrightness:set Brightness = 0x%x\n", u4Brightness);    
}
EXPORT_SYMBOL(VcpSetBrightness);

/*
 *             function : get brightness for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : return value is brightness.
 */
u32 VcpGetBrightness(u32 u4VcpIdx)
{
    u32 u4brightness = 0;

    if (FRONT == u4VcpIdx) {
        u4brightness =
            ((dReadCP((unsigned int)RW_PCLRP_BRIGHT_CONT) & BRIGHT_GAIN) >>
             BRIGHT_GAIN_SHF);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpGetBrightness:get Brightness = 0x%x\n", u4brightness);

    return u4brightness;
}
EXPORT_SYMBOL(VcpGetBrightness);

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
        VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetSaturation invalid saturation value! \n");
        return;
    }

    if (FRONT == u4VcpIdx) {
        vWriteCPMsk((unsigned int)RW_PCLRP_SATURATION, (u4Saturation << SAT_GAIN_SHF),
                 SAT_GAIN);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpSetSaturation:set saturation = 0x%x\n", u4Saturation);    
}
EXPORT_SYMBOL(VcpSetSaturation);

/*
 *             function : get saturation for cp.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : return value is saturation.
 */
u32 VcpGetSaturation(u32 u4VcpIdx)
{
    u32 u4saturation = 0;

    if (FRONT == u4VcpIdx) {
        u4saturation =
            ((dReadCP((unsigned int)RW_PCLRP_SATURATION) & SAT_GAIN) >>
             SAT_GAIN_SHF);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG, "VcpGetSaturation:get saturation = 0x%x\n", u4saturation);

    return u4saturation;
}
EXPORT_SYMBOL(VcpGetSaturation);

#ifndef __ARM2__
/*
 *             function : enable cp's clock.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : void.
 */
void VcpEnableClk(u32 u4VcpIdx)
{
    if (FRONT == u4VcpIdx) {
		clk_prepare_enable(clk_ac8317_vcp);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG,"VcpEnableClk:open id = 0x%x\n", u4VcpIdx);
}
EXPORT_SYMBOL(VcpEnableClk);

/*
 *             function : disable cp's clock.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : void.
 */
void VcpDisableClk(u32 u4VcpIdx)
{
    if (FRONT == u4VcpIdx) {
		clk_disable_unprepare(clk_ac8317_vcp);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG,"VcpDisableClk:close id = 0x%x\n", u4VcpIdx);
}
EXPORT_SYMBOL(VcpDisableClk);

#else

/*
 *             function : enable cp's clock.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : void.
 */
void VcpEnableClk(u32 u4VcpIdx)
{
    if (FRONT == u4VcpIdx) {
		CKGEN_AgtOnClk(e_CLK_LCPROC_VDO);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG,"VcpEnableClk:open id = 0x%x\n", u4VcpIdx);
}

/*
 *             function : disable cp's clock.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : void.
 */
void VcpDisableClk(u32 u4VcpIdx)
{
    if (FRONT == u4VcpIdx) {
		CKGEN_AgtOffClk(e_CLK_LCPROC_VDO);
    } else {

    }
    VCP_LOG(VCP_LOG_LVL_DBG,"VcpDisableClk:close id = 0x%x\n", u4VcpIdx);
}
#endif

/*
 *             function : reset cp's display values.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : void.
 */
void VcpReset(u32 u4VcpIdx)
{
    if (FRONT == u4VcpIdx) {
        /* reset front hue */
        vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
				 0x20U << HUE_DEGREE_SHF, HUE_DEGREE);
        /* reset front yuv */
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_Y,
				 0x80 << MLC_GAIN_Y_SHF, MLC_GAIN_Y);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV,
				 0x5b << MLC_GAIN_U_SHF, MLC_GAIN_U);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV,
				 0x80 << MLC_GAIN_V_SHF, MLC_GAIN_V);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN,
				 MLC_GAIN_Y_EN);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN,
				 MLC_GAIN_U_EN);
        vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN,
				 MLC_GAIN_V_EN);
        /* reset front cbs */
        vWriteCPMsk((unsigned int)RW_PCLRP_BRIGHT_CONT,
				 0x40 << CONTRAST_GAIN_SHF, CONTRAST_GAIN);
        vWriteCPMsk((unsigned int)RW_PCLRP_BRIGHT_CONT,
				 0x80 << BRIGHT_GAIN_SHF, BRIGHT_GAIN);
        vWriteCPMsk((unsigned int)RW_PCLRP_SATURATION,
				 0x80 << SAT_GAIN_SHF, SAT_GAIN);
    } else {
        /* reset rear hue */

        /* reset rear yuv */

        /* reset rear cbs */
    }
    VCP_LOG(VCP_LOG_LVL_DBG,"reset vcp id = 0x%x\n", u4VcpIdx);
}
EXPORT_SYMBOL(VcpReset);

/*
 *             function : init cp module.
 *  input parameter : u4VcpIdx, refer to front or rear display;
 *output parameter : void.
 *                return : void.
 */
void VcpInit(u32 u4VcpIdx)
{
    if (FRONT == u4VcpIdx) {
        vColorProcessEnable(0x1);/* always open cp */

        VcpReset(u4VcpIdx);
    } else {

    }
}
EXPORT_SYMBOL(VcpInit);

u32 VcpTuneOperation(u32 u4VcpIdx, CP_TUNE_ITEM_ENUM Item, u32 u4Value)
{
	u32 return_value = 0;

	if (u4VcpIdx == FRONT) {
		switch (Item) {
		case CP_UNKNOW_TUNE_SEL:
		case CP_TUNE_MAX:
			VCP_LOG(VCP_LOG_LVL_DBG, "unknow CP tune item be selected!:\n");
			break;

		case CP_HUE_RESET:
			/* reset front hue */
			vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
				0x20U << HUE_DEGREE_SHF, HUE_DEGREE);
            break;

		case CP_HUE_SET:
			VcpSetHue(u4VcpIdx, u4Value);
			break;

		case CP_HUE_GET:
			return_value = VcpGetHue(u4VcpIdx);
			break;

		case CP_YGAIN_RESET:
			/* reset front ygain */
			vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_Y,
				0x80 << MLC_GAIN_Y_SHF, MLC_GAIN_Y);
			vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN,
				MLC_GAIN_Y_EN);
            break;

		case CP_YGAIN_SET:
			VcpSetYGain(u4VcpIdx, u4Value);
            break;

		case CP_YGAIN_GET:
			return_value = VcpGetYGain(u4VcpIdx);
			break;

		case CP_UGAIN_RESET:
			/* reset front ugain */
			vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV,
				0x5b << MLC_GAIN_U_SHF, MLC_GAIN_U);
			vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN,
				MLC_GAIN_U_EN);
			break;

		case CP_UGAIN_SET:
			VcpSetUGain(u4VcpIdx, u4Value);
			break;

		case CP_UGAIN_GET:
			return_value = VcpGetUGain(u4VcpIdx);
			break;

		case CP_VGAIN_RESET:
			/* reset front vgain */
			vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV,
				0x80 << MLC_GAIN_V_SHF, MLC_GAIN_V);
			vWriteCPMsk((unsigned int)RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN,
				MLC_GAIN_V_EN);
			break;

		case CP_VGAIN_SET:
			VcpSetVGain(u4VcpIdx, u4Value);
			break;

		case CP_VGAIN_GET:
			return_value = VcpGetVGain(u4VcpIdx);
			break;

		case CP_SATURATION_RESET:
			/* reset front saturation */
			vWriteCPMsk((unsigned int)RW_PCLRP_SATURATION,
				0x80 << SAT_GAIN_SHF, SAT_GAIN);
			break;

		case CP_SATURATION_SET:
			VcpSetSaturation(u4VcpIdx, u4Value);
			break;

		case CP_SATURATION_GET:
			return_value = VcpGetSaturation(u4VcpIdx);
			break;

		case CP_BRIGHTNESS_RESET:
			/* reset front brightness */
			vWriteCPMsk((unsigned int)RW_PCLRP_BRIGHT_CONT,
				0x80 << BRIGHT_GAIN_SHF, BRIGHT_GAIN);
			break;

		case CP_BRIGHTNESS_SET:
			VcpSetBrightness(u4VcpIdx, u4Value);
			break;

		case CP_BRIGHTNESS_GET:
			return_value = VcpGetBrightness(u4VcpIdx);
			break;

		case CP_CONTRAST_RESET:
			/* reset front contrast */
			vWriteCPMsk((unsigned int)RW_PCLRP_BRIGHT_CONT,
				0x40 << CONTRAST_GAIN_SHF, CONTRAST_GAIN);
			break;

		case CP_CONTRAST_SET:
			VcpSetContrast(u4VcpIdx, u4Value);
			break;

		case CP_CONTRAST_GET:
			return_value = VcpGetContrast(u4VcpIdx);
			break;
		}
	}else {
		VCP_LOG(VCP_LOG_LVL_DBG, "rear CP tune has not implement!:\n");
	}

	return return_value;
}
EXPORT_SYMBOL(VcpTuneOperation);

/*******************************************************************************************************/
/*                                          following code not be used currently                                                                             */
/*******************************************************************************************************/

/* Config GlobHuePrec PartialHuePrec UClamp VClamp */
void vCPSCEHueConfig(s32 i4GHuePrec, s32 i4PHuePrec, s32 i4UClamp, s32 i4VClamp)
{
	vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
			 (i4GHuePrec << HUE_GLOB_PREC_SHF) | (i4UClamp << SCE_UCLAMP_SHF) | (i4VClamp <<
											 SCE_VCLAMP_SHF),
			 HUE_GLOB_PREC | SCE_UCLAMP | SCE_VCLAMP);
	vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCE_CFG, i4PHuePrec, HUE_DG_PREC);
    VCP_LOG(VCP_LOG_LVL_INFO,
		 "Set SCE Hue Prec:(1:+/-28,0:+/-14)Global Hue:%d,Partial Hue:%d\n U Clamp:%d, VClamp:%d\n",
		 (int)i4GHuePrec, (int)i4GHuePrec, (int)i4UClamp, (int)i4VClamp);
}

void vCPSetSCE(const u32 * const pu4SCETableWrite)
{
	u32 dwData;
	u16 wCnt;
	u8 bCnt;

	vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
			 SRAM_WR_MODE, SRAM_WR_MODE | SRAM_RD_ENA);	/* SEC sram write mode */
	for (wCnt = 0; wCnt <= (u16)359; wCnt++) {
		if (pu4SCETableWrite == NULL)
			dwData = (_pdCPTestSCETable[wCnt] << 10U) | (u32)(wCnt << 1U);
		else
			dwData = ((*(pu4SCETableWrite + wCnt)) << 10U) | (u32)(wCnt << 1U);

		/* dwData = (0x200080<<10) |(wCnt<<1); // */
		vWriteCP((unsigned int)RW_PCLRP_SCE_TABLE,
			 dwData | 0x01U);	/* write vector and write bit */
		for (bCnt = 0; bCnt <= 10U; bCnt++) {	/* delay?? */
			dReadCP(0x0cU);
		}
		vWriteCP((unsigned int)RW_PCLRP_SCE_TABLE,
			 dwData | 0x00);	/* write vector and clear write bit */
		for (bCnt = 0; bCnt <= 10U; bCnt++) {	/* delay?? */
			dReadCP(0x0cU);
		}
        VCP_LOG(VCP_LOG_LVL_INFO, "[Write]SCE idx:%d (%d,%d,%d) Tab:0x%x\n",
				(int)wCnt, 0xFF & ((int)dwData >> 24),
				0x3F & ((int)dwData >> 18), 0xFF & ((int)dwData >> 10), (int)dwData >> 10);
	}
	vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,
		 MLC_SCE_ENA, SRAM_RD_ENA | MLC_SCE_ENA);	/* SEC enable, */
}

#if defined(CP_FEATURE_SCE_TAB_READ)
void vCPGetSCE(u32 * const pu4SCETableRead)
{
	u32 u4Index = 0;
	u32 u4Data = 0;
	u8 u1Luma = 0;
	u8 u1Hue = 0;
	u8 u1Sat = 0;

/* step 1 Disable SCE & Enable ReadMode */
	vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL, 0,
			 MLC_SCE_ENA | SRAM_RD_ENA);	/* SEC Disable & Read Enable, */
#ifndef __ARM2__
	mdelay((unsigned long)2);
#endif
/* step2 */
	for (u4Index = 0; u4Index < 360U; u4Index++) {
#ifndef __ARM2__
		mdelay(2U);
#endif
		vWriteCPMsk((unsigned int)RW_PCLRP_SCE_TABLE_READ,
			 (u4Index << MLC_SCE_READ_ADDR_SHF) | MLC_SCE_READ_ENA,
			 MLC_SCE_READ_ADDR | MLC_SCE_READ_ENA);	/* SEC enable, */
		/* mdelay(5); */
		/* vWriteCPMsk(RW_PCLRP_SCE_TABLE_READ, ~MLC_SCE_READ_ENA,MLC_SCE_READ_ENA);// SEC enable, */
#ifndef __ARM2__
		mdelay(2U);
#endif

		u4Data = dReadCP((unsigned int)RW_PCLRP_SCE_TABLE_READ);
#ifndef __ARM2__
		mdelay(2U);
#endif
		if (pu4SCETableRead != NULL)
			*(pu4SCETableRead + u4Index) = u4Data >> MLC_SCE_P_SAT_SHF;
		u1Luma = (u8)(u4Data & (u32)MLC_SCE_P_LUMA) >> (u32)MLC_SCE_P_LUMA_SHF;
		u1Hue = (u4Data & MLC_SCE_P_HUE) >> MLC_SCE_P_HUE_SHF;
		u1Sat = (u4Data & MLC_SCE_P_SAT) >> MLC_SCE_P_SAT_SHF;
        VCP_LOG(VCP_LOG_LVL_INFO, "[Read]SCE idx:%d(%d,%d,%d) Tab:0x%x Reg:0x%x\n",
			 (int)u4Index, (int)u1Luma, (int)u1Hue,
			 (int)u1Sat, (unsigned int)u4Data >> MLC_SCE_P_SAT_SHF, (unsigned int)u4Data);
	}
	vWriteCPMsk(RW_PCLRP_SCE_TABLE_READ,
		 ~MLC_SCE_READ_ENA, MLC_SCE_READ_ENA);	/* SEC enable, */
	/* vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,MLC_SCE_ENA,MLC_SCE_ENA|SRAM_RD_ENA_SHF);// SEC enable, */
}

#else
void vCPGetSCE(u32 * const pu4SCETableRead)
{

}


#endif
#if 1				/* add by sxj */
void vCPPatterGenerate(u8 bChanel, u32 u4En, u32 u4Step, u32 u4Prec)
{
	bChanel %= 4U;
	u4En %= 2U;
	u4Step %= 8U;
	u4Prec %= 4U;

	/* vWriteCPMsk(RW_PCLRP_PTNGEN_C,(~MLC_XFRONT),MLC_XFRONT); */
	switch (bChanel) {

	case CLRP_Y_CHANEL:
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_L,
				 u4En << MLC_PTN_ENA_SHF, MLC_PTN_ENA);
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_L,
				 u4Step << MLC_PTN_STEP_SHF, MLC_PTN_STEP);
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_L,
				 u4Prec << MLC_PTN_PREC_SHF, MLC_PTN_PREC);
		VCP_LOG(VCP_LOG_LVL_INFO, "Ptn_Gen_Y: EN:%d, Step:%d, Prec:%d\n", (int)u4En, (int)u4Step, (int)u4Prec);
		break;
	case CLRP_U_CHANEL:
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_C,
				 u4En << MLC_PTN_ENA_U_SHF, MLC_PTN_ENA_U);
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_C,
				 u4Step << MLC_PTN_STEP_U_SHF, MLC_PTN_STEP_U);
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_C,
				 u4Prec << MLC_PTN_PREC_U_SHF, MLC_PTN_PREC_U);
		VCP_LOG(VCP_LOG_LVL_INFO, "Ptn_Gen_U: EN:%d, Step:%d, Prec:%d\n", (int)u4En, (int)u4Step, (int)u4Prec);
		break;
	case CLRP_V_CHANEL:
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_C,
				 u4En << MLC_PTN_ENA_V_SHF, MLC_PTN_ENA_V);
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_C,
				 u4Step << MLC_PTN_STEP_V_SHF, MLC_PTN_STEP_V);
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_C,
				 u4Prec << MLC_PTN_PREC_V_SHF, MLC_PTN_PREC_V);
		VCP_LOG(VCP_LOG_LVL_INFO, "Ptn_Gen_V: EN:%d, Step:%d, Prec:%d\n", (int)u4En, (int)u4Step, (int)u4Prec);
		break;
	case CLRP_ALL_CHANEL:
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_L, (u4En << MLC_PTN_ENA_SHF) |
			(u4Step << MLC_PTN_STEP_SHF) | (u4Prec << MLC_PTN_PREC_SHF),
			MLC_PTN_ENA | MLC_PTN_STEP | MLC_PTN_PREC);
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_C, (u4En << MLC_PTN_ENA_U_SHF) |
			(u4Step << MLC_PTN_STEP_U_SHF) | (u4Prec << MLC_PTN_PREC_U_SHF),
			MLC_PTN_ENA_U | MLC_PTN_STEP_U | MLC_PTN_PREC_U);
		vWriteCPMsk((unsigned int)RW_PCLRP_PTNGEN_C, (u4En << MLC_PTN_ENA_V_SHF) |
			(u4Step << MLC_PTN_STEP_V_SHF) | (u4Prec << MLC_PTN_PREC_V_SHF),
			MLC_PTN_ENA_V | MLC_PTN_STEP_V | MLC_PTN_PREC_V);
		VCP_LOG(VCP_LOG_LVL_INFO, "Ptn_Gen_YUV: EN:%d, Step:%d, Prec:%d\n", (int)u4En, (int)u4Step, (int)u4Prec);
	default:
		break;
	}
}


/* General Purpose process using */
void vCPGppDelay(int y, int u, int v)
{
	vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_Y, (y % 8) << DELAY_Y_SHF,
			 DELAY_Y);
	vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_U, (u % 8) << DELAY_U_SHF,
			 DELAY_U);
	vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_V, (v % 8) << DELAY_V_SHF,
			 DELAY_V);

	VCP_LOG(VCP_LOG_LVL_INFO, "GPP Delay:YUV, (%d,%d,%d) T\n", ((y & 0x4) ? (~(y & 0x3) + 1) : (y)),
		((u & 0x4) ? (~(u & 0x3) + 1) : (u)), ((v & 0x4) ? (~(v & 0x3) + 1) : (v)));
}

void vCPGppOffset1(int y, int u, int v)
{
	vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_Y, y << OFFSET1_Y_SHF, OFFSET1_Y);
	vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_U, u << OFFSET1_U_SHF,
			 OFFSET1_U);
	vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_V, v << OFFSET1_V_SHF,
			 OFFSET1_V);
	VCP_LOG(VCP_LOG_LVL_INFO, "GPP Offset1:YUV, (0x%x,0x%x,0x%x)\n", y, u, v);
}

void vCPGppOffset2(int y, int u, int v)
{
	vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_Y, y << 12, OFFSET2_Y);
	vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_U, u << 12, OFFSET2_U);
	vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_V, v << 12, OFFSET2_V);
	VCP_LOG(VCP_LOG_LVL_INFO, "GPP Offset2:YUV, (0x%x,0x%x,0x%x)\n", y, u, v);
}

void vCPGppInv(int y, int u, int v)
{
	if (y % 2 == 1)
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_Y, INV_Y, INV_Y);	/* enable */
	else
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_Y, ~INV_Y, INV_Y);	/* disable */

	if (u % 2 == 1)
		vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_U, INV_U, INV_U);
	else
		vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_U, ~INV_U, INV_U);

	if (v % 2 == 1)
		vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_V, INV_V, INV_V);
	else
		vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_V, ~INV_V, INV_V);
    VCP_LOG(VCP_LOG_LVL_INFO, "GPP INV:(Y %d, U %d, V %d)\n", y, u, v);
}

void vCPGppFix(int y, int u, int v, int y_En, int u_En, int v_En)
{
	vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_Y, y << FIX_Y_SHF, FIX_Y);
	vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_U, u << FIX_U_SHF, FIX_U);
	vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_V, v << FIX_V_SHF, FIX_V);
	if (y_En == 1)
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_Y, FIX_Y_EN, FIX_Y_EN);
	else
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_Y, ~FIX_Y_EN, FIX_Y_EN);

	if (u_En == 1)
		vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_U, FIX_U_EN, FIX_U_EN);
	else
		vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_U, ~FIX_U_EN, FIX_U_EN);

	if (v_En == 1)
		vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_V, FIX_V_EN, FIX_V_EN);
	else
		vWriteCPMsk((unsigned int)RW_PCLRP_CHROMA_V, ~FIX_V_EN, FIX_V_EN);
	VCP_LOG(VCP_LOG_LVL_INFO, "---GPP Fix Value---\n");
	VCP_LOG(VCP_LOG_LVL_INFO, "FixY %d, Value 0x%x\n", y_En, y);
	VCP_LOG(VCP_LOG_LVL_INFO, "FixU %d, Value 0x%x\n", u_En, u);
	VCP_LOG(VCP_LOG_LVL_INFO, "FixV %d, Value 0x%x\n", v_En, v);
}

void vCPGppProcess(s32 i4Mode, s32 i4DatY, s32 i4DatU, s32 i4DatV, s32 i4EnY,
		   s32 i4EnU, s32 i4EnV)
{
	switch (i4Mode) {

	case CLRP_GPP_DELAY:
		vCPGppDelay(i4DatY, i4DatU, i4DatV);
		break;
	case CLRP_GPP_OFFSET1:
		vCPGppOffset1(i4DatY, i4DatU, i4DatV);
		break;
	case CLRP_GPP_OFFSET2:
		vCPGppOffset2(i4DatY, i4DatU, i4DatV);
		break;
	case CLRP_GPP_GAIN:

		break;
	case CLRP_GPP_INV:
		vCPGppInv(i4DatY, i4DatU, i4DatV);
		break;
	case CLRP_GPP_FIX:
		vCPGppFix(i4DatY, i4DatU, i4DatV, i4EnY, i4EnU, i4EnV);
		break;
	case CLRP_GPP_RST:
		vCPGppDelay(0, 0, 0);	/* delay 0 T */
		vCPGppOffset1(0, 0, 0);	/* offset1 set	0 before gain */
		vCPGppOffset2(0, 0, 0);	/* offset2 set	0 after gain */

		vCPGppInv(0, 0, 0);	/* no inv */
		vCPGppFix(0x80, 0x80, 0x80, 0, 0, 0);	/* no fix */
		break;
	}
}

void vCPBlackWhiteLevelEx(s32 i4Mode, s32 i4Slope, s32 i4Anchor)
{
	s32 i4DisB, i4DisW;

	i4DisB = i4Slope % 2;
	i4DisW = i4Anchor % 2;
	switch (i4Mode)	{

	case BLACK_LEVEL_EX:
		vWriteCPMsk((unsigned int)RW_PCLRP_BLACK_EXTENSION,
				 i4Slope << BLEND_SLOPE_SHF, BLEND_SLOPE);	/* slope */
		vWriteCPMsk((unsigned int)RW_PCLRP_BLACK_EXTENSION,
				 i4Anchor << BLEND_ANCHOR_SHF, BLEND_ANCHOR);	/* start */
		vWriteCPMsk((unsigned int)RW_PCLRP_BLACK_EXTENSION,
				 BLEND_ENA, BLEND_ENA);	/* en */
		vWriteCPMsk((unsigned int)RW_PCLRP_BLACK_EXTENSION,
				 ~BLEV_EDIF, BLEV_EDIF);	/* disable err diff */
        VCP_LOG(VCP_LOG_LVL_INFO, "Black Level Ex:(Slop 0x%x, Anchor 0x%x)\n",
				 (unsigned int)i4Slope, (unsigned int)i4Anchor);
		break;
	case WHITE_LEVEL_EX:
		vWriteCPMsk((unsigned int)RW_PCLRP_WHITE_EXTENSION,
				 i4Slope << WLEND_SLOPE_SHF, WLEND_SLOPE);	/* slope */
		vWriteCPMsk((unsigned int)RW_PCLRP_WHITE_EXTENSION,
				 i4Anchor << WLEND_ANCHOR_SHF, WLEND_ANCHOR);	/* start */
		vWriteCPMsk((unsigned int)RW_PCLRP_WHITE_EXTENSION,
				 WLEND_ENA, WLEND_ENA);	/* en */
		vWriteCPMsk((unsigned int)RW_PCLRP_WHITE_EXTENSION,
				 ~WLEV_EDIF, WLEV_EDIF);	/* disable err diff */
        VCP_LOG(VCP_LOG_LVL_INFO, "White Level Ex:(Slop 0x%x, Anchor 0x%x)\n",
				 (unsigned int)i4Slope, (unsigned int)i4Anchor);
		break;
	case BW_LEVEL_EX_OFF:
		if (i4DisB) {
			vWriteCPMsk((unsigned int)RW_PCLRP_BLACK_EXTENSION,
				 ~BLEND_ENA, BLEND_ENA);	/* en */
			vWriteCPMsk((unsigned int)RW_PCLRP_BLACK_EXTENSION,
				 ~BLEV_EDIF, BLEV_EDIF);	/* disable err diff */
		} else{
			vWriteCPMsk((unsigned int)RW_PCLRP_BLACK_EXTENSION,
				 BLEND_ENA, BLEND_ENA);	/* en */
			vWriteCPMsk((unsigned int)RW_PCLRP_BLACK_EXTENSION,
				 ~BLEV_EDIF, BLEV_EDIF);	/* disable err diff */
		}
		if (i4DisW) {
			vWriteCPMsk((unsigned int)RW_PCLRP_WHITE_EXTENSION,
				 ~WLEND_ENA, WLEND_ENA);	/* en */
			vWriteCPMsk((unsigned int)RW_PCLRP_WHITE_EXTENSION,
				 ~WLEV_EDIF, WLEV_EDIF);	/* disable err diff */
		} else{
			vWriteCPMsk((unsigned int)RW_PCLRP_WHITE_EXTENSION,
				 WLEND_ENA, WLEND_ENA);	/* en */
			vWriteCPMsk((unsigned int)RW_PCLRP_WHITE_EXTENSION,
				 ~WLEV_EDIF, WLEV_EDIF);	/* disable err diff */
		}
        VCP_LOG(VCP_LOG_LVL_INFO, "BW_Level_Ex Disabel:(BlackDis %d, WhiteDis %d)\n", (unsigned int)i4Slope,
				   (unsigned int)i4Anchor);
		break;
	default:
		break;
	}
}

void vCPSetSceEn(s32 i4En)
{
	if (i4En) {
		vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
				 MLC_SCE_ENA, MLC_SCE_ENA);	/* SEC disable, */
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_SCECTRL,
				 MLC_MOD_ENA, MLC_MOD_ENA);	/* SEC enable,Y */
        VCP_LOG(VCP_LOG_LVL_INFO, "SEC Enable\n");
	} else{
		vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
				 ~MLC_SCE_ENA, MLC_SCE_ENA);	/* SEC disable, */
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_SCECTRL,
				 ~MLC_MOD_ENA, MLC_MOD_ENA);	/* SEC enable,Y */
        VCP_LOG(VCP_LOG_LVL_INFO, "SEC Disable\n");
	}
}

void vCPSetSCE2(unsigned int sceEn, unsigned int luma, unsigned int hue, unsigned int sat)
{
	u32 dwData;
	u16 wCnt;
	u8 bCnt;
	unsigned int sce_data = 0;

	sce_data = (luma << 14) | (hue << 8) | (sat << 0);
	if (sceEn == 1)	{
		vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
				 SRAM_WR_MODE, SRAM_WR_MODE);	/* SEC sram write mode */
		for (wCnt = 0; wCnt <= 359; wCnt++) {
			dwData = (sce_data << 10) | (wCnt << 1);
			vWriteCP((unsigned int)RW_PCLRP_SCE_TABLE,
				 dwData | 0x01);	/* write vector and write bit */
			for (bCnt = 0; bCnt <= 10; bCnt++) {	/* delay?? */
				dReadCP((unsigned int)0x0c);
			}
			vWriteCP((unsigned int)RW_PCLRP_SCE_TABLE,
				 dwData | 0x00);	/* write vector and clear write bit */
			for (bCnt = 0; bCnt <= 10; bCnt++) {	/* delay?? */
				dReadCP((unsigned int)0x0c);
			}
		}
		vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
					 MLC_SCE_ENA, MLC_SCE_ENA);	/* SEC enable, UV */
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_SCECTRL,
					 MLC_MOD_ENA, MLC_MOD_ENA);	/* SEC enable,Y */
        VCP_LOG(VCP_LOG_LVL_INFO,
			 "SCE En:%d, luma:0x%x, hue:0x%x,sat:0x%x, lum_limit:255, LumaType: Gain)\n",
			 sceEn, luma, hue, sat);
	} else{
		vWriteCPMsk((unsigned int)RW_PCLRP_HUE_SCECTRL,
					 ~MLC_SCE_ENA, MLC_SCE_ENA);	/* SEC disable, */
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_SCECTRL,
					 ~MLC_MOD_ENA, MLC_MOD_ENA);	/* SEC enable,Y */
        VCP_LOG(VCP_LOG_LVL_INFO, "SEC Disable\n");
	}
}

void vCPSetSCELumaMode(unsigned int mode, s32 i4Limit)
{
	if (mode == 1) {
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_SCECTRL,
				 MOD_MUL, MLC_MOD_TYPE);	/* gain mode */
        VCP_LOG(VCP_LOG_LVL_INFO, "SEC Luma Type : Gain\n");
	} else{
		vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_SCECTRL,
			 MOD_SUB, MLC_MOD_TYPE);	/* sub mode */
        VCP_LOG(VCP_LOG_LVL_INFO, "SEC Luma Type : Add\n");
	}
	vWriteCPMsk((unsigned int)RW_PCLRP_LUMA_SCECTRL,
			 (i4Limit << MLC_MOD_LMT_SHF), MLC_MOD_LMT);	/* sub mode */
    VCP_LOG(VCP_LOG_LVL_INFO, "SEC Luma Limit : 0x%x\n", (unsigned int)i4Limit);
}

void vCPSCE(s32 i4Mode, s32 i4Luma, s32 i4Hue, s32 i4Sat, s32 i4Dat)
{
	s32 i4Type, i4Limit;

	UNUSED(i4Type);
	UNUSED(i4Limit);
	switch (i4Mode)	{

	case CLRP_SCE_TABLE:
		vCPSetSCE2(0x01, i4Luma, i4Hue, i4Sat);
		break;
	case CLRP_SCE_ONOFF:
		vCPSetSceEn(i4Luma);

/* vPanelSetSCE2(0x0, i4Luma, i4Hue, i4Sat); */
		break;
	case CLRP_SCE_LUMATYPE:
		i4Type = i4Luma % 2;
		i4Limit = i4Hue & 0xff;
		vCPSetSCELumaMode(i4Type, i4Limit);
		break;
	case CLRP_SCE_TEST:
		vCPSetSCE(NULL);
		break;
	case CLRP_SCE_HUE_GLOBAL:

		/* Hue Range 0x0~0x3F (from arg1:i4Luma) */
		/* vCPSetGlobalHue(i4Luma); */
		break;
	case CLRP_SCE_SETTING_CFG:

		/* Config GlobHuePrec PartialHuePrec UClamp VClamp (from arg1 to arg 4) */
		vCPSCEHueConfig(i4Luma, i4Hue, i4Sat, i4Dat);
		break;
	case CLRP_SCE_GET_TABLE:
		vCPGetSCE(NULL);
		break;
	case CLRP_SCE_USER:
		VCP_LOG(VCP_LOG_LVL_INFO, "Please Load SCE Table First\n");
		break;
	}
}


/* void vTconCTI(INT32 i4Level) */
void vCPCTI(s32 i4Mode, s32 i4Value0, s32 i4Value1)
{
	s32 i4En, i4T_Sel, i4Gain_Sharp;
	u32 i4Config;

	UNUSED(i4En);
	UNUSED(i4T_Sel);
	UNUSED(i4Gain_Sharp);
	UNUSED(i4Config);
	switch (i4Mode) {

	case 0:		/* tune T_sel & gain sharp (coarse gain && fine gain) */
		i4T_Sel = i4Value0 < 3 ? i4Value0 : 7;
		i4Gain_Sharp = i4Value1 & 0x7f;
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI,
				 i4T_Sel << CTI_T_SELECT_SHF, CTI_T_SELECT);
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI,
				 i4Gain_Sharp << CTI_GAIN_SHARP_SHF, CTI_GAIN_SHARP);
		VCP_LOG(VCP_LOG_LVL_INFO, "CTI CoarseGain 0x%x, FineGain 0x%x\n",
				 (unsigned int)i4T_Sel, (unsigned int)i4Gain_Sharp);
		break;
	case 1:		/* LPF Configure */
		i4En = i4Value0 % 2;
		i4Config = i4Value1 & 0x3;
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI, i4En << CTI_LP_ENA_SHF,
				 CTI_LP_ENA);
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI,
				 i4Config << CTI_LP_SEL_SHF, CTI_LP_SEL);
		VCP_LOG(VCP_LOG_LVL_INFO, "CTI LPF EN0x%x, LPF SEL 0x%x\n", 
				 (unsigned int)i4En, (unsigned int)i4Config);
		break;
	case 2:
		i4En = i4Value0 % 2;
		i4Config = i4Value1 & 0xff;
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI2, i4En << HDDETECT_EN_SHF,
				 HDDETECT_EN);
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI2, i4Config << HD_AMP_SHF,
				 HD_AMP);
		VCP_LOG(VCP_LOG_LVL_INFO, "CTI AMP_EN 0x%x, HD_AMP 0x%x\n", 
				 (unsigned int)i4En, (unsigned int)i4Config);
		break;
	case 3:
		i4En = i4Value0 % 2;
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI,
				 i4En << CTI_MAX_MIN_JG_SHF, CTI_MAX_MIN_JG);
		VCP_LOG(VCP_LOG_LVL_INFO, "CTI MAX_MIN_JG En 0x%x\n", (unsigned int)i4En);
		break;
	case 4:
		i4En = i4Value0 % 2;
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI,
				 i4En << CTI_PTADDSUB_INV_SHF, CTI_PTADDSUB_INV);
		VCP_LOG(VCP_LOG_LVL_INFO, "CTI ANDSUB_INV En 0x%x\n", (unsigned int)i4En);
		break;
	case 5:
		i4Config = i4Value0 & 0xF;
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI,
				 i4Config << CTI_PTADDSUB_INV_SHF, CTI_PTADDSUB_INV);
		VCP_LOG(VCP_LOG_LVL_INFO, "CTI DZONE 0x%x\n", (unsigned int)i4Config);
		break;
	case 6:
		i4Config = i4Value0 & 0xFFFFF;
		vWriteCPMsk((unsigned int)RW_PCLRP_CTI1,
				 i4Config << CTI_FIR_COEFF_SHF, CTI_FIR_COEFF);
		VCP_LOG(VCP_LOG_LVL_INFO, "CTI FIR Coeff 0x%x Default:[0x15654]\n", (unsigned int)i4Config);
		break;
	default:
		VCP_LOG(VCP_LOG_LVL_ERR, "Please check cmd Arg before test cti\n");
		break;
	}
}

void vCPUV2CbCr(u32 i4En, u32 u4GainU, u32 u4GainV, u32 u4Sign)
{
	vWriteCPMsk((unsigned int)RW_UV2CBCR_CONV,
			 (u4GainU << MLC_U2CB_GAIN_SHF | u4GainV << MLC_V2CR_GAIN_SHF | i4En <<
			  MLC_UV_CONV_EN_SHF | u4Sign << MLC_OUTFRONT_SHF),
			 MLC_U2CB_GAIN | MLC_V2CR_GAIN | MLC_UV_CONV_EN | MLC_OUTFRONT);
    VCP_LOG(VCP_LOG_LVL_INFO, "UV2CbCr: En %d, UGain:0x%x, VGain:0x%x, Sign:%d\n", (int)i4En,
		   (unsigned int)u4GainU,
		   (unsigned int)u4GainV,
		   (unsigned int)u4Sign);
}

void vCPSuppression(u32 u4Mode, u32 u4En, u32 u4Gain, u32 u4Offset,
			u32 u4SubDiv, u32 u4Spc, u32 u4Spcc)
{
	switch (u4Mode)	{

	case 0:
		vWriteCPMsk((unsigned int)RW_PCLRP_SUPPRESSION,
				   (u4Gain << SPC_GAIN_SHF | u4Offset << SEED_OFFSET_SHF | u4En <<
				SPC_ENA_SHF), SPC_GAIN | SEED_OFFSET | SPC_ENA);
		vWriteCPMsk((unsigned int)RW_PCLRP_SUPPRESSION,
				 (u4SubDiv << SPC_SUB_DIV_SHF | u4Spc << SPC_SEL_SHF | u4Spcc <<
				  SPCC_SEL_SHF), SPC_SUB_DIV | SPC_SEL | SPCC_SEL);
        VCP_LOG(VCP_LOG_LVL_INFO, "SPC:En %d, Gain %d, Offset %d, SubDiv %d, Spc %d, Spcc %d\n",
			 (int)u4En, (int)u4Gain, (int)u4Offset, (int)u4SubDiv, (int)u4Spc, (int)u4Spcc);
		break;
	case 1:		/* Low pass filter	En & Sel */
		u4Gain %= 0x4;
		vWriteCPMsk((unsigned int)RW_PCLRP_SUPPRESSION,
				 (u4En << SPC_LP_ENA_SHF | u4Gain << SPC_LP_SEL_SHF),
				 SPC_LP_ENA | SPC_LP_SEL);
        VCP_LOG(VCP_LOG_LVL_INFO, "SPC(Low Pass Filter): En %d, Sel: %d\n", (int)u4En, (int)u4Gain);
		break;
	}
}


#endif

#else/*android*/

#ifndef __ARM2__
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <media/atc/cp.h>
#include <media/atc/display.h>
#include "x_debug.h"
#include "x_os.h"
#include "x_stl_lib.h"
#else
#include "x_types.h"
#include "cp.h"
#include "display.h"
#endif
#include "x_ckgen.h"
#include "x_assert.h"
#include "x_typedef.h"
#include "x_lint.h"
#include "cp_reg.h"
#include "cp_def.h"
#include "x_printf.h"

//#include <linux/module.h>
//#include "x_assert.h"
//#include "x_typedef.h"
//#include "x_os.h"
//#include "x_lint.h"
//#include "cp.h"
//#include "cp_reg.h"
//#include "cp_def.h"
//#include "x_printf.h"
//#include "x_stl_lib.h"
//#include "display.h"

//extern VOID DisplaySclInit(BOOL fgHwReset);


#define DEFINE_IS_LOG	CLI_IsLog
#include "x_debug.h"
//#include "display_inc.h"
//#include "x_bim.h"
//#define SRCTYPE_NUM
//int srctype_num = 2;
CP_CONFIG vcpinfo_c[srcMax];
static bool fgavin_1[] = {false,false,false};
static bool fgavin_2[] = {false,false,false};
static bool fgavin_3[] = {false,false,false};
static bool fgavin_4[] = {false,false,false};
static bool fgavin_5[] = {false,false,false};
static bool fgbkcar[] = {false,false,false};
static bool fgusb[] = {false,false,false};
static bool fgdvd[] = {false,false,false};
static bool fgypbpr[] = {false,false,false};
static bool fgvga[] = {false,false,false};
static bool fgdgi[] = {false,false,false};
static bool fghdmi[] = {false,false,false};
static bool fgSrcType = true;

#define HUE    0
#define YUV    1
#define CBS    2
#define vColorProcessEnable(dVal)  vWriteReg(0x1F080,dVal)
//#define malloc(n) x_mem_alloc(n)//x_alloc_afifo_mem(n)

void vCPBypass(s32 i4Bypass)//i4bypass=0 enable cp
{
	u32 u4CpEn = (i4Bypass+1)%2;
	vColorProcessEnable(u4CpEn);
	UTIL_Printf("[VCP][INFO]Color Process bypass: %d \n",i4Bypass);
	return;
}
void vCPOn(s32 i4On)//i4On=0 cp on
{
	vCPBypass(i4On);
}

void vCPOff(s32 i4Off)//i4Off=1 cp off
{
	vCPBypass(i4Off);
}
void vCPVideoOn(void)
{
    u32 u4CpEn = 1;
	vColorProcessEnable(u4CpEn);
	UTIL_Printf("[VCP][INFO]Color Process on: \n");
}
EXPORT_SYMBOL(vCPVideoOn);

void vCPVideoOff(void)
{
    u32 u4CpEn = 0;
	vColorProcessEnable(u4CpEn);
	UTIL_Printf("[VCP][INFO]Color Process off: \n");
}
EXPORT_SYMBOL(vCPVideoOff);
/*void vCPSetSrcTypeNum()
{
    vcpinfo_c = (CP_CONFIG*)malloc(sizeof(CP_CONFIG)*srctype_num);
    UTIL_Printf("vCPSetSrcTypeNum:srctype_num = %d \n",srctype_num);
}
EXPORT_SYMBOL(vCPSetSrcTypeNum);*/

void VcpInit()
{
//    *(unsigned long *)0xfd01f080 = 0x00000001; //enable cp
    *(unsigned long *)0xfd042654 = 0x10603207; //CTI high gain
    *(unsigned long *)0xfd042654 = 0x010603f7; //CTI fine gain to 7f
    *(unsigned long *)0xfd042660 = 0x0001ff00; //set CTI operation range to max
    *(unsigned long *)0xfd04260c = 0x0f801050; //set refine contrast and core value
}
//Set SCE Global Hue
void vCPSetGlobalHue(s32 i4GHue)
{
    vWriteCPMsk(RW_PCLRP_HUE_SCECTRL, i4GHue<<HUE_DEGREE_SHF, HUE_DEGREE); //turn on gamma
    UTIL_Printf("[VCP][INFO]vCPSetGlobalHue:Set SCE Global Hue = 0x%x\n",i4GHue);
}

VIDEO_SRC_TYPE vCPGetSrcType(int i)
{
    //printk("[xzr] vcp%d: srctype = %d\r\n", i, vcpinfo_c[i].SrcType);
    return vcpinfo_c[i].SrcType;
}
EXPORT_SYMBOL(vCPGetSrcType);

void vCPVideoSetGlobalHue(VIDEO_SRC_TYPE src_type);
void vCPAppSetGlobalHue(s32 i4GHue,VIDEO_SRC_TYPE src_type)
{
    int srcnum = -1;

    if (i4GHue < 0 || i4GHue > 0x3f) {
        UTIL_Printf("[VCP][ERR]vCPAppSetGlobalHue:Set SCE Global Hue = 0x%x, error\n",i4GHue);
        return;
    }
    if (src_type < 0xa || src_type > 0x15) {
        UTIL_Printf("[VCP][ERR]vCPAppSetGlobalHue error src_type: 0x%x, error\n",src_type);
        return;
    }

    vCPSetGlobalHue(i4GHue);

    switch(src_type)
    {
         case AVIN_1:
            {
                if(fgavin_1[HUE]){}
                else{
                    srcnum = 0;
                    fgavin_1[HUE] = true;
                }
                break;
            }
         case AVIN_2:
            {
                if(fgavin_2[HUE]){}
                else{
                    srcnum = 1;
                    fgavin_2[HUE] = true;
                }
                break;
            }
         case AVIN_3:
            {
                if(fgavin_3[HUE]){}
                else{
                    srcnum = 2;
                    fgavin_3[HUE] = true;
                }
                break;
            }
         case AVIN_4:
            {
                if(fgavin_4[HUE]){}
                else{
                    srcnum = 3;
                    fgavin_4[HUE] = true;
                }
                break;
            }
         case AVIN_5:
            {
                if(fgavin_5[HUE]){}
                else{
                    srcnum = 4;
                    fgavin_5[HUE] = true;
                }
                break;
            }
         case BACKCAR:
            {
                if(fgbkcar[HUE]){}
                else{
                    srcnum = 5;
                    fgbkcar[HUE] = true;
                }
                break;
            }
         case USB:
            {
                if(fgusb[HUE]){}
                else{
                    srcnum = 6;
                    fgusb[HUE] = true;
                }
                break;
            }
         case DVD:
            {
                if(fgdvd[HUE]){}
                else{
                    srcnum = 7;
                    fgdvd[HUE] = true;
                }
                break;
            }
         case YPBPR:
            {
                if(fgypbpr[HUE]){}
                else{
                    srcnum = 8;
                    fgypbpr[HUE] = true;
                }
                break;
            }
         case VGA:
            {
                if(fgvga[HUE]){}
                else{
                    srcnum = 9;
                    fgvga[HUE] = true;
                }
                break;
            }
         case DGI:
            {
                if(fgdgi[HUE]){}
                else{
                    srcnum = 10;
                    fgdgi[HUE] = true;
                }
                break;
            }
         case VDO_HDMI:
            {
                if(fghdmi[HUE]){}
                else{
                    srcnum = 11;
                    fghdmi[HUE] = true;
                }
                break;
            }
         default:
            {
                UTIL_Printf("[VCP][INFO]vCPAppSetGlobalHue:not surport this srctype\n");
                break;
            }
    } 

    if(srcnum != -1)
    {
        vcpinfo_c[srcnum].i4GHue = i4GHue;
        vcpinfo_c[srcnum].SrcType = src_type;
        vCPVideoSetGlobalHue(src_type);
    } else {
	for(srcnum=0; srcnum < srcMax; srcnum++)
                vcpinfo_c[srcnum].SrcType = 0;
    }

    UTIL_Printf("[VCP][INFO]vCPAppSetGlobalHue:Pre Set SCE Global Hue = 0x%x\n",vcpinfo_c[srcnum].i4GHue);
}
void vCPVideoSetGlobalHue(VIDEO_SRC_TYPE src_type)
{
    int srcnum = -1;
    bool fgsetvalue = false;
    
    switch(src_type)
    {
         case AVIN_1:
            {
                srcnum = 0;
                if(fgavin_1[HUE]) {
                    fgsetvalue = true;
                    fgavin_1[HUE] = false;
                }
                break;
            }
         case AVIN_2:
            {
                srcnum = 1;
                if(fgavin_2[HUE]) {
                    fgsetvalue = true;
                    fgavin_2[HUE] = false;
                }
                break;
            }
         case AVIN_3:
            {
                srcnum = 2;
                if(fgavin_3[HUE]) {
                    fgsetvalue = true;
                    fgavin_3[HUE] = false;
                }
                break;
            }
         case AVIN_4:
            {
                srcnum = 3;
                if(fgavin_4[HUE]) {
                    fgsetvalue = true;
                    fgavin_4[HUE] = false;
                }
                break;
            }
         case AVIN_5:
            {
                srcnum = 4;
                if(fgavin_5[HUE]) {
                    fgsetvalue = true;
                    fgavin_5[HUE] = false;
                }
                break;
            }
         case BACKCAR:
            {
                srcnum = 5;
                if(fgbkcar[HUE]) {
                    fgsetvalue = true;
                    fgbkcar[HUE] = false;
                }
                break;
            }
         case USB:
            {
                srcnum = 6;
                if(fgusb[HUE]) {
                    fgsetvalue = true;
                    fgusb[HUE] = false;
                }
                break;
            }
         case DVD:
            {
                srcnum = 7;
                if(fgdvd[HUE]) {
                    fgsetvalue = true;
                    fgdvd[HUE] = false;
                }
                break;
            }
         case YPBPR:
            {
                srcnum = 8;
                if(fgypbpr[HUE]) {
                    fgsetvalue = true;
                    fgypbpr[HUE] =false;
                }
                break;
            }
         case VGA:
            {
                srcnum = 9;
                if(fgvga[HUE]) {
                    fgsetvalue = true;
                    fgvga[HUE] = false;
                }
                break;
            }
         case DGI:
            {
                srcnum = 10;
                if(fgdgi[HUE]) {
                    fgsetvalue = true;
                    fgdgi[HUE] = false;
                }
                break;
            }
         case VDO_HDMI:
            {
                srcnum = 11;
                if(fghdmi[HUE]) {
                    fgsetvalue = true;
                    fghdmi[HUE] = false;
                }
                break;
            }
         default:
            {
                UTIL_Printf("[VCP][INFO]vCPVideoSetGlobalHue:not surport this srctype\n");
                break;
            }
    } 
    if(fgsetvalue)
    {
    	//vWriteCPMsk(RW_PCLRP_HUE_SCECTRL, vcpinfo_c[srcnum].i4GHue<<HUE_DEGREE_SHF, HUE_DEGREE); //turn on gamma
        UTIL_Printf("[VCP][INFO]vCPVideoSetGlobalHue:Set SCE Global Hue = 0x%x\n",vcpinfo_c[srcnum].i4GHue);
    }
    else
    {
        //vWriteCPMsk(RW_PCLRP_HUE_SCECTRL, 0x20<<HUE_DEGREE_SHF, HUE_DEGREE);
    }
}
EXPORT_SYMBOL(vCPVideoSetGlobalHue);

s32 vCPGetGlobalHue()
{
    u32 i4GHue;
    s32 i4GHue_r;
    i4GHue = dReadCP(RW_PCLRP_HUE_SCECTRL);
    i4GHue_r = (INT32)((i4GHue & HUE_DEGREE)>>HUE_DEGREE_SHF);
	UTIL_Printf("[VCP][INFO]vCPGetGlobalHue:Get SCE Global Hue = 0x%x\n",i4GHue_r);
    return i4GHue_r;
}

//Config GlobHuePrec PartialHuePrec UClamp VClamp
void vCPSCEHueConfig(s32 i4GHuePrec, s32 i4PHuePrec, s32 i4UClamp, s32 i4VClamp)
{
    vWriteCPMsk(RW_PCLRP_HUE_SCECTRL, (i4GHuePrec<<HUE_GLOB_PREC_SHF)|(i4UClamp<<SCE_UCLAMP_SHF)|(i4VClamp<<SCE_VCLAMP_SHF), HUE_GLOB_PREC|SCE_UCLAMP|SCE_VCLAMP);
    vWriteCPMsk(RW_PCLRP_HUE_SCE_CFG, i4PHuePrec, HUE_DG_PREC);
    UTIL_Printf("[VCP][INFO]Set SCE Hue Prec:(1:+/-28,0:+/-14)Global Hue:%d, Partial Hue:%d\n U Clamp:%d, VClamp:%d\n",\
	 	i4GHuePrec,i4GHuePrec,i4UClamp,i4VClamp);
}
void vCPSetSCE(const u32 *const pu4SCETableWrite)
{
    DWRD dwData;
    WORD wCnt;
    BYTE bCnt;

    vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,SRAM_WR_MODE,SRAM_WR_MODE|SRAM_RD_ENA);// SEC sram write mode
    for (wCnt=0;wCnt<=359;wCnt++)
    {
    	if(pu4SCETableWrite == NULL)
    	{
	    	dwData = (_pdCPTestSCETable[wCnt]<<10) |(wCnt<<1); //
    	}else
    	{
	    	dwData = ((*(pu4SCETableWrite+wCnt))<<10) |(wCnt<<1); //
    	}

        //dwData = (0x200080<<10) |(wCnt<<1); //
        vWriteCP(RW_PCLRP_SCE_TABLE,dwData|0x01); //write vector and write bit
        for (bCnt=0;bCnt<=10;bCnt++)//delay??
        {
            dReadCP(0x0c);
        }
        vWriteCP(RW_PCLRP_SCE_TABLE,dwData|0x00); //write vector and clear write bit
        for (bCnt=0;bCnt<=10;bCnt++)//delay??
        {
            dReadCP(0x0c);
        }
		UTIL_Printf("[VCP][INFO][Write]SCE idx:%d (%d,%d,%d) Tab:0x%x \n",wCnt,0xFF&(dwData>>24),0x3F&(dwData>>18),0xFF&(dwData>>10),dwData>>10);
    }

    vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,MLC_SCE_ENA,SRAM_RD_ENA|MLC_SCE_ENA);// SEC enable,
}

#if defined(CP_FEATURE_SCE_TAB_READ)
void vCPGetSCE(u32 * const pu4SCETableRead)
{
	u32 u4Index=0;
	u32 u4Data = 0;
	u8 u1Luma = 0;
	u8 u1Hue = 0;
	u8 u1Sat = 0;
//step 1 Disable SCE & Enable ReadMode
	vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,0,MLC_SCE_ENA|SRAM_RD_ENA);// SEC Disable & Read Enable,
	mdelay(2);
//step2
	for(u4Index=0; u4Index<360; u4Index++)
	{
		mdelay(2);
		vWriteCPMsk(RW_PCLRP_SCE_TABLE_READ, (u4Index<<MLC_SCE_READ_ADDR_SHF)|MLC_SCE_READ_ENA,MLC_SCE_READ_ADDR|MLC_SCE_READ_ENA);// SEC enable,
		//mdelay(5);
		//vWriteCPMsk(RW_PCLRP_SCE_TABLE_READ, ~MLC_SCE_READ_ENA,MLC_SCE_READ_ENA);// SEC enable,
		mdelay(2);
		u4Data = dReadCP(RW_PCLRP_SCE_TABLE_READ);
		mdelay(2);
		if(pu4SCETableRead != NULL)
		{
		    *(pu4SCETableRead+u4Index) = u4Data>>MLC_SCE_P_SAT_SHF;
		}
		u1Luma = (u4Data&MLC_SCE_P_LUMA)>>MLC_SCE_P_LUMA_SHF;
		u1Hue = (u4Data &MLC_SCE_P_HUE)>>MLC_SCE_P_HUE_SHF;
		u1Sat = (u4Data&MLC_SCE_P_SAT)>>MLC_SCE_P_SAT_SHF;
		UTIL_Printf("[VCP][INFO][Read]SCE idx:%d(%d,%d,%d) Tab:0x%x Reg:0x%x\n",u4Index,u1Luma,u1Hue,u1Sat,u4Data>>MLC_SCE_P_SAT_SHF,u4Data);
	}
	vWriteCPMsk(RW_PCLRP_SCE_TABLE_READ, ~MLC_SCE_READ_ENA,MLC_SCE_READ_ENA);// SEC enable,
    //vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,MLC_SCE_ENA,MLC_SCE_ENA|SRAM_RD_ENA_SHF);// SEC enable,
    return ;
}

#else
void vCPGetSCE(u32 * const pu4SCETableRead)
{
    return ;
}

#endif
#if 1  // add by sxj

void vCPPatterGenerate(BYTE bChanel, u32 u4En, u32 u4Step, u32 u4Prec)
{
    bChanel %= 4;
    u4En %= 2;
    u4Step %= 8;
    u4Prec %= 4;
    //vWriteCPMsk(RW_PCLRP_PTNGEN_C,(~MLC_XFRONT),MLC_XFRONT);
    switch (bChanel)
    {
    case CLRP_Y_CHANEL:
        vWriteCPMsk(RW_PCLRP_PTNGEN_L,u4En<<MLC_PTN_ENA_SHF,MLC_PTN_ENA);
        vWriteCPMsk(RW_PCLRP_PTNGEN_L,u4Step<<MLC_PTN_STEP_SHF,MLC_PTN_STEP);
        vWriteCPMsk(RW_PCLRP_PTNGEN_L,u4Prec<<MLC_PTN_PREC_SHF,MLC_PTN_PREC);
        UTIL_Printf("[VCP][INFO]Ptn_Gen_Y: EN:%d, Step:%d, Prec:%d\n", u4En, u4Step, u4Prec);
        break;
    case CLRP_U_CHANEL:
        vWriteCPMsk(RW_PCLRP_PTNGEN_C,u4En<<MLC_PTN_ENA_U_SHF,MLC_PTN_ENA_U);
        vWriteCPMsk(RW_PCLRP_PTNGEN_C,u4Step<<MLC_PTN_STEP_U_SHF,MLC_PTN_STEP_U);
        vWriteCPMsk(RW_PCLRP_PTNGEN_C,u4Prec<<MLC_PTN_PREC_U_SHF,MLC_PTN_PREC_U);
        UTIL_Printf("[VCP][INFO]Ptn_Gen_U: EN:%d, Step:%d, Prec:%d\n", u4En, u4Step, u4Prec);
        break;
    case CLRP_V_CHANEL:
        vWriteCPMsk(RW_PCLRP_PTNGEN_C,u4En<<MLC_PTN_ENA_V_SHF,MLC_PTN_ENA_V);
        vWriteCPMsk(RW_PCLRP_PTNGEN_C,u4Step<<MLC_PTN_STEP_V_SHF,MLC_PTN_STEP_V);
        vWriteCPMsk(RW_PCLRP_PTNGEN_C,u4Prec<<MLC_PTN_PREC_V_SHF,MLC_PTN_PREC_V);
        UTIL_Printf("[VCP][INFO]Ptn_Gen_V: EN:%d, Step:%d, Prec:%d\n", u4En, u4Step, u4Prec);
        break;
    case CLRP_ALL_CHANEL:
        vWriteCPMsk(RW_PCLRP_PTNGEN_L,(u4En<<MLC_PTN_ENA_SHF)|(u4Step<<MLC_PTN_STEP_SHF)|(u4Prec<<MLC_PTN_PREC_SHF),MLC_PTN_ENA|MLC_PTN_STEP|MLC_PTN_PREC);
        vWriteCPMsk(RW_PCLRP_PTNGEN_C,(u4En<<MLC_PTN_ENA_U_SHF)|(u4Step<<MLC_PTN_STEP_U_SHF)|(u4Prec<<MLC_PTN_PREC_U_SHF),MLC_PTN_ENA_U|MLC_PTN_STEP_U|MLC_PTN_PREC_U);
        vWriteCPMsk(RW_PCLRP_PTNGEN_C,(u4En<<MLC_PTN_ENA_V_SHF)|(u4Step<<MLC_PTN_STEP_V_SHF)|(u4Prec<<MLC_PTN_PREC_V_SHF),MLC_PTN_ENA_V|MLC_PTN_STEP_V|MLC_PTN_PREC_V);

        UTIL_Printf("[VCP][INFO]Ptn_Gen_YUV: EN:%d, Step:%d, Prec:%d\n", u4En, u4Step, u4Prec);
    default :
        break;
    }

}

// General Purpose process using
void vCPGppDelay(int y,int u,int v)
{
    vWriteCPMsk(RW_PCLRP_LUMA_Y, (y%8)<<DELAY_Y_SHF, DELAY_Y);
    vWriteCPMsk(RW_PCLRP_CHROMA_U, (u%8)<<DELAY_U_SHF, DELAY_U);
    vWriteCPMsk(RW_PCLRP_CHROMA_V, (v%8)<<DELAY_V_SHF, DELAY_V);
//    UTIL_Printf("GPP Delay:YUV, (%d,%d,%d) T\n",((y%8)-3),((u%8)-3),((v%8)-3));
 	UTIL_Printf("[VCP][INFO]GPP Delay:YUV, (%d,%d,%d) T\n",((y&0x4)?(~(y&0x3)+1):(y)),
 	((u&0x4)?(~(u&0x3)+1):(u)),((v&0x4)?(~(v&0x3)+1):(v)) );

}

void vCPGppOffset1(int y,int u,int v)
{
    vWriteCPMsk(RW_PCLRP_LUMA_Y, y<<OFFSET1_Y_SHF, OFFSET1_Y);
    vWriteCPMsk(RW_PCLRP_CHROMA_U, u<<OFFSET1_U_SHF, OFFSET1_U);
    vWriteCPMsk(RW_PCLRP_CHROMA_V, v<<OFFSET1_V_SHF, OFFSET1_V);
    UTIL_Printf("[VCP][INFO]GPP Offset1:YUV, (0x%x,0x%x,0x%x) \n",y, u, v);
}

void vCPGppOffset2(int y,int u,int v)
{
    vWriteCPMsk(RW_PCLRP_LUMA_Y, y<<12, OFFSET2_Y);
    vWriteCPMsk(RW_PCLRP_CHROMA_U, u<<12, OFFSET2_U);
    vWriteCPMsk(RW_PCLRP_CHROMA_V, v<<12, OFFSET2_V);
    UTIL_Printf("[VCP][INFO]GPP Offset2:YUV, (0x%x,0x%x,0x%x) \n",y, u, v);
}
void vCPSetYUVGain(s32 i4YGain, s32 i4UGain, s32 i4VGain)
{
    vWriteCPMsk(RW_PCLRP_GAIN_Y, i4YGain<<MLC_GAIN_Y_SHF, MLC_GAIN_Y);
    vWriteCPMsk(RW_PCLRP_GAIN_UV, i4UGain<<MLC_GAIN_U_SHF, MLC_GAIN_U);
    vWriteCPMsk(RW_PCLRP_GAIN_UV, i4VGain<<MLC_GAIN_V_SHF, MLC_GAIN_V);

    vWriteCPMsk(RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN, MLC_GAIN_Y_EN);
    vWriteCPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN, MLC_GAIN_U_EN);
    vWriteCPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN, MLC_GAIN_V_EN);
    UTIL_Printf("[VCP][INFO]vCPSetYUVGain:GPP set Gain:YUV, (0x%x,0x%x,0x%x) \n",i4YGain,i4UGain,i4VGain);
}

void vCPVideoSetYUVGain(VIDEO_SRC_TYPE src_type);
void vCPAppSetYUVGain(s32 i4YGain,s32 i4UGain,s32 i4VGain,VIDEO_SRC_TYPE src_type)//void vCPGppGain(int y,int u,int v)
{
    int srcnum = -1;

    if (i4YGain < 0 || i4YGain > 0x1ff || i4UGain < 0 || i4UGain > 0x1ff || i4VGain < 0 || i4VGain > 0x1ff) {
        UTIL_Printf("[VCP][ERR]vCPAppSetYUVGain:Set SCE Global ygain = 0x%x, ugain = 0x%x, vgain = 0x%x error\n", i4YGain, i4UGain, i4VGain);
        return;
    }
    if (src_type < 0xa || src_type > 0x15) {
        UTIL_Printf("[VCP][ERR]vCPAppSetYUVGain error src_type: 0x%x, error\n",src_type);
        return;
    }

    vCPSetYUVGain(i4YGain, i4UGain, i4VGain);

    switch(src_type)
    {
         case AVIN_1:
            {
                if(fgavin_1[YUV]){}
                else{
                    srcnum = 0;
                    fgavin_1[YUV] = true;
                }
                break;
            }
         case AVIN_2:
            {
                if(fgavin_2[YUV]){}
                else{
                    srcnum = 1;
                    fgavin_2[YUV] = true;
                }
                break;
            }
         case AVIN_3:
            {
                if(fgavin_3[YUV]){}
                else{
                    srcnum = 2;
                    fgavin_3[YUV] = true;
                }
                break;
            }
         case AVIN_4:
            {
                if(fgavin_4[YUV]){}
                else{
                    srcnum = 3;
                    fgavin_4[YUV] = true;
                }
                break;
            }
         case AVIN_5:
            {
                if(fgavin_5[YUV]){}
                else{
                    srcnum = 4;
                    fgavin_5[YUV] = true;
                }
                break;
            }
         case BACKCAR:
            {
                if(fgbkcar[YUV]){}
                else{
                    srcnum = 5;
                    fgbkcar[YUV] = true;
                }
                break;
            }
         case USB:
            {
                if(fgusb[YUV]){}
                else{
                    srcnum = 6;
                    fgusb[YUV] = true;
                }
                break;
            }
         case DVD:
            {
                if(fgdvd[YUV]){}
                else{
                    srcnum = 7;
                    fgdvd[YUV] = true;
                }
                break;
            }
         case YPBPR:
            {
                if(fgypbpr[YUV]){}
                else{
                    srcnum = 8;
                    fgypbpr[YUV] = true;
                }
                break;
            }
         case VGA:
            {
                if(fgvga[YUV]){}
                else{
                    srcnum = 9;
                    fgvga[YUV] = true;
                }
                break;
            }
         case DGI:
            {
                if(fgdgi[YUV]){}
                else{
                    srcnum = 10;
                    fgdgi[YUV] = true;
                }
                break;
            }
         case VDO_HDMI:
            {
                if(fghdmi[YUV]){}
                else{
                    srcnum = 11;
                    fghdmi[YUV] = true;
                }
                break;
            }
         default:
            {
                UTIL_Printf("[VCP][INFO]vCPAppSetYUVGain:not surport this srctype\n");
                break;
            }
    } 

    if(srcnum != -1)
    {
        vcpinfo_c[srcnum].i4YGain = i4YGain;
        vcpinfo_c[srcnum].i4UGain = i4UGain;
        vcpinfo_c[srcnum].i4VGain = i4VGain;
        vcpinfo_c[srcnum].SrcType = src_type;
        vCPVideoSetYUVGain(src_type);
    } else {
	for (srcnum = 0; srcnum < srcMax; srcnum++)
                vcpinfo_c[srcnum].SrcType = 0;
    }
    
    UTIL_Printf("[VCP][INFO]vCPAppSetYUVGain:Pre GPP set Gain:YUV, (0x%x,0x%x,0x%x) \n",vcpinfo_c[srcnum].i4YGain,vcpinfo_c[srcnum].i4UGain,vcpinfo_c[srcnum].i4VGain);
}

void vCPVideoSetYUVGain(VIDEO_SRC_TYPE src_type)
{
    int srcnum = -1;
    bool fgsetvalue = false;

    switch(src_type)
    {
         case AVIN_1:
            {
                srcnum = 0;
                if(fgavin_1[YUV]) {
                    fgsetvalue = true;
                    fgavin_1[YUV] = false;
                }
                break;
            }
         case AVIN_2:
            {
                srcnum = 1;
                if(fgavin_2[YUV]) {
                    fgsetvalue = true;
                    fgavin_2[YUV] = false;
                }
                break;
            }
         case AVIN_3:
            {
                srcnum = 2;
                if(fgavin_3[YUV]) {
                    fgsetvalue = true;
                    fgavin_3[YUV] = false;
                }
                break;
            }
         case AVIN_4:
            {
                srcnum = 3;
                if(fgavin_4[YUV]) {
                    fgsetvalue = true;
                    fgavin_4[YUV] = false;
                }
                break;
            }
         case AVIN_5:
            {
                srcnum = 4;
                if(fgavin_5[YUV]) {
                    fgsetvalue = true;
                    fgavin_5[YUV] = false;
                }
                break;
            }
         case BACKCAR:
            {
                srcnum = 5;
                if(fgbkcar[YUV]) {
                    fgsetvalue = true;
                    fgbkcar[YUV] = false;
                }
                break;
            }
         case USB:
            {
                srcnum = 6;
                if(fgusb[YUV]) {
                    fgsetvalue = true;
                    fgusb[YUV] = false;
                }
                break;
            }
         case DVD:
            {
                srcnum = 7;
                if(fgdvd[YUV]) {
                    fgsetvalue = true;
                    fgdvd[YUV] = false;
                }
                break;
            }
         case YPBPR:
            {
                srcnum = 8;
                if(fgypbpr[YUV]) {
                    fgsetvalue = true;
                    fgypbpr[YUV] = false;
                }
                break;
            }
         case VGA:
            {
                srcnum = 9;
                if(fgvga[YUV]) {
                    fgsetvalue = true;
                    fgvga[YUV] =false;
                }
                break;
            }
         case DGI:
            {
                srcnum = 10;
                if(fgdgi[YUV]) {
                    fgsetvalue = true;
                    fgdgi[YUV] = false;
                }
                break;
            }
         case VDO_HDMI:
            {
                srcnum = 11;
                if(fghdmi[YUV]) {
                    fgsetvalue = true;
                    fghdmi[YUV] = false;
                }
                break;
            }
         default:
            {
                UTIL_Printf("[VCP][INFO]vCPVideoSetYUVGain:not surport this srctype\n");
                break;
            }
    }  
    if(fgsetvalue)
    {/*
        vWriteCPMsk(RW_PCLRP_GAIN_Y, vcpinfo_c[srcnum].i4YGain<<MLC_GAIN_Y_SHF, MLC_GAIN_Y);
        vWriteCPMsk(RW_PCLRP_GAIN_UV, vcpinfo_c[srcnum].i4UGain<<MLC_GAIN_U_SHF, MLC_GAIN_U);
        vWriteCPMsk(RW_PCLRP_GAIN_UV, vcpinfo_c[srcnum].i4VGain<<MLC_GAIN_V_SHF, MLC_GAIN_V);

        vWriteCPMsk(RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN, MLC_GAIN_Y_EN);
        vWriteCPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN, MLC_GAIN_U_EN);
        vWriteCPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN, MLC_GAIN_V_EN);*/
        UTIL_Printf("[VCP][INFO]vCPVideoSetYUVGain:GPP set Gain:YUV, (0x%x,0x%x,0x%x) \n",vcpinfo_c[srcnum].i4YGain,
            vcpinfo_c[srcnum].i4UGain,vcpinfo_c[srcnum].i4VGain);
    }
    else
    {/*
        vWriteCPMsk(RW_PCLRP_GAIN_Y, 0x80<<MLC_GAIN_Y_SHF, MLC_GAIN_Y);
        vWriteCPMsk(RW_PCLRP_GAIN_UV, 0x5b<<MLC_GAIN_U_SHF, MLC_GAIN_U);
        vWriteCPMsk(RW_PCLRP_GAIN_UV, 0x80<<MLC_GAIN_V_SHF, MLC_GAIN_V);

        vWriteCPMsk(RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN, MLC_GAIN_Y_EN);
        vWriteCPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN, MLC_GAIN_U_EN);
        vWriteCPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN, MLC_GAIN_V_EN);*/
    }

}
EXPORT_SYMBOL(vCPVideoSetYUVGain);

void vCPGetYUVGain(s32 *i4YGain,s32 *i4UGain,s32 *i4VGain)//void vCPGppGain(int y,int u,int v)
{
    u32 ygain,uvgain;

    ygain = dReadCP(RW_PCLRP_GAIN_Y);
    uvgain = dReadCP(RW_PCLRP_GAIN_UV);

    *i4YGain = ((ygain & MLC_GAIN_Y) >> MLC_GAIN_Y_SHF);
    *i4UGain = ((uvgain & MLC_GAIN_U) >> MLC_GAIN_U_SHF);
    *i4VGain = ((uvgain & MLC_GAIN_V) >> MLC_GAIN_V_SHF);
    UTIL_Printf("[VCP][INFO]vCPGetYUVGain:GPP get Gain:YUV, (0x%x,0x%x,0x%x) \n",*i4YGain,*i4UGain,*i4VGain);
}


void vCPGppInv(int y,int u,int v)
{
    if (y%2==1)
        vWriteCPMsk(RW_PCLRP_LUMA_Y, INV_Y, INV_Y);//enable
    else
        vWriteCPMsk(RW_PCLRP_LUMA_Y, ~INV_Y, INV_Y);//disable

    if (u%2==1)
        vWriteCPMsk(RW_PCLRP_CHROMA_U, INV_U, INV_U);
    else
        vWriteCPMsk(RW_PCLRP_CHROMA_U, ~INV_U, INV_U);

    if (v%2==1)
        vWriteCPMsk(RW_PCLRP_CHROMA_V, INV_V, INV_V);
    else
        vWriteCPMsk(RW_PCLRP_CHROMA_V, ~INV_V, INV_V);
    UTIL_Printf("[VCP][INFO]GPP INV:(Y %d, U %d, V %d)\n",y, u, v);
}
void vCPGppFix(int y,int u,int v,int y_En,int u_En,int v_En)
{
    vWriteCPMsk(RW_PCLRP_LUMA_Y, y<<FIX_Y_SHF, FIX_Y);
    vWriteCPMsk(RW_PCLRP_CHROMA_U, u<<FIX_U_SHF, FIX_U);
    vWriteCPMsk(RW_PCLRP_CHROMA_V, v<<FIX_V_SHF, FIX_V);

    if (y_En == 1)
        vWriteCPMsk(RW_PCLRP_LUMA_Y, FIX_Y_EN, FIX_Y_EN);
    else
        vWriteCPMsk(RW_PCLRP_LUMA_Y, ~FIX_Y_EN, FIX_Y_EN);
    if (u_En == 1)
        vWriteCPMsk(RW_PCLRP_CHROMA_U, FIX_U_EN, FIX_U_EN);
    else
        vWriteCPMsk(RW_PCLRP_CHROMA_U, ~FIX_U_EN, FIX_U_EN);
    if (v_En == 1)
        vWriteCPMsk(RW_PCLRP_CHROMA_V, FIX_V_EN, FIX_V_EN);
    else
        vWriteCPMsk(RW_PCLRP_CHROMA_V, ~FIX_V_EN, FIX_V_EN);
    UTIL_Printf("[VCP][INFO]---GPP Fix Value--- \n");
    UTIL_Printf("[VCP][INFO]FixY %d, Value 0x%x \n",y_En,y);
    UTIL_Printf("[VCP][INFO]FixU %d, Value 0x%x \n",u_En,u);
    UTIL_Printf("[VCP][INFO]FixV %d, Value 0x%x \n",v_En,v);
}

void vCPGppProcess(s32 i4Mode, s32 i4DatY, s32 i4DatU, s32 i4DatV, s32 i4EnY, s32 i4EnU, s32 i4EnV)
{

    switch (i4Mode)
    {
    case CLRP_GPP_DELAY:
        vCPGppDelay(i4DatY,i4DatU,i4DatV);
        break;
    case CLRP_GPP_OFFSET1:
        vCPGppOffset1(i4DatY,i4DatU,i4DatV);
        break;
    case CLRP_GPP_OFFSET2:
        vCPGppOffset2(i4DatY,i4DatU,i4DatV);
        break;
    case CLRP_GPP_GAIN:
        vCPSetYUVGain(i4DatY,i4DatU,i4DatV);
        break;
    case CLRP_GPP_INV:
        vCPGppInv(i4DatY,i4DatU,i4DatV);
        break;
    case CLRP_GPP_FIX:
        vCPGppFix(i4DatY,i4DatU,i4DatV, i4EnY, i4EnU, i4EnV);
        break;
    case CLRP_GPP_RST:
        vCPGppDelay(0,0,0);             //delay 0 T
        vCPGppOffset1(0,0,0);           //offset1 set  0 before gain
        vCPGppOffset2(0,0,0);           //offset2 set  0 after gain
        vCPSetYUVGain(0x80,0x80,0x80);     //set gain to 1.0 ,as 0x80
        vCPGppInv(0,0,0);               //no inv
        vCPGppFix(0x80,0x80,0x80,0,0,0);//no fix
        break;
    }
}

void vCPBlackWhiteLevelEx(s32 i4Mode, s32 i4Slope, s32 i4Anchor)
{
    s32 i4DisB,i4DisW;
    i4DisB = i4Slope % 2;
    i4DisW = i4Anchor % 2;
    switch (i4Mode)
    {
    case BLACK_LEVEL_EX:
        vWriteCPMsk(RW_PCLRP_BLACK_EXTENSION, i4Slope<<BLEND_SLOPE_SHF, BLEND_SLOPE);   //slope
        vWriteCPMsk(RW_PCLRP_BLACK_EXTENSION, i4Anchor<<BLEND_ANCHOR_SHF, BLEND_ANCHOR);  //start
        vWriteCPMsk(RW_PCLRP_BLACK_EXTENSION, BLEND_ENA, BLEND_ENA);     //en
        vWriteCPMsk(RW_PCLRP_BLACK_EXTENSION, ~BLEV_EDIF, BLEV_EDIF);   //disable err diff
        UTIL_Printf("[VCP][INFO]Black Level Ex:(Slop 0x%x, Anchor 0x%x)\n",i4Slope, i4Anchor);
        break;
    case WHITE_LEVEL_EX:
        vWriteCPMsk(RW_PCLRP_WHITE_EXTENSION, i4Slope<<WLEND_SLOPE_SHF, WLEND_SLOPE);   //slope
        vWriteCPMsk(RW_PCLRP_WHITE_EXTENSION, i4Anchor<<WLEND_ANCHOR_SHF, WLEND_ANCHOR);  //start
        vWriteCPMsk(RW_PCLRP_WHITE_EXTENSION, WLEND_ENA, WLEND_ENA);     //en
        vWriteCPMsk(RW_PCLRP_WHITE_EXTENSION, ~WLEV_EDIF, WLEV_EDIF);   //disable err diff
        UTIL_Printf("[VCP][INFO]White Level Ex:(Slop 0x%x, Anchor 0x%x)\n",i4Slope, i4Anchor);
        break;
    case BW_LEVEL_EX_OFF:
        if (i4DisB)
        {
            vWriteCPMsk(RW_PCLRP_BLACK_EXTENSION, ~BLEND_ENA, BLEND_ENA);     //en
            vWriteCPMsk(RW_PCLRP_BLACK_EXTENSION, ~BLEV_EDIF, BLEV_EDIF);   //disable err diff
        }
		else
		{
			vWriteCPMsk(RW_PCLRP_BLACK_EXTENSION, BLEND_ENA, BLEND_ENA);	 //en
			vWriteCPMsk(RW_PCLRP_BLACK_EXTENSION, ~BLEV_EDIF, BLEV_EDIF);   //disable err diff

		}
        if (i4DisW)
        {
            vWriteCPMsk(RW_PCLRP_WHITE_EXTENSION, ~WLEND_ENA, WLEND_ENA);     //en
            vWriteCPMsk(RW_PCLRP_WHITE_EXTENSION, ~WLEV_EDIF, WLEV_EDIF);   //disable err diff
        }
		else
		{
			vWriteCPMsk(RW_PCLRP_WHITE_EXTENSION, WLEND_ENA, WLEND_ENA);	 //en
			vWriteCPMsk(RW_PCLRP_WHITE_EXTENSION, ~WLEV_EDIF, WLEV_EDIF);   //disable err diff

		}
        UTIL_Printf("[VCP][INFO]BW_Level_Ex Disabel:(BlackDis %d, WhiteDis %d)\n",i4Slope, i4Anchor);
        break;
    default:
        break;
    }
}
#if 0
void vCPSetContrBritSatr(s32 i4Mode, s32 i4Dat)
{
    switch (i4Mode)
    {
    case CP_CONTR_GAIN:
        vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(i4Dat<<CONTRAST_GAIN_SHF),CONTRAST_GAIN);
        UTIL_Printf("vCPSetContrBritSatr:Contrast(Gain): 0x%x )\n",i4Dat);
        break;
    case CP_CONTR_BRIT:
        vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(i4Dat<<BRIGHT_GAIN_SHF),BRIGHT_GAIN);
        UTIL_Printf("vCPSetContrBritSatr:Bright(Offset): 0x%x )\n",i4Dat);
        break;
    case CP_CONTR_SATR:
        vWriteCPMsk(RW_PCLRP_SATURATION,(i4Dat<<SAT_GAIN_SHF),SAT_GAIN);
        UTIL_Printf("vCPSetContrBritSatr:Saturation: 0x%x )\n",i4Dat);
        break;
    case CP_CONTR_RESET:
        vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(0x40<<CONTRAST_GAIN_SHF|0x80<<BRIGHT_GAIN_SHF|0x10<<CONTRINFLUM_SHF),CONTRAST_GAIN|BRIGHT_GAIN|CONTRINFLUM|CONTR_CORE);
        vWriteCPMsk(RW_PCLRP_SATURATION,(0x80<<SAT_GAIN_SHF),SAT_GAIN);
        UTIL_Printf("vCPSetContrBritSatr:Contrast Reset: Gain 0x40(1.0) Bright 0x80(0) Influence 0x10 CoreRej(0) Saturation 0x80\n");
        break;
/*   case CP_CONTR_INFLUM:
             vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(i4Dat<<CONTRINFLUM_SHF),CONTRINFLUM);
             UTIL_Printf("Contrast Influence: 0x%x )\n",i4Dat);
             break;
       case CP_CONTR_CORE:
             vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(i4Dat<<CONTR_CORE_SHF),CONTR_CORE);
             UTIL_Printf("Contrast Core Rejection: 0x%x )\n",i4Dat);
             break;      */
    }

}
#else
void vCPSetContrBritSatr(s32 i4Contr,s32 i4Brit,s32 i4Satr)
{
    vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(i4Contr<<CONTRAST_GAIN_SHF),CONTRAST_GAIN);
    vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(i4Brit<<BRIGHT_GAIN_SHF),BRIGHT_GAIN);
    vWriteCPMsk(RW_PCLRP_SATURATION,(i4Satr<<SAT_GAIN_SHF),SAT_GAIN);
    UTIL_Printf("[VCP][INFO]vCPSetContrBritSatr:Contrast(Gain): 0x%x )\n",i4Contr);
    UTIL_Printf("[VCP][INFO]vCPSetContrBritSatr:Bright(Offset): 0x%x )\n",i4Brit);
    UTIL_Printf("[VCP][INFO]vCPSetContrBritSatr:Saturation: 0x%x )\n",i4Satr);
}

void vCPVideoSetContrBritSatr(VIDEO_SRC_TYPE src_type);
void vCPAppSetContrBritSatr(s32 i4Contr,s32 i4Brit,s32 i4Satr,VIDEO_SRC_TYPE src_type)
{
    int srcnum = -1;

    if (i4Contr < 0 || i4Contr > 0xff || i4Brit < 0 || i4Brit > 0xff || i4Satr < 0 || i4Satr > 0xff) {
        UTIL_Printf("[VCP][ERR]vCPAppSetGlobalHue:Set SCE Global contrast = 0x%x, brightness = 0x%x, saturation = 0x%x error\n", i4Contr, i4Brit, i4Satr);
        return;
    }
    if (src_type < 0xa || src_type > 0x15) {
        UTIL_Printf("[VCP][ERR]vCPAppSetContrBritSatr error src_type: 0x%x, error\n",src_type);
        return;
    }

    vCPSetContrBritSatr(i4Contr, i4Brit, i4Satr);

    switch(src_type)
    {
         case AVIN_1:
            {
                if(fgavin_1[CBS])
                    break;
                else{
                    srcnum = 0;
                    fgavin_1[CBS] = true;
                    break;
                }
            }
         case AVIN_2:
            {
                if(fgavin_2[CBS])
                    break;
                else{
                    srcnum = 1;
                    fgavin_2[CBS] = true;
                    break;
                }
            }
         case AVIN_3:
            {
                if(fgavin_3[CBS])
                    break;
                else{
                    srcnum = 2;
                    fgavin_3[CBS] = true;
                    break;
                }
            }
         case AVIN_4:
            {
                if(fgavin_4[CBS])
                    break;
                else{
                    srcnum = 3;
                    fgavin_4[CBS] = true;
                    break;
                }
            }
         case AVIN_5:
            {
                if(fgavin_5[CBS])
                    break;
                else{
                    srcnum = 4;
                    fgavin_5[CBS] = true;
                    break;
                }
            }
         case BACKCAR:
            {
                if(fgbkcar[CBS])
                    break;
                else{
                    srcnum = 5;
                    fgbkcar[CBS] = true;
                    break;
                }
            }
         case USB:
            {
                if(fgusb[CBS])
                    break;
                else{
                    srcnum = 6;
                    fgusb[CBS] = true;
                    break;
                }
            }
         case DVD:
            {
                if(fgdvd[CBS])
                    break;
                else{
                    srcnum = 7;
                    fgdvd[CBS] = true;
                    break;
                }
            }
         case YPBPR:
            {
                if(fgypbpr[CBS])
                    break;
                else{
                    srcnum = 8;
                    fgypbpr[CBS] = true;
                    break;
                }
            }
         case VGA:
            {
                if(fgvga[CBS])
                    break;
                else{
                    srcnum = 9;
                    fgvga[CBS] = true;
                    break;
                }
            }
         case DGI:
            {
                if(fgdgi[CBS])
                    break;
                else{
                    srcnum = 10;
                    fgdgi[CBS] = true;
                    break;
                }
            }
         case VDO_HDMI:
            {
                if(fghdmi[CBS])
                    break;
                else{
                    srcnum = 11;
                    fghdmi[CBS] = true;
                    break;
                }
            }
         default:
            {
                UTIL_Printf("[VCP][INFO]vCPAppSetContrBritSatr:not surport this srctype\n");
                break;
            }
    }  

    if(srcnum != -1)
    {
        vcpinfo_c[srcnum].i4Contr = i4Contr;
        vcpinfo_c[srcnum].i4Brit = i4Brit;
        vcpinfo_c[srcnum].i4Satr = i4Satr;
        vcpinfo_c[srcnum].SrcType = src_type;
        vCPVideoSetContrBritSatr(src_type);
    } else {
	for (srcnum = 0; srcnum < srcMax; srcnum++)
                vcpinfo_c[srcnum].SrcType = 0;
    }
 
    UTIL_Printf("[VCP][INFO]vCPAppSetContrBritSatr:Pre GPP set Gain:CBS, (0x%x,0x%x,0x%x) \n",vcpinfo_c[srcnum].i4Contr,vcpinfo_c[srcnum].i4Brit,vcpinfo_c[srcnum].i4Satr);

}

void vCPVideoSetContrBritSatr(VIDEO_SRC_TYPE src_type)
{
    int srcnum = -1;
    bool fgsetvalue = false;

    switch(src_type)
    {
         case AVIN_1:
            {
                srcnum = 0;
                if(fgavin_1[CBS]) {
                    fgsetvalue = true;
                    fgavin_1[CBS] = false;
                }
                break;
            }
         case AVIN_2:
            {
                srcnum = 1;
                if(fgavin_2[CBS]) {
                    fgsetvalue = true;
                    fgavin_2[CBS] = false;
                }
                break;
            }
         case AVIN_3:
            {
                srcnum = 2;
                if(fgavin_3[CBS]) {
                    fgsetvalue = true;
                    fgavin_3[CBS] = false;
                }
                break;
            }
         case AVIN_4:
            {
                srcnum = 3;
                if(fgavin_4[CBS]) {
                    fgsetvalue = true;
                    fgavin_4[CBS] = false;
                }
                break;
            }
         case AVIN_5:
            {
                srcnum = 4;
                if(fgavin_5[CBS]) {
                    fgsetvalue = true;
                    fgavin_5[CBS] = false;
                }
                break;
            }
         case BACKCAR:
            {
                srcnum = 5;
                if(fgbkcar[CBS]) {
                    fgsetvalue = true;
                    fgbkcar[CBS] = false;
                }
                break;
            }
         case USB:
            {
                srcnum = 6;
                if(fgusb[CBS]) {
                    fgsetvalue = true;
                    fgusb[CBS] = false;
                }
                break;
            }
         case DVD:
            {
                srcnum = 7;
                if(fgdvd[CBS]) {
                    fgsetvalue = true;
                    fgdvd[CBS] = false;
                }
                break;
            }
         case YPBPR:
            {
                srcnum = 8;
                if(fgypbpr[CBS]) {
                    fgsetvalue = true;
                    fgypbpr[CBS] = false;
                }
                break;
            }
         case VGA:
            {
                srcnum = 9;
                if(fgvga[CBS]) {
                    fgsetvalue = true;
                    fgvga[CBS] = false;
                }
                break;
            }
         case DGI:
            {
                srcnum = 10;
                if(fgdgi[CBS]) {
                    fgsetvalue = true;
                    fgdgi[CBS] = false;
                }
                break;
            }
         case VDO_HDMI:
            {
                srcnum = 11;
                if(fghdmi[CBS]) {
                    fgsetvalue = true;
                    fghdmi[CBS] = false;
                }
                break;
            }
         default:
            {
                UTIL_Printf("[VCP][INFO]vCPVideoSetContrBritSatr:not surport this srctype\n");
                break;
            }
    } 
    if(fgsetvalue)
    {/*
        vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(vcpinfo_c[srcnum].i4Contr<<CONTRAST_GAIN_SHF),CONTRAST_GAIN);
        vWriteCPMsk(RW_PCLRP_BRIGHT_CONT,(vcpinfo_c[srcnum].i4Brit<<BRIGHT_GAIN_SHF),BRIGHT_GAIN);
        vWriteCPMsk(RW_PCLRP_SATURATION,(vcpinfo_c[srcnum].i4Satr<<SAT_GAIN_SHF),SAT_GAIN);*/
        UTIL_Printf("[VCP][INFO]vCPSetContrBritSatr:Contrast(Gain): 0x%x )\n",vcpinfo_c[srcnum].i4Contr);
        UTIL_Printf("[VCP][INFO]vCPSetContrBritSatr:Bright(Offset): 0x%x )\n",vcpinfo_c[srcnum].i4Brit);
        UTIL_Printf("[VCP][INFO]vCPSetContrBritSatr:Saturation: 0x%x )\n",vcpinfo_c[srcnum].i4Satr);
    }
    else
    {/*
        vWriteCPMsk(RW_PCLRP_BRIGHT_CONT, 0x40<<CONTRAST_GAIN_SHF, CONTRAST_GAIN);
        vWriteCPMsk(RW_PCLRP_BRIGHT_CONT, 0x80<<BRIGHT_GAIN_SHF, BRIGHT_GAIN);
        vWriteCPMsk(RW_PCLRP_SATURATION, 0x80<<SAT_GAIN_SHF, SAT_GAIN);*/
    }

}
EXPORT_SYMBOL(vCPVideoSetContrBritSatr);

void vCPVideoSetVCP(VIDEO_SRC_TYPE src_type)
{
    vCPVideoSetGlobalHue(src_type);
    vCPVideoSetYUVGain(src_type);
    vCPVideoSetContrBritSatr(src_type);
}
EXPORT_SYMBOL(vCPVideoSetVCP);

void vCPGetContrBritSatr(s32 *i4Contr,s32 *i4Brit,s32 *i4Satr)
{
    *i4Contr = ((dReadCP(RW_PCLRP_BRIGHT_CONT)&CONTRAST_GAIN)>>CONTRAST_GAIN_SHF);
    *i4Brit = ((dReadCP(RW_PCLRP_BRIGHT_CONT)&BRIGHT_GAIN)>>BRIGHT_GAIN_SHF);
    *i4Satr = ((dReadCP(RW_PCLRP_SATURATION)&SAT_GAIN)>>SAT_GAIN_SHF);
    UTIL_Printf("[VCP][INFO]vCPGetContrBritSatr:Saturation: 0x%x )\n",*i4Contr);
    UTIL_Printf("[VCP][INFO]vCPGetContrBritSatr:Bright(Offset): 0x%x )\n",*i4Brit);
    UTIL_Printf("[VCP][INFO]vCPGetContrBritSatr:Contrast(Gain): 0x%x )\n",*i4Satr);
}

#endif
void vCPSetSceEn(s32 i4En)
{
    if (i4En)
    {
        vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,MLC_SCE_ENA,MLC_SCE_ENA);// SEC disable,
        vWriteCPMsk(RW_PCLRP_LUMA_SCECTRL,MLC_MOD_ENA,MLC_MOD_ENA);  //SEC enable,Y
        UTIL_Printf("[VCP][INFO]SEC Enable\n");
    }
    else
    {
        vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,~MLC_SCE_ENA,MLC_SCE_ENA);// SEC disable,
        vWriteCPMsk(RW_PCLRP_LUMA_SCECTRL,~MLC_MOD_ENA,MLC_MOD_ENA);  //SEC enable,Y
        UTIL_Printf("[VCP][INFO]SEC Disable\n");
    }
}
void vCPSetSCE2(unsigned int sceEn,unsigned int luma,unsigned int hue,unsigned int sat)
{
    DWRD dwData;
    WORD wCnt;
    BYTE bCnt;
    unsigned int sce_data = 0;
    sce_data = (luma<<14) | (hue<<8) | (sat<<0);

    if (sceEn == 1)
    {
        vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,SRAM_WR_MODE,SRAM_WR_MODE);// SEC sram write mode
        for (wCnt=0;wCnt<=359;wCnt++)
        {
            dwData = (sce_data<<10) |(wCnt<<1); //
            //dwData = (0x200080<<10) |(wCnt<<1); //
            vWriteCP(RW_PCLRP_SCE_TABLE,dwData|0x01); //write vector and write bit


            for (bCnt=0;bCnt<=10;bCnt++)//delay??
            {
                dReadCP(0x0c);
            }
            vWriteCP(RW_PCLRP_SCE_TABLE,dwData|0x00); //write vector and clear write bit
            for (bCnt=0;bCnt<=10;bCnt++)//delay??
            {
                dReadCP(0x0c);
            }
        }
        vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,MLC_SCE_ENA,MLC_SCE_ENA);// SEC enable, UV

        vWriteCPMsk(RW_PCLRP_LUMA_SCECTRL,MLC_MOD_ENA, MLC_MOD_ENA);  //SEC enable,Y

//        vWriteCPMsk(RW_PCLRP_LUMA_SCECTRL, MLC_MOD_LMT, MLC_MOD_LMT);  //set limit to 255
  //      vWritePCLRPMsk(RW_PCLRP_LUMA_SCECTRL, MOD_MUL, MLC_MOD_TYPE);    //gain mode
        UTIL_Printf("[VCP][INFO]SCE En:%d, luma:0x%x, hue:0x%x, sat:0x%x, lum_limit:255, LumaType: Gain)\n",sceEn, luma, hue, sat);
    }
    else
    {
        vWriteCPMsk(RW_PCLRP_HUE_SCECTRL,~MLC_SCE_ENA,MLC_SCE_ENA);// SEC disable,

        vWriteCPMsk(RW_PCLRP_LUMA_SCECTRL,~MLC_MOD_ENA,MLC_MOD_ENA);  //SEC enable,Y

        UTIL_Printf("[VCP][INFO]SEC Disable\n");
    }
}


void vCPSetSCELumaMode(unsigned int mode, s32 i4Limit)
{
    if (mode == 1)
    {
        vWriteCPMsk(RW_PCLRP_LUMA_SCECTRL, MOD_MUL, MLC_MOD_TYPE);    //gain mode
        UTIL_Printf("[VCP][INFO]SEC Luma Type : Gain\n");
    }
    else
    {
        vWriteCPMsk(RW_PCLRP_LUMA_SCECTRL, MOD_SUB, MLC_MOD_TYPE);    //sub mode
        UTIL_Printf("[VCP][INFO]SEC Luma Type : Add\n");
    }
    vWriteCPMsk(RW_PCLRP_LUMA_SCECTRL, (i4Limit<<MLC_MOD_LMT_SHF), MLC_MOD_LMT);    //sub mode
    UTIL_Printf("[VCP][INFO]SEC Luma Limit : 0x%x\n", i4Limit);
}
void vCPSCE(s32 i4Mode, s32 i4Luma, s32 i4Hue, s32 i4Sat, s32 i4Dat)
{
    s32 i4Type,i4Limit;
    UNUSED(i4Type);
    UNUSED(i4Limit);
    switch (i4Mode)
    {
    case CLRP_SCE_TABLE:
        vCPSetSCE2(0x01, i4Luma, i4Hue, i4Sat);
        break;
    case CLRP_SCE_ONOFF:
        vCPSetSceEn(i4Luma);
//		vPanelSetSCE2(0x0, i4Luma, i4Hue, i4Sat);
        break;
    case CLRP_SCE_LUMATYPE:
        i4Type = i4Luma %2;
        i4Limit = i4Hue & 0xff;
        vCPSetSCELumaMode(i4Type,i4Limit);
        break;
    case CLRP_SCE_TEST:
        vCPSetSCE(NULL);
        break;
    case CLRP_SCE_HUE_GLOBAL:
		// Hue Range 0x0~0x3F (from arg1:i4Luma)
		vCPSetGlobalHue(i4Luma);
		break;
    case CLRP_SCE_SETTING_CFG:
		//Config GlobHuePrec PartialHuePrec UClamp VClamp (from arg1 to arg 4)
		vCPSCEHueConfig(i4Luma, i4Hue, i4Sat, i4Dat);
		break;
    case CLRP_SCE_GET_TABLE:
 		vCPGetSCE(NULL);
		break;
    case CLRP_SCE_USER:
        UTIL_Printf("[VCP][INFO]Please Load SCE Table First\n");
        break;
    }
}

//void vTconCTI(INT32 i4Level)
void vCPCTI(s32 i4Mode, s32 i4Value0, s32 i4Value1)
{
    s32 i4En,i4T_Sel,i4Gain_Sharp;
	u32 i4Config;
	UNUSED(i4En);
	UNUSED(i4T_Sel);
	UNUSED(i4Gain_Sharp);
	UNUSED(i4Config);
	switch (i4Mode)
	{
	case 0:	 	 //tune T_sel & gain sharp (coarse gain && fine gain)
		i4T_Sel = i4Value0 < 3 ? i4Value0 : 7;
		i4Gain_Sharp = i4Value1 & 0x7f;
	    vWriteCPMsk(RW_PCLRP_CTI,i4T_Sel<<CTI_T_SELECT_SHF,CTI_T_SELECT);
		vWriteCPMsk(RW_PCLRP_CTI,i4Gain_Sharp<<CTI_GAIN_SHARP_SHF,CTI_GAIN_SHARP);
		UTIL_Printf("[VCP][INFO]CTI CoarseGain 0x%x, FineGain 0x%x\n",i4T_Sel,i4Gain_Sharp);
	 	break;
	case 1:  //LPF Configure
	   i4En = i4Value0%2;
	   i4Config = i4Value1&0x3;
	   vWriteCPMsk(RW_PCLRP_CTI,i4En<<CTI_LP_ENA_SHF,CTI_LP_ENA);
	   vWriteCPMsk(RW_PCLRP_CTI,i4Config<<CTI_LP_SEL_SHF,CTI_LP_SEL);
	   UTIL_Printf("[VCP][INFO]CTI LPF EN0x%x, LPF SEL 0x%x\n",i4En, i4Config);
	   break;
	case 2:
		i4En = i4Value0%2;
		i4Config = i4Value1&0xff;
 	    vWriteCPMsk(RW_PCLRP_CTI2,i4En<<HDDETECT_EN_SHF,HDDETECT_EN);
		vWriteCPMsk(RW_PCLRP_CTI2,i4Config<<HD_AMP_SHF,HD_AMP);
		UTIL_Printf("[VCP][INFO]CTI AMP_EN 0x%x, HD_AMP 0x%x\n",i4En, i4Config);
		break;
	case 3:
		i4En = i4Value0%2;
 	    vWriteCPMsk(RW_PCLRP_CTI,i4En<<CTI_MAX_MIN_JG_SHF,CTI_MAX_MIN_JG);
		UTIL_Printf("[VCP][INFO]CTI MAX_MIN_JG En 0x%x\n",i4En);
		break;
	case 4:
		i4En = i4Value0%2;
		vWriteCPMsk(RW_PCLRP_CTI,i4En<<CTI_PTADDSUB_INV_SHF,CTI_PTADDSUB_INV);
		UTIL_Printf("[VCP][INFO]CTI ANDSUB_INV En 0x%x\n",i4En);
		break;
	case 5:
		i4Config = i4Value0&0xF;
		vWriteCPMsk(RW_PCLRP_CTI,i4Config<<CTI_PTADDSUB_INV_SHF,CTI_PTADDSUB_INV);
		UTIL_Printf("[VCP][INFO]CTI DZONE 0x%x\n",i4Config);
		break;
	case 6:
		i4Config = i4Value0&0xFFFFF;
		vWriteCPMsk(RW_PCLRP_CTI1,i4Config<<CTI_FIR_COEFF_SHF,CTI_FIR_COEFF);
		UTIL_Printf("[VCP][INFO]CTI FIR Coeff 0x%x Default:[0x15654]\n",i4Config);
		break;
	default:
		UTIL_Printf("[VCP][WARN]Please check cmd Arg before test cti\n");
		break;
	 }
}
void vCPUV2CbCr(u32 i4En, u32 u4GainU, u32 u4GainV, u32 u4Sign)
{
    vWriteCPMsk(RW_UV2CBCR_CONV,(u4GainU<<MLC_U2CB_GAIN_SHF|u4GainV<<MLC_V2CR_GAIN_SHF|\
                                    i4En<<MLC_UV_CONV_EN_SHF|u4Sign<<MLC_OUTFRONT_SHF),MLC_U2CB_GAIN|MLC_V2CR_GAIN|MLC_UV_CONV_EN|MLC_OUTFRONT);
    UTIL_Printf("[VCP][INFO]UV2CbCr: En %d, UGain:0x%x, VGain:0x%x, Sign:%d\n",i4En, u4GainU, u4GainV, u4Sign);
}

void vCPSuppression(u32 u4Mode, u32 u4En, u32 u4Gain, u32 u4Offset, u32 u4SubDiv, u32 u4Spc, u32 u4Spcc)
{
    switch (u4Mode)
    {
    case 0:

        vWriteCPMsk(RW_PCLRP_SUPPRESSION,(u4Gain<<SPC_GAIN_SHF|u4Offset<<SEED_OFFSET_SHF|u4En<<SPC_ENA_SHF),SPC_GAIN|SEED_OFFSET|SPC_ENA);
        vWriteCPMsk(RW_PCLRP_SUPPRESSION,(u4SubDiv<<SPC_SUB_DIV_SHF|u4Spc<<SPC_SEL_SHF|u4Spcc<<SPCC_SEL_SHF),SPC_SUB_DIV|SPC_SEL|SPCC_SEL);
        UTIL_Printf("[VCP][INFO]SPC:En %d, Gain %d, Offset %d, SubDiv %d, Spc %d, Spcc %d\n", u4En, u4Gain, u4Offset, u4SubDiv, u4Spc, u4Spcc);
        break;
    case 1:    // Low pass filter  En & Sel
        u4Gain %= 0x4;
        vWriteCPMsk(RW_PCLRP_SUPPRESSION, (u4En<<SPC_LP_ENA_SHF|u4Gain<<SPC_LP_SEL_SHF), SPC_LP_ENA|SPC_LP_SEL);
        UTIL_Printf("[VCP][INFO]SPC(Low Pass Filter): En %d, Sel: %d\n",u4En, u4Gain);
        break;
    }
}

#endif


#endif
