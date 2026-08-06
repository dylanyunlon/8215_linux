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

/********************************************************************************************
*	Description:
*         This is Audio-In   driver API source file
************************************************************************************************/

/*-----------------------------------------------------------------------------
-----------------------Include files------------------- -----------------------
------------------------------------------------------------------------------*/
#include "aud_oal.h"
#include "audmhl_if.h"
#include "aud_debug.h"
#include "audmhl_mw.h"
#include "aud_debug.h"

#if CONFIG_DRV_HDMI_RX

extern bool g_fgDateGeted;


/*------------------------------------------------------------------------------
-----------------------Functions definition-------------------------------------
-------------------------------------------------------------------------------*/

/************************************************************************************************
 Function Name: AudmhlSetCtrl(u32 u4SetType)
 Function Description:  set audio hdmi play/stop
 input para: u4SetType: 
 output para:
************************************************************************************************/
s32 AudmhlSetCtrl(u32 u4SetType)
{
    AUDMHL_SET_T  rMhlSetInfo = {0};
    
    rMhlSetInfo.u1Input = AUDINPORT_INTERNAL_HDMI_RX;
    rMhlSetInfo.eAudioDigiDet = AUDIN_ENABLE;
    rMhlSetInfo.eAudmhlInType = AUDMHL_IN;

#ifdef MHLBonding
    if(!fgAudBondingSupport("PROT_HomeTheater"))
    {
         LOG(LOG_CTRLF, TEXT("Bonding: Audmhl In unSupport by bonding.\r\n"));
         return (RMR_DRV_SET_FAILED);  
    }
#endif

    switch (u4SetType)
    {
    case AUDMHL_SET_INIT:        
        LOG(LOG_DATAF, TEXT("Set Audio mhl in Init.\r\n"));   
        AudmhlInit(&rMhlSetInfo);
        break;    

    case AUDMHL_SET_INPUT_SWITCH:  
        // Please always set Audio In Type before Turn on  multiple line in module            
        LOG(LOG_DATAF, TEXT("Set Audio mhl input switch.\r\n"));
        
        if (rMhlSetInfo.eAudmhlInType == AUDMHL_IN)
        {           
            AudmhlSwitchFunc(rMhlSetInfo.u1Input, rMhlSetInfo.eAudioDigiDet);   
        }
        break;
        
    case AUDMHL_SET_ON:
    case AUDMHL_SET_OFF:    
        LOG(LOG_DATAF, TEXT("Set audio mhl input onoff = 0x%x.\r\n"), u4SetType);
        if(AUDMHL_IN == rMhlSetInfo.eAudmhlInType)
        {
            if(AUDMHL_SET_ON == u4SetType)
            {
                AudmhlInCtrl(TRUE);
            }
            else if(AUDMHL_SET_OFF == u4SetType)
            {
                g_fgDateGeted = FALSE;   
                AudmhlSetAudOnOff(FALSE);
            }
        }
        break;

    case AUDMHL_SET_UNINIT:
        LOG(LOG_CTRLF, TEXT("AUDIN_SET_UNINIT.\r\n"));
        AudmhlUnInit(rMhlSetInfo.eAudmhlInType);

        break;

      default:
          return (RMR_DRV_INV_SET_INFO);
    } 

    return (AUDMHL_OK);
}

void AudmhlSwitch(AUDMHL_OPEN_CTRL eCtrl)
{
    if(AUDMHL_OPEN == eCtrl)
    {
        LOG(LOG_CTRLF, TEXT("AUDMHL_OPEN.\r\n"));
        AudmhlSetCtrl(AUDMHL_SET_INIT);        
        AudmhlSetCtrl(AUDMHL_SET_INPUT_SWITCH);
    }
    else if(AUDMHL_CLOSE == eCtrl)
    {
        LOG(LOG_CTRLF, TEXT("AUDMHL_CLOSE.\r\n"));
        AudmhlSetCtrl(AUDMHL_SET_OFF);
        AudmhlSetCtrl(AUDMHL_SET_UNINIT);        
    }
    else if(AUDMHL_START == eCtrl)
    {
        LOG(LOG_CTRLF, TEXT("AUDMHL_START.\r\n"));
        AudmhlSetCtrl(AUDMHL_SET_ON);
    }
    else if(AUDMHL_STOP== eCtrl)
    {
        LOG(LOG_CTRLF, TEXT("AUDMHL_STOP.\r\n"));
        AudmhlSetCtrl(AUDMHL_SET_OFF);
    }
}
   


/*-----------------------------------------------------------------------------
---------------------------Local Function definitions
-------------------------
------------------------------------------------------------------------------*/


#endif
