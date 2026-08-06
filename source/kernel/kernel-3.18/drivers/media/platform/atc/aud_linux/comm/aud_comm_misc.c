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




#include "aud_oal.h"
#include "aud_comm_misc.h"


//====================================================//
    #define CodeSight_AudMisc_BufferData
//====================================================//

void AudMisc_BufferData_Read(u32 u4SAdr, u32 u4Size)
{  
    u32 i;
    u8 *pbBuf;
    pbBuf = (u8 *)u4SAdr;

    AUDLOG_NO_PREFIX(ALOG_INFO, T("[Read Buffer Data] SAdr(0x%x) Size(%d)>>>>>>>>>>>>>>> \r\n"),  (u32)u4SAdr, (s32)u4Size); 
    for (i = 0; i < u4Size; i++)
    {
        if(i % 16 == 0) {
            AUDLOG_NO_PREFIX(ALOG_INFO, T("\n0x%8X:  "), (u32)pbBuf); 
        } 
        AUDLOG_NO_PREFIX(ALOG_INFO, T("%2X "), *pbBuf);  
        pbBuf++;
    }
    AUDLOG_NO_PREFIX(ALOG_INFO, T("\n[Read Buffer Data]  End! <<<<<<<<<<<<<<<<<<  \r\n"));   
}


void AudMisc_BufferData_Draw(u32 u4SAdr, u32 u4Size, u32 u4BW, u32 u4CutBit, u32 u4CutSpace)
{  
    u32 i, j;
    u8 *pbBuf = (u8 *)u4SAdr;

    u32 u4Max = (0x1 << u4BW);
    u32 u4Boundary = u4Max >> 1;
    u32 u4ByteNum = u4BW >> 3;

    u32 u4SpaceLoop;
    u32 u4Value;
    
    AUDLOG_NO_PREFIX(ALOG_INFO, T("[Draw Buffer Data] SAdr(0x%x) Size(%d)>>>>>>>>>>>>>>> \r\n"),  (u32)u4SAdr, (s32)u4Size); 
    for (i = 0; i < u4Size; i += u4ByteNum)
    {
        u4Value = 0;
        for (j = 0; j < u4BW; j += 8) {
            u4Value +=  (*pbBuf++) << j;
        }      
        if (u4Value < u4Boundary) {
            u4Value += u4Boundary;
        } else {
            u4Value = u4Boundary - (u4Max - u4Value);
        } 
        
        u4SpaceLoop = (u4Value >> u4CutBit) - u4CutSpace;
        AUDLOG_NO_PREFIX(ALOG_INFO, T("0x%8x"), (u32)u4Value);      
        while(u4SpaceLoop--)
        {      
            AUDLOG_NO_PREFIX(ALOG_INFO, T(" "));
        }
        AUDLOG_NO_PREFIX(ALOG_INFO, T("*        \n"));
    }
    AUDLOG_NO_PREFIX(ALOG_INFO, T("\n[Draw Buffer Data]  End! <<<<<<<<<<<<<<<<<<  \r\n"));   
}



//====================================================//
    #define CodeSight_AudMisc_CopyData
//====================================================//

#define AUD_READ_BUF_DATA(i4Data, prSrc, prSrcBW, prDstBW)  \
    {\
        u32 i = 0; \
        i4Data = 0; \
        for (i = 0; i < prSrcBW; i += 8) { \
            i4Data += (*prSrc++) << i;\
        }\
        if (prDstBW > prSrcBW) { \
            i4Data <<= prDstBW - prSrcBW;\
        } else { \
            i4Data >>= prSrcBW - prDstBW;\
        } \
    }

#define AUD_WRITE_BUF_DATA(i4Data, prDst, prDstBW)  \
    { \
        u32 i = 0; \
        for (i = 0; i < prDstBW; i += 8) { \
            *prDst++ = (i4Data >> i) & 0xFF;\
        } \
    }


void AudMisc_CopyData_File2Buf(PAUD_COPY_DATA_T prCopy, u32 u4Param)
{   
    u8 *pSrc  = (u8 *)prCopy->u4SrcSAdr;
    u8 *pDstL = (u8 *)prCopy->u4DstSAdr;
    u8 *pDstR = pDstL + prCopy->u4DstChBufSz; 
    s32 i4Data;

    while(prCopy->u4CopySamples)
    {
        AUD_READ_BUF_DATA(i4Data, pSrc, prCopy->u4SrcBW, prCopy->u4DstBW)

        AUD_WRITE_BUF_DATA(i4Data, pDstL, prCopy->u4DstBW)

        if (prCopy->u4SrcChn == 2) {           
            AUD_READ_BUF_DATA(i4Data, pSrc, prCopy->u4SrcBW, prCopy->u4DstBW)
        }

        if (prCopy->u4DstChn == 2) {
            AUD_WRITE_BUF_DATA(i4Data, pDstR, prCopy->u4DstBW)
        }
        
        prCopy->u4CopySamples--;
    }    
}

    
void AudMisc_CopyData_Buf2File(PAUD_COPY_DATA_T prCopy, u32 u4Param)
{
    u8 *pDst = (u8 *)prCopy->u4DstSAdr;
    u8 *pSrcL = (u8 *)prCopy->u4SrcSAdr;
    u8 *pSrcR = pSrcL + prCopy->u4SrcChBufSz;
    s32 i4Data; 
    
    while(prCopy->u4CopySamples)
    {       
        AUD_READ_BUF_DATA(i4Data, pSrcL, prCopy->u4SrcBW, prCopy->u4DstBW)

        AUD_WRITE_BUF_DATA(i4Data, pDst, prCopy->u4DstBW)

        if (prCopy->u4SrcChn == 2) {           
            AUD_READ_BUF_DATA(i4Data, pSrcR, prCopy->u4SrcBW, prCopy->u4DstBW)
        }

        if (prCopy->u4DstChn == 2) {
            AUD_WRITE_BUF_DATA(i4Data, pDst, prCopy->u4DstBW)
        }
        
        prCopy->u4CopySamples--;
    }  
}


void AudMisc_CopyData(PAUD_COPY_DATA_T prCopy, u32 u4Param)
{
    u32 i, u4DstSAdr, u4SrcSAdr;
    u32 u4Chn = (prCopy->u4DstChn < prCopy->u4SrcChn) ? prCopy->u4DstChn : prCopy->u4SrcChn;
    for (i = 0; i < u4Chn; i++)
    {
        u4DstSAdr = prCopy->u4DstSAdr + prCopy->u4DstChBufSz * i;
        u4SrcSAdr = prCopy->u4SrcSAdr + prCopy->u4SrcChBufSz * i;
        x_memcpy((void *)u4DstSAdr, (void *)u4SrcSAdr, prCopy->u4CopySamples * (prCopy->u4DstBW >> 3));
    }
}


void AudMisc_CopyData_Mgr(PAUD_DATA_BUF_T prDst, PAUD_DATA_BUF_T prSrc, PFN_COPY_DATA pfnCopy, u32 u4Param)
{
    AUD_COPY_DATA_T rCopy;
    u32 i, j, u4RP, u4WP;
    u32 au4TranSamples[2], au4CopySamples[2];   

    u32 u4DstSamples = AUD_BYTE2SAMPLE(prDst->u4DataSize, prDst->u4BW);
    u32 u4SrcSamples = AUD_BYTE2SAMPLE(prSrc->u4DataSize, prSrc->u4BW);
    u32 u4TotalCopySamples = AUD_MIN(u4DstSamples, u4SrcSamples);
    
    rCopy.u4DstChBufSz = prDst->u4ChBufSz;
    rCopy.u4SrcChBufSz = prSrc->u4ChBufSz;
    rCopy.u4DstChn = prDst->u4Chn;
    rCopy.u4SrcChn = prSrc->u4Chn;
    rCopy.u4DstBW  = prDst->u4BW;      
    rCopy.u4SrcBW  = prSrc->u4BW;      

    prSrc->u4DataSize -= AUD_SAMPLE2BYTE(u4TotalCopySamples, prSrc->u4BW);
    u4RP = prSrc->u4DataOff;
    
    if (u4TotalCopySamples >= AUD_BYTE2SAMPLE(prSrc->u4ChBufSz - prSrc->u4DataOff, prSrc->u4BW))
    {
        au4TranSamples[0] = AUD_BYTE2SAMPLE(prSrc->u4ChBufSz - prSrc->u4DataOff, prSrc->u4BW);
        au4TranSamples[1] = u4TotalCopySamples - au4TranSamples[0];
        prSrc->u4DataOff  = AUD_SAMPLE2BYTE(au4TranSamples[1], prSrc->u4BW);
    }
    else 
    {
        au4TranSamples[0] = u4TotalCopySamples;
        au4TranSamples[1] = 0;
        prSrc->u4DataOff += AUD_SAMPLE2BYTE(au4TranSamples[0], prSrc->u4BW);
    }

    for (i = 0; i < 2; i++)
    {
        u32 u4CopySamples = au4TranSamples[i];
        prDst->u4DataSize -= AUD_SAMPLE2BYTE(u4CopySamples, prDst->u4BW);
        if (!u4CopySamples) {
            break;
        }
        
        rCopy.u4SrcSAdr = prSrc->u4VirSAdr + u4RP;           
        u4WP = prDst->u4DataOff; 
        
        if (u4CopySamples >= AUD_BYTE2SAMPLE(prDst->u4ChBufSz - prDst->u4DataOff, prDst->u4BW)) 
        {
            au4CopySamples[0] = AUD_BYTE2SAMPLE(prDst->u4ChBufSz - prDst->u4DataOff, prDst->u4BW);
            au4CopySamples[1] = u4CopySamples - au4CopySamples[0];
            prDst->u4DataOff  = AUD_SAMPLE2BYTE(au4CopySamples[1], prDst->u4BW);
        } 
        else
        {
            au4CopySamples[0] = u4CopySamples;
            au4CopySamples[1] = 0;
            prDst->u4DataOff += AUD_SAMPLE2BYTE(au4CopySamples[0], prDst->u4BW);
        }
        
        for (j = 0; j < 2; j++)
        { 
            rCopy.u4CopySamples = au4CopySamples[j];
            if (!rCopy.u4CopySamples) {
                break;
            }

            rCopy.u4DstSAdr = prDst->u4VirSAdr + u4WP;
            pfnCopy(&rCopy, u4Param);
            
            u4WP = 0;   //dst buffer rollback
        }

        u4RP = 0;       //src buffer rollback
    }
}


//====================================================//
    #define CodeSight_AudMisc_FreeBuf
//====================================================//

u32 AudMisc_FifoFreeSize_Get(u32 u4Wp, u32 u4Rp, u32 BufLen)
{
    u32 u4FreeSize;
    
    if (u4Rp >= u4Wp)
    {
        u4FreeSize = u4Rp - u4Wp;
    }
    else if (u4Rp < u4Wp)
    {
        u4FreeSize = BufLen - u4Wp + u4Rp;
    }

    return u4FreeSize;
}

u32 AudMisc_FifoDataSize_Get(u32 u4Wp, u32 u4Rp, u32 BufLen)
{
    u32 u4UsedSize;
    
    if (u4Rp > u4Wp)
    {
        u4UsedSize = BufLen - u4Rp + u4Wp;
    }
    else if (u4Rp <= u4Wp)
    {
        u4UsedSize = u4Wp - u4Rp;
    }

    return u4UsedSize;
}


