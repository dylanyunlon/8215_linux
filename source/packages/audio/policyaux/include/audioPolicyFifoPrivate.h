
#ifndef AUDIO_POLICY_FIFO_MSG_HEAD
#define AUDIO_POLICY_FIFO_MSG_HEAD


#ifdef __cplusplus
extern "C"{
#endif

typedef struct StreamState{
    int handle;
    int state;
}StreamState;

typedef struct StreamVol{
    int streamType;
    int volume;
}StreamVol;


#define AUDIO_POLICY_FIFO_SVR "/tmp/audio_policy_fifo_svr"
#define AUDIO_POLICY_MSG_VERSION (0x00100)
typedef struct AudioPolicyMsg{
    int ver;
    int pid;
    int needAck;
    int msgType;
    int msgLen;
    struct timeval timeStamp;

    union{
        StreamState streamState;
        StreamVol   streamVol;
    }data;

}AudioPolicyMsg;

#define AP_MSG_TYPE_STREAM_EVENT 0
#define AP_MSG_TYPE_SET_VOLUME 1

#define APOLICY_TYPE_VOLUME_MAX 0x20000



#ifdef __cplusplus
}
#endif


#endif
