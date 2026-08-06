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
#include <idc_server.h>

#include <unistd.h>
#include <sys/mman.h>

#include <stdlib.h>
#include <fcntl.h>

#include "idc_info.h"

#include <drm/drm_fourcc.h>
#include <atcsurface.h>
#include <drmBuffer.h>
#include <ATCIDCMessagerServiceImp.hpp>

static IAtcSurface *g_overlay = NULL;

class IDCMonitorListenerImpl : public IIDCMonitorListener {
public:
    IDCMonitorListenerImpl(void) {
    }

    ~IDCMonitorListenerImpl(void) {
    }

    void onEvent(const char *domain, const char *channel, idc_event_t *event);
};

class IDCProxyListenerImpl : public IIDCProxyListener {
public:
    IDCProxyListenerImpl(void) {
    }

    ~IDCProxyListenerImpl(void) {
    }

    void onEvent(IInterdomainChannelProxy *proxy, idc_event_t *event, void *reply);
};

void IDCMonitorListenerImpl::onEvent(const char *domain, const char *channel,
                                     idc_event_t *event) {
    printf("[jackey] server test app %s -> domain: %s, channel: %s, event id: %d\n",
           __func__, domain, channel, (int)event->id);
}


void IDCProxyListenerImpl::onEvent(IInterdomainChannelProxy *proxy,
                                   idc_event_t *event, void *reply) {
    //printf("[jackey] %s -> event: %u\n", __func__, event->id);
    if (IDC_EVENT_CONNECTED == event->id) {
        printf("[idc] %s -> handle IDC_EVENT_CONNECTED\n", __func__);
        IAtcSurface_show(g_overlay);
    } else if (IDC_EVENT_DISCONNECTED == event->id) {
        printf("[idc] %s -> handle IDC_EVENT_DISCONNECTED\n", __func__);
        IAtcSurface_hide(g_overlay);
    } else if (IDC_EVENT_DMA_BUFFER == event->id) {
        idc_buffer_t *buf = (idc_buffer_t *)event->param1;

        printf("[jackey] %s -> dma buffer event, fd: %d, size: %u\n",
               __func__, (int)buf->fd, (unsigned int)buf->size);

        if (buf->fd >= 0) {
            uint32_t *data_buf = (uint32_t *)mmap(NULL, buf->size, PROT_READ,
                                                  MAP_SHARED, buf->fd, 0);
            printf("[jackey] %s -> data buf: %p\n", __func__, data_buf);
            for (int i = 0; i < (int)buf->size/4; i++) {
                if (i < 25) {
                    printf("[jackey] %s -> data[%d] = %x\n", __func__, i, data_buf[i]);
                }
            }
            munmap(data_buf, buf->size);
        }
    } else if (IDC_EVENT_GFX_DMA_BUFFER == event->id) {
        idc_gfx_buffer_t *gfx_buf = (idc_gfx_buffer_t *)event->param1;

#if 0
        printf("[jackey] %s -> gfx buffer event, fd: %d, width: %u, height: %u\n",
               __func__, (int)gfx_buf->fd, gfx_buf->width, gfx_buf->height);
#endif

        atc_buffer_t buf;
        atc_overlay_buffer_t *param;
        int ret;

	if ((gfx_buf->width != 1024) || (gfx_buf->height != 600)) {
            IAtcSurface_setBuffersSize(g_overlay, gfx_buf->width, gfx_buf->height);
            IAtcSurface_setBuffersFormat(g_overlay, gfx_buf->format, 0);
        }

#if 0
        int size = gfx_buf->width * gfx_buf->height * 4;

	if (gfx_buf->format == DRM_FORMAT_NV12) {
            size = gfx_buf->width * gfx_buf->height * 1.5;
        }
        uint32_t *data_buf = (uint32_t *)mmap(NULL, size, PROT_READ,
                                              MAP_SHARED, gfx_buf->fd, 0);
        if (MAP_FAILED == data_buf) {
            printf("[idc_server_test] %s -> mmap failed, line: %d\n", __func__, __LINE__);
        }
        printf("[jackey] %s -> data buf: %p\n", __func__, data_buf);
        for (int i = 0; i < (int)1024; i++) {
            if (i < 25) {
                printf("[jackey] %s -> data[%d] = %x\n", __func__, i, data_buf[i]);
            }
        }
        munmap(data_buf, size);
#endif

        ret = IAtcSurface_dequeueBuffer(g_overlay, &buf);
        if (0 > ret) {
            printf("[idc] %s -> Failed to IAtcSurface_dequeueBuffer\n", __func__);
        }
        param = (atc_overlay_buffer_t *)buf.bits;
        param->fd = gfx_buf->fd;
        param->stride = gfx_buf->stride;
        buf.width = gfx_buf->width;
	buf.height = gfx_buf->height;
        ret = IAtcSurface_queueBuffer(g_overlay, &buf);
        if (0 > ret) {
            printf("[idc] %s -> Failed to IAtcSurface_queueBuffer\n", __func__);
        }
    } else if (IDC_EVENT_RAW_DATA == event->id) {
        char *data = (char *)event->param1;
        uint32_t size = (uint32_t)event->param2;

        data[size - 1] = 0;
        printf("[jackey] %s -> raw data: %s, size: %d\n", __func__, data, (int)size);
    } else if (IDC_EVENT_PARCEL == event->id) {
        printf("[jackey] %s -> received parcel event and data id: %x\n",
               __func__, (uint32_t)(event->param1));
        IDCParcel *parcel = (IDCParcel *)event->param2;

        IDCInfo info;

        parcel->readParcelable(&info);

        char name[20], phone[20], city[20];

        info.getName(name);
        info.getPhone(phone);
        info.getCity(city);
        printf("[jackey] %s -> name: %s, phone: %s, city: %s, temperature: %d, age: %d\n",
               __func__, name, phone, city, (int)info.getTemperature(), (int)info.getAge());
    } else if (IDC_EVENT_MESSAGE == event->id) {
        printf("[idc] %s -> received idc message event!\n", __func__);
        IDCMessage *idc_msg = (IDCMessage *)event->param2;
        IDCMessage *idc_reply_msg = (IDCMessage *)reply;

        printf("[idc] %s -> msg: %p, what: %d\n", __func__, idc_msg, idc_msg->m_msg.what);
        int avm1 = idc_msg->getIntExtra("avm1", 8888);
        printf("[idc] %s -> avm1: %d\n", __func__, avm1);

        int avm2 = idc_msg->getIntExtra("avm2", 9999);
        printf("[idc] %s -> avm2: %d\n", __func__, avm2);

        std::string avm3 = idc_msg->getStringExtra("avm3", "kkkkkk");
        printf("[idc] %s -> avm3: %s\n", __func__, avm3.c_str());

        bool enable = idc_msg->getBoolExtra("avm4", false);
        if (enable) {
            printf("[idc] %s -> avm4: true\n", __func__);
        } else {
            printf("[idc] %s -> avm4: false\n", __func__);
        }

        unsigned int data_len;
        const unsigned char *data = idc_msg->getArrayExtra("avm5", &data_len, NULL);
        printf("[idc] %s -> data: %p, data len: %d\n", __func__, data, (int)data_len);

        idc_reply_msg->putExtra("avm1", 4444);
        idc_reply_msg->putExtra("avm2", 6666);
    }
}

int main(int argc, char **argv) {

//listener common api
    std::shared_ptr<CommonAPI::Runtime> runtime = CommonAPI::Runtime::get();
    std::shared_ptr<atclibidc::ATCIDCMessagerServiceImp> myService = std::make_shared<atclibidc::ATCIDCMessagerServiceImp>();
    if (runtime == NULL) {
       // ATC_STREAM_LOGE() <<"CommonAPI::Runtime::get fail\r\n";
        return 1;
    }

    if (myService == NULL) {
        //ATC_STREAM_LOGE() << "std::make_shared<DeviceManagerStubImpl> fail!\r\n";
        return 1;
    }
    
    runtime->registerService("local", "atclibsidc", myService);
	
    IInterdomainChannelMonitor *idc_monitor = NULL;
    IInterdomainChannelProxy *channel = NULL;
    char *channel_name = (char *)"idc.test";
    int i = 1;

    printf("[idc] idc_server_test -> argc: %d\n", argc);
    while (i < argc) {
        if (0 == strncmp(argv[i], "--channel-name=", 15)) {
            channel_name = argv[i] + 15;
        }
        i++;
    }

    idc_monitor = IInterdomainChannelMonitor::get();
    if (!idc_monitor) {
        return (-1);
    }

    IDCMonitorListenerImpl monitor_listener;
    idc_monitor->addListener(&monitor_listener);

    //g_overlay = atc_createsurface(ATCSURF_TYPE_DEFAULT, 1024, 600, ATC_PIX_FMT_NV12);
    g_overlay = atc_createsurface(ATCSURF_TYPE_DEFAULT, 1024, 600, DRM_FORMAT_RGBA8888);
    if (!g_overlay) {
        printf("[idc] Failed to atc_createsurface\n");
    } else {
        IAtcSurface_setBufferCount(g_overlay, 2);
        IAtcSurface_setWindow(g_overlay, 100, 100, 600, 400);
        IAtcSurface_setLayerZOrder(g_overlay, 82);
        IAtcSurface_show(g_overlay);
    }

lbRetry:
    channel = IInterdomainChannelProxy::get(channel_name);
    if (!channel) {
        printf("[jackey] Failed to get map.projection channel proxy!\n");
        sleep(3);
        goto lbRetry;
        return (-1);
    }
    IDCProxyListenerImpl listener;

    char name[100];

    channel->getName(name);
    printf("[idc] %s -> channel name: %s\n", __func__, name);

    channel->addListener(&listener);
    while (1) {
        idc_event_t  event;

        if (channel->getEvent(&event)) {
        }
        sleep(1);
    }
    channel->removeListener(&listener);

    return (0);
}
