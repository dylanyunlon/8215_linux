#ifndef TCPSERVICE_H
#define TCPSERVICE_H
#include "clusterthread.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <set>
#include "callback.h"
#include <unistd.h>

class TcpService : public ClusterThread
{
public:
    TcpService(ConnectionCallback *clientConnection);
    ~TcpService();
    void start();
    void stop();

private:
    void threadRun() override;

    int init();
    void deinit();
    void select();
    void breakSelect();
    int accept();

    int m_serverFd = -1;
    int m_eventfd = -1;
    const int SERVICE_PORT = 18888;
    const int MAX_LINSTEN_NUM = 100;
    ConnectionCallback *m_connection = nullptr;
};

#endif // TCPSERVICE_H
