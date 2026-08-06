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

#ifndef AUTOTHREAD_H
#define AUTOTHREAD_H
#include <pthread.h>
#include <stddef.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include "clog.h"

namespace universal_utils
{

class CAutoThread
{
public:
    static const unsigned long DEFAULT_THREAD_TERMINATED_TIME = 5000;
    static const unsigned int MAX_TRHEAD_NAME_LENGTH = 16;

public:
    CAutoThread(const char *name = NULL, unsigned long threadTerminateTime = DEFAULT_THREAD_TERMINATED_TIME);
    virtual ~CAutoThread();

    virtual bool threadStart();
    virtual bool threadStop();

    bool setPriority(int priority);

    bool threadTerminated();

    /*
    * Abandoned param milliSeconds.
    * we must wait thread exit, if thread not exit it must be bug, fix it right now.
    */
    bool waitThreadComplete(unsigned long milliSeconds = DEFAULT_THREAD_TERMINATED_TIME);
    bool forceTerminate();
    bool isTerminated() const;

    bool getExitCodeThread(void *exitCode);
    pthread_t getThreadId() const;

protected:
    virtual unsigned long threadRun() = 0;
    bool m_terminated;
    pthread_t m_threadId;

private:
    static void* threadProc(void *arg);

private:
    unsigned long m_exitCode;
    unsigned long m_threadTerminatedTime;
    char m_name[MAX_TRHEAD_NAME_LENGTH];
};

}

#endif // AUTOTHREAD_H

