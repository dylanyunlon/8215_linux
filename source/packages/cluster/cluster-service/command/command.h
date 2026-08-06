#ifndef COMMAND_H
#define COMMAND_H
#include "transport.h"
#include "common.h"
class Command
{
public:
    Command() = default;
    virtual ~Command() = default;
    virtual bool execute() = 0;
    void setTransport(ITransport *transport) {m_transport = transport;}
    unsigned char getCmdId() const {return m_cmdId;}
    unsigned char getCmdType() const {return m_cmdType;}

protected:
    ITransport *m_transport = nullptr;
    unsigned char m_cmdId;
    unsigned char m_cmdType = CommonConstant::Request;
};

#endif // COMMAND_H
