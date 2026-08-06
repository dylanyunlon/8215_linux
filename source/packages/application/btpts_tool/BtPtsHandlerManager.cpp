#include <unistd.h>
#include "BtPtsGapHandler.h"
#include "BtPtsHfpHandler.h"
#include "BtPtsPbapHandler.h"
#include "BtPtsAvrcpHandler.h"
#include "BtPtsHidHandler.h"
#include "BtPtsGattHandler.h"
#include "BtPtsHandlerManager.h"

const map<string, HANDLE_FUN> BtPtsHandlerManager::HANDLE_MAP = {
    {CMD_HELP,                static_cast<HANDLE_FUN>(&BtPtsHandlerManager::help)},
    {CMD_EXIT,                static_cast<HANDLE_FUN>(&BtPtsHandlerManager::quit)},
    {CMD_DELAY,               static_cast<HANDLE_FUN>(&BtPtsHandlerManager::delay)},
    {CMD_ADDR,                static_cast<HANDLE_FUN>(&BtPtsHandlerManager::setAddress)},
    {CMD_PROFILE_CONNECT,     static_cast<HANDLE_FUN>(&BtPtsHandlerManager::connect)},
    {CMD_PROFILE_DISCONNECT,  static_cast<HANDLE_FUN>(&BtPtsHandlerManager::disconnect)},
    {CMD_BLUETOOTH_DUMP,      static_cast<HANDLE_FUN>(&BtPtsHandlerManager::dump)}
};

BtPtsHandlerManager::BtPtsHandlerManager() : BtPtsHandler(PROFILE_MANAGER)
{
    IBluetoothClient *client = NULL;
    client = getBluetoothClient();
    EXPECT_NOT_NULL(client, "client");
    registerHandlers(client);
}

BtPtsHandlerManager::~BtPtsHandlerManager()
{
    deregisterHandlers();
}

void BtPtsHandlerManager::registerHandlers(IBluetoothClient *client)
{
    deregisterHandlers();
    m_handlers.push_back(new BtPtsGapHandler(client));
    m_handlers.push_back(new BtPtsHfpHandler(client));
    m_handlers.push_back(new BtPtsPbapHandler(client));
    m_handlers.push_back(new BtPtsAvrcpHandler(client));
    m_handlers.push_back(new BtPtsHidHandler(client));
    m_handlers.push_back(new BtPtsGattHandler(client));
}

void BtPtsHandlerManager::deregisterHandlers()
{
    list<BtPtsHandler*>::iterator it = m_handlers.begin();
    for (; it != m_handlers.end(); ++it) {
        if (NULL != *it) {
            SAFE_DELETE(*it);
        }
    }

    m_handlers.clear();
}
const map<string, HANDLE_FUN>& BtPtsHandlerManager::getHandleFunMap()
{
   return HANDLE_MAP;
}

int BtPtsHandlerManager::handle(const BtPtsCmd& ptsCmd)
{
    int ret = BTPTS_OK;
    if (ptsCmd.m_profile == PROFILE_MANAGER) {
        ret = doFun(ptsCmd);
    } else {
        list<BtPtsHandler*>::iterator it = m_handlers.begin();
        for (; it != m_handlers.end(); ++it) {
            if (ptsCmd.m_profile == (*it)->getProfileName()) {
                ret = (*it)->doFun(ptsCmd);
                break;
            }
        }
    }

    return ret;
}

int BtPtsHandlerManager::help()
{
    printf("************* help instruction **************\n");
    for (BtPtsCmd cmd : BTPTS_CMDS) {
        if (string(cmd.m_helpInfo) != "") {
            if (cmd.m_profile == PROFILE_MANAGER)
                printf("%s : %s\n", cmd.m_cmd.c_str(), cmd.m_helpInfo);
            else
                printf("%s %s : %s\n", cmd.m_profile.c_str(), cmd.m_cmd.c_str(), cmd.m_helpInfo);
        }
    }
    printf("******************** end *********************\n");

    return BTPTS_OK;
}

int BtPtsHandlerManager::quit()
{
    exit(0);
    return BTPTS_OK;
}

int BtPtsHandlerManager::delay()
{
    int delayTime = m_args.getIntExtra("int1", 0);
    sleep(delayTime);

    return BTPTS_OK;
}

int BtPtsHandlerManager::setAddress()
{
    string address = m_args.getStringExtra("str1", "");
    m_address = address;

    return BTPTS_OK;
}

int BtPtsHandlerManager::connect()
{
    BtPtsCmd ptsCmd;
    ptsCmd.m_cmd = CMD_PROFILE_CONNECT;
    ptsCmd.m_args = m_args;
    list<BtPtsHandler*>::iterator it = m_handlers.begin();
    for (; it != m_handlers.end(); ++it) {
        if (NULL != *it && (*it)->isAutoConnected()) {
            (*it)->doFun(ptsCmd);
        }
    }
}

int BtPtsHandlerManager::disconnect()
{
    BtPtsCmd ptsCmd;
    ptsCmd.m_cmd = CMD_PROFILE_DISCONNECT;
    list<BtPtsHandler*>::iterator it = m_handlers.begin();
    for (; it != m_handlers.end(); ++it) {
        if (NULL != *it) {
            (*it)->doFun(ptsCmd);
        }
    }
}

int BtPtsHandlerManager::dump()
{
    string dumpInfo = "";

    list<BtPtsHandler*>::iterator it = m_handlers.begin();
    for (; it != m_handlers.end(); ++it) {
        if (NULL != *it) {
            (*it)->dump(dumpInfo);
        }
    }

    printf("================ bluetooth dump info ================\n");
    printf("%s", dumpInfo.c_str());
    printf("================ bluetooth dump  end ================\n");
}