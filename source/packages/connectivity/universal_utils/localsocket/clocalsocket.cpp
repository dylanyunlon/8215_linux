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

#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stddef.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include "clocalsocket.h"
#include "universalutilstype.h"
#include "clog.h"

namespace universal_utils {

static const char *TAG = "CLocalSocket";

CLocalSocket::CLocalSocket()
    : m_socketFd(-1)
    , m_error(0)
    , m_localAddressLen(0)
    , m_peerAddressLen(0)
    , m_inboundFds(NULL)
    , m_inboundFdsLen(0)
{
    memset(&m_localAddress, 0, sizeof(m_localAddress));
    memset(&m_peerAddress, 0, sizeof(m_peerAddress));
}

CLocalSocket::~CLocalSocket()
{
    SAFE_DELETE_ARRAY(m_inboundFds);
}

int CLocalSocket::createSocket (int sockType)
{
    int fd = -1;

    if ((fd = socket(AF_LOCAL, sockType, 0)) < 0) {
        setLastError(errno);
        return -1;
    }

#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "createSocket, sockType: %d, fd: %d", sockType, fd);
#endif

    m_socketFd = fd;

    return m_socketFd;
}

void CLocalSocket::closeSocket()
{
    if (m_socketFd > -1) {
#ifdef OPEN_DEBUG_LOG
        UTILS_LOGI(TAG, "closeSocket, fd: %d", m_socketFd);
#endif
        close(m_socketFd);
        m_socketFd = -1;
    }
}

int CLocalSocket::bindSocket()
{
    int ret = 0;

#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "bindSocket, fd: %d", m_socketFd);
#endif

    if (bind(m_socketFd, (struct sockaddr*)&m_localAddress, m_localAddressLen) < 0) {
        setLastError(errno);
        ret = -1;
    }

    return ret;
}

int CLocalSocket::bindSocket(const std::string &localAddress)
{
    setLocalAddress(localAddress);

    return bindSocket();
}

int CLocalSocket::connectSocket()
{
    int ret = 0;

#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "connectSocket, fd: %d", m_socketFd);
#endif
    if (connect(m_socketFd, (const struct sockaddr*)&m_peerAddress, m_peerAddressLen) < 0) {
        setLastError(errno);
        ret = -1;
    }

    return ret;
}

int CLocalSocket::connectSocket(const std::string &peerAddress)
{
    setPeerAddress(peerAddress);

    return connectSocket();
}

int CLocalSocket::pending()
{
    if (m_socketFd < 0) {
        return -1;
    }

    int pending = 0;
    if (ioctl(m_socketFd, TIOCOUTQ, &pending) < 0) {
        setLastError(errno);
        return -1;
    }

    return pending;
}

int CLocalSocket::available()
{
    if (m_socketFd < 0) {
        return -1;
    }

    int avail = 0;
    if (ioctl(m_socketFd, FIONREAD, &avail) < 0) {
        setLastError(errno);
        return -1;
    }

    return avail;
}

int CLocalSocket::shutdownSocket(bool input)
{
    if (m_socketFd < 0) {
        return -1;
    }

    int ret = 0;
    if (shutdown(m_socketFd, input ? SHUT_RD : SHUT_WR) < 0) {
        setLastError(errno);
        ret = -1;
    }
#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "shutdownSocket, fd: %d, input: %d", m_socketFd, input);
#endif

    return ret;
}

int CLocalSocket::read()
{
    if (m_socketFd < 0) {
        return -1;
    }

    unsigned char buf;
    int err = readAll(&buf, 1);
    if (err < 0) {
        return 0;
    }
    if (err == 0) {
        // end of file
        return -1;
    }

    return (int)buf;
}

int CLocalSocket::read(char data[], int offset, int length)
{
    if (m_socketFd < 0) {
        UTILS_LOGI(TAG, "read, invalid fd: %d, this: %p", m_socketFd, this);
        return -1;
    }

    if (NULL == data) {
        UTILS_LOGI(TAG, "read, data is null, this: %p", this);
        return -1;
    }

    if (offset < 0 || length < 0) {
        UTILS_LOGI(TAG, "read, invalid params, offset: %d, length: %d, this: %p",
                offset, length, this);
        return -1;
    }

    if (0 == length) {
        // because socket_read_all returns 0 on EOF
        return 0;
    }

    int ret = readAll(&data[offset], length);

    // A return of -1 above means an exception is pending
    return (ret == 0) ? -1 : ret;
}


int CLocalSocket::write(const int data)
{
    if (m_socketFd < 0) {
        return -1;
    }

    return writeAll(&data, 1);
}

int CLocalSocket::write(const char data[], int offset, int length)
{
    if (m_socketFd < 0) {
        UTILS_LOGI(TAG, "write, invalid fd: %d, this: %p", m_socketFd, this);
        return -1;
    }

    if (NULL == data) {
        UTILS_LOGI(TAG, "write, data is null, this: %p", this);
        return -1;
    }

    if (offset < 0 || length < 0) {
        UTILS_LOGI(TAG, "write, invalid params, offset: %d, length: %d, this: %p",
                offset, length, this);
        return -1;
    }

    return writeAll(&data[offset], length);
}

void CLocalSocket::setFileDescriptor(int fd)
{
    m_socketFd = fd;
}

int CLocalSocket::getFileDescriptor() const
{
    return m_socketFd;
}

void CLocalSocket::getAncillaryFileDescriptors(int* fds, int len)
{
    if (NULL == fds) {
        UTILS_LOGE(TAG, "fds is NULL");
        return;
    }

    if (NULL == m_inboundFds) {
        UTILS_LOGE(TAG, "m_inboundFds is NULL");
        return;
    }

    for (int i = 0; i < len && i < m_inboundFdsLen; i++) {
        fds[i] = m_inboundFds[i];
    }
}

int CLocalSocket::getAncillaryFdsLength() const
{
    return m_inboundFdsLen;
}

int CLocalSocket::getLastError () const
{
    return m_error;
}

void CLocalSocket::setLastError(int error)
{
    m_error = error;
}

int CLocalSocket::setLocalAddress(const std::string &localAddress)
{
    memset(&m_localAddress, 0, sizeof(m_localAddress));
    m_localAddress.sun_family = AF_UNIX;

    m_localAddress.sun_path[0] = '\0'; /*abstract address*/
    strncpy(m_localAddress.sun_path + 1, localAddress.c_str(), localAddress.length());
    m_localAddressLen = offsetof(struct sockaddr_un, sun_path) + localAddress.length() + 1;

    return 0;
}

const struct sockaddr_un *CLocalSocket::getLocalAddress() const
{
    return &m_localAddress;
}

int CLocalSocket::setPeerAddress(const std::string &peerAddress)
{
    memset(&m_peerAddress, 0, sizeof(m_peerAddress));
    m_peerAddress.sun_family = AF_UNIX;

    m_peerAddress.sun_path[0] = '\0';/*abstract address*/
    strncpy(m_peerAddress.sun_path + 1, peerAddress.c_str(), peerAddress.length());
    m_peerAddressLen = offsetof(struct sockaddr_un, sun_path) + peerAddress.length() + 1;

    return 0;
}

int CLocalSocket::setPeerAddress(struct sockaddr_un *addr)
{
    if (NULL == addr) {
        return -1;
    }

    memset(&m_peerAddress, 0, sizeof(m_peerAddress));
    memcpy(&m_peerAddress, addr, sizeof(struct sockaddr_un));
    m_peerAddressLen = offsetof(struct sockaddr_un, sun_path) + strlen(m_peerAddress.sun_path + 1) + 1;

    return 0;
}

const struct sockaddr_un *CLocalSocket::getPeerAddress() const
{
    return &m_peerAddress;
}

int CLocalSocket::setReceiveBufferSize(int size)
{
    int boolValue = -1;

    return setOption(SO_RCVBUF, boolValue, size);
}

int CLocalSocket::getReceiveBufferSize()
{
    return getOption(SO_RCVBUF);
}

int CLocalSocket::setSendBufferSize(int size)
{
    int boolValue = -1;

    return setOption(SO_SNDBUF, boolValue, size);
}

int CLocalSocket::getSendBufferSize()
{
    int size = getOption(SO_SNDBUF);

    return size;
}

int CLocalSocket::setSoTimeout(int timeoutMillis)
{
    int boolValue = -1;

    return setOption(SO_SNDTIMEO, boolValue, timeoutMillis);
}

int CLocalSocket::getSoTimeout()
{
    int timeoutMillis = getOption(SO_SNDTIMEO);

    return timeoutMillis;
}

int CLocalSocket::setReuseAddress(bool reuse)
{
    int boolValue = -1;

    return setOption(SO_REUSEADDR, boolValue, reuse);
}

bool CLocalSocket::getReuseAddress()
{
    return getOption(SO_REUSEADDR);
}

int CLocalSocket::getOption(int optname)
{
    int ret = -1;
    int value = 0;
    int level = 0;
    socklen_t size = sizeof(int);

    if (!getOptionLevel(optname, level)) {
        return 0;
    }

    if (m_socketFd < 0) {
        return 0;
    }

    switch (optname) {
        case SO_LINGER: {
            struct linger lingr;
            size = sizeof(lingr);
            ret = getsockopt(m_socketFd, level, optname, &lingr, &size);
            if (!lingr.l_onoff) {
                value = -1;
            } else {
                value = lingr.l_linger;
            }
        }
        break;

        default:
            ret = getsockopt(m_socketFd, level, optname, &value, &size);
            break;
    }

    if (ret != 0) {
        setLastError(errno);
        return 0;
    }

    return value;
}

int CLocalSocket::setOption(int optname, int boolValue, int intValue)
{
    int ret = -1;
    int level = 0;

    if (!getOptionLevel(optname, level)) {
        return -1;
    }

    if (m_socketFd < 0) {
        return -1;
    }

    switch (optname) {
        case SO_LINGER: {
            /*
                     * SO_LINGER is special because it needs to use a special
                     * "linger" struct as well as use the incoming boolean
                     * argument specially.
                     */
            struct linger lingr;
            lingr.l_onoff = boolValue ? 1 : 0; // Force it to be 0 or 1.
            lingr.l_linger = intValue;
            ret = setsockopt(m_socketFd, level, optname, &lingr, sizeof(lingr));
        }
        break;

        case SO_SNDTIMEO: {
            /*
                     * SO_SNDTIMEO is supposed to set both send and receive timeouts.
                     * Note: The incoming timeout value is in milliseconds.
                     */
            struct timeval timeout;
            timeout.tv_sec = intValue / 1000;
            timeout.tv_usec = (intValue % 1000) * 1000;

            ret = setsockopt(m_socketFd, SOL_SOCKET, SO_RCVTIMEO,
                    (void *)&timeout, sizeof(timeout));

            if (ret == 0) {
                ret = setsockopt(m_socketFd, SOL_SOCKET, SO_SNDTIMEO,
                        (void *)&timeout, sizeof(timeout));
            }
        }
        break;

        default: {
            /*
                     * In all other cases, the translated option level and
                     * optname may be used directly for a call to setsockopt().
                     */
            ret = setsockopt(m_socketFd, level, optname, &intValue, sizeof(intValue));
        }
        break;
    }

    if (ret != 0) {
        setLastError(errno);
    }

    return ret;
}

bool CLocalSocket::getOptionLevel(int optname, int &level)
{
    bool ret = true;

    switch (optname) {
        case SO_RCVBUF:
            level = SOL_SOCKET;
            break;

        case SO_SNDBUF:
            level = SOL_SOCKET;
            break;

        case SO_SNDTIMEO:
            level = SOL_SOCKET;
            break;

        case SO_LINGER:
            level = SOL_SOCKET;
            break;

        case TCP_NODELAY:
            level = IPPROTO_TCP;
            break;

        case SO_REUSEADDR:
            level = SOL_SOCKET;
            break;

        default:
            ret = false;
            break;
    }

    return ret;
}

int CLocalSocket::sendFileDescriptor(const void* buf, int len, int send_fd)
{
    ssize_t ret;
    struct msghdr msg;
    unsigned char *buffer = (unsigned char *)buf;
    memset(&msg, 0, sizeof(msg));

    struct cmsghdr *cmsg;
    char msgbuf[CMSG_SPACE(1)];

    if(m_socketFd == -1 || send_fd == -1) {
        UTILS_LOGE(TAG, "fd: %d, send_fd: %d", m_socketFd, send_fd);
        return -1;
    }

    msg.msg_name = &m_peerAddress;
    msg.msg_namelen = m_peerAddressLen;
    msg.msg_control = msgbuf;
    msg.msg_controllen = sizeof msgbuf;
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof send_fd);
    memcpy(CMSG_DATA(cmsg), &send_fd, sizeof send_fd);

    // We only write our msg_control during the first write
    int ret_len = len;
    while (len > 0) {
        struct iovec iv;
        memset(&iv, 0, sizeof(iv));

        iv.iov_base = buffer;
        iv.iov_len = len;

        msg.msg_iov = &iv;
        msg.msg_iovlen = 1;

        do {
            ret = sendmsg(m_socketFd, &msg, MSG_NOSIGNAL);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            UTILS_LOGI(TAG, "fd:%d, send_fd:%d, sendmsg ret:%d, errno:%d, %s",
                    m_socketFd, send_fd, (int)ret, errno, strerror(errno));
            ret_len = -1;
            break;
        }

        buffer += ret;
        len -= ret;

        // Wipes out any msg_control too
        memset(&msg, 0, sizeof(msg));
    }

    close(send_fd);

    return ret_len;
}

/**
 * Reads data from a socket into buffer, processing any ancillary data.
 *
 * Returns the length of normal data read, or -1 if has error in this function.
 */
int CLocalSocket::readAll(void *buffer, int length)
{
    ssize_t ret = 0;
    struct msghdr msg;
    struct iovec iv;
    unsigned char *buf = (unsigned char *)buffer;
    // Enough buffer for a pile of fd's. We return -1 if this buffer is too small.
    struct cmsghdr cmsgbuf[2*sizeof(cmsghdr) + 0x100];

    memset(&msg, 0, sizeof(msg));
    memset(&iv, 0, sizeof(iv));

    iv.iov_base = buf;
    iv.iov_len = length;

    msg.msg_iov = &iv;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);

    do {
        ret = recvmsg(m_socketFd, &msg, MSG_NOSIGNAL);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0 && errno == EPIPE) {
        // Treat this as an end of stream
        return 0;
    }

    if (ret < 0) {
        setLastError(errno);
        return -1;
    }

    if ((msg.msg_flags & (MSG_CTRUNC | MSG_OOB | MSG_ERRQUEUE)) != 0) {
        // To us, any of the above flags are a fatal error
        return -1;
    }

    if (ret >= 0) {
        processCmsg(&msg);
    }

    return (int)ret;
}

/**
 * Writes all the data in the specified buffer to the specified socket.
 *
 * Returns the length of normal data write on success or -1 if has error.
 */
int CLocalSocket::writeAll(const void *buffer, int length)
{
    ssize_t ret = 0;
    struct msghdr msg;
    unsigned char *buf = (unsigned char *)buffer;
    memset(&msg, 0, sizeof(msg));

    // We only write our msg_control during the first write
    while (length > 0) {
        struct iovec iv;
        memset(&iv, 0, sizeof(iv));

        iv.iov_base = buf;
        iv.iov_len = length;

        msg.msg_iov = &iv;
        msg.msg_iovlen = 1;

        do {
            ret = sendmsg(m_socketFd, &msg, MSG_NOSIGNAL);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            setLastError(errno);
            return -1;
        }

        buf += ret;
        length -= ret;

        // Wipes out any msg_control too
        memset(&msg, 0, sizeof(msg));
    }

    return length;
}

/**
 * Processes ancillary data, handling only SCM_RIGHTS.
 * Creates appropriate m_inboundFds and set the received value to  m_inboundFds.
 *
 * Returns 0 on success or -1 if has error.
 */
int CLocalSocket::processCmsg(struct msghdr *pMsg)
{
    struct cmsghdr *cmsgptr = NULL;

    for (cmsgptr = CMSG_FIRSTHDR(pMsg);
         cmsgptr != NULL;
         cmsgptr = CMSG_NXTHDR(pMsg, cmsgptr)) {

        if (cmsgptr->cmsg_level != SOL_SOCKET) {
            continue;
        }

        if (cmsgptr->cmsg_type == SCM_RIGHTS) {
            int *pDescriptors = (int *)CMSG_DATA(cmsgptr);
            int count = ((cmsgptr->cmsg_len - CMSG_LEN(0)) / sizeof(int));

            if (count < 0) {
                return -1;
            }

            clear();
            setAncillaryFileDescriptors(pDescriptors, count);
        }
    }

    return 0;
}

void CLocalSocket::clear()
{
    SAFE_DELETE_ARRAY(m_inboundFds);
    m_inboundFdsLen = 0;
}

bool CLocalSocket::setAncillaryFileDescriptors(int *buf, int len)
{
    bool ret = false;

    clear();
    m_inboundFds = new int[len];
    if (NULL != m_inboundFds && NULL != buf) {
        memset(m_inboundFds, 0, sizeof(int) * (len));
        memcpy(m_inboundFds, buf, (len * sizeof(int)));
        m_inboundFdsLen = len;
        ret = true;
    }

    return ret;
}


}

