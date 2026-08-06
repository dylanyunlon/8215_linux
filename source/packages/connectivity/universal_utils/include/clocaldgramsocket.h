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

#ifndef CLOCALDGRAMSOCKET_H
#define CLOCALDGRAMSOCKET_H

#include <list>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <stddef.h>

#include "clog.h"

namespace universal_utils {

#define CtrlPipe_Shutdown 0

const static char* SOCK_TAG = "CLocalDGRAMSocket";

class CLocalDGRAMSocket
{
public:
    CLocalDGRAMSocket() :
        m_socketFd(-1),
        m_error(0),
        m_localAddressLen(0),
        m_peerAddressLen(0)
    {
        memset(&m_localAddress, 0, sizeof(m_localAddress));
        memset(&m_peerAddress, 0, sizeof(m_peerAddress));

        if (pipe(mCtrlPipe)) {
            UTILS_LOGE(SOCK_TAG, "pipe failed (%s)", strerror(errno));
            mCtrlPipe[0] = -1;
            mCtrlPipe[1] = -1;
        }
    }

    ~CLocalDGRAMSocket()
    {
        if (-1 != mCtrlPipe[0]) {
            close(mCtrlPipe[0]);
        }
        if (-1 != mCtrlPipe[0]) {
            close(mCtrlPipe[1]);
        }
        mCtrlPipe[0] = -1;
        mCtrlPipe[1] = -1;
    }

    int setLocalAddress(const std::string name)
    {
        memset(&m_localAddress, 0, sizeof(m_localAddress));
        m_localAddress.sun_family = AF_UNIX;

        m_localAddress.sun_path[0] = '\0'; /*abstract address*/
        strncpy(m_localAddress.sun_path + 1, name.c_str(), name.length());
        m_localAddressLen = offsetof(struct sockaddr_un, sun_path) + name.length() + 1;

        return 0;
    }

    const struct sockaddr_un *getLocalAddress() const
    {
        return &m_localAddress;
    }

    int setPeerAddress(const std::string name)
    {
        memset(&m_peerAddress, 0, sizeof(m_peerAddress));
        m_peerAddress.sun_family = AF_UNIX;

        m_peerAddress.sun_path[0] = '\0';/*abstract address*/
        strncpy(m_peerAddress.sun_path + 1, name.c_str(), name.length());
        m_peerAddressLen = offsetof(struct sockaddr_un, sun_path) + name.length() + 1;

        return 0;
    }

    int setPeerAddress(const struct sockaddr_un *addr)
    {
        if (NULL == addr) {
            return -1;
        }

        memset(&m_peerAddress, 0, sizeof(m_peerAddress));
        memcpy(&m_peerAddress, addr, sizeof(struct sockaddr_un));
        m_peerAddressLen = offsetof(struct sockaddr_un, sun_path) + strlen(m_peerAddress.sun_path + 1) + 1;

        return 0;

    }

    const struct sockaddr_un *getPeerAddress() const
    {
        return &m_peerAddress;
    }

    int initSocket()
    {
        int s = 0;

        if ((s = socket(AF_LOCAL, SOCK_DGRAM, 0)) < 0) {
            setLastError(errno);
            return -1;
        }

        m_socketFd = s;

        return m_socketFd;
    }

    int bindSocket()
    {
        unlink(m_localAddress.sun_path);

        if (bind(m_socketFd, (struct sockaddr*)&m_localAddress, m_localAddressLen) < 0) {
            setLastError(errno);
            return -1;
        }

        return 0;
    }

    int bindSocket(const std::string name)
    {
        setLocalAddress(name);

        bindSocket();

        return 0;
    }

    int connectToServer()
    {
        if (connect(m_socketFd, (const struct sockaddr*)&m_peerAddress, m_peerAddressLen) < 0) {
            setLastError(errno);
            return -1;
        }

        return 0;
    }

    int getSocket() const
    {
        return m_socketFd;
    }

    int getLastError ()
    {
        return m_error;
    }

    void setLastError(int error)
    {
        m_error = error;
    }

    void closeSocket()
    {
        char c = CtrlPipe_Shutdown;
        int rc = -1;

        if (-1 != mCtrlPipe[1]) {
            rc = write(mCtrlPipe[1], &c, 1);
            if (rc != 1) {
               UTILS_LOGW(SOCK_TAG, "Error writing to control pipe (%s)", strerror(errno));
            }
        }

        if (m_socketFd > 0) {
            int ret = close(m_socketFd);
            if (ret == -1) {
                UTILS_LOGE(SOCK_TAG, "close error:%s", strerror(errno));
                return;
            }
            m_socketFd = -1;
        }
    }

    int readData(void *data, unsigned int length, unsigned int milliseconds = 0)
    {
        int ret = 0;
        struct timeval timeout = {0, 0};
        struct timeval *pTimeout = NULL;
        int max = -1;

        fd_set fds;

        FD_ZERO(&fds);

        max = m_socketFd;
        FD_SET(m_socketFd, &fds);
        FD_SET(mCtrlPipe[0], &fds);

        if (mCtrlPipe[0] > max) {
            max = mCtrlPipe[0];
        }

        if (0 != milliseconds) {
            timeout.tv_usec = (milliseconds % 1000) * 1000;
            timeout.tv_sec = milliseconds / 1000;
            pTimeout = &timeout;
        }

        do {
            ret = select(max + 1, &fds, NULL, NULL, pTimeout);

            if (FD_ISSET(mCtrlPipe[0], &fds)) {
                char c = CtrlPipe_Shutdown;
                int rc = -1;

                rc  = read(mCtrlPipe[0], &c, 1);
                if (rc < 0) {
                    UTILS_LOGW(SOCK_TAG, "read  fail from mCtrlPipe(%s)", strerror(errno));

                } else if (c == CtrlPipe_Shutdown) {
                    UTILS_LOGD(SOCK_TAG, "read CtrlPipe_Shutdown from mCtrlPipe");

                    ret = -1;
                    break;
                }
                continue;
            }
        } while(ret < 0 && errno == EINTR);

        if (ret < 0) {
            setLastError(errno);
            return -1;
        } else if (ret == 0) {
            errno = ETIMEDOUT;
            setLastError(errno);
            return 0;   /*time out*/
        } else {
            ret = recvfrom(m_socketFd, (void*)data, length, 0,
                (struct sockaddr*)&m_peerAddress, &m_peerAddressLen);
            if (ret < 0) {
                setLastError(errno);
                return -1;
            }
        }

        return ret;
    }

    int sendData(const void *data, int length)
    {
        int ret = 0;

        ret = sendto(m_socketFd, data, length, 0, (struct sockaddr*)&m_peerAddress, m_peerAddressLen);
        if (ret < 0) {
            setLastError(errno);
            return -1;
        }

        return ret;
    }

    int setReadBufferSize(unsigned int size)
    {
        int ret = 0;

        ret = setsockopt(m_socketFd, SOL_SOCKET , SO_RCVBUF , &size, sizeof(size));
        if (ret < 0) {
            setLastError(errno);
            return -1;
        }

        return ret;
    }

    static bool compareAddress(const struct sockaddr_un *addr1,
        const struct sockaddr_un *addr2)
    {
        if (addr1->sun_family == addr2->sun_family) {
            if ((addr1->sun_path[0] == '\0') && (addr2->sun_path[0] == '\0')) {
                return (0 == strcmp(addr1->sun_path + 1, addr2->sun_path + 1)) ? true : false;
            } else {
                return (0 == strcmp(addr1->sun_path, addr2->sun_path)) ? true : false;
            }
        } else {
            return false;
        }
    }
protected:


private:
    int m_socketFd;
    int mCtrlPipe[2];
    int m_error;

    struct sockaddr_un m_localAddress;
    unsigned int m_localAddressLen;

    struct sockaddr_un m_peerAddress;
    unsigned int m_peerAddressLen;
};

}

#endif // CLOCALDGRAMSOCKET_H
