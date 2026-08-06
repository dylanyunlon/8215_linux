#ifndef SENDPACKET_H
#define SENDPACKET_H
#include "packet.h"

class ClusterToIVIPacket : public Packet
{
public:
    void pack(unsigned char cmdType, unsigned char cmdId, const CharArray& array); //一定是数据的长度
    const CharArray&  getPacket() const;
    unsigned short getPacketLength() const;

private:
    CharArray m_packet;
};

#endif // SENDPACKET_H
