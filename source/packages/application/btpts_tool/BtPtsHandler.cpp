#include "BtPtsHandler.h"

BluetoothAddress BtPtsHandler::m_address;
BtPtsHandler::BtPtsHandler(const string &profileName)
    : m_profileName(profileName)
{

}

BtPtsHandler::~BtPtsHandler()
{

}

string BtPtsHandler::getProfileName()
{
    return m_profileName;
}

bool BtPtsHandler::isAutoConnected()
{
    return false;
}

int BtPtsHandler::doFun(const BtPtsCmd &ptsCmd)
{
    map<string, HANDLE_FUN>::const_iterator iter;
    int ret = 0;
    for(iter = getHandleFunMap().begin(); iter != getHandleFunMap().end(); iter++) {
        if (ptsCmd.m_cmd == iter->first) {
            m_args = ptsCmd.m_args;        //update args
            ret = (this->*(iter->second))();
            break;
        }
    }

    return ret;
}

bool BtPtsHandler::dump(string &dumpInfo)
{
    return false;
}

string BtPtsHandler::connectionStateToString(E_BLUETOOTH_PROFILE_STATE state)
{
    string stateStr = "";
    switch(state) {
    case BLUETOOTH_PROFILE_IDLE:
        stateStr = "disconnected";
        break;

    case BLUETOOTH_PROFILE_CONNECTING:
        stateStr = "connecting";
        break;

    case BLUETOOTH_PROFILE_CONNECTED:
        stateStr = "connected";
        break;

    case BLUETOOTH_PROFILE_DISCONNECTING:
        stateStr = "disconnecting";
        break;

    default:
        stateStr = "disconnected";
    }

    return stateStr;
}

BtPtsCallBack::BtPtsCallBack()
    : m_expectedIndication(0)
    , m_expectedValue(0)
{
    m_conditionId = m_condition.newCondition();
}

BtPtsCallBack::~BtPtsCallBack()
{
    m_condition.releaseCondition(m_conditionId);
}

int BtPtsCallBack::checkIndication(int expectedIndication, int expectedValue)
{
    m_condition.lock();

    if (m_expectedIndication == expectedIndication && m_expectedValue == expectedValue) {
        m_condition.signal(m_conditionId);
    }

    m_condition.unlock();

    return 0;
}

bool BtPtsCallBack::waitCommandDone(int expectedIndication, int timeout, int expectedValue)
{
    bool ret;

    m_condition.lock();
    m_expectedIndication = expectedIndication;
    m_expectedValue = expectedValue;
    ret = m_condition.await(m_conditionId, timeout);
    m_condition.unlock();

    return ret;
}



