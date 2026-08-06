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




/************************************************************************************************
                                                       Head File Include
*************************************************************************************************/
#include "aud_oal.h"
#include "drv_thread.h"

#include "aud_drv_config.h"
#include "aud_se.h"     // (aud_se_v2)
#include "GpsMix_mw.h"
#include "aud_config.h"
#include "aud_drv.h"
#include "aud_ioctrl.h"
#include "aud_if.h"
#include "audin_if.h"

#include "aud_esm.h"
#include "aud_test_if.h"
#include "aud_debug.h"
#include "audmhl_if.h"
#include "aud_aout_hal_if.h"
#include "aud_dsp_cfg.h"
#include "DspFunc.h"
#include "DspShm.h"
#include "DspUop.h"
#if CONFIG_AUD_ADSP_ERR_RECOVER_EN
#include "DspErrProc.h"
#endif

/************************************************************************************************
                                                   Classify Audio CLI cmd 
*************************************************************************************************/
#define ARG_INIT_VAL                    0xA5A5A5A5
#define LOG_CLI                         0            //LOG_CTRLF

#define AUD_CLI_GRP_HELP                0

#define AUD_CLI_SYSTEM_GRP              100
#define AUD_CLI_SYSTEM_HELP             (AUD_CLI_SYSTEM_GRP    + 0)
#define AUD_CLI_OPEN_LOG                (AUD_CLI_SYSTEM_GRP    + 1)
#define AUD_CLI_CHECK_DSP_STATE         (AUD_CLI_SYSTEM_GRP    + 2)
#define AUD_CLI_SHOW_DSP_STATUS         (AUD_CLI_SYSTEM_GRP    + 3)
#define AUD_CLI_SHOW_CONFIG             (AUD_CLI_SYSTEM_GRP    + 4)
#define AUD_CLI_MW_CMD                  (AUD_CLI_SYSTEM_GRP    + 5)
#define AUD_CLI_UNDERRUN                (AUD_CLI_SYSTEM_GRP    + 6)
#define AUD_CLI_ASRC                    (AUD_CLI_SYSTEM_GRP    + 7)
#define AUD_CLI_SHOW_VERSION            (AUD_CLI_SYSTEM_GRP    + 8)
#define AUD_CLI_SHOW_INTHIST            (AUD_CLI_SYSTEM_GRP    + 9)
#define AUD_CLI_AP_DVP_INT              (AUD_CLI_SYSTEM_GRP    + 10)
#define AUD_CLI_ERR_RECOVER_LOG         (AUD_CLI_SYSTEM_GRP    + 11)
#define AUD_CLI_TEMP_TEST               (AUD_CLI_SYSTEM_GRP    + 99)

#define AUD_CLI_MEMORY_GRP              200
#define AUD_CLI_MEMORY_HELP             (AUD_CLI_MEMORY_GRP    + 0)
#define AUD_CLI_R_CDRAM                 (AUD_CLI_MEMORY_GRP    + 1)
#define AUD_CLI_W_CDRAM                 (AUD_CLI_MEMORY_GRP    + 2)
#define AUD_CLI_R_SRAM                  (AUD_CLI_MEMORY_GRP    + 3)
#define AUD_CLI_W_SRAM                  (AUD_CLI_MEMORY_GRP    + 4)
#define AUD_CLI_R_SHM                   (AUD_CLI_MEMORY_GRP    + 5)
#define AUD_CLI_W_SHM                   (AUD_CLI_MEMORY_GRP    + 6)
#define AUD_CLI_R_DRV                   (AUD_CLI_MEMORY_GRP    + 7)
#define AUD_CLI_W_DRV                   (AUD_CLI_MEMORY_GRP    + 8)
#define AUD_CLI_DUMP_DRAM               (AUD_CLI_MEMORY_GRP    + 9)
#define AUD_CLI_AUD_WORKINGBUF          (AUD_CLI_MEMORY_GRP    + 10)



#define AUD_CLI_POST_GRP                300
#define AUD_CLI_POST_HELP               (AUD_CLI_POST_GRP + 0)
#define AUD_CLI_PP_BYPASS               (AUD_CLI_POST_GRP + 1)
#define AUD_CLI_REVERB                  (AUD_CLI_POST_GRP + 2)
#define AUD_CLI_EQUALIZER               (AUD_CLI_POST_GRP + 3)
#define AUD_CLI_BASSM                   (AUD_CLI_POST_GRP + 4)
#define AUD_CLI_PROLOGICII              (AUD_CLI_POST_GRP + 5)
#define AUD_CLI_UPMIX                   (AUD_CLI_POST_GRP + 6)
#define AUD_CLI_CSII                    (AUD_CLI_POST_GRP + 7)
#define AUD_CLI_SPEC                    (AUD_CLI_POST_GRP + 8)
#define AUD_CLI_LOUDNESS                (AUD_CLI_POST_GRP + 9)
#define AUD_CLI_LR_MIX                  (AUD_CLI_POST_GRP + 10)
#define AUD_CLI_TESTTONE                (AUD_CLI_POST_GRP + 11)
#define AUD_CLI_EQ_ONE_BAND             (AUD_CLI_POST_GRP + 12)
#define AUD_CLI_MVS                     (AUD_CLI_POST_GRP + 13)
#define AUD_CLI_ATS                     (AUD_CLI_POST_GRP + 14)
#define AUD_CLI_EQ_IIR_COEF             (AUD_CLI_POST_GRP + 15)
#define AUD_CLI_ARM_EXT_PP              (AUD_CLI_POST_GRP + 16)



#define AUD_CLI_AOUT_GRP                400
#define AUD_CLI_AOUT_HELP               (AUD_CLI_AOUT_GRP + 0)
#define AUD_CLI_SET_FAOUT               (AUD_CLI_AOUT_GRP + 1)
#define AUD_CLI_SET_RAOUT               (AUD_CLI_AOUT_GRP + 2)
#define AUD_CLI_AOUT_STATE              (AUD_CLI_AOUT_GRP + 3)
#define AUD_CLI_EXC_AOUT                (AUD_CLI_AOUT_GRP + 4)
#define AUD_CLI_EXC_IEC                 (AUD_CLI_AOUT_GRP + 5)
#define AUD_CLI_SET_IEC_REG             (AUD_CLI_AOUT_GRP + 6)
#define AUD_CLI_SET_DAC_TYPE            (AUD_CLI_AOUT_GRP + 7)
#define AUD_CLI_EXC_PWMDAC              (AUD_CLI_AOUT_GRP + 8)
#define AUD_CLI_SET_SPDIF               (AUD_CLI_AOUT_GRP + 9)
#define AUD_CLI_SEL_FRN_RERA            (AUD_CLI_AOUT_GRP + 10)
#define AUD_CLI_SET_SPDIF_IEC_TYPE      (AUD_CLI_AOUT_GRP + 11)

#define AUD_CLI_VOLUME_GRP              500
#define AUD_CLI_VOLUME_HELP             (AUD_CLI_VOLUME_GRP + 0)
#define AUD_CLI_GET_OUTPUT_VOL          (AUD_CLI_VOLUME_GRP + 1)
#define AUD_CLI_PRINT_VOL_GAIN          (AUD_CLI_VOLUME_GRP + 2)
#define AUD_CLI_REAR_VOL_CTRL           (AUD_CLI_VOLUME_GRP + 3)
#define AUD_CLI_MODIFY_VOL              (AUD_CLI_VOLUME_GRP + 4)
#define AUD_CLI_VOL_DETECT              (AUD_CLI_VOLUME_GRP + 5)
#define AUD_CLI_VOL_SET                 (AUD_CLI_VOLUME_GRP + 6)


#define AUD_CLI_MEDIA_GRP               600
#define AUD_CLI_MEDIA_HELP              (AUD_CLI_MEDIA_GRP + 0)
#define AUD_CLI_DEC1_CTRL               (AUD_CLI_MEDIA_GRP + 1)
#define AUD_CLI_DEC4_CTRL               (AUD_CLI_MEDIA_GRP + 2)
#define AUD_CLI_GPSMIX_CTRL             (AUD_CLI_MEDIA_GRP + 3)
#define AUD_CLI_LINEIN_REAR_BYPASS      (AUD_CLI_MEDIA_GRP + 4)
#define AUD_CLI_PLAYBACK_TIME           (AUD_CLI_MEDIA_GRP + 5)
#define AUD_CLI_MHL_TEST                (AUD_CLI_MEDIA_GRP + 6)

#define AUD_CLI_ESM_PTS_GRP             700
#define AUD_CLI_ESM_PTS_HELP            (AUD_CLI_ESM_PTS_GRP + 0)
#define AUD_CLI_PRINT_AU                (AUD_CLI_ESM_PTS_GRP + 1)
#define AUD_CLI_DUMP_AFIFO              (AUD_CLI_ESM_PTS_GRP + 2)
#define AUD_CLI_DUMP_AOUT               (AUD_CLI_ESM_PTS_GRP + 3)
#define AUD_CLI_ESM_STATE               (AUD_CLI_ESM_PTS_GRP + 4)
#define AUD_CLI_GET_PTS_INFO            (AUD_CLI_ESM_PTS_GRP + 5)
#define AUD_CLI_GET_PTS_QUEUE           (AUD_CLI_ESM_PTS_GRP + 6)
#define AUD_CLI_UPDATE_PTS_QUEUE        (AUD_CLI_ESM_PTS_GRP + 7)
#define AUD_CLI_SET_ADJUSTMIRACAST      (AUD_CLI_ESM_PTS_GRP + 8)

#define AUD_CLI_AUDIN_GRP               800
#define AUD_CLI_AUDIN_PARAMS            (AUD_CLI_AUDIN_GRP + 0)

#define AUD_CLI_MHL_GRP                 900
#define AUD_CLI_MHL_INFO                (AUD_CLI_MHL_GRP + 0)



/************************************************************************************************
      Struct and Global Variables Define
*************************************************************************************************/
typedef struct _tagAudDumpFifo
{
    void *  hFs;
    s8 * filename;
    void *pUsr;

    u32(*u4GetFIFOSAdr)(void* pUsr);
    u32(*u4GetFIFOEAdr)(void* pUsr);
    u32(*u4GetWPtr)(void* pUsr);
} AUD_DUMP_FIFO;

AUD_DUMP_FIFO tAudDumpFifo;
bool _bAudDumpFifoThreadStart = FALSE;
struct task_struct *g_hAudDumpFifoTask = NULL;


typedef struct _tagAudDumpAout
{
    void *  hFs;
    u32  u4ChInd;

    u32(*u4GetFIFOSAdr)(void* pUsr);
    u32(*u4GetFIFOEAdr)(void* pUsr);
    u32(*u4GetWPtr)(void* pUsr);
} AUD_DUMP_AOUT;

#define AUD_AOUT_CHNUM 6

AUD_DUMP_AOUT g_rAudDumpAout;
void *  g_hAudDumAoutFs[AUD_AOUT_CHNUM] = {NULL};
bool _bAudDumpAoutThreadStart = FALSE;
struct task_struct *g_hAudDumpAoutTask = NULL;

extern CLICmd g_rCLICmd1;

extern AUD_ESM_CONTEXT_T g_rAudEsmContext[];

/************************************************************************************************
                                                       Extern Function Include
*************************************************************************************************/
extern  u32 dReadDspASRCDram(u32 addr);
extern void AudShowDspStatus(void);
extern void AudShowConfig(void);
extern void AudDispIECRegisters(void);
extern void vDspCLIGetPtsInfo(AUD_DSP_PTS_INF* prInfo);
extern void vWriteCommDram(u16 u2Addr,u32 u4Value);
extern  u32 u4CliDbgReadDspSram(u8 u1DspId, u32 u4Addr);
extern  void vCliDbgWriteDspSram (u8 u1DspId,u32 u4Addr, u32 u4Value);
extern void AudDispStates(void);
extern void vAdspMasterVolume (u8 u1Volume);
extern void vAdspMasterVolumeGain(u32 u4Volume);
extern u32 u4AudHalGetDSPDataPageStartAddr(u32 u4PageId);
extern void vAdspSetSpeakerConfig(AUD_DEC_SPEAKER_LAYOUT_T rSpeakerLayout);
#ifdef HDMI_AUDIO_IN_SW_DEBUG
extern void vAudMhlDbgTst(u32 u4OnOff, u32 u4StrType);
#endif
/************************************************************************************************
                             Function Define
*************************************************************************************************/
void AudCliComDram(u32 arg1,u32 arg2);
void AudCliDspShareInfo(u32 arg1);
void AudCliDspShareInfoWrite(u32 arg1,u32 arg2,u32 arg3,u32 arg4);
void AudCliUnderRunCounter(u32 arg1);
void AudCliCmdLog(u32 arg1);
void AudCliGetPtsQueue(u32 arg1,u32 arg2);
void AudCliPtsUpdateQueue(u32 arg1,u32  arg2);
void AudCliDspCommDramWhite(u32 arg1, u32 arg2);
void AudCliDspReadSram(u32 arg1,u32 arg2,u32 arg3);
void AudCliDspWriteSram(u32 arg1,u32 arg2,u32 arg3);
void AudCliPrintAu(u32 arg1);
void AudCliDispStates(void);
void AudCliCmpUopVolume(u32 arg1,u32 arg2);
void AudCliExangeIEC(u32 arg1);
void AudCliExangeAout(u32 arg1);
void AudCliDumpAout(u32 u4Cmdid,const s8 ** filename);
void AudCliShowEsmStatus(u32 arg1);
void AudCliReverbSt(void);
void AudCliPl2St(void);
void AudCliEqSt(void);
void AudCliSpectSt(void);
void AudCliBassMSt(u32 arg1, u32 arg2);
void AudCliByPassSE(u32 arg1, u32 arg2);

 extern u32 g_u4AudLogLevel;
 extern bool g_fgPrintAU;

#define AUD_CLI_DUMP_AFIFO_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Dump AFIFO data: \n ")\
    TEXT("                start[1/2/4][0][0][filename] \n")\
    TEXT("                [9]Get AFIFO read/write pointer \n")\
    TEXT("                stop [0]\n"), AUD_CLI_DUMP_AFIFO)        

#define AUD_CLI_DUMP_AOUT_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Dump Aout data[0:stop, other: start][0][0][filename] \n"), AUD_CLI_DUMP_AOUT)

#ifndef __linux__
/************************************************************************************************
 Function Name: vDumpAfifoCloseFile
 Function Description:close the file
 Input para:hFs:file handle
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
static void vDumpFifoCloseFile(void * hFs)
{
    if (INVALID_HANDLE_VALUE != hFs)
    {
        filp_close((struct file *)hFs, NULL);
    }

}

/************************************************************************************************
 Function Name: i4DumpAFifoFsWriteFile
 Function Description:write afifo data was dumped  to file
 input para:hFs:file handle,pbBuf:buffer,u4Size:write size,pWriteSize:actual write size
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
static bool i4DumpFifoFsWriteFile(void * hFs, const void *pbBuf, u32 u4Size,u32 *pWriteSize,void *pNone)
{
    bool i4Ret;

    i4Ret = WriteFile(hFs,pbBuf,u4Size,pWriteSize,NULL);

    if (*pWriteSize != u4Size)
    {
        LOG(LOG_OTHER, TEXT("[AudCliCmd] [DUMPAFIFO]Try write[%d], Real write [%d] \n"),
            u4Size,*pWriteSize);
    }
    else
    {
        //RETAILMSG(1, (TEXT("[AudCliCmd] [DUMPAFIFO]wirte file[hFs=0x%8x] [Actual Write=%d]\n"),
        //    hFs,*pWriteSize));
    }

    return i4Ret;
}

/************************************************************************************************
  Function Name: vAudDumpAfifoTask
  Function Description:Dump Afifo Thread
  Author:fei.zhu
  Data:2010.12.12
  Modify Data:
           2011.3.7  modify format according coding standard
*************************************************************************************************/
static s32 vAudDumpFifoTask(void* pvArg)
{
    bool i4Ret = FALSE;
    bool* bRunFlag = &_bAudDumpFifoThreadStart;
    u32 u4Size = 0;
    u32 u4TotalSz = 0;
    AUD_DUMP_FIFO *pDump = &tAudDumpFifo;
    u32 dwWriteSize = 0;  
    u32 u4SAdr = pDump->u4GetFIFOSAdr(pDump->pUsr);
    u32 u4EAdr = pDump->u4GetFIFOEAdr(pDump->pUsr);
    u32 u4FifoSize = u4EAdr - u4SAdr;
    u32 u4WPtr = pDump->u4GetFIFOSAdr(pDump->pUsr);//dump read pointer
    u32 u4CWPtr = 0;//real afifo write pointer

    LOG(LOG_CTRLF,TEXT("[AudCliCmd] Enter DumpFifoThreadTask! \r\n"));

    while(*bRunFlag)
    {
        if(*bRunFlag)
        {
            u4CWPtr = pDump->u4GetWPtr(pDump->pUsr);
        }
        if (u4CWPtr != u4WPtr)
        {
            // Write data to file
            if (u4CWPtr > u4WPtr)
            {
                u4Size = u4CWPtr - u4WPtr;
            }
            else
            {
                u4Size = u4FifoSize - (u4WPtr - u4CWPtr);
            }

            if(*bRunFlag)
            {
                u4Size &= 0xFFFFFF00;       // 256 alignment
                if (u4Size < (4*1024))      //data < 4K
                {
                    mdelay(50);
                    continue;
                }
             }

            if ((u4Size + u4WPtr) > u4EAdr)
            {
                u4Size = u4EAdr - u4WPtr;
            }
            LOG(LOG_CTRLF,TEXT("[AudCliCmd]DUMPFIFO u4WPtr = 0x%x,u4CWPtr = 0x%x,u4Size = 0x%x,\r\n"),
                         u4WPtr, u4CWPtr, u4Size);

            i4Ret = i4DumpFifoFsWriteFile(pDump->hFs,(void *)u4WPtr,u4Size,&dwWriteSize,NULL);

            if(dwWriteSize > 0)
            {
                 u4TotalSz += (u32)dwWriteSize;
                 LOG(LOG_CTRLF, TEXT("[AudCliCmd]DUMPFIFO wirte file successed TotalSz = 0x%x \r\n"),
                    u4TotalSz);
            }

            if (dwWriteSize != u4Size)
            {
                LOG(LOG_CTRLF, TEXT("[AudCliCmd]DUMPAFIFO wirte file failed![%d]--->[%d] \r\n"),
                    u4Size,dwWriteSize);
                break;
            }

            //update reference pointer
            u4WPtr += u4Size;
            if (u4WPtr >= u4EAdr)
            {
                u4WPtr = u4SAdr;
            }
        }

        if(*bRunFlag)        
        {
            mdelay(100);
        }
    }
    vDumpFifoCloseFile(pDump->hFs);

    //fgCliDumpAfifoEnd = FALSE;

    LOG(LOG_CTRLF, TEXT("[AudCliCmd] DUMPFIFO total wirte file size =%d(k) \n"),u4TotalSz/1024);
    LOG(LOG_CTRLF, TEXT("[AudCliCmd] DUMPFIFO Exit _bAudDumpAfifoThread"));

    complete_and_exit(NULL, 0);
}

static s32 vAudDumpAoutTask(void* pvArg)
{
    bool* bRunFlag = NULL;
    u32 u4Size[AUD_AOUT_CHNUM] = {0};
    u32 u4TotalSz[AUD_AOUT_CHNUM] = {0};
    u32 dwWriteSize[AUD_AOUT_CHNUM] = {0};  
    u32 u4SAdr[AUD_AOUT_CHNUM] = {0};
    u32 u4EAdr[AUD_AOUT_CHNUM] = {0};
    u32 u4FifoSize[AUD_AOUT_CHNUM] = {0};
    u32 u4WPtr[AUD_AOUT_CHNUM] = {0};
    u32 u4CWPtr[AUD_AOUT_CHNUM] = {0};//real afifo write pointer
    u32 u4Ind =0;

    bRunFlag = &_bAudDumpAoutThreadStart;
    
    for(u4Ind = 0; u4Ind <AUD_AOUT_CHNUM; u4Ind++)
    {
        u4SAdr[u4Ind] = g_rAudDumpAout.u4GetFIFOSAdr((void*)(u4Ind+1));
        u4EAdr[u4Ind] = g_rAudDumpAout.u4GetFIFOEAdr((void*)(u4Ind+1));
        u4FifoSize[u4Ind] = u4EAdr[u4Ind] - u4SAdr[u4Ind];
        u4WPtr[u4Ind] = g_rAudDumpAout.u4GetFIFOSAdr((void*)(u4Ind+1));//dump read pointer
    }    

    LOG(LOG_CTRLF,TEXT("[AudCliCmd] Enter DumpFifoThreadTask.\r\n"));
    LOG(LOG_CTRLF,TEXT("[AudCliCmd]u4SAdr = 0x%x,u4EAdr = 0x%x,u4FifoSize = 0x%x,\r\n"),
                         u4SAdr[0], u4EAdr[0], u4FifoSize[0]);

    while(*bRunFlag)
    {
        if(*bRunFlag)
        {
            for(u4Ind = 0; u4Ind <AUD_AOUT_CHNUM; u4Ind++)
            {                
                u4CWPtr[u4Ind] = g_rAudDumpAout.u4GetWPtr((void*)(u4Ind+1));
            }
        }
        
        for(u4Ind = 0; u4Ind < AUD_AOUT_CHNUM; u4Ind++)
        { 
            if (u4CWPtr[u4Ind] != u4WPtr[u4Ind])
            {
                // Write data to file
                if (u4CWPtr[u4Ind] > u4WPtr[u4Ind])
                {
                    u4Size[u4Ind] = u4CWPtr[u4Ind] - u4WPtr[u4Ind];
                }
                else
                {
                    u4Size[u4Ind] = u4FifoSize[u4Ind] - (u4WPtr[u4Ind] - u4CWPtr[u4Ind]);
                }

                
                if(*bRunFlag)
                {
                    u4Size[u4Ind] &= 0xFFFFFF00;       //256 alignment
                    if (u4Size[u4Ind] < (4*1024))      //data < 4K
                    {                    
                        //mdelay(10);
                        continue;
                    }
                }

                if ((u4Size[u4Ind] + u4WPtr[u4Ind]) > u4EAdr[u4Ind])
                {
                    u4Size[u4Ind] = u4EAdr[u4Ind] - u4WPtr[u4Ind];
                }
                          
                i4DumpFifoFsWriteFile(g_hAudDumAoutFs[u4Ind],(void *)(u4WPtr[u4Ind]),u4Size[u4Ind],&(dwWriteSize[u4Ind]),NULL);

                if(dwWriteSize[u4Ind] > 0)
                {
                    u4TotalSz[u4Ind] += (u32)dwWriteSize[u4Ind];
                }

                if (dwWriteSize[u4Ind] != u4Size[u4Ind])
                {
                    LOG(LOG_CTRLF, TEXT("[AudCliCmd]DUMPAFIFO wirte file failed![%d]--->[%d] \r\n"),
                        u4Size[u4Ind],dwWriteSize[u4Ind]);
                    break;
                }

                //update reference pointer
                u4WPtr[u4Ind] += u4Size[u4Ind];
                if(u4WPtr[u4Ind] >= u4EAdr[u4Ind])
                {
                    u4WPtr[u4Ind] = u4SAdr[u4Ind];
                }
            }
        }

        
        //RETAILMSG(1,(TEXT("[AudCliCmd]DUMPFIFO u4WPtr = 0x%x,u4CWPtr = 0x%x,u4Size = 0x%x,\r\n"),
                           //u4WPtr[0], u4CWPtr[0], u4Size[0]));

        if(*bRunFlag)        
        {
            mdelay(30);
        }
    }

    for(u4Ind = 0; u4Ind <AUD_AOUT_CHNUM; u4Ind++)
    { 
        vDumpFifoCloseFile(g_hAudDumAoutFs[u4Ind]);
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] DUMPFIFO total wirte file size =%d(k) \n"),u4TotalSz[u4Ind]/1024);
    }  

    LOG(LOG_CTRLF, TEXT("[AudCliCmd] DUMPFIFO Exit _bAudDumpAfifoThread"));

    complete_and_exit(NULL, 0);
}

/************************************************************************************************
 Function Name: vAudDumpAfifoThreadStart
 Function Description:create dump afifo Task
 input para: pAudDumpAfifoThread:dumo afifo struct
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
static void vAudDumpFifoThreadStart(AUD_DUMP_FIFO *pAudDumpFifoThread)
{
    if(!_bAudDumpFifoThreadStart)
    {
        if (pAudDumpFifoThread)
        {
            _bAudDumpFifoThreadStart = TRUE;
            x_memcpy(&tAudDumpFifo, pAudDumpFifoThread, sizeof(tAudDumpFifo));

            LOG(LOG_CTRLF, TEXT("[AudCliCmd] Start DumpFifo task! \r\n"));
            //Create dump afifo Task
            
            g_hAudDumpFifoTask = kthread_create(vAudDumpFifoTask, (void *)NULL, "AudDumpFifoThread");
        	if (IS_ERR(g_hAudDumpFifoTask)) {
        		LOG(LOG_CTRLF, TEXT("[AudmhlTaskInit]AudmhlTaskMain create fail \r\n"));
        		g_hAudDumpFifoTask = NULL;
        		return;
        	}
            else
            {
                struct sched_param param;
                s32 ret;

                param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
                ret = sched_setscheduler_nocheck(g_hAudDumpFifoTask, SCHED_RR, &param);
                ASSERT(ret == 0);
            }
        	wake_up_process(g_hAudDumpFifoTask); 
        }
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] DumpFifo Task is running! \r\n"));
    }
}


static void vAudDumpAoutThreadStart(u32 u4Chnum, s8* pName)
{
    if(!_bAudDumpAoutThreadStart)
    {        
        _bAudDumpAoutThreadStart = TRUE;
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] Start DumpFifo task! \r\n"));
        
        //Create dump afifo Task
        g_hAudDumpAoutTask = kthread_create(vAudDumpAoutTask, (void *)NULL, pName);
    	if (IS_ERR(g_hAudDumpAoutTask)) {
    		LOG(LOG_CTRLF, TEXT("[vAudDumpAoutThreadStart]vAudDumpAoutTask create fail \r\n"));
    		g_hAudDumpAoutTask = NULL;
    		return;
    	}
        else
        {
            struct sched_param param;
            s32 ret;

            param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
            ret = sched_setscheduler_nocheck(g_hAudDumpAoutTask, SCHED_RR, &param);
            ASSERT(ret == 0);
        }
        
    	wake_up_process(g_hAudDumpAoutTask); 
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] DumpFifo Task is running! \r\n"));
    }
}


/************************************************************************************************
 Function Name: u4AudDUMPAFIFOGetFIFOSAdr
 Function Description:Get the Afifo start addr
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
static u32 u4AudDUMPAFIFOGetFIFOSAdr(void* pUsr)
{
    u32 u4DecId = (u32)pUsr;
    return u4AudHalGetAFIFOStartAddr(u4DecId);
}

/************************************************************************************************
 Function Name: u4AudDUMPAFIFOGetFIFOEAdr
 Function Description:Get the Afifo end addr
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
static u32 u4AudDUMPAFIFOGetFIFOEAdr(void* pUsr)
{
    u32 u4DecId = (u32)pUsr;
    return u4AudHalGetAFIFOEndAddr(u4DecId);
}

/************************************************************************************************
 Function Name: u4AudDUMPAFIFOGetWPtr
 Function Description:Get the Afifo write pointer
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
 2011.3.7     modify format according coding standard
*************************************************************************************************/
static u32 u4AudDUMPAFIFOGetWPtr(void* pUsr)
{
    u32 u4DecId = (u32)pUsr;
    return u4AudHalGetAFIFOWPtr(u4DecId);
}

/************************************************************************************************
 Function Name: u4AudDUMPAOUTGetFIFOSAdr
 Function Description:Get the Aout start address
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.3.18
 Modify Data:
*************************************************************************************************/
static u32 u4AudDUMPAOUTGetFIFOSAdr(void* pUsr)
{
    u32 u4CmdId = (u32)pUsr;
    u32 u4AoutId =  (u4CmdId>>4) & 0x0F;
    u32 u4ChId = u4CmdId & 0x0F;
    u32 u4BaseAddr, u4Ch1SAdr, u4Ch1Size;
    u32 u4SAdr = 0;

    if (u4AoutId == 0)
    {
        // AOUT
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(5);
        u4Ch1SAdr = u4CliDbgReadDspSram(0, 0x240);
        u4Ch1Size = u4CliDbgReadDspSram(0, 0x241);
        u4SAdr = u4BaseAddr + ((u4Ch1SAdr+(u4Ch1Size*(u4ChId-1)))*4);
    }
    else if (u4AoutId == 1)
    {
        // AOUT2
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(7);
        u4Ch1SAdr = u4CliDbgReadDspSram(0, 0x2C0);
        u4Ch1Size = u4CliDbgReadDspSram(0, 0x2C1);
        u4SAdr = u4BaseAddr + ((u4Ch1SAdr+(u4Ch1Size*(u4ChId-1)))*4);
    }

    return u4SAdr;
}


/************************************************************************************************
 Function Name: u4AudDUMPAOUTGetFIFOEAdr
 Function Description:Get the Aout end address
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.3.18
 Modify Data:

*************************************************************************************************/
static u32 u4AudDUMPAOUTGetFIFOEAdr(void* pUsr)
{
    u32 u4CmdId = (u32)pUsr;
    u32 u4AoutId =(u4CmdId>>4) & 0x0F;
    u32 u4ChId = u4CmdId & 0x0F;
    u32 u4BaseAddr, u4Ch1SAdr, u4Ch1Size;
    u32 u4EAdr = 0;

    if (u4AoutId == 0)
    {
        // AOUT
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(5);
        u4Ch1SAdr = u4CliDbgReadDspSram(0, 0x240);
        u4Ch1Size = u4CliDbgReadDspSram(0, 0x241);
        u4EAdr = u4BaseAddr + ((u4Ch1SAdr+(u4Ch1Size*(u4ChId)))*4);
    }
    else if (u4AoutId == 1)
    {
        // AOUT2
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(7);
        u4Ch1SAdr = u4CliDbgReadDspSram(0, 0x2C0);
        u4Ch1Size = u4CliDbgReadDspSram(0, 0x2C1);
        u4EAdr = u4BaseAddr + ((u4Ch1SAdr+(u4Ch1Size*(u4ChId)))*4);
    }

    return u4EAdr;
}


/************************************************************************************************
 Function Name: u4AudDUMPAOUTGetRefPtr
 Function Description:Get the Aout RefPtr
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.3.18
 Modify Data:
*************************************************************************************************/
static u32 u4AudDUMPAOUTGetRefPtr(void* pUsr)
{
    u32 u4CmdId = (u32)pUsr;
    u32 u4AoutId = (u4CmdId>>4) & 0x0F;
    u32 u4ChId = u4CmdId & 0x0F;
    u32 u4BaseAddr, u4NSAdr, u4NSAdrId;
    u32 u4RPtr = 0;

    if (u4AoutId == 0)
    {
        // AOUT1
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(5);
        u4NSAdrId = 0x241 + u4ChId;
        u4NSAdr = u4CliDbgReadDspSram(0, u4NSAdrId);
        u4RPtr = u4BaseAddr + (u4NSAdr*4);
    }
    else if (u4AoutId == 1)
    {
        // AOUT2
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(7);
        u4NSAdrId = 0x2C1 + u4ChId;
        u4NSAdr = u4CliDbgReadDspSram(0, u4NSAdrId);
        u4RPtr = u4BaseAddr + (u4NSAdr*4);
    }

    return u4RPtr;
}


/************************************************************************************************
 Function Name: vAudDumpAfifoThreadStop
 Function Description:Stop the dump afifo thread
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
static void vAudDumpFifoThreadStop(void)
{
    _bAudDumpFifoThreadStart = FALSE;
    LOG(LOG_CTRLF, TEXT("[AudCliCmd] dump fifo thread stop! \r\n"));
    mdelay(200);
}


static void vAudDumpAoutThreadStop(void)
{
        
    _bAudDumpAoutThreadStart = FALSE;
    
    LOG(LOG_CTRLF, TEXT("[AudCliCmd] dump fifo thread stop! \r\n"));
    mdelay(200);
}


/************************************************************************************************
 Function Name: AudCliDumpAfifo
 Function Description:dump afifo main function
 input para:u4Cmdid:0:stop dump,1:start dump;filename:write dump afifo data to file
 output para:NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
 2011.3.7     modify format according coding standard
*************************************************************************************************/
void AudCliDumpAfifo(u32 u4Cmdid,const s8 ** filename)
{
    void *  hFs = INVALID_HANDLE_VALUE;
    AUD_ESM_CONTEXT_T* pContext = &g_rAudEsmContext[PRI_DEC];

    AUD_DUMP_FIFO tDumpAfifo;
    s8   szFilePath[64];
    tDumpAfifo.u4GetFIFOSAdr = u4AudDUMPAFIFOGetFIFOSAdr;
    tDumpAfifo.u4GetFIFOEAdr = u4AudDUMPAFIFOGetFIFOEAdr;
    tDumpAfifo.u4GetWPtr = u4AudDUMPAFIFOGetWPtr;

    if(9 == u4Cmdid)
    {
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] MAP m_afifo_info u4AfifoWPtr= 0x%x \r\n"), u4AudEsmGetWritePtr(pContext)); 
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] MAP m_afifo_info u4AfifoRPtr= 0x%x \r\n"), u4AudEsmGetReadPtr(pContext));       
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] u4AfifoWPtrForApp=0x%x\r\n"), pContext->u4AfifoWPtrForApp);
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] AFIFO HW wptr 0x5088= %08x \r\n"), u4AudDUMPAFIFOGetWPtr((void*)PRI_DEC));    
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] AFIFO RD rptr 0x50B0= %08x \r\n"), u4AudHalGetBufRPtr(PRI_DEC, DSP_AFIFO));       
       return;
    }

    if(10 == u4Cmdid)
    {
       pContext = &g_rAudEsmContext[SEC_DEC];
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] MAP m_afifo_info u4AfifoWPtr= 0x%x \r\n"), u4AudEsmGetWritePtr(pContext)); 
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] MAP m_afifo_info u4AfifoRPtr= 0x%x \r\n"), u4AudEsmGetReadPtr(pContext));  
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] u4AfifoWPtrForApp=0x%x\r\n"), pContext->u4AfifoWPtrForApp);
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] AFIFO HW wptr = %08x \r\n"), u4AudDUMPAFIFOGetWPtr((void*)SEC_DEC));
       LOG(LOG_CTRLF, TEXT("[AudCliCmd] AFIFO RD rptr = %08x \r\n"), u4AudHalGetBufRPtr(SEC_DEC, DSP_AFIFO)); ;        
       return;
    }
    
    if(11 == u4Cmdid)
    {
       u32 u4DecId = TER_DEC;
       pContext = &g_rAudEsmContext[TER_DEC];
       LOG(LOG_CTRLF, TEXT("MAP m_afifo_info u4AfifoWPtr= 0x%x \r\n"), u4AudEsmGetWritePtr(pContext)); 
       LOG(LOG_CTRLF, TEXT("MAP m_afifo_info u4AfifoRPtr= 0x%x \r\n"), u4AudEsmGetReadPtr(pContext));  
       LOG(LOG_CTRLF, TEXT("u4AfifoWPtrForApp=0x%x\r\n"), pContext->u4AfifoWPtrForApp);
       LOG(LOG_CTRLF, TEXT("AFIFO HW wptr = %08x \r\n"), u4AudDUMPAFIFOGetWPtr((void*)TER_DEC));
       LOG(LOG_CTRLF, TEXT("AFIFO RD rptr = %08x \r\n"), u4AudHalGetBufRPtr(TER_DEC, DSP_AFIFO));
       LOG(LOG_CTRLF, TEXT("SA = %08x, EA = %08x \r\n"),
        u4AudDUMPAFIFOGetFIFOSAdr((void*)u4DecId), u4AudDUMPAFIFOGetFIFOEAdr((void*)u4DecId)); 
       return;
    }

    LOG(LOG_CTRLF, TEXT("[AudCliCmd] u4Cmdid = %d \r\n"), u4Cmdid);
    LOG(LOG_CTRLF, TEXT("[AudCliCmd] filename = %s \r\n"), filename);

    if(0 == u4Cmdid)
    {
        vAudDumpFifoThreadStop();
    }
    else if ((1 == u4Cmdid) || (2 == u4Cmdid) || (3 == u4Cmdid))
    {
        u4Cmdid -= 1;

        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AFIFO u4Cmdid = %d \r\n"), u4Cmdid);
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AFIFO Start Addr = 0x%08x \r\n"), u4AudDUMPAFIFOGetFIFOSAdr((void*)u4Cmdid));
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AFIFO End Addr = 0x%08x \r\n"), u4AudDUMPAFIFOGetFIFOEAdr((void*)u4Cmdid));
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AFIFO wptr = 0x%08x \r\n"), u4AudDUMPAFIFOGetWPtr((void*)u4Cmdid));

        tDumpAfifo.pUsr = (void *)(u4Cmdid);

        x_snwprintf(szFilePath, sizeof(szFilePath), TEXT("%s%s"), TEXT("/"), filename);

        hFs = (void *)filp_open(szFilePath, O_RDWR | O_CREAT, 0);

        if(INVALID_HANDLE_VALUE != hFs)
        {
            tDumpAfifo.hFs = hFs;
            vAudDumpFifoThreadStart(&tDumpAfifo);
        }
     }
     else
     {
         AUD_CLI_DUMP_AFIFO_STR();
     }
}

/************************************************************************************************
 Function Name: AudCliDumpAout
 Function Description:dump aout buffer
 input para:u4Cmdid:decid,filename:dump value to file
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
void AudCliDumpAout(u32 u4Cmdid,const s8 ** filename)
{
    void *  hFs = INVALID_HANDLE_VALUE;

    s8   szFilePath[6][64]; 
	s32 i4ind = 0;


    if (0 == u4Cmdid)
    {
        vAudDumpAoutThreadStop();
    }
    else
    {
        g_rAudDumpAout.u4GetFIFOSAdr = u4AudDUMPAOUTGetFIFOSAdr;
        g_rAudDumpAout.u4GetFIFOEAdr = u4AudDUMPAOUTGetFIFOEAdr;
        g_rAudDumpAout.u4GetWPtr = u4AudDUMPAOUTGetRefPtr;
        g_rAudDumpAout.u4ChInd = 1;
        g_rAudDumpAout.hFs = NULL;

        LOG(LOG_CTRLF, TEXT("[AudCliCmd] filename: %s \r\n"), filename);
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AOUT u4Cmdid = 0x%x \r\n"), u4Cmdid);
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AOUT Start Addr = %08x \r\n"), u4AudDUMPAOUTGetFIFOSAdr((void*)u4Cmdid));
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AOUT End Addr = %08x \r\n"), u4AudDUMPAOUTGetFIFOEAdr((void*)u4Cmdid));

        x_snwprintf(szFilePath[0], sizeof(szFilePath), TEXT("%s%s_L.bin"), TEXT("/"), filename);        
        x_snwprintf(szFilePath[1], sizeof(szFilePath), TEXT("%s%s_C.bin"), TEXT("/"), filename);
        x_snwprintf(szFilePath[2], sizeof(szFilePath), TEXT("%s%s_R.bin"), TEXT("/"), filename);
        x_snwprintf(szFilePath[3], sizeof(szFilePath), TEXT("%s%s_Ls.bin"), TEXT("/"), filename);
        x_snwprintf(szFilePath[4], sizeof(szFilePath), TEXT("%s%s_Rs.bin"), TEXT("/"), filename);
        x_snwprintf(szFilePath[5], sizeof(szFilePath), TEXT("%s%s_Sub.bin"), TEXT("/"), filename);

        for(i4ind = 0; i4ind < AUD_AOUT_CHNUM; i4ind++)
        {
            hFs = (void *)filp_open(szFilePath[i4ind], O_RDWR | O_CREAT, 0);

            if(INVALID_HANDLE_VALUE != hFs)
            {
                g_hAudDumAoutFs[i4ind] = hFs;
            }
            else
            {
                LOG(LOG_CTRLF, TEXT("[AudCliCmd] Create file error. \r\n"));
            }
        }
        
        vAudDumpAoutThreadStart(0, "AudDumpFifoThread1");
        
    }
}
#else // #ifndef __linux__
#define WRITEBLOCK                          (4 * 1024)

#define GET_INODE_FROM_FILEP(filp)          (filp)->f_path.dentry->d_inode

static s8 g_Buff[WRITEBLOCK];   //every write 4 K
static s8   szFilePath_dump[AUD_AOUT_CHNUM][64];


/************************************************************************************************
  Function Name: android_readwrite_file
  Function Description:write function
  Author:zjian.zhou
  Data:2012.5.14
  Modify Data:
  
           2012.5.14  modify format according coding standard
 *************************************************************************************************/
static s32 write_file(const s8 *filename, const s8 *wbuf, u32 length)
{
    s32 re = 0;
    struct file *fp;
    mm_segment_t fs;
    fs = get_fs();
    set_fs(KERNEL_DS);
    
    fp = filp_open(filename, (O_RDWR | O_CREAT | O_APPEND), S_IRUSR);
    if (IS_ERR(fp) || !fp->f_op)
    {
        LOG(0, "file name is %s, filp_open error\n", filename);
        re = -ENOENT;       
    }
            
    if (wbuf)
    {
        if ((re = fp->f_op->write(fp, wbuf, length, &fp->f_pos)) < 0)
        {
            LOG(0, "Write %u bytes to file %s error %d\n", length, filename, re);            
        }
    }
     
    if (!IS_ERR(fp))
    {
        filp_close(fp, NULL);
    }
    set_fs(fs);
    return re;
}

/************************************************************************************************
 Function Name: i4DumpAFifoFsWriteFile
 Function Description:write afifo data was dumped  to file
 input para:hFs:file handle,pbBuf:buffer,u4Size:write size,pWriteSize:actual write size
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:2012.5.14
 Modify By:zjian.zhou
 2011.3.7     modify format according coding standard
 *************************************************************************************************/
static bool i4DumpFifoFsWriteFile(const s8 *filename, const void *pbBuf, u32 u4Size)
{
    s32 i;
    s32 u4count;
    s32 u4wrcount;
    s32 u4totalwr;
    s32 u4left;

    u4totalwr = 0;
    u4count = u4Size / (WRITEBLOCK);

    for (i = 0;i < u4count; i++)
    {
        memcpy(g_Buff, (s8 *)(pbBuf + i * WRITEBLOCK), WRITEBLOCK);
        u4wrcount = write_file(filename, g_Buff, WRITEBLOCK);
        u4totalwr += u4wrcount;
    }

    if (0 != (u4Size % (WRITEBLOCK)))
    {
        u4left = u4Size - u4count * WRITEBLOCK;
        memcpy(g_Buff, (s8 *)(pbBuf + u4count * WRITEBLOCK), u4left);
        u4wrcount = write_file(filename, g_Buff, u4left); 
        u4totalwr += u4wrcount;
    }
    
    return ((u4totalwr == u4Size) ? true : false);
}

/************************************************************************************************
  Function Name: vAudDumpAfifoTask
  Function Description:Dump Afifo Thread
  Author:fei.zhu
  Data:2010.12.12
  Modify Data:2012.5.14
  Modify By:zjian.zhou
  2011.3.7  modify format according coding standard
  *************************************************************************************************/
static s32 vAudDumpFifoTask(void* pvArg)
{
    bool i4Ret;
    bool bRunFlag = TRUE;
    bool bStartFlag = FALSE;
    u32 u4SAdr, u4EAdr, u4WPtr, u4CWPtr, u4FifoSize, u4Size;
    u32 u4TotalSz = 0;
    AUD_DUMP_FIFO *pDump = &tAudDumpFifo;
    
    LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] Enter DumpFifoThreadTask! \r\n"));

    u4SAdr = 0;
    u4EAdr = 0;
    u4FifoSize = 0;
    u4WPtr = pDump->u4GetFIFOSAdr(pDump->pUsr);
    u4CWPtr = 0;

    bRunFlag = _bAudDumpFifoThreadStart;

    while(bRunFlag)
    {
        if(_bAudDumpFifoThreadStart)
        {
            u4CWPtr = pDump->u4GetWPtr(pDump->pUsr);
        }
        if(u4CWPtr != u4WPtr)
        {
            if(!bStartFlag)
            {
                bStartFlag = TRUE;
                u4SAdr = pDump->u4GetFIFOSAdr(pDump->pUsr);
                u4EAdr = pDump->u4GetFIFOEAdr(pDump->pUsr);

                u4FifoSize = u4EAdr - u4SAdr;
            }
            // Write data to file
            if (u4CWPtr > u4WPtr)
            {
                u4Size = u4CWPtr - u4WPtr;
            }

            else
            {
                u4Size = u4FifoSize - (u4WPtr - u4CWPtr);
            }

            if(_bAudDumpFifoThreadStart)
            {
                u4Size &= 0xFFFFFF00;       // 256 alignment

                if (u4Size < (4*1024))
                {
                    // data < 4K, delay, wait for data
                    mdelay(50);
                    continue;
                }
             }

            if ((u4Size + u4WPtr) > u4EAdr)
            {
                u4Size = u4EAdr - u4WPtr;
            }
            LOG(LOG_CTRLF,TEXT("*****[_AudCliCmd] DUMPFIFO u4WPtr = 0x%x,u4CWPtr = 0x%x,u4FifoSize = 0x%x,u4Size = 0x%x,u4SAdr = 0x%x,u4EAdr = 0x%x\r\n"),
                          (u32)u4WPtr, (u32)u4CWPtr, (u32)u4FifoSize, (u32)u4Size, (u32)u4SAdr, (u32)u4EAdr);

            i4Ret = i4DumpFifoFsWriteFile(pDump->filename, (const void *)u4WPtr, u4Size);            
            if(i4Ret)
            {
                 u4TotalSz += (u32)u4Size;
                 LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DUMPFIFO write file successed u4TotalSz =  0x%x \r\n"), (u32)u4TotalSz);
            }
            else
            {
                LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DUMPAFIFO write file failed! exit loop *****\r\n"));
                break;
            }

            // update reference pointer
            u4WPtr += u4Size;
            if (u4WPtr >= u4EAdr)
            {
                u4WPtr = u4SAdr;
            }
        }

        if(!_bAudDumpFifoThreadStart)
        {
            bRunFlag = FALSE;
        }
        else
        {
            mdelay(100);
        }
    }
    LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DUMPFIFO total write file size =%d(k) \n"),(u32)u4TotalSz / 1024);
    LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DUMPFIFO Exit _bAudDumpAfifoThread(ZF)"));

    complete_and_exit(NULL, 0);
}

static s32 vAudDumpAoutTask(void* pvArg)
{
    bool i4Ret;
    bool bRunFlag = TRUE;
    u32 u4SAdr[AUD_AOUT_CHNUM], u4EAdr[AUD_AOUT_CHNUM], u4WPtr[AUD_AOUT_CHNUM];
    u32 u4CWPtr[AUD_AOUT_CHNUM], u4FifoSize[AUD_AOUT_CHNUM], u4Size[AUD_AOUT_CHNUM];
    u32 u4TotalSz[AUD_AOUT_CHNUM] = {0};
    s32 u4Ind;

    bRunFlag = _bAudDumpAoutThreadStart;

    for(u4Ind = 0; u4Ind <AUD_AOUT_CHNUM; u4Ind++)
    {
        u4SAdr[u4Ind] = g_rAudDumpAout.u4GetFIFOSAdr((void*)(u4Ind+1));
        u4EAdr[u4Ind] = g_rAudDumpAout.u4GetFIFOEAdr((void*)(u4Ind+1));
        u4FifoSize[u4Ind] = u4EAdr[u4Ind] - u4SAdr[u4Ind];
        u4WPtr[u4Ind] = g_rAudDumpAout.u4GetWPtr((void*)(u4Ind+1));//dump read pointer
    }

    while(bRunFlag)
    {
        if(_bAudDumpAoutThreadStart)
        {
            for(u4Ind = 0; u4Ind <AUD_AOUT_CHNUM; u4Ind++)
            {                
                u4CWPtr[u4Ind] = g_rAudDumpAout.u4GetWPtr((void*)(u4Ind+1));
            }
        }
        
        for(u4Ind = 0; u4Ind <AUD_AOUT_CHNUM; u4Ind++)
        {
            if(u4CWPtr[u4Ind] != u4WPtr[u4Ind])
            {
                // Write data to file
                if (u4CWPtr[u4Ind] > u4WPtr[u4Ind])
                {
                    u4Size[u4Ind] = u4CWPtr[u4Ind] - u4WPtr[u4Ind];
                }

                else
                {
                    u4Size[u4Ind] = u4FifoSize[u4Ind] - (u4WPtr[u4Ind] - u4CWPtr[u4Ind]);
                }

                if(_bAudDumpAoutThreadStart)
                {
                    u4Size[u4Ind] &= 0xFFFFFF00;       // 256 alignment

                    if (u4Size[u4Ind] < (4*1024))
                    {
                        // data < 4K, delay, wait for data
                        mdelay(50);
                        continue;
                    }
                 }

                if ((u4Size[u4Ind] + u4WPtr[u4Ind]) > u4EAdr[u4Ind])
                {
                    u4Size[u4Ind] = u4EAdr[u4Ind] - u4WPtr[u4Ind];
                }
                LOG(LOG_CTRLF,TEXT("zjian2*****[_AudCliCmd] DUMPFIFO u4WPtr = 0x%x,u4CWPtr = 0x%x,u4FifoSize = 0x%x,u4Size = 0x%x,u4SAdr = 0x%x,u4EAdr = 0x%x\r\n"),
                              u4WPtr[u4Ind], u4CWPtr[u4Ind], u4FifoSize[u4Ind], u4Size[u4Ind], u4SAdr[u4Ind], u4EAdr[u4Ind]);

                i4Ret = i4DumpFifoFsWriteFile(szFilePath_dump[u4Ind], (const void *)u4WPtr[u4Ind], u4Size[u4Ind]);            
                if(i4Ret)
                {
                     u4TotalSz[u4Ind] += u4Size[u4Ind];
                     LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DUMPFIFO write file successed u4TotalSz =  0x%x \r\n"), (u32)u4TotalSz[u4Ind]);
                }
                else
                {
                    LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DUMPAFIFO write file failed! exit loop *****\r\n"));
                    break;
                }

                // update reference pointer
                u4WPtr[u4Ind] += u4Size[u4Ind];
                if (u4WPtr[u4Ind] >= u4EAdr[u4Ind])
                {
                    u4WPtr[u4Ind] = u4SAdr[u4Ind];
                }
            }

        }
        
        if(!_bAudDumpAoutThreadStart)
        {
            bRunFlag = FALSE;
        }
        else
        {
            mdelay(100);
        }
    }
    
    complete_and_exit(NULL, 0);
}

/************************************************************************************************
 Function Name: vAudDumpAfifoThreadStart
 Function Description:create dump afifo Task
 input para: pAudDumpAfifoThread:dumo afifo struct
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
 *************************************************************************************************/
static void vAudDumpFifoThreadStart(AUD_DUMP_FIFO *pAudDumpFifoThread)
{
    if(!_bAudDumpFifoThreadStart)
    {
        if (pAudDumpFifoThread)
        {
            _bAudDumpFifoThreadStart = TRUE;
            x_memcpy(&tAudDumpFifo, pAudDumpFifoThread, sizeof(tAudDumpFifo));
            LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] start DumpFifo Task! \r\n"));

             // create dump afifo Task
            g_hAudDumpFifoTask = kthread_create(vAudDumpFifoTask, (void *)NULL, "AudDumpFifoThread");
            if (IS_ERR(g_hAudDumpFifoTask)) {
                LOG(LOG_CTRLF, TEXT("[AudmhlTaskInit]AudmhlTaskMain create fail \r\n"));
                g_hAudDumpFifoTask = NULL;
                return;
            }
            else
            {
                struct sched_param param;
                s32 ret;

                param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
                ret = sched_setscheduler_nocheck(g_hAudDumpFifoTask, SCHED_RR, &param);
                ASSERT(ret == 0);
            }
             
             wake_up_process(g_hAudDumpFifoTask); 
        }
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DumpFifo Task is running! \r\n"));
    }
}

static void vAudDumpAoutThreadStart(u32 u4Chnum, s8* pName)
{
    if(!_bAudDumpAoutThreadStart)
    {        
        _bAudDumpAoutThreadStart = TRUE;
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] Start DumpAout task! \r\n"));
        
        //Create dump afifo Task
        g_hAudDumpAoutTask = kthread_create(vAudDumpAoutTask, (void *)NULL, pName);
    	if (IS_ERR(g_hAudDumpAoutTask)) {
    		LOG(LOG_CTRLF, TEXT("[vAudDumpAoutThreadStart]vAudDumpAoutTask create fail \r\n"));
    		g_hAudDumpAoutTask = NULL;
    		return;
    	}
        else
        {
            struct sched_param param;
            s32 ret;
        
            param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
            ret = sched_setscheduler_nocheck(g_hAudDumpAoutTask, SCHED_RR, &param);
            ASSERT(ret == 0);
        }
        
    	wake_up_process(g_hAudDumpAoutTask); 
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] DumpAout Task is running! \r\n"));
    }
}

/************************************************************************************************
 Function Name: u4AudDUMPAFIFOGetFIFOSAdr
 Function Description:Get the Afifo start addr
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
 *************************************************************************************************/
static u32 u4AudDUMPAFIFOGetFIFOSAdr(void* pUsr)
{
    u32 u4DecId = (u32)pUsr;
    return u4AudHalGetAFIFOStartAddr(u4DecId);
}

/************************************************************************************************
 Function Name: u4AudDUMPAFIFOGetFIFOEAdr
 Function Description:Get the Afifo end addr
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
 *************************************************************************************************/
static u32 u4AudDUMPAFIFOGetFIFOEAdr(void* pUsr)
{
    u32 u4DecId = (u32)pUsr;
    return u4AudHalGetAFIFOEndAddr(u4DecId);
}

/************************************************************************************************
 Function Name: u4AudDUMPAFIFOGetWPtr
 Function Description:Get the Afifo write pointer
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
 *************************************************************************************************/
static u32 u4AudDUMPAFIFOGetWPtr(void* pUsr)
{
    u32 u4DecId = (u32)pUsr;
    return u4AudHalGetAFIFOWPtr(u4DecId);
}

/************************************************************************************************
 Function Name: u4AudDUMPAOUTGetFIFOSAdr
 Function Description:Get the Aout start address
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.3.18
 Modify Data:

 *************************************************************************************************/
static u32 u4AudDUMPAOUTGetFIFOSAdr(void* pUsr)
{
    u32 u4CmdId = (u32)pUsr;
    u32 u4AoutId =  (u4CmdId>>4) & 0x0F;
    u32 u4ChId = u4CmdId & 0x0F;
    u32 u4BaseAddr, u4Ch1SAdr, u4Ch1Size;
    u32 u4SAdr = 0;

    if (u4AoutId == 0)
    {
        // AOUT
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(5);
        u4Ch1SAdr = u4CliDbgReadDspSram(0, 0x240);
        u4Ch1Size = u4CliDbgReadDspSram(0, 0x241);
        u4SAdr = u4BaseAddr + ((u4Ch1SAdr+(u4Ch1Size*(u4ChId-1)))*4);
    }
    else if (u4AoutId == 1)
    {
        // AOUT2
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(7);
        u4Ch1SAdr = u4CliDbgReadDspSram(0, 0x2C0);
        u4Ch1Size = u4CliDbgReadDspSram(0, 0x2C1);
        u4SAdr = u4BaseAddr + ((u4Ch1SAdr+(u4Ch1Size*(u4ChId-1)))*4);
    }

    return u4SAdr;
}


/************************************************************************************************
 Function Name: u4AudDUMPAOUTGetFIFOEAdr
 Function Description:Get the Aout end address
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.3.18
 Modify Data:

 *************************************************************************************************/
static u32 u4AudDUMPAOUTGetFIFOEAdr(void* pUsr)
{
    u32 u4CmdId = (u32)pUsr;
    u32 u4AoutId =(u4CmdId>>4) & 0x0F;
    u32 u4ChId = u4CmdId & 0x0F;
    u32 u4BaseAddr, u4Ch1SAdr, u4Ch1Size;
    u32 u4EAdr = 0;

    if (u4AoutId == 0)
    {
        // AOUT
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(5);
        u4Ch1SAdr = u4CliDbgReadDspSram(0, 0x240);
        u4Ch1Size = u4CliDbgReadDspSram(0, 0x241);
        u4EAdr = u4BaseAddr + ((u4Ch1SAdr+(u4Ch1Size*(u4ChId)))*4);
    }
    else if (u4AoutId == 1)
    {
        // AOUT2
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(7);
        u4Ch1SAdr = u4CliDbgReadDspSram(0, 0x2C0);
        u4Ch1Size = u4CliDbgReadDspSram(0, 0x2C1);
        u4EAdr = u4BaseAddr + ((u4Ch1SAdr+(u4Ch1Size*(u4ChId)))*4);
    }

    return u4EAdr;
}


/************************************************************************************************
 Function Name: u4AudDUMPAOUTGetRefPtr
 Function Description:Get the Aout RefPtr
 input para:pUsr:decid
 Author:fei.zhu
 Data:2010.3.18
 Modify Data:

 *************************************************************************************************/
static u32 u4AudDUMPAOUTGetRefPtr(void* pUsr)
{
    u32 u4CmdId = (u32)pUsr;
    u32 u4AoutId = (u4CmdId>>4) & 0x0F;
    u32 u4ChId = u4CmdId & 0x0F;
    u32 u4BaseAddr, u4NSAdr, u4NSAdrId;
    u32 u4RPtr = 0;

    if (u4AoutId == 0)
    {
        // AOUT1
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(5);
        u4NSAdrId = 0x241 + u4ChId;
        u4NSAdr = u4CliDbgReadDspSram(0, u4NSAdrId);
        u4RPtr = u4BaseAddr + (u4NSAdr*4);
    }
    else if (u4AoutId == 1)
    {
        // AOUT2
        u4BaseAddr = u4AudHalGetDSPDataPageStartAddr(7);
        u4NSAdrId = 0x2C1 + u4ChId;
        u4NSAdr = u4CliDbgReadDspSram(0, u4NSAdrId);
        u4RPtr = u4BaseAddr + (u4NSAdr*4);
    }

    return u4RPtr;
}


/************************************************************************************************
 Function Name: vAudDumpAfifoThreadStop
 Function Description:Stop the dump afifo thread
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
 *************************************************************************************************/
static void vAudDumpFifoThreadStop(void)
{
    _bAudDumpFifoThreadStart = FALSE;
    LOG(LOG_CTRLF, TEXT("[AudCliCmd] dump fifo thread stop! \r\n"));
    mdelay(200);
}

static void vAudDumpAoutThreadStop(void)
{
        
    _bAudDumpAoutThreadStart = FALSE;
    
    LOG(LOG_CTRLF, TEXT("[AudCliCmd] dump aout thread stop! \r\n"));
    mdelay(200);
}


/************************************************************************************************
 Function Name: AudCliDumpAfifo
 Function Description:dump afifo main function
 input para:u4Cmdid:0:stop dump,1:start dump;filename:write dump afifo data to file
 output para:NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:2012.5.14
 Modify By:zjian.zhou
                       2011.3.7     modify format according coding standard
 *************************************************************************************************/
void AudCliDumpAfifo(u32 u4Cmdid,const s8 ** filename)
{
    AUD_DUMP_FIFO tDumpAfifo;
    AUD_ESM_CONTEXT_T* pContext = &g_rAudEsmContext[PRI_DEC];

    tDumpAfifo.u4GetFIFOSAdr = u4AudDUMPAFIFOGetFIFOSAdr;
    tDumpAfifo.u4GetFIFOEAdr = u4AudDUMPAFIFOGetFIFOEAdr;
    tDumpAfifo.u4GetWPtr = u4AudDUMPAFIFOGetWPtr;

    if(9 == u4Cmdid)
    {
       LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] MAP m_afifo_info u4AfifoWPtr= 0x%x \r\n"), (u32)u4AudEsmGetWritePtr(pContext));
       LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] MAP m_afifo_info u4AfifoRPtr= 0x%x \r\n"), (u32)u4AudEsmGetReadPtr(pContext));
       LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] u4AfifoWPtrForApp=0x%X\r\n"), (u32)pContext->u4AfifoWPtrForApp);
       LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] AFIFO HW wptr 0x5088= %08x \r\n"), (u32)u4AudDUMPAFIFOGetWPtr((void*)PRI_DEC));
       LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] AFIFO RD rptr 0x50B0= %08x \r\n"), (u32)u4AudHalGetBufRPtr(PRI_DEC, DSP_AFIFO));
       return;
    }

    LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] u4Cmdid = %d \r\n"), (s32)u4Cmdid);
    LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] filename = %s \r\n"), (s8*)filename);

    if (0 == u4Cmdid)
    {
        sprintf(szFilePath_dump[0],"%s%s", "//", (s8*)filename);
        tAudDumpFifo.filename = szFilePath_dump[0]; 
        vAudDumpFifoThreadStop();        
    }
    else if ((1 == u4Cmdid) || (2 == u4Cmdid) || (3 == u4Cmdid))
    {
        u4Cmdid -= 1;

        LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] AFIFO u4Cmdid = %d \r\n"), (s32)u4Cmdid);
        LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] AFIFO Start_Addr = %08x \r\n"), (u32)u4AudDUMPAFIFOGetFIFOSAdr((void*)u4Cmdid));
        LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] AFIFO End_Addr = %08x \r\n"), (u32)u4AudDUMPAFIFOGetFIFOEAdr((void*)u4Cmdid));
        LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] AFIFO wptr = %08x \r\n"), (u32)u4AudDUMPAFIFOGetWPtr((void*)u4Cmdid));
        
        sprintf(szFilePath_dump[0],"%s%s", "//", (s8*)filename);
        tDumpAfifo.filename = szFilePath_dump[0];        
        tDumpAfifo.pUsr = (void *)(u4Cmdid);
        vAudDumpFifoThreadStart(&tDumpAfifo);
     }
     else
     {
         AUD_CLI_DUMP_AFIFO_STR();
     }
}

/************************************************************************************************
 Function Name: AudCliDumpAout
 Function Description:dump aout buffer
 input para:u4Cmdid:decid,filename:dump value to file
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:2012.5.14
 Modify By:zjian.zhou
                       2011.3.7     modify format according coding standard
 *************************************************************************************************/

void AudCliDumpAout(u32 u4Cmdid,const s8 ** filename)
{
    LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] filename: %s \r\n"), (s8*)filename);

    if (0 == u4Cmdid)
    {
        vAudDumpAoutThreadStop();
    }
    else
    {
        g_rAudDumpAout.u4GetFIFOSAdr = u4AudDUMPAOUTGetFIFOSAdr;
        g_rAudDumpAout.u4GetFIFOEAdr = u4AudDUMPAOUTGetFIFOEAdr;
        g_rAudDumpAout.u4GetWPtr = u4AudDUMPAOUTGetRefPtr;
        g_rAudDumpAout.u4ChInd = 1;
        
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] filename: %s \r\n"), (s8*)filename);
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AOUT u4Cmdid = 0x%x \r\n"), u4Cmdid);
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AOUT Start Addr = %08x \r\n"), u4AudDUMPAOUTGetFIFOSAdr((void*)u4Cmdid));
        LOG(LOG_CTRLF, TEXT("[AudCliCmd] AOUT End Addr = %08x \r\n"), u4AudDUMPAOUTGetFIFOEAdr((void*)u4Cmdid));

        sprintf(szFilePath_dump[0],"%s%s_L.bin", "//data//", (s8*)filename);
        sprintf(szFilePath_dump[1],"%s%s_C.bin", "//data//", (s8*)filename);
        sprintf(szFilePath_dump[2],"%s%s_R.bin", "//data//", (s8*)filename);
        sprintf(szFilePath_dump[3],"%s%s_Ls.bin", "//data//", (s8*)filename);
        sprintf(szFilePath_dump[4],"%s%s_Rs.bin", "//data//", (s8*)filename);
        sprintf(szFilePath_dump[5],"%s%s_Sub.bin", "//data//", (s8*)filename);

        vAudDumpAoutThreadStart(0, "AudDumpFifoThread1");
        
    }
}
#endif // #ifndef __linux__


/************************************************************************************************
 Function Name: vAudSeCliCmd
 Function Description:Call one post Cli
 input para: post type
 Author:Tongfa.Luo
 Data:2012.02.14
 Modify Data:
                       
*************************************************************************************************/
static void vAudSeCliCmd(AUD_SE_TYPE_T rPostType)
{
    AUD_SE_CLICMD_T *rCliCmd = NULL;
    AUD_SE_CLICMD_T CliCmd;

    CliCmd.u1Type = rPostType;
    CliCmd.u1CliCmd = AUD_SE_CLI_ST_SUPER;
    rCliCmd = &CliCmd;
    
    fgAudSeProcessCLIOpCmd((void *)rCliCmd);
}


/************************************************************************************************
 Function Name: AudCliTest
 Function Description:Cli test function just a example
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
void AudCliTest(void)
{
    LOG(LOG_CTRLF, TEXT("[AudCliCmd] this is just a test for cli! \r\n"));
}


/************************************************************************************************
 Function Name: AudCliTest
 Function Description:Cli help function,list the all cli cmd
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
void AudCliHelp(void)
{
     LOG(LOG_CTRLF, TEXT("[AudCliCmd][HELP] CLI HELP \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [LOG]drv.aud.log [loglevel(0~9)]\r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [DUMPAFIFO->Start dump]:drv.aud.df 1 XXX.bin  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [DUMPAFIFO->Stopdump]:drv.aud.df 0 XXX.bin  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [ShowDspShareMemory]:drv.aud.dsp.sh [groupid]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [WirteDspShareMemory]:drv.aud.dsp.shw [groupid] [byteaddr][value][size=2] \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [ShowComDram]:drv.aud.dsp.cm [addr][len]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [checkdsp underrun]:drv.aud.ur [1]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [show dsp status]:drv.aud.dsp.q  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [show pts info]:drv.aud.dsp.pi  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [check dsp queue]:drv.aud.dsp.pl[decid] [number] \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [check pts update STC]:drv.aud.dsp.pu[decid] [number]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [show channel status]:drv.aud.dsp.iecregs  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [modify dsp comm dram]:drv.aud.dsp.wcm[addr][value]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [read dsp sram value]:drv.aud.dsp.r[0:a,1:b,2:c][addr]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [write dsp sram value]:drv.aud.dsp.w[0:a,1:b,2:c][addr][value]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [printAU]:drv.aud.pa [0(off) 1(on)]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [show audio status]:drv.aud.st  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [show afifo wptr and rptr]:drv.esm.st 1  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [control auido volume]:drv.aud.v[decid][value]  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [exchange aout1 and aout2]:drv.aud.digout exa(0:off,1:on)  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [exchange IEC2 to IEC1 output]:drv.aud.digout exi(0:off,1:on)  \r\n"));
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [switch uart to normal mode]: drv.uart2.i \r\n")); //(aud_cli_demo by mtk40292)
     LOG(LOG_CTRLF, TEXT("[AudCliCmd] [switch uart to TP mode]: drv.uart2.ui \r\n"));
}


/************************************************************************************************
 Function Name: AudCliCmdLog
 Function Description:modify log level,defalut 9 open all level
 inputpara:arg1:log level(0~9)
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_OPEN_LOG_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Turn on audio log [log_level, default: 9]!\n"), AUD_CLI_OPEN_LOG)
void AudCliCmdLog(u32 arg1)
{
    if(ARG_INIT_VAL == arg1)
    {
        arg1 = 9;
    }
    
    g_u4AudLogLevel= arg1;
    LOG(LOG_CTRLF, TEXT("[AudCliCmd] Turn on audio log, default level = %d\n"), (u32)g_u4AudLogLevel);
}


 /************************************************************************************************
 Function Name: AudCliCmdCheckCmd
 Function Description:check the cmd send from the mw
 inputpara:no
 Author:yucai.yang
 Data:2011.8.22
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_MW_CMD_STR() LOG(LOG_CLI, \
    TEXT("[CLI]%d: Check the flow cmd send from the mw. \n"), AUD_CLI_MW_CMD)
void AudCliCmdCheckCmd(void)
{
    u32 mwcmd;

    for(g_rCLICmd1.rdptr=0; g_rCLICmd1.rdptr < g_rCLICmd1.wrptr; g_rCLICmd1.rdptr++)
    {
        mwcmd = g_rCLICmd1.mw_cmd[g_rCLICmd1.rdptr];
        switch(mwcmd)
        {
        case  AUD_DEC_CTRL_RESET:
              LOG(LOG_CTRLF, TEXT(" AUD_DEC_CTRL_RESET = %d\n"), (u32)mwcmd);
              break;
         case AUD_DEC_CTRL_STOP:
              LOG(LOG_CTRLF, TEXT(" AUD_DEC_CTRL_STOP = %d\n"), (u32)mwcmd);
              break;
         case AUD_DEC_CTRL_PLAY:
              LOG(LOG_CTRLF, TEXT(" AUD_DEC_CTRL_PLAY = %d\n"), (u32)mwcmd);
              break;
         case AUD_DEC_CTRL_PLAY_SYNC:
              LOG(LOG_CTRLF, TEXT(" AUD_DEC_CTRL_PLAY_SYNC = %d\n"), (u32)mwcmd);
              break;
         case AUD_DEC_CTRL_FLUSH:
              LOG(LOG_CTRLF, TEXT(" AUD_DEC_CTRL_FLUSH = %d\n"), (u32)mwcmd);
              break;
         case AUD_DEC_CTRL_PAUSE:
              LOG(LOG_CTRLF, TEXT(" AUD_DEC_CTRL_PAUSE = %d\n"), (u32)mwcmd);
              break;
         case AUD_DEC_CTRL_RESUME:
              LOG(LOG_CTRLF, TEXT(" AUD_DEC_CTRL_RESUME = %d\n"), (u32)mwcmd);
              break;
        }
    }
}


/************************************************************************************************
 Function Name: vAudCliGetOutputVol
 Function Description: Get all ch output volume
 inputpara:none
 Author:tongfa
 Data:2012.2.21
 Modify Data:

*************************************************************************************************/
#define AUD_CLI_GET_OUTPUT_VOL_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Get All Channel Output Volume. \n"), AUD_CLI_GET_OUTPUT_VOL)
static void vAudCliGetOutputVol(void)
{
    AUD_OUTPUT_VOL rChOutputVol = {0};
    AUD_OUTPUT_VOL *prChVol = &rChOutputVol;
    vAudGetOutputVol(prChVol);
}


/************************************************************************************************
 Function Name: vAudCliLRMixing
 Function Description: Set LR mix
 inputpara: mix type
 Author:tongfa
 Data:2012.2.21
 Modify Data:

*************************************************************************************************/
#define AUD_CLI_LR_MIX_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: LR_MIX: [0: STEREO, 1: LEFT, 2: RIGHT]. \n"), AUD_CLI_LR_MIX)
static void vAudCliLRMixing(AUD_DEC_LRMIX_OUTPUT_T t_lrmix_mode)
{
    if(ARG_INIT_VAL == t_lrmix_mode)
    {
        AUD_CLI_LR_MIX_STR();
    }
    else
    {
        vAudLRMixing(t_lrmix_mode);
    }
}


/************************************************************************************************
 Function Name: AudCliPrintAu
 Function Description:printf afifo sa,ea and pts info
 input para:arg1:0:off,1:on
 output para: NULL
 Author:yucai.yang
 Data:2011.8.3
 Modify Data:

*************************************************************************************************/
#define AUD_CLI_PRINT_AU_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: PrintAu[0: close, other: open] \n"), AUD_CLI_PRINT_AU)
void AudCliPrintAu(u32 arg1)
{
    if(ARG_INIT_VAL == arg1)
    {
        AUD_CLI_PRINT_AU_STR();
    }
    else if (arg1)
    {
        g_fgPrintAU = TRUE;
        LOG(LOG_CTRLF, TEXT("[AUD]Open audio au info print\n"));
    }
    else
    {
        g_fgPrintAU = FALSE;
        LOG(LOG_CTRLF, TEXT("[AUD]Close audio au info print\n"));
    }
}


/************************************************************************************************
 Function Name: AudCliEsmState
 Function Description: Get ESM State
 inputpara: 
 Author:tongfa
 Data:2012.2.21
 Modify Data:

*************************************************************************************************/
#define AUD_CLI_ESM_STATE_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Get Audio ESM CONTEXT [AudDrvCompId] [0:All, 3:AFIFO ...]\n"), AUD_CLI_ESM_STATE)
void AudCliEsmState(u32 arg1, u32 arg2)
{
    AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[arg1];

    if(arg1 > 3) 
    {
        AUD_CLI_ESM_STATE_STR();
        return;
    } 

    if(arg2 == ARG_INIT_VAL || arg2 == 0)
    {
        arg2 = 0xFFFF;
    }
                 
    LOG(LOG_CLI, TEXT("======== [ESM_State]: ========\n"));    
    LOG(LOG_CLI, TEXT(">>>u2AudDrvCompId: 0x%x\n"), pContext->u2AudDrvCompId);
    
    if(arg2 & 0x1)
    {
        LOG(LOG_CLI, TEXT(">>>u4AfifoSA: 0x%x\n"), pContext->u4AfifoSA);
        LOG(LOG_CLI, TEXT(">>>u4AfifoEA: 0x%x\n"), pContext->u4AfifoEA);
    }
    if(arg2 & 0x2)
    {
        LOG(LOG_CLI, TEXT(">>>u4AfifoRPtr: 0x%x\n"), pContext->u4AfifoRPtr);
        LOG(LOG_CLI, TEXT(">>>u4AfifoWPtr: 0x%x\n"), pContext->u4AfifoWPtr);
        LOG(LOG_CLI, TEXT(">>>u4AfifoWPtrForApp: 0x%x\n"), pContext->u4AfifoWPtrForApp);
    }

    if(arg2 & 0x4)
    {
        LOG(LOG_CLI, TEXT(">>>u4ReadCnt: 0x%x\n"), pContext->u4ReadCnt);
        LOG(LOG_CLI, TEXT(">>>fgIsPlay: 0x%x\n"), pContext->fgIsPlay);
        LOG(LOG_CLI, TEXT(">>>fgFirstAUArrive: 0x%x\n"), pContext->fgFirstAUArrive);
    }
    
    if(arg2 & 0x8)
    {
        LOG(LOG_CLI, TEXT(">>>u4TotalPBBankCount: 0x%x\n\n"), pContext->u4TotalPBBankCount);
        LOG(LOG_CLI, TEXT(">>>u4TotalPBBankCount: 0x%x\n\n"), pContext->u4TotalPBFrameCount);
    }

    if(arg2 & 0x10000)  // not use now
    {
        LOG(LOG_CLI, TEXT(">>>u4PTSQRPtr: 0x%x\n"), pContext->u4PTSQRPtr);
        LOG(LOG_CLI, TEXT(">>>u4PTSQWPtr: 0x%x\n"), pContext->u4PTSQWPtr);
        LOG(LOG_CLI, TEXT(">>>u4PTSQStrAddr: 0x%x\n"), pContext->u4PTSQStrAddr);
        LOG(LOG_CLI, TEXT(">>>u4PTSQEndAddr: 0x%x\n"), pContext->u4PTSQEndAddr);
        
        LOG(LOG_CLI, TEXT(">>>u4IBCQRPtr: 0x%x\n\n"), pContext->u4IBCQRPtr);        
        LOG(LOG_CLI, TEXT(">>>u4IBCQWPtr: 0x%x\n"), pContext->u4IBCQWPtr);        
        LOG(LOG_CLI, TEXT(">>>u4IBCQStrAddr: 0x%x\n"), pContext->u4IBCQStrAddr);
        LOG(LOG_CLI, TEXT(">>>u4IBCQEndAddr: 0x%x\n\n"), pContext->u4IBCQEndAddr);
             
        LOG(LOG_CLI, TEXT(">>>eType: 0x%x\n"), pContext->eType);
        LOG(LOG_CLI, TEXT(">>>u4Handle: 0x%x\n\n"), pContext->u4Handle);

        LOG(LOG_CLI, TEXT(">>>u4LastUpdatedRptr: 0x%x\n"), pContext->u4LastUpdatedRptr);
        LOG(LOG_CLI, TEXT(">>>u4LastIteratedAUIdx: 0x%x\n"), pContext->u4LastIteratedAUIdx);
        LOG(LOG_CLI, TEXT(">>>u4Read: 0x%x\n\n"), pContext->u4Read);
        LOG(LOG_CLI, TEXT(">>>u4CurWrIdx: 0x%x\n"), pContext->u4CurWrIdx);
    }
    
    LOG(LOG_CLI, TEXT("=================================\n"));
}


void vCommDramDump(u32 u4Address, u32 u4Len)
{
    u32 u4Idx = 0;
    LOG(LOG_FEATURE, TEXT("\n"));
    for (u4Idx = 0; u4Idx < u4Len; u4Idx += 4)
    {
        LOG(LOG_FEATURE, TEXT("*****0x%04x | %08x %08x %08x %08x \n"),
            u4Address + u4Idx,
            dReadDspCommDram(u4Address + u4Idx + 0),
            dReadDspCommDram(u4Address + u4Idx + 1),
            dReadDspCommDram(u4Address + u4Idx + 2),
            dReadDspCommDram(u4Address + u4Idx + 3));
    }
}
/************************************************************************************************
 Function Name: AudCliComDram
 Function Description:Read comm dram value
 input para:arg1: SrcAddr,arg2:Length
 output para:NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
             2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_R_CDRAM_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Read Comm Dram [addr][len] \n"), AUD_CLI_R_CDRAM)
void AudCliComDram(u32 arg1,u32 arg2)
{
    if(0 == arg2 || ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2)
    {
        AUD_CLI_R_CDRAM_STR();
    } 
    else 
    {
        u32 u4SrcAddr = arg1;
        u32 u4Len = arg2;
        LOG(LOG_CLI, TEXT("[CLI] Read Comm Dram: \n"));
        vCommDramDump(u4SrcAddr, u4Len);
    }    
}

/************************************************************************************************
 Function Name: AudCliDumpDspDram
 Function Description:Read comm dram value
 input para:arg1: SrcAddr,arg2:Length
 output para:NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_DUMP_DRAM_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Dump DSP Dram [page][addr][len][filename] \n"), AUD_CLI_DUMP_DRAM)
void AudCliDumpDspDram(u32 arg1,u32 arg2,u32 arg3,const s8 ** filename)
{
#ifndef __linux__
    void *  hFs = INVALID_HANDLE_VALUE;
    u32 u4Idx, dwWriteSize,u4WPtr,i4Ret;    
    s8  szFilePath[64];
    u32 u4PageId = arg1;
    u32 u4SrcAddr = arg2;
    u32 u4Len = arg3;    

    if(0 == arg3 || ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2|| ARG_INIT_VAL == arg3)
    {
        AUD_CLI_DUMP_DRAM_STR();
        return;
    } 
    
    u4WPtr = u4AudHalGetDSPDataPageStartAddr(u4PageId) + (u4SrcAddr<<2);

    LOG(LOG_CTRLF, TEXT("*****[_Aud Cli Cmd] Dump DSP DRAM \r\n"));

    x_snwprintf(szFilePath, sizeof(szFilePath), TEXT("%s%s"), TEXT("/"), filename);

    hFs = (void *)filp_open(szFilePath, O_RDWR | O_CREAT, 0);

    if(INVALID_HANDLE_VALUE == hFs)
    {
        filp_close((struct file *)hFs, NULL);
    }    
    
    if(0 == u4Len)
    {
        LOG(LOG_CTRLF, TEXT("*****Usage:dram [page] [addr] [len]\n"));
        return;
    }

    for (u4Idx = 0; u4Idx < u4Len; u4Idx += 4)
    {
        LOG(LOG_CTRLF, TEXT("*****0x%04x | %08x %08x %08x %08x \n"),
                    u4SrcAddr + u4Idx,
                    dReadDspDram(u4PageId, u4SrcAddr + u4Idx + 0),
                    dReadDspDram(u4PageId, u4SrcAddr + u4Idx + 1),
                    dReadDspDram(u4PageId, u4SrcAddr + u4Idx + 2),
                    dReadDspDram(u4PageId, u4SrcAddr + u4Idx + 3));        
    }

    i4Ret = i4DumpFifoFsWriteFile(hFs,(void *)u4WPtr, u4Len, &dwWriteSize, NULL);
    
    if(dwWriteSize>0)
    {
         LOG(LOG_CTRLF, TEXT("[AudCliCmd] DUMP DRAM wirte file successed u4TotalSz =  0x%x \r\n"),
            dwWriteSize);
    }
    
    if (dwWriteSize != u4Len)
    {
        LOG(LOG_CTRLF, TEXT("*[AudCliCmd] DUMP DRAM wirte file failed! Len=[%d]--dwWriteSize=[%d] \r\n"),
            u4Len,dwWriteSize);
    }    
        
    vDumpFifoCloseFile(hFs); 

#else
    u32 u4Idx, u4WPtr; 
    s8  szFilePath[64];
    u32 u4PageId = arg1;
    u32 u4SrcAddr = arg2;
    u32 u4Len = arg3;  

    if(0 == arg3 || ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2|| ARG_INIT_VAL == arg3)
    {
        AUD_CLI_DUMP_DRAM_STR();
        return;
    } 
         
    u4WPtr = u4AudHalGetDSPDataPageStartAddr(u4PageId) + (u4SrcAddr<<2);
       
    LOG(LOG_CTRLF, TEXT("*****[_Aud Cli Cmd] Dump DSP DRAM \r\n"));

    if(0 == u4Len)
    {
        LOG(LOG_CTRLF, TEXT("*****Usage:dram [page] [addr] [len]\n"));
        return;
    }
    
    if((filename == NULL)||(*filename == NULL))
    {
        for (u4Idx = 0; u4Idx < u4Len; u4Idx += 4)
        {
            LOG(LOG_CTRLF, TEXT("*****0x%04x | %08x %08x %08x %08x \n"),
                        (u32)(u4SrcAddr + u4Idx),
                        (u32)dReadDspDram(u4PageId, u4SrcAddr + u4Idx + 0),
                        (u32)dReadDspDram(u4PageId, u4SrcAddr + u4Idx + 1),
                        (u32)dReadDspDram(u4PageId, u4SrcAddr + u4Idx + 2),
                        (u32)dReadDspDram(u4PageId, u4SrcAddr + u4Idx + 3));        
        }
        return;
    }
    
    u4Len = u4Len << 2; // dword -> bytes
    sprintf(szFilePath,"%s%s", "//data//", (s8*)filename);
    if(i4DumpFifoFsWriteFile(szFilePath, (const void *)u4WPtr, u4Len))
    {
        LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DUMPBUFFER write file success! *****\r\n"));
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("*****[_AudCliCmd] DUMPBUFFER write file failed! *****\r\n"));
    }    
#endif
    return;
}


/************************************************************************************************
 Function Name: AudCliShowAudWorkingBuf
 Function Description:Read comm dram value
 input para:arg1: SrcAddr,arg2:Length
 output para:NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
extern u8 *g_ucAdspWorkingBuffer;
#define AUD_CLI_WORKINGBUF_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Show audio work buffer address. \n"), AUD_CLI_AUD_WORKINGBUF)
void AudCliShowAudWorkingBuf(u32 arg1,u32 arg2)
{
    if(0 == arg1)
    {
        u32 u4BufTmp = (u32)g_ucAdspWorkingBuffer;
        LOG(LOG_FEATURE, _T("Audio working Buffer = 0x%x\r\n"), u4BufTmp);
    }
    
    
    return;
}

void vShareInforDump(u8 u1Group)
{
    s32 i4Index;

    LOG(LOG_FEATURE, TEXT("\n"));
    LOG(LOG_FEATURE, TEXT("  +---- Group %2d ----+\n"), u1Group);

    LOG(LOG_FEATURE, TEXT("  +----+--3--2--1--0-+\n"));
    for (i4Index = 0; i4Index < 0x20; i4Index+=4)
    {
        LOG(LOG_FEATURE, TEXT("   0x%02x| %02x %02x %02x %02x \n"),
            i4Index,
            uReadDspShmBYTE(u1Group*32 + i4Index+3),
            uReadDspShmBYTE(u1Group*32 + i4Index+2),
            uReadDspShmBYTE(u1Group*32 + i4Index+1),
            uReadDspShmBYTE(u1Group*32 + i4Index));
    }

    LOG(LOG_FEATURE, TEXT("  +----+-------------+\n"));
}

/************************************************************************************************
 Function Name: AudCliDspShareInfo
 Function Description:Read Dsp share info value
 input para:arg1: GroupId
 output para:NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_R_SHM_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Read Share Info [GroupId] \n"), AUD_CLI_R_SHM)
void AudCliDspShareInfo(u32 arg1)
{
    if(ARG_INIT_VAL == arg1)
    {
        AUD_CLI_R_SHM_STR();
    } 
    else
    {
        u32 u4GroupId = arg1;
        LOG(LOG_FEATURE, TEXT("AudCliDspShareInfo u4GroupId = %d \r\n"),u4GroupId);
        vShareInforDump(u4GroupId);
    }
 }

/************************************************************************************************
 Function Name: AudCliDspShareInfoWrite
 Function Description:write Dsp share info value
 input para:arg1: GroupId,arg2:ByteAddr,arg3:value,arg4:size
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_W_SHM_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Write Share Info [GroupId][ByteAddr][Value] \n"), AUD_CLI_W_SHM)
void  AudCliDspShareInfoWrite(u32 arg1,u32 arg2,u32 arg3,u32 arg4)
{
    if(ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2 || ARG_INIT_VAL == arg3)
    {
        AUD_CLI_W_SHM_STR();
    }
    else    
    {

        u32 u4GroupId = arg1;
        u32 u4ByteAddr = arg2;
        u32 u4Value = arg3;
        //u32 u4Size = arg4;
        vWriteDspShmWORD(u4GroupId * 32 + u4ByteAddr, u4Value);
    }
}

/************************************************************************************************
 Function Name: AudCliDrvRead
 Function Description: Read Drv Info
 inputpara: 
 Author:tongfa
 Data:2012.2.21
 Modify Data:

*************************************************************************************************/
#define AUD_CLI_R_DRV_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Read Drv Info(Need to add...) \n"), AUD_CLI_R_DRV)
void  AudCliDrvRead(u32 arg1,u32 arg2)
{
    AUD_CLI_R_DRV_STR();
}


/************************************************************************************************
 Function Name: AudCliDrvWrite
 Function Description: Write Drv Info
 inputpara: 
 Author:tongfa
 Data:2012.2.21
 Modify Data:

*************************************************************************************************/
#define AUD_CLI_W_DRV_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Write Drv Info(Need to add...) \n"), AUD_CLI_W_DRV)
void  AudCliDrvWrite(u32 arg1,u32 arg2)
{
    AUD_CLI_W_DRV_STR();
}


/************************************************************************************************
 Function Name: AudCliUnderRunCounter
 Function Description:check dsp underrun
 input para:arg1: 0:OFF 1:ON
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_UNDERRUN_STR()         LOG(LOG_CLI, \
    TEXT("[CLI]%d: Check DSP Underrun [0: off, 1: on] (Arg must 0 or 1!). \n"), AUD_CLI_UNDERRUN)
void AudCliUnderRunCounter(u32 arg1)
{
    switch(arg1)
    {
    case 0:
        LOG(LOG_CLI, TEXT("Check Dsp Underrun OFF!\n"));
        DspSetUnderRun(FALSE);
        vCommDramDump(0xe1,1);
        vCommDramDump(0xe3,1);
        break;

    case 1:
        LOG(LOG_CLI, TEXT("Check Dsp Underrun ON!\n"));
        DspSetUnderRun(TRUE);
        vCommDramDump(0xe1,1);
        vCommDramDump(0xe3,1);
        break;

    default:
        AUD_CLI_UNDERRUN_STR();
        break;
    }
}

/************************************************************************************************
 Function Name: AudCliPlaybackTime
 Function Description:Get playback time
 input para:NULL
 output para: NULL
 Author:Jingjian.Yu
 Data:2012.2.22
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_PLAYBACK_TIME_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Aud DspPlayBackTime \n"), AUD_CLI_PLAYBACK_TIME)
void AudCliPlaybackTime(void)
{
    u32 u4Data;
    PBINF_A tAudPbInfo;
    AUD_GetPbInfo(PRI_DEC,&tAudPbInfo);
    u4Data = tAudPbInfo.u8DspPlayBackTime;
    
    LOG(0, TEXT("Aud DspPlayBackTime = %dMin %dS %dmS\n"),(u4Data/60000),((u4Data%60000)/1000),((u4Data%60000)%1000));  
}

/************************************************************************************************
 Function Name: AudCliMhlTest
 Function Description:mhl audio debug testing
 input para:arg1:stream type
 output para: NULL
 Author:zhongjie.su
 Data:2014.5.15
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_MHL_TEST_STR()     LOG(LOG_CLI,         \
    TEXT("[CLI]%d: MHL AUDIO CLI:  \n")                     \
    TEXT("               Set STOP     [0]\n")                            \
    TEXT("               Set PLAY     [1][0: PCM 2ch, 1: PCM 6ch, 2: AC3]\n")  \
    TEXT("               note: MW should be open before. \n"), AUD_CLI_MHL_TEST)
void AudCliMhlTest(u32 arg1, u32 arg2)
{
    if(ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2)
    {
        AUD_CLI_MHL_TEST_STR();
    }
    else
    {
        u32 u4OnOff = arg1;
        u32 u4StrType = arg2;

        LOG(LOG_CTRLF, TEXT("u4OnOff = %d, u4StrType = %d\n"), (u32)u4OnOff, (u32)u4StrType);
        
        #ifdef HDMI_AUDIO_IN_SW_DEBUG
        LOG(LOG_CTRLF, TEXT("Run vAudMhlDbgTst() \n"));
        vAudMhlDbgTst(u4OnOff, u4StrType);
        #endif
    }
}

/************************************************************************************************
 Function Name: AudCliGetPtsQueue
 Function Description:check if pts queue ok?
 input para:arg1:decid,arg2:index<0x80
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_GET_PTS_QUEUE_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Get PTS Queue[decId][index: <0x80]\n"), AUD_CLI_GET_PTS_QUEUE)
void AudCliGetPtsQueue(u32 arg1,u32 arg2)
{
    if(ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2)
    {
        AUD_CLI_GET_PTS_QUEUE_STR();
    }
    else
    {
        //u32 u1DecId = arg1;
        u32 i,index = arg2;

        LOG(LOG_CTRLF, TEXT(" Dram addr,  aput_bank,PTS PRSP,PTS High,PTS LOW,PTS\n"));
        for (i = 0; i < index; i++)
        {
            //vDspCLIGetPtsLegalQueue(u1DecId, index);
        }
    }
}

/************************************************************************************************
 Function Name: AudCliShowDspStatus
 Function Description:show dsp status
 input para:NULL
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_SHOW_DSP_STATUS_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Get DSP Sram variable(connect, aout, aput...)\n"), AUD_CLI_SHOW_DSP_STATUS)
void AudCliShowDspStatus(void)
{
    AudShowDspStatus();
}

/************************************************************************************************
 Function Name: AudCliShowConfig
 Function Description:show dsp src parm and output para
 input para:NULL
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_SHOW_CONFIG_STR()        LOG(LOG_OTHER, \
    TEXT("[CLI]%d: Show dsp source param and output param. \n"), AUD_CLI_SHOW_CONFIG)
void AudCliShowConfig(void)
{
     AudShowConfig();
}

/************************************************************************************************
 Function Name: AudCliDispIECRegisters
 Function Description:show digital output register channel status,except hdmi to pcm
 input para:NULL
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_SET_IEC_REG_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Show digital output register channel status,except hdmi to pcm.\n"), \
    AUD_CLI_SET_IEC_REG)
void AudCliDispIECRegisters(void)
{
    AudDispIECRegisters();
}

static void AudCmdCLIGetPtsInfo(void)
{
    AUD_DSP_PTS_INF rInfo;
	vDspCLIGetPtsInfo(&rInfo);

    LOG(0, TEXT("Aud DSP UPDATED STC = 0x%X  0x%X\n"),
    rInfo.u4UpdatePtsStcH, rInfo.u4UpdatePtsStcL);
    LOG(0, TEXT("Aud FIRST PTS of Bitstream  = 0x%X  0x%X\n"),
		rInfo.u4FirstAudPtsStcH, rInfo.u4FirstAudPtsStcL);
    LOG(0, TEXT("Aud START PTS from sync ctrl  = 0x%X  0x%X\n"),
		rInfo.u4StartPtsStcH, rInfo.u4StartPtsStcL);
    LOG(0, TEXT("Aud END PTS from sync ctrl  = 0x%X ,0x%X\n"),
        rInfo.u4EndPtsStcH, rInfo.u4EndPtsStcL);

    LOG(0, TEXT("Aud internal primary pts = 0x%X  0x%X\n"),
		rInfo.u4PrimaryPtsH, rInfo.u4PrimaryPtsL);
}

/************************************************************************************************
 Function Name: AudCliDspCLIGetPtsInfo
 Function Description:show pts info include STC,first pts
 input para:NULL
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_GET_PTS_INFO_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Get PTS Info include STC,first pts... \n"), AUD_CLI_GET_PTS_INFO)
void AudCliDspCLIGetPtsInfo(void)
{
    AudCmdCLIGetPtsInfo();
}

/************************************************************************************************
 Function Name: AudCliPtsUpdateQueue
 Function Description:check if dsp update STC is correct
 input para:arg1:decid,arg2:index
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_UPDATE_PTS_QUEUE_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Get PTS Update Queue[DecId][index] \n"), AUD_CLI_UPDATE_PTS_QUEUE)
void AudCliPtsUpdateQueue(u32 arg1,u32 arg2)
{
    if(ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2)
    {
        AUD_CLI_UPDATE_PTS_QUEUE_STR();
    }
    else 
    {
        //u32 u4DecId = arg1;
        u32 i,index = arg2;

        LOG(LOG_CTRLF, TEXT(" Dram addr,  PTS High,PTS Low, STC High,STC Low,PTS\n"));
        for (i = 0; i < index; i++)
        {
            //vDspCLIGetPtsUpdateQueue(u4DecId,i);
        }
    }
}

/************************************************************************************************
 Function Name: AudCliAdjustMiracast
 Function Description: Set adjust miracast parameter
 input para:arg1:parameter,arg2:value
 output para: NULL
 Author:
 Data:2013.10.09
 Modify Data:
*************************************************************************************************/
#define AUD_CLI_SET_ADJUSTMIRACAST_STR()  LOG(LOG_CLI,             \
    TEXT("[CLI]%d: Set adjust miracast parameter:[Para][Value]:\r\n") \
    TEXT("[para]0: Show value,1: HighThrehold,2:LowThrehold,3: AdjustScale, 4: Sleep Time,5:on/off.\n")\
    , AUD_CLI_SET_ADJUSTMIRACAST)
void AudCliAdjustMiracast(u32 arg1,u32 arg2)
{
    //const u32 u4Scale = 0x218DE;
    if(ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2)
    {
        AUD_CLI_SET_ADJUSTMIRACAST_STR();
    }
    else 
    {
        if(0 == arg1)
        {
            LOG(LOG_CLI, TEXT(" HighThrehold = %d.\r\n"), (u32)(i8Mira_GetHighThreshold()));
            LOG(LOG_CLI, TEXT(" LowThrehold = %d.\r\n"), (u32)i8Mira_GetLowThreshold()); 
            LOG(LOG_CLI, TEXT(" AdjustScale = %d.\r\n"), i2Mira_GetApllScale());
            LOG(LOG_CLI, TEXT(" sleep time = %d.\r\n"), u2Mira_GetSleepTime());
        }    
        else if(1 == arg1) // set HighThrehold 
        {          
            vMira_SetHighThreshold(arg2);
        }
        else if(2 == arg1)   //Set LowThrehold   
        {  
            vMira_SetLowThreshold(arg2); 
        }
        else if(3 == arg1)  //adjust sacle
        {
             vMira_SetApllScale(arg2);
        }
        else if(4 == arg1)
        {
            vMira_SetSleepTime(arg2);
        }
        else if(5 == arg1)
        {
            AUD_MIRACAST_CTRL_T eMiracastCtrl = AUD_MIRACAST_OFF;
            if(arg2>0)
            {
                eMiracastCtrl = AUD_MIRACAST_ON;

            }        
            vAudDrvIf_SetMiracastOnOff(eMiracastCtrl);
        }
    }
}

/************************************************************************************************
 Function Name: AudCliDspCommDramWrite
 Function Description:modify DSP comm dram value
 input para:arg1:addr,arg2:value
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_W_CDRAM_STR() LOG(LOG_CLI, \
    TEXT("[CLI]%d: Write Comm Dram [addr][value] \n"), AUD_CLI_W_CDRAM)
void AudCliDspCommDramWrite(u32 arg1,u32 arg2)
{
    if(ARG_INIT_VAL == arg1)
    {
        AUD_CLI_W_CDRAM_STR();
    }
    else
    {
        vWriteCommDram(arg1,arg2);
    }
}

/************************************************************************************************
 Function Name: AudCliDspReadSram
 Function Description:read dspa,b,c sram value
 input para:arg1:dspid,arg2:addr
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_R_SRAM_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Read Dsp Sram [DspId][addr][len] \n"), AUD_CLI_R_SRAM)
void AudCliDspReadSram(u32 arg1,u32 arg2,u32 arg3)
{
    if(ARG_INIT_VAL ==    arg1 || ARG_INIT_VAL ==  arg2)
    {
        AUD_CLI_R_SRAM_STR();
    }
    else
    {
        u8 u1DspId = (u8)arg1;
        u32 u4Addr = arg2;    
        u32 u4Len = arg3;
        u32 u4Idx;

        if(ARG_INIT_VAL == arg3 || 0 == arg3 || 1 == arg3)
        {
            LOG(LOG_CTRLF, TEXT("SRAM(addr,data) = (0x%x,0x%x)\r\n"),
                u4Addr,u4CliDbgReadDspSram(u1DspId,u4Addr));
        }
        else
        {
            LOG(LOG_CTRLF, TEXT("[CLI] Read Dsp Sram: \r\n"));
            for (u4Idx = 0; u4Idx < u4Len; u4Idx += 4)
            {
                LOG(LOG_CTRLF, TEXT("*****0x%04x | %08x %08x %08x %08x \n"),
                        (u32)(u4Addr + u4Idx),
                        (u32)u4CliDbgReadDspSram(u1DspId,u4Addr+u4Idx+0),
                        (u32)u4CliDbgReadDspSram(u1DspId,u4Addr+u4Idx+1),
                        (u32)u4CliDbgReadDspSram(u1DspId,u4Addr+u4Idx+2),
                        (u32)u4CliDbgReadDspSram(u1DspId,u4Addr+u4Idx+3));
            }
        }            
    }
}

/************************************************************************************************
 Function Name: AudCliDspWriteSram
 Function Description:write dspa,b,c sram value
 input para:arg1:dspid,arg2:addr,arg3:value
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_W_SRAM_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Write Dsp Sram [DspId][addr][value] \n"), AUD_CLI_W_SRAM)
void AudCliDspWriteSram(u32 arg1,u32 arg2,u32 arg3)
{
    if(ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2 || ARG_INIT_VAL == arg3)
    {
        AUD_CLI_W_SRAM_STR();
    }
    else
    {
        u32 u1DspId = arg1;
        u32 u4Addr = arg2;
        u32 u4Data = arg3;
        vCliDbgWriteDspSram(u1DspId, u4Addr, u4Data);
    }
}

/************************************************************************************************
 Function Name: AudCliDispStates
 Function Description:check if audio status is ok?
 input para:NULL
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_CHECK_DSP_STATE_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Get Audio DRV and DSP State! \n"), AUD_CLI_CHECK_DSP_STATE)
void AudCliDispStates()
{
    AudDispStates();
}

/************************************************************************************************
 Function Name: AudCliCmdUopVolume
 Function Description:modify audio volume
 input para:arg1:decid,arg2:volume value(0~100)
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_MODIFY_VOL_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Modify D_VOL by UOP [DecId][volume: 0 ~ 100]. \n"), AUD_CLI_MODIFY_VOL)
void  AudCliCmdUopVolume(u32 arg1,u32 arg2)
{ 
    if(ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2)
    {
        AUD_CLI_MODIFY_VOL_STR();
    }
    else
    {
        u32 u1Volume = arg2;
        vAdspMasterVolume(u1Volume);
    }
}

extern void AudSetDetectVolThr(AUD_THRESHOLD_T rAudThrshld);
#define AUD_CLI_VOL_DETECT_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Modify Detect Vol Threshold [FrontThreshold][RearThreshold][WaveformThreshold]. \n"), AUD_CLI_VOL_DETECT)
void  AudCliCmdDetectVolThreshold(u32 arg1,u32 arg2,u32 arg3)
{
    AUD_THRESHOLD_T rAudThrshld = {0};
    if(ARG_INIT_VAL == arg1 || ARG_INIT_VAL == arg2 || ARG_INIT_VAL == arg3)
    {
        AUD_CLI_VOL_DETECT_STR();
    }
    else
    {
        rAudThrshld.u4FrontThrshld = arg1;
        rAudThrshld.u4RearThrshld = arg2;
        rAudThrshld.u4WaveFormThrshld = arg3;
        AudSetDetectVolThr(rAudThrshld);
    }
}

/************************************************************************************************
 Function Name: AudCliSetFrontAout
 Function Description: 
 input para: 
 output para:  
 Author: 
 Data: 
 Modify Data:
*************************************************************************************************/
#define AUD_CLI_SET_FAOUT_STR()    LOG(LOG_OTHER, \
    TEXT("[CLI]%d: Set front aout(need to add...). \n"), AUD_CLI_SET_FAOUT)
void AudCliSetFrontAout(void)
{
    AUD_CLI_SET_FAOUT_STR();
}


/************************************************************************************************
 Function Name: AudCliSetRearAout
 Function Description: 
 input para: 
 output para:  
 Author: 
 Data: 
 Modify Data:
                        
*************************************************************************************************/
#define AUD_CLI_SET_RAOUT_STR()    LOG(LOG_OTHER, \
    TEXT("[CLI]%d: Set rear aout(need to add...). \n"), AUD_CLI_SET_RAOUT)
void AudCliSetRearAout(void)
{
    AUD_CLI_SET_RAOUT_STR();
}

/************************************************************************************************
 Function Name: AudCliGetAoutSt(u32 arg1, u32 arg2)
 Function Description: 
 input para: 
 output para:  
 Author: 
 Data: 
 Modify Data:
                        
*************************************************************************************************/
#define AUD_CLI_AOUT_STATE_STR()    LOG(LOG_OTHER, \
    TEXT("[CLI]%d: Get aout state, [1]: aout clock. \n"), AUD_CLI_AOUT_STATE)    
void AudCliGetAoutSt(u32 arg1, u32 arg2)
{
    if(0 == arg1)
    {
        AUD_CLI_AOUT_STATE_STR();
    }
    else if(1 == arg1)
    {
        if(arg2 < 5)
        {
            AudAout_ShowStatus(arg2);
        }
        else
        {            
            LOG(LOG_FEATURE, _T("Clock ID out of range.\r\n"));
        }
    }
}

/************************************************************************************************
 Function Name: AudCliExangeAout
 Function Description:exange aout1 and aout2 output
 input para:arg1:0:off,1:on
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard and |value
*************************************************************************************************/
#define AUD_CLI_EXC_AOUT_STR()    LOG(LOG_OTHER, \
    TEXT("[CLI]%d: Exange aout1 and aout2 output[0: Off, 1: On]. \n"), AUD_CLI_EXC_AOUT)
void AudCliExangeAout(u32 arg1)
{
}

/************************************************************************************************
 Function Name: AudCliExangeIEC
 Function Description:exange iec1 and iec2 output
 input para:arg1:0:off,1:on
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
                       2011.3.7     modify format according coding standard
*************************************************************************************************/
#define AUD_CLI_EXC_IEC_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Exange iec1 and iec2 output[0: off, 1: on]. \n"), AUD_CLI_EXC_IEC)
void AudCliExangeIEC(u32 arg1)
{
}

/************************************************************************************************
 Function Name: AudCliUartSwitch
 Function Description:This is the implementation for cli to uart switch to normal mode function
 input para: NONE
 output para: NULL
 Author:
 Data:
 Modify Data:
*************************************************************************************************/
bool AudCliUartSwitch(bool fgmode)
{
    AUD_ENABLE_UART_LOG(fgmode);
}

/************************************************************************************************
 Function Name: AudCliShowVersion
 Function Description:Show Audio Driver, DSP COMM, DSP DECODER version.
 input para: NONE
 output para: NULL
 Author:
 Data:
 Modify Data:
*************************************************************************************************/
#define AUD_CLI_SHOW_VERSION_STR() LOG(LOG_CLI, \
    TEXT("[CLI]%d: Show Audio Driver, DSP COMM, DSP DECODER version. \n"), AUD_CLI_SHOW_VERSION)
void AudCliShowVersion(void)
{
    AudShowVerInfo();
    LOG(LOG_CLI, TEXT("[AUD]Code Check In Date:%s.%s.%s\r\n"),AUD_VER_MONTH, AUD_VER_DAY, AUD_VER_YEAR );
    LOG(LOG_CLI, TEXT("Dsp Comm version: 0x%x\n"), DspGetCommCodeVersion());
    LOG(LOG_CLI, TEXT("Dsp Decoder version: 0x%x\n"), DspGetDecVersion());
}


/************************************************************************************************
 Function Name: AudCliShowIntHistory
 Function Description:Show Audio Driver, DSP COMM, DSP DECODER version.
 input para: NONE
 output para: NULL
 Author:
 Data:
 Modify Data:
*************************************************************************************************/
#define AUD_CLI_SHOW_INTHIST_STR() LOG(LOG_CLI, \
    TEXT("[CLI]%d: Show Uop cmd and INT[cmd] 1: s32, 2: uop. \n"), AUD_CLI_SHOW_INTHIST)    
extern void AudDispIntHistory(void);
void AudCliShowIntHistory(u32 arg1)
{
    if (1 == arg1)
    {
        AudDispIntHistory();
    }
    else if (2 == arg1)
    {
        AudDispUopHistory();
    }
}


#define AUD_CLI_AP_DVP_INT_STR() LOG(LOG_CLI, \
    TEXT("[CLI]%d: AP<->DVP Int [counter]. \n"), AUD_CLI_AP_DVP_INT)    
void AudCliApDvpInt(u32 arg1, u32 arg2)
{
    LOG(LOG_CTRLF, _T("unused dvp int."));
}


#define AUD_CLI_ADSP_ERR_RECOVER_LOG_STR() LOG(LOG_CLI, \
    TEXT("[CLI]%d: Adsp Err Recover debug log. \n"), AUD_CLI_ERR_RECOVER_LOG)
#if CONFIG_AUD_ADSP_ERR_RECOVER_EN
extern void vAdspErrRecoveryDbgLog(void);
#endif
void AudCliErrRcvyLog(u32 arg1)
{
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    if(arg1 == 0)
    {
        vAdspErrRecoveryDbgLog();
    }
    else if(arg1 == 1)
    {
        u4AdspErrProcStateSet(DSP_PROC_RUN);
        LOG(LOG_CTRLF, TEXT("ErrProcState: DSP_PROC_RUN \n"));
    }
    else if(arg1 == 2)
    {
        u4AdspErrProcStateSet(DSP_PROC_RESET);
        LOG(LOG_CTRLF, TEXT("ErrProcState: DSP_PROC_RESET \n"));
    }
    else
    #endif
    {
        LOG(LOG_CTRLF, TEXT("0: log; 1: PROC_RUN; 2: PROC_RESET; \n"));
    }
}


/************************************************************************************************
Function Name: AudCliReverbSt
Function Description:show reverb status
input para:NULL
output para: NULL
Author:fei.zhu
Data:2011.4.16
Modify Data:
*************************************************************************************************/
#define AUD_CLI_REVERB_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Reverb . \n"), AUD_CLI_REVERB)
void AudCliReverbSt()
{
    vAudSeCliCmd(AUD_SE_REVERB);
}


/************************************************************************************************
Function Name: AudCliPl2St
Function Description:show PL2 status
input para:NULL
output para: NULL
Author:fei.zhu
Data:2011.4.16
Modify Data:
*************************************************************************************************/
#define AUD_CLI_PROLOGICII_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: PrologicII. \n"), AUD_CLI_PROLOGICII)
void AudCliPl2St()
{
    vAudSeCliCmd(AUD_SE_PROLOGICII);
}


/************************************************************************************************
Function Name: AudCliEqSt
Function Description:show Eq status
input para:NULL
output para: NULL
Author:fei.zhu
Data:2011.4.16
Modify Data:
*************************************************************************************************/
#define AUD_CLI_EQUALIZER_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Equalizer. \n"), AUD_CLI_EQUALIZER)
void AudCliEqSt()
{
    vAudSeCliCmd(AUD_SE_EQUALIZER);
}


#define AUD_CLI_EQ_ONE_BAND_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Equalizer ctrl one bank [bank: 1~10][gain idx: 0~28]. \n"), \
    AUD_CLI_EQ_ONE_BAND)
void AudCliEqOneBand(u32 arg1, u32 arg2)
{
    if(arg1 == ARG_INIT_VAL || arg2 == ARG_INIT_VAL
        || (arg1 < 1 || arg1 > 10) || (arg2 > 28)) 
    {
        AUD_CLI_EQ_ONE_BAND_STR();
        return;
    }
    vAudSeEQOneBandCLICmd(arg1, arg2);
}

#define AUD_CLI_EQ_SET_IIR_COEF_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Equalizer Set coefficient [bank: 1~10][coef No:0~5][Coef data]. \n"), \
    AUD_CLI_EQ_IIR_COEF)
void AudCliEqSetIIRCoef(u32 arg1, u32 arg2, u32 arg3)
{
    if(arg1 == ARG_INIT_VAL || arg2 == ARG_INIT_VAL|| arg3 == ARG_INIT_VAL
        || (arg1 < 1 || arg1 > 10) || (arg2 > 5)) 
    {
        AUD_CLI_EQ_SET_IIR_COEF_STR();
        return;
    }
    vAudSeEQIIRCoefCLICmd(arg1, arg2, arg3);
}

/************************************************************************************************
Function Name: AudCliSpecSt
Function Description:show Spectrum bar value
input para:NULL
output para: NULL
Author:fei.zhu
Data:2011.4.16
Modify Data:
*************************************************************************************************/
#define AUD_CLI_SPEC_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: SPEC State. \n"), AUD_CLI_SPEC)
void AudCliSpectSt()
{
    u32 i = 0;
    AUD_DEC_SPECTRUM_INFO_T rInfo;
    DspGetSpectrumInfo(&rInfo);

    for (i = 0; i < AUD_SPECTRUM_INFO_NUM; i++)
    {
        LOG(LOG_FEATURE, TEXT("BAR_0:%08X\n"), rInfo.u4_aud_spectrum_bar[i]);
        LOG(LOG_FEATURE, TEXT("SPEC_0:%08X\n"), rInfo.u4_aud_spectrum[i]);
    }
}

/************************************************************************************************
Function Name: AudCliBassMSt
Function Description:show bass management
input para:NULL
output para: NULL
Author:fei.zhu
Data:2011.4.16
Modify Data:
*************************************************************************************************/

/* ui8_spk_layout format definition:
* ---------
* bit0 ~ bit2 (2~5ch)
* 0: LT/RT                    bit 3: CB (ch6)
* 1: Mono                       bit 4: ch7 exist or not
* 2: Stereo                   bit 5: subwoofer exist or not
* 3: L/R/C
* 4: L/R/S
* 5: L/R/C/S
* 6. L/R/LS/RS
* 7: L/R/C/LS/RS (over 7 ch, bit 0~2 should be set as 7)
*
* bit 12: Center Channel large(1)/small(0)
* bit 13: Left Channel large(1)/small(0)
* bit 14: Right Channel large(1)/small(0)
* bit 15: Left Surround Channel large(1)/small(0)
* bit 16: Right Surround Channel large(1)/small(0)
* bit 17: Center Back Channel large(1)/small(0)
* bit 18: No.7 Channel large(1)/small(0)
*
* bit32 ~ bit63
* represent the channel set
* 0: no remapping is required
* Other config is followed the rule below:
* bit 32: Center exist          bit 40: Overhead (Oh)
* bit 33: LR                     bit 41: LC/RC
* bit 34: LS/RS                 bit 42: LW/RW
* bit 35: LFE                     bit 43: LSS/RSS
* bit 36: CS (CB)                 bit 44: LFE2
* bit 37: Lh/Rh                 bit 45: LHS/RHS
* bit 38: LSR/RSR                 bit 46: CHR
* bit 39: Center high (Ch)      bit 47: LHR/RHR
*/

#define AUD_CLI_BASSM_STR()     LOG(LOG_CLI,    \
    TEXT("[CLI]%d: 0,0 :Show Bass management; \n")    \
    TEXT("            1,0 :Show Bass filter coeff; \n")    \
    TEXT("            2,0~300 :Front spk size freq; \n")    \
    TEXT("            3,0~300 :Center spk size freq; \n")    \
    TEXT("            4,0~300 :Rear spk size freq; \n")    \
    TEXT("            5,0~300 :Sub spk size freq; \n")    \
    TEXT("            6,0/300 :0:sub follow dolby; 300:sub force out; \n")    \
    TEXT("            note: 0~300: 0/5/10/15/.../300.(5hz/unit)\n"), AUD_CLI_BASSM)
void AudCliBassMSt(u32 arg1, u32 arg2)
{
    if((arg1 > 8) || (!IS_NORMAL_CUT_FREQ_VALUE(arg2)))
    {
        AUD_CLI_BASSM_STR();
        return;
    }

    switch(arg1)
    {
    case 0:
    {
        u32 u4tmpSpk ;
        u32 u4tmpSpk2;
        DspGetSpeakerConfig(&u4tmpSpk, &u4tmpSpk2);
        LOG(LOG_FEATURE, TEXT("speaker configure: 0x%08x, configure2: 0x%08x\n"), u4tmpSpk, u4tmpSpk2);

        switch(u4tmpSpk&0x7)
        {
        case 0:
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select LT/RT\n"));
            break;
        case 1:            
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select Mono\n"));
            break;
        case 2:
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select Stereo\n"));
            break;
        case 3: 
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select L/R/C\n"));
            break;
        case 4: 
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select L/R/S\n"));
            break; 
        case 5: 
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select L/R/C/S\n"));
            break;
        case 6: 
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select L/R/LS/RS\n"));
            break;
        case 7:
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select L/R/C/LS/RS\n"));
            break;
        default:                
            LOG(LOG_FAIL, TEXT("[SPKCFG]:failed.\n"));
            break;
        }

        if(u4tmpSpk & 0x8) //3    //3bit
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Select Center Back(channel 6)\n"));
        }

        if(u4tmpSpk  & 0x10) //4bit
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Channel 7 is exist\n"));
        }

        if(u4tmpSpk & 0x20) //5
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Subwoofer is exist\n"));
        }
        else
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Subwoofer is not exist\n"));
        }

        if(u4tmpSpk  & 0x1000) //12
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Center Channel is large\n"));
        }
        else
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Center Channel is Small\n"));
        }

        if(u4tmpSpk & 0x2000) //13
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Left Channel is large\n"));
        }
        else
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Left Channel is Small\n"));
        }

        if(u4tmpSpk & 0x4000) //14
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Right Channel is large\n"));
        }
        else
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Right Channel is Small\n"));
        }

        if(u4tmpSpk & 0x8000) //15
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Left Sur Channel is large\n"));
        }
        else
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Left Sur Channel is Small\n"));
        }

        if(u4tmpSpk & 0x10000) //16
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Right Sur Channel is large\n"));
        }
        else
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Right Sur Channel is Small\n"));
        }

        if(u4tmpSpk & 0x20000) //17
        {
            LOG(LOG_FEATURE, TEXT("[SPKCFG]:Center Back Channel is large\n"));
        }
    }
        break;
        
    case 1:
    {
        u32 u4Spk, u4Spk2;
        DspGetSpeakerConfig(&u4Spk, &u4Spk2);
        LOG(0 ,TEXT("speaker configure: 0x%x\n"), u4Spk);
        LOG(0 ,TEXT("D_SPKCFG: 0x%x\n"),u4ReadDspShmDWRD(D_SPKCFG));
        LOG(0 ,TEXT("W_FRONT_FREQ: %d\n"),u2ReadDspShmWORD(W_FRONT_FREQ));
        LOG(0 ,TEXT("W_CENTER_FREQ: %d\n"),u2ReadDspShmWORD(W_CENTER_FREQ));
        LOG(0 ,TEXT("W_REAR_FREQ: %d\n"),u2ReadDspShmWORD(W_REAR_FREQ));
        LOG(0 ,TEXT("W_SUB_FREQ: %d\n"),u2ReadDspShmWORD(W_SUB_FREQ));
    }
        break;
            
    case 2:
        vWriteDspShmWORD(W_FRONT_FREQ, (u16)arg2);        
        LOG(LOG_FEATURE ,TEXT("Set front speaker freq = 0x%x\n"),arg2);
        if(arg2)
        {
            vWriteDspShmDWRD(D_SPKCFG, (u4ReadDspShmDWRD(D_SPKCFG)&0xffff9fffL));
        }            
        else
        {
            vWriteDspShmDWRD(D_SPKCFG, (u4ReadDspShmDWRD(D_SPKCFG)|0x6000));
        }
        
        vSendDspCmd(UOP_DSP_CONFIG_SPEAKER);
        break;
        
    case 3:
        vWriteDspShmWORD(W_CENTER_FREQ, (u16)arg2);        
        LOG(LOG_FEATURE ,TEXT("Set center speaker freq = 0x%x\n"),arg2);
        if(arg2)
        {
            vWriteDspShmDWRD(D_SPKCFG, (u4ReadDspShmDWRD(D_SPKCFG)&0xffffefffL));
        }
        else
        {
            vWriteDspShmDWRD(D_SPKCFG, (u4ReadDspShmDWRD(D_SPKCFG)|0x1000));
        }
        
        vSendDspCmd(UOP_DSP_CONFIG_SPEAKER);
        break;

    case 4:
        vWriteDspShmWORD(W_REAR_FREQ, (u16)arg2);
        LOG(LOG_FEATURE ,TEXT("Set rear speaker freq = 0x%x\n"),arg2);
        if(arg2)
        {
            vWriteDspShmDWRD(D_SPKCFG, (u4ReadDspShmDWRD(D_SPKCFG)&0xfffe7fffL));
        }
        else
        {
            vWriteDspShmDWRD(D_SPKCFG, (u4ReadDspShmDWRD(D_SPKCFG)|0x18000));
        }
        vSendDspCmd(UOP_DSP_CONFIG_SPEAKER);
        break;

    case 5:
        vWriteDspShmWORD(W_SUB_FREQ, (u16)arg2);
        LOG(LOG_FEATURE ,TEXT("Set subwoofer speaker freq = 0x%x\n"),arg2);
        if(arg2)
        {            
            vWriteDspShmDWRD(D_SPKCFG, (u4ReadDspShmDWRD(D_SPKCFG)|0x20));
        }
        else
        {
            vWriteDspShmDWRD(D_SPKCFG, (u4ReadDspShmDWRD(D_SPKCFG)&0xffffffdfL));  // close sub
        }
        vSendDspCmd(UOP_DSP_CONFIG_SPEAKER);
        break;

    case 6:
        if(arg2 == 0)
        {
            DspSetSpeaker_subOutCtrl(0);
            LOG(0, TEXT("[AUD] only small mix-to sub, follow dolby behavior.\n"));
        }
        else
        {
            DspSetSpeaker_subOutCtrl(1<<8);
            LOG(0, TEXT("[AUD] force large mix-to sub.\n"));
        }
        break;

    case 7:
        vWriteDspShmWORD(W_FRONT_FREQ, 80);
        vWriteDspShmWORD(W_CENTER_FREQ, 120);
        vWriteDspShmWORD(W_REAR_FREQ, 160);
        vWriteDspShmWORD(W_SUB_FREQ, 200);
        DspSetSpeaker_subOutCtrl(1<<8);
        LOG(0, TEXT("[AUD] F/C/R/S : 80/120/160/200.\n"));
        break;

    case 8:            
        vWriteDspShmWORD(W_FRONT_FREQ, 200);
        vWriteDspShmWORD(W_CENTER_FREQ, 160);
        vWriteDspShmWORD(W_REAR_FREQ, 120);
        vWriteDspShmWORD(W_SUB_FREQ, 80);
        DspSetSpeaker_subOutCtrl(1<<8);
        LOG(0, TEXT("[AUD] F/C/R/S : 200/160/120/80.\n"));
        break;

        default:            
            LOG(0, TEXT("bass management error cmd.\n"));
            break;
    }  
}

/************************************************************************************************
Function Name: AudCliByPassSE
Function Description:ByPass Se (1:reverb,2:Eq,3:Pl2)
input para:NULL
output para: NULL
Author:fei.zhu
Data:2011.4.16
Modify Data:
*************************************************************************************************/
#define AUD_CLI_PP_BYPASS_STR()        LOG(LOG_CLI,                         \
    TEXT("[CLI]%d: Bypass Postprocess \n")                                    \
    TEXT("            [0: Reverb, 1: EQ, 2: PAE, 3: DBASS, \n")            \
    TEXT("             4: NEO6, 5: POSTMIX, 6: RBASS, 7: PL2, \n")            \
    TEXT("             8: SWAP, 9: TVS, 10: LPF, 11: PEQ, 12: BEQ]\n")        \
    TEXT("            [0: off, other: on]. \n"),                             \
    AUD_CLI_PP_BYPASS)
void AudCliByPassSE(u32 arg1, u32 arg2)
{
    u32 u4Type = arg1;
    bool bypassFlag = FALSE;
    if(arg2 != 0)
    {
        bypassFlag = TRUE;
    }

    LOG(LOG_CLI, TEXT("[SE]debug SEType is %d, flag is %d.\n"),u4Type, bypassFlag);
    SetAudSeDebugMode(u4Type, bypassFlag);
}


/************************************************************************************************
Function Name: dwASRCSampleRateMapping
Function Description: 
input para:NULL
output para: NULL
Author:fei.zhu
Data:2011.4.16
Modify Data:
*************************************************************************************************/
u32 dwASRCSampleRateMapping(u32 InSampleRate)
{
    u32 dwSamp;

    switch(InSampleRate)
    {
    case SFREQ_8K:
        dwSamp =8000;
        break;
    case SFREQ_11K:
        dwSamp =11025;
        break;
    case SFREQ_12K:
        dwSamp =12000;
        break;
    case SFREQ_16K:          // 16K
        dwSamp =16000;
        break;
    case SFREQ_22K:          // 22K
          dwSamp =22050;
        break;
    case SFREQ_24K:          // 24K
        dwSamp =24000;
        break;
    case SFREQ_32K:          // 32K
        dwSamp =32000;
        break;
    case SFREQ_44K:          // 44K
        dwSamp =44100;
        break;
    case SFREQ_48K:          // 48K
        dwSamp =48000;
        break;
    case SFREQ_64K:          //64K
        dwSamp = 64000;
        break;
    case SFREQ_88K:
        dwSamp  = 88200;  //88K
        break;
    case SFREQ_96K:
        dwSamp = 96000;  //96K
        break;
    case SFREQ_176K:
        dwSamp = 176400; //176K
        break;
    case SFREQ_192K:        //192K
        dwSamp = 192000;
        break;
    default:
        dwSamp =48000;
        break;
    }

    return(dwSamp);
}


/************************************************************************************************
Function Name: AudCliAsrcSt
Function Description: 
input para:NULL
output para: NULL
Author:fei.zhu
Data:2011.4.16
Modify Data:
*************************************************************************************************/
#define AUD_CLI_ASRC_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d:[0]Show ASRC Status[DecId: 1 or 3]. \n")\
    TEXT("        [1]Set ASRC [DecId: 1 or 3][1/0]on/off. \n"), AUD_CLI_ASRC)

void AudCliAsrcSt(u32 u1DecID)
{
    u32 InputSampleRate,OutSampleRate, dwSamp;
	u32 u4Ctrl = 0;

	DspGetAsrcControl(&u4Ctrl);

    if (u1DecID == PRI_DEC)
    {
        LOG(LOG_CLI, TEXT("[ASRC]Status: \n"));
        LOG(LOG_CLI, TEXT("Primary DEC ASRC on/off   = %s. \n"),
            (u4Ctrl &  (1<<0))?TEXT("on"):TEXT("off"));

        if (u4Ctrl & (1<<0))
        {
            DspGetAsrcSample(PRI_DEC, &InputSampleRate, &OutSampleRate);
            dwSamp=dwASRCSampleRateMapping(InputSampleRate);
            
            LOG(LOG_CLI, TEXT(">> Primary ASRC input samplerate = %dHz\n"), dwSamp);
            dwSamp=dwASRCSampleRateMapping(OutSampleRate);
            LOG(LOG_CLI, TEXT(">> Primary ASRC output samplerate = %dHz\n"), dwSamp);
        }
    }
    else if (u1DecID == SEC_DEC)
    {
        LOG(LOG_CLI, TEXT("[ASRC]Status: \n"));
        LOG(LOG_CLI, TEXT( ">> SEC_DEC ASRC on/off   = %s. \n"),
            (u4Ctrl &  (1<<1))?TEXT("on"):TEXT("off"));
        
        if(u4Ctrl &  (1<<1))
        {
            DspGetAsrcSample(SEC_DEC, &InputSampleRate, &OutSampleRate);

            dwSamp=dwASRCSampleRateMapping(InputSampleRate);
            LOG(LOG_CLI, TEXT( ">> Sec ASRC input samplerate = %dHz\n"), dwSamp);

            dwSamp=dwASRCSampleRateMapping(InputSampleRate);
            LOG(LOG_CLI, TEXT(">> Sec ASRC output samplerate = %dHz\n"), dwSamp);
        }
    }
    else
    {
        AUD_CLI_ASRC_STR();
    }
}



void AudCliAsrcTest(u32 u4Item, u32 u1DecID, u32 u4Cmd)
{
    if(0 == u4Item)
    {
        AudCliAsrcSt(u1DecID);
    }
    else if(1 == u4Item)
    {
        vAdspEnableASRC(u1DecID, u4Cmd);
    }
    else
    {
        AUD_CLI_ASRC_STR();
    }

}



/************************************************************************************************
Function Name: AudCliGpxMixSvc
Function Description: Set GpsMix Cmd
input para: cmd type
output para: NULL
Author:tongfa
Data:2012.2.21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_GPSMIX_CTRL_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: GpsMix Cmd [0: stop, 1: start, 2: pause, 3: resume]. \n"), \
    AUD_CLI_GPSMIX_CTRL)
void AudCliGpxMixSvc(u32 u4Type)
{
    switch(u4Type)
    {
    case AUD_DEC_GPS_MIX_CTRL_STOP:
        LOG(LOG_CLI, TEXT("[CLI] Enter Gpsmix Stop Cmd CLI.\n"));
        _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_CTRL_STOP);
        break;

    case AUD_DEC_GPS_MIX_CTRL_START:
        LOG(LOG_CLI, TEXT("[CLI] Enter Gpsmix Statr Cmd CLI.\n"));
        _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_CTRL_START);
        break;

    case AUD_DEC_GPS_MIX_CTRL_PAUSE:
        LOG(LOG_CLI, TEXT("[CLI] Enter Gpsmix Pause Cmd CLI.\n"));
        _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_CTRL_PAUSE);
        break;

    case AUD_DEC_GPS_MIX_CTRL_RESUME:
        LOG(LOG_CLI, TEXT("[CLI] Enter Gpsmix Resume Cmd CLI.\n"));
        _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_CTRL_RESUME);
        break;

    default:
        AUD_CLI_GPSMIX_CTRL_STR();
        break;
    }
}


/************************************************************************************************
Function Name: AudCli_PrintTTInfo
Function Description:  
input para:  
output para: 
Author:
Data:
Modify Data:
*************************************************************************************************/
void AudCli_PrintTTInfo(void)
{
    u32 u4FrnFlag, u4RearFlag;
    DspGetTestToneFlag(&u4FrnFlag, &u4RearFlag);
    LOG(LOG_CLI, TEXT("~~~~~~~~~~Front Test Tone~~~~~~~~~~~~~~~\n"));
    LOG(LOG_CLI, TEXT("Front TT Flag = 0x%x\n"),(u4FrnFlag>>8));
    LOG(LOG_CLI, TEXT("~~~~~~~~~~Front Test Tone End~~~~~~~~~~~~~~~\n"));

    LOG(LOG_CLI, TEXT("~~~~~~~~~~Rear Test Tone~~~~~~~~~~~~~~~\n"));
    LOG(LOG_CLI, TEXT("Rear TT Flag = 0x%x\n"),(u4RearFlag>>8));

    LOG(LOG_CLI, TEXT("~~~~~~~~~~Rear Test Tone End~~~~~~~~~~~~~~~\n"));

    LOG(LOG_CLI, TEXT("Bit 0: ON/OFF\n"));
    LOG(LOG_CLI, TEXT("Bit 4: Pink Noise\n"));
    LOG(LOG_CLI, TEXT("Bit 5: Triangle wave\n"));
    LOG(LOG_CLI, TEXT("Bit 6: sine wave\n"));
}


/************************************************************************************************
Function Name: AudCliTestToneSvc
Function Description:  
input para:  
output para: NULL
Author:tongfa
Data:2012.2.21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_TESTTONE_STR()     LOG(LOG_CLI,                                                         \
    TEXT("[CLI]%d: TESTTONE CLI:  \n")                                                                    \
    TEXT("            Set On Off     [0][0:Off, 1:On][0: Front, 1: Rear]\n")                            \
    TEXT("            Select Type    [1][0~4: pn, triangle, sine, white, pnd][0: Front, 1: Rear]\n")      \
    TEXT("            Select Channel [2][AUD_DEC_LS_T][0: Front, 1: Rear]\n")                            \
    TEXT("            Print TT Info  [3]\n")                                                             \
    , AUD_CLI_TESTTONE)
void AudCliTestToneSvc(u32 u4Type, u32 arg1, u32 arg2)
{
    switch(u4Type)
    {
    case 0:
        LOG(LOG_CLI, TEXT("[CLI]TestTone Switch: %s, %s.\n"), 
            (arg1 == AUD_DEC_TESTTONE_ENABLE ? TEXT("on") : TEXT("off")), 
            (arg2 == AUD_DEC_TESTTONE_FRONT ? TEXT("Front") : TEXT("Rear")));
        vAudTestToneSwitch(arg1, arg2);
        break;
        
    case 1:
        LOG(LOG_CLI, TEXT("[CLI]TestTone Select Type: %d, %s.\n"), arg1, 
            (arg2 == AUD_DEC_TESTTONE_FRONT ? TEXT("Front") : TEXT("Rear")));
        vAudTestToneSetType(arg1, arg2);
        break;

    case 2:
        LOG(LOG_CLI, TEXT("[CLI]TestTone Select Channel: %d, %s.\n"), arg1, 
            (arg2 == AUD_DEC_TESTTONE_FRONT ? TEXT("Front") : TEXT("Rear")));
        vAudTestToneSetChannel(arg1,arg2);
        break;

    case 3:
        AudCli_PrintTTInfo();
        break;
        
    default:
        AUD_CLI_TESTTONE_STR();
        break;
    }
}

/************************************************************************************************
Function Name: AudCliSelFrnRear
Function Description:  
input para:  
output para: 
Author:
Data:
Modify Data:
*************************************************************************************************/
#define AUD_CLI_SEL_FRN_RERA_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Select Front Rear spk[output: 0~5][source: 0~4]. \n"), AUD_CLI_SEL_FRN_RERA)
void AudCliSelFrnRear(AUD_CFG_ID eOutPut,AUD_OUT_TYPE_T eSource)
{
    if(ARG_INIT_VAL == eOutPut || ARG_INIT_VAL == eSource)
    {
        AUD_CLI_SEL_FRN_RERA_STR();
    }
    else 
    {
        AUD_OUTPUT_PATH_T rSelectRearFrn = {0};
        rSelectRearFrn.eOut = eOutPut;
        rSelectRearFrn.eSrc = eSource;
        AudAout_PathSet(&rSelectRearFrn);
    }
}

/************************************************************************************************
Function Name: AudCliCsiiSetting
Function Description:  
input para:  
output para: 
Author:
Data:
Modify Data:
*************************************************************************************************/
#define AUD_CLI_CSII_STR()     LOG(LOG_CLI,                    \
    TEXT("[CLI]%d: CSII [ctrlId: 0 ~ %d][value]. \n")           \
    TEXT("0~3: switch ; mode; phantom; FB; \n")                     \
    TEXT("4~10: Focus_C,Focus_F,Focus_R; TB,F_SS,S_SS,R_SS; \n")         \
    TEXT("11~15: F2R,C2R; TBF,TBS,TBR;  \n")                     \
    TEXT("Level Setting: 16~18: F_Center_LEVEL, F_Front_LEVEL, F_Rear_LEVEL;\n ") \
    TEXT("19~ 23: TB_Front_LEVEL, TB_Rear_LEVEL, TB_Sub_LEVEL; Front2Rear, Center2Rear; \n")\
    ,AUD_CLI_CSII, AUD_SE_CSII_CTRL_C2R)
void AudCliCsiiSetting(u32 u4Param1, u32 u4Param2)
{
    AUD_SE_CSII_CTRL_INFO_T rCSIICtrl;
    AUD_SE_CSII_LEVEL_CTRL_INFO_T rCSIICtrlLevel;
    AUD_SE_OPCMD_T *Op_SePost=NULL;
    AUD_SE_OPCMD_T  OpCmd;    
    Op_SePost = &OpCmd;

    if(ARG_INIT_VAL == u4Param1 || ARG_INIT_VAL == u4Param2)
    {
        AUD_CLI_CSII_STR();
        return;
    }
    
    //CSII ctrl setting
    if(u4Param1 <= AUD_SE_CSII_CTRL_TBR)
    {
        LOG(LOG_FEATURE, TEXT("CSII ctrl setting, u4Param1 = 0x%x\r\n"),u4Param1);
        rCSIICtrl.e_ctrlID = u4Param1;
        switch (u4Param1)
        {
        case AUD_SE_CSII_CTRL_SWITCH:
            rCSIICtrl.u.e_csii_switch = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_MODE:
            rCSIICtrl.u.e_mode = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_PHANTOM:
            rCSIICtrl.u.e_phantom = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_FB:
            rCSIICtrl.u.e_fb = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_FOCUS_CENTER:
            rCSIICtrl.u.e_focuscenter = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_FOCUS_FRONT:
            rCSIICtrl.u.e_focusfront = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_FOCUS_REAR:
            rCSIICtrl.u.e_focusrear = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_TBF:
        case AUD_SE_CSII_CTRL_TBS:
        case AUD_SE_CSII_CTRL_TBR:        
            rCSIICtrl.u.e_TB = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_F_SS:
            rCSIICtrl.u.e_front_ss = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_S_SS:
            rCSIICtrl.u.e_sub_ss = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_R_SS:
            rCSIICtrl.u.e_rear_ss = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_F2R:
            rCSIICtrl.u.e_f2r = u4Param2;
            break;

        case AUD_SE_CSII_CTRL_C2R:
            rCSIICtrl.u.e_c2r = u4Param2;
            break;


        default:
            AUD_CLI_CSII_STR();
            return ;
        }
        //vAudSESetCSII(&rCSIICtrl);

        Op_SePost->u4OpCode=AUD_SE_OPCODE_CTRL;
        Op_SePost->pvData=(void*)&rCSIICtrl;
        Op_SePost->u4DataSize=sizeof(AUD_SE_CSII_CTRL_INFO_T);
        Op_SePost->u1Type=AUD_SE_CSII;

        fgAudSeProcessOpCmd((void*)&OpCmd);     
    }
    else if((u4Param1>=AUD_SE_CSII_FOCUS_CENTER_LEVEL)&&(u4Param1<=AUD_SE_CSII_CENTER2REAR_LEVEL)) //AUD_SE_CSII_LEVEL_CTRL_INFO_T ID from 16~23
    {
        LOG(LOG_FEATURE, TEXT("CSII ctrl level setting, u4Param1 = 0x%x"),u4Param1);
        rCSIICtrlLevel.e_LevelctrlID = u4Param1;
        switch (u4Param1)
        {
        case AUD_SE_CSII_FOCUS_CENTER_LEVEL:
            rCSIICtrlLevel.u.i4FocusCenterLevel = u4Param2;
            break;
            
        case AUD_SE_CSII_FOCUS_FRONT_LEVEL:
            rCSIICtrlLevel.u.i4FocusFrontLevel = u4Param2;
            break;

        case AUD_SE_CSII_FOCUS_REAR_LEVEL:
            rCSIICtrlLevel.u.i4FocusRearLevel = u4Param2;
            break;
            
        case AUD_SE_CSII_TRUBASS_FRONT_LEVEL:
            rCSIICtrlLevel.u.i4TBFrontLevel = u4Param2;
            break;

        case AUD_SE_CSII_TRUBASS_SUB_LEVEL:
            rCSIICtrlLevel.u.i4TBSubLevel = u4Param2;
            break;

        case AUD_SE_CSII_TRUBASS_REAR_LEVEL:
            rCSIICtrlLevel.u.i4TBRearLevel = u4Param2;
            break;

        case AUD_SE_CSII_FRONT2REAR_LEVEL:
            rCSIICtrlLevel.u.i4Front2RearLevel = u4Param2;
            break;

        case AUD_SE_CSII_CENTER2REAR_LEVEL:
            rCSIICtrlLevel.u.i4Center2RearLevel = u4Param2;
            break;

       default:
            AUD_CLI_CSII_STR();
            return ;           
         }

        Op_SePost->u4OpCode=AUD_SE_OPCODE_SET_LEVEL;        
        Op_SePost->pvData=(void*)&rCSIICtrlLevel;
        Op_SePost->u4DataSize=sizeof(AUD_SE_CSII_LEVEL_CTRL_INFO_T);
        Op_SePost->u1Type=AUD_SE_CSII;

        fgAudSeProcessOpCmd((void*)&OpCmd); 
        }
}



/************************************************************************************************
Function Name: AudCli_RearVol
Function Description:  
input para:  
output para: 
Author:
Data:
Modify Data:
*************************************************************************************************/
#define AUD_CLI_REAR_VOL_CTRL_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Set Rear Volume gain[level: 0 ~ 100]. \n"), AUD_CLI_REAR_VOL_CTRL)
void AudCli_RearVol(u32 u1Level)
{
    if(ARG_INIT_VAL == u1Level)
    {
        AUD_CLI_REAR_VOL_CTRL_STR();
    }
    else 
    {
        AUD_DEC_REAR_VOLUME_INFO_T tRearVol={0};
        AUD_OUTPUT_PATH_T rRearVolSelect = {0};
        rRearVolSelect.eOut = AUD_REAR;
        rRearVolSelect.eSrc = AUD_AOUT2;

        tRearVol.ui1_level = u1Level;
        LOG(LOG_FEATURE, TEXT("[AudCli_RearVol]Rear Vol Level = %x\n"),tRearVol.ui1_level);

        AudAout_PathSet(&rRearVolSelect);

        fgAdspSetRearAoutMediaType(AUD_OUT_MEDIA_USB);

        AudSetRearVolume(&tRearVol);
    }
}


/************************************************************************************************
Function Name: AudCli_Upmix
Function Description:  
input para:  
output para: 
Author:
Data:
Modify Data:
*************************************************************************************************/
#define AUD_CLI_UPMIX_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: Upmix mode[0: show log, 1:open, 2 close]. \n"), AUD_CLI_UPMIX)
void AudCli_Upmix(u32 uPmode,u32 Opcode)
{
    u32 i;
    u32 UPMIX_GAIN[8] = 
           {0x4CCCCC,0x199999,0x199999,0x4CCCCC,
            0x333333,0x333333,0x666666,0x666666};
    
    AUD_SE_UPMIX_CTRL_INFO_T *Gain_Value=NULL;

    AUD_SE_OPCMD_T *Op_SePost=NULL;
    AUD_SE_OPCMD_T  OpCmd;

    if (uPmode > 2)
    {
        AUD_CLI_UPMIX_STR();
    }
    
    LOG(LOG_CTRLF, TEXT("[SE_UPMIX]mode = %d [0: show log, 1:open, 2 close]\n"), (u32)uPmode);
    if(uPmode == 0)
    {
        vAudSeCliCmd(AUD_SE_UPMIX);
        return;
    }

    Gain_Value = kzalloc(sizeof(AUD_SE_UPMIX_CTRL_INFO_T), GFP_KERNEL);
    if (Gain_Value == NULL)
    {
        return;
    }
    Op_SePost = &OpCmd;
    
    if(uPmode == 1)
    { 
        Opcode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
        Gain_Value->e_upmix_mode = AUD_SE_UPMIX_MODE_ON;
    }
    else if(uPmode == 2)
    {
        Opcode = AUD_SE_OPCODE_OFF;
    }

    for (i = 0; i < 8; i++)
    {
        Gain_Value->UPMIX_GAIN[i] = UPMIX_GAIN[i];
    }

    Op_SePost->u4OpCode = Opcode; 
    Op_SePost->pvData=(void*)Gain_Value;
    Op_SePost->u4DataSize=sizeof(AUD_SE_UPMIX_CTRL_INFO_T);
    Op_SePost->u1Type=AUD_SE_UPMIX;

    fgAudSeProcessOpCmd((void*)&OpCmd);
    kfree(Gain_Value);
}

/************************************************************************************************
Function Name: AudCli_ATS
Function Description:  
input para:  
output para: 
Author:
Data:
Modify Data:
*************************************************************************************************/
const AUD_SE_ATS_COEF_T _rAudSe_ATS_Coef_Cli = {
            0x0,         // Control mode  default: 0
        0x1A000,         // input Gain  0dB:0x20000
        0x20000,        //Center Output Gain    0dB:0x20000
        0x20000,         // L/R Output Gain  0dB:0x20000
        0x20000,         // LS/RS Output Gain  0dB:0x20000
        0x20000,         // LFE Output Gain  0dB:0x20000
        0x20000,         //  Center input gain   0dB:0x20000
        0x20000,         //  LFE input gain  0dB:0x20000
        0x169FB,         //  LSRS input gain  0dB:0x20000            
            0x0,         // Center mix to LR gain: 0~0x7FFFFF
            0x0,         // Center mix to LSRS gain  0~0x7FFFFF
            0x0,         //  LR mix to LSRS gain  0~0x7FFFFF
           0x10,         //LSRS channel overall delay, unit:banks (1.33ms/bank for 48K), 16banks means about 21ms            
    };

extern u32 g_u4sleeptime;
extern u32 _u4ATS_LSRS_BAND_IIRCoef;
u32 u4CtrlMode;   
u32 u4InputGain;                         
u32 u4CenterGain;
u32 u4LRGain;
u32 u4LsRsGain;
u32 u4LfeGain;
u32 u4CenterInGain;
u32 u4LfeInGain;
u32 u4LsRsInGain;    
u32 u4C2LRGain;
u32 u4C2LsRsGain;
u32 u4Lr2LsRsGain;  
u32 u4OverallDelay;

#define AUD_CLI_ATS_STR()     LOG(LOG_CLI, \
    TEXT("[CLI]%d: ATS mode[0:Close, 1:open, 2 show log,3 open without UPG_COEF,4 change para]. \n")\
    TEXT("1~4: Switch; u4CtrlMode ; u4InputGain; u4CenterGain;  \n")                     \
    TEXT("5~8: LRGain,LsRsGain,LfeGain; CenterInGain; \n")         \
    TEXT("9~11: CenterInGain,LfeInGain; LsRsInGain;  \n")                     \
    TEXT("12~15: C2LRGain,C2LsRsGain;C2LsRsGain;Lr2LsRsGain  \n")                     \
    TEXT("16: u4OverallDelay; \n")\
    , AUD_CLI_ATS);
void AudCli_ATS(u32 uPmode,u32 uPara2,u32 uPara3,const s8 ** filename)
{
    u32 Opcode = 0;

    AUD_SE_OPCMD_T *Op_SePost=NULL;
    AUD_SE_OPCMD_T  OpCmd;

    AUD_SE_ATS_COEF_T * Coef_Value;
    AUD_SE_ATS_CTRL_INFO_T * Ctrl_Info;   

    if(uPmode > 5)
    {
        AUD_CLI_ATS_STR();
        return;
    }
    
    LOG(LOG_CTRLF, TEXT("[SE_ATS]mode = %d [0:Close, 1:open, 2 show log,3 open without UPG_COEF,4 change para]\n"), (u32)uPmode);
    if(uPmode == 2)
    {
        vAudSeCliCmd(AUD_SE_ATS);
        return;
    }

    if(uPmode == 5)
    {
        g_u4sleeptime = uPara2;
        return;
    }

    Coef_Value = kzalloc(sizeof(AUD_SE_ATS_COEF_T), GFP_KERNEL);
    
    if (Coef_Value == NULL)
    {
        return;
    }

    LOG(LOG_CTRLF, TEXT("[SE_ATS]before COPY %d.\n"), (u32)uPmode);

    x_memcpy(Coef_Value, &_rAudSe_ATS_Coef_Cli, sizeof(AUD_SE_ATS_COEF_T));  // Get Default Coefficient   

    LOG(LOG_CTRLF, TEXT("[SE_ATS]AFTER COPY %d.\n"), (u32)uPmode);

    Op_SePost = &OpCmd;
    
    if(uPmode == 1)
    { 
        Opcode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
        Coef_Value->u4CtrlMode = AUD_SE_ATS_MODE_ON;
    }
    else if(uPmode == 0)
    {
        Opcode = AUD_SE_OPCODE_OFF;
    }
    else if(uPmode == 3)
    {
        Opcode = AUD_SE_OPCODE_ON;
    }
    else if(uPmode == 4)
    {
            if(uPara2 == 0)
            {
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 CTRL_SWITCH = %d, para3 On:1, off:0 \n"), AUD_SE_ATS_CTRL_SWITCH);   
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 CTRL_MODE = %d \n"), AUD_SE_ATS_CTRL_MODE);  
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 INPUT_GAIN = %d, para3 0dB:0x20000\n"), AUD_SE_ATS_INPUT_GAIN);   
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 CENTER_OUTPUT_GAIN = %d, para3 0dB:0x20000 \n"), AUD_SE_ATS_CENTER_OUTPUT_GAIN);                    
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 LR_OUTPUT_GAIN = %d, para3 0dB:0x20000 \n"), AUD_SE_ATS_LR_OUTPUT_GAIN);   
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 LSRS_OUTPUT_GAIN = %d, para3 0dB:0x20000 \n"), AUD_SE_ATS_LSRS_OUTPUT_GAIN);  
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 LFE_OUTPUT_GAIN = %d, para3 0dB:0x20000 \n"), AUD_SE_ATS_LFE_OUTPUT_GAIN);   
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 CENTER_INPUT_GAIN = %d, para3 0dB:0x20000 \n"), AUD_SE_ATS_CENTER_INPUT_GAIN);       
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 LFE_INPUT_GAIN = %d, para3 0dB:0x20000 \n"), AUD_SE_ATS_LFE_INPUT_GAIN);   
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 LSRS_INPUT_GAIN = %d, para3 0dB:0x20000 \n"), AUD_SE_ATS_LSRS_INPUT_GAIN);                    
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 C_2_LR_GAIN= %d, para3 0dB:0x7FFFFF \n"), AUD_SE_ATS_C_2_LR_GAIN);   
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 C_2_LSRS_GAIN = %d, para3 0dB:0x7FFFFF \n"), AUD_SE_ATS_C_2_LSRS_GAIN);  
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 LR_2_LSRS_GAIN= %d, para3 0dB:0x7FFFFF \n"), AUD_SE_ATS_LR_2_LSRS_GAIN);   
                LOG(LOG_CTRLF, TEXT("[SE_ATS]para2 SURROUND_DELAY = %d, para3 0dB:0x7FFFFF \n"), AUD_SE_ATS_SURROUND_DELAY);                  
                kfree(Coef_Value);
                return;
            }
        if(uPara2 > AUD_SE_ATS_SURROUND_DELAY)
            {
                kfree(Coef_Value);
                return;
            }

        Opcode = AUD_SE_OPCODE_CTRL;
        Ctrl_Info = kzalloc(sizeof(AUD_SE_ATS_CTRL_INFO_T), GFP_KERNEL);
        
        if(NULL != Ctrl_Info)
        {
        Ctrl_Info->e_ctrlID=uPara2;
        Ctrl_Info->u.u4CtrlMode=uPara3;
        Op_SePost->u4OpCode = Opcode; 
        Op_SePost->pvData=(void*)Ctrl_Info;
        Op_SePost->u4DataSize=sizeof(AUD_SE_ATS_CTRL_INFO_T);
        Op_SePost->u1Type=AUD_SE_ATS;

        LOG(LOG_CTRLF, TEXT("[SE_ATS]CALL fgAudSeProcessOpCmd1 %d\n"), (u32)uPmode);
        fgAudSeProcessOpCmd((void*)&OpCmd);
        kfree(Ctrl_Info); 
        }
        else
        {
            LOG(LOG_FAIL, TEXT("[memory alloc  Ctrl_Info failed.\r\n"));
        }
        kfree(Coef_Value);
        return;
    }
    

    Op_SePost->u4OpCode = Opcode; 
    Op_SePost->pvData=(void*)Coef_Value;
    Op_SePost->u4DataSize=sizeof(AUD_SE_ATS_COEF_T);
    Op_SePost->u1Type=AUD_SE_ATS;

    LOG(LOG_CTRLF, TEXT("[SE_ATS]CALL fgAudSeProcessOpCmd2 %d\n"), (u32)uPmode);

    fgAudSeProcessOpCmd((void*)&OpCmd);
    kfree(Coef_Value);
    LOG(LOG_CTRLF, TEXT("[SE_ATS]CLI return\n"));    
}

#if ((CONFIG_DRV_AUDIO_EXTERNAL_POST_PROC_SUPPORT)||(CONFIG_DRV_AUDIO_EXTERNAL_POST_PROC_ON_ARM1))
/************************************************************************************************
Function Name: AudCli_ext_PP
Function Description:  
input para:  
output para: 
Author: Jingjian Yu
Data: 2014/08/1
Modify Data:
*************************************************************************************************/
/* External Post Processing Control Definition ****************************************/
typedef enum
{
     AUD_SE_CLI_EXT_PP_TYPE,
     AUD_SE_CLI_EXT_PP_MODE,
     AUD_SE_CLI_EXT_PP_USER_PARM,
     AUD_SE_CLI_EXT_PP_CLI,
     AUD_SE_CLI_EXT_PP_SET_PROC_PRIORITY,
     AUD_SE_CLI_EXT_PP_GET_USER_PARM,
     AUD_SE_CLI_EXT_PP_POS,         // Add for extpp position before or after BM (aud_se_extpp_pos) -- Water 2011-10-25
     AUD_SE_CLI_EXT_PP_INIT_APP_PARM,
     AUD_SE_CLI_EXT_PP_UOP,
     AUD_SE_CLI_EXT_SHOW_INFO,
     AUD_SE_CLI_EXT_TEST_GAIN,     
} AUD_SE_CLI_EXT_PP_CTRL_TYPE_T;


#define AUD_CLI_EXT_PP_STR()        LOG(LOG_CLI,                         \
    TEXT("[CLI]%d: ARM Ext PP: \n")                                    \
    TEXT("            0: Set EXT_PP_TYPE; \n")            \
    TEXT("            1: On/Off; \n")            \
    TEXT("            2: Set EXT_PP_USER_PARM; \n")            \
    TEXT("            3: Set EXT_PP_CLI; \n")            \
    TEXT("            4: Set PROC PRIORITY; \n")            \
    TEXT("            5: Set GET_USER_PARM(Not Ready),; \n")            \
    TEXT("            6: Set EXT_PP_POS; \n")            \
    TEXT("            7: Set INIT_APP_PARM(Not Ready); \n")            \
    TEXT("            8: Set EXT_PP_UOP; \n")            \
    TEXT("            9: Show EXT_PP Info; \n")            \
    TEXT("            10: Test Gain; \n"),            \
    AUD_CLI_ARM_EXT_PP)
void AudCliExtPP(u32 u4Opmod,u32 uPara2,u32 uPara3,const s8 ** filename)
{   
    extern u32 _u4SmallGainValue;
    extern u32 _u4LargeGainValue;

    if((u4Opmod != AUD_SE_CLI_EXT_PP_TYPE)
        && (u4Opmod != AUD_SE_CLI_EXT_PP_MODE)        
        && (u4Opmod != AUD_SE_CLI_EXT_PP_USER_PARM)
        && (u4Opmod != AUD_SE_CLI_EXT_PP_CLI)   
        && (u4Opmod != AUD_SE_CLI_EXT_PP_SET_PROC_PRIORITY)         
        && (u4Opmod != AUD_SE_CLI_EXT_PP_POS)  
        && (u4Opmod != AUD_SE_CLI_EXT_PP_UOP)     
        && (u4Opmod != AUD_SE_CLI_EXT_SHOW_INFO) 
        && (u4Opmod != AUD_SE_CLI_EXT_TEST_GAIN)
      )
    {
        AUD_CLI_EXT_PP_STR();
        return;
    }
    
    LOG(LOG_CTRLF, TEXT("[SE_EXT_PP]u4Opmode = %d \n"), u4Opmod);
    if(u4Opmod == AUD_SE_CLI_EXT_SHOW_INFO)
    {
        vAudSeCliCmd(AUD_SE_EXT_PP);
        return;
    }
    if(u4Opmod == AUD_SE_CLI_EXT_TEST_GAIN)
    {
        if(uPara2 == 1)
            {
              _u4LargeGainValue = uPara3;
            }
        if(uPara2 == 2)
            {
              _u4SmallGainValue = uPara3;
            }        
        return;
    }    
    vAudSeExtPPCLICmd(u4Opmod,uPara2,uPara3);
  
}
#endif  

/************************************************************************************************
Function Name: AudCliMvs
Function Description:  
input para:  
output para: 
Author: Zhongjie
Data: 2012/05/10
Modify Data:
*************************************************************************************************/
#define AUD_CLI_MVS_STR()        LOG(LOG_CLI,                         \
    TEXT("[CLI]%d: Mvs mode: \n")                                    \
    TEXT("            0: Show log; \n")            \
    TEXT("            1: Off; \n")            \
    TEXT("            2: standard; \n")            \
    TEXT("            3: Music; \n")            \
    TEXT("            4: Movie; \n")            \
    TEXT("            5: Mode 0; \n")            \
    TEXT("            6: Mode 1(bypass 2ch); \n"),            \
    AUD_CLI_MVS)
void AudCliMvs(u32 u4Opmod)
{
    u32 Opcode;
    u32 MVS_Coef_Default[9]={0x300,0xA0000,0x200000,0xA0000,0xA0000,0x150000,0xC0000,0x33333,0};
    u32 MVS_Coef_Music[9]=  {0x300,0xB4FDF,0x166666,0x100000,0x4CCCC,0x100000,0x100000,0x21DE6,0};
    u32 MVS_Coef_Movie[9]=  {0x300,0xCCCCC,0x199999,0x80000,0x33333,0x66666,0x100000,0x25A91,0};
    void *Gain_Value=NULL;

    AUD_SE_OPCMD_T *Op_SePost=NULL;
    AUD_SE_OPCMD_T  OpCmd;

    if(u4Opmod > 6)
    {
        AUD_CLI_MVS_STR();
        return;
    }
    
    LOG(LOG_CTRLF, TEXT("[SE_MVS]u4Opmode = %d \n"), (u32)u4Opmod);
    if (u4Opmod == 0)
    {
        vAudSeCliCmd(AUD_SE_MVS);
        return;
    }

    Gain_Value = kzalloc(sizeof(u32)*9, GFP_KERNEL);
    if (Gain_Value == NULL)
    {
        return;
    }
    Op_SePost = &OpCmd;
    
    if(u4Opmod == 1)       //off
    {
        Opcode = AUD_SE_OPCODE_OFF;
    }
    else if(u4Opmod == 2)  //default
    {
        Opcode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
        x_memcpy(Gain_Value, MVS_Coef_Default, sizeof(u32)*9);
    }
    else if(u4Opmod == 3)  //music
    {
        Opcode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
        x_memcpy(Gain_Value, MVS_Coef_Music, sizeof(u32)*9);
    }
    else if(u4Opmod == 4)  //movie
    {
        Opcode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
        x_memcpy(Gain_Value, MVS_Coef_Movie, sizeof(u32)*9);
    }
    else if(u4Opmod == 5)  //mode 0 (default mode)
    {
        Opcode = (1<<4); //AUD_SE_MVS_OPCODE_MOD0;
    }
    else if(u4Opmod == 6)  //mode 1 (bypass 2.0 ch)
    {
        Opcode = (1<<5); //AUD_SE_MVS_OPCODE_MOD1;
    }

    Op_SePost->u4OpCode = Opcode; 
    Op_SePost->pvData=(void*)Gain_Value;
    Op_SePost->u4DataSize=sizeof(u32)*9;
    Op_SePost->u1Type=AUD_SE_MVS;

    fgAudSeProcessOpCmd((void*)&OpCmd);
    kfree(Gain_Value);
}

void AudCli_PrintLoudnessGain(void)
{
}


void AudCli_LoudNess(u32 uPmode)
{
    u32 LOUD_GAIN[20][6] ={ 
        {0x100000,0xe0b3d9,0x0f4e20,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe0be81,0x0f43b6,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe0c9cb,0x0f38b1,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe0d5c0,0x0f2d0a,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe0e26a,0x0f20b6,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe0efd5,0x0f13ae,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe0fe0b,0x0f05e5,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe10d19,0x0ef753,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe11d0a,0x0ee7eb,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe12dee,0x0ed7a2,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe13fd2,0x0ec66c,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe152c6,0x0eb43b,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe166d8,0x0ea103,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe17c1c,0x0e8cb6,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe192a2,0x0e7743,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe1aa7d,0x0e609d,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe1c3c3,0x0e48b2,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe1de87,0x0e2f72,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe1fae2,0x0e14cc,0x1f5636,0xf0a807,0x5a9c78},
        {0x100000,0xe218eb,0x0df8ad,0x1f5636,0xf0a807,0x5a9c78}
    };
    AUD_SE_LOUDNESS_CTRL_INFO_T *Loud_Value=NULL;
    AUD_SE_OPCMD_T *Op_SePost=NULL;
    AUD_SE_OPCMD_T  OpCmd;
    u32 i;

    Loud_Value = kzalloc(sizeof(AUD_SE_LOUDNESS_CTRL_INFO_T), GFP_KERNEL);
    if (Loud_Value == NULL)
    {
        return;
    }
    Op_SePost = &OpCmd;
    LOG(LOG_CTRLF, TEXT("loudness_mode = %d\n"), (u32)uPmode);
    switch(uPmode)
    {
    case AUD_SE_LOUDNESS_1dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_1dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[0][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;

    case AUD_SE_LOUDNESS_2dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_2dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[1][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_3dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_3dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[2][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_4dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_4dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[3][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_5dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_5dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[4][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_6dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_6dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[5][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_7dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_7dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i] = LOUD_GAIN[6][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_8dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_8dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i] = LOUD_GAIN[7][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_9dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_9dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i] = LOUD_GAIN[8][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_10dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_10dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[9][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_11dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_11dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[10][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_12dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_12dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[11][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_13dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_13dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[12][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_14dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_14dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[13][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_15dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_15dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[14][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_16dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_16dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[15][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_17dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_17dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[16][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_18dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_18dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[17][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_19dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_19dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[18][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;
    case AUD_SE_LOUDNESS_20dB:
        Loud_Value->e_loudness_mode= AUD_SE_LOUDNESS_20dB;
        for (i = 0; i < 6; i++)
        {
            Loud_Value->Loud_GAIN[i]=LOUD_GAIN[19][i];
            LOG(LOG_CTRLF, TEXT("Gain_Value = %d\n"), (u32)Loud_Value->Loud_GAIN[i]);
        }
        break;

    default:
        {
            LOG(LOG_CTRLF, TEXT("[LOUDNESS]First Parameter Error!! \n"));
            kfree(Loud_Value);
            return ;
        }
    }

    Op_SePost->u4OpCode=70;
    Op_SePost->pvData=(void*)Loud_Value;
    Op_SePost->u4DataSize=sizeof(AUD_SE_LOUDNESS_CTRL_INFO_T);
    Op_SePost->u1Type=AUD_SE_LOUDNESS;

    fgAudSeProcessOpCmd((void*)&OpCmd);
    kfree(Loud_Value);
    return ;
}

/************************************************************************************************
Function Name: AudCli_LoudNess_Svc
Function Description:  
input para:  
output para: 
Author: tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_LOUNDNESS_STR()     LOG(LOG_CLI,     \
    TEXT("[CLI]%d: LOUDNESS CLI:  \n")                    \
    TEXT("            Set loudness gain:   [0][gain: 0~20] \n")        \
    TEXT("            Print loudness gain: [1]\n"), AUD_CLI_LOUDNESS)
void AudCli_LoudNess_Svc(u32 u4Type, u32 u4Gain)
{
    switch(u4Type)
    {
    case 0:
        AudCli_LoudNess(u4Gain);
        break;
        
    case 1:
        AudCli_PrintLoudnessGain();
        break;
        
    default:
        AUD_CLI_LOUNDNESS_STR();
        break;
    }    
}

/************************************************************************************************
Function Name: AudCli_DacSel
Function Description:  
input para:  
output para: 
Author: tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_SET_DAC_TYPE_STR()        LOG(LOG_CLI,                 \
    TEXT("[CLI]%d: Set Dac Type: [OutCfg][DacType]\n")                    \
    TEXT("            OutCfg  >>> 0: Front, 1: Rear, 2: GPS, 3: FR, 4: SPDIF \n")    \
    TEXT("            DacType >>> 0: PWM, 1: EXT. \n"), AUD_CLI_SET_DAC_TYPE)
extern PAOUT_EXTPARAMS_T AudAout_GetHalParam(AUD_AOUT_DEVID eAoutID);
void AudCli_DacSel(AUD_CFG_ID eOut, AUD_DAC_TYPE_T eDacType, u32 u4Pin)
{
    if(ARG_INIT_VAL == eOut || ARG_INIT_VAL == eDacType)
    {
        AUD_CLI_SET_DAC_TYPE_STR();
    }
    else 
    {
        AUD_DAC_TYPE_SEL_T rDACSel;
        PAOUT_EXTPARAMS_T pParam = NULL;
        rDACSel.eDacType = eDacType;
        rDACSel.eOut = eOut;
        LOG(LOG_FEATURE, _T("eOut is %d, Dac is %d, u4Pin is %d.\r\n"),
            eOut, eDacType, u4Pin);
        if(AUD_DAC_EXT == eDacType)
        {
            if(AUD_FRONT == eOut)
            {
                pParam = AudAout_GetHalParam(AUDID_AOUT1);
                pParam->ePinMuxFsExtDac = u4Pin;
            }
            else
            {
                pParam = AudAout_GetHalParam(AUDID_AOUT2);
                pParam->ePinMuxRsExtDac = u4Pin;
            }
        }
        AudAout_DacTypeSet(&rDACSel);
    }
}

/************************************************************************************************
Function Name: AudCli_PrintVolGain
Function Description:  
input para:  
output para: 
Author: 
Data: 
Modify Data:
*************************************************************************************************/
#define AUD_CLI_PRINT_VOL_GAIN_STR()        LOG(LOG_CLI, \
        TEXT("[CLI]%d: Print Volume gain. \n"), \
        AUD_CLI_PRINT_VOL_GAIN)
extern AUD_DEC_CH_VOL_GAIN_T g_rFrnVolGain;
extern AUD_DEC_REAR_VOLUME_GAIN_INFO_T g_rRearVolGain;
void AudCli_PrintVolGain(u32 u4VolMode)
{
    LOG(LOG_CLI, TEXT("[CLI]Front Vol Gain = 0x%x\n"), g_rFrnVolGain.u4VolMaster);
    LOG(LOG_CLI, TEXT("[CLI]Rear Vol Gain = 0x%x\n"), g_rRearVolGain.u4RearVolGain);
    
    LOG(LOG_CLI, TEXT("[CLI]Left Vol Gain = 0x%x\n"),g_rFrnVolGain.u4VolL);
    LOG(LOG_CLI, TEXT("[CLI]Right Vol Gain = 0x%x\n"),g_rFrnVolGain.u4VolR);
    LOG(LOG_CLI, TEXT("[CLI]Center Vol Gain = 0x%x\n"),g_rFrnVolGain.u4VolC);
    LOG(LOG_CLI, TEXT("[CLI]Left Suround Vol Gain = 0x%x\n"),g_rFrnVolGain.u4VolSL);
    LOG(LOG_CLI, TEXT("[CLI]Right Suround Vol Gain = 0x%x\n"),g_rFrnVolGain.u4VolSR);
    LOG(LOG_CLI, TEXT("[CLI]Subwoofer Vol Gain = 0x%x\n"),g_rFrnVolGain.u4VolSW);
}

/************************************************************************************************
Function Name: AudCli_LinSet
Function Description:  
input para:  
output para: 
Author: 
Data: 
Modify Data:
*************************************************************************************************/
#define AUD_CLI_LINEIN_REAR_BYPASS_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: LineIn Rear Bypass [0: Front, 1: Rear][0: Off, 1: On][group: 0~5]. \n"), \
    AUD_CLI_LINEIN_REAR_BYPASS)
void AudCli_LinSet(u32 arg1, u32 arg2, u32 arg3)
{
    AUDIN_SET_ONOFF AudInOnOff = {0};
    if(ARG_INIT_VAL == arg1|| 
       ARG_INIT_VAL == arg2|| 
       ARG_INIT_VAL == arg3)
    {
        AUD_CLI_LINEIN_REAR_BYPASS_STR();
    }
    else 
    {
        AudInOnOff.lMode = arg1;
        AudInOnOff.fgAudInOnOff = arg2;
        AudInOnOff.eLineINGroupSel = arg3;

        DspCfgSetRearMediaType(0);
    }
}

/************************************************************************************************
Function Name: AudCliCfg_SpdifEnable
Function Description:  
input para:  
output para: 
Author: 
Data: 
Modify Data:
*************************************************************************************************/
#define AUD_CLI_SET_SPDIF_STR()        LOG(LOG_CLI, \
    TEXT("[CLI]%d: Set Spdif: [(SrcId)0: Aout1, 1: Aout2, 2: DVD, 3:GPS]. \n"), \
    AUD_CLI_SET_SPDIF)
void AudCliCfg_SpdifEnable(AUD_OUT_TYPE_T eSrcID)
{
    if(ARG_INIT_VAL == eSrcID)
    {
        AUD_CLI_SET_SPDIF_STR();
    }
    else 
    {
        AudCfg_SpdifEnable(eSrcID);
    }
}

/************************************************************************************************
Function Name: AudCliCfg_SpdifType
Function Description:  
input para:  
output para: 
Author: 
Data: 
Modify Data:
*************************************************************************************************/
#define AUD_CLI_SET_SPDIF_IEC_TYPE_STR()        LOG(LOG_CLI, \
        TEXT("[CLI]%d: Set Spdif Type: [(SpdifType)0: Off, 1: Raw, 2: Pcm]. \n"), \
        AUD_CLI_SET_SPDIF_IEC_TYPE)
void AudCliCfg_SpdifType(AUD_DEC_SPDIF_TYPE_T eSpdif)
{
    if(ARG_INIT_VAL == eSpdif)
    {
        AUD_CLI_SET_SPDIF_IEC_TYPE_STR();
    }
    else 
    {
        AudSetSpdif(eSpdif);
    }
}


/************************************************************************************************
Function Name: AudCliCfg_UseExtLdo
Function Description:  
input para:  
output para: 
Author: 
Data: 
Modify Data:
*************************************************************************************************/
#define AUD_CLI_EXC_PWMDAC_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Set PWMDAC [0: Internal, 1: External][AUD_DAC_CLASS_T]. \n"), \
    AUD_CLI_EXC_PWMDAC)
void AudCliCfg_UseExtLdo(bool fgUseExtLdo, AUD_DAC_CLASS_T eDacCls)
{
    if(ARG_INIT_VAL == fgUseExtLdo || ARG_INIT_VAL == eDacCls)
    {
        AUD_CLI_EXC_PWMDAC_STR();
    }
    else 
    {
        AudCfg_UseExtLdo(fgUseExtLdo, eDacCls);
    }
}

/************************************************************************************************
Function Name: AudCliCfg_UseExtLdo
Function Description:  
input para:  
output para: 
Author: 
Data: 
Modify Data:
*************************************************************************************************/
#define AUD_CLI_TEMP_TEST_STR()    LOG(LOG_CLI, \
    TEXT("[CLI]%d: Just test...... \n"), AUD_CLI_TEMP_TEST)
void AudCli_TempTest(u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    AUD_CLI_TEMP_TEST_STR();
}


/************************************************************************************************
Function Name: AudCli_SystemSvc
Function Description:  
input para:  
output para: 
Author:  tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_SYSTEM_GRP_STR()    LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: System group: Open log, Dsp state, Underrun, ASRC, UART ...\n"), AUD_CLI_SYSTEM_GRP)
void AudCli_SystemSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_OPEN_LOG:
        AudCliCmdLog(arg1);
        break;
        
    case AUD_CLI_CHECK_DSP_STATE:
        AudCliDispStates();
        break;

    case AUD_CLI_SHOW_DSP_STATUS:
        AudCliShowDspStatus();
        break;

    case AUD_CLI_SHOW_CONFIG:
        AudCliShowConfig();
        break;
        
    case AUD_CLI_MW_CMD:
        AudCliCmdCheckCmd(); 
        break;

    case AUD_CLI_UNDERRUN:
        AudCliUnderRunCounter(arg1);
        break;

    case AUD_CLI_ASRC:
        AudCliAsrcTest(arg1, arg2, arg3);
        break;
        
    case AUD_CLI_SHOW_VERSION:
        AudCliShowVersion();
        break;

    case AUD_CLI_SHOW_INTHIST:
        AudCliShowIntHistory(arg1);        
        break;

    case AUD_CLI_AP_DVP_INT:
        AudCliApDvpInt(arg1,arg2);
        break;

    case AUD_CLI_ERR_RECOVER_LOG:
        AudCliErrRcvyLog(arg1);
        break;

    case AUD_CLI_TEMP_TEST:
        AudCli_TempTest(arg1, arg2, arg3, pfilename);
        break;
        
    case AUD_CLI_SYSTEM_HELP:
    default:
        AUD_CLI_SYSTEM_GRP_STR();
        AUD_CLI_OPEN_LOG_STR();
        AUD_CLI_CHECK_DSP_STATE_STR();
        AUD_CLI_SHOW_DSP_STATUS_STR();
        AUD_CLI_SHOW_CONFIG_STR();
        AUD_CLI_MW_CMD_STR();
        AUD_CLI_UNDERRUN_STR();
        AUD_CLI_ASRC_STR();
        AUD_CLI_SHOW_VERSION_STR();
        AUD_CLI_SHOW_INTHIST_STR();
        AUD_CLI_AP_DVP_INT_STR();
        AUD_CLI_ADSP_ERR_RECOVER_LOG_STR();
        AUD_CLI_TEMP_TEST_STR();
        break;
    }
}

/************************************************************************************************
Function Name: AudCli_MemorySvc
Function Description:  
input para:  
output para: 
Author:  tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_MEMORY_GRP_STR()    LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: Memory group: read / write DRAM, SHM, SRAM ...\n"), AUD_CLI_MEMORY_GRP)
void AudCli_MemorySvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_R_CDRAM:
        AudCliComDram(arg1,arg2);
        break;

    case AUD_CLI_W_CDRAM:
        AudCliDspCommDramWrite(arg1,arg2);
        break;

    case AUD_CLI_R_SRAM:
        AudCliDspReadSram(arg1,arg2,arg3);
        break;

    case AUD_CLI_W_SRAM:
        AudCliDspWriteSram(arg1, arg2, arg3);
        break;

    case AUD_CLI_R_SHM:
        AudCliDspShareInfo(arg1);
        break;

    case AUD_CLI_W_SHM:
        AudCliDspShareInfoWrite(arg1, arg2, arg3, 0);
        break;

    case AUD_CLI_R_DRV:
        AudCliDrvRead(arg1,arg2);
        break;

    case AUD_CLI_W_DRV:
        AudCliDrvWrite(arg1,arg2);
        break;
        
    case AUD_CLI_DUMP_DRAM:
        AudCliDumpDspDram(arg1,arg2,arg3,pfilename);
        break;
    case AUD_CLI_AUD_WORKINGBUF:
        AudCliShowAudWorkingBuf(arg1, arg2);        
        break;
        
    case AUD_CLI_MEMORY_HELP:
    default:
        AUD_CLI_MEMORY_GRP_STR();
        AUD_CLI_R_CDRAM_STR();
        AUD_CLI_W_CDRAM_STR();
        AUD_CLI_R_SRAM_STR();
        AUD_CLI_W_SRAM_STR();
        AUD_CLI_R_SHM_STR();
        AUD_CLI_W_SHM_STR();
        AUD_CLI_R_DRV_STR();
        AUD_CLI_W_DRV_STR();
        AUD_CLI_DUMP_DRAM_STR();
        AUD_CLI_WORKINGBUF_STR();
        break;
    }
}

/************************************************************************************************
Function Name: AudCli_PostSvc
Function Description:  
input para:  
output para: 
Author:  tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_POST_GRP_STR()    LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: Post processing Group...\n"), AUD_CLI_POST_GRP)
void AudCli_PostSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_PP_BYPASS:
        AudCliByPassSE(arg1, arg2);
        break;

    case AUD_CLI_REVERB:
        AudCliReverbSt();
        break;
        
    case AUD_CLI_EQUALIZER:
        AudCliEqSt();
        break;
        
    case AUD_CLI_BASSM:
        AudCliBassMSt(arg1,arg2);
        break;
        
    case AUD_CLI_PROLOGICII:
        AudCliPl2St();
        break;
        
    case AUD_CLI_UPMIX:
        AudCli_Upmix(arg1, arg2);
        break;
        
    case AUD_CLI_CSII:
        AudCliCsiiSetting(arg1, arg2);
        break;
        
    case AUD_CLI_SPEC:
        AudCliSpectSt();
        break;
        
    case AUD_CLI_LOUDNESS:
        AudCli_LoudNess_Svc(arg1, arg2);
        break;
        
    case AUD_CLI_LR_MIX:
        vAudCliLRMixing(arg1);
        break;
        
    case AUD_CLI_TESTTONE:
        AudCliTestToneSvc(arg1, arg2, arg3);
        break;

    case AUD_CLI_EQ_ONE_BAND:
        AudCliEqOneBand(arg1, arg2);
        break;

    case AUD_CLI_EQ_IIR_COEF:
        AudCliEqSetIIRCoef(arg1, arg2, arg3);        
        break;

    case AUD_CLI_MVS:
        AudCliMvs(arg1);
        break;
        
    case AUD_CLI_ATS:
        AudCli_ATS(arg1, arg2, arg3, pfilename);
        break;        
        
    case AUD_CLI_ARM_EXT_PP:
      #if ((CONFIG_DRV_AUDIO_EXTERNAL_POST_PROC_SUPPORT)||(CONFIG_DRV_AUDIO_EXTERNAL_POST_PROC_ON_ARM1))        
        AudCliExtPP(arg1, arg2, arg3, pfilename);
      #endif
        break;
        
    case AUD_CLI_POST_HELP:
    default:
        AUD_CLI_POST_GRP_STR();
        AUD_CLI_PP_BYPASS_STR();
        AUD_CLI_REVERB_STR();
        AUD_CLI_EQUALIZER_STR();
        AUD_CLI_BASSM_STR();
        AUD_CLI_PROLOGICII_STR();
        AUD_CLI_UPMIX_STR();
        AUD_CLI_CSII_STR();
        AUD_CLI_SPEC_STR();
        AUD_CLI_LOUNDNESS_STR();
        AUD_CLI_LR_MIX_STR();
        AUD_CLI_TESTTONE_STR();
        AUD_CLI_EQ_ONE_BAND_STR();
        AUD_CLI_MVS_STR();
        break;
    }
}

/************************************************************************************************
Function Name: AudCli_AoutSvc
Function Description:  
input para:  
output para: 
Author:  tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_AOUT_GRP_STR()    LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: Aout group: Aout, IEC, DAC, SPDIF, PTS ...\n"), AUD_CLI_AOUT_GRP)
void AudCli_AoutSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_SET_FAOUT:
        AudCliSetFrontAout();
        break;
        
    case AUD_CLI_SET_RAOUT:
        AudCliSetRearAout();
        break;
        
    case AUD_CLI_AOUT_STATE:
        AudCliGetAoutSt(arg1, arg2);
        break;
        
    case AUD_CLI_EXC_AOUT:
        AudCliExangeAout(arg1);
        break;
    
    case AUD_CLI_EXC_IEC:
        AudCliExangeIEC(arg1);
        break;
        
    case AUD_CLI_SET_IEC_REG:
        AudCliDispIECRegisters();
        break;
        
    case AUD_CLI_SET_DAC_TYPE:
        AudCli_DacSel(arg1, arg2, arg3);
        break;
        
    case AUD_CLI_EXC_PWMDAC:
        AudCliCfg_UseExtLdo(arg1, arg2);
        break;
        
    case AUD_CLI_SET_SPDIF:
        AudCliCfg_SpdifEnable(arg1);
        break;

    case AUD_CLI_SEL_FRN_RERA:            
        AudCliSelFrnRear(arg1,arg2);
        break;
                
    case AUD_CLI_SET_SPDIF_IEC_TYPE:
        AudCliCfg_SpdifType(arg1);
        break;
                
    case AUD_CLI_AOUT_HELP:
    default:
        AUD_CLI_AOUT_GRP_STR();
        AUD_CLI_SET_FAOUT_STR();
        AUD_CLI_SET_RAOUT_STR();
        AUD_CLI_AOUT_STATE_STR();
        AUD_CLI_EXC_AOUT_STR();
        AUD_CLI_EXC_IEC_STR();
        AUD_CLI_SET_IEC_REG_STR();
        AUD_CLI_SET_DAC_TYPE_STR();
        AUD_CLI_EXC_PWMDAC_STR();
        AUD_CLI_SET_SPDIF_STR();
        AUD_CLI_SEL_FRN_RERA_STR();
        AUD_CLI_SET_SPDIF_IEC_TYPE_STR();
        break;
    }
}

/************************************************************************************************
Function Name: AudCli_VolumeSvc
Function Description:  
input para:  
output para: 
Author:  tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_VOLUME_GRP_STR()    LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: Volume group: Set vol gain, Get ch vol ...\n"), AUD_CLI_VOLUME_GRP)
void AudCli_VolumeSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_GET_OUTPUT_VOL:
        vAudCliGetOutputVol();
        break;
    
    case AUD_CLI_PRINT_VOL_GAIN:
        AudCli_PrintVolGain(arg1);
        break;
        
    case AUD_CLI_REAR_VOL_CTRL:
        AudCli_RearVol(arg1);
        break;

    case AUD_CLI_MODIFY_VOL:
        AudCliCmdUopVolume(arg1,arg2);
        break;

    case AUD_CLI_VOL_DETECT:
        AudCliCmdDetectVolThreshold(arg1, arg2, arg3);
        break;
        
    case AUD_CLI_VOL_SET:
        vAdspMasterVolumeGain(arg1);
        LOG(0, TEXT("========set aud volume gain test ======== volume is 0x%x\r\n"), arg1);
        break;
        
    case AUD_CLI_VOLUME_HELP:
    default:
        AUD_CLI_VOLUME_GRP_STR();
        AUD_CLI_GET_OUTPUT_VOL_STR();
        AUD_CLI_PRINT_VOL_GAIN_STR();
        AUD_CLI_REAR_VOL_CTRL_STR();
        AUD_CLI_MODIFY_VOL_STR();
        AUD_CLI_VOL_DETECT_STR();
        break;
    }
}

/************************************************************************************************
Function Name: AudCli_MediaSvc
Function Description:  
input para:  
output para: 
Author:  tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_MEDIA_GRP_STR()        LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: Media group: Dec 1/4, Line in, Gpsmix ...\n"), AUD_CLI_MEDIA_GRP)
void AudCli_MediaSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_DEC1_CTRL:

        break;

    case AUD_CLI_DEC4_CTRL:

        break;

    case AUD_CLI_GPSMIX_CTRL:
        AudCliGpxMixSvc(arg1);
        break;

    case AUD_CLI_LINEIN_REAR_BYPASS:
        AudCli_LinSet(arg1,arg2,arg3);
        break;

    case AUD_CLI_PLAYBACK_TIME:
        AudCliPlaybackTime();
        break;
        
    case AUD_CLI_MHL_TEST:
        AudCliMhlTest(arg1,arg2);
        break;
        
    case AUD_CLI_MEDIA_HELP:
    default:
        AUD_CLI_MEDIA_GRP_STR();
        AUD_CLI_GPSMIX_CTRL_STR();
        AUD_CLI_LINEIN_REAR_BYPASS_STR();
        AUD_CLI_PLAYBACK_TIME_STR();        
        AUD_CLI_MHL_TEST_STR();
        break;
    }
}

/************************************************************************************************
Function Name: AudCli_EsmPtsSvc
Function Description:  
input para:  
output para: 
Author:  tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
#define AUD_CLI_ESM_PTS_GRP_STR()    LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: ESM/PTS group: AU Info, Dump AFIFO, PTS Info ...\n"), AUD_CLI_ESM_PTS_GRP)
void AudCli_EsmPtsSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_PRINT_AU:
        AudCliPrintAu(arg1);
        break;

    case AUD_CLI_DUMP_AFIFO:
        AudCliDumpAfifo(arg1, pfilename);
        break;

    case AUD_CLI_DUMP_AOUT:
        AudCliDumpAout(arg1, pfilename);
        break;

    case AUD_CLI_ESM_STATE:
        AudCliEsmState(arg1, arg2);
        break;

    case AUD_CLI_GET_PTS_INFO:
        AudCliDspCLIGetPtsInfo();
        break;

    case AUD_CLI_GET_PTS_QUEUE:
        AudCliGetPtsQueue(arg1,arg2);
        break;

    case AUD_CLI_UPDATE_PTS_QUEUE:
        AudCliPtsUpdateQueue(arg1,arg2);    
        break;
    case AUD_CLI_SET_ADJUSTMIRACAST:
        AudCliAdjustMiracast(arg1,arg2);
        break;
        
    case AUD_CLI_ESM_PTS_HELP:
    default:
        AUD_CLI_ESM_PTS_GRP_STR();
        AUD_CLI_PRINT_AU_STR();
        AUD_CLI_DUMP_AFIFO_STR();
        AUD_CLI_DUMP_AOUT_STR();
        AUD_CLI_ESM_STATE_STR();
        AUD_CLI_GET_PTS_INFO_STR();
        AUD_CLI_GET_PTS_QUEUE_STR();
        AUD_CLI_UPDATE_PTS_QUEUE_STR();
        AUD_CLI_SET_ADJUSTMIRACAST_STR();
        break;
    }
}

void AudCli_AudioIoTest(u32 u4ModuleId, u32 arg1, u32 arg2, u32 arg3)
{
    switch(u4ModuleId)
    {
        case 0:
          LOG(0, TEXT("======== aud mic in test ======== %d, %d, %d, %d \n"), u4ModuleId, arg1, arg2, arg3);

          AudMicTest(arg1, arg2, arg3);
          break;
        
        case 1:
          LOG(0, TEXT("======== aud pcm test ======== %d, %d, %d, %d \n"), u4ModuleId, arg1, arg2, arg3);

          AudPcmTest(arg1, arg2, arg3);
          break;

        case 2:
          //LOG(0, TEXT("======== aud lin test ======== %d, %d, %d, %d \n"), u4ModuleId, arg1, arg2, arg3);

          //AudLinTest(arg1, arg2, arg3);
          break;

        case 3:
          LOG(0, TEXT("======== aud bypss test ======== %d, %d, %d, %d \n"), u4ModuleId, arg1, arg2, arg3);
          
          AudBypsTest(arg1, arg2, arg3);
          break;

        case 4:
          LOG(0, TEXT("======== aud aout test ======== %d, %d, %d, %d \n"), u4ModuleId, arg1, arg2, arg3);

          AudAoutTest(arg1, arg2, arg3);
          break;

        case 5:
          //LOG(0, TEXT("======== aud mlin test ======== %d, %d, %d, %d \n"), u4ModuleId, arg1, arg2, arg3);
          
          //AudMlinTest(arg1, arg2, arg3);
          break;

        case 6:
          LOG(0, TEXT("======== aud io test ======== %d, %d, %d, %d \n"), u4ModuleId, arg1, arg2, arg3);
          
          AudIOTest(arg1, arg2, arg3);
          break;


        default:
          break;
    }
}

#define AUD_CLI_AUDIN_GRP_STR()    LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: Audion in group: audio config Info, .\n"), AUD_CLI_AUDIN_GRP)

void AudCli_AudinSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_AUDIN_PARAMS:
        Aud_Linein_ShowParams(arg1);
        break;
        
    default:
        AUD_CLI_AUDIN_GRP_STR();
        break;
    }

}

extern AUDIN_INFO_T* g_prAudInNfyInfo;
void Audmhl_ShowInfo(void)
{
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1AudinPauseStatus 0x%x ===\r\n"), g_prAudInNfyInfo->u1AudinPauseStatus);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1AudinLockStatus 0x%x ===\r\n"), g_prAudInNfyInfo->u1AudinLockStatus);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1AudinChStatus 0x%x ===\r\n"), g_prAudInNfyInfo->u1AudinChStatus);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1AudinSampleRate 0x%x ===\r\n"), g_prAudInNfyInfo->u1AudinSampleRate);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1AudinSwitchOK 0x%x ===\r\n"), g_prAudInNfyInfo->u1AudinSwitchOK);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1AudinOnOffOK 0x%x ===\r\n"), g_prAudInNfyInfo->u1AudinOnOffOK);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1HdmiRxINT 0x%x ===\r\n"), g_prAudInNfyInfo->u1HdmiRxINT);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1SpdifAudinType 0x%x ===\r\n"), g_prAudInNfyInfo->u1SpdifAudinType);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1SpdifRawDataType 0x%x ===\r\n"), g_prAudInNfyInfo->u1SpdifRawDataType);
    LOG(LOG_CLI, TEXT("=== MHL AUD CLI u1AudinUSBNo 0x%x ===\r\n"), g_prAudInNfyInfo->u1AudinUSBNo);
}

#define AUD_CLI_MHL_GRP_STR()    LOG(LOG_CLI, \
    TEXT("[CLI_GRP]%d: Audion in group: audio config Info, .\n"), AUD_CLI_AUDIN_GRP)

void AudCli_MHLSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    switch(u4Type)
    {
    case AUD_CLI_MHL_INFO:
        Audmhl_ShowInfo();
        break;
        
    default:
        AUD_CLI_MHL_GRP_STR();
        break;
    }

}


/************************************************************************************************
Function Name: AudCliCmdSvc
Function Description:  
input para:  
output para: 
Author:  tongfa
Data: 2012/02/21
Modify Data:
*************************************************************************************************/
void AudCliCmdSvc(u32 u4Type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
    u32 u4GrpType = u4Type / 100 * 100;
    
    LOG(10, TEXT("======== AudCliCmdSvc ========\n"));
    LOG(10, TEXT(">>> u4Type: %d\n"), u4Type);
    LOG(10, TEXT(">>> arg1: %d\n"), arg1);
    LOG(10, TEXT(">>> arg2: %d\n"), arg2);
    LOG(10, TEXT(">>> arg3: %d\n"), arg3);
    LOG(10, TEXT(">>> filename: %s\n"), (s8 *)pfilename);
    LOG(10, TEXT("==============================\n"));
    
    switch(u4GrpType)
    {
    case AUD_CLI_SYSTEM_GRP:
        AudCli_SystemSvc(u4Type, arg1, arg2, arg3, pfilename);
        break;

    case AUD_CLI_MEMORY_GRP:    
        AudCli_MemorySvc(u4Type, arg1, arg2, arg3, pfilename);
        break;

    case AUD_CLI_POST_GRP:
        AudCli_PostSvc(u4Type, arg1, arg2, arg3, pfilename);
        break;

    case AUD_CLI_AOUT_GRP:
        AudCli_AoutSvc(u4Type, arg1, arg2, arg3, pfilename);
        break;

    case AUD_CLI_VOLUME_GRP:
        AudCli_VolumeSvc(u4Type, arg1, arg2, arg3, pfilename);
        break;

    case AUD_CLI_MEDIA_GRP:
        AudCli_MediaSvc(u4Type, arg1, arg2, arg3, pfilename);
        break;

    case AUD_CLI_ESM_PTS_GRP:
        AudCli_EsmPtsSvc(u4Type, arg1, arg2, arg3, pfilename);
        break;
    case AUD_CLI_AUDIN_GRP:
        AudCli_AudinSvc(u4Type, arg1, arg2, arg3, pfilename);
        break;
        
    case AUD_CLI_MHL_GRP:
        AudCli_MHLSvc(u4Type, arg1, arg2, arg3, pfilename);
        break;
    
    case AUD_CLI_GRP_HELP:
        LOG(LOG_CLI, TEXT("============ Audio CLI Cmd Group ============\n"));
        AUD_CLI_SYSTEM_GRP_STR();
        AUD_CLI_MEMORY_GRP_STR();
        AUD_CLI_POST_GRP_STR();
        AUD_CLI_AOUT_GRP_STR();
        AUD_CLI_VOLUME_GRP_STR();
        AUD_CLI_MEDIA_GRP_STR();
        AUD_CLI_ESM_PTS_GRP_STR();
        LOG(LOG_CLI, TEXT("=============================================\n"));
        break;
        
    default:
        LOG(LOG_CLI, TEXT("=============== Audio CLI Cmd ===============\n"));
        AudCli_SystemSvc(AUD_CLI_SYSTEM_HELP, arg1, arg2, arg3, pfilename);
        LOG(LOG_CLI, TEXT("---------------------------------------------\n"));
        AudCli_MemorySvc(AUD_CLI_MEMORY_HELP, arg1, arg2, arg3, pfilename);
        LOG(LOG_CLI, TEXT("---------------------------------------------\n"));
        AudCli_PostSvc(AUD_CLI_POST_HELP, arg1, arg2, arg3, pfilename);
        LOG(LOG_CLI, TEXT("---------------------------------------------\n"));
        AudCli_AoutSvc(AUD_CLI_AOUT_HELP, arg1, arg2, arg3, pfilename);
        LOG(LOG_CLI, TEXT("---------------------------------------------\n"));
        AudCli_VolumeSvc(AUD_CLI_VOLUME_HELP, arg1, arg2, arg3, pfilename);
        LOG(LOG_CLI, TEXT("---------------------------------------------\n"));
        AudCli_MediaSvc(AUD_CLI_MEDIA_HELP, arg1, arg2, arg3, pfilename);
        LOG(LOG_CLI, TEXT("---------------------------------------------\n"));
        AudCli_EsmPtsSvc(AUD_CLI_ESM_PTS_HELP, arg1, arg2, arg3, pfilename);
        LOG(LOG_CLI, TEXT("=============================================\n"));
        break;
    }
}

/************************************************************************************************
 Function Name: AudSetCliCmd
 Function Description:cli enter funciton
 input para:eAudcli:cli type,arg1,arg2,arg3,arg4:each cli need para,pfilename:need filename
 output para: NULL
 Author:fei.zhu
 Data:2010.12.12
 Modify Data:
 2011.3.7     modify format according coding standard
 *************************************************************************************************/
void  AudSetCliCmd(AUD_DEC_CLI_TYPE eAudCli,u32 arg1,u32 arg2,u32 arg3,u32 arg4,const s8 **pfilename)
{
    switch(eAudCli)
    {
    case AUD_DEC_CLI_TEST:
        AudCliTest();
        break;

    case AUD_DEC_CLI_HELP:
        AudCliHelp();
        break;

    //yucai yang
    case AUD_DEC_CLI_AUD_LOG:
        AudCliCmdLog(arg1);
        break;
      //yucai yang

      //yucai yang
    case AUD_DEC_CLI_ST:
        AudCliDispStates();
        break;

    case AUD_DEC_CLI_V:
        AudCliCmdUopVolume(arg1,arg2);
        break;

      //yucai yang
    case AUD_DEC_CLI_CMD_LOG:
        AudCliCmdLog(arg1);
        break;

    case AUD_DEC_CLI_CHECK:
        AudCliCmdCheckCmd();
        break;

    case AUD_DEC_CLI_DUMP_AFIFO:
        AudCliDumpAfifo(arg1,pfilename);
        break;
    //yucai yang
    case AUD_DEC_CLI_DSP_CM:
        AudCliComDram(arg1,arg2);
        break;

    case AUD_DEC_CLI_DSP_SH:
        AudCliDspShareInfo(arg1);
        break;

    case AUD_DEC_CLI_DSP_SHW:
        AudCliDspShareInfoWrite(arg1,arg2,arg3,arg4);
        break;

    case AUD_DEC_CLI_UR:
        AudCliUnderRunCounter(arg1);
        break;

    case AUD_DEC_CLI_Q:
        AudCliShowDspStatus();
        break;

    case AUD_DEC_CLI_DSP_CFG:
        AudCliShowConfig();
        break;

    case AUD_DEC_CLI_IECREGS:
        AudCliDispIECRegisters();
        break;

    case AUD_DEC_CLI_PI:
        AudCliDspCLIGetPtsInfo();
        break;

    case AUD_DEC_CLI_PL:
        AudCliGetPtsQueue(arg1,arg2);
        break;

    case AUD_DEC_CLI_PU:
        AudCliPtsUpdateQueue(arg1,arg2);
        break;

    case AUD_DEC_CLI_WCM:
        AudCliDspCommDramWrite(arg1,arg2);
        break;

    case AUD_DEC_CLI_DSP_R:
        AudCliDspReadSram(arg1,arg2,arg3);
        break;

    case AUD_DEC_CLI_DSP_W:
        AudCliDspWriteSram(arg1,arg2,arg3);
        break;

    case AUD_DEC_CLI_ESM_ST:
        LOG(LOG_CTRLF, TEXT("*****[_Aud Cli Cmd] need to add\r\n"));
        //AudCliShowEsmStatus(arg1);
        break;

    case AUD_DEC_CLI_AUD_PA:
        AudCliPrintAu(arg1);
        break;

    case AUD_DEC_CLI_AUD_ST:
        AudCliDispStates();
        break;

    case AUD_DEC_CLI_AUD_V:
        AudCliCmdUopVolume(arg1,arg2);
        break;

    case AUD_DEC_CLI_AUD_BYPASS:
        AudCliByPassSE(arg1, 1);
        //RETAILMSG(1, (TEXT("*****[_Aud Cli Cmd] need to add\r\n")));
        break;

    case AUD_DEC_CLI_DRV_R:
        LOG(LOG_CTRLF, TEXT("*****[_Aud Cli Cmd] need to add\r\n"));
        break;

    case AUD_DEC_CLI_DRV_W:
        LOG(LOG_CTRLF, TEXT("*****[_Aud Cli Cmd] need to add\r\n"));
        break;

    case AUD_DEC_CLI_DIGOUT_EXA:
        AudCliExangeAout(arg1);
        break;

    case AUD_DEC_CLI_DIGOUT_EXI:
        AudCliExangeIEC(arg1);
        break;

    case AUD_DEC_CLI_DUMP_AOUT:
        AudCliDumpAout(arg1,pfilename);
        break;

    case AUD_DEC_CLI_UART_SWITCH: //(aud_cli_demo by mtk40292)
        if ( TRUE == AudCliUartSwitch((bool)arg1))
        {
            LOG(LOG_CTRLF, TEXT("uart switch ok! \r\n"));
        }
        break;

    case AUD_DEC_CLI_RB_ST:
        AudCliReverbSt();
        break;

    case AUD_DEC_CLI_PL2_ST:
        AudCliPl2St();
        break;

    case AUD_DEC_CLI_EQ_ST:
        AudCliEqSt();
        break;

    case AUD_DEC_CLI_SPECT_ST:
        AudCliSpectSt();
        break;

    case AUD_DEC_CLI_BASSM_ST:
        AudCliBassMSt(arg1,arg2);
        break;

    case AUD_DEC_CLI_ASRC_ST:
        AudCliAsrcTest(arg1,arg2,arg3);
        break;

    case AUD_DEC_CLI_TESTTONE_SELTYPE:
        LOG(LOG_CTRLF, TEXT(">>[AudCliTTSelType] 1eTTType = %x\n"),(u32)arg1);
        LOG(LOG_CTRLF, TEXT(">>[AudCliTTSelType] 1eTTOut = %x\n"),(u32)arg2);
        vAudTestToneSetType(arg1,arg2);
        break;

    case AUD_DEC_CLI_TESTTONE_SELCHANNEL:
        LOG(LOG_CTRLF, TEXT(">>[AudCliTTSelChannel] 2eTTType = %x\n"),(u32)arg1);
        LOG(LOG_CTRLF, TEXT(">>[AudCliTTSelChannel] 2eTTOut = %x\n"),(u32)arg2);
        vAudTestToneSetChannel(arg1,arg2);
        break;

    case AUD_DEC_CLI_TESTTONE_ONOFF:
        LOG(LOG_CTRLF, TEXT(">>[AudCliTTOnOff] 1fgEnable = %x\n"),(u32)arg1);
        LOG(LOG_CTRLF, TEXT(">> [AudCliTTOnOff] 1eTTOut = %x\n"),(u32)arg2);
        vAudTestToneSwitch(arg1,arg2);
        break;

    case AUD_DEC_CLI_TESTTONE_SELFRNREAR:
        AudCliSelFrnRear(arg1,arg2);
        break;

    case AUD_DEC_CLI_GPSMIX_PLAY_CMD:
        LOG(LOG_CTRLF, TEXT(">> Enter Gpsmix Play Cmd CLI\n"));
        _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_CTRL_START);
        break;

    case AUD_DEC_CLI_GPSMIX_STOP_CMD:
        LOG(LOG_CTRLF, TEXT(">> Enter Gpsmix Stop Cmd CLI\n"));
        _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_CTRL_STOP);        
        break;

    case AUD_DEC_CLI_GPSMIX_PAUSE_CMD:
        LOG(LOG_CTRLF, TEXT(">> Enter Gpsmix Pause Cmd CLI\n"));
        _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_CTRL_PAUSE);
        break;

    case AUD_DEC_CLI_GPSMIX_RESUME_CMD:
        LOG(LOG_CTRLF, TEXT(">> Enter Gpsmix Resume Cmd CLI\n"));
        _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_CTRL_RESUME);
        break;

    case AUD_DEC_CLI_CSII:
        AudCliCsiiSetting(arg1, arg2);
        break;
    case AUD_DEC_CLI_EXT_LIN_TEST:

        break;

    case AUD_DEC_CLI_PWMDAC_EXT_LDO:
        AudCfg_UseExtLdo(arg1, arg2);
        break;

    case AUD_DEC_CLI_SPDIF:
        AudCfg_SpdifEnable(arg1);
        break;

    case AUD_DEC_CLI_REAR_VOL_CONTROL:
        AudCli_RearVol(arg1);
        break;
    case AUD_DEC_CLI_UPMIX:
        LOG(LOG_CTRLF, TEXT("*****[_Aud Cli Cmd] AUD_DEC_CLI_UPMIX\n"));
        AudCli_Upmix(arg1, arg2);
        break;

    case AUD_DEC_CLI_LOUDNESS:
        LOG(LOG_CTRLF, TEXT("*****[_Aud Cli Cmd] AUD_DEC_CLI_LOUDNESS\n"));
        AudCli_LoudNess(arg1);
        break;

    case AUD_DEC_CLI_DACSEL:
        AudCli_DacSel(arg1, arg2, arg3);
        break;

    case AUD_DEC_CLI_PRINT_VOL_GAIN:
        AudCli_PrintVolGain(arg1);
        break;

    case AUD_DEC_CLI_PRINT_TT_INFO:
        AudCli_PrintTTInfo();
        break;

    case AUD_DEC_CLI_PRINT_UPMIX_GAIN:
        AudCli_Upmix(0, 0);
        break;

    case AUD_DEC_CLI_PRINT_LOUDNESS_GAIN:
        AudCli_PrintLoudnessGain();
        break;

    case AUD_DEC_CLI_LIN_REAR_BYPASS:
        AudCli_LinSet(arg1,arg2,arg3);
        break;

    case AUD_DEC_CLI_GET_OUTPUT_VOL:
    {
        AUD_OUTPUT_VOL rChVol = {0,0,0,0,0,0,0,0,0,0,0};
        AUD_OUTPUT_VOL *prChVol = &rChVol;
        vAudGetOutputVol(prChVol);
    }
        break;

    case  AUD_DEC_CLI_IO_TEST:
        AudCli_AudioIoTest(arg1, arg2, arg3, arg4);
        break;
        
    case AUD_DEC_CLI_DEBUG:
        AudCliCmdSvc(arg1, arg2, arg3, arg4, pfilename);
        break;
            
    case AUD_DEC_CLI_SET_LRMIX:
        vAudLRMixing(arg1);
        break;
        
    default:
        LOG(LOG_CTRLF, TEXT("no this cli cmd,please try it! \r\n"));
        break;
    }
}

#ifdef __linux__
EXPORT_SYMBOL(AudSetCliCmd);
#endif
