#include "enhrecord.h"
#include "awb.h"
#include "micin.h"
#include <linux/kthread.h>
#include "winutil.h"
#include "aud_pcm_dbg.h"
#include "pcm_debug.h"
#include "pcmcomm.h"
#include "bt_lib.h"
#include "drv_thread.h"
#include "aud_oal.h"

#define ENABLE_DUMP_EPL          1
#define ENH_QUEUE_SIZE           10
#define LOG_TAG "enhrecord"
enum {
    EVENT_FLAG_NONE  = 0,
    EVENT_FLAG_START = 1,
    EVENT_FLAG_STOP  = 2,
    EVENT_FLAG_EXIT  = 3
};

enum {
    ENHREC_SUCCESS = 0,
    ENHREC_FAILED  = 1
};

extern struct semaphore g_rPCMSema;

struct semaphore g_rPCMEnhSema;
SPH_ENH_ctrl_struct g_EnhRecordCtrl;
void *g_EnhMemPtr;
BOOL g_EnhRecordDump = false;
BOOL g_ForceEnableEnhance = true;//false;
u32 g_DumpCount = 0;
char g_MicFileName[30];
char g_RefFileName[30];
char g_OutFileName[30];

int32_t g_EnhanceRecordDelaySample = 0;
uint32_t g_NormalRecordWithEnhance = 1;

//s16 DMNR_data[DMNR_PARAM_NUM];//cgx
//u16 Compen_filter[COMPEN_FILTER_16K];//cgx
static Word16 DMNR_data[DMNR_PARAM_NUM] = //DMNR_cal_data_16k
{
    2,       2,       1,       2,       1,       2,       3,       3,       4,       5,
    5,       4,       3,       3,       2,       2,    8440,    8225,   13206,    8705,
    16069,   14304,   11267,   11796,    9873,    8534,    8843,    9280,   14580,   10279,
    13006,    8520,       1,       7,       0,       7,      11,      16,      20,      21,
    -36,      66,      35,      35,      35,     -56,     -48,      52,     -65,      24,
    4044,    6200,    3166,    7043,   14082,   10028,    8944,    9438,    9223,    8563,
    9838,    7093,    6497,   12792,   14281,   24934,   28072,   29389,       2,       0,
    2,   21930,      68,       0,       0,       0
};


static Word16 Compen_filter[COMPEN_FILTER_16K] =
{
    32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    32767,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//extern uWord32 Sph_Enh_ctrl_16k[AEC_NDC_PARAM_NUM];//cgx
//static uWord32 Sph_Enh_ctrl_16k[AEC_NDC_PARAM_NUM] = {0};

static uWord32 Sph_Enh_ctrl_16k[AEC_NDC_PARAM_NUM] =
{
    32960,  // AEC NLP                 //atc6048 //32960,
    224 ,   // AEC control word
    2218,   // AEC Echo suppression
    30  ,   // NDC UL control word
    57603,  // NDC NR                  //atc6048 //54279,  
    30   ,  // NDC DL control word
    403  ,  // NDC calibration
    8,     // Digital Gain            //cgx //100 ,
    208,   // NDC NR                  //atc6048  //84, //464, 
    4325 ,  // NDC NR aggressive mode
    4193,   // NDC RINI
    0   ,
    2826,  // AEC AES                 //atc6048 //2064, 
    0 ,     // ABF control (0 - ABF off)
    0 ,     // ABF Post filtering (0 - ABF off)
    0 ,
    0 ,
    0 ,
    0 ,
    32767,  // Clipping
    32769,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


void MIC_Transfer(EnhRecord *this)//atc6048
{
    if (this) {
		//MIC_CopyFromAsrcOut
		WAVE_DATA_BUF_T rAsrcOut;

		memset(&rAsrcOut, 0, sizeof(rAsrcOut));

		if(NOERR == this->prMicIn->GetBuffer(this->prMicIn, &rAsrcOut)) {
			RingBuf_Write(this->m_pvMicRingBuf, &rAsrcOut, 0);
			if (NOERR != Asrc_SetORP(this->prMicIn->u4AsrcIdx, rAsrcOut.u4DataOff)) {
				PCM_ERROR(LOG_TAG, "CopyFromAsrcOut: Set ASRC output read pointer error\r\n");
			}
		} else {
			PCM_ERROR(LOG_TAG, "Failed to GetBuffer Virtual Mic In.\r\n");
		}
    }
}

int MIC_Read(EnhRecord *this, PWAVE_DATA_BUF_T prBuf)
{
    if (this) {
        RingBuf_Read(this->m_pvMicRingBuf, prBuf, 0);
    }

    return 0;
}

u32 MIC_GetDataSize(EnhRecord *this)
{
    if (this) {
        return RingBuf_GetDataLen(this->m_pvMicRingBuf);
    }

    return 0;
}


/********************** Frame Queue **************************/

static int EnhRecord_InitFrameQueue(EnhRecord *this)
{
    int ret = ENHREC_SUCCESS;

    this->frame_queue = (ENHANCE_FRAME *)pcm_malloc(sizeof(ENHANCE_FRAME) * ENH_QUEUE_SIZE);
    if (NULL == this->frame_queue) {
		PCM_ERROR(LOG_TAG, "Init: Alloc Ring Buffer err.\r\n");
    }

    this->writeIdx = 0;
    this->readIdx = 0;

    return ret;
}

static ENHANCE_FRAME *EnhRecord_GetFreeFrame(EnhRecord *this)
{
    ENHANCE_FRAME *frame = NULL;
    if (this) {
        if (this->readIdx == ((this->writeIdx + 1) % ENH_QUEUE_SIZE)) {
			PCM_ERROR(LOG_TAG, "GetFreeFrame: No Free Frame, writeIdx = %u, readIdx = %u.\r\n", this->writeIdx, this->readIdx);
            return NULL;
        } else {
            frame = &this->frame_queue[this->writeIdx];
            this->writeIdx = (this->writeIdx + 1) % ENH_QUEUE_SIZE;
            return frame;
        }
    }

    return NULL;
}

static int EnhRecord_ReadOneFrame(EnhRecord *this, ENHANCE_FRAME **frame)
{
    if (this) {
        if (this->writeIdx == this->readIdx) {
			PCM_ERROR(LOG_TAG, "ReadOneFrame: Read Error, writeIdx = %u, readIdx = %u.\r\n", this->writeIdx, this->readIdx);
            return ENHREC_FAILED;
        } else {
            *frame = &this->frame_queue[this->readIdx];
            this->readIdx = (this->readIdx + 1) % ENH_QUEUE_SIZE;

            return ENHREC_SUCCESS;
        }
    }

    return ENHREC_FAILED;
}

static void EnhRecord_ReadDelayData(PEnhRecord this, ENHANCE_FRAME *frame)
{
    if (this->MicDelaySize > 0) {
        memcpy(this->MicDelayBuffer, this->MicTempBuffer, this->MicDelaySize);
        memcpy((char *)(this->MicDelayBuffer) + this->MicDelaySize, frame->ulBuf1, this->frame_byte - this->MicDelaySize);
        memcpy(this->MicTempBuffer, (void *)((char * )(frame->ulBuf1) + this->frame_byte - this->MicDelaySize), this->MicDelaySize);
        memcpy(frame->ulBuf1, this->MicDelayBuffer, this->frame_byte);
    }

    if (this->RefDelaySize > 0) {
        memcpy(this->RefDelayBuffer, this->RefTempBuffer, this->RefDelaySize);
        memcpy((char *)(this->RefDelayBuffer) + this->RefDelaySize, frame->ulRef, this->frame_byte - this->RefDelaySize);
        memcpy(this->RefTempBuffer, (void *)((char * )(frame->ulRef) + this->frame_byte - this->RefDelaySize), this->RefDelaySize);
        memcpy(frame->ulRef, this->RefDelayBuffer, this->frame_byte);
    }

}

static void EnhRecord_ReadFrameData(PEnhRecord this, ENHANCE_FRAME *frame)
{
    WAVE_DATA_BUF_T rBuf;

    if (this && frame) {
        rBuf.u4Buf1 = (UINT32)frame->ulBuf1;
        rBuf.u4Buf2 = (UINT32)frame->ulBuf2;
        rBuf.u4ChBufSz = this->frame_byte;
        rBuf.u4Chn = 1;
        rBuf.u4DataOff = 0;
        rBuf.u4DataSz = rBuf.u4ChBufSz;

        MIC_Read(this, &rBuf);

        if (this->m_fgULDataFromFile) {
            SphDataFile_ReadData(this->m_prULFile, rBuf.u4Buf1, rBuf.u4Buf2, rBuf.u4ChBufSz);
        }

        rBuf.u4Buf1 = (UINT32)frame->ulRef;
        rBuf.u4Buf2 = rBuf.u4Buf1;

        rBuf.u4Chn = 1;
        Awb_Read(this->prAwb, &rBuf);

        EnhRecord_ReadDelayData(this, frame);

        if (this->m_fgRefDataFromFile) {
            SphDataFile_ReadData(this->m_prRefFile, rBuf.u4Buf1, rBuf.u4Buf2, rBuf.u4ChBufSz);
        }
    }
}

/**** Enhance Lib ****/

static void EnhRecordCtrlInit(void)
{
    memset(&g_EnhRecordCtrl, 0, sizeof(g_EnhRecordCtrl));
    g_EnhRecordCtrl.App_table = WB_VOIP;
    g_EnhRecordCtrl.Fea_Cfg_table = 511;
    g_EnhRecordCtrl.MIC_DG = 16;//0;//cgx
    g_EnhRecordCtrl.sample_rate = 16000;
    g_EnhRecordCtrl.frame_rate = 20;
    g_EnhRecordCtrl.MMI_ctrl = 0xFFFFFFBF;
    g_EnhRecordCtrl.RCV_DG = 0;

    memcpy(g_EnhRecordCtrl.DMNR_cal_data, DMNR_data, sizeof(g_EnhRecordCtrl.DMNR_cal_data));
    memcpy(g_EnhRecordCtrl.Compen_filter, Compen_filter, sizeof(g_EnhRecordCtrl.Compen_filter));

    memcpy(g_EnhRecordCtrl.enhance_pars, Sph_Enh_ctrl_16k,
                    sizeof(g_EnhRecordCtrl.enhance_pars));

    g_EnhRecordCtrl.Device_mode = 2;
    g_EnhRecordCtrl.MMI_MIC_GAIN = 184;//0;//cgx
    g_EnhRecordCtrl.Near_end_vad = 0;

    u32 iMemSz = ENH_API_Get_Memory(&g_EnhRecordCtrl);
    g_EnhMemPtr = (int *)pcm_malloc(iMemSz);
    if (NULL == g_EnhMemPtr) {
		PCM_ERROR(LOG_TAG, "g_EnhMemPtr alloc failed. \r\n");
    }
    ENH_API_Alloc(&g_EnhRecordCtrl, (Word32 *)g_EnhMemPtr);
    ENH_API_Rst(&g_EnhRecordCtrl);
}

static int EnhRecordDelayBufferInit(PEnhRecord this)
{
    if (this) {
        memset(this->MicDelayBuffer, 0, sizeof(this->MicDelayBuffer));
        memset(this->MicTempBuffer, 0, sizeof(this->MicTempBuffer));

        memset(this->RefDelayBuffer, 0, sizeof(this->RefDelayBuffer));
        memset(this->RefTempBuffer, 0, sizeof(this->RefTempBuffer));

        if (g_EnhanceRecordDelaySample > 0) {
            this->RefDelaySize = g_EnhanceRecordDelaySample * sizeof(s16);
        } else {
            this->MicDelaySize = (-g_EnhanceRecordDelaySample) * sizeof(s16);
        }
    }

    return ENHREC_SUCCESS;
}

/**** Dump Function ****/
static int write_file(const char *filename, const char *wbuf, size_t length)
{
    int re = 0;
    struct file *fp;
    mm_segment_t fs;
    fs = get_fs();
    set_fs(KERNEL_DS);

    fp = filp_open(filename, (O_RDWR | O_CREAT | O_APPEND), S_IRUSR);
    if (IS_ERR(fp) || !fp->f_op) {
		PCM_ERROR(LOG_TAG, "file name is %s, filp_open error \r\n", filename);
        re = -ENOENT;
    }

    if (wbuf) {
        if ((re = fp->f_op->write(fp, wbuf, length, &fp->f_pos)) < 0) {
			PCM_ERROR(LOG_TAG, "Write %u bytes to file %s error %d \r\n", length, filename, re);
        }
    }

    if (!IS_ERR(fp)) {
        filp_close(fp, NULL);
    }
    set_fs(fs);

    return re;
}

static void dump_out(s16 *data, u32 length)
{
    if (data && g_EnhRecordDump) {
        write_file(g_OutFileName, data, length);
    }
}

static void dump_mic(s16 *data, u32 length)
{
    if (data && g_EnhRecordDump) {
        write_file(g_MicFileName, data, length);
    }
}

static void dump_ref(s16 *data, u32 length)
{
    if (data && g_EnhRecordDump) {
        write_file(g_RefFileName, data, length);
    }
}

static void EnhRecordDumpInit(PEnhRecord this)
{
    if (g_EnhRecordDump) {
        sprintf(g_MicFileName, "/data/ER_mic_%u.pcm", g_DumpCount);
        sprintf(g_RefFileName, "/data/ER_ref_%u.pcm", g_DumpCount);
        sprintf(g_OutFileName, "/data/ER_out_%u.pcm", g_DumpCount);
        g_DumpCount++;
    }
}

/**** EnhRecord Process flow ****/

static void EnhRecord_Process(PEnhRecord this, ENHANCE_FRAME *frame)
{
    if (this && frame) {
        // move data to ENH Structure memory
        memcpy(&g_EnhRecordCtrl.PCM_buffer[0], frame->ulBuf1, this->frame_byte);   // UL_MIC1
        dump_mic(frame->ulBuf1, this->frame_byte);
        memcpy(&g_EnhRecordCtrl.PCM_buffer[320], frame->ulBuf2, this->frame_byte); // UL_MIC2
        memcpy(&g_EnhRecordCtrl.PCM_buffer[960], frame->ulRef, this->frame_byte);  // REF
        dump_ref(frame->ulRef, this->frame_byte);
        if (this->m_EnhanceEnable) {
            ENH_API_Process(&g_EnhRecordCtrl);
            dump_out(&g_EnhRecordCtrl.EPL_buffer[1600], this->frame_byte);
            memcpy(frame->ulBuf1, &g_EnhRecordCtrl.EPL_buffer[1600], this->frame_byte);
        }
    }
}

static void EnhRecord_CopyToUser(PEnhRecord this)
{
    UINT32 u4size = 0, hw_pos = 0, hw_bytes = 0;
    PBYTE m_lpCurrData = 0;
    UINT32 u4DataSz = 0, u4RP = 0;
    WAVE_DATA_BUF_T rBuf;
    PWAVE_DATA_BUF_T prBuf = &rBuf;

    substream_data *prStrmData = &this->m_rStrmData;
    struct snd_pcm_substream *substream = prStrmData->substream;
    struct snd_pcm_runtime *runtime = substream->runtime;

    INT16 i2DataL, i2DataR;

    RingBuf_GetRBuf(this->m_pvRingBuf, prBuf);

    u4DataSz = prBuf->u4DataSz & 0xFFFFFF80;
    if (0 == u4DataSz) {
        PCM_ERROR(LOG_TAG, "CopyToUser: data size must 128 bytes alignment \r\n");//cgx todo
        goto EXIT;
    }

    u4RP = prBuf->u4DataOff;
    hw_pos = frames_to_bytes(runtime, (prStrmData->last_ptr - prStrmData->hw_Base));
    m_lpCurrData = (PBYTE)(prStrmData->dma_start + hw_pos);
    if (!m_lpCurrData || (prStrmData->m_eState != SNDRV_PCM_TRIGGER_START)) {
        goto EXIT;
    }

    while (u4DataSz && m_lpCurrData) {
        if (prStrmData->m_eState != SNDRV_PCM_TRIGGER_START) {
            goto EXIT;
        }

        i2DataL = *(INT16 *)(prBuf->u4Buf1 + u4RP);
        i2DataR = *(INT16 *)(prBuf->u4Buf2 + u4RP);

        switch (prStrmData->m_SampleType) {
        case PCM_TYPE_M8:
            *m_lpCurrData++ = (BYTE)(i2DataL >> 8);
            u4size = 1;
            break;

        case PCM_TYPE_S8:
            *m_lpCurrData++ = (BYTE)(i2DataL >> 8);
            *m_lpCurrData++ = (BYTE)(i2DataR >> 8);
            u4size = 2;
            break;

        case PCM_TYPE_M16:
            *(INT16 *)m_lpCurrData = i2DataL;
            m_lpCurrData += 2;
            u4size = 2;
            break;

        case PCM_TYPE_S16:
            *(INT16 *)m_lpCurrData = i2DataL;
            m_lpCurrData += 2;
            *(INT16 *)m_lpCurrData = i2DataR;
            m_lpCurrData += 2;
            u4size = 4;
            break;

        default:
            goto EXIT;
        }

        u4DataSz -= 2;
        u4RP += 2;
        if (u4RP >= prBuf->u4ChBufSz) {
            u4RP = 0;
        }

        hw_bytes += u4size;
        hw_pos += u4size;
        if (hw_pos >= prStrmData->buffer_size) {
            m_lpCurrData = (PBYTE)prStrmData->dma_start;
            hw_pos = 0;
        }

        prStrmData->Used_size += u4size;
        if (prStrmData->Used_size >= prStrmData->period_size) {
            prStrmData->Used_size %= prStrmData->period_size;
            step_cap_real_hwptr(runtime, hw_bytes);//atc6048
            hw_bytes = 0;
            snd_pcm_period_elapsed(substream);
			//PCM_ERROR(LOG_TAG, "snd_pcm_period_elapsed \r\n");//atc6048
        }

        if (prStrmData->m_eState != SNDRV_PCM_TRIGGER_START) {
            goto EXIT;
        }
    }

    if (prStrmData->m_eState == SNDRV_PCM_TRIGGER_START) {
        step_cap_real_hwptr(runtime, hw_bytes);//atc6048
        //PCM_ERROR(LOG_TAG, "step_cap_real_hwptr \r\n");//atc6048
    }
    RingBuf_SetRP(this->m_pvRingBuf, u4RP);
	//PCM_ERROR(LOG_TAG, "copy to user success\r\n");//atc6048

EXIT:

	//PCM_ERROR(LOG_TAG, "copy to user fail\r\n");//atc6048
    return;
}

static void EnhRecord_CopyToAsrcIn(PEnhRecord this)
{
    WAVE_DATA_BUF_T rAsrcIn;

    memset(&rAsrcIn, 0, sizeof(WAVE_DATA_BUF_T));

    if (NOERR == Asrc_GetIBuf(this->m_u4AsrcIdx, &rAsrcIn)) {
        RingBuf_Read(this->m_pvRingBuf_Asrc, &rAsrcIn, 0xFFFFFFF0);
        Asrc_SetIWP(this->m_u4AsrcIdx, rAsrcIn.u4DataOff);
    } else {
		PCM_ERROR(LOG_TAG, "CopyToAsrcIn: Get ASRC input buffer error \r\n");

    }
}

static void EnhRecord_CopyFromAsrcOut(PEnhRecord this)
{
    WAVE_DATA_BUF_T rAsrcOut;

    memset(&rAsrcOut, 0, sizeof(rAsrcOut));
    if (NOERR == Asrc_GetOBuf(this->m_u4AsrcIdx, &rAsrcOut)) {
        RingBuf_Write(this->m_pvRingBuf, &rAsrcOut, 0);
        if (NOERR != Asrc_SetORP(this->m_u4AsrcIdx, rAsrcOut.u4DataOff)) {
			PCM_ERROR(LOG_TAG, "CopyFromAsrcOut: Set ASRC output read pointer error \r\n");
        }
    } else {
		PCM_ERROR(LOG_TAG, "CopyFromAsrcOut: Get ASRC output buffer error \r\n");
    }
}

static int EnhRecord_Thread(void *data)
{
    PEnhRecord this = (PEnhRecord)data;

    u32 mic_aval_bytes = 0;
    u32 awb_aval_bytes = 0;
    u32 empty_count = 0;
    ENHANCE_FRAME *enhFrame = NULL;
    u32 cmd = 0;

    struct snd_pcm_substream *substream = this->m_rStrmData.substream;
    struct snd_pcm_runtime *runtime = substream->runtime;

    if (this) {
        while (!this->isThreadExit) {
			PCM_DEBUG(LOG_TAG, "Main Thread Wait Command: \r\n");
            wait_event(this->Thread_wq, ((this->Thread_wq_flag == EVENT_FLAG_START) || (this->Thread_wq_flag == EVENT_FLAG_STOP)));
			PCM_DEBUG(LOG_TAG, "Main Thread wq_flag = %d! \r\n", this->Thread_wq_flag);
            if (this->Thread_wq_flag == EVENT_FLAG_STOP) {
				PCM_DEBUG(LOG_TAG, "Main Thread stop!\r\n");
                break;
            }
            this->Thread_wq_flag = EVENT_FLAG_NONE;
			PCM_DEBUG(LOG_TAG, "Main Thread start!\r\n");

            //EnhMicIn_ResetRP(this->prMicIn);
			if(NOERR != this->prMicIn->ResetRP(this->prMicIn)) {//atc6048
				PCM_ERROR(LOG_TAG, "Failed to ResetRP Virtual Mic In.\r\n");
			}

            Awb_ResetRP(this->prAwb);
            Asrc_Start(this->m_u4AsrcIdx);

            while (true) {
                usleep_range(4500, 5500);        // use usleep_range instead of msleep, sleep for 4500us ~ 5500us
                if (this->m_rStrmData.m_eState != SNDRV_PCM_TRIGGER_START) {
                    break;
                }
                MIC_Transfer(this);
                Awb_Transfer(this->prAwb);
                mic_aval_bytes = MIC_GetDataSize(this);
                awb_aval_bytes = Awb_GetDataSize(this->prAwb);
                if (mic_aval_bytes >= (this->frame_byte) &&
                    awb_aval_bytes >= (this->frame_byte)) {
                    enhFrame = EnhRecord_GetFreeFrame(this);
                    if (enhFrame) {
                        EnhRecord_ReadFrameData(this, enhFrame);
                        x_msg_q_send(this->m_EnhHasData, &cmd, sizeof(UINT32), 1);
                    }
                    empty_count = 0;
                } else {
                    empty_count++;
                    if (empty_count > 5) {
						PCM_DEBUG(LOG_TAG, "Mic & Awb Empty: mic(%u), awb(%u)\r\n", mic_aval_bytes, awb_aval_bytes);
                    }
                }
            }

			PCM_DEBUG(LOG_TAG, "Thread stop!\r\n");
            Asrc_Stop(this->m_u4AsrcIdx);
            //EnhMicIn_Stop(this->prMicIn);
			if(NOERR != this->prMicIn->Stop(this->prMicIn)) {//atc6048
				PCM_ERROR(LOG_TAG, "Failed to stop Virtual Mic In.\r\n");
			}

			PCM_DEBUG(LOG_TAG, "stop awb!\r\n");
            Awb_Stop(this->prAwb);
        }
    }

    this->MainThread_exit_flag = EVENT_FLAG_EXIT;
    wake_up(&this->MainThread_exit_wq);
	PCM_DEBUG(LOG_TAG, "Thread exit!\r\n");

    return ENHREC_SUCCESS;
}

static int ENH_Thread(void *data)
{
    PEnhRecord this = (PEnhRecord)data;
    ENHANCE_FRAME *enhFrame = NULL;
    WAVE_DATA_BUF_T buf = {0};
    s32 ret = 0;
    u32 temp = 0;
    size_t z_msg_size = sizeof(u32);
    u32 msg = 0;

    memset(&buf, 0, sizeof(WAVE_DATA_BUF_T));

    if (this) {
        while (!this->isThreadExit) {
			PCM_DEBUG(LOG_TAG, "ENH_Thread Wait Command:\r\n");
            wait_event(this->enh_thread_wq, ((this->enh_thread_flag == EVENT_FLAG_START) ||
                                                          (this->enh_thread_flag == EVENT_FLAG_STOP)));
			PCM_DEBUG(LOG_TAG, "ENH enh_thread_flag = %d!\r\n", this->enh_thread_flag);
            if (this->enh_thread_flag == EVENT_FLAG_STOP) {
				PCM_DEBUG(LOG_TAG, "ENH_Thread Stop!\r\n");
                break;
            }
            this->enh_thread_flag = EVENT_FLAG_NONE;
			PCM_DEBUG(LOG_TAG, "ENH_Thread Start!\r\n");

            while (true) {
                x_msg_q_receive(&temp, &msg, &z_msg_size, &this->m_EnhHasData, 1, X_MSGQ_OPTION_WAIT);
                if ((this->m_rStrmData.m_eState != SNDRV_PCM_TRIGGER_START) || (msg == 1)) {
                    break;
                }

                ret = EnhRecord_ReadOneFrame(this, &enhFrame);

                if (0 == ret) {
                    EnhRecord_Process(this, enhFrame);

                    buf.u4Buf1 = enhFrame->ulBuf1;
                    buf.u4Buf2 = enhFrame->ulBuf2;
                    buf.u4ChBufSz = this->frame_byte;
                    buf.u4Chn = 1;
                    buf.u4DataOff = 0;
                    buf.u4DataSz = buf.u4ChBufSz;

					//m_pvRingBuf_Asrc is only used for temp storage to prevent ASRC buf from becoming full.
                    RingBuf_Write(this->m_pvRingBuf_Asrc, &buf, 0);

                    EnhRecord_CopyFromAsrcOut(this);
                    EnhRecord_CopyToAsrcIn(this); //prevent asrc buf full

                    EnhRecord_CopyToUser(this);
                }
            }
        }
        this->enh_thread_exit_flag = EVENT_FLAG_EXIT;
        wake_up(&this->enh_thread_exit_wq);
		PCM_DEBUG(LOG_TAG, "ENH_Thread Exit!\r\n");
    }
}

/**** EnhRecord Interface ****/

PEnhRecord EnhRecord_Open(struct snd_pcm_substream *substream)
{
    // definition
    PEnhRecord this = NULL;

    // malloc
    this = (PEnhRecord)pcm_malloc(sizeof(EnhRecord));
    if (NULL == this) {
		PCM_ERROR(LOG_TAG, "alloc memory failed\r\n");
        goto ERROR;
    }
    this->m_rStrmData.substream = substream;
    this->m_rStrmData.ac83xx_chip = snd_pcm_substream_chip(substream);
    this->m_IsPrepared = FALSE;

	this->m_pvMicRingBuf = RingBuf_Open(MICIN_SOFT_BUF_SZ);//atc6048

    // mic open
	if (NOERR != CreateVirtualMicIn(VMT_NORMAL, &this->prMicIn)) {//EnhMicIn_Open();
		PCM_ERROR(LOG_TAG, "EnhRecord_Open: Create VirtualMicIn error!\r\n");
		goto ERROR;
	}
    if (NULL == this->prMicIn) {
		PCM_ERROR(LOG_TAG, "alloc mic memory failed\r\n");
        goto ERROR;
    }

    // awb open
    this->prAwb = Awb_Open();
    if (NULL == this->prAwb) {
		PCM_ERROR(LOG_TAG, "alloc awb memory failed\r\n");
        goto ERROR;
    }

    this->m_pvRingBuf = RingBuf_Open(AWB_SOFT_BUF_SZ);
    if (this->m_pvRingBuf ==  NULL) {
		PCM_ERROR(LOG_TAG, "Init: Alloc Ring Buffer err.\r\n");
        goto ERROR;
    }

    this->m_pvRingBuf_Asrc = RingBuf_Open(AWB_SOFT_BUF_SZ);
    if (this->m_pvRingBuf_Asrc ==  NULL) {
		PCM_ERROR(LOG_TAG, "Init: Alloc Ring Buffer For Asrc err.\r\n");
        goto ERROR;
    }

    EnhRecord_InitFrameQueue(this);
    EnhRecordCtrlInit();
    EnhRecordDumpInit(this);
    EnhRecordDelayBufferInit(this);

    this->isThreadExit = FALSE;

    return this;

ERROR:
    EnhRecord_Close(this);
    return NULL;
}

int EnhRecord_Close(PEnhRecord this)
{
    int ret = ENHREC_SUCCESS;

    if (this) {
        this->isThreadExit = TRUE;

		PCM_DEBUG(LOG_TAG, "Enter Close: Notify Main Thread Stop\r\n");
        this->Thread_wq_flag = EVENT_FLAG_STOP;
        wake_up(&this->Thread_wq);

		PCM_DEBUG(LOG_TAG, "Enter Close: Notify ENH Thread Stop\r\n");
        this->enh_thread_flag = EVENT_FLAG_STOP;
        wake_up(&this->enh_thread_wq);

		PCM_DEBUG(LOG_TAG, "Enter Close: wait Enhrecord Thread!\r\n");
        ret = wait_event_timeout(this->MainThread_exit_wq, this->MainThread_exit_flag == EVENT_FLAG_EXIT, HZ * 2);
        this->MainThread_exit_flag = EVENT_FLAG_NONE;
		PCM_DEBUG(LOG_TAG, "wait event ret = %d\r\n", ret);

		PCM_DEBUG(LOG_TAG, "Enter Close: wait ENH Thread!\r\n", ret);
        ret = wait_event_timeout(this->enh_thread_exit_wq, this->enh_thread_exit_flag == EVENT_FLAG_EXIT, HZ * 2);
        this->enh_thread_exit_flag = EVENT_FLAG_NONE;
		PCM_DEBUG(LOG_TAG, "wait event ret = %d\r\n", ret);

        Asrc_UnInit(this->m_u4AsrcIdx);

        if (this->prAwb) {
            Awb_Close(this->prAwb);
            this->prAwb = NULL;
        }

        if (this->prMicIn) {
			DeleteVirtualMicIn(this->prMicIn); //EnhMicIn_Close(this->prMicIn);
            this->prMicIn = NULL;
        }

		if (this->m_pvMicRingBuf) {//atc6048
			RingBuf_Close(this->m_pvMicRingBuf);
			this->m_pvMicRingBuf = NULL;
		}

        if (this->frame_queue) {
            pcm_free(this->frame_queue);
            this->frame_queue = NULL;
        }

        if (this->m_pvRingBuf) {
            RingBuf_Close(this->m_pvRingBuf);
            this->m_pvRingBuf = NULL;
        }

        if (this->m_pvRingBuf_Asrc) {
            RingBuf_Close(this->m_pvRingBuf_Asrc);
            this->m_pvRingBuf_Asrc = NULL;
        }

        x_msg_q_delete(this->m_EnhHasData);

        if (g_EnhMemPtr) {
            pcm_free(g_EnhMemPtr);
            g_EnhMemPtr = NULL;
        }
    }

    return ret;
}

int EnhRecord_CreateThread(PEnhRecord this)
{
    int i4Ret = 0;

	PCM_DEBUG(LOG_TAG, "CreateThread Start!\r\n");

    this->Thread_task = kthread_create(EnhRecord_Thread, (void *)this, "EnhRecordThread");
    if (IS_ERR(this->Thread_task)) {
		PCM_ERROR(LOG_TAG, "CreateThread: Create Thread Err!\r\n");
        PTR_ERR(this->Thread_task);
        this->Thread_task = NULL;
        i4Ret = (-EINVAL);
    } else {
        struct sched_param param;
        s32 ret;
        param.sched_priority = to_sched_priority(ADSPTASK_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(this->Thread_task, SCHED_RR, &param);
        ASSERT(ret == 0);

		PCM_DEBUG(LOG_TAG, "CreateThread Success!\r\n");
        this->Thread_wq_flag = EVENT_FLAG_NONE;
        this->MainThread_exit_flag = EVENT_FLAG_NONE;
        init_waitqueue_head(&this->Thread_wq);
        init_waitqueue_head(&this->MainThread_exit_wq);

        wake_up_process(this->Thread_task);
    }

    this->enh_task = kthread_create(ENH_Thread, (void *)this, "ENHThread");
    if (IS_ERR(this->enh_task)) {
		PCM_ERROR(LOG_TAG, "CreateThread: create enh thread err!\r\n");
        PTR_ERR(this->enh_task);
        this->enh_task = NULL;
        i4Ret = (-EINVAL);
    } else {
        struct sched_param param;
        s32 ret;
        param.sched_priority = to_sched_priority(ADSPTASK_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(this->enh_task, SCHED_RR, &param);
        ASSERT(ret == 0);

		PCM_DEBUG(LOG_TAG, "CreateThread: create enh thread success!\r\n");
        this->enh_thread_exit_flag = EVENT_FLAG_NONE;
        this->enh_thread_flag = EVENT_FLAG_NONE;
        init_waitqueue_head(&this->enh_thread_wq);
        init_waitqueue_head(&this->enh_thread_exit_wq);

		PCM_DEBUG(LOG_TAG, "Create Msg queue!\r\n");
        x_msg_q_create(&this->m_EnhHasData, "has data message", sizeof(u32), 20);
		PCM_DEBUG(LOG_TAG, "Create Msg queue End!\r\n");

        wake_up_process(this->enh_task);
    }

	PCM_DEBUG(LOG_TAG, "CreateThread End!\r\n");

    return i4Ret;
}

int EnhRecord_Prepare(PEnhRecord this)
{
	PCM_DEBUG(LOG_TAG, "Prepare Start >>>>>>>>>>>>>\r\n");
    if (this && (this->m_IsPrepared == FALSE)) {
        substream_data *prStrmData = &(this->m_rStrmData);
        struct snd_pcm_substream *substream = prStrmData->substream;
        struct snd_pcm_runtime *runtime = substream->runtime;

        int is8 = snd_pcm_format_width(runtime->format) == 16 ? 0 : 1;
        int mono = (runtime->channels > 1) ? 0 : 1;
        down(&g_rPCMSema);
        prStrmData->period_size = snd_pcm_lib_period_bytes(substream);
        prStrmData->buffer_size = snd_pcm_lib_buffer_bytes(substream);
        prStrmData->dma_size = snd_pcm_lib_buffer_bytes(substream);
        prStrmData->dma_start = (unsigned int)runtime->dma_area;
        prStrmData->dma_shift = 2 - mono - is8;
        prStrmData->last_ptr = 0;
        prStrmData->Used_size = 0;
        prStrmData->appl_ptr = 0;
        prStrmData->app_Base = 0;
        prStrmData->hw_Base = 0;
        prStrmData->boundary = runtime->boundary;
        prStrmData->IsBtSpeech = 0;

		PCM_DEBUG(LOG_TAG, "Prepare(%d): substream VIRSADR:0x%x, buffersize:0x%x.\r\n", (int)substream->number, (unsigned int)prStrmData->dma_start, (unsigned int)prStrmData->dma_size);
        if (is8) {
            prStrmData->m_SampleType = (mono) ? PCM_TYPE_M8 : PCM_TYPE_S8;
        } else {
            prStrmData->m_SampleType = (mono) ? PCM_TYPE_M16 : PCM_TYPE_S16;
        }
        this->frame_byte = (20 * 16000 / 1000) << (1 - is8);
		PCM_DEBUG(LOG_TAG, "--cgx--frame_byte is (%u), runtime->rate=%d, \r\n", this->frame_byte, runtime->rate);
        up(&g_rPCMSema);

        /* Init Asrc */
        this->m_rAsrcFmt.u4Chn = 2;
        this->m_rAsrcFmt.u4IBW = DEF_DATA_BITS;
        this->m_rAsrcFmt.u4OFS = runtime->rate;
        this->m_rAsrcFmt.u4OBW = DEF_DATA_BITS;
        this->m_rAsrcFmt.u4IFS = 16000;

        if ((16000 == runtime->rate) || g_ForceEnableEnhance) {//cgx
            this->m_EnhanceEnable = true;
			PCM_DEBUG(LOG_TAG, "--cgx--m_EnhanceEnable true\r\n");
        } else {
            this->m_EnhanceEnable = false;
        }

        this->m_u4AsrcIdx = ASRC_CHSET_NUM;
        AsrcMgr_AllocASRC(&this->m_rAsrcFmt, FALSE, &this->m_u4AsrcIdx);//cgx
        if (this->m_u4AsrcIdx == ASRC_CHSET_NUM) {
			PCM_ERROR(LOG_TAG, "Init: Alloc asrc err.\r\n");
            return -1;
        } else {
			PCM_DEBUG(LOG_TAG, "Init: Alloc asrc (%d).\r\n", (int)this->m_u4AsrcIdx);
        }

        this->m_IsPrepared = TRUE;


		PCM_DEBUG(LOG_TAG, "Prepare sample rate (%u), enhance enable(%u) \r\n", runtime->rate, this->m_EnhanceEnable);
    }
	PCM_DEBUG(LOG_TAG, "Prepare End <<<<<<<<<<<<<\r\n");

    return ENHREC_SUCCESS;
}

int EnhRecord_Start(PEnhRecord this)
{
    if (this && this->m_u4State != STATE_STARTED) {

		RingBuf_Reset(this->m_pvMicRingBuf);//atc6048

		//EnhMicIn_Start(this->prMicIn, 16000); //atc6048
		this->prMicIn->Setup(this->prMicIn, 16000);//atc6048
		if(NOERR != this->prMicIn->Start(this->prMicIn)) {//atc6048
			PCM_ERROR(LOG_TAG, "Failed to start Virtual Mic In.\r\n");
			goto ERROR;
		}

        this->m_rStrmData.m_eState = SNDRV_PCM_TRIGGER_START;

		PCM_DEBUG(LOG_TAG, "Notify Main Thread Start\r\n");
        this->Thread_wq_flag = EVENT_FLAG_START;
        wake_up(&this->Thread_wq);

		PCM_DEBUG(LOG_TAG, "Notify Enhance Thread Start\r\n");
        this->enh_thread_flag = EVENT_FLAG_START;
        wake_up(&this->enh_thread_wq);

        this->m_u4State = STATE_STARTED;
    }

    return ENHREC_SUCCESS;

ERROR:
	PCM_ERROR(LOG_TAG, "EnhRecord_Start fail\r\n");
	return (NORESOURCE);

}

int EnhRecord_Stop(PEnhRecord this)
{
    u32 cmd = 1;
	PCM_DEBUG(LOG_TAG, "EnhRecord_Stop >>>\r\n");
    if (this && this->m_u4State == STATE_STARTED) {
		PCM_DEBUG(LOG_TAG, "EnhRecord_Stop: Stop ENH thread  \r\n");
        x_msg_q_send(this->m_EnhHasData, &cmd, sizeof(UINT32), 2);
        this->m_rStrmData.m_eState = SNDRV_PCM_TRIGGER_STOP;
        this->m_u4State = STATE_STOPPED;
    }
    if (this) {
		PCM_DEBUG(LOG_TAG, "EnhRecord_Stop: this->m_u4State(%d) \r\n", this->m_u4State);
    }
	PCM_DEBUG(LOG_TAG, "EnhRecord_Stop (0x%p)<<< \r\n", this);

    return ENHREC_SUCCESS;
}

