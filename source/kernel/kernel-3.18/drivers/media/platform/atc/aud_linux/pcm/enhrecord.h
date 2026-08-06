#ifndef PCM_ENHRECORD_H
#define PCM_ENHRECORD_H

#include <linux/kernel.h>
#include "speechenhance.h"
#include "micin.h"
#include "awb.h"
#include "audiosys.h"
#include "pcm_ac83xx.h"

/* Add DelayBuffer */
#define MIC_DELAY_SAMPLE      (156)
#define REF_DELAY_SAMPLE      (0)

#define MICIN_SOFT_BUF_SZ		(4800 * 2)
typedef void *   PVOID;


typedef struct {
    s16 ulBuf1[16 * 20];
    s16 ulBuf2[16 * 20];
    s16 ulRef[16 * 20];
} ENHANCE_FRAME;

typedef struct {
    substream_data m_rStrmData;
    UINT32 m_u4ProcSamples;

    UINT32 frame_byte;
    UINT32 sample_rate;
    PVirtualMicIn prMicIn;
    PAwb      prAwb;

    UINT32 m_u4State;
    struct task_struct *Thread_task;
    wait_queue_head_t   Thread_wq;
    int                 Thread_wq_flag;
    BOOL                isThreadExit;
    wait_queue_head_t   MainThread_exit_wq;
    int                 MainThread_exit_flag;

    struct task_struct *enh_task;
    wait_queue_head_t   enh_thread_wq;
    int                 enh_thread_flag;
    wait_queue_head_t   enh_thread_exit_wq;
    int                 enh_thread_exit_flag;

    wait_queue_head_t   has_data_wq;
    int                 has_data_flag;

    HANDLE_T m_EnhHasData;              // msg queue

    BOOL m_EnhanceEnable;

    UINT32 m_u4AsrcIdx;
    ASRC_CHS_FMT_T m_rAsrcFmt;
    PVOID m_pvRingBuf_Asrc;

    PVOID m_pvRingBuf;
	PVOID m_pvMicRingBuf;

    BOOL m_IsPrepared;    // cover alsa flow issue
// for debug
    BOOL  m_fgULDataFromFile;
    PVOID m_prULFile;
    BOOL  m_fgRefDataFromFile;
    PVOID m_prRefFile;
    PVOID m_EPLFile;

    ENHANCE_FRAME *frame_queue;
    u32 writeIdx;
    u32 readIdx;

// delay
    char MicDelayBuffer[640];
    char MicTempBuffer[640];
    UINT32 MicDelaySize;

    char RefDelayBuffer[640];
    char RefTempBuffer[640];
    UINT32 RefDelaySize;
} EnhRecord, *PEnhRecord;

extern BOOL g_EnhRecordDump;
extern BOOL g_ForceEnableEnhance;
extern int32_t g_EnhanceRecordDelaySample;
extern uint32_t g_NormalRecordWithEnhance;

PEnhRecord EnhRecord_Open(struct snd_pcm_substream *substream);
int EnhRecord_Close(PEnhRecord this);
int EnhRecord_CreateThread(PEnhRecord this);
int EnhRecord_Prepare(PEnhRecord this);
int EnhRecord_Start(PEnhRecord this);
int EnhRecord_Stop(PEnhRecord this);

#endif /* PCM_ENHRECORD_H */
