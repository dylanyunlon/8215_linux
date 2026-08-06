#ifndef BTPTSHIDHANDLER_H
#define BTPTSHIDHANDLER_H
#include "BtPtsHandler.h"
#include "bluetoothhid.h"

class BtPtsHidCallBack;
class BtPtsHidHandler : public BtPtsHandler
{
public:
    BtPtsHidHandler(IBluetoothClient *client);
    ~BtPtsHidHandler();
private:
    const map<string, HANDLE_FUN> &getHandleFunMap();
    void registerCallBack();
    void deregisterCallBack();
    int connect();
    int disconnect();
    int sendMouseData();
    int sendKeyboardData();
    int sendControlData();

    static const map<string, HANDLE_FUN> HID_HANDLE_MAP;
    IBluetoothHid *m_hidInterface;
    BtPtsHidCallBack *m_hidCallback;
};

class BtPtsHidCallBack : public BtPtsCallBack , public universal_utils::Singleton<BtPtsHidCallBack>
{
public:
    int onIndication(const universal_utils::CMessage &message);
};

#endif // BTPTSHIDHANDLER_H
