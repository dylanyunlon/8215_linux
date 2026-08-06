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
                            HeaderFile define
******************************************************************************/
#include <linux/mm.h>
#include "aud_oal.h"
#include "aud_drv.h"
#include "aud_drv_config.h"
#include "aud_debug.h"
//#include "DspMacro.h"
#include "DspFunc.h"
#include "aud_ioctrl.h"
#include "aud_config.h"
#include "aud_power.h"
#include "audin_if.h"
#include "drv_thread.h"
#include "audmhl_if.h"
#include "aud_if.h"
#include "aud_esm.h"
#include "aud_se.h"
#include "AsvDspCtrl.h"
#ifdef __linux__
#include "aud_cmd.h"
#endif
#include <media/atc/mm_common.h>

#if CONFIG_AUD_ADSP_ERR_RECOVER_EN
#include "DspErrProc.h"
#endif

#include "aud_smix.h"
static void* g_hBTContext = NULL;
u64 g_u8Pts = INVALID_TIMESTAMP;
u64 g_u8STC = INVALID_TIMESTAMP;

u8 g_u1DecOutPath[TER_DEC + 1] = {AUD_AOUT1};

#if (CONFIG_AUD_POWER_MANAGEMENT_SUPPORT == 1)
#ifndef __linux__
CEDEVICE_POWER_STATE g_ADECCurrentDx = D0;
extern u8 _u1DspAoutState;
extern u8 _u1DspAout2State;
extern void* _hDspAStatusHandle;
extern void* _hDspBStatusHandle;
#endif
#endif


bool g_fgHasADE = FALSE;
volatile bool g_fgADEInit = FALSE;
bool g_fgAudioGotEos = FALSE;

struct task_struct *g_hAdeInitThread = NULL;
#ifdef __linux__
extern int32_t card_audio_init(void);
#endif

static s32 ADE_Init_Thread(void* pvArg)
{
#ifdef __linux__
    // put it into this thread, decrease the poweron_time
    s32 i4Ret_card_audio_init;
    i4Ret_card_audio_init = card_audio_init();
    if (NOERR == i4Ret_card_audio_init)
    {
        LOG(LOG_CTRLF, "[card_audio_init] success %d\n", i4Ret_card_audio_init);
    }
    else
    {
        LOG(LOG_CTRLF, "[card_audio_init] failed %d\n", i4Ret_card_audio_init);
    }
#endif
    complete_and_exit(NULL, 0); // for fix memory leak

	return 0;
}

u32 ADE_Init(s8 *pszContext)
{
    s32 i4Ret;

    g_fgHasADE = TRUE;

    LOG(LOG_CTRLF, TEXT("[ADE_Init]ADE_Init, Aud_Ver:%d \r\n"),AUD_VER_REV);

    i4Ret = AudioModuleInit();
    if (i4Ret)
    {
        LOG(LOG_OTHER, TEXT("[ADE_Init]Audio Module Init failed %d\n"),i4Ret);
    }

    LOG(LOG_CTRLF, TEXT("[ADE_Init]Audio Module Init success \n"));
    g_fgADEInit = TRUE;

    g_hAdeInitThread = kthread_create(ADE_Init_Thread, (void *)NULL, "AudInitThread");
	if (IS_ERR(g_hAdeInitThread)) {
		LOG(LOG_FAIL, TEXT("[ADE_Deinit]AudInitThread create fail \n"));
		g_hAdeInitThread = NULL;
		return FALSE;
	}
	wake_up_process(g_hAdeInitThread);

    return (TRUE);
}

bool ADE_Deinit(u32 dwContext)
{
    LOG(LOG_OTHER, TEXT("[ADE_Deinit]ADE_Deinit \n"));
    return TRUE;
}



u32 ADE_Open(u32 dwContext, u32 dwAccessMode, u32 dwShareMode)
{
    AUD_DRV_CONTEXT *pContext;
    pContext = (AUD_DRV_CONTEXT *)kzalloc(sizeof(AUD_DRV_CONTEXT), GFP_KERNEL);
    if(NULL != pContext)
    {
        pContext->u1DecId = (u8)MAX_AUDDRV_NUM;
        pContext->fgPlaying = FALSE;
        pContext->fgEnPlay = FALSE;
        pContext->u1Output = AUD_OUT_MAX;
        pContext->ePlayType = AUD_OUT_MEDIA_NONE;
    }
    else
    {
        LOG(LOG_FAIL, TEXT("[ADE_Open]ADE_Open FAIL! \n"));
        return 0;
    }
    LOG(LOG_OTHER, TEXT("[ADE_Open]ADE_Open Handle:0x%x\n"), (u32)pContext);
    return (u32)pContext;
}

bool ADE_Close(u32 dwContext)
{
    AUD_DRV_CONTEXT *pContext = (AUD_DRV_CONTEXT *)dwContext;

    LOG(LOG_OTHER, TEXT("[ADE_Close]ADE_Close Handle:0x%x\n"), (u32)pContext);

    if ((PRI_DEC == pContext->u1DecId) && pContext->fgEnPlay)
    {
        AUD_MEDIA_TYPE rPriType ={AUD_MEDIA_SOURCE_USB, AUD_MEDIA_OUT_FRONT, AUD_MEDIA_OFF};

        if(pContext->fgPlaying)
        {
            AudSetMwCtrl(pContext->u1DecId, AUD_DEC_CTRL_STOP);
            pContext->fgPlaying  = FALSE;
        }

        i4AudEsm_Disconnect(pContext->u1DecId);

        fgAdspGetMediaTypeStatus(&rPriType);
        if(rPriType.eMediaCtrl == AUD_MEDIA_ON)
        {
            rPriType.eMediaCtrl = AUD_MEDIA_OFF;
            LOG(LOG_CTRLF, TEXT("[ADE_Close]clear front mediatype. \n"));
			if(fgAdspSetMediaType(rPriType))
			{
				AudSetDecMediaContext(rPriType, pContext);
			}
        }

        rPriType.eMediaOut = AUD_MEDIA_OUT_REAR;
        rPriType.eMediaCtrl = AUD_MEDIA_OFF;
        fgAdspGetMediaTypeStatus(&rPriType);
        if(rPriType.eMediaCtrl == AUD_MEDIA_ON)
        {
            rPriType.eMediaCtrl = AUD_MEDIA_OFF;
            LOG(LOG_CTRLF, TEXT("[ADE_Close]clear rear mediatype. \n"));
			if(fgAdspSetMediaType(rPriType))
			{
				AudSetDecMediaContext(rPriType, pContext);
			}
        }
    }
    else if((SWMIX_DEC == pContext->u1DecId) && pContext->fgPlaying){
        AUD_MEDIA_TYPE rType ={AUD_MEDIA_SOURCE_SWMIX, AUD_MEDIA_OUT_FRONT, AUD_MEDIA_OFF};
        LOG(LOG_CTRLF, TEXT("[ADE_Close]clear front media type playing. \n"));
        AudSmixSetMwCtrl(&rType);
    }

    kfree(pContext);
    pContext = NULL;

    return TRUE;
}

static void adec_vma_open(struct vm_area_struct *vma)
{
    LOG(LOG_CTRLF, TEXT("adec_vma_open, virt 0x%x, phys 0x%x.\n"),
                    (u32)vma->vm_start, (u32)(vma->vm_pgoff << PAGE_SHIFT));
}

static void adec_vma_close(struct vm_area_struct *vma)
{
    LOG(LOG_CTRLF, TEXT("adec_vma_close, virt 0x%x, phys 0x%x.\n"),
                    (u32)vma->vm_start, (u32)(vma->vm_pgoff << PAGE_SHIFT));
}

static struct vm_operations_struct adec_remap_vm_ops = {
	.open = adec_vma_open,
	.close = adec_vma_close,
};

s32 ADE_Mmap(u32 dwContext, struct vm_area_struct *vma)
{
    u32 length = 0;

	length = vma->vm_end - vma->vm_start;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, length, vma->vm_page_prot))
	{
		return -EAGAIN;
	}

	vma->vm_ops = &adec_remap_vm_ops;
	adec_vma_open(vma);

	return 0;
}

u32 ADE_Read(u32 context, void *pBuffer, u32 dwCount)
{
    AUDIO_BUF_INFO info;
    s32 i4Ret;
    AUD_DRV_CONTEXT *pContext = (AUD_DRV_CONTEXT *)context;
    LOG(LOG_OTHER, TEXT("[ADE_Read]ADE_Read \n"));

    if (dwCount < 8)
        return -1;
    i4Ret = i4AudEsm_GetAudioBuffer(pContext->u1DecId,&info);
    copy_to_user(pBuffer, &info, dwCount);

    return dwCount;
}

u32 ADE_Write(u32 context, void *pBuffer, u32 dwCount)
{
    AU_AUDIO au;
    AUD_DRV_CONTEXT *pContext = (AUD_DRV_CONTEXT *)context;
    LOG(LOG_OTHER, TEXT("[ADE_Write]ADE_Write \n"));

    if (dwCount != sizeof(au))
        return -1;

    copy_from_user(&au, pBuffer, dwCount);

    i4AudEsm_SendAU(pContext->u1DecId,&au);
    return dwCount;
}

void ADE_PowerUp(void)
{
    LOG(LOG_OTHER, TEXT("[ADE_PowerUp]ADE_PowerUp \n"));
}

void ADE_PowerDown(void)
{
    LOG(LOG_OTHER, TEXT("[ADE_PowerDown]ADE_PowerDown \n"));

}

static bool IOControl_Post(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;

    switch(code)
    {
    case IOCTL_AUDIO_SET_SE:
    {
#ifndef __linux__
        void* pTempSeBuffer = NULL;
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_SE*********\n"));
        if(FAILED(CeOpenCallerBuffer(&pTempSeBuffer, pInBuffer, inSize, ARG_IO_PTR,FALSE)))
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_SE Marshalling Memory Error.\n"));
            fgRet = FALSE;
            break;
        }

        if (pTempSeBuffer != NULL)
        {
            fgAudSeProcessOpCmd(pTempSeBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" Buffer error pTempSeBuffer == NULL.\n"));
            fgRet = FALSE;
        }
        CeCloseCallerBuffer(pTempSeBuffer, pInBuffer, inSize, ARG_IO_PTR);
#else
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_SE*********\n"));
        if (pInBuffer != NULL)
        {
            return fgAudSeProcessOpCmd((void *)pInBuffer);
        }
#endif
        break;
    }

    case IOCTL_AUDIO_GET_SPECTRUM:
        if(pOutBuffer != NULL
            && outSize == sizeof(AUD_DEC_SPECTRUM_INFO_T)
            && pOutSize != NULL)
        {
            AUD_DEC_SPECTRUM_INFO_T *ptAudSpectrumInfo = (AUD_DEC_SPECTRUM_INFO_T *)pOutBuffer;

            AudGetSpectrumInfo(ptAudSpectrumInfo);
            *pOutSize = sizeof(AUD_DEC_SPECTRUM_INFO_T);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_SPECTRUM args error.\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_GET_SPECTRUM_SOURCE:
        if(NULL != pOutBuffer && sizeof(AUD_SPECTRUM_BUF_INFO_T) == outSize &&NULL != pOutSize)
        {
            AUD_SPECTRUM_BUF_INFO_T *prInfo = (AUD_SPECTRUM_BUF_INFO_T *)pOutBuffer;
            if(!AudGetSpectrumData(prInfo->u4buf, prInfo->u4size, prInfo->u4scalingMode))
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_SPECTRUM_SOURCE get data fail.\n"));
                fgRet = FALSE;
            }
            *pOutSize = prInfo->u4size;
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_SPECTRUM_SOURCE args error.\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_FEATURE:
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_FEATURE*********\n"));
        if(pInBuffer != NULL)
        {
            vAdspSetFeatureInfo((AUD_DEC_FEATURE_INFO_T )(*(u32 *)pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_FEATURE args error.\n"));
            fgRet = FALSE;

        }
        break;

    case IOCTL_AUDIO_SET_BMANAGEMENT_MODE:
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_BMANAGEMENT_MODE*********\n"));
        if(pInBuffer != NULL)
        {
            vAdspSetModBManagementInfo((AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T)(*(u32 *)pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_BMANAGEMENT_MODE args error.\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_BASS_MANAGEMENT_MODE:
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_BASS_MANAGEMENT_MODE*********\n"));
        if(pInBuffer != NULL)
        {
            vAudCodecSet_Bass_Management_Mode((AUD_DEC_BASS_MANAGEMENT_MODE_T)(*(u32 *)pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_BASS_MANAGEMENT_MODE args error.\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_LRMIX:
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_LRMIX*********\n"));
        if (NULL != pInBuffer)
        {
            vAudLRMixing((*(AUD_DEC_LRMIX_OUTPUT_T*) pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_LRMIX args error.\n"));
            fgRet = FALSE;
        }
        break;

    default:
        break;
    }

    return fgRet;
}


static bool IOControl_VolSpk(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;

    if (FALSE == g_fgADEInit)
    {
         LOG(LOG_FAIL, TEXT(" *** ADE is not initialized, Aud_Ver:%d *****\r\n"),AUD_VER_REV);
         fgRet = FALSE;
         return fgRet;
    }

    switch(code)
    {
    case IOCTL_AUDIO_GET_VOLUME:
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_GET_VOLUME_GAIN*********\n"));
        if (pOutBuffer != NULL)
        {
            u32 *pGetVolume = NULL;
            AUD_DEC_VOLUME_GAIN_INFO_T tAudDecVolGainInfo = {0};
            tAudDecVolGainInfo.e_vol_type = AUD_DEC_ALL_CH;
            pGetVolume = (u32*)pOutBuffer;
            AudGetFrnVolGain(&tAudDecVolGainInfo);
            *pGetVolume = tAudDecVolGainInfo.u.u4FrontMasterVolGain;
            LOG(LOG_FEATURE, TEXT("Master volume = 0x%x\n"), *(u32*)pOutBuffer);
            *pOutSize = sizeof(u32);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** Get master volume gain: fail *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_VOLUME:
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_VOLUME*********\n"));
        if (NULL != pInBuffer)
        {
            AudSetFrnVolGain((AUD_DEC_VOLUME_GAIN_INFO_T *)pInBuffer, 0);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set front volume parameter error *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_SRC_VOLUME:
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_SRC_VOLUME*********\n"));
        if (NULL != pInBuffer)
        {
            AudSetSrcVolGain((AUD_SRC_VOL_CTL *)pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set src volume parameter error *****\n"));
            fgRet = FALSE;
        }
        break;

	case IOCTL_AUDIO_GET_SRC_VOLUME:
	    LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_GET_SRC_VOLUME*********\n"));
	    if (NULL != pInBuffer && NULL != pOutBuffer)
	    {
		    u32 *pGetVolume = (u32*)pOutBuffer;
		    AudGetSrcVolGain((AUD_SRC_VOL_CTL *)pInBuffer, pGetVolume);
		    LOG(LOG_CTRLF, TEXT("AudGetSrcVolGain volume = 0x%x\n"), *(u32*)pOutBuffer);
	    }
	    else
	    {
		    LOG(LOG_FAIL, TEXT(" *** geet src volume parameter error *****\n"));
		    fgRet = FALSE;
	    }
	    break;

    case IOCTL_AUDIO_SET_VOL_POLICY:
        RETAILMSG(1, (TEXT("[AUD]***********IOCTL_AUDIO_SET_VOL_POLICY*********\n")));
        if (NULL != pInBuffer)
        {
            AUD_VOLUME_POLICY_INFO* prInfo = (AUD_VOLUME_POLICY_INFO*)pInBuffer;
            AudSetFrnVolGain(&(prInfo->rVolGainInfo), prInfo->eType);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** IOCTL_AUDIO_SET_VOL_POLICY parameter error *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_GET_REAR_VOLUME:
        {
            u32 *pGetRearVol = NULL;
            AUD_DEC_REAR_VOLUME_GAIN_INFO_T tAudDecVolRearGainInfo = {0};

            LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_GET_REAR_VOLUME_GAIN*********\n"));
            if (pOutBuffer != NULL)
            {
                pGetRearVol = (u32*)pOutBuffer;
                AudGetRearVolGain(&tAudDecVolRearGainInfo);
                *pGetRearVol = tAudDecVolRearGainInfo.u4RearVolGain;
                LOG(LOG_FEATURE, TEXT("Rear volume = 0x%x\n"),*(u32*)pOutBuffer);
                *pOutSize = sizeof(u32);
            }
            else
            {
                LOG(LOG_FAIL, TEXT(" *** get rear voluem Gain: fail *****\n"));
                fgRet = FALSE;
            }
        }
        break;

    case IOCTL_AUDIO_SET_REAR_VOLUME:
        LOG(LOG_FEATURE, TEXT("[AUD]***********IOCTL_AUDIO_SET_REAR_VOLUME*********\n"));
        if (NULL != pInBuffer)
        {
            AudSetRearVolGain((AUD_DEC_REAR_VOLUME_GAIN_INFO_T *)pInBuffer);
            Aud_Linein_VolGainCtrl((AUD_DEC_REAR_VOLUME_GAIN_INFO_T *)pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set rear voluem Gain: fail *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_MUTE_TYPE:
        LOG(LOG_FEATURE, TEXT("***********IOCTL_AUDIO_SET_MUTE_TYPE*********\n"));
        if(NULL != pInBuffer)
        {
            AudSetMute(*(AUD_DEC1_MUTE_CTRL_T*)pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set mute type: fail *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_MUTE_DEC1:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_SET_MUTE_DEC1*********\n"));
        if(NULL != pInBuffer)
        {
            AudSetDec1Mute(*(AUD_DEC1_MUTE_CTRL_T*)pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set mute dec1: fail *****\n"));
            fgRet = FALSE;
        }
        break;
    case IOCTL_AUDIO_SET_SRC_MUTE:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_SRC_MUTE*********\n"));
        if(NULL != pInBuffer)
        {
            AudSetSrcMute((AUD_SRC_MUTE_CTL *)pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set mute dec1: fail *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_SPEAKER_LAYOUT:
        LOG(LOG_FEATURE, TEXT("***********IOCTL_AUDIO_SET_SPEAKER_LAYOUT*********\n"));
        if (NULL != pInBuffer)
        {
            AudSetSpkCfg(0,*(AUD_DEC_SPEAKER_LAYOUT_T*)(pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set speaker layout: fail *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_TEST_TONE_TYPE:
        LOG(LOG_FEATURE, TEXT("***********IOCTL_AUDIO_SET_TEST_TONE_TYPE*********\n"));
        if (NULL != pInBuffer)
        {
            AUD_TESTTONE_SET_TYPE *pTTSetType = (AUD_TESTTONE_SET_TYPE *)pInBuffer;
            vAudTestToneSetType(pTTSetType->eTTType,pTTSetType->eTTOut);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set test tone type: fail *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_TEST_TONE_CHANNEL:
        LOG(LOG_FEATURE, TEXT("***********IOCTL_AUDIO_SET_TEST_TONE_CHANNEL*********\n"));
        if (pInBuffer != NULL)
        {
            AUD_TESTTONE_SET_CHANNEL *pTTSetCfg = (AUD_TESTTONE_SET_CHANNEL *)pInBuffer;
            vAudTestToneSetChannel(pTTSetCfg->eTTLs,pTTSetCfg->eTTOut);
        }
        else
        {
            LOG(LOG_FAIL, TEXT(" *** set test tone channel: fail *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_TEST_TONE_ONOFF:
        LOG(LOG_FEATURE, TEXT(" ***********IOCTL_AUDIO_SET_TEST_TONE_ONOFF*********\n"));
        if (pInBuffer != NULL)
        {
            AUD_TESTTONE_SWITCH_T *pTTSwitch = (AUD_TESTTONE_SWITCH_T *)pInBuffer;
            vAudTestToneSwitch(pTTSwitch->eTTSwitch,pTTSwitch->eTTOut);
        }else
        {
            LOG(LOG_FAIL, TEXT(" *** set test tone onoff: fail *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_GET_OUTPUT_VOL:
        LOG(LOG_FEATURE, TEXT("***********IOCTL_AUDIO_GET_OUTPUT_VOL*********\n"));
        if(pOutBuffer != NULL
            && outSize == sizeof(AUD_OUTPUT_VOL)
            && pOutSize != NULL)
        {
            AUD_OUTPUT_VOL *prChVol = (AUD_OUTPUT_VOL *)pOutBuffer;
            vAudGetOutputVol(prChVol);
            *pOutSize = sizeof(AUD_OUTPUT_VOL);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_OUTPUT_VOL args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_THRESHOLD:
        LOG(LOG_FEATURE, TEXT("***********IOCTL_AUDIO_SET_THRESHOLD*********\n"));
        if (NULL != pInBuffer)
        {
            AudSetDetectVolThr(*(AUD_THRESHOLD_T*)(pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_THRESHOLD args error\n"));
            fgRet = FALSE;
        }
        break;

    default:
        break;
    }

    return fgRet;
}

static bool IOControl_DecCom(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;
    u32 u4ErrType = AUD_ADSP_NORMAL;
    s32 i4Result;

    if (TRUE != pContext->fgEnPlay)
    {
        LOG(LOG_FAIL, TEXT("allocate audio hw source fail in ctrl.\n"));

        u4ErrType = AUD_ADSP_RESOUCE_ERR;
        fgRet = FALSE;
    }
    else
    {
        switch(code)
        {
        case IOCTL_AUDIO_CTL:    // Set Control
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_CTL*********\n"));
            if (NULL != pInBuffer)
            {
                AUD_DEC_CTRL_T eCtrl = (AUD_DEC_CTRL_T)(*pInBuffer);
                LOG(LOG_CTRLF, TEXT("Play Cmd = 0x%x, Dec = 0x%x.\n"),
                    eCtrl, pContext->u1DecId);

                if (AUD_DEC_CTRL_PLAY == eCtrl)
                {
                    pContext->fgPlaying = TRUE;
                    if(PRI_DEC == pContext->u1DecId)
                    {
                        g_u8Pts = INVALID_TIMESTAMP;
                    }
					DspResetInternalPTS(pContext->u1DecId);
                }
                else if (AUD_DEC_CTRL_STOP == eCtrl)
                {
                    pContext->fgPlaying = FALSE;
                }

                i4Result = AudSetMwCtrl(pContext->u1DecId, eCtrl);
                if(AUD_DSPERROR == i4Result)
                {
                    //dsp error happened
                    LOG(LOG_FAIL, TEXT("Dec control command Error happened.\n"));
                    i4AudEsm_Disconnect(pContext->u1DecId);
                    AudReleaseDecResource(pContext);

                    u4ErrType = AUD_ADSP_RESET_FLAG;
                    fgRet = FALSE;
                }
                else if(AUD_FAIL == i4Result)
                {
                    u4ErrType = AUD_ADSP_NORMAL;
                    fgRet = FALSE;
                }
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_CTL args error\n"));
                u4ErrType = AUD_ADSP_NORMAL;
                fgRet = FALSE;
            }
            break;

        case IOCTL_AUDIO_SET_FORMAT:    // Set Format
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_FORMAT*********\n"));
            if (NULL != pInBuffer)
            {
                AUD_DEC_FMT_INFO_T rAudFmtInfo = {0};
                rAudFmtInfo.e_fmt = (AUD_DEC_FMT_T)(*pInBuffer);

                AudCfg_RestoreAoutRegs();
                fgRet = AudSetFormat(pContext->u1DecId, (AUD_DRV_FMT_INFO_T *)(&rAudFmtInfo));

                if (FALSE == fgRet)
                   u4ErrType = AUD_ADSP_NORMAL;
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_FORMAT args error\n"));
                u4ErrType = AUD_ADSP_NORMAL;
                fgRet = FALSE;
            }
            break;

        case IOCTL_AUDIO_SET_ORIG_SAMPRATE:    // Set SampleRate
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_ORIG_SAMPRATE unused.\n"));
            break;

        case IOCTL_AUDIO_SET_PLAY_SPEED:
            LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_SET_PLAY_SPEED*********\n"));
            if(NULL != pInBuffer)
            {
                AudSetDecPlaySpeed(pContext->u1DecId, (*(AUD_DEC_PB_SPEED_TYPE_T*)pInBuffer));
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_PLAY_SPEED InBuffer NULL\n"));
                fgRet = FALSE;
                u4ErrType = AUD_ADSP_NORMAL;
            }
            break;

        case IOCTL_AUDIO_SET_AUD_INFO: // Set Audio Info
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_AUD_INFO*********\n"));
            if(NULL != pInBuffer)
            {
                AUD_INFO_T *prInfo = (AUD_INFO_T *)pInBuffer;
                fgRet = AudSetMwCodecInfo(pContext->u1DecId, prInfo);
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_AUD_INFO InBuffer NULL\n"));
                fgRet = FALSE;
                u4ErrType = AUD_ADSP_NORMAL;
            }
            break;

        case IOCTL_AUDIO_SET_TARGETPTS:
            if(pInBuffer != NULL)
            {
                AUD_SYNC_CONTROL_INFO *pAudSyncInfo = (AUD_SYNC_CONTROL_INFO*)pInBuffer;
                LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_SET_TARGETPTS, u8DecReadyPTS=%x,u8TargetPTS=%x***********\n"),
                    (u32)pAudSyncInfo->u8DecReadyPTS, (u32)pAudSyncInfo->u8TargetPTS);
                vAudDrvIf_SetTargetPTS(pAudSyncInfo->u1DecId, pAudSyncInfo->u8TargetPTS);
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_TARGETPTS params error"));
                fgRet = FALSE;
                u4ErrType = AUD_ADSP_NORMAL;
            }
            break;

        case IOCTL_AUDIO_GET_CURRENTPTS:
            if(pOutBuffer != NULL)
            {
                AUD_SYNC_CONTROL_INFO *pAudSyncInfo = (AUD_SYNC_CONTROL_INFO*)pOutBuffer;
                vAudDrvIf_GetCurrentPTS(pAudSyncInfo->u1DecId,&(pAudSyncInfo->u8DecReadyPTS));
                LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_GET_CURRENTPTS,u8DecReadyPTS=%x,u8TargetPTS=%x***********\n"),
                    (u32)pAudSyncInfo->u8DecReadyPTS, (u32)pAudSyncInfo->u8TargetPTS);
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_CURRENTPTS params error"));
                fgRet = FALSE;
                u4ErrType = AUD_ADSP_NORMAL;
            }
            break;

        case IOCTL_AUDIO_SET_AVSYNC_DISABLE:
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_AVSYNC_DISABLE*********\n"));
            vAudDrvIf_DisableAVSync(PRI_DEC);
            break;

        case IOCTL_AUDIO_GET_LATEST_PTS:
            LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_GET_LATEST_PTS*********\n"));
            if(pOutBuffer != NULL)
            {
                AUD_PTS_CONTEXT* pAudioPTS = (AUD_PTS_CONTEXT*)pOutBuffer;
                vAudDrvIf_GetLatestPTS(pAudioPTS->u1DecId, &(pAudioPTS->u4AudioPTSHi),
                    &(pAudioPTS->u4AudioPTSLo));
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_LATEST_PTS params error"));
                fgRet = FALSE;
                u4ErrType = AUD_ADSP_NORMAL;
            }
            break;

        case IOCTL_AUDIO_GET_PLAYBACK_INFO:
            if(pOutBuffer != NULL
                && outSize == sizeof(PBINF_A)
                && pOutSize != NULL)
            {
                PBINF_A *ptAudPbInfo = (PBINF_A *)pOutBuffer;
                AUD_GetPbInfo(PRI_DEC, ptAudPbInfo);
                *pOutSize = sizeof(PBINF_A);
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_PLAYBACK_INFO args error\n"));
                fgRet = FALSE;
                u4ErrType = AUD_ADSP_NORMAL;
            }
            break;

    	case IOCTL_AUDIO_SET_ASRC_BYPASS:
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_ASRC_BYPASS*********\n"));
            if(pInBuffer != NULL)
            {
                bool* fgVal = (bool*)pInBuffer;
                AudSetDspAsrcBypass(pContext->u1DecId, *fgVal);
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_ASRC_BYPASS args error\n"));
                fgRet = FALSE;
                u4ErrType = AUD_ADSP_NORMAL;
            }
            break;

        case IOCTL_AUDIO_SET_AUDIO_DEC_INFO:
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_AUDIO_DEC_INFO*********\n"));
            if(pInBuffer != NULL)
            {
                AUD_DEC_AUDIO_PB_INFO_T* prPbInfo =(AUD_DEC_AUDIO_PB_INFO_T*)pInBuffer;
                AudCfg_RestoreAoutRegs();
                fgRet = AudSetDecPlayBackInfo(pContext, prPbInfo);
            }
            else
            {
                LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_AUDIO_DEC_INFO args error\n"));
                fgRet = FALSE;
                u4ErrType = AUD_ADSP_NORMAL;
            }
            break;

        default:
            LOG(LOG_FAIL, TEXT("DecCom IOCTL ID error\n"));
            u4ErrType = AUD_ADSP_NORMAL;
            fgRet = FALSE;
            break;
        }
    }

    if ((NULL != pOutBuffer) && (FALSE == fgRet))
    {
        u32* pu4Val = pOutBuffer;
        *pu4Val = u4ErrType;
    }

    return fgRet;
}

static bool IOControl_Codec(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;

    switch(code)
    {
    case IOCTL_AUDIO_SET_AC3DRC:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_SET_AC3DRC*********\n"));
        if (pInBuffer != NULL)
        {
            AudSetDrc(pContext->u1DecId, (AUD_DEC_DRC_T)(*(u32 *)pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_AC3DRC args error\n"));

            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_DTSDRC:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_SET_DTSDRC*********\n"));
        if (pInBuffer != NULL)
        {
            vAudCodecSet_DTS_DRC(pContext->u1DecId, (AUD_DRV_DTS_DRC_MODE_T)(*(u32 *)pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_DTSDRC args error\n"));

            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_DIVERSITY_INFO:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_SET_DIVERSITY_INFO*********\n"));
        if (pInBuffer != NULL)
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_DIVERSITY_INFO args ok \n"));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_DIVERSITY_INFO args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_APE_SEEKINFO:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_SET_APE_SEEKINFO*********\n"));
        if (pInBuffer != NULL )
        {
            APE_SEEKINFO_INFO_T *pApeSeekInfo = (APE_SEEKINFO_INFO_T *)pInBuffer;
            AudSetApeSeekInfo(0, pApeSeekInfo);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_APE_SEEKINFO args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_DEC4_INFO:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_DEC4_INFO*********\n"));
        if(pInBuffer != NULL)
        {
            AUD_DEC4_INFO_T *pAudDec4Info = (AUD_DEC4_INFO_T *)pInBuffer;
            vAudSetDec4Info(pAudDec4Info);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_DEC4_INFO args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_FEATURE_SUPPORT:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_FEATURE_SUPPORT*********\n"));
        if ((NULL != pInBuffer) && (NULL != pOutBuffer)&&(sizeof(bool) == outSize))
        {
            u32 u4feature = *(u32*)pInBuffer;
            *pOutBuffer = (bool)fgAudFeatureSupport(u4feature);
            LOG(LOG_CTRLF, TEXT("audio feature %d Support:%d.\n"),u4feature, *pOutBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("***********IOCTL_AUDIO_FEATURE_SUPPORT args error*********\n"));
        }
        break;

    case IOCTL_AUDIO_GET_CODEC_STATUS:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_GET_CODEC_STATUS*********\n"));
        if ((NULL == pInBuffer) || (NULL == pOutBuffer))
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_CODEC_STATUS args error\n"));
            pOutBuffer = NULL;
            fgRet = FALSE;
        }
        else
        {
            AUD_DEC_ID_T eDecId = (AUD_DEC_ID_T)(*(u32 *)pInBuffer);
            AUD_DRV_CONTEXT *prDecContext = (AUD_DRV_CONTEXT *)pOutBuffer;

            fgRet = fgAudGetDecStatus(eDecId, prDecContext);
        }

        if (!fgRet){
            pOutBuffer = NULL;
        }

        break;

    case IOCTL_AUDIO_CODEC_RESET:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_CODEC_RESET*********\n"));

        if (NULL == pInBuffer)
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_CODEC_RESET args error\n"));
            fgRet = FALSE;
        }
        else
        {
            AUD_DRV_CONTEXT *prDecContext = (AUD_DRV_CONTEXT *)pInBuffer;

            if ((MAX_AUDDRV_NUM != prDecContext->u1DecId) && prDecContext->fgEnPlay)
            {
                if(prDecContext->fgPlaying)
                {
                    AudSetMwCtrl(prDecContext->u1DecId, AUD_DEC_CTRL_STOP);

                    i4AudEsm_Disconnect(prDecContext->u1DecId);
                    AudReleaseDecResource(prDecContext);
                }
            }

            fgRet = true;
        }
        break;

    default:
        LOG(LOG_FAIL, TEXT("Codec IOCTL ID args error\n"));
        fgRet = FALSE;
        break;
    }

    return fgRet;
}


static bool IOControl_ESM(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;
    u32 u4ErrType = AUD_ADSP_NORMAL;

    switch(code)
    {
    case IOCTL_AUDIO_CONNECT_ESM: // ESM Connect
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_CONNECT_ESM*********\n"));
        if(pContext->fgEnPlay == TRUE)
        {
            bool fgEsmRet = i4AudEsm_Connect(pContext->u1DecId);
            fgRet = (fgEsmRet== AUD_OK ? TRUE : FALSE);
            if (FALSE == fgRet)
                u4ErrType = AUD_ADSP_NORMAL;
        }
        else
        {
            LOG(LOG_FAIL, TEXT("Esm connect allocate audio hw source fail.\n"));
            fgRet = FALSE;
            u4ErrType = AUD_ADSP_RESOUCE_ERR;
        }
        break;

    case IOCTL_AUDIO_DISCONNECT_ESM:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_DISCONNECT_ESM*********\n"));
        if(pContext->fgEnPlay == TRUE)
        {
            bool fgEsmRet = i4AudEsm_Disconnect(pContext->u1DecId);
            fgRet = (fgEsmRet== AUD_OK ? TRUE : FALSE);

            if (FALSE == fgRet)
                u4ErrType = AUD_ADSP_NORMAL;
        }
        else
        {
            LOG(LOG_FAIL, _T("Esm disconnet allocate audio hw source fail.\n"));
            fgRet = FALSE;
            u4ErrType = AUD_ADSP_RESOUCE_ERR;
        }
        break;

    case IOCTL_AUDIO_SEND_AU:
        if (TRUE != pContext->fgEnPlay)
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SEND_AU: allocate audio hw source fail.! ***\n"));
            fgRet =  FALSE;
            u4ErrType = AUD_ADSP_RESOUCE_ERR;
        }
        else if(!pInBuffer || inSize != sizeof(AU_AUDIO))
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SEND_AU: argument error! ***\n"));
            fgRet =  FALSE;
            u4ErrType = AUD_ADSP_NORMAL;
        }
        else
        {
            #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
            if (u4AdspErrProcNotifyFlagGet(pContext->u1DecId))
            {
                u4AdspErrProcNotifyFlagSet(pContext->u1DecId, 0);
                fgRet =  FALSE;
                u4ErrType = AUD_ADSP_RESET_FLAG;
            }
            else
            #endif
            {
                i4AudEsm_SendAU(pContext->u1DecId, (AU_AUDIO *)pInBuffer);
            }
        }
        break;

    case IOCTL_AUDIO_SEND_BUFFER:
        if (TRUE != pContext->fgEnPlay)
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SEND_BUFFER: allocate audio hw source fail.! ***\n"));
            fgRet =  FALSE;
            u4ErrType = AUD_ADSP_RESOUCE_ERR;
        }
        else if(!pInBuffer || inSize != sizeof(AUD_SEND_BUF_INFO))
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SEND_BUFFER: argument error! ***\n"));
            fgRet =  FALSE;
            u4ErrType = AUD_ADSP_NORMAL;
        }
        else
        {
            #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
            if (u4AdspErrProcNotifyFlagGet(pContext->u1DecId))
            {
                LOG(LOG_FAIL, TEXT("send buf Dsp ErrRecover happened.\n"));

                u4AdspErrProcNotifyFlagSet(pContext->u1DecId, 0);
                i4AudEsm_Disconnect(pContext->u1DecId);
                AudReleaseDecResource(pContext);

                fgRet =  FALSE;
                u4ErrType = AUD_ADSP_RESET_FLAG;
            }
            else
            #endif
            {
                AUD_SEND_BUF_INFO *prBufInfo = (AUD_SEND_BUF_INFO *)pInBuffer;
                fgRet = AudEsm_SendBufferInfo(pContext->u1DecId, prBufInfo, TRUE);
                if (FALSE == fgRet)
                {
                    if(0 == prBufInfo->ptrBufAddr || 0 == prBufInfo->u4BufLen)
                    {
						LOG(LOG_FAIL, TEXT("AudEsm_SendBufferInfo para error, BufAddr 0x%x, BufLen 0x%x\n"), prBufInfo->ptrBufAddr, prBufInfo->u4BufLen);
                        u4ErrType = AUD_ADSP_PARA_ERR;
                    }
                    else
                    {
                        u4ErrType = AUD_ADSP_BUF_FULL;
                    }
                }
            }
        }
        break;

    case IOCTL_AUDIO_SEND_BUFFER_KERNEL:
        if (TRUE != pContext->fgEnPlay)
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SEND_BUFFER_KERNEL: allocate audio hw source fail.! ***\n"));
            fgRet =  FALSE;
            u4ErrType = AUD_ADSP_RESOUCE_ERR;
        }
        else if(!pInBuffer || inSize != sizeof(AUD_SEND_BUF_INFO))
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SEND_BUFFER_KERNEL: argument error! ***\n"));
            fgRet =  FALSE;
            u4ErrType = AUD_ADSP_NORMAL;
        }
        else
        {
            #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
            if (u4AdspErrProcNotifyFlagGet(pContext->u1DecId))
            {
                LOG(LOG_FAIL, TEXT("send buf Dsp ErrRecover happened.\n"));

                u4AdspErrProcNotifyFlagSet(pContext->u1DecId, 0);
                i4AudEsm_Disconnect(pContext->u1DecId);
                AudReleaseDecResource(pContext);

                fgRet =  FALSE;
                u4ErrType = AUD_ADSP_RESET_FLAG;
            }
            else
            #endif
            {
                AUD_SEND_BUF_INFO *prBufInfo = (AUD_SEND_BUF_INFO *)pInBuffer;
                fgRet = AudEsm_SendBufferInfo(pContext->u1DecId, prBufInfo, FALSE);
                if (FALSE == fgRet)
                {
                    if(0 == prBufInfo->ptrBufAddr || 0 == prBufInfo->u4BufLen)
                    {
						LOG(LOG_FAIL, TEXT("AudEsm_SendBufferInfo para error, BufAddr 0x%x, BufLen 0x%x\n"), prBufInfo->ptrBufAddr, prBufInfo->u4BufLen);
                        u4ErrType = AUD_ADSP_PARA_ERR;
                    }
                    else
                    {
                        u4ErrType = AUD_ADSP_BUF_FULL;
                    }
                }
            }
        }
        break;


    case IOCTL_AUDIO_SEND_ESM_INFO:
        if (TRUE != pContext->fgEnPlay)
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SEND_ESM_INFO: allocate audio hw source fail.! ***\n"));
            fgRet =  FALSE;
            u4ErrType = AUD_ADSP_RESOUCE_ERR;
        }
        else if (!pInBuffer || inSize != sizeof(ESM_IO_BUF_INFO))
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SEND_ESM_INFO: argument error! ***\n"));
            fgRet =  FALSE;
            u4ErrType = AUD_ADSP_NORMAL;
        }
        else
        {
            #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
            if (u4AdspErrProcNotifyFlagGet(pContext->u1DecId))
            {
                LOG(LOG_FAIL, TEXT("send esm info Dsp ErrRecover happened.\n"));
                i4AudEsm_Disconnect(pContext->u1DecId);
                u4AdspErrProcNotifyFlagSet(pContext->u1DecId, 0);
                AudReleaseDecResource(pContext);

                fgRet =  FALSE;
                u4ErrType = AUD_ADSP_RESET_FLAG;
            }
            else
            #endif
            {
                fgRet = AudEsm_SendEsmInfo(pContext->u1DecId, (ESM_IO_BUF_INFO *)pInBuffer);
                if (FALSE == fgRet)
                    u4ErrType = AUD_ADSP_BUF_FULL;
            }
        }
        break;

    case IOCTL_AUDIO_SEND_END_OF_STREAM:
    {
        //EV_GRP_EVENT_T Vdec_Ev;
        AU_AUDIO au_info = {0};
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SEND_END_OF_STREAM*********\n"));
        if(pContext->fgEnPlay == TRUE)
        {
            if(pContext->u1DecId == PRI_DEC)
            {
                g_fgAudioGotEos = TRUE;
                vAudAddLastFrame();
            }

            au_info.eAuType = AU_CMD;
            i4AudEsm_SendAU(pContext->u1DecId, &au_info);
        }
        else
        {
            LOG(LOG_FAIL, _T("IOCTL_AUDIO_SEND_END_OF_STREAM allocate audio hw source fail.\n"));
            fgRet = FALSE;
            u4ErrType = AUD_ADSP_RESOUCE_ERR;
        }
        break;
    }

    //just for 2nd test app
    case IOCTL_AUDIO_GET_AFIFO_INFO_VIRTUAL:
        if ((pOutBuffer != NULL) && (outSize == sizeof(AUDIO_BUF_INFO)) && (pOutSize != NULL))
        {
            AUDIO_BUF_INFO *pInfo = (AUDIO_BUF_INFO *)pOutBuffer;
            LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_GET_AFIFO_INFO_VIRTUAL*********\n"));
            fgRet = AudEsm_GetAfifoInfoVirtual(pContext->u1DecId, pInfo);
            *pOutSize = sizeof(AUDIO_BUF_INFO);

            if (FALSE == fgRet)
                u4ErrType = AUD_ADSP_NORMAL;
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_GET_AFIFO_INFO: args error ***\n"));
            fgRet = FALSE;
            u4ErrType = AUD_ADSP_NORMAL;
        }
        break;

    //just for 2nd test app
    case IOCTL_AUDIO_GET_AFIFO_WRITE_POINTER:
        if ((pOutBuffer != NULL) && (outSize == sizeof(AUDIO_BUF_INFO)) && (pOutSize != NULL))
        {
            AUDIO_BUF_INFO *pInfo = (AUDIO_BUF_INFO *)pOutBuffer;
            //LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_GET_AFIFO_WRITE_POINTER*********\n"));
            i4AudEsm_GetAudioBuffer(pContext->u1DecId, pInfo);

            pInfo->u4WritePointer -= pInfo->ptrFifoSA;
            *pOutSize = sizeof(AUDIO_BUF_INFO);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_GET_AFIFO_WRITE_POINTER: args error ***\n"));
            fgRet = FALSE;
            u4ErrType = AUD_ADSP_NORMAL;
        }
        break;

    case IOCTL_AUDIO_GET_READ_DATA_SUM:
        if ((pOutBuffer != NULL) && (outSize == sizeof(u32)) && (pOutSize != NULL))
        {
            u32 *pu4ReadCnt = (u32 *)pOutBuffer;
            LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_GET_READ_DATA_SUM*********\n"));
            *pu4ReadCnt = u4AudEsm_GetReadCount(pContext->u1DecId);
            *pOutSize = sizeof(u32);
        }
        else
        {
            LOG(LOG_FAIL,TEXT("*** IOCTL_AUDIO_GET_READ_COUNT: args error ***\n"));
            fgRet = FALSE;
            u4ErrType = AUD_ADSP_NORMAL;
        }
        break;

    case IOCTL_AUDIO_GET_CODEC_FIFO_INFO:
        //LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_GET_CODEC_FIFO_INFO*********\n"));

        if ((NULL != pInBuffer) && (pOutBuffer != NULL) && (outSize == sizeof(AUD_POSINFO_T)) && (pOutSize != NULL))
        {
            AUD_FIFO_TYPE_T eFifoType = *(AUD_FIFO_TYPE_T*)pInBuffer;
            AUD_POSINFO_T *pFifoInfo = (AUD_POSINFO_T *)pOutBuffer;

            if(0 == i4AudEsm_GetAudioCodecFifoInfo(eFifoType, pFifoInfo))
            {
                *pOutSize = sizeof(AUD_POSINFO_T);
                //LOG(LOG_DECINFO,TEXT("*** fifo info : type(0x%x), PhySadr (0x%x),  PhyEadr (0x%x) ***\n"),eFifoType, pFifoInfo->u4AfifoSA, pFifoInfo->u4AfifoEA);
                //LOG(LOG_DECINFO,TEXT("*** fifo info : VirSadr (0x%x),  VirEadr (0x%x) ***\n"),pFifoInfo->u4AfifoVirSA, pFifoInfo->u4AfifoVirEA);
            }
            else
            {
                LOG(LOG_CTRLF,TEXT("*** i4AudEsm_GetAudioCodecFifoInfo  error return ***\n"));
                fgRet = FALSE;
                break;
            }
        }
        else
        {
            LOG(LOG_CTRLF,TEXT("*** IOCTL_AUDIO_GET_CODEC_FIFO_INFO : args media type error ***\n"));
            fgRet = FALSE;
        }
        break;

    default:
        LOG(LOG_FAIL, TEXT("*** ESM IOCTL ID: args error ***\n"));
        fgRet = FALSE;
        u4ErrType = AUD_ADSP_NORMAL;
        break;
    }


    if ((NULL != pOutBuffer) && (FALSE == fgRet))
    {
        u32* pu4Val = pOutBuffer;
        *pu4Val = u4ErrType;
    }

    return fgRet;
}


static bool IOControl_AOut(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;
    switch(code)
    {
    case IOCTL_AUDIO_AOUT_CONFIG:
        LOG(LOG_DAC, TEXT("***********IOCTL_AUDIO_AOUT_CONFIG*********\n"));
        if(NULL != pInBuffer)
        {
            AUD_OUTPUT_PATH_T *prParam = (AUD_OUTPUT_PATH_T*)pInBuffer;
            AudAout_PathSet(prParam);

            if((PRI_DEC == pContext->u1DecId)||
               (SEC_DEC == pContext->u1DecId)||
               (TER_DEC == pContext->u1DecId))
            {
                g_u1DecOutPath[pContext->u1DecId] = prParam->eSrc;
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_AOUT_CONFIG: argument error! ***\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SWITCH_AOUT:
        LOG(LOG_DAC, TEXT("***********IOCTL_AUDIO_SWITCH_AOUT*********\n"));
        if(NULL != pInBuffer)
        {
            u32 *pdwParam = (u32 *)pInBuffer;
            vAudDrvIf_SwitchAout(*pdwParam);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SWITCH_AOUT: argument error! ***\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_DAC_TYPE:
        if (pInBuffer)
        {
            AUD_DAC_TYPE_SEL_T *prDACSel = (AUD_DAC_TYPE_SEL_T *)pInBuffer;
            LOG(LOG_DAC, TEXT("***********IOCTL_AUDIO_SET_DAC_TYPE*********\n"));
            AudAout_DacTypeSet(prDACSel);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SWITCH_AOUT: argument error! ***\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_TYPE_SPDIF:
        if (NULL != pInBuffer)
        {
            LOG(LOG_IO, TEXT("***********IOCTL_AUDIO_SET_TYPE_SPDIF*********\n"));
            AudSetSpdif((AUD_DEC_SPDIF_TYPE_T)*pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SET_TYPE_SPDIF: argument error! ***\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_INFO_FROM_DVP:
        if (NULL != pInBuffer)
        {
            u8 *pbData = (u8*)pInBuffer;
            LOG(LOG_IO, TEXT("***********IOCTL_AUDIO_INFO_FROM_DVP*********\n"));
            AudCfg_RestoreAoutRegs();
            AudSetDvdMixCfg(*pbData);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_INFO_FROM_DVP: argument error! ***\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_REAR_I2S_GROUP:
        {
            AUD_REAR_GROUP_TYPE_E *eType = (AUD_REAR_GROUP_TYPE_E *)pInBuffer;
			if (NULL != pInBuffer)
			{
                LOG(LOG_IO, TEXT("***********IOCTL_AUDIO_SET_REAR_I2S_GROUP*********\n"));
                Aud_RearMultiSel(*eType);
            }
			else
			{
				LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SET_REAR_I2S_GROUP: argument error! ***\n"));
				fgRet = FALSE;
			}
        }
        break;

    case IOCTL_AUDIO_SET_MIRACAST_ONOFF:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_MIRACAST_ONOFF*********\n"));
        if(pInBuffer != NULL)
        {
            vAudDrvIf_SetMiracastOnOff((AUD_MIRACAST_CTRL_T)*pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_MIRACAST_ONOFF args error\n"));

            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_MIRACAST_PARAM:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_MIRACAST_PARAM*********\n"));
        if(pInBuffer != NULL)
        {
            vMira_SetAdjustParam(*(AUD_MIRACAST_PARAM_T*)pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_MIRACAST_PARAM args error\n"));

            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_REAR_OUT_MODE:
        RETAILMSG(1, (TEXT("[AUD]***********IOCTL_AUDIO_SET_REAR_OUT_MODE*********\r\n")));
        if (NULL != pInBuffer)
        {
            u32* pu4Data = (u32*)pInBuffer;
            AudAout_SetFRAoutMode(*pu4Data);
        }
        else
        {
            LOG(LOG_CTRLF, TEXT("*** IOCTL_AUDIO_SET_REAR_OUT_MODE: argument error! ***\r\n"));
            fgRet = FALSE;
        }
        break;

    default:
        LOG(LOG_FAIL, TEXT("*** Aout IOCTL ID(0x%x): argument error! ***\n"), code);
	    fgRet = FALSE;
        break;

    }

    return fgRet;
}


static bool IOControl_Dual(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;

    switch(code)
    {
    case IOCTL_AUDIO_GET_FRONT_STATUS:
        LOG(LOG_IO, TEXT("**********IOCTL_AUDIO_GET_FRONT_STATUS*********\n"));
        if ((pOutBuffer != NULL) && (outSize == sizeof(bool)))
        {
            bool *bIsReadyForSwitch = (bool *)pOutBuffer;
            *bIsReadyForSwitch = vAdspGetFrontAoutStatus();
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_FRONT_STATUS args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_GET_REAR_STATUS:
        LOG(LOG_IO, TEXT("***********IOCTL_AUDIO_GET_REAR_STATUS*********\n"));
        if ((pOutBuffer != NULL)  && (outSize == sizeof(bool)))
        {
            bool *bIsReadyForSwitch = (bool *)pOutBuffer;

            *bIsReadyForSwitch = vAdspGetRearAoutStatus();
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_REAR_STATUS args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_GET_FRONT_TYPE:
        LOG(LOG_IO, TEXT("***********IOCTL_AUDIO_GET_FRONT_TYPE*********\n"));
        if (pOutBuffer != NULL)
        {
            u8* uType = (u8 *)pOutBuffer;
            *uType =  u1AdspGetFrontAoutType();
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_FRONT_TYPE args error\n"));
        }
        break;

    case IOCTL_AUDIO_GET_REAR_TYPE:
        LOG(LOG_IO, TEXT("***********IOCTL_AUDIO_GET_REAR_TYPE*********\n"));
        if (pOutBuffer != NULL)
        {
            u8* uType = (u8 *)pOutBuffer;
            *uType =  u1AdspGetRearAoutType();
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_REAR_TYPE args error\n"));
        }
        break;

    case IOCTL_AUDIO_GET_DEC_CONTEXT:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_GET_DEC_CONTEXT*********\n"));
        if ((pOutBuffer != NULL) && (outSize == sizeof(AUD_DRV_CONTEXT)) && (pOutSize != NULL))
        {
            x_memcpy(pOutBuffer, (void *)pContext, sizeof(AUD_DRV_CONTEXT));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_DEC_CONTEXT args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_DEC_CONTEXT:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIO_SET_DEC_CONTEXT*********\n"));
        if (NULL != pInBuffer)
        {
            AUD_DRV_CONTEXT *pAudContext = (AUD_DRV_CONTEXT *)pInBuffer;
            pContext->u1DecId = pAudContext->u1DecId;
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_DEC_CONTEXT args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE*********\n"));
        if (NULL != pInBuffer)
        {
            if(fgAdspSetFrontAoutMediaType(*(AUD_OUT_MEDIA_TYPE_T*)pInBuffer))
            {
                AudSetDecContext(*(AUD_OUT_MEDIA_TYPE_T*)pInBuffer, pContext, AUD_FRONT);
            }
            else
            {
                LOG(LOG_CTRLF, TEXT("Set front media type error\n"));
                fgRet = FALSE;
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE args error\n"));
            fgRet = FALSE;
        }

        break;

    case IOCTL_AUDIO_SET_REAR_MEDIA_TYPE:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_REAR_MEDIA_TYPE*********\n"));
        if (NULL != pInBuffer)
        {
            if (fgAdspSetRearAoutMediaType(*(AUD_OUT_MEDIA_TYPE_T*)pInBuffer))
            {
                AudSetDecContext(*(AUD_OUT_MEDIA_TYPE_T*)pInBuffer, pContext, AUD_REAR);
            }
            else
            {
                fgRet = FALSE;
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_REAR_MEDIA_TYPE args error\n"));
            fgRet = FALSE;
        }
        break;
	case IOCTL_AUDIO_GET_MEDIA_TYPE_STATUS:
        if (NULL != pOutBuffer)
        {
            fgAdspGetMediaTypeStatus((AUD_MEDIA_TYPE*)pOutBuffer);

         }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_GET_MEDIA_TYPE args error\n"));
            fgRet = FALSE;
        }

        break;
    case IOCTL_AUDIO_SET_MEDIA_TYPE:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_SET_MEDIA_TYPE*********\n"));
        if (NULL != pInBuffer)
        {
            if(fgAdspSetMediaType(*(AUD_MEDIA_TYPE*)pInBuffer))
            {
                AudSetDecMediaContext(*(AUD_MEDIA_TYPE*)pInBuffer, pContext);
            }
            else
            {
                LOG(LOG_FAIL, TEXT("Set audio media type error\n"));
                fgRet = FALSE;
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_MEDIA_TYPE args error\n"));
            fgRet = FALSE;
        }

        break;

    case IOCTL_AUDIO_SET_ASRC_SWITCH:
        LOG(LOG_IO, TEXT("***********IOCTL_AUDIO_SET_ASRC_SWITCH*********\n"));
        if(NULL != pInBuffer)
        {
            vAdspEnableASRC(pContext->u1DecId,*((bool *)pInBuffer));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_SET_ASRC_SWITCH args error\n"));
            fgRet = FALSE;
        }
        break;

    #if CONFIG_AUD_DECONLY_EN
    case IOCTL_AUDIO_DECONLY_ONOFF:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_DECONLY_ONOFF*********\n"));
        if (NULL != pInBuffer)
        {
            if(fgAudDeconlySetOnOff(*(AUD_DECONLY_CTRL_T*)pInBuffer))
            {
                AudSetDecContext(AUD_OUT_MEDIA_USB, pContext, AUD_OUT_MAX); // only care: DecId, fgEnPlay, fgPlaying
            }
            else
            {
                LOG(LOG_CTRLF, TEXT("Set deconly onoff error\n"));
                fgRet = FALSE;
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIO_DECONLY_ONOFF args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_DECONLY_GET_BUF:
        //LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIO_DECONLY_GET_BUF*********\n"));
        if ((pOutBuffer != NULL) && (outSize == sizeof(AUD_DECONLY_GET_BUF)) && (pOutSize != NULL))
        {
            AUD_DECONLY_GET_BUF *pInfo = (AUD_DECONLY_GET_BUF *)pOutBuffer;
            *pOutSize = sizeof(AUD_DECONLY_GET_BUF);
            if(!fgAudDeconlyGetBuff(pInfo))
            {
                LOG(LOG_CTRLF, TEXT("deconly get data fail\n"));
                fgRet = FALSE;
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_DECONLY_GET_BUF: args error ***\n"));
            fgRet = FALSE;
        }
        break;
    #endif

    default:
        LOG(LOG_FAIL, TEXT("Dual IOCTL ID args error\n"));
        fgRet = FALSE;
        break;
    }

    return fgRet;
}


static bool IOControl_AudIn(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;

    switch(code)
    {
    case IOCTL_AUDIN_SET_ONOFF:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIN_SET_ONOFF*********\n"));
        if (NULL != pInBuffer)
        {
            AUDIN_SET_ONOFF *prLinInCmd = (AUDIN_SET_ONOFF *)pInBuffer;
            Aud_Linein_SetCtrl(prLinInCmd, pContext->u1DecId);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIN_SET_ONOFF args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIN_SET_ADDR:
        LOG(LOG_IO, TEXT("***********IOCTL_AUDIN_SET_ADDR*********\n"));
        if (NULL != pInBuffer)
        {
            //AUDIN_SET_ADDR *pAudInAddr = (AUDIN_SET_ADDR *)pInBuffer;
        }

        break;

    case IOCTL_AUDIN_GET_DEC_DATALEN:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIN_GET_DEC_DATALEN*********\n"));
        if ((pOutBuffer != NULL) && (outSize == sizeof(u32)) && (pOutSize != NULL))
        {

            AUDIN_DEC_DATA_LEN *pu4DecDataLen = (AUDIN_DEC_DATA_LEN *)pOutBuffer;
            Aud_Linein_GetDataLen(pu4DecDataLen);
            *pOutSize = sizeof(AUDIN_DEC_DATA_LEN);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIN_GET_DEC_DATALEN args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIN_GET_DEC_CFG_INFO:
        LOG(LOG_DECINFO, TEXT("***********IOCTL_AUDIN_GET_DEC_CFG_INFO*********\n"));
        if ((pOutBuffer != NULL) && (outSize == sizeof(AUDIN_INFO)) && (pOutSize != NULL))
        {
            AUDIN_INFO *pAudioInInfo = (AUDIN_INFO *)pOutBuffer;

            Aud_Linein_GetDecInfo(pAudioInInfo);
            *pOutSize = sizeof(AUDIN_INFO);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIN_GET_DEC_CFG_INFO args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIN_REAR_VOL_GAIN_INFO:
        if (NULL != pInBuffer)
        {
            LOG(LOG_FEATURE, TEXT("Now Not use  IOCTL_AUDIN_REAR_VOL_GAIN_INFO\n"));
        }
        break;

    case IOCTL_AUDIN_FRONT_VOL_GAIN_INFO:
        if (NULL != pInBuffer)
        {
            LOG(LOG_FEATURE, TEXT("Now Not use  IOCTL_AUDIN_FRONT_VOL_GAIN_INFO \n"));
        }
        break;

    case IOCTL_AUDIN_IIS_CTRL:
        if (NULL != pInBuffer)
        {
            AUD_IIS_CTRL_INFO *prI2sInfo = (AUD_IIS_CTRL_INFO *)pInBuffer;
            LOG(LOG_IO, TEXT("***********IOCTL_AUDIN_IIS_CTRL*********\n"));
            fgRet = Aud_Linein_SetIisin(prI2sInfo, pContext->u1DecId);
        }
        break;

    case IOCTL_AUDIN_INPUT_TYPE:
        if(NULL != pInBuffer)
        {
            u32 u4InPut = *((u32*)pInBuffer);
            Aud_Linein_SetInputType((u8)u4InPut);
        }
        break;

    case IOCTL_AUDIN_SET_ADCIN_CTRL:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIN_SET_ADCIN_CTRL*********\n"));
        if (NULL != pInBuffer&& inSize == sizeof(AUDIN_SET_ONOFF))
        {
            AUDIN_SET_ONOFF *prCmd = (AUDIN_SET_ONOFF *)pInBuffer;
            AudCfg_RestoreAoutRegs();
            Aud_Linein_AdcInFlowCtrl(pContext, prCmd);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIN_SET_ADCIN_CTRL args error\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIN_SET_IISIN_CTRL:
        LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIN_SET_IISIN_CTRL*********\n"));
        if (NULL != pInBuffer&& inSize == sizeof(AUD_IIS_CTRL_INFO))
        {
            AUD_IIS_CTRL_INFO* prCmd = (AUD_IIS_CTRL_INFO *)pInBuffer;
            Aud_Linein_IisInFlowCtrl(pContext, prCmd);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("IOCTL_AUDIN_SET_IISIN_CTRL args error\n"));
            fgRet = FALSE;
        }
        break;

    default:
        break;
    }

    return fgRet;
}

#if CONFIG_DRV_HDMI_RX
static bool IOControl_Audmhl(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;
    AUDIN_INFO_T *prAudInfo;

    switch(code)
    {

    case IOCTL_AUDMHL_CTL:
        {
            AUDMHL_OPEN_CTRL u4CtlType = (AUDMHL_OPEN_CTRL)(*pInBuffer);
            LOG(LOG_CTRLF, TEXT("*****IOCTL_AUDMHL_SET,cmd = 0x%x****\n"), u4CtlType);
            AudmhlSwitch(u4CtlType);
        }
        break;

    case IOCTL_AUDMHL_GET_INFO:
        if((NULL != pOutBuffer) && (outSize == sizeof(AUDIN_INFO_T)))
        {
            LOG(LOG_IO, TEXT("*****IOCTL_AUDMHL_GET_INFO*****\n"));
            prAudInfo = (AUDIN_INFO_T *)pOutBuffer;
            AudmhlGetAudInInfo(prAudInfo);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*****IOCTL_AUDMHL_GET_INFO args error*****\n"));
        }
        break;

#ifndef __linux__
    case IOCTL_AUDMHL_PARSING_INFO:
        {
            AUDIN_PARSING_INFO_T *prAudinPsringInfo = NULL;
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDMHL_PARSING_INFO*********\n"));
            if(NULL != pOutBuffer)
            {
                prAudinPsringInfo = (AUDIN_PARSING_INFO_T *)pOutBuffer;
                AudmhlParsingAudInfo(prAudinPsringInfo);
            }
            else
            {
                LOG(LOG_CTRLF, TEXT("IOCTL_AUDMHL_PARSING_INFO params error.\n"));
                fgRet = FALSE;
            }

        }
        break;

    case IOCTL_AUDMHL_RAW_INFO:
        {
            u32* u4Raw = NULL;
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDMHL_RAW_INFO*********\n"));
            if(NULL != pOutBuffer)
            {
                u4Raw = (u32 *)pOutBuffer;
                if(TRUE == AudmhlInIsRAW())
                {
                    *u4Raw = AUDMHL_IS_RAW;
                }
                else
                {
                    *u4Raw = AUDMHL_NOT_RAW;
                }
            }
            else
            {
                LOG(LOG_CTRLF, TEXT("IOCTL_AUDMHL_RAW_INFO params error.\n"));
                fgRet = FALSE;
            }

        }
        break;

    case IOCTL_AUDMHL_GET_AUDMHLBUFFER_INFO:
        {
            #if 0
            READ_BUFFER* prBuf = NULL;
            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDMHL_GET_AUDMHLBUFFER_INFO*********\n"));
            if(NULL != pOutBuffer)
            {
                prBuf = (READ_BUFFER*)pOutBuffer;
                AudmhlGetRecBuf(prBuf);
            }
            else
            {
                fgRet = FALSE;
            }
            #endif
        }
        break;

    case IOCTL_AUDMHL_MHL_SEND_INFO:
        {
            u32 u4AudmhlMsg = 0;
            if(NULL != pInBuffer)
            {
                u4AudmhlMsg = *((u32*)pInBuffer);
                AudmhlSendAudMsg(u4AudmhlMsg, AUDIN_CMD_PRI_HIGH);
            }
            else
            {
                LOG(LOG_CTRLF, TEXT("IOCTL_AUDMHL_MHL_SEND_INFO params error.\n"));
                fgRet = FALSE;
            }
        }
        break;
#endif // #ifndef __linux__

    default:
        break;
    }
    return fgRet;
}
#endif

static bool IOControl_Smix(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;
    u32 u4ErrType = AUD_ADSP_NORMAL;

    switch(code)
    {
    case IOCTL_AUD_SMIX_CTL:
        LOG(LOG_CTRLF, TEXT("*****IOCTL_AUD_SMIX_CTL*****\n"));
        if(NULL != pInBuffer&&inSize == sizeof(AUD_MEDIA_TYPE))
        {
            AUD_MEDIA_TYPE* prType = (AUD_MEDIA_TYPE*) pInBuffer;

            fgRet =  AudSmixSetMwCtrl(prType);
            pContext->u1DecId = SWMIX_DEC;
            if(AUD_MEDIA_ON == prType->eMediaCtrl){
                pContext->fgPlaying = TRUE;
            }
            else if(AUD_MEDIA_OFF == prType->eMediaCtrl){
                pContext->fgPlaying = FALSE;
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*****IOCTL_AUD_SMIX_CTL args error*****\n"));

            fgRet = FALSE;
        }
        break;

    case IOCTL_AUD_SMIX_SEND_BUFFER:
        //LOG(LOG_CTRLF, TEXT("*****IOCTL_AUD_SMIX_SEND_BUFFER*****\n"));
        if(NULL != pInBuffer)
        {
            AUD_SEND_BUF_INFO* prInfo = (AUD_SEND_BUF_INFO* )pInBuffer;

            #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
            if (u4AdspErrProcNotifyFlagGet(SWMIX_DEC))
            {
                LOG(LOG_FAIL, TEXT("*****IOCTL_AUD_SMIX_SEND_BUFFER Error Recover *****\n"));
                u4AdspErrProcNotifyFlagSet(SWMIX_DEC, 0);

                fgRet =  FALSE;
                u4ErrType = AUD_ADSP_RESET_FLAG;
            }
            else
            #endif
            {
                fgRet =  AudSmixSendBuffer(prInfo);
                u4ErrType = AUD_ADSP_NORMAL;
            }

        }
        else
        {
            LOG(LOG_FAIL, TEXT("*****IOCTL_AUD_SMIX_SEND_BUFFER args error*****\n"));
            fgRet = FALSE;
            u4ErrType = AUD_ADSP_NORMAL;
        }
        break;

    default:
        break;
    }

    if ((NULL != pOutBuffer) && (FALSE == fgRet))
    {
        u32* pu4Val = pOutBuffer;
        *pu4Val = u4ErrType;
    }
    return fgRet;
}


extern AUD_ESM_CONTEXT_T g_rAudEsmContext[];
static bool IOControl_Option(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;

    switch(code)
    {
    case IOCTL_AUDIO_SET_CLI_CMD_INFO:
        LOG(LOG_OTHER, TEXT("***********IOCTL_AUDIO_SET_CLI_CMD_INFO*********\n"));
        if (NULL != pInBuffer)
        {
            AUD_DEC_CLI_CFG  *pAudCliCfg = (AUD_DEC_CLI_CFG*)pInBuffer;
            AudSetCliCmd(pAudCliCfg->eAudCliType,pAudCliCfg->u4arg1,pAudCliCfg->u4arg2,pAudCliCfg->u4arg3,pAudCliCfg->u4arg4,(const s8 **)pAudCliCfg->ptParam);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("*** IOCTL_AUDIO_SET_CLI_CMD_INFO invalid para *****\n"));
            fgRet = FALSE;
        }
        break;

    case IOCTL_AUDIO_SET_BT_SCO:

        break;

    case IOCTL_AUDIO_FUNC_OPTION_SET:
        if (NULL != pInBuffer)
        {
            LOG(LOG_FEATURE, TEXT("***********IOCTL_AUDIO_FUNC_OPTION_SET*********\n"));
            AudSetFuncOption((AUD_FUNC_OPTION_T *)pInBuffer);
        }
        else
        {
            LOG(LOG_FAIL, TEXT("***IOCTL_AUDIO_FUNC_OPTION_SET invalid para *****\n"));
            fgRet = FALSE;
        }
        break;
#ifdef __linux__
    case IOCTL_AUDIN_COPY_FROM_USER:
        LOG(LOG_IO, TEXT("***********IOCTL_AUDIN_COPY_FROM_USER*********\n"));

        if (NULL != pInBuffer && inSize == sizeof(AUD_USER_INFO))
        {
            AUD_USER_INFO* prUser_Info = (AUD_USER_INFO*)pInBuffer;
            AUD_ESM_CONTEXT_T *pESMContext = &g_rAudEsmContext[pContext->u1DecId];
            u32 u4LeftSize = 0;
            u32 u4RightSize = 0;
            u32 u4WPtr = 0;

            LOG(LOG_CTRLF, TEXT("pContext->u4AfifoWPtr 0x%x, pContext->u4AfifoSA 0x%x, prUser_Info->puser 0x%x, size 0x%x\n"),
                (u32)pESMContext->u4AfifoWPtr, (u32)pESMContext->u4AfifoSA, (u32)prUser_Info->puser, (u32)prUser_Info->buf_size);

            if (pESMContext->u4AfifoWPtrForApp >= pESMContext->u4AfifoRPtr)
            {
                u4WPtr = pESMContext->u4AfifoWPtrForApp;
                u4LeftSize = pESMContext->u4AfifoEA - u4WPtr;
                u4RightSize = pESMContext->u4AfifoRPtr - pESMContext->u4AfifoSA;

                if (prUser_Info->buf_size < u4LeftSize)
                {
                    copy_from_user((void *)u4WPtr, prUser_Info->puser, prUser_Info->buf_size);
                }
                else
                {
                    copy_from_user((void *)u4WPtr, prUser_Info->puser, u4LeftSize);
                    u4RightSize = min(u4RightSize, (prUser_Info->buf_size-u4LeftSize));
                    copy_from_user((void *)pESMContext->u4AfifoSA, prUser_Info->puser+u4LeftSize, u4RightSize);
                }
            }
            else if (pESMContext->u4AfifoWPtrForApp < pESMContext->u4AfifoRPtr)
            {
                u4WPtr = pESMContext->u4AfifoWPtrForApp;
                u4LeftSize = pESMContext->u4AfifoRPtr - pESMContext->u4AfifoWPtrForApp;
                u4LeftSize = min(u4LeftSize, prUser_Info->buf_size);
                copy_from_user((void *)u4WPtr, prUser_Info->puser, u4LeftSize);
            }

            LOG(LOG_CTRLF, TEXT("***********IOCTL_AUDIN_COPY_FROM_USER success success***********\n"));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("***********IOCTL_AUDIN_COPY_FROM_USER args error*********\n"));
        }
        break;
#endif
    default:
        LOG(LOG_FAIL, TEXT("***Option IOCTL ID invalid para *****\n"));
        fgRet = FALSE;
        break;
    }
    return fgRet;
}


#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
#ifndef __linux__
static bool IOControl_Power(AUD_DRV_CONTEXT *pContext, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;

    switch(code)
    {
    case IOCTL_POWER_CAPABILITIES:
        LOG(LOG_POWER, TEXT("***********IOCTL_POWER_CAPABILITIES*********\n"));
        if (pOutBuffer != NULL && outSize >= sizeof(POWER_CAPABILITIES) && pOutSize != NULL)
        {
            PPOWER_CAPABILITIES pPowerCaps = (PPOWER_CAPABILITIES)pOutBuffer;
            memset(pPowerCaps, 0, sizeof(*pPowerCaps));
            //pPowerCaps->DeviceDx = DX_MASK(D0)| DX_MASK (D3)| DX_MASK(D4);   // support D0  & D3 & D4
            pPowerCaps->DeviceDx = 0x1F;
            *pOutSize = sizeof(*pPowerCaps);
        }
        break;

    case  IOCTL_POWER_QUERY:
        LOG(LOG_POWER, TEXT("***********IOCTL_POWER_QUERY*********\n"));
        if(pOutBuffer != NULL
            && outSize == sizeof(CEDEVICE_POWER_STATE)
            && pOutSize != NULL)
        {
            CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuffer;
            if(VALID_DX(NewDx))
            {
                *pOutSize = sizeof(CEDEVICE_POWER_STATE);
            }
        }
        break;

    // just return CurrentDx value    d0 or d3\d4
    case IOCTL_POWER_GET:
        LOG(LOG_POWER, TEXT("***********IOCTL_POWER_GET*********\n"));
        if(pOutBuffer != NULL
            && outSize == sizeof(CEDEVICE_POWER_STATE)
            && pOutSize != NULL)
        {
            *(PCEDEVICE_POWER_STATE) pOutBuffer = g_ADECCurrentDx;
            *pOutSize = sizeof(CEDEVICE_POWER_STATE);
        }
        break;


    case IOCTL_POWER_SET:
        LOG(LOG_POWER, TEXT("***********IOCTL_POWER_SET*********\n"));

        if(pOutBuffer != NULL
            && outSize == sizeof(CEDEVICE_POWER_STATE)
            && pOutSize != NULL)
        {
            CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE) pOutBuffer;

            if(VALID_DX(NewDx))
            {
                if(D4 == NewDx || D3 == NewDx) //power down
                {
                    if ((D3 == g_ADECCurrentDx) || (D4 == g_ADECCurrentDx))
                    {
                        LOG(LOG_POWER, TEXT("***********audio driver already in power down state *********\n"));
                    }
                    else
                    {
                        AudDev_PowerDown(AUD_DEVICE_ID_PRIMARY);

                        LOG(LOG_POWER, TEXT("***********audio driver enter power down mode *********\n"));
                    }
                }
                else if((D0 == NewDx) || (D1 == NewDx) || (D2== NewDx))
                {
                    if((D0 == g_ADECCurrentDx) || (D1 == g_ADECCurrentDx) || (D2== g_ADECCurrentDx))
                    {
                        LOG(LOG_POWER, TEXT("***********audio driver already in power on state *********\n"));
                    }
                    else
                    {
                        AudDev_PowerOn(AUD_DEVICE_ID_PRIMARY);
                        LOG(LOG_POWER, TEXT("***********Audio just D0 & D4 Power State  sleep ok *********\n"));
                    }
                }
                else
                {
                    LOG(LOG_POWER, TEXT("***********Audio just D0 & D4 Power State *********\n"));
                }

                g_ADECCurrentDx = NewDx;
            }
            else
            {
               fgRet = FALSE;
            }
        }
        break;

    default:
        break;
    }

    return fgRet;
}
#endif
#endif  // #if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT

bool ADE_IOControl(u32 context, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
    bool fgRet = TRUE;
    AUD_DRV_CONTEXT *pContext = (AUD_DRV_CONTEXT *)context;

    if (g_hBTContext && g_hBTContext != (void*)context)
        return TRUE;

    switch (code)
    {
    case IOCTL_AUDIO_SET_SE:
    case IOCTL_AUDIO_GET_SPECTRUM:
    case IOCTL_AUDIO_GET_SPECTRUM_SOURCE:
    case IOCTL_AUDIO_SET_FEATURE:
    case IOCTL_AUDIO_SET_BMANAGEMENT_MODE:
    case IOCTL_AUDIO_SET_BASS_MANAGEMENT_MODE:
    case IOCTL_AUDIO_SET_LRMIX:
        fgRet = IOControl_Post(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

    case IOCTL_AUDIO_GET_VOLUME:
    case IOCTL_AUDIO_SET_VOLUME:
    case IOCTL_AUDIO_SET_VOL_POLICY:
    case IOCTL_AUDIO_GET_REAR_VOLUME:
    case IOCTL_AUDIO_SET_REAR_VOLUME:
    case IOCTL_AUDIO_SET_MUTE_TYPE:
    case IOCTL_AUDIO_SET_SPEAKER_LAYOUT:
    case IOCTL_AUDIO_SET_TEST_TONE_TYPE:
    case IOCTL_AUDIO_SET_TEST_TONE_CHANNEL:
    case IOCTL_AUDIO_SET_TEST_TONE_ONOFF:
    case IOCTL_AUDIO_GET_OUTPUT_VOL:
    case IOCTL_AUDIO_SET_THRESHOLD:
    case IOCTL_AUDIO_SET_MUTE_DEC1:
    case IOCTL_AUDIO_SET_SRC_VOLUME:
    case IOCTL_AUDIO_GET_SRC_VOLUME:
    case IOCTL_AUDIO_SET_SRC_MUTE:
        fgRet = IOControl_VolSpk(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

    case IOCTL_AUDIO_CTL:    // Set Control
    case IOCTL_AUDIO_SET_FORMAT:    // Set Format
    case IOCTL_AUDIO_SET_ORIG_SAMPRATE:    // Set SampleRate
    case IOCTL_AUDIO_SET_PLAY_SPEED:
    case IOCTL_AUDIO_SET_AUD_INFO: // Set Audio Info
    case IOCTL_AUDIO_GET_PLAYBACK_INFO:// Get playback time
    case IOCTL_AUDIO_SET_ASRC_BYPASS:
    case IOCTL_AUDIO_SET_TARGETPTS:
    case IOCTL_AUDIO_GET_CURRENTPTS:
    case IOCTL_AUDIO_SET_AVSYNC_DISABLE:
    case IOCTL_AUDIO_GET_LATEST_PTS:
    case IOCTL_AUDIO_SET_AUDIO_DEC_INFO:
        fgRet = IOControl_DecCom(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

    case IOCTL_AUDIO_SET_AC3DRC:
    case IOCTL_AUDIO_SET_DTSDRC:
    case IOCTL_AUDIO_SET_DIVERSITY_INFO:
    case IOCTL_AUDIO_SET_APE_SEEKINFO:
    case IOCTL_AUDIO_SET_DEC4_INFO:
    case IOCTL_AUDIO_FEATURE_SUPPORT:
    case IOCTL_AUDIO_GET_CODEC_STATUS:
    case IOCTL_AUDIO_CODEC_RESET:
        fgRet = IOControl_Codec(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

    case IOCTL_AUDIO_CONNECT_ESM: // ESM Connect
    case IOCTL_AUDIO_DISCONNECT_ESM:
    case IOCTL_AUDIO_SEND_AU:
    case IOCTL_AUDIO_SEND_BUFFER:
    case IOCTL_AUDIO_SEND_BUFFER_KERNEL:
    case IOCTL_AUDIO_SEND_ESM_INFO:
    case IOCTL_AUDIO_SEND_END_OF_STREAM:
    case IOCTL_AUDIO_GET_AFIFO_INFO_VIRTUAL:
    case IOCTL_AUDIO_GET_AFIFO_WRITE_POINTER:
    case IOCTL_AUDIO_GET_READ_DATA_SUM:
    case IOCTL_AUDIO_GET_CODEC_FIFO_INFO:
        fgRet = IOControl_ESM(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

    case IOCTL_AUDIO_AOUT_CONFIG:
    case IOCTL_AUDIO_SWITCH_AOUT:
    case IOCTL_AUDIO_SET_DAC_TYPE:
    case IOCTL_AUDIO_SPDIF_ENABLE:
    case IOCTL_AUDIO_SET_TYPE_SPDIF:
    case IOCTL_AUDIO_INFO_FROM_DVP:
    case IOCTL_AUDIO_SET_REAR_I2S_GROUP:
    case IOCTL_AUDIO_SET_MIRACAST_ONOFF:
    case IOCTL_AUDIO_SET_MIRACAST_PARAM:
    case IOCTL_AUDIO_SET_REAR_OUT_MODE:
        fgRet = IOControl_AOut(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

    case IOCTL_AUDIO_GET_FRONT_STATUS:
    case IOCTL_AUDIO_GET_REAR_STATUS:
    case IOCTL_AUDIO_GET_FRONT_TYPE:
    case IOCTL_AUDIO_GET_REAR_TYPE:
    case IOCTL_AUDIO_GET_DEC_CONTEXT:
    case IOCTL_AUDIO_SET_DEC_CONTEXT:
    case IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE:
    case IOCTL_AUDIO_SET_REAR_MEDIA_TYPE:
    case IOCTL_AUDIO_SET_MEDIA_TYPE:
    case IOCTL_AUDIO_GET_MEDIA_TYPE_STATUS:
    case IOCTL_AUDIO_SET_ASRC_SWITCH:
    case IOCTL_AUDIO_DECONLY_ONOFF:
    case IOCTL_AUDIO_DECONLY_GET_BUF:
        fgRet = IOControl_Dual(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

    case IOCTL_AUDIN_SET_ONOFF:
    case IOCTL_AUDIN_SET_ADDR:
    case IOCTL_AUDIN_GET_DEC_DATALEN:
    case IOCTL_AUDIN_GET_DEC_CFG_INFO:
    case IOCTL_AUDIN_REAR_VOL_GAIN_INFO:
    case IOCTL_AUDIN_FRONT_VOL_GAIN_INFO:
    case IOCTL_AUDIN_IIS_CTRL:
    case IOCTL_AUDIN_INPUT_TYPE:
    case IOCTL_AUDIN_SET_ADCIN_CTRL:
    case IOCTL_AUDIN_SET_IISIN_CTRL:
        fgRet = IOControl_AudIn(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;


#if CONFIG_DRV_HDMI_RX
    case IOCTL_AUDMHL_CTL:
    case IOCTL_AUDMHL_GET_INFO:
    case IOCTL_AUDMHL_PARSING_INFO:
    case IOCTL_AUDMHL_RAW_INFO:
    case IOCTL_AUDMHL_GET_AUDMHLBUFFER_INFO:
    case IOCTL_AUDMHL_MHL_SEND_INFO:
        fgRet = IOControl_Audmhl(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;
#endif

    case IOCTL_AUDIO_SET_CLI_CMD_INFO:
    case IOCTL_AUDIO_SET_BT_SCO:
    case IOCTL_AUDIO_FUNC_OPTION_SET:
    case IOCTL_AUDIN_COPY_FROM_USER:
        fgRet = IOControl_Option(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

    case IOCTL_AUD_SMIX_CTL:
    case IOCTL_AUD_SMIX_SEND_BUFFER:
        fgRet = IOControl_Smix(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;

#ifndef __linux__
  #if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
    case IOCTL_POWER_CAPABILITIES:
    case IOCTL_POWER_QUERY:
    case IOCTL_POWER_GET:
    case IOCTL_POWER_SET:
        fgRet = IOControl_Power(pContext, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
        break;
  #endif
#endif
    default:
        fgRet = FALSE;
        break;
    }

    return fgRet;
}

#ifndef __linux__
bool WINAPI DllEntry(void* hInstance, u32 reason, void *pvReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls((HMODULE)hInstance);
    }
    return TRUE;
}
#else
EXPORT_SYMBOL(ADE_IOControl);
EXPORT_SYMBOL(ADE_Open);
EXPORT_SYMBOL(ADE_Close);
#endif
