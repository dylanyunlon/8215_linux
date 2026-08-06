#ifndef __AWB_H__
#define __AWB_H__

#include <linux/kernel.h>
#include "audiosys.h"
#include "asrc.h"
#include "pcmcomm.h"

typedef struct {
    UINT32 m_u4State;
    WAVE_DATA_BUF_T m_rBuf;

    struct mutex m_AwbLock;
} AwbHw, *PAwbHw;

typedef struct {
    PVOID m_pvRingBuf;
    UINT32 m_u4AsrcIdx;
    ASRC_CHS_FMT_T m_rAsrcFmt;

    UINT32 m_u4AwbRP;
} Awb, *PAwb;

#define MONO   1
#define STEREO 2
#define AWB_SOFT_BUF_SZ               (4800 * 2)


/* interface */
PAwb Awb_Open(void);
int Awb_Close(PAwb pthis);
int Awb_Read(PAwb pthis, PWAVE_DATA_BUF_T prBuf);
u32 Awb_GetDataSize(PAwb pthis);
void Awb_ResetRP(PAwb pthis);
void Awb_Start(PAwb pthis);
void Awb_Stop(PAwb pthis);
void Awb_Transfer(PAwb pthis);

#endif /* __AWB_H__ */
