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

#ifndef __CLOCALSOCKET_H
#define __CLOCALSOCKET_H

#include <string>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>

namespace universal_utils {

class CLocalSocket
{
public:
    CLocalSocket();
    ~CLocalSocket();

    int createSocket (int sockType);
    void closeSocket();

    int bindSocket();
    int bindSocket(const std::string &localAddress);

    int connectSocket();
    int connectSocket(const std::string &peerAddress);

    int pending();
    int available();
    int shutdownSocket(bool input);

    int read();
    int read(char data[], int offset, int length);

    int write(const int data);
    int write(const char data[], int offset, int length);

    void setFileDescriptor(int fd);
    int getFileDescriptor() const;

    void getAncillaryFileDescriptors(int* fds, int len);
    int getAncillaryFdsLength() const;

    void setLastError(int error);
    int getLastError () const;

    int setLocalAddress(const std::string &localAddress);
    const struct sockaddr_un *getLocalAddress() const;

    int setPeerAddress(const std::string &peerAddress);
    int setPeerAddress(struct sockaddr_un *addr);
    const struct sockaddr_un *getPeerAddress() const;

    int setReceiveBufferSize(int size);
    int getReceiveBufferSize();

    int setSendBufferSize(int size);
    int getSendBufferSize();

    int setSoTimeout(int timeoutMillis);
    int getSoTimeout();

    int setReuseAddress(bool reuse);
    bool getReuseAddress();

    int sendFileDescriptor(const void* buf, int len, int send_fd);

protected:

private:
    CLocalSocket (const CLocalSocket &other);
    CLocalSocket& operator=(const CLocalSocket &socket);

    int getOption(int optID);
    int setOption(int optID, int boolValue, int intValue);
    bool getOptionLevel(int optname, int &level);

    int readAll(void *buffer, int length);
    int writeAll(const void *buf, int len);
    int processCmsg(struct msghdr * pMsg);

    void clear();
    bool setAncillaryFileDescriptors(int *buf, int len);

    int m_socketFd;
    int m_error;

    struct sockaddr_un m_localAddress;
    unsigned int m_localAddressLen;

    struct sockaddr_un m_peerAddress;
    unsigned int m_peerAddressLen;

    int *m_inboundFds;
    int m_inboundFdsLen;
};

}

#endif // __CLOCALSOCKET_H