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
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <errno.h>
#include "cudpsocket.h"
#include "applog.h"

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 254
#endif

static const char TAG[] = "CUDPSocket";

CUDPSocket::CUDPSocket()
{
    m_sockfd = socket(AF_LOCAL, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (-1 == m_sockfd) {
        LOGE(TAG, "socket fail: %s\n", strerror(errno));
    }
}

CUDPSocket::~CUDPSocket()
{
    int ret = close();
    if (-1 == ret) {
        LOGE(TAG, "close fail\n");
    }
}

int CUDPSocket::bind(const std::string &addr)
{
    int ret = 0;
    int len = addr.length();
    int sockAddrLen = 0;
    struct sockaddr_un sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));

    sockAddr.sun_family = AF_UNIX;
    sockAddr.sun_path[0] = '\0';
    len = (len > UNIX_PATH_MAX) ? UNIX_PATH_MAX : len;
    m_addr = addr;
    strncpy(sockAddr.sun_path + 1, addr.c_str(), len);
    sockAddrLen = offsetof(struct sockaddr_un, sun_path)
                    + strlen(sockAddr.sun_path + 1) + 1;
    ret = ::bind(m_sockfd, (struct sockaddr*)&sockAddr, sockAddrLen);
    if (-1 == ret) {
        LOGE(TAG, "bind %s fail: %s\n", addr.c_str(), strerror(errno));
    }

    return ret;
}

const char * CUDPSocket::getAddr() const
{
    return m_addr.c_str();
}

int CUDPSocket::read(void *buff, int buffSize, std::string &addr)
{
    int ret = -1;
    socklen_t sockAddrLen = 0;
    struct sockaddr_un sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));

    if (buff != NULL) {
        ret = ::recvfrom(m_sockfd, buff, buffSize, 0,
                    (struct sockaddr *)&sockAddr, &sockAddrLen);
        if (-1 == ret) {
            LOGE(TAG, "%s recvfrom fail:%s\n", getAddr(), strerror(errno));
        } else if (0 == ret) {
            LOGE(TAG, "the peer has been shutdown\n");
        } else {
            addr = sockAddr.sun_path;
        }
    } else {
        LOGE(TAG, "buff is NULL\n");
    }

    return ret;
}

int CUDPSocket::read(void *buff, int buffSize, std::string &addr,
                        unsigned long milliSecond)
{
    int ret = -1;

    if (buff != NULL) {
        ret = select(milliSecond);
        if (ret > 0) {
            ret = read(buff, buffSize, addr);
        } else if (0 == ret) {
            //wait time out.
            LOGE(TAG, "%s read timeout\n", getAddr());
            ret = -1;
        } else {
            //error.
            LOGE(TAG, "%s read fail\n", getAddr());
            ret = -1;
        }
    } else {
        LOGE(TAG, "buff is NULL\n");
    }

    return ret;
}

int CUDPSocket::write(const void *buff, int buffSize, const std::string &addr)
{
    int ret = -1;
    int len = addr.length();
    socklen_t sockAddrLen = 0;
    struct sockaddr_un sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));

    if (buff != NULL) {
        sockAddr.sun_family = AF_UNIX;
        len = (len > UNIX_PATH_MAX) ? UNIX_PATH_MAX : len;
        sockAddr.sun_path[0] = '\0';
        strncpy(sockAddr.sun_path + 1, addr.c_str(), len);
        sockAddrLen = offsetof(struct sockaddr_un, sun_path)
                        + strlen(sockAddr.sun_path + 1) + 1;
        ret = sendto(m_sockfd, buff, buffSize, 0,
                        (struct sockaddr*)&sockAddr, sockAddrLen);
        if (-1 == ret) {
            LOGE(TAG, "%s sendto %s fail: %s\n",
                    getAddr(), addr.c_str(), strerror(errno));
        }
    } else {
        LOGE(TAG, "buff is NULL\n");
    }

    return ret;
}

int CUDPSocket::select(unsigned long milliSecond)
{
    int ret = -1;
    fd_set readfs;

    FD_ZERO(&readfs);
    FD_SET(m_sockfd, &readfs);

    if (0 == milliSecond) {
        ret = ::select(m_sockfd + 1, &readfs, NULL, NULL, NULL);
    } else {
        struct timeval time;
        time.tv_sec = milliSecond / 1000;
        time.tv_usec = milliSecond % 1000 * 1000;
        ret = ::select(m_sockfd + 1, &readfs, NULL, NULL, &time);
    }

    if (-1 == ret) {
        LOGE(TAG, "select fail: %s\n", strerror(errno));
    }

    return ret;
}

int CUDPSocket::close()
{
    int ret = -1;

    if (m_sockfd > 0) {
        ret = ::close(m_sockfd);
        if (-1 == ret) {
            LOGE(TAG, "close %s fail: %s\n", getAddr(), strerror(errno));
        }
    } else {
        LOGE(TAG, "no socket\n");
    }

    return ret;
}

