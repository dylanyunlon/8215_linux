#ifndef CLUSTERMANAGER_H
#define CLUSTERMANAGER_H
#include "callback.h"
#include <cmessagehandler.h>
#include <map>
#include <mutex>
using namespace  universal_utils;
class CommandHandler;
class TcpService;
class ClusterManager
        : public CMessageHandler
        , public ConnectionCallback
        , public ITranportCallback
{
public:
    ClusterManager(DataReceiveCallback *callback);
    ~ClusterManager();
    void start();
    void sendData(unsigned char cmd, const CharArray& data = CharArray());
    void replyData(int fd, unsigned char cmd, char result, const CharArray& data = CharArray());

private:
    int handleMessage(const CMessage &message);
    void onClientConnected(int fd);
    void onTransportError(int fd);

    std::mutex m_mutex;
    std::map<int, CommandHandler*> m_commandHandlers;
    TcpService *m_tcpService;
    DataReceiveCallback *m_callback;

    enum ClusterMessage {
        ClientConnected = 0,
        TransportError,
    };
};

#endif // CLUSTERMANAGER_H
