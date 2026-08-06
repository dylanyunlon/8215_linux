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

#ifndef CUDPSOCKETSERVICE_H
#define CUDPSOCKETSERVICE_H

#include <sys/unistd.h>
#include <stddef.h>
#include <string>
#include "threadproc.h"

class CUDPSocket
{
public:
    CUDPSocket();
    virtual ~CUDPSocket();

    int bind(const std::string &addr);
    int read(void *buff, int buffSize, std::string &addr);
    int read(void *buff, int buffSize, std::string &addr,
                unsigned long milliSecond);
    int write(const void *buff, int buffSize, const std::string &addr);

protected:
    const char * getAddr() const;
    int select(unsigned long milliSecond = 0);// forever
    int close();

private:
    int m_sockfd;
    std::string m_addr;
};

#endif // CUDPSOCKETSERVICE_H
