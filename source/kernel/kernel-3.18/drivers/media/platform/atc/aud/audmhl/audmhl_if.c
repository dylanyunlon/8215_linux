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
#include "audmhl_var.h"
#include "audmhl_if.h"
#include "audmhl_task.h"
#include "aud_debug.h"

#include <media/atc/dmx_splitter.h>
#include <media/atc/ose_mem.h>
#include "aud_mline_hal_if.h"
#include "DspFunc.h"
#include "aud_if.h"

/****************************************************************************
 ** Variable/struct definitions  & declaration
 ****************************************************************************/
#if CONFIG_DRV_HDMI_RX

#define IOCTL_MHL_AUD_GET_INFO \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x30, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_AUD_SET_INFO \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x31, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef enum
{
    GET_HDMIRX_AUDIO_INFO = 0,
    GET_HDMIRX_AUDIO_INFOFRAME,
    GET_HDMIRX_CHANNEL_STATUS,
    GET_HDMIRX_ACPTYPE
}HDMIRX_AUDIO_GETINFO;

void *g_pvDmxTag = NULL;
pu4SendSlot g_pu4SendSlot = NULL;
DRV_AUDIN_CB_LIST_T g_AudmhlCb = {0};
__u32 (*g_i4UnSubscribeDrvCb)(void *pvDmxTag);
HDMI_REG_AUD_F g_hdmiaud_func;
extern AUDMHL_IN_BUFFER    g_rAudmhlInBuf;
s32 g_i4EventInit = 0;

void *  hAudmhlEventToMW = NULL;

//Mlin control class
PMLIN_HAL_CLS_PUB g_prHdmiMlin = NULL;
MLIN_EXTPARAMS_T  g_rMlinParams = {0};


bool  g_fgSpdifHwErr   = FALSE;
bool  g_fgAudmhlInited = FALSE;

u8 g_u1AudioInType  = 0;// mw set audio type,
AUDIO_IN_TYPE_T g_AudioInType = AUDIN_NONE;  //index audio in type HDMI or spdif

extern AUDIN_INFO_T *g_prAudInNfyInfo;
extern HDMI_RX_AUDIO_INFO_ALL_T g_rHDMIRxAudInfo;
extern u32 g_u4AudmhlChNum;
extern HDMI_RX_AUDIO_FORMAT_T g_eHDMIRxAudFmt;
extern struct task_struct * g_hAudmhlThread;
extern bool   g_fgDateGeted;
extern s32 AudmhlGetAudInSBuf(SEND_BUFFER *prReadBuffer);


/************************************************************************************************
 Function Name: AumdhlHDMIRxIn
 Description: Is HDMI In or not
 Input para: void
 Output para:TRUE, HDMI in, FALSE, Not HDMI in
************************************************************************************************/
bool AumdhlHDMIRxIn(void)
{
    return (g_AudioInType == AUDINPORT_INTERNAL_HDMI_RX);
}


/************************************************************************************************
 Function Name: AudmhlCreateResource
 Description: Create resource for demux & mw
 Input para: void
 Output para:
************************************************************************************************/
s32 AudmhlCreateResource(void)
{
    s32 i4Ret = AUDMHL_OK;

    if(0 == g_i4EventInit)
    {
        //create event for mw
        hAudmhlEventToMW = x_event_create(NULL, FALSE, FALSE, AUDMHL_MW_EVENT);

        if (NULL == hAudmhlEventToMW)
        {
            LOG(LOG_FAIL,TEXT("Audio mhl create mw event failed.\r\n"));
			return (AUDMHL_FAIL);
        }
        g_i4EventInit++;
    }

    if(NULL == g_prHdmiMlin)
    {
        g_prHdmiMlin = MlinHal_New();


        //default mlin in params
        memset(&g_rMlinParams, 0, sizeof(MLIN_EXTPARAMS_T));

        g_rMlinParams.u4SrcBitNum = 24;//source bit num
        g_rMlinParams.eOutBitNum = LIN_24;
        g_rMlinParams.eDataFmt = AUDFMT_IIS;
        g_rMlinParams.eCycle = AUD_LRCK_CYC_32;

        if (NULL == g_prHdmiMlin)
        {
            LOG(LOG_FAIL, TEXT("HDMI Mlin New Obj Fail.\r\n"));
            return(AUDMHL_FAIL);
        }
    }
    //create a timer for send message to demux


    return (i4Ret);
}

/************************************************************************************************
 Function Name: AudmhlResourceInit
 Description: init resource for demux and mw
 Input para: void
 Output para:
************************************************************************************************/
s32 AudmhlResourceInit(void)
{
    return (AudmhlCreateResource());
}

struct atc_hdmiaudio_isr_data isr_data = {NULL, NULL};
bool audio_register_isr = FALSE;

s32 atc_hdmiaudio_register_isr(atc_hdmiaudio_isr_t isr, void *arg)
{
	if (isr == NULL)
		{
		LOG(LOG_DECINFO, "atc_hdmiaudio_register_isr return -EINVAL\n");
		return -EINVAL;
		}

	if ((isr_data.isr != NULL) && (isr_data.isr != isr)
	    && (isr_data.arg != arg))
		{
		LOG(LOG_DECINFO, "atc_hdmiaudio_register_isr return -EBUSY\n");
		return -EBUSY;
		}

	isr_data.isr = isr;
	isr_data.arg = arg;
	audio_register_isr = TRUE;
	LOG(LOG_DECINFO, "atc_hdmi_register_isr return TRUE\n");

	return 0;
}
EXPORT_SYMBOL(atc_hdmiaudio_register_isr);
s32 atc_hdmiaudio_unregister_isr(atc_hdmiaudio_isr_t isr, void *arg)
{
	if (isr == NULL)
		return -EINVAL;

	if ((isr_data.isr == isr) && (isr_data.arg == arg)) {
		isr_data.isr = isr;
		isr_data.arg = arg;
		audio_register_isr = FALSE;

		return 0;
	}

	return -EBUSY;
}
EXPORT_SYMBOL(atc_hdmiaudio_unregister_isr);


/*************************************************************************************
 Function Name: AudmhlInit
 Description: Init audio in for hdmi rx
 Input para: prAudinSetInfo audio in setting information
 Output para:
*************************************************************************************/
s32 AudmhlInit(const AUDMHL_SET_T *prAudinSetInfo)
{
    s32  i4Ret = AUDMHL_OK;
    u32 u4Ret = 0;

    g_fgSpdifHwErr = FALSE;
    g_u1AudioInType = prAudinSetInfo->eAudmhlInType;

    //enable asrc auto tracing mode
    if(0 != (u4Ret=DspStartAsrcAutoTracMode(PRI_DEC)))
    {
         LOG(LOG_FAIL,TEXT("AudmhlInit asrc trac mode ret %d.\r\n"), u4Ret);
    }

    if (g_fgAudmhlInited == FALSE)
    {
        //create resource for demuxer
        if (AUDMHL_FAIL == AudmhlResourceInit())
        {
            LOG(LOG_FAIL,TEXT("audmhl resource init failed.\r\n"));
            return (AUDMHL_FAIL);
        }

        //register information for MW
        g_prAudInNfyInfo = (AUDIN_INFO_T *)kzalloc(sizeof(AUDIN_INFO_T), GFP_KERNEL);

        if( NULL == g_prAudInNfyInfo )
        {
            LOG(LOG_FAIL,TEXT("g_prAudInNfyInfo memory alloc failed.\r\n"));
            return (AUDMHL_FAIL);
        }

        AUD_VERIFY((i4Ret = AudmhlDRVInit(prAudinSetInfo->eAudmhlInType)) > 0);
        g_fgAudmhlInited = TRUE;
    }
    return (i4Ret);
}

/*************************************************************************************
 Function Name: AudmhlUnInit
 Description: UnInit audio in for hdmi rx
 Input para: [u1AudinType] AUDIN
 Output para: Void
*************************************************************************************/
void AudmhlUnInit(AUDMHL_IN_TYPE u1AudinType)
{
    s32 i4Ret = AUDMHL_OK;

    bool fgMultiLineOn = FALSE;

    g_fgSpdifHwErr = FALSE;
    if (g_fgAudmhlInited == TRUE)
    {
        fgMultiLineOn = AudmhlSetAudOnOff(FALSE);
        kfree(g_prAudInNfyInfo);
        g_prAudInNfyInfo = NULL;

        AUD_VERIFY((i4Ret = AudmhlDRVUnInit(u1AudinType)) >= 0);

        if (g_hAudmhlThread)
        {
            g_hAudmhlThread = NULL;
        }

        if(AUDINPORT_INTERNAL_HDMI_RX == g_AudioInType)
        {
            g_hdmiaud_func.enableRxaud(FALSE);
        }

        g_AudioInType = AUDIN_NONE;

        if (NULL != g_prHdmiMlin)
        {
            LOG(LOG_CTRLF, _T("AudMlinTest : g_prHdmiMlin OBJ DELETE \r\n"));
            g_prHdmiMlin->Delete(g_prHdmiMlin);
            g_prHdmiMlin = NULL;
        }

        g_fgAudmhlInited = FALSE;
    }

    g_fgDateGeted = FALSE;
    //disable asrc auto tracing mode
    DspStopAsrcAutoTracMode(PRI_DEC);
}


/****************************************************************************
Function    : void AudmhlInCtrl(bool fgOnOffCmd)
Description : This function set audio mhl input switch
Parameter   : u8 u1OnOffCmd : 1 for turn on ; 0 for turn off
Return      : None
****************************************************************************/
void AudmhlInCtrl(bool fgOnOffCmd)
{
    bool fgmhlIn = FALSE;
    if(TRUE == fgOnOffCmd)
    {
        fgmhlIn = AudmhlSetAudOnOff(FALSE);
        fgmhlIn = AudmhlSetAudOnOff(TRUE);
    }
}

//*********************************************************************
// s32 AudmhlGetAudSendBuf(READ_BUFFER *prReadBuffer)
// Describe: Provide a read buffer for demux/parser's request, PBBUF removes
//           the slot from the SENT linked list
// Parameters: u2CompId      [IN] specify the target PBBUF
//             prReadBuffer  [OUT] the READ_BUFFER pointer
// Return: AUDMHL_OK      A SENT slot is available for reading request
//         AUDMHL_FAIL    No SENT slot can be available.
//*********************************************************************
s32 AudmhlGetAudSendBuf(SEND_BUFFER *prSendBuffer)
{
    s32 i4ErrCode = AUDMHL_OK;

    if(AUDMHL_IN == g_u1AudioInType)
    {
        i4ErrCode = AudmhlGetAudInSBuf(prSendBuffer);
    }
    return (i4ErrCode);
}

//*********************************************************************
// bool AudmhlInIsRAW(void)
// Describe: Check data type is RAW or PCM
// Parameters: None
// Return: TRUE RAW , FALSE Pcm
//*********************************************************************
bool AudmhlInIsRAW(void)
{
    bool fgRAW = FALSE ;

    /*
    if (AudmhlIsMHLIn())
    {
        while (!AudmhlMLinTypeDecided())
        {
            LOG(LOG_FEATURE, _T("Multiple line h/w type has not been decided\n"));
            mdelay(5);
        }
    }
    */
    if((g_eHDMIRxAudFmt==HDMI_RX_SD_RAW) || (g_eHDMIRxAudFmt==HDMI_RX_192k_RAW))
    {
        fgRAW = TRUE;
    }

    if (fgRAW==TRUE)
    {
        LOG(LOG_FEATURE, _T("[AUDIN_MULTI] Input Data is RAW.\n"));
    }
    else
    {
        LOG(LOG_FEATURE, _T("[AUDIN_MULTI] Input Data is non-RAW.\n"));
    }

    return (fgRAW);
}

/*********************************************************************
  bool AudmhlInIsDSD(void)
  Describe:
  Parameters: None
  Return:
*********************************************************************/
bool AudmhlInIsDSD(void)
{
    return ((AudmhlGetAudInType() == AUDINPORT_INTERNAL_HDMI_RX)
        && (g_rHDMIRxAudInfo.u1HDMIRxAudFmt == HDMI_RX_DSD));
}

/*********************************************************************
  u8 AudmhlInDSDChNum(void)
  Describe:
  Parameters: None
  Return:
*********************************************************************/
u8 AudmhlInDSDChNum(void)
{
    return 6;
}

/*********************************************************************
  u32 u4GetPsrPCMSize(void)
  Describe: CFA call this function to get size of Data2AFifo PCM.
  Parameters: None
  Return:
*********************************************************************/
u32 AudmhlGetPCMSize(void)
{
    u32 u4PsrPcmUintSize = 1024 ;

    if ((AudmhlGetAudInType() == AUDINPORT_INTERNAL_HDMI_RX))
    {
        switch (g_rHDMIRxAudInfo.u1HDMIRxAudFmt)
        {
        case HDMI_RX_PCM:
            //one frame = 256 sample * 3 byte/sample * ChNum
            u4PsrPcmUintSize = 256 * 3 * ((g_u4AudmhlChNum == AINACK_CFG_CH_NUM_2) ? 2 : 8) / 4;
            break;

        case HDMI_RX_HBR:
            u4PsrPcmUintSize = 256 * 3 * 8 / 2;
            break;

        case HDMI_RX_DSD:
            u4PsrPcmUintSize = 256 * 3 * 8 / 2;
            break;

        default:
            break;
        }
    }

    u4PsrPcmUintSize = HDMI_AUDIO_IN_SLOT_SIZE;
    LOG(0, _T("[FIXED]: AU size = 0x%x.\r\n"), u4PsrPcmUintSize);

    return u4PsrPcmUintSize;
}

/*********************************************************************
  AUDIN_HBR_AUDIO_T AudInHBRAudType(void)
  Describe: Get HBR type
  Parameters: None
  Return:   :HBR type: AUDIN_HBR_NONE/  AUDIN_HBR_MAT /  AUDIN_HBR_DTSMA
*********************************************************************/
AUDIN_HBR_AUDIO_T AudmhlHBRAudType(void)
{
    return AUDIN_HBR_NONE;
}

//*********************************************************************
// function: bool AudmhlInBufReorder(void)
// Describe: CFA check audio in buffer does re-order or not when HDMI In
// Parameters: None
// Return:  TRUE/FALSE for Re-order/Non-re-order
//*********************************************************************
bool AudmhlInBufReorder(void)
{
    bool fgReorder = FALSE ;

    if ((AudmhlGetAudInType() == AUDINPORT_INTERNAL_HDMI_RX))
    {
        if ((g_rHDMIRxAudInfo.u1HDMIRxAudFmt == HDMI_RX_PCM)
            ||(g_rHDMIRxAudInfo.u1HDMIRxAudFmt == HDMI_RX_HBR))
        {
#ifdef HBR_REORDER
            fgReorder = FALSE ;
            //LOG(9, "[AUDIN_MULTI] HDMI Rx In  Do H/W re-order\n");
#else
            fgReorder = TRUE ;
            //LOG(9, "[AUDIN_MULTI] HDMI Rx In Need F/W re-order.\n");
#endif
        }
        else
        {
            //LOG(9, "[AUDIN_MULTI] HDMI Rx In No need re-order.\n");
        }
    }
    return (fgReorder);
}


/*********************************************************************
void AudmhlParsingAudInfo(AUDIN_PARSING_INFO_T *prAudinPsringInfo)
Describe: Get HBR type
Parameters: None
Return:   :HBR type: AUDIN_HBR_NONE/  AUDIN_HBR_MAT /  AUDIN_HBR_DTSMA
*********************************************************************/
void AudmhlParsingAudInfo(void *pAudinPsringInfo)
{
    AUDIN_PARSING_INFO_T *prAudinPsringInfo = (AUDIN_PARSING_INFO_T *)pAudinPsringInfo;

    if(AUDMHL_IN == g_u1AudioInType)
    {
        prAudinPsringInfo->fgAudinDSD = AudmhlInIsDSD();
        prAudinPsringInfo->u1DSDChNum = AudmhlInDSDChNum();
        prAudinPsringInfo->fgAudinRAW = AudmhlInIsRAW();
        prAudinPsringInfo->fgAudinReOrder = AudmhlInBufReorder();
        prAudinPsringInfo->AudinHBRAudioType = AudmhlHBRAudType();
        prAudinPsringInfo->u4PsrPcmUintSize = AudmhlGetPCMSize();
    }
}

/************************************************************************
Function    : void AudmhlCfgMLin(AUDIO_IN_TYPE_T eInput, AUDIN_DIGITAL_DETECT eDetect)
Description : This function set AIN ACK CFG for MultipleLineIn according to channel input
Parameter   : AUDIO_IN_TYPE_T u1Input, AUDIN_FUNC u1Func
Return      : None
 ********************************************************************/
void AudmhlCfgMLin(AUDIO_IN_TYPE_T eInput, AUDIN_DIGITAL_DETECT eDetect)
{
    g_AudioInType = eInput;

    if(AUDINPORT_INTERNAL_HDMI_RX == eInput)
    {
        g_prAudInNfyInfo->u1AudinLockStatus = TRUE;
        g_hdmiaud_func.enableRxaud(TRUE);

        g_rMlinParams.eSrc = MLIN_SRC_HDMI_RX;
        g_prHdmiMlin->SetSrcType(g_prHdmiMlin, g_rMlinParams.eSrc);

    }
}

/************************************************************************
Function    : void AudmhlSwitchFunc(AUDIO_IN_TYPE_T u1Input, AUDIN_DIGITAL_DETECT u1Detect)
Description : This function set channel input for MultipleLineIn
Parameter   : AUDIO_IN_TYPE_T u1Input, AUDIN_FUNC u1Func
Return      : None ( By call back function)
 ********************************************************************/
void AudmhlSwitchFunc(AUDIO_IN_TYPE_T eAudinType, AUDIN_DIGITAL_DETECT eDetect)
{
    g_prAudInNfyInfo->u1AudinPauseStatus = 0;
    AudmhlSetAudInType(eAudinType); // For MultipleLineIn setting
    AudmhlCfgMLin(eAudinType, eDetect); // Config AIN_ACK_CFG_Multi for MultipleLineIn module
    g_prAudInNfyInfo->u1AudinSwitchOK = TRUE;
    AudmhlMsgToMW(AUDIN_CHG_SPDIFIN_INPUT_SW, g_prAudInNfyInfo->u1AudinSwitchOK);
}


/************************************************************************
Function    : bool AudmhlIsLocked(void)
Description : audmhl in is lock or unlock
Parameter   : AUDIO_IN_TYPE_T u1Input, AUDIN_FUNC u1Func
Return      : None ( By call back function)
 ********************************************************************/
bool AudmhlIsLocked(void)
{
    return g_prAudInNfyInfo ? (!g_prAudInNfyInfo->u1AudinLockStatus) : FALSE;
}

/************************************************************************
Function    : void AudmhlGetAudInInfo(AUDIN_INFO_T *pv_get_info)
Description : This function will let SCOM get AUDIN info
Parameter   : None
Return      : None
 ********************************************************************/
static u32 g_u2DataCount = 0;
void AudmhlGetAudInInfo(AUDIN_INFO_T *pv_get_info)
{
    while ((!g_fgDateGeted) && (g_u2DataCount < 30))
    {
        LOG(LOG_CTRLF, TEXT("[audmhl_task]g_fgDateGeted %d, count %d\r\n"), g_fgDateGeted, g_u2DataCount);
        g_u2DataCount++;
        Sleep(10);
    }
    g_u2DataCount = 0;
    *pv_get_info = *g_prAudInNfyInfo;
}

/************************************************************************
Function    : void AudmhlGetHDMIRXAUDINFO(HDMI_RX_IN_AUDIO_INFO_T *pv_get_info)
Description : This function will let SCOM get AUDIN info
Parameter   : None
Return      : None
 ********************************************************************/
void AudmhlGetHDMIRXAUDINFO(HDMI_RX_IN_AUDIO_INFO_T *pv_get_info)
{
    g_hdmiaud_func.getRxaudinfo(pv_get_info);
}

#ifndef __linux__
/************************************************************************
Function    : void AudmhlHandleCreateMsgQ
Description : This function will let SCOM get AUDIN info
Parameter   : None
Return      : None
 ********************************************************************/
void * AudmhlHandleCreateMsgQ(const s8* pName,size_t zMsgSize,
    u16 u4MsgCount, bool fgRead)
{
    MSGQUEUEOPTIONS options = {0};
    void * hRetHand = NULL;


    options.dwSize = sizeof(MSGQUEUEOPTIONS);
    options.cbMaxMessage = zMsgSize;
    options.dwMaxMessages = u4MsgCount;
    options.dwFlags = 0;
    options.bReadAccess = fgRead;
    hRetHand = (void *)CreateMsgQueue(pName,&options);

    return (hRetHand);

}

void AudMhlHandleDeleteMsgQ(void * hHandle)
{
    if (NULL != hHandle)
    {
        CloseMsgQueue(hHandle);
    }
}

/************************************************************************
Function    : s32 AudmhlCreateMsgQ(u32* pMsgHandle, const s8* pName,
                        size_t zMsgSize, u16 u4MsgCount)
Description : create msg queue
Parameter   : None
Return      : None
**********************************************************************/
s32 AudmhlCreateMsgQ(u32* pMsgHandle, const s8* pName,
                        size_t zMsgSize, u16 u4MsgCount, bool fgRead)
{
    MSGQUEUEOPTIONS rMsgOptions = {0};
    void * hTmp = NULL;
    AUD_MSG_QUEUE_T* hTmpQueue = NULL;

    if(NULL == pMsgHandle)
    {
        LOG(LOG_FAIL, _T("Input handle is NULL.\n"));
        return (OSR_INV_ARG);
    }

    hTmpQueue = (AUD_MSG_QUEUE_T *)LocalAlloc(LPTR, sizeof(AUD_MSG_QUEUE_T));
    if(NULL == hTmpQueue)
    {
        LOG(LOG_FAIL, _T("LocalAlloc memory is error\n"));
        return (OSR_INV_ARG);
    }

    rMsgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    rMsgOptions.cbMaxMessage = zMsgSize;
    rMsgOptions.dwMaxMessages = u4MsgCount;
    rMsgOptions.dwFlags = 0;
    rMsgOptions.bReadAccess = fgRead;
    hTmp = (void *)CreateMsgQueue(pName, &rMsgOptions);
    if (NULL == hTmp)
    {
        LocalFree(hTmpQueue);
        hTmpQueue = NULL;
        return OSR_NO_RESOURCE;
    }
    hTmpQueue->hQueue = hTmp;
    *pMsgHandle = (u32)hTmpQueue;

    return (OSR_OK);
}



u32 AudmhlWriteMsgQ(void * hMsgQ, void* pData, u32 u4Size)
{
    if(hMsgQ == NULL)
    {
        return (OSR_INV_ARG);
    }

    if(!WriteMsgQueue(hMsgQ, pData,u4Size,INFINITE,0))
    {
        LOG(LOG_FAIL, TEXT("Write Queue Value failed./r/n"));
    }
    return (OSR_OK);
}


u32 AudmhlReadMsgQ(void *lpParameter)
{
    AUD_MSG_QUEUE_T* hMsgQ = (AUD_MSG_QUEUE_T*)(lpParameter);
    u32 dwRead = 0;
    u32 dwFlag = 0;
    u32 u4Msg = 0;
    if(hMsgQ == NULL)
    {
        return 0;
    }

    if(ReadMsgQueue(hMsgQ->hQueue,&u4Msg,sizeof(u32),&dwRead,INFINITE,&dwFlag))
    {
        u32 u4RetMsg = u4Msg;
        LOG(LOG_IO,TEXT("Read Queue Value:%d.\r\n"),u4RetMsg);
    }
    else
    {
        LOG(LOG_FAIL, TEXT("Read Queue Value failed/r/n"));
    }
    return (OSR_OK);
}

s32 AudmhlDeleteMsgQ(u32* MsgHandle)
{
    AUD_MSG_QUEUE_T* phMsgQ = (AUD_MSG_QUEUE_T*)MsgHandle;
    if(NULL != phMsgQ)
    {
        CloseMsgQueue(phMsgQ->hQueue);
    }
    LocalFree(phMsgQ);
    return (OSR_OK);

}
#else
void vUpdateRp(__u32 u4RP)
{
    u32 u4Tmp;

    if ((u4RP >= g_rAudmhlInBuf.u4Buf_SA) && (u4RP < g_rAudmhlInBuf.u4Buf_EA))
    {
        if(u4RP <= g_rAudmhlInBuf.u4Buf_W)
        {
            u4Tmp = u4RP + AUD_HDMI_RX_BUF_SIZE - g_rAudmhlInBuf.u4Buf_W;
        }
        else
        {
            u4Tmp = u4RP - g_rAudmhlInBuf.u4Buf_W;
        }

        if(u4Tmp <= HDMI_AUDIO_IN_SLOT_SIZE)
        {
            LOG(LOG_FAIL, _T("VirRP=0x%x,BufWr=0x%x,LineIn Buffer is Full, waiting..\n"), u4RP, g_rAudmhlInBuf.u4Buf_W);
        }
    }
    else
    {
        LOG(LOG_FAIL, _T("[AUD]audmhl error demuxer update rp 0x%x, not between 0x%x and 0x%x\n"),
            u4RP, g_rAudmhlInBuf.u4Buf_SA, g_rAudmhlInBuf.u4Buf_EA);
    }
}

extern u32 g_u4PsrSessionID;
__u32 u4GetPsrSessionID(void)
{
    return (__u32)g_u4PsrSessionID;
}

void Audmhl_Reg_ForDemuxer(void *pvDmxTag,  void*pCallbacks)
{
    IDMXPBBUFCALLBACKS_T* prCallbacks  = (IDMXPBBUFCALLBACKS_T*)pCallbacks;

    g_pvDmxTag = pvDmxTag;
    g_pu4SendSlot = prCallbacks->pu4SendSlot;

    g_AudmhlCb.pvUpdateRpCb = vUpdateRp;
    g_AudmhlCb.pfgAudInIsRaw = AudmhlInIsRAW;
    g_AudmhlCb.pvGetAudInParsingInfo = AudmhlParsingAudInfo;
    g_AudmhlCb.pu4GetPsrSessionID = u4GetPsrSessionID;

    prCallbacks->pi4SubscribeDrvCb(pvDmxTag, &g_AudmhlCb);

    g_i4UnSubscribeDrvCb = prCallbacks->pi4UnSubscribeDrvCb;

    if (g_pu4SendSlot == NULL)
    {
        LOG(LOG_FAIL, _T("g_pu4SendSlot == NULL\n"));
    }

    LOG(LOG_MHL, _T("[AUD]audmhl Audmhl_Reg_ForDemuxer\n"));
}
EXPORT_SYMBOL(Audmhl_Reg_ForDemuxer);

bool RegHDMIAudFunc(HDMI_REG_AUD_F *pRegFun)
{
    g_hdmiaud_func.enableRxaud = pRegFun->enableRxaud;
    g_hdmiaud_func.getRxaudinfo = pRegFun->getRxaudinfo;
    g_hdmiaud_func.getRxchannel = pRegFun->getRxchannel;
    g_hdmiaud_func.getRxaudinfoframe = pRegFun->getRxaudinfoframe;
    g_hdmiaud_func.getRxacptype = pRegFun->getRxacptype;
    g_hdmiaud_func.fgReg = pRegFun->fgReg;
    LOG(LOG_MHL, _T("[AUD]RegHDMIAudFunc\n"));
    return TRUE;
}
EXPORT_SYMBOL(RegHDMIAudFunc);

void Audmhl_UnReg_ForDemuxer(void)
{
    g_pvDmxTag = NULL;
    g_pu4SendSlot = NULL;
    g_i4UnSubscribeDrvCb = NULL;
    LOG(LOG_FAIL, _T("[AUD]Audmhl_UnReg_ForDemuxer\n"));
}
EXPORT_SYMBOL(Audmhl_UnReg_ForDemuxer);
#endif

#endif

