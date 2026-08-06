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

#ifndef _IDC_CLIENT_H
#define _IDC_CLIENT_H

#include <idc_common.h>

class IIDCEventCallback {
public:
    virtual void onResult(void *extra, uint64_t result) {
        IDC_UNUSED(extra);
        IDC_UNUSED(result);
    }
    virtual void onResult(void *extra, uint64_t result, void *reply) {
        IDC_UNUSED(reply);
        onResult(extra, result);
    };

protected:
    virtual ~IIDCEventCallback(void) {
    }
};

class IInterdomainChannel {
public:
    static IInterdomainChannel *get(void);
    static IInterdomainChannel *create(void);
    virtual void setName(const char *name)        = 0;
    virtual void getName(char *name)              = 0;
    virtual void addListener(void)                = 0;
    virtual void removeListener(void)             = 0;
    virtual bool connect(const char *domain_name) = 0;
    virtual void disconnect(void)           = 0;
    virtual bool postEvent(idc_event_t *event, IIDCEventCallback *cb,
                           void *extra)  = 0;
    virtual bool postBuffer(idc_buffer_t *buf, IIDCEventCallback *cb,
                            void *extra) = 0;
    virtual bool postGfxBuffer(idc_gfx_buffer_t *buf, IIDCEventCallback *cb,
                               void *extra) = 0;
    virtual void destroy(void)              = 0;

    virtual int allocateAndFillIonBuffer(const void *data, size_t size, int *ionFd) = 0;
    virtual int createClusterConnection() = 0;
    virtual int ancil_send_fds_with_buffer(int sock, const int *fds, unsigned n_fds) = 0;


protected:
    virtual ~IInterdomainChannel(void) {
    }
};

#endif //_IDC_CLIENT_H
