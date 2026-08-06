#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "clog.h"
#include "cserversocket.h"

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp)            \
  ({                                       \
    decltype(exp) _rc;                     \
    do {                                   \
      _rc = (exp);                         \
    } while (_rc == -1 && errno == EINTR); \
    _rc;                                   \
  })
#endif

namespace universal_utils {

static const char *TAG = "CServerSocket";
static const bool DBG = true;

CServerSocket::CServerSocket(int port) : port(port) {
    // create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        UTILS_LOGW(TAG, "socket failed");
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // initialize address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // create pipe for stop signal
    if (pipe(stop_pipe) == -1) {
        UTILS_LOGW(TAG, "pipe failed");
    }

    // set pipe to non-blocking
    fcntl(stop_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(stop_pipe[1], F_SETFL, O_NONBLOCK);
}

CServerSocket::~CServerSocket() {
    close(stop_pipe[0]);
    close(stop_pipe[1]);
    close(server_fd);
}

void CServerSocket::setReuseAddress(bool reuse) {
    int opt = reuse ? 1 : 0;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        UTILS_LOGW(TAG, "setsockopt SO_REUSEADDR failed");
        close(server_fd);
    }
}

void CServerSocket::setReceiveBufferSize(int size) {
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1) {
        UTILS_LOGW(TAG, "setsockopt SO_RCVBUF failed");
        close(server_fd);
    }
}

void CServerSocket::setSoTimeout(int seconds, int microseconds) {
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = microseconds;
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        UTILS_LOGW(TAG, "setsockopt SO_RCVTIMEO failed");
        close(server_fd);
    }
}

void CServerSocket::setTcpNoDelay(bool noDelay) {
    int opt = noDelay ? 1 : 0;
    if (setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1) {
        UTILS_LOGW(TAG, "setsockopt TCP_NODELAY failed");
        close(server_fd);
    }
}
bool CServerSocket::bindSocket() {
    bool ret = false;

    // bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        UTILS_LOGW(TAG, "bind failed");
        close(server_fd);
        return ret;
    }

    // listen for connections
    if (listen(server_fd, 3) == -1) {
        UTILS_LOGW(TAG, "listen failed");
        close(server_fd);
        return ret;
    }

    if (DBG) UTILS_LOGD(TAG, "Server is listening on port %d", port);

    ret = true;
    return ret;
}

int CServerSocket::start() {
    int client_fd = -1;

    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        FD_SET(stop_pipe[0], &readfds);

        int max_fd = std::max(server_fd, stop_pipe[0]);
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            UTILS_LOGW(TAG, "select activity(%d) error(%d)", activity, errno);
            close(server_fd);
            break;
        }

        if (FD_ISSET(stop_pipe[0], &readfds)) {
            if (DBG) UTILS_LOGD(TAG, "Stop signal received, shutting down...");
            break;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int addrlen = sizeof(address);
            client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
            if (client_fd == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                    if (DBG) UTILS_LOGD(TAG, "Accept timed out, retrying...");
                    continue; // timeout occurred, continue to wait for new connections
                } else {
                    UTILS_LOGW(TAG, "accept failed %d", errno);
                    close(server_fd);
                    break;
                }
            } else {
                break;
            }
        }
    }

    return client_fd;
}

void CServerSocket::stop() {
    const char *stop_msg = "stop";

    if (TEMP_FAILURE_RETRY(write(stop_pipe[1], stop_msg, strlen(stop_msg))) == -1) {
        UTILS_LOGW(TAG, "write to stop_pipe failed");
    }
}

}


