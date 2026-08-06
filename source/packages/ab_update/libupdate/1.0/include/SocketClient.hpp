#pragma once

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include "UpdateMessageType.hpp"

namespace atcupdateservice {

namespace ipc {
namespace socket {
class ByteArray;
}
}
using namespace atcupdateservice::ipc::socket;

class SocketConnection : public std::enable_shared_from_this<SocketConnection> {
public:
    enum Status {
        STATUS_OK,
        RECONNECT,
        SHUTDOWN,
        CONTINUE,
    };
    typedef std::shared_ptr<SocketConnection> ptr;
    typedef std::function<void(std::shared_ptr<ByteArray>)> BroadcastHandlerType;
    SocketConnection();
    ~SocketConnection();
    void registerBroadcastHandler(uint32_t type, BroadcastHandlerType handler);
    std::shared_ptr<ByteArray> handleRequest(std::shared_ptr<ByteArray> ba);
    void disconnect();
    bool connect();
    bool start();
private:
    int subscribe();
    void run();
    std::shared_ptr<ByteArray> recv(int fd);
    Status heartbeat();
    int writeFix(int fd, const uint8_t* buf, uint32_t size);
    int readFix(int fd, uint8_t* buf, uint32_t size);
    Status handleBroadcast();
private:
    int m_fd;
    int m_notifier[2];
    int m_stop;
    std::shared_ptr<std::thread> m_thread;
    std::map<uint32_t, BroadcastHandlerType> m_handlers;
};

}