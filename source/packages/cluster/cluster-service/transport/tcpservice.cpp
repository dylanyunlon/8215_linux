#include "tcpservice.h"
#include "transport.h"
#include <math.h>
#include <sys/eventfd.h>
const static char *TAG = "TcpSocket";
const static  uint64_t BREAKE_SELECT_CMD = 1;
const static int MAX_LINSTEN_NUM = 30;

//using universal_utils::CLog;
TcpService::TcpService(ConnectionCallback *clientConnection)
    : m_connection(clientConnection)
{

}

TcpService::~TcpService()
{
    //UTILS_LOGD(TAG, "~TcpService()");
    stop();
}

void TcpService::start()
{
    m_run = true;
    threadStart();
}

void TcpService::stop()
{
    m_run = false;
    breakSelect();
    threadStop();
}

void TcpService::threadRun()
{
    int ret = init();
    while (ret > 0 && m_run) {
        select();
    }
    deinit();
}

int TcpService::init()
{
    int ret = -1;
    struct sockaddr_in serverAddrss;
    serverAddrss.sin_family = AF_INET;
    serverAddrss.sin_port = htons(SERVICE_PORT);
    serverAddrss.sin_addr.s_addr = htonl(INADDR_ANY);
    m_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverFd < 0) {
      //  UTILS_LOGE(TAG, "socket error %s", strerror(errno));
        return ret;
    }


    m_eventfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (m_eventfd < 0) {
       // UTILS_LOGE(TAG, "eventfd error %s", strerror(errno));
        return ret;
    }

    int on = 1;
    if (setsockopt(m_serverFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int))) {
      //  UTILS_LOGE(TAG, "setsockopt error %d %s", on, strerror(errno));
        return ret;
    }

    if ((ret = bind(m_serverFd, (struct sockaddr*)&serverAddrss, sizeof(serverAddrss))) < 0) {
        //UTILS_LOGE(TAG, "bind error %s", strerror(errno));
        close(m_serverFd);
        return ret;
    }

    if ((ret = listen(m_serverFd, MAX_LINSTEN_NUM) < 0)) {
        //UTILS_LOGE(TAG, "bind error %s", strerror(errno));
        close(m_serverFd);
        return ret;
    }

    //UTILS_LOGD(TAG, "socket init finished");

    return m_serverFd;
}

void TcpService::deinit()
{
   // UTILS_LOGD(TAG, "deinit()");
    if (m_serverFd > 0) {
        close(m_serverFd);
        m_serverFd = -1;
    }

    if (m_eventfd > 0) {
        close(m_eventfd);
        m_eventfd = -1;
    }
}

void TcpService::select()
{
    int count = 0;
    fd_set fdsets;
    FD_ZERO(&fdsets);
    FD_SET(m_serverFd, &fdsets);
    FD_SET(m_eventfd, &fdsets);

    int maxFd = std::max(m_serverFd, m_eventfd);

    do {
        count = ::select(maxFd + 1, &fdsets, nullptr, nullptr, nullptr);
        if (count > 0 && FD_ISSET(m_serverFd, &fdsets)) {
            int clientFd = accept();
          //  UTILS_LOGD(TAG, "select accpet fd %d", clientFd);
            if (clientFd > 0 ) {
                m_connection->onClientConnected(clientFd);
            }
            --count;
        }

        if (count > 0 && FD_ISSET(m_eventfd, &fdsets)) {
            uint64_t cmd = 0;
            ::read(m_eventfd, &cmd, sizeof(cmd));
            --count;
            if (cmd == BREAKE_SELECT_CMD) {
               // UTILS_LOGD(TAG, "select break by eventfd cmd %d", cmd);
                break;
            }
        }
    } while (count < 0 && errno == EINTR);
}

void TcpService::breakSelect()
{
    if (::write(m_eventfd, &BREAKE_SELECT_CMD, sizeof(BREAKE_SELECT_CMD)) != sizeof(BREAKE_SELECT_CMD)) {
        //UTILS_LOGE(TAG, "write eventfd breakSelect failed");
    }
}

int TcpService::accept()
{
    int fd = -1;
    sockaddr_in clientAddress;
    socklen_t len = sizeof(clientAddress);
    do {
        fd = ::accept(m_serverFd, (struct sockaddr*)&clientAddress, &len);
    } while (fd < 0 && (errno == ECONNABORTED || errno == EINTR)); //被中断后重新测试

    if (fd < 0) {
        //UTILS_LOGE(TAG, "accept error %s", strerror(errno));
    } else {
        char clientIP[1024];
        //UTILS_LOGD(TAG, "accept client ip: %s, port: %d, fd: %d",  inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, clientIP, sizeof(clientIP)),
           //        ntohs(clientAddress.sin_port), fd);
    }

    return fd;
}
