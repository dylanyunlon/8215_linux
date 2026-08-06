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



#include <linux/fs.h>
#include <linux/mutex.h>
#include "drv_aud.h"
#include "aud_ioctrl.h"
#include "audio_hal.h"
#include "avin_log.h"


#define LOG_TAG "audio"

static struct file *digital_filp;
static struct file *analog_filp;
static struct mutex linein_lock;


void lineinInit(void)
{
	mutex_init(&linein_lock);
}

bool lineinAudStart(int portNum)
{
	bool ret = false;
	u32 context = 0;
#ifdef ATC_AOSP_ENHANCEMENT_AUDIO
	AUD_MEDIA_TYPE mediaType;
#else
	AUD_OUT_MEDIA_TYPE_T mediaType = AUD_OUT_MEDIA_LINE_IN;
	u8 outBuffer = 0xFF;
#endif
	AUDIN_SET_ONOFF audioInfo;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	mutex_lock(&linein_lock);
	analog_filp = filp_open(AUD_DEV_NAME, O_RDWR | O_NONBLOCK, 0);
	if (IS_ERR(analog_filp)) {
		AVIN_ERROR(LOG_TAG, "open audio device failed!\n");
		analog_filp = NULL;
		goto error;
	}
	context = (u32)analog_filp->private_data;

#ifdef ATC_AOSP_ENHANCEMENT_AUDIO
	mediaType.eMediaSrc = AUD_MEDIA_SOURCE_LINEIN;
	mediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
	mediaType.eMediaCtrl = AUD_MEDIA_ON;
	ret = ADE_IOControl(context, IOCTL_AUDIO_SET_MEDIA_TYPE,
		(u8 *)&mediaType, sizeof(mediaType), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIO_SET_MEDIA_TYPE failed!\n");
		goto error;
	}
#else
	ret = ADE_IOControl(context, IOCTL_AUDIO_GET_FRONT_TYPE,
		NULL, 0, &outBuffer, sizeof(outBuffer), NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "excute IOCTL_AUDIO_GET_FRONT_TYPE command failed\n");
		goto error;
	}
	if(outBuffer) {
		AVIN_ERROR(LOG_TAG, "the media type must be zero,but the current value is %d,so must return\n",
			outBuffer);
		ret = false;
		goto error;
	}

	ret = ADE_IOControl(context, IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE,
		(u8 *)&mediaType, sizeof(mediaType), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE failed!\n");
		goto error;
	}
#endif

	audioInfo.lMode = LIN_DRAM_FRONT;
	audioInfo.fgAudInOnOff = LIN_ON;
	switch (portNum) {
	case 1:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP1;
		break;

	case 2:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP2;
		break;

	case 3:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP3;
		break;

	case 4:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP4;
		break;

	case 5:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP5;
		break;

	default:
		AVIN_ERROR(LOG_TAG, "failed with invalid port num(%d)!\n", portNum);
		ret = false;
		goto error;
	}

	ret = ADE_IOControl(context, IOCTL_AUDIN_SET_ADCIN_CTRL,
		(u8 *)&audioInfo, sizeof(audioInfo), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIN_SET_ADCIN_CTRL failed!\n");
		goto error;
	}
	mutex_unlock(&linein_lock);
	AVIN_DEBUG(LOG_TAG, "play analog audio success!\n");

	return ret;

error:
	if (NULL != analog_filp) {
		filp_close(analog_filp, NULL);
		analog_filp = NULL;
	}
	mutex_unlock(&linein_lock);
	AVIN_ERROR(LOG_TAG, "play analog audio failed!\n");

	return ret;
}

bool lineinAudStop(int portNum)
{
	bool ret = false;
	u32 context = 0;
	AUDIN_SET_ONOFF audioInfo;
#ifdef ATC_AOSP_ENHANCEMENT_AUDIO
	AUD_MEDIA_TYPE mediaType;
#else
	AUD_OUT_MEDIA_TYPE_T eOutMediaType = AUD_OUT_MEDIA_NONE;
#endif

	AVIN_DEBUG(LOG_TAG, "enter\n");
	mutex_lock(&linein_lock);
	if(NULL != analog_filp) {
		context = (u32)analog_filp->private_data;
	} else {
		AVIN_ERROR(LOG_TAG, "analog_filp is null return.\n");
		goto error;
	}
	audioInfo.lMode = LIN_DRAM_FRONT;
	audioInfo.fgAudInOnOff = LIN_OFF;
	switch (portNum) {
	case 1:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP1;
		break;

	case 2:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP2;
		break;

	case 3:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP3;
		break;

	case 4:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP4;
		break;

	case 5:
		audioInfo.eLineINGroupSel = AUD_AADC_LINEIN_INPUT_GROUP5;
		break;

	default:
		AVIN_ERROR(LOG_TAG, "failed with invalid port num(%d)!\n", portNum);
		ret = false;
		goto error;
	}

	ret = ADE_IOControl(context, IOCTL_AUDIN_SET_ADCIN_CTRL,
		(u8 *)&audioInfo, sizeof(audioInfo), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIN_SET_ADCIN_CTRL failed!\n");
		goto error;
	}

#ifdef ATC_AOSP_ENHANCEMENT_AUDIO
	mediaType.eMediaSrc = AUD_MEDIA_SOURCE_LINEIN;
	mediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
	mediaType.eMediaCtrl = AUD_MEDIA_OFF;
	ret = ADE_IOControl(context, IOCTL_AUDIO_SET_MEDIA_TYPE,
		(u8 *)&mediaType, sizeof(mediaType), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIO_SET_MEDIA_TYPE failed!\n");
		goto error;
	}
#else
	ret = ADE_IOControl(context, IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE,
		(u8 *)&eOutMediaType, sizeof(eOutMediaType), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE failed!\n");
		goto error;
	}
#endif

	AVIN_DEBUG(LOG_TAG, "stop analog audio success!\n");

error:
	if (NULL != analog_filp) {
		filp_close(analog_filp, NULL);
		analog_filp = NULL;
	}
	mutex_unlock(&linein_lock);

	return ret;
}

bool start_digitalAud(E_DEST_TYPE_T destination)
{
	bool ret = false;
	u32 context = 0;
#ifdef ATC_AOSP_ENHANCEMENT_AUDIO
	AUD_MEDIA_TYPE mediaType;
#else
	AUD_OUT_MEDIA_TYPE_T mediaType = AUD_OUT_MEDIA_LINE_IN;
#endif
	AUD_IIS_CTRL_INFO audIISCtlInfo;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	digital_filp = filp_open(AUD_DEV_NAME, O_RDWR | O_NONBLOCK, 0);
	if (IS_ERR(digital_filp)) {
		AVIN_ERROR(LOG_TAG, "open audio device failed!\n");
		digital_filp = NULL;
		goto error;
	}
	context = (u32)digital_filp->private_data;

#ifdef ATC_AOSP_ENHANCEMENT_AUDIO
	mediaType.eMediaSrc = AUD_MEDIA_SOURCE_LINEIN;
	mediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
	mediaType.eMediaCtrl = AUD_MEDIA_ON;
	ret = ADE_IOControl(context, IOCTL_AUDIO_SET_MEDIA_TYPE,
		(u8 *)&mediaType, sizeof(mediaType), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIO_SET_MEDIA_TYPE failed!\n");
		goto error;
	}
#else
	ret = ADE_IOControl(context, IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE,
		(u8 *)&mediaType, sizeof(mediaType), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE failed!\n");
		goto error;
	}
#endif

	memset(&audIISCtlInfo, 0, sizeof(AUD_IIS_CTRL_INFO));
	audIISCtlInfo.lMode = LIN_DRAM_FRONT;
	audIISCtlInfo.fgAudInOnOff = LIN_ON;
	audIISCtlInfo.rI2sInfo.eMode = AUD_MASTER_MODE;
	audIISCtlInfo.rI2sInfo.ePinGrp = PINMUX_I2SIN_GROUP1;
	audIISCtlInfo.rI2sInfo.rFmt.eCycle = AUD_LRCK_CYC_32;
	audIISCtlInfo.rI2sInfo.rFmt.eDataFmt = AUDFMT_IIS;
	audIISCtlInfo.rI2sInfo.rFmt.eFs = FS_48K;
	audIISCtlInfo.rI2sInfo.rFmt.eMclkType = AUD_MCLK_256FS;
	audIISCtlInfo.rI2sInfo.rFmt.u4SrcBitNum = 24;

	ret = ADE_IOControl(context, IOCTL_AUDIN_SET_IISIN_CTRL,
		(u8 *)&audIISCtlInfo, sizeof(audIISCtlInfo), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIN_SET_IISIN_CTRL failed!\n");
		goto error;
	}

	AVIN_DEBUG(LOG_TAG, "play digital audio success!\n");

	return ret;

error:
	if (NULL != digital_filp) {
		filp_close(digital_filp, NULL);
		digital_filp = NULL;
	}

	AVIN_ERROR(LOG_TAG, "play digital audio failed!\n");

	return ret;
}

bool stop_digitalAud(E_DEST_TYPE_T destination)
{
	bool ret = false;
	u32 context = 0;
	AUD_IIS_CTRL_INFO audIISCtlInfo;
#ifdef ATC_AOSP_ENHANCEMENT_AUDIO
	AUD_MEDIA_TYPE mediaType;
#else
	AUD_OUT_MEDIA_TYPE_T eOutMediaType = AUD_OUT_MEDIA_NONE;
#endif

	AVIN_DEBUG(LOG_TAG, "enter\n");
	memset(&audIISCtlInfo, 0, sizeof(AUD_IIS_CTRL_INFO));
	audIISCtlInfo.lMode = LIN_DRAM_FRONT;
	audIISCtlInfo.fgAudInOnOff = LIN_OFF;
	context = (u32)digital_filp->private_data;
	ret = ADE_IOControl(context, IOCTL_AUDIN_SET_IISIN_CTRL,
		(u8 *)&audIISCtlInfo, sizeof(audIISCtlInfo), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIN_SET_IISIN_CTRL failed!\n");
		goto error;
	}

#ifdef ATC_AOSP_ENHANCEMENT_AUDIO
	mediaType.eMediaSrc = AUD_MEDIA_SOURCE_LINEIN;
	mediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
	mediaType.eMediaCtrl = AUD_MEDIA_OFF;
	ret = ADE_IOControl(context, IOCTL_AUDIO_SET_MEDIA_TYPE,
		(u8 *)&mediaType, sizeof(mediaType), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIO_SET_MEDIA_TYPE failed!\n");
		goto error;
	}
#else
	ret = ADE_IOControl(context, IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE,
		(u8 *)&eOutMediaType, sizeof(eOutMediaType), NULL, 0, NULL);
	if (!ret) {
		AVIN_ERROR(LOG_TAG, "IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE failed!\n");
		goto error;
	}
#endif

	AVIN_DEBUG(LOG_TAG, "stop digital audio success!\n");

error:
	if (NULL != digital_filp) {
		filp_close(digital_filp, NULL);
		digital_filp = NULL;
	}

	return ret;
}

