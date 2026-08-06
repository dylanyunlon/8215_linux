/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
 
#include "cotpoweroff.h"
#include "applog.h"
#include "funclistener.h"
#include <pthread.h>
#include <sys/wait.h>
#include <errno.h>
#include <QStringList>
#include <QProcess>


static const char* const tag = "COtPowerOff";

typedef enum
{
    E_MSG_GO_TO_POWER_OFF,
    E_MSG_POWER_OFF,
}E_MSG_TYPE;


#if !NOT_USE_QOBJECT
COtPowerOff::COtPowerOff(const std::string& moduleName)
    : m_threadProc(NULL)
    , m_listner(NULL)
    , m_moduleName(moduleName)
{
    LOGD(tag, "contruct enter\n");
    m_evtList.clear ();
    LOGD(tag, "before new cthreadproc\n");
    m_threadProc = new universal_utils::CThreadProc<COtPowerOff>;
    LOGD(tag, "after new cthreadproc\n");

    QObject::connect(this, SIGNAL(sigPowerOff()), this, SLOT(doPowerOff()),
                    Qt::BlockingQueuedConnection);

    if (m_threadProc != NULL) {
        m_threadProc->init(this, &COtPowerOff::threadProc);
        LOGD(tag, "after cthreadproc init\n");
        m_threadProc->threadStart();
        LOGD(tag, "after cthreadproc start\n");
    }
    LOGD(tag, "contruct leave\n");
}

COtPowerOff::~COtPowerOff()
{
    LOGD(tag, "start ~COtPowerOff..\n");
    if (m_threadProc != NULL) {
        m_threadProc->threadStop();
        delete m_threadProc;
        m_threadProc = NULL;
    }
    LOGD(tag, "end ~COtPowerOff..\n");
}

int COtPowerOff::doPowerOff()
{
    LOGD(tag, "doPowerOff\n");
    //onSaveInstance();
    postEvt(E_MSG_POWER_OFF);

    return 0;
}

int COtPowerOff::powerOff()
{
    LOGD(tag, "PowerOff\n");
    postEvt(E_MSG_GO_TO_POWER_OFF);

    return 0;
}

int COtPowerOff::setListner(universal_utils::CFuncListener* funcListner)
{
    m_listner = funcListner;
    return 0;
}

int COtPowerOff::onSaveInstance()
{
    LOGD(tag, "onSaveInstance\n");
    if (m_listner != NULL) {
        m_listner->doFunc((unsigned int)E_POWER_OFF_SAVE_STATE, 0, 0);
    }
    return 0;
}

bool COtPowerOff::threadProc()
{
    unsigned int msg;
    pid_t pid = -1;
    int rtn = -1;
    bool ret = true;
    int err = 0;
    //LOGD(tag, "ThreadProc enter, thread id, 0x%x\n" , pthread_self());
    if (popEvt(msg)) {
        LOGD(tag, "threadProc get msg (%d)\n", msg);
        switch (msg) {
            case E_MSG_GO_TO_POWER_OFF:
            {
#if 0
                pid = fork();
                if (-1 == pid) {
                    LOGE(tag, "threadProc start power off sleep fork fail\n");
                    break;
                }

                if (0 == pid) {
                    err = execl("/app/linux_mm_power_off_test.sh", "ot_power_off", "sleep", NULL);
                    if (err == -1) {
                        LOGE(tag, "execl error: %s\n", strerror(errno));
                        LOGE(tag, "threadProc start power off sleep fail\n");
                        exit(0);
                    }
                } else {
                    rtn = -1;
                    ret = waitpid(pid, &rtn, WNOHANG);// should not go out
                    if (-1 == ret) {
                        LOGE(tag, "threadProc start power off sleep waitpid fail\n");
                        break;
                    }
                    //emit sigPowerOff();
                }
#else 
                QStringList arguments;  
                arguments << "sleep"<< m_moduleName.c_str();
                int code = QProcess::execute("/app/linux_mm_power_off_test.sh", arguments);
                LOGD(tag, "threadProc start power off sleep  code(%d)\n", code);
                if ((code != -2) && (code != -1)) {
                    emit sigPowerOff();
                }
#endif
            }
            break;

            case E_MSG_POWER_OFF:
            {
#if 0
                pid = fork();
                if (-1 == pid) {
                    LOGE(tag, "threadProc start power off sleep fork fail\n");
                    break;
                }

                if (0 == pid) {
                    err = execl("/app/linux_mm_power_off_test.sh", "ot_power_off", "poweroff", NULL);
                    if (err == -1) {
                        LOGE(tag, "execl error: %s\n", strerror(errno));
                        LOGE(tag, "threadProc start power off fail\n");
                        exit(0);
                    }
                }
                else {
                    rtn = -1;
                    ret = waitpid(pid, &rtn, WNOHANG);// should not go out
                    if (-1 == ret) {
                        LOGE(tag, "threadProc start power off waitpid fail\n");
                    }
                }
#else 
                QStringList arguments;  
                arguments << "poweroff" << m_moduleName.c_str();
                LOGD(tag, "threadProc start power off  moduleName(%s)\n", m_moduleName.c_str());
                int code = QProcess::execute("/app/linux_mm_power_off_test.sh", arguments);
                LOGD(tag, "threadProc start power off  code(%d)\n", code);
#endif
            }
            break;

            default:
                break;
        }
    }

    return true;
}

bool COtPowerOff::postEvt(const unsigned int& msg)
{
    bool ret = false;
    m_lock.lock();
    if (m_threadProc != NULL) {
        m_evtList.push_back (msg);
        ret = m_threadProc->triggerProc();
    }
    m_lock.unlock();

    return ret;
}

bool COtPowerOff::popEvt(unsigned int& msg)
{
    bool ret = false;
    std::list<unsigned int>::iterator it;
    m_lock.lock();

    it = m_evtList.begin ();
    if (it != m_evtList.end ()) {
        msg = *it;
        m_evtList.erase (it);
        ret = true;
    }
    m_lock.unlock();

    return ret;
}
#else
COtPowerOff::COtPowerOff(const std::string& moduleName)
    : m_threadProc(NULL)
    , m_listner(NULL)
    , m_moduleName(moduleName)
{
    LOGD(tag, "contruct enter\n");
    m_evtList.clear ();
    LOGD(tag, "before new cthreadproc\n");
    m_threadProc = new CThreadProc<COtPowerOff>;
    LOGD(tag, "after new cthreadproc\n");

    if (m_threadProc != NULL) {
        m_threadProc->init(this, &COtPowerOff::threadProc);
        LOGD(tag, "after cthreadproc init\n");
        m_threadProc->threadStart();
        LOGD(tag, "after cthreadproc start\n");
    }
    LOGD(tag, "contruct leave\n");
}

COtPowerOff::~COtPowerOff()
{
    LOGD(tag, "start ~COtPowerOff..\n");
    if (m_threadProc != NULL) {
        m_threadProc->threadStop();
        delete m_threadProc;
        m_threadProc = NULL;
    }
    LOGD(tag, "end ~COtPowerOff..\n");
}

int COtPowerOff::powerOff()
{
    LOGD(tag, "PowerOff\n");
    postEvt(E_MSG_GO_TO_POWER_OFF);

    return 0;
}

int COtPowerOff::setListner(universal_utils::CFuncListener* funcListner)
{
    m_listner = funcListner;
    return 0;
}

int COtPowerOff::onSaveInstance()
{
    LOGD(tag, "onSaveInstance\n");
    if (m_listner != NULL) {
        m_listner->doFunc((unsigned int)E_POWER_OFF_SAVE_STATE, 0, 0);
    }
    return 0;
}

bool COtPowerOff::threadProc()
{
    unsigned int msg;
    pid_t pid = -1;
    int rtn = -1;
    bool ret = true;
    int err = 0;
    //LOGD(tag, "ThreadProc enter, thread id, 0x%x\n" , pthread_self());
    if (popEvt(msg)) {
        LOGD(tag, "threadProc get msg (%d)\n", msg);
        switch (msg) {
            case E_MSG_GO_TO_POWER_OFF:
            {
#if 0
                pid = fork();
                if (-1 == pid) {
                    LOGE(tag, "threadProc start power off sleep fork fail\n");
                    break;
                }

                if (0 == pid) {
                    err = execl("/app/linux_mm_power_off_test.sh", "ot_power_off", "sleep", NULL);
                    if (err == -1) {
                        LOGE(tag, "execl error: %s\n", strerror(errno));
                        LOGE(tag, "threadProc start power off sleep fail\n");
                        exit(0);
                    }
                } else {
                    rtn = -1;
                    ret = waitpid(pid, &rtn, WNOHANG);// should not go out
                    if (-1 == ret) {
                        LOGE(tag, "threadProc start power off sleep waitpid fail\n");
                        break;
                    }
                    //emit sigPowerOff();
                }
#else
                int code = QProcess::execute("/app/linux_mm_power_off_test.sh");
                LOGE(tag, "threadProc start code(%d)\n", code);

#endif
            }
                break;

            case E_MSG_POWER_OFF:
                pid = fork();
                if (-1 == pid) {
                    LOGE(tag, "threadProc start power off sleep fork fail\n");
                    break;
                }

                if (0 == pid) {
                    err = execl("/app/linux_mm_power_off_test.sh", "ot_power_off", "poweroff", NULL);
                    if (err == -1) {
                        LOGE(tag, "execl error: %s\n", strerror(errno));
                        LOGE(tag, "threadProc start power off fail\n");
                        exit(0);
                    }
                }
                else {
                    rtn = -1;
                    ret = waitpid(pid, &rtn, WNOHANG);// should not go out
                    if (-1 == ret) {
                        LOGE(tag, "threadProc start power off waitpid fail\n");
                    }
                }
                break;

            default:
                break;
        }
    }

    return true;
}

bool COtPowerOff::postEvt(const unsigned int& msg)
{
    bool ret = false;
    m_lock.lock();
    if (m_threadProc != NULL) {
        m_evtList.push_back (msg);
        ret = m_threadProc->triggerProc();
    }
    m_lock.unlock();

    return ret;
}

bool COtPowerOff::popEvt(unsigned int& msg)
{
    bool ret = false;
    std::list<unsigned int>::iterator it;
    m_lock.lock();

    it = m_evtList.begin ();
    if (it != m_evtList.end ()) {
        msg = *it;
        m_evtList.erase (it);
        ret = true;
    }
    m_lock.unlock();

    return ret;
}

#endif
