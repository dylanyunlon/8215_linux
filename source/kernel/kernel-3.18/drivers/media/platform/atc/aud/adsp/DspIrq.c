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

#define _DSP_IRQ_C
/*-----------------------------------------------------------------------------
                    Include header files
-----------------------------------------------------------------------------*/
#include "aud_drv_config.h"
#include "DspD2RCInt.h"
#include "DspConst.h"
#include <linux/types.h>
#include "aud_hal_intf.h"
#include "aud_debug.h"
#include "DspFunc.h"
#include "DspUop.h"
#include "aud_drv.h"

/*-----------------------------------------------------------------------------
                    Functions implementations
-----------------------------------------------------------------------------*/

/***************************************************************************
     Function : vDspAIRQSvc
  Description : DSP Interrupt Handler
    Parameter : None
    Return    : TRUE for bypass interrupt handling in IRQSvc, u8IntMsg packed up for main routine handling
    Note      : ReadINTREG will return 0xaabbccdd, where
                0xdd will be interrupt address, 0xaabbcc will be interrupt data
                In case 0xdd shows it is a flow control interrupt, 0xcc will be zero
***************************************************************************/
void vDspAIRQSvc(void)
{
    u32 u4DspRIntSD, u4DspRIntLD;
    u8 u1DspRIntAddr;

    u1DspRIntAddr = u1AudHalGetDspIntAddr(AUD_HAL_DSP_ID_A);
    u4DspRIntSD = u4AudHalGetDspIntShortData(AUD_HAL_DSP_ID_A);
    u4DspRIntLD = u4AudHalGetDspLongData(AUD_HAL_DSP_ID_A);

    LOG(LOG_FEATURE, TEXT("DSPA IRQ, u1DspRIntAddr=0x%x u4DspRIntSD=0x%x\n"),u1DspRIntAddr, u4DspRIntSD);

    switch (u1DspRIntAddr)
    {
    case INT_D2RC_AOUT_STATUS:
    case INT_D2RC_AOUT2_STATUS:
    case INT_D2RC_DISCONNECT_CMD:
    case INT_D2RC_CONNECT_CMD:
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPA_ID);
        break;

    case INT_D2RC_PTS_1ST_REFRESH:
      //g_fgDspPtsSet = TRUE;
      vMira_SetDspPtsUpdate(TRUE);
      //u4DspRIntSD=dReadDspCommDram(ADDR_RC2D_UPDATE_PTS_STCH);
      //u4DspRIntLD=dReadDspCommDram(ADDR_RC2D_UPDATE_PTS_STCL);
      break;

    case INT_D2RC_DSP_AOUT_NOTIFY:
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD, DSPA_ID);
        if(u4DspRIntSD == FIRST_AOUT_NOTIFY)
        {
            //u4DspRIntSD=dReadDspCommDram(ADDR_RC2D_START_PTS_STCH);
            //u4DspRIntLD=dReadDspCommDram(ADDR_RC2D_START_PTS_STCL);
            LOG(LOG_FEATURE, TEXT("DspIRQ receive DSP s32 Primary FIRST_AOUT_NOTIFY\n"));
        }
        else if(u4DspRIntSD ==LAST_AOUT_NOTIFY)
        {
            LOG(LOG_FEATURE, TEXT("DspIRQ receive DSP s32 Primary LAST_AOUT_NOTIFY\n"));
        }
        else if(u4DspRIntSD == DEC2_FIRST_AOUT_NOTIFY)
        {
            //u4DspRIntSD=dReadDspCommDram(ADDR_RC2D_SECONDARY_START_PTS_STCH);
            //u4DspRIntLD=dReadDspCommDram(ADDR_RC2D_SECONDARY_START_PTS_STCL);
            LOG(LOG_FEATURE, TEXT("DspIRQ receive DSP s32 Secondary FIRST_AOUT_NOTIFY\n"));

        }
        else if(u4DspRIntSD == DEC2_LAST_AOUT_NOTIFY)
        {
            LOG(LOG_FEATURE, TEXT("DspIRQ receive DSP s32 Secondary LAST_AOUT_NOTIFY\n"));
        }
        break;

    case INT_D2RC_REENCODER_STATUS:
    case INT_D2RC_MIXER_REAL_DISCONNECT_OK:
    case INT_D2RC_MIXER_REAL_CONNECT_OK:            // -- Water (AUD_RIPPING)
    case INT_D2RC_MIXER_STEP:
    case INT_D2RC_MIXER_PTS_ACCURATE:
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPA_ID);
        break;

    case INT_D2RC_BIM_TSTA:
        // (aud_8550_emu_migration)
        break;

    //add by fei for gpsmix
    case INT_D2RC_EXTMIX_STATUS:
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPA_ID);        
        break;
        
    case INT_D2RC_DSPA_STATUS:
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPA_ID); 
        break;

    case INT_DVP_MIX_STATUS:
        LOG(LOG_CTRLF, TEXT("[AUD] DspIRQ INT_DVP_MIX_STATUS 0x%x \n"),u4DspRIntSD);
        break;

    case INT_D2RC_POST_REINIT_STATUS:
        LOG(LOG_FEATURE, TEXT("[DSPA_IRQ] POST_REINIT_STATUS, 0x%x \n"),u4DspRIntSD);
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPA_ID); 
        break;
        
    case INT_D2RC_MEDIA_CONNECT_OK:
        LOG(LOG_CTRLF, TEXT("[DSPA_IRQ] INT_D2RC_MEDIA_CONNECT_OK, 0x%x \n"),u4DspRIntSD);
        break;

    case INT_D2RC_MEDIA_DISCONNECT_OK:
        //LOG(LOG_CTRLF, TEXT("[DSPA_IRQ] INT_D2RC_MEDIA_DISCONNECT_OK, 0x%x \n"),u4DspRIntSD);
        u4DspRIntSD = u4DspRIntSD >> 8;
        LOG(LOG_CTRLF, TEXT("[DSPA_IRQ] SRC_MIXER CMD, dec(0x%x), path(0x%x), state(0x%x) \n"),((u4DspRIntSD >> 5) & 0x7),((u4DspRIntSD >> 4) & 0x1),(u4DspRIntSD & 0xF));
        break;
        
    case INT_D2RC_SWMIX_STATUS:
        u4DspRIntSD = u4DspRIntSD >> 8;
        LOG(LOG_CTRLF, TEXT("[DSPA_IRQ] SW_MIXER, path(0x%x), status(0x%x).\n"),
            ((u4DspRIntSD>>4)&0xF), (u4DspRIntSD&0xF));
        break;
    default:
        break;
    }
}

void vDspBIRQSvc(void)
{
    u32 u4DspRIntSD, u4DspRIntLD;
    u8 u1DspRIntAddr;

    u1DspRIntAddr = u1AudHalGetDspIntAddr(AUD_HAL_DSP_ID_B);
    u4DspRIntSD = u4AudHalGetDspIntShortData(AUD_HAL_DSP_ID_B);
    u4DspRIntLD = u4AudHalGetDspLongData(AUD_HAL_DSP_ID_B);
    
    LOG(LOG_FEATURE, TEXT("DSPB IRQ, u1DspRIntAddr=0x%x,0x%x\r\n"),u1DspRIntAddr, u4DspRIntSD);

    switch (u1DspRIntAddr)
    {
    case INT_D2RC_FLOW_CONTROL:
        if(u4DspRIntSD == D2RC_FLOW_CONTROL_AUDIO_UNDERRUN)
        {
            LOG(LOG_FEATURE, TEXT("DspIRQ receive DSP s32 DEC1_UNDERRUN\n"));
            //DmxControlDumpStatus(DmxGetControlInst(0));
            //DmxControlDumpStatus(DmxGetControlInst(1));
        }
        else
        {
            vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPB_ID);
        }
        break;
    case INT_D2RC_FLOW_CONTROL2:
        if(u4DspRIntSD == D2RC_FLOW_CONTROL_AUDIO_UNDERRUN)
        {
            LOG(LOG_FEATURE, TEXT("DspIRQ receive DSP s32 DEC2_UNDERRUN\n"));
            //DmxControlDumpStatus(DmxGetControlInst(0));
            //DmxControlDumpStatus(DmxGetControlInst(1));
        }
        else
        {
            vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPB_ID);
        }
        break;
    case INT_D2RC_FLOW_CONTROL_DEC4:
        if(u4DspRIntSD == D2RC_FLOW_CONTROL_AUDIO_UNDERRUN)
        {
            LOG(LOG_FEATURE, TEXT("DspIRQ receive DSP s32 DEC4_UNDERRUN\n"));
        }
        else
        {
            vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPB_ID);
        }
        break;
    case INT_D2RC_FLOW_CONTROL_DEC5:
        if(u4DspRIntSD == D2RC_FLOW_CONTROL_AUDIO_UNDERRUN)
        {
            LOG(LOG_FEATURE, TEXT("DspIRQ receive DSP s32 DEC5_UNDERRUN\n"));
        }
        else
        {
            vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPB_ID);
        }
        break;
        
    case INT_D2RC_PRIMARY_INBAND_CMD:
    case INT_D2RC_SECONDARY_INBAND_CMD:
    case INT_D2RC_HDCD_TRK_STM_CHG:
    case INT_D2RC_MIXING_METADATA_UPDATE:
    case INT_D2RC_DEEMPHASIS_NOITFY:
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPB_ID);
        break;
    case INT_D2RC_BIM_TST:
        // (aud_8550_emu_migration)
        break;

    case INT_D2RC_VORBIS_CODEBOOK:
    {   
        u8 u1Type;
        DspGetDec1StrType(&u1Type);
        LOG(LOG_CTRLF, TEXT("[Codec]Vorbis recieve s32 :table construction!\n"));
        if(u1Type == VORBIS_STREAM)
        {
            // Construct the Huffman Tree
            //if(fgMakeVorbisCodebook())
            {
               // set dsp for codebook construction ready
               vSendDspCmd(UOP_DSP_VORBIS_TABLE); // short data & interrupt
               //LOG(LOG_FEATURE, TEXT("[Codec]Vorbis table ok!\n"));
            }            
        }
    }
        break;
    case INT_D2RC_DSPB_STATUS:
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD,DSPB_ID);
           break;
    default:
      break;
    }
}


void vDspCIRQSvc(void)
{
    u32 u4DspRIntSD, u4DspRIntLD;
    u8 u1DspRIntAddr;

    u1DspRIntAddr = u1AudHalGetDspIntAddr(AUD_HAL_DSP_ID_C);       // interrupt address
    u4DspRIntSD = u4AudHalGetDspIntShortData(AUD_HAL_DSP_ID_C);  // 24 bit short data
    u4DspRIntLD = u4AudHalGetDspLongData(AUD_HAL_DSP_ID_C);  // 32 bit long data

    switch (u1DspRIntAddr)
    {
    case INT_D2RC_FLOW_CONTROL2:
        if (u4DspRIntSD == D2RC_FLOW_CONTROL_AUDIO_UNDERRUN)
        {
            LOG(5, TEXT("DspIRQ receive DSP s32 DEC2_UNDERRUN\n"));
            //DmxControlDumpStatus(DmxGetControlInst(0));
        }
        else
        {
            vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD, DSPC_ID);
        }
        break;
    case INT_D2RC_REENCODER_STATUS:
    case INT_D2RC_SECONDARY_INBAND_CMD:
    case INT_D2RC_HDCD_TRK_STM_CHG:
    case INT_D2RC_MIXING_METADATA_UPDATE:
    case INT_D2RC_DEEMPHASIS_NOITFY:
        vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD, DSPC_ID);
        break;
    case INT_D2RC_BIM_TST:
        // (aud_8550_emu_migration)
        break;
    case INT_D2RC_FLOW_CONTROL_DEC3:  // 3rd decoder
        if (u4DspRIntSD == D2RC_FLOW_CONTROL_AUDIO_UNDERRUN)
        {
            LOG(5, TEXT("DspIRQ receive DSP s32 DEC3_UNDERRUN\n"));
        }
        else
        {
            vSendDspISR(u1DspRIntAddr, u4DspRIntSD, u4DspRIntLD, DSPC_ID);
        }
        break;
    default:
        break;
    }
}

