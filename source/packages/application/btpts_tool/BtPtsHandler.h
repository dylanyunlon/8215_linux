#ifndef BTPTSHANDER_H
#define BTPTSHANDER_H
#include <map>
#include "BtPtsConstants.h"
#include "bluetoothapi.h"
#include "singleton.h"
#include "bluetoothmessageextras.h"
#include "bluetoothaddress.h"
#include "cconditionlock.h"
class BtPtsHandler;
typedef int (BtPtsHandler::*HANDLE_FUN)();

class BtPtsHandler {
 public:
    BtPtsHandler(const string &profileName);
    virtual ~BtPtsHandler();
    string getProfileName();
    virtual bool isAutoConnected();
    virtual const map<string, HANDLE_FUN> &getHandleFunMap() = 0;
    int doFun(const BtPtsCmd &ptsCmd);
    virtual void registerCallBack(){}
    virtual void deregisterCallBack(){}
    virtual bool dump(string &dumpInfo);

 protected:
    string connectionStateToString(E_BLUETOOTH_PROFILE_STATE state);
    static BluetoothAddress m_address;
    universal_utils::CMessage m_args;
 private:
    const string m_profileName;
};


class BtPtsCallBack : public IBluetoothCallBack
{
public:

    BtPtsCallBack();
    virtual ~BtPtsCallBack();
    int checkIndication(int expectedIndication, int expectedValue = 0);
    bool waitCommandDone(int expectedIndication, int timeout, int expectedValue = 0);

private:
    universal_utils::CConditionLock m_condition;
    int m_conditionId;
    int m_expectedIndication;
    int m_expectedValue;

};

#endif // BTPTSHANDER_H

