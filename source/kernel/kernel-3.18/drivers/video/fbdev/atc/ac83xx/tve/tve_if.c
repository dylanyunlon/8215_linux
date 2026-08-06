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

/*****************************************************************************
*  TV Encoder: Interface
*****************************************************************************/

#ifndef _TVE_IF_C_
#define _TVE_IF_C_

#include "tve_hw.h"
#include "tve_hal.h"
#include "tve_if.h"
/*#include "display_ioctl.h"*/

#define NTSC_ASP_4_3_NORMAL   0x0
#define NTSC_ASP_16_9_NORMAL  0x1
#define NTSC_ASP_4_3_LB       0x2
#define NTSC_NO_CCI           (0xf << 2)
#define PAL_ASP_4_3_FULL      0x8
#define PAL_ASP_16_9_LB       0xb
#define PAL_ASP_16_9_FULL     0x7
#define VBI_ON                (0x1 << 20)/*CAV is same*/
#define VBI_LVL_MASK          ((unsigned)0xff << 24)/*CAV is same*/

static bool fgCheckXDS = FALSE;
static __u32 _dwCgms;
static __u32 _dwMVType;

__u32 dwMirror(__u32 dwVal, __u32 dwBitNum)
{
	__u32 i;
	__u32 dwOut = 0;
	__u32 dwMask = 0;
	__u32 dwShift;


	dwMask = 1;

	dwShift = 0;

	for (i = 0; i < dwBitNum; i++) {
		dwOut = (dwOut << 1);

		dwOut |= ((dwVal & dwMask) >> dwShift);

		dwShift++;
		dwMask = (dwMask << 1);
	}

	return dwOut;
}

__u32 dwCRCC(__u32 dwInput)
{
	__u32 dwOutput;
	__u32 dwFeedback;
	int i;

	dwOutput = 0x3f;  /* preset to all 1*/

	for (i = 0; i < 14; i++) {
		dwFeedback = (dwOutput & 0x1) ^ ((dwInput >> i) & 0x1); /* only bit 0 has value*/
		dwOutput >>= 1; /* bits 0-4 have values*/
		dwOutput ^= (dwFeedback << 4);  /* bit 4 XOR with feedback*/
		dwOutput |= (dwFeedback << 5);  /* bit 5 set to feedback*/
	}
	dwOutput = dwOutput << 14;

	return dwOutput;
}

void vSetNtscVbiSignal(__u32 dwOption, __u32 dwCgms, __u32 dwMVType)
{
	__u32 dwVbi;
	__u32 dwData;
	__u32 dwMask = 0xfffc3c;   /* defaultly mask CRCC, bit 14,13,12, 11,6, 5, 4, 3*/

	fgCheckXDS = TRUE;
	_dwCgms = dwCgms;
	_dwMVType = dwMVType;

	/* set Mask here*/
	if (dwOption & CCI_ASPECT) {
		dwMask = dwMask | 0x3;
	}

	if (dwOption & CCI_CGMS) {
		dwMask = dwMask | 0xC0;
	}

	if (dwOption & CCI_APS) {
		dwMask = dwMask | 0x300;
	}

	dwMask = ~dwMask;

	dwData = ((dwMirror(dwTveGet525VbiData(), 20) & 0xFFFFF) | dwTveGetVbiCtrl());

	dwVbi = (dwData & dwMask) | VBI_ON;
#if 0

	if (dwOption & CCI_ASPECT) {
		switch (_tTv.bAspectRatio) {
		case TV_WIDE:
			dwVbi |= NTSC_ASP_16_9_NORMAL;
			break;

		case TV_LB:
			dwVbi |= NTSC_ASP_4_3_LB;
			break;

		case TV_4_3:
			if ((bSharedInfo(SI_SRC_ASPECT_RATIO) & 0x1) == 0x1) {
				dwVbi |= NTSC_ASP_4_3_LB;
			} else {
				dwVbi |= NTSC_ASP_4_3_NORMAL;
			}

		case TV_PS:
		default:
			dwVbi |= NTSC_ASP_4_3_NORMAL;
			break;
		}
	}

#endif

	if (dwOption & CCI_APS) {
		dwVbi = dwVbi | ((dwMVType & 0x1) << 9);
		dwVbi = dwVbi | ((dwMVType & 0x2) << 7);
	}

	if (dwOption & CCI_CGMS) {
		dwVbi = dwVbi | ((dwCgms & 0x1) << 7);
		dwVbi = dwVbi | ((dwCgms & 0x2) << 5);
	}

	if (!(dwOption & (CCI_APS | CCI_CGMS))) {
		dwVbi |= NTSC_NO_CCI;
	}

	if (dwOption & (CCI_APS | CCI_CGMS | CCI_ASPECT)) {
		dwVbi |= dwCRCC(dwVbi & 0x3fff);   /* calculate the CRCC*/
		dwVbi = ((dwVbi & 0xfff00000) | ((dwMirror((dwVbi & 0xfffff), 20)) & 0xfffff));
		dwVbi = (dwVbi | 0xC8000000);

		vTveHalSetVbi(dwVbi);
	}
}


void vSetPalVbiSignal(__u32 dwOption, __u32 dwCgms, __u32 dwMVType)
{
	__u32 dwVbi;
	__u32 dwData;
	__u32 dwMask = 0xffcff0;

	fgCheckXDS = TRUE;
	_dwCgms = dwCgms;
	_dwMVType = dwMVType;

	if (dwOption & CCI_ASPECT) {
		dwMask = dwMask | 0xf;
	}

	if (dwOption & CCI_CGMS) {
		dwMask = dwMask | 0x3000;
	}

	dwMask = ~dwMask;

	dwData = ((dwMirror(dwTveGet625VbiData(), 14) & 0x3fff) | dwTveGetVbiCtrl());

	dwVbi = (dwData & dwMask) | VBI_ON;
#if 0

	if (dwOption & CCI_ASPECT) {
		switch (_tTv.bAspectRatio) {
		case TV_WIDE:
			dwVbi |= PAL_ASP_16_9_FULL;
			break;

		case TV_LB:
			dwVbi |= PAL_ASP_16_9_LB;
			break;

		case TV_4_3:

			/* 031105: port from Pioneer*/
			if ((bSharedInfo(SI_SRC_ASPECT_RATIO) & 0x1) == 0x1) {
				dwVbi |= 0x40;    /* helper bit mark as 1*/
			}

			/* 031117: Pioneer bug #541:*/
			/* In PAL, "Source Picture Letterboxed" affects the helper bit, not WSS;*/
			/* while in NTSC, "Source Picture Letterboxed" affects WSS.*/
			dwVbi |= PAL_ASP_4_3_FULL;
			break;

		case TV_PS:
		default:
			dwVbi |= PAL_ASP_4_3_FULL;
			break;
		}
	}

#endif

	if (dwOption & CCI_CGMS) {
		dwVbi = dwVbi | ((dwCgms & 0x1) << 13);
		dwVbi = dwVbi | ((dwCgms & 0x2) << 11);
	}

	if (dwOption & (CCI_CGMS | CCI_ASPECT)) {
		dwVbi = ((dwVbi & 0xfff00000) | ((dwMirror((dwVbi & 0x3fff), 14)) & 0x3fff));
		dwVbi = (dwVbi | 0xC8000000);

		vTveHalSetVbi(dwVbi);
	}
}

#define CCXDS_CURRENT_START_CODE      0x01
#define CCXDS_CURRENT_CONTINUE_CODE   0x02
#define CCXDS_CURRENT_CGMS_PACKET     0x08
#define CCXDS_END_CODE                0x0F
#define CCXDS_CGMS_STATE_NONE         0x00
#define CCXDS_CGMS_STATE_START        0x01
#define CCXDS_CGMS_STATE_CHECK_DUMMY  0x02
#define CCXDS_CGMS_STATE_SEND_DATA    0x03
#define CCXDS_CGMS_STATE_CONTINUE     0x04
#define CCXDS_CGMS_STATE_END          0x05
#define CCXDS_TVE_DATA_MASK           0x0000FFFF
#define CCXDS_SERVICE_TIME            100 /*100x16msec = 1.6sec*/

#define TOP_FIELD         0
#define BOTTOM_FIELD      1

#define TOP_CC               0x1 /*(0x1 << 16)*/
#define BTM_CC               0x2 /*(0x1 << 17)  */

static __u32 _dwCCXDSServiceCount;
static __u8 _bCCXDSServiceState;
static __u8 _bCCXDSCGMSStep;
static __u8 _bCCXDSCGMSContinue;
static __u8 _bCCXDSCGMSCheckSum;

bool fgParityChk(__u8 bHi, __u8 bLo)
{
	__u8 bOnes[2] = {0, 0};
	int i;

	for (i = 0; i < 8; i++) {
		if (bHi & (1 << i)) {
			bOnes[0]++;
		}

		if (bLo & (1 << i)) {
			bOnes[1]++;
		}
	}

	if (!(bOnes[0] & 0x1) || !(bOnes[1] & 0x1)) {
		return FALSE;
	}

	return TRUE;
}

void vCCSendDummy(void)
{
	/* send dummy signal*/
	vTveHalSetCc(0x80, 0x80, TOP_CC | BTM_CC);
}

void vCCXDSCGMSService(void)
{
	__u8 bHi, bLo;

	_dwCCXDSServiceCount++;

	if (!fgCheckXDS) {
		return;
	}

	if ((_dwCgms == 0) && (_dwMVType == 0)) {
		fgCheckXDS = FALSE;
	}

	if (fgTveGetTvField() == TOP_FIELD) {
		vTveHalSetCc(0x80, 0x80, TOP_CC);
		return;
	}

	if (_dwCCXDSServiceCount > CCXDS_SERVICE_TIME) {
		if (_bCCXDSCGMSStep == 0) {
			_bCCXDSServiceState = CCXDS_CGMS_STATE_START;
			/*_bCCXDSCGMSStep = 0;*/
			_bCCXDSCGMSContinue = 0;
			_dwCCXDSServiceCount = 0;
		}
	}

	switch (_bCCXDSServiceState) {
	case CCXDS_CGMS_STATE_NONE:
		vCCSendDummy();
		break;

	case CCXDS_CGMS_STATE_START:
		bHi = CCXDS_CURRENT_START_CODE;
		bLo = CCXDS_CURRENT_CGMS_PACKET;
		vTveHalSetCc(bHi, bLo, BTM_CC);
		_bCCXDSServiceState = CCXDS_CGMS_STATE_CHECK_DUMMY;
		_bCCXDSCGMSStep = 1;
		break;

	case CCXDS_CGMS_STATE_CHECK_DUMMY:
		if (fgTveCheckCCDummy()) {
			if (_bCCXDSCGMSContinue == 0) {
				if (_bCCXDSCGMSStep == 1) {
					_bCCXDSServiceState = CCXDS_CGMS_STATE_SEND_DATA; /*don't break;*/
				} else if (_bCCXDSCGMSStep == 2) {
					_bCCXDSServiceState = CCXDS_CGMS_STATE_END; /*don't break;*/
				}
			} else {
				_bCCXDSServiceState = CCXDS_CGMS_STATE_CONTINUE; /*don't break;*/
			}
		} else {
			vCCSendDummy();
			_bCCXDSCGMSContinue = 1;
			break;
		}

	/*no break here;*/
	case CCXDS_CGMS_STATE_SEND_DATA:
		if (_bCCXDSCGMSStep == 1) {
			bLo = 0x40; /*reserved.*/
			bHi = 0x40 | (__u8)((_dwCgms & 0x3) << 3) | (__u8)((_dwMVType & 0x3) << 1); /*bit6 reserved*/
			_bCCXDSCGMSCheckSum = 0x80 - ((CCXDS_CURRENT_START_CODE + CCXDS_CURRENT_CGMS_PACKET +
						       bLo + bHi + CCXDS_END_CODE) & 0x7F);

			if (!fgParityChk(bHi, bHi)) {
				bHi |= 0x80; /*add parity bit.*/
			}

			vTveHalSetCc(bHi, bLo, BTM_CC);
			_bCCXDSCGMSStep = 2;
			_bCCXDSCGMSContinue = 0;
			_bCCXDSServiceState = CCXDS_CGMS_STATE_CHECK_DUMMY;
			break;
		}

	case CCXDS_CGMS_STATE_CONTINUE:
		if (_bCCXDSCGMSContinue == 1) {
			bHi = CCXDS_CURRENT_CONTINUE_CODE;
			bLo = CCXDS_CURRENT_CGMS_PACKET;
			vTveHalSetCc(bHi, bLo, BTM_CC);
			_bCCXDSCGMSContinue = 0;
			_bCCXDSServiceState = CCXDS_CGMS_STATE_CHECK_DUMMY;
			break;
		}

	case CCXDS_CGMS_STATE_END:
		if (_bCCXDSCGMSStep == 2) {
			bLo = _bCCXDSCGMSCheckSum;
			bHi = CCXDS_END_CODE | 0x80; /*end code + parity bit*/

			if (!fgParityChk(bLo, bLo)) {
				bLo |= 0x80; /*add parity bit.*/
			}

			vTveHalSetCc(bHi, bLo, BTM_CC);
			_dwCCXDSServiceCount = 0;
			_bCCXDSServiceState = CCXDS_CGMS_STATE_NONE;
			_bCCXDSCGMSStep = 0;
			break;
		}
	}
}

#endif


