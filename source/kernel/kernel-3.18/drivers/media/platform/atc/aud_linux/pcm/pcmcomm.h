
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
*[File]                   au_pcm_comm.h
*[Author]               mtk68513
*[Description]
*    Interface for pcm comm
******************************************************************************/
#ifndef __AUD_PCM_COMM_H__
#define __AUD_PCM_COMM_H__

#include <linux/fs.h>

#include <windows.h>
#include <sound/pcm.h>
#include "x_os.h"

#include "audiosys.h"
#include "aud_pcm_dbg.h"
#include "drv_thread.h"
#include "speechdev.h"


#define GET_CUR_TIME           (int)(1000 * jiffies / HZ)

// Pcm Log define 
#define T               TEXT

#if 0
#define PcmLog(fg, exp)         if (fg <= g_u4PCMLogLevel) {printk(TEXT("[PCM]")); printk exp;} 
#define PcmLog_E(exp)           if (ZONE_ERROR <= g_u4PCMLogLevel)   {printk(T("[PCM_E]")); printk exp;} 
#define PcmLog_W(exp)           if (ZONE_WARN <= g_u4PCMLogLevel)    {printk(T("[PCM_W]")); printk exp;} 
#define PcmLog_I(exp)           if (ZONE_INFO <= g_u4PCMLogLevel)    {printk(T("[PCM_I]")); printk exp;} 
#define PcmLog_D(exp)           if (ZONE_DBG <= g_u4PCMLogLevel)     {printk(T("[PCM_D]")); printk exp;} 
#define PcmLog_LP(exp)          if (ZONE_LOOP <= g_u4PCMLogLevel)    {printk(T("[PCM_LP]")); printk exp;} 
#endif

// Pcm system define
#define pcm_mutex_init(l)       mutex_init(l)
#define pcm_mutex_lock(l)       mutex_lock(l)
#define pcm_mutex_unlock(l)     mutex_unlock(l)
#define pcm_malloc(size)        kzalloc(size, GFP_KERNEL)
#define pcm_free(ptr)           kfree(ptr)

//Pcm common struct

typedef struct
{
    WAVE_DATA_BUF_T m_rBuf;
    UINT32 m_u4RP;
    UINT32 m_u4WP;
}RingBuf, *PRingBuf;


typedef struct 
{ 
    UINT32 u4ReadPos[2];
    CHAR filename[2][200];
}SphDataFile, *PSphDataFile;

#pragma pack(push, 1)

#pragma pack (pop)


// pcm comm function

UINT32 Pcm_GetBufFreeSz(UINT32 u4RP, UINT32 u4WP, UINT32 BufLen);
UINT32 Pcm_GetBufDataSz(UINT32 u4RP, UINT32 u4WP, UINT32 BufLen); 

VOID Pcm_CopyData(PWAVE_DATA_BUF_T prDst, PWAVE_DATA_BUF_T prSrc, UINT32 u4Alignment);

//========================================================//

PVOID   RingBuf_Open(UINT32 u4ChBufSz);
VOID    RingBuf_Close(PRingBuf prThis);
PVOID   RingBuf_Reset(PRingBuf prThis);

VOID    RingBuf_Read(PRingBuf prThis, PWAVE_DATA_BUF_T prRBuf, UINT32 u4Alignment);
VOID    RingBuf_Write(PRingBuf prThis, PWAVE_DATA_BUF_T prWBuf, UINT32 u4Alignment);

VOID    RingBuf_GetRBuf(PRingBuf prThis, PWAVE_DATA_BUF_T prRBuf);
VOID    RingBuf_GetWBuf(PRingBuf prThis, PWAVE_DATA_BUF_T prWBuf);

VOID    RingBuf_SetRP(PRingBuf prThis, UINT32 u4RP);
VOID    RingBuf_SetWP(PRingBuf prThis, UINT32 u4WP);

UINT32  RingBuf_GetDataLen(PRingBuf prThis);
UINT32  RingBuf_GetFreeLen(PRingBuf prThis);


//================================================//

PSphDataFile SphDataFile_Open(char *filename1, char *filename2, UINT32 u4BufLen);
UINT32       SphDataFile_Close(PSphDataFile prThis);
UINT32       SphDataFile_ReadData(PSphDataFile prThis, UINT32 u4Buf1, UINT32 u4Buf2, UINT32 u4Size);


//=====================================================//
//s32 to_sched_priority(u8 ui1_priority);//cgx


#endif // #ifndef __AUD_PCM_COMM_H__
