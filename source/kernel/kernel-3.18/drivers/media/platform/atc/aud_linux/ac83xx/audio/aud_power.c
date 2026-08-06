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



/************************************************************************************************/
                                 /*Headfile include*/
/************************************************************************************************/
#include "aud_power.h"
#include "DspFunc.h"  
#include "AsvDspCtrl.h"
#include "aud_debug.h"
#include "aud_drv_config.h"
#include "audin_if.h"
#include "aud_io_clock_if.h"
#include "aud_comm_reg_rw.h"
#include "DspUop.h"
#ifdef __linux__
#include "pcm_ac83xx.h"
#else
#include "waveform_if.h"
#endif

#if (0 == CONFIG_AUD_PM_SIMPLE_VERSION)
#ifdef __linux__
static DEFINE_SPINLOCK(ac83xx_aud_power_lock);
#else
CRIT_STATE_T rState;
#endif
#endif


#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
extern void vSendADSPCmd(u32 u4Cmd);
extern AUDIO_POWER_MANAGEMNT_DEVICEID_CONTROL_UNION_T  g_rDevPowerCtrl;
extern AUDIO_SAMPLING_T g_eDvdSamplRate;
extern volatile bool g_fgStopping;

/************************************************************************************************/
                                 /*Function*/
/************************************************************************************************/
/***************************************************************************
*    Function : 
* Description : drv init,should dsp real power on. 
*                    just dsp return interrupt include dspa & dspb               
*   Parameter : 
*   Return    : None
*   Note      : 
***************************************************************************/
void Aud_DrvInitPower(void)
{
    s32 i;
    LOG(LOG_POWER,    TEXT("[Aud_power]Enter aud power init function\r\n"));
    g_rDevPowerCtrl.dPowerControlWord = 0;
    g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE = DEVICE_POWERON;

    i = 100;
    while(i > 0)
    {
        if (fgDspAWakeup() && fgDspBWakeup())
        {
            AudDrvThreadInit();
            Aud_Linein_Init();
            AudGpsMix_DrvInit();
            GpsMix_SetAoutCfg();
            break;
        }
#ifndef __linux__
        Sleep(20);
#else
        mdelay(20);
#endif
        
        i--;
    }
    
    if (i > 0)
    {
        LOG(LOG_POWER,    TEXT("[Aud_power]vAud_PowerOn OK !!! \n"));
    }
    else
    {
        LOG(LOG_POWER,    TEXT("[Aud_power]vAud_PowerOn Fail !!!! \n"));
    }
}


/***************************************************************************
*    Function : 
* Description : power down dsp 
*                    include dsp:5028,bit4&16,47028.bit4
                      dsp clock,
                      dsp interrupt flag
*   Parameter : 
*   Return    : None
*   Note      :  first close dsp ,then close dsp clock
***************************************************************************/
void vAud_PowerDown(void)
{
    AUD_CLK_POWER_CTL_MODULE_ID eClkPowerCtl;
    
    LOG(LOG_POWER, TEXT("Enter vAud_PowerDown Function ......\n"));

#ifndef __linux__
    Wav_HinernationCtrl(FALSE);
#else
    card_audio_hibernation(FALSE);
#endif

    AudCfg_SpdifEnable(5); //Close spdif out
    g_eDvdSamplRate = FS_UNKNOWN;

    AudPower_Deinit();
     
    vDspPowerOff();
    
    for (eClkPowerCtl = CLKPM_MPHONE; eClkPowerCtl < CLKPM_MAX; eClkPowerCtl++)
    {
        IoClk_SetModulePowerDown(eClkPowerCtl);
    }

    LOG(LOG_POWER, TEXT(" power down clk state1 : 0xA8=(0x%x), 0xA8084=(0x%x) \n"), AUDREG_READ(0xA8), AUDREG_READ(0xA8084));
    
    IoClk_SetAdspPowerDown();
 
    vPowerDownDsp();

    g_fgStopping = FALSE;
     
    LOG(LOG_POWER, TEXT(" power down clk state2 : 0xA8=(0x%x), 0xA8084=(0x%x) \n"), AUDREG_READ(0xA8), AUDREG_READ(0xA8084));
     
    LOG(LOG_POWER, TEXT("Leave vAud_PowerDown Function\n"));
}

/***************************************************************************
*    Function : 
* Description : power on dsp 
*                    include dsp:5028,bit4&16,47028.bit4
                      dsp clock,
                      dsp interrupt flag
*   Parameter : 
*   Return    : None
*   Note      : first open dsp clock,then open dsp
***************************************************************************/
void vAud_PowerOn(void)
{
    s32 i;

    vDspHibernationOnInit();

    AudPower_Init();

  #ifndef __linux__
    Wav_HinernationCtrl(TRUE);
  #else
    card_audio_hibernation(TRUE);
  #endif
    
    _u1DspAoutState = ST_DSP_A_AOUT_OFF;
    _u1DspAout2State = ST_DSP_A_AOUT_OFF;
 
    i = 100;
    while(i > 0)
    {
        if(fgDspAWakeup() && fgDspBWakeup())
        {
            u1AsvDspAAoutOn();
            u1AsvDspAAout2On();

            break;
        }
#ifndef __linux__
        Sleep(20);
#else
        msleep(20);
#endif
        i--;
    }
  
    if (i > 0)
    {
        LOG(LOG_POWER,    TEXT("[Aud_power]vAud_PowerOn OK !!! \n"));
    }
    else
    {
        LOG(LOG_POWER,    TEXT("[Aud_power]vAud_PowerOn Fail !!!! \n"));
    }

    LOG(LOG_POWER,    TEXT("[Aud_power]iec config.\n"));
    vSendADSPCmd(UOP_DSP_IEC_FLAG);
    vSendADSPCmd(UOP_DSP_MASTER_VOLUME);
}

/***************************************************************************
*    Function : 
* Description :  revice every module powerdown request 
*                   
*   Parameter : 
*   Return    : None
*   Note      : 
***************************************************************************/
#if CONFIG_AUD_PM_SIMPLE_VERSION
void AudDev_PowerDown(DEVICE_ID_PM eDId)
{
    vAud_PowerDown();
}

void AudDev_PowerOn(DEVICE_ID_PM eDId)
{
    vAud_PowerOn();
}

#else
void AudDev_PowerDown(DEVICE_ID_PM eDId)
{
   #ifdef __linux__
    u32 flags;
   #endif
    
    //LOG(LOG_POWER, TEXT("Enter AudDev_PowerDown Function ....\n")); 
    LOG(LOG_POWER, TEXT("[AudDev_PowerDown Start] DSP_STATE = 0x%x\n"),g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE);

    switch (eDId)
    {
    case AUD_DEVICE_ID_PRIMARY:
        g_rDevPowerCtrl.DeviceIdPowerControlBit.PRIMARY_DEVICEID = DEVICE_POWERDOWN;
        LOG(LOG_POWER, TEXT("Now you Will Power Down Primary Module.....\n")); 
        break;

    case AUD_DEVICE_ID_FOUR:
        g_rDevPowerCtrl.DeviceIdPowerControlBit.FOUR_DEVICEID = DEVICE_POWERDOWN;
        LOG(LOG_POWER, TEXT("Now you Will Power Down FOUR Module.....\n"));
        break;

    case AUD_DEVICE_ID_GPSMIX:
        g_rDevPowerCtrl.DeviceIdPowerControlBit.GPSMIX_DEVICEID = DEVICE_POWERDOWN;
        LOG(LOG_POWER, TEXT("Now you Will Power Down GPsMix Module.....\n"));
        break;

    case AUD_DEVICE_ID_DVD:
        g_rDevPowerCtrl.DeviceIdPowerControlBit.DVD_DEVICEID = DEVICE_POWERDOWN;
        LOG(LOG_POWER, TEXT("Now you Will Power Down DVD Module.....\n")); 
        break;

    default:
        LOG(LOG_POWER, TEXT("No This Device ID,Can Not Power Down please\n"));
        break;
    }
     
    if(g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE == DEVICE_POWERON)
    {
        if ((g_rDevPowerCtrl.dPowerControlWord & 0x7) == 0) //if use AUD_DEVICE_ID_DVD, use 0xF
        {
            vAud_PowerDown();
        }   
    }
    else
    {
        LOG(LOG_POWER, TEXT("DSP Already Power Down,Cant Access !\n"));
    }
    
    spin_lock_irqsave(&ac83xx_aud_power_lock, flags);

    g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE = DEVICE_POWERDOWN;

    spin_unlock_irqrestore(&ac83xx_aud_power_lock, flags);
      
    LOG(LOG_POWER, TEXT("[AudDev_PowerDown Finish] DSP_STATE = 0x%x\n"),g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE);
    //LOG(LOG_POWER, TEXT("Leave AudDev_PowerDown\n"));
}

/***************************************************************************
*    Function : 
* Description :  revice every module power on request 
*                   
*   Parameter : 
*   Return    : None
*   Note      : 
***************************************************************************/
void AudDev_PowerOn(DEVICE_ID_PM eDId)
{   
   #ifdef __linux__
    u32 flags;
   #endif
    
    //LOG(LOG_POWER, TEXT("Enter AudDev_PowerOn Function\n"));
    LOG(LOG_POWER, TEXT("[AudDev_PowerOn Start] DSP_STATE = 0x%x\n"),g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE);
    
    switch (eDId)
    {
    case AUD_DEVICE_ID_PRIMARY:
        g_rDevPowerCtrl.DeviceIdPowerControlBit.PRIMARY_DEVICEID = DEVICE_POWERON;
        LOG(LOG_POWER, TEXT("Now you Will Power On Primary Module.....\n")); 
        break;

    case AUD_DEVICE_ID_FOUR:
        g_rDevPowerCtrl.DeviceIdPowerControlBit.FOUR_DEVICEID = DEVICE_POWERON;
        LOG(LOG_POWER, TEXT("Now you Will Power On FOUR Module.....\n"));
        break;

    case AUD_DEVICE_ID_GPSMIX:
        g_rDevPowerCtrl.DeviceIdPowerControlBit.GPSMIX_DEVICEID = DEVICE_POWERON;
        LOG(LOG_POWER, TEXT("Now you Will Power On GPsMix Module.....\n"));
        break;
    
    case AUD_DEVICE_ID_DVD:
        g_rDevPowerCtrl.DeviceIdPowerControlBit.DVD_DEVICEID = DEVICE_POWERON;
        LOG(LOG_POWER, TEXT("Now you Will Power On DVD Module.....\n")); 
        break;
    
    default:
        LOG(LOG_POWER, TEXT("No This Device ID,Can Not Power On please\n"));
        break;
    }

    if(g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE == DEVICE_POWERDOWN)
    {
        LOG(LOG_POWER, TEXT("You Will Power On Four Module ....\n"));
        vAud_PowerOn();
    }
    else
    {        
        LOG(LOG_POWER, TEXT("DSP Already Power On,Cant Access !\n"));
    }
    
    spin_lock_irqsave(&ac83xx_aud_power_lock, flags);
    g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE = DEVICE_POWERON;
    spin_unlock_irqrestore(&ac83xx_aud_power_lock, flags);

    LOG(LOG_POWER, TEXT("[AudDev_PowerOn Finish]  DSP_STATE = 0x%x\n"), g_rDevPowerCtrl.DeviceIdPowerControlBit.DSP_STATE);    
    //LOG(LOG_POWER, TEXT("Level AudDev_PowerOn Function\n"));
}
#endif //#if CONFIG_AUD_PM_SIMPLE_VERSION

#endif // #if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT


