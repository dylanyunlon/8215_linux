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

#ifndef _IDC_SERVER_H
#define _IDC_SERVER_H

#include <idc_common.h>

class IInterdomainChannelProxy;

class IIDCProxyListener {
public:
    virtual void onEvent(IInterdomainChannelProxy *proxy,
                         idc_event_t *event) {
        IDC_UNUSED(proxy);
        IDC_UNUSED(event);
    };
    virtual void onEvent(IInterdomainChannelProxy *proxy,
                         idc_event_t *event, void *reply) {
        IDC_UNUSED(reply);
        onEvent(proxy, event);
    };

protected:
    ~IIDCProxyListener(void) {
    }
};

class IIDCMonitorListener {
public:
    virtual void onEvent(const char *domain, const char *channel,
                         idc_event_t *event) = 0;

protected:
    ~IIDCMonitorListener(void) {
    }
};

class IInterdomainChannelProxy {
public:
    static IInterdomainChannelProxy *get(const char *name);

    virtual void addListener(IIDCProxyListener *listener) = 0;
    virtual void removeListener(IIDCProxyListener *listener) = 0;
    virtual void getName(char *name) = 0;
    virtual bool getEvent(idc_event_t *event) = 0;
    virtual bool getBuffer(idc_buffer_t *buf) = 0;
    virtual void release(void) = 0;

protected:
    virtual ~IInterdomainChannelProxy(void) {
    }
};


class IInterdomainChannelMonitor {
public:
    static IInterdomainChannelMonitor *get(void);

    virtual void addListener(IIDCMonitorListener *listener) = 0;
    virtual void removeListener(IIDCMonitorListener *listener) = 0;

protected:
    virtual ~IInterdomainChannelMonitor(void) {
    }
};

#endif //_IDC_SERVER_H
