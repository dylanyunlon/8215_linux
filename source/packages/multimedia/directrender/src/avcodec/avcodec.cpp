/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */
#define LOG_TAG "[AVCodec]"
#include "atcdtlog.h"
#include <stdbool.h>
#include <linux/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include "avcodec.h"
#include "atcvideosink.h"
#include "atcomxvdecinst.h"
#include "async_queue.h"

MMLOG_DECLARATION(LOG_MOD_DT);

void onEmptyBufferDone(void *pAppData, VDEC_BUFFER_INFO_T* pBuffer);
void onFillBufferDone(void *pAppData, VDEC_BUFFER_INFO_T* pBuffer);

static BUFFER_CALLBACKTYPE buffercallbacks =
    { onEmptyBufferDone, onFillBufferDone };

void onEmptyBufferDone(void *pAppData, VDEC_BUFFER_INFO_T* pBuffer)
{
    AVCodec *inst = (AVCodec *)pAppData;

    //AVCodecBuffer *buf = new AVCodecBuffer(pBuffer->buffer, pBuffer->bufSz);
    PRINT_DEBUG("onEmptyBufferDone before inst:%p buf:%p, pts:%lld\r\n",
            inst, pBuffer->buffer, pBuffer->timestampus);
    AVCodecBuffer *buf = inst->dequeueInputBuffer();
    PRINT_DEBUG("onEmptyBufferDone after inst:%p codecbuf:%p, flags:0x%x\r\n",
            inst, buf, buf->flags());
    //buf->setBufferInfo(0, 0, 0, 0);
    buf->setBuffer((uint8_t *)pBuffer->buffer, pBuffer->bufSz);
    inst->postMessageAsync(kMsgEmptyBufferDone, buf);
}

void onFillBufferDone(void *pAppData, VDEC_BUFFER_INFO_T* pBuffer)
{
    AVCodec *inst = (AVCodec *)pAppData;
    uint32_t flags = 0;

    PRINT_DEBUG("[%p] pBuffer:%p, width:%d, height:%d, len:%d, flags:0x%x, pts:%lld\r\n",
            inst, pBuffer->buffer, pBuffer->width, pBuffer->height,
            pBuffer->datasz, pBuffer->flags, pBuffer->timestampus);

    AVCodecBuffer *buf = inst->dequeueOutputBuffer();
    if (pBuffer->flags & OMX_BUFFERFLAG_EOS) {
        flags |= BUFFER_FLAG_EOS;
    }
    if (pBuffer->flags & OMX_BUFFERFLAG_SIZECHANGE) {
        flags |= BUFFER_FLAG_SIZECHANGE;
    }

    buf->setBuffer((uint8_t *)pBuffer->buffer, pBuffer->datasz);
    buf->setBufferInfo(pBuffer->width, pBuffer->height, pBuffer->timestampus, flags);
    inst->postMessageAsync(kMsgFillBufferDone, buf);
}

AVCodecBuffer::AVCodecBuffer(size_t capacity)
    : mRangeOffset(0) {
    mData = malloc(capacity);
    if (mData == NULL) {
        mCapacity = 0;
        mRangeLength = 0;
    } else {
        mCapacity = capacity;
        mRangeLength = 0;
    }
    mOwnsData = true;
    mPresentationTimeUs = 0;
    mWidth = 0;
    mHeight = 0;
    mFlags = 0;
}

AVCodecBuffer::AVCodecBuffer(uint8_t *base, size_t capacity)
{
    mData = base;
    mCapacity = capacity;
    mOwnsData = false;
    mRangeLength = 0;
    mRangeOffset = 0;
    mPresentationTimeUs = 0;
    mWidth = 0;
    mHeight = 0;
    mFlags = 0;

}

AVCodecBuffer::~AVCodecBuffer() {
    if (mOwnsData && (mData != NULL)) {
        free(mData);
        mData = NULL;
    }
}

void AVCodecBuffer::setRange(size_t offset, size_t size) {
    if ((offset >= mCapacity) || (offset + size >= mCapacity))
        return;

    mRangeOffset = offset;
    mRangeLength = size;
}

void AVCodecBuffer::setBuffer(uint8_t *base, size_t capacity)
{
    mData = base;
    mCapacity = capacity;
    mRangeLength = capacity;
    mRangeOffset = 0;
}

void AVCodecBuffer::setBufferInfo(uint32_t w, uint32_t h, int64_t pts, uint32_t flags) {
    mWidth = w;
    mHeight = h;
    mPresentationTimeUs = pts;
    mFlags = flags;
}

void *message_handler(void *data)
{
    AVCodec *inst = (AVCodec *)data;
    MsgInfo *msg = NULL;
    int msgType;
    if (inst == NULL)
        return NULL;
    PRINT_INFO("message_handler enter inst:%p\n", inst);

    do
    {
        msg = (MsgInfo *)async_queue_pop((AsyncQueue *)inst->mMsgQueue);
        PRINT_DEBUG("get msg:%p, type:%d\n", msg, msg ? msg->type : -1);
        msgType = msg->type;
        switch (msgType) {
        case kMsgQueueInputBuffer:
        {
            AVCodecBuffer *buf = (AVCodecBuffer *)msg->buffer_done.codecBuffer;
            if (inst->mState == AVCodec::FLUSHING) {
                PRINT_INFO("kMsgQueueInputBuffer buf:%p, InputBufQ len:%d\n", buf, async_queue_length((AsyncQueue*)inst->mInputBufQ));
                async_queue_push((AsyncQueue*)inst->mInputBufQ, (void*)buf);
                inst->changeStateIfWeOwnAllBuffers();
            } else {
                inst->onQueueInputBuffer(buf);
            }
            break;
        }
        case kMsgReleaseOutputBuffer:
        {
            AVCodecBuffer *buf = (AVCodecBuffer *)msg->buffer_done.codecBuffer;
            if (inst->mState == AVCodec::FLUSHING) {
                PRINT_INFO("kMsgReleaseOutputBuffer buf:%p, InputBufQ len:%d\n", buf, async_queue_length((AsyncQueue*)inst->mOutputBufQ));
                buf->setBufferInfo(0, 0, 0, BUFFER_FLAG_DISCARD);
                async_queue_push((AsyncQueue*)inst->mOutputBufQ, (void*)buf);
                inst->changeStateIfWeOwnAllBuffers();
            } else {
                inst->onReleaseOutputBuffer(buf);
            }
            break;
        }
        case kMsgEmptyBufferDone:
        {
            AVCodecBuffer *buf = (AVCodecBuffer *)msg->buffer_done.codecBuffer;
            PRINT_DEBUG("kMsgEmptyBufferDone codecbuf:%p\n", buf);
            if (inst->mState == AVCodec::FLUSHING) {
                async_queue_push((AsyncQueue*)inst->mInputBufQ, (void*)buf);
                inst->changeStateIfWeOwnAllBuffers();
            } else {
                if (inst->mCallback != NULL)
                    inst->mCallback->onInputBufferAvailable(inst, buf);
            }
            break;
        }
        case kMsgFillBufferDone:
        {
            AVCodecBuffer *buf = (AVCodecBuffer *)msg->buffer_done.codecBuffer;
            PRINT_DEBUG("kMsgFillBufferDone codecbuf:%p\n", buf);
            if (inst->mState == AVCodec::FLUSHING) {
                buf->setBufferInfo(0, 0, 0, BUFFER_FLAG_DISCARD);
                async_queue_push((AsyncQueue*)inst->mOutputBufQ, (void*)buf);
                inst->changeStateIfWeOwnAllBuffers();
            } else {
                if (inst->mCallback != NULL)
                    inst->mCallback->onOutputBufferAvailable(inst, buf);
            }
            break;
        }
        case kMsgStart:
        {
            msg->err = inst->onStart();
            break;
        }
        case kMsgFlush:
        {
            msg->err = inst->onFlush();
            break;
        }
        default:
            break;
        }

        if (msg->isSync) {
            pthread_mutex_lock(&inst->mMsgMutex);
            msg->processed = true;
            pthread_cond_signal(&inst->mMsgCond);
            pthread_mutex_unlock(&inst->mMsgMutex);
        } else {
            delete msg;
        }
    }
    while(msgType != kMsgStop);
    PRINT_INFO("message_handler exit\n");

    return NULL;
}

int AVCodec::postMessageSync(int msgType, void *data)
{
    int ret = 0;
    MsgInfo *msg = new MsgInfo;
    msg->type = msgType;
    msg->processed = false;
    msg->isSync = true;
    msg->buffer_done.codecBuffer = (AVCodecBuffer *)data;
    PRINT_DEBUG("post_message:%d, mMsgQueue:%p\n", msgType, mMsgQueue);

    pthread_mutex_lock(&mMsgMutex);
    async_queue_push((AsyncQueue *)mMsgQueue, msg);
    while (!msg->processed) {
        pthread_cond_wait(&mMsgCond, &mMsgMutex);//or pthread_cond_timedwait
    }
    pthread_mutex_unlock(&mMsgMutex);
    ret = msg->err;

    delete msg;

    return ret;
}

int AVCodec::postMessageAsync(int msgType, void *data)
{
    MsgInfo *msg = new MsgInfo;
    msg->type = msgType;
    msg->processed = false;
    msg->isSync = false;
    msg->buffer_done.codecBuffer = (AVCodecBuffer *)data;
    PRINT_DEBUG("post_message:%d, mMsgQueue:%p\n", msgType, mMsgQueue);
    async_queue_push((AsyncQueue *)mMsgQueue, msg);

    return 0;
}


AVCodec *AVCodec::CreateByType(const std::string mime)
{
    PRINT_INFO("CreateByType enter\n");
    AVCodec *codec = new AVCodec;
    codec->mVdecInst = atc_vdec_open();
    if (NULL == codec->mVdecInst) {
        PRINT_ERROR( "atc_vdec_open failed\r\n");
        delete codec;
        return NULL;
    }
    atc_vdec_set_callbacks(codec->mVdecInst, &buffercallbacks, codec);
    codec->mVsinkInst = atc_video_sink_open();
    if (NULL == codec->mVsinkInst) {
        PRINT_ERROR( "atc_video_sink_open failed\r\n");
        atc_video_sink_close(codec->mVdecInst);
        delete codec;
        return NULL;
    }

    const char *pLogLvl = getenv("DT_DEBUG_LOG");
    if (pLogLvl != NULL) {
        int lvl = atoi(pLogLvl);
        PRINT_INFO("set dt log level:%d\n", lvl);
        LOG_ModSetLevel(LOG_MOD_DT, lvl);
    }
    return codec;
}
int AVCodec::configure(
        uint32_t width, uint32_t height,
        IAtcSurface *nativeWindow)
{
    ATC_VSINK_CFG_T rSinkCfg;
    ATC_VSINK_FMT_INFO_T rSinkformat;
    uint64_t support_out_formats = 0;
    uint32_t outfmt = 0;

    PRINT_INFO( "atc_video_sink_set_surface\r\n");
    if (!atc_video_sink_set_surface(mVsinkInst, nativeWindow)) {
        PRINT_ERROR("atc_video_sink_set_surface failed\r\n");
        return -1;
    }

    PRINT_INFO( "atc_video_sink_get_config\r\n");
    memset(&rSinkCfg, 0, sizeof(ATC_VSINK_CFG_T));
    if (!atc_video_sink_get_config(mVsinkInst, &rSinkCfg)) {
        PRINT_ERROR("atc_video_sink_get_config(%p) failed\r\n", mVsinkInst);
        return -1;
    }
    if (!atc_vdec_get_output_formats(mVdecInst, &support_out_formats, &outfmt)) {
        PRINT_ERROR("atc_vdec_get_output_formats(%p) failed\r\n", mVdecInst);
        return -1;
    }

    if (0 == ((rSinkCfg.support_fmts) & support_out_formats)) {
        PRINT_ERROR("videosink's support formats(0x%llx) are not intersect with vdec support output formats(0x%llx)\r\n",
                  rSinkCfg.support_fmts, support_out_formats);
        return -1;
    }

    if (0 == ((rSinkCfg.support_fmts) &(1 << outfmt))) {
        PRINT_ERROR("don't support the format(%d), (support_fmts: 0x%llx), vdecinst: %p\r\n",
                 outfmt, rSinkCfg.support_fmts, mVdecInst);
        return -1;
    }

    PRINT_INFO("vsink's buffers (min: %d, max: %d, use: %d)\r\n",
                rSinkCfg.min_count, rSinkCfg.max_count, rSinkCfg.use_count);
    if (!atc_video_sink_set_buffer_count(mVsinkInst, rSinkCfg.use_count)) {
        PRINT_ERROR("fail in atc_video_sink_set_buffer_count(%p, %d)\r\n",
                   mVsinkInst, rSinkCfg.use_count);
        return -1;
    }

    memset(&rSinkformat, 0, sizeof(rSinkformat));
    rSinkformat.format = ATC_PIX_FMT_NV12M_PRIVATE1;
    rSinkformat.width  = width;
    rSinkformat.height = height;
    rSinkformat.stride = (width + 15) / 16 * 16;
    rSinkformat.interlaced = false;
    rSinkformat.fourcc = 0;

    PRINT_INFO("atc_video_sink_set_format, width:%d, height:%d\r\n",
                  width, height);
    if (!atc_video_sink_set_format(mVsinkInst, &rSinkformat)) {
        PRINT_ERROR("atc_video_sink_set_format(%p) failed\r\n", mVsinkInst);
        return -1;
    }

    ATC_VDEC_INPUT_FMT_INFO_T rVdecInputFmt;
    memset(&rVdecInputFmt, 0, sizeof(rVdecInputFmt));
    rVdecInputFmt.eVType = OMX_VIDEO_CodingAVC;
    rVdecInputFmt.width  = width;
    rVdecInputFmt.height = height;
    rVdecInputFmt.interlaced = false;
    rVdecInputFmt.fps_d = 1;
    rVdecInputFmt.fps_n = 30;
    if (!atc_vdec_set_input_format(mVdecInst, &rVdecInputFmt)) {
        PRINT_ERROR("atc_vdec_set_input_format(%p) failed\r\n", mVdecInst);
        return -1;
    }
    if (NULL != ts) {
        delete ts;
        ts = NULL;
    }
    ts = new SystemTimeSource();
    return 0;
}

int AVCodec::setCallback(BufferCallback *callback)
{
    if (callback == NULL)
        return -1;
    mCallback = callback;
    return 0;
}

int AVCodec::onStart()
{
    PRINT_INFO("AVCodec::start\r\n");
    if ((mVsinkInst == NULL) || (mVdecInst == NULL)) {
        PRINT_ERROR("invalid params:sink %p, vdec %p\r\n", mVsinkInst, mVdecInst);
        return -1;
    }

    if (mState == UNINITIALIZED) {
        mInputBufQ = (void*)async_queue_new();
        if (mInputBufQ == NULL)
            goto fail;

        mOutputBufQ = (void*)async_queue_new();
        if (mOutputBufQ == NULL)
            goto fail;

        for (int i = 0; i< kMaxCodecInBufferCnt; i++) {
            AVCodecBuffer *buf = new AVCodecBuffer(NULL, 0);
            mInputBufs[i] = buf;
            async_queue_push((AsyncQueue*)mInputBufQ, (void*)buf);
        }
        for (int i = 0; i< kMaxCodecOutBufferCnt; i++) {
            AVCodecBuffer *buf = new AVCodecBuffer(NULL, 0);
            mOutputBufs[i] = buf;
            async_queue_push((AsyncQueue*)mOutputBufQ, (void*)buf);
        }

        PRINT_INFO("atc_video_sink_start\r\n");
        if (!atc_video_sink_start(mVsinkInst)) {
          PRINT_ERROR( "atc_video_sink_start(%p) failed\r\n", mVsinkInst);
          goto fail;
        }

        mState = INITIALIZED;
    } else if (mState == FLUSHED) {
        for (int i = 0; i< kMaxCodecInBufferCnt; i++) {
            AVCodecBuffer *buf = (AVCodecBuffer *)async_queue_pop_nb((AsyncQueue*)mInputBufQ);
            PRINT_INFO("onStart dequeue buf:%p\r\n", buf);
            if ((mCallback != NULL) && (buf != NULL))
                mCallback->onInputBufferAvailable(this, buf);
        }
        for (int i = 0; i< kMaxCodecOutBufferCnt; i++) {
            AVCodecBuffer *buf = (AVCodecBuffer *)async_queue_pop_nb((AsyncQueue*)mOutputBufQ);
            if ((buf != NULL) && (buf->flags() == BUFFER_FLAG_DISCARD)) {
                PRINT_INFO("onStart release output buf:%p\r\n", buf);
                VDEC_BUFFER_INFO_T rVdecOutInfo;
                memset(&rVdecOutInfo, 0, sizeof(rVdecOutInfo));
                rVdecOutInfo.buffer = buf->base();
                rVdecOutInfo.bufSz = buf->size();
                if (RET_ATCVDECINST_OK != atc_vdec_release_output_buffer(mVdecInst, &rVdecOutInfo)) {
                    PRINT_ERROR("release buffer failed, buf:%p len:%d\n", rVdecOutInfo.buffer, rVdecOutInfo.bufSz);
                    goto vdec_start_fail;
                }
            }
            async_queue_push((AsyncQueue*)mOutputBufQ, (void*)buf);
        }
    }

    PRINT_INFO("atc_vdec_start\r\n");
    if (!atc_vdec_start(mVdecInst)) {
        PRINT_ERROR("atc_vdec_start(%p) failed\r\n", mVdecInst);
        goto vdec_start_fail;
    }

    mOutputBufferCounter = 0;
    mState = STARTED;
    first_render = 1;

    //if (NULL != ts) {
    //    ts->setTimeUs(0);
    //}

    return 0;

vdec_start_fail:
    atc_video_sink_stop(mVsinkInst);

fail:
    if (mInputBufQ != NULL)
        async_queue_free((AsyncQueue*)mInputBufQ);
    if (mOutputBufQ != NULL)
        async_queue_free((AsyncQueue*)mOutputBufQ);
    for (int i = 0; i< kMaxCodecInBufferCnt; i++) {
        if (mInputBufs[i] != NULL) {
            delete mInputBufs[i];
            mInputBufs[i] = NULL;
        }
    }
    for (int i = 0; i< kMaxCodecOutBufferCnt; i++) {
        if (mOutputBufs[i] != NULL) {
            delete mOutputBufs[i];
            mOutputBufs[i] = NULL;
        }
    }

    return -1;
}

int AVCodec::onFlush()
{
    PRINT_INFO("AVCodec::onFlush\r\n");
    if ((mVsinkInst == NULL) || (mVdecInst == NULL)) {
        PRINT_ERROR("invalid params:sink %p, vdec %p\r\n", mVsinkInst, mVdecInst);
        return -1;
    }

    mState = FLUSHING;
    if (!atc_vdec_flush(mVdecInst)) {
        PRINT_ERROR("atc_vdec_flush(%p) failed\r\n", mVdecInst);
        return -1;
    }
    changeStateIfWeOwnAllBuffers();
    first_render = 1;

    return 0;
}

int AVCodec::start()
{
    PRINT_INFO("enter\r\n");
    return postMessageSync(kMsgStart, NULL);
}

void AVCodec::stop()
{
    PRINT_INFO("AVCodec stop, mThread:%d, vdec:%p, vsink:%p\r\n",
                mThread, mVdecInst, mVsinkInst);
    if (0 != mThread)
    {
        postMessageAsync(kMsgStop, NULL);
        PRINT_INFO("Waiting on omx message thread exit\r\n");
        pthread_join(mThread, NULL);
        mThread = 0;
        PRINT_INFO("omx message thread exit\r\n");
    }
    if (mVdecInst)
    {
        atc_vdec_stop(mVdecInst);
    }
    if (mVsinkInst)
    {
        atc_video_sink_stop(mVsinkInst);
    }
    PRINT_INFO("now free input buffer header\r\n");
    for (int i = 0; i< kMaxCodecInBufferCnt; i++)
    {
        if (mInputBufs[i] != NULL) {
            delete mInputBufs[i];
            mInputBufs[i] = NULL;
        }
    }
    PRINT_INFO("now free output buffer header\r\n");
    for (int i = 0; i< kMaxCodecOutBufferCnt; i++)
    {
        if (mOutputBufs[i] != NULL) {
            delete mOutputBufs[i];
            mOutputBufs[i] = NULL;
        }
    }
    PRINT_INFO("exit\r\n");
}

void AVCodec::release()
{
    PRINT_INFO("AVCodec release, vdec:%p, vsink:%p\r\n",
                    mVdecInst, mVsinkInst);
    if (mVdecInst) {
        atc_vdec_close(mVdecInst);
        mVdecInst = NULL;
        PRINT_INFO("atc_vdec_close done\r\n");
    }

    if (mVsinkInst) {
        atc_video_sink_close(mVsinkInst);
        mVsinkInst = NULL;
        PRINT_INFO("atc_video_sink_close done\r\n");
    }
}

int AVCodec::flush()
{
    PRINT_INFO("enter\n");
    return postMessageSync(kMsgFlush, NULL);
}

void AVCodec::switchTS()
{
    first_render = 1;
}

void AVCodec::pause()
{
    ts->pause();
}

void AVCodec::onInputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer)
{
    if (mCallback != NULL)
        mCallback->onInputBufferAvailable(inst, codecBuffer);
}

void AVCodec::onOutputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer)
{
    if (mCallback != NULL)
        mCallback->onOutputBufferAvailable(inst, codecBuffer);
}

void AVCodec::changeStateIfWeOwnAllBuffers()
{
    PRINT_INFO("nputBufQ len:%d, mOutputBufQ len:%d\n",
           async_queue_length((AsyncQueue*)mInputBufQ), async_queue_length((AsyncQueue*)mOutputBufQ));
    if ((async_queue_length((AsyncQueue*)mInputBufQ) == kMaxCodecInBufferCnt) &&
        (async_queue_length((AsyncQueue*)mOutputBufQ) == kMaxCodecOutBufferCnt)) {
        PRINT_INFO("change state to FLUSHED\n");
        mState = FLUSHED;
    }
}

int AVCodec::queueInputBuffer(AVCodecBuffer *codecBuffer)
{
    PRINT_DEBUG("queueInputBuffer codecbuf:%p\n", codecBuffer);
    postMessageAsync(kMsgQueueInputBuffer, (void*)codecBuffer);
    return 0;
}

int AVCodec::releaseOutputBuffer(AVCodecBuffer *codecBuffer)
{
    postMessageAsync(kMsgReleaseOutputBuffer, (void*)codecBuffer);
    return 0;
}
AVCodecBuffer *AVCodec::dequeueInputBuffer()
{
    AVCodecBuffer *buf = (AVCodecBuffer *)async_queue_pop((AsyncQueue*)mInputBufQ);
    return buf;
}
AVCodecBuffer *AVCodec::dequeueOutputBuffer()
{
    AVCodecBuffer *buf = (AVCodecBuffer *)async_queue_pop((AsyncQueue*)mOutputBufQ);
    return buf;
}

AVCodec::AVCodec()
{
    mThread = 0;
    mVdecInst = NULL;
    mVsinkInst = NULL;
    mMsgQueue = NULL;
    mInputBufQ = NULL;
    mOutputBufQ = NULL;
    mCallback = NULL;
    mOutputSwapBuf = NULL;
    mState = UNINITIALIZED;
    ts = NULL;
    m_start = -1;
    mOutputBufferCounter = 0;
    first_render = 0;
    for (int i = 0; i< kMaxCodecInBufferCnt; i++) {
        mInputBufs[i] = NULL;
    }
    for (int i = 0; i< kMaxCodecOutBufferCnt; i++) {
        mOutputBufs[i] = NULL;
    }
    mMsgQueue = (void*)async_queue_new();
    if (mMsgQueue == NULL) {
        PRINT_ERROR("allocate MsgQueue failed.\n");
        return;
    }
    PRINT_INFO("[%p] allocate MsgQueue %p.\n", this, mMsgQueue);
    if (pthread_create(&mThread, 0, message_handler, this) < 0) {
        PRINT_ERROR("Error: Fail in creating omx message thread!\r\n");
    }

    pthread_cond_init(&mMsgCond, NULL);
    pthread_mutex_init(&mMsgMutex, NULL);
}
AVCodec::~AVCodec()
{
    if (mMsgQueue != NULL)
        async_queue_free((AsyncQueue *)mMsgQueue);
    if (mOutputSwapBuf != NULL)
        free(mOutputSwapBuf);
    if (NULL != ts) {
        delete ts;
        ts = NULL;
    }
}

int AVCodec::onQueueInputBuffer(AVCodecBuffer *codecBuffer)
{
    uint32_t flags = 0;
    if (codecBuffer == NULL)
        return -1;

    PRINT_DEBUG("onQueueInputBuffer, %p base:%p, size:%d, flags:0x%x, pts:%lld\r\n",
               codecBuffer, codecBuffer->base(), codecBuffer->size(), codecBuffer->flags(), codecBuffer->pts());
    if (codecBuffer->flags() & BUFFER_FLAG_EOS) {
        flags |= OMX_BUFFERFLAG_EOS;
    }
    atc_vdec_decode(mVdecInst, codecBuffer->data(), codecBuffer->size(), codecBuffer->pts(), flags);
    async_queue_push((AsyncQueue*)mInputBufQ, (void*)codecBuffer);
    return 0;
}

int AVCodec::onReleaseOutputBuffer(AVCodecBuffer *codecBuffer)
{
    void *pvOutbuf = NULL;
    __u32 OutBufLen = 0;
    VDEC_BUFFER_INFO_T rVdecOutInfo;
    if (codecBuffer == NULL) {
        PRINT_ERROR("invalid params, %p\r\n", codecBuffer);
        return -1;
    }
    if (codecBuffer->size() == 0) {
        async_queue_push((AsyncQueue*)mOutputBufQ, (void*)codecBuffer);
        return 0;
    }

    pvOutbuf = NULL;
    OutBufLen = 0;
    PRINT_DEBUG("atc_video_sink_dequeue_buffer\r\n");
    if (!atc_video_sink_dequeue_buffer(mVsinkInst, &pvOutbuf, &OutBufLen)) {
        PRINT_ERROR( "atc_video_sink_dequeue_buffer(%p) failed\r\n", mVsinkInst);
        return -1;
    }

    if (codecBuffer->capacity() != OutBufLen) {
        PRINT_ERROR( "invalid params %d VS %d\r\n",
            codecBuffer->capacity(), OutBufLen);
        return -1;
    }
    if (mOutputSwapBuf == NULL) {
        mOutputSwapBuf = malloc(OutBufLen);
        if (mOutputSwapBuf == NULL)
            return -1;
    }

    memcpy(mOutputSwapBuf, pvOutbuf, OutBufLen);
    memcpy (pvOutbuf, codecBuffer->data(), codecBuffer->size());
    memcpy(codecBuffer->base(), mOutputSwapBuf, OutBufLen);
    codecBuffer->setRange(0, OutBufLen);

    memset(&rVdecOutInfo, 0, sizeof(rVdecOutInfo));
    rVdecOutInfo.buffer = codecBuffer->base();
    rVdecOutInfo.bufSz = OutBufLen;

    PRINT_DEBUG("atc_vdec_release_output_buffer(%p, bufsz: %d)\r\n", pvOutbuf, OutBufLen);
    if (RET_ATCVDECINST_OK != atc_vdec_release_output_buffer(mVdecInst, &rVdecOutInfo)) {
        PRINT_ERROR("release_output_buffer failed, buf:%p len:%d\n", pvOutbuf, OutBufLen);
        atc_video_sink_cancel_buffer(mVsinkInst, pvOutbuf, OutBufLen);
        return -1;
    }

    ATC_VSINK_FMT_INFO_T rSinkformat;
    memset(&rSinkformat, 0, sizeof(rSinkformat));

    if (codecBuffer->flags() & BUFFER_FLAG_SIZECHANGE) {
      rSinkformat.format = ATC_PIX_FMT_NV12M_PRIVATE1;
      rSinkformat.width  = codecBuffer->width();
      rSinkformat.height = codecBuffer->height();
      rSinkformat.stride = (rSinkformat.width + 15) / 16 * 16;
      rSinkformat.interlaced = false;
      rSinkformat.fourcc = 0;

      PRINT_INFO("atc_video_sink_set_format %d x %d\r\n", rSinkformat.width, rSinkformat.height);
      if (!atc_video_sink_set_format(mVsinkInst, &rSinkformat)) {
          PRINT_ERROR("atc_video_sink_set_format failed\r\n");
          return -1;
      }
    }
    __u64 curpos = 0;
    if (NULL != ts) {
        if(first_render == 1) {
          first_render = 0;
          PRINT_INFO("start: %lldms\r\n", codecBuffer->pts()/1000);
          ts->setTimeUs(codecBuffer->pts());
          ts->start();
        }
        curpos = ts->getRealTimeUs();
    }
    if (m_start == -1) {
        m_start = codecBuffer->pts();
    }

    if (curpos < (__u64)(codecBuffer->pts() - m_start)) {
        __u64 delayus = codecBuffer->pts() - m_start - curpos;
        PRINT_INFO("m_start:%lldms, start: %lldms, curpos: %lldms\r\n",
                  m_start/1000, codecBuffer->pts()/1000, curpos/1000);
        usleep(delayus);
    }

    PRINT_DEBUG("atc_video_sink_queue_buffer(%p, sz: %d)\r\n", pvOutbuf, OutBufLen);
    if (!atc_video_sink_queue_buffer(mVsinkInst, pvOutbuf, OutBufLen, OutBufLen)) {
        PRINT_ERROR("queue_buffer(%p, dz: %d) failed\r\n", pvOutbuf, OutBufLen);
        return -1;
    }

    mOutputBufferCounter++;
    async_queue_push((AsyncQueue*)mOutputBufQ, (void*)codecBuffer);
    PRINT_DEBUG("atc_video_sink_queue_buffer exit, counter:%d\r\n", mOutputBufferCounter);
    return 0;
}

