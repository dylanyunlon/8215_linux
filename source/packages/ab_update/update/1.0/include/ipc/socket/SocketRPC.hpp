#pragma once

#include "ipc/socket/SocketServer.hpp"
#include "ipc/socket/SocketProto.hpp"

namespace atcupdateservice {
namespace ipc {
namespace socket {

class SocketIPC : public IPCBase {
public:
    SocketIPC() {}
    bool sendMessage(UpdateMessageType action, const std::string &msg) override {
        SocketServer::ptr server = SocketServer::Instance::getInstance();
        server->broadcast(make_response(SEND_MESSAGE, 0, uint32_t(action), msg));
        return true;
    }
    bool sendProgress(unsigned progress) override {
        SocketServer::ptr server = SocketServer::Instance::getInstance();
        server->broadcast(make_response(SEND_PROGRESS, 0, progress));
        return true;
    }
};

}
}
}