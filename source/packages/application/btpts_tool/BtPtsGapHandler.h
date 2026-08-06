#ifndef BTPTSGAPHANDLER_H
#define BTPTSGAPHANDLER_H
#include "BtPtsHandler.h"
class BtPtsGAPCallBack;
class BtPtsGapHandler : public BtPtsHandler
{
public:
    BtPtsGapHandler(IBluetoothClient *client);
    ~BtPtsGapHandler();
private:
    const map<string, HANDLE_FUN> &getHandleFunMap();
    void registerCallBack();
    void deregisterCallBack();
    int open();
    int close();
    int discovery();
    int cancelDiscovery();
    int bond();
    int unbond();
    int scanMode();
    int setName();
    bool dump(string &dumpInfo);

    string localStateToString(E_LOCAL_DEVICE_STATE state);
    string scanModeToString(E_GAP_SCAN_MODE mode);

    static const map<string, HANDLE_FUN> GAP_HANDLE_MAP;
    IBluetoothClient *m_client;
    IBluetoothLocalDevice *m_localDevice;
    BluetoothRemoteDevice *m_remoteDevice;
    BtPtsGAPCallBack *m_gapCallback;


};

class BtPtsGAPCallBack : public BtPtsCallBack, public universal_utils::Singleton<BtPtsGAPCallBack>
{
public:
    int onIndication(const universal_utils::CMessage &message);
};

#endif // BTPTSGAPHANDLER_H
