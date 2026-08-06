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

#ifndef THREADPROC
#define THREADPROC
#include "cautothread.h"
#include "cconditionlock.h"

namespace universal_utils
{

template <class T>
class CThreadProc : public CAutoThread
{
public:
    CThreadProc(const char* name = NULL)
        : CAutoThread(name)
        , m_object(NULL)
        , m_procFunc(NULL)
    {
        m_trigerCond = m_lock.newCondition();
        m_trigerCount = 0;
    }

    ~CThreadProc(){
        m_lock.releaseCondition(m_trigerCond);
    }

    unsigned long threadRun()
    {
        while (!isTerminated())
        {
            m_lock.lock();
            if (m_trigerCount == 0) {
                m_lock.await(m_trigerCond);
            }
            m_trigerCount--;
            m_lock.unlock();

            if (!isTerminated())
            {
                if (m_object && m_procFunc)
                {
                    (m_object->*m_procFunc)();
                }

            }
        }
        return 0;
    }

    bool threadStop()
    {
        threadTerminated();
        m_lock.signalAll(m_trigerCond);
        return CAutoThread::threadStop();
    }

    bool init(T *object, bool (T::*procFunc)())
    {
        m_object = object;
        m_procFunc = procFunc;
        return true;
    }

    bool triggerProc()
    {
        m_lock.lock();
        m_trigerCount++;
        m_lock.unlock();
        return m_lock.signal(m_trigerCond);
    }

private:
    T *m_object;
    bool (T::*m_procFunc)();

    CConditionLock m_lock;
    int m_trigerCond;
    int m_trigerCount;
};

}

namespace universal_utils {
template <class T>
class LoopThread : public CAutoThread
{
public:
    LoopThread(const char *name = NULL, unsigned long time = CAutoThread::DEFAULT_THREAD_TERMINATED_TIME)
        : CAutoThread(name, time)
        , m_object(NULL)
        , m_procFunc(NULL)
    {
    }

    ~LoopThread()
    {
    }

    unsigned long threadRun()
    {
        while (!isTerminated()) {
            if (m_object && m_procFunc) {
                (m_object->*m_procFunc)();
            }
        }

        return 0;
    }

    bool init(T *object, bool (T::*procFunc)())
    {
        m_object = object;
        m_procFunc = procFunc;
        return true;
    }

private:
    T *m_object;
    bool (T::*m_procFunc)();
};
}

namespace universal_utils {
template <class T>
class SingleThread : public CAutoThread
{
public:
    SingleThread()
        : m_object(NULL)
        , m_procFunc(NULL)
    {
    }

    ~SingleThread()
    {
    }

    bool threadStart()
    {
        int err = -1;

        if (CAutoThread::threadStart()) {
            err = pthread_detach(m_threadId);
            if (0 != err) {
                UTILS_LOGE(tag, "pthread_detach fail: %s\n", strerror(err));
            } else
                m_threadId = 0;
        }

        return (0 == err);
    }

    bool init(T *object, bool (T::*procFunc)())
    {
        m_object = object;
        m_procFunc = procFunc;
        return true;
    }

    unsigned long threadRun()
    {
        if (m_object && m_procFunc) {
            (m_object->*m_procFunc)();
        }

        return 0;
    }

private:
    static const char *tag;
    T *m_object;
    bool (T::*m_procFunc)();
};
}
namespace universal_utils {
template <class T>
const char *SingleThread<T>::tag = "SingleThread";
}
#endif // THREADPROC

