#ifndef PACKET_H
#define PACKET_H
#include "common.h"
#include "chararray.h"

class Packet
{
public:
    virtual ~Packet() = default;
    unsigned char getHeader() const;
    unsigned char getCmdId() const;
    short getCheckCode() const;

    void setHeader(unsigned char header);
    void setCmdId(unsigned char cmdId);
    void setCheckCode(short checkCode);

protected:
    short generateCheckCode(const char *data, unsigned short len) const;

    unsigned char m_header = PacketRequestHeader;  //头类型，AA表示发送， BB表示接收
    unsigned char m_cmdId = -1; //操作码
    short m_checkCode = 0; //校验码

    enum PacketCommon {
        PacketRequestHeader = CommonConstant::Request,
        PacketReplyHeader = CommonConstant::Reply,

        PacketHeaderLen = 1,
        PacketCmdIdLen = 1,
        PacketResultLen = 1,
        PacketCheckCodeLen = 2,
        PacketLengthLen = 2,
        PacketRequestMinLen = PacketHeaderLen + PacketLengthLen + PacketCmdIdLen  + PacketCheckCodeLen,
        PacketRevMinLen = PacketRequestMinLen + PacketResultLen,
    };
};


#endif // PACKET_H
