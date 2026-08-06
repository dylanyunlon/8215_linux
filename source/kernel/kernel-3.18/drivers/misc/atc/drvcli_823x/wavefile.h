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
*[File]            wavefile.h
*[Author]        mtk40004
*[Description]
*    Interface of wavefile class
******************************************************************************/
#ifndef __WAVEFILE_H_
#define __WAVEFILE_H_

#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"
#include "mmsystem.h"

#include <linux/types.h>

#pragma pack(push, 1)
typedef struct {
    u8     riff[4];          // "RIFF"
    u32   filesize;         // File size - 8
    u8     wave[4];          // "WAVE"
    u8     fmt[4];           // "fmt "
    u32   fmtsize;          // 0x10
    u16   wFormatTag;       // 0x01 -> PCM
    u16   nChannels;        // Channels
    u32   nSamplesPerSec;  // Sampling Rate (samples per second)
    u32   nAvgBytesPerSec; // Average Bytes per second
    u16   nBlockAlign;
    u16   wBitsPerSample;   // Bits per sample
    u8     data[4];          // "data"
    u32   datasize;         // Data Size(Byte)
} WaveHeader;

#pragma pack (pop)
#define MAX_CACHE_BUF 40
#define CACHE_BUF_SIZE 32000 // 1 seconds data

class WaveFile 
{
public:
    WaveFile();
    ~WaveFile();
public:
    u32 Close();
    u32 Open();
    u32 Create(WAVEFORMATEX *prFmt);
    u32 GetFormat(WAVEFORMATEX *prFmt);
    u32 SetFileName(char *szFileName);
    u32 SetFileName(s8 *szFileName);
    u32 GetFileName(s8 szFile[])
    {
        _tcscpy(szFile, m_szFileName);
        return (MMSYSERR_NOERROR);
    };
    u32 GetDataSize();
    u32 SetCacheMode(bool fgCache);
    bool IsCacheMode()
    {
        return (m_fgCache);
    }
    u32 GetCurrentPosition();
    u32 SetCurrentPosition(u32 u4Pos);
    u32 ReadData(void * lpData, u32  u4Size);
    u32 WriteData(void * lpData, u32  u4Size);

    u32 FlushThread();
    u32 SendToUart(s16 *pi2Data, u32 u4Size);

private:
    void WriteHeader();
    HANDLE m_hFile;
    WaveHeader m_rWaveHdr;
    s8 m_szFileName[MAX_PATH];
    u32 m_u4Pos;
    u32 m_u4Size;
    bool m_fgIsRead;
    bool m_fgCache;
    u8 *m_ppbTbl[MAX_CACHE_BUF];
    u8 *m_pbWritePtr;
    volatile bool m_fgHasData[MAX_CACHE_BUF];
    volatile u32 m_u4FlushIdx;
    volatile u32 m_u4WriteIdx;
    HANDLE volatile m_hEvent;
    HANDLE volatile m_hThread;
    volatile bool m_fgFlushFinished;
    s16 m_ai2LCache[160];
    s16 m_ai2RCache[160];
    u32 m_u4CacheIdx;
};


#endif // __WAVEFILE_H_


