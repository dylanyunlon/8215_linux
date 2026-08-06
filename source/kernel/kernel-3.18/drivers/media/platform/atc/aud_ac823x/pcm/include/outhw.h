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

#ifndef OUT_HW_H
#define OUT_HW_H


#include <ceddk.h>
#include "audiosys.h"
#include "aud_if_hw_asrc.h"
#include "oemsettings.h"
#include "GpsMix_if.h"
#include "drv_thread.h"


#define DSPMIX_WAIT_TIMEOUT			20U

typedef enum {
	AUD_NORMAL_MODE = 0,
	AUD_BT_CALL_MODE,
	AUD_OUTPUT_MODE_NUM,
} AUD_OUTPUT_MODE;

#if CONFIG_DRV_AUD_AC83XX
typedef enum {
	DSP_MIX_FRONT_LR_CH =	(u32)1 << 0,
	DSP_MIX_SURROUND_CH =	(u32)1 << 1,
	DSP_MIX_CENTER_CH =		(u32)1 << 2,
	DSP_MIX_SUBWOOFER_CH =	(u32)1 << 3,
	DSP_MIX_CH910 =			(u32)1 << 4,
	DSP_MIX_CH1112 =		(u32)1 << 5,
	DSP_MIX_CH78 =			(u32)1 << 6,
} DSP_MIX_OUTPUT_CH;

typedef struct {
	union {
	u32 u4Value;
	struct {
		u32 u4DspMixEnable		:1;		/* bit 0 DSP mix enable */
		u32 u4DspMixFrontLRCH	:1;		/* bit1 Front L/R ch mix config */
		u32 u4DspMixSurroundCH	:1;		/* bit 2 surround ch mix config */
		u32 u4DspMixCenterCH		:1;		/* bit 3  center ch mix config */
		u32 u4DspMixCh78			:1;		/* bit 4  ch 7/8 */
		u32 u4DspMixCh910		:1;		/* bit 5  ch 9/10 */
		u32 u4DspMixCh1112		:1;		/* bit 6  ch 11/12 */
		u32 u4DspMixSubwooferCH	:1;		/* bit 7   subwoofer ch mix config */
		u32 u4Reserve			:24;	/* bit [8~31] */
	};
	} Union;
} DSP_MIX_CFG;
#else

typedef enum {
	DSP_MIX_FRONT_L_CH =	(u32)1 << 0,
	DSP_MIX_SURROUND_LSCH =	(u32)1 << 1,
	DSP_MIX_CENTER_CH =		(u32)1 << 2,
	DSP_MIX_CH7 =			(u32)1 << 3,
	DSP_MIX_CH9 =			(u32)1 << 4,
	DSP_MIX_CH11 =		    (u32)1 << 5,
	DSP_MIX_SUBWOOFER_CH =	(u32)1 << 6,
	DSP_MIX_FRONT_R_CH =	(u32)1 << 8,
	DSP_MIX_SURROUND_RSCH = (u32)1 << 9,
	DSP_MIX_CH8 =			(u32)1 << 10,
	DSP_MIX_CH10 =			(u32)1 << 11,
	DSP_MIX_CH12 =		    (u32)1 << 12,	
} DSP_MIX_OUTPUT_CH;

typedef struct {
	union {
	u32 u4Value;
	struct {
		u32 u4DspMixEnable		:1;		/* bit 0 DSP mix enable */
		u32 u4DspMixFrontLCH	:1;		/* bit1 Front L ch mix config */
		u32 u4DspMixSurroundLSCH	:1;		/* bit 2 LS mix config */
		u32 u4DspMixCenterCH		:1;		/* bit 3  center ch mix config */
		u32 u4DspMixCh7			:1;		/* bit 4  ch 7 */
		u32 u4DspMixCh9		    :1;		/* bit 5  ch 9 */
		u32 u4DspMixCh11		:1;		/* bit 6 ch 12 */
		u32 u4DspMixSubwooferCH	:1;		/* bit 7   subwoofer ch mix config */
		u32 u4DspMixBit8        :1;		
		u32 u4DspMixFrontRCH    :1;      /* bit9 Front R ch mix config */
		u32 u4DspMixSurroundRSCH :1;     /* bit 10 RS mix config */
		u32 u4DspMixCh8			:1;		/* bit 11  ch 8 */
		u32 u4DspMixCh10		:1;		/* bit 12  ch 10 */
		u32 u4DspMixCh12		:1;		/* bit 13  ch 12 */
		u32 u4Reserve			:19;	/* bit [14~31] */
	};
	} Union;
} DSP_MIX_CFG;

#endif

typedef struct _Dsp_Mix_Out_ {
	u32 m_u4IntrNum;
	u32 m_u4EmptyNum;

	u32 m_u4State;
	PCMFMT_T m_rFmt;

	AUD_DATA_BUF_T m_rOutBuf;
	uintptr_t m_u4VirAddr;
	u32 m_u4MemSize;

	u32 m_u4WP;
	u32 m_u4RP;

	DSP_MIX_CFG m_rDspMixCfg[AUD_OUTPUT_MODE_NUM];
	AUD_OUTPUT_MODE m_eCurUsedMode;

	u32 m_u4CbTime; /* Interrupt interval (miniseconds) */
	PFN_ISR_CB m_pfnCb;
	u32 m_u4CbParam;
	bool m_fgHibernated;
} DspMixOut;


extern uintptr_t g_hStrmProcMsgQ;
extern void*	g_hStrmProcEvent;

s32 DspMixOut_Init(void);
u32 DspMixOut_UnInit(void);

void DspMixOut_Setup(void);
s32 DspMixOut_Start(void);
s32 DspMixOut_Stop(void);
void DspMixOut_HibernationCtrl(bool fgWakeUp);

u32 DspMixOut_SetFormat(const PCMFMT_T *prFmt);
u32 DspMixOut_GetFormat(PCMFMT_T *prFmt);


u32 DspMixOut_initOutputCh(void);
u32 DspMixOut_SetOutputCh(AUD_OUTPUT_MODE eAudOutPutMode, u32 u4Ch);
u32 DspMixOut_SwitchOutPut(AUD_OUTPUT_MODE eAudOutPutMode);

s32 DspMixOut_initBuf(void);
u32 DspMixOut_setBuffer(void);
s32 DspMixOut_GetBuffer(AUD_DATA_BUF_T *prBuffer);

u32 DspMixOut_GetRP(void);
u32 DspMixOut_GetNextRP(u32 *pu4SLen);
u32 DspMixOut_UpdateWP(u32 u4WP);
void DspMixNotifyEvent(void);

u32 DspMixOut_InterruptThread(void);
u32 DspMixOut_resetMem(u32 u4Rptr);

u32 AudioOut_RegISTCB(PFN_ISR_CB pfnCb, u32 u4Param, u32 u4CbTime);

#endif
