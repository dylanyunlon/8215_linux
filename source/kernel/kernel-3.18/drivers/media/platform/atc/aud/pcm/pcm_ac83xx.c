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

/******************************************************************************
*
*[Description]
*	 ALSA Audio board driver.
*
******************************************************************************/

#include <linux/platform_device.h>
#include <linux/soundcard.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/miscdevice.h>

#include "x_typedef.h"
#include "windev.h"
#include "oal.h"
#include "DspAsvInc.h"
#include "pcm_ac83xx.h"
#include "outhw.h"
#include "micin.h"
#include "aud_pcm_dbg.h"
#include "aud_power.h"
#include "pcm_hardware.h"
#include "input.h"
#include "strmproc.h"
#include "speechdev.h"
#include "drv_thread.h"
#include "aud_if.h"

#include "pcm_debug.h"
#define LOG_TAG "pcm_ac83xx"

#define DEVICENUM 3
#define PCM_MODE_NORMAL 0
#define PCM_MODE_VoIP 3

struct semaphore g_rPCMSema;
//struct semaphore g_rPCMSema_tx;
struct semaphore g_rPCMSema_ref;
struct semaphore g_rPCMSema_ref_mic;

extern SpeechDeviceContext g_prSpeechDev;
static void *cap_substream_l = NULL;


struct task_struct	*CopyMicDataThread_task		= NULL;

ac_83xx *snd_chip = NULL;

static bool fgInit;/*Default initialise statics to false*/
static struct platform_device *card_device;
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;

static struct snd_pcm_hardware card_playback = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = (SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S16_LE |
	SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_U16_LE),
	.rates = SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000,
	.rate_min = SAMPLE_RATE_8000,
	.rate_max = SAMPLE_RATE_48000,
	.channels_min = CHANNEL_MONO,
	.channels_max = CHANNEL_STEREO,
	.buffer_bytes_max = 64 * 1024,
	.period_bytes_min	 = 64,
	.period_bytes_max	 = 64 * 1024,
	.periods_min		= 1,
	.periods_max		= 1024,
	.fifo_size		  = 0,
};

static struct snd_pcm_hardware card_capture = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = (SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S16_LE),
	.rates = SNDRV_PCM_RATE_8000_48000,
	.rate_min = SAMPLE_RATE_8000,
	.rate_max = SAMPLE_RATE_48000,
	.channels_min = CHANNEL_MONO,
	.channels_max = CHANNEL_STEREO,
	.buffer_bytes_max = 64 * 1024,
	.period_bytes_min = 64,
	.period_bytes_max = 64 * 1024,
	.periods_min	  = 1,
	.periods_max	  = 1024,
	.fifo_size		  = 0,
};

static void snd_pcm_free_substream(struct snd_pcm_runtime *runtime)
{
	PCM_DEBUG(LOG_TAG, "free substream(%x)\n", runtime->private_data);
    if(runtime->private_data != NULL){
	kfree(runtime->private_data);
}
}
static s32 snd_create(struct snd_card *card, ac_83xx **rchip)
{
	ac_83xx *chip = NULL;

	*rchip = NULL;
	chip = kzalloc(sizeof(*chip), GFP_KERNEL);

	if(chip == NULL) {
		PCM_ERROR(LOG_TAG, "snd_create: kzalloc chip mem err!\r\n");
		return -ENOMEM;
	}

	mutex_init(&chip->m_HeadLock);

	chip->card = card;
	chip->irq = -1;
	chip->m_Intialized = false;
	chip->m_pbu4State = STATE_UNINIT;
	chip->m_capu4State = STATE_UNINIT;
	chip->m_u4StartCount = 0;
	chip->m_prSPHead = NULL;
	chip->m_u4IntrTime = 20;
	chip->m_u4IntrSamples = chip->m_u4IntrTime * 48;

	pcm_audconf_set();
	HardWare_Init(chip);

	chip->irq = VECTOR_AOUT_GPS_RC;
	*rchip = chip;

	return NOERR;
}

static int32_t pcm_card_pb_open(struct snd_pcm_substream  *substream)
{
	ac_83xx *chip = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	substrm_data = kzalloc(sizeof(*substrm_data), GFP_KERNEL);

	if(substrm_data == NULL) {
		return -ENOMEM;
	}

	down(&g_rPCMSema);
	chip = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;
	substrm_data->ac83xx_chip = chip;
	substrm_data->substream = substream;
	substrm_data->m_fgPrepare = false;
    substrm_data->m_SampleRate = chip->m_u4SampleRate;
    substrm_data->m_mode = snd_chip->m_u4pcmMode;

	runtime->private_data = substrm_data;
	runtime->private_free = snd_pcm_free_substream;

	runtime->hw = card_playback;

	if(substream->number >= MAX_PLAYBACK_STREAMS) {
		up(&g_rPCMSema);
		PCM_ERROR(LOG_TAG, "pcm_card_pb_open: error substream->number=%i\r\n", substream->number);
		return -EINVAL;
	}

	if(substream->pcm->device & 1) {
		runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
		runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
	}

	if(substream->pcm->device & 2) {
		runtime->hw.info &= ~(SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_MMAP_VALID);
	}

	up(&g_rPCMSema);
	PCM_DEBUG(LOG_TAG, "pb_open: Success, private(%x) substream->number=%i\r\n", runtime->private_data, substream->number);

	return NOERR;
}


static int32_t pcm_card_pb_ref_open(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_pb_ref_open\n");

	ac_83xx *chip = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	substrm_data = kzalloc(sizeof(*substrm_data), GFP_KERNEL);

	if(substrm_data == NULL) {
		return -ENOMEM;
	}

	//down(&g_rPCMSema_tx);
	chip = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;
	substrm_data->ac83xx_chip = chip;
	substrm_data->substream = substream;
	substrm_data->m_fgPrepare = false;
	substrm_data->m_SampleRate = chip->m_u4SampleRate;

	runtime->private_data = substrm_data;
	runtime->private_free = snd_pcm_free_substream;

	runtime->hw = card_playback;

	if(substream->number >= MAX_PLAYBACK_STREAMS) {
		//up(&g_rPCMSema_tx);
		pr_err("[PCM ERR]pcm_card_pb_open: error substream->number=%i\r\n", substream->number);
		return -EINVAL;
	}

	//up(&g_rPCMSema_tx);
	pr_debug("[PCM]pb_open: Success, private(%p) substream->number=%i\r\n", runtime->private_data, substream->number);

	return NOERR;
}

static int32_t pcm_card_pb_ref_prepare(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_pb_ref_prepare\n");

	int u = -1;
	int is8 = -1;
	int mono = -1;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	//down(&g_rPCMSema_tx);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;

	if(true == (substrm_data->m_fgPrepare)) {
		//up(&g_rPCMSema_tx);
		return NOERR;
	}

	pr_debug("[PCM]pcm_card_pb_prepare: substream->number=%i\r\n", substream->number);

	mono = (runtime->channels > 1) ? 0 : 1;
	is8 = snd_pcm_format_width(runtime->format) == 16 ? 0 : 1;
	u = snd_pcm_format_unsigned(runtime->format);

	substrm_data->period_size = snd_pcm_lib_period_bytes(substream);
	substrm_data->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_start = (uintptr_t)runtime->dma_area;
	substrm_data->dma_shift = 2 - mono - is8;
	substrm_data->last_ptr = 0;
	substrm_data->Used_size = 0;
	substrm_data->IsBtSpeech = 0;
	substrm_data->appl_ptr = 0;
	substrm_data->app_Base = 0;
	substrm_data->hw_Base = 0;
	substrm_data->boundary = runtime->boundary;

	pr_debug("[PCM]pcm_card_pb_prepare: substrm_data->period_size=%i\r\n", substrm_data->period_size);
	//up(&g_rPCMSema_tx);
	substrm_data->m_fgPrepare = true;

	return NOERR;
}

static int32_t pcm_card_pb_ref_trigger(struct snd_pcm_substream *substream, int cmd){
	pr_debug("call pcm_card_pb_ref_trigger\n");

	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	pr_debug("[PCM]pcm_card_pb_ref_trigger: cmd = %d\r\n", (s32)cmd);
	//down(&g_rPCMSema_tx);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;

	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		pr_debug("[PCM]pcm_card_pb_ref_trigger: SNDRV_PCM_TRIGGER_START(%i).\r\n",
			 substream->number);
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_START;
		//StateChangeInform(substream);
		SpeechDev_SetSubstream(substream);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		pr_debug("[PCM]pcm_card_pb_ref_trigger: SNDRV_PCM_TRIGGER_STOP(%i).\r\n",
			 substream->number);
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_STOP;

		SpeechDev_SetSubstream(NULL);
		break;

	default:
		pr_err("[PCM ERR]pcm_card_pb_ref_trigger: EINVAL.\r\n");
        //up(&g_rPCMSema_tx);
		return (-EINVAL);
	}
	//up(&g_rPCMSema_tx);
	return NOERR;
}

static int32_t pcm_card_pb_ref_close(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_pb_ref_close\n");

	return NOERR;
}

static int32_t pcm_card_pb_ref_open_voip(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_pb_ref_open_voip\n");

	ac_83xx *chip = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	substrm_data = kzalloc(sizeof(*substrm_data), GFP_KERNEL);

	if(substrm_data == NULL) {
		return -ENOMEM;
	}

	down(&g_rPCMSema);
	chip = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;
	substrm_data->ac83xx_chip = chip;
	substrm_data->substream = substream;
	substrm_data->m_fgPrepare = false;

	runtime->private_data = substrm_data;
	runtime->private_free = snd_pcm_free_substream;

	runtime->hw = card_playback;

	if(substream->number >= MAX_PLAYBACK_STREAMS) {
		up(&g_rPCMSema);
		PCM_ERROR(LOG_TAG, "pcm_card_pb_ref_open_voip: error substream->number=%i\r\n", substream->number);
		return -EINVAL;
	}

	if(substream->pcm->device & 1) {
		runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
		runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
	}

	if(substream->pcm->device & 2) {
		runtime->hw.info &= ~(SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_MMAP_VALID);
	}

	up(&g_rPCMSema);
	PCM_DEBUG(LOG_TAG, "pcm_card_pb_ref_open_voip: Success, private(%x) substream->number=%i\r\n", runtime->private_data, substream->number);

	return NOERR;
}

static int32_t pcm_card_pb_ref_prepare_voip(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_pb_ref_prepare_voip\n");
	int u = -1;
	int is8 = -1;
	int mono = -1;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	down(&g_rPCMSema);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;

	if(true == (substrm_data->m_fgPrepare)) {
		up(&g_rPCMSema);
		return NOERR;
	}

	PCM_DEBUG(LOG_TAG, "pcm_card_pb_ref_prepare_voip: substream->number=%i\r\n", substream->number);

	if(NOERR != GetStreamProcess(substream)) {
		up(&g_rPCMSema);
		PCM_ERROR(LOG_TAG, "pcm_card_pb_ref_prepare_voip: GetStreamProcess(%i) error!\r\n", substream->number);
		return -EPERM;
	}

	mono = (runtime->channels > 1) ? 0 : 1;
	is8 = snd_pcm_format_width(runtime->format) == 16 ? 0 : 1;
	u = snd_pcm_format_unsigned(runtime->format);

	substrm_data->period_size = snd_pcm_lib_period_bytes(substream);
	substrm_data->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_start = (u32)runtime->dma_area;
	substrm_data->dma_shift = 2 - mono - is8;
	substrm_data->last_ptr = 0;
	substrm_data->Used_size = 0;
	substrm_data->IsBtSpeech = 0;
	substrm_data->IsVoIP= 1;
	substrm_data->appl_ptr = 0;
	substrm_data->app_Base = 0;
	substrm_data->hw_Base = 0;
	substrm_data->boundary = runtime->boundary;

	if(NOERR != PrepareStream(substream)) {
		PCM_ERROR(LOG_TAG, "pcm_card_pb_ref_prepare_voip: PrepareStream(%i) err!\r\n", substream->number);
		up(&g_rPCMSema);
		return -EPERM;
	}

	up(&g_rPCMSema);
	substrm_data->m_fgPrepare = true;

	return NOERR;
}

static int32_t pcm_card_pb_ref_trigger_voip(struct snd_pcm_substream *substream, int cmd){
	pr_debug("call pcm_card_pb_ref_trigger_voip\n");

	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	PCM_DEBUG(LOG_TAG, "pcm_card_pb_ref_trigger_voip: cmd = %d\r\n", (s32)cmd);

	runtime = substream->runtime;
	substrm_data = runtime->private_data;

	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		PCM_DEBUG(LOG_TAG, "pcm_card_pb_ref_trigger_voip: SNDRV_PCM_TRIGGER_START(%i).\r\n",
			 substream->number);
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_START;
		StateChangeInform(substream);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		PCM_DEBUG(LOG_TAG, "pcm_card_pb_ref_trigger_voip: SNDRV_PCM_TRIGGER_STOP(%i).\r\n",
			 substream->number);
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_STOP;
		StateChangeInform(substream);
		break;

	default:
		PCM_ERROR(LOG_TAG, "pcm_card_pb_ref_trigger_voip: EINVAL.\r\n");
		return (-EINVAL);
	}

	return NOERR;
}

static int32_t pcm_card_pb_ref_close_voip(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_pb_ref_close_voip\n");
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	StreamProcess *strmProc = substrm_data->pSubstreamPro;

	PCM_DEBUG(LOG_TAG, "pcm_card_pb_ref_close_voip Success, private(%x) substream->number=%i\r\n", runtime->private_data, substream->number);

	if(NULL == strmProc) {
		return NOERR;
	}

	down(&g_rPCMSema);
	ReleaseStreamProcess(substream);
	up(&g_rPCMSema);

	return NOERR;
}

static int32_t pcm_card_capture_ref_open(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card__capture_echoRef_open\n");

	ac_83xx *chip = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_ref_stream *ref_stream = NULL;
	int32_t err = -1;
	down(&g_rPCMSema_ref);
	substrm_data = kzalloc(sizeof(atc_ref_stream), GFP_KERNEL);

	if(substrm_data == NULL) {
		pr_err("[PCM ERR]pcm_card_capture_open: kzalloc substrm_data err!\r\n");
        up(&g_rPCMSema_ref);
		return (-ENOMEM);
	}

	chip = snd_pcm_substream_chip(substream);
	ref_stream = (atc_ref_stream *) substrm_data;
	sema_init(&substrm_data->rThreadExitSema, 0);
	runtime = substream->runtime;
	substrm_data->ac83xx_chip = chip;
	substrm_data->substream = substream;
	substrm_data->m_fgPrepare = false;
	substrm_data->m_SampleRate = chip->m_u4SampleRate;
	runtime->private_data = substrm_data;
	runtime->private_free = snd_pcm_free_substream;
	runtime->hw = card_capture;
	substrm_data->m_eOpen_status = 1;

	up(&g_rPCMSema_ref);
	return NOERR;
}

static int32_t pcm_card_capture_ref_prepare(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_capture_ref_prepare\n");
	int u = -1;
	int is8 = -1;
	int mono = -1;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	down(&g_rPCMSema_ref);

	runtime = substream->runtime;
	substrm_data = runtime->private_data;
	if(true == (substrm_data->m_fgPrepare)) {
        up(&g_rPCMSema_ref);
		return NOERR;
	}

	mono = (runtime->channels > 1) ? 0 : 1;
	is8 = snd_pcm_format_width(runtime->format) == 16 ? 0 : 1;
	u = snd_pcm_format_unsigned(runtime->format);

	substrm_data->period_size = snd_pcm_lib_period_bytes(substream);
	substrm_data->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_start = (uintptr_t)runtime->dma_area;
	substrm_data->dma_shift = 2 - mono - is8;
	substrm_data->last_ptr = 0;
	substrm_data->Used_size = 0;
	substrm_data->appl_ptr = 0;
	substrm_data->app_Base = 0;
	substrm_data->hw_Base = 0;
	substrm_data->boundary = runtime->boundary;
	substrm_data->IsBtSpeech = 0;
	substrm_data->IsNeedRef = 1;

	pr_debug("[PCM]pcm_card_capture_ref_prepare(%d): substream VIRSADR:0x%p, buffersize:0x%x,period_size:%d,mono:%d,is8:%d\r\n",
		 substream->number, (void*)substrm_data->dma_start, substrm_data->dma_size,substrm_data->period_size,mono,is8);

	if(is8) {
		if(mono) {
			substrm_data->m_SampleType = PCM_TYPE_M8;
		} else {
			substrm_data->m_SampleType = PCM_TYPE_S8;
		}
	} else {
		if(mono) {
			substrm_data->m_SampleType = PCM_TYPE_M16;
		} else {
			substrm_data->m_SampleType = PCM_TYPE_S16;
		}
	}

	substrm_data->m_fgPrepare = true;
	up(&g_rPCMSema_ref);
	return NOERR;
}

static int32_t pcm_card_capture_ref_trigger(struct snd_pcm_substream  *substream, int cmd){
	pr_debug("call pcm_card_capture_ref_trigger\n");
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_ref_stream *ref_stream = NULL;

	down(&g_rPCMSema_ref);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;
	ref_stream = runtime->private_data;
	pr_debug("substrm_data->m_eState:%d\n",substrm_data->m_eState);


	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_START;
		SetRefSubStream(substream);
		pr_debug("[PCM]pcm_card_capture_ref_trigger: SNDRV_PCM_TRIGGER_START(%i).\r\n",
			 substream->number);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		pr_debug("[PCM]pcm_card_capture_ref_trigger: SNDRV_PCM_TRIGGER_STOP(%i).\r\n",
			 substream->number);
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_STOP;
		break;

	default:
		pr_err("[PCM ERR]pcm_card_capture_ref_trigger: EINVAL.\r\n");
		up(&g_rPCMSema_ref);
		return (-EINVAL);
	}
	up(&g_rPCMSema_ref);
	return NOERR;
}

static int32_t pcm_card_capture_ref_close(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card__capture_echoRef_close\n");
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_ref_stream *ref_stream = NULL;

	down(&g_rPCMSema_ref);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;
	ref_stream = (atc_ref_stream *) substrm_data;

        down(&g_prSpeechDev.m_refLock);
	SetRefSubStream(NULL);
        up(&g_prSpeechDev.m_refLock);
	pr_debug("[PCM]pcm_card__capture_Ref_close\r\n");

	up(&g_rPCMSema_ref);
	return NOERR;
}

static int32_t pcm_card_capture_ref_open_mic(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_capture_ref_open_mic\n");
	ac_83xx *chip = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_ref_stream *ref_stream = NULL;
	int32_t err = -1;
	//down(&g_rPCMSema_ref_mic);
	substrm_data = kzalloc(sizeof(atc_ref_stream), GFP_KERNEL);

	if(substrm_data == NULL) {
		return (-ENOMEM);
	}
	memset(substrm_data,0,sizeof(atc_ref_stream));
	chip = snd_pcm_substream_chip(substream);
	ref_stream = (atc_ref_stream *) substrm_data;
	sema_init(&substrm_data->rThreadExitSema, 0);
	runtime = substream->runtime;
	substrm_data->ac83xx_chip = chip;
	substrm_data->substream = substream;
	substrm_data->m_fgPrepare = false;
	ref_stream->isReseted = false;
	substrm_data->m_SampleRate = chip->m_u4SampleRate;
    substrm_data->m_SpeechMode = chip->m_u4speechMode;
	runtime->private_data = substrm_data;
	runtime->private_free = snd_pcm_free_substream;
	runtime->hw = card_capture;
	substrm_data->m_eOpen_status = 1;
	cap_substream_l = ref_stream;
		ref_stream->refDataThread_task = NULL;
		ref_stream->refDataThread_task = kthread_create(CopyRfDataThread, (void *)substrm_data, "refDataThread");

		if(IS_ERR(ref_stream->refDataThread_task)) {
			pr_err("[PCM ERR] Create CopyMicDataThread_task ERR!\r\n");
			err = PTR_ERR(ref_stream->refDataThread_task);
			ref_stream->refDataThread_task = NULL;
	        //up(&g_rPCMSema_ref_mic);
			return (-EINVAL);
		}
		/*else
		{
		    struct sched_param param;
		    s32 ret;

		    param.sched_priority = to_sched_priority(ADSPTASK_THREAD_PRIORITY);
		    ret = sched_setscheduler_nocheck(ref_stream->refDataThread_task , SCHED_RR, &param);
		    AUD_ASSERT(ret == 0);
		}*/

		init_waitqueue_head(&ref_stream->refDataThread_wq);
		ref_stream->refDataThread_exit = 0;
	if(substrm_data->m_SampleRate == SAMPLE_RATE_8000 || substrm_data->m_SampleRate == SAMPLE_RATE_16000)
	{
		ref_stream->refDataThread_wq_flag = 1;
	}
	else
	{
			ref_stream->refDataThread_wq_flag = 0;
	}
		wake_up_process(ref_stream->refDataThread_task);

	//up(&g_rPCMSema_ref_mic);
	return NOERR;
}

static int32_t pcm_card_capture_ref_prepare_mic(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_capture_ref_prepare_mic\n");
	int u = -1;
	int is8 = -1;
	int mono = -1;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	//down(&g_rPCMSema_ref_mic);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;
	if(true == (substrm_data->m_fgPrepare)) {
        //up(&g_rPCMSema_ref_mic);
		return NOERR;
	}

	mono = (runtime->channels > 1) ? 0 : 1;
	is8 = snd_pcm_format_width(runtime->format) == 16 ? 0 : 1;
	u = snd_pcm_format_unsigned(runtime->format);

	substrm_data->period_size = snd_pcm_lib_period_bytes(substream);
	substrm_data->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_start = (uintptr_t)runtime->dma_area;
	substrm_data->dma_shift = 2 - mono - is8;
	substrm_data->last_ptr = 0;
	substrm_data->Used_size = 0;
	substrm_data->appl_ptr = 0;
	substrm_data->app_Base = 0;
	substrm_data->hw_Base = 0;
	substrm_data->boundary = runtime->boundary;
	substrm_data->IsBtSpeech = 0;

	pr_debug("[PCM]pcm_card_capture_ref_prepare_mic(%d): substream VIRSADR:0x%p, buffersize:0x%x,period_size:%d,mono:%d,is8:%d\r\n",
		 substream->number, (void*)substrm_data->dma_start, substrm_data->dma_size,substrm_data->period_size,mono,is8);

	if(is8) {
		if(mono) {
			substrm_data->m_SampleType = PCM_TYPE_M8;
		} else {
			substrm_data->m_SampleType = PCM_TYPE_S8;
		}
	} else {
		if(mono) {
			substrm_data->m_SampleType = PCM_TYPE_M16;
		} else {
			substrm_data->m_SampleType = PCM_TYPE_S16;
		}
	}
	substrm_data->m_eState = SNDRV_PCM_TRIGGER_START;
	substrm_data->m_fgPrepare = true;
	//up(&g_rPCMSema_ref_mic);
	return NOERR;
}

static int32_t pcm_card_capture_ref_trigger_mic(struct snd_pcm_substream  *substream, int cmd){
	pr_debug("call pcm_card_capture_ref_trigger_mic\n");
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_ref_stream *ref_stream = NULL;

	//down(&g_rPCMSema_ref_mic);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;
	ref_stream = runtime->private_data;
	pr_debug("substrm_data->m_eState:%d\n",substrm_data->m_eState);


	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_START;
		if(substrm_data->m_SampleRate == SAMPLE_RATE_8000 || substrm_data->m_SampleRate == SAMPLE_RATE_16000){
            snd_chip->m_u4BtMute = 0;
			SpeechDev_SetCaptureSubStream(substream);
		}
		else
		{
			SetCapSubStream(substream);
		}

		pr_debug("[PCM]pcm_card_capture_ref_trigger_mic: SNDRV_PCM_TRIGGER_START(%i).\r\n",
			 substream->number);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		pr_debug("[PCM]pcm_card_capture_ref_trigger_mic: SNDRV_PCM_TRIGGER_STOP(%i).\r\n",
			 substream->number);

		if(substrm_data == NULL || ref_stream== NULL){
			pr_debug("substrm_data NULL\n");
			break;
		}
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_STOP;

		substrm_data->m_eOpen_status = 0;
		ref_stream->refDataThread_wq_flag = 1;
		wake_up_interruptible(&ref_stream->refDataThread_wq);
		if(substrm_data->m_SampleRate == SAMPLE_RATE_8000 || substrm_data->m_SampleRate == SAMPLE_RATE_16000){
		SpeechDev_SetCaptureSubStream(NULL);
            snd_chip->m_u4SampleRate = 0;
		}
		break;

	default:
		pr_err("[PCM ERR]pcm_card_capture_ref_trigger_mic: EINVAL.\r\n");
		return (-EINVAL);
	}
	//up(&g_rPCMSema_ref_mic);
	return NOERR;
}

static int32_t pcm_card_capture_ref_close_mic(struct snd_pcm_substream  *substream){
	pr_debug("call pcm_card_capture_ref_close_mic\n");
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_ref_stream *ref_stream = NULL;

	//down(&g_rPCMSema_ref_mic);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;
	ref_stream = (atc_ref_stream *) substrm_data;
	pr_debug("[PCM] pcm_card_capture_ref_close_mic  substream->number=%i\r\n",substream->number);
    if(substrm_data->ac83xx_chip->m_u4speechMode == 0)
    {
        SetCapSubStream(NULL);
    }

	if(ref_stream != NULL&&ref_stream->refDataThread_task) {
		SpeechDev_SetCaptureSubStream(NULL);
		ref_stream->refDataThread_exit = 1;
		ref_stream->refDataThread_wq_flag = 1;
		wake_up_interruptible(&ref_stream->refDataThread_wq);
		ref_stream->refDataThread_task = NULL;
	}

	down(&substrm_data->rThreadExitSema);
	pr_debug("[PCM]pcm_card_capture_ref_close_mic: wait ref thread exit done\r\n");
	return NOERR;
}

static int32_t pcm_card_capture_open(struct snd_pcm_substream  *substream)
{
	ac_83xx *chip = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_capture_stream *capture_stream = NULL;
	int32_t err = -1;

	substrm_data = kzalloc(sizeof(atc_capture_stream), GFP_KERNEL);

	if(substrm_data == NULL) {
		PCM_ERROR(LOG_TAG, "pcm_card_capture_open: kzalloc substrm_data err!\r\n");
		return (-ENOMEM);
	}

	substrm_data->m_u4Idx = ASRC_CHSET_NUM;

	capture_stream = (atc_capture_stream *) substrm_data;

	sema_init(&substrm_data->rThreadExitSema, 0);

	down(&g_rPCMSema);
	chip = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;
	substrm_data->ac83xx_chip = chip;
	substrm_data->substream = substream;
	substrm_data->m_fgPrepare = false;

	runtime->private_data = substrm_data;
	runtime->private_free = snd_pcm_free_substream;
	runtime->hw = card_capture;

	capture_stream->CopyMicDataThread_task = NULL;
	capture_stream->CopyMicDataThread_task = kthread_create(CopyMicDataThread, (void *)substrm_data, "CopyMicDataThread");

	if(IS_ERR(capture_stream->CopyMicDataThread_task)) {
		PCM_ERROR(LOG_TAG, "Create CopyMicDataThread_task ERR!\r\n");
		err = PTR_ERR(capture_stream->CopyMicDataThread_task);
		capture_stream->CopyMicDataThread_task = NULL;
		up(&substrm_data->rThreadExitSema);
		up(&g_rPCMSema);
		return (-EINVAL);
	} else {
        struct sched_param param;
        s32 ret;
        param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(capture_stream->CopyMicDataThread_task, SCHED_RR, &param);
        AUD_ASSERT(ret == 0);
    }

	init_waitqueue_head(&capture_stream->CopyMicDataThread_wq);
	capture_stream->CopyMicDataThread_wq_flag = 0;
	wake_up_process(capture_stream->CopyMicDataThread_task);

	PCM_DEBUG(LOG_TAG, "capture_open: Success, private(%x) substream->number=%i\r\n", runtime->private_data, substream->number);
	up(&g_rPCMSema);

	return NOERR;
}

static int32_t pcm_card_pb_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	StreamProcess *strmProc = substrm_data->pSubstreamPro;

	PCM_DEBUG(LOG_TAG, "pb_close Success, private(%x) substream->number=%i\r\n", runtime->private_data, substream->number);

	if(NULL == strmProc) {
		return NOERR;
	}

	down(&g_rPCMSema);
	pr_debug("pcm_card_pb_close\n");
    substrm_data->m_mode = 0;
    substrm_data->m_SampleRate = 0;
	ReleaseStreamProcess(substream);
	up(&g_rPCMSema);

	return NOERR;
}

static int32_t pcm_card_capture_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_capture_stream *capture_stream = NULL;

	down(&g_rPCMSema);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;
	capture_stream = (atc_capture_stream *) substrm_data;
	PCM_DEBUG(LOG_TAG, "capture_close private(%x) substream->number=%i\r\n", runtime->private_data, substream->number);

	if(capture_stream->CopyMicDataThread_task) {
		/* Inform copy data thread to exit*/
		capture_stream->CopyMicDataThread_exit = 1;
		capture_stream->CopyMicDataThread_wq_flag = 1;
		wake_up_interruptible(&capture_stream->CopyMicDataThread_wq);
	}

	up(&g_rPCMSema);

	/* Wait for copy data thread exit*/

	down(&substrm_data->rThreadExitSema);
	PCM_DEBUG(LOG_TAG, "pcm_card_capture_close: wait copy thread exit done\r\n");

	down(&g_rPCMSema);

	capture_stream->CopyMicDataThread_task = NULL;

	up(&g_rPCMSema);

	return NOERR;
}

static int32_t pcm_card_pb_prepare(struct snd_pcm_substream *substream)
{
	int u = -1;
	int is8 = -1;
	int mono = -1;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	down(&g_rPCMSema);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;

	if(true == (substrm_data->m_fgPrepare)) {
		up(&g_rPCMSema);
		return NOERR;
	}

	PCM_DEBUG(LOG_TAG, "pcm_card_pb_prepare: substream->number=%i\r\n", substream->number);

	if(NOERR != GetStreamProcess(substream)) {
		up(&g_rPCMSema);
		PCM_ERROR(LOG_TAG, "pcm_card_pb_prepare: GetStreamProcess(%i) error!\r\n", substream->number);
		return -EPERM;
	}

	mono = (runtime->channels > 1) ? 0 : 1;
	is8 = snd_pcm_format_width(runtime->format) == 16 ? 0 : 1;
	u = snd_pcm_format_unsigned(runtime->format);

	substrm_data->period_size = snd_pcm_lib_period_bytes(substream);
	substrm_data->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_start = (u32)runtime->dma_area;
	substrm_data->dma_shift = 2 - mono - is8;
	substrm_data->last_ptr = 0;
	substrm_data->Used_size = 0;
	substrm_data->IsBtSpeech = 0;
	substrm_data->appl_ptr = 0;
	substrm_data->app_Base = 0;
	substrm_data->hw_Base = 0;
	substrm_data->boundary = runtime->boundary;

	if(NOERR != PrepareStream(substream)) {
		PCM_ERROR(LOG_TAG, "pcm_card_pb_prepare: PrepareStream(%i) err!\r\n", substream->number);
		up(&g_rPCMSema);
		return -EPERM;
	}

	up(&g_rPCMSema);
	substrm_data->m_fgPrepare = true;

	return NOERR;
}

static int32_t pcm_card_capture_prepare(struct snd_pcm_substream *substream)
{
	int u = -1;
	int is8 = -1;
	int mono = -1;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	down(&g_rPCMSema);
	runtime = substream->runtime;
	substrm_data = runtime->private_data;

	if(true == (substrm_data->m_fgPrepare)) {
		up(&g_rPCMSema);
		return NOERR;
	}

	mono = (runtime->channels > 1) ? 0 : 1;
	is8 = snd_pcm_format_width(runtime->format) == 16 ? 0 : 1;
	u = snd_pcm_format_unsigned(runtime->format);

	substrm_data->period_size = snd_pcm_lib_period_bytes(substream);
	substrm_data->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_size = snd_pcm_lib_buffer_bytes(substream);
	substrm_data->dma_start = (u32)runtime->dma_area;
	substrm_data->dma_shift = 2 - mono - is8;
	substrm_data->last_ptr = 0;
	substrm_data->Used_size = 0;
	substrm_data->appl_ptr = 0;
	substrm_data->app_Base = 0;
	substrm_data->hw_Base = 0;
	substrm_data->boundary = runtime->boundary;
	substrm_data->IsBtSpeech = 0;

	PCM_DEBUG(LOG_TAG, "pcm_card_capture_prepare(%d): substream VIRSADR:0x%x, buffersize:0x%x\r\n",
		 substream->number, substrm_data->dma_start, substrm_data->dma_size);

	if(is8) {
		if(mono) {
			substrm_data->m_SampleType = PCM_TYPE_M8;
		} else {
			substrm_data->m_SampleType = PCM_TYPE_S8;
		}
	} else {
		if(mono) {
			substrm_data->m_SampleType = PCM_TYPE_M16;
		} else {
			substrm_data->m_SampleType = PCM_TYPE_S16;
		}
	}

	substrm_data->m_fgPrepare = true;
	up(&g_rPCMSema);

	return NOERR;
}

static int32_t pcm_card_pb_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	PCM_DEBUG(LOG_TAG, "pcm_card_pb_trigger: cmd = %d\r\n", (s32)cmd);

	runtime = substream->runtime;
	substrm_data = runtime->private_data;

	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		PCM_DEBUG(LOG_TAG, "pcm_card_pb_trigger: SNDRV_PCM_TRIGGER_START(%i).\r\n",
			 substream->number);
        if(substrm_data->m_mode == PCM_MODE_VoIP && substrm_data->m_SampleRate != SAMPLE_RATE_8000 && substrm_data->m_SampleRate != SAMPLE_RATE_16000)
    	{
    	    substrm_data->IsVoIP = 1;
    	}
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_START;
		StateChangeInform(substream);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		PCM_DEBUG(LOG_TAG, "pcm_card_pb_trigger: SNDRV_PCM_TRIGGER_STOP(%i).\r\n",
			 substream->number);
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_STOP;
		StateChangeInform(substream);
		break;

	default:
		PCM_ERROR(LOG_TAG, "pcm_card_pb_trigger: EINVAL.\r\n");
		return (-EINVAL);
	}

	return NOERR;
}

static int32_t pcm_card_capture_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;
	atc_capture_stream *capture_stream = NULL;

	PCM_DEBUG(LOG_TAG, "pcm_card_capture_trigger: cmd = %d\r\n", (s32)cmd);
	runtime = substream->runtime;
	capture_stream = runtime->private_data;
	substrm_data = runtime->private_data;

	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		PCM_DEBUG(LOG_TAG, "pcm_card_capture_trigger: SNDRV_PCM_TRIGGER_START(%i) rate(%d)\r\n",
			 substream->number, (s32)runtime->rate);

		substrm_data->m_eState = SNDRV_PCM_TRIGGER_START;
		capture_stream->CopyMicDataThread_wq_flag = 1;
		wake_up_interruptible(&capture_stream->CopyMicDataThread_wq);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		PCM_DEBUG(LOG_TAG, "pcm_card_capture_trigger: SNDRV_PCM_TRIGGER_STOP(%i).\r\n",
			 substream->number);
		substrm_data->m_eState = SNDRV_PCM_TRIGGER_STOP;
		break;

	default:
		PCM_ERROR(LOG_TAG, "pcm_card_capture_trigger: EINVAL.\r\n");
		return (-EINVAL);
	}

	return NOERR;
}

static int32_t pcm_card_pb_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *hw_params)
{
	int32_t err = -1;

	down(&g_rPCMSema);
	err = (int32_t)snd_pcm_lib_malloc_pages(substream, (u32)params_buffer_bytes(hw_params));

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "pcm_card_pb_hw_params: snd_pcm_lib_malloc_pages err(%i)! \r\n", err);
		up(&g_rPCMSema);
		return err;
	}

	up(&g_rPCMSema);
	PCM_DEBUG(LOG_TAG, "pcm_card_pb_hw_params: substream->number=%i\r\n", substream->number);

	return NOERR;
}

static snd_pcm_uframes_t pcm_card_pb_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	uint32_t hw_ofs = substrm_data->last_ptr - substrm_data->hw_Base;

	return (snd_pcm_uframes_t)hw_ofs;
}

static int32_t pcm_card_hw_free(struct snd_pcm_substream *substream)
{
	int32_t i4Ret = -1;
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;

	if(false == (substrm_data->m_fgPrepare)) {
		return NOERR;
	}

	PCM_DEBUG(LOG_TAG, "pcm_card_hw_free: substream->number=%i\r\n", substream->number);
	down(&g_rPCMSema);
	i4Ret = snd_pcm_lib_free_pages(substream);
	up(&g_rPCMSema);
	substrm_data->m_fgPrepare = false;

	return i4Ret;
}

static int32_t pcm_card_hw_free_ref(struct snd_pcm_substream *substream)
{
	pr_debug("[PCM]pcm_card_hw_free_ref: substream->number=%i\r\n", substream->number);
    int32_t i4Ret = -1;
    struct snd_pcm_runtime *runtime = substream->runtime;
    substream_data *substrm_data = runtime->private_data;

    if(false == (substrm_data->m_fgPrepare)) {
        return NOERR;
    }

	pr_debug("[PCM]pcm_card_hw_free_ref: substream->number=%i\r\n", substream->number);
    down(&g_rPCMSema_ref);
    i4Ret = snd_pcm_lib_free_pages(substream);
    up(&g_rPCMSema_ref);
    substrm_data->m_fgPrepare = false;

    return i4Ret;
}

static int32_t pcm_card_hw_free_ref_mic(struct snd_pcm_substream *substream)
{
	pr_debug("[PCM]pcm_card_hw_free_ref_mic: substream->number=%i\r\n", substream->number);
    int32_t i4Ret = -1;
    struct snd_pcm_runtime *runtime = substream->runtime;
    substream_data *substrm_data = runtime->private_data;

    if(false == (substrm_data->m_fgPrepare)) {
        return NOERR;
    }

	pr_debug("[PCM]pcm_card_hw_free_ref_mic: substream->number=%i\r\n", substream->number);
    //down(&g_rPCMSema_ref_mic);
    i4Ret = snd_pcm_lib_free_pages(substream);
    //up(&g_rPCMSema_ref_mic);
    substrm_data->m_fgPrepare = false;

    return i4Ret;
}



static int32_t pcm_card_pb_vol_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
        uinfo->value.integer.max = 256;
	uinfo->value.integer.step = 1;

	return NOERR;
}

static int32_t pcm_card_pb_vol_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	int32_t global_volume = uvalue->value.integer.value[0];
	//SpeechDev_SetDevVolume(200 , 200);
	SpeechDev_SetSCOVolume(global_volume , global_volume);
    pr_debug("[PCM]pcm_card_pb_vol_put: global_volume=%d.\r\n", global_volume);

    return NOERR;
}

static int32_t pcm_card_pb_vol_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
    return NOERR;
}

static int32_t pcm_mode_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}

static int32_t pcm_mode_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	snd_chip->m_u4pcmMode = uvalue->value.integer.value[0];

	pr_debug("[PCM]pcm_mode_put: snd_chip.m_u4pcmMode =%d.\r\n", snd_chip->m_u4pcmMode);

	return NOERR;
}

static int32_t pcm_mode_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}

static int32_t BT_SampleRate_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}

static int32_t BT_SampleRate_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	snd_chip->m_u4SampleRate = uvalue->value.integer.value[0];

	pr_debug("[PCM]BT_SampleRate_put: snd_chip.m_u4SampleRate =%d.\r\n", snd_chip->m_u4SampleRate);

	return NOERR;
}

static int32_t BT_SampleRate_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}

static int32_t pcm_bt_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}

static int32_t pcm_bt_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	snd_chip->m_u4pcmBt = uvalue->value.integer.value[0];

	pr_debug("[PCM]pcm_bt_put: snd_chip->m_u4pcmBt =%d.\r\n", snd_chip->m_u4pcmBt);

	return NOERR;
}

static int32_t pcm_bt_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}

static int32_t pcm_speechmode_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}

static int32_t pcm_speechmode_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	snd_chip->m_u4speechMode = uvalue->value.integer.value[0];

	pr_debug("[PCM]pcm_speechmode_put: snd_chip->m_u4speechMode =%d.\r\n", snd_chip->m_u4speechMode);

	return NOERR;
}

static int32_t pcm_speechmode_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}


static int32_t pcm_dump_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}

static int32_t pcm_dump_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	snd_chip->m_u4pcmDump = uvalue->value.integer.value[0];

	pr_debug("[PCM]pcm_dump_put: snd_chip.m_u4pcmDump =%d.\r\n", snd_chip->m_u4pcmDump);

	return NOERR;
}

static int32_t pcm_dump_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}


static struct snd_kcontrol_new card_playback_vol = {
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "card Volum",
	.index = 0,
	.info = pcm_card_pb_vol_info,
	.get = pcm_card_pb_vol_get,
	.put = pcm_card_pb_vol_put,
};


static struct snd_kcontrol_new ATC_card_controls[] = {
	{
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "card Volum",
	.index = 0,
	.info = pcm_card_pb_vol_info,
	.get = pcm_card_pb_vol_get,
	.put = pcm_card_pb_vol_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "BT_SampleRate",
		.index = 0,
		.info = BT_SampleRate_info,
		.get = BT_SampleRate_get,
		.put = BT_SampleRate_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "pcm mode",
		.index = 0,
		.info = pcm_mode_info,
		.get = pcm_mode_get,
		.put = pcm_mode_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "pcm bt",
		.index = 0,
		.info = pcm_bt_info,
		.get = pcm_bt_get,
		.put = pcm_bt_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "pcm speechmode",
		.index = 0,
		.info = pcm_speechmode_info,
		.get = pcm_speechmode_get,
		.put = pcm_speechmode_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "pcm dump",
		.index = 0,
		.info = pcm_dump_info,
		.get = pcm_dump_get,
		.put = pcm_dump_put,
	},
};

static struct snd_pcm_ops card_playback_ops = {
	.open	   = pcm_card_pb_open,
	.close	   = pcm_card_pb_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = pcm_card_pb_hw_params,
	.hw_free   = pcm_card_hw_free,
	.prepare   = pcm_card_pb_prepare,
	.trigger   = pcm_card_pb_trigger,
	.pointer   = pcm_card_pb_pcm_pointer,
};

static struct snd_pcm_ops card_capture_ops = {
	.open	   = pcm_card_capture_open,
	.close	   = pcm_card_capture_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = pcm_card_pb_hw_params,
	.hw_free   = pcm_card_hw_free,
	.prepare   = pcm_card_capture_prepare,
	.trigger   = pcm_card_capture_trigger,
	.pointer   = pcm_card_pb_pcm_pointer,
};

static struct snd_pcm_ops card_playback_ref_ops = {
	.open	   = pcm_card_pb_ref_open,
	.close	   = pcm_card_pb_ref_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = pcm_card_pb_hw_params,
	.hw_free   = pcm_card_hw_free_ref,
	.prepare   = pcm_card_pb_ref_prepare,
	.trigger   = pcm_card_pb_ref_trigger,
	.pointer   = pcm_card_pb_pcm_pointer,
};

static struct snd_pcm_ops card_capture_ref_ops = {
	.open	   = pcm_card_capture_ref_open,
	.close	   = pcm_card_capture_ref_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = pcm_card_pb_hw_params,
	.hw_free   = pcm_card_hw_free_ref,
	.prepare   = pcm_card_capture_ref_prepare,
	.trigger   = pcm_card_capture_ref_trigger,
	.pointer   = pcm_card_pb_pcm_pointer,
};

static struct snd_pcm_ops card_playback_ops_1 = {
	.open	   = pcm_card_pb_ref_open_voip,
	.close	   = pcm_card_pb_ref_close_voip,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = pcm_card_pb_hw_params,
	.hw_free   = pcm_card_hw_free_ref_mic,
	.prepare   = pcm_card_pb_ref_prepare_voip,
	.trigger   = pcm_card_pb_ref_trigger_voip,
	.pointer   = pcm_card_pb_pcm_pointer,
};

static struct snd_pcm_ops card_capture_ops_1 = {
	.open	   = pcm_card_capture_ref_open_mic,
	.close	   = pcm_card_capture_ref_close_mic,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = pcm_card_pb_hw_params,
	.hw_free   = pcm_card_hw_free_ref_mic,
	.prepare   = pcm_card_capture_ref_prepare_mic,
	.trigger   = pcm_card_capture_ref_trigger_mic,
	.pointer   = pcm_card_pb_pcm_pointer,
};


#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
static int32_t card_audio_suspend(struct platform_device *dev, pm_message_t state)
{
	PCM_DEBUG(LOG_TAG, "card_audio_suspend: Start\r\n");
	AudDev_PowerDown(AUD_DEVICE_ID_GPSMIX);
	PCM_DEBUG(LOG_TAG, "card_audio_suspend: End\r\n");

	return NOERR;
}

static int32_t card_audio_resume(struct platform_device *dev)
{
	PCM_DEBUG(LOG_TAG, "card_audio_resume: Start\r\n");
	AudDev_PowerOn(AUD_DEVICE_ID_GPSMIX);
	PCM_DEBUG(LOG_TAG, "card_audio_resume: End\r\n");

	return NOERR;
}
#endif

static int32_t card_audio_probe(struct platform_device *dev)
{
	struct snd_card *card = NULL;
	struct snd_pcm *pcm = NULL;
	int ctl_private = -1;
	int32_t err = -1;

	err = (int32_t)snd_card_new(&dev->dev, -1, id[dev->id], THIS_MODULE, 0, &card);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe: snd_card_new err(%i)!\r\n", err);
		return err;
	}

	err = snd_create(card, &snd_chip);

	if(err < 0) {
		snd_card_free(card);
		PCM_ERROR(LOG_TAG, "card_audio_probe: snd_create err(%i)!\r\n", err);
		return err;
	}

	card->private_data = snd_chip;
	strcpy(card->driver, "ac_83xx");
	strcpy(card->shortname, "ac_83xx (Solo-1)");
	sprintf(card->longname, "%s rev %i, irq %i",
		card->shortname, snd_chip->revision, snd_chip->irq);

	err = snd_pcm_new(card, "card_pcm_drv", 0, MAX_PLAYBACK_STREAMS, 1, &pcm);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe: snd_pcm_new err(%i)!\r\n", err);
		return err;
	}

	pcm->private_data = snd_chip;
	pcm->info_flags = 0;
	err = snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS,
						    snd_dma_continuous_data(GFP_KERNEL), 64 * 1024, 64 * 1024);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe: snd_pcm_lib_preallocate_pages_for_all err(%i)!\r\n", err);
		return err;
	}
	sema_init(&g_rPCMSema, 1);

	snd_chip->pcm = pcm;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &card_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &card_capture_ops);
	snd_ctl_add(card, snd_ctl_new1(&card_playback_vol, &ctl_private));

	err = snd_card_register(card);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe: snd_card_register err(%i)!\r\n", err);
		return err;
	}

    MOD_VERSION_INFO(ALSA_MOD_NAME, ALSA_VER_MAIN, ALSA_VER_MINOR, ALSA_VER_REV);

	return NOERR;
}

static int32_t card_audio_probe_user(struct platform_device *dev)
{
	struct snd_card *card = NULL;
	struct snd_pcm *pcm[DEVICENUM] = {0};
	char pcm_id[16] = { 0 };

	int ctl_private = -1;
	int32_t err = -1;
    int i = 0;

	err = (int32_t)snd_card_new(&dev->dev, -1, id[dev->id], THIS_MODULE, 0, &card);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe_user: snd_card_new err(%i)!\r\n", err);
		return err;
	}

	err = snd_create(card, &snd_chip);

	if(err < 0) {
		snd_card_free(card);
		PCM_ERROR(LOG_TAG, "card_audio_probe_user: snd_create err(%i)!\r\n", err);
		return err;
	}

	card->private_data = snd_chip;
	strcpy(card->driver, "ac_83xx");
	strcpy(card->shortname, "ac_83xx (Solo-1)");
	sprintf(card->longname, "%s rev %i, irq %i",
		card->shortname, snd_chip->revision, snd_chip->irq);
    for(i = 0;i < DEVICENUM;i++){
		memset(pcm_id,0,16);
		sprintf(pcm_id,"%s_%d","card_pcm",i);
        err = snd_pcm_new(card, pcm_id, i, MAX_PLAYBACK_STREAMS, 1, &pcm[i]);
	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe_user: snd_pcm_new err(%i)!\r\n", err);
		return err;
	}

		err = snd_pcm_lib_preallocate_pages_for_all(pcm[i], SNDRV_DMA_TYPE_CONTINUOUS,
						    snd_dma_continuous_data(GFP_KERNEL), 64 * 1024, 64 * 1024);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe_user: snd_pcm_lib_preallocate_pages_for_all err(%i)!\r\n", err);
		return err;
	}
		if( i == 0)
		{
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_PLAYBACK, &card_playback_ops);
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_CAPTURE, &card_capture_ops);
		}
		else if(i == 1)
		{
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_PLAYBACK, &card_playback_ref_ops);
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_CAPTURE, &card_capture_ref_ops);
		}
		else
		{
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_PLAYBACK, &card_playback_ops_1);
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_CAPTURE, &card_capture_ops_1);
		}
		//snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_PLAYBACK, (i==0)?&card_playback_ops:&card_playback_ref_ops);
		//snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_CAPTURE, (i==0)?&card_capture_ops:&card_capture_ref_ops);
		pcm[i]->private_data = snd_chip;
		pcm[i]->info_flags = 0;
	}

	sema_init(&g_rPCMSema, 1);
    //sema_init(&g_rPCMSema_tx, 1);
	sema_init(&g_rPCMSema_ref, 1);
	sema_init(&g_rPCMSema_ref_mic, 1);

    snd_chip->pcm = pcm[0];
    snd_chip->m_u4SampleRate = 0;
    pr_debug("sizeof(ATC_card_controls:%d\n",sizeof(ATC_card_controls)/sizeof(ATC_card_controls[0]));
    for(i = 0;i < sizeof(ATC_card_controls)/sizeof(ATC_card_controls[0]);i++)
	snd_ctl_add(card, snd_ctl_new1(&ATC_card_controls[i], &ctl_private));
	//snd_ctl_add(card, snd_ctl_new1(&card_playback_vol, &ctl_private));

	err = snd_card_register(card);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe_user: snd_card_register err(%i)!\r\n", err);
		return err;
	}

    MOD_VERSION_INFO(ALSA_MOD_NAME, ALSA_VER_MAIN, ALSA_VER_MINOR, ALSA_VER_REV);

	return NOERR;
}

static int32_t __exit card_audio_remove(struct platform_device *dev)
{
	snd_card_free(platform_get_drvdata(dev));
	platform_set_drvdata(dev, NULL);

	AsrcMgr_UnInit();
	DspMixOut_UnInit();
	MicIn_UnInit();

	return NOERR;
}

static struct platform_driver card_audio_driver = {
	#if AEC_NDC_USERSPACE
	.probe = card_audio_probe_user,
	#else
	.probe = card_audio_probe,
	#endif
	.remove = __exit_p(card_audio_remove),
#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
	.suspend = card_audio_suspend,
	.resume = card_audio_resume,
#endif
	.driver = {
		.name = "card_ALSA",
	},
};

/***************************miscdev********************************/
#include "pcm_audconf.h"
static s32 pcm_aud_ioctl(struct file *filp, u32 cmd, u32 arg)
{
	#if AEC_NDC_USERSPACE

    if(false == fgInit) {
		pr_debug("pcm_aud_ioctl: alsa pcm driver has not loaded completely\n");
		return -EPERM;
	}

	if((!snd_chip) || (!snd_chip->m_Intialized)) {
		PCM_ERROR(LOG_TAG, "pcm_aud_ioctl: AUD_VERIFY snd_chip err(%i)!\r\n", -EPERM);
		return -EPERM;
	}

    switch(cmd)
    {
    	case BT_SCO_DISABLE:
            pr_debug("mute Bt call BT_SCO_DISABLE\n");
            if((snd_chip->m_u4pcmBt == 1)&& (snd_chip->m_u4SampleRate == SAMPLE_RATE_8000 || snd_chip->m_u4SampleRate == SAMPLE_RATE_16000)){
                snd_chip->m_u4BtMute = 1;
                pr_debug("ioctl mute the BT snd_chip->m_u4BtMute:%d\n",snd_chip->m_u4BtMute);
            }
    		break;
        default:
    		break;
	}

	return 0;
	#else
	int32_t i4ret = 0;
	u32 u4Data = 0;
	PCM_VOLUME rPCMVolume = {0};
	PCM_DTMF_CONF rDTMFConf = {0};
	PCM_DTMF_THRESHOLD rDTMFThres = {0};
	WIN32_IOCTL_DATA win_ioctl = {0};

	if(false == fgInit) {
		PCM_ERROR(LOG_TAG, "pcm_aud_ioctl: alsa pcm driver has not loaded completely!\r\n");
		return -EPERM;
	}

	if((!snd_chip) || (!snd_chip->m_Intialized)) {
		PCM_ERROR(LOG_TAG, "pcm_aud_ioctl: AUD_VERIFY snd_chip err(%i)!\r\n", -EPERM);
		return -EPERM;
	}

	if(copy_from_user((void *)&win_ioctl, (void *)arg, sizeof(win_ioctl))) {
		PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
		return (-EPERM);
	}

	down(&g_rPCMSema);

	switch(cmd) {
	case BT_SCO_DISABLE:
		i4ret = pcm_audconf_ioctl(0, cmd, &u4Data, sizeof(u4Data));
		break;

	case SET_BT_SPH_GAIN:
	case SET_DEVICE_SPH_GAIN:
		if(copy_from_user((void *)(&rPCMVolume), win_ioctl.pInBuf, sizeof(rPCMVolume))) {
			PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
			return (-EPERM);
		}

		i4ret = pcm_audconf_ioctl(0, cmd, (void *)(&rPCMVolume), sizeof(rPCMVolume));
		break;

	case GET_BT_SPH_GAIN:
	case GET_DEVICE_SPH_GAIN:
		i4ret = pcm_audconf_ioctl(0, cmd, (void *)(&rPCMVolume), sizeof(rPCMVolume));

		if(copy_to_user(win_ioctl.pOutBuf, (void *)(&rPCMVolume), sizeof(rPCMVolume))) {
			PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
			return (-EPERM);
		}

		break;

	case SET_MIC_MUTE:
	case SET_DSP_MIX_CH:
	case SET_PRIMARY_MIC:
	case SET_SPH_DELAY:
	case SET_SPH_MIC_GAIN:
	case BT_SCO_ENABLE:
    case SET_CAPTURE_NDC_ENABLE:
		if(copy_from_user((void *)(&u4Data), win_ioctl.pInBuf, sizeof(u4Data))) {

			PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
			return (-EPERM);
		}

		i4ret = pcm_audconf_ioctl(0, cmd, (void *)(&u4Data), sizeof(u4Data));
		break;

	case GET_SPH_DELAY:
	case GET_SPH_MIC_GAIN:
		i4ret = pcm_audconf_ioctl(0, cmd, (void *)(&u4Data), sizeof(u4Data));

		if(copy_to_user(win_ioctl.pOutBuf, (void *)(&u4Data), sizeof(u4Data))) {

			PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
			return (-EPERM);
		}

		break;

#if (ENABLE_DTMF_FUNCTION)

	case SET_PCM_DTMF_CTRL:
	case SET_PCM_DTMF_NOISE_RATIO:
	case SET_PCM_DTMF_VALID_TIME:
	case SET_PCM_DTMF_INVALID_TIME:
	case SET_PCM_DTMF_NEW_SAMPLES:
	case SET_PCM_DTMF_MAX_SCALE:
	case SET_PCM_DTMF_INFO_SENDER_EN:
	case SET_PCM_DTMF_INFO_SENDER_WRITE:
	case SET_PCM_DTMF_TEST_USE_FILE:
		if(copy_from_user((void *)(&rDTMFConf), win_ioctl.pInBuf, sizeof(rDTMFConf))) {

			PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
			return (-EPERM);
		}

		i4ret = pcm_audconf_ioctl(0, cmd, (void *)(&rDTMFConf), sizeof(rDTMFConf));
		break;

	case SET_PCM_DTMF_THRESHOLD:
		if(copy_from_user((void *)(&rDTMFThres), win_ioctl.pInBuf, sizeof(rDTMFThres))) {
			PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
			return (-EPERM);
		}

		i4ret = pcm_audconf_ioctl(0, cmd, (void *)(&rDTMFThres), sizeof(rDTMFThres));
		break;
#endif

	default:
		PCM_ERROR(LOG_TAG, "pcm_aud_ioctl: unknown cmd(0x%02x)\r\n", (cmd - SET_SPH_DELAY) >> 2);
		break;
	}

	up(&g_rPCMSema);

	return i4ret;
	#endif
}

static int32_t pcm_aud_read(struct file *filp, char __user *buf, u32 count, loff_t *f_pos)
{
	int32_t i4ret = 0;

	PCM_ERROR(LOG_TAG, "We don't support read function!\r\n");

	return i4ret;
}

static int32_t pcm_aud_write(struct file *filp, const char __user *buf, u32 count, loff_t *f_pos)
{
	int32_t i4ret = 0;

	PCM_ERROR(LOG_TAG, "We don't support write function!\r\n");

	return i4ret;
}


const struct file_operations pcm_aud_fops = {
	.read = pcm_aud_read,
	.write = pcm_aud_write,
	.unlocked_ioctl = pcm_aud_ioctl,
};

static struct miscdevice pcm_aud_dev = {
	/*
	 * We don't care what minor number we end up with, so tell the kernel to just pick one.
	 */
	.minor = MISC_DYNAMIC_MINOR,
	/*
	 * Name ourselves /dev/pcm_aud.
	 */
	.name = "pcm_aud",
	/*
	 * What functions to call when a program performs file operations on the device.
	 */
	.fops = &pcm_aud_fops,
};

/************************miscdev***************************/
#define USING_OLD_PCM_DRIVER 	0

#if 0//USING_OLD_PCM_DRIVER
int32_t card_audio_init(void)
{
	int err = 0;

	err = misc_register(&pcm_aud_dev);

	if(err) {
		PCM_ERROR(LOG_TAG, "card_audio_init: misc_register err(%i)!\r\n", err);
		return err;
	}

	err = os_driver_register(&card_audio_driver);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_init: os_driver_register err(%i)!\r\n", err);
		os_driver_unregister(&card_audio_driver);
		return err;
	}

	card_device = os_device_register_simple("card_ALSA", -1, NULL, 0);

	PCM_INFO(LOG_TAG, "card_audio_init: success!\r\n");
	fgInit = true;

	return NOERR;
}

void __exit card_audio_exit(void)
{
	os_device_unregister(card_device);

	os_driver_unregister(&card_audio_driver);
	misc_deregister(&pcm_aud_dev);

	fgInit = false;
}

int32_t card_audio_hibernation(bool fgWakeUp)
{
	DspMixOut_HibernationCtrl(fgWakeUp);
	Asrc_HibernationCtrl(fgWakeUp);
	SpeechDev_HibernationCtrl(fgWakeUp);
	fgInit = fgWakeUp;

	return NOERR;
}

#else
#include "apcm_scdrv.h"

int32_t card_audio_init(void)
{
	return scdrv_init();
}


void __exit card_audio_exit(void)
{
	scdrv_exit();
}


int32_t card_audio_hibernation(bool fgWakeUp)
{
	return scdrv_hibernation(fgWakeUp);
}

#endif

