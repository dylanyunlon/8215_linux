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
#include "audmhl_var.h"
#include "aud_if.h"


//This file for hdmi rx test.

#if 0
/****************************************************************************
** Audio Command
****************************************************************************/
u32 StrToInt(const s8* pszStr);

#define UTIL_Printf(a,...)

AUDIN_INFO_T  *g_pAudinInfo = NULL;

extern HDMI_RX_IN_AUDIO_INFO_T g_rHdmiAudInfo;
extern HDMI_RX_Audio_InfoFrame g_rHdmiAudInfFrame;
extern HDMI_RX_AUDIO_CHSTS g_rHdmiAudChSts;

static s32 _AudinMultiType(s32 i4Argc, const s8 **szArgv)
{
    u8 AudInType ;
    AudInType = (u8)StrToInt(szArgv[1]);

    if ((AudInType > 8) || (AudInType == 0) || (AudInType == 2)  || (AudInType == 3))
    {
        UTIL_Printf("Usage: type [Audio In Type]\n");
        UTIL_Printf("[Audio In Type] 1 : Coaxial, 4 : Optical1, 5 : Optical2 , 6 : HDMI Rx , 7 : Line In, 8 : MicIn \n");
        return (-1);
    }
    else
    {
        AudmhlSetAudInType((AUDIO_IN_TYPE_T)AudInType) ;
        return (1);

    }
}

static s32 _AudinHDMIInType(s32 i4Argc, const s8 **szArgv)
{
    u8 i ;
    u8  u1PCMMultiCh = 1 ;

    if (g_pAudinInfo == NULL)
    {
        g_pAudinInfo = (AUDIN_INFO_T *)kzalloc(sizeof(AUDIN_INFO_T), GFP_KERNEL);

        if (NULL == g_pAudinInfo)
        {
            UTIL_Printf("[AUDIN_MULTI] ERR: AudinInfo alloc memory failed!\n");
            return -1;
        }

        x_memset(g_pAudinInfo, 0, (sizeof(AUDIN_INFO_T)));
    }

    //HDMI_RX_AUDIO_CHSTS *pHDMIRxAudCHSTS=NULL;
    g_pAudinInfo->u1AudinPauseStatus = 0 ;       // PLAY
    g_pAudinInfo->u1AudinLockStatus = 0 ;   //Lock
    g_pAudinInfo->u1AudinChStatus = 0x40 ;       //RX6
    g_pAudinInfo->u1AudinSampleRate = 0x9 ;    //48K
    g_pAudinInfo->u1AudinSwitchOK = 1 ;            //Input switch OK
    g_pAudinInfo->u1AudinOnOffOK  = 1 ;             //Input On/Off OK
    g_pAudinInfo->u1SpdifAudinType =  0 ; //PCM
    g_pAudinInfo->u1SpdifRawDataType = AUD_DRV_FMT_HDMI_IN_PCM ;// PCM

    g_pAudinInfo->u1HDMIRxAudFmt = HDMI_RX_PCM ;
    //PCM Info : Default 2 Channel
    g_pAudinInfo->u4HDMIIRxPCMInfo.AudioChannelCount = COUNT_2CH; //HDMI_RX_AUDIO_CHANNEL_COUNT_T
    g_pAudinInfo->u4HDMIIRxPCMInfo.DM_INH = 0 ;
    g_pAudinInfo->u4HDMIIRxPCMInfo.AudioCodingType = CODE_PCM; //Useless ,HDMI_RX_AUDIO_CODING_TYPE_T
    g_pAudinInfo->u4HDMIIRxPCMInfo.LevelShiftValue = LEVEL_0DB; //HDMI_RX_AUD_LEVEL_TYPE_T
    g_pAudinInfo->u4HDMIIRxPCMInfo.SampleSize = SAMPLE_SIZE_24BIT; //HDMI_RX_SAMPLE_SIZE_TYPE_T
    g_pAudinInfo->u4HDMIIRxPCMInfo.SpeakerPlacement = CA_FL_FR; //HDMI_RX_SPEAKER_ALLOCATE_T
    //Channel Status Info : Default : NULL
    g_pAudinInfo->u8HDMIRxAudCHSTS.rev = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.ISLPCM = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.CopyRight = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.AdditionFormatInfo = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.ChannelStatusMode = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.CategoryCode = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.SourceNumber = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.ChannelNumber = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.SamplingFreq = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.ClockAccuary = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.rev2 = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.WorldLen = 0;
    g_pAudinInfo->u8HDMIRxAudCHSTS.OriginalSamplingFreq = 0;

    for (i = 1; i < i4Argc; i++)
    {
        const s8 *command = szArgv[i] ;
        const s8 *parameter = (i + 1 < i4Argc ? szArgv[i + 1] : NULL) ;

        if (command[0] == '-')
        {
            switch (command[1])
            {
            case 'f':
                g_pAudinInfo->u1HDMIRxAudFmt  = (HDMI_RX_AUDIO_FORMAT_T)StrToInt(parameter) ;
                i++ ;
                break ;

            case 's':
                g_pAudinInfo->u1AudinSampleRate = (u8)StrToInt(parameter) ;
                i++ ;
                break ;

            case 'c':
                u1PCMMultiCh = (u8)StrToInt(parameter) ;
                i++ ;
                break ;
            }
        }
    }

    AudmhlSetAudInfo(g_pAudinInfo, u1PCMMultiCh);


    //GetMhlInAudInfo
    if (g_pAudinInfo->u1HDMIRxAudFmt == HDMI_RX_HBR)
    {
        g_rHdmiAudInfo.u1HBRAudio = TRUE;
    }
    else
    {
        g_rHdmiAudInfo.u1HBRAudio = FALSE;
    }

    if (g_pAudinInfo->u1HDMIRxAudFmt == HDMI_RX_DSD)
    {
        g_rHdmiAudInfo.u1DSDAudio = TRUE;
    }
    else
    {
        g_rHdmiAudInfo.u1DSDAudio = FALSE;
    }

    if ((g_pAudinInfo->u1HDMIRxAudFmt == HDMI_RX_SD_RAW) || (g_pAudinInfo->u1HDMIRxAudFmt == HDMI_RX_192k_RAW))
    {
        g_rHdmiAudInfo.u1RawSDAudio = TRUE;
    }
    else
    {
        g_rHdmiAudInfo.u1RawSDAudio = FALSE;
    }

    g_rHdmiAudInfo.u1PCMMultiCh =  u1PCMMultiCh;

    //; 7~0xF : 32/44.1/48/64/88.2/96/128/176.4/192 KHz
    if (g_pAudinInfo->u1AudinSampleRate == 0x7)
    {
        g_rHdmiAudInfo.u1FsDec =  0x3;
    }
    else   if (g_pAudinInfo->u1AudinSampleRate == 0x8)
    {
        g_rHdmiAudInfo.u1FsDec =  0x0;
    }
    else   if (g_pAudinInfo->u1AudinSampleRate == 0x9)
    {
        g_rHdmiAudInfo.u1FsDec =  0x2;
    }
    else   if (g_pAudinInfo->u1AudinSampleRate == 0xB)
    {
        g_rHdmiAudInfo.u1FsDec =  0x8;
    }
    else   if (g_pAudinInfo->u1AudinSampleRate == 0xC)
    {
        g_rHdmiAudInfo.u1FsDec =  0xA;
    }
    else   if (g_pAudinInfo->u1AudinSampleRate == 0xE)
    {
        g_rHdmiAudInfo.u1FsDec =  0xC;
    }
    else   if (g_pAudinInfo->u1AudinSampleRate == 0xF)
    {
        g_rHdmiAudInfo.u1FsDec =  0xE;
    }
    else
        // OUT of range
    {
        g_rHdmiAudInfo.u1FsDec =  0x1;
    }

    //GetMhlInAudInfoFrame
    g_rHdmiAudInfFrame.info.Type = 0x84;
    g_rHdmiAudInfFrame.info.Ver = 0x1;
    g_rHdmiAudInfFrame.info.Len = 0xA;
    g_rHdmiAudInfFrame.info.AudioChannelCount = g_pAudinInfo->u4HDMIIRxPCMInfo.AudioChannelCount;
    g_rHdmiAudInfFrame.info.RSVD1 = 0;
    g_rHdmiAudInfFrame.info.AudioCodingType = g_pAudinInfo->u4HDMIIRxPCMInfo.AudioCodingType;
    g_rHdmiAudInfFrame.info.SampleSize = g_pAudinInfo->u4HDMIIRxPCMInfo.SampleSize;
    g_rHdmiAudInfFrame.info.SampleFreq = 0;
    g_rHdmiAudInfFrame.info.Rsvd2 = 0;
    g_rHdmiAudInfFrame.info.FmtCoding = 0;
    g_rHdmiAudInfFrame.info.SpeakerPlacement = g_pAudinInfo->u4HDMIIRxPCMInfo.SpeakerPlacement;
    g_rHdmiAudInfFrame.info.Rsvd3 = 0;
    g_rHdmiAudInfFrame.info.LevelShiftValue = g_pAudinInfo->u4HDMIIRxPCMInfo.LevelShiftValue;
    g_rHdmiAudInfFrame.info.DM_INH = g_pAudinInfo->u4HDMIIRxPCMInfo.DM_INH ;
    //
    g_rHdmiAudChSts.ISLPCM =  g_pAudinInfo->u8HDMIRxAudCHSTS.ISLPCM ;
    g_rHdmiAudChSts.CopyRight = g_pAudinInfo->u8HDMIRxAudCHSTS.CopyRight;
    g_rHdmiAudChSts.AdditionFormatInfo =  g_pAudinInfo->u8HDMIRxAudCHSTS.AdditionFormatInfo;
    g_rHdmiAudChSts.ChannelStatusMode = g_pAudinInfo->u8HDMIRxAudCHSTS.ChannelStatusMode;
    g_rHdmiAudChSts.CategoryCode = g_pAudinInfo->u8HDMIRxAudCHSTS.CategoryCode;
    g_rHdmiAudChSts.SourceNumber = g_pAudinInfo->u8HDMIRxAudCHSTS.SourceNumber;
    g_rHdmiAudChSts.ChannelNumber = g_pAudinInfo->u8HDMIRxAudCHSTS.ChannelNumber;
    g_rHdmiAudChSts.SamplingFreq = g_pAudinInfo->u8HDMIRxAudCHSTS.SamplingFreq;
    g_rHdmiAudChSts.ClockAccuary =  g_pAudinInfo->u8HDMIRxAudCHSTS.ClockAccuary;
    g_rHdmiAudChSts.WorldLen = g_pAudinInfo->u8HDMIRxAudCHSTS.WorldLen;
    g_rHdmiAudChSts.OriginalSamplingFreq = g_pAudinInfo->u8HDMIRxAudCHSTS.OriginalSamplingFreq;
    return (AUDMHL_OK);
}

static s32 _AudinMultiLineInOn(s32 i4Argc, const s8 **szArgv)
{
    // Turn on Multiple Line In module
    AudmhlSetAudOnOff(TRUE);
    return (AUDMHL_OK);
}

static s32 _AudinMultiLineInOff(s32 i4Argc, const s8 **szArgv)
{
    // Turn off Multiple Line In module
    AudmhlSetAudOnOff(FALSE);
    return (AUDMHL_OK);
}

u32 StrToHex(const s8* pszStr, u32 u4Len)
{
	u32 u4Idx;
	u32 u4ReturnValue = 0;

	if ((pszStr == NULL) || (u4Len == 0))
	{
		return 0;
	}

	u4Len = (u4Len > 8) ? 8 : u4Len;

	for (u4Idx = 0;
		u4Idx < u4Len;
		u4Idx++)
	{
		if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (u32)(u8)(pszStr[u4Idx] - '0');
		}
		else
		if ((pszStr[u4Idx] >= 'A') && (pszStr[u4Idx] <= 'F'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (u32)(u8)(pszStr[u4Idx] - 'A' ) + 10;
		}
		else
		if ((pszStr[u4Idx] >= 'a') && (pszStr[u4Idx] <= 'f'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (u32)(u8)(pszStr[u4Idx] - 'a') + 10;
		}
		else
		{
			return 0;
		}
	}

	return u4ReturnValue;
}

u32 StrToDec(const s8* pszStr, u32 u4Len)
{
	u32 u4Idx;
	u32 u4ReturnValue = 0;

	if ((pszStr == NULL) || (u4Len == 0))
	{
		return 0;
	}

	// 0xFFFFFFFF = 4294967295
	u4Len = (u4Len > 10) ? 10 : u4Len;

	for (u4Idx = 0;
		u4Idx < u4Len;
		u4Idx++)
	{
		if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9'))
		{
			u4ReturnValue *= 10;
			u4ReturnValue += (u32)(u8)(pszStr[u4Idx] - '0');
		}
		else
		{
			return 0;
		}
	}

	return u4ReturnValue;
}

u32 StrToInt(const s8* pszStr)
{
	u32 u4Len;

	if (pszStr == NULL)
	{
		return 0;
	}

	u4Len = x_strlen(pszStr);

	if (u4Len > 2)
	{
		if ((pszStr[0] == '0') && (pszStr[1] == 'x'))
		{
			return StrToHex(&pszStr[2], u4Len - 2);
		}
	}

	return StrToDec(pszStr, u4Len);
}

#endif


