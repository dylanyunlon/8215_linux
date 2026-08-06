#ifndef BTPTSPBAPHANDLER_H
#define BTPTSPBAPHANDLER_H
#include "BtPtsHandler.h"
#include "bluetoothpbapindications.h"
class BtPtsPBAPCallBack;
class BtPtsPbapHandler : public BtPtsHandler
{
public:
    BtPtsPbapHandler(IBluetoothClient *client);
    ~BtPtsPbapHandler();
private:
    const map<string, HANDLE_FUN> &getHandleFunMap();
    void registerCallBack();
    void deregisterCallBack();
    int connect();
    int disconnect();
    int download();
    int stopdownload();

    static const map<string, HANDLE_FUN> PBAP_HANDLE_MAP;
    IBluetoothPBAP *m_pbapInterface;
    BtPtsPBAPCallBack *m_pbapCallback;
};

class BtPtsPBAPCallBack : public BtPtsCallBack , public universal_utils::Singleton<BtPtsPBAPCallBack>
{
public:
    int onIndication(const universal_utils::CMessage &message);
};

#endif // BTPTSPBAPHANDLER_H
