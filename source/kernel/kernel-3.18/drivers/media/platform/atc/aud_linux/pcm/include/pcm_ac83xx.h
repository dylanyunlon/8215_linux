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



#ifndef PCM_AC83XX_H
#define PCM_AC83XX_H

#include "audiosys.h"
#include "virtual_mic.h"
#include <sound/pcm.h>
#include "strmproc.h"
#include "speechdev.h"
#include "aud_pcm_dbg.h"
#include "aud_oal.h"
#include "x_ver.h"


#define MAX_PLAYBACK_STREAMS 3U

#define ALSA_MOD_NAME        TEXT("ATC_ALSA")  //Moudle name
#define ALSA_VER_MAIN        01           //main version
#define ALSA_VER_MINOR       01           //minor version, Large feature tens add 1 and clear unit,
                                         //other feature and add new function need add 1.    
#define ALSA_VER_REV         03           //version number, you should add 1 when check in aud

typedef struct {
	s32 irq;
	u8 revision;

	bool m_Intialized;
	struct snd_card *card;
	struct snd_pcm *pcm;

	StreamProcess *m_prSPHead; /* playback stream->StreamProcess list */

	u32 m_pbu4State; /* playback stream state */
	u32 m_u4StartCount;
	u32 m_u4IntrTime; /* Interrupt interval (miniseconds) */
	u32 m_u4IntrSamples;

	u32 m_capu4State;	 /* capture stream state */

	struct mutex m_HeadLock;
} ac_83xx;

enum PCM_TYPE {
	PCM_TYPE_M8,
	PCM_TYPE_M16,
	PCM_TYPE_S8,
	PCM_TYPE_S16
};


typedef struct {
	u8 IsBtSpeech;
	ac_83xx *ac83xx_chip;
	struct snd_pcm_substream *substream;
	u32 m_u4Idx;
	u32 m_u4MicRP;
	bool m_fgPrepare;
	u8 mprepare;	/* 0=open(),1=prepare */
	StreamProcess *pSubstreamPro;
	u32 m_eState; /* runtime->status->state,substream_status */
	u32 dma_start;
	u32 dma_size;
	u32 dma_shift;
	u32 buffer_size;
	u32 period_size;

	volatile u32 last_ptr;    /* read pos inframes */
	volatile u32 real_last_ptr;    /* read pos inframes to HW Buffer */
	volatile u32 appl_ptr;    /* app write pos inframes */
	u32	hw_Base;		   /* hw read pos base inframes */
	u32	real_hw_Base;		   /* hw read pos base inframes */
	u32	app_Base;		   /* app write pos base inframes */
    u32 hw_ofs;
	u32 Used_size;   /* has read_size inbytes */
	u32 boundary;	  /* boundary inframes */
	struct semaphore rThreadExitSema;

	enum PCM_TYPE m_SampleType; /* capture sampletype */
} substream_data;

typedef struct {
	substream_data atcstream;

	/* Private data for record stream*/
	s32                CopyMicDataThread_exit;
	s32                CopyMicDataThread_wq_flag;
	struct task_struct *CopyMicDataThread_task;
	wait_queue_head_t  CopyMicDataThread_wq;

	PVirtualMicIn prMic;

} atc_capture_stream;



extern ac_83xx *snd_chip;

static inline u32 step_appptr(struct snd_pcm_runtime  *runtime, u32 byteCount)
{
	substream_data *substrm_data = runtime->private_data;
	u32 app_ptr_tmp = substrm_data->appl_ptr;
	u32 buffer_size_frames = (u32)bytes_to_frames(runtime, substrm_data->buffer_size);

	app_ptr_tmp += (u32)bytes_to_frames(runtime, (int32_t)byteCount);

	if(app_ptr_tmp >= substrm_data->app_Base + buffer_size_frames) {
		substrm_data->app_Base += buffer_size_frames;
	}

	/* app_base & appl_ptr return 0 ... (boundary - 1) */
	if(substrm_data->app_Base >= substrm_data->boundary) {
		substrm_data->app_Base = 0;
		app_ptr_tmp -= substrm_data->boundary;
	}

	substrm_data->appl_ptr = app_ptr_tmp;

	return app_ptr_tmp;
}

static inline u32 step_hwptr(struct snd_pcm_runtime  *runtime, u32 byteCount)
{
	substream_data *substrm_data = runtime->private_data;
	u32 hw_ptr_tmp = substrm_data->last_ptr;
	u32 buffer_size_frames = (u32)bytes_to_frames(runtime, substrm_data->buffer_size);

	hw_ptr_tmp += (u32)bytes_to_frames(runtime, (int32_t)byteCount);

	if(hw_ptr_tmp >= substrm_data->hw_Base + buffer_size_frames) {
		substrm_data->hw_Base += buffer_size_frames;
	}

	/* hw_base & hw_pos return 0 ... (boundary - 1) */
	if(substrm_data->hw_Base >= substrm_data->boundary) {
		substrm_data->hw_Base = 0;
		hw_ptr_tmp -= substrm_data->boundary;
	}

	substrm_data->last_ptr = hw_ptr_tmp;

	return hw_ptr_tmp;
}

static inline u32 step_cap_real_hwptr(struct snd_pcm_runtime  *runtime, u32 byteCount)
{
    substream_data *substrm_data = runtime->private_data;
    step_hwptr(runtime, byteCount);
    substrm_data->hw_ofs = substrm_data->last_ptr - substrm_data->hw_Base;

    return substrm_data->last_ptr;
}

static inline u32 step_pb_real_hwptr(struct snd_pcm_runtime  *runtime, u32 frames)
{
	substream_data *substrm_data = runtime->private_data;
	u32 hw_ptr_tmp = substrm_data->real_last_ptr;


	if (hw_ptr_tmp != substrm_data->last_ptr)
	{
		u32 buffer_size_frames = (u32)bytes_to_frames(runtime, substrm_data->buffer_size);

		hw_ptr_tmp += frames;

		if(hw_ptr_tmp >= (substrm_data->real_hw_Base + buffer_size_frames)) {
			substrm_data->real_hw_Base += buffer_size_frames;
		}

		/* hw_base & hw_pos return 0 ... (boundary - 1) */
		if(substrm_data->real_hw_Base >= substrm_data->boundary) {
			substrm_data->real_hw_Base = 0;
			hw_ptr_tmp -= substrm_data->boundary;
		}

		substrm_data->real_last_ptr = hw_ptr_tmp;

		if (hw_ptr_tmp >= substrm_data->last_ptr )
        {
            if ((hw_ptr_tmp - substrm_data->last_ptr) < buffer_size_frames)
            {
                substrm_data->real_last_ptr = substrm_data->last_ptr;
                substrm_data->real_hw_Base = substrm_data->hw_Base;
            }
        }
        substrm_data->hw_ofs = substrm_data->real_last_ptr - substrm_data->real_hw_Base;
	}


	return hw_ptr_tmp;
}

static inline u32 GetPbAvailableBytes(const struct snd_pcm_runtime *runtime)
{
	substream_data *substrm_data = runtime->private_data;
	u32 avail = 0;

	if(substrm_data->IsBtSpeech) {
		if(substrm_data->appl_ptr <= substrm_data->last_ptr) {
			avail = substrm_data->boundary - substrm_data->last_ptr + substrm_data->appl_ptr;

			if(avail >= substrm_data->boundary) {
				avail = 0;
			}
		} else {
			avail = substrm_data->appl_ptr - substrm_data->last_ptr;
		}
	} else {
		if(runtime->control->appl_ptr <= substrm_data->last_ptr) {
			avail = substrm_data->boundary - substrm_data->last_ptr + runtime->control->appl_ptr;

			if(avail >= substrm_data->boundary) {
				avail = 0;
			}
		} else {
			avail = runtime->control->appl_ptr - substrm_data->last_ptr;
		}
	}

	return avail;
}

s32 card_audio_hibernation(bool fgWakeUp);

#endif



