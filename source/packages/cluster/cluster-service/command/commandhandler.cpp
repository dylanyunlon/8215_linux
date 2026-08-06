#include "commandhandler.h"
#include "transport.h"
#include "commonfun.h"
#include <sys/eventfd.h>
#include "datasplitmanager.h"
const static char *TAG = "CommandHandler";
static const uint64_t BREAKE_SELECT_CMD = 1;
CommandHandler::CommandHandler(int fd, DataReceiveCallback *receiver, ITranportCallback *callback)
    : m_transport(new Transport(fd))
    , m_dataReceiver(receiver)
    , m_tranportCallback(callback)
    , m_fd(fd)
{
    m_transport->setTransportCallback(m_tranportCallback);
    m_reciverCmdProc = new ThreadProc(std::bind(&CommandHandler::handleReceiveCommand, this));
    m_sendHeartCmdProc = new ThreadProc(std::bind(&CommandHandler::sendHeartbeatCommand, this));

    m_eventfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (m_eventfd < 0) {
      //  UTILS_LOGE(TAG, "eventfd error %s", strerror(errno));
    }

    startProcess();
}

CommandHandler::~CommandHandler()
{
    //UTILS_LOGD(TAG, "disconstruct");
    stopProcess();
}

void CommandHandler::startProcess()
{
    if (!m_run) {
        m_run = true;
        threadStart();
        m_reciverCmdProc->threadStart();
        m_reciverCmdProc->triggerProc();
        m_sendHeartCmdProc->threadStart();
        std::unique_ptr<ClusterToIVICommand> command(new ClusterToIVICommand(CommonConstant::HeartbeatCmd, 2000));
        sendCommand(command);
    }
}

void CommandHandler::stopProcess()
{
    //UTILS_LOGD(TAG, "stopProcess enter");
    m_run = false;
    m_condition.notify_one();
    threadStop();
    stopReceiveProcess();
    m_sendHeartCmdProc->threadStop();
    m_commands.clear();
    m_waitReplyCommands.clear();
   // UTILS_LOGD(TAG, "stopProcess leave");
}

void CommandHandler::sendCommand(std::unique_ptr<ClusterToIVICommand> &command)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_commands.push_back(std::move(command));
    m_condition.notify_one();
}

void CommandHandler::receiveData(unsigned char cmd, const char *data, unsigned int length)
{
    if (m_dataReceiver) {
        m_dataReceiver->onReceiveData(m_fd, cmd, data, length);
    }
}

void CommandHandler::threadRun()
{
    handleSendCommand();
}

void CommandHandler::handleSendCommand()
{
    while (m_run) {
        if (!m_commands.empty()) {  //加锁经典的双重判断, 等待回复完成才去读下一条
            std::unique_lock<std::mutex> lock(m_mutex);
            if (!m_commands.empty()) {
                std::unique_ptr<ClusterToIVICommand> command(std::move(m_commands.front()));
                m_commands.pop_front();
                lock.unlock();
                command->setTransport(m_transport);
                command->execute();
                if (command->getCmdType() != CommonConstant::Reply) {
                    std::unique_lock<std::mutex> lock2(m_waitReplyCmdMutex);
                    m_waitReplyCommands.push_back(std::move(command));
                }
            }
        }

        handleTimeoutCommand(); //处理超时消息
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait_for(lock, std::chrono::milliseconds(WAIT_TIME), [this]{return !(m_run && m_commands.empty());});
    }
}

void CommandHandler::handleReceiveCommand()
{
    while (m_run) {
        fd_set fdsets;
        FD_ZERO(&fdsets);
        FD_SET(m_fd, &fdsets);
        FD_SET(m_eventfd, &fdsets);
        int count = select(std::max(m_eventfd, m_fd) + 1, &fdsets, nullptr, nullptr, nullptr);

        if (count > 0 && FD_ISSET(m_fd, &fdsets)) {
            --count;
            IVIToClusterCommand receivedCommand(this);
            receivedCommand.setTransport(m_transport);
            if (!receivedCommand.execute())
                continue;

            if (receivedCommand.getCmdType() == CommonConstant::Reply) { //如果是回复命令，则移除等待回复的命令
                std::unique_lock<std::mutex> lock(m_waitReplyCmdMutex);
                for (auto it = m_waitReplyCommands.begin(); it != m_waitReplyCommands.end();) {
                    if ((*it)->getCmdId() == receivedCommand.getCmdId()) {
                        it = m_waitReplyCommands.erase(it);
                        break;
                    } else {
                         ++it;
                     }
                }
            } else if (receivedCommand.getCmdType() == CommonConstant::Request
                       && receivedCommand.getCmdId() != CommonConstant::DownloadUpdatePackageCheck) {
                //如果是通知命令，直接发送回复命令
                std::unique_ptr<ClusterToIVICommand> replyCommand(new ClusterToIVICommand(receivedCommand.getCmdId()));
                replyCommand->setType(CommonConstant::Reply);
                sendCommand(replyCommand);
            }
        }


        if (count > 0 && FD_ISSET(m_eventfd, &fdsets)) {
            uint64_t cmd = 0;
            ::read(m_eventfd, &cmd, sizeof(cmd));
            if (cmd == BREAKE_SELECT_CMD) {
              //  UTILS_LOGD(TAG, "select break by eventfd cmd %d", cmd);
                break;
            }
        }
    }
}

void CommandHandler::stopReceiveProcess()
{
    m_transport->close();
    if (::write(m_eventfd, &BREAKE_SELECT_CMD, sizeof(BREAKE_SELECT_CMD)) != sizeof(BREAKE_SELECT_CMD)) {
       // UTILS_LOGE(TAG, "write eventfd breakSelect failed");
    }
    m_reciverCmdProc->threadStop();
    close(m_eventfd);
}

void CommandHandler::sendHeartbeatCommand()
{
    sleep(2);
    std::unique_ptr<ClusterToIVICommand> command(new ClusterToIVICommand(CommonConstant::HeartbeatCmd, 2000));
    sendCommand(command);
}

void CommandHandler::onReceiveReply(unsigned char cmd, unsigned char result, const CharArray &data)
{
    if (cmd == CommonConstant::HeartbeatCmd) {
        //UTILS_LOGD(TAG, "onReceiveReply cmd=%d result=%d", cmd, result);
        m_sendHeartCmdProc->triggerProc();
    }
}

void CommandHandler::onReceiveRequest(unsigned char cmd, const CharArray &data)
{
    //UTILS_LOGD(TAG, "onReceiveRequest cmd=%d dataLen=%d", cmd, data.getLength());
    receiveData(cmd, data.getData(), data.getLength());
}

void CommandHandler::onReceiveSplitData(unsigned char cmd, const CharArray &data)
{
    if (!m_dataSplitManager) {
        m_dataSplitManager = new DataSplitManager(this);
    }
    m_dataSplitManager->receiveSplitData(cmd, data);
}

void CommandHandler::handleTimeoutCommand()
{
    std::unique_lock<std::mutex> lock(m_waitReplyCmdMutex);
    for (auto it = m_waitReplyCommands.begin(); it != m_waitReplyCommands.end();) {
        if ((*it)->checkTimeout()) {
          //  UTILS_LOGE(TAG, "handleTimeoutCommand %d", (*it)->getCmdId());
            m_tranportCallback->onTransportError(m_fd);
            m_dataReceiver->onReceiveReplyTimeOut(m_fd, (*it)->getCmdId());
            it = m_waitReplyCommands.erase(it);

        } else {
            ++it;
        }
    }
}
