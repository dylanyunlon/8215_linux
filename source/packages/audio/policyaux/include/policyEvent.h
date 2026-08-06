/*
* Copyright (c) 2016 AutoChips Inc.
*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/



#ifndef _POLICYAUX_H_
#define _POLICYAUX_H_

#include <linux/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//#include "streamTypePublic.h"

typedef void * STREAM_HANDLE;

#define MM_HANDLE       (STREAM_HANDLE)1
#define BTHFP_HANDLE    (STREAM_HANDLE)2
#define AVIN_HANDLE     (STREAM_HANDLE)3
#define RDS_HANDLE      (STREAM_HANDLE)4
#define BACKCAR_HANDLE  (STREAM_HANDLE)5


//#define VolIndexMax     40
#define VOLUME_INDEX_MAX     ((int)40)
#define VOLUME_INDEX_MIN     ((int)0)

#define MM_GAIN_TYPE        (0)
#define COMMON_API_CLIENT_LIB_PATH "/usr/lib/libCommandControlGenericCommonAPI.so"
#define AM_TMP_FLAG_FILE  "/tmp/useAM"
#define TMP_DIR "/tmp"
#define AUDIO_POLICY_SERVER_READY_FILE "/tmp/audioPolicyReady"


#define AUDIO_POLICY_SHM_MEM_NAME "/audio_policy_shm"
#define AUDIO_POLICY_SHM_MEM_LEN ((int)100)



#define AM_DRIVER_SINK_NAME  "Driver"


typedef enum{
    START,
    STOP
}AUDIO_STATE;



static const char *APP_NAME[] =
{
    "none",
    "mm_atc",
    "bthfp_atc",
    "avin_atc",
    "rds_atc",
    "backcar_atc"
};


/*******************************************************
*function:      send_policy_event
*author:        Vincent.Liu
*created time:  2015-11-20
*arg[0]:        hStream:    XXX_HANDLE
*arg[0]:        eAudState:   START / STOP
*return:        int:  0 as success, others fail
*note:          tell audiopolicy there is module using decoder
*               if server is not ready, may blocked 6 seconds max
*
*********************************************************/
int send_policy_event(STREAM_HANDLE hStream, AUDIO_STATE eAudState);

int sendPolicyEvent(STREAM_HANDLE hStream, AUDIO_STATE eAudState);

/*******************************************************
*function:      send_policy_event_no_block
*author:        Vincent.Liu
*created time:  2015-11-20
*arg[0]:        hStream:    XXX_HANDLE
*arg[0]:        eAudState:   START / STOP
*return:        int:  0 as success, others fail
*note:          tell audiopolicy there is module using decoder
*               return right now
*
*********************************************************/
int send_policy_event_no_block(STREAM_HANDLE hStream, AUDIO_STATE eAudState);

int sendPolicyEventNoBlock(STREAM_HANDLE hStream, AUDIO_STATE eAudState);



/*******************************************************
*function:      send_stream_type_vol
*author:        Vincent.Liu
*created time:  2015-11-20
*arg[0]:        streamType:
*arg[0]:        volume:   0 ~ 0x20000
*return:        int:  0 as success, others fail
*note:          set stream type volume
*               if server is not ready, may blocked 6 seconds max
*
*********************************************************/
int send_stream_type_vol(int streamType, int volume);

int sendStreamTypeVol(int streamType, int volume);

/*******************************************************
*function:      send_stream_type_vol_no_block
*author:        Vincent.Liu
*created time:  2015-11-20
*arg[0]:        streamType:
*arg[0]:        volume:   0 ~ 0x20000
*return:        int:  0 as success, others fail
*note:          set stream type volume
*               return right now
*********************************************************/
int send_stream_type_vol_no_block(int streamType, int volume);

int sendStreamTypeVolNoBlock(int streamType, int volume);


int receiveStreamTypeVol(int streamType);


int policy_event_get_streams();

int btsco_start(int samplerate);

int btsco_stop(void);

#ifdef __cplusplus
}
#endif

#endif

