#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H
#include "clusterthread.h"
#include "clustertoivicommand.h"
#include "ivitoclustercommand.h"
#include "clusterthreadproc.h"
#include <list>
#include <condition_variable>
#include "callback.h"
#include <unistd.h>

class DataSplitManager;
class CommandHandler : public ClusterThread, public DataReceiver
{
public:
    CommandHandler(int fd, DataReceiveCallback *receiver, ITranportCallback *callback = nullptr);
    ~CommandHandler();
    void sendCommand(std::unique_ptr<ClusterToIVICommand> &command);
    void receiveData(unsigned char cmd, const char* data, unsigned int length);

private:
    void threadRun() override;
    void startProcess();
    void stopProcess();
    void handleSendCommand();
    void handleTimeoutCommand();
    void handleReceiveCommand();
    void sendHeartbeatCommand();
    void stopReceiveProcess();
    void onReceiveReply(unsigned char cmd, unsigned char result, const CharArray& data);
    void onReceiveRequest(unsigned char cmd, const CharArray& data);
    void onReceiveSplitData(unsigned char cmd, const CharArray& data);


    std::mutex m_mutex;
    std::mutex m_waitReplyCmdMutex;
    std::condition_variable m_condition;
    std::list<std::unique_ptr<ClusterToIVICommand>> m_commands;
    std::list<std::unique_ptr<ClusterToIVICommand>> m_waitReplyCommands;
    ITransport *m_transport = nullptr;
    ThreadProc *m_reciverCmdProc = nullptr;
    ThreadProc *m_sendHeartCmdProc = nullptr;
    DataReceiveCallback *m_dataReceiver = nullptr;
    const time_t WAIT_TIME = 200;
    DataSplitManager *m_dataSplitManager = nullptr;
    ITranportCallback *m_tranportCallback = nullptr;
    int m_fd;
    int m_eventfd = -1;
};

#endif // COMMANDHANDLER_H
