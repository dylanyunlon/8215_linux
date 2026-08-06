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
#include "dtmf.h"

#include "pcm_debug.h"
#define LOG_TAG "dtmf"

static Dtmf g_rDtmf;
static DtmfInfoSender g_DtmfInfoSender;

static s32 DTMF_LOW_FREQ[DTMF_FREQ_SIZE] = {
	697, 770, 852, 941
};
static s32 DTMF_HIGH_FREQ[DTMF_FREQ_SIZE] = {
	1209, 1336, 1477, 1633
};
static char DTMF_ASCII_TB[DTMF_FREQ_SIZE][DTMF_FREQ_SIZE] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}
};


#define PI			(double)3.1415926
#define PI_DIV_2	(double)1.5707963


static double sin(double x)
{
	s32 sign = 1;
	s32 itemCnt = 6;
	s32 k = 0;
	s32 factorial = 1;
	double result = 0.0;
	double tx = 0.0;

	if (x < 0.0) {
		x = -x;
		sign *= -1;
	}

	if (x > (PI * 2.0)) {
		x -= (PI * 2.0);
	}

	if (x > PI) {
		x -= PI;
		sign *= (-1);
	}

	if (x > PI_DIV_2) {
		x = PI - x;
	}

	tx = x;
	for (k = 0; k < itemCnt; k++) {
		if (k % 2 == 0) {
			result += (tx / (double)factorial);
		} else {
			result -= (tx / (double)factorial);
		}

		tx *= (x * x);
		factorial *= ((s32)2 * (k + (s32)1));
		factorial *= ((s32)2 * (k + (s32)1) + (s32)1);
	}

	return ((sign == 1) ? result : (-result));
}

static double cos(double x)
{
	return sin(PI_DIV_2 - x);
}

static double sqrt(double x)
{
	double val = x;
	double last = 0.0;

	do {
		last = val;
		val = (val + x / val) / 2.0;
	} while (((val - last) > 0.00000001) || ((last - val) > 0.00000001));

	return val;
}

static bool LogRange(void)
{
	return ((g_rDtmf.m_u4DbgCnt >= g_rDtmf.m_u4LogSt) &&
		(g_rDtmf.m_u4DbgCnt < g_rDtmf.m_u4LogEnd));
}

static u32 Dtmf_GetLeastCnt(u32 u4Time, double dlScale)
{
	double dlSample = ((double)DTMF_SAMPLING_RATE / 1000.0 * (double)u4Time);
	u32 u4Least = (u32)(dlSample * dlScale / (double)g_rDtmf.m_u4NewSamples);

	return u4Least;
}

u32 Dtmf_GetDtmfCtrlType(void)
{
	return g_rDtmf.m_u4DtmfCtrlType;
}

/***************************************************************************************************
Name : Dtmf_GoertzelMag
Description : magnitude of target frequency compute use goertzel algorithm

Inputs : target frequency, sample pcm data
Outputs :
Return :
Others : magnitude of target frequency
***************************************************************************************************/
static double Dtmf_GoertzelMag(s32 u4Freq, const s16 *prBuffer)
{
	double	dlMagnitude;
	u32	k, i;
	double	dlNumSamples;
	double	omega, sine, cosine, coeff, real, imag;
	double	q0 = 0.0, q1 = 0.0, q2 = 0.0;
	double	dlScalingFactor = (double)DTMF_FRAME_SAMPLES / 2.0;

	dlNumSamples = (double)DTMF_FRAME_SAMPLES;
	k = (u32)(0.5 + ((dlNumSamples * (double)u4Freq) / (double)DTMF_SAMPLING_RATE));
	omega = (2.0 * DTMF_PI * (double)k) / dlNumSamples;
	sine = sin(omega);
	cosine = cos(omega);
	coeff = 2.0 * cosine;

	for (i = 0; i < DTMF_FRAME_SAMPLES; i++) {
		q0 = coeff * q1 - q2 + (double)prBuffer[i];
		q2 = q1;
		q1 = q0;
	}

	/* calculate the real and imaginary results
	    scaling appropriately */
	real = (q1 - q2 * cosine) / dlScalingFactor;
	imag = (q2 * sine) / dlScalingFactor;

	dlMagnitude = sqrt(real * real + imag * imag);

	return dlMagnitude;
}

static void Dtmf_FindMaxDataIdx(void)
{
	u32 i = 0;
	double dlLowMax = 0.0, u4HighMax = 0.0;

	g_rDtmf.m_u4LowIdx = 0;
	g_rDtmf.m_u4HighIdx = 0;
	for (i = 0; i < DTMF_FREQ_SIZE; i++) {
		g_rDtmf.m_adlLowMag[i] = Dtmf_GoertzelMag(DTMF_LOW_FREQ[i], g_rDtmf.m_ai2DtmfBuf);
		if (g_rDtmf.m_adlLowMag[i] > dlLowMax) {
			dlLowMax = g_rDtmf.m_adlLowMag[i];
			g_rDtmf.m_u4LowIdx = i;
		}

		g_rDtmf.m_adlHighMag[i] = Dtmf_GoertzelMag(DTMF_HIGH_FREQ[i], g_rDtmf.m_ai2DtmfBuf);
		if (g_rDtmf.m_adlHighMag[i] > u4HighMax) {
			u4HighMax = g_rDtmf.m_adlHighMag[i];
			g_rDtmf.m_u4HighIdx = i;
		}
	}

	if (LogRange()) {
		PCM_DEBUG(LOG_TAG, "Dtmf_FindMaxDataIdx: Magnitude(Cnt: %d)\r\n",
			(s32)g_rDtmf.m_u4DbgCnt);
		for (i = 0; i < (u32)DTMF_FREQ_SIZE; i++)
			PCM_DEBUG(LOG_TAG, "Dtmf_FindMaxDataIdx: %d:\t%d/100\t%d/100\r\n",
				(s32)i, (u32)(g_rDtmf.m_adlLowMag[i] * 100.0),
				(u32)(g_rDtmf.m_adlHighMag[i] * 100.0));
	}
}

static bool Dtmf_MaxDataEnough(void)
{
	if ((g_rDtmf.m_adlLowMag[g_rDtmf.m_u4LowIdx] < g_rDtmf.m_dlLowThreshold) ||
		(g_rDtmf.m_adlHighMag[g_rDtmf.m_u4HighIdx] < g_rDtmf.m_dlHighThreshold)) {
		if (LogRange()) {
			PCM_DEBUG(LOG_TAG, "Dtmf_MaxDataEnough: Magnitude not great enough!\r\n");
		}
		return false;
	}

	return true;
}

static bool Dtmf_CheckNoNoise(void)
{
	u32 i = 0;
	double dlLowNoiseThres = g_rDtmf.m_adlLowMag[g_rDtmf.m_u4LowIdx] * g_rDtmf.m_dlNoiseRatio;
	double dlHighNoiseThres = g_rDtmf.m_adlHighMag[g_rDtmf.m_u4HighIdx] * g_rDtmf.m_dlNoiseRatio;
	u32 u4NoiseCheckCnt = 0;

	for (i = 0; i < DTMF_FREQ_SIZE; i++) {
		if (g_rDtmf.m_adlLowMag[i] > dlLowNoiseThres) {
			u4NoiseCheckCnt++;
		}

		if (g_rDtmf.m_adlHighMag[i] > dlHighNoiseThres) {
			u4NoiseCheckCnt++;
		}
	}

	if (u4NoiseCheckCnt > 2U) {
		if (LogRange()) {
			PCM_DEBUG(LOG_TAG, "Dtmf_CheckNoNoise: Noise too great!!!\r\n");
		}
		return false;
	}

	return true;
}

static void Dtmf_GetNewDtmfSignal(u32 u4NotifyMsg)
{
	PCM_DEBUG(LOG_TAG, "Dtmf_GetNewDtmfSignal: (%d: %d, %d, %d, %d -- %d)!!!!\r\n",
		(s32)g_rDtmf.m_u4DbgCnt, (s32)g_rDtmf.m_u4ValidCnt1, (s32)g_rDtmf.m_u4ValidCnt2,
		(s32)g_rDtmf.m_u4BadValidCnt, (s32)g_rDtmf.m_u4InValidCnt, (s32)u4NotifyMsg);
	g_rDtmf.m_u4Signal1 = u4NotifyMsg;
	g_rDtmf.m_u4ValidCnt1 = 1;

	g_rDtmf.m_u4Signal2 = 0xFF;
	g_rDtmf.m_u4ValidCnt2 = 0;

	g_rDtmf.m_u4BadValidCnt = 0;
}

static void Dtmf_AnalyseDtmfSignal(u32 u4NotifyMsg)
{
	if (g_rDtmf.m_u4Signal1 == u4NotifyMsg) {
		g_rDtmf.m_u4ValidCnt1++;
	} else if (g_rDtmf.m_u4Signal2 == u4NotifyMsg) {
		g_rDtmf.m_u4ValidCnt2++;
	} else {
		if (g_rDtmf.m_u4ValidCnt1 < g_rDtmf.m_u4ValidCnt2) {
			PCM_DEBUG(LOG_TAG, "Dtmf_AnalyseDtmfSignal: 1(%d: %d, %d, %d, %d -- %d)\r\n",
				(s32)g_rDtmf.m_u4DbgCnt, (s32)g_rDtmf.m_u4ValidCnt1, (s32)g_rDtmf.m_u4ValidCnt2,
				(s32)g_rDtmf.m_u4BadValidCnt, (s32)g_rDtmf.m_u4InValidCnt, (s32)u4NotifyMsg);
			g_rDtmf.m_u4Signal1 = u4NotifyMsg;
			g_rDtmf.m_u4ValidCnt1 = 1;
		} else {
			PCM_DEBUG(LOG_TAG, "Dtmf_AnalyseDtmfSignal: 2(%d: %d, %d, %d, %d -- %d)\r\n",
				(s32)g_rDtmf.m_u4DbgCnt, (s32)g_rDtmf.m_u4ValidCnt1, (s32)g_rDtmf.m_u4ValidCnt2,
				(s32)g_rDtmf.m_u4BadValidCnt, (s32)g_rDtmf.m_u4InValidCnt, (s32)u4NotifyMsg);
			g_rDtmf.m_u4Signal2 = u4NotifyMsg;
			g_rDtmf.m_u4ValidCnt2 = 1;
		}
	}
}

static void Dtmf_NotifyDtmfSignal(u32 u4NotifyMsg)
{
	DtmfMsg rDtmfMsg = {0};

	g_rDtmf.m_u4SignalCnt++;
	g_rDtmf.m_u4Signal = u4NotifyMsg;
	rDtmfMsg.u4NotifyId = g_rDtmf.m_u4NotifyId;
	rDtmfMsg.u4Param1 = u4NotifyMsg;
	PCM_DEBUG(LOG_TAG, "Dtmf_NotifyDtmfSignal: NotifyMsg(%c)\r\n", (s32)u4NotifyMsg);
	/*68031 DA must to do
	SendMsgToUser(&rDtmfMsg, sizeof(rDtmfMsg));
	*/
	/*68031  PostMessage(HWND_BROADCAST, g_rDtmf.m_u4NotifyId, u4NotifyMsg, 0);*/
}

static void Dtmf_AnalyseInput(void)
{
	Dtmf_FindMaxDataIdx();

	if ((Dtmf_MaxDataEnough()) && (Dtmf_CheckNoNoise())) {
		/*detect valid signal*/
		u32 u4NotifyMsg = DTMF_ASCII_TB[g_rDtmf.m_u4LowIdx][g_rDtmf.m_u4HighIdx];

		switch (g_rDtmf.m_u4DetectState) {
		case ST_INVALID:
			Dtmf_GetNewDtmfSignal(u4NotifyMsg);
			g_rDtmf.m_u4DetectState = ST_VALID_START;
			break;

		case ST_VALID_START:
			Dtmf_AnalyseDtmfSignal(u4NotifyMsg);
			if ((g_rDtmf.m_u4ValidCnt1 >= g_rDtmf.m_u4ValidLeast) ||
				(g_rDtmf.m_u4ValidCnt2 >= g_rDtmf.m_u4ValidLeast)) {
				Dtmf_NotifyDtmfSignal(u4NotifyMsg);
				g_rDtmf.m_u4DetectState = ST_VALID_NOTIFY;
			}
			break;

		case ST_VALID_NOTIFY:
		default:
			Dtmf_AnalyseDtmfSignal(u4NotifyMsg);
			break;
		}
		g_rDtmf.m_u4InValidCnt = 0;
	} else {
		/*detect invalid signal*/
		g_rDtmf.m_u4InValidCnt++;
		if (g_rDtmf.m_u4DetectState != ST_INVALID) {
			g_rDtmf.m_u4BadValidCnt++;
			if (g_rDtmf.m_u4InValidCnt >= g_rDtmf.m_u4InValidLeast) {
				PCM_DEBUG(LOG_TAG, "Dtmf_AnalyseInput:");
				PCM_DEBUG(LOG_TAG, "InValid(%d: %d, %d, %d, %d --%d)\r\n",
					(s32)g_rDtmf.m_u4DbgCnt, (s32)g_rDtmf.m_u4ValidCnt1, (s32)g_rDtmf.m_u4ValidCnt2,
					(s32)g_rDtmf.m_u4BadValidCnt, (s32)g_rDtmf.m_u4InValidCnt,
					(s32)g_rDtmf.m_u4Signal1);
				g_rDtmf.m_u4DetectState = ST_INVALID;
			}
		}
	}

	if (g_rDtmf.m_u4DetectState != ST_INVALID) {
		if (g_rDtmf.m_u4ValidCnt1 + g_rDtmf.m_u4ValidCnt2 + g_rDtmf.m_u4BadValidCnt > g_rDtmf.m_u4MaxLen) {
			PCM_DEBUG(LOG_TAG, "Dtmf_AnalyseInput: Over Max Len(%d: %d, %d, %d, %d -- %d)\r\n",
				(s32)g_rDtmf.m_u4DbgCnt, (s32)g_rDtmf.m_u4ValidCnt1, (s32)g_rDtmf.m_u4ValidCnt2,
				(s32)g_rDtmf.m_u4BadValidCnt, (s32)g_rDtmf.m_u4InValidCnt, (s32)g_rDtmf.m_u4Signal1);
			g_rDtmf.m_u4DetectState = ST_INVALID;
		}
	}
	g_rDtmf.m_u4DbgCnt++;
}

static void Dtmf_ProcAnalyse(const s16 *prBuffer, u32 u4BufSize)
{
	u32 i = 0;
	u32 u4NewSize = (u32)(g_rDtmf.m_u4NewSamples * 2U);
	u32 u4ReUseSize = (u32)((u32)DTMF_FRAME_BYTES - u4NewSize);

	s16 *ai2NewBufSt = g_rDtmf.m_ai2DtmfBuf + DTMF_FRAME_SAMPLES - g_rDtmf.m_u4NewSamples;
	s16 *ai2ReUseBufSt = g_rDtmf.m_ai2DtmfBuf + g_rDtmf.m_u4NewSamples;

	for (i = 0; i < u4BufSize / 2U; i += g_rDtmf.m_u4NewSamples) {
		x_memcpy((void *)(ai2NewBufSt), (void *)(prBuffer + i), u4NewSize);
		Dtmf_AnalyseInput();
		x_memcpy((void *)(g_rDtmf.m_ai2DtmfBuf), (void *)(ai2ReUseBufSt), u4ReUseSize);
	}
}

static void Dtmf_ProcCpData(const s16 *prBuffer, u32 u4BufSize)
{
	if (g_rDtmf.m_hECarDtmfDecodeADC != NULL) {
		PCM_DEBUG(LOG_TAG, "Dtmf_ProcCpData....\r\n");
		x_memcpy((void *)(g_rDtmf.m_ai2DtmfBuf), (void *)(prBuffer), u4BufSize);
		/*68031 DeviceIoControl(g_rDtmf.m_hECarDtmfDecodeADC,
		DAD_TIMER0_PUTBUFF, (LPBYTE)g_rDtmf.m_ai2DtmfBuf, u4BufSize, NULL, 0, &dwOut, NULL);*/
	}
}

void Dtmf_Process(const s16 *prBuffer)
{
	switch (g_rDtmf.m_u4DtmfCtrlType) {
	case DTMF_CTRL_ANALYSE:
		Dtmf_ProcAnalyse(prBuffer, g_rDtmf.m_u4UpdateBufLen);
		break;

	case DTMF_CTRL_CPDATA:
		Dtmf_ProcCpData(prBuffer, g_rDtmf.m_u4UpdateBufLen);
		break;

	default:
		break;
	}
}

/**********************************************************************
*
* DTMF parameter Setting: threshold, NoiseRatio, Valid**, InValid**, NewSamples
*
**********************************************************************/
void Dtmf_SetThreshold(double dlLowThreshold, double dlHighThreshold)
{
	g_rDtmf.m_dlLowThreshold = dlLowThreshold;
	g_rDtmf.m_dlHighThreshold = dlHighThreshold;
    PCM_DEBUG(LOG_TAG, "Dtmf_SetThreshold: Threshold(Low: %d/100, High: %d/100)\r\n",
		(u32)(g_rDtmf.m_dlLowThreshold * 100.0), (u32)(g_rDtmf.m_dlHighThreshold * 100.0));
}

void Dtmf_SetNoiseRatio(u32 u4NoiseRatio)
{
	g_rDtmf.m_dlNoiseRatio = ((double)u4NoiseRatio / 100.0);
	PCM_DEBUG(LOG_TAG, "Dtmf_SetNoiseRatio: NoiseRatio(%d/100)\r\n", (s32)u4NoiseRatio);
}

void Dtmf_SetValidTime(u32 u4ValidTime, u32 u4Scale)
{
	g_rDtmf.m_u4ValidTime = u4ValidTime;
	g_rDtmf.m_dlValidScale = (double)u4Scale / 100.0;
	g_rDtmf.m_u4ValidLeast = Dtmf_GetLeastCnt(g_rDtmf.m_u4ValidTime, g_rDtmf.m_dlValidScale);
	PCM_DEBUG(LOG_TAG, "Dtmf_SetValidTime: ValidLeast(%d, %d/100 -- %d)\r\n",
		(s32)g_rDtmf.m_u4ValidTime, (u32)(g_rDtmf.m_dlValidScale * 100.0), (s32)g_rDtmf.m_u4ValidLeast);
}

void Dtmf_SetInValidTime(u32 u4InValidTime, u32 u4Scale)
{
	g_rDtmf.m_u4InValidTime = u4InValidTime;
	g_rDtmf.m_dlInValidScale = (double)u4Scale / 100.0;
	g_rDtmf.m_u4InValidLeast = Dtmf_GetLeastCnt(g_rDtmf.m_u4InValidTime, g_rDtmf.m_dlInValidScale);
	PCM_DEBUG(LOG_TAG, "Dtmf_SetInValidTime: InValidLeast(%d, %d/100 -- %d)\r\n",
		(s32)g_rDtmf.m_u4InValidTime, (u32)(g_rDtmf.m_dlInValidScale * 100.0),
		(s32)g_rDtmf.m_u4InValidLeast);
}

void Dtmf_SetNewSamples(u32 u4NewSamples)
{
	u32 u4Samples;

	u4Samples = g_rDtmf.m_u4UpdateBufLen;
	u4Samples = u4Samples >> 1U;
	while (u4Samples > 10U) {
		if (u4NewSamples >= u4Samples) {
			g_rDtmf.m_u4NewSamples = u4Samples;
			break;
		}
		u4Samples >>= 1U;
	}
	PCM_DEBUG(LOG_TAG, "Dtmf_SetNewSamples: u4NewSamples(%d)\r\n", (s32)g_rDtmf.m_u4NewSamples);
}

void Dtmf_SetMaxScale(u32 u4MaxScale)
{
	g_rDtmf.m_dlMaxSacle = (double)u4MaxScale / 100.0;
	g_rDtmf.m_u4MaxLen = Dtmf_GetLeastCnt(g_rDtmf.m_u4ValidTime, g_rDtmf.m_dlMaxSacle);
	PCM_DEBUG(LOG_TAG, "Dtmf_SetMaxScale: Max Len(%d, %d/100 -- %d)\r\n",
		(s32)g_rDtmf.m_u4ValidTime, (u32)(g_rDtmf.m_dlMaxSacle * 100.0), (s32)g_rDtmf.m_u4MaxLen);
}

void Dtmf_SetLogRange(u32 u4Start, u32 u4End)
{
	g_rDtmf.m_u4LogSt = u4Start;
	g_rDtmf.m_u4LogEnd = u4End;
	PCM_DEBUG(LOG_TAG, "Dtmf_SetLogRange: (%d -- %d) \r\n",
		(s32)g_rDtmf.m_u4LogSt, (s32)g_rDtmf.m_u4LogEnd);
}


void Dtmf_LogParams(void)
{
	PCM_DEBUG(LOG_TAG, "============Dtmf Paramters============\r\n");
	PCM_DEBUG(LOG_TAG, "Threshold: low(%d/100), high(%d/100)\r\n",
		(u32)(g_rDtmf.m_dlLowThreshold * 100.0), (u32)(g_rDtmf.m_dlHighThreshold * 100.0));
    PCM_DEBUG(LOG_TAG, "Noise Ratio: %d/100 \r\n", (u32)(g_rDtmf.m_dlNoiseRatio * 100.0));
	PCM_DEBUG(LOG_TAG, "Valid Time: %d, %d/100 \r\n",
		(s32)g_rDtmf.m_u4ValidTime, (u32)(g_rDtmf.m_dlValidScale * 100.0));
	PCM_DEBUG(LOG_TAG, "InValid Time: %d, %d/100 \r\n",
		(s32)g_rDtmf.m_u4InValidTime, (u32)(g_rDtmf.m_dlInValidScale * 100.0));
	PCM_DEBUG(LOG_TAG, "New Samples: %d	\r\n", (s32)(g_rDtmf.m_u4NewSamples));
	PCM_DEBUG(LOG_TAG, "Max Scale: %d/100 \r\n", (u32)(g_rDtmf.m_dlMaxSacle * 100.0));
	PCM_DEBUG(LOG_TAG, "Least cnt: valid(%d), max(%d), invalid(%d)\n",
		(s32)g_rDtmf.m_u4ValidLeast, (s32)g_rDtmf.m_u4MaxLen, (s32)g_rDtmf.m_u4InValidLeast);
	PCM_DEBUG(LOG_TAG, "============Dtmf Paramters end=========\r\n");
}

s32 Dtmf_Init(void)
{
	g_rDtmf.m_u4UpdateBufLen = SPEECH_FRAME_BYTES;
	g_rDtmf.m_u4DtmfCtrlType = DTMF_CTRL_OFF;

	g_rDtmf.m_ai2DtmfBuf = kmalloc(DTMF_FRAME_BYTES, GFP_KERNEL);
	if (!g_rDtmf.m_ai2DtmfBuf) {
		return NORESOURCE;
	}
	memset(g_rDtmf.m_ai2DtmfBuf, 0, DTMF_FRAME_BYTES);
	PCM_DEBUG(LOG_TAG, "[PCM]Dtmf_Init: m_ai2DtmfBuf(0x%x).\r\n",
		(u32)g_rDtmf.m_ai2DtmfBuf);

	g_rDtmf.m_dlLowThreshold = DTMF_DEFAULT_THRESHOLD;
	g_rDtmf.m_dlHighThreshold = DTMF_DEFAULT_THRESHOLD;
	g_rDtmf.m_dlNoiseRatio = DTMF_DEFAULT_NOISERADIO;

	g_rDtmf.m_u4NewSamples = DTMF_DEFAULT_NEW_SAMPLES;

	g_rDtmf.m_u4ValidTime = DTMF_DEFAULT_VALID_TIME;
	g_rDtmf.m_dlValidScale = DTMF_DEFAULT_VALID_SCALE;
	g_rDtmf.m_dlMaxSacle = DTMF_DEFAULT_MAX_SCALE;

	g_rDtmf.m_u4InValidTime = DTMF_DEFAULT_INVALID_TIME;
	g_rDtmf.m_dlInValidScale = DTMF_DEFAULT_INVALID_SCALE;

	g_rDtmf.m_hECarDtmfDecodeADC = NULL;/*For copy data Using IO Ctrl*/

	/* Just For debug
	//g_rDtmf.m_u4DtmfLog = 0;*/
	g_rDtmf.m_u4LogSt = 0;
	g_rDtmf.m_u4LogEnd = 0;

	return NOERR;
}

s32 Dtmf_UnInit(void)
{
	if (NULL != g_rDtmf.m_ai2DtmfBuf) {
		kfree(g_rDtmf.m_ai2DtmfBuf);
		g_rDtmf.m_ai2DtmfBuf = NULL;
	}

	return NOERR;
}

static s32 Dtmf_Set(void)
{
	g_rDtmf.m_u4DetectState = ST_INVALID;

	g_rDtmf.m_u4SignalCnt = 0;
	g_rDtmf.m_u4ValidCnt1 = 0;
	g_rDtmf.m_u4ValidCnt2 = 0;
	g_rDtmf.m_u4BadValidCnt = 0;
	g_rDtmf.m_u4InValidCnt = 0;
	g_rDtmf.m_u4DbgCnt = 0;

	g_rDtmf.m_u4ValidLeast = Dtmf_GetLeastCnt(g_rDtmf.m_u4ValidTime, g_rDtmf.m_dlValidScale);
	g_rDtmf.m_u4MaxLen = Dtmf_GetLeastCnt(g_rDtmf.m_u4ValidTime, g_rDtmf.m_dlMaxSacle);
	g_rDtmf.m_u4InValidLeast = Dtmf_GetLeastCnt(g_rDtmf.m_u4InValidTime, g_rDtmf.m_dlInValidScale);

	memset(g_rDtmf.m_ai2DtmfBuf, 0, DTMF_FRAME_BYTES);

	/*68031
	//For copy data Using IO Ctrl
	if(g_rDtmf.m_u4DtmfCtrlType == DTMF_CTRL_CPDATA)
	{
		g_rDtmf.m_hECarDtmfDecodeADC = CreateFile(L"DAD1:",
			GENERIC_READ,							// open for reading
			FILE_SHARE_READ|FILE_SHARE_READ,		// share for reading
			NULL,									// no security
			OPEN_EXISTING,							// existing file only
			FILE_ATTRIBUTE_NORMAL,					// normal file
			NULL);									// no attr. template
		WAVELOG(Log(DTMF_INFO), (TEXT("[DTMF]hECarDtmfDecodeADC: 0x%x\n"), m_hECarDtmfDecodeADC));
	}
	*/
	Dtmf_LogParams();

	return NOERR;
}

s32 Dtmf_EnableDtmfFunc(u32 u4DtmfCtrlType, u32 u4NotifyId)
{
	if (g_rDtmf.m_u4DtmfCtrlType != u4DtmfCtrlType) {
		g_rDtmf.m_u4DtmfCtrlType = u4DtmfCtrlType;
		PCM_DEBUG(LOG_TAG, "[PCM]Dtmf_EnableDtmfFunc: %d, NotifyId: 0x%x\r\n",
			(s32)g_rDtmf.m_u4DtmfCtrlType, (u32)u4NotifyId);
		if (g_rDtmf.m_u4DtmfCtrlType) {
			g_rDtmf.m_u4NotifyId = u4NotifyId;
			Dtmf_Set();
			/*68031
			if (Log(DTMF_CPDATA))
			{
				s16 buf[5] = {0x123, 0x4876, 0x95, -0x7122, -0x376};
				vDtmfProcCpData(buf, 10);
			}*/
		}
	}

	return NOERR;
}


static PDTMF_INFO_BUF_T DtmfInfoSender_CreateNewDataInfo(u32 u4SAdr, u32 u4Size)
{
	PDTMF_INFO_BUF_T prBufInfo = (PDTMF_INFO_BUF_T)kmalloc(sizeof(struct DTMF_INFO_BUF_T), GFP_KERNEL);

	if (prBufInfo) {
		prBufInfo->prNextBuf = NULL;
		prBufInfo->pi2AsyncBuf = kmalloc(u4Size, GFP_KERNEL);
		if (prBufInfo->pi2AsyncBuf) {
			memcpy(prBufInfo->pi2AsyncBuf, (s16 *)u4SAdr, u4Size);
		}
		prBufInfo->u4Size = u4Size;
		prBufInfo->u4RP = 0;
	}

	return prBufInfo;
}

static void DtmfInfoSender_InsertDataInfo(PDTMF_INFO_BUF_T prBufInfo)
{
	if (g_DtmfInfoSender.m_prBufList == NULL) {
		g_DtmfInfoSender.m_prBufList = prBufInfo;
	} else {
		g_DtmfInfoSender.m_prBufList->prNextBuf = prBufInfo;
	}
}

static void DtmfInfoSender_DeleteDataInfo(void)
{
	PDTMF_INFO_BUF_T prBuf = g_DtmfInfoSender.m_prBufList;

	if (prBuf) {
		g_DtmfInfoSender.m_prBufList = g_DtmfInfoSender.m_prBufList->prNextBuf;
		kfree(prBuf->pi2AsyncBuf);
		prBuf->pi2AsyncBuf = NULL;
		kfree(prBuf);
		prBuf = NULL;
	}
}

static void DtmfInfoSender_Notify(void)
{
	/*68031 PostMessage(HWND_BROADCAST, m_u4NotifyId, m_u4ProcBufCnt, m_u4RecvBufCnt);*/
	PCM_DEBUG(LOG_TAG, "DtmfInfoSender_Notify: NotifyId(0x%x), Cnt(%d, %d)\r\n",
		(u32)g_DtmfInfoSender.m_u4NotifyId, (s32)g_DtmfInfoSender.m_u4ProcBufCnt,
		(s32)g_DtmfInfoSender.m_u4RecvBufCnt);
}

static void DtmfInfoSender_ShowDataInfo(u32 u4Cnt)
{
	u32 i = 0;
	PDTMF_INFO_BUF_T prBuf = g_DtmfInfoSender.m_prBufList;

	while (NULL != prBuf) {
		PCM_DEBUG(LOG_TAG, "DtmfInfoSender_ShowDataInfo:[Proc(%d)]0x%x ->\r\n",
			(s32)g_DtmfInfoSender.m_u4ProcBufCnt, (u32)(prBuf->pi2AsyncBuf));

		u4Cnt = (u4Cnt == 0) ? 10 : u4Cnt;
		for (i = 0; i < u4Cnt; i++) {
			pr_debug("0x%x ", *(prBuf->pi2AsyncBuf + i));
		}
		pr_debug("\r\n");
		prBuf = prBuf->prNextBuf;
	}
}

void DtmfInfoSender_Init(u32 u4NotifyId)
{
	PCM_DEBUG(LOG_TAG, "DtmfInfoSender_Init: NotifyId(0x%x)\r\n", (u32)u4NotifyId);
	g_DtmfInfoSender.m_u4NotifyId = u4NotifyId;
	g_DtmfInfoSender.m_u4ProcLen = SPEECH_FRAME_BYTES;
	g_DtmfInfoSender.m_prBufList = NULL;

	g_DtmfInfoSender.m_u4ProcBufCnt = 0;
	g_DtmfInfoSender.m_u4RecvBufCnt = 0;
	g_DtmfInfoSender.m_fgSender = true;
}

void DtmfInfoSender_UnInit(void)
{
	PCM_DEBUG(LOG_TAG, "DtmfInfoSender_UnInit: Delete\r\n");
	g_DtmfInfoSender.m_prBufList = NULL;
	g_DtmfInfoSender.m_fgSender = false;
}

s32 DtmfInfoSender_GetNewData(u32 u4SAdr, u32 u4Size)
{
	s32 i4Ret = NOERR;

	if (u4Size % g_DtmfInfoSender.m_u4ProcLen) {
		PCM_ERROR(LOG_TAG, "DtmfInfoSender_GetNewData: The size(%d)of new buf is error! \r\n", (s32)u4Size);
		i4Ret = NORESOURCE;
	} else {
		PDTMF_INFO_BUF_T prBufInfo = DtmfInfoSender_CreateNewDataInfo(u4SAdr, u4Size);

		if (prBufInfo) {
			DtmfInfoSender_InsertDataInfo(prBufInfo);
			g_DtmfInfoSender.m_u4RecvBufCnt++;
		}
	}

	return i4Ret;
}

u32 DtmfInfoSender_ProcessData(s16 *prBuffer)
{
	if (g_DtmfInfoSender.m_prBufList) {
		s16 *i2SAddr = (g_DtmfInfoSender.m_prBufList->pi2AsyncBuf + g_DtmfInfoSender.m_prBufList->u4RP / 2);

		memcpy((void *)(prBuffer), (void *)(i2SAddr), g_DtmfInfoSender.m_u4ProcLen);

		g_DtmfInfoSender.m_prBufList->u4RP += g_DtmfInfoSender.m_u4ProcLen;
		if (g_DtmfInfoSender.m_prBufList->u4RP == g_DtmfInfoSender.m_prBufList->u4Size) {
			g_DtmfInfoSender.m_u4ProcBufCnt++;
			DtmfInfoSender_Notify();
			DtmfInfoSender_DeleteDataInfo();
		}
	}

	return NOERR;
}

void DtmfInfoSender_SimulateProcessData(u32 u4Cnt)
{
	s16  *prMicBuf = NULL;

	DtmfInfoSender_ShowDataInfo(u4Cnt);

	prMicBuf = kmalloc(g_DtmfInfoSender.m_u4ProcLen, GFP_KERNEL);
	if (prMicBuf) {
		u32 i = g_DtmfInfoSender.m_prBufList->u4Size / g_DtmfInfoSender.m_u4ProcLen;

		while (i--) {
			DtmfInfoSender_ProcessData(prMicBuf);
		}
		kfree(prMicBuf);
		prMicBuf = NULL;
	}
}

bool DtmfInfoSender_GetSenderEn(void)
{
	return g_DtmfInfoSender.m_fgSender;
}


s32 DtmfTest_AnalyseFiles(u32 u4NotifyId, u32 u4Range)
{
	u32 u4Idx = 0;
	u32 u4Start = (u4Range >> 16U);
	u32 u4End = (u4Range & 0xFFFFU);
	s16 *pi2Buf = NULL;
	char szFileName[200] = {0};
	int32_t u4FileSize = 0;
	int32_t u4ReadSize = 0;
	mm_segment_t fs;
	struct file *m_pfDL;
	loff_t t_cur_pos = 0;

	PCM_DEBUG(LOG_TAG, "DtmfTest_AnalyseFiles: u4NotifyId(%d) u4Range(0x%x) \r\n",
		(s32)u4NotifyId, (s32)u4Range);
	for (u4Idx = u4Start; u4Idx <= u4End; u4Idx++) {
		u32 u4ReadPtr = 0;

		/*Read data from File*/
		sprintf(szFileName, "/data/%dfe.wav", (s32)u4Idx);
		PCM_DEBUG(LOG_TAG, "DtmfTest_AnalyseFiles: %s\r\n", szFileName);
		fs = get_fs();
		set_fs(KERNEL_DS);
		m_pfDL = filp_open(szFileName, O_RDONLY, 0);
		if (IS_ERR(m_pfDL)) {
			PCM_ERROR(LOG_TAG, "DtmfTest_AnalyseFiles: filp_open err(%d)!\r\n", (u32)m_pfDL);
			set_fs(fs);
			return NORESOURCE;
		}
		u4FileSize = vfs_llseek(m_pfDL, 0, SEEK_END) - sizeof(WaveHeader);
		pi2Buf = kmalloc(u4FileSize, GFP_KERNEL);
		if (NULL == pi2Buf) {
			PCM_ERROR(LOG_TAG, "DtmfTest_AnalyseFiles: kmalloc err!\r\n");
			filp_close(m_pfDL, NULL);
			set_fs(fs);
			return NORESOURCE;
		}

		t_cur_pos = vfs_llseek(m_pfDL, sizeof(WaveHeader), SEEK_SET);
		u4ReadSize = vfs_read(m_pfDL, (void *)pi2Buf, u4FileSize, &t_cur_pos);
		if (u4ReadSize < 0) {
			PCM_ERROR(LOG_TAG, "DtmfTest_AnalyseFiles: vfs_read err(%i)!\r\n", u4ReadSize);
			filp_close(m_pfDL, NULL);
			set_fs(fs);
			kfree(pi2Buf);
			pi2Buf = NULL;
			return NORESOURCE;
		}
		filp_close(m_pfDL, NULL);
		set_fs(fs);

		/*Analyse file
		//68031 m_u4GroupCnt = 0;
		//68031 m_u4SignalCnt = 0;
		//68031 m_fgGignalShowed = false;
		//68031 PostMessage(HWND_BROADCAST, (m_u4NotifyId+1), DTMFTEST_OPEN, m_u4FileIdx);*/
		Dtmf_EnableDtmfFunc(DTMF_CTRL_ANALYSE, u4NotifyId);
		while (((u32)u4ReadSize - u4ReadPtr) >= SPEECH_FRAME_BYTES) {
			Dtmf_Process((s16 *)(pi2Buf + u4ReadPtr / 2U));
			u4ReadPtr += SPEECH_FRAME_BYTES;
		}
		Dtmf_EnableDtmfFunc(DTMF_CTRL_OFF, u4NotifyId);

		kfree(pi2Buf);
		pi2Buf = NULL;
	}

	return NOERR;
}
