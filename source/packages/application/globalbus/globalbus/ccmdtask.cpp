/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ccmdtask.h"

using namespace universal_utils;

unsigned long CCmdTask::s_cmdSyncCount = 0;

CCmdTask::CCmdTask()
    : CTask()
{
}

CCmdTask::CCmdTask(unsigned char mainFunc,
                        unsigned char subFunc,
                        unsigned char appID,
                        unsigned int data,
                        unsigned char retType,
                        const unsigned char *retData,
                        unsigned long retDataLen)
{
    makeNewCmd(mainFunc, subFunc, appID, data, retType, retData, retDataLen);
}

CCmdTask::CCmdTask(const unsigned char *data, unsigned long dataLen)
{
    setCmdTask(data, dataLen);
}

CCmdTask::CCmdTask(const CMDPacket &pack)
{
    setCmdTask((const unsigned char *)&pack, sizeof(pack));
}

bool CCmdTask::operator==(const CCmdTask &rhs) const
{
    const unsigned int COMPARE_DATA_LEN = 6;
    bool ret = false;
    const CCmdTask *cmdTask = dynamic_cast<const CCmdTask *>(&rhs);

    if (&rhs == this
        || (cmdTask
            && cmdTask->getCmdBufLen() >= COMPARE_DATA_LEN
            && 0 == m_strCmdBuf.compare(cmdTask->m_strCmdBuf.getStr(),
                                        COMPARE_DATA_LEN))) {
        ret = true;
    }

    return ret;
}

CCmdTask::~CCmdTask()
{
}

bool CCmdTask::makeNewCmd(unsigned char mainFunc,
                                unsigned char subFunc,
                                unsigned char appID,
                                unsigned int data,
                                unsigned char retType,
                                const unsigned char *retData,
                                unsigned long retDataLen)
{
    m_strCmdBuf.clear();
    s_cmdSyncCount++;
    CMDPacket packet(s_cmdSyncCount, mainFunc, subFunc, appID, data, retType);
    m_strCmdBuf.addTail((const unsigned char*)&packet, sizeof(packet));
    setRetData(retType, retData, retDataLen);

    return true;
}

bool CCmdTask::setCmdTask(const unsigned char *data, unsigned int dataLen)
{
    bool ret = false;

    if (data) {
        m_strCmdBuf.clear();
        ret = m_strCmdBuf.addTail(data, dataLen);
    }

    return ret;
}

bool CCmdTask::setCmdData(const CRawString &data)
{
    m_strCmdBuf = data;
    return true;
}

bool CCmdTask::setRetData(unsigned char retType,
                            const unsigned char *retData,
                            unsigned long retDataLen)
{
    bool ret = false;
    if ((unsigned int)m_strCmdBuf.getStrLen() >= sizeof(CMDPacket))
    {
        CMDPacket packet(0,0,0,0,0,0);
        m_strCmdBuf.setStr(&retType, 1,
                        (unsigned long)&packet.m_retType - (unsigned long)&packet);
        ret = m_strCmdBuf.setStr(retData, retDataLen, sizeof(CMDPacket));
    }

    return ret;
}

unsigned long CCmdTask::getCmdBufLen() const
{
    return (unsigned long) m_strCmdBuf.getStrLen();
}

const unsigned char* CCmdTask::getCmdBuf() const
{
    return m_strCmdBuf.getStr();
}

unsigned char CCmdTask::getMainFunc() const
{
    unsigned char mainFunc = 0;

    if (m_strCmdBuf.getStrLen() >= (int)sizeof(CMDPacket)) {
        CMDPacket *packet = (CMDPacket *)m_strCmdBuf.getStr();
        mainFunc = packet->m_mainFunc;
    }

    return mainFunc;
}

unsigned char CCmdTask::getSubFunc() const
{
    unsigned char subFunc = 0;

    if (m_strCmdBuf.getStrLen() >= (int)sizeof(CMDPacket)) {
        CMDPacket *packet = (CMDPacket *)m_strCmdBuf.getStr();
        subFunc = packet->m_subFunc;
    }

    return subFunc;
}

unsigned int CCmdTask::getData() const
{
    unsigned int data = 0;

    if (m_strCmdBuf.getStrLen() >= (int)sizeof(CMDPacket)) {
        CMDPacket *packet = (CMDPacket *)m_strCmdBuf.getStr();
        data = packet->m_data;
    }

    return data;
}

unsigned char CCmdTask::getAppID() const
{
    unsigned char appID = 0;

    if (m_strCmdBuf.getStrLen() >= (int)sizeof(CMDPacket)) {
        CMDPacket *packet = (CMDPacket *)m_strCmdBuf.getStr();
        appID = packet->m_appID;
    }

    return appID;
}

unsigned char CCmdTask::getRetType() const
{
    unsigned char retType = 0;

    if (m_strCmdBuf.getStrLen() >= (int)sizeof(CMDPacket)) {
        CMDPacket *packet = (CMDPacket *)m_strCmdBuf.getStr();
        retType = packet->m_retType;
    }

    return retType;
}

const unsigned char *CCmdTask::getRetData() const
{
    const unsigned char *data = NULL;
    if (m_strCmdBuf.getStrLen() > (int)sizeof(CMDPacket)) {
        data = m_strCmdBuf.getStr() + sizeof(CMDPacket);
    }

    return data;
}

unsigned long CCmdTask::getRetDataLen() const
{
    unsigned long len = 0;

    if (m_strCmdBuf.getStrLen() > (int)sizeof(CMDPacket)) {
        len = m_strCmdBuf.getStrLen() - sizeof(CMDPacket);
    }

    return len;
}

unsigned long CCmdTask::getSyncCount() const
{
    unsigned long syncCount = 0;

    if (m_strCmdBuf.getStrLen() >= (int)sizeof(CMDPacket)) {
        CMDPacket *packet = (CMDPacket *)m_strCmdBuf.getStr();
        syncCount = packet->m_cmdSync;
    }

    return syncCount;
}

const CRawString & CCmdTask::getSetCmdData() const
{
    return m_strCmdBuf;
}

void CCmdTask::resetSyncCount()
{
    s_cmdSyncCount = 0;
}

const char *CCmdTask::decode(E_MAINFUNC code)
{
    const char *result;

    switch (code) {
    case MAIN_FUNC_ACK:
        result = "MAIN_FUNC_ACK";
        break;
    case MAIN_FUNC_CDDVD:
        result = "MAIN_FUNC_CDDVD";
        break;
    case MAIN_FUNC_USB:
        result = "MAIN_FUNC_USB";
        break;
    case MAIN_FUNC_SD:
        result = "MAIN_FUNC_SD";
        break;
    case MAIN_FUNC_FM:
        result = "MAIN_FUNC_FM";
        break;
    case MAIN_FUNC_AM:
        result = "MAIN_FUNC_AM";
        break;
    case MAIN_FUNC_CMMB:
        result = "MAIN_FUNC_CMMB";
        break;
    case MAIN_FUNC_AVIN:
        result = "MAIN_FUNC_AVIN";
        break;
    case MAIN_FUNC_CDC:
        result = "MAIN_FUNC_CDC";
        break;
    case MAIN_FUNC_IPOD:
        result = "MAIN_FUNC_IPOD";
        break;
    case MAIN_FUNC_BT:
        result = "MAIN_FUNC_BT";
        break;
    case MAIN_FUNC_NAVI_FUNC:
        result = "MAIN_FUNC_NAVI_FUNC";
        break;
    case MAIN_FUNC_STEERING_WHEEL:
        result = "MAIN_FUNC_STEERING_WHEEL";
        break;
    case MAIN_FUNC_BACKCAR:
        result = "MAIN_FUNC_BACKCAR";
        break;
    case MAIN_FUNC_TIRE_PRESSURE:
        result = "MAIN_FUNC_TIRE_PRESSURE";
        break;
    case MAIN_FUNC_MAIN_VOLUME:
        result = "MAIN_FUNC_MAIN_VOLUME";
        break;
    case MAIN_FUNC_SOUND_EFFECT:
        result = "MAIN_FUNC_SOUND_EFFECT";
        break;
    case MAIN_FUNC_DISPLAY:
        result = "MAIN_FUNC_DISPLAY";
        break;
    case MAIN_FUNC_FACTORY:
        result = "MAIN_FUNC_FACTORY";
        break;
    case MAIN_FUNC_PUSH_TO_TALK:
        result = "MAIN_FUNC_PUSH_TO_TALK";
        break;
    case MAIN_FUNC_SPEECH_RECOGNITION:
        result = "MAIN_FUNC_SPEECH_RECOGNITION";
        break;
    case MAIN_FUNC_RESET:
        result = "MAIN_FUNC_RESET";
        break;
    case MAIN_FUNC_CALIBRATE:
        result = "MAIN_FUNC_CALIBRATE";
        break;
    case MAIN_FUNC_GPS:
        result = "MAIN_FUNC_GPS";
        break;
    case MAIN_FUNC_STEERING_WHEEL_STUDY:
        result = "MAIN_FUNC_STEERING_WHEEL_STUDY";
        break;
    case MAIN_FUNC_BACKCAR_LOCUS:
        result = "MAIN_FUNC_BACKCAR_LOCUS";
        break;
    case MAIN_FUNC_KEY:
        result = "MAIN_FUNC_KEY";
        break;
    case MAIN_FUNC_APP_ACTION:
        result = "MAIN_FUNC_APP_ACTION";
        break;
    case MAIN_FUNC_APP_JUMP:
        result = "MAIN_FUNC_APP_JUMP";
        break;

    default:
        result = "ERROR";
        break;
    }

    return result;
}

