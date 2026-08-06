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
#include <asm/uaccess.h>
#include "hdmi_adec.h"
#include "x_aud_dec.h"
#include "windev.h"
#include "memdbg_c.h"
#include "aud_ioctrl.h"
#include "drv_esm_if.h"
#include "drv_aud.h"
#include "audio_hal.h"

BOOL g_ADecIsUsed = FALSE;
BOOL g_fgDumpData = FALSE;
static struct file  *g_Afifofd;

static const char * const MediaTypeStr[] = {
	"AUD_OUT_MEDIA_NONE", /* = 0,*/
	"AUD_OUT_MEDIA_USB", /* = 1,*/
	"AUD_OUT_MEDIA_LINE_IN", /* = 2,*/
	"AUD_OUT_MEDIA_DVD", /* = 3,*/
	"AUD_OUT_MEDIA_LINE_IN2", /* = 4,*/
	"AUD_OUT_MEDIA_UNDEF", /* = 5,*/
};

const char *AFormatStr[] = {
	"AUD_DEC_FMT_UNKNOWN",/*= 0*/
	"AUD_DEC_FMT_MPEG",
	"AUD_DEC_FMT_AC3",
	"AUD_DEC_FMT_PCM",
	"AUD_DEC_FMT_MP3",
	"AUD_DEC_FMT_AAC", /*5*/
	"AUD_DEC_FMT_DTS",
	"AUD_DEC_FMT_WMA",
	"AUD_DEC_FMT_RA",
	"AUD_DEC_FMT_HDCD",
	"AUD_DEC_FMT_MLP",  /*10*/
	"AUD_DEC_FMT_MTS",
	"AUD_DEC_FMT_EU_CANAL_PLUS",
	"AUD_DEC_FMT_TV_SYS",
	"AUD_DEC_FMT_EAC3",
	"AUD_DEC_FMT_EAC3_SEC", /*15*/
	"AUD_DEC_FMT_DTSHD_PRI_XLL",
	"AUD_DEC_FMT_DTSHD_PRI_NO_XLL",
	"AUD_DEC_FMT_DTSHD_SEC",
	"AUD_DEC_FMT_DTSCD",
	"AUD_DEC_FMT_TRUE_HD",/*20*/
	"AUD_DEC_FMT_LOSSLESS_AC3",
	"AUD_DEC_FMT_CDDA",
	"AUD_DEC_FMT_SACD",/*DSD*/
	"AUD_DEC_FMT_VORBIS",
	"AUD_DEC_FMT_DST", /*25*/
	"AUD_DEC_FMT_AAC_PURE",
	"AUD_DEC_FMT_DTS_ES_6_1_MATRIX",
	"AUD_DEC_FMT_DTS_ES_6_1_DISCRETE",
	"AUD_DEC_FMT_DTS_ES_8_DISCRETE",
	"AUD_DEC_FMT_DTS_96_24",  /*30*/
	"AUD_DEC_FMT_DTS_96_24_ES_MATRIX",
	"AUD_DEC_FMT_DVDA",
	"AUD_DEC_FMT_DTSHD_ES_6_1_MATRIX",
	"AUD_DEC_FMT_DTSHD_ES_6_1_DISCRETE",
	"AUD_DEC_FMT_DTSHD_ES_8_DISCRETE", /*35*/
	"AUD_DEC_FMT_DTSHD_96_24",
	"AUD_DEC_FMT_DTSHD_96_24_ES_MATRIX",
	"AUD_DEC_FMT_RA_COOK",
	"AUD_DEC_FMT_AACPLUS",
	"AUD_DEC_FMT_PURE_AACPLUS",  /*40*/
	"AUD_DEC_FMT_HEAAC_V1",
	"AUD_DEC_FMT_HEAAC_V2",
	"AUD_DEC_FMT_HDMI_IN_PCM",
	"AUD_DEC_FMT_DRA",
	"AUD_DEC_FMT_DRA_EXT", /*45*/
	"AUD_DEC_FMT_APE",
	"AUD_DEC_FMT_FLAC",
	"AUD_DEC_FMT_A2DP"
};

int AudioIoCtl(struct file *Audiofilp, DWORD IoControlCOde, VOID *lpInBuf, DWORD InBufSize,
	       VOID *lpOutBuf, DWORD OutBufSize , DWORD *lpBytesReturned)
{
	uintptr_t context = 0;
	int ret = FALSE;

	context = (uintptr_t)Audiofilp->private_data;

	ret = ADE_IOControl(context, IoControlCOde, lpInBuf, InBufSize, lpOutBuf, OutBufSize, lpBytesReturned);

	if (!ret) {
		pr_err("%s send IoControlCOde = %d  fail", __func__, IoControlCOde);
		return 0;
	}

	return 1;
}


void ADec_Release(HANDLE hInst)
{
	AUDIO_DECODER *pADec;

	pr_info("[hdmi_adec]%s enter\r\n", __func__);
	pADec = (AUDIO_DECODER *)hInst;
	if (NULL == pADec) {
		pr_info("[hdmi_adec]%s param pADec is null\r\n", __func__);
		return;
	}

	pADec->eAudCmdStatus = AUD_CMD_STATUS_IDLE;

	if (NULL != pADec->AudioFilp) {
		filp_close((struct file *)pADec->AudioFilp, NULL);
		pADec->AudioFilp = NULL;
	}

	/*kfree(pADec);//need change, need free*/
	pADec = NULL;

	g_ADecIsUsed = FALSE;

	if (g_Afifofd != NULL) {
		filp_close((struct file *)g_Afifofd, NULL);
		g_Afifofd = NULL;
	}

	pr_info("[AVIN][hdmi_adec]%s leave\r\n", __func__);
}

BOOL ADec_SetParam(HANDLE hInst, ParamType eAudioParamType, VOID *prParam, bool flag)
{
	AUD_DEC_AUDIO_PB_INFO_T AudPb_Info;
	static AUD_INFO_T   Aud_Info;
	WAVEFORMATEX *pWave_Info = NULL;
	AUD_DEC_VOLUME_GAIN_INFO_T sAudioVolume;
	AUD_DEC_REAR_VOLUME_GAIN_INFO_T sRearAudioVolume;
	AUDIO_DECODER *pADec = (AUDIO_DECODER *)hInst;

	AUD_MEDIA_TYPE audMediaType;
	mm_memset(&audMediaType, 0, sizeof(AUD_MEDIA_TYPE));

	//mm_memset(&Aud_Info, 0, sizeof(AUD_INFO_T));
	mm_memset(&sAudioVolume, 0, sizeof(AUD_DEC_VOLUME_GAIN_INFO_T));
	mm_memset(&sRearAudioVolume, 0, sizeof(AUD_DEC_REAR_VOLUME_GAIN_INFO_T));

	pr_info("[AVIN][hdmi_adec]%s param is %s\r\n", __func__, Param2Str[eAudioParamType]);

	if (NULL == pADec) {
		pr_info("[AVIN][hdmi_adec]%s AUDIO_DECODER Instance is null!\r\n", __func__);
		return FALSE;
	}
	if (flag == true) {
		AudPb_Info.eSpeed = 1;
		AudPb_Info.prInfo = &Aud_Info;
		AudPb_Info.eAudDecFmt = pADec->u4AudFmt;
		AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_AUDIO_DEC_INFO,
				(void *)&AudPb_Info, sizeof(AUD_DEC_AUDIO_PB_INFO_T), NULL, 0, NULL);
		mm_memset(&sAudioVolume, 0, sizeof(AUD_DEC_VOLUME_GAIN_INFO_T));
		return true;
	}
	if (eAudioParamType == APARAM_PCM_T) {
		if (!pADec->bIsOurDmx) {
			pWave_Info = (WAVEFORMATEX *)prParam;

			if (NULL == pWave_Info) {
				pr_info("[AVIN][hdmi_adec]%s pWave_Info is null!\r\n", __func__);
				return FALSE;
			}

			Aud_Info.e_aud_fmt = AUD_DEC_FMT_PCM;

			if (pWave_Info->nChannels == 1) {
				Aud_Info.e_aud_type = AUD_DEC_TYPE_MONO;
			} else {
				Aud_Info.e_aud_type = AUD_DEC_TYPE_STEREO;
			}

			Aud_Info.ui1_bit_depth = (UINT8)pWave_Info->wBitsPerSample;
			Aud_Info.ui2_pid = 0;
			Aud_Info.ui4_data_rate =
				(pWave_Info->wBitsPerSample * pWave_Info->nSamplesPerSec * pWave_Info->nChannels) / 8;
			Aud_Info.ui4_sample_rate = pWave_Info->nSamplesPerSec;
			Aud_Info.pcm_info.ePCM_Format = AUD_DEC_PCM_FMT_WAVE;
			Aud_Info.pcm_info.b_dlna_exist = FALSE;
			Aud_Info.pcm_info.b_de_emphasis = FALSE;
			Aud_Info.pcm_info.u2BlockAlign = 0;
			pr_info("[AVIN][hdmi_adec]%s [PCM] bit depth is %d!\r\n", __func__, Aud_Info.ui1_bit_depth);
			pr_info("[AVIN][hdmi_adec]%s [PCM] SampleRate is %d!\r\n", __func__, pWave_Info->nSamplesPerSec);
			pr_info("[AVIN][hdmi_adec]%s [PCM] Channels is %d!\r\n", __func__, pWave_Info->nChannels);

			/*AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_AUD_INFO,
				(void *)&Aud_Info, sizeof(AUD_INFO_T), NULL, 0, NULL);*/
		} else {
			AUD_DEC_AUD_INFO_T *prPcmInfo = NULL;

			prPcmInfo = (AUD_DEC_AUD_INFO_T *)prParam;
			pr_info("[AVIN][hdmi_adec]%s The decoder info address is 0x%x!\r\n", __func__, (unsigned int)prPcmInfo);

			if (NULL == prPcmInfo) {
				pr_info("[AVIN][hdmi_adec]%s PCM information is null!\r\n", __func__);
				return FALSE;
			}

			Aud_Info.e_aud_fmt = prPcmInfo->e_aud_fmt;
			Aud_Info.e_aud_type = prPcmInfo->e_aud_type;
			Aud_Info.ui1_bit_depth = prPcmInfo->ui1_bit_depth;
			Aud_Info.ui2_pid = 0;
			Aud_Info.ui4_data_rate = prPcmInfo->ui4_data_rate;
			Aud_Info.ui4_sample_rate = prPcmInfo->ui4_sample_rate;
			Aud_Info.pcm_info.ePCM_Format = prPcmInfo->u_fmt_spec.pt_pcm_info->ePCM_Format;
			Aud_Info.pcm_info.b_dlna_exist = prPcmInfo->u_fmt_spec.pt_pcm_info->b_dlna_exist;
			Aud_Info.pcm_info.b_de_emphasis = prPcmInfo->u_fmt_spec.pt_pcm_info->b_de_emphasis;
			Aud_Info.pcm_info.u2BlockAlign = prPcmInfo->u_fmt_spec.pt_pcm_info->u2BlockAlign;

			/*AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_AUD_INFO,
				(void *)&Aud_Info, sizeof(AUD_INFO_T), NULL, 0, NULL);*/
		}
	}

#ifndef DISABLE_AUDIO_MASTER
	else if (eAudioParamType == APARAM_DISABLE_AVSYNC) {
		if (pADec->fgFreeze) {
			pr_info("[AVIN][hdmi_adec]%s Don't disable avsync because of pause status!\r\n", __func__);
			return FALSE;
		}
		AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_AVSYNC_DISABLE, NULL, 0, NULL, 0, NULL);
	}

#endif
	else if (eAudioParamType == APARAM_SET_FORMAT) {
		pr_info("[AVIN][hdmi_adec]%s set audio format %s\r\n", __func__, AFormatStr[pADec->u4AudFmt]);

		/*if (!AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_FORMAT,
			&pADec->u4AudFmt, sizeof(pADec->u4AudFmt), NULL, 0, NULL)) {
			pr_info("[AVIN][hdmi_adec]%s Set format fail\r\n", __func__);
			return FALSE;
		}*/

		if (AUD_OUTPUT_NONE == pADec->eOutputType) {
			audMediaType.eMediaSrc = AUD_MEDIA_SOURCE_USB;
			audMediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
			audMediaType.eMediaCtrl = AUD_MEDIA_ON;

			if (!AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
				&audMediaType, sizeof(audMediaType), NULL, 0, NULL)) {
				pr_info("[AVIN][hdmi_adec]%s Set front media type fail\r\n", __func__);
				return FALSE;
			}
		}
	} else {
		pr_info("[AVIN][hdmi_adec]%s The parameter type(%d) is unknown!\r\n", __func__, eAudioParamType);
	}

	return TRUE;
}

void ADec_GetParam(HANDLE hInst, ParamType eAudioParamType, VOID *prParam)
{
}

BOOL ADec_SetInputBuf(HANDLE hInst, VOID *pvBuf, UINT32 u4BufSz, VOID *pvOutBuf, UINT32 u4OutBufSz)
{

	AUDIO_DECODER *pADec = (AUDIO_DECODER *)hInst;
	ESM_IO_BUF_INFO *pEsmBufInfo = (ESM_IO_BUF_INFO *) pvBuf;
	mm_segment_t old_fs;
	BOOL fgRet = FALSE;



	if (NULL == pADec) {
		pr_info("[AVIN][hdmi_adec]%s audio instance is null!\r\n", __func__);
		return FALSE;
	}

	
	if (!pADec->bIsOurDmx) {

		AUD_SEND_BUF_INFO rSendBufInfo;

		old_fs = get_fs();
		set_fs(get_ds());/*for kernel file op*/

		mm_memset(&rSendBufInfo, 0X00, sizeof(AUD_SEND_BUF_INFO));
		rSendBufInfo.ptrBufAddr = (uintptr_t)pvBuf;
		rSendBufInfo.u4BufLen  = u4BufSz;
		
		fgRet = AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SEND_BUFFER,
			&rSendBufInfo, sizeof(AUD_SEND_BUF_INFO), NULL, 0, NULL);

		if (fgRet && g_fgDumpData) {
			if (NULL == g_Afifofd) {
				/*g_Afifofd = fopen("/data4write/dumpafifo.txt", "w");*/
				g_Afifofd = filp_open("/data4write/dumpafifo.txt", O_RDWR, 0);

				if (NULL == g_Afifofd) {
					pr_info("[AVIN][hdmi_adec]%s open file fail\r\n", __func__);
				} else {
					pr_info("[AVIN][hdmi_adec]%s open file success\r\n", __func__);
					/*if (-1 == fseek(g_Afifofd, 0, SEEK_END)) {
					    pr_info("[AVIN][hdmi_adec]%s set file pointer fail--1--\r\n", __func__);
					}
					else {
					    pr_info("[AVIN][hdmi_adec]%s set file pointer success--1--,
					    write %d bytes in file\r\n", __func__, u4BufSz);
					    fwrite((char *)pvBuf, sizeof(char), u4BufSz, g_Afifofd);
					}*/
					pr_info("[AVIN][hdmi_adec]%s set file pointer success--1--\r\n", __func__);
					pr_info("[AVIN][hdmi_adec]%s write %u bytes in file", __func__, (unsigned int)u4BufSz);
					g_Afifofd->f_op->write(g_Afifofd, (char *)pvBuf, u4BufSz, &g_Afifofd->f_pos);
				}
			} else {
				/*if (-1 == fseek(g_Afifofd, 0, SEEK_END)) {
				    pr_info("[AVIN][hdmi_adec]%s set file pointer fail--2--\r\n", __func__);
				}
				else {
				    pr_info("[AVIN][hdmi_adec]%s set file pointer success--2--,
				    write %d bytes in file\r\n", __func__, u4BufSz);
				    fwrite((char *)pvBuf, sizeof(char), u4BufSz, g_Afifofd);
				}*/
				pr_info("[AVIN][hdmi_adec]%s set file pointer success--2--\r\n", __func__);
				pr_info("[AVIN][hdmi_adec]%s write %u bytes in file", __func__, (unsigned int)u4BufSz);
				g_Afifofd->f_op->write(g_Afifofd, (char *)pvBuf, u4BufSz, &g_Afifofd->f_pos);
			}
		}

		set_fs(old_fs);
		/*return fgRet;*/
	} else {
		AU_AUDIO g_tAU;

		mm_memset(&g_tAU, 0, sizeof(AU_AUDIO));

		if (AVCODEC_ID_APE == pADec->eAUDCodec) {
			APE_SEEKINFO_INFO_T Ape_Info;
			BOOL fgSetSeekInfo;

			mm_memset(&Ape_Info, 0, sizeof(APE_SEEKINFO_INFO_T));

			fgSetSeekInfo = pEsmBufInfo->rAUEx.rAudEx.rApe.fgSetSeekInfo;

			if (fgSetSeekInfo) {
				Ape_Info.ui4_mute_bank_numbers = pEsmBufInfo->rAUEx.rAudEx.rApe.au4SeekInfo[0];
				Ape_Info.ui4_invalid_bytes     = pEsmBufInfo->rAUEx.rAudEx.rApe.au4SeekInfo[1];

				AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_APE_SEEKINFO,
					(void *)&Ape_Info, sizeof(APE_SEEKINFO_INFO_T), NULL, 0, NULL);
			}
		}

		g_tAU.ptrSAddr = pEsmBufInfo->rAU.rAudioAU.ptrSAddr;
		g_tAU.ptrEAddr = pEsmBufInfo->rAU.rAudioAU.ptrEAddr;

		/*Send AU command*/
		g_tAU.eAuType = AU_DATA;
		g_tAU.fgSkipData = FALSE;

		set_fs(old_fs);
		
		fgRet = AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SEND_ESM_INFO, pvBuf, u4BufSz, NULL, 0, NULL);
	}

	return fgRet;

}

void ADec_SetOutputBuf(HANDLE hInst, void *pvBuf, UINT32 u4BufSz)
{
}
void *ADec_GetOutputBuf(HANDLE hInst, UINT32 *pu4BufSz)
{
	return NULL;
}

BOOL  ADec_Start(HANDLE hInst, HANDLE hEvent)
{

	AUDIO_DECODER *pADec = (AUDIO_DECODER *)hInst;
	AUD_DEC_CTRL_T eAudCtrl = AUD_DEC_CTRL_RESET;

	if (NULL == pADec) {
		pr_info("[AVIN][hdmi_adec]%s audio Instance is NULL!\r\n", __func__);
		return FALSE;
	}

	pr_info("[AVIN][hdmi_adec]%s enter, the current status is %s!\r\n", __func__, StatusStr[pADec->eAudCmdStatus]);

	if ((AUD_CMD_STATUS_IDLE == pADec->eAudCmdStatus) ||
	    (AUD_CMD_STATUS_STOP == pADec->eAudCmdStatus)) {
		if (!AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_CONNECT_ESM, NULL, 0, NULL, 0, NULL)) {
			pr_info("[AVIN][hdmi_adec]%s CONNECT ESM fail\r\n", __func__);
			return FALSE;
		}

		eAudCtrl = AUD_DEC_CTRL_PLAY;

		if (!AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_CTL, &eAudCtrl, sizeof(eAudCtrl), NULL, 0, NULL)) {
			pr_info("[AVIN][hdmi_adec]%s Call Play IOCTL fail!\r\n", __func__);
			return FALSE;
		}

		pr_info("[AVIN][hdmi_adec]%s Play OK!\r\n", __func__);
		g_ADecIsUsed = TRUE;

		pADec->eAudCmdStatus = AUD_CMD_STATUS_PLAY;
	}

	pr_info("[AVIN][hdmi_adec]%s leave\r\n", __func__);

	return TRUE;
}

void ADec_Stop(HANDLE hInst)
{

	AUDIO_DECODER *pADec = (AUDIO_DECODER *)hInst;
	AUD_DEC_CTRL_T eAudCtrl = AUD_DEC_CTRL_RESET;

	if (NULL == pADec) {
		pr_info("[AVIN][hdmi_adec]%s AUDIO_DECODER Instance is NULL!\r\n", __func__);
		return;
	}

	pr_info("[AVIN][hdmi_adec]%s the current status is %s!\r\n", __func__, StatusStr[pADec->eAudCmdStatus]);

	if (AUD_CMD_STATUS_STOP == pADec->eAudCmdStatus) {
		pr_info("[AVIN][hdmi_adec]%s [ADecoder] ADec_Stop: The status is already stop!\r\n", __func__);
		return;
	}

	if (AUD_CMD_STATUS_PLAY == pADec->eAudCmdStatus ||
	    AUD_CMD_STATUS_PAUSE == pADec->eAudCmdStatus) {
		eAudCtrl = AUD_DEC_CTRL_STOP;
		pr_info("[AVIN][hdmi_adec]%s Call Stop IOCTL before\r\n", __func__);
		if (!AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_CTL, &eAudCtrl, sizeof(eAudCtrl), NULL, 0, NULL)) {
			pr_info("[AVIN][hdmi_adec]%s Call Stop IOCTL fail!\r\n", __func__);
			return;
		}

		pr_info("[AVIN][hdmi_adec]%s [ADecoder] ADec_Stop: Stop OK!\r\n", __func__);
		g_ADecIsUsed = FALSE;

		pADec->eAudCmdStatus = AUD_CMD_STATUS_STOP;

		AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_DISCONNECT_ESM, NULL, 0, NULL, 0, NULL);
	}

	pr_info("[AVIN][hdmi_adec]%s success\r\n", __func__);
}

BOOL ADec_Freeze(HANDLE hInst)
{
	return TRUE;
}

BOOL ADec_UNFreeze(HANDLE hInst)
{
	return TRUE;
}

BOOL ADec_Refresh(HANDLE hInst)
{
	return TRUE;
}

void ADec_GetSampleInfo(HAVDECINST hInst, AVSAMPLEINFO_T *prSampleInfo)
{
}

BOOL ADec_SetDiversityInfo(HANDLE hIns, AVCODEC_AUD_DIV_INFO_T  *ptAud_div_info)
{
	return TRUE;
}

BOOL ADec_SetSpeed(HANDLE hIns, UINT32 u4Speed)
{
    AUDIO_DECODER *pADec = (AUDIO_DECODER *)hIns;

    AUD_DEC_PB_SPEED_TYPE_T  t_aud_speed_type;

	pr_info("[AVIN][hdmi_adec]%s enter\r\n", __func__);

    mm_memset(&t_aud_speed_type, 0, sizeof(AUD_DEC_PB_SPEED_TYPE_T));

    if (NULL == pADec)
    {
        pr_info("[AVIN][hdmi_adec]%s param is NULL\r\n", __func__);
        return FALSE;
    }

    switch(u4Speed)
    {
        case 1:
            t_aud_speed_type = AUD_DEC_SPEED_TYPE_NORMAL;
            break;

        case 2:
            t_aud_speed_type = AUD_DEC_SPEED_TYPE_FF_02_00X;
            break;

        case 4:
            t_aud_speed_type = AUD_DEC_SPEED_TYPE_FF_04_00X;
            break;

        case 8:
            t_aud_speed_type = AUD_DEC_SPEED_TYPE_FF_08_00X;
            break;

        case 16:
            t_aud_speed_type =  AUD_DEC_SPEED_TYPE_FF_16_00X;
            break;

        case 32:
            t_aud_speed_type = AUD_DEC_SPEED_TYPE_FF_32_00X;
            break;

        default:
            break;
    }

    AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_PLAY_SPEED, (void *)&t_aud_speed_type, sizeof(t_aud_speed_type), NULL, 0, NULL);
	pr_info("[AVIN][hdmi_adec]%s leave\r\n", __func__);

    return TRUE;

}

void *ADec_GetDevHandle(HANDLE hIns)
{
	return NULL;
}

BOOL ADec_GetAfifoAddr(HANDLE hInst, VOID *prParam)
{
	return TRUE;
}

BOOL ADec_CloseAOut(HANDLE hInst, AUD_OUTPUT_T AOutPut)
{

	AUDIO_DECODER *pADec = (AUDIO_DECODER *)hInst;
	AUD_MEDIA_TYPE audMediaType;
	mm_memset(&audMediaType, 0, sizeof(AUD_MEDIA_TYPE));
	audMediaType.eMediaSrc = AUD_MEDIA_SOURCE_USB;
	audMediaType.eMediaCtrl = AUD_MEDIA_OFF;

	pr_info("[AVIN][hdmi_adec]%s enter\r\n", __func__);

	if (NULL == pADec || NULL == pADec->AudioFilp) {
		pr_info("[AVIN][hdmi_adec]%s The Parameter is NULL, return FALSE!\r\n", __func__);
		return FALSE;
	}

	switch (AOutPut) {
	case AUD_OUTPUT_FRONT:
		if (pADec->eAudCmdStatus != AUD_CMD_STATUS_STOP) {
			pr_info("[hdmi_adec]%s close front type do nothing with status(%s)\r\n", __func__,
				StatusStr[pADec->eAudCmdStatus]);
		} else {
			audMediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
			AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
			&audMediaType, sizeof(audMediaType), NULL, 0, NULL);
		}

		break;

	case AUD_OUTPUT_REAR:
		if (pADec->eAudCmdStatus != AUD_CMD_STATUS_STOP) {
			pr_info("[hdmi_adec]%s close rear type do nothing with status(%s)\r\n", __func__,
				StatusStr[pADec->eAudCmdStatus]);
		} else {
			audMediaType.eMediaOut = AUD_MEDIA_OUT_REAR;
			AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
				&audMediaType, sizeof(audMediaType), NULL, 0, NULL);
		}

		break;

	case AUD_OUTPUT_FRONT_REAR:
		if (pADec->eAudCmdStatus != AUD_CMD_STATUS_STOP) {
			pr_info("[hdmi_adec]%s close rear type do nothing with status(%s)\r\n", __func__,
				StatusStr[pADec->eAudCmdStatus]);
		} else {
			audMediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
			AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
				&audMediaType, sizeof(audMediaType), NULL, 0, NULL);

			audMediaType.eMediaOut = AUD_MEDIA_OUT_REAR;
			AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
				&audMediaType, sizeof(audMediaType), NULL, 0, NULL);
		}

		Sleep(100);
		break;

	default:
		break;
	}

	pr_info("[AVIN][hdmi_adec]%s leave\r\n", __func__);

	return TRUE;
}

BOOL ADec_OpenAOut(HANDLE hInst, AUD_OUTPUT_T AOutPut)
{

	AUDIO_DECODER *pADec = (AUDIO_DECODER *)hInst;
	AUD_MEDIA_TYPE audMediaType;
	mm_memset(&audMediaType, 0, sizeof(AUD_MEDIA_TYPE));
	audMediaType.eMediaSrc = AUD_MEDIA_SOURCE_USB;
	audMediaType.eMediaCtrl = AUD_MEDIA_ON;

	pr_info("[AVIN][hdmi_adec]%s enter\r\n", __func__);

	if (NULL == pADec) {
		pr_info("[AVIN][hdmi_adec]%s pADec param is NULL\r\n", __func__);
		return FALSE;
	}

	switch (AOutPut) {
	case AUD_OUTPUT_FRONT:
		audMediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
		AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
			&audMediaType, sizeof(audMediaType), NULL, 0, NULL);
		pADec->eOutputType = AUD_OUTPUT_FRONT;
		break;

	case AUD_OUTPUT_REAR:
		audMediaType.eMediaOut = AUD_MEDIA_OUT_REAR;
		AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
			&audMediaType, sizeof(audMediaType), NULL, 0, NULL);
		pADec->eOutputType = AUD_OUTPUT_REAR;
		break;

	case AUD_OUTPUT_FRONT_REAR:
		audMediaType.eMediaOut = AUD_MEDIA_OUT_FRONT;
		AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
			&audMediaType, sizeof(audMediaType), NULL, 0, NULL);
	audMediaType.eMediaOut = AUD_MEDIA_OUT_REAR;
		AudioIoCtl(pADec->AudioFilp, IOCTL_AUDIO_SET_MEDIA_TYPE,
			&audMediaType, sizeof(audMediaType), NULL, 0, NULL);
		pADec->eOutputType = AUD_OUTPUT_FRONT_REAR;
		break;

	default:
		break;
	}

	pr_info("[AVIN][hdmi_adec]%s leave\r\n", __func__);
	return TRUE;
}

BOOL  ADec_GetWritePoint(HANDLE hInst, VOID *prParam)
{
	return TRUE;
}

BOOL  ADec_GetAfifoSpareLen(HANDLE hInst, UINT32 *pu4SpareLen)
{
	return TRUE;
}


AV_BASE sAudioBase = {
	0,                            /* eAVCodeC*/
	ADec_Release,
	ADec_SetParam,
	ADec_GetParam,
	ADec_SetInputBuf,
	ADec_SetOutputBuf,
	ADec_GetOutputBuf,
	ADec_Start,
	ADec_Stop,
	ADec_Freeze,
	ADec_UNFreeze,
	ADec_Refresh,
	ADec_GetSampleInfo,
	ADec_SetDiversityInfo,
	ADec_SetSpeed,
	ADec_GetDevHandle,
	ADec_GetAfifoAddr,
	ADec_OpenAOut,
	ADec_CloseAOut,
	ADec_GetWritePoint,
	ADec_GetAfifoSpareLen
};

AUDIO_DECODER *ADec_CreateInstance(AVCODECID_T codec_type, UINT32 u4Flag)
{
	struct filp *audiofilp     = INVALID_HANDLE_VALUE;
	/*DWORD  dwErr     = 0;*/
	AV_BASE *pAVBase = NULL;
	AUDIO_DECODER *pADec;

	pr_info("[AVIN][hdmi_adec]%s enter\r\n", __func__);

	pADec = (AUDIO_DECODER *)MM_ALLOC(sizeof(AUDIO_DECODER));

	if (NULL == pADec) {
		pr_info("[AVIN][hdmi_adec]%s malloc fail\r\n", __func__);
		goto FAIL_EXIT;
	}

	mm_memset(pADec, 0, sizeof(AUDIO_DECODER));

	if (g_ADecIsUsed) {
		pr_info("[AVIN][hdmi_adec]%s Driver has been opened\r\n", __func__);
		goto FAIL_EXIT;
	}

	audiofilp = (struct filp *)filp_open("/dev/adec", O_RDWR, 0);

	if (IS_ERR(audiofilp)) {
		pr_info("[hdmi_adec]%s filp_open fail\r\n", __func__);
		goto FAIL_EXIT;
	}

	g_ADecIsUsed = TRUE;

	pAVBase = (AV_BASE *)(&pADec->sAVBase);

	pADec->AudioFilp = (struct file *)audiofilp;
	*pAVBase = sAudioBase;
	pAVBase->eAVCodeC = codec_type;

	if (u4Flag & INS_FLAG_ISOURDMX) {
		pADec->bIsOurDmx = TRUE;
	}

	switch (codec_type) {
	case AVCODEC_ID_AC3:
		pr_info("[AVIN][hdmi_adec]%s AUD_DEC_FMT_AC3\r\n", __func__);
		pADec->u4AudFmt = AUD_DEC_FMT_AC3;
		break;

	case AVCODEC_ID_DTS:
		pr_info("[AVIN][hdmi_adec]%s AUD_DEC_FMT_DTS\r\n", __func__);
		pADec->u4AudFmt = AUD_DEC_FMT_DTS;
		break;

	case AVCODEC_ID_HDMI_PCM:
		pr_info("[AVIN][hdmi_adec]%s AUD_DEC_FMT_HDMI_IN_PCM\r\n", __func__);
		pADec->u4AudFmt = AUD_DEC_FMT_HDMI_IN_PCM;
		break;

	default:
		break;
	}

	pr_info("[AVIN][hdmi_adec]%s leave success, pADec is 0X%08X\r\n", __func__, (unsigned int)pADec);
	return pADec;

FAIL_EXIT:

	if (pADec != NULL) {
		if (NULL != pADec->AudioFilp) {
			filp_close((struct file *)pADec->AudioFilp, NULL);
			pADec->AudioFilp = NULL;
		}

		MM_FREE(pADec);
		pADec = NULL;
	}

	pr_info("[AVIN][hdmi_adec]%s leave fail\r\n", __func__);

	return NULL;
}
