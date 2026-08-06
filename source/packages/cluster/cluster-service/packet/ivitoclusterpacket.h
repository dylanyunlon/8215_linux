#ifndef RECPACKET_H
#define RECPACKET_H
#include "packet.h"

class IVIToClusterPacket : public Packet
{
public:
    bool unpack(const CharArray &array);
    void setResult(unsigned char result);

    unsigned char getResult() const;
    const CharArray& getData() const;
    unsigned short getDataLength() const;

private:
    bool check(const CharArray &array) const;
    unsigned char m_result = CommonConstant::CmdUnKnown;
    CharArray m_data;
};

#endif // RECPACKET_H
