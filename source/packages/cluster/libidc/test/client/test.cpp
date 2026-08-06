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
#include <stdio.h>
#include <idc_client.h>
//#include <ism.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <pthread.h>

#include "idc_info.h"
#include <drm/drm_fourcc.h>
#include <string>


#define ARG1 "arg1"
#define ARG2 "arg2"
#define ARG3 "arg3"
#define ARG4 "arg4"
#define ARG5 "arg5"
#define ARG6 "arg6"
#define ARG7 "arg7"
#define ARG8 "arg8"
//#include <ui/Fence.h>

using namespace std;

enum {
    IDC_CB_TYPE_EVENT,
    IDC_CB_TYPE_DMA_BUFFER,
    IDC_CB_TYPE_MESSAGE,
    IDC_CB_TYPE_GFX_DMA_BUFFER
};

enum
{
	RECEIVE_SONG_NAME = 0x01,
	RECEIVE_PLAY_STATE,
	RECEIVE_MEDIA_STATE,
	RECEIVE_PHONE_NUM,
	RECEIVE_CONTACT_NAME,
	RECEIVE_PHONE_TIME,
	RECEIVE_PHONE_STATE,
	RECEIVE_CONTACT_IMAGE,
	RECEIVE_ALBUM_IMAGE,
	RECEIVE_PLAYBACK_STATE,
	RECEIVE_MEDIA_METADATA,
	RECEIVE_ROUTE_GUIDANCE_STATE,
	RECEIVE_ROUTE_GUIDANCE_INFORMATION,
};
enum CallStatus {
	Incoming = 1,     //来电
	Dialing,          //拨打中
	Calling,          //通话中
	Idle,             //空闲,挂断
};

enum
{
	MEDIA_STATE_STOPPED= 0x00,
	MEDIA_STATE_PLAYING,
	MEDIA_STATE_PAUSED,
	MEDIA_STATE_NEXT_PLAYING,
	MEDIA_STATE_PREVIOUS_PLAYING,
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
	//=============
	SEND_PLAYBACK_STATE,
	SEND_MEDIA_METADATA,
	SEND_ROUTE_GUIDANCE_STATE,
	SEND_ROUTE_GUIDANCE_INFORMATION,
};

class IDCEventCallbackImpl : public IIDCEventCallback {
public:
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
            int avm1 = reply_msg->getIntExtra("avm1", 1234);
            printf("[idc] %s -> avm1: %d\n", __func__, avm1);

            int avm2 = reply_msg->getIntExtra("avm2", 1234);
            printf("[idc] %s -> avm2: %d\n", __func__, avm2);
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

int main(int argc, char **argv) {
    char *channel_name = (char *)"cluster.transport";
    char *domain_name = (char *)IDC_CLUSTER_DOMAIN_NAME;
    int i = 1;

    printf("[idc] idc_test -> argc: %d\n", argc);
    while (i < argc) {
        if (0 == strncmp(argv[i], "--channel-name=", 15)) {
	    channel_name = argv[i] + 15;
        } else if (0 == strncmp(argv[i], "--domain-name=", 14)) {
	    domain_name = argv[i] + 14;
	}
	i++;
    }

    printf("[idc] channel name: %s, domain name: %s\n", channel_name, domain_name);

    IInterdomainChannel *channel = IInterdomainChannel::get();
    if (!channel) {
        printf("[idc] IInterdomainChannel::get -> failed, channel is NULL\n");
        return (-1);
    }
    channel->setName(channel_name);
    channel->connect(domain_name);
    idc_event_t  event;
    char idc_data[100];
#if 0
    int ism_fd = ism_open();
    int shm_fd = -1;

    if (0 != ism_alloc(ism_fd, 4096, 0, &shm_fd)) {
        printf("[idc] ism_alloc failed\n");
        shm_fd = -1;
    } else {
        printf("[idc] ism_alloc success\n");
    }
    idc_buffer_t  buf;

    buf.fd = shm_fd;
    buf.size = 4096;

    uint32_t *data_buf = (uint32_t *)mmap(NULL, buf.size, PROT_READ | PROT_WRITE,
                                          MAP_SHARED, shm_fd, 0);
    printf("[idc] %s -> data_buf: %p\n", __func__, data_buf);
    if (data_buf) {
        for (int i = 0; i < 4096/4; i++) {
            *(data_buf + i) = 0xfefefefe;
        }
        munmap(data_buf, buf.size);
    }

    if (!channel->postBuffer(&buf, NULL, NULL)) {
        printf("[idc] channel(%p) postBuffers failed\n", channel);
        //return (-1);
    }
    if (shm_fd >= 0) {
        close(shm_fd);
    }
    if (buf.fence >= 0) {
        close(buf.fence);
    }

    idc_event_t  event;
    strcpy(idc_data, "[idc] IVI -> cluster message: This is a test");

    event.id = IDC_EVENT_RAW_DATA;
    event.param1 = (uint64_t)idc_data;
    event.param2 = strlen(idc_data) + 1;
    if (!channel->postEvent(&event, NULL, NULL)) {
        printf("[idc] channel(%p) postEvent IDC_EVENT_RAW_DATA failed\n", channel);
        return (-1);
    }

    IDCInfo info;

    info.setName("Jackey");
    info.setCity("Hefei");
    info.setPhone("13856045404");
    info.setTemperature(36);
    info.setAge(40);

    IDCParcel parcel;

    parcel.writeParcelable(info);

    event.id = IDC_EVENT_PARCEL;
    event.param1 = 0xFEFEFEFE;
    event.param2 = (uint64_t)&parcel;
    if (!channel->postEvent(&event, NULL, NULL)) {
        printf("[idc] channel(%p) postEvent IDC_EVENT_PARCEL failed\n", channel);
    }

    info.setName("Jiechen");
    info.setCity("Hefei");
    info.setPhone("18815519365");
    info.setTemperature(38);
    info.setAge(44);

    IDCParcel parcel2;

    event.param2 = (uint64_t)&parcel2;
    parcel2.writeParcelable(info);

    IDCEventCallbackImpl cb(IDC_CB_TYPE_EVENT);

    if (!channel->postEvent(&event, &cb, channel)) {
        printf("[idc] channel(%p) postEvent IDC_EVENT_PARCEL with callback failed\n", channel);
    }

    IDCEventCallbackImpl cb2(IDC_CB_TYPE_DMA_BUFFER);

    if (0 != ism_alloc(ism_fd, 4096, 0, &shm_fd)) {
        printf("[idc] ism_alloc failed\n");
        shm_fd = -1;
    } else {
        printf("[idc] ism_alloc success\n");
    }
    buf.fd = shm_fd;
    buf.size = 4096;

    if (!channel->postBuffer(&buf, &cb2, channel)) {
        printf("[idc] channel(%p) postBuffer with callback failed\n", channel);
    } else {
        printf("[idc] channel(%p) postBuffer with callback wait ++++++++++\n", channel);
        cb2.wait();
        printf("[idc] channel(%p) postBuffer with callback wait ----------\n", channel);
    }
    close(shm_fd);
    if (buf.fence >= 0) {
        close(buf.fence);
    }

    IDCEventCallbackImpl gfx_buf_cb(IDC_CB_TYPE_GFX_DMA_BUFFER);

    idc_gfx_buffer_t gfx_buf;
    uint32_t buf_sz = 1024*600*4;
    shm_fd = -1;

    if (0 != ism_alloc(ism_fd, buf_sz, 0, &shm_fd)) {
        printf("[idc] %s -> ism_alloc 1024x600*4 failed, line: %d\n", __func__, __LINE__);
    } else {
        printf("[idc] %s -> ism_alloc 1024x600*4 success, line: %d\n", __func__, __LINE__);

        data_buf = (uint32_t *)mmap(NULL, buf_sz, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        printf("[idc] %s -> data_buf: %p\n", __func__, data_buf);
        if (data_buf) {
            for (int i = 0; i < 1024*600; i++) {
                *(data_buf + i) = 0xff0000ff;
            }
            munmap(data_buf, buf.size);
        }
    }

    gfx_buf.width = 1024;
    gfx_buf.height = 600;
    gfx_buf.stride = 1024*4;
    gfx_buf.format = DRM_FORMAT_ARGB8888;
    gfx_buf.fd = shm_fd;
    if (!channel->postGfxBuffer(&gfx_buf, &gfx_buf_cb, channel)) {
        printf("[idc] channel(%p) postGfxBuffer with callback failed\n", channel);
    }
    close(shm_fd);
    if (gfx_buf.fence >= 0) {
        status_t status;
        sp<Fence> releaseFence = new Fence(gfx_buf.fence);
        status = releaseFence->wait(5000);
        if (NO_ERROR == status) {
	    printf("[idc] %s -> fence signaled\n", __func__);
	} else {
	    printf("[idc] %s -> timeout\n", __func__);
	}
    }

    //gfx_buf.format = DRM_FORMAT_NV12;
    //buf_sz = 1024*600*1.5;

    if (0 != ism_alloc(ism_fd, buf_sz, 0, &shm_fd)) {
        printf("[idc] ism_alloc failed\n");
        shm_fd = -1;
    } else {
        printf("[idc] ism_alloc success\n");
    }

    int data_fd = open("/sdcard/1024x600_NV12.bin", O_RDWR | O_CLOEXEC);
    if (data_fd >= 0) {
        data_buf = (uint32_t *)mmap(NULL, buf_sz, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, shm_fd, 0);
        if (data_buf) {
            read(data_fd, data_buf, buf_sz);
            munmap(data_buf, buf_sz);
        }
        close(data_fd);
        printf("[idc] %s -> data fd: %d, data buf: %p\n", __func__, data_fd, data_buf);
    } else {
        printf("[idc] %s -> open /sdcard/1024x600_NV12.bin failed!\n", __func__);
    }
    gfx_buf.fd = shm_fd;
    if (!channel->postGfxBuffer(&gfx_buf, &gfx_buf_cb, channel)) {
        printf("[idc] channel(%p) postGfxBuffer with callback failed\n", channel);
    }
    close(shm_fd);
    if (gfx_buf.fence >= 0) {
        close(gfx_buf.fence);
    }
#endif
    IDCEventCallbackImpl msg_cb(IDC_CB_TYPE_MESSAGE);
    uint8_t *data = (uint8_t *)malloc(100*100*4);
    memset(data, 255, 100*100*4);


    IDCMessage msg3(12345);
    uint8_t *data2 = (uint8_t *)malloc(100*100*4);
    memset(data2, 255, 100*100*4);

    msg3.m_msg.what = SEND_MEDIA_METADATA;
    msg3.putExtra(ARG2,  string("可惜不是你"));
    msg3.putExtra(ARG5, data, 100*100*4);

    idc_event_t  event3;
    event3.id = IDC_EVENT_MESSAGE;
    event3.param1 = 0xFAFAFAFA;
    event3.param2 = (uint64_t)&msg3;
    if (!channel->postEvent(&event3, &msg_cb, NULL)) {
         printf("[idc] channel(%p) postEvent IDC_EVENT_MESSAGE failed\n", channel);
    }
 usleep(5000000);
/**
 	IDCMessage msg13(12345);
    msg13.m_msg.what = SEND_PLAYBACK_STATE;
    msg13.putExtra(ARG1, MEDIA_STATE_STOPPED);

    idc_event_t  event13;
    event13.id = IDC_EVENT_MESSAGE;
    event13.param1 = 0xFAFAFAFA;
    event13.param2 = (uint64_t)&msg13;
    if (!channel->postEvent(&event13, &msg_cb, NULL)) {
         printf("[idc] channel(%p) postEvent IDC_EVENT_MESSAGE failed\n", channel);
    }

 usleep(5000000);
 **/
 	IDCMessage msg4(12345);
idc_event_t  event4;
    msg4.m_msg.what = SEND_PHONE_STATE;
    msg4.putExtra(ARG1, 1);

    event4.id = IDC_EVENT_MESSAGE;
    event4.param1 = 0xFAFAFAFA;
    event4.param2 = (uint64_t)&msg4;
    if (!channel->postEvent(&event4, &msg_cb, NULL)) {
        printf("[%s] size(%d) postEvent IDC_EVENT_MESSAGE failed\n", __func__, msg4.size());
    }
idc_event_t  event5;
    usleep(2000000);
	IDCMessage msg5(12345);
    msg5.m_msg.what = SEND_CONTACT_IMAGE;

    msg5.putExtra(ARG1, data, 100*100*4);

    event5.id = IDC_EVENT_MESSAGE;
    event5.param1 = 0xFAFAFAFA;
    event5.param2 = (uint64_t)&msg5;
    if (!channel->postEvent(&event5, &msg_cb, NULL)) {
        printf( "[%s] size(%d) postEvent IDC_EVENT_MESSAGE failed\n", __func__, msg5.size());
    }

	usleep(2000000);
	IDCMessage msg6(12345);
idc_event_t  event6;
    msg6.m_msg.what = SEND_CONTACT_NAME;
    msg6.putExtra(ARG1,  string("guohua"));

    event6.id = IDC_EVENT_MESSAGE;
    event6.param1 = 0xFAFAFAFA;
    event6.param2 = (uint64_t)&msg6;
    if (!channel->postEvent(&event6, &msg_cb, NULL)) {
        printf("[%s] size(%d) postEvent IDC_EVENT_MESSAGE failed\n", __func__, msg6.size());
    }

	usleep(2000000);
    idc_event_t  event7;
	IDCMessage msg7(12345);
    msg7.m_msg.what = SEND_PHONE_NUM;
    msg7.putExtra(ARG1, string("18121212121"));

    event7.id = IDC_EVENT_MESSAGE;
    event7.param1 = 0xFAFAFAFA;
    event7.param2 = (uint64_t)&msg7;
    if (!channel->postEvent(&event7, &msg_cb, NULL)) {
        printf("[%s] size(%d) postEvent IDC_EVENT_MESSAGE failed\n", __func__, msg7.size());
    }
	
	usleep(2000000);
    idc_event_t  event9;
	IDCMessage msg9(12345);
    msg9.m_msg.what = SEND_PHONE_STATE;
    msg9.putExtra(ARG1, Incoming);

    event9.id = IDC_EVENT_MESSAGE;
    event9.param1 = 0xFAFAFAFA;
    event9.param2 = (uint64_t)&msg9;
    if (!channel->postEvent(&event9, &msg_cb, NULL)) {
        printf("[%s] size(%d) postEvent IDC_EVENT_MESSAGE failed\n", __func__, msg9.size());
    }

	usleep(2000000);
    idc_event_t  event11;
	IDCMessage msg11(12345);
    msg11.m_msg.what = SEND_PHONE_STATE;
    msg11.putExtra(ARG1, Dialing);

    event11.id = IDC_EVENT_MESSAGE;
    event11.param1 = 0xFAFAFAFA;
    event11.param2 = (uint64_t)&msg11;
    if (!channel->postEvent(&event11, &msg_cb, NULL)) {
        printf("[%s] size(%d) postEvent IDC_EVENT_MESSAGE failed\n", __func__, msg11.size());
    }
	usleep(9000000);
    idc_event_t  event10;
	IDCMessage msg10(12345);
    msg10.m_msg.what = SEND_PHONE_STATE;
    msg10.putExtra(ARG1, Calling);

    event10.id = IDC_EVENT_MESSAGE;
    event10.param1 = 0xFAFAFAFA;
    event10.param2 = (uint64_t)&msg10;
    if (!channel->postEvent(&event10, &msg_cb, NULL)) {
        printf("[%s] size(%d) postEvent IDC_EVENT_MESSAGE failed\n", __func__, msg10.size());
    }

	usleep(80000000);
    idc_event_t  event8;
	IDCMessage msg8(12345);
    msg8.m_msg.what = SEND_PHONE_STATE;
    msg8.putExtra(ARG1, Idle);

    event8.id = IDC_EVENT_MESSAGE;
    event8.param1 = 0xFAFAFAFA;
    event8.param2 = (uint64_t)&msg8;
    if (!channel->postEvent(&event8, &msg_cb, NULL)) {
        printf("[%s] size(%d) postEvent IDC_EVENT_MESSAGE failed\n", __func__, msg8.size());
    }

    free(data);
    while (1) {
        usleep(1000);
    }
    channel->disconnect();

    return (0);
}
