#ifndef BTPTSHFPHANDLER_H
#define BTPTSHFPHANDLER_H
#include "BtPtsHandler.h"
#include "bluetoothhandsfree.h"
#include "bluetoothhandsfreeindications.h"

class BtPtsHFPCallBack;

class BtPtsHfpHandler : public BtPtsHandler
{
public:
    BtPtsHfpHandler(IBluetoothClient *client);
    ~BtPtsHfpHandler();
    bool isAutoConnected();
private:
    const map<string, HANDLE_FUN> &getHandleFunMap();
    void registerCallBack();
    void deregisterCallBack();
    void sleepSecondsForRetry(const string &retryCmd);
    int connect();
    int disconnect();
    int dial();
    int accept();
    int terminate();
    int hold();
    int dtmf();
    int switchAudio();
    int redial();
    int speakerGain();
    int micGain();
    bool dump(string &dumpInfo);

    static const map<string, HANDLE_FUN> HFP_HANDLE_MAP;
    IBluetoothHandsfree *m_hfpInterface;
    BtPtsHFPCallBack *m_hfpCallback;
};

class BtPtsHFPCallBack : public BtPtsCallBack , public universal_utils::Singleton<BtPtsHFPCallBack>
{
public:
    int onIndication(const universal_utils::CMessage &message);
};

#endif // BTPTSHFPHANDLER_H
