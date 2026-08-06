#include "BtPtsCmdConsole.h"
#include <fstream>
static const char* TAG = "BtPtsCmdConsole";

BtPtsCmdConsole::BtPtsCmdConsole()
                : m_parser(NULL)
                , m_hander(NULL)
{
    m_parser = new BtPtsCmdParser();
    m_hander = new BtPtsHandlerManager();
}

BtPtsCmdConsole::~BtPtsCmdConsole()
{
    SAFE_DELETE(m_parser);
    SAFE_DELETE(m_hander);
}

void BtPtsCmdConsole::run()
{
    string cmdStr;
    printf(BTPTS_CMD_INFO_STR.c_str());
    printf(BTPTS_CMD_INQUIRY_STR.c_str());
    while(1) {
      getline(cin, cmdStr);
      run(cmdStr);
      printf(BTPTS_CMD_INQUIRY_STR.c_str());
    }
}

void BtPtsCmdConsole::run(const string &cmdStr)
{
    BtPtsCmd ptsCmd;
    if (m_parser->parse(cmdStr, ptsCmd) == PARSE_OK) {
        m_hander->handle(ptsCmd);
    }
}

bool BtPtsCmdConsole::runCmdFile(const string &filePath, int times)
{
    vector<BtPtsCmd> cmds;
    if (!parseCmdsFromFile(filePath, cmds)) {
        return false;
    }

    unsigned long long loopTimes = 0;
    bool isTestFail = false;

    //parse sucessful, to handle!
    do {
        int lineNum = 0;
        ++loopTimes;
        PRINTF_TO_CONSOLE(TAG, "handle autotest times %llu\n", loopTimes);
        for (BtPtsCmd cmd : cmds) {
            PRINTF_TO_CONSOLE(TAG, "handle line: %d profile: %s cmd: %s", (++lineNum),
                               cmd.m_profile.c_str(), cmd.m_cmd.c_str());

            if (BTPTS_OK != m_hander->handle(cmd)) {
                isTestFail = true;
                break;
            }
        }

        if (times != LOOP_ALWAYS)
            --times;

    } while(times && !isTestFail);

    if (isTestFail) {
        PRINTF_TO_CONSOLE(TAG, "btpts autotest failed! test times is %llu\n", loopTimes);
    } else {
        PRINTF_TO_CONSOLE(TAG, "btpts autotest success! test times is %llu\n", loopTimes);
    }

    return !isTestFail;
}

bool BtPtsCmdConsole::dumpInfoCmdFile(const string &dumpCmdFilePath)
{
    //dump info
    BtPtsCmd cmdDump;
    m_parser->parse(CMD_BLUETOOTH_DUMP, cmdDump);

    vector<BtPtsCmd> cmds;
    //dump info is must when test fail, so add it
    cmds.push_back(cmdDump);

    //parse all cmds from configs file, dump more info
    parseCmdsFromFile(dumpCmdFilePath, cmds);

    int lineNum = 0;
    for (BtPtsCmd cmd : cmds) {
        PRINTF_TO_CONSOLE(TAG, "handle line: %d profile: %s cmd: %s", (++lineNum),
                           cmd.m_profile.c_str(), cmd.m_cmd.c_str());
        if (BTPTS_OK != m_hander->handle(cmd)) {
            PRINTF_TO_CONSOLE(TAG, " cmd: %s failed", cmd.m_cmd.c_str());
        }
    }

    PRINTF_TO_ERROR(TAG, "atuotest failed, exit PtsTest !!");
    exit(1);
}


bool BtPtsCmdConsole::parseCmdsFromFile(const string &cmdFilePath, vector<BtPtsCmd> &cmds) {
    if (cmdFilePath.empty()) {
        PRINTF_TO_WARN(TAG, "cmdFilePath is empty.");
        return false;
    }

    string cmdStr = "";
    fstream cmdFile(cmdFilePath);
    if (!cmdFile) {
        PRINTF_TO_ERROR(TAG, "%s file open fail, please check", cmdFilePath.c_str());
        return false;
    }

    int lineNum = 0;
    //parse all cmds
    while (getline(cmdFile, cmdStr)) {
        ++lineNum;
        int ret;
        BtPtsCmd cmd;
        if ((ret = m_parser->parse(cmdStr, cmd)) == PARSE_OK) {
            cmds.push_back(cmd);
        } else if (ret < 0){
            PRINTF_TO_ERROR(TAG, "%s file parse fail, please check line %d",
                    cmdFilePath.c_str(), lineNum);
            return false;
        }
    }

    return true;
}




