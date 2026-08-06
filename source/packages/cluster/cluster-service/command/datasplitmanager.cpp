#include "datasplitmanager.h"
#include "commandhandler.h"
#include <memory.h>
#include <functional>
const static char *TAG = "DataSplitManager";
//using universal_utils::CLog;
const static int PATH_MAX_SIZE = 255;
DataSplitManager::DataSplitManager(CommandHandler *commandHandler)
    : m_commandHandler(commandHandler)
{

}

SplitReceiver::SplitReceiver(CommandHandler *commandHandler)
    : m_commandHandler(commandHandler)
{

}

bool DataSplitReceiver::startReceive(const CharArray &data)
{
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
        m_offset = 0;
    }

    bool ret = false;
    if (data.getLength() > sizeof(m_size)) {
        memcpy(&m_size, data.getData() + 1, sizeof(m_size));
        m_data = new char[m_size];
       // UTILS_LOGD(TAG, "DataSplitReceiver startReceive dataLen %d", m_size);
        ret = true;
    } else {
      //  UTILS_LOGE(TAG, "DataSplitReceiver startReceive error %d", data.getLength());
    }
    m_currentIndex = -1;

    return ret;
}

bool DataSplitReceiver::continueStartReceive(const CharArray &data)
{
    //todo unsupport
    return false;
}

bool DataSplitReceiver::receive(const CharArray &data)
{
    bool ret = false;
    int index = 0;
    memcpy(&index, data.getData() + 1, sizeof(index));
    int dataLen = data.getLength() - 1 - sizeof(index);
    //UTILS_LOGD(TAG, "DataSplitReceiver receive dataLen=%d, index=%d %d", dataLen, index, m_offset);
    if (m_offset + dataLen <= m_size) {
        if ((index - m_currentIndex) != 1) {
           // UTILS_LOGE(TAG, "receive index error %d %d", index, m_currentIndex);
        } else {
            memcpy(m_data + m_offset, data.getData() + 1 + sizeof(index), dataLen);
            m_offset += dataLen;
            m_currentIndex = index;
            ret = true;
        }
    } else {
      //  UTILS_LOGE(TAG, "DataSplitReceiver receive length error, size %d, offset %d length %d", m_size, m_offset, dataLen);
    }

    return ret;
}

bool DataSplitReceiver::endReceive(const CharArray &data)
{
    //todo check
  //  UTILS_LOGD(TAG, "DataSplitReceiver endReceive");
    m_commandHandler->receiveData(m_cmd, m_data, m_size);

    return true;
}

bool FileSplitReceiver::startReceive(const CharArray &data)
{
    bool ret = false;
    if (data.getLength() > sizeof(m_size)) {
        memcpy(&m_size, data.getData() + 1, sizeof(m_size));
       // UTILS_LOGD(TAG, "FileSplitReceiver startReceive dataLen %d", m_size);
        char path[PATH_MAX_SIZE] = {0};
        snprintf(path, PATH_MAX_SIZE, "%s/clustertemp%d", CommonConstant::FILE_PATH, m_cmd);
        m_stream.open(path, std::ios::out | std::ios::binary);
        MD5_Init(&m_md5Context);
        m_currentIndex = -1;
        ret = true;
    } else {
      //  UTILS_LOGE(TAG, "FileSplitReceiver startReceive error %d", data.getLength());
    }

    return ret;
}

bool FileSplitReceiver::continueStartReceive(const CharArray &data)
{
    char path[PATH_MAX_SIZE] = {0};
    snprintf(path, PATH_MAX_SIZE, "%s/clustertemp%d", CommonConstant::FILE_PATH, m_cmd);
    m_stream.open(path, std::ios::out | std::ios::binary | std::ios::app);

    return true;
}

bool FileSplitReceiver::receive(const CharArray &data)
{
    bool ret = false;
    int index = 0;
    memcpy(&index, data.getData() + 1, sizeof(index));
    int dataLen = data.getLength() - 1 - sizeof(index);

    //UTILS_LOGD(TAG, "FileSplitReceiver receive index %d dataLen %d", index, dataLen);

    if ((index - m_currentIndex) != 1) {
      ///  UTILS_LOGE(TAG, "FileSplitReceiver receive index error %d %d", index, m_currentIndex);
    } else {
        m_stream.write(data.getData() + 1 + sizeof(index), dataLen);
        MD5_Update(&m_md5Context, data.getData() + 1 + sizeof(index), dataLen);
        m_currentIndex = index;
        ret = true;
    }

    return ret;
}

bool FileSplitReceiver::endReceive(const CharArray &data)
{
    bool ret = true;
  //  UTILS_LOGD(TAG, "FileSplitReceiver endReceive dataLen %d", m_size);
    const int MD5_LENGTH = 16;
    if (data.getLength() > MD5_LENGTH && m_currentIndex > -1) {
        unsigned char md5Value[MD5_LENGTH]  = {0};
        memcpy(&md5Value, data.getData() + 1, MD5_LENGTH);
        unsigned char md5Ret[MD5_LENGTH]  = {0};
        MD5_Final(md5Ret, &m_md5Context);
     //   CLog::dump((char*)md5Value, MD5_LENGTH);
      //  CLog::dump((char*)md5Ret, MD5_LENGTH);
        ret = (0 == memcmp(md5Value, md5Ret, MD5_LENGTH));
       // UTILS_LOGD(TAG, "endReceive check md5 %s",  ret ? "sucess" : "fail");
    }

    m_stream.close();

    if (!ret) {
        return false;
    }

    char path[PATH_MAX_SIZE] = {0};
    snprintf(path, PATH_MAX_SIZE, "%s/clustertemp%d", CommonConstant::FILE_PATH, m_cmd);
    std::string filepath = std::string(path);
    size_t size = getFileSize(filepath);
    if (size != m_size) {
     //  UTILS_LOGE(TAG, "endReceive check size error %d %d", size, m_size);
        ret = false;
    } else {
        m_commandHandler->receiveData(m_cmd, filepath.c_str(), filepath.size());
    }

    return ret;
}

size_t FileSplitReceiver::getFileSize(const std::string& path)
{

    std::ifstream in(path);
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    in.close();

    return size;
}

//handleSplitData
void DataSplitManager::receiveSplitData(unsigned char cmd, const CharArray &data)
{
    SplitReceiver * receiver = nullptr;
    auto it = m_splitReceivers.find(cmd);
    if (it != m_splitReceivers.end()) {
        receiver = (it->second);
    } else {
        if (isFileData(cmd))
            receiver =  new FileSplitReceiver(m_commandHandler);
        else
            receiver = new DataSplitReceiver(m_commandHandler);
        receiver->setCmd(cmd);
        m_splitReceivers.insert(std::pair<unsigned char, SplitReceiver *>(cmd, receiver));
    }

    bool ret = true;

    switch (data[0]) {
        case CommonConstant::SplitDataStart:
            ret = receiver->startReceive(data);
        break;
        case CommonConstant::SplitData:
            ret = receiver->receive(data);
        break;
        case CommonConstant::SplitDataEnd:
            ret = receiver->endReceive(data);
        break;
        case CommonConstant::SplitDataContinueStart:
            ret = receiver->continueStartReceive(data);
        break;
    }

    std::unique_ptr<ClusterToIVICommand> replyCommand(new ClusterToIVICommand(cmd));
    replyCommand->setType(CommonConstant::Reply);
    CharArray array(1);
    if (data[0] != CommonConstant::SplitDataEnd) {
        array = CharArray(5);
        array.copyData(data.getData(), 0, 5);
    } else {
        array.copyData(data.getData(), 0, 1);
    }

    replyCommand->setReplyResult(ret ? CommonConstant::CmdSucess : CommonConstant::CmdFailed);
    replyCommand->setData(array);
    m_commandHandler->sendCommand(replyCommand);
}

bool DataSplitManager::isFileData(unsigned char cmd)
{
    return FILE_DATA_CMD.find(cmd) != FILE_DATA_CMD.end();
}


