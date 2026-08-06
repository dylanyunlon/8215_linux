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
#include "windows.h"
#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"
#include "drv_aud.h"
#include "x_aud_dec.h"
#include "aud_ioctrl.h"
#include "x_typedef.h"

#include <linux/types.h>

#ifndef __linux__
#include "stdlib.h"
#else
#include "winutil.h"
#include "linux/kernel.h"
#include "aud_cmd.h"
#include "pcm_audconf.h"
#endif


#define x_strncasecmp strncmp

AUD_DEC_CLI_CFG tCliCfg;

u32 g_u4Value = 0;
PCM_DTMF_CONF           g_rDtmfConf = {0};


#ifndef __linux__
static HANDLE g_hAudCliCmdIOCTL = INVALID_HANDLE_VALUE;
static bool _CLI_AUD_OpenDrv(void);

#define CLI_AUD_CHECK_DRV_HANDLE()              \
{                                               \
	if (!_CLI_AUD_OpenDrv()) {                  \
		return -1;                              \
	}                                           \
}

#define CLI_AUD_CLOSE_DRV()                     \
{                                               \
	CloseHandle(g_hAudCliCmdIOCTL);             \
	g_hAudCliCmdIOCTL = INVALID_HANDLE_VALUE;   \
	return 0;                                   \
}

#define CLI_AUD_IOCTL()                                          \
{                                                                \
	if (!DeviceIoControl(g_hAudCliCmdIOCTL,                      \
			IOCTL_AUDIO_SET_CLI_CMD_INFO,                        \
			&tCliCfg, sizeof(tCliCfg), NULL, 0, NULL, NULL)) {   \
		pr_err(TEXT("IOCTL_AUDIO_SET_CLI_CMD_INFO err = %d\r\n"),\
				GetLastError());                                 \
	}                                                            \
}

static bool _CLI_AUD_OpenDrv(void)
{
	if (g_hAudCliCmdIOCTL == INVALID_HANDLE_VALUE) {
		g_hAudCliCmdIOCTL = CreateFile(AUD_DEV_NAME,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, 0, NULL);
		if (g_hAudCliCmdIOCTL == INVALID_HANDLE_VALUE) {
			pr_err(TEXT("[AUD_CLI]CreateFile Fail: INVALID_HANDLE_VALUE\r\n"));
			return FALSE;
		}
	}
	return TRUE;
}
#else

#define _ttoi(a)   StrToInt(a)
#define CLI_AUD_CHECK_DRV_HANDLE()
#define CLI_AUD_CLOSE_DRV()                     \
{                                               \
	return 0;                                   \
}

#define CLI_AUD_IOCTL()   AudSetCliCmd(tCliCfg.eAudCliType, tCliCfg.u4arg1,\
			tCliCfg.u4arg2, tCliCfg.u4arg3, tCliCfg.u4arg4,\
			(const char **)tCliCfg.ptParam)

#define CLI_PCM_CHECK_DRV_HANDLE()
#define CLI_PCM_CLOSE_DRV()
#define CLI_PCM_IOCTL()   pcm_audconf_ioctl(0, tCliCfg.eAudCliType,\
			(void *)tCliCfg.u4arg1, tCliCfg.u4arg2)

#endif


static s32 _CLI_AudTurnOnLog(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		tCliCfg.u4arg1 = 9;
		pr_info(TEXT("[Command]drv.aud.log 9\r\n"));
	} else {
		tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		pr_info(TEXT("[Command]drv.aud.log %d\r\n"), (int)tCliCfg.u4arg1);
	}
	tCliCfg.eAudCliType = AUD_DEC_CLI_CMD_LOG;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CheckCmd(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType = AUD_DEC_CLI_CHECK;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_SeRbst(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.se.rb \r\n"));
	} else {
		pr_info(TEXT("[Command]drv.se.rb \r\n"));
		return -1;
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_RB_ST;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_SeEqst(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.se.eq \r\n"));
	} else {
		pr_info(TEXT("[Command]drv.se.eq \r\n"));
		return -1;
	}

	tCliCfg.eAudCliType =  AUD_DEC_CLI_EQ_ST;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_SePl2st(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.se.pl2 \r\n"));
	} else {
		pr_info(TEXT("[Command]drv.se.pl2 \r\n"));
		return -1;
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_PL2_ST;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_SeBassmst(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.se.bassm \r\n"));
	} else {
		pr_info(TEXT("[Command]drv.se.bassm \r\n"));
		return -1;
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_BASSM_ST;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_SeSpecst(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.se.spec \r\n"));
	} else {
		pr_info(TEXT("[Command]drv.se.spec \r\n"));
		return -1;
	}

	tCliCfg.eAudCliType =  AUD_DEC_CLI_SPECT_ST;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_AudDumpAfifo(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.afifo [0 or 1] [filename]\r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.ptParam = *(void **)&szArgv[2];
	pr_info(TEXT("[Command]drv.aud.afifo[filename] %s\r\n"),
			(char *)tCliCfg.ptParam);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_DUMP_AFIFO;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspStatus(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType = AUD_DEC_CLI_Q;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspShowDram(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info(TEXT("Usage:chdelay[addr][length]\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);

	tCliCfg.eAudCliType = AUD_DEC_CLI_DSP_CM;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspCfg(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType = AUD_DEC_CLI_DSP_CFG;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspShowIecRegs(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType = AUD_DEC_CLI_IECREGS;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspGetPTSInfo(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType = AUD_DEC_CLI_PI;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


/* yucai */
static s32 _CLI_AudCheckState(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType = AUD_DEC_CLI_ST;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_AudChangeVol(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info(TEXT("Usage:chdelay[decoder id][value]\n"));
		pr_info(TEXT("decoder[id]   0:FIRST        1:SECOND\n"));
		pr_info(TEXT("[Volume]    0~100\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	tCliCfg.eAudCliType = AUD_DEC_CLI_V;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_AudChangeSrcVol(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info(TEXT("Usage:src volume[source id][value]\n"));
		pr_info(TEXT("source [id] 0~5: front: usb, linein1, linein2, smix1, smix2, dvp \n"));
		pr_info(TEXT("source [id] 6~11: rear: usb, linein1, linein2, smix1, smix2, dvp \n"));
		pr_info(TEXT("[Volume]    0~0x20000\n"));
		return -1;
	}
	pr_info(TEXT("_CLI_AudChangeSrcVol , tCliCfg.u4arg1 %d, tCliCfg.u4arg1 0x%lx\n"), tCliCfg.u4arg1,tCliCfg.u4arg2);
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	tCliCfg.eAudCliType = AUD_DEC_CLI_V_SRC;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_AudBitstreamUr(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("Usage:need a prameter"));
		pr_info(TEXT("   0:OFF       1:ON\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType = AUD_DEC_CLI_UR;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_AudPrintAu(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("Usage:need a prameter"));
		pr_info(TEXT("   0:OFF       1:ON\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType = AUD_DEC_CLI_AUD_PA;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_AudBypassSE(s32 i4Argc, const s8 **szArgv)
{
	char *Pubf1;

	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("Usage:need a prameter"));
		pr_info(TEXT("   0:OFF       1:ON\n"));
		return -1;
	}
	tCliCfg.ptParam = *(void **)&szArgv[1];
	Pubf1 = (char *)szArgv[1];

	pr_info(TEXT("[Command]drv.aud.afifo[filename] %s\r\n"),
			(char *)tCliCfg.ptParam);
	/* yucai */
	if (0 == x_strncasecmp(Pubf1, "r", 1)) {
		tCliCfg.u4arg1 = 1;
		pr_info(TEXT("[Command]drv.aud.afifo[filename] %d\r\n"),
				(int)tCliCfg.u4arg1);
	} else if (0 == x_strncasecmp(Pubf1, "e", 1)) {
		tCliCfg.u4arg1 = 2;
		pr_info(TEXT("[Command]drv.aud.afifo[filename] %d\r\n"),
				(int)tCliCfg.u4arg1);
	} else if (0 == x_strncasecmp(Pubf1, "p", 1)) {
		tCliCfg.u4arg1 = 3;
		pr_info(TEXT("[Command]drv.aud.afifo[filename] %d\r\n"),
				(int)tCliCfg.u4arg1);
	}
	tCliCfg.eAudCliType = AUD_DEC_CLI_AUD_BYPASS;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_AudAout(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info(TEXT("[Command]drv.aud.aout [0:stop 1~12 AOUT1channel id 16-17AOUT2channelId] [filename]\r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.ptParam = *(void **)&szArgv[2];
	pr_info(TEXT("[Command]drv.aud.aout [0:stop 1~12 AOUT1channel id 16-17AOUT2channelId] [filename] %s \r\n"),
			(char *)tCliCfg.ptParam);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_DUMP_AOUT;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspDspShareInfo(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.dsp.sh [groupID] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	pr_info(TEXT("[Command]drv.aud.dsp.sh [groupID:%d] \r\n"),
			(int)tCliCfg.u4arg1);

	tCliCfg.eAudCliType = AUD_DEC_CLI_DSP_SH;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspDspShareInfoWrite(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 5) {
		pr_info(TEXT("[Command]drv.aud.dsp.shw [groupID][byte_address][value][size] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	tCliCfg.u4arg3 = (u32)_ttoi(szArgv[3]);
	tCliCfg.u4arg4 = (u32)_ttoi(szArgv[4]);
	pr_info(TEXT("[Command]drv.aud.dsp.sh [groupID:%d] \r\n"),
			(int)tCliCfg.u4arg1);

	tCliCfg.eAudCliType = AUD_DEC_CLI_DSP_SHW;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspDspShareInfoRead(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info(TEXT("[Command]drv.aud.dsp.r [DSPA:0 DSPB:1  DSPC:2][address][len] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)simple_strtol(szArgv[2], NULL, 16);
	tCliCfg.u4arg3 = (u32)_ttoi(szArgv[3]);

	pr_info(TEXT("[Command]drv.aud.dsp.r [DSP:%d][address:0x%x][len]\r\n"),
			(int)tCliCfg.u4arg1, (unsigned int)tCliCfg.u4arg2);

	tCliCfg.eAudCliType = AUD_DEC_CLI_DSP_R;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspDspSramWrite(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 4) {
		pr_info(TEXT("[Command]drv.aud.dsp.w [DSPA:0 DSPB:1  DSPC:2][address][value] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	tCliCfg.u4arg3 = (u32)_ttoi(szArgv[3]);
	pr_info(TEXT("[Command]drv.aud.dsp.w [DSP%d][address]  \r\n"),
			(int)tCliCfg.u4arg1);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_DSP_W;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_ADspDspWriteCommandDram(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info(TEXT("[Command]drv.aud.dsp.wcm [address][value] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	pr_info(TEXT("[Command]drv.aud.dsp.wcm [address : 0x%x]  \r\n"),
			(unsigned int)tCliCfg.u4arg1);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_WCM;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspDspPtsUpdateQueue(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info(TEXT("[Command]drv.aud.dsp.pl [DecID][number<0x80] \r\n"));
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	pr_info(TEXT("[Command]drv.aud.dsp.pu [Pts Number%d] \r\n"),
			(int)tCliCfg.u4arg2);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_PU;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ADspDspPtsLegalQueue(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info(TEXT("[Command]drv.aud.dsp.pl [DecID][number<0x80] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	pr_info(TEXT("[Command]drv.aud.dsp.pl [Pts Number%d]  \r\n"),
			(int)tCliCfg.u4arg2);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_PL;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_AUDExchangeIEC(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.digout.exi [0:OFF,1:ON] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	pr_info(TEXT("[Command]drv.aud.digout.exi [0:OFF,1:ON%d]  \r\n"),
			(int)tCliCfg.u4arg2);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_DIGOUT_EXI;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_AUDExchangeAout(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.digout.exa [0:OFF  1:ON] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	pr_info(TEXT("[Command]drv.aud.digout.exa [%d] \r\n \r\n"),
			(int)tCliCfg.u4arg2);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_DIGOUT_EXA;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_AUDSetFrontAout(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.digout.front [0:NONE  1:USB 2: LineIN ] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	pr_info(TEXT("[Command]drv.aud.digout.front [%d] \r\n \r\n"),
			(int)tCliCfg.u4arg1);

	tCliCfg.eAudCliType =  IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_AUDSetRearAout(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.digout.rear [0:NONE  1:USB 2: LineIN ] \r\n"));
		return -1;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	pr_info(TEXT("[Command]drv.aud.digout.rear [%d] \r\n \r\n"),
			(int)tCliCfg.u4arg1);

	tCliCfg.eAudCliType =  IOCTL_AUDIO_SET_REAR_MEDIA_TYPE;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_GpsMixPlayCmd(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.gpscmd.gpsplay\r\n"));
	}

	tCliCfg.eAudCliType =  AUD_DEC_CLI_GPSMIX_PLAY_CMD;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_GpsMixStopCmd(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.gpscmd.gpsstop\r\n"));
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_GPSMIX_STOP_CMD;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_GpsMixPauseCmd(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.gpscmd.gpspause\r\n"));
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_GPSMIX_PAUSE_CMD;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_GpsMixResumeCmd(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.gpscmd.gpsresuem\r\n"));
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_GPSMIX_RESUME_CMD;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_TestToneSelOut(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.tt.selout\r\n"));
	} else {
		tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_TESTTONE_SELFRNREAR;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_TestToneOnOff(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.tt.onoff\r\n"));
	} else {
		tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_TESTTONE_ONOFF;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_TestToneSelCh(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();
	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.tt.selch\r\n"));
	} else {
		tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_TESTTONE_SELCHANNEL;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_TestToneSelType(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();
	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.tt.seltype\r\n"));
	} else {
		tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	}
	tCliCfg.eAudCliType =  AUD_DEC_CLI_TESTTONE_SELTYPE;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_CSII_Help(s32 i4Argc, const s8 **szArgv)
{
	pr_info(TEXT("CSII CLI HELP:\r\n"));
	pr_info(TEXT(">>>  0-Switch, 0:Off, 1:On, 2:Auto\r\n"));
	pr_info(TEXT(">>>  1-Mode, 0:CINEMA, 1:PRO, 2:MUSIC, 3:MONO, 4:LCRS\r\n"));
	pr_info(TEXT(">>>  2-Phantom, 0:Off, 1:On\r\n"));
	pr_info(TEXT(">>>  3-FullBand, 0:Off, 1:On\r\n"));
	pr_info(TEXT(">>>  4-Focus Center, 0:Off, 1:On\r\n"));
	pr_info(TEXT(">>>  5-Focus Front, 0:Off, 1:On\r\n"));
	pr_info(TEXT(">>>  6-Focus Rear, 0:Off, 1:On\r\n"));
	pr_info(TEXT(">>>  7-TB Front, 0:Off, 1:On\r\n"));
	pr_info(TEXT(">>>  8-TB Sub, 0:Off, 1:On\r\n"));
	pr_info(TEXT(">>>  9-TB Rear, 0:Off, 1:On\r\n"));
	pr_info(TEXT(">>>  10-Front SS, 0x1:40HZ, 0x2:60HZ, 0x4:100HZ, 0x8:150HZ, 0x10:200HZ, 0x20:250HZ, 0x40:300HZ, 0x80:400HZ\r\n"));
	pr_info(TEXT(">>>  11-Sub SS, 0x1:40HZ, 0x2:60HZ, 0x4:100HZ, 0x8:150HZ, 0x10:200HZ, 0x20:250HZ, 0x40:300HZ, 0x80:400HZ\r\n"));
	pr_info(TEXT(">>>  12-Rear SS, 0x1:40HZ, 0x2:60HZ, 0x4:100HZ, 0x8:150HZ, 0x10:200HZ, 0x20:250HZ, 0x40:300HZ, 0x80:400HZ\r\n"));

	return 0;
}

static s32 _CLI_CSII_Switch(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_SWITCH;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_Mode(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_MODE;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_Phantom(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_PHANTOM;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_FB(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_FB;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_FocusC(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_FOCUS_CENTER;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_FocusF(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_FOCUS_FRONT;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_FocusR(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_FOCUS_REAR;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_TB(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_TB;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_SSF(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_F_SS;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_SSS(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_S_SS;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_CSII_SSR(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = AUD_SE_CSII_CTRL_R_SS;
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CSII;
	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

#ifndef __linux__
static HANDLE _hAudDec4Hdr = INVALID_HANDLE_VALUE;
static HANDLE g_hPlayThreadHdr = INVALID_HANDLE_VALUE;
static bool _bAudDec4PlayOn = FALSE;
#define AUDIO_FIFO_SIZE (3*2*1024)
AUD_DEC4_INFO_T _tAudA2dpInf;
#endif

static s32 _CLI_ADspDec4Init(s32 i4Argc, const s8 **szArgv)
{
#ifndef __linux__
	AUD_DRV_CONTEXT tAudContext;

	_hAudDec4Hdr = INVALID_HANDLE_VALUE;
	if (i4Argc < 4) {
		RETAILMSG(1, (L"dec4.init <bitdepth> <data endian> <samplerate><channel cnt>i4Argc = %D\r\n",
				i4Argc));
		RETAILMSG(1, (L"Example: <bitdepth> 0:8bit, 1:16bit, 2:24bit\r\n"));
		RETAILMSG(1, (L"Example: <samplerate> 0:big endian, 1:little endian\r\n"));
		RETAILMSG(1, (L"Example: <channel cnt> 0:mono, 1:listereo\r\n"));
		RETAILMSG(1, (L"Example: dec4.init 1 1 48000 1\r\n"));
		return 0;
	}
	_tAudA2dpInf.e_aud_fmt = AUD_DEC_FMT_A2DP;
	_tAudA2dpInf.t_aud_a2dp_info.eBitDepth = (AUD_DEC_A2DP_BIT_DEPTH)_ttoi(szArgv[1]);
	_tAudA2dpInf.t_aud_a2dp_info.eDataEndian = (AUD_DEC_A2DP_DATA_ENDIAN)_ttoi(szArgv[2]);
	_tAudA2dpInf.t_aud_a2dp_info.u4SmpRate = (u32)_ttoi(szArgv[3]);
	_tAudA2dpInf.t_aud_a2dp_info.u4channel_cnt = (u32)_ttoi(szArgv[4]);
	RETAILMSG(1, (L"[_AudDec4Play]_tAudA2dpInf bitdepth %d endian %d samplerate%d channel count %d. \r\n",
			_tAudA2dpInf.t_aud_a2dp_info.eBitDepth,
			_tAudA2dpInf.t_aud_a2dp_info.eDataEndian,
			_tAudA2dpInf.t_aud_a2dp_info.u4SmpRate,
			_tAudA2dpInf.t_aud_a2dp_info.u4channel_cnt));

	/* 1. Create the Hdr for the 4th instance. */
	_hAudDec4Hdr = CreateFile(AUD_DEV_NAME, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);

	if (_hAudDec4Hdr == INVALID_HANDLE_VALUE) {
		RETAILMSG(1, (L"[_CLI_ADspDec4Init]CreateFile _hAudDec4Hdr: INVALID_HANDLE_VALUE\r\n"));
	}

	/* 2. Set the context for audio 4th decode instance */
	tAudContext.u1DecId = (u8)SEC_DEC;
	DeviceIoControl(_hAudDec4Hdr, IOCTL_AUDIO_SET_DEC_CONTEXT,
			&tAudContext, sizeof(tAudContext),
			NULL, 0, NULL, NULL);
	RETAILMSG(1, (L"Create the 4th decode instance sucessful\r\n"));

	#endif

	return 0;
}

static s32 _CLI_ADspDec4Play(s32 i4Argc, const s8 **szArgv)
{
#ifndef __linux__
	if (g_hPlayThreadHdr == INVALID_HANDLE_VALUE) {
		g_hPlayThreadHdr = CreateThread(NULL, /* lpThreadAttributes */
				0, /* Stack Size */
				(LPTHREAD_START_ROUTINE)_AudDec4Play, /* lpStartAddress */
				szArgv[1],  /* lpParameter */
				0,      /* creation flags */
				NULL);  /* thread id */
	}
#endif
	return 0;
}

static s32 _CLI_ADspDec4Stop(s32 i4Argc, const s8 **szArgv)
{
#ifndef __linux__
	_bAudDec4PlayOn  = FALSE;
	RETAILMSG(1, (L"[_CLI_ADspDec4Stop]Stop audio decode \r\n"));
#endif
	return 0;
}

static s32 _CLI_ADspDec4UnInit(s32 i4Argc, const s8 **szArgv)
{
#ifndef __linux__
	if (g_hPlayThreadHdr != INVALID_HANDLE_VALUE) {
		CloseHandle(g_hPlayThreadHdr); /* Delete the dec4 thread */
	}
	g_hPlayThreadHdr = INVALID_HANDLE_VALUE;

	if (_hAudDec4Hdr != INVALID_HANDLE_VALUE) {
		CloseHandle(_hAudDec4Hdr);  /* Delete the audio dec4 instance */
	}
	_hAudDec4Hdr = INVALID_HANDLE_VALUE;
	RETAILMSG(1, (L"Uninstall the 4th decode instance sucessful\r\n"));
#endif
	return 0;
}


static s32 _CLI_EXTLINTEST(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();
	if (i4Argc <= 3) {
		pr_info(TEXT("please input args, according to follow Log:\n"));
		pr_info(TEXT("Usage: [LRCLK_FREQ] [BCLK_CYCLE] [BIT_WIDTH] [AUD_FMT_INTF] [AUD_OUT_SRC] [BIT_MODE] [BCK Reverse]\n"));
		pr_info(TEXT("[LRCLK_FREQ] 0 - 8K_HZ, 1 - 32K_HZ, 2-441K_HZ,3-48K_HZ, 4-88K2_HZ, 5-96K_HZ,6-192K_HZ, Now Use 48KHZ\n"));
		pr_info(TEXT("[BCLK_CYCLE] arg1:  0 - 16_CYCLE, 1 - 24_CYCLE, 2 - 32_CYCLE\n"));
		pr_info(TEXT("[u4ADCBitNum][BIT_WIDTH]arg2:  16/24\n"));
		pr_info(TEXT("[AUD_FMT_INTF]  0 - RJ, 1 - LJ, 2 - Reserved , 3- I2S,Now Use I2S\n"));
		pr_info(TEXT("[CLK_SRC] arg3: 0 - AOUT1 , 1- AOUT2, 2- Mline 3- Mphone, 4- Mline_26M, 5- Mline 27M,6-Mphone 26M,7-Mphone 27M,etc..\n"));
		pr_info(TEXT("[BIT_MODE]  0 - 16 1 - 24,Now Use 24BitMode\n"));
		pr_info(TEXT("[BCK Reverse] arg4: 0 - No Reverse 1 - Reverse\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	tCliCfg.u4arg3 = (u32)_ttoi(szArgv[3]);
	tCliCfg.u4arg4 = (u32)_ttoi(szArgv[4]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_EXT_LIN_TEST;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_ExtLdo(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("Please input args...\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_PWMDAC_EXT_LDO;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}
static s32 _CLI_SPDIFSelect(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("Please input args...\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_SPDIF;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}
static s32 _CLI_REARVOLUME(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();
	if (i4Argc < 2) {
		pr_info(TEXT("[Command]drv.aud.rvol\r\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_REAR_VOL_CONTROL;

	CLI_AUD_IOCTL();
	CLI_AUD_CLOSE_DRV();
}


static s32 _CLI_UpMix(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		tCliCfg.u4arg1 = 0;
		tCliCfg.u4arg2 = 0;
	} else {
		tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	}

	tCliCfg.eAudCliType =  AUD_DEC_CLI_UPMIX;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_LoudNess(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("drv.aud.ld[0-20dB]\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.eAudCliType =  AUD_DEC_CLI_LOUDNESS;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}



static s32 _CLI_DACTypeSel(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("drv.aud.ld[0-20dB]\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	tCliCfg.u4arg3 = (u32)_ttoi(szArgv[3]);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_DACSEL;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_Front_VolGain(s32 i4Argc, const s8 **szArgv)
{
	u32 u4FrontVolIndex;
	u32 u4MasterVolumeBase = (0x20000) / (100);
	u32 u4Gain = 0;

	CLI_AUD_CHECK_DRV_HANDLE();
	if (i4Argc < 1) {
		pr_info(TEXT("drv.aud.f_ml[0-100]\n"));
		return 0;
	}

	u4FrontVolIndex = (u32)_ttoi(szArgv[1]);

	u4Gain = u4MasterVolumeBase * u4FrontVolIndex;
	pr_info(TEXT("u4MasterVolumeBase=0x%x,u4Gain = 0x%x\r\n"),
			u4MasterVolumeBase, u4Gain);

	tCliCfg.u4arg1 = u4Gain;

	tCliCfg.eAudCliType =  AUD_DEC_CLI_FRNTVOL;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_Rear_VolGain(s32 i4Argc, const s8 **szArgv)
{
	u32 u4RearVolIndex;
	u32 u4MasterVolumeBase = 0x20000/100;
	u32 u4Gain = 0;

	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 1) {
		pr_info(TEXT("drv.aud.r_ml[0-100]\n"));
		return 0;
	}
	u4RearVolIndex = (u32)_ttoi(szArgv[1]);
	u4Gain = u4MasterVolumeBase * u4RearVolIndex;

	pr_info(TEXT("u4MasterVolumeBase=0x%x,u4Gain = 0x%x\r\n"),
			u4MasterVolumeBase, u4Gain);
	tCliCfg.u4arg1 = u4Gain;
	tCliCfg.eAudCliType =  AUD_DEC_CLI_REARVOL;
	CLI_AUD_IOCTL();
	CLI_AUD_CLOSE_DRV();
}
static const u32 _chTrimGain[41] = {
	0x0000A1E9, 0x0000AB81, 0x0000B5AA, 0x0000C06E, 0x0000CBD5, 0x0000D7E9, 0x0000E4B4, 0x0000F241,
	0x0001009C, 0x00010FD0, 0x00011FEB, 0x000130FB, 0x0001430D, 0x00015631, 0x00016A78, 0x00017FF2,
	0x000196B2, 0x0001AECB, 0x0001C852, 0x0001E35C, 0x00020000, 0x00021E57, 0x00023E79, 0x00026083,
	0x00028492, 0x0002AAC3, 0x0002D338, 0x0002FE13, 0x00032B77, 0x00035B8C, 0x00038E7B, 0x0003C46E,
	0x0003FD93, 0x00043A1B, 0x00047A3A, 0x0004BE25, 0x00050616, 0x0005524B, 0x0005A303, 0x0005F884,
	0x00065316
};

static s32 _CLI_Ch_VolGain(s32 i4Argc, const s8 **szArgv)
{
	u32 u4TrimIndex;
	u32 u4ChGain = 0;

	CLI_AUD_CHECK_DRV_HANDLE();
	if (i4Argc < 2) {
		pr_info(TEXT("Usage:[Channel][Index(0~40)]\n"));

		pr_info(TEXT("Usage:[Channel]0:L,1:R,2:Ls,3:Rs\n"));
		pr_info(TEXT("drv.aud.ch_vl[Channel][Index]\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);

	u4TrimIndex = (u32)_ttoi(szArgv[2]);

	u4ChGain = _chTrimGain[u4TrimIndex+20];


	pr_info(TEXT("tCliCfg.u4arg1=0x%x,u4ChGain = 0x%x\r\n"),
			(unsigned int)(tCliCfg.u4arg1), (unsigned int)u4ChGain);


	tCliCfg.u4arg2 = u4ChGain;
	tCliCfg.eAudCliType =  AUD_DEC_CLI_CHVOL;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_Print_VolGain(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();
	if (i4Argc < 2) {
		pr_info(TEXT("Usage:[u4VolMode][front:0,rear:1,Left:2,Right:3,Ls:4,Rs:5]\n"));
		return 0;
	}

	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);


	tCliCfg.eAudCliType =  AUD_DEC_CLI_PRINT_VOL_GAIN;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_Print_TTInfo(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType =  AUD_DEC_CLI_PRINT_TT_INFO;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_Print_UpMixGain(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType =  AUD_DEC_CLI_PRINT_UPMIX_GAIN;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_Print_LoudnessGain(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType =  AUD_DEC_CLI_PRINT_LOUDNESS_GAIN;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}
static s32 _CLI_LinRearBypassSet(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("drv.aud.rear_bypass[Lmode(0:front,1:rear),OnOffSel(0:on,1:off),GroupSel(1~5)]\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	tCliCfg.u4arg3 = (u32)_ttoi(szArgv[3]);


	tCliCfg.eAudCliType =  AUD_DEC_CLI_LIN_REAR_BYPASS;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_LRMixSet(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("drv.aud.lr_mix <Lmode(0: stero. 1: left only, 2: right only>\n"));
		return 0;
	}
	tCliCfg.u4arg1 = (AUD_DEC_LRMIX_OUTPUT_T)_ttoi(szArgv[1]);

	tCliCfg.eAudCliType =  AUD_DEC_CLI_SET_LRMIX;

	CLI_AUD_IOCTL();

	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_AudioDebug(s32 i4Argc, const s8 **szArgv)
{
	CLI_AUD_CHECK_DRV_HANDLE();

	tCliCfg.u4arg1 = 0xA5A5A5A5;
	tCliCfg.u4arg2 = 0xA5A5A5A5;
	tCliCfg.u4arg3 = 0xA5A5A5A5;
	tCliCfg.u4arg4 = 0xA5A5A5A5;

	if (i4Argc > 1) {
		tCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		if (i4Argc > 2) {
			tCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
			if (i4Argc > 3) {
				tCliCfg.u4arg3 = (u32)_ttoi(szArgv[3]);
				if (i4Argc > 4) {
					tCliCfg.u4arg4 = (u32)_ttoi(szArgv[4]);
					if (i4Argc > 5) {
						tCliCfg.ptParam = *(void **)&szArgv[5];
					}
				}
			}
		}
	}

	tCliCfg.eAudCliType =  AUD_DEC_CLI_DEBUG;

	CLI_AUD_IOCTL();
	CLI_AUD_CLOSE_DRV();
}

static s32 _CLI_EnableMicMute(s32 i4Argc, const s8 **szArgv)
{
	CLI_PCM_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		pr_info(TEXT("drv.aud.pcm.micmute <0:Disable 1:Enable>\r\n"));
		return 0;
	}

	tCliCfg.eAudCliType = SET_MIC_MUTE;
	g_u4Value = _ttoi(szArgv[1]);
	tCliCfg.u4arg1 = (UINT32)(&g_u4Value);
	tCliCfg.u4arg2 = sizeof(g_u4Value);

	CLI_PCM_IOCTL();

	CLI_PCM_CLOSE_DRV();

	return 0;
}

static s32 _CLI_MicGain(s32 i4Argc, const s8 **szArgv)
{
	CLI_PCM_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		tCliCfg.eAudCliType = GET_SPH_MIC_GAIN;
		g_u4Value = 0;
	} else {
		tCliCfg.eAudCliType = SET_SPH_MIC_GAIN;
		g_u4Value = _ttoi(szArgv[1]);
	}
	tCliCfg.u4arg1 = (UINT32)(&g_u4Value);
	tCliCfg.u4arg2 = sizeof(g_u4Value);

	CLI_PCM_IOCTL();

	CLI_PCM_CLOSE_DRV();

	return 0;
}

static s32 _CLI_TestDTMF(s32 i4Argc, const s8 **szArgv)
{
	CLI_PCM_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_err(TEXT("drv.aud.pcm.dt must set two parameters\r\n"));
		return 0;
	}
	tCliCfg.eAudCliType = SET_PCM_DTMF_TEST_USE_FILE;
	g_rDtmfConf.u4Param1 = _ttoi(szArgv[1]);
	g_rDtmfConf.u4Param2 = _ttoi(szArgv[2]);

	tCliCfg.u4arg1 = (UINT32)(&g_rDtmfConf);
	tCliCfg.u4arg2 = sizeof(g_rDtmfConf);

	CLI_PCM_IOCTL();

	CLI_PCM_CLOSE_DRV();

	return 0;
}

static s32 _CLI_SetDTMFLogRange(s32 i4Argc, const s8 **szArgv)
{
	CLI_PCM_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_err(TEXT("drv.aud.pcm.dr must set two parameters\r\n"));
		return 0;
	}
	tCliCfg.eAudCliType = SET_PCM_DTMF_LOG_RANGE;
	g_rDtmfConf.u4Param1 = _ttoi(szArgv[1]);
	g_rDtmfConf.u4Param2 = _ttoi(szArgv[2]);

	tCliCfg.u4arg1 = (UINT32)(&g_rDtmfConf);
	tCliCfg.u4arg2 = sizeof(g_rDtmfConf);

	CLI_PCM_IOCTL();

	CLI_PCM_CLOSE_DRV();

	return 0;
}

static s32 _CLI_DTMFParam(s32 i4Argc, const s8 **szArgv)
{
	CLI_PCM_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType = GET_PCM_DTMF_PARAM;
	g_u4Value = 0;
	tCliCfg.u4arg1 = (UINT32)(&g_u4Value);
	tCliCfg.u4arg2 = sizeof(g_u4Value);

	CLI_PCM_IOCTL();

	CLI_PCM_CLOSE_DRV();

	return 0;
}


static s32 _CLI_TurnOnPCMLog(s32 i4Argc, const s8 **szArgv)
{
	if (i4Argc < 2) {
		g_u4Value = 9;
		pr_info(TEXT("[Command]drv.aud.pcm.log 9\r\n"));
	} else {
		g_u4Value = _ttoi(szArgv[1]);
		pr_info(TEXT("[Command]drv.aud.pcm.log %d\r\n"), g_u4Value);
	}

	CLI_PCM_CHECK_DRV_HANDLE();

	tCliCfg.eAudCliType = SET_PCM_LOG;
	tCliCfg.u4arg1 = (UINT32)(&g_u4Value);
	tCliCfg.u4arg2 = sizeof(g_u4Value);

	CLI_PCM_IOCTL();

	CLI_PCM_CLOSE_DRV();

	return 0;
}

/******************************************************************************
* Variable : cli default table
******************************************************************************/
CLI_EXEC_T _arAudDspCmdTbl[] = {
	{TEXT("q"), TEXT("q"), _CLI_ADspStatus,
			NULL, TEXT("DSP status"), CLI_GUEST},
	{TEXT("cfg"), TEXT("cfg"), _CLI_ADspCfg,
			NULL, TEXT("DSP output info"), CLI_GUEST},
	/* yucai */
	{TEXT("share info"), TEXT("sh"), _CLI_ADspDspShareInfo,
			NULL, TEXT("Read Dsp Share Memory"), CLI_GUEST},
	{TEXT("share info write"), TEXT("shw"), _CLI_ADspDspShareInfoWrite,
			NULL, TEXT("Write Dsp Share Memory"), CLI_GUEST},
	{TEXT("PtsLegal"), TEXT("pl"), _CLI_ADspDspPtsLegalQueue,
			NULL, TEXT("Get Check Legal Pts"), CLI_GUEST},
	{TEXT("PtsUpdate"), TEXT("pu"), _CLI_ADspDspPtsUpdateQueue,
			NULL, TEXT("Get Update Pts"), CLI_GUEST},
	{TEXT("Write Command Dram"), TEXT("wcm"), _CLI_ADspDspWriteCommandDram,
			NULL, TEXT("Write Dsp Command Memory"), CLI_GUEST},
	{TEXT(" read sram"), TEXT("r"), _CLI_ADspDspShareInfoRead,
			NULL, TEXT("Read Dsp Sram"), CLI_GUEST},
	{TEXT("cm"), TEXT("cm"), _CLI_ADspShowDram,
			NULL, TEXT("DRAM Info"), CLI_GUEST},
	/* yucai */
	{TEXT("iecregs"), TEXT("iecregs"), _CLI_ADspShowIecRegs,
			NULL, TEXT("Show IEC register"), CLI_GUEST},
	{TEXT("pi"), TEXT("pi"), _CLI_ADspGetPTSInfo,
			NULL, TEXT("Get PTS Info"), CLI_GUEST},
	{TEXT("w"), TEXT("w"), _CLI_ADspDspSramWrite,
			NULL, TEXT("Write DSP SRAM"), CLI_GUEST},

	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};

/******************************************************************************
* Variable      : cli default table
******************************************************************************/
CLI_EXEC_T _arAudDspDigOutCmdTbl[] = {
	{TEXT("exchangeaout"), TEXT("exa"), _CLI_AUDExchangeAout,
			NULL, TEXT("Exchange Aout1 and Aout2 output"),
			CLI_GUEST},
	{TEXT("exchangeiec"), TEXT("exi"), _CLI_AUDExchangeIEC,
			NULL, TEXT("Exchange IEC2 to IEC1 output"), CLI_GUEST},
	{TEXT("setfront"),  TEXT("front"), _CLI_AUDSetFrontAout,
			NULL, TEXT("Set front aout media"), CLI_GUEST},
	{TEXT("setrear"),  TEXT("rear"), _CLI_AUDSetRearAout,
			NULL, TEXT("Set rear aout media"), CLI_GUEST},
	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}

};

CLI_EXEC_T _arAudGpsCmdTbl[] = {
	{TEXT("gpsplay"), TEXT("gpsplay"), _CLI_GpsMixPlayCmd,
			NULL, TEXT("GPS PLAY CMD"), CLI_GUEST},
	{TEXT("gpsstop"), TEXT("gpsstop"), _CLI_GpsMixStopCmd,
			NULL, TEXT("GPS STOP CMD"), CLI_GUEST},
	{TEXT("gpspause"), TEXT("gpspause"), _CLI_GpsMixPauseCmd,
			NULL, TEXT("GPS PAUSE CMD"), CLI_GUEST},
	{TEXT("gpsresume"), TEXT("gpsresume"), _CLI_GpsMixResumeCmd,
			NULL, TEXT("GPS RESUME CMD"), CLI_GUEST},

	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};

CLI_EXEC_T _arAudTTCmdTbl[] = {
	{TEXT("TestTone select OutPut"), TEXT("selout"), _CLI_TestToneSelOut,
			NULL, TEXT("Select TestTone OutPut"), CLI_GUEST},
	{TEXT("TestTone ONOFF"), TEXT("onoff"), _CLI_TestToneOnOff,
			NULL, TEXT("Select TestTone ONOFF"), CLI_GUEST},
	{TEXT("TestTone select channel"), TEXT("selch"), _CLI_TestToneSelCh,
			NULL, TEXT("Select TestTone Channel"), CLI_GUEST},
	{TEXT("TestTone select type"), TEXT("seltype"), _CLI_TestToneSelType,
			NULL, TEXT("Select TestTone Type"), CLI_GUEST},

	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};


CLI_EXEC_T _arAudCSIICmdTbl[] = {
	/* pszCmdStr */
	{TEXT("Help"),                TEXT("help"),         _CLI_CSII_Help,
		NULL, TEXT("Help"),                CLI_GUEST},
	{TEXT("ON/OFF"),              TEXT("switch"),       _CLI_CSII_Switch,
		NULL, TEXT("ON/OFF"),              CLI_GUEST},
	{TEXT("Mode select"),         TEXT("mode"),         _CLI_CSII_Mode,
		NULL, TEXT("Mode select"),         CLI_GUEST},
	{TEXT("Phantom On off"),      TEXT("ph"),           _CLI_CSII_Phantom,
		NULL, TEXT("Phantom On off"),      CLI_GUEST},
	{TEXT("FB On Off"),           TEXT("fb"),           _CLI_CSII_FB,
		NULL, TEXT("FB On Off"),           CLI_GUEST},
	{TEXT("Focus Center On Off"), TEXT("fc"),           _CLI_CSII_FocusC,
		NULL, TEXT("Focus Center On Off"), CLI_GUEST},
	{TEXT("Focus Front On Off"),  TEXT("ff"),           _CLI_CSII_FocusF,
		NULL, TEXT("Focus Front On Off"),  CLI_GUEST},
	{TEXT("Focus Rear On Off"),   TEXT("fr"),           _CLI_CSII_FocusR,
		NULL, TEXT("Focus Rear On Off"),   CLI_GUEST},
	{TEXT("TB On Off"),           TEXT("tbf"),          _CLI_CSII_TB,
		NULL, TEXT("TB On Off"),           CLI_GUEST},
	{TEXT("Speaker Size Front"),  TEXT("ssf"),          _CLI_CSII_SSF,
		NULL, TEXT("Speaker Size Front"),  CLI_GUEST},
	{TEXT("Speaker Size Sub"),    TEXT("sss"),          _CLI_CSII_SSS,
		NULL, TEXT("Speaker Size Sub"),    CLI_GUEST},
	{TEXT("Speaker Size Rear"),   TEXT("ssr"),          _CLI_CSII_SSR,
		NULL, TEXT("Speaker Size Rear"),   CLI_GUEST},
	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};

CLI_EXEC_T _arSeCmdTbl[] = { /* pszCmdStr */
	{TEXT("reverb"),            TEXT("rb"),    _CLI_SeRbst,
		NULL, TEXT("reverb log"), CLI_GUEST},
	{TEXT("equlizer"),          TEXT("eq"),    _CLI_SeEqst,
		NULL, TEXT("equlizer log"), CLI_GUEST},
	{TEXT("prologic"),          TEXT("pl2"),   _CLI_SePl2st,
		NULL, TEXT("prologic log"), CLI_GUEST},
	{TEXT("bass manegement"),   TEXT("bassm"), _CLI_SeBassmst,
		NULL, TEXT("bass manegement log"), CLI_GUEST},
	{TEXT("spectrum"),          TEXT("spec"),  _CLI_SeSpecst,
		NULL, TEXT("spectrum log"), CLI_GUEST},
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};


CLI_EXEC_T _arAudDec4CmdTbl[] = {
	{TEXT("init"), TEXT("init"), _CLI_ADspDec4Init,
		NULL, TEXT("Create the DEC4 Instance"), CLI_GUEST},
	{TEXT("play"), TEXT("play"), _CLI_ADspDec4Play,
		NULL, TEXT("DEC4 Play"), CLI_GUEST},
	{TEXT("stop"), TEXT("stop"), _CLI_ADspDec4Stop,
		NULL, TEXT("DEC4 Stop"), CLI_GUEST},
	{TEXT("uninit"), TEXT("uninit"), _CLI_ADspDec4UnInit,
		NULL, TEXT("Destroy the DEC4 instance"), CLI_GUEST},
	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};

CLI_EXEC_T _arPCMCmdTbl[] = {
	{TEXT("Set Mic Mute"), TEXT("micmute"), _CLI_EnableMicMute,
		NULL, TEXT("Set Mic Mute"), CLI_GUEST},
	{TEXT("Set/Get Mic Gain"), TEXT("micgain"), _CLI_MicGain,
		NULL, TEXT("Set/Get Mic Gain"), CLI_GUEST},
	{TEXT("Get DTMF Param"), TEXT("dp"), _CLI_DTMFParam,
		NULL, TEXT("Get DTMF Param"), CLI_GUEST},
	{TEXT("Set DTMF Log Range"), TEXT("dr"), _CLI_SetDTMFLogRange,
		NULL, TEXT("Set DTMF Log Range"), CLI_GUEST},
	{TEXT("Test DTMF From File"), TEXT("dt"), _CLI_TestDTMF,
		NULL, TEXT("Test DTMF From File"), CLI_GUEST},
	{TEXT("Turn On PCM Log"), TEXT("log"), _CLI_TurnOnPCMLog,
		NULL, TEXT("Turn On PCM Log"), CLI_GUEST},

	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};


/******************************************************************************
* Variable      : cli aud table
******************************************************************************/
CLI_EXEC_T _arAudCmdTbl[] = {
	{TEXT("dsp"), TEXT("dsp"), NULL, _arAudDspCmdTbl,
		TEXT("dsp command"), CLI_GUEST},
	{TEXT("digtal out"), TEXT("digout"), NULL, _arAudDspDigOutCmdTbl,
		TEXT("dsp command"), CLI_GUEST},
	{TEXT("gpscmd"), TEXT("gpscmd"), NULL, _arAudGpsCmdTbl,
		TEXT("gpsmix command"), CLI_GUEST},
	{TEXT("TestTone"), TEXT("tt"), NULL, _arAudTTCmdTbl,
		TEXT("Test Tone command"), CLI_GUEST},

	{TEXT("SRS CSII"), TEXT("csii"), NULL, _arAudCSIICmdTbl,
		TEXT("Set SRS CSII command"), CLI_GUEST},
	{TEXT("dec4"), TEXT("dec4"), NULL, _arAudDec4CmdTbl,
		TEXT("dsp dec4 command"), CLI_GUEST},

	{TEXT("pcm"), TEXT("pcm"), NULL, _arPCMCmdTbl,
		TEXT("PCM command"), CLI_GUEST},

	/* yucai */
	{TEXT("log"), TEXT("log"), _CLI_AudTurnOnLog, NULL,
		TEXT("Turn Audio driver log"), CLI_GUEST},
	{TEXT("state"), TEXT("st"), _CLI_AudCheckState, NULL,
		TEXT("check the state"), CLI_GUEST},
	{TEXT("volume"), TEXT("v"), _CLI_AudChangeVol, NULL,
		TEXT("change the volume"), CLI_GUEST},
	{TEXT("underrun"), TEXT("ur"), _CLI_AudBitstreamUr, NULL,
		TEXT("Check afifo under run"), CLI_GUEST},
	{TEXT("afifo"), TEXT("afifo"), _CLI_AudDumpAfifo, NULL,
		TEXT("Dump Afifo"), CLI_GUEST},
	{TEXT("PrintAu"), TEXT("pa"), _CLI_AudPrintAu, NULL,
		TEXT("PrintAu[0/1]"), CLI_GUEST},
	{TEXT("bypass"), TEXT("bypass"), _CLI_AudBypassSE, NULL,
		TEXT("Audio SE Debug CLI"), CLI_GUEST},
	{TEXT("audiout"), TEXT("aout"), _CLI_AudAout, NULL,
		TEXT("Check the status of Audio out  CLI"), CLI_GUEST},
	{TEXT("checkcmd"), TEXT("check"), _CLI_CheckCmd, NULL,
		TEXT("Check the cmd send from the mw"), CLI_GUEST},
	{TEXT("extlin test"), TEXT("elin"), _CLI_EXTLINTEST, NULL,
		TEXT("ext linein test"), CLI_GUEST},
	{TEXT("PWMDAC External LDO"), TEXT("ext_ldo"), _CLI_ExtLdo, NULL,
		TEXT("PWMDAC Ext LDO"), CLI_GUEST},
	{TEXT("SPDIF Select"), TEXT("spdif"), _CLI_SPDIFSelect, NULL,
		TEXT("SPDIF Select"), CLI_GUEST},
	{TEXT("RearVolume"), TEXT("rvol"), _CLI_REARVOLUME, NULL,
		TEXT("rear volume control"), CLI_GUEST},
	{TEXT("UPMIX debug"), TEXT("upmix"), _CLI_UpMix, NULL,
		TEXT("UPMIX debug"), CLI_GUEST},
	{TEXT("Loudness debug"), TEXT("ld"), _CLI_LoudNess, NULL,
		TEXT("Loudness debug"), CLI_GUEST},
	{TEXT("DAC Type Select"), TEXT("dac_sel"), _CLI_DACTypeSel, NULL,
		TEXT("DAC Type Select"), CLI_GUEST},

	{TEXT("Front Master Volome Gain Ctrl"), TEXT("f_ml"),
		_CLI_Front_VolGain, NULL,
		TEXT("Front Master Volome Gain Ctrl"), CLI_GUEST},

	{TEXT("Rear Master Volome Gain Ctrl"), TEXT("r_ml"),
		_CLI_Rear_VolGain, NULL,
		TEXT("Rear Master Volome Gain Ctrl"), CLI_GUEST},

	{TEXT("Channel Volome Gain Ctrl"), TEXT("ch_vl"),
		_CLI_Ch_VolGain, NULL,
		TEXT("Channel Volome Gain Ctrl"), CLI_GUEST},
	{TEXT("PrintVolGain"), TEXT("p_vol"),
		_CLI_Print_VolGain, NULL,
		TEXT("PrintVolGain"), CLI_GUEST},

	{TEXT("PrintTTInfo"), TEXT("p_tt"), _CLI_Print_TTInfo,
		NULL, TEXT("PrintTTInfo"), CLI_GUEST},

	{TEXT("PrintUpmixinfo"), TEXT("p_upmix"), _CLI_Print_UpMixGain,
		NULL, TEXT("PrintUpmixinfo"), CLI_GUEST},

	{TEXT("PrintLoudinfo"), TEXT("p_loud"), _CLI_Print_LoudnessGain,
		NULL, TEXT("PrintLondinfo"), CLI_GUEST},
	{TEXT("Line in Bypass"), TEXT("rear_bypass"), _CLI_LinRearBypassSet,
		NULL, TEXT("Line in Bypass"), CLI_GUEST},
	{TEXT("LR Mix"), TEXT("lr_mix"), _CLI_LRMixSet, NULL,
		TEXT("LR Mix"), CLI_GUEST},
	{TEXT("Debug"), TEXT("dbg"), _CLI_AudioDebug, NULL,
		TEXT("Audio debug..."), CLI_GUEST},
	{TEXT("srv volume"), TEXT("srcvol"), _CLI_AudChangeSrcVol, NULL,
		TEXT("change src volume"), CLI_GUEST},

	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};

