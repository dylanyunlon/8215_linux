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
//#include "x_typedef.h"
#include <linux/types.h>
#include "audmhl_if.h"
#include "audmhl_task.h"
#include "audmhl_var.h"
#include "aud_debug.h"
#include "aud_config.h"

#include "drv_thread.h"

#include <media/atc/dmx_splitter.h>


#if CONFIG_DRV_HDMI_RX    

/****************************************************************************
 **                           Function prototypes
 ****************************************************************************/
//3363 function

// by IEC937 and IEC 958 Spec, 
//0x00:44.1khz, 0x02:48khz, 0x03:32khz, 0x08:88.2khz
//0x0A:96khz, 0x0c:176khz, 0x0E:192khz, 0x09:768khz(HBR)
typedef enum
{
    MHL_IEC_44k  = 0,
    MHL_IEC_48k  = 0x02,
    MHL_IEC_32k  = 0x03,
    MHL_IEC_88k  = 0x08,
    MHL_IEC_96k  = 0x0A,    
    MHL_IEC_176k = 0x0c,   
    MHL_IEC_192k = 0x0E,
    MHL_IEC_768k = 0x09
}MHL_IEC_FS;


static s32 AudmhlTaskInit(void);
static s32 AudmhlClearAudMsg(u32 *pr_u4Msg);
static s32 AudmhlGetAudMsg(u32 *pr_u4Msg);

s32 AudmhlTaskMain(void* pvArg);
void AudmhlCmdDispatch(u32 u4Msg, u16  u2MsgArg);
void AudmhlTaskDelete(void);
void AudmhlTransAudInfo(void);

u8 AudmhlIECFsToHdmi(u8 u1Msg);


/****************************************************************************
 **              variable definitions
 ****************************************************************************/
//message queue variable
uintptr_t g_hAudInCmdQueue = 0;

AUDIN_INFO_T* g_prAudInNfyInfo = NULL;

HDMI_RX_IN_AUDIO_INFO_T g_rHdmiAudInfo = {0};
HDMI_RX_Audio_InfoFrame g_rHdmiAudInfFrame;
HDMI_RX_AUDIO_CHSTS     g_rHdmiAudChSts;

HDMI_RX_AUDIO_FORMAT_T g_eHdmiAudInFmt  = HDMI_RX_NONE;
HDMI_RX_SPEAKER_ALLOCATE_T  g_eHdmiPCMSpkPlacement = CA_FL_FR ;

u8 g_u1HdmiACPType = ACP_TYPE_GENERAL_AUDIO ;       
u8 g_u1FSChange = 0xFF;
u8 g_u1HdmiRxAudChStatus = 0xFF;
u8 g_u1Emphasis = 0;
u8 g_u1HDMIRxAudTmtType  = 0 ;

u8 g_u1HDMIRxSampRate = SPDIFIN_48K ;


/****************************************************************************
 **       variable declaration
 ****************************************************************************/
extern bool  g_fgSpdifHwErr;
extern bool  g_fgAudmhlInited;
extern u8 g_u1AudioInType;
extern void*  hAudmhlEventToMW;
extern AUDIO_IN_TYPE_T g_AudioInType;
extern bool  g_fgAudFmtChg;

extern HDMI_REG_AUD_F g_hdmiaud_func;
extern void *g_pvDmxTag;
extern pu4SendSlot g_pu4SendSlot;
struct task_struct * g_hAudmhlThread = NULL;



/****************************************************************************
 **                           Function definitions
 ****************************************************************************/

/************************************************************************
Function    : void AudmhlSendHWErr();
Description : send HW error to audio mhl
Parameter   : None
Return      : None
 ********************************************************************/
static void AudmhlSendHWErr(void)
{
    if (FALSE == g_fgSpdifHwErr)
    {
        AudmhlSendAudMsg(AUDIN_SPDIF_HW_ERROR, AUDIN_CMD_PRI_HIGH);
        g_fgSpdifHwErr = TRUE;
    }
}

/************************************************************************
Function    : void AudmhlSendAudMsg();
Description : send message among mhl audio driver thread  
Parameter   : u4Cmd: cmd id, bPri: cmd priority
Return      : None
********************************************************************/
void AudmhlSendAudMsg(u32 u4Cmd, u8 bPri)
{
	s32  i4MsgRet = 0;
	u32 u4Msg = 0;

    if(g_hAudInCmdQueue == 0)
    {
        LOG(LOG_DATAF, TEXT("g_hAudInCmdQueue is deleted.\r\n"));
        return;
    }
    i4MsgRet = x_msg_q_send(g_hAudInCmdQueue, &u4Cmd, sizeof(u32), bPri);

	if(i4MsgRet != OSR_OK)
	{
		if (i4MsgRet == OSR_TOO_MANY)
		{
			do
			{
				i4MsgRet = AudmhlClearAudMsg(&u4Msg);
			}
			while (i4MsgRet == OSR_OK);
			LOG(LOG_DATAF, TEXT("[audmhl_task]AudmhlSendAudMsg queue full=%x.\r\n"), i4MsgRet);
			AudmhlSendHWErr();
		}
		else
		{
			LOG(LOG_DATAF, TEXT("[audmhl_task]AudmhlSendAudMsg Fail!!!!=%x\n"), i4MsgRet);
            }
    }
}
EXPORT_SYMBOL(AudmhlSendAudMsg);


/************************************************************************
Function    : void AudmhlMsgToMW(u32 u4Msg, u8  u1MsgArg);
Description : send message to mw
Parameter   : None
Return      : None
 ********************************************************************/
void AudmhlMsgToMW(u32 u4Msg, u8  u1MsgArg)
{
    u32 dwMsg = u4Msg|(u1MsgArg<<8);
    LOG(LOG_CTRLF, TEXT("audio mhl set event to mw data = 0x%x\n"), dwMsg);
    
    if(u4Msg != AUDIN_CHG_SPDIFIN_INT_SIGNAL)
    {
        LOG(LOG_CTRLF, TEXT("drop this event \n"));
        return;
    }
	
	pr_info("hdmiaudio: isr_data.isr start\r\n");
	if (audio_register_isr) {
		isr_data.isr(&dwMsg);
		pr_info("hdmiaudio: isr_data.isr call success\n");
			}
	else {
		pr_info("hdmiaudio: isr_data.isr call fail\r\n");
	}

    x_event_set_data(hAudmhlEventToMW, dwMsg);
    x_event_set(hAudmhlEventToMW);
}
extern SEND_BUFFER g_rDmxBuf;

/************************************************************************
Function    : void AudmhlSendDataBuf(void * hCmdQ)
Description : If Audio mhl In slot buffer is ready , send data buffer to parser
Parameter   : hCmdQ: message handle
Return      : None
 ********************************************************************/
void AudmhlSendDataBuf(void* hCmdQ)
{
    #ifdef __linux__
    if (g_pu4SendSlot == NULL)
    {
        LOG(LOG_FAIL, TEXT("send slot func is NULL\r\n"));
    }
    else
    {
        g_pu4SendSlot(g_pvDmxTag, &g_rDmxBuf);
    }
    #else
    AudmhlWriteMsgQ(hCmdQ, &g_rDmxBuf, sizeof(SEND_BUFFER));
    #endif
}


void vAudMhlParaStatus(void)
{
    LOG(0, TEXT("HDMI-RX audio para status: \r\n"));
    LOG(0, TEXT("---------------------------------------------------; \r\n"));
    LOG(0, TEXT("g_rHdmiAudInfo.u1HBRAudio = %d; \r\n"), g_rHdmiAudInfo.u1HBRAudio);
    LOG(0, TEXT("g_rHdmiAudInfo.u1DSDAudio = %d; \r\n"), g_rHdmiAudInfo.u1DSDAudio);
    LOG(0, TEXT("g_rHdmiAudInfo.u1RawSDAudio = %d; \r\n"), g_rHdmiAudInfo.u1RawSDAudio);
    LOG(0, TEXT("g_rHdmiAudInfo.u1PCMMultiCh = %d; \r\n"), g_rHdmiAudInfo.u1PCMMultiCh);
    LOG(0, TEXT("g_rHdmiAudInfo.u1FsDec = %d; \r\n"), g_rHdmiAudInfo.u1FsDec);
    LOG(0, TEXT("g_rHdmiAudInfo.u1I2sChanValid = %d; \r\n"), g_rHdmiAudInfo.u1I2sChanValid);
    LOG(0, TEXT("g_rHdmiAudInfo.u1I2sCh0Sel = %d; \r\n"), g_rHdmiAudInfo.u1I2sCh0Sel);
    LOG(0, TEXT("g_rHdmiAudInfo.u1I2sCh1Sel = %d; \r\n"), g_rHdmiAudInfo.u1I2sCh1Sel);
    LOG(0, TEXT("g_rHdmiAudInfo.u1I2sCh2Sel = %d; \r\n"), g_rHdmiAudInfo.u1I2sCh2Sel);
    LOG(0, TEXT("g_rHdmiAudInfo.u1I2sCh3Sel = %d; \r\n"), g_rHdmiAudInfo.u1I2sCh3Sel);
    LOG(0, TEXT("g_rHdmiAudInfo.u1MCLKRatio = %d; \r\n"), g_rHdmiAudInfo.u1MCLKRatio);
    LOG(0, TEXT("---------------------------------------------------; \r\n"));
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.Type = %d; \r\n"), g_rHdmiAudInfFrame.info.Type);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.Ver = %d; \r\n"), g_rHdmiAudInfFrame.info.Ver);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.Len = %d; \r\n"), g_rHdmiAudInfFrame.info.Len);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.AudioChannelCount = %d; \r\n"), g_rHdmiAudInfFrame.info.AudioChannelCount);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.RSVD1 = %d; \r\n"), g_rHdmiAudInfFrame.info.RSVD1);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.AudioCodingType = %d; \r\n"), g_rHdmiAudInfFrame.info.AudioCodingType);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.SampleSize = %d; \r\n"), g_rHdmiAudInfFrame.info.SampleSize);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.SampleFreq = %d; \r\n"), g_rHdmiAudInfFrame.info.SampleFreq);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.Rsvd2 = %d; \r\n"), g_rHdmiAudInfFrame.info.Rsvd2);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.FmtCoding = %d; \r\n"), g_rHdmiAudInfFrame.info.FmtCoding);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.SpeakerPlacement = %d; \r\n"), g_rHdmiAudInfFrame.info.SpeakerPlacement);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.Rsvd3 = %d; \r\n"), g_rHdmiAudInfFrame.info.Rsvd3);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.LevelShiftValue = %d; \r\n"), g_rHdmiAudInfFrame.info.LevelShiftValue);
    LOG(0, TEXT("g_rHdmiAudInfFrame.info.DM_INH = %d; \r\n"), g_rHdmiAudInfFrame.info.DM_INH);
    LOG(0, TEXT("---------------------------------------------------; \r\n"));
    LOG(0, TEXT("g_rHdmiAudChSts.rev = %d; \r\n"), g_rHdmiAudChSts.rev);
    LOG(0, TEXT("g_rHdmiAudChSts.ISLPCM = %d; \r\n"), g_rHdmiAudChSts.ISLPCM);
    LOG(0, TEXT("g_rHdmiAudChSts.CopyRight = %d; \r\n"), g_rHdmiAudChSts.CopyRight);
    LOG(0, TEXT("g_rHdmiAudChSts.AdditionFormatInfo = %d; \r\n"), g_rHdmiAudChSts.AdditionFormatInfo);
    LOG(0, TEXT("g_rHdmiAudChSts.ChannelStatusMode = %d; \r\n"), g_rHdmiAudChSts.ChannelStatusMode);
    LOG(0, TEXT("g_rHdmiAudChSts.CategoryCode = %d; \r\n"), g_rHdmiAudChSts.CategoryCode);
    LOG(0, TEXT("g_rHdmiAudChSts.SourceNumber = %d; \r\n"), g_rHdmiAudChSts.SourceNumber);
    LOG(0, TEXT("g_rHdmiAudChSts.ChannelNumber = %d; \r\n"), g_rHdmiAudChSts.ChannelNumber);
    LOG(0, TEXT("g_rHdmiAudChSts.SamplingFreq = %d; \r\n"), g_rHdmiAudChSts.SamplingFreq);
    LOG(0, TEXT("g_rHdmiAudChSts.ClockAccuary = %d; \r\n"), g_rHdmiAudChSts.ClockAccuary);
    LOG(0, TEXT("g_rHdmiAudChSts.rev2 = %d; \r\n"), g_rHdmiAudChSts.rev2);
    LOG(0, TEXT("g_rHdmiAudChSts.WorldLen = %d; \r\n"), g_rHdmiAudChSts.WorldLen);
    LOG(0, TEXT("g_rHdmiAudChSts.OriginalSamplingFreq = %d; \r\n"), g_rHdmiAudChSts.OriginalSamplingFreq);
      
}


/************************************************************************
Function    : void AudmhlCmdDispatch(u32 u4Msg, u8  u1MsgArg);
Description : handle received messages
Parameter   : u4Msg: command id, u2MsgArg: command
Return      : None
 ********************************************************************/
void AudmhlCmdDispatch(u32 u4Msg, u16  u2MsgArg)
{
    u8 u1MsgArg = 0;
    //SAMPLE_FREQUENCY_T u1MCLKFS = MCLK_256FS;
    
    u1MsgArg = u2MsgArg &0xff; // parameters    

    switch(u4Msg)  // commands
    {
    case AUDIN_CHG_PAUSE_STATUS:
        g_prAudInNfyInfo->u1AudinPauseStatus = u1MsgArg;
        if(!g_prAudInNfyInfo->u1AudinLockStatus)
        {            
            AudmhlMsgToMW(AUDIN_CHG_PAUSE_STATUS, g_prAudInNfyInfo->u1AudinLockStatus);            
        }
        else
        {
            LOG(LOG_DATAF, TEXT("Now In Unlock stage,don't Send AUDIN_CHG_PAUSE_STATUS!![%s].\r\n"),
                __FUNCTION__);
        }
        
        break;

    case AUDIN_CHG_SPDIFIN_INT_SIGNAL:
        if((g_prAudInNfyInfo != NULL) && ((g_AudioInType == AUDINPORT_INTERNAL_HDMI_RX) 
            ||(g_prAudInNfyInfo->u1AudinLockStatus != u1MsgArg)))
        {
                      
            g_prAudInNfyInfo->u1AudinLockStatus = u1MsgArg;
            AudmhlMsgToMW(AUDIN_CHG_SPDIFIN_INT_SIGNAL, g_prAudInNfyInfo->u1AudinLockStatus);

            LOG(LOG_DATAF, TEXT("[audmhl_task]receive AUDIN_CHG_SPDIFIN_INT_SIGNAL: %u.\r\n"),
                u1MsgArg);
        }

        break;

    case AUDIN_CHG_SPDIFIN_INT_INDET:
        {
            g_prAudInNfyInfo->u1AudinChStatus = u1MsgArg;            
            AudmhlMsgToMW(AUDIN_CHG_SPDIFIN_INT_INDET, g_prAudInNfyInfo->u1AudinChStatus);
        }
        break;

    case AUDIN_CHG_SPDIFIN_INT_FSCHG:
        if((g_prAudInNfyInfo != NULL) && 
           ((g_prAudInNfyInfo->u1AudinSampleRate != u1MsgArg)||
            (g_AudioInType == AUDINPORT_INTERNAL_HDMI_RX)))
        {
            if(g_AudioInType != AUDINPORT_INTERNAL_HDMI_RX)
            {
                //vSetAOSDATAOutput(SDATA0_MUTE|SDATA1_MUTE|SDATA2_MUTE|SDATA3_MUTE|SDATA4_MUTE|SDATA5_MUTE);
            }
            if ((g_u1AudioInType == IPODIN) || (g_u1AudioInType == PARTYIN))
            {
                //u1MCLKFS = MCLK_256FS;
            }
            else
            {
                LOG(LOG_DATAF, TEXT("[audmhl_task]AUDIN_CHG_SPDIFIN_INT_FSCHG.\r\n"));
                
                if(g_AudioInType == AUDINPORT_INTERNAL_HDMI_RX)
                {
                    //u1MCLKFS =  g_rHdmiAudInfo.u1MCLKRatio;
                    //LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI Rx output MCLK = 0x%x.\r\n"), u1MCLKFS);                   
                }
                else
                {
                            // HDMI Rx SD RAW and others
                    if (u1MsgArg <= SPDIFIN_48K)   // 512 fs
                    {
                        //u1MCLKFS = MCLK_512FS;
                        LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI send 512 MCLK!\r\n"));
                    }
                    else if (u1MsgArg <= SPDIFIN_96K)    // 256  fs
                    {
                        //u1MCLKFS = MCLK_256FS;
                        LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI send 256 MCLK!\r\n"));
                    }
                    else //  if (u1MsgArg <= SPDIFIN_192K)    // 128  fs
                    {
                        //u1MCLKFS = MCLK_128FS;                        
                        LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI send 128 MCLK!\r\n"));
                    }
                }
            }
            //g_prAudInNfyInfo->u1AudinMCLKSSRC = u1MCLKFS;
            g_prAudInNfyInfo->u1AudinSampleRate = u1MsgArg;
            
            if(!g_prAudInNfyInfo->u1AudinLockStatus)
            {                
                AudmhlMsgToMW(AUDIN_CHG_SPDIFIN_INT_FSCHG, g_prAudInNfyInfo->u1AudinSampleRate);
            }
        }
        break;

    case AUDIN_CHG_HDMIRX_INT:
        if(u1MsgArg == HDMI_RX_AUDIO_ON)
        {
            LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI_RX_AUDIO_ON!\r\n"));
            break;
        }
        else if(u1MsgArg == HDMI_RX_AUDIO_UNLOCK)
        {
            AudmhlSendAudMsg(AUDIN_CHG_SPDIFIN_INT_SIGNAL|(0x1<<8), AUDIN_CMD_PRI_HIGH);
            LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI_RX_AUDIO_UNLOCK!\r\n"));
            g_u1HdmiRxAudChStatus = 0xFF;
            g_fgAudFmtChg = TRUE;
            AudmhlSetAudOnOff(FALSE);
            break;
        }
        else if(u1MsgArg==HDMI_RX_ACPPKT_CHG)
        {
            g_u1HdmiACPType = g_hdmiaud_func.getRxacptype();
            LOG(LOG_DATAF, TEXT("HDMIRX ACPTYPE is 0x%x.\r\n"), g_u1HdmiACPType);
            if(g_prAudInNfyInfo->u1SpdifRawDataType !=AUD_DRV_FMT_UNKNOWN)
            {
                AudmhlNotifySPDDataType(g_prAudInNfyInfo->u1SpdifRawDataType);
            }
            break;
        }
        else if(u1MsgArg == HDMI_RX_PLUG_OUT)
        {
            g_prAudInNfyInfo->u1AudinChStatus &= (~(0x1<<AUDINPORT_INTERNAL_HDMI_RX));
            AudmhlMsgToMW(AUDIN_CHG_SPDIFIN_INT_INDET, g_prAudInNfyInfo->u1AudinChStatus);
            LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI_RX_PLUG_OUT.\r\n"));
            break;
        }
        else if(u1MsgArg == HDMI_RX_AUDIO_BIT_STREAM_CHANGE)
        {
            LOG(LOG_DATAF, TEXT("HDMI_RX_AUDIO_BIT_STREAM_CHANGE.\r\n"));
            
            g_hdmiaud_func.getRxaudinfo(&g_rHdmiAudInfo);
            g_hdmiaud_func.getRxaudinfoframe(&g_rHdmiAudInfFrame);
            g_hdmiaud_func.getRxchannel(&g_rHdmiAudChSts);
            g_rHdmiAudChSts.WorldLen = 0xb;  // fixed to 24bit
            vAudMhlParaStatus();

            g_prAudInNfyInfo->u1AudinSampleRate=AudmhlIECFsToHdmi(g_rHdmiAudInfo.u1FsDec);
            AudmhlTransAudInfo();
            if(g_rHdmiAudInfo.u1HBRAudio)
            { // Hardware return 0 if HBR, so we should change it before setting multiple line in module
                g_rHdmiAudInfo.u1PCMMultiCh=1;
            }
            if(0 == g_rHdmiAudInfo.u1PCMMultiCh)
            {
                g_prAudInNfyInfo->u4HDMIIRxPCMInfo.SpeakerPlacement = 0;
            }
            AudmhlSetAudInfo(g_prAudInNfyInfo,g_rHdmiAudInfo.u1PCMMultiCh);

            
            // ignore HDMI RX Notify case when non-HBR with RawSD case.
            //if ((g_rHdmiAudInfo.u1HBRAudio == 0) && (g_rHdmiAudInfo.u1RawSDAudio==1))
            //YC_TEST , Only SD RAW pass to SPDIF Rx
            if ((g_rHdmiAudInfo.u1HBRAudio == 0) && (g_rHdmiAudInfo.u1RawSDAudio==1) &&
                (g_prAudInNfyInfo->u1AudinSampleRate<=SPDIFIN_48K))
            {
                LOG(LOG_DATAF, TEXT("HDMI Rx : SPDIF RAW.\r\n"));
                g_eHdmiAudInFmt = HDMI_RX_SD_RAW;
                
                AudmhlSetMLinHWClk(AIN_MUTLI_16BIT, AIN_MUTLI_LRCK_CYC_32);
                AudmhlSendAudMsg(AUDIN_CHG_SPDIFIN_INT_SIGNAL|(0x0<<8), AUDIN_CMD_PRI_HIGH);

                // set I2S or RawSD mode and output data from HDMIRX.
                //vSetHdmiRxI2sOn(0x00);
                //vSetHdmiRxSpdifOn(TRUE);
                
                g_eHdmiAudInFmt = HDMI_RX_SD_RAW;
                break;
            }
            else
            {
                if(g_rHdmiAudInfo.u1HBRAudio==1)
                {
                    AudmhlSetMLinHWClk(AIN_MUTLI_16BIT, AIN_MUTLI_LRCK_CYC_16);
                    //vWriteAUDMsk(AIN_ACK_CFG_Multi, (AIN_MUTLI_16BIT|AIN_MUTLI_FMT_LJ|AIN_MUTLI_LRCK_CYC_16|AIN_MUTLI_LRCK_INV), AIN_MUTLI_BNUM_MASK);//
                }
                else
                {
                    AudmhlSetMLinHWClk(AIN_MUTLI_24BIT, AIN_MUTLI_LRCK_CYC_32);
                    //vWriteAUDMsk(AIN_ACK_CFG_Multi, (AIN_MUTLI_24BIT|AIN_MUTLI_FMT_LJ|AIN_MUTLI_LRCK_CYC_32|AIN_MUTLI_LRCK_INV), AIN_MUTLI_BNUM_MASK);
                }
                // 2010/08/23 ,ychung mark delay
                // mdelay(100);

                //vSetHdmiRxSpdifOn(FALSE);
                //vSetHdmiRxI2sOn(0x0F);
                // Locked callback
                // long delay to avoid l/r exchange due to h/w's issue.
                //mdelay(100);

                AudmhlSendAudMsg(AUDIN_CHG_SPDIFIN_INT_SIGNAL|(0x0<<8), AUDIN_CMD_PRI_HIGH);
                LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI Rx : I2S path.\r\n"));
            }

            g_rHdmiAudInfo.u1FsDec = AudmhlIECFsToHdmi(g_rHdmiAudInfo.u1FsDec);
            if ((g_u1FSChange != g_rHdmiAudInfo.u1FsDec)||
                ( g_eHdmiAudInFmt != g_prAudInNfyInfo->u1HDMIRxAudFmt))
            {
                // 2010/01/07,ychung : Because there are different settings for aout/iec config between SD/PCM/HD.
                g_u1FSChange = g_rHdmiAudInfo.u1FsDec;
                AudmhlSendAudMsg(AUDIN_CHG_SPDIFIN_INT_FSCHG|(g_u1FSChange<<8), AUDIN_CMD_PRI_HIGH);
            }

            g_prAudInNfyInfo->u1AudinSampleRate = g_u1FSChange;
            g_eHdmiAudInFmt = g_prAudInNfyInfo->u1HDMIRxAudFmt;
            //Plug In
            g_prAudInNfyInfo->u1AudinChStatus |= (0x1<<AUDINPORT_INTERNAL_HDMI_RX);

            AudmhlMsgToMW(AUDIN_CHG_SPDIFIN_INT_INDET, g_prAudInNfyInfo->u1AudinChStatus);

            if(g_u1HdmiRxAudChStatus != g_rHdmiAudChSts.ISLPCM)
            {
                g_u1HdmiRxAudChStatus = g_rHdmiAudChSts.ISLPCM;
                
                LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI RX Audio channel status changed.\r\n"));
                AudmhlSendAudMsg(AUDIN_CHG_PAUSE_STATUS |(g_u1HdmiRxAudChStatus<<8), AUDIN_CMD_PRI_HIGH); //PCM or non-PCM
            }

        }
		else if(HDMI_RX_AUDIO_CHG_PAUSE_STATUS == u1MsgArg)
	    {
	        g_hdmiaud_func.getRxchannel(&g_rHdmiAudChSts);
			if(g_u1HdmiRxAudChStatus != g_rHdmiAudChSts.ISLPCM)
            {
                g_u1HdmiRxAudChStatus = g_rHdmiAudChSts.ISLPCM;
                LOG(LOG_DATAF, TEXT("[audmhl_task]HDMI RX Audio channel status changed.\r\n"));
                AudmhlSendAudMsg(AUDIN_CHG_PAUSE_STATUS |(g_u1HdmiRxAudChStatus<<8), AUDIN_CMD_PRI_HIGH); //PCM or non-PCM
            }
	    }

        break;

    case AUDIN_SPDIF_AUDIN_TYPE:
        g_prAudInNfyInfo->u1SpdifAudinType = u1MsgArg;
        if(g_prAudInNfyInfo->u1AudinLockStatus)
        {           
            LOG(LOG_DATAF, TEXT("[audmhl_task]AUDIN_SPDIF_AUDIN_TYPE : Unlocked.\r\n"));
            break;
        }
        AudmhlMsgToMW(AUDIN_SPDIF_AUDIN_TYPE, g_prAudInNfyInfo->u1SpdifAudinType);
        break;

    case AUDIN_SPDIF_RAW_DATA_TYPE:
        g_prAudInNfyInfo->u1SpdifRawDataType = u1MsgArg;

        // Check if unlocked, don't callback AIH when unlocked
        g_u1HDMIRxAudTmtType = u1MsgArg;
        if((g_AudioInType == AUDINPORT_INTERNAL_HDMI_RX)&&((g_u1HdmiACPType == ACP_TYPE_DVD_AUDIO)||
          (g_u1HdmiACPType == ACP_TYPE_SACD)))
        {
            // Send invalid codec if ACP Type is DVD-Audio or SACD to stop playback.
            u1MsgArg = AUD_DRV_FMT_VORBIS;
        }

        if(g_prAudInNfyInfo->u1AudinLockStatus)
        {          
            LOG(LOG_DATAF, TEXT("[audmhl_task]AUDIN_SPDIF_RAW_DATA_TYPE : Unlocked.\r\n"));            
            break;
        }
        AudmhlMsgToMW(AUDIN_SPDIF_RAW_DATA_TYPE, g_prAudInNfyInfo->u1SpdifRawDataType);
        break;

    case AUDIN_CHG_SPDIFIN_OEMPF:
        if(g_u1Emphasis != u1MsgArg )
        {
            AudmhlSetEmphasisFlag((bool)u1MsgArg);// notify dsp the status of emphasis
            g_u1Emphasis = u1MsgArg ;
        }
        break;

    case AUDIN_CHG_INT_CHNOCHG:
        g_prAudInNfyInfo->u1AudinUSBNo = u1MsgArg;
        AudmhlMsgToMW(AUDIN_CHG_INT_CHNOCHG, g_prAudInNfyInfo->u1AudinUSBNo);
        
        break;

    case AUDIN_CHG_HDMI_RX_PCM_CHLAYOUT:
        g_prAudInNfyInfo->u4HDMIIRxPCMInfo.SpeakerPlacement = u1MsgArg;        
        AudmhlMsgToMW(AUDIN_CHG_HDMI_RX_PCM_CHLAYOUT, g_prAudInNfyInfo->u4HDMIIRxPCMInfo.SpeakerPlacement);
     break;
     
    case AUDIN_SPDIF_HW_ERROR:
        g_prAudInNfyInfo->u1AudinPauseStatus = u1MsgArg;
        AudmhlMsgToMW(AUDIN_SPDIF_HW_ERROR, g_prAudInNfyInfo->u1AudinPauseStatus);
        break;
        
    case AUDIN_TASK_DELETE:
        g_fgAudmhlInited = FALSE;

    default:
        break;
    }
}

/************************************************************************
Function    : void AudmhlDRVInit();
Description : This function will init AUDIn driver
Parameter   : None
Return      : None
 ********************************************************************/
s32 AudmhlDRVInit(AUDMHL_IN_TYPE eAudinType)
{
    s32 i4Ret = AUDMHL_FAIL;
    
    if(AUDMHL_FAIL == (i4Ret = AudmhlTaskInit()))
    {
       LOG(LOG_FAIL, TEXT("AudmhlDRVInit Failed.\r\n"));
    }
    return (i4Ret);
}

/************************************************************************
Function    : void AudmhlDRVUnInit();
Description : This function will uninit AUDIn driver
Parameter   : eAudinType: audio in type
Return      : None
 ********************************************************************/
s32 AudmhlDRVUnInit(AUDMHL_IN_TYPE eAudinType)
{   
    x_memset(&g_rDmxBuf, 0, sizeof(g_rDmxBuf));
    AudmhlTaskDelete();
    return (AUDMHL_OK);
}

/************************************************************************
Function    : AudmhlGetAudMsg();
Description : This function get audio mhl cmd
Parameter   : pr_u4Msg: Message pointer
Return      : None
*********************************************************************/
static s32 AudmhlGetAudMsg(u32 *pr_u4Msg)
{
    u16 u2MsgIdx = 0;
    s32  i4MsgRet = 0;
    u32 zMsgSize = sizeof(u32);

    i4MsgRet = x_msg_q_receive(&u2MsgIdx, pr_u4Msg, &zMsgSize, &g_hAudInCmdQueue, 1, X_MSGQ_OPTION_WAIT);
    VERIFY((i4MsgRet == OSR_OK) || (i4MsgRet == OSR_NO_MSG));
    return (i4MsgRet);
}

/************************************************************************
Function    : void AudmhlClearAudMsg();
Description : This function uses to clear audmhl cmd.
Parameter   : None
Return      : None
************************************************************************/
static s32 AudmhlClearAudMsg(u32 *pr_u4Msg)
{
    u16 u2MsgIdx = 0;
    s32  i4MsgRet = 0;
    u32 zMsgSize = sizeof(u32);

    i4MsgRet = x_msg_q_receive(&u2MsgIdx, pr_u4Msg, &zMsgSize, &g_hAudInCmdQueue, 1, X_MSGQ_OPTION_NOWAIT);
    VERIFY((i4MsgRet == OSR_OK) || (i4MsgRet == OSR_NO_MSG));
    return (i4MsgRet);
}

/******************************************************************************
 * Function      : AudmhlTaskMain
 * Description   : main routine for audio mhl Task
 * Parameter     : pvArg: thread param, default null
 * Return        : None
 ******************************************************************************/
s32 AudmhlTaskMain(void* pvArg)
{
    u32  u4Msg    = 0;
    s32   i4MsgRet = 0;
    u16  u2MsgArg = 0;
    
    LOG(LOG_DATAF, TEXT("AudmhlTaskMain in.\r\n"));
    
    VERIFY(NULL == pvArg);

    while(g_fgAudmhlInited == TRUE)
    {
        i4MsgRet = AudmhlGetAudMsg(&u4Msg);
        
        if(OSR_OK == i4MsgRet)
        {
            u2MsgArg =  (u4Msg>>8)&0xffff;
            u4Msg = u4Msg &0xff;
            AudmhlCmdDispatch(u4Msg, u2MsgArg);
        }
    }

    /* release all OS resource */
    VERIFY(x_msg_q_delete(g_hAudInCmdQueue) == OSR_OK);
    g_hAudInCmdQueue = 0;

    complete_and_exit(NULL, 0);

	return 0;
}

/************************************************************************
Function    : s32 AudmhlTaskInit(void)
Description : init audmhl task
Parameter   : None
Return      : None
 ********************************************************************/
static s32 AudmhlTaskInit(void)
{
    //create timer for updating data to demux.
    VERIFY(x_msg_q_create(&g_hAudInCmdQueue, AUDINTASK_SPDIFIN_CMD_Q_NAME, sizeof(u32), 30) == OSR_OK);
    
    g_fgAudmhlInited = TRUE;
    g_u1HDMIRxSampRate = SPDIFIN_48K ;
    g_eHdmiAudInFmt  =HDMI_RX_NONE ;
    g_eHdmiPCMSpkPlacement = CA_FL_FR ;
    g_u1FSChange = 0xFF;
    
    // Create audio mhl in task

    g_hAudmhlThread = kthread_create(AudmhlTaskMain, (void *)NULL, "AUDINTask Thread");
	if (IS_ERR(g_hAudmhlThread)) {
		LOG(LOG_CTRLF, TEXT("[AudmhlTaskInit]AudmhlTaskMain create fail \r\n"));
		g_hAudmhlThread = NULL;
		return AUDMHL_FAIL;
	}
	wake_up_process(g_hAudmhlThread);

    
    return (AUDMHL_OK);
}


/************************************************************************
Function    : s32 AudmhlTaskDelete(void)
Description : This function will delete AUDIn task
Parameter   : None
Return      : None
 ********************************************************************/
void AudmhlTaskDelete(void)
{
    AudmhlSendAudMsg(AUDIN_TASK_DELETE |(FALSE<<8), AUDIN_CMD_PRI_HIGH);
}

/************************************************************************
Function    : AudmhlSetMLinDataBit(bool fgPCM)
Description : This function will notify Data bit PCM or non-PCM
Parameter   : 0 or 1
Return      : None
 ************************************************************************/
void AudmhlSetMLinDataBit(bool fgPCM)
{
    AudmhlSendAudMsg(AUDIN_CHG_PAUSE_STATUS|(fgPCM<<8), AUDIN_CMD_PRI_HIGH);
}


/************************************************************************
Function    : AudmhlFsToAud(u8 u1Msg)
Description : change spdif in fs to audio fs
Parameter   : u1Msg: spdif in fs
Return      : audio fs
 ************************************************************************/
u8 AudmhlFsToAud(u8 u1Msg)
{
    u8 u1AudFs = 0;

    switch (u1Msg)
    {
    case SPDIFIN_32K:
        u1AudFs = FS_32K;
        break;
    case SPDIFIN_44K:
        u1AudFs = FS_44K;
        break;
    case SPDIFIN_48K:
        u1AudFs = FS_48K;
        break;
    case SPDIFIN_88K:
        u1AudFs = FS_88K;
        break;
    case SPDIFIN_96K:
        u1AudFs = FS_96K;
        break;
    case SPDIFIN_176K:
        u1AudFs = FS_176K;
        break;
    case SPDIFIN_192K:
        u1AudFs = FS_192K;
        break;
    default:
        u1AudFs = FS_UNKNOWN;
        break;
    }
    return (u1AudFs);
}


/************************************************************************
Function    : void AudmhlIECFsToHdmi(u8 u1Msg);
Description : audio SF switch to hdmi sf
Parameter   : u1Msg
Return      : None
********************************************************************/
u8 AudmhlIECFsToHdmi(u8 u1Msg)
{
    u8 u1DesHdmiFs = 0;
    switch (u1Msg)
    {
    case MHL_IEC_32k:
        u1DesHdmiFs = SPDIFIN_32K;
        break;
    case MHL_IEC_44k:
        u1DesHdmiFs = SPDIFIN_44K;
        break;
    case MHL_IEC_48k:
        u1DesHdmiFs = SPDIFIN_48K;
        break;
    case MHL_IEC_88k:
        u1DesHdmiFs = SPDIFIN_88K;
        break;
    case MHL_IEC_96k:
        u1DesHdmiFs = SPDIFIN_96K;
        break;
    case MHL_IEC_176k:
        u1DesHdmiFs = SPDIFIN_176K;
        break;
    case MHL_IEC_192k:
        u1DesHdmiFs = SPDIFIN_192K;
        break;
    case MHL_IEC_768k:
        //u1DesHdmiFs = FS_768K;  Not define 768kHz yet
        u1DesHdmiFs = FS_768K;
        break;
    default:
        u1DesHdmiFs = SPDIFIN_OUT_RANGE;
        break;
    }
    return (u1DesHdmiFs);
}

/************************************************************************
Function    : u8 AudmhlCheckBit(u8 u1Bit);
Description : check non zero bit
Parameter   : None
Return      : None
********************************************************************/
u8 AudmhlCheckBit(u8 u1Bit)
{
    u8 u1_count = 0;
    for (u1_count=0;u1_count<8;u1_count++)
    {
        if (u1Bit & (1<<u1_count))
        {
            u1Bit = u1_count;
            break;
        }
    }
    return (u1Bit);
}

/************************************************************************
Function    : void AudmhlAudFormatType(void)
Description : 
Parameter   : None
Return      : None
********************************************************************/
void AudmhlAudFormatType(void)
{
    if(g_rHdmiAudInfo.u1HBRAudio==1)
    {
        g_prAudInNfyInfo->u1HDMIRxAudFmt = HDMI_RX_HBR ;
    }
    else if((g_rHdmiAudInfo.u1HBRAudio==0) &&(g_rHdmiAudInfo.u1DSDAudio==1) && 
        (g_rHdmiAudInfo.u1RawSDAudio==0))
    {
        g_prAudInNfyInfo->u1HDMIRxAudFmt = HDMI_RX_DSD ;
    }
    else if((g_rHdmiAudInfo.u1HBRAudio==0) &&(g_rHdmiAudInfo.u1DSDAudio==0) && 
        (g_rHdmiAudInfo.u1RawSDAudio==1))
    {
        if(g_prAudInNfyInfo->u1AudinSampleRate> SPDIFIN_48K)
        {
            g_prAudInNfyInfo->u1HDMIRxAudFmt = HDMI_RX_192k_RAW ;
        }            
        else
        {
            g_prAudInNfyInfo->u1HDMIRxAudFmt = HDMI_RX_SD_RAW ;
        }
    }
    else if((g_rHdmiAudInfo.u1HBRAudio==0) &&(g_rHdmiAudInfo.u1DSDAudio==0) &&
        (g_rHdmiAudInfo.u1RawSDAudio==0))
    {
        g_prAudInNfyInfo->u1HDMIRxAudFmt = HDMI_RX_PCM ;
    }
    else
    {
        g_prAudInNfyInfo->u1HDMIRxAudFmt = HDMI_RX_NONE ;
    }
}


/************************************************************************
Function    : void AudmhlHDMIRXPCMInfo(void)
Description : 
Parameter   : None
Return      : None
********************************************************************/
void AudmhlHDMIRXPCMInfo(void)
{
    g_prAudInNfyInfo->u4HDMIIRxPCMInfo.AudioChannelCount = g_rHdmiAudInfFrame.info.AudioChannelCount;
    g_prAudInNfyInfo->u4HDMIIRxPCMInfo.DM_INH = g_rHdmiAudInfFrame.info.DM_INH;
    g_prAudInNfyInfo->u4HDMIIRxPCMInfo.AudioCodingType = g_rHdmiAudInfFrame.info.AudioCodingType;
    g_prAudInNfyInfo->u4HDMIIRxPCMInfo.LevelShiftValue = g_rHdmiAudInfFrame.info.LevelShiftValue;
    g_prAudInNfyInfo->u4HDMIIRxPCMInfo.SampleSize = g_rHdmiAudInfFrame.info.SampleSize;
    g_prAudInNfyInfo->u4HDMIIRxPCMInfo.SpeakerPlacement = g_rHdmiAudInfFrame.info.SpeakerPlacement;
}

/************************************************************************
Function    : void AudmhlHDMIRXCHSTS(void)
Description : 
Parameter   : None
Return      : None
********************************************************************/
void AudmhlHDMIRXCHSTS(void)
{
    g_prAudInNfyInfo->u8HDMIRxAudCHSTS = g_rHdmiAudChSts;
    /*_HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.rev = g_rHdmiAudChSts.rev;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.ISLPCM = g_rHdmiAudChSts.ISLPCM;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.CopyRight = g_rHdmiAudChSts.CopyRight;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.AdditionFormatInfo = g_rHdmiAudChSts.AdditionFormatInfo;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.ChannelStatusMode = g_rHdmiAudChSts.ChannelStatusMode;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.CategoryCode = g_rHdmiAudChSts.CategoryCode;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.SourceNumber = g_rHdmiAudChSts.SourceNumber;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.ChannelNumber = g_rHdmiAudChSts.ChannelNumber;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.SamplingFreq = g_rHdmiAudChSts.SamplingFreq;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.ClockAccuary = g_rHdmiAudChSts.ClockAccuary;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.rev2 = g_rHdmiAudChSts.rev2;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.WorldLen = g_rHdmiAudChSts.WorldLen;
      _HdmiRxAudioAllInf.u8HDMIRxAudCHSTS.OriginalSamplingFreq = g_rHdmiAudChSts.OriginalSamplingFreq;
      */
}

/************************************************************************
Function    : void AudmhlTransAudInfo(void)
Description : translate hdmi audio information to audio mhl variable
Parameter   : None
Return      : None
********************************************************************/
void AudmhlTransAudInfo(void)
{
    AudmhlAudFormatType();
    AudmhlHDMIRXPCMInfo();
    AudmhlHDMIRXCHSTS();
}


#endif

