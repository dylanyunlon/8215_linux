#ifndef BTPTSCMDPARSER_H
#define BTPTSCMDPARSER_H
#include "BtPtsConstants.h"
class BtPtsCmdParser
{
public:
    BtPtsCmdParser();
    int parse(const string &cmdStr, BtPtsCmd& ptsCmd);

private:
    vector<string> split(const string &str,const string &pattern);
    void trim(string &str);
    bool isValidAddress(const string &address);
    bool isNumber(const string &str);
    bool isDTMFCode(const string &str);
    int  str2int(const string &str);
    int  check(const vector<string> &strArray, BtPtsCmd& ptsCmd);
    int  checkValid(const BtPtsCmd &ptsCmd, const vector<string> &strArray);
    bool checkType(const vector<int> &types, const vector<string> &strArray);
    void getArg(const vector<string> &strArray, BtPtsCmd &ptsCmd);
    void dumpLog(int error, const string &cmdStr);
};

#endif // BTPTSCMDPARSER_H
