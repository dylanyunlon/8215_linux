#include <iostream>
#include "BtPtsCmdConsole.h"
#include "BtPtsHandlerManager.h"
static const char* TAG = "btpts_Main";

static bool processAutoTestFlow(int argc, char *argv[], BtPtsCmdConsole &cmdConsole) {
    bool ret = false;

    PRINTF_TO_CONSOLE(TAG, "bt pts atuo test cmd argc (%d) !!!", argc);

    string filePath = string(argv[2]); //cmds file path
    string dumpCmdFilePath = "";

    string parmStr = "";
    int runTimes = 0;

    PRINTF_TO_CONSOLE(TAG, "bt pts atuo test cmd preapre !!!");
    for (int k = TWO_PARAS; (k + 1) <= argc; ++k) {
        parmStr = string(argv[k]);
        if (AUTO_TEST_TIMES == parmStr) { //time key
            ++k; //next value is times
            runTimes = atoi(argv[k]); //test times parm
            PRINTF_TO_CONSOLE(TAG, "run times: (%d)", runTimes);
        } else if (AUTO_TEST_DUMP_CMD == parmStr) { //autodump key
            ++k; //next value is the dump cmd file path
            dumpCmdFilePath = string(argv[k]); //dump cmd file path
            PRINTF_TO_CONSOLE(TAG, "dum cmd file path (%s) when test fail",
                    dumpCmdFilePath.c_str());
        }
    }
    PRINTF_TO_CONSOLE(TAG, "bt pts atuo test start !!!");

    if (runTimes > 0) {
        ret = cmdConsole.runCmdFile(filePath, runTimes);
    } else {
        ret = cmdConsole.runCmdFile(filePath);
    }

    if (ret) {
        PRINTF_TO_CONSOLE(TAG, "bt pts atuo test pass !!!");
    } else {
        PRINTF_TO_CONSOLE(TAG, "bt pts auto test fail , start dump !!!");
        cmdConsole.dumpInfoCmdFile(dumpCmdFilePath);
        PRINTF_TO_CONSOLE(TAG, "bt pts auto test fail !!!");
    }

    return ret;
}

int main(int argc, char *argv[])
{
    BtPtsCmdConsole cmdConsole;

    switch (argc) {
        case NO_PARA:
            cmdConsole.run();
            break;

        case ONE_PARA:
            if (argv[1] == AUTO_TEST_CMD) {
                PRINTF_TO_ERROR(TAG, "parameter error, please input cmd file path!");
            } else {
                cmdConsole.run(string(argv[1]));
            }
            break;

        default: {
            if (argv[1] == AUTO_TEST_CMD) {
                processAutoTestFlow(argc, argv, cmdConsole);
            } else {
                string cmdStr = "";
                for (int i = 1; i < argc; i++) {
                    cmdStr += string(argv[i]);
                    cmdStr += " ";
                }
                cmdConsole.run(cmdStr);
            }
        } break;
    }

    return 0;
}
