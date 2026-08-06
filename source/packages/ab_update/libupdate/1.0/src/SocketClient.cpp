#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>

#include "ipc/socket/SocketProto.hpp"
#include "utils/macro.hpp"
#include "SocketClient.hpp"

#include "private/Tag.hpp"

namespace atcupdateservice {

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

static int setNonb(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

bool SocketConnection::connect() {
    struct sockaddr_un addr;
    int retry = 3;
    size_t addrlen;
    if (m_fd == -1) {
        m_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (m_fd < 0) {
            ATC_STREAM_LOGE() << "socket create failed!" << std::endl;
            return false;
        }
        if (setTimedout(m_fd, 1000, 1) < 0) {
            ATC_STREAM_LOGE() << "set read timedout failed!" << std::endl;
            return false;
        }
        if (setTimedout(m_fd, 1000, 0) < 0) {
            ATC_STREAM_LOGE() << "set write timedout failed!" << std::endl;
            return false;
        }
    }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_SOCKETADDR, strlen(UNIX_SOCKETADDR));
    addrlen = offsetof(sockaddr_un, sun_path) + strlen(UNIX_SOCKETADDR);
    while (retry--) {
        if (::connect(m_fd, (struct sockaddr*)&addr, addrlen) < 0) {
            ATC_STREAM_LOGE() << "socket connect failed(" << retry << "): " << strerror(errno) << std::endl;
            continue;
        }
        if (subscribe() < 0) {
            disconnect();
            ATC_STREAM_LOGE() << "subscribe broadcast failed, " << std::endl;
            return false;
        }
        ATC_STREAM_LOGI() << "connection established!" << std::endl;
        return true;
    }
    // send subscribe package
    return false;
}

int SocketConnection::subscribe() {
    ByteArray::ptr ba = make_request(SUBSCRIBE, 0);
    ProtocolHeader header;
    memset(&header, 0, sizeof(header));
    if (writeFix(m_fd, ba->getBufferRef(), ba->size()) < 0) {
        ATC_STREAM_LOGE() << "send broadcast package failed!!!" << std::endl;
        return -1;
    }
    ba = recv(m_fd);
    if (ba == nullptr) {
        ATC_STREAM_LOGE() << "failed to recv subscribe response package!!!" << std::endl;
        return -1;
    }
    ba->seek(0);
    if (ba->readBinary((uint8_t*)&header, sizeof(header)) == false) {
        ATC_STREAM_LOGE() << "failed to read header from ByteArray!" << std::endl;
        return -1;
    }
    int status = header.status;
    if (status) {
        ATC_STREAM_LOGE() << "subscribe response error!!!" << std::endl;
    }
    return status;
}

uint64_t getCurrentTimeMs() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return (spec.tv_sec * 1000 + spec.tv_nsec / (1000 * 1000));
}

SocketConnection::SocketConnection()
    : m_fd(-1) {
    int rt = pipe(m_notifier);
    if (rt != 0) {
        ATCLOGE("failure to create pipe rt : %d, errno : %d , error : %s\n", rt, errno, strerror(errno));
        throw std::logic_error("create pipe failure!");
    }
    rt = setNonb(m_notifier[0]);
    if (rt != 0) {
        ATCLOGE("failure to set nonb rt : %d errno : %d , error : %s\n", rt, errno, strerror(errno));
        throw std::logic_error("create set nonb!");
    }
    rt = setNonb(m_notifier[1]);
    if (rt != 0) {
        ATCLOGE("failure to create pipe rt : %d, errno : %d , error : %s\n",rt, errno, strerror(errno));
        throw std::logic_error("create pipe failure!");
    }
}

SocketConnection::~SocketConnection() {
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
    close(m_notifier[0]);
    close(m_notifier[1]);
    close(m_fd);
}

struct timeval getTimeval(uint64_t ms) {
    timeval tv;
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000 ) * 1000;
    return tv;
}
#define     HEARTBEAT_INTERVAL          2000

void SocketConnection::run() {
    fd_set allset;
    bool reconnect = false;
    do {
        if (reconnect) {
            sleep(1);
        }
        if (connect() == false) {
            break;
        }
        reconnect = true;
        uint64_t expireAt = getCurrentTimeMs() + HEARTBEAT_INTERVAL;
        FD_ZERO(&allset);
        FD_SET(m_notifier[0], &allset);
        FD_SET(m_fd, &allset);
        while (1) {
            fd_set rdset = allset;
            uint64_t curMs = getCurrentTimeMs();
            if (expireAt <= curMs) {
                if (heartbeat() == RECONNECT) {
                    disconnect();
                    break;
                }
                expireAt = curMs + HEARTBEAT_INTERVAL;
            }
            struct timeval tv = getTimeval(expireAt - curMs);
            int rt = select(std::max(m_notifier[0], m_fd) + 1, &rdset, NULL, NULL, &tv);
            if (rt < 0) {
                if (errno == EINTR || errno == ETIMEDOUT) {
                    continue;
                } else {
                    ATC_STREAM_LOGE() << "select failed: " << strerror(errno) << std::endl;
                    break;
                }
            }
            if (FD_ISSET(m_fd, &rdset)) {
                if (handleBroadcast() == RECONNECT) {
                    disconnect();
                    break;
                }
            }
        };
    } while(!m_stop);
}

int SocketConnection::writeFix(int fd, const uint8_t* buf, uint32_t size) {
    if (fd < 0) {
        errno = EBADFD;
        return -1;
    }
    uint32_t nwrite = 0;
    while (nwrite < size) {
        int rt = ::write(fd, buf + nwrite, size - nwrite);
        if (rt <= 0) {
            if (errno == EPIPE) {
                ATC_STREAM_LOGW() << "client exit, session would be closed" << std::endl;
                return -1;
            } else {
                ATC_STREAM_LOGW() << "socket error: " << strerror(errno) << std::endl;
                return -2;
            }
        }
        nwrite += rt;
    }
    return 0;
}

int SocketConnection::readFix(int fd, uint8_t* buf, uint32_t size) {
    if (fd < 0) {
        ATC_STREAM_LOGE() << "invalid fd: " << fd << std::endl;
        errno = EBADFD;
        return -1;
    }
    uint32_t nread = 0;
    while (nread < size) {
        int rt = ::read(fd, buf + nread, size - nread);
        if (rt < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                continue;
            }
            ATC_STREAM_LOGW() << "readFix failed: " << strerror(errno) << std::endl;
            return -1;
        } else if (rt == 0) {
            ATC_STREAM_LOGW() << "readFix failed: connection closed, rt=0" << std::endl;
            return -1;
        }
        nread += rt;
    }

    return 0;
}

ByteArray::ptr SocketConnection::recv(int fd) {
    ProtocolHeader  header;
    int32_t bodySize = 0;
    ByteArray::ptr ba;
    if (readFix(fd, (uint8_t*)&header, sizeof(header)) < 0) {
        ATC_STREAM_LOGW() << "read package header failed!" << std::endl;
        return nullptr;
    }
    if (header.size > BYTEARRAY_MAX_SIZE || header.size < sizeof(header)) {
        ATC_STREAM_LOGE() << "invalid package size: " << bodySize << std::endl;
        return nullptr;
    }
    bodySize = header.size - sizeof(header);
    ba.reset(new ByteArray((uint8_t*)&header, sizeof(header)));
    if (bodySize) {
        std::shared_ptr<uint8_t> buf((uint8_t*)malloc(bodySize), [](uint8_t *ptr){
            if (ptr) free(ptr);
        });
        if (buf == nullptr) {
            ATC_STREAM_LOGE() << "allocate buffer failed!" << std::endl;
            return nullptr;
        }
        if (readFix(fd, buf.get(), bodySize) < 0) {
            ATC_STREAM_LOGE() << "read package body failed, bodySize:" << bodySize << std::endl;
            return nullptr;
        }
        ba->writeBinary(buf.get(), bodySize);
    }
    if (ProtocolHeader::verifyPackage(ba, RESPONSE_TYPE, (uint32_t)-1) == false) {
        ATC_STREAM_LOGI() << "invalid package!" << std::endl;
        return nullptr;
    }
    return ba;
}

SocketConnection::Status SocketConnection::handleBroadcast() {
    ByteArray::ptr ba;
    ProtocolHeader header;
    memset(&header, 0, sizeof(header));
    ba = recv(m_fd);
    if (ba == nullptr) {
        ATC_STREAM_LOGE() << "failed to receive broadcast!" << std::endl;
        return RECONNECT;
    }
    ba->seek(0);
    ba->readBinary((uint8_t*)&header, sizeof(header));
    auto iter = m_handlers.find(header.op);
    if (iter == m_handlers.end()) {
        ATC_STREAM_LOGW() << std::hex << " operation: " << header.op << " not register!" << std::dec << std::endl;
        return CONTINUE;
    }
    ba->seek(0);
    BroadcastHandlerType handler = iter->second;
    if (handler) {
        handler(ba);
    }
    return STATUS_OK;
}

SocketConnection::Status SocketConnection::heartbeat() {
    ByteArray::ptr req = make_request(HEARTBEAT, 0);
    const uint8_t *buf = req->getBufferRef();
    //immediate disconnect when heart beat package send fail
    if (writeFix(m_fd, buf, req->size()) < 0) {
        return RECONNECT;
    }
    return STATUS_OK;
}

void SocketConnection::registerBroadcastHandler(uint32_t type, BroadcastHandlerType handler) {
    m_handlers.insert(std::make_pair(type, handler));
}

static void safeCloseConnection(int fd) {
    setNonb(fd);
    ::shutdown(fd, SHUT_WR);
    while (1) {
        char buf[64];
        int rt = read(fd, buf, 64);
        if (rt <= 0) {
            break;
        }
    }
    close(fd);
}

void SocketConnection::disconnect() {
    safeCloseConnection(m_fd);
    m_fd = -1;
}

ByteArray::ptr SocketConnection::handleRequest(ByteArray::ptr ba) {
    struct sockaddr_un addr;
    ByteArray::ptr resp;
    ProtocolHeader header;
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    size_t addrlen;
    if (fd < 0) {
        ATC_STREAM_LOGE() << "failed to create socket: " << strerror(errno) << std::endl;
        goto out;
    }
    setTimedout(fd, 1000, 1);
    setTimedout(fd, 1000, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_SOCKETADDR, strlen(UNIX_SOCKETADDR));
    addrlen = offsetof(sockaddr_un, sun_path) + strlen(UNIX_SOCKETADDR);
    memset((uint8_t*)&header, 0, sizeof(header));
    if (ProtocolHeader::verifyPackage(ba, REQUEST_TYPE, -1) == false) {
        ATC_STREAM_LOGE() << "failed to verify request package" << strerror(errno) << std::endl;
        goto out;
    }
    ba->seek(0);
    if (ba->readBinary((uint8_t*)&header, sizeof(header)) == false) {
        ATC_STREAM_LOGE() << "failed to read request header" << strerror(errno) << std::endl;
        goto out;
    }
    if (::connect(fd, (struct sockaddr*)&addr, addrlen) < 0) {
        ATC_STREAM_LOGE() << "failed to connect: " << strerror(errno) << std::endl;
        goto out;
    }
    if (writeFix(fd, ba->getBufferRef(), ba->size()) < 0) {
        ATC_STREAM_LOGE() << "failed to write data: " << strerror(errno) << std::endl;
        goto out;
    }
    resp = recv(fd);
    if (resp == nullptr || ProtocolHeader::verifyPackage(resp, RESPONSE_TYPE, header.op) == false) {
        resp.reset();
        goto out;
    }
    resp->seek(0);
out:
    ::close(fd);
    return resp;
}

bool SocketConnection::start() {
    //pass this is safte cause the destructor would wait for the thread in the end
    m_thread.reset(new std::thread(&SocketConnection::run, this));
    return true;
}

}