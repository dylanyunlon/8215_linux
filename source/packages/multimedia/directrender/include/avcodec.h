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

#ifndef ATCOMX_UTILS_H
#define ATCOMX_UTILS_H

#include <stdbool.h>
#include <functional>
#include <linux/types.h>
#include <string.h>
#include <string>
#include "atcsurface.h"
#include "atcmediaclock.h"

struct AVCodec;

class AVCodecBuffer{
public:
    AVCodecBuffer(size_t capacity);
    AVCodecBuffer(uint8_t *base, size_t capacity);
    virtual ~AVCodecBuffer();
    uint8_t *base() { return (uint8_t *)mData; }
    uint8_t *data() { return (uint8_t *)mData + mRangeOffset; }
    size_t capacity() const { return mCapacity; }
    size_t size() const { return mRangeLength; }
    size_t offset() const { return mRangeOffset; }
    int64_t pts() const { return mPresentationTimeUs; }
    uint32_t flags() const { return mFlags; }
    uint32_t width() const { return mWidth; }
    uint32_t height() const { return mHeight; }
    void setBuffer(uint8_t *base, size_t capacity);
    void setRange(size_t offset, size_t size);
    void setBufferInfo(uint32_t w, uint32_t h, int64_t pts, uint32_t flags);

private:
    void *mData;
    bool mOwnsData;
    size_t mCapacity;
    size_t mRangeOffset;
    size_t mRangeLength;

    int64_t mPresentationTimeUs;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFlags;
};

struct BufferCallback
{
    std::function<void(AVCodec *inst, AVCodecBuffer *codecBuffer)> onInputBufferAvailable;
    std::function<void(AVCodec *inst, AVCodecBuffer *codecBuffer)> onOutputBufferAvailable;
    std::function<void(AVCodec *inst, int errorCode)> onError;
};

enum {
    kMsgStart,
    kMsgFlush,
    kMsgStop,
    kMsgQueueInputBuffer,
    kMsgReleaseOutputBuffer,
    kMsgEmptyBufferDone,
    kMsgFillBufferDone,
};

struct MsgInfo
{
    int type;
    bool processed;
    bool isSync;
    int err;
    struct {
      AVCodecBuffer *codecBuffer;
    } buffer_done;
};

enum BufferFlags {
    BUFFER_FLAG_SYNCFRAME     = 1,
    BUFFER_FLAG_CODECCONFIG   = 2,
    BUFFER_FLAG_EOS           = 4,
    BUFFER_FLAG_SIZECHANGE    = 8,
    BUFFER_FLAG_DISCARD       = 16,
};

class AVCodec
{
public:
static AVCodec *CreateByType(const std::string mime);
int configure(
        uint32_t width, uint32_t height,
        IAtcSurface *nativeWindow);
int setCallback(BufferCallback *callback);
int start();
int flush();
void stop();
void switchTS();
void pause();
void release();
int queueInputBuffer(AVCodecBuffer *codecBuffer);
int releaseOutputBuffer(AVCodecBuffer *codecBuffer);

AVCodecBuffer *dequeueInputBuffer();
AVCodecBuffer *dequeueOutputBuffer();

virtual ~AVCodec();
TimeSource         *ts;

int postMessageSync(int msgType, void *data);
int postMessageAsync(int msgType, void *data);

private:
    AVCodec();

    int onStart();
    int onFlush();
    int onQueueInputBuffer(AVCodecBuffer *codecBuffer);
    int onReleaseOutputBuffer(AVCodecBuffer *codecBuffer);
    void onInputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer);
    void onOutputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer);
    void changeStateIfWeOwnAllBuffers();
    friend void *message_handler(void *data);

    pthread_mutex_t mMsgMutex;
    pthread_cond_t mMsgCond;

    enum State {
        UNINITIALIZED,
        INITIALIZING,
        INITIALIZED,
        CONFIGURING,
        CONFIGURED,
        STARTING,
        STARTED,
        FLUSHING,
        FLUSHED,
        STOPPING,
        RELEASING,
    };

    enum {
        kMaxCodecInBufferCnt = 1,
        kMaxCodecOutBufferCnt = 5,
        kMaxCodecBufferSize = 100 * 4096,
    };

    pthread_t mThread;
    void *mVdecInst;
    void *mVsinkInst;
    void *mMsgQueue;
    void *mInputBufQ; //buf slot
    void *mOutputSwapBuf;
    AVCodecBuffer *mInputBufs[kMaxCodecInBufferCnt];
    void *mOutputBufQ;
    AVCodecBuffer *mOutputBufs[kMaxCodecOutBufferCnt];
    BufferCallback *mCallback;
    State mState;
    __s64 m_start;
    int first_render;
    int64_t mOutputBufferCounter;  // number of output buffers queued since last reset/flush
};
#endif /* ATCOMX_UTILS_H */
