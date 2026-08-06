#ifndef BTPTSCMDHANDER_H
#define BTPTSCMDHANDER_H
#include <list>
#include "BtPtsCmdParser.h"
#include "BtPtsHandler.h"

class BtPtsHandlerManager : BtPtsHandler
{
public:
    BtPtsHandlerManager();
    ~BtPtsHandlerManager();
    int handle(const BtPtsCmd& ptsCmd);

private:
    const map<string, HANDLE_FUN> &getHandleFunMap();
    void registerHandlers(IBluetoothClient *client);
    void deregisterHandlers();
    int help();
    int quit();
    int delay();
    int setAddress();
    int connect();
    int disconnect();
    int dump();
    list<BtPtsHandler*> m_handlers;
    static const map<string, HANDLE_FUN> HANDLE_MAP;
};

#endif // BTPTSCMDHANDER_H
