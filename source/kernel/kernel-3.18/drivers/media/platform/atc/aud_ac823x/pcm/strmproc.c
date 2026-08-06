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


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>
#include <linux/delay.h>

#include "winutil.h"
#include "strmproc.h"
#include "outhw.h"
#include "aud_pcm_dbg.h"
#include "speechdev.h"
#include "pcm_ac83xx.h"




#define INSAMPLES2BYTES(sample) ((sample) >> (m_u4InShift))
#define INBYTES2SAMPLES(sample) ((sample) << (m_u4InShift))
#define OUTSAMPLES2BYTES(sample) ((sample) << 1)
#define OUTBYTES2SAMPLES(sample) ((sample) >> 1)

#define OPTIMIZE_FILL_DATA 1
#define SINGLE_STREAM	   0
#define NO_GAIN_CTRL	   0

extern SpeechDeviceContext g_prSpeechDev;

#define MAX_SIZE_CAHCE_IN_INBUF  3
#define MAX_SIZE_CAHCE_IN_OUTBUF 4

#define AUD_STRM_OUT_MAX_LEN	(19200U)


#define Sat_16BIT(a)	((a) > 0x7fff ? 0x7fff : (a) < -32768 ? -32768 : (a))
#define Limit_Sat(a) {\
			if (a > 0x7fff)\
				a = 0x7ffff;\
			else if (a < -32767)\
				a = -32768;\
			else\
				a = a;\
		}



/********************For ChangeGain****************************/
const u32 GainMap[] = {
	0xf1ad, /* 0: -0.50 dB */
	0xe429, /* 1: -1.00 dB */
	0xd765, /* 2: -1.50 dB */
	0xcb59, /* 3: -2.00 dB */
	0xbff9, /* 4: -2.50 dB */
	0xb53b, /* 5: -3.00 dB */
	0xab18, /* 6: -3.50 dB */
	0xa186, /* 7: -4.00 dB */
	0x987d, /* 8: -4.50 dB */
	0x8ff5, /* 9: -5.00 dB */
	0x87e8, /* 10: -5.50 dB */
	0x804d, /* 11: -6.00 dB */
	0x7920, /* 12: -6.50 dB */
	0x7259, /* 13: -7.00 dB */
	0x6bf4, /* 14: -7.50 dB */
	0x65ea, /* 15: -8.00 dB */
	0x6036, /* 16: -8.50 dB */
	0x5ad5, /* 17: -9.00 dB */
	0x55c0, /* 18: -9.50 dB */
	0x50f4, /* 19: -10.00 dB */
	0x4c6d, /* 20: -10.50 dB */
	0x4826, /* 21: -11.00 dB */
	0x441d, /* 22: -11.50 dB */
	0x404d, /* 23: -12.00 dB */
	0x3cb5, /* 24: -12.50 dB */
	0x394f, /* 25: -13.00 dB */
	0x361a, /* 26: -13.50 dB */
	0x3314, /* 27: -14.00 dB */
	0x3038, /* 28: -14.50 dB */
	0x2d86, /* 29: -15.00 dB */
	0x2afa, /* 30: -15.50 dB */
	0x2892, /* 31: -16.00 dB */
	0x264d, /* 32: -16.50 dB */
	0x2429, /* 33: -17.00 dB */
	0x2223, /* 34: -17.50 dB */
	0x203a, /* 35: -18.00 dB */
	0x1e6c, /* 36: -18.50 dB */
	0x1cb9, /* 37: -19.00 dB */
	0x1b1d, /* 38: -19.50 dB */
	0x1999, /* 39: -20.00 dB */
	0x182a, /* 40: -20.50 dB */
	0x16d0, /* 41: -21.00 dB */
	0x158a, /* 42: -21.50 dB */
	0x1455, /* 43: -22.00 dB */
	0x1332, /* 44: -22.50 dB */
	0x121f, /* 45: -23.00 dB */
	0x111c, /* 46: -23.50 dB */
	0x1027, /* 47: -24.00 dB */
	0x0f3f, /* 48: -24.50 dB */
	0x0e65, /* 49: -25.00 dB */
	0x0d97, /* 50: -25.50 dB */
	0x0cd4, /* 51: -26.00 dB */
	0x0c1c, /* 52: -26.50 dB */
	0x0b6f, /* 53: -27.00 dB */
	0x0acb, /* 54: -27.50 dB */
	0x0a31, /* 55: -28.00 dB */
	0x099f, /* 56: -28.50 dB */
	0x0915, /* 57: -29.00 dB */
	0x0893, /* 58: -29.50 dB */
	0x0818, /* 59: -30.00 dB */
	0x07a4, /* 60: -30.50 dB */
	0x0737, /* 61: -31.00 dB */
	0x06cf, /* 62: -31.50 dB */
	0x066e, /* 63: -32.00 dB */
	0x0612, /* 64: -32.50 dB */
	0x05bb, /* 65: -33.00 dB */
	0x0569, /* 66: -33.50 dB */
	0x051b, /* 67: -34.00 dB */
	0x04d2, /* 68: -34.50 dB */
	0x048d, /* 69: -35.00 dB */
	0x044c, /* 70: -35.50 dB */
	0x040e, /* 71: -36.00 dB */
	0x03d4, /* 72: -36.50 dB */
	0x039d, /* 73: -37.00 dB */
	0x0369, /* 74: -37.50 dB */
	0x0339, /* 75: -38.00 dB */
	0x030a, /* 76: -38.50 dB */
	0x02df, /* 77: -39.00 dB */
	0x02b6, /* 78: -39.50 dB */
	0x028f, /* 79: -40.00 dB */
	0x026a, /* 80: -40.50 dB */
	0x0248, /* 81: -41.00 dB */
	0x0227, /* 82: -41.50 dB */
	0x0208, /* 83: -42.00 dB */
	0x01eb, /* 84: -42.50 dB */
	0x01cf, /* 85: -43.00 dB */
	0x01b6, /* 86: -43.50 dB */
	0x019d, /* 87: -44.00 dB */
	0x0186, /* 88: -44.50 dB */
	0x0170, /* 89: -45.00 dB */
	0x015b, /* 90: -45.50 dB */
	0x0148, /* 91: -46.00 dB */
	0x0136, /* 92: -46.50 dB */
	0x0124, /* 93: -47.00 dB */
	0x0114, /* 94: -47.50 dB */
	0x0104, /* 95: -48.00 dB */
	0x00f6, /* 96: -48.50 dB */
	0x00e8, /* 97: -49.00 dB */
	0x00db, /* 98: -49.50 dB */
	0x00cf, /* 99: -50.00 dB */
	0x00c3, /* 100: -50.50 dB */
	0x00b8, /* 101: -51.00 dB */
	0x00ae, /* 102: -51.50 dB */
	0x00a4, /* 103: -52.00 dB */
	0x009b, /* 104: -52.50 dB */
	0x0092, /* 105: -53.00 dB */
	0x008a, /* 106: -53.50 dB */
	0x0082, /* 107: -54.00 dB */
	0x007b, /* 108: -54.50 dB */
	0x0074, /* 109: -55.00 dB */
	0x006e, /* 110: -55.50 dB */
	0x0067, /* 111: -56.00 dB */
	0x0062, /* 112: -56.50 dB */
	0x005c, /* 113: -57.00 dB */
	0x0057, /* 114: -57.50 dB */
	0x0052, /* 115: -58.00 dB */
	0x004d, /* 116: -58.50 dB */
	0x0049, /* 117: -59.00 dB */
	0x0045, /* 118: -59.50 dB */
	0x0041, /* 119: -60.00 dB */
	0x003d, /* 120: -60.50 dB */
	0x003a, /* 121: -61.00 dB */
	0x0037, /* 122: -61.50 dB */
	0x0034, /* 123: -62.00 dB */
	0x0031, /* 124: -62.50 dB */
	0x002e, /* 125: -63.00 dB */
	0x002b, /* 126: -63.50 dB */
	0x0029, /* 127: -64.00 dB */
	0x0027, /* 128: -64.50 dB */
	0x0024, /* 129: -65.00 dB */
	0x0022, /* 130: -65.50 dB */
	0x0020, /* 131: -66.00 dB */
	0x001f, /* 132: -66.50 dB */
	0x001d, /* 133: -67.00 dB */
	0x001b, /* 134: -67.50 dB */
	0x001a, /* 135: -68.00 dB */
	0x0018, /* 136: -68.50 dB */
	0x0017, /* 137: -69.00 dB */
	0x0015, /* 138: -69.50 dB */
	0x0014, /* 139: -70.00 dB */
	0x0013, /* 140: -70.50 dB */
	0x0012, /* 141: -71.00 dB */
	0x0011, /* 142: -71.50 dB */
	0x0010, /* 143: -72.00 dB */
	0x000f, /* 144: -72.50 dB */
	0x000e, /* 145: -73.00 dB */
	0x000d, /* 146: -73.50 dB */
	0x000d, /* 147: -74.00 dB */
	0x000c, /* 148: -74.50 dB */
	0x000b, /* 149: -75.00 dB */
	0x000b, /* 150: -75.50 dB */
	0x000a, /* 151: -76.00 dB */
	0x0009, /* 152: -76.50 dB */
	0x0009, /* 153: -77.00 dB */
	0x0008, /* 154: -77.50 dB */
	0x0008, /* 155: -78.00 dB */
	0x0007, /* 156: -78.50 dB */
	0x0007, /* 157: -79.00 dB */
	0x0006, /* 158: -79.50 dB */
	0x0006, /* 159: -80.00 dB */
	0x0006, /* 160: -80.50 dB */
	0x0005, /* 161: -81.00 dB */
	0x0005, /* 162: -81.50 dB */
	0x0005, /* 163: -82.00 dB */
	0x0004, /* 164: -82.50 dB */
	0x0004, /* 165: -83.00 dB */
	0x0004, /* 166: -83.50 dB */
	0x0004, /* 167: -84.00 dB */
	0x0003, /* 168: -84.50 dB */
	0x0003, /* 169: -85.00 dB */
	0x0003, /* 170: -85.50 dB */
	0x0003, /* 171: -86.00 dB */
	0x0003, /* 172: -86.50 dB */
	0x0002, /* 173: -87.00 dB */
	0x0002, /* 174: -87.50 dB */
	0x0002, /* 175: -88.00 dB */
	0x0002, /* 176: -88.50 dB */
	0x0002, /* 177: -89.00 dB */
	0x0002, /* 178: -89.50 dB */
	0x0002, /* 179: -90.00 dB */
	0x0001, /* 180: -90.50 dB */
	0x0001, /* 181: -91.00 dB */
	0x0001, /* 182: -91.50 dB */
	0x0001, /* 183: -92.00 dB */
	0x0001, /* 184: -92.50 dB */
	0x0001, /* 185: -93.00 dB */
	0x0001, /* 186: -93.50 dB */
	0x0001, /* 187: -94.00 dB */
	0x0001, /* 188: -94.50 dB */
	0x0001, /* 189: -95.00 dB */
	0x0001, /* 190: -95.50 dB */
	0x0001, /* 191: -96.00 dB */
	0x0000, /* 192: -96.50 dB */
	0x0000, /* 193: -97.00 dB */
	0x0000, /* 194: -97.50 dB */
	0x0000, /* 195: -98.00 dB */
	0x0000, /* 196: -98.50 dB */
	0x0000, /* 197: -99.00 dB */
	0x0000, /* 198: -99.50 dB */
	0x0000, /* 199: -100.00 dB */
};

static s32 m_dwGainRange = DEVICE_GAIN_RANGE;
static s32 m_dwStreamGainRange = STREAM_GAIN_RANGE;
static s32 m_dwGain = (s32)MAX_GAIN;
static s32 m_dwBTGain = (s32)MAX_GAIN;
static s32 m_i4MaxStreamGain = STREAM_GAIN_MAX;
static s32 m_i4MaxGain = DEVICE_GAIN_MAX;

static s32				SetBtSPHOnly;
static u32		m_fxpGain[2] = {0};
static u32		m_fxpBTGain[2] = {0};
static u32		m_dwGainShift;
static u32		m_dwBTGainShift;
static u32		m_fxpLastGain[2] = {0};
static u32		m_fxpLastBTGain[2] = {0};

s32 BtOutput_GetGain(void)
{
	return (s32)MAX_GAIN;
}

static s32 BtOutput_GetGainRange(void)
{
	return m_dwGainRange;
}

static s32 BtOutput_GetStreamGainRange(void)
{
	return m_dwStreamGainRange;
}

static s32 BtOutput_GetStreamMaxGain(void)
{
	return m_i4MaxStreamGain;
}

static s32 BtOutput_GetMaxGain(void)
{
	return m_i4MaxGain;
}

/* Channel 0 is the left channel, which is the low 16-bits of volume data */
static s32 BtOutput_MapGain(s32 StreamGain, s32 Channel)
{
	/* Get correct stream gain based on channel */
	s32 dwDeviceRange = BtOutput_GetGainRange();
	s32 dwStreamRange = BtOutput_GetStreamGainRange();

	s32 DeviceGain;
	s32 dwGainMultiplier;

	if (Channel == 1) {
		StreamGain = StreamGain >> 16;
	}
	StreamGain = StreamGain & 0xFFFF;

	DeviceGain = BtOutput_GetGain();
	if (Channel == 1) {
		DeviceGain = DeviceGain >> 16;
	}
	DeviceGain = DeviceGain & 0xFFFF;

	/* Special handling- if any gain is totally 0, mute the output */
	if ((StreamGain <= MUTE_VOLUME) || (DeviceGain <= MUTE_VOLUME)) {
		pr_debug("[PCM]BtOutput_MapGain: MUTE VOLUME\r\n");
		dwGainMultiplier = 0;
	} else {
		s32 dBAttenStream, dBAttenDevice, dBAttenTotal;
		s32 i4Temp = BtOutput_GetStreamMaxGain();

		while (i4Temp > 0) {
			i4Temp -= 6;
		}
		dBAttenStream = (-i4Temp) << 16;
		dBAttenStream += ((0xFFFF - StreamGain) * dwStreamRange);

		i4Temp = BtOutput_GetMaxGain();
		while (i4Temp > 0) {
			i4Temp -= 6;
		}
		dBAttenDevice = (-i4Temp) << 16;
		dBAttenDevice += ((0xFFFF - DeviceGain) * dwDeviceRange);

		/* Add together
		dBAttenTotal = dBAttenStream + dBAttenDevice	+ dBAttenProcess;*/
		dBAttenTotal = dBAttenStream + dBAttenDevice;

		/* Multiply result by 2 for .5 dB steps in the table */
		dBAttenTotal *= 2;

		/* Round up to account for rounding errors in lower 16 bits */
		dBAttenTotal += 0x8000;

		/* Now shift back to the lowest 16 bits to get an index into the table */
		dBAttenTotal = dBAttenTotal >> 16;

		/* processgain = (processgain/2 + 100)/processgain;
		dBAttenTotal += processgain;

		 dBAttenTotal should range from 0 to something like 340 (if all terms were close to 0)

		 Special case 0 as totally muted. The table starts at -.5dB, rather than 0dB, since
		 0dB would take more than the 16-bits we allowed per entry. */
		if (dBAttenTotal == 0) {
			dwGainMultiplier = 0x10000;
		} else if (dBAttenTotal > 200) {
			dwGainMultiplier = 0;
		} else {
			dwGainMultiplier = (s32)GainMap[dBAttenTotal - 1];
		}
	}

	pr_debug("[PCM]BtOutput_MapGain dwGainMultiplier(0x%x) \r\n", dwGainMultiplier);

	return dwGainMultiplier;
}

static void BtOutput_GainChange(void)
{
	s32 i = 0;

	for (i = 0; i < 2; i++) {
		#if (MONO_GAIN)
		m_fxpBTGain[i] = BtOutput_MapGain(m_dwBTGain, 0);
		#else
		m_fxpBTGain[i] = BtOutput_MapGain(m_dwBTGain, i);
		#endif
	}
	m_dwBTGainShift = 16U;
	if (BtOutput_GetMaxGain() > 0) {
		m_dwBTGainShift -= (u32)((BtOutput_GetMaxGain() + 5) / 6);
	}

	if (BtOutput_GetStreamMaxGain() > 0) {
		m_dwBTGainShift -= (u32)((BtOutput_GetStreamMaxGain() + 5) / 6);
	}

	m_fxpLastBTGain[0] = m_fxpBTGain[0];
	m_fxpLastBTGain[1] = m_fxpBTGain[1];
}

s32 BtOutput_SetGain(s32 dwGain)
{
	SetBtSPHOnly = 1;
	m_dwBTGain = dwGain;

	BtOutput_GainChange();

	return m_dwBTGain;
}

static void DeviceSPH_GainChange(void)
{
	s32 j = 0;

	for (j = 0; j < 2; j++) {
		#if (MONO_GAIN)
		m_fxpGain[j] = BtOutput_MapGain(m_dwGain, 0);
		#else
		m_fxpGain[j] = BtOutput_MapGain(m_dwGain, j);
		#endif
	}

	m_dwGainShift = 16U;
	if (BtOutput_GetMaxGain() > 0) {
		m_dwGainShift -= (u32)((BtOutput_GetMaxGain() + 5) / 6);
	}

	if (BtOutput_GetStreamMaxGain() > 0) {
		m_dwGainShift -= (u32)((BtOutput_GetStreamMaxGain() + 5) / 6);
	}

	m_fxpLastGain[0] = m_fxpGain[0];
	m_fxpLastGain[1] = m_fxpGain[1];
}

s32 DeviceSPH_SetGain(s32 dwGain)
{
	SetBtSPHOnly = 0;
	m_dwGain = dwGain;

	DeviceSPH_GainChange();

	return m_dwGain;
}

/*************************************************************/



static void ProcessInput(StreamProcess *strmProc);
static void ProcessOutput(StreamProcess *strmProc);

void StreamProcess_Init(StreamProcess *strmProc)
{
	strmProc->m_prNext = NULL;
	strmProc->m_prStream = NULL;
	strmProc->m_rOutFmt.u4FS = 48000;
	strmProc->m_rOutFmt.u4Chn = 2;
	strmProc->m_rOutFmt.u4BW = DEF_DATA_BITS;
	strmProc->m_u4Remain = 0;
	strmProc->m_rInFmt.u4FS = 8000;
	strmProc->m_rInFmt.u4Chn = 2;
	strmProc->m_prAsrc = NULL;
	strmProc->m_u4State = STATE_UNINIT;
	strmProc->m_u4OutLen = 0;
	strmProc->m_u4OutWP = 0;
	strmProc->m_u4OState = STATE_UNINIT;
	strmProc->m_u4InputTotal = 0;
	strmProc->m_u4OutputTotal = 0;

	strmProc->m_u4UnderrunCount = 0;

	mutex_init(&strmProc->m_ProcLock);
	pr_debug("[PCM]StreamProcess_Init: end\r\n");
}

s32 StrmProcDSPMixCh(u32 u4DspMixCh)
{
	u32 u4MixCh = 0;

	if ((((u32)0x0) == (u4DspMixCh & 0x3FU)) || /*  Mix to NULL */
		(0x1U == (u4DspMixCh & 0x3FU)) || /*  Mix to L */
		(0x2U == (u4DspMixCh & 0x3FU)) || /*  Mix to R */
		(0x3U == (u4DspMixCh & 0x3FU)) || /*  Mix to L R */
		(0x4U == (u4DspMixCh & 0x3FU)) || /*  Mix to Ls */
		(0x5U == (u4DspMixCh & 0x3FU)) || /*  Mix to L Ls */
		(0x8U == (u4DspMixCh & 0x3FU)) || /*  Mix to Rs */
		(0xAU == (u4DspMixCh & 0x3FU)) || /*  Mix to R Rs */
		(0xCU == (u4DspMixCh & 0x3FU)) || /*  Mix to Ls Rs */
		(0xFU == (u4DspMixCh & 0x3FU)) || /*  Mix to L R Ls Rs */
		(0x10U == (u4DspMixCh & 0x3FU)) || /*  Mix to C */
		(0x11U == (u4DspMixCh & 0x3FU)) || /*  Mix to L C */
		(0x13U == (u4DspMixCh & 0x3FU)) || /*  Mix to L R C */
		(0x14U == (u4DspMixCh & 0x3FU)) || /*  Mix to Ls C */
		(0x15U == (u4DspMixCh & 0x3FU)) || /*  Mix to L Ls C */
		(0x1CU == (u4DspMixCh & 0x3FU)) || /*  Mix to Ls Rs C */
		(0x1FU == (u4DspMixCh & 0x3FU)) || /*  Mix to L R Ls Rs C */
		(0x20U == (u4DspMixCh & 0x3FU)) || /*  Mix to Sub */
		(0x22U == (u4DspMixCh & 0x3FU)) || /*  Mix to R Sub */
		(0x23U == (u4DspMixCh & 0x3FU)) || /*  Mix to L R Sub */
		(0x28U == (u4DspMixCh & 0x3FU)) || /*  Mix to Rs Sub */
		(0x2AU == (u4DspMixCh & 0x3FU)) || /*  Mix to R Rs Sub */
		(0x2CU == (u4DspMixCh & 0x3FU)) || /*  Mix to Ls Rs Sub */
		(0x2FU == (u4DspMixCh & 0x3FU)) || /*  Mix to L R Ls Rs Sub */
		(0x30U == (u4DspMixCh & 0x3FU)) || /*  Mix to C Sub */
		(0x33U == (u4DspMixCh & 0x3FU)) || /*  Mix to L R C Sub */
		(0x3CU == (u4DspMixCh & 0x3FU)) || /*  Mix to Ls Rs C Sub*/
		(0x3FU == (u4DspMixCh & 0x3FU))) { /*  Mix to L R Ls Rs C Sub */
	} else {
		pr_err("[PCM ERR]StrmProcDSPMix: Dsp Can not mix to Ch(0x%x)\r\n",
			(u32)u4DspMixCh);
		return INVALIDPRAM;
	}

	if (u4DspMixCh & 0x3U) {
		u4MixCh |= (DSP_MIX_FRONT_L_CH | DSP_MIX_FRONT_R_CH);
	}
	if (u4DspMixCh & 0xCU) {
		u4MixCh |= DSP_MIX_SURROUND_LSCH | DSP_MIX_SURROUND_RSCH;
	}
	if (u4DspMixCh & 0x10U) {
		u4MixCh |= DSP_MIX_CENTER_CH;
	}
	if (u4DspMixCh & 0x20U) {
		u4MixCh |= DSP_MIX_SUBWOOFER_CH;
	}

	if ((!(u4DspMixCh & 0x15U)) && (!(u4DspMixCh & 0x2AU))) {
		/* Mute L & R */
		m_fxpGain[0] = 0;
		m_fxpBTGain[0] = 0;
		m_fxpGain[1] = 0;
		m_fxpBTGain[1] = 0;
	} else if ((!(u4DspMixCh & 0x15U)) && (u4DspMixCh & 0x2AU)) {
		/* Mute L */
		m_fxpGain[0] = 0;
		m_fxpBTGain[0] = 0;
		m_fxpGain[1] = m_fxpLastGain[1];
		m_fxpBTGain[1] = m_fxpLastBTGain[1];
	} else if ((u4DspMixCh & 0x15U) && (!(u4DspMixCh & 0x2AU))) {
		/* Mute R */
		m_fxpGain[0] = m_fxpLastGain[0];
		m_fxpBTGain[0] = m_fxpLastBTGain[0];
		m_fxpGain[1] = 0;
		m_fxpBTGain[1] = 0;
	} else { /*((u4DspMixCh & 0x15) && (u4DspMixCh & 0x2A))*/
		m_fxpGain[0] = m_fxpLastGain[0];
		m_fxpBTGain[0] = m_fxpLastBTGain[0];
		m_fxpGain[1] = m_fxpLastGain[1];
		m_fxpBTGain[1] = m_fxpLastBTGain[1];
	}

	if (u4DspMixCh & 0x40U) {
		/* Mix to Spdif out */
		u4MixCh |= (DSP_MIX_CH9 | DSP_MIX_CH10);
		if ((!(u4DspMixCh & 0x15U)) && (!(u4DspMixCh & 0x2AU))) {
			m_fxpGain[0] = m_fxpLastGain[0];
			m_fxpBTGain[0] = m_fxpLastBTGain[0];
			m_fxpGain[1] = m_fxpLastGain[1];
			m_fxpBTGain[1] = m_fxpLastBTGain[1];
		}
	}
	pr_debug("[PCM]StrmProcDSPMix: u4DspMixCh=0x%x, u4MixCh=0x%x\r\n",
		(u32)u4DspMixCh, (u32)u4MixCh);
	DspMixOut_SetOutputCh(AUD_NORMAL_MODE, u4MixCh);
	pr_debug("[PCM]StrmProcDSPMix: m_fxpGain[0]=0x%x, m_fxpGain[1]=0x%x\r\n",
		m_fxpGain[0], m_fxpGain[1]);
	pr_debug("[PCM]StrmProcDSPMix: m_fxpBTGain[0]=0x%x, m_fxpBTGain[1]=0x%x\r\n",
		m_fxpBTGain[0], m_fxpBTGain[1]);

	return NOERR;
}

bool StrmProcIsCanDelete(const StreamProcess *strmProc)
{
	return (STATE_UNINIT == strmProc->m_u4State);
}

static s32 StrmProcStart(u32 u4StrmProcIdx)
{
	StrmProc_MSG_T rStrmProcMsg = {0};

	rStrmProcMsg.u4MsgID = STREAM_PROC_MSG_START;
	rStrmProcMsg.u4StrmProcID = u4StrmProcIdx;
	if (OSR_OK != x_msg_q_send(g_hStrmProcMsgQ, &rStrmProcMsg, sizeof(StrmProc_MSG_T), 1)) {
		pr_err("[PCM ERR]StrmProcStart: Send StrmProc Start msg fail!\r\n");
		return NORESOURCE;
	}

	x_event_set(g_hStrmProcEvent);


	pr_debug("[PCM]StrmProcStart: Start StrmProc.\r\n");

	return NOERR;
}

s32 StrmProcStop(u32 u4StrmProcIdx)
{
	StrmProc_MSG_T rStrmProcMsg = {0};

	rStrmProcMsg.u4MsgID = STREAM_PROC_MSG_STOP;
	rStrmProcMsg.u4StrmProcID = u4StrmProcIdx;
	if (OSR_OK != x_msg_q_send(g_hStrmProcMsgQ, &rStrmProcMsg, sizeof(StrmProc_MSG_T), 1)) {
		pr_err("[PCM ERR]StrmProcStop: Send StrmProc Stop msg fail!\r\n");
		return NORESOURCE;
	}

	x_event_set(g_hStrmProcEvent);

	pr_debug("[PCM]StrmProcStop: Stop StrmProc.\r\n");

	return NOERR;
}

u32 StateChangeInform(const struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	StreamProcess *strmProc = substrm_data->pSubstreamPro;

	if (SNDRV_PCM_TRIGGER_START == substrm_data->m_eState) {
		if (STATE_INITED == strmProc->m_u4State) {
			pr_debug("[PCM]StateChangeInform: Start ASRC(%d).\r\n",
				(s32)substrm_data->m_u4Idx);
			Asrc_Start(substrm_data->m_u4Idx);
			strmProc->m_u4State = STATE_STARTED;
		}
		pr_debug("[PCM]StateChangeInform: StrmProcStart(%d).\r\n",
			(s32)substrm_data->m_u4Idx);
		StrmProcStart(strmProc->m_u4Idx);
	} else if (SNDRV_PCM_TRIGGER_STOP == substrm_data->m_eState) {
		if (STATE_STARTED == strmProc->m_u4State) {
			pr_debug("[PCM]StateChangeInform: Stop ASRC(%d).\r\n",
				(s32)substrm_data->m_u4Idx);
			Asrc_Stop(substrm_data->m_u4Idx);
			strmProc->m_u4State = STATE_STOPPED;
		}
		pr_debug("[PCM]StateChangeInform: StrmProcStop(%d).\r\n",
			(s32)substrm_data->m_u4Idx);
		StrmProcStop(strmProc->m_u4Idx);
	} else {
		pr_debug("[PCM]StateChangeInform: substream(%d) state(%d)\r\n",
				(s32)substrm_data->m_u4Idx, (s32)substrm_data->m_eState);
	}

	return NOERR;
}

s32 PrepareStream(struct snd_pcm_substream *substream)
{
	ac_83xx *chip;
	struct snd_pcm_runtime *runtime;
	substream_data *substrm_data;
	StreamProcess *strmProc;

	if ((!substream) || (!substream->runtime) || (!substream->runtime->private_data)) {
		ASSERT(substream);
		pr_err("[PCM ERR]PrepareStream: Parameter Error.\r\n");
		return INVALIDPRAM;
	}
	chip = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;
	strmProc = substrm_data->pSubstreamPro;

	strmProc->m_rInFmt.u4BW = snd_pcm_format_width(runtime->format);
	if (runtime->channels == 2U) {
		strmProc->m_rInFmt.u4Chn = 2U;
	} else {
		strmProc->m_rInFmt.u4Chn = 1U;
	}

	strmProc->m_rInFmt.u4FS = runtime->rate;

	DspMixOut_GetBuffer(&(strmProc->m_rOutBuf));

	strmProc->m_u4MaxOutData = chip->m_u4IntrTime;
	strmProc->m_u4MaxInData = strmProc->m_u4MaxOutData;

	strmProc->m_u4MaxOutData *= strmProc->m_rOutFmt.u4FS;
	strmProc->m_u4MaxOutData /= 1000U;
	strmProc->m_u4MaxOutData *= strmProc->m_rOutFmt.u4BW / 8U;
	strmProc->m_u4MaxOutData *= MAX_SIZE_CAHCE_IN_OUTBUF;


	strmProc->m_u4MaxInData *= strmProc->m_rInFmt.u4FS;
	strmProc->m_u4MaxInData /= 1000U;
	strmProc->m_u4MaxInData *= strmProc->m_rInFmt.u4BW / 8U;
	strmProc->m_u4MaxInData *= MAX_SIZE_CAHCE_IN_INBUF;

	strmProc->m_prStream = substream;
	strmProc->m_u4SampleShift = substrm_data->dma_shift;
	strmProc->m_u4MiniBytes = 16U;
	strmProc->m_u4MiniBytes <<= strmProc->m_u4SampleShift;
	strmProc->m_u4BytesMask = 0xFFFFFFF0U << strmProc->m_u4SampleShift;
	strmProc->m_u4MiniOutLen = 48U * 30U * 2U;
	strmProc->m_u4OutLen = 0;
	strmProc->m_u4OutWP = 0;

	strmProc->m_u4State = STATE_INITED;
	strmProc->m_u4OState = STATE_INITED;
	pr_debug("[PCM]PrepareStream: successfully\r\n");

	return NOERR;
}

s32 AttachStream(struct snd_pcm_substream *substream, StreamProcess *strmProc)
{
	u32 u4Idx = 0;
	struct snd_pcm_runtime *runtime;
	substream_data *substrm_data;
	ASRC_CHS_FMT_T rAsrFmt;

	ASSERT(substream && strmProc);

	if (strmProc->m_prStream) {
		return INVALIDPRAM;
	}

	runtime = substream->runtime;
	substrm_data = runtime->private_data;

	rAsrFmt.u4Chn = runtime->channels;
	rAsrFmt.u4IFS = runtime->rate;
	rAsrFmt.u4IBW = DEF_DATA_BITS;
	rAsrFmt.u4OFS = strmProc->m_rOutFmt.u4FS;
	rAsrFmt.u4OBW = strmProc->m_rOutFmt.u4BW == 24 ? 24 : 16;
	pr_debug("[PCM]AttachStream: IFS(%d), OFS(%d), IBW(%d), OBW(%d), Channels(%d)\r\n",
		(s32)rAsrFmt.u4IFS, (s32)rAsrFmt.u4OFS, (s32)rAsrFmt.u4IBW, (s32)rAsrFmt.u4OBW, (s32)rAsrFmt.u4Chn);

	if (NOERR != AsrcMgr_AllocASRC(&rAsrFmt, substrm_data->IsBtSpeech, &u4Idx)) {
		pr_err("[PCM ERR]AttachStream: AsrcMgr_AllocASRC err.\r\n");
		return NORESOURCE;
	}
	substrm_data->m_u4Idx = u4Idx;
	strmProc->m_u4Idx = u4Idx;
	strmProc->m_prStream = substream;
	pr_debug("[PCM]AttachStream: successfully\r\n");

	return NOERR;
}

s32 DetachStream(struct snd_pcm_substream *substream)
{
	u32 u4Count = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	StreamProcess *strmProc = substrm_data->pSubstreamPro;

	if (strmProc->m_prStream != substream) {
		pr_err("[PCM ERR]DetachStream: m_prStream(0x%x) != substream(0x%x)\r\n",
			(u32)strmProc->m_prStream, (u32)substream);
		return INVALIDPRAM;
	}

	while (STATE_STARTED == strmProc->m_u4OState) {
		u4Count++;
		Sleep(1);
		if (5000U == u4Count) {
			pr_err("[PCM ERR]DetachStreamProc: Stream(%d) is in running state!\r\n",
				(s32)strmProc->m_u4Idx);
			return INVALIDSTATE;
		}
	}
	
	pr_debug("[PCM]DetachStreamProc: Start to detach.\r\n");
	Asrc_UnInit(strmProc->m_u4Idx);
	strmProc->m_prAsrc = NULL;
	strmProc->m_prStream = NULL;
	strmProc->m_u4State = STATE_UNINIT;
	strmProc->m_u4OState = STATE_UNINIT;
	strmProc->m_u4InputTotal = 0;
	strmProc->m_u4OutputTotal = 0;
	substrm_data->pSubstreamPro = NULL;

	pr_debug("[PCM]DetachStream: end.\r\n");

	return NOERR;
}

s32 EventInform(u32 u4Event, u32 u4Param, StreamProcess *strmProc)
{
	u32 u4Tmp = u4Param;

	if (STATE_STARTED != strmProc->m_u4State) {
		return NOERR;
	}

	switch (u4Event) {
	case EVT_UPDATE_PLAYED_SAMPLE:
		if (STATE_STARTED == strmProc->m_u4OState) {
			if (0 == strmProc->m_rOutFmt.u4FS) {
				pr_err("[PCM ERR]EventInform: error u4SampleRate = 0\r\n");
				return INVALIDPRAM;
			}
			u4Tmp *= strmProc->m_rInFmt.u4FS;
			u4Tmp += strmProc->m_u4Remain;
			strmProc->m_u4Remain = u4Tmp % (strmProc->m_rOutFmt.u4FS);
			u4Tmp /= strmProc->m_rOutFmt.u4FS;

			u4Param <<= 1U;
			if (strmProc->m_u4OutLen > u4Param) {
				strmProc->m_u4OutLen  -= u4Param;
			} else {
				strmProc->m_u4OutLen = 0;
			}

			ProcessOutput(strmProc);
			ProcessInput(strmProc);
		}
		break;

	case EVT_SRC_DATA_FINISH:
		ProcessOutput(strmProc);
		break;

	default:
		break;
	}

	return NOERR;
}

static s32 GetCurrentData(struct snd_pcm_runtime *runtime, PMIXER_DATA_T prData)
{
	u32 hw_pos;
	substream_data *substrm_data = runtime->private_data;

	prData->u4Chn = runtime->channels;
	prData->u4BitsPerSample = snd_pcm_format_width(runtime->format);
	prData->u4Used = 0;

	hw_pos = (u32)frames_to_bytes(runtime, (snd_pcm_sframes_t)(substrm_data->last_ptr - substrm_data->hw_Base));
	prData->u4Buffer = substrm_data->dma_start + hw_pos;

	return 0;
}

s32 DataAvailableInform(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	StreamProcess *strmProc = substrm_data->pSubstreamPro;

	if (strmProc->m_prStream != substream) {
		pr_err("[PCM ERR]DataAvailableInform: strmProc->m_prStream != substream\r\n");
		return INVALIDPRAM;
	}
	
	if ((STATE_STARTED == strmProc->m_u4State) && (strmProc->m_u4OutLen < strmProc->m_u4MiniOutLen)) {
		ProcessInput(strmProc);
	}

	return NOERR;
}

static void ProcessInput(StreamProcess *strmProc)
{
	u32 dwLGain, dwRGain, dwGainShift;
	u32 u4STime = 0;
	u32 u4AvaiBytes, u4AsrcBytes, u4IDataSz, u4ODataSz;
	u32 u4OldOff = 0;
	snd_pcm_sframes_t avail;
	WAVE_DATA_BUF_T rInBuf;
	struct snd_pcm_substream *substream = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	if (NULL == strmProc) {
		pr_err("[PCM ERR]ProcessInput: strmProc is NULL!\r\n");
		return;
	}

	substream = strmProc->m_prStream;
	if (NULL == substream) {
		pr_err("[PCM ERR]ProcessInput: substream is NULL!\r\n");
		return;
	}

	runtime = substream->runtime;
	if (NULL == runtime) {
		pr_err("[PCM ERR]ProcessInput: runtime is NULL!\r\n");
		return;
	}

	substrm_data = runtime->private_data;
	if (NULL == substrm_data) {
		pr_err("[PCM ERR]ProcessInput: substrm_data is NULL!\r\n");
		return;
	}

	u4STime = (u32)1000 * (u32)jiffies / (u32)HZ;

	if (NOERR != Asrc_GetODataSz(strmProc->m_u4Idx, &u4ODataSz)) {
		return;
	}

	u4AvaiBytes = strmProc->m_u4OutLen + u4ODataSz;
	if (u4AvaiBytes >= strmProc->m_u4MaxOutData) {
		return;
	}

	if (NOERR != Asrc_GetIDataSz(strmProc->m_u4Idx, &u4AvaiBytes)) {
		return;
	}
	if (u4AvaiBytes >= strmProc->m_u4MaxInData) {
		return;
	}

	if (NOERR != Asrc_GetOBuf(strmProc->m_u4Idx, &rInBuf)) {
		return;
	}

	if (rInBuf.u4DataSz >= (rInBuf.u4ChBufSz >> 1)) {
		pr_err("[PCM ERR]ProcessInput: rInBuf.u4DataSize = %d, rInBuf.u4BufSize/2 = %d\r\n",
			(s32)rInBuf.u4DataSz, (s32)(rInBuf.u4ChBufSz >> 1));
		return;
	}

	/*if (SetBtSPHOnly && (1 == substrm_data->IsBtSpeech)) {
		dwLGain = m_fxpBTGain[0];
		dwRGain = m_fxpBTGain[1];
		dwGainShift = m_dwBTGainShift;
	} else {
		dwLGain = m_fxpGain[0];
		dwRGain = m_fxpGain[1];
		dwGainShift = m_dwGainShift;
	}*/

	avail = (snd_pcm_sframes_t)GetPbAvailableBytes(runtime);
	u4AvaiBytes = (u32)frames_to_bytes(runtime, (snd_pcm_sframes_t)avail);

	if (u4AvaiBytes < strmProc->m_u4MiniBytes) {
		if (u4AvaiBytes && (!strmProc->m_u4OutLen)) {
			if (NOERR != Asrc_GetIBuf(strmProc->m_u4Idx, &rInBuf)) {
				return;
			}
			if ((rInBuf.u4ChBufSz - rInBuf.u4DataSz) <= ASRC_BUFFER_RESERVE_SIZE) {
				MIXER_DATA_T rStrmData;

				GetCurrentData(runtime, &rStrmData);
				pr_err("[PCM ERR]ProcessInput: discard data < 16 samples.\r\n");
			}
		}
		return;
	}

	if (NOERR != Asrc_GetIBuf(strmProc->m_u4Idx, &rInBuf)) {
		return;
	}

	u4OldOff = rInBuf.u4DataOff;

	u4AvaiBytes &= strmProc->m_u4BytesMask;

	if (NOERR != Asrc_GetIDataSz(strmProc->m_u4Idx, &u4IDataSz)) {
		return;
	}
	if ((rInBuf.u4DataSz + u4IDataSz) > strmProc->m_u4MaxInData) {
		rInBuf.u4DataSz = strmProc->m_u4MaxInData - u4IDataSz;
	}

	if (rInBuf.u4DataSz >= (rInBuf.u4ChBufSz >> 1)) {
		rInBuf.u4DataSz = rInBuf.u4ChBufSz >> 1;
	}
	rInBuf.u4DataSz &= 0xFFFFFFF0;
	u4AsrcBytes = u4AvaiBytes >> strmProc->m_u4SampleShift;
	u4AsrcBytes <<= 1U;
	if (rInBuf.u4DataSz >= u4AsrcBytes) {
		rInBuf.u4DataSz = u4AsrcBytes;
	} else {
		u4AvaiBytes = rInBuf.u4DataSz >> (u32)1;
		u4AvaiBytes <<= strmProc->m_u4SampleShift;
	}

	while (u4AvaiBytes) {
		MIXER_DATA_T rStrmData;
		u32 u4OnceOff = rInBuf.u4DataOff;
		s16 *pi2DstL, *pi2DstR;
		u32 hw_pos;

		pi2DstL = (s16 *)(rInBuf.u4Buf1 + rInBuf.u4DataOff);
		pi2DstR = (s16 *)(rInBuf.u4Buf2 + rInBuf.u4DataOff);
		GetCurrentData(runtime, &rStrmData);
		hw_pos = (u32)frames_to_bytes(runtime,
			(snd_pcm_sframes_t)(substrm_data->last_ptr - substrm_data->hw_Base));
		if ((substrm_data->buffer_size - hw_pos) < u4AvaiBytes) {
			rStrmData.u4Size = (substrm_data->buffer_size - hw_pos);
		} else {
			rStrmData.u4Size = u4AvaiBytes;
		}

		u4AsrcBytes = rStrmData.u4Size >> strmProc->m_u4SampleShift;
		u4AsrcBytes <<= 1U;
		u4AvaiBytes -= rStrmData.u4Size;

		if (rStrmData.u4BitsPerSample == 8) {
			u8 *pbData = (u8 *)rStrmData.u4Buffer;
			u32 u4Size = rInBuf.u4ChBufSz - u4OnceOff;

			if (u4Size > u4AsrcBytes) {
				u4Size = u4AsrcBytes;
			}
			rInBuf.u4DataOff += u4Size;
			ASSERT(!(u4Size & 0x01U));
			while (u4Size) {
				s32 i4Data, i4DataR;

				i4DataR = (s32)*pbData - 128;
				pbData++;
/*#if (!NO_GAIN_CTRL)
				i4Data = i4DataR * (s32)dwLGain;
				i4Data >>= (s32)dwGainShift - 8;
#else
				i4Data = i4DataR << 8;
#endif*/	
				i4Data = i4DataR;
				Limit_Sat(i4Data);
				*pi2DstL = (s16)i4Data;
				pi2DstL++;
				if (rStrmData.u4Chn == 2) {
					i4DataR = (s32)*pbData - 128;
					pbData++;
				}
				if (strmProc->m_rInFmt.u4Chn == 2) {
/*#if (!NO_GAIN_CTRL)
					i4Data = i4DataR * (s32)dwRGain;
					i4Data >>= (s32)dwGainShift - 8;
#else
					i4Data = i4DataR << 8;
#endif*/
					i4Data = i4DataR;
					Limit_Sat(i4Data);
					*pi2DstR = (s16)i4Data;
					pi2DstR++;
				}
				u4Size -= 2U;
			}

			if (u4AsrcBytes >= (rInBuf.u4ChBufSz - u4OnceOff)) {
				u4Size = u4AsrcBytes - (rInBuf.u4ChBufSz - u4OnceOff);
				rInBuf.u4DataOff = u4Size;
				pi2DstL = (s16 *)(rInBuf.u4Buf1);
				pi2DstR = (s16 *)(rInBuf.u4Buf2);
				ASSERT(!(u4Size & ((u32)0x01)));
				while (u4Size) {
					s32 i4Data, i4DataR;

					i4DataR = (s32)*pbData - 128;
					pbData++;
/*#if (!NO_GAIN_CTRL)
					i4Data = i4DataR * (s32)dwLGain;
					i4Data >>= (s32)dwGainShift - 8;
#else
					i4Data = i4DataR << 8;
#endif*/
					i4Data = i4DataR;
					Limit_Sat(i4Data);
					*pi2DstL = (s16)i4Data;
					pi2DstL++;
					if (rStrmData.u4Chn == 2) {
						i4DataR = (s32)*pbData - 128;
						pbData++;
					}
					if (strmProc->m_rInFmt.u4Chn == 2) {
/*#if (!NO_GAIN_CTRL)
						i4Data = i4DataR * (s32)dwRGain;
						i4Data >>= (s32)dwGainShift - 8;
#else
						i4Data = i4DataR << 8;
#endif*/
						i4Data = i4DataR;
						Limit_Sat(i4Data);
						*pi2DstR = (s16)i4Data;
						pi2DstR++;
					}
					u4Size -= 2;
				}
			}

		} else {
			s16 *pi2Data = (s16 *)rStrmData.u4Buffer;
			u32 u4Size = rInBuf.u4ChBufSz - u4OnceOff;

			if (u4Size > u4AsrcBytes) {
				u4Size = u4AsrcBytes;
			}
			rInBuf.u4DataOff += u4Size;
			ASSERT(!(u4Size & ((u32)0x01)));

			while (u4Size) {
				s32 i4Data, i4DataR;

				i4DataR = *pi2Data;
				pi2Data++;
/*#if (!NO_GAIN_CTRL)
				i4Data = i4DataR * (s32)dwLGain;
				i4Data >>= dwGainShift;
#else
				i4Data = i4DataR;
#endif*/
				i4Data = i4DataR;
				Limit_Sat(i4Data);
				*pi2DstL = (s16)i4Data;
				pi2DstL++;
				if (rStrmData.u4Chn == 2) {
					i4DataR = (s32)*pi2Data;
					pi2Data++;
				}
				if (strmProc->m_rInFmt.u4Chn == 2) {
/*#if (!NO_GAIN_CTRL)
					i4Data = i4DataR * (s32)dwRGain;
					i4Data >>= dwGainShift;
#else
					i4Data = i4DataR;
#endif*/
					i4Data = i4DataR;
					Limit_Sat(i4Data);
					*pi2DstR = (s16)i4Data;
					pi2DstR++;
				}
				u4Size -= 2;
			}
			if (u4AsrcBytes >= (rInBuf.u4ChBufSz - u4OnceOff)) {
				u4Size = u4AsrcBytes - (rInBuf.u4ChBufSz - u4OnceOff);
				rInBuf.u4DataOff = u4Size;
				pi2DstL = (s16 *)(rInBuf.u4Buf1);
				pi2DstR = (s16 *)(rInBuf.u4Buf2);
				ASSERT(!(u4Size & 0x01U));

				while (u4Size) {
					s32 i4Data, i4DataR;

					i4DataR = (s32)*pi2Data;
					pi2Data++;
/*#if (!NO_GAIN_CTRL)
					i4Data = i4DataR * (s32)dwLGain;
					i4Data >>= dwGainShift;
#else
					i4Data = i4DataR;
#endif*/
					i4Data = i4DataR;
					Limit_Sat(i4Data);
					*pi2DstL = (s16)i4Data;
					pi2DstL++;
					if (rStrmData.u4Chn == 2) {
						i4DataR = (s32)*pi2Data;
						pi2Data++;
					}
					if (strmProc->m_rInFmt.u4Chn == 2) {
/*#if (!NO_GAIN_CTRL)
						i4Data = i4DataR * (s32)dwRGain;
						i4Data >>= dwGainShift;
#else
						i4Data = i4DataR;
#endif*/
						i4Data = i4DataR;
						Limit_Sat(i4Data);
						*pi2DstR = (s16)i4Data;
						pi2DstR++;
					}
					u4Size -= 2;
				}
			}
		}

		step_hwptr(runtime, rStrmData.u4Size);
		substrm_data->Used_size += rStrmData.u4Size;
		if (substrm_data->Used_size >= substrm_data->period_size) {
			substrm_data->Used_size %= substrm_data->period_size;
			if (substrm_data->IsBtSpeech == 0) {
				snd_pcm_period_elapsed(substream);
			}
		}
	}

	if (u4OldOff != rInBuf.u4DataOff) {
		if (NOERR != Asrc_SetIWP(strmProc->m_u4Idx, rInBuf.u4DataOff)) {
			return;
		}

		strmProc->m_u4ProcInTime = (u32)1000 * (u32)jiffies / (u32)HZ - u4STime;
		if (strmProc->m_u4ProcInTime > 5) {
			pr_debug("[PCM]ProcessInput: cost %d ms.\r\n", (s32)strmProc->m_u4ProcInTime);
		}
	}
}

static struct snd_pcm_substream *ref_substream = NULL;
static int canCapture = 0;

u32 SetRefSubStream(struct snd_pcm_substream *substream){
	ref_substream = substream;
	return 0;
}

void CopyDataToRef(PWAVE_DATA_BUF_T prAsrcOut, u32 u4TranLen,StreamProcess *strmProc,substream_data *DLsubstrm_data)
{
	u32 u4size = 0;
	u32 hw_pos = 0;
	u32 hw_bytes = 0;
	s8 *m_lpCurrData;
	u32 i, j, u4RP, u4WP, au4TranLen[2], au4CopySz[2];
	u32 re_pos = 0;
	int dma_freeSize = 0;
	
	u4RP = prAsrcOut->u4DataOff;
	if (u4TranLen >= prAsrcOut->u4ChBufSz - prAsrcOut->u4DataOff) {
		au4TranLen[0] = prAsrcOut->u4ChBufSz - prAsrcOut->u4DataOff;
		au4TranLen[1] = u4TranLen - au4TranLen[0];
		prAsrcOut->u4DataOff = au4TranLen[1];
	} else {
		au4TranLen[0] = u4TranLen;
		au4TranLen[1] = 0;
		prAsrcOut->u4DataOff += au4TranLen[0];
	}

	if(DLsubstrm_data->IsBtSpeech == 1)
	{
        down(&g_prSpeechDev.m_refLock);
    	if(ref_substream == NULL || strmProc->m_u4BufState != BUF_STATE_HAS_DATA)
        {
            up(&g_prSpeechDev.m_refLock);
            return;
        }
	    struct snd_pcm_runtime *runtime = ref_substream->runtime;
	    substream_data *substrm_data = runtime->private_data;
	    atc_capture_stream *capture_stream = (atc_capture_stream *)substrm_data;
	
	    if(substrm_data->m_eState != SNDRV_PCM_TRIGGER_START){
            up(&g_prSpeechDev.m_refLock);
		    return;
	    }
	
	    hw_pos = (u32)frames_to_bytes(runtime, (snd_pcm_sframes_t)(substrm_data->last_ptr - substrm_data->hw_Base));
	    m_lpCurrData = (s8 *)(substrm_data->dma_start + hw_pos);
	    re_pos = hw_pos;

	    if((!m_lpCurrData)) {
		    pr_debug("m_lpCurrData null,substrm_data->m_eState:%d\n",substrm_data->m_eState);
            up(&g_prSpeechDev.m_refLock);
		    return 0;
	    }
	
	    dma_freeSize = (runtime->status->hw_ptr > runtime->control->appl_ptr)?(runtime->status->hw_ptr - runtime->control->appl_ptr):(substrm_data->buffer_size-runtime->control->appl_ptr+runtime->status->hw_ptr);

	    for (i = 0; (i < 2U) && (au4TranLen[i] > 0); i++) {
		    s16 *pi2Src = (s16 *)(prAsrcOut->u4Buf1 + u4RP);

		    u4WP = hw_pos;
		    if (au4TranLen[i] >= substrm_data->buffer_size - hw_pos) {
			    au4CopySz[0] = substrm_data->buffer_size - hw_pos;
			    au4CopySz[1] = au4TranLen[i] - au4CopySz[0];
			    hw_pos = au4CopySz[1];
		    } else {
			    au4CopySz[0] = au4TranLen[i];
			    au4CopySz[1] = 0;
			    hw_pos += au4CopySz[0];
		    }

		    for (j = 0; (j < 2U) && (au4CopySz[j] > 0); j++) {
			    s16 *pi2Dst = (s16 *)(substrm_data->dma_start + u4WP);
			    while (au4CopySz[j]) {
				    *pi2Dst = *pi2Src;
				    pi2Src++;
				    pi2Dst++;
				    au4CopySz[j] -= 2;

				    substrm_data->Used_size += 2;
				    hw_bytes += 2;

				    if(substrm_data->Used_size >= substrm_data->period_size) {
					    substrm_data->Used_size %= substrm_data->period_size;
					    step_hwptr(runtime, hw_bytes);
					    hw_bytes = 0;
					    if(ref_substream != NULL && substrm_data->m_eState == SNDRV_PCM_TRIGGER_START){
					    snd_pcm_period_elapsed(ref_substream);
				        }
			        }
			    }
			    u4WP = 0;
		    }
		    u4RP = 0;
	    }
	
	    step_hwptr(runtime, hw_bytes);
        up(&g_prSpeechDev.m_refLock);
    }
}

void OutputDataCopy(StreamProcess *strmProc, PWAVE_DATA_BUF_T prAsrcOut, u32 u4TranLen,substream_data *substrm_data)
{
	u32 u4MuteChn = 0;
	u32 u4LGain = 0;
	u32 u4RGain = 0;
	u32 dwLGain, dwRGain, dwGainShift;
	u32 i, j, u4RP, u4WP, au4TranLen[2], au4CopySz[2];
	s32 i4Data;
	MIXER_DATA_T rStrmData;
	struct snd_pcm_runtime *runtime = NULL;

    /* change volume smoothly */
    u32 needSmoothL = 0;
    u32 needSmoothR = 0;
    static int oldGainL = 0;
    static int oldGainR = 0;
    int newGainL = 0;
    int newGainR = 0;
    int stepGainL = 0;  // signed int, step gain may be less than 0
    int stepGainR = 0;
    int realGainL = 0;
    int realGainR = 0;

    int attenuationSize = 0;

	if (NULL == substrm_data) {
		pr_err("[PCM ERR]ProcessInput: substrm_data is NULL!\r\n");
		return;
	}
	
	runtime = substrm_data->substream->runtime;
	
	if (SetBtSPHOnly && (1 == substrm_data->IsBtSpeech)) {
		dwLGain = m_fxpBTGain[0];
		dwRGain = m_fxpBTGain[1];
		dwGainShift = m_dwBTGainShift;
	} else {
		dwLGain = m_fxpGain[0];
		dwRGain = m_fxpGain[1];
		dwGainShift = m_dwGainShift;
	}

	u4MuteChn = 0;
	u4LGain = (u4MuteChn & MUTE_LEFT_CH) ? 0 : 1;
	u4RGain = (u4MuteChn & MUTE_RIGHT_CH) ? 0 : 1;

	GetCurrentData(runtime, &rStrmData);
	
	u4RP = prAsrcOut->u4DataOff;
	if (u4TranLen >= prAsrcOut->u4ChBufSz - prAsrcOut->u4DataOff) {
		au4TranLen[0] = prAsrcOut->u4ChBufSz - prAsrcOut->u4DataOff;
		au4TranLen[1] = u4TranLen - au4TranLen[0];
		//prAsrcOut->u4DataOff = au4TranLen[1];
		/*if(BUF_STATE_NO_FILL == strmProc->m_u4BufState){
		prAsrcOut->u4DataOff = au4TranLen[1];
		}*/
	} else {
		au4TranLen[0] = u4TranLen;
		au4TranLen[1] = 0;
		//prAsrcOut->u4DataOff += au4TranLen[0];
		/*if(BUF_STATE_NO_FILL == strmProc->m_u4BufState){
			prAsrcOut->u4DataOff = au4TranLen[1];
		}*/
	}

    if (SetBtSPHOnly && (1 == substrm_data->IsBtSpeech)) {
        /* smooth volume */
        if (u4TranLen >= 256) {
            attenuationSize = 128;  // use 128 samples to do volume-smoothing
        } else {
            attenuationSize = u4TranLen / 2;
        }

        newGainL = dwLGain;
        newGainR = dwRGain;

        if ((newGainL != oldGainL) || (newGainR != oldGainR)) {
            stepGainL = (newGainL - oldGainL) / attenuationSize;
            stepGainR = (newGainR - oldGainR) / attenuationSize;
            if (0 == stepGainL) {
                needSmoothL = 0; // delta gain is small, no need to do smooth
            } else {
                needSmoothL = 1;
            }

            if (0 == stepGainR) {
                needSmoothR = 0;
            } else {
                needSmoothR = 1;
            }
        }

        realGainL = oldGainL;
        realGainR = oldGainR;

        oldGainL = newGainL;
        oldGainR = newGainR;
        /* end of smoothing */
    } else {
        needSmoothL = 0;
        needSmoothR = 0;
    }
    
	for (i = 0; (i < 2U) && (au4TranLen[i] > 0); i++) {
		s16 *pi2SrcL = (s16 *)(prAsrcOut->u4Buf1 + u4RP);
		s16 *pi2SrcR = (1 == strmProc->m_rInFmt.u4Chn) ?
			pi2SrcL : (s16 *)(prAsrcOut->u4Buf2 + u4RP);

		u4WP = strmProc->m_u4OutWP;
		if (au4TranLen[i] >= strmProc->m_rOutBuf.u4ChBufSz - strmProc->m_u4OutWP) {
			au4CopySz[0] = strmProc->m_rOutBuf.u4ChBufSz - strmProc->m_u4OutWP;
			au4CopySz[1] = au4TranLen[i] - au4CopySz[0];
			strmProc->m_u4OutWP = au4CopySz[1];
		} else {
			au4CopySz[0] = au4TranLen[i];
			au4CopySz[1] = 0;
			strmProc->m_u4OutWP += au4CopySz[0];
		}

        /* Make volume change smoothly, add by ATC6112 */
        
        /* End of smoothing */

		for (j = 0; (j < 2U) && (au4CopySz[j] > 0); j++) {
			s16 *pi2DstL = (s16 *)(strmProc->m_rOutBuf.u4VirSAdr + u4WP);
			s16 *pi2DstR = (1 == strmProc->m_rOutBuf.u4Chn) ? pi2DstL :
				(s16 *)(strmProc->m_rOutBuf.u4VirSAdr + strmProc->m_rOutBuf.u4ChBufSz + u4WP);
			u32 flush_length = au4CopySz[j];
			while (au4CopySz[j]) {
				if (rStrmData.u4BitsPerSample == 8) {
					i4Data = (s32)(*pi2DstL + ((*pi2SrcL * (s32)dwLGain)>>dwGainShift-8));
				pi2SrcL++;
				*pi2DstL = Sat_16BIT(i4Data);
				pi2DstL++;

					i4Data = (s32)(*pi2DstR + ((*pi2SrcR * (s32)dwRGain)>>dwGainShift-8));
				pi2SrcR++;
				*pi2DstR = Sat_16BIT(i4Data);
				pi2DstR++;

				au4CopySz[j] -= 2;
					
				}else{
				    if (needSmoothL && (attenuationSize > 0)) {
                        realGainL += stepGainL;
                        attenuationSize --;
                    } else {
                        realGainL = dwLGain; 
                    }

                    if (needSmoothR && (attenuationSize > 0)) {
                        realGainR += stepGainR;
                        attenuationSize --;
                    } else {
                        realGainR = dwRGain; 
                    }
                    
					i4Data = (s32)(*pi2DstL + ((*pi2SrcL * (s32)realGainL)>>dwGainShift));
					pi2SrcL++;
					*pi2DstL = Sat_16BIT(i4Data);
					pi2DstL++;

					i4Data = (s32)(*pi2DstR + ((*pi2SrcR * (s32)realGainR)>>dwGainShift));
					pi2SrcR++;
					*pi2DstR = Sat_16BIT(i4Data);
					pi2DstR++;

					au4CopySz[j] -= 2;
				}
			}
			__flush_dcache_area((void*)(strmProc->m_rOutBuf.u4VirSAdr + u4WP), flush_length);
			
			if(2 == strmProc->m_rOutBuf.u4Chn){
				__flush_dcache_area((void*)(strmProc->m_rOutBuf.u4VirSAdr + strmProc->m_rOutBuf.u4ChBufSz + u4WP), flush_length);
			}
			u4WP = 0;	/*for Aout rollback*/
		}
		u4RP = 0;	/*for Asrc rollback*/
	}
}

static void ProcessOutput(StreamProcess *strmProc)
{
	WAVE_DATA_BUF_T rAsrcOut;
	u32 u4OldOff = 0;
	u32 u4TranLen = 0;
	u32 u4STime = (u32)1000 * (u32)jiffies / (u32)HZ;
	struct snd_pcm_substream *substream = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	if (NULL == strmProc) {
		pr_err("[PCM ERR]ProcessOutput: strmProc is NULL!\r\n");
		return;
	}

	substream = strmProc->m_prStream;
	if (NULL == substream) {
		pr_err("[PCM ERR]ProcessOutput: substream is NULL!\r\n");
		return;
	}

	runtime = substream->runtime;
	if (NULL == runtime) {
		pr_err("[PCM ERR]ProcessOutput: runtime is NULL!\r\n");
		return;
	}

	substrm_data = runtime->private_data;
	if (NULL == substrm_data) {
		pr_err("[PCM ERR]ProcessOutput: substrm_data is NULL!\r\n");
		return;
	}

	if (strmProc->m_u4OutLen >= AUD_STRM_OUT_MAX_LEN) {
		goto EXIT;
	}

	if (NOERR != Asrc_GetOBuf(strmProc->m_u4Idx, &rAsrcOut)) {
		goto EXIT;
	}

	u4OldOff = rAsrcOut.u4DataOff;
	u4TranLen = rAsrcOut.u4DataSz;

	if (u4TranLen) {
		u32 u4OldOutLen = strmProc->m_u4OutLen;

		if (!strmProc->m_u4OutLen) {
			strmProc->m_u4OutWP = DspMixOut_GetNextRP(&strmProc->m_u4OutLen);
			if (BUF_STATE_NO_FILL != strmProc->m_u4BufState) // Ingore first times.
			{
				if (!strmProc->m_u4UnderrunCount)
				{
				    // Output error message.
				    // Stream buffer is underrun first times.
					pr_err("[PCM ERR] stream(%d) bt(%d) underrun!!! (%d) ms\r\n",
						(s32)strmProc->m_u4Idx, substrm_data->IsBtSpeech, (s32)(1000 * jiffies / HZ));
				}
				strmProc->m_u4UnderrunCount ++;
			}
		}
		else
		{
			if (strmProc->m_u4UnderrunCount)
			{
				// Output error message.
				// Stream buffer underrun count.
				pr_err("[PCM ERR] stream(%d) bt(%d) underrun count(%d) (%d) ms\r\n",
					(s32)strmProc->m_u4Idx, substrm_data->IsBtSpeech, strmProc->m_u4UnderrunCount, 
					(s32)(1000 * jiffies / HZ));
				strmProc->m_u4UnderrunCount = 0;
			}
		}

		if (u4TranLen > ((u32)AUD_STRM_OUT_MAX_LEN - strmProc->m_u4OutLen)) {
			u4TranLen = (u32)AUD_STRM_OUT_MAX_LEN - strmProc->m_u4OutLen;
		}

		if (u4TranLen > (strmProc->m_rOutBuf.u4DataSize - strmProc->m_u4OutLen)) {
			u4TranLen = strmProc->m_rOutBuf.u4DataSize - strmProc->m_u4OutLen;
		}
		
		if(ref_substream != NULL&& substrm_data->IsBtSpeech == 1)
		{
			struct snd_pcm_runtime *runtime_l = ref_substream->runtime;
			substream_data *substrm_data_l = runtime_l->private_data;
			atc_capture_stream *capture_stream = (atc_capture_stream *)substrm_data_l;

			u32 dma_freeSize = (substrm_data_l->last_ptr >= runtime_l->control->appl_ptr)?(substrm_data_l->buffer_size+runtime_l->control->appl_ptr-substrm_data_l->last_ptr):(runtime_l->control->appl_ptr - substrm_data_l->last_ptr);
			dma_freeSize = (u32)frames_to_bytes(runtime_l, (snd_pcm_sframes_t)dma_freeSize);
			if(u4TranLen > dma_freeSize)u4TranLen = dma_freeSize;
		}
		
		u4TranLen &= 0xFFFFFF80U;/* 128 bytes alignment*/

		if (!u4TranLen) {
			strmProc->m_u4OutLen = u4OldOutLen;
			goto EXIT;
		}

		strmProc->m_u4OutLen += u4TranLen;
		strmProc->m_u4OutputTotal += u4TranLen;

		OutputDataCopy(strmProc, &rAsrcOut, u4TranLen,substrm_data);

		DspMixOut_UpdateWP(strmProc->m_u4OutWP);
		CopyDataToRef(&rAsrcOut, u4TranLen,strmProc,substrm_data);
		
		if (NOERR != Asrc_SetORP(strmProc->m_u4Idx, rAsrcOut.u4DataOff)) {
			goto EXIT;
		}

		if (AUD_STATE_STARTED != strmProc->m_u4OState) {
			if (NOERR != DspMixOut_Start()) {
				goto EXIT;
			}
			strmProc->m_u4OState = AUD_STATE_STARTED;
		}
		
	}

	if (strmProc->m_u4OutLen) {
		 if (BUF_STATE_NO_FILL == strmProc->m_u4BufState) {
			if (substrm_data->IsBtSpeech) {
				SpeechDev_SynchronizationEx();
			}
		}
		strmProc->m_u4BufState = BUF_STATE_HAS_DATA;
	} else if (BUF_STATE_HAS_DATA == strmProc->m_u4BufState) {
		strmProc->m_u4BufState = BUF_STATE_EMPTY;
	}

	u4STime = (u32)1000 * (u32)jiffies / (u32)HZ - u4STime;
	if (u4STime > 10U) {
		pr_debug("[PCM]ProcessOutput: cost %d ms. \r\n", (s32)u4STime);
	}

EXIT:
	return;
}


