#ifndef COMMON_H
#define COMMON_H

class CommonConstant
{
public:
    static const unsigned short MIN_RECEIVER_PACKET_LEN = 5; //包最小接收长度
    static constexpr const char *FILE_PATH = "/data/cluster";

    enum CommandType {
        Request = 0x01,
        Reply = 0x02,
        SplitDataRequest = 0x03,
    };

    enum SplitCmdType {
        SplitDataStart = 0x01, //大数据开始
        SplitData = 0x02,      //传送
        SplitDataEnd = 0x03,   //数据结束
        SplitDataContinueStart = 0x04, //断点重传
    };

    enum CommandId {
        //music indication cmd 0x00 ~ 0x1F
        MusicStatus = 0x00,    //播放状态
        MusicName = 0x01,      //播放音乐歌曲名

        //call indication cmd 0x10 ~ 0x1F
        CallStatus = 0x10,     //电话状态
        CallPersonName = 0x11, //联系人姓名
        CallNumber = 0x12,     //电话号码
        CallTime = 0x13,       //通话时间
        CallPicture = 0x14,    //联系人图片

        //download update package cmd 0x20 ~ 0x2F
        DownloadUpdatePackage = 0x20, //下载cluster升级包
        DownloadUpdatePackageCheck = 0x21, //下载升级包前校验状态、空间

        //control cmd 0xE0 ~ 0xFE
        MediaPlayPause = 0xE0,
        MediaPre = 0xE1,
        MediaNext = 0xE2,
        MediaStop = 0xE3,
        Call = 0xE4,
        HandUp = 0xE5,

        HeartbeatCmd = 0xFF,      //心跳指令
     };

    enum CommandResult {
        CmdSucess = 0x00,     //解析成功
        CmdFailed = 0x01,     //执行失败
        CmdCheckError = 0x02, //校验失败
        CmdBusy= 0x03,        //其他错误

        CmdTimeOut = 0xFE,    //超时
        CmdUnKnown = 0xFF,    //未知结果
    };

};

#endif // COMMON_H
