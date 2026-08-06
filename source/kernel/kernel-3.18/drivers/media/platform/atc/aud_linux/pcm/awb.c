#include "awb.h"
#include "DspFunc.h"
#include "oemsettings.h"
#include "aud_pcm_dbg.h"
#include "pcm_debug.h"

#define LOG_TAG "awb"

/* Begin of AwbHw */
AwbHw g_AwbHw;

int AwbHw_Init(u32 channels)
{
    memset(&g_AwbHw, 0, sizeof(g_AwbHw));
    if (channels != MONO && channels != STEREO) {
		PCM_ERROR(LOG_TAG, "Init fail. channels(%u)\r\n", channels);
        return -1;
    }
    g_AwbHw.m_rBuf.u4Buf1 = DspAwbGetSadr();//cgx

    if (channels == MONO) {
        g_AwbHw.m_rBuf.u4Buf2 = g_AwbHw.m_rBuf.u4Buf1;
    } else {
        g_AwbHw.m_rBuf.u4Buf2 = DspAwbGetSadr() + DspAwbGetChSize();
    }

    g_AwbHw.m_rBuf.u4ChBufSz = DspAwbGetChSize();
    g_AwbHw.m_rBuf.u4Chn = channels;
    g_AwbHw.m_rBuf.u4DataSz = g_AwbHw.m_rBuf.u4ChBufSz;
    g_AwbHw.m_rBuf.u4DataOff = DspAwbGetWptr();

    return 0;
}

int AwbHw_GetBuffer(PWAVE_DATA_BUF_T buf, u32 RP)
{
    u32 WP = DspAwbGetWptr();

    if (NULL != buf) {
        memcpy(buf, &g_AwbHw.m_rBuf, sizeof(WAVE_DATA_BUF_T));
        buf->u4DataOff = RP;
        buf->u4DataSz = Pcm_GetBufDataSz(RP, WP, buf->u4ChBufSz);
        return 0;
    }

    return -1;
}
/* End of AwbHw */

/* Begin of Awb instance */
static int Awb_Init(PAwb pthis)
{
    int ret = TRUE;

    /* Init Asrc */
    pthis->m_u4AwbRP = 0;
    pthis->m_rAsrcFmt.u4Chn = 2;
    pthis->m_rAsrcFmt.u4IBW = DEF_DATA_BITS;
    pthis->m_rAsrcFmt.u4OFS = 16000;
    pthis->m_rAsrcFmt.u4OBW = DEF_DATA_BITS;
    pthis->m_rAsrcFmt.u4IFS = 48000;

    pthis->m_u4AsrcIdx = ASRC_CHSET_NUM;
    AsrcMgr_AllocASRC(&pthis->m_rAsrcFmt, FALSE, &pthis->m_u4AsrcIdx);
    if (pthis->m_u4AsrcIdx == ASRC_CHSET_NUM) {
		PCM_ERROR(LOG_TAG, "Init: Alloc asrc err.\r\n");
        ret = FALSE;
    } else {
		PCM_DEBUG(LOG_TAG, "Init: Alloc asrc (%d).\r\n", (int)pthis->m_u4AsrcIdx);
    }

    /* Init Ringbuf */
    pthis->m_pvRingBuf = RingBuf_Open(AWB_SOFT_BUF_SZ);
    if (pthis->m_pvRingBuf ==  NULL) {
		PCM_ERROR(LOG_TAG, "Init: Alloc Ring Buffer err.\r\n");
        ret = FALSE;
    }

    AwbHw_Init(STEREO);

    return (ret);
}

void Awb_Uninit(PAwb pthis)
{
    Asrc_UnInit(pthis->m_u4AsrcIdx);
    RingBuf_Close(pthis->m_pvRingBuf);
	PCM_DEBUG(LOG_TAG, "Uninit: Alloc asrc (%d).\r\n", (int)pthis->m_u4AsrcIdx);
}

PAwb Awb_Open(void)
{
    PAwb pthis = NULL;

    pthis = (PAwb)pcm_malloc(sizeof(Awb));
    if (pthis) {
		PCM_DEBUG(LOG_TAG, "Open (0x%x).\r\n", pthis);
        memset(pthis, 0, sizeof(Awb));
        Awb_Init(pthis);
    }

    return pthis;
}

void Awb_Start(PAwb pthis)
{

}

void Awb_Stop(PAwb pthis)
{
    if (pthis) {
        Asrc_Stop(pthis->m_u4AsrcIdx);
    }
}

void Awb_ResetRP(PAwb pthis)
{
    if (pthis) {
        pthis->m_u4AwbRP = DspAwbGetWptr();
		PCM_DEBUG(LOG_TAG, "ResetRP: RP(%d) Time(%d)\r\n", (int)pthis->m_u4AwbRP, GET_CUR_TIME);
        Asrc_Start(pthis->m_u4AsrcIdx);
    }
}

static int Awb_CopyFromAsrcOut(PAwb pthis)
{
    WAVE_DATA_BUF_T rAsrcOut;

    memset(&rAsrcOut, 0, sizeof(rAsrcOut));

    if (NOERR == Asrc_GetOBuf(pthis->m_u4AsrcIdx, &rAsrcOut)) {
        RingBuf_Write(pthis->m_pvRingBuf, &rAsrcOut, 0);
        if (NOERR != Asrc_SetORP(pthis->m_u4AsrcIdx, rAsrcOut.u4DataOff)) {
			PCM_ERROR(LOG_TAG, "CopyFromAsrcOut: Set ASRC output read pointer error\r\n");
        }
    } else {
		PCM_ERROR(LOG_TAG, "CopyFromAsrcOut: Get ASRC output buffer error\r\n");
    }

    return 0;
}

static int Awb_CopyToAsrcIn(PAwb pthis)
{
    WAVE_DATA_BUF_T rAwbHwBuf;
    WAVE_DATA_BUF_T rAsrcIn;

    memset(&rAwbHwBuf, 0, sizeof(WAVE_DATA_BUF_T));
    memset(&rAsrcIn, 0, sizeof(WAVE_DATA_BUF_T));

    AwbHw_GetBuffer(&rAwbHwBuf, pthis->m_u4AwbRP);
    if (NOERR == Asrc_GetIBuf(pthis->m_u4AsrcIdx, &rAsrcIn)) {
        Pcm_CopyData(&rAsrcIn, &rAwbHwBuf, 0xFFFFFFF0);
        Asrc_SetIWP(pthis->m_u4AsrcIdx, rAsrcIn.u4DataOff);
        pthis->m_u4AwbRP = rAwbHwBuf.u4DataOff;
    } else {
		PCM_ERROR(LOG_TAG, "CopyToAsrcIn: Get ASRC input buffer error\r\n");
    }

    return 0;
}

int Awb_Close(PAwb pthis)
{
    int ret = 0;

    if (pthis) {
		PCM_DEBUG(LOG_TAG, "Awb Close!\r\n");
        Awb_Uninit(pthis);
        pcm_free(pthis);
        pthis = NULL;
        ret = 1;
    }

    return ret;
}

void Awb_Transfer(PAwb pthis)
{
    if (pthis) {
        Awb_CopyFromAsrcOut(pthis);
        Awb_CopyToAsrcIn(pthis);
    }
}

int Awb_Read(PAwb pthis, PWAVE_DATA_BUF_T prBuf)
{
    if (pthis) {
        RingBuf_Read(pthis->m_pvRingBuf, prBuf, 0);
    }

    return 0;
}

u32 Awb_GetDataSize(PAwb pthis)
{
    if (pthis) {
        return RingBuf_GetDataLen(pthis->m_pvRingBuf);
    }

    return 0;
}
/* End of Awb instance */
