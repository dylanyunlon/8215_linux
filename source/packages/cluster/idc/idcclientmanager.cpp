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
#include "idcclientmanager.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>

#define TAG_ATC_CLUSTER "AtcIdcClientManager"

namespace clusteridcclient {

const static char *TAG = "AtcIdcClientManager";
AtcIdcClientManager::AtcIdcClientManager(int appID)
{
    char *channel_name = (char *)"cluster.transport";
    char *domain_name = (char *)IDC_CLUSTER_DOMAIN_NAME;

    m_channel = IInterdomainChannel::get();
    if (m_channel) {
        m_channel->setName(channel_name);
        m_channel->connect(domain_name);
    } else {
        UTILS_LOGE(TAG,"[idc] IInterdomainChannel::get -> failed, channel is NULL\n");
    }

    if (appID == 2) {
        // create Socket
        client_fd = m_channel->createClusterConnection();
        if (client_fd == -1) {
            UTILS_LOGE(TAG,"Socket creation failed");
        }
    }
}

AtcIdcClientManager::~AtcIdcClientManager()
{

}

void AtcIdcClientManager::sendPlaybackState(int id,int mediaId, int state)
{
    (void)id; //unused warning

    IDCMessage msg;
    msg.m_msg.what = SEND_PLAYBACK_STATE;
    msg.putExtra(ARG1, state);
    msg.putExtra(ARG2, mediaId);

    idc_event_t  event;
    event.id = IDC_EVENT_MESSAGE;
    event.param1 = 0xFAFAFAFA;
    event.param2 = (uint64_t)&msg;

    printf("sendPlaybackState, state:%d\n", state);

    IDCEventCallbackImpl msg_cb(IDC_CB_TYPE_MESSAGE);
    if (!m_channel->postEvent(&event, &msg_cb, NULL)) {
        printf("[idc] channel(%p) postEvent IDC_EVENT_MESSAGE failed\n", m_channel);
    }

    return;
}

void AtcIdcClientManager::sendMediaMetadata(int id, AtcClusterMediaMetadata *data)
{
    (void)id; //unused warning
    IDCMessage msg;
    int ionFd;
    msg.m_msg.what = SEND_MEDIA_METADATA;
    msg.putExtra(ARG1, data->getMediaId());
    msg.putExtra(ARG2, data->getTitle());
    msg.putExtra(ARG3, data->getArtist());
    msg.putExtra(ARG4, data->getAction());
    //msg.putExtra(ARG5, data->getBitmap().data(), data->getBitmap().size());

    unsigned char *picBuf = NULL;
    unsigned int picSize = 0;
    std::vector<uint8_t> data2(picBuf, picBuf + picSize);
    msg.putExtra(ARG5, data2.data(), 0);
    idc_event_t  event;
    event.id = IDC_EVENT_MESSAGE;
    event.param1 = 0xFAFAFAFA;
    event.param2 = (uint64_t)&msg;

    printf("sendMediaMetadata, mediaId:%s, title:%s, artist:%s\n",
            data->getMediaId().c_str(), data->getTitle().c_str(), data->getArtist().c_str());

    IDCEventCallbackImpl msg_cb(IDC_CB_TYPE_MESSAGE);
    if (!m_channel->postEvent(&event, &msg_cb, NULL)) {
        printf("[idc] channel(%p) postEvent IDC_EVENT_MESSAGE failed\n", m_channel);
    }

    if (data->getBitmap().size()> 0 &&  m_channel->allocateAndFillIonBuffer(data->getBitmap().data(), data->getBitmap().size(), &ionFd) == 0) {
        UTILS_LOGI(TAG, "send Ion buffer fd ionFd =%d",ionFd);
        m_channel->ancil_send_fds_with_buffer(client_fd, &ionFd, 1);
        close(ionFd);
    }
    return;
}

int AtcIdcClientManager::sendPhoneNum(int id, string phoneNum)
{
    printf("sendPhoneNum, id:%d, phoneNum:%s\n", id, phoneNum.c_str());

    IDCMessage msg;
    msg.m_msg.what = SEND_PHONE_NUM;
    msg.putExtra(ARG1, phoneNum);

    idc_event_t  event;
    event.id = IDC_EVENT_MESSAGE;
    event.param1 = 0xFAFAFAFA;
    event.param2 = (uint64_t)&msg;
    if (m_channel && !m_channel->postEvent(&event, NULL, NULL)) {
        printf("[idc] [sendPhoneNum] channel(%p) postEvent IDC_EVENT_MESSAGE failed\n", m_channel);
        return -1;
    }
    return 0;
}

int AtcIdcClientManager::sendContactName(int id, string name)
{
    printf("sendContactName, id:%d, name:%s\n", id, name.c_str());

    IDCMessage msg;
    msg.m_msg.what = SEND_CONTACT_NAME;
    msg.putExtra(ARG1, name);

    idc_event_t  event;
    event.id = IDC_EVENT_MESSAGE;
    event.param1 = 0xFAFAFAFA;
    event.param2 = (uint64_t)&msg;
    if (m_channel && !m_channel->postEvent(&event, NULL, NULL)) {
        printf("[idc] [sendContactName] channel(%p) postEvent IDC_EVENT_MESSAGE failed\n", m_channel);
        return -1;
    }
    return 0;
}

int AtcIdcClientManager::sendPhoneState(int id, int state)
{
    printf("sendPhoneState, id:%d, state:%d\n", id, state);

    IDCMessage msg;
    msg.m_msg.what = SEND_PHONE_STATE;
    msg.putExtra(ARG1, state);

    idc_event_t  event;
    event.id = IDC_EVENT_MESSAGE;
    event.param1 = 0xFAFAFAFA;
    event.param2 = (uint64_t)&msg;
    if (m_channel && !m_channel->postEvent(&event, NULL, NULL)) {
        printf("[idc] [sendPhoneState] channel(%p) postEvent IDC_EVENT_MESSAGE failed\n", m_channel);
        return -1;
    }
    return 0;
}

}  //clusteridcclient

