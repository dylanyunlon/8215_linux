#ifndef IVITOCLUSTERCOMMAND_H
#define IVITOCLUSTERCOMMAND_H
#include "command.h"
class DataReceiver
{
public:
    virtual void onReceiveReply(unsigned char cmd, unsigned char result, const CharArray& = CharArray()) = 0;
    virtual void onReceiveRequest(unsigned char cmd, const CharArray& = CharArray()) = 0;
    virtual void onReceiveSplitData(unsigned char cmd, const CharArray& = CharArray()) = 0;
};

class IVIToClusterCommand : public Command
{
public:
    IVIToClusterCommand(DataReceiver *receiver = nullptr);
    bool execute() override;

private:
    DataReceiver *m_receiver = nullptr;
};

#endif // IVITOCLUSTERCOMMAND_H
