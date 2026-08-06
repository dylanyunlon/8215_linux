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

/* #if (DRV_SUPPORT_HDMI_RX) */
#include "hdmi_rx_aud_task.h"
#include "spdif_if.h"
#include "hdmi_rx_hal.h"
#include "mhl_drv_if.h"
#include "hdmi_rx_ctrl.h"
#include "hdmi_debug.h"
#include "windows.h"
#include <linux/err.h>

#define AUDIO_READY_TIMEOUT   1  /* step size = 20ms */
#define DEFAULT_START_FIXED_AUD_SAMPLEFREQ AUDFS_192KHz

HDMI_RX_IN_AUDIO_INFO_T _hdmi_rx_aud;

Audio_State_Type AState = ASTATE_AudioOff;
static BOOL MuteByPKG = OFF;
static UINT32 AudioCountingTimer;
UINT8 _u1ACPTYPE = ACP_TYPE_GENERAL_AUDIO;
BOOL _fgACPMUTE = FALSE;
BOOL _fgSendACR  = TRUE;


static _XDATA AUDIO_CAPS AudioCaps;
BOOL _fgHDMIRxAudTaskEnable = FALSE;
BOOL _fgAudUnderrun = FALSE;
BOOL _fgAudOverrun = FALSE;
BOOL _fgAudAutoConfig = FALSE;
BOOL  _fgHDMIRxBypassMode = FALSE;
UINT8 _u1HDMIAFifoErrCnt = 0;
UINT8 _u1HDMIAudMCLKErrCnt = 0;


#if 0
/* audio message queue variable */
UINT32 u4CreateMsgFlag = 0;
HANDLE_T hMhl2AudCmdQueue = 0;
#define MHL2AUD_CMD_Q_NAME   _T("AUDInHDMICmd")
#endif



void MuteAudioOutALL(void)
{
}
void UnMuteAudioOutALL(void)
{
}

void vSendAUDINCmd(UINT32 u4Cmd, BYTE bPri)
{
#ifndef __linux__

	if (FALSE == MhlSendAudInfo(u4Cmd))
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]MHL send audio information failed. \r\n");

#else
	AudmhlSendAudMsg(u4Cmd, bPri);/*just for build err quzhi */
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]MHL send audio information u4Cmd 0x%x. \r\n", (unsigned int)u4Cmd);
#endif
}

const CHAR *AStateStr[6] = {
	"ASTATE_AudioOff",
	"ASTATE_RequestAudio",
	"ASTATE_ResetAudio",
	"ASTATE_WaitForReady",
	"ASTATE_AudioOn",
	"ASTATE_Reserved"
};

/*    Audio Driver Main Loop
      {

      State - ... ... :

      State - Set Decoder Format :
      1.  Set Decoder Type to PCM decoder.

      State - Before Play :
      1. Switch Audio Clock to HDMI source

      State - On Play :
      1. Polling HDMI Status for every 20 ms.
      _AudHdmiOnPlayStateHandler( )

      State - ... ...


      }
*/

/*************************************************************************************
 void vEnableAOCLK(BOOL fgEn)
 Describe: This function to enable/disable AOMCLK/AOBCK/AOLRCK pins to avoid unstable clock output.

 Parameters: BOOL fgEn

 Return: Non

*************************************************************************************/

void vEnableAOCLK(BOOL fgEn)
{

	/*static BOOL s_fgEn = TRUE;


	if (s_fgEn != fgEn)
	{
	if (fgEn)
	{
	    BSP_PinSet(AUD_MCLK_SEL, 1);
	    ADAC_SpecialMute(FALSE);
	}
	else
	{
	    ADAC_SpecialMute(TRUE);
	    GPIO_Config(PIN_AOMCLK, OUTPUT, LOW);
	    GPIO_Config(PIN_AOBCK , OUTPUT, LOW);
	    GPIO_Config(PIN_AOLRCK, OUTPUT, LOW);
	}

	s_fgEn = fgEn;
	}
	*/
}
EXPORT_SYMBOL(vEnableAOCLK);
/*************************************************************************************
 void vEnableHdmiRxAudTask(BOOL fgOn)
 Describe: This function to  enable/disable hdmi rx audio task , called by audio in task

 Parameters: fgOn = TRUE for turn on hdmi rx audio task, FALSE or turn off hdmi rx audio task

 Return: Non

*************************************************************************************/



void vEnableHdmiRxAudTask(BOOL fgOn)
{
	if (fgOn) {

		HalHDMIRxEnableAudPktReceive();
		/* HalHdmiRxAudResetAudio(); */
		SwitchAudioState(ASTATE_RequestAudio);
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]Enable HDMI Rx Audio Task. \r\n");
	} else {
		SwitchAudioState(ASTATE_AudioOff);
		HalHdmiRxAudBypass(FALSE, FALSE);
		_fgHDMIRxBypassMode = FALSE;
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]Disable HDMI Rx Audio Task. \r\n");
	}

	_fgHDMIRxAudTaskEnable = fgOn;

}


/*************************************************************************************
 UINT8 u1HDMIRxAudErrorGet(void)
 Describe: This function is to get audio error information

 Parameters: Non

 Return: Non

*************************************************************************************/
UINT32 u4HDMIRxAudErrorGet(void)
{
	UINT32 u4AudErr = HalHDMIRxAudErrorGet();

	if (u4AudErr & HDMIRX_HBR_PACKET) {
		/*   vHalSetHDMIRxHBR(); */
	}

	if (u4AudErr & HDMIRX_DSD_PACKET) {
		/*  vRxWriteReg(0x1F100,0x00f20002); */
	}

	if (u4AudErr & HDMIRX_AFIFO_UNDERRUN) {
		/* write "1" to clean audio underrun
		vRegWriteFldAlign(INTR_STATE1,0x1,INTR4_UNDERRUN);
		vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR4_UNDERRUN));
		msleep(1); */
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]HDMI Rx Audio Error : Audio Fifo Under-run. \r\n");
	}

	if (u4AudErr & HDMIRX_AFIFO_OVERRUN) {
		/* write "1" to clean audio underrun
		vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR4_OVERRUN));
		vRegWriteFldAlign(INTR_STATE1,0x1,INTR4_OVERRUN);
		msleep(1); */
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]HDMI Rx Audio Error : Audio Fifo Over-run. \r\n");
	}

	if (u4AudErr & HDMIRX_AUD_FS_CHG) {
		/* write "1" to clean audio underrun
		vRegWriteFldAlign(INTR_STATE1,0x1,INTR4_HDCP_PKT_ERR_ALERT);
		vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR4_HDCP_PKT_ERR_ALERT));
		msleep(1); */
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]HDMI Rx Audio Error : FS CHG Error. \r\n");
	}

	if (u4AudErr & HDMIRX_TERC4_ERROR) {
		/* write "1" to clean audio underrun
		vRegWriteFldAlign(INTR_STATE1,0x1,INTR4_T4_PKT_ERR_ALERT);
		vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR4_T4_PKT_ERR_ALERT));
		msleep(1); */
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]HDMI Rx Audio Error : TREC4 Packet Error. \r\n");
	}

	if (u4AudErr & HDMIRX_HDCP_ERROR) {
		/* write "1" to clean audio underrun
		vRegWriteFldAlign(INTR_STATE1,0x1,INTR4_HDCP_PKT_ERR_ALERT);
		vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR4_HDCP_PKT_ERR_ALERT));
		msleep(1); */
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]HDMI Rx Audio Error : HDCP Packet Error. \r\n");
	}

	return u4AudErr;
}

/*************************************************************************************
 BOOL fgIsHDMIRxAud(void)
 Describe: This function indicates if the UI select HDMI Rx Audio Input or not.

 Parameters: Non

 Return: Non

*************************************************************************************/
BOOL IsHDMIRxAud(void)
{
	return TRUE;  /* zhongjie: consider removing */
}


/*************************************************************************************
void vHDMIRxUnPlugNotifyAud(void)
Describe: This function is called by hdmi rx ctrl to notify audio task when the HDMI is plug out.

 Parameters: Non

 Return: Non

*************************************************************************************/
void HDMIRxUnPlugNotifyAud(void)
{
	/* Check if hdmi rx audio task is ready or not. */
	if (!IsHDMIRxAud())
		return;

	/* Plug out */
	MuteAudioOutALL();
	vSendAUDINCmd(AUDIN_CHG_HDMIRX_INT | (HDMI_RX_PLUG_OUT << 8), AUDIN_CMD_PRI_HIGH);
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]HDMI Cable is plug out. \r\n");
}
/*************************************************************************************
 BOOL GetAudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus, UINT8 audio_status)
 Describe: This function to get audio channel status

 Parameters: Non

 Return: Non

*************************************************************************************/
BOOL GetAudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus, UINT8 audio_status)
{
	BYTE uc;

	if (((audio_status & T_AUDIO_MASK) == T_AUDIO_OFF) ||
	    ((audio_status & T_AUDIO_MASK) == T_AUDIO_DSD)) {
		/* return false if no audio or one-bit audio. */
		return FALSE;
	}

	/* SwitchHDMIRXBank(0); */
	uc = HalHDMIRxAudioCHSTAT0();

	if ((audio_status & T_AUDIO_MASK) == T_AUDIO_HBR)
		RegChannelstatus->ISLPCM = 1;
	else
		RegChannelstatus->ISLPCM = (uc & 0x02) >> 1;

	RegChannelstatus->CopyRight = (uc & 0x04) >> 2;
	RegChannelstatus->AdditionFormatInfo = (uc & 0x18) >> 3;
	RegChannelstatus->ChannelStatusMode = (uc & 0xE0) >> 5;
	RegChannelstatus->CategoryCode = HalHDMIRxAudioCHSTAT1();
	RegChannelstatus->SourceNumber = HalHDMIRxAudioCHSTAT2() & 0x0F;
	RegChannelstatus->ChannelNumber = (HalHDMIRxAudioCHSTAT2() & 0xf0) >> 4;
	uc = HalHDMIRxAudFsGet();

	if (HalHDMIRxHDAudio())
		RegChannelstatus->SamplingFreq = B_Fs_HBR;
	else
		RegChannelstatus->SamplingFreq = uc & 0x0F;

	RegChannelstatus->ClockAccuary = HalHDMIRxAudioCHSTAT3() & 0x0F;
	RegChannelstatus->WorldLen = (HalHDMIRxAudioCHSTAT3() & 0xF0) >> 4;
	RegChannelstatus->OriginalSamplingFreq = (HalHDMIRxAudioCHSTAT4() & 0xF0) >> 4;
	return TRUE;
}


/* This function is for HDMI RX task get adio inforframe */
int ExportAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame)
{
	if (pAudioInfoFrame == NULL)
		return ER_FAIL;

	pAudioInfoFrame->pktbyte.AUD_HB[0] =  AudioCaps.AudInf.pktbyte.AUD_HB[0]; /* AUDIO InfoFrame */
	pAudioInfoFrame->pktbyte.AUD_HB[1] = AudioCaps.AudInf.pktbyte.AUD_HB[1];
	pAudioInfoFrame->pktbyte.AUD_HB[2] = AudioCaps.AudInf.pktbyte.AUD_HB[2];
	pAudioInfoFrame->info.Type = AudioCaps.AudInf.info.Type;
	pAudioInfoFrame->info.Ver = AudioCaps.AudInf.info.Ver;
	pAudioInfoFrame->info.Len = AudioCaps.AudInf.info.Len;
	pAudioInfoFrame->info.AudioChannelCount = AudioCaps.AudInf.info.AudioChannelCount;
	pAudioInfoFrame->info.RSVD1 = AudioCaps.AudInf.info.RSVD1;
	pAudioInfoFrame->info.AudioCodingType = AudioCaps.AudInf.info.AudioCodingType;
	pAudioInfoFrame->info.SampleSize = AudioCaps.AudInf.info.SampleSize;
	pAudioInfoFrame->info.SampleFreq = AudioCaps.AudInf.info.SampleFreq;
	pAudioInfoFrame->info.Rsvd2 = AudioCaps.AudInf.info.Rsvd2;
	pAudioInfoFrame->info.FmtCoding = AudioCaps.AudInf.info.FmtCoding; /* actually spec does not have this */
	pAudioInfoFrame->info.SpeakerPlacement = AudioCaps.AudInf.info.SpeakerPlacement;
	pAudioInfoFrame->info.Rsvd3 = AudioCaps.AudInf.info.Rsvd3;
	pAudioInfoFrame->info.LevelShiftValue = AudioCaps.AudInf.info.LevelShiftValue;
	pAudioInfoFrame->info.DM_INH = AudioCaps.AudInf.info.DM_INH;
	return ER_SUCCESS;
}

void ExportAudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus)
{
	RegChannelstatus->ISLPCM = AudioCaps.AudChStat.ISLPCM;
	RegChannelstatus->CopyRight = AudioCaps.AudChStat.CopyRight;
	RegChannelstatus->AdditionFormatInfo = AudioCaps.AudChStat.AdditionFormatInfo;
	RegChannelstatus->ChannelStatusMode = AudioCaps.AudChStat.ChannelStatusMode;
	RegChannelstatus->CategoryCode = AudioCaps.AudChStat.CategoryCode;
	RegChannelstatus->SourceNumber = AudioCaps.AudChStat.SourceNumber;
	RegChannelstatus->ChannelNumber = AudioCaps.AudChStat.ChannelNumber;
	RegChannelstatus->SamplingFreq = AudioCaps.AudChStat.SamplingFreq;
	RegChannelstatus->ClockAccuary = AudioCaps.AudChStat.ClockAccuary;
	RegChannelstatus->WorldLen = AudioCaps.AudChStat.WorldLen;
	RegChannelstatus->OriginalSamplingFreq = AudioCaps.AudChStat.OriginalSamplingFreq;
}


void vGetHdmiRxAudioParameter(HDMI_RX_IN_AUDIO_INFO_T *prAudIf)
{
	prAudIf->u1HBRAudio =  _hdmi_rx_aud.u1HBRAudio;
	prAudIf->u1DSDAudio =  _hdmi_rx_aud.u1DSDAudio;
	prAudIf->u1RawSDAudio =  _hdmi_rx_aud.u1RawSDAudio;
	prAudIf->u1PCMMultiCh =  _hdmi_rx_aud.u1PCMMultiCh;

	if (_hdmi_rx_aud.u1HBRAudio) {
		/* Hardware return 0 if HBR, so we should change it before setting multiple line in module */
		_hdmi_rx_aud.u1PCMMultiCh = 1;
	}

	prAudIf->u1FsDec =  _hdmi_rx_aud.u1FsDec;
	prAudIf->u1MCLKRatio = _hdmi_rx_aud.u1MCLKRatio;
	prAudIf->u1I2sChanValid = _hdmi_rx_aud.u1I2sChanValid;
	prAudIf->u1I2sCh0Sel = _hdmi_rx_aud.u1I2sCh0Sel;
	prAudIf->u1I2sCh1Sel = _hdmi_rx_aud.u1I2sCh1Sel;
	prAudIf->u1I2sCh2Sel = _hdmi_rx_aud.u1I2sCh2Sel;
	prAudIf->u1I2sCh3Sel = _hdmi_rx_aud.u1I2sCh3Sel;
}

int GetHdmiRxAudioInfoFrame(HDMI_RX_Audio_InfoFrame *pAudioInfoFrame)
	/* HDMI_RX_Audio_InfoFrame. please see x_audin.h, just info is used */
{
	Audio_InfoFrame *pAudInfoFrame = (Audio_InfoFrame *)pAudioInfoFrame;

	ExportAudioInfoFrame(pAudInfoFrame);
	HDMI_LOG(HDMI_LOG_DEBUG, "get hdmi rx audio infoframe \r\n");
	return 0;
}

UINT8 u1GetHDMIRxACPType(void)
{
	HDMI_LOG(HDMI_LOG_DEBUG, "get hdmi rx acp type \r\n");
	return 0;
}

int GetHdmiRxAudioChannelStatus(HDMI_RX_AUDIO_CHSTS *pHdmiRxChStat)/* HDMI_RX_AUDIO_CHSTS in x_audin.h */
{
	RX_REG_AUDIO_CHSTS *pRxRegChannelstatus = NULL;

	HDMI_LOG(HDMI_LOG_DEBUG, "get hdmi rx audio channel status \r\n");
	pRxRegChannelstatus = (RX_REG_AUDIO_CHSTS *)pHdmiRxChStat;
	ExportAudioChannelStatus(pRxRegChannelstatus);
	return 0;
}

#if 0
INT32 CreateAudWrMsgQueue(HANDLE_T *pMsgHandle, const TCHAR *pName,
			  SIZE_T zMsgSize, UINT16 u4MsgCount)
{
	MSGQUEUEOPTIONS options = {0};


	HANDLE_T *hTmp = (HANDLE_T *)LocalAlloc(LPTR, sizeof(*pMsgHandle));

	if (NULL == pMsgHandle || hTmp == NULL)
		return OSR_INV_ARG;

	options.dwSize = sizeof(MSGQUEUEOPTIONS);
	options.cbMaxMessage = zMsgSize;
	options.dwMaxMessages = u4MsgCount;
	options.dwFlags = 0;
	options.bReadAccess = FALSE;

	hTmp = CreateMsgQueue(pName, &options);

	if (NULL == hTmp) {
		LocalFree(hTmp);
		return OSR_NO_RESOURCE;
	}

	*pMsgHandle = (HANDLE_T)hTmp;

	return OSR_OK;

}

INT32 WriteAudMsgQueue(VOID *pvMsg, HANDLE_T *pMsgHandle)
{
	UINT32 timeout = 0;
	DWORD dwRead = 0;
	DWORD dwFlag = 0;

	if (WriteMsgQueue(pMsgHandle, pvMsg, MAX_PATH, INFINITE, 0))
		return OSR_OK;
	else
		return OSR_INV_ARG;
}

INT32 DeleteAudMsgQueue(HANDLE_T MsgHandle)
{
	HANDLE phMsg = (HANDLE)MsgHandle;

	if (NULL != phMsg)
		CloseMsgQueue(phMsg);

	LocalFree(phMsg);
	return OSR_OK;

}
#endif

void vHDMIRxAudMainTask(void) /* AudioTimerHandler */
{
	static UINT16 u2TimeCounter = 0xFFF;
	AUDIO_CAPS CurAudioCaps;

	if (!IsHDMIRxAud())
		return;

	switch (AState) {
	case ASTATE_RequestAudio:
		SetupAudio();
		break;

	case ASTATE_WaitForReady:

		/* 03032010 Reg77.bit7=1 for i2s and SPDIF can output signal on NLPCM. */
		/*HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL, HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL) | 0x80); */
		/* end */
		if (AudioCountingTimer == 0) {
			if (HalHDMIRxAPLLStatus() && ((u4HDMIRxAudErrorGet()&HDMIRX_INT_STATUS_CHK) == 0x00)
				&& (AudioCaps.SampleFreq != B_Fs_UNKNOW)) {
				if (HalCheckIsAAC()) {
					HDMI_LOG(HDMI_LOG_DEBUG,
					"[HDMI RX AUD]ASTATE_WaitForReady, Audio AutoMute. \r\n");
				} else {
					SwitchAudioState(ASTATE_AudioOn);
					_u1HDMIAFifoErrCnt = 0;
					u2TimeCounter = 0xFFF;
				}
			} else {
				_u1HDMIAFifoErrCnt++;

				if (_u1HDMIAFifoErrCnt == 50) {
					if (!HalHDMIRxAPLLStatus()) {
						HDMI_LOG(HDMI_LOG_INFO,
							"[HDMI RX AUD]ASTATE_WaitForReady, RX MCLK is not stable , Reset Audio. \r\n");
						HalHdmiRxAudResetAudio();
					}

					_u1HDMIAFifoErrCnt = 0;
				} else {
					if (u4HDMIRxAudErrorGet()&HDMIRX_INT_STATUS_CHK) {
						/* Reset Audio Fifo if error occurs */

					if (_u1HDMIAFifoErrCnt % 2 == 0) {
						HDMI_LOG(HDMI_LOG_INFO,
							"[HDMI RX AUD]ASTATE_WaitForReady, RX AFIFO Error , Reset AFIFO. \r\n");
						HalHdmiRxAudResetAfifo();
					}
					}
				}
			}
		} else {
			AudioCountingTimer--;
		}
		break;

	case ASTATE_AudioOn: {
		BOOL fgError = FALSE;

		getHDMIRxInputAudio(&CurAudioCaps);

		if ((AudioCaps.AudioFlag & (~B_SPDIF)) != (CurAudioCaps.AudioFlag & (~B_SPDIF))) {
			HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX AUD]ASTATE_AudioOn : AudioFlag %0x != %0x. \r\n",
				AudioCaps.AudioFlag , CurAudioCaps.AudioFlag);
			fgError = TRUE;
		}

		if (AudioCaps.AudSrcEnable != CurAudioCaps.AudSrcEnable) {
			HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX AUD]ASTATE_AudioOn : AudSrcEnable %0x != %0x. \r\n",
				AudioCaps.AudSrcEnable , CurAudioCaps.AudSrcEnable);
			fgError = TRUE;
		}

		if (AudioCaps.SampleFreq != CurAudioCaps.SampleFreq) {
			HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX AUD]ASTATE_AudioOn : SampleFreq %0x != %0x. \r\n",
				AudioCaps.SampleFreq , CurAudioCaps.SampleFreq);
			fgError = TRUE;
		}

		if (CurAudioCaps.SampleFreq == B_Fs_UNKNOW) {
			HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]ASTATE_AudioOn : Sample frequency not indicated. \r\n");
			fgError = TRUE;
		}

		if (CurAudioCaps.AudChStat.ISLPCM != AudioCaps.AudChStat.ISLPCM) {
			HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX AUD]ASTATE_AudioOn : ISPCM %0x != %0x. \r\n",
				AudioCaps.AudChStat.ISLPCM, CurAudioCaps.AudChStat.ISLPCM);
			AudioCaps.AudChStat.ISLPCM = CurAudioCaps.AudChStat.ISLPCM;
			/* vSendAUDINCmd(AUDIN_CHG_HDMIRX_INT | (HDMI_RX_AUDIO_CHG_PAUSE_STATUS << 8),
			AUDIN_CMD_PRI_HIGH); */
			fgError = TRUE;
		}

		if (fgError) {
			HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]ASTATE_AudioOn: Stream change, re-start audio. \r\n");

			if (HalChkAudPktReady()) {
				HalHdmiRxSetApll();
				HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]ASTATE_AudioOn: ResetAudio and APLL \r\n");
			}

			SwitchAudioState(ASTATE_AudioOff);
			SwitchAudioState(ASTATE_RequestAudio);
		} else {
			/* 2010/08/23,ychung check rx_mclk stale or not */
			if (!HalHDMIRxAPLLStatus()) {
				HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]ASTATE_AudioOn: RxAPLL unstable \r\n");
			} else {
				if (u2TimeCounter < 500)
					u2TimeCounter++;
				else
					u2TimeCounter = 0xFFF;

				if (u4HDMIRxAudErrorGet()&HDMIRX_INT_STATUS_CHK) {
					if (u2TimeCounter == 0xFFF) {
						u2TimeCounter = 0;
						_u1HDMIAFifoErrCnt = 0;
					}

					if (_u1HDMIAFifoErrCnt % 2 == 0) {
						HDMI_LOG(HDMI_LOG_INFO,
							"[HDMI RX AUD]ASTATE_AudioOn : Afifo error, Reset AFIFO.(%u) \r\n",
							_u1HDMIAFifoErrCnt);
						HalHdmiRxAudResetAfifo();
					}

					_u1HDMIAFifoErrCnt++;

					if (_u1HDMIAFifoErrCnt > 5) {
						/* HalHdmiRxAudResetAudio(); */
						/* RETAILMSG(1,(TEXT("[HDMI RX AUD]ASTATE_AudioOn :
						AFIFO Error Hdmi Rx  ResetAudio. \r\n"))); */
						HalHdmiRxSetApll();
						HDMI_LOG(HDMI_LOG_INFO,
						"[HDMI AUD RESET]ASTATE_AudioOn : AFIFO Error, Reset ACR can't work,Force Reset APLL.(%u)\r\n",
						u2TimeCounter);
						SwitchAudioState(ASTATE_AudioOff);
						SwitchAudioState(ASTATE_RequestAudio);
					}
				}
			}

			if ((AudioCaps.AudInf.info.SpeakerPlacement != CurAudioCaps.AudInf.info.SpeakerPlacement) ||
			    (AudioCaps.AudInf.info.AudioCodingType != CurAudioCaps.AudInf.info.AudioCodingType) ||
			    (AudioCaps.AudInf.info.SampleFreq != CurAudioCaps.AudInf.info.SampleFreq)
			   ) {
				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]ASTATE_AudioOn Audio : InforFrame Change \r\n");
				HDMI_LOG(HDMI_LOG_DEBUG,
					"[HDMI RX AUD]AudioCaps.AudInf.info.SpeakerPlacement =0x%x \r\n",
					CurAudioCaps.AudInf.info.SpeakerPlacement);
				HDMI_LOG(HDMI_LOG_DEBUG,
					"[HDMI RX AUD]AudioCaps.AudInf.info.AudioCodingType =0x%x \r\n",
					CurAudioCaps.AudInf.info.AudioCodingType);
				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]AudioCaps.AudInf.info.SampleFreq =0x%x \r\n",
					CurAudioCaps.AudInf.info.SampleFreq);
			}

			if (AudioCountingTimer != 0)
				AudioCountingTimer--;
		}
	}
	break;

	default:
		break;
	}
}



/*//////////////////////////////////////////////////////////
 Connection Interface
/////////////////////////////////////////////////////////// */
/*void Check_HDMInterrupt(void)
  {
  Interrupt_Handler() ;
  }
  */
/* This function is for HDMI RX task get adio inforframe */
void  vUpdateHdmiRxAudio(void)
{
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]AudioCaps.AudioFlag=0x%x \r\n", AudioCaps.AudioFlag);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]AudioCaps.AudSrcEnable=0x%x \r\n", AudioCaps.AudSrcEnable);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]AudioCaps.SampleFreq=0x%x \r\n", AudioCaps.SampleFreq);

	if (HalHDMIRxHDAudio()) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]B_CAP_HBR_AUDIO \r\n");
		_hdmi_rx_aud.u1HBRAudio = 1;
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]NON B_CAP_HBR_AUDIO \r\n");
		_hdmi_rx_aud.u1HBRAudio = 0;
	}

	if (HalHDMIRxDSDAudio()) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]B_CAP_DSD_AUDIO \r\n");
		_hdmi_rx_aud.u1DSDAudio = 1;
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]NON B_CAP_DSD_AUDIO \r\n");
		_hdmi_rx_aud.u1DSDAudio = 0;
	}

	if (HalHDMIRxMultiPCM()) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]B_LAYOUT = 1, Muti-Channel \r\n");
		_hdmi_rx_aud.u1PCMMultiCh = 1;
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]B_LAYOUT = 0, 2-Channel \r\n");
		_hdmi_rx_aud.u1PCMMultiCh = 0;
	}

	if (AudioCaps.AudioFlag & B_CAP_LPCM) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]stream is PCM \r\n");
		_hdmi_rx_aud.u1RawSDAudio = 0;
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]stream is non-PCM \r\n");

		if (AudioCaps.AudioFlag & B_CAP_DSD_AUDIO)
			_hdmi_rx_aud.u1RawSDAudio = 0;
		else
			_hdmi_rx_aud.u1RawSDAudio = 1;
	}

	_hdmi_rx_aud.u1FsDec = AudioCaps.SampleFreq;

	if (AudioCaps.SampleFreq == 0)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 44.1kHz \r\n");

	if (AudioCaps.SampleFreq == 2)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 48kHz \r\n");

	if (AudioCaps.SampleFreq == 3)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 32kHz \r\n");

	if (AudioCaps.SampleFreq == 8)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 88.2kHz \r\n");

	if (AudioCaps.SampleFreq == 9)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 768kHz \r\n");

	if (AudioCaps.SampleFreq == 0x0a)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 96kHz \r\n");

	if (AudioCaps.SampleFreq == 0x0c)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 176kHz \r\n");

	if (AudioCaps.SampleFreq == 0x0E)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 192kHz \r\n");

	_hdmi_rx_aud.u1MCLKRatio = HDMI_HalGetI2sMclk();

	switch (_hdmi_rx_aud.u1MCLKRatio) {
	case  RX_MCLK_128FS:
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]Get Audio Output MCLK  is 128FS \r\n");
		break;

	case  RX_MCLK_256FS:
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]Get Audio Output MCLK  is 256FS \r\n");
		break;

	case  RX_MCLK_384FS:
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]Get Audio Output MCLK  is 384FS \r\n");
		break;

	case  RX_MCLK_512FS:
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]Get Audio Output MCLK  is 512FS \r\n");
		break;

	case  RX_MCLK_768FS:
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]Get Audio Output MCLK  is 768FS \r\n");
		break;

	default:
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]Get Audio Output MCLK  is 128FS \r\n");
		break;
	}

	_hdmi_rx_aud.u1I2sChanValid = AudioCaps.AudSrcEnable;
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]I2S ch0%s Valid \r\n",
		(AudioCaps.AudSrcEnable & 0x01) ? TEXT("") : TEXT(" not"));
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]I2S ch1%s Valid \r\n",
		(AudioCaps.AudSrcEnable & 0x02) ? TEXT("") : TEXT(" not"));
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]I2S ch2%s Valid \r\n",
		(AudioCaps.AudSrcEnable & 0x04) ? TEXT("") : TEXT(" not"));
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]I2S ch3%s Valid \r\n",
		(AudioCaps.AudSrcEnable & 0x08) ? TEXT("") : TEXT(" not"));
	_hdmi_rx_aud.u1I2sCh0Sel = HDMIRX_I2S_L_R;
	_hdmi_rx_aud.u1I2sCh1Sel = HDMIRX_I2S_C_LFE;
	_hdmi_rx_aud.u1I2sCh2Sel = HDMIRX_I2S_LS_RS;
	_hdmi_rx_aud.u1I2sCh3Sel = HDMIRX_I2S_RLS_RRS;
}


void SetAudioMute(BOOL bMute)
{
	if (bMute) {
		/* Mute */
		/* vHalHDMIRxSetAudMuteCH((UINT8)B_MUTE_AUDIO); */
		HDMI_HalMuteAudio(); /*  mtk68528 */
		/* HDMIRxAud_Printf("Audio Mute\n"); */
	} else {
		/* Un-Mute */
		if ((_u1ACPTYPE != ACP_TYPE_DVD_AUDIO) && (_u1ACPTYPE != ACP_TYPE_SACD)) {
			/* vHalHDMIRxSetAudMuteCH((UINT8)~B_MUTE_AUDIO); */
			HDMI_HalUnMuteAudio(); /*  mtk68528 */
		}

		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]Audio Un-Mute \r\n");
	}
}

#define RX_APLL_UNSTABLE_CNT_REACHED (50)  /* 50 times, 20ms each time. */
void SetupAudio(void)
{
	static UINT32 s_u4APLLUnstableRstCnt;

	/* RETAILMSG(1,(TEXT("[HDMI RX AUD]SetupAudio \r\n"))); */
	getHDMIRxInputAudio(&AudioCaps);

	if (AudioCaps.AudioFlag & B_CAP_AUDIO_ON) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]SetupAudio, B_CAP_AUDIO_ON \r\n");

		/* Check if rx_mclk is stable , if no reset audio */
		if (!HalHDMIRxAPLLStatus()) {
			++s_u4APLLUnstableRstCnt;
			HDMI_LOG(HDMI_LOG_INFO,
				"[HDMI RX AUD]SetupAudio, RX MCLK is not stable.(%u) \r\n",
				(unsigned int)s_u4APLLUnstableRstCnt);

			if (s_u4APLLUnstableRstCnt >= RX_APLL_UNSTABLE_CNT_REACHED) {
				s_u4APLLUnstableRstCnt = 0;
				HalHdmiRxAudResetAudio();
			}
		} else {
			s_u4APLLUnstableRstCnt = 0;

			if (u4HDMIRxAudErrorGet()&HDMIRX_INT_STATUS_CHK) {
				/*  Reset Audio Fifo if error occurs */
				HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]SetupAudio, RX AFIFO Error , Reset AFIFO.\r\n");
				HalHdmiRxAudResetAfifo();
			} else  if (AudioCaps.SampleFreq == B_Fs_UNKNOW) {
				HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX AUD]Audio Sampling Rate is Unknow.\r\n");
			} else {
				if (AudioCaps.AudioFlag & B_CAP_HBR_AUDIO) {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]SetupAudio, B_CAP_HBR_AUDIO\r\n");
				} else if (AudioCaps.AudioFlag & B_CAP_DSD_AUDIO) {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]SetupAudio, B_CAP_DSD_AUDIO\r\n");
					} else { /*  if(AudioCaps.AudioFlag& B_CAP_LPCM)*/
					/* not only LPCM but all use audio sample packet need this fixing. */
						HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]SetupAudio, B_CAP_LPCM\r\n");

					if (HalHDMIRxMultiPCM()) {
							/*  if multi channel , then bypass I2S to Tx */
							HDMI_LOG(HDMI_LOG_DEBUG,
							"[HDMI RX AUD]Hdmi Rx%s Bypass I2S path to Tx(MultiChPCM)\r\n",
							_fgHDMIRxBypassMode ? TEXT("") : TEXT(" No"));
					} else {
							/*  if 2 channel , then bypass SPDIF to Tx */
							HDMI_LOG(HDMI_LOG_DEBUG,
							"[HDMI RX AUD]Hdmi Rx%s Bypass I2S path to Tx(2ChPCM)\r\n",
							_fgHDMIRxBypassMode ? TEXT("") : TEXT(" No"));
						}
					}

				SwitchAudioState(ASTATE_WaitForReady);
			}
		}
	} else {
		s_u4APLLUnstableRstCnt = 0;
		/* RETAILMSG(1,(TEXT("[HDMI RX AUD]Audio Off, mhl is pause... \r\n"))); */
	}
}


void getHDMIRxInputAudio(AUDIO_CAPS *pAudioCaps)
{
	BYTE uc;

	if (!pAudioCaps)
		return;

	uc = HalHDMIRxAudFsGet();
	pAudioCaps->SampleFreq = uc & M_Fs;
	/* uc = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT); */
	pAudioCaps->AudioFlag = ((HalHDMIRxAudioPkt() << B_CAP_AUDIO_ON_SFT) |
				(HalHDMIRxHDAudio() << B_CAP_HBR_AUDIO_SFT) |
				(HalHDMIRxDSDAudio() << B_CAP_DSD_AUDIO_SFT));

	if (pAudioCaps->AudioFlag & B_HBRAUDIO)
		pAudioCaps->SampleFreq = B_Fs_HBR;

	if (pAudioCaps->AudioFlag & B_DSDAUDIO)
		pAudioCaps->SampleFreq = B_Fs_44p1KHz;

	/* Check n@polling  0x1F070[17] & 0x1F078[24] & 0x1F078[25] 0 Twaudio T */
	if (pAudioCaps->AudioFlag != 0)
		pAudioCaps->AudioFlag |= B_CAP_AUDIO_ON;

	pAudioCaps->AudioFlag |= (HalHDMIRxMultiPCM() << B_MULTICH_SFT);

	if ((pAudioCaps->AudioFlag & B_LAYOUT) == B_MULTICH) {
		/* multi channel layout */
		pAudioCaps->AudSrcEnable = HalHDMIRxAudValidCHGet()&M_AUDIO_CH;
	} else {
		/* 2 channel layout */
		pAudioCaps->AudSrcEnable = B_AUDIO_SRC_VALID_0;
	}

	if ((pAudioCaps->AudioFlag  & (B_HBRAUDIO | B_DSDAUDIO)) == 0) {
		uc =  HalHDMIRxAudioCHSTAT0();

		if ((uc & B_AUD_NLPCM) == 0)
			pAudioCaps->AudioFlag |= B_CAP_LPCM;
		else
			pAudioCaps->AudioFlag |= B_SPDIF;
	}

	HDMI_HalGetAudioInfoFrameExt(&(pAudioCaps->AudInf));
	GetAudioChannelStatus(&(pAudioCaps->AudChStat), pAudioCaps->AudioFlag);
}



void AssignAudioTimerTimeout(UINT32 TimeOut)
{
	AudioCountingTimer = TimeOut;
}


UINT8 GetHDMIRxACPType(void)
{
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]u1GetHDMIRxACPType : ACP Type is 0x%x. \r\n", _u1ACPTYPE);
	return _u1ACPTYPE;
}
EXPORT_SYMBOL(GetHDMIRxACPType);

void vNotifyHDMIRxACPTypeChange(UINT8 u1ACPType)
{
	if (_fgHDMIRxAudTaskEnable) { /* &&(AState!=ASTATE_AudioOff)) 2010/10/29,ychung , always send ACP_TYPE change */
		if (u1ACPType == ACP_TYPE_GENERAL_AUDIO) {
			_u1ACPTYPE = ACP_TYPE_GENERAL_AUDIO;

			if (_fgACPMUTE) {
				SetAudioMute(OFF);
				_fgACPMUTE = FALSE;
				_fgSendACR = TRUE;
				/* vHdmiRxDrvSetAvd(HDMI_RX_SET_TX_SEND_ACR, &_fgSendACR); */
			}

			HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX AUD]vNotifyHDMIRxACPTypeChange : ACP Type is General Audio. \r\n");
		} else if (u1ACPType == ACP_TYPE_IEC60958) {
			_u1ACPTYPE = ACP_TYPE_IEC60958;

			if (_fgACPMUTE) {
				SetAudioMute(OFF);
				_fgACPMUTE = FALSE;
				_fgSendACR = TRUE;
				/* vHdmiRxDrvSetAvd(HDMI_RX_SET_TX_SEND_ACR, &_fgSendACR); */
			}

			HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX AUD]vNotifyHDMIRxACPTypeChange : ACP Type is IEC-89058. \r\n");
		} else if (u1ACPType == ACP_TYPE_DVD_AUDIO) {
#if HDMI_RX_IGNORE_ACP_PACKET
			_u1ACPTYPE = ACP_TYPE_GENERAL_AUDIO;

			if (_fgACPMUTE) {
				SetAudioMute(OFF);
				_fgACPMUTE = FALSE;
				_fgSendACR = TRUE;
				vHdmiRxDrvSetAvd(HDMI_RX_SET_TX_SEND_ACR, &_fgSendACR);
			}

#else
			_u1ACPTYPE = ACP_TYPE_DVD_AUDIO;

			if (!_fgACPMUTE) {
				SetAudioMute(ON);
				_fgACPMUTE = TRUE;
				_fgSendACR = FALSE;
				/* vHdmiRxDrvSetAvd(HDMI_RX_SET_TX_SEND_ACR, &_fgSendACR);  // Sony Stop ACR sending */
			}

#endif
			HDMI_LOG(HDMI_LOG_DEBUG,
			"[HDMI RX AUD]vNotifyHDMIRxACPTypeChange : ACP Type is DVD-Audio. \r\n");
		} else if (u1ACPType == ACP_TYPE_SACD) {
#if HDMI_RX_IGNORE_ACP_PACKET
			_u1ACPTYPE = ACP_TYPE_GENERAL_AUDIO;

			if (_fgACPMUTE) {
				SetAudioMute(OFF);
				_fgACPMUTE = FALSE;
				_fgSendACR = TRUE;
				/* vHdmiRxDrvSetAvd(HDMI_RX_SET_TX_SEND_ACR, &_fgSendACR); */
			}

#else
			_u1ACPTYPE = ACP_TYPE_SACD;

			if (!_fgACPMUTE) {
				SetAudioMute(ON);
				_fgACPMUTE = TRUE;
				_fgSendACR = FALSE;
				/* vHdmiRxDrvSetAvd(HDMI_RX_SET_TX_SEND_ACR, &_fgSendACR);  // Sony Stop ACR sending */
			}

#endif
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]vNotifyHDMIRxACPTypeChange : ACP Type is SACD. \r\n");
		} else {
			_u1ACPTYPE = ACP_TYPE_GENERAL_AUDIO;

			if (_fgACPMUTE) {
				SetAudioMute(OFF);
				_fgACPMUTE = FALSE;
				_fgSendACR = TRUE;
				/* vHdmiRxDrvSetAvd(HDMI_RX_SET_TX_SEND_ACR, &_fgSendACR);*/
				/* Sony Stop ACR and audio packet sending */
			}

			HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX AUD]vNotifyHDMIRxACPTypeChange : ACP Type is Reserved. \r\n");
		}

		vSendAUDINCmd(AUDIN_CHG_HDMIRX_INT | (HDMI_RX_ACPPKT_CHG << 8), AUDIN_CMD_PRI_HIGH);
	}
}

void SwitchAudioState(Audio_State_Type state)
{
	HDMI_RX_AUDIO_INT_TYPE eIRQSrcSend;

	AState = state;

	if (!_fgHDMIRxAudTaskEnable)
		return;

	/* RETAILMSG(1,(TEXT("[HDMI RX AUD]SwitchAudioState AState[%d] -> %s.  \r\n"), AState, AStateStr[AState])); */
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]SwitchAudioState AState[%d] \r\n", AState);

	switch (AState) {
	case ASTATE_AudioOff:
		/*         eIRQSrcSend = HDMI_RX_AUDIO_MUTE; */
		/*       vSendAUDINCmd(AUDIN_CHG_HDMIRX_INT|(eIRQSrcSend<<8), AUDIN_CMD_PRI_HIGH); */
		eIRQSrcSend = HDMI_RX_AUDIO_UNLOCK;/* kenny add it 2009/12/08 */

		vSendAUDINCmd(AUDIN_CHG_HDMIRX_INT | (eIRQSrcSend << 8), AUDIN_CMD_PRI_HIGH);
		HalHdmiAudRegReset();

		/* SetAudioMute(TRUE); */

		/* MuteAudioOutALL(); */

		break;

	case ASTATE_WaitForReady:
		AssignAudioTimerTimeout(AUDIO_READY_TIMEOUT);
		break;

	case ASTATE_AudioOn:
		/*         SetAudioMute(MuteByPKG); */
		SetAudioMute(OFF);


		/* AssignAudioTimerTimeout(AUDIO_CLEARERROR_TIMEOUT);*/
		/* set one second adjusting to reset ucAudioErrorCount. */
		if (MuteByPKG) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]AudioOn, but still in mute. \r\n");
			/* EnableMuteProcessTimer(); */
		} else {
			eIRQSrcSend = HDMI_RX_AUDIO_ON;
			vSendAUDINCmd(AUDIN_CHG_HDMIRX_INT | (eIRQSrcSend << 8), AUDIN_CMD_PRI_HIGH);

			getHDMIRxInputAudio(&AudioCaps);
			vUpdateHdmiRxAudio();

			/* RETAILMSG(1,(TEXT("[HDMI RX AUD]AudioOn, but no send stream_change. \r\n"))); */
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]AudioOn, send stream_change to enable audio. \r\n");
			eIRQSrcSend = HDMI_RX_AUDIO_BIT_STREAM_CHANGE;
			vSendAUDINCmd(AUDIN_CHG_HDMIRX_INT | (eIRQSrcSend << 8), AUDIN_CMD_PRI_HIGH);
		}

		break;

	default:
		break;
	}
}

UINT8 GetRxAudioState(void)
{
	return AState;
}


/* ------------------------hdmi rx audio main loop */
#define WAIT_SIGNAL_STABLE_INIT (0)
#define WAIT_AUDIO_PACKET_INIT  (5)
#define WAIT_VIDEO_STABLE_TIME  (10)
#define WAIT_AUDIO_STABLE_TIME  (5)

typedef enum {
	RX_AUD_WAIT_V_STABLE = 0,
	RX_AUD_WAIT_A_STABLE,
	RX_AUD_NORMAL_OUT
} HDMIRX_AUDIO_STATE;


UINT32 _u4HdmiRxWaitVCnt = WAIT_SIGNAL_STABLE_INIT;
UINT32 _u4HdmiRxWaitACnt = WAIT_SIGNAL_STABLE_INIT;
UINT32 _u4HdmiRxAudState = RX_AUD_WAIT_V_STABLE;


void vHdmiRxAudLoop(void)
{

	switch (_u4HdmiRxAudState) {
	case RX_AUD_WAIT_V_STABLE:
		if (HdmiIsHVStable() && fgNotifySignal) {
			if (_u4HdmiRxWaitVCnt < WAIT_VIDEO_STABLE_TIME)
				_u4HdmiRxWaitVCnt++;
			else {
				if (HalChkAudPktReady()) {
					HalHdmiRxAudInitSetting();
					SwitchAudioState(ASTATE_RequestAudio);
					HDMI_LOG(HDMI_LOG_INFO, "[HDMI AUD LOOP]video signal stable. \r\n");
					_u4HdmiRxAudState = RX_AUD_WAIT_A_STABLE;
					_u4HdmiRxWaitACnt = WAIT_SIGNAL_STABLE_INIT;
				} else
					_u4HdmiRxWaitACnt = WAIT_AUDIO_PACKET_INIT;
			}
		} else
			_u4HdmiRxWaitVCnt = WAIT_SIGNAL_STABLE_INIT;

		break;

	case RX_AUD_WAIT_A_STABLE:
		if (HdmiIsHVStable()) {
			if (_u4HdmiRxWaitACnt < WAIT_AUDIO_STABLE_TIME)
				_u4HdmiRxWaitACnt++;
			else {
				if (HalChkAudPktReady()) {
					if (HalHdmiRxAudResetAudio()) {
						_u4HdmiRxAudState = RX_AUD_NORMAL_OUT;
						HDMI_LOG(HDMI_LOG_INFO,
							"[HDMI AUD LOOP]reset afifo, audio signal stable. \r\n");
					} else {
						_u4HdmiRxWaitACnt = WAIT_SIGNAL_STABLE_INIT;
						HDMI_LOG(HDMI_LOG_INFO,
							"[HDMI AUD LOOP]reset afifo, audio signal still un-stable. \r\n");
					}
				} else {
					_u4HdmiRxWaitVCnt = WAIT_SIGNAL_STABLE_INIT;
					_u4HdmiRxAudState = RX_AUD_WAIT_V_STABLE;
				}
			}
		} else {
			HDMI_LOG(HDMI_LOG_INFO, "[HDMI AUD LOOP]video un-stable, change to check video. \r\n");
			SwitchAudioState(ASTATE_AudioOff);
			_u4HdmiRxWaitVCnt = WAIT_SIGNAL_STABLE_INIT;
			_u4HdmiRxAudState = RX_AUD_WAIT_V_STABLE;
		}

		break;

	case RX_AUD_NORMAL_OUT:
		if (HdmiIsHVStable() && _fgHDMIRxAudTaskEnable) {
			vHDMIRxAudMainTask();  /* 20ms loop */
		} else {
			HDMI_LOG(HDMI_LOG_INFO,
			"[HDMI AUD LOOP]video un-stable,stop audio and change to check video.\r\n");
			SwitchAudioState(ASTATE_AudioOff);
			_u4HdmiRxWaitVCnt = WAIT_SIGNAL_STABLE_INIT;
			_u4HdmiRxAudState = RX_AUD_WAIT_V_STABLE;

			if (!_fgHDMIRxAudTaskEnable)
				HDMI_LOG(HDMI_LOG_INFO, "[HDMI AUD LOOP]Quit HDMI. \r\n");
		}

		break;

	default:
		break;
	}
}

/* ------------------------*/


typedef enum {
	GET_HDMIRX_AUDIO_INFO = 0,
	GET_HDMIRX_AUDIO_INFOFRAME,
	GET_HDMIRX_CHANNEL_STATUS,
	GET_HDMIRX_ACPTYPE
} HDMIRX_AUDIO_GETINFO;

/************************************************************************************************
 Function: AudDrvGetMhlInfo(void* pInBuffer, void* pOutBuffer, DWORD* pOutSize)
 Description:  Audio driver get Hdmi Audio information
 input para:  pInBuffer: information type,
 output para: pOutBuffer: audio information, pOutSize: information size
************************************************************************************************/
void AudDrvGetMhlInfo(void *pInBuffer, void *pOutBuffer, DWORD *pOutSize)
{
	UINT32 *pVal = (UINT32 *)pInBuffer;

	if (pOutBuffer != NULL) {
		if (GET_HDMIRX_AUDIO_INFO == *pVal) {
			HDMI_RX_IN_AUDIO_INFO_T *prAudIf = (HDMI_RX_IN_AUDIO_INFO_T *)pOutBuffer;

			vGetHdmiRxAudioParameter(prAudIf);
			*pOutSize = sizeof(HDMI_RX_IN_AUDIO_INFO_T);
		} else if (GET_HDMIRX_AUDIO_INFOFRAME == *pVal) {
			Audio_InfoFrame *pAudioInfoFrame = (Audio_InfoFrame *)pOutBuffer;

			ExportAudioInfoFrame(pAudioInfoFrame);
			*pOutSize = sizeof(Audio_InfoFrame);
		} else if (GET_HDMIRX_CHANNEL_STATUS == *pVal) {
			RX_REG_AUDIO_CHSTS *RegChannelstatus = (RX_REG_AUDIO_CHSTS *)pOutBuffer;

			ExportAudioChannelStatus(RegChannelstatus);
			*pOutSize = sizeof(RX_REG_AUDIO_CHSTS);

		} else if (GET_HDMIRX_ACPTYPE == *pVal) {
			UINT8 *u1AcpType = (UINT8 *)pOutBuffer;
			*u1AcpType = GetHDMIRxACPType();
			*pOutSize = sizeof(UINT8);

		}
	}
}



/************************************************************************************************
 Function: AudDrvSetMhlInfo(void* pInBuffer)
 Description:  Audio driver Set Hdmi information
 input para:  pInBuffer: information type,
 output para:
************************************************************************************************/
void AudDrvSetMhlInfo(void *pInBuffer)
{
	UINT32 *pVal = (UINT32 *)pInBuffer;

	if (0 != *pVal)
		vEnableHdmiRxAudTask(TRUE);
	else
		vEnableHdmiRxAudTask(FALSE);
}

#ifndef __linux__
BOOL MhlSendAudInfo(UINT32 u4Msg)
{
	HANDLE  hAudDev = NULL;
	bool ret;

	hAudDev = filp_open(L"ADE1:", O_RDWR, 0);
	/*hAudDev = CreateFile(L"ADE1:", GENERIC_READ, 0, NULL, OPEN_EXISTING,
			     FILE_ATTRIBUTE_NORMAL, NULL);*/
	ret = IS_ERR(hAudDev);
	if (hAudDev) {
		if (!DeviceIoControl(hAudDev, IOCTL_AUDMHL_MHL_SEND_INFO,
				     &u4Msg, sizeof(UINT32), NULL, 0, NULL, NULL)) {
			/* CloseHandle(hAudDev); */
			if (!ret)
				filp_close((struct file *)hAudDev, NULL);
			hAudDev = NULL;
			return FALSE;
		}
	}
	if (!ret)
		filp_close((struct file *)hAudDev, NULL);
	/* CloseHandle(hAudDev); */
	hAudDev = NULL;
	return TRUE;
}
#endif
