#include "clustermanager.h"
#include "clog.h"
#include "commandhandler.h"
#include "tcpservice.h"
const static char *TAG = "ClusterManager";
ClusterManager::ClusterManager(DataReceiveCallback *callback)
    : m_tcpService(new TcpService(this))
    , m_callback(callback)
{

}

ClusterManager::~ClusterManager()
{
    m_tcpService->stop();
}

void ClusterManager::start()
{
    startProcess();
    m_tcpService->start(); //启动Service， 怎么建立通信
}

void ClusterManager::sendData(unsigned char cmd, const CharArray &data)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto handler : m_commandHandlers) {
        std::unique_ptr<ClusterToIVICommand> command(new ClusterToIVICommand(cmd));
        command->setData(data);
        handler.second->sendCommand(command);
    }
}

void ClusterManager::replyData(int fd, unsigned char cmd, char result, const CharArray &data)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_commandHandlers.find(fd);
    if (it != m_commandHandlers.end()) {
        //如果是通知命令，直接发送回复命令
        std::unique_ptr<ClusterToIVICommand> replyCommand(new ClusterToIVICommand(cmd));
        replyCommand->setType(CommonConstant::Reply);
        replyCommand->setReplyResult(result);
        it->second->sendCommand(replyCommand);
    }
}

int ClusterManager::handleMessage(const CMessage &message)
{
    int what = message.what;
    int fd = message.arg1;
    std::unique_lock<std::mutex> lock(m_mutex);

    auto it = m_commandHandlers.find(fd);
    if (it != m_commandHandlers.end()) {
        if (what == TransportError) {
            UTILS_LOGD(TAG, "remove it");
            delete (it->second);
            m_commandHandlers.erase(it);
        }
    } else {
        if (what == ClientConnected) {
            m_commandHandlers[fd] = new CommandHandler(fd, m_callback, this);
        }
    }

    return 0;
}

void ClusterManager::onClientConnected(int fd)
{
    UTILS_LOGD(TAG, "onClientConnected %d", fd);
    CMessage message;
    message.what = ClientConnected;
    message.arg1 = fd;
    sendMessage(message);
}

void ClusterManager::onTransportError(int fd)
{
    UTILS_LOGE(TAG, "onTransportError %d", fd);

    CMessage message;
    message.what = TransportError;
    message.arg1 = fd;
    sendMessage(message);
}


