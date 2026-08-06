
#ifndef FIFO_CLIENT_H
#define FIFO_CLIENT_H


#ifdef __cplusplus
extern "C"{
#endif

int sendPolicyEventByFifo(int handle, int state);


#define LOG_TAG "POLICY_API"

static struct timeval gTime;
static struct timezone gTz;

#define POLICY_INFO_LOG(...)  do{ \
            asyslogi(LOG_TAG, __VA_ARGS__); \
        }while(0)

#define POLICY_ERR_LOG(...)  do{ \
			asysloge(LOG_TAG, __VA_ARGS__); \
		}while(0)

#define POLICY_DEBUG_LOG(...)  do{ \
			asyslogd(LOG_TAG, __VA_ARGS__); \
		}while(0)


#define POLICY_EVEN_LOG(...)  do{ \
            gettimeofday(&gTime, &gTz); \
            asyslogi(LOG_TAG, "Send Time:sec :%ld, usec :%ld.", gTime.tv_sec, gTime.tv_usec); \
            asyslogi(LOG_TAG, __VA_ARGS__); \
        }while(0)


#ifdef __cplusplus
}
#endif


#endif
