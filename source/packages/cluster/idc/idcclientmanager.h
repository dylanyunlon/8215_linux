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
 * AutoChips Inc. (C) 2023. All rights reserved.
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

#ifndef ATC_IDCCLIENTMANAGER
#define ATC_IDCCLIENTMANAGER

#include "idcmediadata.h"
#include "idc_client.h"
#include <mutex>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/un.h>
#include "clog.h"

#define ARG1 "arg1"
#define ARG2 "arg2"
#define ARG3 "arg3"
#define ARG4 "arg4"
#define ARG5 "arg5"
#define ARG6 "arg6"
#define ARG7 "arg7"
#define ARG8 "arg8"

using namespace std;
using namespace clusteridcdata;

namespace clusteridcclient {

class IDCEventCallbackImpl : public IIDCEventCallback
{
public:

    enum {
        IDC_CB_TYPE_EVENT,
        IDC_CB_TYPE_DMA_BUFFER,
        IDC_CB_TYPE_MESSAGE,
        IDC_CB_TYPE_GFX_DMA_BUFFER
    };

    IDCEventCallbackImpl(int type) {
        m_type = type;
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_cond, NULL);
    }

    ~IDCEventCallbackImpl(void) {
        pthread_cond_destroy(&m_cond);
    }

    void onResult(void *extra, uint64_t result, void *reply) {
        printf("[idc] %s -> extra: %p, result: %d\n", __func__, extra, (int)result);
        pthread_mutex_lock(&m_mutex);
        pthread_cond_signal(&m_cond);
        pthread_mutex_unlock(&m_mutex);

	    if (m_type == IDC_CB_TYPE_MESSAGE) {
            IDCMessage *reply_msg = (IDCMessage *)reply;
            printf("[idc] %s -> reply msg what: %d\n", __func__, reply_msg->m_msg.what);
        }
    }

    void wait(void) {
        pthread_mutex_lock(&m_mutex);
        pthread_cond_wait(&m_cond, &m_mutex);
        pthread_mutex_unlock(&m_mutex);
    }

private:
    int m_type;
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
};


class AtcIdcClientManager
{
public:
    AtcIdcClientManager(int appID = 1);
    virtual ~AtcIdcClientManager();

    enum
    {
        MEDIA_STATE_STOPPED= 0x00,
        MEDIA_STATE_PLAYING,
        MEDIA_STATE_PAUSED,
        MEDIA_STATE_NEXT_PLAYING,
        MEDIA_STATE_PREVIOUS_PLAYING,
    };

    enum {
        IDC_CB_TYPE_EVENT,
        IDC_CB_TYPE_DMA_BUFFER,
        IDC_CB_TYPE_MESSAGE,
        IDC_CB_TYPE_GFX_DMA_BUFFER
    };

    enum {
        Incoming = 1,
        Dialing,
        Calling,
        Idle,
    };

    enum
    {
        SEND_SONG_NAME = 0x01,
        SEND_PLAY_STATE,
        SEND_MEDIA_STATE,
        SEND_PHONE_NUM,
        SEND_CONTACT_NAME,
        SEND_PHONE_TIME,
        SEND_PHONE_STATE,
        SEND_CONTACT_IMAGE,
        SEND_ALBUM_IMAGE,

        SEND_PLAYBACK_STATE,
        SEND_MEDIA_METADATA,
        SEND_ROUTE_GUIDANCE_STATE,
        SEND_ROUTE_GUIDANCE_INFORMATION,
    };

    enum
    {
        BT_APP = 1,
        AUDIOPLAYER,
    };

    void sendPlaybackState(int id, int mediaId,int state);
    void sendMediaMetadata(int id, AtcClusterMediaMetadata *data);
    int sendPhoneNum(int id, string phoneNum);
    int sendContactName(int id, string name);
    int sendPhoneState(int id, int state);

protected:

private:
    int allocateAndFillIonBuffer(const void *data, size_t size, int *ionFd);
    IInterdomainChannel *m_channel;
    int client_fd;
};

}  //namespace clusteridcclient

#endif  // ATC_IDCCLIENTMANAGER

