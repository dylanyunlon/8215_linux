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

/******************************************************************************
*[File]                 aud_comm_file.c
*[Author]               tongfa.luo@autochips.com
*[Description]
*       
*[Copyright]
*       
******************************************************************************/

#include "aud_oal.h"
#include "aud_comm_file.h"

typedef struct 
{ 
    FILE_CLS_PUB rPub;
        
    FILE_CLS_TYPE eType;    
    void * hFile;
    bool fgRead;
    AUD_DATA_BUF_T rBuf;   
    
    u32 u4ReadLoops;
    
}FILE_CLS, *PFILE_CLS;


//=======================================//
    #define CodeSight_AudFile_Phy
//=======================================//

static void PhyFile_AllocBuffer(PFILE_CLS prThis, u32 u4Chn, u32 u4ChBufSz, u32 u4BW)
{
    uintptr_t u4PhyAddr;

    prThis->rBuf.u4Chn = u4Chn;
    prThis->rBuf.u4BW = u4BW;
    prThis->rBuf.u4ChBufSz = AUD_ALIGNMENT_MASK(u4ChBufSz, u4BW >> 3);   
    prThis->rBuf.u4VirSAdr = AudOS_Memory_Alloc(u4Chn * u4ChBufSz, 0, &u4PhyAddr);
}


static bool PhyFile_Open(void * pThis, bool fgRead, void * pParams)
{
    bool fgRet = TRUE;

    PFILE_CLS prThis = (PFILE_CLS)pThis;
    PFILE_PHY_PARAMS_T prParams =(PFILE_PHY_PARAMS_T)pParams;
    
    prThis->fgRead = fgRead;  
    prThis->hFile = AudOS_File_Open(prThis->hFile, fgRead, prParams->szFileName);
   
    PhyFile_AllocBuffer(prThis, prParams->u4Chn, prParams->u4ChBufSz, prParams->u4BW);
   
    COMMLOG_DBG((T("[PhyFile(0x%x)]Open: fgRead(%d), ChSize(%d) Chn(%d) BW(%d) SAdr(0x%lx)\r\n"), (u32)prThis,
        (s32)prThis->fgRead, (s32)prThis->rBuf.u4ChBufSz, (s32)prThis->rBuf.u4Chn, (s32)prThis->rBuf.u4BW, (uintptr_t)prThis->rBuf.u4VirSAdr));
    
    return (fgRet);
}


static bool PhyFile_Close(void * pThis)
{
    bool fgRet = TRUE;

    PFILE_CLS prThis = (PFILE_CLS)pThis;
    AudOS_Memory_Free(&prThis->rBuf.u4VirSAdr);
    prThis->hFile = AudOS_File_Close(prThis->hFile);
    
    return (fgRet);
}


static bool PhyFile_Read(void * pThis, PAUD_DATA_BUF_T prDst)
{
    u32 u4Size, u4DstSamples, u4SrcSamples, u4ChSamples;
    PFILE_CLS prThis = (PFILE_CLS)pThis;
    
    PAUD_DATA_BUF_T prSrc = &prThis->rBuf;
    bool fgRet = TRUE;

    if (!prThis->fgRead)
    {
        COMMLOG_ERR((T("[PhyFile] This file is not for read! \n")));
        fgRet = FALSE;
        goto EXIT;
    }

    u4SrcSamples = AUD_BYTE2SAMPLE(prSrc->u4ChBufSz, prSrc->u4BW);
    while (prDst->u4DataSize)
    {    
        u4DstSamples = AUD_BYTE2SAMPLE(prDst->u4DataSize, prDst->u4BW);
        u4ChSamples = AUD_MIN(u4DstSamples, u4SrcSamples);

        u4Size =  AUD_SAMPLE2BYTE(u4ChSamples * prSrc->u4Chn, prSrc->u4BW); 
        u4Size = AudOS_File_Read(prThis->hFile, (void *)prThis->rBuf.u4VirSAdr, u4Size);

        if (u4Size)   
        {
            prThis->rBuf.u4DataOff = 0;
            prThis->rBuf.u4DataSize = u4Size / prSrc->u4Chn;            
            AudMisc_CopyData_Mgr(prDst, &prThis->rBuf, AudMisc_CopyData_File2Buf, 0);
        }
        else
        {
            fgRet = FALSE;
            break;
        }
    }
    
EXIT:
    return (fgRet);  
}


static bool PhyFile_Write(void * pThis, PAUD_DATA_BUF_T prSrc)
{
    u32 u4Size, u4DstSamples, u4SrcSamples, u4ChSamples;
    PFILE_CLS prThis = (PFILE_CLS)pThis;
    
    PAUD_DATA_BUF_T prDst = &prThis->rBuf;
    bool fgRet = TRUE;

    if (prThis->fgRead)
    {
        COMMLOG_ERR((T("[PhyFile] This file is not for write! \n")));
        fgRet = FALSE;
        goto EXIT;
    }

    u4DstSamples = AUD_BYTE2SAMPLE(prDst->u4ChBufSz, prDst->u4BW);
    while (prSrc->u4DataSize)
    {           
        u4SrcSamples = AUD_BYTE2SAMPLE(prSrc->u4DataSize, prSrc->u4BW);
        u4ChSamples = AUD_MIN(u4DstSamples, u4SrcSamples);
        u4Size =  AUD_SAMPLE2BYTE(u4ChSamples * prDst->u4Chn, prDst->u4BW); 
        
        prDst->u4DataOff = 0;
        prDst->u4DataSize = u4Size / prDst->u4Chn;
        AudMisc_CopyData_Mgr(prDst, prSrc, AudMisc_CopyData_Buf2File, 0);

        AudOS_File_Write(prThis->hFile, (void *)prDst->u4VirSAdr, u4Size);
    } 

EXIT:
    return (fgRet);
}


//=======================================//
    #define CodeSight_AudFile_Vir
//=======================================//

static bool VirFile_Open(void * pThis, bool fgRead, void * pvParams)
{
    bool fgRet = TRUE;  
    u32 i, u4VirAddr, u4PhyAddr;
    PFILE_CLS prThis = (PFILE_CLS)pThis;
    
    PAUD_DATA_BUF_T prBuf = &prThis->rBuf;
    PFILE_VIR_PARAMS_T prParams = (PFILE_VIR_PARAMS_T)pvParams;
        
    prThis->fgRead = fgRead;
    prThis->u4ReadLoops = 0;    
    
    prBuf->u4Chn = prParams->u4Chn;
    prBuf->u4BW = prParams->u4BW; 
    prBuf->u4ChBufSz = AUD_ALIGNMENT_MASK(prParams->u4ChBufSz, prBuf->u4BW >> 3);
    prBuf->u4VirSAdr = AudOS_Memory_Alloc(prBuf->u4ChBufSz * prBuf->u4Chn, 0, &u4PhyAddr);
    
    if (prThis->fgRead) 
    {   
        prThis->u4ReadLoops = prParams->u4ReadLoops;
        for (i = 0; i < prBuf->u4Chn; i++)
        {
            u4VirAddr = prBuf->u4VirSAdr + prBuf->u4ChBufSz * i;
            x_memcpy((void *)u4VirAddr, (void *)prParams->u4TblSAdr, prBuf->u4ChBufSz);
        }       
    }

    COMMLOG_DBG((T("[VirFile(0x%x)]Open: Read(%d, %d) ChSz(%d) Chn(%d) BW(%d) SAdr(0x%x, 0x%x)\r\n"), (u32)prThis, 
            (s32)prThis->fgRead, (s32)prThis->u4ReadLoops, (s32)prBuf->u4ChBufSz, (s32)prBuf->u4Chn, (s32)prBuf->u4BW, (u32)prBuf->u4VirSAdr, (u32)u4PhyAddr)); 
    
    prBuf->u4DataOff = 0;
    prBuf->u4DataSize = prThis->rBuf.u4ChBufSz;
   
    return (fgRet);
}


static bool VirFile_Close(void * pThis)
{
    bool fgRet = TRUE;
    PFILE_CLS prThis = (PFILE_CLS)pThis;
    
    if (!prThis->fgRead) {
        AudOS_Memory_Free(&prThis->rBuf.u4VirSAdr);
    }

    return (fgRet);
}


static bool VirFile_Read(void * pThis, PAUD_DATA_BUF_T prDst)
{
    bool fgRet = TRUE;
    PFILE_CLS prThis = (PFILE_CLS)pThis;

    if (!prThis->fgRead)
    {
        COMMLOG_ERR((T("[VirFile] This Vir file is not for read! \n")));
        fgRet = FALSE;
        goto EXIT;
    }

    //loop copy table data to buffer
    while (prDst->u4DataSize >= prThis->rBuf.u4ChBufSz)
    {
        prThis->rBuf.u4DataOff = 0;
        prThis->rBuf.u4DataSize = prThis->rBuf.u4ChBufSz;
        AudMisc_CopyData_Mgr(prDst, &prThis->rBuf, AudMisc_CopyData, 0);

        if (!prThis->u4ReadLoops || !(--prThis->u4ReadLoops))  
        {
            fgRet = FALSE;
            break;
        }
    }

EXIT:
    return (fgRet);
}


static bool VirFile_Write(void * pThis, PAUD_DATA_BUF_T prSrc)
{
    bool fgRet = TRUE;
    PFILE_CLS prThis = (PFILE_CLS)pThis;

    if (prThis->fgRead)
    {
        COMMLOG_ERR((T("[VirFile] This Vir file is not for write! \n")));
        fgRet = FALSE;
        goto EXIT;
    }

    if (prThis->rBuf.u4DataOff + prSrc->u4DataSize > prThis->rBuf.u4ChBufSz)
    {
        fgRet = FALSE;
        COMMLOG_DBG((T("[VirFile] Backup buffer is full! \n")));
        goto EXIT;
    }
        
    AudMisc_CopyData_Mgr(&prThis->rBuf, prSrc, AudMisc_CopyData, 0);

EXIT:    
    return (fgRet);
}


//=======================================//
    #define CodeSight_AudFile_Create
//=======================================//

static u32 File_Delete(void * pThis)
{
    PFILE_CLS prThis = (PFILE_CLS)pThis;
    
    COMMLOG_DBG((T("Delete File[0x%x]  \n"), (u32)prThis));
    AUD_CLASS_DELETE();

    return (AUD_RET_OK);
}


PFILE_CLS_PUB File_New(FILE_CLS_TYPE eType)
{
    PFILE_CLS prThis = AUD_CLASS_NEW(FILE_CLS);

    if (prThis) 
    {    
        COMMLOG_DBG((T("New File[0x%x] Type(%d)! \n"), (u32)prThis, eType)); 
        
        prThis->eType = eType;
        prThis->hFile = (void *)AUD_FILE_INVALID_HANDLE;
        
        prThis->rPub.Delete = File_Delete;
        if (prThis->eType == PHY_FILE)
        {
            prThis->rPub.Open = PhyFile_Open;
            prThis->rPub.Close = PhyFile_Close;
            prThis->rPub.Read = PhyFile_Read;
            prThis->rPub.Write = PhyFile_Write;
        } 
        else
        {
            prThis->rPub.Open = VirFile_Open;
            prThis->rPub.Close = VirFile_Close;
            prThis->rPub.Read = VirFile_Read;
            prThis->rPub.Write = VirFile_Write;
        }
    }
    
    return ((PFILE_CLS_PUB)prThis);
}


