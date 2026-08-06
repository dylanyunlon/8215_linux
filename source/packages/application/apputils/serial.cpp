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

#include <poll.h>
#include "applog.h"
#include "serial.h"
#include <errno.h>

using namespace universal_utils;

static const char TAG[] = "CSerial";
#define MIN_BUF_ARRAY_SIZE 2

static speed_t getBaudrate(int baudrate)
{
    speed_t speed = B0;

    switch(baudrate) {
    case 0: speed = B0; break;
    case 50: speed = B50; break;
    case 75: speed = B75; break;
    case 110: speed = B110; break;
    case 134: speed = B134; break;
    case 150: speed = B150; break;
    case 200: speed = B200; break;
    case 300: speed = B300; break;
    case 600: speed = B600; break;
    case 1200: speed = B1200; break;
    case 1800: speed = B1800; break;
    case 2400: speed = B2400; break;
    case 4800: speed = B4800; break;
    case 9600: speed = B9600; break;
    case 19200: speed = B19200; break;
    case 38400: speed = B38400; break;
    case 57600: speed = B57600; break;
    case 115200: speed = B115200; break;
    case 230400: speed = B230400; break;
    case 460800: speed = B460800; break;
    case 500000: speed = B500000; break;
    case 576000: speed = B576000; break;
    case 921600: speed = B921600; break;
    case 1000000: speed = B1000000; break;
    case 1152000: speed = B1152000; break;
    case 1500000: speed = B1500000; break;
    case 2000000: speed = B2000000; break;
    case 2500000: speed = B2500000; break;
    case 3000000: speed = B3000000; break;
    case 3500000: speed = B3500000; break;
    case 4000000: speed = B4000000; break;
    default: speed = -1; break;
    }

    return speed;
}


int setUartOption(int fd, speed_t speed, int bits, char parity, int stop)
{
    struct termios newtio, oldtio;

    if (tcgetattr(fd, &oldtio) !=  0) {
        LOGE(__func__, "tcgetattr fail: %s\n", strerror(errno));
        return -1;
    }

    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag  |=  CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch (bits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch (parity)
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }

    cfsetispeed(&newtio, speed);
    cfsetospeed(&newtio, speed);

    if (stop == 1) {
        newtio.c_cflag &=  ~CSTOPB;
    } else if (stop == 2) {
        newtio.c_cflag |=  CSTOPB;
    }

    newtio.c_cc[VTIME]  = 0;
    newtio.c_cc[VMIN] = 0;

    tcflush(fd,TCIFLUSH);

    if((tcsetattr(fd,TCSANOW,&newtio)) != 0) {
        LOGD(__func__, "com set error: %s\n", strerror(errno));
        return -1;
    }
    LOGD(__func__, "set uart success %d %d %d %d\r\n", speed, bits, parity, stop);
    return 0;
}

CSerial::CSerial(CSerialObject *pObject,
                    PSERIAL_DATA_CALLBACK_FUNC pDataFunc,
                    unsigned int bufLen)
    : CAutoThread()
    , m_bufLen(bufLen)
    , m_comDev_fd(-1)
    , m_pBuf(0)
{
    if (m_bufLen < MIN_BUF_ARRAY_SIZE) {
        m_bufLen = MIN_BUF_ARRAY_SIZE;
    }

    m_pBuf = new unsigned char[m_bufLen];
    if (m_pBuf) {
        memset(m_pBuf, 0, m_bufLen);
    }

    initSerial(pObject, pDataFunc);
}

CSerial::~CSerial(void)
{
    closeSerial();
    SAFE_DELETE_ARRAY(m_pBuf);
}

bool CSerial::initSerial(CSerialObject *pObject,
                        PSERIAL_DATA_CALLBACK_FUNC pDataFunc)
{
    m_pDataFunc = pDataFunc;
    m_pObject = pObject;

    return true;
}

//"/dev/ttyS0"
bool CSerial::openSerial(const char *devPort,
                            unsigned int baudRate,
                            unsigned char byteSize,
                            unsigned char parity,
                            unsigned char stopBits)
{
    bool bRet = false;

    if (-1 != m_comDev_fd) {
        closeSerial();
    }

    m_comDev_fd = open(devPort, O_RDWR | O_NOCTTY | O_NDELAY);
    if (-1 == m_comDev_fd) {
        LOGE(TAG, "Can't Open Serial Port: %s failed\n", devPort);
    } else {
        LOGD(TAG, "open serial port: %s success\n", devPort);
        speed_t speed = getBaudrate(baudRate);
        setUartOption(m_comDev_fd, speed, byteSize, parity, (int)stopBits);

        bRet = true;
    }

    return bRet;
}

bool CSerial::closeSerial()
{
    bool bRet = false;

    threadStop();

    if (m_comDev_fd != -1) {
        close(m_comDev_fd);
        m_comDev_fd = -1;
    }

    return bRet;
}

bool CSerial::writeData(const void *pBuf, unsigned int bufLen)
{
    bool bRet = false;

    if (pBuf && bufLen && m_comDev_fd) {
        unsigned int writtenBytes = 0;
        writtenBytes = write(m_comDev_fd, pBuf, bufLen);
        if (writtenBytes == bufLen) {
            bRet = true;
        } else {
            LOGE(TAG, "write fail: %s\n", strerror(errno));
        }
    }

    return bRet;
}

bool CSerial::isOpen() const
{
    return -1 != m_comDev_fd;
}


unsigned long CSerial::threadRun()
{
    while (!isTerminated()
            && NULL != m_pBuf
            && NULL != m_pObject
            && -1 != m_comDev_fd
            && NULL != m_pDataFunc) {
        int res = -1;
        struct pollfd readfd;
        memset(&readfd, 0, sizeof(readfd));

        readfd.fd = m_comDev_fd;
        readfd.events = POLLIN;

        res = poll(&readfd, 1, -1);
        if (res > 0) {
            unsigned int readBytes = 0;

            readBytes = read(m_comDev_fd, m_pBuf, m_bufLen);
            if (readBytes > 0) {
                (m_pObject->*m_pDataFunc)(m_pBuf, readBytes);
            }
        } else if (0 == res) {
            LOGE(TAG, "poll timeout\n");
        } else {
            LOGE(TAG, "poll fail: %s\n", strerror(errno));
        }
    }

    return 0;
}

