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
*[File]            strmdev.h
*[Author]        mtk40004
*[Description]
*    Interface of stream player & recording class
******************************************************************************/
#ifndef __STREAMDEV_H_
#define __STREAMDEV_H_

#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"
#include "mmsystem.h"

#include <linux/types.h>

#include "pcmstream.h"

typedef enum
{
    STR_UNINITED = 0,
    STR_STOPPED,
    STR_WAITFORSTOP,
    STR_RUNNING,
    STR_PAUSE,
    STR_NODATA,
    STR_RECPAUSE
}STR_STATE;


class StrmDev
{
public:
    StrmDev();
    ~StrmDev();
    STR_STATE GetState()
    {
        return (m_eState);
    };
    virtual u32 Init(PcmStream *prStream)
    {
        if (STR_UNINITED == m_eState)
        {
            m_prStream = prStream;
            m_eState = STR_STOPPED;
            if (m_u4HdrNum && !m_pHdr)
            {
                m_pHdr = new WAVEHDR[m_u4HdrNum];
            }
            if (m_pHdr)
            {
                for (u32 i=0; i< m_u4HdrNum; i++)
                    m_pHdr[i].dwFlags = 0;
            }

            return (MMSYSERR_NOERROR);
        }
        return (MMSYSERR_ERROR);
    }

    virtual u32 UnInit()
    {
        if (m_pHdr)
        {
            delete [] m_pHdr;
            m_pHdr = NULL;
            m_u4HdrNum = 0;
        }
        return (MMSYSERR_ERROR);
    }

    u32 GetEstTime();
    u32 GetRealTime(); // in minisecond
    virtual u32 DoneMsgHandle(LPWAVEHDR lpHdr) = 0;

protected:    
    void   CalcEstTime();

protected:
    u32    m_dwEstTime;
    u32    m_dwDataLen;
    u32   m_u4HdrNum;
    WAVEHDR  *m_pHdr;
    WAVEFORMATEX  m_rWFX;
    STR_STATE m_eState;
    u32 m_dwLastTime;
    PcmStream *m_prStream;
};

class StrmPlayer: public StrmDev
{
public:
    StrmPlayer();
    virtual ~StrmPlayer();
    u32 Init(PcmStream *prStream);
    u32 UnInit();
    u32 Start(u32 u4Volume, u32 dwCallback, u32 dwOpenFlag);
    u32 Stop();
    u32 DoneMsgHandle(LPWAVEHDR lpHdr);
    u32 Pause();
    u32 Resume();
    u32 SetVolume(u32 u4Volume);
    u32 SetID(u32 u4ID)
    {
        m_u4ID = u4ID;
        return (0);
    }

protected:
    u32 PlayFrame(LPWAVEHDR lpHdr);

    u32 m_u4ID;
    HWAVEOUT  m_hwo;
};

class StrmRecord : public StrmDev
{
public:
    StrmRecord();
    virtual ~StrmRecord();
    u32 Init(PcmStream *prStream);
    u32 UnInit();
    u32 Start(u32 dwCallback, u32 dwOpenFlag,  WAVEFORMATEX * prWfx);
    u32 Stop();
    u32 DoneMsgHandle(LPWAVEHDR lpHdr);
    u32 Pause();
    u32 Resume();
    bool SetRecSize(u32 u4Size)
    {
        m_u4RecSize = u4Size;
        return (TRUE);
    };

private:
    u32 RecOneFrame(LPWAVEHDR lpHdr);

    HWAVEIN m_hwi;
    RecordStream *m_prRec;
    u32 m_u4RecSize;
};


#endif // __STREAMPLAYER_H_


