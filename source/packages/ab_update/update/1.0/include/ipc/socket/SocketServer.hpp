#pragma once

#include <memory>
#include <set>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>

#include "utils/TSQueue.hpp"
#include "ipc/socket/SocketProto.hpp"
#include "utils/Lock.hpp"
#include "utils/Singleton.hpp"
#include "utils/Sleeper.hpp"

namespace atcupdateservice {
namespace ipc {
namespace socket {

typedef ByteArray::ptr response_type;

class Session : public std::enable_shared_from_this<Session> {
public:
    enum Type {
        ONESHOT,
        KEEPALIVE,
        DEAD,
    };
    typedef std::shared_ptr<Session> ptr;
    Session(int fd);
    ~Session() {
        ATC_STREAM_LOGE() << "session exit!" << std::endl;
        close();
    }
    void close();
    void start();
    int send(ByteArray::ptr ba);
    ByteArray::ptr recv();
    bool startHeartBeatDetector();
private:
    void heartbeatDetector();
    int readFix(uint8_t *buf, uint32_t size);
private:
    bool m_status;
    int m_fd;
    std::shared_ptr<std::thread> m_heartbeatDetector;
};

class SocketServer {
public:
    typedef std::shared_ptr<SocketServer> ptr;
    typedef std::function<response_type(ByteArray::ptr)> HandlerType;
    friend  utils::SingletonPtr<SocketServer>;
    typedef utils::SingletonPtr<SocketServer> Instance;
    ~SocketServer();
    bool start();
    void registerHandler(uint32_t type, HandlerType handler) {
        std::lock_guard<std::mutex> lg(m_mx);
        m_handlers.insert(std::make_pair(type, handler));
    }
    void addSubscriber(Session::ptr session) {
        if (session == nullptr || session->startHeartBeatDetector() == false) {
            ATC_STREAM_LOGE() << "add subscriber failed!!!" << std::endl;
            return;
        }
        std::lock_guard<std::mutex> lg(m_mx);
        m_subscribers.insert(session);
    }
    void removeSubscriber(Session::ptr session) {
        std::lock_guard<std::mutex> lg(m_mx);
        m_subscribers.erase(session);
    }
    void broadcast(ByteArray::ptr ba);
private:
    SocketServer();
    void run();
    bool init();
    Session::ptr handleAccept();
    Session::Type handleSession(Session::ptr session);
    void broadcastThread();
private:
    std::mutex m_mx;
    std::shared_ptr<std::thread> m_mainThread;
    std::shared_ptr<std::thread> m_broadcastThread;
    std::set<Session::ptr> m_subscribers;
    std::map<uint32_t, HandlerType> m_handlers;
    utils::TSQueue<ByteArray::ptr> m_broadcastQueue;
    utils::Sleeper::ptr m_sleeper;
    int m_sockfd;
    bool m_stop;
};

struct RegisterHandlerHelper {
    RegisterHandlerHelper(uint32_t type, SocketServer::HandlerType handler) {
        SocketServer::ptr server = SocketServer::Instance::getInstance();
        server->registerHandler(type, handler);
    }
};

}
}
}

#define REGISTER_SOCKET_HANDLER(type, handler)               \
    static atcupdateservice::ipc::socket::RegisterHandlerHelper __handler_ ## type ## __(type, handler)
