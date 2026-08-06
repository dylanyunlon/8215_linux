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
*[File]             strmdev.cpp
*[Author]               mtk40004
*[Description]
*    implementation for stream player & recording class.    
******************************************************************************/

#include "strmdev.h"
#include "cli_common.h"

#include <linux/types.h>


#define DMNR_REC_FILE TEXT("\\Flash Disk\\dmnr_rec.wav")
#define DMNR_PLAY_FILE TEXT("\\Flash Disk\\dmnr_play.wav")
#define REV_BUF_SIZE			5*1024

static StrmRecord *_prRecorder =NULL;
static RecordStream *_prRecStream = NULL;
static WaveFile *_prPlayFile = NULL;
static u32 _u4RecSize = 0;
static WAVEFORMATEX _rRecFmt;
static u32 _u4PlaySize = 0;
static u32 _u4ReceiveSize = 0;
static u8 _pbReceiveBuf[REV_BUF_SIZE];

static StrmPlayer *_prPlayer =NULL;
static WaveFileStream *_prPlayStream = NULL;

void CALLBACK waveOutProc(
                          HWAVEOUT hwo,
                          u32 uMsg,
                          u32 dwInstance,
                          u32 dwParam1,
                          u32 dwParam2
                          );

static void CALLBACK waveInProc(
                                HWAVEIN hwi,
                                u32 uMsg,
                                u32 dwInstance,
                                u32 dwParam1,
                                u32 dwParam2
                                );

StrmDev::StrmDev()
{
    m_eState = STR_UNINITED;
    m_dwDataLen = 0;
    m_dwLastTime = 0;
    m_dwEstTime = 0;
    m_u4HdrNum = 0;
    m_pHdr = NULL;
}


extern "C" 
{
    u32 DMNR_Record(u32 u4Size)
    {
        _rRecFmt.wFormatTag = WAVE_FORMAT_PCM;
        _rRecFmt.nChannels  = 2;
        _rRecFmt.nSamplesPerSec   = 8000;
        _rRecFmt.wBitsPerSample    = 16;
        _rRecFmt.nBlockAlign     = _rRecFmt.nChannels * _rRecFmt.wBitsPerSample / 8;
        _rRecFmt.nAvgBytesPerSec     = _rRecFmt.nBlockAlign * _rRecFmt.nSamplesPerSec;
        if (!_prRecorder)
            _prRecorder = new StrmRecord;

        if (!_prRecStream)
            _prRecStream = new RecordStream;
        g_fgDmnrRec = TRUE;

        _prRecStream->SetFileName(DMNR_REC_FILE);
        _prRecorder->Init(_prRecStream);
        _prRecorder->SetRecSize(u4Size);
        _prRecorder->Start((u32)waveInProc, CALLBACK_FUNCTION , &_rRecFmt);
        return (0);

    };

    u32 DMNR_Stop_Record()
    {
        g_fgDmnrRec = FALSE;
        _prRecorder->Stop();
        _prRecorder->UnInit();
        return (0);

    };


    u32 DMNR_Init_PlayFile(u32 u4SampleRate, u32 u4Channels, u32 u4Size)
    {
        WAVEFORMATEX rFmt;
        rFmt.wFormatTag = WAVE_FORMAT_PCM;
        rFmt.nChannels  = u4Channels;
        rFmt.nSamplesPerSec   = u4SampleRate;
        rFmt.wBitsPerSample    = 16;
        rFmt.nBlockAlign     = rFmt.nChannels * rFmt.wBitsPerSample / 8;
        rFmt.nAvgBytesPerSec     = rFmt.nBlockAlign * rFmt.nSamplesPerSec;
        if (!_prPlayFile)
            _prPlayFile = new WaveFile;
        _prPlayFile->Close();
        _prPlayFile->SetFileName(DMNR_PLAY_FILE);
        _prPlayFile->SetCacheMode(FALSE);
        _prPlayFile->Create(&rFmt);
        _u4PlaySize = u4Size;
        _u4ReceiveSize = 0;
        return (0);
    };


    u32 DMNR_Receive_PlayData()
    {
        u32 u4Size = 0;
        while (_u4ReceiveSize < _u4PlaySize)
        {
			u4Size = UartReadBytes(_pbReceiveBuf, REV_BUF_SIZE, 100);
            if (u4Size)
            {
                _prPlayFile->WriteData((VOID *)_pbReceiveBuf, u4Size);
                _u4ReceiveSize += u4Size;
            }
            if (REV_BUF_SIZE != u4Size)
                break;
        } 
        if (_u4ReceiveSize < _u4PlaySize)
            return (1);
        else
        {
            _prPlayFile->Close();
            return (0);
        }
    };


    u32 DMNR_StartPlay()
    {
        if (!_prPlayer)
            _prPlayer = new StrmPlayer;

        if (!_prPlayStream)
            _prPlayStream = new WaveFileStream;
        g_fgDmnrPlay = TRUE;

        _prPlayStream->SetFileName(DMNR_PLAY_FILE);
        _prPlayer->Init(_prPlayStream);
        _prPlayer->Start(0xFFFFFFFF, (u32)waveOutProc, CALLBACK_FUNCTION );
        return (0);
    };

    u32 DMNR_StopPlay()
    {
        g_fgDmnrPlay = FALSE;
        _prPlayer->Stop();
        _prPlayer->UnInit();
        return (0);
    };

}



void CALLBACK waveOutProc(
                          HWAVEOUT hwo,
                          u32 uMsg,
                          u32 dwInstance,
                          u32 dwParam1,
                          u32 dwParam2
                          )
{
    if (!g_fgDmnrPlay)
        return;
    switch(uMsg)
    {
    case WOM_DONE:
        if (_prPlayer->DoneMsgHandle((LPWAVEHDR)dwParam1))
        {
            g_fgDmnrPlay = FALSE;
        }
        break;
    default:
        break;

    }

};


static void CALLBACK waveInProc(
                                HWAVEIN hwi,
                                u32 uMsg,
                                u32 dwInstance,
                                u32 dwParam1,
                                u32 dwParam2
                                )
{
    if (!g_fgDmnrRec)
        return;
    switch(uMsg)
    {
    case WIM_DATA:
        if (_prRecorder->DoneMsgHandle((LPWAVEHDR)dwParam1))
        {
            g_fgDmnrRec = FALSE;
        }
        break;
    default:
        break;

    }
};

StrmDev::~StrmDev()
{
    if (m_pHdr)
        delete [] m_pHdr;
    m_pHdr = NULL;
}


VOID StrmDev::CalcEstTime()
{
    u32 dwOldTime = m_dwLastTime;
    m_dwLastTime = GetTickCount();
    m_dwEstTime += m_dwLastTime - dwOldTime;
}


u32 StrmDev::GetEstTime()
{
    if (STR_RUNNING == m_eState)
        CalcEstTime();
    return (m_dwEstTime);
}


u32 StrmDev::GetRealTime()
{
    u32 u4MiniSecond = (u32)((UINT64)m_dwDataLen * 1000 / m_rWFX.nAvgBytesPerSec);
    return (u4MiniSecond);
}


StrmPlayer::StrmPlayer()
{
    m_hwo = NULL;
}


StrmPlayer::~StrmPlayer()
{
    UnInit();
}

u32 StrmPlayer::Init(PcmStream *prStream)
{
    m_u4HdrNum = PLAYED_BUF_NUM;
    if (MMSYSERR_ERROR == StrmDev::Init(prStream))
        return (MMSYSERR_ERROR);

    if (MMSYSERR_NOERROR != m_prStream->Open())
    {
        return (MMSYSERR_ERROR);
    }

    m_prStream->GetFormat(m_rWFX);


    return (MMSYSERR_NOERROR);
}


u32 StrmPlayer::UnInit()
{
    if (STR_UNINITED == m_eState)
        return (MMSYSERR_NOERROR);

    if (STR_STOPPED != m_eState)
    {
        Stop();
    }
    m_prStream->Close();
    m_eState = STR_UNINITED;
    StrmDev::UnInit();

    return (MMSYSERR_NOERROR);
}


u32 StrmPlayer::PlayFrame(LPWAVEHDR lpHdr)
{
    MMRESULT mmRet;

    lpHdr->dwBytesRecorded = 0;
    lpHdr->dwUser = 0;
    lpHdr->dwBufferLength  = 0;
    lpHdr->lpData = (s8 ** )m_prStream->GetDataBuffer(lpHdr->dwBufferLength );
    if (!lpHdr->lpData)
    {
        RETAILMSG(1, (TEXT("[Player] Stream no free buffer!\r\n")));
        return (MMSYSERR_ERROR);
    }

    lpHdr->dwUser = m_u4ID;
    if (m_prStream->IsEndOfStream())
        lpHdr->dwUser |= 0x80000000; // Last frame.
    lpHdr->dwLoops = 0;
    lpHdr->dwFlags = 0;

    if (MMSYSERR_NOERROR != (mmRet = waveOutPrepareHeader(m_hwo, lpHdr, sizeof(WAVEHDR))))
    {
        RETAILMSG(1, (TEXT("[Player] waveOutPrepareHeader failed(0x%x)!\r\n"), mmRet));
        RETAILMSG(1, (TEXT("m_hwo(0x%x) lpHdr(0x%x), lpData(0x%x) Len(%d)\r\n"), m_hwo, lpHdr, lpHdr->lpData, lpHdr->dwBufferLength));
        return (mmRet);
    }
    if (MMSYSERR_NOERROR != (mmRet = waveOutWrite( m_hwo, lpHdr, sizeof(WAVEHDR))))
    {
        RETAILMSG(1, (TEXT("[Player] waveOutWrite failed(0x%x)!\r\n"), mmRet));
        RETAILMSG(1, (TEXT("m_hwo(0x%x) lpHdr(0x%x), lpData(0x%x) Len(%d)\r\n"), m_hwo, lpHdr, lpHdr->lpData, lpHdr->dwBufferLength));
    }
    return (mmRet);
}


u32 StrmPlayer::Start(u32 u4Volume, u32 dwCallback, u32 dwOpenFlag)
{
    MMRESULT mr;
    u32 i;
    if (STR_UNINITED == m_eState)
    {
        RETAILMSG(1, (TEXT("[Error] StrmPlayer::Start is called in uninit state.!\r\n")));
        return (MMSYSERR_ERROR);
    }

    if (STR_STOPPED == m_eState)
    {
        m_rWFX.cbSize = sizeof(WAVEFORMATEX);
        mr = waveOutOpen(&m_hwo, WAVE_MAPPER,  &m_rWFX, dwCallback, NULL, dwOpenFlag);
        if(MMSYSERR_NOERROR != mr) 
        {
            RETAILMSG(1, (TEXT("[Player] waveOutOpen failed(0x%x)!\r\n"), mr));
            return (MMSYSERR_ERROR);
        }
        u32 nPos = u4Volume;

        nPos |= nPos << 16;
        waveOutSetVolume(m_hwo, nPos);
    } 
    else
    {
        mr = waveOutReset(m_hwo);
        if(MMSYSERR_NOERROR != mr) 
        {
            RETAILMSG(1, (TEXT("[Player] waveOutReset failed(0x%x)!\r\n"), mr));
            return (MMSYSERR_ERROR);
        }
        for (i=0; i < m_u4HdrNum; i++)
        {
            waveOutUnprepareHeader(m_hwo, m_pHdr + i, sizeof(WAVEHDR));
        }
    }
    ASSERT(m_pHdr);
    for (i=0; i < m_u4HdrNum; i++)
    {
        PlayFrame(m_pHdr + i);
    }
    m_eState = STR_RUNNING;
    m_dwEstTime = 0;
    m_dwDataLen = 0;
    m_dwLastTime = GetTickCount();

    return (MMSYSERR_NOERROR);
}

u32 StrmPlayer::Stop()
{
    MMRESULT mr;
    if ((STR_STOPPED == m_eState) || (STR_UNINITED == m_eState))
        return (MMSYSERR_NOERROR);

    m_eState = STR_WAITFORSTOP;
    mr = waveOutReset(m_hwo);
    if(MMSYSERR_NOERROR != mr) {
        return (MMSYSERR_ERROR);
    }
    for (u32 i= 0; i < m_u4HdrNum; i++)
    {
        waveOutUnprepareHeader(m_hwo, m_pHdr + i, sizeof(WAVEHDR));
        m_pHdr[i].dwFlags = 0;
    }

    mr = waveOutClose(m_hwo);
    if(MMSYSERR_NOERROR != mr) {
        return (MMSYSERR_ERROR);
    }

    m_hwo = NULL;
    m_eState = STR_STOPPED;

    m_dwEstTime = 0;
    m_dwDataLen = 0;

    return (MMSYSERR_NOERROR);
}



u32 StrmPlayer::SetVolume(u32 u4Volume)
{
    if (m_hwo)
        return (waveOutSetVolume(m_hwo, u4Volume));
    else
        return (MMSYSERR_ERROR);
}



u32 StrmPlayer::DoneMsgHandle(LPWAVEHDR lpHdr)
{
    if((lpHdr->dwFlags && WHDR_DONE) == WHDR_DONE)
    {
        m_prStream->ReleaseBuffer((s8 *)lpHdr->lpData);
        waveOutUnprepareHeader(m_hwo, lpHdr, sizeof(WAVEHDR));
        if ((STR_STOPPED != m_eState) && 
            (STR_UNINITED != m_eState))
        {
            u32 dwUser = lpHdr->dwUser;
            m_dwDataLen += lpHdr->dwBufferLength;
            if (STR_RUNNING == m_eState)
            {
                if (dwUser & 0x80000000)
                {
                    m_eState = STR_NODATA;
                    CalcEstTime();
                    return (1);
                } 
                else if (!m_prStream->IsEndOfStream())
                {
                    PlayFrame(lpHdr);
                }
            }
        }
    }
    return (0);
}




u32 StrmPlayer::Pause()
{
    MMRESULT mr;
    if (STR_RUNNING == m_eState)
    {
        mr = waveOutPause(m_hwo);
        if(mr != MMSYSERR_NOERROR)
        {
            return (MMSYSERR_ERROR);
        }
        m_eState = STR_PAUSE;
        CalcEstTime();
    }
    return (MMSYSERR_NOERROR);
}
u32 StrmPlayer::Resume()
{
    // Resume
    if (STR_PAUSE != m_eState)
        return (MMSYSERR_NOERROR);
    MMRESULT mr = waveOutRestart(m_hwo);
    if(mr != MMSYSERR_NOERROR)
    {
        return (MMSYSERR_ERROR);
    }
    m_eState = STR_RUNNING;
    m_dwLastTime = GetTickCount();
    for (u32 i= 0; i < m_u4HdrNum; i++)
    {
        if (!(m_pHdr[i].dwFlags & WHDR_PREPARED))
            PlayFrame(m_pHdr + i);
    }
    return (MMSYSERR_NOERROR);
}


StrmRecord::StrmRecord()
{
    m_eState = STR_UNINITED;
}


StrmRecord::~StrmRecord()
{
}


u32 StrmRecord::Init(PcmStream *prStream)
{
    m_u4HdrNum = REC_BUF_NUM;
    if (MMSYSERR_ERROR == StrmDev::Init(prStream))
        return (MMSYSERR_ERROR);
    m_prRec = (RecordStream *)prStream;
    m_u4RecSize = 0;
    RETAILMSG(0, (TEXT("%[StrmRecord::Init]m_eState (0x%x)!\r\n"), m_eState));
    return (MMSYSERR_NOERROR);
}


u32 StrmRecord::UnInit()
{
    if ((STR_RUNNING == m_eState) || (STR_RECPAUSE == m_eState))
    {
        Stop();
    }
    StrmDev::UnInit();
    m_eState = STR_UNINITED;
    return (MMSYSERR_NOERROR);
}


u32 StrmRecord::Start(u32 dwCallback, u32 dwOpenFlag,  WAVEFORMATEX * prWfx)
{
    RETAILMSG(0, (TEXT("%[StrmRecord::Start]m_eState (0x%x)!\r\n"), m_eState));
    if (STR_STOPPED != m_eState)
    {
        return (MMSYSERR_ERROR);
    }
    MMRESULT mr;
    RETAILMSG(1, (TEXT("%[Record] Start FS(%d), WB(%d), Channels(%d)!\r\n"),
        prWfx->nSamplesPerSec, prWfx->wBitsPerSample, prWfx->nChannels));
    m_prRec->StartRecord(prWfx);
    memcpy(&m_rWFX, prWfx, sizeof(WAVEFORMATEX));
    m_rWFX.cbSize = sizeof(WAVEFORMATEX);
    mr = waveInOpen(&m_hwi, WAVE_MAPPER,  &m_rWFX, dwCallback, NULL, dwOpenFlag);
    if(MMSYSERR_NOERROR != mr) 
    {
        RETAILMSG(1, (TEXT("[waveInOpen] Failed (0x%x)!\r\n"), mr));
        return (MMSYSERR_ERROR);
    }
    ASSERT(m_pHdr);
    for (u32 i=0; i < m_u4HdrNum; i++)
    {
        RecOneFrame(m_pHdr + i);
    }
    m_eState = STR_RUNNING;
    m_dwEstTime = 0;
    m_dwDataLen = 0;
    m_dwLastTime = GetTickCount();
    return (MMSYSERR_NOERROR);
}


u32 StrmRecord::RecOneFrame(LPWAVEHDR lpHdr)
{
    MMRESULT mmRet;

    lpHdr->dwBytesRecorded = 0;
    lpHdr->dwUser = 0;
    lpHdr->dwBufferLength  = 0;
    lpHdr->lpData = (s8 ** )m_prRec->GetBuffer(lpHdr->dwBufferLength );
    if (!lpHdr->lpData)
    {
        RETAILMSG(1, (TEXT("[Record] Stream no free buffer!\r\n")));
        return (MMSYSERR_ERROR);
    }

    lpHdr->dwLoops = 0;
    lpHdr->dwFlags = 0;

    if (MMSYSERR_NOERROR != (mmRet = waveInPrepareHeader (m_hwi, lpHdr, sizeof(WAVEHDR))))
    {
        RETAILMSG(1, (TEXT("[Record] waveInPrepareHeader(0x%x)!\r\n"), mmRet));
        return (mmRet);
    }
    mmRet =waveInAddBuffer ( m_hwi, lpHdr, sizeof(WAVEHDR));
    return (mmRet);

}



u32 StrmRecord::Stop()
{
    MMRESULT mr;
    if ((STR_STOPPED == m_eState) || (STR_UNINITED == m_eState))
        return (MMSYSERR_NOERROR);

    m_eState = STR_WAITFORSTOP;
    mr = waveInReset(m_hwi);
    Sleep(100);
    if(MMSYSERR_NOERROR != mr) 
    {
        RETAILMSG(1, (TEXT("[waveInReset] Failed (0x%x)!\r\n"), mr));
        return (MMSYSERR_ERROR);
    }
    for (u32 i= 0; i < m_u4HdrNum; i++)
    {
        waveInUnprepareHeader(m_hwi, m_pHdr + i, sizeof(WAVEHDR));
        m_pHdr[i].dwFlags = 0;
    }

    mr = waveInClose(m_hwi);
    if(MMSYSERR_NOERROR != mr) 
    {
        RETAILMSG(1, (TEXT("[waveInClose] Failed (0x%x)!\r\n"), mr));
        return (MMSYSERR_ERROR);
    }
    m_prRec->StopRecord();

    m_hwi = NULL;
    m_eState = STR_STOPPED;

    m_dwEstTime = 0;
    m_dwDataLen = 0;
    return (MMSYSERR_NOERROR);
}

u32 StrmRecord::DoneMsgHandle(LPWAVEHDR lpHdr)
{
    if (STR_RUNNING == m_eState)
    {
        if((lpHdr->dwFlags && WHDR_DONE) == WHDR_DONE)
        {
            if ((STR_STOPPED != m_eState) && 
                (STR_UNINITED != m_eState))
            {
                m_dwDataLen += lpHdr->dwBytesRecorded;
                m_prRec->TransferData((s8 *)lpHdr->lpData, lpHdr->dwBytesRecorded);
                waveInUnprepareHeader(m_hwi, lpHdr, sizeof(WAVEHDR));
                if ((STR_RUNNING == m_eState) && (!m_u4RecSize ||  (m_u4RecSize > m_dwDataLen)) )
                {
                    RecOneFrame(lpHdr);
                }
            }
        }
    }
    return (m_u4RecSize && (m_dwDataLen >= m_u4RecSize));
}

u32 StrmRecord::Pause()
{
    if (STR_RUNNING == m_eState)
    {
        m_eState = STR_RECPAUSE;
        waveInReset(m_hwi);
        waveInStop(m_hwi);
        CalcEstTime();
    }
    return (MMSYSERR_NOERROR);
}


u32 StrmRecord::Resume()
{
    // Resume
    if (STR_RECPAUSE != m_eState)
        return (MMSYSERR_NOERROR);
    MMRESULT mr = waveInStart(m_hwi);
    if(mr != MMSYSERR_NOERROR)
    {
        return (MMSYSERR_ERROR);
    }
    m_eState = STR_RUNNING;
    m_dwLastTime = GetTickCount();
    for (u32 i= 0; i < m_u4HdrNum; i++)
    {
        if (!(m_pHdr[i].dwFlags & WHDR_PREPARED))
            RecOneFrame(m_pHdr + i);
    }
    return (MMSYSERR_NOERROR);
}





