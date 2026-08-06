#include "BtPtsCmdParser.h"
#include <stdio.h>
#include <algorithm>
#include <sstream>

static const char* TAG = "BtPtsCmdParser";

BtPtsCmdParser::BtPtsCmdParser()
{

}

int BtPtsCmdParser::parse(const string &cmdStr, BtPtsCmd& ptsCmd)
{
    int ret = UNKONW_CMD;
    string tempStr = cmdStr;
    trim(tempStr);
    transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
    vector<string> strArray = split(tempStr, " ");
    if (strArray.size() < 1)
        return EMPTY_CMD;

    if (find(CONSOLE_CMDS.begin(), CONSOLE_CMDS.end(), strArray[0]) != CONSOLE_CMDS.end()) {
        strArray.insert(strArray.begin(), PROFILE_MANAGER);
    }

    ret = check(strArray, ptsCmd);

    if (ret == PARSE_OK)
        getArg(strArray, ptsCmd);
     else
        dumpLog(ret, cmdStr);

    return ret;
}

void BtPtsCmdParser::dumpLog(int error, const string & cmdStr)
{
    string errorInfo = "";
    switch (error) {
        case UNKONW_CMD:
        errorInfo = "unknow cmd!";
        break;
    case IVALID_ARG:
        errorInfo = "invalid parameter!";
        break;
    case ERROR_ARG_NUMS:
        errorInfo = "numbers of parameter error!";
        break;
    }

    PRINTF_TO_ERROR(TAG, "%s command parse error: %s", cmdStr.c_str(), errorInfo.c_str());
}
vector<string> BtPtsCmdParser::split(const string &str,const string &pattern)
{
    vector<string> resVec;

    if (str.empty()){
        return resVec;
    }

    string strs = str + pattern;
    size_t pos = strs.find(pattern);
    size_t size = strs.size();

    while (pos != string::npos){
        string x = strs.substr(0,pos);
        if (x != "")
            resVec.push_back(x);
        strs = strs.substr(pos+1, size);
        pos = strs.find(pattern);
    }

    return resVec;
}

void BtPtsCmdParser::trim(string &str)
{
    if(!str.empty()){
        const char whitespace[] = " \n\t\v\r\f";
        str.erase( 0, str.find_first_not_of(whitespace) );
        str.erase( str.find_last_not_of(whitespace) + 1U );
    }
}

bool BtPtsCmdParser::isValidAddress(const string &address)
{
    if (address.size() != 17) {
        return false;
    }

    for(char c : address) {
        if(!(((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == ':'))) {
            return false;
        }
    }

    if (address == "00:00:00:00:00:00")
        return false;

    return true;
}

int BtPtsCmdParser::str2int(const string &str)
{
    int ret = 0;
    stringstream stream(str);
    stream >> ret;

    return ret;
}

 bool BtPtsCmdParser::isNumber(const string &str)
 {
     for(char c : str) {
        if(c < '0' || c > '9')
            return false;
     }

     return true;
}

bool BtPtsCmdParser::isDTMFCode(const string &str)
{
     for(char c : str) {
        if(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || c == '*' || c == '#'))
             return false;
     }

     return true;
}

int BtPtsCmdParser::check(const vector<string> &strArray, BtPtsCmd& ptsCmd)
{
     int ret = UNKONW_CMD;

     for(BtPtsCmd cmd : BTPTS_CMDS) {
         if (cmd.m_profile == strArray[0] && cmd.m_cmd == strArray[1]) {
             ret = checkValid(cmd, strArray);
             if (ret == PARSE_OK) {
                 ptsCmd = cmd;    //support mutiple parameters
                 break;
             }
         }
     }

     return ret;
}
int BtPtsCmdParser::checkValid(const BtPtsCmd &ptsCmd, const vector<string> &strArray)
{
    int ret = PARSE_OK;
    if (ptsCmd.m_paraType.size() == strArray.size() - 2) {
        if (!checkType(ptsCmd.m_paraType, strArray))
            ret = IVALID_ARG;
    } else
        ret = ERROR_ARG_NUMS;

    return ret;
}
bool BtPtsCmdParser::checkType(const vector<int> &types, const vector<string> &strArray)
{
    bool ret = true;
    if (types.size() < 1)
        return ret;

    string arg = "";
    for (size_t i = 0; i < types.size(); i++) {
        arg = strArray[i+2];
        switch (types[i]) {
        case TYPE_INT:
            ret = isNumber(arg);
            break;
        case TYPE_ADDR:
            ret = isValidAddress(arg);
            break;
        case TYPE_DTMF:
            ret = isDTMFCode(arg);
            break;
        case TYPE_CHARATER:
            ret = (arg.size() == 1);
            break;

        default:
            break;
        }
        if (!ret)
            break;
    }

    return ret;
}

void BtPtsCmdParser::getArg(const vector<string> &strArray, BtPtsCmd &ptsCmd)
{
    string arg = "";
    for (size_t i = 0; i < ptsCmd.m_paraType.size(); i++) {
        arg = strArray[i+2];
        switch (ptsCmd.m_paraType[i]) {
        case TYPE_INT:
            ptsCmd.m_args.putExtra("int" + to_string(i+1), str2int(arg));
            break;
        case TYPE_CHARATER:
            ptsCmd.m_args.putExtra("char" + to_string(i+1), arg[0]);
            break;
        default:
            ptsCmd.m_args.putExtra("str" + to_string(i+1), arg);
            break;
        }
    }
}



