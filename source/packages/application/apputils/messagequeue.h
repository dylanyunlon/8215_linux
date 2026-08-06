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
 
#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <string>
#include <string.h>

using std::string;

class Message
{
public:
    static const size_t m_bufSize = 256;

    Message()
        : m_type(0)
    {
        memset(m_cbuf, 0, sizeof(char) * m_bufSize);
    }

    long m_type;
    union {
        char m_cbuf[m_bufSize];
        int m_ibuf[m_bufSize / sizeof(int)];
    };
};

class MessageQueue
{
public:
    typedef enum {
        MSG_CLIENT = 0,
        MSG_SERVICE,
    } E_MSG_ROLE;

    MessageQueue(const string &pathname, int proj_id, E_MSG_ROLE role = MSG_CLIENT);
    virtual ~MessageQueue()
    {
    }

    int sendMsg(const Message *msgp, size_t msgsz, int msgflg);
    int sendMsg(long type, const string &flag);
    int sendMsg(long type, int num);
    int recvMsg(Message *msgp, size_t msgsz, long msgtyp, int msgflag);
    int recvMsg(long type, string &str);
    int recvMsg(long type);

private:
    int m_qid;
};

#endif
