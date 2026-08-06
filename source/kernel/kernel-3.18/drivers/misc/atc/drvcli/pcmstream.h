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
*[File]            pcmstream.h
*[Author]        mtk40004
*[Description]
*    Interface of pcmstream class
******************************************************************************/
#ifndef __PCMSTREAM_H_
#define __PCMSTREAM_H_

#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"
#include "mmsystem.h"

#include <linux/types.h>

#include "wavefile.h"

class PcmStream
{
public:
    PcmStream();
    ~PcmStream();

    virtual u32 Open() = 0;
    virtual u32 Close() = 0;

    bool IsEndOfStream()
    {
        return (m_fgEndOfStream);
    }
    bool IsOpened()
    {
        return (m_fgOpened);
    };
    virtual s8 *  GetDataBuffer(u32 &dwSize) = 0;
    u32 GetFormat(WAVEFORMATEX &rWfx)
    {
        memcpy(&rWfx, &m_rWfx, sizeof(WAVEFORMATEX));
        return MMSYSERR_NOERROR;
    };
    u32 GetSize()
    {
        return (m_u4Size);
    };
    virtual u32 ReleaseBuffer(s8 * pbBuffer) = 0;

protected:

    WAVEFORMATEX m_rWfx;
    u32 m_u4Size;
    bool m_fgOpened;
    bool m_fgEndOfStream;

};


class FileStream : public PcmStream
{
public:
    FileStream();
    ~FileStream();
    virtual u32 Open();
    virtual u32 Close();
    u32 SetFileName(s8 szFile[])
    {
        return m_rWaveFile.SetFileName(szFile);
    };
    u32 GetFileName(s8 szFile[])
    {
        return m_rWaveFile.GetFileName(szFile);
    };

    virtual u32 Restart() = 0;
protected:

    WaveFile m_rWaveFile;
    u32 m_u4BufNum;
    u32 m_u4BufSize;
    s8 *  *m_pbBuf;
    u32 *m_u4BufState;
};


#define PLAYED_BUF_NUM  2
#define BUF_FREE    0x0000
#define BUF_IN_USED 0x0001

class WaveFileStream : public FileStream
{
public:
    WaveFileStream();
    virtual ~WaveFileStream();

    u32 Open();
    u32 Close();
    s8 *  GetDataBuffer(u32 &dwSize);
    u32 ReleaseBuffer(s8 * pbBuffer);
    u32 Restart();
private:
    u32 m_u4UsedLen;
};

#define BUF_HAS_RECORDED  0x0002
#define BUF_IN_PLAYED     0x0004

#define REC_BUF_NUM 4
#define TOTAL_BUF_NUM (PLAYED_BUF_NUM + REC_BUF_NUM)

class RecordStream : public FileStream
{
public:
    RecordStream();
    virtual ~RecordStream();

    u32 Open();
    u32 Close();

    s8 *  GetDataBuffer(u32 &dwSize);
    u32 ReleaseBuffer(s8 * pbBuffer);

    u32 StartRecord(WAVEFORMATEX *prWfx);
    u32 StopRecord();
    s8 *  GetBuffer(u32 &dwSize);
    u32 TransferData(s8 * pbBuffer, u32 dwSize);
    u32 Restart();
private:
    u32 *m_u4DataSize;
    u32 m_u4WriteID;
    u32 m_u4ReadID;
    u32 m_u4RealNum;

};




#endif // __PCMSTREAM_H_


