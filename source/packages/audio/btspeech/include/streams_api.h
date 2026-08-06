
#ifndef STREAMS_API_H
#define STREAMS_API_H


#ifdef __cplusplus
extern "C"{
#endif

int bt_up_stream_thread(int rate);
int bt_down_stream_thread(int rate);

int is_bt_down_stream_started(void);
void bt_down_stream_stop(void);

int is_bt_up_stream_started(void);
void bt_up_stream_stop(void);

int start_audio_bt(int rate);

int stop_audio_bt(void);




#ifdef __cplusplus
}
#endif


#endif
