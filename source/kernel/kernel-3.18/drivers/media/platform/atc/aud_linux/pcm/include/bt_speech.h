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


#ifndef AUDIO_SPEECH_H
#define AUDIO_SPEECH_H

#include "bt_lib.h"

/* event message */
#define BT_SET_HW_RESOURCE		0x0001U
#define BT_SET_PARAMETER		0x0002U
#define BT_SCO_AUDIO_CONTROL	0x0003U
#define BT_WRITE_FRAME			0x0004U

#define BT_MSG_COMPLETED		0x0100U
#define BT_FRAME_COMPLETED		0x0200U
#define BT_STATE_CHANGED		0x0300U
#define BT_ERROR				0x0400U

/* state */
#define BT_STATE_UNINIT			0x0000U
#define BT_STATE_INIT			0x0001U
#define BT_STATE_IDLE			0x0002U
#define BT_STATE_SCO			0x0003U

/* Parameter for message BT_SET_PARAMETER */
#define BT_SPH_PARAMETER		0x0001U
#define BT_DMNR_PARAMETER		0x0002U
#define BT_ALL_PARAMETER		0x0007U

/* Parameter for message BT_STATE_SCO */
#define BT_SUCCESS				0x00000000U
#define BT_FAILURE				0xFFFFFFFFU

#define SPEECH_FRAME_COUNT		10U
#define SPEECH_FRAME_SAMPLES	160U
#define SPEECH_FRAME_BYTES		320U

#define DATA_REQ_POST_AEC		0x0100U
#define DATA_REQ_POST_ABF		0x0200U
#define DATA_REQ_POST_NDC		0x0400U
#define DATA_REQ_ALL			0x0700U

#define FRAME_OPT_DL			0x0001U

#define FRAME_OPT_AEC			0x0002U
#define FRAME_OPT_NDC			0x0004U
#define FRAME_OPT_DMNR			0x0008U
#define FRAME_OPT_PLC			0x0010U
#define OPT_OUTPUT_LOG			0x0F00U

#define BT_SCO_REQ_ALL		(FRAME_OPT_AEC|FRAME_OPT_NDC|FRAME_OPT_DMNR|FRAME_OPT_PLC)
#define BT_SCO_REQ_AECNDC	(FRAME_OPT_AEC|FRAME_OPT_NDC)


typedef struct {
	u32 u4Opt;
	u32 u4Param1;
	u32 u4Param2;
	u32 u4Param3;
	s16 DLBuf[SPEECH_FRAME_SAMPLES * 2U];
	s16 ULBuf1[SPEECH_FRAME_SAMPLES * 2U];
	s16 ULBuf2[SPEECH_FRAME_SAMPLES * 2U];
	s16 DLDelayBuf[SPEECH_FRAME_SAMPLES * 2U];
} SPEECH_FRAME_T;


typedef struct {
	u32 u4Size;
	u32 u4Version;
	u32 u4State;
	u32 u4MaxFrame;
	u32 u4WriteIdx;
	u32 u4ReadIdx;
	u32 u4SampleRate;
	u32 u4FrameSample;
	u32 u4FrameByte;

	SPH_ENH_08K_ctrl_struct rSphParam;
	SPH_ENH_08K_ctrl_struct rSphParam2;
	AEC_COM_RX_struct rAecRxParam;
	AEC_COM_TX_struct rAecTxParam;
	DMNR_PARAM_T rDmnrParam;
	uWord32 Sph_Enh_ctrl_16k[AEC_NDC_PARAM_NUM];
	Word16 DMNR_cal_data_16k[DMNR_PARAM_NUM_16K];
	Word16 Compen_filter_16k[COMPEN_FILTER_16K];

	SPEECH_FRAME_T rFrame[1];
} BT_SHARE_MEM_T;


typedef struct {
	u32 u4Size;
	u32 u4Version;
	u32 u4AECState;
	u32 u4MaxFrame;
	u32 u4WriteIdx;
	u32 u4ReadIdx;
	u32 u4SampleRate;
	u32 u4FrameSample;
	u32 u4FrameByte;

	SPH_ENH_08K_ctrl_struct rSphParam;
	SPH_ENH_08K_ctrl_struct rSphParam2;
	AEC_COM_RX_struct rAecRxParam;
	AEC_COM_TX_struct rAecTxParam;
	DMNR_PARAM_T rDmnrParam;
	uWord32 Sph_Enh_ctrl_16k[AEC_NDC_PARAM_NUM];
	Word16 DMNR_cal_data_16k[DMNR_PARAM_NUM_16K];
	Word16 Compen_filter_16k[COMPEN_FILTER_16K];

	SPEECH_FRAME_T rFrame[SPEECH_FRAME_COUNT];
} BT_SHARE_MEM_EX_T;


#endif
