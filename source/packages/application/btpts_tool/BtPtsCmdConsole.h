#ifndef BTPTSCMDRECEIVER_H
#define BTPTSCMDRECEIVER_H
#include "BtPtsConstants.h"
#include "BtPtsHandlerManager.h"
#include "BtPtsCmdParser.h"
class BtPtsCmdConsole
{
public:
    BtPtsCmdConsole();
    ~BtPtsCmdConsole();
    void run();
    bool runCmdFile(const string &filePath, int times = LOOP_ALWAYS);
    void run(const string &cmdStr);
    bool dumpInfoCmdFile(const string &dumpCmdFilePath);

private:
    bool parseCmdsFromFile(const string &cmdFile, vector<BtPtsCmd> &cmds);
    BtPtsCmdParser *m_parser;
    BtPtsHandlerManager *m_hander;
};

#endif // BTPTSCMDRECEIVER_H
