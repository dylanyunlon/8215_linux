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

#include "bt_osl.h"
#include "bt_perf_stat.h"
#include "bt_lib.h"
#include "bt_speech.h"
#include "aud_pcm_dbg.h"

#include "pcm_debug.h"
#define LOG_TAG "bt_arm1_sph"

SPH_ENH_08K_ctrl_struct Sph_Enh_ctrl;

static Word16 ABF_cal_data[96];
static Word16 aec_com_rx[22];
static Word16 aec_com_tx[22];

static SPH_ENH_ctrl_struct g_Sph_Enh_ctrl_16k;


static u32 _u4HwSemaphore;
static u32 _u4State;

static BT_SHARE_MEM_T	 *g_prShareMem;

static u32 _u4BTDataReq;
static SPEECH_FRAME_T *_prFrame;

static s16 *_pi2SMDL;
static s16 *_pi2SMUL;
static s16 *_pi2SMUL2;
static s16 *_pi2SMDLDelay;


#define INCREASE_READ_IDX  (g_prShareMem->u4ReadIdx  = (g_prShareMem->u4ReadIdx  + 1) % (g_prShareMem->u4MaxFrame << 1))
#define INCREASE_WRITE_IDX (g_prShareMem->u4WriteIdx = (g_prShareMem->u4WriteIdx + 1) % (g_prShareMem->u4MaxFrame << 1))
#define IS_FRAME_FOR_AEC   (g_prShareMem->u4ReadIdx != g_prShareMem->u4WriteIdx)
#define NUM_FRAME_FOR_AEC  ((g_prShareMem->u4WriteIdx  >= g_prShareMem->u4ReadIdx) ? \
							(g_prShareMem->u4WriteIdx - g_prShareMem->u4ReadIdx) : \
							(g_prShareMem->u4WriteIdx + (g_prShareMem->u4MaxFrame << 1) - \
							g_prShareMem->u4ReadIdx))
#define IS_FRAME_FOR_WRITE (NUM_FRAME_FOR_AEC < g_prShareMem->u4MaxFrame)


struct BT_DELAYED_MSG {
	u32 u4MsgId;
	u32 u4P1;
	u32 u4P2;
	u32 u4P3;

	bool fgHasDelayed;
} _rDelayedMsg = {0, 0, 0, 0, false};

static u16 _ai2BufDL[SPEECH_FRAME_SAMPLES * 2];
static u16 _ai2BufUL[SPEECH_FRAME_SAMPLES * 2];
static u16 _ai2BufUL2[SPEECH_FRAME_SAMPLES * 2];
static u16 _ai2BufDLDelay[SPEECH_FRAME_SAMPLES * 2];


#ifdef BSP_ARM2
static u32 _u4DbgMiniSecond;
#endif


/*==========================================
  #define CodeSight_Speech_AEC
   ==========================================*/
static u32 AECInit(void)
{
	s32  i, iAECMemSz = 0;
	s32 *piAECMemPtr = NULL;
	u8 *pbSignSAdr = NULL;

	iAECMemSz = (s32)ENH_08K_API_Get_Memory(&Sph_Enh_ctrl);
	piAECMemPtr = (s32 *)BT_Malloc((u32)iAECMemSz);
	if (NULL == piAECMemPtr) {
		PCM_ERROR(LOG_TAG, "AECInit: malloc piAECMemPtr error!\r\n");
		return AEC_RESULT_FAILED;
	}
	PCM_DEBUG(LOG_TAG, "AECInit: SAdr(0x%x) size(%d)\r\n",
		(u32)piAECMemPtr, iAECMemSz);

	pbSignSAdr = (u8 *)piAECMemPtr;
	for (i = 0; i < iAECMemSz; i++) {
		*pbSignSAdr = 0;
		pbSignSAdr++;
	}

	ENH_08K_API_Alloc(&Sph_Enh_ctrl, (Word32 *)piAECMemPtr);

	ENH_08K_API_Init_AEC(&Sph_Enh_ctrl, &aec_com_rx[0], &aec_com_tx[0]);
	ENH_08K_API_Init_ABF(&Sph_Enh_ctrl, ABF_cal_data);
	ENH_08K_API_AGC_Init((Word32)14);
	ENH_08K_API_Init_PLC();

	PCM_DEBUG(LOG_TAG, "AECInit: successs!\r\n");

	return AEC_RESULT_SUCESS;
}

static u32 AECULProcess(s16 *pi2UL_sp, s16 *pi2DL_sp, s16 *pi2UL2_sp)
{
	ENH_08K_API_AGC_2(pi2UL_sp, pi2UL2_sp);
	ENH_08K_API_Run_Aec_UL(&Sph_Enh_ctrl, pi2UL_sp, pi2DL_sp, pi2UL2_sp);/* run UL  AEC + ABF */

	return AEC_RESULT_SUCESS;
}

static u32 AECDLProcess(s16 *pi2DL_sp)
{
	ENH_08K_API_Run_Aec_DL(&Sph_Enh_ctrl, pi2DL_sp);

	return AEC_RESULT_SUCESS;
}

static u32 AECDeinit(void)
{
	ENH_08K_API_Free_AEC();

	return AEC_RESULT_SUCESS;
}


/*==========================================
  #define CodeSight_Speech_NDC
  ==========================================*/
static u32 NDCInit(void)
{
	NDC_08K_Com_Init(Sph_Enh_ctrl.enhance_pars);
	NDC_08K_UL_Init();
	NDC_08K_DL_Init();

	return AEC_RESULT_SUCESS;
}

static u32 NDCULProcess(s16 *pwLinkBuffer)
{
	NDC_08K_UL_MAIN(pwLinkBuffer);

	return AEC_RESULT_SUCESS;
}

static u32 NDCDLProcess(s16 *pwLinkBuffer)
{
	NDC_08K_DL_MAIN(pwLinkBuffer);

	return AEC_RESULT_SUCESS;
}

static u32 NDCDeinit(void)
{
	return AEC_RESULT_SUCESS;
}


/*==========================================
  #define CodeSight_Speech_Process_Srv
  ==========================================*/
static bool InitFramePointers(void)
{
	bool fgRet = false;

	TAKE_BT_HW_SEMAPHORE();
	if (IS_FRAME_FOR_AEC) {
		u32 u4Idx = g_prShareMem->u4ReadIdx % g_prShareMem->u4MaxFrame;

		_prFrame = g_prShareMem->rFrame + u4Idx;

		_pi2SMUL  = _prFrame->ULBuf1;
		_pi2SMDL  = _prFrame->DLBuf;
		_pi2SMUL2 = _prFrame->ULBuf2;
		_pi2SMDLDelay = _prFrame->DLDelayBuf;

		fgRet = true;
	}
	RELEASE_BT_HW_SEMAPHORE();

	return fgRet;
}

static bool UpdateReadIndex(void)
{
	u32 u4ReadIdx;

	TAKE_BT_HW_SEMAPHORE();
	u4ReadIdx = g_prShareMem->u4ReadIdx;
	INCREASE_READ_IDX;
	RELEASE_BT_HW_SEMAPHORE();
	AECSendMessage((u32)BT_FRAME_COMPLETED, u4ReadIdx, (u32)0, (u32)0);

	return true;
}

static bool SpeechDLProcess(void)
{
	s16 *pi2DL = _ai2BufDL;

	BTMemCopy(pi2DL, _pi2SMDL, g_prShareMem->u4FrameByte);

	TimeStatEnter(STAT_IDX_DL);

	if (_prFrame->u4Opt & FRAME_OPT_PLC) {
		TimeStatEnter(STAT_IDX_DL_PLC);
		ENH_08K_API_PLC(pi2DL);
		TimeStatLeave(STAT_IDX_DL_PLC);
	}

	if (_prFrame->u4Opt & FRAME_OPT_NDC) {
		TimeStatEnter(STAT_IDX_DL_NDC);
		NDCDLProcess(pi2DL);
		TimeStatLeave(STAT_IDX_DL_NDC);
	}

	/* Copy data after NDC back to UL Buffer1*/
	if (_prFrame->u4Opt & DATA_REQ_POST_NDC) {
		BTMemCopy(_pi2SMDLDelay, pi2DL, g_prShareMem->u4FrameByte);
	}

	if (_prFrame->u4Opt & FRAME_OPT_AEC) {
		TimeStatEnter(STAT_IDX_DL_AEC);
		AECDLProcess(pi2DL);
		TimeStatLeave(STAT_IDX_DL_AEC);
	}

	TimeStatLeave(STAT_IDX_DL);

	/* Copy final data back to DL buffer.*/
	BTMemCopy(_pi2SMDL, pi2DL, g_prShareMem->u4FrameByte);

	UpdateReadIndex();

	return true;
}

static bool SpeechULProcess(void)
{
	s16 *pi2DL  = (s16 *)_ai2BufDLDelay;
	s16 *pi2UL  = (s16 *)_ai2BufUL;
	s16 *pi2UL2 = (s16 *)_ai2BufUL2;

	BTMemCopy(pi2UL,  _pi2SMUL,  g_prShareMem->u4FrameByte);
	BTMemCopy(pi2DL,  _pi2SMDLDelay,  g_prShareMem->u4FrameByte);
	BTMemCopy(pi2UL2, _pi2SMUL2, g_prShareMem->u4FrameByte);

	TimeStatEnter(STAT_IDX_UL);

	if (_prFrame->u4Opt & FRAME_OPT_AEC) {
		TimeStatEnter(STAT_IDX_UL_AEC);
		AECULProcess(pi2UL, pi2DL, pi2UL2);
		TimeStatLeave(STAT_IDX_UL_AEC);
	}

	/* Copy data after AEC to UL Buffer*/
	if (_prFrame->u4Opt & DATA_REQ_POST_AEC) {
		BTMemCopy(_pi2SMUL2, pi2UL, g_prShareMem->u4FrameByte);
	}

	if (_prFrame->u4Opt & FRAME_OPT_NDC) {
		TimeStatEnter(STAT_IDX_UL_NDC);
		NDCULProcess(pi2UL);
		TimeStatLeave(STAT_IDX_UL_NDC);
	}

	TimeStatLeave(STAT_IDX_UL);

	/* Copy final data back to UL Buffer*/
	BTMemCopy(_pi2SMUL, pi2UL, g_prShareMem->u4FrameByte);

	UpdateReadIndex();

	return true;
}

/*Process 16k sample rate data*/
static bool SpeechProcess(u32 u4Opt)
{
	TimeStatEnter(STAT_IDX_UL);

	BTMemSet(g_Sph_Enh_ctrl_16k.PCM_buffer, 0, sizeof(g_Sph_Enh_ctrl_16k.PCM_buffer));
	if (_prFrame->u4Opt & FRAME_OPT_DL)
		BTMemCopy(&g_Sph_Enh_ctrl_16k.PCM_buffer[640],
			_pi2SMDL, g_prShareMem->u4FrameByte);/* DL_In */
	else {
		BTMemCopy(&g_Sph_Enh_ctrl_16k.PCM_buffer[0],
			_pi2SMUL, g_prShareMem->u4FrameByte);/* UL_Mic1 */
		if (_prFrame->u4Opt & FRAME_OPT_DMNR)
			BTMemCopy(&g_Sph_Enh_ctrl_16k.PCM_buffer[320],
				_pi2SMUL2, g_prShareMem->u4FrameByte);/* UL_Mic2 */

		BTMemCopy(&g_Sph_Enh_ctrl_16k.PCM_buffer[640],
			_pi2SMDL, g_prShareMem->u4FrameByte);/* DL_In */
		BTMemCopy(&g_Sph_Enh_ctrl_16k.PCM_buffer[960],
			_pi2SMDLDelay, g_prShareMem->u4FrameByte);/* DL_In_Delay */
	}

	ENH_API_Process(&g_Sph_Enh_ctrl_16k);

	TimeStatLeave(STAT_IDX_UL);

	/* Copy data after AEC to UL2 Buffer */
	if (_prFrame->u4Opt & DATA_REQ_POST_AEC) {
		BTMemCopy(_pi2SMUL2, &g_Sph_Enh_ctrl_16k.EPL_buffer[1280], g_prShareMem->u4FrameByte);
	}

	/* Copy final data to UL Buffer */
	BTMemCopy(_pi2SMUL, &g_Sph_Enh_ctrl_16k.EPL_buffer[1600], g_prShareMem->u4FrameByte);

	/* Copy DL data after NDC to  DL delay Buffer */
	if (_prFrame->u4Opt & DATA_REQ_POST_NDC) {
		BTMemCopy(_pi2SMDLDelay, &g_Sph_Enh_ctrl_16k.EPL_buffer[2240], g_prShareMem->u4FrameByte);
	}

	/* Copy final data to DL Buffer */
	BTMemCopy(_pi2SMDL, &g_Sph_Enh_ctrl_16k.EPL_buffer[2560], g_prShareMem->u4FrameByte);

	UpdateReadIndex();

	return true;
}

static bool SpeechFrameProcess(void)
{
	u32 u4MaxProFrame = 1U;

	TimeStatEnter(STAT_IDX_AEC_NDC);
	while (u4MaxProFrame && (InitFramePointers())) {
		if (8000 == g_prShareMem->u4SampleRate) {
			if (_prFrame->u4Opt & FRAME_OPT_DL) {
				SpeechDLProcess();
			} else {
				SpeechULProcess();
			}
		} else {
			SpeechProcess(_prFrame->u4Opt);
		}

		u4MaxProFrame--;
	}
	TimeStatLeave(STAT_IDX_AEC_NDC);

	return (bool)(u4MaxProFrame < 10U);
}


/*==========================================
  #define CodeSight_Speech_EventSrv
  ==========================================*/
static u32 BTSetHWResource(u32 u4PhyAddr, u32 u4Size, u32 u4HwSema)
{
	u32 u4Ret = BT_SUCCESS;

	if ((BT_STATE_SCO == _u4State) || (BT_STATE_UNINIT == _u4State)) {
		u4Ret = BT_FAILURE;        /* Can't handle BT_SET_HW_RESOURCE in these states */
	} else if ((!u4PhyAddr) && (!u4Size) && (!u4HwSema)) {
		u4Ret = BT_FAILURE;
	} else {
		g_prShareMem = (BT_SHARE_MEM_T *)ARM1PHY2ARM2UCV(u4PhyAddr);
		_u4HwSemaphore = u4HwSema;
		if (BT_STATE_INIT == _u4State) {
			_u4State = BT_STATE_IDLE;
			g_prShareMem->u4State = _u4State;
			PCM_DEBUG(LOG_TAG, "BTSetHWResource: Enter IDLE State. g_prShareMem(0x%x)\r\n",
				(u32)g_prShareMem);
		}
		AECSendMessage((u32)BT_MSG_COMPLETED, (u32)BT_SET_HW_RESOURCE, _u4State, (u32)0);
		u4Ret = BT_SUCCESS;
	}

	return u4Ret;
}

static u32 BTSetParameter(u32 u4Which)
{
	u32 u4Ret = BT_SUCCESS;

	if ((BT_STATE_UNINIT == _u4State) || (BT_STATE_SCO == _u4State)) {
		u4Ret = BT_FAILURE;        /* Don't handle this message in these state.*/
	} else {
		TAKE_BT_HW_SEMAPHORE();
		if (BT_SPH_PARAMETER & u4Which) {
			SPH_PARAM_T *prSphParam = NULL;

			prSphParam = &g_prShareMem->rSphParam;
		}
		if (BT_DMNR_PARAMETER & u4Which) {
			DMNR_PARAM_T *prDmnrParam = NULL;

			prDmnrParam = &g_prShareMem->rDmnrParam;
		}
		RELEASE_BT_HW_SEMAPHORE();
	}

	return u4Ret;
}

static u32 BTWriteFrame(u32 u4FrameIdx)
{
	return BT_SUCCESS;
}

static u32 BTEnterSCO(bool fgEnter, u32 u4Request)
{
	u32	u4Ret = BT_SUCCESS;
	s32		iMemSz = 0;
	s32		*piMemPtr = NULL;

	if ((8000 != g_prShareMem->u4SampleRate) && (16000 != g_prShareMem->u4SampleRate)) {
		PCM_ERROR(LOG_TAG, "BTEnterSCO: Sample rate(%d) error!\r\n",
			(s32)g_prShareMem->u4SampleRate);
		u4Ret = BT_FAILURE;
		goto EXIT;
	}

	if ((fgEnter && (BT_STATE_IDLE != _u4State)) || ((!fgEnter) && (BT_STATE_SCO != _u4State))) {
		u4Ret = BT_FAILURE;
		goto EXIT;
	}

	if (fgEnter) {
		u32 i, u4Return;

		BT_MemoryInit();
		_u4BTDataReq = u4Request;

		if (8000 == g_prShareMem->u4SampleRate) {
			if (u4Request & FRAME_OPT_DMNR) {
				BTMemCopy(&Sph_Enh_ctrl, &g_prShareMem->rSphParam2, sizeof(SPH_ENH_08K_ctrl_struct));
			} else {
				BTMemCopy(&Sph_Enh_ctrl, &g_prShareMem->rSphParam, sizeof(SPH_ENH_08K_ctrl_struct));
			}

			BTMemCopy(ABF_cal_data, g_prShareMem->rDmnrParam.dmnrParm, sizeof(DMNR_PARAM_T));

			PCM_DEBUG(LOG_TAG, "BTEnterSCO: AEC Parameters Request(0x%x)\r\n",
				u4Request);
			for (i = 0; i < AEC_NDC_PARAM_NUM; i++) {
				if (0 == (i % 10U)) {
					PCM_DEBUG(LOG_TAG, "\r\n");
				}
				PCM_DEBUG(LOG_TAG, "%d, ", (s32)Sph_Enh_ctrl.enhance_pars[i]);
			}
			PCM_DEBUG(LOG_TAG, "\r\n");

			if (u4Request & FRAME_OPT_AEC) {
				BTMemCopy(aec_com_rx, &g_prShareMem->rAecRxParam, sizeof(AEC_COM_RX_struct));
				BTMemCopy(aec_com_tx, &g_prShareMem->rAecTxParam, sizeof(AEC_COM_TX_struct));
				u4Return = AECInit();
				PCM_DEBUG(LOG_TAG, "BTEnterSCO: AEC Init. Return(0x%x)\r\n", u4Return);
			}

			if (u4Request & FRAME_OPT_NDC) {
				u4Return = NDCInit();
				PCM_DEBUG(LOG_TAG, "BTEnterSCO: NDCInit. Return(0x%x)\r\n", u4Return);
			}
		} else {
			BTMemSet(&g_Sph_Enh_ctrl_16k, 0, sizeof(g_Sph_Enh_ctrl_16k));
			g_Sph_Enh_ctrl_16k.App_table = WB_VOIP;
			g_Sph_Enh_ctrl_16k.Fea_Cfg_table = 511;
			g_Sph_Enh_ctrl_16k.MIC_DG = 14;
			g_Sph_Enh_ctrl_16k.sample_rate = 16000;
			g_Sph_Enh_ctrl_16k.frame_rate = 20;
			g_Sph_Enh_ctrl_16k.MMI_ctrl = 0xFFFFFFBF;

			BTMemCopy(g_Sph_Enh_ctrl_16k.enhance_pars, g_prShareMem->Sph_Enh_ctrl_16k,
					sizeof(g_Sph_Enh_ctrl_16k.enhance_pars));

			PCM_DEBUG(LOG_TAG, "BTEnterSCO: AEC 16k Param =\r\n");
			for (i = 0; i < AEC_NDC_PARAM_NUM; i++) {
				if (0 == (i % 10U)) {
					PCM_DEBUG(LOG_TAG, "\r\n");
				}
				PCM_DEBUG(LOG_TAG, "%d, ", (s32)Sph_Enh_ctrl.enhance_pars[i]);
			}
			PCM_DEBUG(LOG_TAG, "\r\n");

			if (u4Request & FRAME_OPT_DMNR)
				BTMemCopy(g_Sph_Enh_ctrl_16k.DMNR_cal_data, g_prShareMem->DMNR_cal_data_16k,
					sizeof(g_Sph_Enh_ctrl_16k.DMNR_cal_data));
			else {
				BTMemSet(g_Sph_Enh_ctrl_16k.DMNR_cal_data, 0, sizeof(g_Sph_Enh_ctrl_16k.DMNR_cal_data));
			}

			PCM_DEBUG(LOG_TAG, "BTEnterSCO: DMNR_cal_data 16k Param =\r\n");
			for (i = 0; i < DMNR_PARAM_NUM_16K; i++) {
				if (0 == (i % 10U)) {
					PCM_DEBUG(LOG_TAG, "\r\n");
				}
				PCM_DEBUG(LOG_TAG, "%d, ", (s32)g_Sph_Enh_ctrl_16k.DMNR_cal_data[i]);
			}
			PCM_DEBUG(LOG_TAG, "\r\n");

			BTMemCopy(g_Sph_Enh_ctrl_16k.Compen_filter, g_prShareMem->Compen_filter_16k,
					sizeof(g_Sph_Enh_ctrl_16k.Compen_filter));
			PCM_DEBUG(LOG_TAG, "BTEnterSCO: COMPEN_FILTER 16k Param =\r\n");
			for (i = 0; i < COMPEN_FILTER_16K; i++) {
				if (0 == (i % 10U)) {
					PCM_DEBUG(LOG_TAG, "\r\n");
				}
				PCM_DEBUG(LOG_TAG, "%d, ", (s32)g_Sph_Enh_ctrl_16k.Compen_filter[i]);
			}
			PCM_DEBUG(LOG_TAG, "\r\n");

			iMemSz = ENH_API_Get_Memory(&g_Sph_Enh_ctrl_16k);
			piMemPtr = (s32 *)BT_Malloc(iMemSz);
			if (NULL == piMemPtr) {
				PCM_ERROR(LOG_TAG, "BTEnterSCO: malloc piMemPtr error!\r\n");
				return BT_FAILURE;
			}
			PCM_DEBUG(LOG_TAG, "BTEnterSCO: SAdr(0x%x) size(%d)\r\n",
				(u32)piMemPtr, (s32)iMemSz);
			BTMemSet(piMemPtr, 0, iMemSz);
			ENH_API_Alloc(&g_Sph_Enh_ctrl_16k, (Word32 *)piMemPtr);
			ENH_API_Rst(&g_Sph_Enh_ctrl_16k);
		}

		TimeStatInit();
		PCM_DEBUG(LOG_TAG, "BTEnterSCO: g_prShareMem(0x%x) MaxFrame(%d)\r\n",
			(u32)g_prShareMem, (s32)g_prShareMem->u4MaxFrame);
		PCM_DEBUG(LOG_TAG, "BTEnterSCO: ReadIdx(%d) WriteIdx(%d)\r\n",
			(s32)g_prShareMem->u4ReadIdx, (s32)g_prShareMem->u4WriteIdx);
		_u4State = BT_STATE_SCO;
		PCM_DEBUG(LOG_TAG, "BTEnterSCO: Change state from IDLE to SCO.\r\n");
	} else {
		if (8000 == g_prShareMem->u4SampleRate) {
			if (_u4BTDataReq & FRAME_OPT_AEC) {
				AECDeinit();
			}
			if (_u4BTDataReq & FRAME_OPT_NDC) {
				NDCDeinit();
			}
		} else {
			ENH_API_Free(&g_Sph_Enh_ctrl_16k);
		}
		TimeStatUnInit();
		_u4BTDataReq = 0;
		_u4State = BT_STATE_IDLE;
		BT_MemoryUninit();
		PCM_DEBUG(LOG_TAG, "BTEnterSCO: Change state from SCO to IDLE.\r\n");
	}

	g_prShareMem->u4State = _u4State;
	AECSendMessage(BT_STATE_CHANGED, _u4State, 0, 0);

EXIT:
	return u4Ret;
}

static u32 BTHandleMsg(u32 u4MsgID, u32 u4P1, u32 u4P2, u32 u4P3)
{
	u32 u4Ret = BT_SUCCESS;

	switch (u4MsgID & 0xFFFFU) {
	case BT_SET_HW_RESOURCE:
		u4Ret = BTSetHWResource(u4P1, u4P2, u4P3);
		break;

	case BT_SET_PARAMETER:
		u4Ret = BTSetParameter(u4P1);
		break;

	case BT_SCO_AUDIO_CONTROL:
		u4Ret = BTEnterSCO(u4P1, u4P2);
		break;

	case BT_WRITE_FRAME:
		u4Ret = BTWriteFrame(u4P1);
		break;

	default:
		break;
	}

	return u4Ret;
}


/*==========================================
  #define CodeSight_Speech_Interface
  ==========================================*/
u32 SpeechInit(void)
{
	_u4State = BT_STATE_INIT;

	return 0;
}

u32 SpeechStateMachine(void)
{
	u32 u4Ret = TASK_IDLE;

#ifdef BSP_ARM2
	u32 u4Temp = GetARM2TickCount();

	if (((u4Temp - _u4DbgMiniSecond) > 30000) && (BT_STATE_SCO ==  _u4State)) {
		PCM_DEBUG(LOG_TAG, "SpeechStateMachine: (%d ms)\r\n"), (s32)u4Temp);
		_u4DbgMiniSecond = u4Temp;
	}
#endif

	if (_rDelayedMsg.fgHasDelayed) {
		BTHandleMsg(_rDelayedMsg.u4MsgId, _rDelayedMsg.u4P1, _rDelayedMsg.u4P2, _rDelayedMsg.u4P3);
		_rDelayedMsg.fgHasDelayed = false;
		u4Ret = TASK_BUSY;
	}

	switch (_u4State) {
	case BT_STATE_SCO:
		if (IS_FRAME_FOR_AEC) {
			SpeechFrameProcess();
		}
		u4Ret = TASK_BUSY;
		break;

	case BT_STATE_IDLE:
	case BT_STATE_INIT:
	case BT_STATE_UNINIT:
	default:
		break;
	}

	return u4Ret;
}

u32 SpeechCB(u32 u4MsgID, u32 u4P1, u32 u4P2, u32 u4P3)
{
	u32 u4Ret = BT_SUCCESS;

	if (((u32)BT_SCO_AUDIO_CONTROL) == (u4MsgID & ((u32)0xFFFF))) {
		_rDelayedMsg.u4MsgId = u4MsgID;
		_rDelayedMsg.u4P1 = u4P1;
		_rDelayedMsg.u4P2 = u4P2;
		_rDelayedMsg.u4P3 = u4P3;
		_rDelayedMsg.fgHasDelayed = true;
	} else {
		u4Ret = BTHandleMsg(u4MsgID, u4P1, u4P2, u4P3);
	}

	return u4Ret;
}

