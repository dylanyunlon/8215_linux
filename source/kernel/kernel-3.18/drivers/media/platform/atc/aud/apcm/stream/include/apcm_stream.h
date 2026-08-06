/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of AutoChips Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AutoChips SOFTWARE") RECEIVED
 *     FROM AutoChips AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. AutoChips EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES AutoChips PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE AutoChips SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AutoChips SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY AutoChips SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND AutoChips'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE AutoChips SOFTWARE RELEASED HEREUNDER WILL BE, AT AutoChips'S OPTION,
 *     TO REVISE OR REPLACE THE AutoChips SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO AutoChips FOR SUCH AutoChips SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/******************************************************************************
*[File]             apcm_stream.h
*[Author]           atc6013
*[Description]
*
******************************************************************************/
#ifndef __APCM_STREAM_H_
#define __APCM_STREAM_H_

#include <windows.h>
#include <sound/pcm.h>
#include "oemsettings.h"

#include "apcm_hw.h"
#include "apcm_btcall.h"
#ifdef ATC_AOSP_ENHANCEMENT_CTS
#define STRM_TMP_BUF_SZ         4800
#else
#define STRM_TMP_BUF_SZ         9600
#endif

typedef enum {
	STRM_IN_START = 0,

	STRM_IN_SPH_MIC = STRM_IN_START,
	STRM_IN_SPH_REF,
	STRM_IN_SPH_RX,
	STRM_IN_MIC,

	STRM_IN_MAX,

	STRM_OUT_START = STRM_IN_MAX,

	STRM_OUT_SPH_TX = STRM_OUT_START,
	STRM_OUT_SPH_RX,
	STRM_OUT_RESERVE,

	STRM_OUT_NORMAL,
	STRM_OUT_NORMAL2,
	STRM_OUT_NORMAL3,
	STRM_OUT_NORMAL4,

	STRM_OUT_MAX,

	STRM_NUM = STRM_OUT_MAX,

} strm_type_e;

#define IS_REC_STREAM(idx)	(idx >= STRM_IN_START  && idx < STRM_IN_MAX)
#define IS_PB_STREAM(idx)	(idx >= STRM_OUT_START && idx < STRM_OUT_MAX)


typedef struct
{
	struct snd_pcm_substream *substream;

	volatile u32 state;
	u32 type;
	char name[20];

	u32 channels;
	u32 fs;
	u32 buffer_frames;
	u32 period_size;
	u32 used_size;

	apcm_filebuf_t *strm_buf;
	apcm_buf_t *proc_buf;
	void *hw;

	apcm_sema_t stop_sema;
	apcm_thread_t *thread;

} stream_t;


void stream_init(void);

stream_t *stream_open(struct snd_pcm_substream *substream, u32 type);
stream_t *stream_close(stream_t *this);

void stream_prepare(stream_t *this);

void stream_start(stream_t *this);
void stream_stop(stream_t *this);

u32 stream_get_ptr(stream_t *this);


#endif // __APCM_STREAM_H_


