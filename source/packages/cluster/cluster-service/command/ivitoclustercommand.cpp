#include "ivitoclustercommand.h"
#include "ivitoclusterpacket.h"
#include <unistd.h>
//using universal_utils::CLog;
const static char *TAG = "IVIToClusterCommand";
IVIToClusterCommand::IVIToClusterCommand(DataReceiver *receiver)
    : m_receiver(receiver)
{

}

bool IVIToClusterCommand::execute()
{
    if (!m_transport) {
        return false;
    }

    CharArray array = m_transport->read();
    if (!array.isNull()) {
        IVIToClusterPacket reciverPacket;
        if (reciverPacket.unpack(array)) { //进行解包
            m_cmdType = reciverPacket.getHeader();
            m_cmdId = reciverPacket.getCmdId();
            //UTILS_LOGD(TAG, "reciver id %d", m_cmdId);
            if (m_receiver) {
                m_cmdType == CommonConstant::Reply ? m_receiver->onReceiveReply(m_cmdId, reciverPacket.getResult(), reciverPacket.getData())
                                                   :  (m_cmdType == CommonConstant::Request ? m_receiver->onReceiveRequest(m_cmdId, reciverPacket.getData())
                                                                                            : m_receiver->onReceiveSplitData(m_cmdId, reciverPacket.getData()));
            }
            return true;
        }
    } else {
       // UTILS_LOGD(TAG, "read null");
        usleep(10000);
    }

    return false;
}
