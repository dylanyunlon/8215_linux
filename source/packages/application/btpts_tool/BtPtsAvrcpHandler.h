#ifndef BTPTSAVRCPHANDLER_H
#define BTPTSAVRCPHANDLER_H
#include "BtPtsHandler.h"
#include "bluetootha2dp.h"
#include "bluetootha2dpindications.h"
#include "bluetoothavrcp.h"
#include "bluetoothavrcpindications.h"

class BtPtsA2DPCallBack;
class BtPtsAVRCPCallBack;
class BtPtsAvrcpHandler : public BtPtsHandler
{
public:
    BtPtsAvrcpHandler(IBluetoothClient *client);
    ~BtPtsAvrcpHandler();
    bool isAutoConnected();
private:
    const map<string, HANDLE_FUN> &getHandleFunMap();
    void registerCallBack();
    void deregisterCallBack();
    int connect();
    int disconnect();
    int play();
    int pause();
    int stop();
    int pre();
    int next();
    int updatePlayStatus();
    int updateMediaInfo();
    int setVolume();
    int registerInterimVolume();
    void sleepSecondsForRetry(const string &retryCmd);
    bool dump(string &dumpInfo);

    static const map<string, HANDLE_FUN> AVRCP_HANDLE_MAP;
    IBluetoothA2dp *m_a2dpInterface;
    IBluetoothAvrcp *m_avrcpInterface;
    BtPtsA2DPCallBack *m_a2dpCallback;
    BtPtsAVRCPCallBack *m_avrcpCallback;
};

class BtPtsA2DPCallBack : public BtPtsCallBack , public universal_utils::Singleton<BtPtsA2DPCallBack>
{
    int onIndication(const universal_utils::CMessage &message);
};

class BtPtsAVRCPCallBack : public BtPtsCallBack , public universal_utils::Singleton<BtPtsAVRCPCallBack>
{
public:
    BtPtsAVRCPCallBack();
    int onIndication(const universal_utils::CMessage &message);
    int getMusicState();
private:
    int m_musicState;
};

#endif // BTPTSAVRCPHANDLER_H
