#include "clustertoivipacket.h"
#include "macrodefine.h"
void ClusterToIVIPacket::pack(unsigned char cmdType, unsigned char cmdId, const CharArray& array)
{
    m_header = cmdType;
    m_cmdId = cmdId;
    unsigned short packetLen = PacketRequestMinLen + array.getLength();
    m_packet = CharArray(packetLen); //参数构造， 移动=赋值

    unsigned short offset = 0;
    m_packet.copyData(&m_header, offset, PacketHeaderLen);  //拷贝头 0x01  1 byte
    offset += PacketHeaderLen;
    m_packet.copyData(&packetLen, offset, PacketLengthLen); //拷贝长度 4 + dataLen   1 byte
    offset += PacketLengthLen;
    m_packet.copyData(&m_cmdId, offset, PacketCmdIdLen);    //拷贝cmdId 1 byte
    offset += PacketCmdIdLen;
    if (!array.isNull()) {                                  //拷贝数据区
        m_packet.copyData(array.getData(), offset, array.getLength());
        offset += array.getLength();
    }

    m_checkCode = generateCheckCode(m_packet.getData(), packetLen - PacketCheckCodeLen);
    m_packet.copyData(&m_checkCode, offset, PacketCheckCodeLen); 
}

const CharArray& ClusterToIVIPacket::getPacket() const
{
     return m_packet;
}

unsigned short ClusterToIVIPacket::getPacketLength() const
{
    return m_packet.getLength();
}

