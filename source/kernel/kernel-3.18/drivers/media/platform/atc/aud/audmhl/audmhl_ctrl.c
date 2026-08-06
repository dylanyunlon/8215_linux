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
/*-----------------------------------------------------------------------------
                              Include header files
-----------------------------------------------------------------------------*/
#include "aud_oal.h"
//#include "x_typedef.h"
#include <linux/types.h>
#include "drv_config.h"
#include "audmhl_var.h"
#include "audmhl_if.h"
#include "audmhl_hal.h"
#include "audmhl_task.h"
#include "x_bim.h"
#include <media/atc/ose_mem.h>
#include <media/atc/dmx_splitter.h>
#include "aud_mline_hal_if.h"
#include "aud_drv.h"
#include "aud_if.h"


#if  (CONFIG_DRV_HDMI_RX)

/****************************************************************************
 ************************variable definitions  & declaration*****************
 ****************************************************************************/

bool   g_fgAudFmtChg   = FALSE ;
bool   g_fgFirstInt    = TRUE;
bool   g_fgDateGeted    = FALSE;
u8  g_u1HdmiRxSampleRate = 0xFF;
//u8  g_u1PsrTimeSlot  = INT_TIME_SLOT;
u32 g_u4AudmhlChNum  = AINACK_CFG_CH_NUM_8;
u64 g_u8PSRSrcOffset = 0;
u32 g_u4PsrSessionID = 0;

AUDMHL_IN_BUFFER    g_rAudmhlInBuf = {0};
AUDIN_MULTI_INFO_T  g_rAudInInfo;
SPDIFIN_IN_FORMAT_T g_eDataType;

volatile u8 g_u1RawType = 0;
volatile HDMI_RX_AUDIO_INFO_ALL_T g_rHDMIRxAudInfo;
volatile HDMI_RX_AUDIO_FORMAT_T   g_eHDMIRxAudFmt = HDMI_RX_NONE;

//message data for demuxer
SEND_BUFFER g_rDmxBuf;

extern PMLIN_HAL_CLS_PUB g_prHdmiMlin;
extern MLIN_EXTPARAMS_T  g_rMlinParams;


/************************************************************************
 ************************function declaration****************************
 ************************************************************************/
//irq functions
static void AudmhlIrqFunc(void);
extern s32 AudmhlGetAudSendBuf(SEND_BUFFER *prReadBuffer);

/************************************************************************
 ************************function definitions****************************
 ************************************************************************/

/************************************************************************
Function    : bool AudmhlIsMHLIn(void)
Description : This function will check Audio In Type is HDMI In & (PCM||HBR||DSD) or not
Parameter   : None
Return      : TRUE for HDMI PCM/HBR/DSD, otherwise FALSE
************************************************************************/
bool AudmhlIsMHLIn(void)
{
    return (((AudmhlGetAudInType() == AUDINPORT_INTERNAL_HDMI_RX) &&
             (g_eHDMIRxAudFmt != HDMI_RX_NONE))? TRUE:FALSE);
}

/************************************************************************
Function    : void AudmhlIRQHandler(void)
Description : Line in IQR main route
Parameter   : None
Return      : None
************************************************************************/
static void AudmhlIRQHandler(u32 u2Vector)
{
    AudmhlIrqFunc();
}

/************************************************************************
Function    : void AudmhlBufInit()
Description : This function will initialize Audio In buffer's address
Parameter   : None
Return      : None
************************************************************************/
void  AudmhlBufInit(void)
{
    u32 *pu1PTR = NULL;
    u32 u4Size = 0;

    //Set audio in buffer SA and EA
    g_rAudmhlInBuf.u4Buf_SA = AUD_HDMI_RX_BUF_VA;
    g_rAudmhlInBuf.u4Buf_W = g_rAudmhlInBuf.u4Buf_SA;
    g_rAudmhlInBuf.u4Buf_R = g_rAudmhlInBuf.u4Buf_SA;
    g_rAudmhlInBuf.u4Buf_EA = g_rAudmhlInBuf.u4Buf_SA + AUD_HDMI_RX_BUF_SIZE;

    LOG(LOG_CTRLF, _T("Multiple Line SA is 0x%08x, EA is 0x%08x\r\n"), g_rAudmhlInBuf.u4Buf_SA, g_rAudmhlInBuf.u4Buf_EA);

    //Check Buf address is right
    AUD_VERIFY(((g_rAudmhlInBuf.u4Buf_EA -g_rAudmhlInBuf.u4Buf_SA)%(AUDIO_IN_INT_PERIOD*HDMI_INT_TIME_SLOT))==0);
    // Set Multiple Line In buffer  , Start and End Address MUST BE block address.
    AUD_VERIFY((g_rAudmhlInBuf.u4Buf_SA & 0xFF)==0);
    AUD_VERIFY((g_rAudmhlInBuf.u4Buf_EA & 0xFF)==0);
    // SPDIF In Buffer MUST BE  times of parser's slot.
    AUD_VERIFY(((g_rAudmhlInBuf.u4Buf_EA -g_rAudmhlInBuf.u4Buf_SA)%(AUDIO_IN_INT_PERIOD*INT_TIME_SLOT))==0);
    // SPDIF Buffer MUST BE times of stereo PCM (2*3 Bytes) to avoid buffer write through error.
    AUD_VERIFY(((g_rAudmhlInBuf.u4Buf_EA -g_rAudmhlInBuf.u4Buf_SA)%6)==0);
    // Check AFIFO' Size must be equal to times of 6 bytes

    pu1PTR = (u32*)g_rAudmhlInBuf.u4Buf_SA;
    u4Size = AUD_HDMI_RX_BUF_SIZE;
    while (u4Size -= 4)
    {
        *(pu1PTR++) = 0;
    }
}

/************************************************************************
Function    : void AudmhlBufRst()
Description : Reset Audio In buffer's read/write pointer
Parameter   : None
Return      : None
************************************************************************/
void AudmhlBufRst(void)
{
    g_rAudmhlInBuf.u4Buf_R = g_rAudmhlInBuf.u4Buf_SA;
    g_rAudmhlInBuf.u4Buf_W = g_rAudmhlInBuf.u4Buf_SA;
}

/************************************************************************
Function    : bool AudmhlMLinTypeDecided(void)
Description : This function will check if data is decided or not when SPDIF input.
Parameter   : None
Return      : TRUE for Data Decided , otherwise FALSE
************************************************************************/
bool AudmhlMLinTypeDecided(void)
{
    MLIN_SPDIF_INFO_T rSpdInfo;
    g_prHdmiMlin->GetSpdifType(g_prHdmiMlin, &rSpdInfo);
    return ((rSpdInfo.DEC == 1)? TRUE : FALSE);
}

/************************************************************************
Function    : void AudmhlClrTypeDecided(void)
Description :
Parameter   : None
Return      :
************************************************************************/
void AudmhlClrTypeDecided(void)
{
    while (AudmhlMLinTypeDecided())
    {
        mdelay(3);
        // Clean SPDIF Decided bit
        g_prHdmiMlin->ClrSpdTypeDec(g_prHdmiMlin, 0);
    }
}

/************************************************************************
Function    : u8 AudmhlGetMLinDataType(void)
Description : This function will return SPDIF's data Type
Parameter   : None
Return      : 0 : PCM  , 1 : RAW , 2 : DTSCD-16 , 3 : DTSCD-14
************************************************************************/
u8 AudmhlGetMLinDataType(void)
{
    MLIN_SPDIF_INFO_T rSpdInfo;
    g_prHdmiMlin->GetSpdifType(g_prHdmiMlin, &rSpdInfo);
    return (rSpdInfo.ROUGH);

     //return ((AudmhlGetMLinSPDType()&SPDIFIN_ROUGH)>>8);
}

/************************************************************************
Function    : u8 AudmhlGetMLinRAWType(void)
Description : This function will return RAW's audio Type
Parameter   : None
Return      : Detailed type for IEC61937 RAW data
************************************************************************/
u8 AudmhlGetMLinRAWType(void)
{
    MLIN_SPDIF_INFO_T rSpdInfo;
    g_prHdmiMlin->GetSpdifType(g_prHdmiMlin, &rSpdInfo);
    return (rSpdInfo.DETAIL);

    //return (AudmhlGetMLinSPDType()&SPDIFIN_DETAIL);
}

/************************************************************************
Function    : u32 AudmhlGetAudInBufSA(void)
Description : Get audmhl in buffer sa
Parameter   : None
Return      :
************************************************************************/
u32 AudmhlGetAudInBufSA(void)
{
    return g_rAudmhlInBuf.u4Buf_SA;
}

/************************************************************************
Function    : u32 AudmhlGetAudInBufEA(void)
Description : Get audmhl in buffer ea
Parameter   : None
Return      :
************************************************************************/
u32 AudmhlGetAudInBufEA(void)
{
    return g_rAudmhlInBuf.u4Buf_EA;
}

/************************************************************************
Function    : u32 AudmhlGetAudInBufWP(void)
Description : Get audmhl in buffer write pointer
Parameter   : None
Return      :
************************************************************************/
u32 AudmhlGetAudInBufWP(void)
{
    return g_rAudmhlInBuf.u4Buf_W;
}

/************************************************************************
Function    : u32 AudmhlGetAudInBufRP(void)
Description : Get audmhl in buffer read pointer
Parameter   : None
Return      :
************************************************************************/
u32 AudmhlGetAudInBufRP(void)
{
    return g_rAudmhlInBuf.u4Buf_R;
}

/************************************************************************
Function    : u32 AudmhlBufDataSize(void)
Description : Get audmhl in buffer avail data size
Parameter   : None
Return      :
************************************************************************/
u32 AudmhlBufDataSize(void)
{
    return (g_rAudmhlInBuf.u4Buf_R <= g_rAudmhlInBuf.u4Buf_W)
         ? (g_rAudmhlInBuf.u4Buf_W - g_rAudmhlInBuf.u4Buf_R)
         : (g_rAudmhlInBuf.u4Buf_W + AUD_HDMI_RX_BUF_SIZE - g_rAudmhlInBuf.u4Buf_R);
}

/************************************************************************
Function    : u32 AudmhlSlotSize(void)
Description : Get audmhl in buffer slot size
Parameter   : None
Return      :
************************************************************************/
u32 AudmhlSlotSize(void)
{
    return (HDMI_AUDIO_IN_SLOT_SIZE);
    //return (AUDIO_IN_INT_PERIOD *g_u1PsrTimeSlot);
}

/************************************************************************
Function    : u32 AudmhlUpAudInBufWPtrByMLinHW(void)
Description : update writer pointer
Parameter   : None
Return      :
************************************************************************/
void AudmhlUpAudInBufWPtrByMLinHW(void)
{
    do
    {
        g_rAudmhlInBuf.u4Buf_W = g_prHdmiMlin->rHwIf.GetPoint(g_prHdmiMlin) + g_rAudmhlInBuf.u4Buf_SA;
    }
    while (  (g_rAudmhlInBuf.u4Buf_W < g_rAudmhlInBuf.u4Buf_SA)
          || (g_rAudmhlInBuf.u4Buf_W > g_rAudmhlInBuf.u4Buf_EA) );


    if(g_rAudmhlInBuf.u4Buf_W == g_rAudmhlInBuf.u4Buf_EA)
    {
        g_rAudmhlInBuf.u4Buf_W = g_rAudmhlInBuf.u4Buf_SA;
    }

    if(g_rAudmhlInBuf.u4Buf_W > g_rAudmhlInBuf.u4Buf_EA)
    {
        LOG(LOG_MHL, _T("[Overflow] g_rAudmhlInBuf.u4Buf_W = 0x%x. \n"), g_rAudmhlInBuf.u4Buf_W);
        LOG(LOG_MHL, _T("[Overflow] g_rAudmhlInBuf.u4Buf_SA = 0x%x. \n"), g_rAudmhlInBuf.u4Buf_SA);
        LOG(LOG_MHL, _T("[Overflow] g_rAudmhlInBuf.u4Buf_EA = 0x%x. \n"), g_rAudmhlInBuf.u4Buf_EA);
        g_rAudmhlInBuf.u4Buf_W = g_rAudmhlInBuf.u4Buf_SA;
    }

    if (!g_fgAudFmtChg)
    {
        if (AudmhlBufDataSize() > AudmhlSlotSize())
        {
            AudmhlGetAudSendBuf(&g_rDmxBuf);
            AudmhlSendDataBuf(NULL);
        }
    }
}


void AudmhlNotifyStreamChange(void)
{
    AudmhlSendAudMsg(AUDIN_CHG_SPDIFIN_INT_SIGNAL|(0x0<<8), AUDIN_CMD_PRI_HIGH);
}

/************************************************************************
Function    : AudmhlNotifySPDDataType(AUD_DRV_FMT_T uDataType)
Description : This function will notify  SPDIF In Audio Codec Type
Parameter   : AUD_DRV_FMT_T
Return      : None
************************************************************************/
void AudmhlNotifySPDDataType(AUD_DRV_FMT_T uDataType)
{
    AudmhlSendAudMsg(AUDIN_SPDIF_RAW_DATA_TYPE|(uDataType<<8), AUDIN_CMD_PRI_HIGH);
}

/************************************************************************
Function    : AudmhlSetEmphasisFlag(bool fgEmphasis)
Description : This function is called by audio in driver,
              set the emphasis bit for decoder
Parameter   : bool
Return      : None
************************************************************************/
void AudmhlSetEmphasisFlag(bool fgEmphasis)
{
    vAudInSetEmphasisFlag(fgEmphasis);
}

/************************************************************************
Function    : AudmhlNotifySPDAudinType(SPDIFIN_IN_FORMAT_T uAudinType)
Description : This function will notify  SPDIF In Data Type :
              PCM/RAW/DTSCD-16/DTSCD-14
Parameter   : 0 : PCM , 1 : RAW , 2 : DTSCD-16 , 3 : DTSCD-14
Return      : None
************************************************************************/
void AudmhlNotifySPDAudinType(SPDIFIN_IN_FORMAT_T uAudinType)
{
    AudmhlSendAudMsg(AUDIN_SPDIF_AUDIN_TYPE|(uAudinType<<8), AUDIN_CMD_PRI_HIGH);
}

/************************************************************************
Function    : void AudmhlSetAudInType(AUDIO_IN_TYPE_T uAudinFmt)
Description : This function "MUST" be called by AudIn to provide Input mode before turning on multiple line in module
Parameter   :  0: line in , 1 : spdif in , 2 : multiple line in
Return      : None
************************************************************************/
void AudmhlSetAudInType(AUDIO_IN_TYPE_T eAudinFmt)
{
    g_rAudInInfo.eAudinType = eAudinFmt;
}

/************************************************************************
Function    : AUDIO_IN_TYPE_T AudmhlGetAudInType(void)
Description : This function will check Audio In Type is LineIn/SPDIF/Multiple LineIn
Parameter   :
Return      : 0 : Line In , 1 : SPDIFIn , 2 : Multiple Line In ,
************************************************************************/
AUDIO_IN_TYPE_T AudmhlGetAudInType(void)
{
    return (g_rAudInInfo.eAudinType);
}

/************************************************************************
Function    : AudmhlSetAudOn
Description : This function on audio hdmi
Parameter   : void
Return      : None
************************************************************************/
static void AudmhlSetAudOn(void)
{
    AUD_VERIFY(AudmhlGetAudInType()!= AUDIN_NONE);
    g_prHdmiMlin->rHwIf.Stop(g_prHdmiMlin, 0);

    AudmhlClrTypeDecided();
    //Set Multiple Line In IRQ On
    //g_prHdmiMlin->SetIRQOnOff(g_prHdmiMlin, AudmhlIRQHandler, TRUE);

    AudmhlBufInit();

    g_eHDMIRxAudFmt =  HDMI_RX_NONE ;
    if (AudmhlGetAudInType() == AUDINPORT_INTERNAL_HDMI_RX)
    {
        g_eHDMIRxAudFmt =  g_rHDMIRxAudInfo.u1HDMIRxAudFmt ;
    }
    else
    {
        LOG(LOG_FAIL, TEXT("audio type is error = 0x%x"), AudmhlGetAudInType());
    }

    g_rMlinParams.u4BufPhyAdr = HDMI_RX_PHYSICAL(g_rAudmhlInBuf.u4Buf_SA);
    g_rMlinParams.u4BufSz = AUD_HDMI_RX_BUF_SIZE;
    g_rMlinParams.eMlinChNum = g_u4AudmhlChNum;
    g_rMlinParams.PFN_ISR_CB = AudmhlIRQHandler;

    LOG(LOG_MHL, _T("[AUDIN_MULTI] g_rAudmhlInBuf.u4Buf_SA = 0x%x. \n"), g_rAudmhlInBuf.u4Buf_SA);
    LOG(LOG_MHL, _T("[AUDIN_MULTI] g_rMlinParams.u4BufPhyAdr = 0x%x. \n"), g_rMlinParams.u4BufPhyAdr);
    LOG(LOG_MHL, _T("[AUDIN_MULTI] g_rMlinParams.u4BufSz = 0x%x. \n"), g_rMlinParams.u4BufSz);
    LOG(LOG_MHL, _T("[AUDIN_MULTI] g_rMlinParams.eMlinChNum = 0x%x. \n"), g_rMlinParams.eMlinChNum);

    g_fgAudFmtChg = FALSE;
    g_fgFirstInt = TRUE;

    ++g_u4PsrSessionID;
    g_u8PSRSrcOffset = 0;
    LOG(LOG_CTRLF, _T("[AUDIN_MULTI]g_u4PsrSessionID = 0x%x. \n"), g_u4PsrSessionID);

    if (AudmhlGetAudInType() == AUDINPORT_INTERNAL_HDMI_RX) // AUDIN_MULTI_LINEIN
    {
        if ((g_eHDMIRxAudFmt==HDMI_RX_PCM)  ||
            (g_eHDMIRxAudFmt == HDMI_RX_HBR)||
            (g_eHDMIRxAudFmt==HDMI_RX_192k_RAW))
        {
            LOG(LOG_MHL, _T("[AUDIN_MULTI] HDMI Rx PCM/HBR/192kRAW\n"));

            // Set Multi Line In H/W
            if ((g_eHDMIRxAudFmt==HDMI_RX_HBR)|| (g_eHDMIRxAudFmt==HDMI_RX_192k_RAW))
            {
                if (g_u4AudmhlChNum==AINACK_CFG_CH_NUM_2)
                {
                    g_rMlinParams.eIntPeriod = MLIN_INTPERID_64DW;
                    g_rMlinParams.eOutBitNum = LIN_16;
                }
                else
                {
                    g_rMlinParams.eIntPeriod = MLIN_INTPERID_256DW;
                    g_rMlinParams.eOutBitNum = LIN_16;
                }
            }
            else
            {
                if (g_u4AudmhlChNum==AINACK_CFG_CH_NUM_2)
                {
                    g_rMlinParams.eIntPeriod = MLIN_INTPERID_64DW;
                    g_rMlinParams.eOutBitNum = LIN_24;
                }
                else
                {
                    g_rMlinParams.eIntPeriod = MLIN_INTPERID_256DW;
                    g_rMlinParams.eOutBitNum = LIN_24;
                }
            }

            LOG(LOG_MHL, _T("[AUDIN_MULTI] Turn On multiple line In H/W\n"));
        }
        else if (g_eHDMIRxAudFmt==HDMI_RX_SD_RAW)
        {
            LOG(LOG_MHL, _T("[AUDIN_MULTI] HDMI Rx SD RAW\n"));

            g_rMlinParams.eIntPeriod = MLIN_INTPERID_64DW;
            g_rMlinParams.eOutBitNum = LIN_16;


            LOG(LOG_MHL, _T("Turn On Multiple Line In H/W.\r\n"));
        }
        else
        {
            LOG(LOG_FAIL, _T(" Turn On multiple line  In H/W failed, g_eHDMIRxAudFmt is %d.\r\n"), g_eHDMIRxAudFmt);
            return;
        }

        g_prHdmiMlin->rHwIf.Setup(g_prHdmiMlin, &g_rMlinParams);
        g_prHdmiMlin->rHwIf.Start(g_prHdmiMlin, 0);
    }
}

/************************************************************************
Function    : AudmhlSetAudOff
Description : This function on audio hdmi
Parameter   : void
Return      : None
************************************************************************/
static void AudmhlSetAudOff(void)
{
    //Disable Multiple Line In and SPDIFIn H/W
    if (NULL != g_prHdmiMlin)
    {
        g_prHdmiMlin->rHwIf.Stop(g_prHdmiMlin, 0);
		g_prHdmiMlin->SetIRQOnOff(g_prHdmiMlin, AudmhlIRQHandler, FALSE);
    }
    else
        LOG(LOG_MHL, _T("Audio In H/W has been stop\n"));

    g_u8PSRSrcOffset = 0;
    g_eHDMIRxAudFmt = HDMI_RX_NONE ;
    LOG(LOG_MHL, _T("Turn Off Audio In H/W\n"));
}

bool AudmhlSetAudOnOff(bool fgOn)
{
    static bool fgAudmhlInStatus = FALSE;

    if (fgAudmhlInStatus != fgOn)
    {
        fgOn ? AudmhlSetAudOn() : AudmhlSetAudOff();
        fgAudmhlInStatus = fgOn;
    }

    return (fgAudmhlInStatus);
}


//*********************************************************************
// s32 AudmhlGetMhlSendBuf(SEND_BUFFER *prSendBuffer)
// Describe  : Provide a read buffer for demux/parser's request, PBBUF removes
//             the slot from the SENT linked list
// Parameters: u2CompId      [IN] specify the target PBBUF
//             prReadBuffer  [OUT] the READ_BUFFER pointer
// Return    : AUDMHL_OK      A SENT slot is available for reading request
//             AUDMHL_FAIL    No SENT slot can be available.
//*********************************************************************
u32 u4MhlSzjCnt = 0;
s32 AudmhlGetAudInSBuf(SEND_BUFFER *prSendBuffer)
{
    s32 iErrCode = AUDMHL_FAIL;
    u32 u4SlotSize = AudmhlSlotSize();


    if(g_u8PSRSrcOffset == 0)
    {
        u4MhlSzjCnt = 0;
    }

    // Check slot is enough or not
    if (AudmhlBufDataSize() > u4SlotSize)
    {
        if((u4MhlSzjCnt % 99) == 0)
        {
            LOG(LOG_MHL, _T("RdPt:%x, WtPt:%x, ID:0x%x, ofstt:%x \n"),
                g_rAudmhlInBuf.u4Buf_R,g_rAudmhlInBuf.u4Buf_W, g_u4PsrSessionID, (u32)g_u8PSRSrcOffset);
        }
        u4MhlSzjCnt++;

        prSendBuffer->pcBuffer = (u8 *)(g_rAudmhlInBuf.u4Buf_R);
        prSendBuffer->u4BufferSize = u4SlotSize;
                // side to decode from playoffset , the same as DataSize here.
        prSendBuffer->u4DataOffset = 0;
        prSendBuffer->u4DataSize = u4SlotSize;
        prSendBuffer->u4PlayOffset = 0;
        prSendBuffer->u4PlaySize = u4SlotSize;

        //offset for slot in one file
        prSendBuffer->u4SessionID = g_u4PsrSessionID;
        prSendBuffer->u8SrcOffset = g_u8PSRSrcOffset ;
        g_u8PSRSrcOffset += u4SlotSize;
        g_rAudmhlInBuf.u4Buf_R += u4SlotSize;

        iErrCode = AUDMHL_OK;

        // Update Read Pointer to avoid cross reading slot issue :
        // read data between 2 slots will not call	i4AudInDrvIFBufferReceived to release previous slot,
        // it will get the same Read pointer for next slot
        // I assume that read pointer could be release immediately after parsering in audio in
        if (g_rAudmhlInBuf.u4Buf_R == g_rAudmhlInBuf.u4Buf_EA)
        {
            g_rAudmhlInBuf.u4Buf_R = g_rAudmhlInBuf.u4Buf_SA;
        }
    }

    return (iErrCode);
}

//*********************************************************************
//void AudmhlNtfLPEToWaitState(void)
// Describe: This funtion will trigger LPE's state machine to Wait State for inquiry when audio type changes.
// Parameters: None
// Return:
//*********************************************************************
void AudmhlNtfLPEToWaitState(void)
{
    AudmhlSendAudMsg(AUDIN_CHG_SPDIFIN_INT_SIGNAL|(0x0<<8), AUDIN_CMD_PRI_HIGH);
    AudmhlNotifySPDDataType(AUD_DRV_FMT_VORBIS);
}

//*********************************************************************
// void  AudmhlSetAudInfo(void)
// Describe: Audio In driver set HDMI Rx information when HDMI Rx is bit-stream changed
// Parameters: None
// Return: AUMHL_OK
//*********************************************************************
void AudmhlSetAudInfo(AUDIN_INFO_T *pHDMIRxAudioInfo, u8  u1MuliChFlag)
{
    // Stop parsing when this function is called because it means bit-stream changed
    if (u1MuliChFlag==0)
    {
        g_u4AudmhlChNum = AINACK_CFG_CH_NUM_2;
        LOG(LOG_FEATURE, _T("multiple line in H/W should be 2-channels.\n"));
    }
    else
    {
        g_u4AudmhlChNum = AINACK_CFG_CH_NUM_8;
        LOG(LOG_FEATURE, _T("multiple line in H/W should be  8-channels.\n"));
    }

    if (g_rHDMIRxAudInfo.u1HDMIRxAudFmt != pHDMIRxAudioInfo->u1HDMIRxAudFmt)
    {
        g_rHDMIRxAudInfo.u1HDMIRxAudFmt =  pHDMIRxAudioInfo->u1HDMIRxAudFmt;
        g_eHDMIRxAudFmt = pHDMIRxAudioInfo->u1HDMIRxAudFmt;
        // Nontify AudIn module codec type , Use unknow to keep wait state
        //AudmhlNtfLPEToWaitState();
        g_fgAudFmtChg = TRUE ;
        LOG(LOG_FEATURE, _T(" HDMI Rx Input type is changed.\r\n"));
    }
    if (g_rHDMIRxAudInfo.u1HDMIRxAudFmt == HDMI_RX_PCM)
    {
        LOG(LOG_FEATURE, _T("HDMI Rx In is PCM.\r\n"));
        g_fgAudFmtChg = TRUE ;
        if(AINACK_CFG_CH_NUM_2 == g_u4AudmhlChNum)
        {
            pHDMIRxAudioInfo->u4HDMIIRxPCMInfo.SpeakerPlacement = 0;
        }
        //AudmhlNtfLPEToWaitState();

        //Check if
        if (g_rHDMIRxAudInfo.u4HDMIIRxPCMInfo.SpeakerPlacement !=
            pHDMIRxAudioInfo->u4HDMIIRxPCMInfo.SpeakerPlacement)
        {
            LOG(LOG_FEATURE, _T("HDMI Rx In is PCM and SpeakerPlacement is changed from %02x to %02x.\n"),
                g_rHDMIRxAudInfo.u4HDMIIRxPCMInfo.SpeakerPlacement,
                pHDMIRxAudioInfo->u4HDMIIRxPCMInfo.SpeakerPlacement );

            g_rHDMIRxAudInfo.u4HDMIIRxPCMInfo.SpeakerPlacement = pHDMIRxAudioInfo->u4HDMIIRxPCMInfo.SpeakerPlacement;
             g_u1HdmiRxSampleRate = pHDMIRxAudioInfo->u1AudinSampleRate ;
            AudmhlSendAudMsg(AUDIN_CHG_HDMI_RX_PCM_CHLAYOUT|(pHDMIRxAudioInfo->u4HDMIIRxPCMInfo.SpeakerPlacement<<8), AUDIN_CMD_PRI_HIGH);
        }
        if (g_u1HdmiRxSampleRate !=pHDMIRxAudioInfo->u1AudinSampleRate)
        {
         LOG(LOG_FEATURE, _T("HDMI Rx In is PCM and SampleRate is changed from %02x to %02x.\n"),
                        g_u1HdmiRxSampleRate,pHDMIRxAudioInfo->u1AudinSampleRate);

            g_u1HdmiRxSampleRate = pHDMIRxAudioInfo->u1AudinSampleRate ;
            g_rHDMIRxAudInfo.u4HDMIIRxPCMInfo.SpeakerPlacement = pHDMIRxAudioInfo->u4HDMIIRxPCMInfo.SpeakerPlacement;
        }
    }
    else
    {
         pHDMIRxAudioInfo->u4HDMIIRxPCMInfo.SpeakerPlacement = CA_UNKNOWN;
         LOG(LOG_FEATURE, _T("[AUDIN_MULTI] HDMI Rx In is 0x%x.\n"),
            g_rHDMIRxAudInfo.u1HDMIRxAudFmt);
    }
    g_rHDMIRxAudInfo.u4HDMIIRxPCMInfo = 	pHDMIRxAudioInfo->u4HDMIIRxPCMInfo;
    g_rHDMIRxAudInfo.u8HDMIRxAudCHSTS = pHDMIRxAudioInfo->u8HDMIRxAudCHSTS;
}


//*********************************************************************
// static AUD_DRV_FMT_T AudmhlDataToAudFormat(SPDIFIN_IN_FORMAT_T eDataType, u8 uRawType)
// Describe  :switch hdmi data to audio format
// Parameters: None
// Return    : None
//*********************************************************************
static AUD_DRV_FMT_T AudmhlDataToAudFormat(SPDIFIN_IN_FORMAT_T eDataType, u8 uRawType)
{
    if (g_eHDMIRxAudFmt == HDMI_RX_HBR && uRawType == HBR_RAW_HBR_MAT)
    {
        //do not care about eDataType,
        //because multiline-in detects SPDIFIN_PCM and SPDIFIN_RAW alternately when MAT input
        return AUD_DRV_FMT_MLP;
    }

    switch (eDataType)
    {
    case SPDIFIN_PCM:
        return (AudmhlIsMHLIn() ? AUD_DRV_FMT_HDMI_IN_PCM : AUD_DRV_FMT_CDDA);

    case SPDIFIN_RAW:
        switch (uRawType)
        {
        case SPDIF_RAW_AC3:
            return AUD_DRV_FMT_AC3;

        case SPDIF_RAW_MP1L1:
        case SPDIF_RAW_MP1L23_MP2_WOEXT:
        case SPDIF_RAW_MP2_WEXT:
        case SPDIF_RAW_MP2L1_LSF:
        case SPDIF_RAW_MP2L23_LSF:
            return AUD_DRV_FMT_MPEG;

        case SPDIF_RAW_DTS_I:
        case SPDIF_RAW_DTS_II:
        case SPDIF_RAW_DTS_III:
            return AUD_DRV_FMT_DTS;

        case SPDIF_RAW_MP2_AAC:
        case SPDIF_RAW_MP2_AAC1:
            return AUD_DRV_FMT_AAC_PURE;

        case SPDIF_RAW_EAC3:
            return AUD_DRV_FMT_EAC3;

        case SPDIF_RAW_DTS_IV:
            if (  g_eHDMIRxAudFmt == HDMI_RX_SD_RAW
               || g_eHDMIRxAudFmt == HDMI_RX_PCM )
            {
                //return AUD_DRV_FMT_DTSHD_LBR;
                return 0;
            }
            else if (g_eHDMIRxAudFmt == HDMI_RX_192k_RAW)
            {
                //note:
                //There is also DTS in this case. Please see DTS-HD_Design_Guidelines_version1.0's chapter 8
                //If it is DTS, AUD_DRV_FMT_DTSHD_PRI_NO_XLL will also load DTS(0x9094) decoder, so there is no decoing problem.
                //But the UI will show "DTS-HD HR" wrongly.
                //solution: DTSDec V9094B5 + audio-driver adds function AUD_Get_DTSHDHR_Stream_Source_Format()
                return AUD_DRV_FMT_DTSHD_PRI_NO_XLL;
            }
            else if (g_eHDMIRxAudFmt == HDMI_RX_HBR)
            {
                return AUD_DRV_FMT_DTSHD_PRI_XLL;
            }
            else
            {
                return AUD_DRV_FMT_VORBIS;
            }

        case SPDIF_RAW_MP4_AAC:
        case SPDIF_RAW_NULL:
        default :
            // Use unknow to keep state
            return AUD_DRV_FMT_VORBIS;
        }

    case SPDIFIN_DTSCD_16:
    case SPDIFIN_DTSCD_14:
        return AUD_DRV_FMT_DTSCD;

    default:
        AUD_ASSERT(0);
        return AUD_DRV_FMT_VORBIS;
    }
}

/************************************************************************
Function  : void AudmhlIrqFunc(void)
Describe  : SPDIF In Interrupt
Parameters: None
Return    : None
*************************************************************************/
static void AudmhlIrqFunc(void)
{
    AUD_DRV_FMT_T eAudFmt = AUD_DRV_FMT_UNKNOWN;

    if ((g_eHDMIRxAudFmt == HDMI_RX_PCM) && (g_u4AudmhlChNum != AINACK_CFG_CH_NUM_2))
    {
        if (g_fgFirstInt)
        {
            g_eDataType = (SPDIFIN_IN_FORMAT_T)AudmhlGetMLinDataType();
            if(g_eDataType != SPDIFIN_PCM)
            {
                LOG(LOG_CTRLF, _T("[AUDIN_IRQ]HDMI Detect PCM, but MLIN Detect RAW(%d). Force to PCM. \n"), g_eDataType);
                g_eDataType = SPDIFIN_PCM;
            }
            AudmhlNotifySPDAudinType(g_eDataType);
            eAudFmt = AUD_DRV_FMT_HDMI_IN_PCM ;
            AudmhlNotifySPDDataType(eAudFmt);
            g_fgFirstInt =FALSE ;
            g_fgDateGeted = TRUE;
            LOG(LOG_MHL, _T("[AUDIN_IRQ] HDMI Rx In, Decoder is LPCM Multi-ch.\n"));
        }
        AudmhlUpAudInBufWPtrByMLinHW();
    }
    else if((g_eHDMIRxAudFmt == HDMI_RX_SD_RAW)||
            ((g_eHDMIRxAudFmt == HDMI_RX_PCM)&&(g_u4AudmhlChNum == AINACK_CFG_CH_NUM_2)))
    {
        //Check if first time type decided
        if (g_fgFirstInt)
        {
            // The data in AudInBuf may be not correct now!
            // New SPDIFInCtrl should be set according to the different stream type.
            if (!AudmhlMLinTypeDecided())
            {
                LOG(LOG_MHL, _T("[AUDIN_IRQ]First s32(HDMI_RX_SD_RAW||PCM_2ch), type not decided.\r\n"));
                return;
            }

            g_eDataType = (SPDIFIN_IN_FORMAT_T)AudmhlGetMLinDataType();
            g_u1RawType = AudmhlGetMLinRAWType();
            g_fgFirstInt = FALSE;
            if (g_eDataType == SPDIFIN_RAW)
            {
                if(g_eHDMIRxAudFmt == HDMI_RX_PCM)
                    LOG(LOG_CTRLF, _T("[AUDIN_IRQ]ERR: HDMI Detect PCM, but MLIN Detect RAW \n"));
                g_rHDMIRxAudInfo.u1HDMIRxAudFmt = HDMI_RX_SD_RAW;
                g_eHDMIRxAudFmt = HDMI_RX_SD_RAW ;
                LOG(LOG_MHL, _T("[AUDIN_IRQ]First s32(HDMI_RX_SD_RAW||PCM_2ch): SD RAW.\r\n"));
            }
            else
            {
                if(g_eHDMIRxAudFmt == HDMI_RX_SD_RAW)
                    LOG(LOG_CTRLF, _T("[AUDIN_IRQ]ERR: HDMI Detect RAW, but MLIN Detect PCM \n"));
                g_rHDMIRxAudInfo.u1HDMIRxAudFmt = HDMI_RX_PCM;
                g_eHDMIRxAudFmt = HDMI_RX_PCM ;
                LOG(LOG_MHL, _T("[AUDIN_IRQ]First s32(HDMI_RX_SD_RAW||PCM_2ch): 2ch PCM.\r\n"));
            }

            //Stop Multiple Line In H/W
            //vSetMultiLineInCtrl(0x0);
            g_prHdmiMlin->rHwIf.Stop(g_prHdmiMlin, 0);
            // Reset Audio In buffer R/W Address
            AudmhlBufRst();
            AudmhlNotifySPDAudinType(g_eDataType);
            eAudFmt = AudmhlDataToAudFormat(g_eDataType, g_u1RawType);

            LOG(LOG_CTRLF, _T("[AUDIN_IRQ]g_eDataType =0x%x, g_u1RawType = %u, eAudioFormat = %u.\n"),
                g_eDataType, g_u1RawType, eAudFmt);

            AudmhlNotifySPDDataType(eAudFmt);
            g_fgDateGeted = TRUE;
            g_rMlinParams.eOutBitNum = LIN_16;
            g_rMlinParams.eIntPeriod = MLIN_INTPERID_64DW;

            if (g_eDataType == SPDIFIN_PCM)
            {
                g_rMlinParams.eOutBitNum = LIN_24;
            }
            else if (g_eDataType == SPDIFIN_DTSCD_14|| g_eDataType == SPDIFIN_DTSCD_16)
            {
            }
            g_prHdmiMlin->rHwIf.CfgUpd(g_prHdmiMlin, &g_rMlinParams);


            g_prHdmiMlin->rHwIf.Start(g_prHdmiMlin, 0);
            //HAL_GetTime(&g_rAudInDbgTime0);
            return;
        }

        // Now, the data in AudInBuf is correct!
        if (AudmhlMLinTypeDecided() && g_eDataType != AudmhlGetMLinDataType()) // data type change
        {
            //g_fgAudFmtChg = TRUE;
            g_eDataType = (SPDIFIN_IN_FORMAT_T)AudmhlGetMLinDataType();
            g_u1RawType = AudmhlGetMLinRAWType();
            if (g_eDataType == SPDIFIN_RAW)
            {
                g_rHDMIRxAudInfo.u1HDMIRxAudFmt = HDMI_RX_SD_RAW;
                g_eHDMIRxAudFmt = HDMI_RX_SD_RAW;
                LOG(LOG_MHL, _T("[AUDIN_IRQ](HDMI_RX_SD_RAW||PCM_2ch): SD RAW\n"));
            }
            else
            {
                g_rHDMIRxAudInfo.u1HDMIRxAudFmt = HDMI_RX_PCM;
                g_eHDMIRxAudFmt = HDMI_RX_PCM;
                LOG(LOG_MHL, _T("[AUDIN_IRQ]((HDMI_RX_SD_RAW||PCM_2ch): 2ch PCM\n"));
            }
            AudmhlNotifySPDAudinType((SPDIFIN_IN_FORMAT_T)g_eDataType);
            eAudFmt = AudmhlDataToAudFormat(g_eDataType, g_u1RawType);
            LOG(LOG_CTRLF, _T("[AUDIN_IRQ]PCM/Raw, g_eDataType = %u, g_u1RawType = %u, eAudioFormat = %u.\n"),
            g_eDataType, g_u1RawType, eAudFmt);
            AudmhlNotifySPDDataType(eAudFmt);
            g_fgDateGeted = TRUE;
            AudmhlNotifyStreamChange();
        }
        else if (AudmhlMLinTypeDecided() &&
                 g_eDataType == SPDIFIN_RAW &&
                 g_u1RawType != AudmhlGetMLinRAWType()) // raw type change
        {
            //g_fgAudFmtChg = TRUE ;
            g_u1RawType = AudmhlGetMLinRAWType();
            eAudFmt = AudmhlDataToAudFormat(g_eDataType, g_u1RawType);
            LOG(LOG_CTRLF, _T("[AUDIN_IRQ]SPDIFIN_RAW codec change: g_eDataType = %u, g_u1RawType = %u, eAudioFormat = %u.\n"),
                g_eDataType, g_u1RawType, eAudFmt);
            AudmhlNotifySPDDataType(eAudFmt);
            g_fgDateGeted = TRUE;
            AudmhlNotifyStreamChange();
        }
        else
        {
            //if (!g_fgAudFmtChg)
            {
                // Get Write pointer if use multiple Line-In module
                AudmhlUpAudInBufWPtrByMLinHW();
            }
        }
    }
}

//*********************************************************************
// void AudmhlSetMLinHWClk(u32 u4SrcBitNum, u32 eCycle)
// Describe  : Multiple Line H/W Clock mode setting
// Parameters: Non
// Return    : Non
//*********************************************************************
void AudmhlSetMLinHWClk(u32 u4SrcBitNum, u32 u4Cycle)
{

    g_rMlinParams.u4SrcBitNum = (u4SrcBitNum>>8) + 1;
    switch(u4Cycle)
    {
    case AIN_MUTLI_LRCK_CYC_16:
        g_rMlinParams.eCycle = AUD_LRCK_CYC_16;
        break;

    case AIN_MUTLI_LRCK_CYC_24:
        g_rMlinParams.eCycle = AUD_LRCK_CYC_24;
        break;

    case AIN_MUTLI_LRCK_CYC_32:
        g_rMlinParams.eCycle = AUD_LRCK_CYC_32;
        break;

        default:
            LOG(LOG_CTRLF, TEXT("AudmhlSetMLinHWClk u4Cycle type error = 0x%x"), u4Cycle);
            break;
    }

    g_prHdmiMlin->rHwIf.CfgUpd(g_prHdmiMlin, &g_rMlinParams);

}

#endif


