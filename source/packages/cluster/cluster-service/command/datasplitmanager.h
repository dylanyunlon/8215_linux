#ifndef SPLITDATAHANDLER_H
#define SPLITDATAHANDLER_H
#include "common.h"
#include "callback.h"
#include "chararray.h"
#include <set>
#include <map>
#include <fstream>
#include <openssl/md5.h>
//如果是文件还是图片数据，对于是文件的形式， 文件形式+fd形式， 如果是数据

//如果是文件,怎么拆分
class CommandHandler;
class SplitReceiver {
public:
    SplitReceiver(CommandHandler *commandHandler);
    virtual bool startReceive(const CharArray &data) = 0;
    virtual bool receive(const CharArray &data) = 0;
    virtual bool endReceive(const CharArray &data) = 0;
    virtual bool continueStartReceive(const CharArray &data) = 0;

    unsigned int size() const {return m_size;}
    void setCmd(unsigned char cmd) {m_cmd = cmd;}

protected:
    CommandHandler *m_commandHandler = nullptr;
    unsigned int m_size = 0;
    unsigned char m_cmd = 0;
    int m_currentIndex = -1;
};

class DataSplitReceiver : public SplitReceiver {
public:
    using SplitReceiver::SplitReceiver;
    bool startReceive(const CharArray &data) override;
    bool continueStartReceive(const CharArray &data) override;
    bool receive(const CharArray &data) override;
    bool endReceive(const CharArray &data) override;

private:
    char *m_data = nullptr;
    unsigned int m_offset = 0;
};

class FileSplitReceiver : public SplitReceiver {
public:
    using SplitReceiver::SplitReceiver;
    bool startReceive(const CharArray &data) override;
    bool continueStartReceive(const CharArray &data) override;
    bool receive(const CharArray &data) override;
    bool endReceive(const CharArray &data) override;

private:
    size_t getFileSize(const std::string &path);
    std::fstream m_stream; //stream
    MD5_CTX m_md5Context;
};


class DataSplitManager
{
public:
    DataSplitManager(CommandHandler *commandHandler);
    void receiveSplitData(unsigned char cmd, const CharArray &data);

private:
    bool isFileData(unsigned char cmd);
    const std::set<unsigned char> FILE_DATA_CMD = {CommonConstant::DownloadUpdatePackage};
    std::map<unsigned char, SplitReceiver *> m_splitReceivers;
    CommandHandler *m_commandHandler = nullptr;
};

#endif // SPLITDATAHANDLER_H
