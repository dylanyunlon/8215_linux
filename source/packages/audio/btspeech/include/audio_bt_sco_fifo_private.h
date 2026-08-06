
#ifndef AUDIO_BT_SCO_FIFO_PRIVATE_H
#define AUDIO_BT_SCO_FIFO_PRIVATE_H


#ifdef __cplusplus
extern "C"{
#endif

#define AUDIO_BT_SCO_FIFO_SVR "/tmp/audio_bt_sco_fifo_svr"
#define AUDIO_BT_SCO_MSG_VERSION (0x00100)
typedef struct AudioBtScoMsg{
    int ver;
    int pid;
    int needAck;
    int msgType;
    int msgLen;
    struct timeval timeStamp;

    struct {
        int     start;
        int     rate;
    }data;

}AudioBtScoMsg;


#ifdef __cplusplus
}
#endif


#endif
