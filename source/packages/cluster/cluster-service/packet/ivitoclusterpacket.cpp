#include "ivitoclusterpacket.h"
#include "macrodefine.h"
#include <string.h>
const static char * TAG = "ReceivePacket";
//using universal_utils::CLog;
bool IVIToClusterPacket::unpack(const CharArray &array)
{
    if (!check(array))
        return false;

    m_header = array[0];
    unsigned short offset = PacketHeaderLen + PacketLengthLen;
    m_cmdId = array[offset];
    offset += PacketCmdIdLen;
    if (m_header == CommonConstant::Reply) {
        m_result = array[offset];
        offset += PacketResultLen;
    }

    memcpy(&m_checkCode, array.getData() + array.getLength() - PacketCheckCodeLen, PacketCheckCodeLen);
    unsigned short dataLen = array.getLength() - offset - PacketCheckCodeLen; //PacketLength
    if (dataLen > 0) {
        m_data = CharArray(dataLen); //
        m_data.copyData(array.getData() + offset, 0, dataLen);
    }

    return true;
}

bool IVIToClusterPacket::check(const CharArray &array) const
{
    bool ret = false;
    if (!array.isNull() && array.getLength() >= PacketRevMinLen) {
        short checkCode = 0;
        memcpy(&checkCode, array.getData() + array.getLength() - PacketCheckCodeLen, PacketCheckCodeLen);
        ret = (checkCode == generateCheckCode(array.getData(), array.getLength() - PacketCheckCodeLen));
        if (!ret) {
          //  UTILS_LOGE(TAG, "check error: %d %d", checkCode, generateCheckCode(array.getData(), array.getLength() - PacketCheckCodeLen));
        }
    } else {
        // UTILS_LOGE(TAG, "unpact error");
    }

    return ret;
}

unsigned char IVIToClusterPacket::getResult() const
{
    return m_result;
}

void IVIToClusterPacket::setResult(unsigned char result)
{
    m_result = result;
}

const CharArray& IVIToClusterPacket::getData() const
{
    return m_data;
}

unsigned short IVIToClusterPacket::getDataLength() const
{
    return m_data.getLength();
}
