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

#ifndef CMESSAGEHANDLER_H
#define CMESSAGEHANDLER_H

#include "cconditionlock.h"
#include "cautothread.h"
#include "cmessage.h"

namespace universal_utils
{

class CMessageHandler: private CAutoThread
{
public:
    CMessageHandler(const char *name = NULL);
    virtual ~CMessageHandler();

    const static unsigned int DELAYED_MESSAGE_CHECK_MILLIS = 100;

    virtual int sendMessage(const CMessage &message);
    virtual int sendMessageAtFrontOfQueue(const CMessage &message);
    virtual int sendMessageDelayed(const CMessage &message, unsigned long delayMillis);
    virtual void startProcess();
    virtual void stopProcess();
    virtual void flushMessage();
    virtual void removeMessages(int what);
    virtual bool hasMessages(int what);

protected:
    virtual int handleMessage(const CMessage &message);

private:
    CMessageHandler(const CMessageHandler& other);
    CMessageHandler& operator=(const CMessageHandler& other);

    unsigned long threadRun();
    bool threadStart();
    bool threadStop();

    void checkDelayedMessages();
    long long nowMilliSecond() const;

    CConditionLock *m_messageCond;
    int m_messageCondIndex;

    std::list<CMessage> m_messages;
    std::list<CMessage> m_delayMessages;

    bool m_enableDebug;
};

}

#endif // CMESSAGEHANDLER_H