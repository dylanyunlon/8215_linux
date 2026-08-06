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
*[File]             pcmstream.cpp
*[Author]               mtk40004
*[Description]
*    implementation for  pcmstream class.   
******************************************************************************/

#include "pcmstream.h"

#include <linux/types.h>

#define STRM_INFO 0
PcmStream::PcmStream()
{
    m_fgOpened = FALSE;
    m_u4Size = 0;
}


PcmStream::~PcmStream()
{
    m_fgOpened = FALSE;
    m_u4Size = 0;
}


FileStream::FileStream()
{
    m_pbBuf = NULL;
    m_u4BufState = NULL;
}

FileStream::~FileStream()
{
}


u32 FileStream::Open()
{
    u32 i;
    for (i=0; i < m_u4BufNum; i++)
    {
        if (!m_pbBuf[i])
        {
            m_pbBuf[i] = (s8 *)new short[(m_u4BufSize + 1) >>1];
            m_u4BufState[i] = BUF_FREE;
        }
    }
    return (MMSYSERR_NOERROR);
}

u32 FileStream::Close()
{
    u32 i;
    for (i=0; i < m_u4BufNum; i++)
    {
        if (m_pbBuf[i])
        {
            delete (short *)m_pbBuf[i];
            m_pbBuf[i] = NULL;
            m_u4BufState[i] = BUF_FREE;
        }
    }
    return (MMSYSERR_NOERROR);
}


WaveFileStream::WaveFileStream()
{
    s32 i;

    m_u4BufNum = PLAYED_BUF_NUM;
    m_pbBuf = (s8 **)new u32[PLAYED_BUF_NUM];
    m_u4BufState = new u32[PLAYED_BUF_NUM];

    for (i=0; i < (s32)m_u4BufNum; i++)
    {
        m_pbBuf[i] = NULL;
        m_u4BufState[i] = BUF_FREE;
    }
    m_u4BufSize = 0;
}


WaveFileStream::~WaveFileStream()
{
    Close();
    if (m_pbBuf)
    delete(m_pbBuf);
    m_pbBuf = NULL;
    if (m_u4BufState)
    delete(m_u4BufState);
    m_u4BufState = NULL;
}



u32 WaveFileStream::Open()
{
    if (!m_fgOpened)
    {
        if (MMSYSERR_NOERROR != m_rWaveFile.Open())
        {
            return (MMSYSERR_ERROR);
        }
        m_fgOpened = TRUE;
        m_rWaveFile.GetFormat(&m_rWfx);
        m_u4BufSize = m_rWfx.nAvgBytesPerSec;
        m_u4UsedLen = 0;
        m_u4Size = m_rWaveFile.GetDataSize();
        FileStream::Open();
        m_fgEndOfStream = FALSE;

    }
    return (MMSYSERR_NOERROR);
}

u32 WaveFileStream::Close()
{
    if (m_fgOpened)
    {
        m_rWaveFile.Close();
        FileStream::Close();
        m_fgOpened = FALSE;
        m_fgEndOfStream = FALSE;
    }
    return (MMSYSERR_NOERROR);
}

s8 * WaveFileStream::GetDataBuffer(u32 &dwSize)
{
    u32 dwReadLen = m_u4Size - m_u4UsedLen;
    int u4Idx;
    dwSize = 0;
    if (dwReadLen > m_u4BufSize)
        dwReadLen = m_u4BufSize;
    s8 * pbRet = NULL;

    for (u4Idx=0; u4Idx< (int)m_u4BufNum; u4Idx ++)
    {
        if (m_u4BufState[u4Idx] == BUF_FREE)
        {
            pbRet = m_pbBuf[u4Idx];
            break;
        }
    }

    if (!pbRet)
        return NULL;

    dwSize = m_rWaveFile.ReadData(pbRet, dwReadLen);
    RETAILMSG(0 && (m_rWfx.nSamplesPerSec == 44100), (TEXT("[AudioTest] GetDataBuffer dwReadLen(%d) dwSize(%d) m_u4UsedLen(%d) m_u4BufSize(%d)\r\n"),
        dwReadLen, dwSize, m_u4UsedLen, m_u4BufSize ));
    m_u4UsedLen += dwSize;
    if (m_u4UsedLen >= m_u4Size)
    {
        m_fgEndOfStream = TRUE;
    }
    if (dwSize)
    {
        m_u4BufState[u4Idx] = BUF_IN_USED;
        return (pbRet);
    }
    return (NULL);
}


u32 WaveFileStream::ReleaseBuffer(s8 * pbBuffer)
{
    for (int i=0; i< (int)m_u4BufNum; i ++)
    {
        if (m_pbBuf[i] == pbBuffer)
        {
            m_u4BufState[i] = BUF_FREE;
            break;
        }
    }
    return (MMSYSERR_NOERROR);
}

u32 WaveFileStream::Restart()
{
    if (m_fgOpened)
    {
        m_u4UsedLen = 0;
        m_rWaveFile.SetCurrentPosition(0);
    }

    for (int i=0; i< (int)m_u4BufNum; i ++)
    {
        m_u4BufState[i] = BUF_FREE;
    }
    m_fgEndOfStream = FALSE;
    return (MMSYSERR_NOERROR);
}



RecordStream::RecordStream()
{
    s32 i;

    m_u4BufNum = TOTAL_BUF_NUM;
    m_pbBuf = (s8 * *)new u32[TOTAL_BUF_NUM];
    m_u4BufState = new u32[TOTAL_BUF_NUM];

    for (i=0; i < (s32)m_u4BufNum; i++)
    {
        m_pbBuf[i] = NULL;
        m_u4BufState[i] = BUF_FREE;
    }
    m_u4BufSize = 0;
}


RecordStream::~RecordStream()
{
    StopRecord();
    delete(m_pbBuf);
    delete(m_u4BufState);
}

u32 RecordStream::Open()
{
    if (m_fgOpened)
    {
        m_u4ReadID = 0x10000;
        return (MMSYSERR_NOERROR);
    }
    return (MMSYSERR_ERROR);
}

u32 RecordStream::Close()
{
    m_u4ReadID = 0x10000;
    return (MMSYSERR_NOERROR);
}



s8 *  RecordStream::GetDataBuffer(u32 &dwSize)
{
    s8 * pbRet = NULL;
    u32 u4NextID;
    if (m_u4ReadID < 0x1000)
        u4NextID = (m_u4ReadID + 1) & 0xFFFF;
    else
    {
        u4NextID = m_u4WriteID;
        if (!u4NextID)
            return (NULL);
        u4NextID --;
        if (u4NextID)
            u4NextID --;
        u4NextID &= 0xFFFF;
    }


    for (int i=0; i< (int)m_u4BufNum; i ++)
    {
        if ((m_u4BufState[i] & 0xFFFF) == BUF_HAS_RECORDED)
        {
            if (((m_u4BufState[i] >> 16)& 0xFFFF) == u4NextID)
            {
                m_u4BufState[i] &= 0xFFFF0000;
                m_u4BufState[i] |= BUF_IN_PLAYED;
                m_u4ReadID = u4NextID;
                return ((s8 *)m_pbBuf[i]);
            }
        }
    }
    return (NULL);
}


u32 RecordStream::ReleaseBuffer(s8 * pbBuffer)
{
    for (int i=0; i< (int)m_u4BufNum; i ++)
    {
        if (pbBuffer == (s8 *)m_pbBuf[i])
        {
            if ((m_u4BufState[i] & 0xFFFF) == BUF_IN_PLAYED)
            {
                m_u4BufState[i] = BUF_FREE;
            }
            break;
        }
    }
    return (MMSYSERR_NOERROR);
}



u32 RecordStream::StartRecord(WAVEFORMATEX *prWfx)
{
    if (!m_fgOpened)
    {
        memcpy(&m_rWfx, prWfx, sizeof(WAVEFORMATEX));
        if (MMSYSERR_NOERROR != m_rWaveFile.Create(prWfx))
        {
            return (MMSYSERR_ERROR);
        }
        m_fgOpened = TRUE;
        m_u4BufSize = m_rWfx.nAvgBytesPerSec;
        m_u4Size = 0;
        FileStream::Open();
        m_fgEndOfStream = FALSE;
        m_u4WriteID = 0;
        m_u4ReadID  = 0;
    }
    return (MMSYSERR_NOERROR);
}


u32 RecordStream::StopRecord()
{
    if (m_fgOpened)
    {
        m_rWaveFile.Close();
        FileStream::Close();
        m_fgOpened = FALSE;
        m_fgEndOfStream = FALSE;
    }
    return (MMSYSERR_NOERROR);
}


s8 *  RecordStream::GetBuffer(u32 &dwSize)
{
    u32 u4ID = 0x10000;
    u32 iIdx = -1;

    for (int i=0; i< (int)m_u4BufNum; i ++)
    {
        if ((m_u4BufState[i] == BUF_FREE))
        {
            m_u4BufState[i] = BUF_IN_USED;
            dwSize = m_u4BufSize;
            return ((s8 *)m_pbBuf[i]);
        }
        if (BUF_HAS_RECORDED == (m_u4BufState[i]& 0xFFFF))
        {
            if (u4ID > (m_u4BufState[i] >> 16))
            {
                iIdx = i;
                u4ID = (m_u4BufState[i] >> 16);
            }
        }
    }
    if (iIdx != -1)
    {
        m_u4BufState[iIdx] = BUF_IN_USED;
        dwSize = m_u4BufSize;
        return ((s8 *)m_pbBuf[iIdx]);
    }

    return (NULL);
}


u32 RecordStream::TransferData(s8 * pbBuffer, u32 dwSize)
{
    for (int i=0; i< (int)m_u4BufNum; i ++)
    {
        if (m_pbBuf[i] == pbBuffer)
        {
            m_u4BufState[i] = BUF_HAS_RECORDED | (m_u4WriteID << 16);
            m_u4WriteID ++;
            m_u4Size += dwSize;
            m_rWaveFile.WriteData(pbBuffer, dwSize);
            return (MMSYSERR_NOERROR);
        }
    }
    return (MMSYSERR_ERROR);
}


u32 RecordStream::Restart()
{
    m_u4ReadID = 0x10000;
    return (MMSYSERR_NOERROR);
}





