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

#include "apcm_scdrv.h"

#include "apcm_stream.h"
#include "aud_power.h"


#define LOG_TAG 		"[scdrv]"

#define MAX_PLAYBACK_STREAMS 	3U
#define APCM_DEVICENUM 		4U

static DEFINE_SPINLOCK(scdrv_lock);

static scdrv_t *_scdrv = NULL;

static char *scdrv_id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;


static void scdrv_free_substream(struct snd_pcm_runtime *runtime)
{
	PR_D("free substream(%x) \n", runtime->private_data);
	//kfree(runtime->private_data);
}


static int32_t scdrv_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *hw_params)
{
	int32_t err = -1;
	spin_lock(&scdrv_lock);
	err = (int32_t)snd_pcm_lib_malloc_pages(substream, (u32)params_buffer_bytes(hw_params));
	if(err < 0) {
		PR_E("[hw_params] snd_pcm_lib_malloc_pages err(%i)! \n", err);
	} else {
		PR_D("[hw_params] substream->number = %i \n", substream->number);
		err = RET_NOERR;
	}

	spin_unlock(&scdrv_lock);
	return (err);
}


static int32_t scdrv_hw_free(struct snd_pcm_substream *substream)
{
	int32_t result = RET_NOERR;

	PR_D("[hw_free] substream->number = %i \n", substream->number);
	spin_lock(&scdrv_lock);
	result = snd_pcm_lib_free_pages(substream);
	spin_unlock(&scdrv_lock);

	return (result);
}

static int32_t scdrv_close(struct snd_pcm_substream *substream)
{
	PR_D("[close]  %p \n", substream->runtime->private_data);
	stream_close(substream->runtime->private_data);
	return NOERR;
}


static int32_t scdrv_prepare(struct snd_pcm_substream *substream)
{
	PR_D("[prepare]  %p \n", substream->runtime->private_data);
	stream_prepare(substream->runtime->private_data);
	return NOERR;
}


static int32_t scdrv_trigger(struct snd_pcm_substream *substream, int cmd)
{
	int32_t result = NOERR;
	PR_D("[trigger] cmd(%d). %p \n", cmd, substream->runtime->private_data);
	switch(cmd)
	{
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		stream_start(substream->runtime->private_data);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		stream_stop(substream->runtime->private_data);
		break;

	default:
		PR_E("[pb_trigger] EINVAL. \n");
		result = (-EINVAL);
	}
	return (result);
}


static snd_pcm_uframes_t scdrv_pointer(struct snd_pcm_substream *substream)
{
	return (stream_get_ptr(substream->runtime->private_data));
}


//===============================================================//

static struct snd_pcm_hardware _scdrv_pb_hw = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = (SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_U16_LE),
	.rates = SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = 64 * 1024,
	.period_bytes_min = 64,
	.period_bytes_max = 64 * 1024,
	.periods_min      = 1,
	.periods_max      = 1024,
	.fifo_size        = 0,
};


static int32_t scdrv_pb_open(struct snd_pcm_substream  *substream, strm_type_e type)
{
	int32_t result = NOERR;
	struct snd_pcm_runtime *runtime = substream->runtime;
	stream_t *strm = stream_open(substream, type);
	PR_D("[pb_open(%d)] strm(0x%p) substrm(0x%p) >>>\n", type, strm, substream);
	if (strm) {
		snd_pcm_substream_chip(substream);
		runtime->private_data = strm;
		runtime->private_free = scdrv_free_substream;
		runtime->hw = _scdrv_pb_hw;

		if (type != STRM_OUT_SPH_TX && type != STRM_OUT_SPH_RX) {
			if(substream->pcm->device & 1) {
				runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
				runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
			}

			if(substream->pcm->device & 2) {
				runtime->hw.info &= ~(SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_MMAP_VALID);
			}
		}
	} else {
		result = (-EINVAL);
	}
	PR_D("[pb_open(%d)] strm(0x%p) substrm(0x%p) result(%d) <<<\n", type, strm, substream, result);

	return (result);
}


static int32_t scdrv_pb_open_normal(struct snd_pcm_substream  *substream)
{
	return scdrv_pb_open(substream, STRM_OUT_NORMAL);
}


static int32_t scdrv_pb_open_voip(struct snd_pcm_substream  *substream)
{
	return scdrv_pb_open(substream, STRM_OUT_RESERVE);
}


static int32_t scdrv_pb_open_speech_tx(struct snd_pcm_substream  *substream)
{
	return scdrv_pb_open(substream, STRM_OUT_SPH_TX);
}


static int32_t scdrv_pb_open_speech_rx(struct snd_pcm_substream  *substream)
{
	return scdrv_pb_open(substream, STRM_OUT_SPH_RX);
}


static struct snd_pcm_ops _scdrv_pb_normal_ops = {
	.open	   = scdrv_pb_open_normal,
	.close	   = scdrv_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = scdrv_hw_params,
	.hw_free   = scdrv_hw_free,
	.prepare   = scdrv_prepare,
	.trigger   = scdrv_trigger,
	.pointer   = scdrv_pointer,
};


static struct snd_pcm_ops _scdrv_pb_voip_ops = {
	.open	   = scdrv_pb_open_voip,
	.close	   = scdrv_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = scdrv_hw_params,
	.hw_free   = scdrv_hw_free,
	.prepare   = scdrv_prepare,
	.trigger   = scdrv_trigger,
	.pointer   = scdrv_pointer,
};


static struct snd_pcm_ops _scdrv_pb_speech_tx_ops = {
	.open	   = scdrv_pb_open_speech_tx,
	.close	   = scdrv_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = scdrv_hw_params,
	.hw_free   = scdrv_hw_free,
	.prepare   = scdrv_prepare,
	.trigger   = scdrv_trigger,
	.pointer   = scdrv_pointer,
};


static struct snd_pcm_ops _scdrv_pb_speech_rx_ops = {
	.open	   = scdrv_pb_open_speech_rx,
	.close	   = scdrv_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = scdrv_hw_params,
	.hw_free   = scdrv_hw_free,
	.prepare   = scdrv_prepare,
	.trigger   = scdrv_trigger,
	.pointer   = scdrv_pointer,
};


//========================================================================//

static struct snd_pcm_hardware _scdrv_rec_hw = {
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
	.fifo_size	  = 0,
};


static int32_t scdrv_rec_open(struct snd_pcm_substream  *substream, strm_type_e type)
{
	int32_t result = NOERR;
	stream_t *strm = stream_open(substream, type);
	struct snd_pcm_runtime *runtime = substream->runtime;

	if (strm) {
		runtime->private_data = (void *)strm;
		runtime->private_free = scdrv_free_substream;
		runtime->hw = _scdrv_rec_hw;
	} else {
		result = (-ENOMEM);
	}

	return (result);
}


static int32_t scdrv_rec_open_normal(struct snd_pcm_substream  *substream)
{
	return scdrv_rec_open(substream, STRM_IN_MIC);
}


static int32_t scdrv_rec_open_speech_mic(struct snd_pcm_substream  *substream)
{
	return scdrv_rec_open(substream, STRM_IN_SPH_MIC);
}


static int32_t scdrv_rec_open_speech_ref(struct snd_pcm_substream  *substream)
{
	return scdrv_rec_open(substream, STRM_IN_SPH_REF);
}


static int32_t scdrv_rec_open_speech_rx(struct snd_pcm_substream  *substream)
{
	return scdrv_rec_open(substream, STRM_IN_SPH_RX);
}


static struct snd_pcm_ops _scdrv_rec_normal_ops = {
	.open	   = scdrv_rec_open_normal,
	.close	   = scdrv_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = scdrv_hw_params,
	.hw_free   = scdrv_hw_free,
	.prepare   = scdrv_prepare,
	.trigger   = scdrv_trigger,
	.pointer   = scdrv_pointer,
};


static struct snd_pcm_ops _scdrv_rec_speech_mic_ops = {
	.open	   = scdrv_rec_open_speech_mic,
	.close	   = scdrv_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = scdrv_hw_params,
	.hw_free   = scdrv_hw_free,
	.prepare   = scdrv_prepare,
	.trigger   = scdrv_trigger,
	.pointer   = scdrv_pointer,
};


static struct snd_pcm_ops _scdrv_rec_speech_ref_ops = {
	.open	   = scdrv_rec_open_speech_ref,
	.close	   = scdrv_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = scdrv_hw_params,
	.hw_free   = scdrv_hw_free,
	.prepare   = scdrv_prepare,
	.trigger   = scdrv_trigger,
	.pointer   = scdrv_pointer,
};


static struct snd_pcm_ops _scdrv_rec_speech_rx_ops = {
	.open	   = scdrv_rec_open_speech_rx,
	.close	   = scdrv_close,
	.ioctl	   = snd_pcm_lib_ioctl,
	.hw_params = scdrv_hw_params,
	.hw_free   = scdrv_hw_free,
	.prepare   = scdrv_prepare,
	.trigger   = scdrv_trigger,
	.pointer   = scdrv_pointer,
};


//=================================================================//

static int32_t scdrv_pb_vol_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
        uinfo->value.integer.max = 256;
	uinfo->value.integer.step = 1;

	return NOERR;
}

static int32_t scdrv_pb_vol_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	int32_t vol = uvalue->value.integer.value[0];
	u32 gain = VOL_0dB * vol / VOL_0dB_SETTING;
	PR_D("[vol_put] vol(%d) => gain(0x%x) \n", vol, gain);

	gain = (gain << 16) + gain;
	outhw_set_gain(APCM_BT_CALL_MODE, gain);
	return (NOERR);
}


static int32_t scdrv_pb_vol_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
    	return NOERR;
}


static int32_t scdrv_bt_fs_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}


static int32_t scdrv_bt_fs_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	u32 fs = uvalue->value.integer.value[0];
	PR_D("[bt_speech_fs] fs(%d) time(%d) \n", fs, GET_SYS_TIME);

	if (fs == SAMPLE_RATE_16K || fs == SAMPLE_RATE_8K) {
		txrx_set_fs(fs);
	}

	return NOERR;
}


static int32_t scdrv_bt_fs_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}


static int32_t scdrv_mode_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}


static int32_t scdrv_mode_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	PR_D("[mode_put] mode(%d) time(%d) \n", uvalue->value.integer.value[0], GET_SYS_TIME);
	//snd_chip->m_u4pcmMode = uvalue->value.integer.value[0];  // PCM_MODE_VOIP 3

	return NOERR;
}


static int32_t scdrv_mode_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}


static int32_t scdrv_bt_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}


static int32_t scdrv_bt_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	u32 pcm_bt = uvalue->value.integer.value[0];
	PR_D("[bt_speech_put] %d. time(%d)\n", pcm_bt, GET_SYS_TIME);

	if (pcm_bt) {
		btcall_start();
	} else {
		btcall_stop();
	}

	return NOERR;
}


static int32_t scdrv_bt_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}


static int32_t scdrv_dump_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 65536;

	return NOERR;
}


static int32_t scdrv_dump_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	u32 dump_put = uvalue->value.integer.value[0];
	PR_D("[dump_put] %d.\r\n", dump_put);

	return NOERR;
}


static int32_t scdrv_dump_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *uvalue)
{
	return NOERR;
}


static struct snd_kcontrol_new atc_card_controls[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "card Volum",
		.index = 0,
		.info = scdrv_pb_vol_info,
		.get = scdrv_pb_vol_get,
		.put = scdrv_pb_vol_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "BT_SampleRate",
		.index = 0,
		.info = scdrv_bt_fs_info,
		.get = scdrv_bt_fs_get,
		.put = scdrv_bt_fs_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "pcm mode",
		.index = 0,
		.info = scdrv_mode_info,
		.get = scdrv_mode_get,
		.put = scdrv_mode_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "pcm bt",
		.index = 0,
		.info = scdrv_bt_info,
		.get = scdrv_bt_get,
		.put = scdrv_bt_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "pcm dump",
		.index = 0,
		.info = scdrv_dump_info,
		.get = scdrv_dump_get,
		.put = scdrv_dump_put,
	},
};


//=========================================================//

static int32_t scdrv_pf_suspend(struct platform_device *dev, pm_message_t state)
{
#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
	PR_D("[suspend] Start \n");
	AudDev_PowerDown(AUD_DEVICE_ID_GPSMIX);
	PR_D("[suspend] End \n");
#endif
	return (RET_NOERR);
}


static int32_t scdrv_pf_resume(struct platform_device *dev)
{
#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
	PR_D("[resume] Start \n");
	AudDev_PowerOn(AUD_DEVICE_ID_GPSMIX);
	PR_D("[resume] End \n");
#endif
	return (RET_NOERR);
}


static int32_t scdrv_pf_probe(struct platform_device *dev)
{
	struct snd_card *card = NULL;
	struct snd_pcm *pcm[APCM_DEVICENUM] = {0};
	char pcm_id[16] = { 0 };
	int i, ctl_private = -1;
	int32_t err = -1;

	PR_I("[probe] start >>>>>>>>>> \n");
	err = (int32_t)snd_card_new(&dev->dev, -1, scdrv_id[dev->id], THIS_MODULE, 0, &card);
	if(err < 0) {
		PR_E("[probe] snd_card_new err(%i)! \n", err);
		return (err);
	}

	_scdrv = kzalloc(sizeof(scdrv_t), GFP_KERNEL);
	_scdrv->inited = false;

 	mutex_init(&_scdrv->head_lock);
	_scdrv->irq = VECTOR_AOUT_GPS_RC;

	hw_init();
	btcall_init();
	stream_init();

	_scdrv->card = card;
	card->private_data =_scdrv;
	strcpy(card->driver, 	"ac_83xx");
	strcpy(card->shortname, "ac_83xx (Solo-1)");
	sprintf(card->longname, "%s rev %i, irq %i", card->shortname, _scdrv->revision, _scdrv->irq);

	for(i = 0; i < APCM_DEVICENUM; i++)
	{
		memset(pcm_id, 0, 16);
		sprintf(pcm_id,"%s_%d","card_pcm",i);
	        err = snd_pcm_new(card, pcm_id, i, MAX_PLAYBACK_STREAMS, 1, &pcm[i]);
		if(err < 0) {
			PR_E("[probe] snd_pcm_new err(%i)!\r\n", err);
			return err;
		}

		err = snd_pcm_lib_preallocate_pages_for_all(pcm[i], SNDRV_DMA_TYPE_CONTINUOUS,
			snd_dma_continuous_data(GFP_KERNEL), 64 * 1024, 64 * 1024);
		if(err < 0) {
			PR_E("[probe] snd_pcm_lib_preallocate_pages_for_all err(%i)!\r\n", err);
			return err;
		}

		if( i == 0) {
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_PLAYBACK, &_scdrv_pb_normal_ops);
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_CAPTURE, &_scdrv_rec_normal_ops);
		} else if(i == 1){
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_PLAYBACK, &_scdrv_pb_speech_tx_ops);
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_CAPTURE, &_scdrv_rec_speech_ref_ops);
		} else if(i == 2) {
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_PLAYBACK, &_scdrv_pb_voip_ops);
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_CAPTURE, &_scdrv_rec_speech_mic_ops);
		} else if(i == 3) {
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_PLAYBACK, &_scdrv_pb_speech_rx_ops);
			snd_pcm_set_ops(pcm[i], SNDRV_PCM_STREAM_CAPTURE, &_scdrv_rec_speech_rx_ops);
		}
		pcm[i]->private_data = _scdrv;
		pcm[i]->info_flags = 0;
	}
	_scdrv->pcm = pcm[0];

	PR_I("[probe] sizeof(ATC_card_controls:%d\n",sizeof(atc_card_controls)/sizeof(atc_card_controls[0]));
	for(i = 0;i < sizeof(atc_card_controls)/sizeof(atc_card_controls[0]);i++) {
		snd_ctl_add(card, snd_ctl_new1(&atc_card_controls[i], &ctl_private));
	}

	err = snd_card_register(card);
	if(err < 0) {
		PR_E("[probe] snd_card_register err(%i)! \n", err);
		return (err);
	}

    	MOD_VERSION_INFO(APCM_MOD_NAME, APCM_VER_MAIN, APCM_VER_MINOR, APCM_VER_REV);
	return (RET_NOERR);
}


static int32_t __exit scdrv_pf_remove(struct platform_device *dev)
{
	snd_card_free(platform_get_drvdata(dev));
	platform_set_drvdata(dev, NULL);
	hw_uninit();

	return (RET_NOERR);
}

static struct platform_driver _scdrv_pf_drv = {
	.probe 	= scdrv_pf_probe,
	.remove = __exit_p(scdrv_pf_remove),
	.suspend= scdrv_pf_suspend,
	.resume = scdrv_pf_resume,
	.driver = {
		.name = "card_ALSA",
	},
};


//======================================================================//
#include "pcm_audconf.h"

static s32 scdrv_fops_ioctl(struct file *filp, u32 cmd, u32 arg)
{
	int32_t i4ret = RET_NOERR;
	u32 u4Data = 0;
	WIN32_IOCTL_DATA win_ioctl = {0};

	if(copy_from_user((void *)&win_ioctl, (void *)arg, sizeof(win_ioctl))) {
		PR_E("[ioctl] copy_from_user error err(%i)!\r\n", -EPERM);
		return (-EPERM);
	}

	if (_scdrv->inited)
	{
		switch(cmd)
		{
		case BT_SCO_DISABLE:
			PR_D("[ioctrl] BT_SCO_DISABLE: _speech_ stop! time(%d)\n", GET_SYS_TIME);
			btcall_stop();
			break;
		case SET_DSP_MIX_CH:
			if(copy_from_user((void *)(&u4Data), win_ioctl.pInBuf, sizeof(u4Data))) {
				PR_E("[ioctl] copy_from_user error err(%i)!\r\n", -EPERM);
				return (-EPERM);
			}
			PR_D("[ioctl] SET_DSP_MIX_CH\n");
			i4ret = outhw_set_dsp_mix_ch(u4Data);
			break;

		case SET_PRIMARY_MIC:
			if(copy_from_user((void *)(&u4Data), win_ioctl.pInBuf, sizeof(u4Data))) {
				PR_E("[ioctl] copy_from_user error err(%i)!\r\n", -EPERM);
				return (-EPERM);
			}
			PR_D("[ioctrl] SET_PRIMARY_MIC (%d) \n", u4Data);
			mic_set_primary_idx(u4Data);
			break;

		default:
			break;
		}
	}
	else
	{
		PR_E("[ioctl] alsa pcm driver has not loaded completely\n");
		i4ret = -EPERM;
	}

	return (i4ret);
}


static int32_t scdrv_fops_read(struct file *filp, char __user *buf, u32 count, loff_t *f_pos)
{
	PR_E("We don't support read function! \n");
	return 0;
}


static int32_t scdrv_fops_write(struct file *filp, const char __user *buf, u32 count, loff_t *f_pos)
{
	PR_E("We don't support write function! \n");
	return 0;
}


const struct file_operations _scdrv_file_ops = {
	.read = scdrv_fops_read,
	.write = scdrv_fops_write,
	.unlocked_ioctl = scdrv_fops_ioctl,
};


static struct miscdevice _scdrv_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,	/* We don't care what minor number we end up with,
					   so tell the kernel to just pick one. */
	.name = "pcm_aud",		/* Name ourselves /dev/pcm_aud. */
	.fops = &_scdrv_file_ops,	/* What functions to call when a program performs
					   file operations on the device. */
};


static struct platform_device *_scdrv_dev;

//=============================================================//


int32_t scdrv_init(void)
{
	int err = RET_NOERR;
	PR_I("[init]  >>>>>>>>>>>  0707 11:28\n");

	err = misc_register(&_scdrv_misc_dev);
	if(err) {
		PR_E("[init] misc_register err(%i)! \n", err);
		return (err);
	}

	err = os_driver_register(&_scdrv_pf_drv);
	if(err < 0) {
		PR_E("[init] os_driver_register err(%i)! \n", err);
		os_driver_unregister(&_scdrv_pf_drv);
		return (err);
	}

	_scdrv_dev = os_device_register_simple("card_ALSA", -1, NULL, 0);
	_scdrv->inited = true;
	PR_I("[init] success <<<<<<<<<<<< \n");

	return (RET_NOERR);
}



void __exit scdrv_exit(void)
{
	os_device_unregister(_scdrv_dev);
	os_driver_unregister(&_scdrv_pf_drv);
	misc_deregister(&_scdrv_misc_dev);
	_scdrv->inited = false;
}


int32_t scdrv_hibernation(bool wake_up)
{
	PR_I("[hibernation] wake_up(%d) old hibernate(%d)\n", wake_up, g_hibernate);
	g_hibernate = !wake_up;

	mic_hibernation(wake_up);
	txrx_hibernation(wake_up);
	outhw_hibernation(wake_up);
	asrc_hibernation(wake_up);

	return NOERR;
}


void scdrv_errhandle(void)
{
	PR_E("[errhandle]  \n");
	outhw_errhandle();
}

