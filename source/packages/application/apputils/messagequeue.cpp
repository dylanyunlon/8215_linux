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
 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "messagequeue.h"
#include "applog.h"

const char TAG[] = "MessageQueue";

MessageQueue::MessageQueue(const string &pathname, int proj_id, E_MSG_ROLE role)
{
    int key = ftok(pathname.c_str(), proj_id);
    if (-1 == key) {
        LOGE(TAG, "ftok fail: %s\n", strerror(errno));
    }

    m_qid = msgget(key, IPC_CREAT | 0666);
    if (-1 == m_qid) {
        LOGE(TAG, "msgget fail: %s\n", strerror(errno));
    }

    if (MSG_SERVICE == role) {
        Message msg;
        while (msgrcv(m_qid, &msg, msg.m_bufSize, 0, IPC_NOWAIT) != -1);
    }
}

int MessageQueue::sendMsg(const Message *msgp, size_t msgsz, int msgflg)
{
    int ret = -1;

    ret = msgsnd(m_qid, (void *)msgp, msgsz, msgflg);
    if (-1 == ret) {
        LOGE(TAG, "msgsnd fail: %s\n", strerror(errno));
    }

    return ret;
}

int MessageQueue::sendMsg(long type, const string &str)
{
    Message msg;
    msg.m_type = type;
    memcpy(msg.m_cbuf, str.c_str(), str.size() + 1);
    return sendMsg(&msg, str.size() + 1, 0);
}

int MessageQueue::sendMsg(long type, int num)
{
    Message msg;
    msg.m_type = type;
    msg.m_ibuf[0] = num;
    return sendMsg(&msg, sizeof(num), 0);
}

int MessageQueue::recvMsg(Message *msgp, size_t msgsz, long msgtyp, int msgflag)
{
    int ret = -1;

    ret = msgrcv(m_qid, (void *)msgp, msgsz, msgtyp, msgflag);
    if (-1 == ret) {
        LOGE(TAG, "msgrcv fail: %s\n", strerror(errno));
    }

    return ret;
}

int MessageQueue::recvMsg(long type, string &str)
{
    int ret = -1;
    Message msg;

    str.clear();

    ret = recvMsg(&msg, msg.m_bufSize, type, 0);
    if (ret >= 0) {
        str.append(msg.m_cbuf);
    }

    return ret;
}

int MessageQueue::recvMsg(long type)
{
    int ret = -1, res = -1;
    Message msg;

    res = recvMsg(&msg, sizeof(int), type, 0);
    if (sizeof(int) != res) {
        ret = -1;
    } else {
        ret = msg.m_ibuf[0];
    }

    return ret;
}


