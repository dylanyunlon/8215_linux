#include "transport.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <stddef.h>
const static char *TAG = "Transport";
//using universal_utils::CLog;
using namespace std;
Transport::Transport(int fd)
    : m_fd(fd)
{
    m_run = true;
    struct timeval timeout = {0, 500000}; //500ms
    setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

CharArray Transport::read()
{
    const int HEAD_LENGTH_SIZE = 3;
    char headerBuffer[HEAD_LENGTH_SIZE];
    if (read(headerBuffer, HEAD_LENGTH_SIZE) <= 0) {
       // UTILS_LOGE(TAG, "read data len error");
        return CharArray();
    }

    int dataLen = 0;
    memcpy(&dataLen, headerBuffer + 1, 2);
    int len = dataLen - HEAD_LENGTH_SIZE;
    if (len > 0) {
        CharArray dataBuffer(HEAD_LENGTH_SIZE + len);
        if (read(dataBuffer.getData() + HEAD_LENGTH_SIZE, len) > 0) {
            dataBuffer.copyData(headerBuffer, 0, HEAD_LENGTH_SIZE);
            return dataBuffer;
        }
    }

    //UTILS_LOGE(TAG, "read data error");
    return CharArray();
}

int Transport::read(char *data, std::size_t len)
{
    std::size_t nread = 0;
    std::size_t nleft = len;
    while (m_run && nleft > 0) {
        if ((nread = ::read(m_fd, data + len - nleft, nleft)) < 0) {
            if (errno == EINTR || errno == EAGAIN)
                nread = 0;
            else {
              //  UTILS_LOGE(TAG, "read error nread %d", nread);
                notifyTransforError();
                return -1;
            }
        } else if (nread == 0) { //对方关闭，应该通知断开过程
            //UTILS_LOGE(TAG, "read error nread = 0");
            notifyTransforError();
            return -1;
        }
        nleft -= nread;
    }

    return len - nleft;
}

int Transport::write(char *data, std::size_t len)
{
    //UTILS_LOGD(TAG, "write ");

    ssize_t nleft = len;
    ssize_t nwriten = 0;
    while (nleft > 0) {
        if ((nwriten = ::write(m_fd, data + len - nleft, nleft)) <= 0) {
            if (nwriten < 0 && errno == EINTR)
                nwriten = 0;
            else {
               // UTILS_LOGE(TAG, "write error nwriten = %d, fd=%d error=%s", nwriten, m_fd, strerror(errno));
                notifyTransforError();
                return -1;
            }
        }
        nleft -= nwriten;
    }

    return len;
}

void Transport::close()
{
    m_run = false;
    ::close(m_fd);
}

void Transport::notifyTransforError()
{
    //close(m_fd);
    if (m_callback)
        m_callback->onTransportError(m_fd);
}

int Transport::write(const CharArray &array)
{
    return write(array.getData(), array.getLength());
}
