/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef HDMI_HAL_H_
#define HDMI_HAL_H_
#include <linux/io.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/videodev2.h>

/*#include "hdmi_capture.h"*/
#include "hdmi_adec.h"
#include "avin_common.h"
#include "ac823x/wch_if.h"

typedef enum {
	HDMI_AUD_STATE_IDLE,
	HDMI_AUD_STATE_PLAY,
	HDMI_AUD_STATE_STOP
} HDMI_AUD_E_STATE_T;

typedef enum {
	HDMI_AUD_FOR_SIGNALON,
	HDMI_AUD_FOR_SIGNALONOVER,
	HDMI_AUD_FOR_SIGNALOFF,
	HDMI_AUD_FOR_NONE
} HDMI_AUD_SIGNAL_STATE_T;

enum {
	HDMI_SAMPLE_RATE_32K = 32000,
	HDMI_SAMPLE_RATE_44K = 44100,
	HDMI_SAMPLE_RATE_48K = 48000,
	HDMI_SAMPLE_RATE_64K = 64000,
	HDMI_SAMPLE_RATE_88K = 88200,
	HDMI_SAMPLE_RATE_96K = 96000,
	HDMI_SAMPLE_RATE_128K = 128000,
	HDMI_SAMPLE_RATE_176K = 176000,
	HDMI_SAMPLE_RATE_192K = 192000,
};

enum {
	IEC_BIT_DEPTH_16 = 16,
	IEC_BIT_DEPTH_17 = 17,
	IEC_BIT_DEPTH_18 = 18,
	IEC_BIT_DEPTH_19 = 19,
	IEC_BIT_DEPTH_20 = 20,
	IEC_BIT_DEPTH_21 = 21,
	IEC_BIT_DEPTH_22 = 22,
	IEC_BIT_DEPTH_23 = 23,
	IEC_BIT_DEPTH_24 = 24,
};

enum {
	IEC_BIT_DEPTH_INDEX_0 = 0,
	IEC_BIT_DEPTH_INDEX_1 = 1,
	IEC_BIT_DEPTH_INDEX_2 = 2,
	IEC_BIT_DEPTH_INDEX_3 = 3,
	IEC_BIT_DEPTH_INDEX_4 = 4,
	IEC_BIT_DEPTH_INDEX_5 = 5,
};

typedef enum {
	HDMI_AUD_DEST_TYPE_INVALID,
	HDMI_AUD_DEST_TYPE_FRONT,
	HDMI_AUD_DEST_TYPE_REAR,
	HDMI_AUD_DEST_TYPE_FRONT_REAR,
} HDMI_AUD_E_DEST_TYPE_T;

enum {
	E_HDMIAUD_SIGNAL_LOCK = 0,
	E_HDMIAUD_SIGNAL_UNLOCK,
	E_HDMIAUD_SIGNAL_UNKNOW,
};

/*extern WCH_BUFF_INFO_T mWchBufferInfo;*/
extern int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data);
extern WCH_CFG_T mWchCfg;
/*extern int mWidth;
extern int mHeight;*/
extern int device_type;
extern struct mutex audiolock;
extern struct mutex videolock;

//extern int hdmiControl(int CtrlCode);
//extern void wch_buffer_done(u32 buf_index, bool need_hide, HDMI_SIG_STATE_T signal_status);
extern int hdmi_getDeviceType(void);
extern int hdmi_getSignalStatus(void);
extern BOOL hdmi_getOrientation(void);
void _hdmi_getOrientation(u32 buffer_id);

extern void sendRcp(int rcpKey);
extern void getVideoRect(int cmd, int outRect[]);

//extern int getResolution(int freq, bool fgProgressive, int *ptiming);
//extern void wch_buffer_get(u32 *bufindex);
//extern void onVdoNoSignal(void);
//extern void onVdoSignalChange(void);
//extern void onVdoSignal(void);
extern void atc_hdmi_signal_status(void *signal_status);
extern void atc_hdmi_isr(void *arg);
extern int hdmi_init_video(int index);
extern int hdmi_start_video(int index);
extern int hdmi_stop_video(int index);
//extern bool GetSampleRateByIndex(UINT8 uIndex, UINT32 *u4SampleRate);
//extern bool GetSampleBitDepth(bool fgMaxBit, UINT8 uIndex, UINT8 *uBitDepth);
//extern bool GetSpeakerLayOut(UINT8 uSpeakerAlloc, AUD_DEC_AUD_TYPE_T *eAudDecType);
//extern bool openAOut(HDMI_AUD_E_DEST_TYPE_T eDestType);
//extern bool closeAOut(HDMI_AUD_E_DEST_TYPE_T eDestType);
//extern bool startAdec(void);
//extern bool stopAdec(void);
//extern bool HDMIRXParseOn(void);
//extern bool HDMIRXParseOff(void);
/*extern BOOL Audio_stop_state;*/
//extern void onAudSignal(void);
//extern void onAudNoSignal(void);
extern void atc_hdmiaudio_signal_status(void *pdwMsg);
extern void atc_hdmiaudio_isr(void *arg);
//extern int Audio_SetInputBuf_thread(void *data);
extern struct task_struct *audio_setinputbuf_task;
extern int hdmi_init_audio(int index);
extern int hdmi_start_audio(int index);
extern int hdmi_stop_audio(int index);

//extern void wch_buffer_done(u32 buf_index, bool need_hide, HDMI_SIG_STATE_T signal_status);


extern int AudioIoCtl(struct file *Audiofilp, DWORD IoControlCOde, VOID *lpInBuf, DWORD InBufSize,
		      VOID *lpOutBuf, DWORD OutBufSize , DWORD *lpBytesReturned);
extern void ADec_Release(HANDLE hInst);
extern BOOL ADec_SetParam(HANDLE hInst, ParamType eAudioParamType, VOID *prParam, bool flag);
extern BOOL ADec_SetInputBuf(HANDLE hInst, VOID *pvBuf, UINT32 u4BufSz, VOID *pvOutBuf, UINT32 u4OutBufSz);
extern BOOL  ADec_Start(HANDLE hInst, HANDLE hEvent);
extern void ADec_Stop(HANDLE hInst);
extern BOOL ADec_SetSpeed(HANDLE hIns, UINT32 u4Speed);
extern BOOL ADec_CloseAOut(HANDLE hInst, AUD_OUTPUT_T AOutPut);
extern BOOL ADec_OpenAOut(HANDLE hInst, AUD_OUTPUT_T AOutPut);
extern AUDIO_DECODER *ADec_CreateInstance(AVCODECID_T codec_type, UINT32 u4Flag);
typedef void (*atc_hdmiaudio_isr_t)(void *arg);
extern int atc_hdmiaudio_register_isr(atc_hdmiaudio_isr_t isr, void *arg);
extern int atc_hdmiaudio_unregister_isr(atc_hdmiaudio_isr_t isr, void *arg);
typedef struct _atc_hdmiaudio_isr_data {
	atc_hdmiaudio_isr_t	isr;
	void			*arg;
} atc_hdmiaudio_isr_data;

#endif /*HDMI_HAL_H_*/
