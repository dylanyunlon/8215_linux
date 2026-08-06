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
 * @file aud_io_manager.c source file
 * 
 * aud io manager source code
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_io_manager.h"
#include "aud_3360_reg_rw.h"



//==============================================//
#define CodeSight_IoManager_Basic_Fun
//==============================================//

/**
 * allocate buffer for aout
 *
 * @param [in]  pThis : hal class
 * @param [out] 
 *
 * @return
 */
void AudIoMg_InitHw(bool fgPowerOnByArm9)
{
    AUD_CLK_POWER_CTL_MODULE_ID eClkPowerCtl;
    
#ifdef audio_clock_standardization
    IoClk_getAudioClock();
#endif
    if (!fgPowerOnByArm9)
    {
        IoClk_SetAdspPowerOn();

        for (eClkPowerCtl = CLKPM_MPHONE; eClkPowerCtl < CLKPM_MAX; eClkPowerCtl++)
        {
            IoClk_SetModulePowerOn(eClkPowerCtl);
        }
        
#ifdef audio_clock_standardization
        IoClk_SetGateClock(1);
        IoClk_SetGateResetEnable(1);
#endif
        //audio related hw reset
        IoClk_SetDspHwRest();
        IoClk_SetPwmHwRest();
        
        //pwm dac basic setting
        DacHal_SetPwmBasicSetting();
    }

    //pin mux default setting
    IoPinMux_SetDefaultCfg(fgPowerOnByArm9);   
}

