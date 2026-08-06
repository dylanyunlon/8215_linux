#include <linux/fs.h>
#include <asm/uaccess.h>

#include <media/v4l2-ioctl.h>
#include "dvp_hal.h"
#include "aud_ioctrl.h"
#include "drv_aud.h"
#include "aud_output.h"
#include "x_aud_dec.h"

#include "wch_drv.h"

#include "slab.h"

#include "windev.h"

#include <signal.h>

#include <linux/types.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/semaphore.h>

#define VEDIO_WFFDONE_WAIT_TIMEOUT (200)
#define ADSP1_ADSP4_SPERATE

struct sock *nl_sk = NULL;

u32                context = 0;
static u32         g_u4AudFlag;

static WCH_SRC_APP_ID_E    mSrcAppId = SRC_APP_DISPLAY;
WCH_BUFF_INFO_T            mWchBufferInfo;
static WCH_CTL_PARAM_T     mWchCtl;
static WCH_CFG_T           mWchCfg;

static u8 wcIndex = 0;
DEFINE_SEMAPHORE(wchFrameSem);

extern bool g_isOpenWch;


static void wch_buffer_dvpdone(u32 *bufindex)
{
    u32 index = *bufindex;

    wcIndex = index;
    up(&wchFrameSem);
}

int dvp_close_audio(bool IsFront)
{
#ifndef ADSP1_ADSP4_SPERATE

    AUD_OUT_MEDIA_TYPE_T eSrcType = AUD_OUT_MEDIA_NONE;
    pr_info("[dvp][drv] close DVP_AudOutput: %d\n", IsFront);
    if (TRUE == IsFront) {
        AUD_OUT_MEDIA_TYPE_T eFrontSrcType = AUD_OUT_MEDIA_NONE;
        ADE_IOControl(context, IOCTL_AUDIO_GET_FRONT_TYPE, NULL, 0, &eFrontSrcType, sizeof(u8), NULL);
        if (eFrontSrcType == AUD_OUT_MEDIA_DVD)
        {
            g_u4AudFlag  = (g_u4AudFlag & (~(DIR_FRONT)));
            ADE_IOControl(context, IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE, (u8 *)&eSrcType,
            sizeof(AUD_OUT_MEDIA_TYPE_T), NULL, 0, NULL);
            pr_debug("close Audio : set media type for dvd front\n");
        }
    }
    else
    {
        AUD_OUT_MEDIA_TYPE_T eFrontSrcType = AUD_OUT_MEDIA_NONE;
        ADE_IOControl(context, IOCTL_AUDIO_GET_FRONT_TYPE, NULL, 0, &eFrontSrcType, sizeof(u8), NULL);
        if (eFrontSrcType == AUD_OUT_MEDIA_DVD)
        {
            g_u4AudFlag  = (g_u4AudFlag & (~(DIR_REAR)));
            ADE_IOControl(context, IOCTL_AUDIO_SET_REAR_MEDIA_TYPE, (u8 *)&eSrcType,
            sizeof(AUD_OUT_MEDIA_TYPE_T), NULL, 0, NULL);
            pr_debug("close Audio : set media type for dvd rear\n");
        }
    }

#else // sperate adsp1 and adsp4

    AUD_MEDIA_TYPE eSrcType = {0};
    pr_info("[dvp][drv](sperate14)+++close DVP_AudOutput:isFront = %d\n", IsFront);

    eSrcType.eMediaCtrl = AUD_MEDIA_OFF;
    eSrcType.eMediaSrc = AUD_MEDIA_SOURCE_DVP;

    if (TRUE == IsFront)
    {
        g_u4AudFlag  = (g_u4AudFlag & (~(DIR_FRONT)));
        eSrcType.eMediaOut = AUD_MEDIA_OUT_FRONT;
        pr_debug("close Audio : set media type for dvd front\n");

    }
    else
    {
        g_u4AudFlag  = (g_u4AudFlag & (~(DIR_REAR)));
        eSrcType.eMediaOut = AUD_MEDIA_OUT_REAR;
        pr_debug("close Audio : set media type for dvd rear\n");
    }

    int ret = ADE_IOControl(context, IOCTL_AUDIO_SET_MEDIA_TYPE, (u8 *)&eSrcType,
                sizeof(AUD_MEDIA_TYPE), NULL, 0, NULL);
    pr_debug("[dvp][drv]close Audio, ret = %d\n", ret);

#endif
    if (g_u4AudFlag == 0)
    {
        if (NULL != context)
        {
            ADE_Close(context);
            context = NULL;
            pr_info("[dvp][drv]dvp_close_audio: close Audio Device\n");
        }
    }
    return 0;
}



int dvp_open_audio(bool IsFront)
{

#ifndef ADSP1_ADSP4_SPERATE

    AUD_OUT_MEDIA_TYPE_T  eSrcType = AUD_OUT_MEDIA_DVD;
    int ret = false;

    pr_info("[dvp][drv]++open Audio device+++\n");

    context = ADE_Open(0, 0, 0);
    if (IS_ERR(context))
    {
        pr_err("[dvp][drv] open Audio device Fail:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }

    if (TRUE == IsFront)
    {
        g_u4AudFlag  = g_u4AudFlag | DIR_FRONT;
        ret = ADE_IOControl(context, IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE, (u8 *)&eSrcType,
        sizeof(AUD_OUT_MEDIA_TYPE_T), NULL, 0, NULL);
        pr_debug("[dvp][drv]open Audio : set media type for dvd front\n");
    }
    else
    {
        g_u4AudFlag  = g_u4AudFlag | DIR_REAR;
        ret = ADE_IOControl(context, IOCTL_AUDIO_SET_REAR_MEDIA_TYPE, (u8 *)&eSrcType, sizeof(AUD_OUT_MEDIA_TYPE_T), NULL, 0, NULL);
        pr_debug("[dvp][drv]open Audio : set media type for dvd rear\n");
    }

#else // sperate adsp1 and adsp4

    AUD_MEDIA_TYPE eSrcType = {0};
    int ret = 0;
    pr_info("[dvp][drv](sperate14) +++open Audio device+++\n");
    context = ADE_Open(0, 0, 0);
    if (IS_ERR(context))
    {
        pr_err("[dvp][drv] open Audio device Fail:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }
    eSrcType.eMediaSrc = AUD_MEDIA_SOURCE_DVP;
    pr_debug("[dvp][drv]open Audio :MediaSrc = %d\n", eSrcType.eMediaSrc);
    if (TRUE == IsFront)
    {
        g_u4AudFlag  = g_u4AudFlag | DIR_FRONT;
        eSrcType.eMediaOut = AUD_MEDIA_OUT_FRONT;
        pr_debug("[dvp][drv]open Audio : set media type for dvd front\n");
    }
    else
    {
        g_u4AudFlag  = g_u4AudFlag | DIR_REAR;
        eSrcType.eMediaOut = AUD_MEDIA_OUT_REAR;
        pr_debug("[dvp][drv]open Audio : set media type for dvd rear\n");
    }
    eSrcType.eMediaCtrl = AUD_MEDIA_ON;
    ret = ADE_IOControl(context, IOCTL_AUDIO_SET_MEDIA_TYPE, (u8 *)&eSrcType,
    sizeof(AUD_MEDIA_TYPE), NULL, 0, NULL);
    pr_debug("[dvp][drv]open Audio, ret = %d\n", ret);

#endif
   if (g_u4AudFlag == 0)
    {
        if (NULL != context)
        {
            ADE_Close(context);
            context = NULL;
            pr_info("[dvp][drv]dvp_open_audio: close Audio Device\n");
        }
    }
    return 0;
}

int dvp_informAudioSampleRate(u8 *spectrum)
{
    u8 rate = *spectrum;
    u32 audiofilpSample;
    pr_info("[dvp][drv]open Audio device for set dvd sampleRate: %d\n", rate);

    if (g_u4AudFlag == 0) {
        pr_err("[dvp][drv]need not to inform sampleRate, g_u4AudFlag = 0:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }

    audiofilpSample = ADE_Open(0, 0, 0);

    pr_info("[dvp][drv]dvp_informAudioSampleRate: audiofilpSample = %d\n", audiofilpSample);

    if (IS_ERR(audiofilpSample)) {
        pr_err("[dvp][drv]open Audio device Fail:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }

    ADE_IOControl(audiofilpSample, IOCTL_AUDIO_INFO_FROM_DVP, (u8 *)&rate,
              sizeof(u8), NULL, 0, NULL);
    if (NULL != audiofilpSample) {
        ADE_Close(audiofilpSample);
        audiofilpSample = NULL;
    }

    pr_info("[dvp][drv]dvp_informAudioSampleRate leave\n");
    return 0;
}

int dvp_getAudioSpectrum(void *spectrum, u32 length)
{
    unsigned int *temp = spectrum;
    unsigned int retByte = 0;
    pr_debug("[dvp][drv]open Audio device for getAudioSpectrum\n");

    if (g_u4AudFlag == 0) {
        pr_info("[dvp][drv]need not to inform sampleRate, g_u4AudFlag = 0\n");
        return -1;
    }
    if (context == 0) {
        pr_info("[dvp][drv]adec is not open\n");
        return -1;
    }
    ADE_IOControl(context, IOCTL_AUDIO_GET_SPECTRUM, NULL,
              0, spectrum, length, &retByte);

    pr_debug("[dvp][drv]spectrum %d, %d, %d, %d, %d, %d, %d, %d\n", *temp, *(temp+1),
        *(temp+2), *(temp+3), *(temp+4), *(temp+5), *(temp+6), *(temp+7));

    return 0;
}


int dvp_getWcDVpIndex(void *index, u32 length)
{
    int err = 0;

    err = down_timeout(&wchFrameSem, msecs_to_jiffies(VEDIO_WFFDONE_WAIT_TIMEOUT));
    if (err == 0) {
        *(char *)index = wcIndex;
    } else {
        *(char *)index = 0xff;
    }
    return 0;
}

int dvp_open_video(WCH_BUFF_INFO_T *bufferAddr, u32 length)
{
    int ret = 0;
    int mWchId;
    WCH_BUFF_INFO_T bufferInfo;

    wcIndex = 0;
    pr_info("[dvp][drv] dvp_open_video Enter !\n");

    if (g_isOpenWch) {
        pr_info("[dvp][drv] wch has opened.\n");
    } else {
        if (WchIoControl(1, IOCTL_WCH_OPEN, (u8 *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),
                 (u8 *)&mWchId, sizeof(int), NULL)) {
            pr_err("[dvp][drv]VideoIn::start() wch open fail![file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
            return -1;
        }
        g_isOpenWch = true;
    }
    memset(&bufferInfo, 0, sizeof(WCH_BUFF_INFO_T));

    if (WchIoControl(1, IOCTL_WCH_GET_ADDR, (u8 *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),
             (u8 *)&mWchBufferInfo, sizeof(WCH_BUFF_INFO_T), NULL)) {
        pr_err("[dvp][drv]VideoIn::start() IOCTL_WCH_GET_ADDR L_FAILED![file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }

    memcpy(bufferAddr, &mWchBufferInfo, length);
    dvp_config_video();

    if (WchIoControl(1, IOCTL_WCH_CONFIG, (u8 *)&mWchCtl, sizeof(WCH_CTL_PARAM_T),
             NULL, 0, NULL)) {
        pr_err("[dvp][drv]Video Config: wch config fail:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);

        return -1;
    }

    pr_info("[dvp][drv] Video Config: wch config success!\n");

    if (WchIoControl(1, IOCTL_WCH_START, (u8 *)&mSrcAppId, sizeof(mSrcAppId),
             NULL, 0, NULL)) {
        pr_err("[dvp][drv]onVdoSignal(): wch start fail:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);

        return -1;
    }
    pr_info("[dvp][drv] Video wch start OK.\n");
    return ret;
}

int dvp_config_video()
{
    mWchCfg.fgVSyncPolarity = TRUE; /* FALSE is LOW level present sync.*/
    mWchCfg.fgHSyncPolarity = TRUE; /* TRUE is High.*/

    mWchCfg.fgBotFieldFirst = TRUE; // TRUE is bottom field first

    mWchCfg.eInputSrc = DATA_SRC_DVD;
    mWchCfg.eInputFmt = DATA_FMT_YUV444;
    mWchCfg.u4SrcWidth = WCH_WIDTH;
    mWchCfg.u4SrcHeight = WCH_HEIGHT; //tobecheck

    mWchCfg.u4DstWidth = WCH_WIDTH;
    mWchCfg.u4DstHeight = WCH_HEIGHT;
    mWchCfg.fgProgressive = TRUE;
    mWchCfg.eOutputFmt = DATA_FMT_YUV420;

    mWchCfg.u1YUVMask = WCH_YUV_SEL_MASK;//may change cause of different hw
    mWchCfg.u1USel = 4;//above
    mWchCfg.u1VSel = 4;//above

    mWchCfg.u4SrcStartX = 0x61;
    mWchCfg.u4SrcStartYTop = 0xF;
    mWchCfg.u4SrcStartYBot = 0;

    mWchCtl.tWchCfg = mWchCfg;
    mWchCtl.eSrcId = mSrcAppId;
    mWchCtl.tWchCfg.GetWchBufIndx = wch_buffer_dvpdone;

    return 0;
}

int dvp_close_video()
{
    if (!g_isOpenWch) {
        pr_info("[dvp][drv] wch has closed.\n");
        return -1;
    }

    g_isOpenWch = false;
    if (WchIoControl(1, IOCTL_WCH_STOP, (u8 *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),
             NULL, 0, NULL)) {
        pr_debug(":cvbs_stop_video IOCTL_WCH_STOP L_FAILED\r\n");
        return -1;
    }

    if (WchIoControl(1, IOCTL_WCH_CLOSE, (u8 *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),
             NULL, 0, NULL)) {
        pr_err("[dvp][drv]cvbs_stop_video IOCTL_WCH_CLOSE L_FAILED:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }
    return 0;
}


