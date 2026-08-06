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
#include "enhrecord.h"
#include "aud_pcm_dbg.h"
#include "aud_power.h"
#include "pcm_hardware.h"
#include "input.h"

#include "pcm_debug.h"
#define LOG_TAG "pcm_ac83xx"

struct semaphore g_rPCMSema;

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
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
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
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
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
	kfree(runtime->private_data);
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
	}
    set_user_nice(capture_stream->CopyMicDataThread_task, PCM_TASK_THREAD_NICE_PRIORITY);
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
	substrm_data->real_last_ptr = 0;
	substrm_data->Used_size = 0;
	substrm_data->IsBtSpeech = 0;
	substrm_data->appl_ptr = 0;
	substrm_data->app_Base = 0;
	substrm_data->hw_Base = 0;
	substrm_data->real_hw_Base = 0;
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
	substrm_data->real_last_ptr = 0;
	substrm_data->Used_size = 0;
	substrm_data->appl_ptr = 0;
	substrm_data->app_Base = 0;
	substrm_data->hw_Base = 0;
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

	return (snd_pcm_uframes_t)substrm_data->hw_ofs;
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

static int32_t pcm_card_pb_vol_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 10;
	uinfo->value.integer.step = 1;

	return NOERR;
}

static int32_t pcm_card_pb_vol_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	int32_t global_volume = uvalue->value.integer.value[0];

	PCM_DEBUG(LOG_TAG, "pcm_card_pb_vol_put: global_volume=%d.\r\n", global_volume);

	return NOERR;
}

static int32_t pcm_card_pb_vol_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}

/* begin of enhcap */

static int32_t pcm_card_enhcap_open(struct snd_pcm_substream  *substream)
{
    int err = NOERR;
    struct snd_pcm_runtime *runtime = substream->runtime;

    PVOID enhRecord = (PVOID)EnhRecord_Open(substream);
    if (enhRecord) {
        down(&g_rPCMSema);
        runtime->private_data = enhRecord;
        runtime->private_free = snd_pcm_free_substream;
        runtime->hw = card_capture;
        EnhRecord_CreateThread(enhRecord);  // TODO: Should call this in open ?
        up(&g_rPCMSema);
    } else {
        err = (-ENOMEM);
    }

    //PRINTMSG(ZONE_INFO && (err == NOERR),
    //    (TEXT("pcm_card_enhcap_open: Success, substream->number=%i (2018-08-24)\r\n"), substream->number));//cgx todo

    return (err);
}

static int32_t pcm_card_enhcap_close(struct snd_pcm_substream *substream)
{
    //PRINTMSG(ZONE_INFO, (TEXT("pcm_card_enhcap_close: substream->number=%i  >>>\r\n"), substream->number));//cgx todo
    EnhRecord_Close(substream->runtime->private_data);
    //PRINTMSG(ZONE_INFO, (TEXT("pcm_card_enhcap_close: substream->number=%i  <<<\r\n"), substream->number));//cgx todo

    return (NOERR);
}

static int32_t pcm_card_enhcap_prepare(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;

    //PRINTMSG(ZONE_INFO, (TEXT("pcm_card_enhcap_prepare: substream->number=%i\r\n"), substream->number));//cgx todo
    if (NULL == runtime) {
        //PRINTMSG(ZONE_INFO, (TEXT("pcm_card_enhcap_prepare: substream->runtime=%p\r\n"), runtime));//cgx todo
    }
    //PRINTMSG(ZONE_INFO, (TEXT("pcm_card_enhcap_prepare: substream->runtime=%p\r\n"), runtime));//cgx todo
    //PRINTMSG(ZONE_INFO, (TEXT("pcm_card_enhcap_prepare: substream->runtime->private_data=%p\r\n"), runtime->private_data));//cgx todo
    EnhRecord_Prepare((PEnhRecord)(runtime->private_data));

    return (NOERR);
}

static int32_t pcm_card_enhcap_trigger(struct snd_pcm_substream *substream, int cmd)
{
    int32_t i4Ret = NOERR;
    //PRINTMSG(ZONE_INFO, (TEXT("pcm_card_enhcap_trigger: cmd = %d\r\n"), (int)cmd));//cgx todo

    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        //PRINTMSG(ZONE_DBG, (TEXT("pcm_card_capture_trigger: START(%i) rate(%d)\r\n"), substream->number, (int)substream->runtime->rate));//cgx todo
        EnhRecord_Start(substream->runtime->private_data);  // TODO: add sample rate support
        break;

    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        //PRINTMSG(ZONE_DBG, (TEXT("pcm_card_enhcap_trigger: STOP(%i).\r\n"), substream->number));//cgx todo
        EnhRecord_Stop(substream->runtime->private_data);
        //PRINTMSG(ZONE_DBG, (TEXT("pcm_card_enhcap_trigger: STOP(0x%p) Down.\r\n"), substream));//cgx todo
        break;

    default:
        //PRINTMSG(ZONE_ERROR, (TEXT("pcm_card_enhcap_trigger: EINVAL.\r\n")));//cgx todo
        i4Ret = (-EINVAL);
    }

    return (i4Ret);
}
/* end of enhcap */

static struct snd_kcontrol_new card_playback_vol = {
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "card Volum",
	.index = 0,
	.info = pcm_card_pb_vol_info,
	.get = pcm_card_pb_vol_get,
	.put = pcm_card_pb_vol_put,
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




static struct snd_pcm_hardware bt_ul_playback = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = (SNDRV_PCM_FMTBIT_S16_LE),
	.rates = (SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
	.rate_min = 8000,
	.rate_max = 16000,
	.channels_min = 1,
	.channels_max = 1,
	.buffer_bytes_max = 64 * 1024,
	.period_bytes_min	 = 64,
	.period_bytes_max	 = 64 * 1024,
	.periods_min		= 1,
	.periods_max		= 1024,
	.fifo_size		  = 0,
};


static struct snd_pcm_hardware bt_dl_capture = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = (SNDRV_PCM_FMTBIT_S16_LE ),
	.rates = (SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
	.rate_min = 8000,
	.rate_max = 16000,
	.channels_min = 1,
	.channels_max = 1,
	.buffer_bytes_max = 64 * 1024,
	.period_bytes_min = 64,
	.period_bytes_max = 64 * 1024,
	.periods_min	  = 1,
	.periods_max	  = 1024,
	.fifo_size		  = 0,
};



typedef struct Downlink_Capture_Info {
    struct snd_pcm_substream * substream;
    int hwPos;  /* in frames*/
    int start;
    struct semaphore sem;
}Downlink_Capture_Info;

typedef struct Uplink_Playback_Info {
    struct snd_pcm_substream * substream;
    int hwPos;  /* in frames*/
    int start;
    struct semaphore sem;
}Uplink_Playback_Info;


static Downlink_Capture_Info    gDlCaptureInfo;
static Uplink_Playback_Info     gUlPlaybackInfo;


static void pcm_bt_sem_init(void)
{
    sema_init(&gUlPlaybackInfo.sem, 1);
    sema_init(&gDlCaptureInfo.sem, 1);
    return;
}

static void dl_capture_down(void)
{
    down(&gDlCaptureInfo.sem);
    return;
}

static void dl_capture_up(void)
{
    up(&gDlCaptureInfo.sem);
    return;
}

static void ul_playback_down(void)
{
    down(&gUlPlaybackInfo.sem);
    return;
}

static void ul_playback_up(void)
{
    up(&gUlPlaybackInfo.sem);
    return;
}


static void clear_downlink_capture(void)
{
    gDlCaptureInfo.substream = NULL;
    gDlCaptureInfo.hwPos = 0;
    gDlCaptureInfo.start = 0;
    return;
}

static void clear_uplink_playback(void)
{
    gUlPlaybackInfo.substream = NULL;
    gUlPlaybackInfo.hwPos = 0;
    gUlPlaybackInfo.start = 0;
    return;
}

static void start_downlink_capture(struct snd_pcm_substream *substream)
{
    gDlCaptureInfo.substream = substream;
    gDlCaptureInfo.start = 1;
    return;
}

static void start_uplink_playback(struct snd_pcm_substream *substream)
{
    gUlPlaybackInfo.substream = substream;
    gUlPlaybackInfo.start = 1;
    return;
}

int get_uplink_playback_hwpos(void)
{
    return gUlPlaybackInfo.hwPos;
}

int get_downlink_capture_hwpos(void)
{
    return gDlCaptureInfo.hwPos;
}

static  struct snd_pcm_substream* get_uplink_playback_substream(void)
{
    return gUlPlaybackInfo.substream;
}



static struct snd_pcm_substream* get_downlink_capture_substream(void)
{
    return gDlCaptureInfo.substream;
}

static  int is_uplink_playback_start(void)
{
    return gUlPlaybackInfo.start;
}

static  int is_downlink_capture_start(void)
{
    return gDlCaptureInfo.start;
}


static void step_downlink_capture(int count)
{
    struct snd_pcm_substream* substream = gDlCaptureInfo.substream;
    struct snd_pcm_runtime *runtime = NULL;
    static snd_pcm_uframes_t used = 0;

    if(NULL == substream)
    {
        //error
        return;
    }
    runtime = substream->runtime;
    if(NULL == runtime)
    {
        //error
        return;
    }

    gDlCaptureInfo.hwPos += count;
    if(gDlCaptureInfo.hwPos >= runtime->buffer_size)
    {
        gDlCaptureInfo.hwPos  %= runtime->buffer_size;
    }

    used += count;
    if(used >= runtime->period_size)
    {
        snd_pcm_period_elapsed(substream);
        used = 0;
    }
    return;
}

static void step_uplink_playback(int count)

{
    struct snd_pcm_substream* substream = gUlPlaybackInfo.substream;
    struct snd_pcm_runtime *runtime = NULL;
    static snd_pcm_uframes_t used = 0;

    if(NULL == substream)
    {
        //error
        return;
    }
    runtime = substream->runtime;
    if(NULL == runtime)
    {
        //error
        return;
    }

    gUlPlaybackInfo.hwPos += count;
    if(gUlPlaybackInfo.hwPos >= runtime->buffer_size)
    {
        gUlPlaybackInfo.hwPos  = gUlPlaybackInfo.hwPos % runtime->buffer_size;
    }

    used += count;
    if(used >= runtime->period_size)
    {
        snd_pcm_period_elapsed(substream);
        used -= runtime->period_size;
    }
    return;
}

static void bt_snd_pcm_free_substream(struct snd_pcm_runtime *runtime)
{
	pr_debug("[PCM] free substream(%x)\n", runtime->private_data);
	kfree(runtime->private_data);
}


static int32_t bt_ul_playback_open(struct snd_pcm_substream  *substream)
{
	ac_83xx *chip = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	substrm_data = kzalloc(sizeof(*substrm_data), GFP_KERNEL);

	if(substrm_data == NULL) {
		return -ENOMEM;
	}

	chip = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;
	substrm_data->ac83xx_chip = chip;
	substrm_data->substream = substream;

	runtime->private_data = substrm_data;
	runtime->private_free = bt_snd_pcm_free_substream;

	runtime->hw = bt_ul_playback;

	if(substream->number >= 1) {
		pr_err("[PCM ERR]bt_ul_playback_open: error substream->number=%i\r\n", substream->number);
		return -EINVAL;
	}

	return NOERR;
}

static int32_t bt_dl_capture_open(struct snd_pcm_substream  *substream)
{
	ac_83xx *chip = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = NULL;

	substrm_data = kzalloc(sizeof(*substrm_data), GFP_KERNEL);

	if(substrm_data == NULL) {
		return -ENOMEM;
	}

	chip = snd_pcm_substream_chip(substream);
	runtime = substream->runtime;
	substrm_data->ac83xx_chip = chip;
	substrm_data->substream = substream;

	runtime->private_data = substrm_data;
	runtime->private_free = bt_snd_pcm_free_substream;

	runtime->hw = bt_dl_capture;

	if(substream->number >= 1) {
		pr_err("[PCM ERR]bt_dl_capture_open: error substream->number=%i\r\n", substream->number);
		return -EINVAL;
	}

	return NOERR;
}


static int32_t bt_close(struct snd_pcm_substream *substream)
{
	return NOERR;
}

static int32_t bt_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *hw_params)
{
	int32_t err = -1;
	int i  = 0;
	int val = 0;

    struct snd_pcm_runtime *runtime = substream->runtime;

	err = (int32_t)snd_pcm_lib_malloc_pages(substream, (u32)params_buffer_bytes(hw_params));

	if(err < 0) {
		pr_err("[PCM ERR]bt_hw_params: snd_pcm_lib_malloc_pages err(%i)! \r\n", err);

		return err;
	}


	pr_debug("[PCM]bt_hw_params: substream->number=%i\r\n", substream->number);

	return NOERR;
}



static int32_t bt_ul_playback_hw_free(struct snd_pcm_substream *substream)
{
	int32_t i4Ret = -1;

	pr_debug("[PCM]bt_hw_free: substream->number=%i\r\n", substream->number);

	ul_playback_down();
	i4Ret = snd_pcm_lib_free_pages(substream);
	ul_playback_up();

	return i4Ret;
}


static int32_t bt_dl_capture_hw_free(struct snd_pcm_substream *substream)
{
	int32_t i4Ret = -1;

	pr_debug("[PCM]bt_hw_free: substream->number=%i\r\n", substream->number);

	dl_capture_down();
	i4Ret = snd_pcm_lib_free_pages(substream);
	dl_capture_up();

	return i4Ret;
}


static int32_t bt_prepare(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;

	return NOERR;
}

static int32_t bt_ul_playback_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = NULL;
	runtime = substream->runtime;

	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		start_uplink_playback(substream);
		BtPCMHw_StartEx(runtime->rate, HW_MODE_TX);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		clear_uplink_playback();
		BtPCMHw_StopEx(HW_MODE_TX);
		break;

	default:
		pr_err("[PCM ERR]bt_ul_playback_trigger: EINVAL.\r\n");
		return (-EINVAL);
	}
	return NOERR;
}


static int32_t bt_dl_capture_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = NULL;
	runtime = substream->runtime;

	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		start_downlink_capture(substream);
		BtPCMHw_StartEx(runtime->rate, HW_MODE_RX);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		clear_downlink_capture();
		BtPCMHw_StopEx(HW_MODE_RX);
		break;

	default:
		pr_err("[PCM ERR]bt_dl_capture_trigger: EINVAL.\r\n");
		return (-EINVAL);
	}
	return NOERR;
}



static snd_pcm_uframes_t bt_ul_playback_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;


	return (snd_pcm_uframes_t)get_uplink_playback_hwpos();


}

static snd_pcm_uframes_t bt_dl_capture_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	return (snd_pcm_uframes_t) get_downlink_capture_hwpos();
}



static struct snd_pcm_ops bt_ul_playback_ops = {
	.open	   = bt_ul_playback_open,
	.close	   = bt_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = bt_hw_params,
	.hw_free   = bt_ul_playback_hw_free,
	.prepare   = bt_prepare,
	.trigger   = bt_ul_playback_trigger,
	.pointer   = bt_ul_playback_pointer,
};

static struct snd_pcm_ops bt_dl_capture_ops = {
	.open	   = bt_dl_capture_open,
	.close	   = bt_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = bt_hw_params,
	.hw_free   = bt_dl_capture_hw_free,
	.prepare   = bt_prepare,
	.trigger   = bt_dl_capture_trigger,
	.pointer   = bt_dl_capture_pointer,
};


/* add for enhance capture */
static struct snd_pcm_ops card_enhcap_ops = {
    .open      = pcm_card_enhcap_open,
    .close     = pcm_card_enhcap_close,
    .ioctl     = snd_pcm_lib_ioctl,
    .hw_params = pcm_card_pb_hw_params,
    .hw_free   = pcm_card_hw_free,
    .prepare   = pcm_card_enhcap_prepare,
    .trigger   = pcm_card_enhcap_trigger,
    .pointer   = pcm_card_pb_pcm_pointer,
};

static int playback2BufByFrames(void *data, unsigned char * buffer, int frameSize)
{
    int hwpos = get_uplink_playback_hwpos();

    struct snd_pcm_substream * substream = get_uplink_playback_substream();
    struct snd_pcm_runtime * runtime = NULL;

    if(substream)
    {
        runtime = substream->runtime;
    }

    if(runtime  && is_uplink_playback_start())
    {
        int size = frameSize;
        int len = 0;
        int len2 = 0;
        int playBufferSize = runtime->buffer_size;

        int hwAvail = snd_pcm_playback_hw_avail(runtime);
        if(size > hwAvail)
        {
            PCM_DEBUG(LOG_TAG, "app not write enough data to playback buffer\r\n");
            return -1;
        }

        if((hwpos + size) >= playBufferSize)
        {
            len = frames_to_bytes(runtime, (playBufferSize - hwpos));
            len2 = frames_to_bytes(runtime, size) - len;
            memcpy(buffer,
                    runtime->dma_area + frames_to_bytes(runtime, hwpos),
                    len);
            memcpy(buffer + len,
                    runtime->dma_area,
                    len2);
        }
        else
        {
            memcpy(buffer,
                    runtime->dma_area + frames_to_bytes(runtime, hwpos),
                    frames_to_bytes(runtime, size));
        }

        step_uplink_playback(size);
        return (size);

    }
    return (0);
}




static int  playback2BufByBytes(void *data, unsigned char * buffer, int ByteSize)
{
    struct snd_pcm_substream * substream = get_uplink_playback_substream();
    struct snd_pcm_runtime * runtime = NULL;

    if(substream)
    {
        runtime = substream->runtime;
    }

    if(NULL != runtime && is_uplink_playback_start())
    {
        int frameSize = ByteSize / frames_to_bytes(runtime, 1);

        return playback2BufByFrames(data, buffer, frameSize) * frames_to_bytes(runtime, 1);

    }
    return 0;

}

int  playback2Buf(void *data, unsigned char * buffer, int ByteSize)
{
    int ret = 0;
    ul_playback_down();
    ret = playback2BufByBytes(data, buffer, ByteSize);
    ul_playback_up();

    return ret;
}

int getPlaybackAvail(void)
{
    int ret = 0;
    struct snd_pcm_substream * substream = NULL;
    struct snd_pcm_runtime * runtime = NULL;
    ul_playback_down();
    substream = get_uplink_playback_substream();
    if(substream)
    {
        runtime = substream->runtime;
    }
    if(runtime  && is_uplink_playback_start())
    {
        ret = snd_pcm_playback_hw_avail(runtime);
    }
    ul_playback_up();

    return (ret);
}

static int  buf2CaptureByFrames(void *data, unsigned char * buffer, int frameSize)
{
    int hwpos = get_downlink_capture_hwpos();

    struct snd_pcm_substream * substream = get_downlink_capture_substream();
    struct snd_pcm_runtime * runtime = NULL;

    if(substream)
    {
        runtime = substream->runtime;
    }

    if(runtime  && is_downlink_capture_start())
    {
        int size = frameSize;
        int len = size;
        int len2 = 0;
        int recordBufferSize = runtime->buffer_size;

        int hwAvail = snd_pcm_capture_hw_avail(runtime);
        if(size > hwAvail)
        {
            size = hwAvail;
        }

        if( (hwpos + size) >= recordBufferSize)
        {
            len = frames_to_bytes(runtime, (recordBufferSize - hwpos));
            len2 = frames_to_bytes(runtime, size) - len;
            memcpy(runtime->dma_area + frames_to_bytes(runtime, hwpos),
                    buffer,
                    len);
            memcpy(runtime->dma_area,
                    buffer + len,
                    len2);
        }
        else
        {
            memcpy(runtime->dma_area + frames_to_bytes(runtime, hwpos),
                    buffer,
                    frames_to_bytes(runtime, size));
        }

        step_downlink_capture(size);

	    return (size);
    }


    return 0;

}

static int  buf2CaptureByBytes(void *data, unsigned char * buffer, int ByteSize)
{
    struct snd_pcm_substream * substream = get_downlink_capture_substream();
    struct snd_pcm_runtime * runtime = NULL;

    if(substream)
    {
        runtime = substream->runtime;
    }

    if(NULL != runtime && is_downlink_capture_start())
    {
        int frameSize = ByteSize / frames_to_bytes(runtime, 1);

        return buf2CaptureByFrames(data, buffer, frameSize) * frames_to_bytes(runtime, 1);
    }

	return (0);
}

int  buf2Capture(void *data, unsigned char * buffer, int ByteSize)
{
    int ret = 0;
    dl_capture_down();
    ret = buf2CaptureByBytes(data, buffer, ByteSize);
    dl_capture_up();
    return ret;
}


s32 debug_copy_data_thread(void *data)
{
    while(1)
    {
        if(is_downlink_capture_start() && is_uplink_playback_start())
        {


            int size = 16 * 1000  / 100;
            char buf[16 * 1000 * 2 / 100] ;

            //playback2BufByFrames(data, buf, size);
            playback2Buf(data, buf, size * 2);
            //buf2CaptureByFrames(data, buf, mul * size);
            buf2Capture(data, buf, size * 2);
        }
        msleep(10);

    }
	return (0);
}



static int pcm_bt_enhance_init(struct snd_card *card)
{
	int32_t err = -1;
    struct snd_pcm *pcm_bt_enhance = NULL;

    err = snd_pcm_new(card, "card_pcm_bt_enhance", 1, 1, 1, &pcm_bt_enhance);

    if(err < 0) {
        pr_err("[PCM ERR]card_audio_probe: snd_pcm_new err(%i)!\r\n", err);
        return err;
    }

    err = snd_pcm_lib_preallocate_pages_for_all(pcm_bt_enhance, SNDRV_DMA_TYPE_CONTINUOUS,
                            snd_dma_continuous_data(GFP_KERNEL), 64 * 1024, 64 * 1024);

    if(err < 0) {
        pr_err("[PCM ERR]card_audio_probe: snd_pcm_lib_preallocate_pages_for_all err(%i)!\r\n", err);
        return err;
    }
	snd_pcm_set_ops(pcm_bt_enhance, SNDRV_PCM_STREAM_PLAYBACK, &bt_ul_playback_ops);
	snd_pcm_set_ops(pcm_bt_enhance, SNDRV_PCM_STREAM_CAPTURE, &bt_dl_capture_ops);

	clear_uplink_playback();
    clear_downlink_capture();
    pcm_bt_sem_init();

    /*for debuging*/
	//struct task_struct *  copy_task = kthread_create(debug_copy_data_thread, (void *)0, "debug_copy_data_thread");
    //wake_up_process(copy_task);

    return 0;

}






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
    struct snd_pcm *pcm_enh = NULL;
	int ctl_private = -1;
	int32_t err = -1;

	err = (int32_t)snd_card_new(&dev->dev, 0, id[dev->id], THIS_MODULE, 0, &card);

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

	//err = snd_pcm_new(card, "card_pcm", 0, MAX_PLAYBACK_STREAMS, 1, &pcm);
	err = snd_pcm_new(card, "card_pcm", 0, MAX_PLAYBACK_STREAMS, 2, &pcm);

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

    if (!g_NormalRecordWithEnhance) {
        if ((err = snd_pcm_new(card, "enh_pcm", 2, 1, 1, &pcm_enh)) < 0) {
            //PRINTMSG(ZONE_ERROR, (TEXT("[PCM]card_audio_probe: snd_pcm_new err(%i)!\r\n"), err));//cgx todo
            return (err);
        }
        //PRINTMSG(ZONE_INFO, (TEXT("[PCM]card_audio_probe: snd_pcm_new success(enh_pcm)!\r\n")));//cgx todo
        pcm_enh->private_data = snd_chip;
        pcm_enh->info_flags = 0;
        err = snd_pcm_lib_preallocate_pages_for_all(pcm_enh, SNDRV_DMA_TYPE_CONTINUOUS,
                                                    snd_dma_continuous_data(GFP_KERNEL), 64 * 1024, 64 * 1024);
    }

    if (err < 0)
    {
        //PRINTMSG(ZONE_ERROR, (TEXT("[PCM]card_audio_probe: snd_pcm_lib_preallocate_pages_for_all err(%i)!\r\n"), err));//cgx todo
        return (err);
    }
	snd_chip->pcm = pcm;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &card_playback_ops);
    //PRINTMSG(ZONE_INFO, (TEXT("[PCM]card_audio_probe: snd_pcm_set_ops->card_playback_ops.\r\n")));//cgx todo
    if (g_NormalRecordWithEnhance) {
        snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &card_enhcap_ops);
        //PRINTMSG(ZONE_INFO, (TEXT("[PCM]card_audio_probe: snd_pcm_set_ops->card_enhcap_ops.\r\n")));//cgx todo
    } else {
        snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &card_capture_ops);//cgx test
        //PRINTMSG(ZONE_INFO, (TEXT("[PCM]card_audio_probe: snd_pcm_set_ops->card_capture_ops.\r\n")));//cgx todo
        snd_pcm_set_ops(pcm_enh, SNDRV_PCM_STREAM_CAPTURE, &card_enhcap_ops);
        //PRINTMSG(ZONE_INFO, (TEXT("[PCM]card_audio_probe: snd_pcm_set_ops->card_enhcap_ops.\r\n")));//cgx todo
    }
	snd_ctl_add(card, snd_ctl_new1(&card_playback_vol, &ctl_private));

    pcm_bt_enhance_init(card);
	err = snd_card_register(card);

	if(err < 0) {
		PCM_ERROR(LOG_TAG, "card_audio_probe: snd_card_register err(%i)!\r\n", err);
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
	.probe = card_audio_probe,
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
		PCM_ERROR(LOG_TAG, "pcm_aud_ioctl: verify snd_chip err(%i)!\r\n", -EPERM);
		return -EPERM;
	}

	if(copy_from_user((void *)&win_ioctl, (void *)arg, sizeof(win_ioctl))) {
		PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
		return (-EPERM);
	}

	down(&g_rPCMSema);

	switch(cmd) {
	case BT_SCO_DISABLE:
#ifdef BT_SCO_USERSPACE_ENHANCEMENT
#else
		i4ret = pcm_audconf_ioctl(0, cmd, &u4Data, sizeof(u4Data));
#endif
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
    case SET_CAPTURE_NDC_ENABLE:
		if(copy_from_user((void *)(&u4Data), win_ioctl.pInBuf, sizeof(u4Data))) {

			PCM_ERROR(LOG_TAG, "copy_from_user error err(%i)!\r\n", -EPERM);
			return (-EPERM);
		}

		i4ret = pcm_audconf_ioctl(0, cmd, (void *)(&u4Data), sizeof(u4Data));
		break;

    case BT_SCO_ENABLE:

#ifdef BT_SCO_USERSPACE_ENHANCEMENT
#else
        if(copy_from_user((void *)(&u4Data), win_ioctl.pInBuf, sizeof(u4Data))) {

            pr_err("[PCM ERR]copy_from_user error err(%i)!\r\n", -EPERM);
            return (-EPERM);
        }

        i4ret = pcm_audconf_ioctl(0, cmd, (void *)(&u4Data), sizeof(u4Data));
#endif
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



