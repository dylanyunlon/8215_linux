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



#ifndef DTMF_H
#define DTMF_H

#include "x_typedef.h"
#include "speechdev.h"


#define DTMF_FRAME_SAMPLES				205U
#define DTMF_FRAME_BYTES				(DTMF_FRAME_SAMPLES * 2U)

#define DTMF_PI							3.1415
#define DTMF_SAMPLING_RATE				8000U
#define DTMF_FREQ_SIZE					4U

#define DTMF_DEFAULT_THRESHOLD			328.0
#define DTMF_DEFAULT_NOISERADIO			0.7

#define DTMF_DEFAULT_NEW_SAMPLES		(SPEECH_FRAME_SAMPLES >> 2U)

#define DTMF_DEFAULT_VALID_TIME			100U
#define DTMF_DEFAULT_VALID_SCALE		0.5
#define DTMF_DEFAULT_MAX_SCALE			1.3

#define DTMF_DEFAULT_INVALID_TIME		80U
#define DTMF_DEFAULT_INVALID_SCALE		0.6

/*For debug log*/
#define DTMF_INFO			(0x1U << 0)
#define DTMF_STATE			(0x1U << 1)
#define DTMF_WARN			(0x1U << 2)
#define DTMF_DATA			(0x1U << 3)

#define DTMF_CPDATA			(0x1U << 4)

#define DTMF_TEST			(0x1U << 8)

enum {
	ST_INVALID = 0,
	ST_VALID_START,
	ST_VALID_NOTIFY
};

enum {
	DTMF_CTRL_OFF = 0,
	DTMF_CTRL_ANALYSE,
	DTMF_CTRL_CPDATA
};


typedef struct _DTMF_ {
	void* m_hECarDtmfDecodeADC; /* For copy data Using IO Ctrl */

	u32 m_u4DtmfCtrlType;
	u32 m_u4NotifyId;

	s16 *m_ai2DtmfBuf;
	u32 m_u4UpdateBufLen;

	double m_adlLowMag[DTMF_FREQ_SIZE];
	double m_adlHighMag[DTMF_FREQ_SIZE];
	u32 m_u4LowIdx;
	u32 m_u4HighIdx;

	double m_dlLowThreshold;
	double m_dlHighThreshold;
	double m_dlNoiseRatio;

	u32 m_u4NewSamples;

	u32 m_u4DetectState;
	u32 m_u4SignalCnt;
	u32 m_u4Signal;
	u32 m_u4Signal1;
	u32 m_u4Signal2;
	u32 m_u4ValidCnt1;
	u32 m_u4ValidCnt2;
	u32 m_u4BadValidCnt;
	u32 m_u4InValidCnt;

	u32 m_u4ValidTime;
	double m_dlValidScale;
	u32 m_u4ValidLeast;

	double m_dlMaxSacle;
	u32 m_u4MaxLen;

	u32 m_u4InValidTime;
	double m_dlInValidScale;
	u32 m_u4InValidLeast;

	/* For debug
	//u32 m_u4DtmfLog;*/
	u32 m_u4DbgCnt;
	u32 m_u4LogSt;
	u32 m_u4LogEnd;
} Dtmf;

typedef struct _DTMF_MSG_ {
	u32 u4NotifyId;
	u32 u4Param1;
	u32 u4Param2;
} DtmfMsg;

s32 Dtmf_Init(void);
s32 Dtmf_UnInit(void);
void Dtmf_LogParams(void);
void Dtmf_SetLogRange(u32 u4Start, u32 u4End);
s32 Dtmf_EnableDtmfFunc(u32 u4DtmfCtrlType, u32 u4NotifyId);
void Dtmf_SetThreshold(double dlLowThreshold, double dlHighThreshold);
void Dtmf_SetNoiseRatio(u32 u4NoiseRatio);
void Dtmf_SetValidTime(u32 u4ValidTime, u32 u4Scale);
void Dtmf_SetInValidTime(u32 u4InValidTime, u32 u4Scale);
void Dtmf_SetNewSamples(u32 u4NewSamples);
void Dtmf_SetMaxScale(u32 u4MaxScale);
void Dtmf_Process(const s16 *prBuffer);
u32 Dtmf_GetDtmfCtrlType(void);



typedef struct DTMF_INFO_BUF_T {
	s16 *pi2DestBuf;
	void *pvSrcBuf;
	s16 *pi2AsyncBuf;
	u32 u4Size;
	u32 u4RP;
	struct DTMF_INFO_BUF_T *prNextBuf;
} *PDTMF_INFO_BUF_T;

typedef struct _DTMF_INFO_SENDER_ {
	bool   m_fgSender;
	u32 m_u4NotifyId;
	u32 m_u4ProcLen;

	u32 m_u4ProcBufCnt;
	u32 m_u4RecvBufCnt;

	PDTMF_INFO_BUF_T m_prBufList;
} DtmfInfoSender;

void DtmfInfoSender_Init(u32 u4NotifyId);
void DtmfInfoSender_UnInit(void);
s32 DtmfInfoSender_GetNewData(u32 u4SAdr, u32 u4Size);
u32 DtmfInfoSender_ProcessData(s16 *prBuffer);
void DtmfInfoSender_SimulateProcessData(u32 u4Cnt);
bool DtmfInfoSender_GetSenderEn(void);

s32 DtmfTest_AnalyseFiles(u32 u4NotifyId, u32 u4Range);

#endif

