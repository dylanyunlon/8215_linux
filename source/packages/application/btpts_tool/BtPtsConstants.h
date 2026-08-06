#ifndef BTPTSCONSTANTS_H
#define BTPTSCONSTANTS_H
#include <vector>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include "cmessage.h"

using namespace std;
/* input information */
const string BTPTS_CMD_INFO_STR =
        "************Bluetooth PtsTool  Test*******************/\n"
        "This system is used for Bluetooth authentication PTS test,\n"
        "which can test the Bluetooth function that the App UI does\n"
        "not do.It can also be as bluetooth automated test tool to \n"
        "customize test scenarios\n";

const string BTPTS_CMD_INQUIRY_STR = "\nPlease enter command:\n";
const string AUTO_TEST_CMD = "autotest";
const string AUTO_TEST_LOOP= "loop";
const string AUTO_TEST_TIMES= "autotesttimes";
const string AUTO_TEST_DUMP_CMD = "autodump";

#define PRINTF_TO_CONSOLE(tag, format, ...) {printf("[%s] ", tag); printf(format, ## __VA_ARGS__); printf("\n");}
#define PRINTF_TO_WARN(tag, format, ...) {printf("[%s] Warn - ", tag);    printf(format, ## __VA_ARGS__); printf("\n");}
#define PRINTF_TO_ERROR(tag, format, ...) {printf("[%s] Error - ", tag);   printf(format, ## __VA_ARGS__); printf("\n");}


#define SAFE_DELETE(a) {if (a) {delete a; a = NULL;}}

#define EXPECT_EQ(a, b) { if (a != b) {printf("[EXPECT_EQ] Error - Expect Failed, Bt %s Test Failed, quit system, Please Cheack line: %d\n", \
                                        __FUNCTION__, __LINE__); exit(1);}}
#define EXPECT_NOT_NULL(p, pname) {if (!p) {printf("[EXPECT_EQ] Error - Expect Failed, %s %s pointer is null, quit system, Please Cheack line: %d\n", \
                                            __FUNCTION__, pname,  __LINE__); exit(1);}}

const int BTPTS_OK = 0;
const int BTPTS_ERROR = -1;
const int DISCOVERY_MAX_DEVS = 15;
const int WAIT_MAX_TIME = 20000;
const int WAIT_MAX_DISC_TIME = 200000;
const int WAIT_TIME = 10000;
const int WAIT_MIN_TIME = 5000;
const int LOOP_ALWAYS = -1;
//profile name

const string PROFILE_GAP =    "gap";
const string PROFILE_HFP =    "hfp";
const string PROFILE_PBAP=    "pbap";
const string PROFILE_AVRCP =  "rc";
const string PROFILE_HID =    "hid";
const string PROFILE_GATT =   "gatt";

const string PROFILE_MANAGER = "console"; //is not bt profile
//console cmd
const string CMD_HELP =  "help";
const string CMD_EXIT =  "exit";
const string CMD_ADDR =  "addr";
const string CMD_DELAY = "delay";

const string CMD_PROFILE_CONNECT = "connect";
const string CMD_PROFILE_DISCONNECT = "disconnect";
const string CMD_BLUETOOTH_DUMP = "dump";
const vector<string> CONSOLE_CMDS = {CMD_HELP, CMD_EXIT, CMD_ADDR, CMD_DELAY,
                                       CMD_PROFILE_CONNECT, CMD_PROFILE_DISCONNECT, CMD_BLUETOOTH_DUMP};

//gap cmd
const string CMD_GAP_OPEN = "open";
const string CMD_GAP_CLOSE = "close";
const string CMD_GAP_DISCOVERY =  "discovery";
const string CMD_GAP_CANCEL_DISCOVERY =  "cancel_discovery";
const string CMD_GAP_BOND = "bond";
const string CMD_GAP_UNBOND = "unbond";
const string CMD_GAP_SCANMODE = "scanmode";
const string CMD_GAP_SETNAME = "setname";

//hfp cmd
const string CMD_HFP_DIAL = "dial";
const string CMD_HFP_ACCEPT = "accept";
const string CMD_HFP_TERMINATE = "terminate";
const string CMD_HFP_HOLD = "hold";
const string CMD_HFP_DTMF = "dtmf";
const string CMD_HFP_SWITCH = "switch";
const string CMD_HFP_REDIAL = "redial";
const string CMD_HFP_SPEAKERGAIN = "speakergain";
const string CMD_HFP_MICGAIN = "micgain";

//pbap cmd
const string CMD_PBAP_DOWNLOAD =  "download";
const string CMD_PBAP_STOPDOWNLOAD =  "stopdownload";

//avrcp cmd
const string CMD_RC_PLAY =  "play";
const string CMD_RC_PAUSE = "pause";
const string CMD_RC_STOP =  "stop";
const string CMD_RC_PRE =   "pre";
const string CMD_RC_NEXT =  "next";
const string CMD_RC_UPPLAYSTATUS =   "upps";
const string CMD_RC_UPID3 =  "upid3";
const string CMD_RC_SET_VOL =  "setvol";
const string CMD_RC_REG_INTERIM_VOL =  "reginterimvol";

//hid cmd
const string CMD_HID_SENDMOUSEDATA =  "mouse";
const string CMD_HID_SENDKEYBOARDDATA = "keyboard";
const string CMD_HID_SENDCONTROLDATA =  "control";

//gatt cmd
const string CMD_GATT_ADDSERVICE = "addservice";
const string CMD_GATT_REMOVESERVICE = "removeservice";
const string CMD_GATT_CLEARSERVICES = "clearservices";
const string CMD_GATT_STARTADVERTISING = "startadvertising";
const string CMD_GATT_STOPADVERTISING = "stopadvertising";

class BtPtsCmd
{
public:
    BtPtsCmd() : m_profile("")
               , m_cmd("")
               , m_helpInfo("")
    {

    }
    BtPtsCmd(const string &profile, const string &cmd, const char* helpInfo = "")
             : m_profile("")
             , m_cmd("")

    {
        m_profile = profile;
        m_cmd = cmd;
        m_helpInfo = helpInfo;
    }

    BtPtsCmd(const string &profile, const string &cmd, const vector<int> &paraType, const char* helpInfo = "")
             : m_profile("")
             , m_cmd("")

    {
        m_profile = profile;
        m_cmd = cmd;
        m_paraType = paraType;
        m_helpInfo = helpInfo;
    }
    string m_profile;
    string m_cmd;
    const char* m_helpInfo;
    vector<int> m_paraType;
    universal_utils::CMessage m_args;
};

typedef enum
{
  TYPE_INT,
  TYPE_CHARATER,
  TYPE_ADDR,
  TYPE_DTMF,
  TYPE_STRING
} E_TYPE;


const vector<BtPtsCmd> BTPTS_CMDS = {
    //gap cmds
    BtPtsCmd(PROFILE_GAP, CMD_GAP_OPEN, "open bluetooth"),
    BtPtsCmd(PROFILE_GAP, CMD_GAP_CLOSE, "close bluetooth"),
    BtPtsCmd(PROFILE_GAP, CMD_GAP_DISCOVERY, "discovery bluetooth devices"),
    BtPtsCmd(PROFILE_GAP, CMD_GAP_CANCEL_DISCOVERY, "cancel discovery bluetooth devices"),
    BtPtsCmd(PROFILE_GAP, CMD_GAP_BOND, {TYPE_STRING}, "pair and authentication device"),
    BtPtsCmd(PROFILE_GAP, CMD_GAP_UNBOND, {TYPE_STRING}, "unpair device"),
    BtPtsCmd(PROFILE_GAP, CMD_GAP_SCANMODE, {TYPE_INT}, "set bluetooth scan mode."),
    BtPtsCmd(PROFILE_GAP, CMD_GAP_SETNAME, {TYPE_STRING}, "set local bluetooth name."),
    //hfp cmds
    BtPtsCmd(PROFILE_HFP, CMD_PROFILE_CONNECT, "connect hfp profile"),
    BtPtsCmd(PROFILE_HFP, CMD_PROFILE_CONNECT, {TYPE_ADDR, TYPE_INT, TYPE_INT}, "connect hfp profile with retry times"),
    BtPtsCmd(PROFILE_HFP, CMD_PROFILE_DISCONNECT, "disconnect hfp profile"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_DIAL, {TYPE_STRING}, "dial phone number"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_ACCEPT, "answer the phone"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_TERMINATE, "hang up the phone"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_HOLD, {TYPE_INT}, "hold call or establish three-way call"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_DTMF, {TYPE_DTMF}, "send dtmf charater during call"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_SWITCH, "switch voice between AG and HF"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_REDIAL, "redial call"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_SPEAKERGAIN, {TYPE_INT}, "setting speaker volume"),
    BtPtsCmd(PROFILE_HFP, CMD_HFP_MICGAIN, {TYPE_INT}, "setting microphone volume"),
    //pbap cmds
    BtPtsCmd(PROFILE_PBAP, CMD_PROFILE_CONNECT, "connect pbap profile"),
    BtPtsCmd(PROFILE_PBAP, CMD_PROFILE_CONNECT, {TYPE_ADDR}),
    BtPtsCmd(PROFILE_PBAP, CMD_PROFILE_DISCONNECT, "disconnect pbap profile"),
    BtPtsCmd(PROFILE_PBAP, CMD_PBAP_DOWNLOAD, {TYPE_STRING}, "download phone book"),
    BtPtsCmd(PROFILE_PBAP, CMD_PBAP_STOPDOWNLOAD, "stop download phone book"),

    //rc cmds
    BtPtsCmd(PROFILE_AVRCP, CMD_PROFILE_CONNECT, "connect a2dp and avrcp profile"),
    BtPtsCmd(PROFILE_AVRCP, CMD_PROFILE_CONNECT, {TYPE_ADDR, TYPE_INT, TYPE_INT}, "connect rc profile with retry times"),
    BtPtsCmd(PROFILE_AVRCP, CMD_PROFILE_DISCONNECT, "disconnect a2dp and avrcp profile"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_PLAY, "play music"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_PAUSE, "pause music"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_STOP, "stop music"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_PRE, "previous track"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_NEXT, "next track"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_UPPLAYSTATUS, "update play status"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_UPID3, "update ID3 media information"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_SET_VOL, {TYPE_INT}, "carkit modify volume"),
    BtPtsCmd(PROFILE_AVRCP, CMD_RC_REG_INTERIM_VOL, {TYPE_INT}, "when phone notify volume change event, carkit need reply interim"),

    //hid cmds
    BtPtsCmd(PROFILE_HID, CMD_PROFILE_CONNECT, "connect hid profile"),
    BtPtsCmd(PROFILE_HID, CMD_PROFILE_CONNECT, {TYPE_ADDR}),
    BtPtsCmd(PROFILE_HID, CMD_PROFILE_DISCONNECT, "disconnect hid profile"),
    BtPtsCmd(PROFILE_HID, CMD_HID_SENDMOUSEDATA, {TYPE_INT, TYPE_INT, TYPE_INT}, "send mouse data"),
    BtPtsCmd(PROFILE_HID, CMD_HID_SENDKEYBOARDDATA, {TYPE_CHARATER}, "send keyboard data"),
    BtPtsCmd(PROFILE_HID, CMD_HID_SENDCONTROLDATA, {TYPE_INT}, "send control data"),

    //gatt cmds
    BtPtsCmd(PROFILE_GATT, CMD_GATT_ADDSERVICE, "addService"),
    BtPtsCmd(PROFILE_GATT, CMD_GATT_REMOVESERVICE, "removeService"),
    BtPtsCmd(PROFILE_GATT, CMD_GATT_CLEARSERVICES, "clearServices"),
    BtPtsCmd(PROFILE_GATT, CMD_GATT_STARTADVERTISING, "startAdvertising"),
    BtPtsCmd(PROFILE_GATT, CMD_GATT_STOPADVERTISING, "stopAdvertising"),

    //other cmds
    BtPtsCmd(PROFILE_MANAGER, CMD_PROFILE_CONNECT, "connect all profiles"),
    BtPtsCmd(PROFILE_MANAGER, CMD_PROFILE_CONNECT, {TYPE_ADDR}),
    BtPtsCmd(PROFILE_MANAGER, CMD_PROFILE_DISCONNECT, "disconnect all profiles"),
    BtPtsCmd(PROFILE_MANAGER, CMD_HELP, "help command"),
    BtPtsCmd(PROFILE_MANAGER, CMD_EXIT, "exit system"),
    BtPtsCmd(PROFILE_MANAGER, CMD_DELAY, {TYPE_INT}, "delay command"),
    BtPtsCmd(PROFILE_MANAGER, CMD_ADDR,  {TYPE_ADDR}, "settting address"),
    BtPtsCmd(PROFILE_MANAGER, CMD_BLUETOOTH_DUMP, "dump bluetooth info")
};

typedef enum
{
  ERROR_ARG_NUMS = -4,
  IVALID_ARG = -3,
  UNKNOW_CHAR = -2,
  UNKONW_CMD = -1,
  PARSE_OK = 0,
  EMPTY_CMD = 1
} E_PARSE_RESULT;

typedef enum
{
  NO_PARA = 1,
  ONE_PARA,
  TWO_PARAS,
  THREE_PARAS,
} E_ARG_NUMS;
#endif // BTPTSCONSTANTS_H


