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

#ifndef _HDMI_RX_AUD_IF_H_
#define _HDMI_RX_AUD_IF_H_

#include "hdmi_rx_ctrl.h"
#include "drv_aud.h"

typedef enum {
	RX_SW_MCLK_128FS,
	RX_SW_MCLK_256FS,
	RX_SW_MCLK_384FS,
	RX_SW_MCLK_512FS
} RX_SW_MCLK_FREQUNECY_T;


typedef enum {
	FORMAT_RJ = 0,
	FORMAT_LJ = 1,
	FORMAT_I2S = 2
} RX_AUD_DATA_FORMAT_T;


typedef SAMPLE_FREQUENCY_T RX_AUD_MCLK_FREQUENCY_T;
typedef LRCK_CYC_T RX_AUD_LRCK_CYC_T;
typedef DAC_DATA_NUMBER_T RX_AUD_DAC_DATA_NUMBER_T;


typedef enum {
	LRCK_EDGE_FALL,
	LRCK_EDGE_RISING,
} RX_AUD_LRCK_EDGE_T;

/* Audio ain configuration table. */
typedef struct {
	AUD_DRV_STREAM_FROM_T eStrSrc;      /* < Stream source */
	RX_AUD_DATA_FORMAT_T      eFormat;         /* < format of alignment */
	RX_AUD_DAC_DATA_NUMBER_T  eBits;           /* < number of bits per sample */
	RX_AUD_LRCK_CYC_T         eCycle;          /* < cycles per sample */
	RX_AUD_MCLK_FREQUENCY_T   eSampleFreq;     /* < DAC sampling frequence */
	BOOL               fgDataInvert;    /* < Invert audio output for OP phase */
	BOOL               fgLRInvert;      /* < Invert L/R audio output */
	/* ======================================================================== */
	UINT8 uFormat;                      /* < format of alignment */
	UINT8 uBits;                        /* < number of bits per sample */
	UINT8 uCycle;                       /* < cycles per sample */
	BOOL  fgIsSPDIFin;                  /* < TRUE is slave mode */
	BOOL  fgLrckInv;                    /* < TRUE while LRCK Low is Left channel */
} RX_AUD_AIN_CFG_T;

#endif

