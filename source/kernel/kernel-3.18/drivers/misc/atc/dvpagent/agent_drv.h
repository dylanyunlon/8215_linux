#ifndef _AGENT_DRV_H_
#define _AGENT_DRV_H_


#include <windows.h>
#include "chip_ver.h"
#include "dvp_protocol.h"
#include "x_typedef.h"
#include "wch_drv.h"

#include <linux/types.h>
#include <linux/printk.h>

#define STREAM_DEVICE_DVPHOST                  0x1000

#define DEF_FUNCTION_CODE(FuncNum)            \
    CTL_CODE(STREAM_DEVICE_DVPHOST, FuncNum, \
        METHOD_BUFFERED, FILE_ANY_ACCESS)

#define DVP_IOCTL_SENDCMD                   DEF_FUNCTION_CODE(0x101)
#define DVP_IOCTL_GETEVENT                  DEF_FUNCTION_CODE(0x102)
#define DVP_IOCTL_READ                      DEF_FUNCTION_CODE(0x104)

#define DVPAGENT_IOCTL_SETPEER                 DEF_FUNCTION_CODE(0x101)
#define DVPAGENT_IOCTL_READ                    DEF_FUNCTION_CODE(0x102)
#define DVPAGENT_IOCTL_WRITE                   DEF_FUNCTION_CODE(0x103)
#define DVPAGENT_IOCTL_RAWWRITE                DEF_FUNCTION_CODE(0x104)
#define DVPAGENT_IOCTL_QUERYID                 DEF_FUNCTION_CODE(0x105)
#define DVPAGENT_IOCTL_SENDCMD                 DEF_FUNCTION_CODE(0x106)
#define DVPAGENT_IOCTL_GETEVENT                DEF_FUNCTION_CODE(0x107)
#define DVPAGENT_IOCTL_SETRIPPATH              DEF_FUNCTION_CODE(0x108)
#define DVPAGENT_IOCTL_WRITEPT110              DEF_FUNCTION_CODE(0x109)
#define DVPAGENT_IOCTL_DVP2APMIX               DEF_FUNCTION_CODE(0x10a)
#define DVPAGENT_IOCTL_COPYLYRICSBUF           DEF_FUNCTION_CODE(0x10b)
#define DVPAGENT_IOCTL_COPYID3PICBUF           DEF_FUNCTION_CODE(0x10c)
#define DVPAGENT_IOCTL_COPYADOEQBUF            DEF_FUNCTION_CODE(0x10d)
#define DVPAGENT_IOCTL_COPYRIPDATABUF          DEF_FUNCTION_CODE(0x10e)
#define DVPAGENT_IOCTL_SETDVPLOG               DEF_FUNCTION_CODE(0x10f)
#define DVPAGENT_IOCTL_WRITEDATA2DRAM          DEF_FUNCTION_CODE(0x110)
#define DVPAGENT_IOCTL_READDATAFROMDRAM        DEF_FUNCTION_CODE(0x111)
#define DVPAGENT_IOCTL_READFLINFOFROMDRAM      DEF_FUNCTION_CODE(0x112)
#define DVPAGENT_IOCTL_SETDVPCODEPAGE          DEF_FUNCTION_CODE(0x113)
#define DVPAGENT_IOCTL_CREATEMSGQUEUE          DEF_FUNCTION_CODE(0x115)
#define DVPAGENT_IOCTL_GETDRMADDR              DEF_FUNCTION_CODE(0x116)

#define DVPAGENT_IOCTL_AVSWITCH                DEF_FUNCTION_CODE(0x118)
#define DVPAGENT_IOCTL_SET_SPRCTRUM            DEF_FUNCTION_CODE(0x119)
#define DVPAGENT_IOCTL_GET_SPRCTRUM            DEF_FUNCTION_CODE(0x120)
#define DVPAGENT_IOCTL_VIDEOINFO               DEF_FUNCTION_CODE(0x121)
#define DVPAGENT_IOCTL_GETINDEX                DEF_FUNCTION_CODE(0x122)


struct DVPAGENT_INST_T {
    u32        dwID;
    u16        szMsgQName[15];
    HANDLE_T     hMsgQ;
};

#define MAX_DVPAGENT_INS_CNT    8
extern struct DVPAGENT_INST_T  *_arDVPAgentInsTable[MAX_DVPAGENT_INS_CNT];
bool DVP_Init(void);
bool DVP_Deinit(void);
u32  DVP_Open(u32 hDeviceContext, u32 AccessCode, u32 ShareMode);
bool DVP_Close(u32 hOpenContext);
bool DVP_IOControl(u32 hOpenContext, u32 dwCode, u8 *pBufIn, u32 dwLenIn,
            s8 *pBufOut, u32 dwLenOut, u32 *pdwActualOut);
bool DVP_IOGetEvent(char *pBufOut, u32 dwLenOut, u32 *pdwActualOut,
    struct DVPAGENT_INST_T *pIns);
u32 DVPHost_avswitch(u8 *puData);

extern int dvp_open_audio(bool IsFront);
extern int dvp_close_audio(bool IsFront);
extern int dvp_open_video(WCH_BUFF_INFO_T *bufferAddr, u32 length);
extern int dvp_getAudioSpectrum(void *spectrum, u32 length);
extern int dvp_close_video(void);
extern int dvp_getWcDVpIndex(void *index, u32 length);
extern int dvp_informAudioSampleRate(u8 *samplerate);

#endif

