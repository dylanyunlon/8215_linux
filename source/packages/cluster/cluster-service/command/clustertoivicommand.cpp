#include "clustertoivicommand.h"
#include "ivitoclusterpacket.h"
#include "clustertoivipacket.h"
#include "commonfun.h"

const static char *TAG = "ClusterToIVICommand";

ClusterToIVICommand::ClusterToIVICommand(unsigned char cmdId, time_t timeoutMills)
    : m_timeoutMillis(timeoutMills)
{
    m_cmdId = cmdId;
}

bool ClusterToIVICommand::execute()
{
    CharArray array = dataPack();
    ClusterToIVIPacket packet;
    packet.pack(m_cmdType, m_cmdId, array);
    if (m_transport) {
        m_transport->write(packet.getPacket());
    }

    if (m_cmdType == CommonConstant::Request) {
        m_timeoutPoint = CommonFun::getTimeStamp() + m_timeoutMillis;
    }

    return true;
}

CharArray ClusterToIVICommand::dataPack()
{
    if (m_cmdType == CommonConstant::Reply) {
        CharArray array(m_data.getLength() + 1);
        array.copyData(&m_result, 0, sizeof(m_result));
        if (!m_data.isNull()) {
            array.copyData(m_data.getData(), 1, m_data.getLength());
        }
        return array;

    } else {
        return m_data;
    }
}

void ClusterToIVICommand::setData(const CharArray &data)
{
    m_data = data;
}

void ClusterToIVICommand::setReplyResult(char result)
{
    m_result = result;
}

bool ClusterToIVICommand::checkTimeout()
{
   return CommonFun::getTimeStamp() >= m_timeoutPoint;
}

void ClusterToIVICommand::setType(unsigned char type)
{
    m_cmdType = type;
}

