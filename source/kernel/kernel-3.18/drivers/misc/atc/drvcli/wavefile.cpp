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
*[File]             wavefile.cpp
*[Author]               mtk40004
*[Description]
*    implementation for  wavefile class.    
******************************************************************************/

#include "wavefile.h"

#include <linux/types.h>

#include "cli_common.h"
WaveFile::WaveFile()
{
    m_hFile = INVALID_HANDLE_VALUE ;
    m_u4Pos = 0;
    m_u4Size = 0;
    m_fgCache = TRUE;
    m_fgIsRead = TRUE;
};


WaveFile::~WaveFile()
{
    if (INVALID_HANDLE_VALUE != m_hFile)
        Close();
};




u32 WaveFile::Close()
{
    if ((INVALID_HANDLE_VALUE != m_hFile) && !m_fgIsRead)
    {
        if (m_fgCache)
        {
            m_fgFlushFinished = TRUE;
            SetEvent(m_hEvent);
            while(m_hThread)
                Sleep(1);
            free(m_ppbTbl[0]);
            CloseHandle(m_hEvent);
        }
        SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN );
        WriteHeader();
        FlushFileBuffers(m_hFile);
    }
    if (m_hFile != INVALID_HANDLE_VALUE )
        CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE ;
    m_u4Pos = 0;
    m_rWaveHdr.datasize = 0;
    return (MMSYSERR_NOERROR);
}


u32 WaveFile::Open()
{
    u32 dwLen;
    if (INVALID_HANDLE_VALUE != m_hFile)
        return (MMSYSERR_ERROR);
    m_hFile = CreateFile(m_szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE )
    {
        RETAILMSG(1, (TEXT("[WaveFile] OPen file %s failed.\r\n"), m_szFileName));
        return (MMSYSERR_ERROR);
    }
    if (ReadFile(m_hFile, &m_rWaveHdr, sizeof(WaveHeader), &dwLen, NULL))
    {
        if (m_rWaveHdr.wFormatTag == WAVE_FORMAT_PCM)
        {
            m_u4Pos = sizeof(WaveHeader);
            m_fgIsRead = TRUE;
            RETAILMSG(0, (TEXT("[AudioTest] Sample Rate(%d), Bit Per Sample(%d), Channels(%d) AvgSize(%d) DataSize(%d) \r\n"), 
                m_rWaveHdr.nSamplesPerSec, m_rWaveHdr.wBitsPerSample, m_rWaveHdr.nChannels, m_rWaveHdr.nAvgBytesPerSec, m_rWaveHdr.datasize));
            return (MMSYSERR_NOERROR);
        }
    }
    CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
    return MMSYSERR_ERROR;
}

static u32 WINAPI 
WaveFile_FlushThread(
                     void *pPddContext
                     )
{
    WaveFile *prWavefile = (WaveFile *)pPddContext;
    prWavefile->FlushThread();

    return 0;
}

u32 WaveFile::SetCacheMode(bool fgCache)
{
    if (INVALID_HANDLE_VALUE != m_hFile)
        return (MMSYSERR_ERROR);
    m_fgCache = fgCache;
    return MMSYSERR_NOERROR;
}

u32 WaveFile::Create(WAVEFORMATEX *prFmt)
{
    if (INVALID_HANDLE_VALUE != m_hFile)
        return (MMSYSERR_ERROR);
    m_rWaveHdr.wBitsPerSample = prFmt->wBitsPerSample;
    m_rWaveHdr.nChannels  = prFmt->nChannels;
    m_rWaveHdr.nSamplesPerSec  = prFmt->nSamplesPerSec;

    m_fgIsRead = FALSE;
    m_u4CacheIdx = 0;


    if (m_fgCache)
    {
        s32 i4Loop;
        m_ppbTbl[0] = (u8 *) malloc(MAX_CACHE_BUF * CACHE_BUF_SIZE);
        for (i4Loop=1; i4Loop < MAX_CACHE_BUF; i4Loop++)
        {
            m_ppbTbl[i4Loop] = m_ppbTbl[i4Loop-1] + CACHE_BUF_SIZE;
        }
        for (i4Loop=0; i4Loop < MAX_CACHE_BUF; i4Loop++)
            m_fgHasData[i4Loop] = FALSE;
        m_u4FlushIdx = 0;
        m_u4WriteIdx = 0;
        m_pbWritePtr = m_ppbTbl[0];
        m_fgFlushFinished = FALSE;
        m_hEvent = CreateEvent(0, FALSE, FALSE, NULL);
        m_hThread = CreateThread(NULL, 0, WaveFile_FlushThread, (void *)this, 0, NULL);
    }
    else
    {
        m_hFile = CreateFile(m_szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS , 0, NULL);
        if (m_hFile == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1, (TEXT("[WaveFile] Create %s Failure(0x%x)!\r\n"), m_szFileName, GetLastError()));
            return (MMSYSERR_ERROR);
        }
        m_rWaveHdr.datasize = 0;
        WriteHeader();
        m_u4Pos = sizeof(WaveHeader);
    }

    return MMSYSERR_NOERROR;
}



u32 WaveFile::FlushThread()
{
    u32 code = 0;
    u32 u4Size;
    m_hFile = CreateFile(m_szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS , 0, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(1, (TEXT("[WaveFile] Create %s Failure(0x%x)!\r\n"), m_szFileName, GetLastError()));
        return (MMSYSERR_ERROR);
    }
    m_rWaveHdr.datasize = 0;
    WriteHeader();
    m_u4Pos = sizeof(WaveHeader);

    while (!m_fgFlushFinished)
    {
        code = WaitForSingleObject(m_hEvent, INFINITE);
        if (code != WAIT_OBJECT_0)
            break;
        while(m_fgHasData[m_u4FlushIdx])
        {
            if (!WriteFile(m_hFile, m_ppbTbl[m_u4FlushIdx], CACHE_BUF_SIZE, &u4Size, NULL))
            {
                RETAILMSG(1, (TEXT("[FlushThread]  Failure(0x%x)!\r\n"),  GetLastError()));
                return (0);
            }
            SendToUart((s16 *)(m_ppbTbl[m_u4FlushIdx]), CACHE_BUF_SIZE);
            m_u4Pos += u4Size;
            m_rWaveHdr.datasize += u4Size;
            m_fgHasData[m_u4FlushIdx] = FALSE;
            m_u4FlushIdx ++;
            if (m_u4FlushIdx >= MAX_CACHE_BUF)
                m_u4FlushIdx = 0;
        }
    }
    m_hThread = NULL;
    return MMSYSERR_NOERROR;
}




u32 WaveFile::GetFormat(WAVEFORMATEX *prFmt)
{
    if (m_hFile && prFmt)
        memcpy(prFmt, &m_rWaveHdr.wFormatTag, sizeof(WAVEFORMATEX) - sizeof(u16));
    return MMSYSERR_NOERROR;
}


u32 WaveFile::SetFileName(char *szFileName)
{
    s32 i4Loop = 0;
    while(szFileName[i4Loop])
    {
        m_szFileName[i4Loop] = (s8) szFileName[i4Loop];
        i4Loop ++;
    }
    m_szFileName[i4Loop] = 0;

    return MMSYSERR_NOERROR;
}


u32 WaveFile::SetFileName(s8 *szFileName)
{
    _tcscpy(m_szFileName, szFileName);
    return MMSYSERR_NOERROR;
}


u32 WaveFile::GetDataSize()
{
    return m_rWaveHdr.datasize;
}


u32 WaveFile::GetCurrentPosition()
{
    if (INVALID_HANDLE_VALUE != m_hFile)
        return 0;
    return (m_u4Pos - sizeof(WaveHeader));
}


u32 WaveFile::SetCurrentPosition(u32 u4Pos)
{
    if ((INVALID_HANDLE_VALUE == m_hFile) || 
        ( u4Pos > m_rWaveHdr.datasize) ||
        !m_fgIsRead)
        return (MMSYSERR_ERROR);
    m_u4Pos = u4Pos + sizeof(WaveHeader);
    SetFilePointer(m_hFile, m_u4Pos, NULL, FILE_BEGIN );
    return MMSYSERR_NOERROR;
}


u32 WaveFile::ReadData(void * lpData, u32 u4Size)
{
    if ((INVALID_HANDLE_VALUE == m_hFile) || !m_fgIsRead || (u4Size > m_rWaveHdr.datasize))
        return 0;
    if ((m_u4Pos - sizeof(WaveHeader) + u4Size) > m_rWaveHdr.datasize)
    {
        m_u4Pos = sizeof(WaveHeader);
        SetFilePointer(m_hFile, m_u4Pos, NULL, FILE_BEGIN );
    }
    if (!ReadFile(m_hFile, lpData, u4Size, &u4Size, NULL))
    {
        ASSERT(0);
        return (0);
    }
    m_u4Pos += u4Size;

    return (u4Size);
}


u32 WaveFile::WriteData(void * lpData, u32  u4Size)
{
    if ( m_fgIsRead || ((INVALID_HANDLE_VALUE == m_hFile) && !m_fgCache))
        return 0;
    if (m_fgCache)
    {
        u32 u4CpySize = u4Size;
        u8 *pbSrc = (u8 *)lpData;
        while(u4CpySize)
        {
            u32 u4OnceCpy = CACHE_BUF_SIZE - (m_pbWritePtr - m_ppbTbl[m_u4WriteIdx]);
            if (m_fgHasData[m_u4WriteIdx] == TRUE)
            {
                RETAILMSG(1, (TEXT("[0x%x->WriteData]  Cache is full\r\n"),  this));
                return (u4Size - u4CpySize);
            }
            if (u4OnceCpy > u4CpySize)
                u4OnceCpy = u4CpySize;
            memcpy(m_pbWritePtr, pbSrc, u4OnceCpy);
            u4CpySize -= u4OnceCpy;
            m_pbWritePtr += u4OnceCpy;
            pbSrc += u4OnceCpy;
            if ((m_pbWritePtr - m_ppbTbl[m_u4WriteIdx]) >= CACHE_BUF_SIZE)
            {
                m_fgHasData[m_u4WriteIdx] = TRUE;
                m_u4WriteIdx ++;
                if (m_u4WriteIdx >= MAX_CACHE_BUF)
                    m_u4WriteIdx = 0;
                m_pbWritePtr = m_ppbTbl[m_u4WriteIdx];
                SetEvent(m_hEvent);
            }
        }
        return (u4Size - u4CpySize);
    }
    if (!WriteFile(m_hFile, lpData, u4Size, &u4Size, NULL))
    {
        RETAILMSG(1, (TEXT("[WriteData]  Failure(0x%x)!\r\n"),  GetLastError()));
        ASSERT(0);
        return (0);
    }

    m_u4Pos += u4Size;
    m_rWaveHdr.datasize += u4Size;

    return u4Size;
}

void WaveFile::WriteHeader()
{
    u32 dwLen;
    if ((INVALID_HANDLE_VALUE == m_hFile) || m_fgIsRead)
        return;
    m_rWaveHdr.riff[0] = 'R';
    m_rWaveHdr.riff[1] = 'I';
    m_rWaveHdr.riff[2] = 'F';
    m_rWaveHdr.riff[3] = 'F';
    m_rWaveHdr.filesize = m_rWaveHdr.datasize+ 36;
    m_rWaveHdr.wave[0] = 'W';
    m_rWaveHdr.wave[1] = 'A';
    m_rWaveHdr.wave[2] = 'V';
    m_rWaveHdr.wave[3] = 'E';
    m_rWaveHdr.fmt[0] = 'f';
    m_rWaveHdr.fmt[1] = 'm';
    m_rWaveHdr.fmt[2] = 't';
    m_rWaveHdr.fmt[3] = ' ';
    m_rWaveHdr.fmtsize = 0x10;
    m_rWaveHdr.wFormatTag = 0x01;
    m_rWaveHdr.nAvgBytesPerSec = m_rWaveHdr.nSamplesPerSec * m_rWaveHdr.nChannels * (m_rWaveHdr.wBitsPerSample / 8);
    m_rWaveHdr.nBlockAlign = m_rWaveHdr.wBitsPerSample / 8  * m_rWaveHdr.nChannels;;
    m_rWaveHdr.data[0] = 'd';
    m_rWaveHdr.data[1] = 'a';
    m_rWaveHdr.data[2] = 't';
    m_rWaveHdr.data[3] = 'a';

    WriteFile(m_hFile, &m_rWaveHdr, sizeof(WaveHeader), &dwLen, NULL);
    ASSERT(dwLen == sizeof(WaveHeader));
}

u32 WaveFile::SendToUart(s16 *pi2Data, u32 u4Size)
{
    u4Size &= 0xFFFFFFFC;
    while (u4Size)
    {
        m_ai2LCache[m_u4CacheIdx]= *pi2Data ++;
        u4Size -= 2;
        //        if ( 2 == m_rWaveHdr.nChannels )
        {
            m_ai2RCache[m_u4CacheIdx]= *pi2Data ++;
            u4Size -= 2;
        }
        m_u4CacheIdx ++;
        if (160 <= m_u4CacheIdx )
        {
            u16 u4Gain = 0;
            UartWriteBytes((u8 *)m_ai2LCache, 320);
            UartWriteBytes((u8 *)m_ai2RCache, 320);
            UartWriteBytes((u8 *)&u4Gain, 2);
            UartWriteBytes((u8 *)&u4Gain, 2);
            m_u4CacheIdx  = 0;
        }
    }
    return (0);
}




