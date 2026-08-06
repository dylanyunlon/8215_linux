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

#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"
#include "cli_common.h"

#include <linux/types.h>


#ifdef __linux__
#define WM_USER         0x400
#endif

#define MM_WOM_SETSECONDARYGAINCLASS   (WM_USER)
#define MM_WOM_SETSECONDARYGAINLIMIT   (WM_USER+1)

/* send sound through speaker if dw1 is TRUE,
	send through default if FALSE */
#define MM_WOM_FORCESPEAKER   (WM_USER+2)

#define MM_WAV_MTK_BASE         (WM_USER +  0x10)
#define MM_SPEECH_ENABLE        (MM_WAV_MTK_BASE + 0x0)
#define MM_SPH_GETVOLUME        (MM_WAV_MTK_BASE + 0x1)
#define MM_SPH_SETVOLUME        (MM_WAV_MTK_BASE + 0x2)    /* 0x412 */

#define WODM_MTK_VOLUME           (MM_WAV_MTK_BASE + 0x10)
#define WODM_MTK_ENABLE_FADEOUT   (MM_WAV_MTK_BASE + 0x11)

#define WODM_MTK_GET_SPH_PARA     (MM_WAV_MTK_BASE + 0x12)
#define WODM_MTK_SET_SPH_PARA     (MM_WAV_MTK_BASE + 0x13)
#define WODM_MTK_GET_DMNR_PARA    (MM_WAV_MTK_BASE + 0x14)
#define WODM_MTK_SET_DMNR_PARA    (MM_WAV_MTK_BASE + 0x15)

#define WODM_MTK_DUMP_REGISTERS   (MM_WAV_MTK_BASE + 0x16)

#define WODM_MTK_GET_SPH_GAIN     (MM_WAV_MTK_BASE + 0x18)
#define WODM_MTK_SET_SPH_GAIN     (MM_WAV_MTK_BASE + 0x19)

#define WODM_MTK_GET_DMNR_EN      (MM_WAV_MTK_BASE + 0x1A)
#define WODM_MTK_SET_DMNR_EN       (MM_WAV_MTK_BASE + 0x1B)

#define WODM_MTK_GET_SPH_LOG      (MM_WAV_MTK_BASE + 0x1C)
#define WODM_MTK_SET_SPH_LOG      (MM_WAV_MTK_BASE + 0x1D)

#define WODM_MTK_GET_REG_KEY      (MM_WAV_MTK_BASE + 0x1E)
#define WODM_MTK_SET_REG_KEY      (MM_WAV_MTK_BASE + 0x1F)

#define WODM_MTK_GET_SPH_DELAY    (MM_WAV_MTK_BASE + 0x20)
#define WODM_MTK_SET_SPH_DELAY    (MM_WAV_MTK_BASE + 0x21)

#define WODM_MTK_UL_USE_FILE    (MM_WAV_MTK_BASE + 0x22)
#define WODM_MTK_DL_USE_FILE    (MM_WAV_MTK_BASE + 0x24)
#define WODM_MTK_BT_LOOPBACK    (MM_WAV_MTK_BASE + 0x26)

#define WODM_MTK_DL_GAIN        (MM_WAV_MTK_BASE + 0x27)

#define WODM_MTK_GET_SPH_PARA2     (MM_WAV_MTK_BASE + 0x30)
#define WODM_MTK_SET_SPH_PARA2     (MM_WAV_MTK_BASE + 0x31)


#define DUMP_AOUT_REG              0
#define DUMP_MIC_REG               1
#define DUMP_ASRC_REG              2
#define DUMP_BT_REG                3

extern bool g_fgDmnrConnected;

#define DMNR_STATE_IDLE     0x0001
#define DMNR_STATE_REC      0x0002
#define DMNR_STATE_PLAY_REC 0x0004
#define DMNR_STATE_R_DATA   0x0003

#define DMNR_CMD_EXIT        ('e'<<24|'x'<<16|'i'<<8|'t')
#define DMNR_CMD_REC         ('r'<<24|'e'<<16|'c'<<8|'o')
#define DMNR_CMD_STOP        ('s'<<24|'t'<<16|'o'<<8|'p')
#define DMNR_CMD_PLAY_REC    ('p'<<24|'r'<<16|'e'<<8|'c')
#define DMNR_CMD_QUERY       ('q'<<24|'u'<<16|'e'<<8|'r')
#define DMNR_CMD_GET_PARA    ('g'<<24|'p'<<16|'a'<<8|'r')
#define DMNR_CMD_SET_PARA    ('s'<<24|'p'<<16|'a'<<8|'r')
#define DMNR_CMD_SET_ULGAIN  ('s'<<24|'u'<<16|'l'<<8|'g')
#define DMNR_CMD_SET_DLGAIN  ('s'<<24|'d'<<16|'l'<<8|'g')
#define DMNR_CMD_GET_ULGAIN  ('g'<<24|'u'<<16|'l'<<8|'g')
#define DMNR_CMD_GET_DLGAIN  ('g'<<24|'d'<<16|'l'<<8|'g')
#define DMNR_CMD_GET_AEC     ('g'<<24|'a'<<16|'e'<<8|'c')
#define DMNR_CMD_SET_AEC     ('s'<<24|'a'<<16|'e'<<8|'c')

#define DMNR_STATE_IDLE_STR  ('i'<<24|'d'<<16|'l'<<8|'e')
bool g_fgDmnrPlay = FALSE;
bool g_fgDmnrRec = FALSE;

static u32 _u4DmnrState = DMNR_STATE_IDLE;
static u32 _DmnrRecInit();
static u32 _DmnrPlayRecInit();

bool DMNR_Get_Byte(u32 u4TimeOut, u8 *pbValue)
{
	u32 dwRet;

	dwRet = WaitForSingleObject(_hDmnrSema, u4TimeOut);
	if (WAIT_OBJECT_0 == dwRet) {
		EnterCriticalSection(&_rCliCs);
		*pbValue = (u8)_szCliCmdBuffer[_u4CmdReadPtr++];
		if (_u4CmdReadPtr >= CLI_CMD_BUF_SIZE)
			_u4CmdReadPtr = 0;
		LeaveCriticalSection(&_rCliCs);
		return TRUE;
	}

	return FALSE;
}

void EmptyUARTReadFifo()
{
	u8 bTemp;

	while (DMNR_Get_Byte(0, &bTemp))
		;
}

u32 ReadDMNRCmd(u32 *pu4Value , u32 u4TimeOut)
{
	u32 u4ReadByte = 0;
	u8 *pbValue = (u8 *)pu4Value;

	pbValue += 3;
	*pu4Value = 0;
	do {
		if (!DMNR_Get_Byte(u4TimeOut, pbValue)) {
			break;
		}
		if ((*pbValue >= 'a') && (*pbValue <= 'z') ||
				(*pbValue >= '0') && (*pbValue <= '9')) {
			pbValue--;
			u4ReadByte++;
		} else {
			pbValue = (u8 *)pu4Value;
			pbValue += 3;
			u4ReadByte = 0;
		}
	} while (u4ReadByte < 4);

	return u4ReadByte;
}

u32 UartRead32(u32 *pu4Value , u32 u4TimeOut)
{
	u32 u4ReadByte = 0;
	u8 *pbValue = (u8 *)pu4Value;

	pbValue += 3;
	*pu4Value = 0;
	do {
		if (!DMNR_Get_Byte(u4TimeOut, pbValue)) {
			break;
		}
		pbValue--;
		u4ReadByte++;
	} while (u4ReadByte < 4);

	return u4ReadByte;

}

u32 UartReadBytes(u8 *pbValue , u32 u4Size, u32 u4TimeOut)
{
	u32 u4ReadByte = 0;

	do {
		if (!DMNR_Get_Byte(u4TimeOut, pbValue)) {
			break;
		}
		pbValue++;
		u4ReadByte++;
	} while (u4ReadByte < u4Size);

	return u4ReadByte;

}

#define DMNR_PARAM_NUM 96
static u32 _u4RecSize = 0;

u32 WINAPI DmnrCliThreadProc(void *lpParameter)
{
	u32 u4Cmd;
	u32 u4Size;

	CeSetThreadPriority(GetCurrentThread, 100);
	while (b_cli_init) {
		switch (_u4DmnrState) {
		case DMNR_STATE_IDLE:
			u4Size = ReadDMNRCmd(&u4Cmd, INFINITE);
			switch (u4Cmd) {
			case DMNR_CMD_EXIT:
				g_fgDmnrConnected = FALSE;
				cli_enable_kernel_log(_fgKernelLogEnable);
				Sleep(500);
				break;
			case DMNR_CMD_GET_PARA:
				{
					s16 ai2Params[DMNR_PARAM_NUM];

					waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_DMNR_PARA,
							(u32)ai2Params, 0);
					UartWriteBytes((u8 *)ai2Params, DMNR_PARAM_NUM*2);
				}
				break;
			case DMNR_CMD_SET_PARA:
				{
					s16 ai2Params[DMNR_PARAM_NUM];

					u4Size = UartReadBytes((u8 *)ai2Params, DMNR_PARAM_NUM*2,
							2000);
					if (u4Size == (DMNR_PARAM_NUM*2))
						waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_DMNR_PARA,
								(u32)ai2Params, 0);
				}
				break;
			case DMNR_CMD_GET_AEC:
				{
					u32 au4Params[28];

					waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_SPH_PARA2,
							(u32)au4Params, 0);
					UARTWriteData32(au4Params[13]);
					UARTWriteData32(au4Params[14]);
				}
				break;
			case DMNR_CMD_SET_AEC:
				{
					u32 au4Params[28];

					waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_SPH_PARA2,
							(u32)au4Params, 0);
					UartRead32(&au4Params[13], 5000);
					UartRead32(&au4Params[14], 5000);
					waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_SPH_PARA2,
							(u32)au4Params, 0);
				}
				break;
			case DMNR_CMD_SET_ULGAIN:
				{
					s32 i4Gain;

					if (4 ==  UartRead32((u32 *)&i4Gain, 5000)) {
						s32 i4CurGain = waveOutMessage((HWAVEOUT)0,
								WODM_MTK_GET_SPH_GAIN, 0, 0);

						i4CurGain += i4Gain;
						waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_SPH_GAIN,
								i4CurGain, 0);
					}
				}
				break;
			case DMNR_CMD_SET_DLGAIN:
				{
					s32 i4Gain;

					if (4 ==  UartRead32((u32 *)&i4Gain, 5000)) {
						s32 i4CurGain = waveOutMessage((HWAVEOUT)0,
								WODM_MTK_DL_GAIN, 0, 0);

						i4CurGain += i4Gain;
						waveOutMessage((HWAVEOUT)0, WODM_MTK_DL_GAIN, 1,
								i4CurGain);
					}
				}
				break;
			case DMNR_CMD_GET_ULGAIN:
				{
					s32 i4CurGain = waveOutMessage((HWAVEOUT)0,
							WODM_MTK_GET_SPH_GAIN, 0, 0);

					UARTWriteData32(i4CurGain);
				}
				break;
			case DMNR_CMD_GET_DLGAIN:
				{
					s32 i4CurGain = waveOutMessage((HWAVEOUT)0,
							WODM_MTK_DL_GAIN, 0, 0);

					UARTWriteData32(i4CurGain);
				}
				break;
			case DMNR_CMD_QUERY:
				UARTWriteData32(DMNR_STATE_IDLE_STR);
				break;
			case DMNR_CMD_REC:
				if (!_DmnrRecInit()) {
					_u4DmnrState = DMNR_STATE_REC;
				}
				EmptyUARTReadFifo();
				break;
			case DMNR_CMD_PLAY_REC:
				if (!_DmnrPlayRecInit()) {
					_u4DmnrState = DMNR_STATE_R_DATA;
					if (!DMNR_Receive_PlayData()) {
						_u4DmnrState = DMNR_STATE_PLAY_REC;
						DMNR_StartPlay();
						DMNR_Record(_u4RecSize);
						EmptyUARTReadFifo();
					}
				}
				break;
			default:
				break;
			}
			EmptyUARTReadFifo();
			break;
		case DMNR_STATE_REC:
			u4Size = ReadDMNRCmd(&u4Cmd, 200);
			if (((4 == u4Size) && (DMNR_CMD_STOP == u4Cmd)) || !g_fgDmnrRec) {
				_u4DmnrState = DMNR_STATE_IDLE;
				DMNR_Stop_Record();
				UARTWriteData32(DMNR_CMD_STOP);
			}
			break;
		case DMNR_STATE_PLAY_REC:
			u4Size = ReadDMNRCmd(&u4Cmd, 200);
			if (((4 == u4Size) && (DMNR_CMD_STOP == u4Cmd))
					|| (!g_fgDmnrRec && !g_fgDmnrPlay)) {
				DMNR_Stop_Record();
				DMNR_StopPlay();
				_u4DmnrState = DMNR_STATE_IDLE;
				UARTWriteData32(DMNR_CMD_STOP);
			}
			break;
		case DMNR_STATE_R_DATA:
			if (!DMNR_Receive_PlayData()) {
				_u4DmnrState = DMNR_STATE_PLAY_REC;
				DMNR_StartPlay();
				DMNR_Record(_u4RecSize);
			}
			break;
		}
	}
	return 0;
}


static s32 _CLI_DumpReg(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Reg = DUMP_AOUT_REG;

	if (i4Argc < 2)
		return -1;

	u4Reg = _ttoi(szArgv[1]);
	if (u4Reg > DUMP_BT_REG)
		return -1;
	waveOutMessage((HWAVEOUT)0, WODM_MTK_DUMP_REGISTERS, u4Reg, 0);

	return 0;
}

static s32 _CLI_MicGain(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Gain = 0;

	if (i4Argc < 2) {
		u4Gain = waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_SPH_GAIN, 0, 0);
		pr_info(TEXT("Read Speech MIC Gain = %d (%d DB) \r\n"), u4Gain,
				u4Gain-14);
		return 0;
	}

	u4Gain = _ttoi(szArgv[1]);

	waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_SPH_GAIN, u4Gain, 0);
	pr_info(TEXT("Set Speech MIC Gain = %d (%d DB) \r\n"), u4Gain, u4Gain-14);

	return 0;
}

static s32 _CLI_EnableSphLog(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Enable = 0;

	if (i4Argc < 2) {
		u4Enable = waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_SPH_LOG, 0, 0);
		pr_info(TEXT("Read Speech Log = %d \r\n"), u4Enable);
		return 0;
	}

	u4Enable = _ttoi(szArgv[1]);

	waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_SPH_LOG, u4Enable, 0);
	pr_info(TEXT("Write Speech Log = %d \r\n"), u4Enable);

	return 0;
}


static s32 _CLI_DmnrEnalbe(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Enable = 0;

	if (i4Argc < 2) {
		u4Enable = waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_DMNR_EN, 0, 0);
		pr_info(TEXT("Read DMNR Enable = %d \r\n"), u4Enable);
		return 0;
	}

	u4Enable = _ttoi(szArgv[1]);

	waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_DMNR_EN, u4Enable, 0);
	pr_info(TEXT("Set DMNR Enable = %d \r\n"), u4Enable);

	return 0;
}
#define AEC_PARAM_NUM 28
static s32 _CLI_SphParams(s32 i4Argc, const s8 **szArgv)
{
	u32 au4Params[AEC_PARAM_NUM];
	u32 u4Start = 0;
	s32 i4PStart = 1;

	waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_SPH_PARA, (u32)au4Params, 0);
	if (i4Argc < 2) {
		u32 u4Size;

		pr_info(TEXT("Speech Parameter"));
		for (u4Size = 0; u4Size < AEC_PARAM_NUM; u4Size++) {
			if (!(u4Size % 10))
				pr_info(TEXT("\r\n"));
			pr_info(TEXT("%d, "), au4Params[u4Size]);
		}
		return 0;
	}

	if ((i4Argc >= 3) && (i4Argc <= AEC_PARAM_NUM)) {
		u4Start = _ttoi(szArgv[1]);
		i4PStart = 2;
	}
	while ((i4PStart < i4Argc) && (u4Start < AEC_PARAM_NUM)) {
		au4Params[u4Start] = _ttoi(szArgv[i4PStart]);
		i4PStart++;
		u4Start++;
	}

	waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_SPH_PARA, (u32)au4Params, 0);

	return 0;
}

static s32 _CLI_SphParams2(s32 i4Argc, const s8 **szArgv)
{
	u32 au4Params[AEC_PARAM_NUM];
	u32 u4Start = 0;
	s32 i4PStart = 1;

	waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_SPH_PARA2, (u32)au4Params, 0);
	if (i4Argc < 2) {
		u32 u4Size;

		pr_info(TEXT("Speech Parameter"));
		for (u4Size = 0; u4Size < AEC_PARAM_NUM; u4Size++) {
			if (!(u4Size % 10))
				pr_info(TEXT("\r\n"));
			pr_info(TEXT("%d, "), au4Params[u4Size]);
		}
		return 0;
	}

	if ((i4Argc >= 3) && (i4Argc <= AEC_PARAM_NUM)) {
		u4Start = _ttoi(szArgv[1]);
		i4PStart = 2;
	}
	while ((i4PStart < i4Argc) && (u4Start < AEC_PARAM_NUM)) {
		au4Params[u4Start] = _ttoi(szArgv[i4PStart]);
		i4PStart++;
		u4Start++;
	}

	waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_SPH_PARA2, (u32)au4Params, 0);

	return 0;
}


static s32 _CLI_DmnrParams(s32 i4Argc, const s8 **szArgv)
{
	s16 ai2Params[DMNR_PARAM_NUM];
	u32 u4Start = 0;
	s32 i4PStart = 1;

	waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_DMNR_PARA, (u32)ai2Params, 0);
	if (i4Argc < 2) {
		u32 u4Size;

		pr_info(TEXT("Speech DMNR Parameter"));
		for (u4Size = 0; u4Size < DMNR_PARAM_NUM; u4Size++) {
			if (!(u4Size % 10))
				pr_info(TEXT("\r\n"));
			pr_info(TEXT("%d, "), ai2Params[u4Size]);
		}
		return 0;
	}

	if ((i4Argc >= 3) && (i4Argc <= DMNR_PARAM_NUM)) {
		u4Start = _ttoi(szArgv[1]);
		i4PStart = 2;
	}
	while ((i4PStart < i4Argc) && (u4Start < DMNR_PARAM_NUM)) {
		ai2Params[u4Start] = (s16)_ttoi(szArgv[i4PStart]);
		i4PStart++;
		u4Start++;
	}

	waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_DMNR_PARA, (u32)ai2Params, 0);

	return 0;
}

static s32 _CLI_SpeechDelay(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Delay = 0;

	if (i4Argc < 2) {
		u4Delay = waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_SPH_DELAY, 0, 0);
		pr_info(TEXT("Speech delay = %d \r\n"), u4Delay);
		return 0;
	}

	u4Delay = _ttoi(szArgv[1]);

	waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_SPH_DELAY, u4Delay, 0);
	pr_info(TEXT("Speech delay = %d \r\n"), u4Delay);

	return 0;
}


static s32 _CLI_RegKey(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Value = 0;
	s8 szKey[100];

	if (i4Argc < 2) {
		return -1;
	}

	while (szArgv[1][u4Value]) {
		szKey[u4Value] = szArgv[1][u4Value];
		u4Value++;
	}
	szKey[u4Value] = 0;

	if (i4Argc < 3) {
		u4Value = waveOutMessage((HWAVEOUT)0, WODM_MTK_GET_REG_KEY,
				(u32)szKey, 0);
		pr_info(TEXT("Key %s = %d \r\n"), szKey, u4Value);
		return 0;
	}

	u4Value = _ttoi(szArgv[2]);

	waveOutMessage((HWAVEOUT)0, WODM_MTK_SET_REG_KEY, (u32)szKey, u4Value);
	pr_info(TEXT("Set Key %s = %d \r\n"), szKey, u4Value);

	return 0;
}


static s32 _CLI_ToolConnect(s32 i4Argc, const s8 **szArgv)
{
	g_fgDmnrConnected = TRUE;
	cli_enable_kernel_log(FALSE);
	Sleep(50);
	UARTWriteData32(DMNR_STATE_IDLE_STR);

	return 0;
}


static s32 _CLI_BTAttrGetSet(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Idx = 0;
	u32 u4Value;

	if (i4Argc < 2) {
		return 0;
	}

	u4Idx = _ttoi(szArgv[1]);
	switch (u4Idx) {
	case 0:
		u4Idx = WODM_MTK_UL_USE_FILE;
		break;

	case 1:
		u4Idx = WODM_MTK_DL_USE_FILE;
		break;
	case 3:
		u4Idx = WODM_MTK_DL_GAIN;
		break;
	default:
	case 2:
		u4Idx = WODM_MTK_BT_LOOPBACK;
		break;
	}
	if (i4Argc < 3) {
		u4Value = waveOutMessage((HWAVEOUT)0, u4Idx, 0, 0);
		pr_info(TEXT("Read BT Data Setting (0x%x) = %d \r\n"), u4Idx, u4Value);
		return 0;
	}

	u4Value = _ttoi(szArgv[2]);
	waveOutMessage((HWAVEOUT)0, u4Idx, 1, u4Value);
	pr_info(TEXT("Set BT Data Setting (0x%x) = %d \r\n"), u4Idx, u4Value);

	return 0;
}




/******************************************************************************
* Variable      : cli default table
******************************************************************************/
CLI_EXEC_T _arWavCmdTbl[] = {
	{
		TEXT("DumpReg"),                    /* pszCmdStr */
		TEXT("dr"),
		_CLI_DumpReg,                       /* execution function */
		NULL,
		TEXT("dr reg (0: AOUT, 1: MIC, 2: ASRC, 3: BT"),
		CLI_GUEST
	},
	{
		TEXT("MicGain"),                  /* pszCmdStr */
		TEXT("mg"),
		_CLI_MicGain,              /* execution function */
		NULL,
		TEXT("mg gain(0-63)"), /* Read or Set MIC In Gian (Value - 14 DB) */
		CLI_GUEST
	},
	{
		TEXT("SphLog"),                    /* pszCmdStr */
		TEXT("sl"),
		_CLI_EnableSphLog,                       /* execution function */
		NULL,
		TEXT("sl 1/0 (enable/disable)"),
		CLI_GUEST
	},
	{
		TEXT("Dmnr"),                  /* pszCmdStr */
		TEXT("dmnr"),
		_CLI_DmnrEnalbe,              /* execution function */
		NULL,
		TEXT("dmnr 1/0 (enable/disable)"),
		CLI_GUEST
	},
	{
		TEXT("SphParams"),                  /* pszCmdStr */
		TEXT("sp"),
		_CLI_SphParams,              /* execution function */
		NULL,
		TEXT("sp start param1 param2 ..."),
		CLI_GUEST
	},
	{
		TEXT("SphParams2"),					/* pszCmdStr */
		TEXT("sp2"),
		_CLI_SphParams2,            /* execution function */
		NULL,
		TEXT("sp2 start param1 param2 ..."),
		CLI_GUEST
	},
	{
		TEXT("DmnrParams"),                  /* pszCmdStr */
		TEXT("dp"),
		_CLI_DmnrParams,              /* execution function */
		NULL,
		TEXT("dp start param1 param2 ..."),
		CLI_GUEST
	},
	{
		TEXT("SphDelay"),                  /* pszCmdStr */
		TEXT("sd"),
		_CLI_SpeechDelay,              /* execution function */
		NULL,
		TEXT("sd delay (samples)"),
		CLI_GUEST
	},
	{
		TEXT("RegistryKey"),                  /* pszCmdStr */
		TEXT("rk"),
		_CLI_RegKey,              /* execution function */
		NULL,
		TEXT("rk key value"),
		CLI_GUEST
	},
	{
		TEXT("ToolConnect"),                  /* pszCmdStr */
		TEXT("tc"),
		_CLI_ToolConnect,              /* execution function */
		NULL,
		TEXT("tc"),
		CLI_GUEST
	},
	{
		TEXT("BTSetting"),				  /* pszCmdStr */
		TEXT("bt"),
		_CLI_BTAttrGetSet,			   /* execution function */
		NULL,
		TEXT("bt index(0:UL, 1: DL, 2: loopback) enable(0/1)."),
		CLI_GUEST
	},

	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};

static u32 _u4RecCurSize = 0;
static u32 _u4RecFS = 8000;
static u32 _u4RecBPS = 16;
static u32 _u4RecChannles = 2;

static u32 _DmnrRecInit()
{
	if (4 !=  UartRead32(&_u4RecSize, 500)) {
		_u4RecSize = 8000*2*2*20;
	} else if (_u4RecSize < (8000*2*2*10)) {
		_u4RecSize = 8000*2*2*10;
	} else if (_u4RecSize > (8000*2*2*40)) {
		_u4RecSize = 8000*2*2*40;
	}

	DMNR_Record(_u4RecSize);
	return 0;
}

static u32 _u4PlaySize = 0;
static u32 _u4PlaycCurSize = 0;
static u32 _u4ReceiveSize = 0;
static u32 _u4PlayFS = 8000;
static u32 _u4PlayBPS = 16;
static u32 _u4PlayChannles = 2;

static u32 _DmnrPlayRecInit()
{
	_u4PlaycCurSize = 0;
	_u4ReceiveSize = 0;
	_u4RecCurSize = 0;
	_u4RecSize = 8000*2*2*20;
#if 1
	if (4 !=  UartRead32(&_u4PlayFS, 5000)) {
		return 1;
	}
	if (4 !=  UartRead32(&_u4PlayChannles, 5000)) {
		return 1;
	}
	if (4 !=  UartRead32(&_u4PlayBPS, 5000)) {
		return 1;
	}
	if (4 !=  UartRead32(&_u4PlaySize, 5000)) {
		return 1;
	}
	if (4 !=  UartRead32(&_u4RecSize, 5000)) {
		return 1;
	}
#endif

	if (_u4RecSize < (8000*2*2*10)) {
		_u4RecSize = 8000*2*2*10;
	} else if (_u4RecSize > (8000*2*2*40)) {
		_u4RecSize = 8000*2*2*40;
	}
	return DMNR_Init_PlayFile(_u4PlayFS, _u4PlayChannles, _u4PlaySize);
}



