/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/

/******************************************************************************
*[File]                       pcmcomm.c       
*[Author]                   mtk68513
*[Description]
*    implementation for  PCM Comm
******************************************************************************/
#include "pcmcomm.h"
#include "pcm_debug.h"
//=====================================//
#define CodeSight_Pcm_Buff
//=====================================//
#define LOG_TAG "pcmcomm"

UINT32 Pcm_GetBufFreeSz(UINT32 u4RP, UINT32 u4WP, UINT32 BufLen)
{
    UINT32 u4FreeSize;
    
    if (u4RP > u4WP) {
        u4FreeSize = u4RP - u4WP;
    } else  {
        u4FreeSize = BufLen - u4WP + u4RP;
    }
    return (u4FreeSize);
}

UINT32 Pcm_GetBufDataSz(UINT32 u4RP, UINT32 u4WP, UINT32 BufLen)
{
    UINT32 u4DataSize;
    
    if (u4WP >= u4RP) {
        u4DataSize = u4WP - u4RP;
    } else {
        u4DataSize = BufLen - u4RP + u4WP;
    }

    return u4DataSize;
} 


//============================================================================//
VOID Pcm_CopyData(PWAVE_DATA_BUF_T prDst, PWAVE_DATA_BUF_T prSrc, UINT32 u4Alignment)
{
    UINT32 i, j, u4RP, u4WP, u4DataSz[2], u4CopySz[2];
    UINT32 u4Size = (prDst->u4DataSz < prSrc->u4DataSz) ? prDst->u4DataSz : prSrc->u4DataSz;

    if (u4Alignment != 0) {
        u4Size &= u4Alignment;
    }
    if (u4Size == 0) {
        goto EXIT;
    }

    u4RP = prSrc->u4DataOff;
    if (prSrc->u4DataOff + u4Size >= prSrc->u4ChBufSz) {
        u4DataSz[0] = prSrc->u4ChBufSz - prSrc->u4DataOff;
        u4DataSz[1] = u4Size - u4DataSz[0];
        prSrc->u4DataOff = u4DataSz[1];
    } else {
        u4DataSz[0] = u4Size;
        u4DataSz[1] = 0;  
        prSrc->u4DataOff += u4Size;
    }

    for (i = 0; i < 2 && u4DataSz[i]; i++)
    {
        u4WP = prDst->u4DataOff;
        if (prDst->u4DataOff + u4DataSz[i] >= prDst->u4ChBufSz) {
            u4CopySz[0] = prDst->u4ChBufSz - prDst->u4DataOff;
            u4CopySz[1] = u4DataSz[i] - u4CopySz[0];
            prDst->u4DataOff = u4CopySz[1];
        } else {
            u4CopySz[0] = u4DataSz[i];
            u4CopySz[1] = 0;  
            prDst->u4DataOff += u4DataSz[i];
        }

        for (j = 0; j < 2 && u4CopySz[j]; j++)
        {
            x_memcpy((VOID *)(prDst->u4Buf1 + u4WP), (VOID *)(prSrc->u4Buf1 + u4RP), u4CopySz[j]);
            if (prDst->u4Chn == 2) {
                UINT32 u4SrcBuf2 = (prSrc->u4Chn == 2) ? prSrc->u4Buf2 : prSrc->u4Buf1;
                x_memcpy((VOID *)(prDst->u4Buf2 + u4WP), (VOID *)(u4SrcBuf2 + u4RP), u4CopySz[j]);  
            }
            
            u4RP += u4CopySz[j];
            u4WP = 0;   // for rollback Dst Buffer
        }
        
        u4RP = 0;       // for rollback Src Buffer
    }

EXIT:
    return;
}

//=====================================//
#define CodeSight_RingBuffer
//=====================================//

PVOID RingBuf_Open(UINT32 u4ChBufSz)
{
    PRingBuf prThis = NULL;

    prThis = (PRingBuf)pcm_malloc(sizeof(RingBuf));
    if (prThis) {
        prThis->m_rBuf.u4Buf1 = (UINT32)pcm_malloc(u4ChBufSz << 1);
        if (prThis->m_rBuf.u4Buf1) {
            prThis->m_rBuf.u4ChBufSz = u4ChBufSz;
            prThis->m_rBuf.u4Buf2 = prThis->m_rBuf.u4Buf1 + u4ChBufSz;
            prThis->m_rBuf.u4Chn = 2;
            RingBuf_Reset(prThis);
        } else {
			PCM_ERROR(LOG_TAG, "Open: alloc buffer error!! \r\n");
        }
    } else {
		PCM_ERROR(LOG_TAG, "Open: alloc RingBuf struct error!! \r\n");
    }

    return (PVOID)(prThis);
}


VOID RingBuf_Close(PRingBuf prThis)
{
    if (prThis) {
        if (prThis->m_rBuf.u4Buf1) {
            pcm_free(prThis->m_rBuf.u4Buf1);
            prThis->m_rBuf.u4Buf1 = NULL;
        }
        pcm_free(prThis);
        prThis = NULL;
    }
}


PVOID RingBuf_Reset(PRingBuf prThis)
{
    if (prThis) {
        prThis->m_u4RP = 0;
        prThis->m_u4WP = 0;

        memset(prThis->m_rBuf.u4Buf1, 0, (prThis->m_rBuf.u4ChBufSz << (prThis->m_rBuf.u4Chn - 1)));
    }
}


VOID RingBuf_Read(PRingBuf prThis, PWAVE_DATA_BUF_T prRBuf,  UINT32 u4Alignment)
{
    if (prThis) {
        prThis->m_rBuf.u4DataOff = prThis->m_u4RP;
        prThis->m_rBuf.u4DataSz = Pcm_GetBufDataSz(prThis->m_u4RP, prThis->m_u4WP, prThis->m_rBuf.u4ChBufSz);

        Pcm_CopyData(prRBuf, &prThis->m_rBuf, u4Alignment);
        prThis->m_u4RP = prThis->m_rBuf.u4DataOff;
    }
}


VOID RingBuf_Write(PRingBuf prThis, PWAVE_DATA_BUF_T prWBuf, UINT32 u4Alignment)
{
    if (prThis) {
        prThis->m_rBuf.u4DataOff = prThis->m_u4WP;
        prThis->m_rBuf.u4DataSz = Pcm_GetBufFreeSz(prThis->m_u4RP, prThis->m_u4WP, prThis->m_rBuf.u4ChBufSz);

        Pcm_CopyData(&prThis->m_rBuf, prWBuf, u4Alignment);
        prThis->m_u4WP = prThis->m_rBuf.u4DataOff;
    }
}


VOID RingBuf_GetRBuf(PRingBuf prThis, PWAVE_DATA_BUF_T prRBuf)
{
    if (prThis) {
        prThis->m_rBuf.u4DataOff = prThis->m_u4RP;
        prThis->m_rBuf.u4DataSz = Pcm_GetBufDataSz(prThis->m_u4RP, prThis->m_u4WP, prThis->m_rBuf.u4ChBufSz);
        x_memcpy(prRBuf, &prThis->m_rBuf, sizeof(WAVE_DATA_BUF_T));
    }
}


VOID RingBuf_GetWBuf(PRingBuf prThis, PWAVE_DATA_BUF_T prWBuf)
{
    if (prThis) {
        prThis->m_rBuf.u4DataOff = prThis->m_u4WP;
        prThis->m_rBuf.u4DataSz = Pcm_GetBufFreeSz(prThis->m_u4RP, prThis->m_u4WP, prThis->m_rBuf.u4ChBufSz);
        x_memcpy(prWBuf, &prThis->m_rBuf, sizeof(WAVE_DATA_BUF_T));
    }
}


VOID RingBuf_SetRP(PRingBuf prThis, UINT32 u4RP)
{
    if (prThis) {
        prThis->m_u4RP = u4RP;
    }
}


VOID RingBuf_SetWP(PRingBuf prThis, UINT32 u4WP)
{
    if (prThis) {
        prThis->m_u4WP = u4WP;
    }
}


UINT32 RingBuf_GetDataLen(PRingBuf prThis)
{
    UINT32 u4DataLen = 0;
    if (prThis) {
        u4DataLen = Pcm_GetBufDataSz(prThis->m_u4RP, prThis->m_u4WP, prThis->m_rBuf.u4ChBufSz);;
    }
    return (u4DataLen);
}


UINT32 RingBuf_GetFreeLen(PRingBuf prThis)
{
    UINT32 u4FreeLen = 0;
    if (prThis) {
        u4FreeLen = Pcm_GetBufFreeSz(prThis->m_u4RP, prThis->m_u4WP, prThis->m_rBuf.u4ChBufSz);;
    }
    return (u4FreeLen);
}


//=====================================//
#define CodeSight_SphDataFile
//=====================================//

PSphDataFile SphDataFile_Open(char *filename1, char *filename2, UINT32 u4BufLen)
{
    PSphDataFile prThis = (PSphDataFile)pcm_malloc(sizeof(SphDataFile));

	PCM_DEBUG(LOG_TAG, "[SphDataFile(0x%x)]Open ! \r\n", (UINT32)prThis);
    if (prThis)
    {
        if (filename1) {
            sprintf(prThis->filename[0], filename1);
            prThis->u4ReadPos[0] = sizeof(WaveHeader);
        } 
        
        if (filename2) {
            sprintf(prThis->filename[1], filename2);
            prThis->u4ReadPos[1] = sizeof(WaveHeader);
        }              
    }
    
    goto EXIT;
ERROR:
    SphDataFile_Close(prThis);
EXIT:
    
    return (prThis);
}


UINT32 SphDataFile_Close(PSphDataFile prThis)
{
	PCM_DEBUG(LOG_TAG, "[SphDataFile(0x%x)]Close ! \r\n", (UINT32)prThis);
    if (prThis)
    { 
        pcm_free(prThis);
        prThis = NULL;
    }

    return (NOERR);
}


UINT32 SphDataFile_ReadData(PSphDataFile prThis, UINT32 u4Buf1, UINT32 u4Buf2, UINT32 u4ReadSize)
{     
    ssize_t u4size = 0;   
    mm_segment_t fs = get_fs();
    set_fs(KERNEL_DS);
    
    if (prThis)
    {       
        UINT32 u4DstBuf[2] = {u4Buf1, u4Buf2};
        loff_t t_cur_pos = 0;
        UINT32 i = 0;
        
        for (i = 0; i < 2 && u4DstBuf[i]; i++)
        {
            struct file *pFile = filp_open(prThis->filename[i], O_RDONLY, 0);
            if (IS_ERR(pFile)) {
				PCM_ERROR(LOG_TAG, "[SphDataFile(0x%x)]ReadData: filp_open(%d) err(%lu)! \r\n", (UINT32)prThis, i, (UINT32)pFile);
                continue;
            }

            t_cur_pos = vfs_llseek(pFile, (loff_t)prThis->u4ReadPos[i], SEEK_SET);
            u4size = vfs_read(pFile, (VOID *)u4DstBuf[i], u4ReadSize, &t_cur_pos);
            prThis->u4ReadPos[i] += u4ReadSize;
            filp_close(pFile, NULL);    
            if (u4size < 0) {
				PCM_ERROR(LOG_TAG, "[SphDataFile(0x%x)]ReadData: vfs_read(i) err(%i)!\r\n", (UINT32)prThis, i, u4size);
            }                
        }
    }
EXIT:
    set_fs(fs);
    
    return (u4size);
}


//============================================================================//
#if 0//cgx
s32 to_sched_priority(u8 ui1_priority)
{
	s32 sched_priority;

	sched_priority = 100 - (s32)ui1_priority * 100 / 256;
	if (sched_priority < 1)
		sched_priority = 1;
	if (sched_priority > 99)
		sched_priority = 99;
	return sched_priority;
}
#endif
