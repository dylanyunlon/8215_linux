#include <sys/types.h>
#include <sys/socket.h>

#include <sys/un.h>
#include <pthread.h>
#include <signal.h>

#include <stddef.h>

#include "ipc/socket/SocketServer.hpp"
#include "ipc/socket/SocketProto.hpp"
#include "utils/Util.hpp"
#include "ipc/IPC.hpp"

namespace atcupdateservice {
namespace ipc {
namespace socket {

static int setTimedout(int fd, uint32_t ms, bool isRead) {
    if (fd < 0) {
        return -EBADF;
    }
    int type = isRead ? SO_RCVTIMEO : SO_SNDTIMEO;
    struct timeval tv;
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, type, &tv, sizeof(tv)) < 0) {
        return -1;
    }
    return 0;
}

SocketServer::SocketServer()
    : m_sockfd(-1), m_stop(false) {
    m_sleeper.reset(new utils::Sleeper());
}

bool SocketServer::init() {
    //ignore the SIGPIPE signal, which would cause server crash when client quit abonormally
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    struct sockaddr_un addr;
    size_t addrlen;
    if (pthread_sigmask(SIG_BLOCK, &set, NULL)) {
        ATC_STREAM_LOGC() << "failed to block SIGPIPE signal" << std::endl;
        goto out_err;
    }

    if (access(UNIX_SOCKETADDR, F_OK) == 0) {
        unlink(UNIX_SOCKETADDR);
    }
    m_sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_sockfd < 0) {
        ATC_STREAM_LOGC() << "failed to create socket: " << strerror(errno) << std::endl;
        goto out_err;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_SOCKETADDR, strlen(UNIX_SOCKETADDR));
    addrlen = offsetof(sockaddr_un, sun_path) + strlen(UNIX_SOCKETADDR);
    if (bind(m_sockfd, (struct sockaddr*)&addr, addrlen) < 0) {
        ATC_STREAM_LOGC() <<  "failed to bind unix addr: " << strerror(errno) << std::endl;
        goto out_err;
    }
    if (listen(m_sockfd, SOMAXCONN) < 0) {
        ATC_STREAM_LOGC() << "failed to set listen backlog: " << strerror(errno) << std::endl;
        goto out_err;
    }
    //set accept timedout
    if (setTimedout(m_sockfd, 1000, 1) < 0) {
        ATC_STREAM_LOGC() << "failed to set receive timedout: " << strerror(errno) << std::endl;
        goto out_err;
    }
    ATC_STREAM_LOGI() << "socket setup succeed!!!" << std::endl;
    return true;
out_err:
    if (m_sockfd < 0) {
        close(m_sockfd);
    }
    return false;
}

void SocketServer::run() {
    do {
        Session::ptr session = handleAccept();
        if (session == nullptr) {
            ATC_STREAM_LOGE() << "failed to accept!" << std::endl;
            continue;
        }
        if (handleSession(session) == Session::KEEPALIVE) {
            ATC_STREAM_LOGD() << "new subscriber!!!" << std::endl;
            addSubscriber(session);
        } else {
            session->close();
        }
    } while(!m_stop);
}

bool SocketServer::start() {
    if (init() == false) {
        return false;
    }
    m_stop = false;
    m_mainThread.reset(new std::thread(std::bind(&SocketServer::run, this)));
    if (m_mainThread == nullptr) {
        m_stop = true;
        return false;
    }
    m_broadcastThread.reset(new std::thread(std::bind(&SocketServer::broadcastThread, this)));
    if (m_broadcastThread == nullptr) {
        m_stop = true;
        return false;
    }
    return true;
}

//handle new client connection
Session::ptr SocketServer::handleAccept() {
    while (!m_stop) {
        int fd = accept(m_sockfd, nullptr, nullptr);
        if (fd < 0) {
            if (errno == EINTR || errno == ETIMEDOUT ||
                errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            ATC_STREAM_LOGW() << "SocketIPC accept failed: " << strerror(errno) << std::endl;
            return nullptr;
        } else {
            Session::ptr session(new Session(fd));
            return session;
        }
    };
    return nullptr;
}

Session::Type SocketServer::handleSession(Session::ptr session) {
    Session::Type  type = Session::ONESHOT;
    if (session == nullptr) {
        ATC_STREAM_LOGE() << "empty session!!" << std::endl;
        return type;
    }
    ByteArray::ptr resp;
    ProtocolHeader header;
    ByteArray::ptr ba = session->recv();
    if (ba == nullptr) {
        ATC_STREAM_LOGE() << "recv request type failed!" << std::endl;
        return Session::ONESHOT;
    }
    memset(&header, 0, sizeof(header));
    ba->seek(0);
    if (ProtocolHeader::verifyPackage(ba, REQUEST_TYPE, (uint32_t)-1) == false) {
        ATC_STREAM_LOGE() << "verify package failed!" << std::endl;
        resp = make_response((uint32_t)-1, -SOCKET_BROKEN_PACKAGE);
        goto out;
    }
    memcpy((uint8_t*)&header, ba->getBufferRef(), sizeof(header));
    ATC_STREAM_LOGI() << "handling operation: " << header.op << std::endl;
    if (header.op == Operations::SUBSCRIBE) {
        resp = make_response(header.op, SOCKET_OK);
        type = Session::KEEPALIVE;
    } else {
        auto iter = m_handlers.find(header.op);
        if (iter == m_handlers.end()) {
            resp = make_response(header.op, -SOCKET_NOOP);
        } else {
            ba->seek(sizeof(ProtocolHeader));
            resp = iter->second(ba);
        }
    }
out:
    if (resp == nullptr) {
        ATC_STREAM_LOGE() << "failed to response client, failed to allocate buffer!" << std::endl;
    } else {
        session->send(resp);
    }
    return type;
}

SocketServer::~SocketServer() {
    if (m_mainThread && m_mainThread->joinable()) {
        m_mainThread->join();
    }
    if (m_broadcastThread && m_broadcastThread->joinable()) {
        m_broadcastThread->join();
    }
    {
        std::lock_guard<std::mutex> lg(m_mx);
        for (auto client : m_subscribers) {
            client->close();
        }
        m_subscribers.clear();
    }
    close(m_sockfd);
}

void SocketServer::broadcast(ByteArray::ptr ba) {
    m_broadcastQueue.push(ba);
    m_sleeper->wakeUp();
}

void SocketServer::broadcastThread() {
    while (!m_stop) {
        ByteArray::ptr ba;
        while (m_broadcastQueue.front(&ba)) {
            if (ba == nullptr) {
                continue;
            }
            std::vector<Session::ptr> deadSessions;
            std::set<Session::ptr> sessions;
            {
                std::lock_guard<std::mutex> lg(m_mx);
                sessions = m_subscribers;
            }
            for (Session::ptr session: sessions) {
                if (session->send(ba) < 0) {
                    deadSessions.push_back(session);
                }
            }
            {
                std::lock_guard<std::mutex> lg(m_mx);
                for (uint32_t i = 0; i < deadSessions.size(); ++i) {
                    m_subscribers.erase(deadSessions[i]);
                }
            }
            ba.reset();
        }
        m_sleeper->sleepFor(5000);
    }
}

Session::Session(int fd)
    : m_fd(fd) {
    if (setTimedout(m_fd, 1000, 1) < 0) {
        ATC_STREAM_LOGE() << "failed to set read timedout: " << strerror(errno) << std::endl;
        ::close(m_fd);
        m_fd = -1;
    }
    if (setTimedout(m_fd, 1000, 0) < 0) {
        ATC_STREAM_LOGE() << "failed to set write timedout: " << strerror(errno) << std::endl;
        ::close(m_fd);
        m_fd = -1;
    }
}

//never close a session during read or write!!!
void Session::close() {
    if (m_fd < 0) return;
    ::close(m_fd);
    m_fd = -1;
}

int Session::send(ByteArray::ptr ba) {
    if (m_fd < 0) {
        errno = EBADFD;
        return -1;
    }
    const uint8_t *resp = ba->getBufferRef();
    uint32_t size = ba->size();
    uint32_t nwrite = 0;
    int rt = 0;
    while (nwrite < size) {
        rt = write(m_fd, resp + nwrite, size - nwrite);
        if (rt <= 0) {
            if (errno == EPIPE) {
                ATC_STREAM_LOGW() << "client exit, session would be closed" << std::endl;
            } else {
                ATC_STREAM_LOGW() << "socket error: " << strerror(errno) << std::endl;
            }
            return -1;
        }
        nwrite += rt;
    }

    return 0;
}

ByteArray::ptr Session::recv() {
    if (m_fd < 0) {
        errno = EBADFD;
        return nullptr;
    }
    ByteArray::ptr ba(new ByteArray());
    ProtocolHeader header;
    uint32_t bodySize = 0;

    if (readFix((uint8_t*)&header, sizeof(header)) < 0) {
        ATC_STREAM_LOGE() << "read header failed: " << strerror(errno) << std::endl;
        return nullptr;
    }
    if (header.size > BYTEARRAY_MAX_SIZE || header.size < sizeof(header)) {
        ATC_STREAM_LOGE() << "invalid header size: " << header.size << std::endl;
        return nullptr;
    }
    ba->writeBinary((uint8_t*)&header, sizeof(header));
    bodySize = header.size - sizeof(header);
    if (bodySize == 0) {
        return ba;
    }
    std::shared_ptr<uint8_t> buf((uint8_t*)malloc(bodySize), [] (uint8_t *ptr) {
        if (ptr) free(ptr);
    });
    if (readFix(buf.get(), bodySize) < 0) {
        return nullptr;
    }
    ba->writeBinary(buf.get(), bodySize);

    return ba;
}

int Session::readFix(uint8_t *buf, uint32_t size) {
    if (m_fd < 0) {
        errno = EBADFD;
        return -1;
    }
    uint32_t nread = 0;
    while (nread < size) {
        int rt = ::read(m_fd, buf + nread, size - nread);
        if (rt < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                continue;
            }
            ATC_STREAM_LOGW() << "readFix failed: " << strerror(errno) << ", rt=" << rt << std::endl;
            return -1;
        } else if (rt == 0) {
            ATC_STREAM_LOGW() << "readFix failed: connection closed, rt=0" << std::endl;
            return -1;
        }
        nread += rt;
    }

    return nread;
}

// read is not used in SUBSCRIBE mode, so use this to achieve heartbeat detect
void Session::heartbeatDetector() {
    while (1) {
        // read block at most 4s
        setTimedout(m_fd, 4000, 1);
        ByteArray::ptr ba = recv();
        if (ba == nullptr) {
            ATC_STREAM_LOGI() << "failed to receive heart beat package, disconnecting session..." << std::endl;
            break;
        }
        if (ProtocolHeader::verifyPackage(ba, REQUEST_TYPE, HEARTBEAT) == false) {
            ATC_STREAM_LOGW() << "receive a invalid heart beat package, disconnecting session..." << std::endl;
            break;
        }
    }
    SocketServer::ptr server = SocketServer::Instance::getInstance();
    server->removeSubscriber(shared_from_this());
    ATC_STREAM_LOGI() << "session removed!" << std::endl;
}

bool Session::startHeartBeatDetector() {
    m_heartbeatDetector.reset(new std::thread(&Session::heartbeatDetector, shared_from_this()));
    m_heartbeatDetector->detach();
    if (m_heartbeatDetector) {
        return true;
    } else {
        return false;
    }
}

}
}
}