#ifndef CLUSTERTOIVICOMMAND_H
#define CLUSTERTOIVICOMMAND_H
#include "command.h"
#include <time.h>

class ClusterToIVICommand : public Command
{
    const static time_t DEFAULT_TIME_OUT = 2000;
public:
    ClusterToIVICommand(unsigned char cmdId, time_t timeoutMills = DEFAULT_TIME_OUT);
    bool execute();
    virtual CharArray dataPack();
    void setData(const CharArray &data);
    void setReplyResult(char result);
    bool checkTimeout();
    void setType(unsigned char type);

private:
    time_t m_timeoutMillis;
    time_t m_timeoutPoint;
    int m_timeoutTimes = 0;

    int MAX_TIME_OUT_TIMES = 2;
    CharArray m_data;
    char m_result = CommonConstant::CmdSucess;
};

#endif // CLUSTERTOIVICOMMAND_H
