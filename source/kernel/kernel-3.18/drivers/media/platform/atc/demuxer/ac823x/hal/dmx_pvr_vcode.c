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

/*!
 * @file dmx_pvr_vcode.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Demuxer pvr video related interfaces definitions
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#endif /* __linux__*/
#include "x_os.h"
#include "x_hal_ic.h"
#include "x_hal_1176.h"
#include "x_assert.h"
#include "drv_config.h"
#include "dmx_def.h"
#include "dmx_pvr.h"

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/
/* store video type per PID*/
typedef struct {
	bool fgEnable;
	PVR_VIDEO_TYPE_T eType;			/*< Type of video*/
} PVR_VIDEO_TYPE_PER_PID_T;

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

EXTERN PVR_VIDEO_TYPE_T g_eDmxHwVideoType[DMX_DEV_CNT];
/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

/* VC1 Start Code*/
const PVR_STARTCODE_T _arVC1StartCode_NoIntr[] = {
	{ 0x00000000, 0x010C0000, 0x0000FFFF, 0xFFFF0000 },  /* Field Start Code*/
	{ 0x00000000, 0x010D0000, 0x0000FFFF, 0xFFFF0000 },  /* Frame Start Code*/
	{ 0x00000000, 0x0000010A, 0x00000000, 0xFFFFFFFF },  /* Sequence End Start Code*/
	{ 0x00000000, 0x010B0000, 0x0000FFFF, 0xFFFF0000 }	 /* Slice Start Code*/
};

const PVR_STARTCODE_T _arVC1StartCode_Intr[] = {
	{ 0x00000000, 0x010E0000, 0x0000FFFF, 0xFFFF0000 },  /* Entry Point Start Code*/
	{ 0x00000000, 0x010F0000, 0x0000FFFF, 0xFFFF0000 },  /* Sequence Start Code*/
	{ 0x00000000, 0x01180000, 0x0000FFFF, 0xFFFF0000 }	 /* SMPTE Reserved*/
};
static s8 au1DmxVOffsetVC1[10] = {
	-5, -5, -3, -5, PVR_VNULL,
	-5, -5, -5, PVR_VNULL, PVR_VNULL };

/* MPEG2 Start Code*/
const PVR_STARTCODE_T _arMPEG2StartCode_NoIntr[] = {
	{ 0x00000000, 0x01000000, 0x0000FFFF, 0xFFFF0000 }, /* Picture*/
	{ 0x00000000, 0x01B30000, 0x0000FFFF, 0xFFFF0000 }, /* SEQ START*/
	{ 0x00000000, 0x000001B7, 0x00000000, 0xFFFFFFFF }, /* SEQ END*/
	{ 0x00000000, 0x01B80000, 0x0000FFFF, 0xFFFF0000 }	/* GOP*/
};

static s8 au1DmxVOffsetMPEG2[10] = {
	-5, -5, -3, -5, PVR_VNULL,
	PVR_VNULL, PVR_VNULL, PVR_VNULL, PVR_VNULL, PVR_VNULL};

/* MP4 Start Code*/
const PVR_STARTCODE_T _arMPEG4StartCode_NoIntr[] = {
	{ 0x00000000, 0x01B60000, 0x0000FFFF, 0xFFFF0000 }, /* VOP Start Code*/
	{ 0x00000000, 0x01B00000, 0x0000FFFF, 0xFFFF0000 }, /* Sequence Start Code*/
	{ 0x00000000, 0x01B30000, 0x0000FFFF, 0xFFFF0000 }, /* Goup of VOP Start Code*/
	{ 0x00000000, 0x01B50000, 0x0000FFFF, 0xFFFF0000 }	/* Visual Object Start Code*/
};

const PVR_STARTCODE_T _arMPEG4StartCode_Intr[] = {
	{ 0x00000000, 0x01200000, 0x0000FFFF, 0xFFF00000 }	/* Video Object Layer Start Code*/
};

static s8 au1DmxVOffsetMPEG4[10] = {
	-5, -5, -5, -5, PVR_VNULL,
	-5, PVR_VNULL, PVR_VNULL, PVR_VNULL, PVR_VNULL};

/* H263*/
const PVR_STARTCODE_T _arH263StartCode_NoIntr[] = {
	{ 0x00000000, 0x80000000, 0x0000FFFF, 0xFC000200 },  /* H263 I Frame*/
	{ 0x00000000, 0x80000200, 0x0000FFFF, 0xFC000200 }	 /* H263 P Frame*/
};

static s8 au1DmxVOffsetH263[10] = {
	-5, -5, PVR_VNULL, PVR_VNULL, PVR_VNULL,
	PVR_VNULL, PVR_VNULL, PVR_VNULL, PVR_VNULL, PVR_VNULL};

/* H264*/
const PVR_STARTCODE_T _arH264StartCode_NoIntr[] = {
	{ 0x00000000, 0x01060000, 0x0000FFFF, 0xFF1F0000 },  /* H264_SEI*/
	{ 0x00000000, 0x01070000, 0x0000FFFF, 0xFF1F0000 },  /* H264_SPS*/
	{ 0x00000000, 0x0000010A, 0x00000000, 0xFFFFFF1F },  /* H264_END_SEQ*/
	{ 0x00000000, 0x01080000, 0x0000FFFF, 0xFF1F0000 }	 /* H264_PPS*/
};

const PVR_STARTCODE_T _arH264StartCode_Intr[] = {
	{ 0x00000000, 0x01090000, 0x0000FFFF, 0xFF1F0000 },  /* H264_AUD*/
	{ 0x00000000, 0x01010000, 0x0000FFFF, 0xFF1F0000 },  /* H264_NON_IDR*/
	{ 0x00000000, 0x01050000, 0x0000FFFF, 0xFF1F0000 },  /* H264_IDR*/
	{ 0x00000000, 0x0000010C, 0x00000000, 0xFFFFFF1F },  /* H264_FILTERDATA*/
	{ 0x00000000, 0x0000010B, 0x00000000, 0xFFFFFF1F }	 /* H264_STMEND*/
};

static s8 au1DmxVOffsetH264[10] = {
	-5, -5, -3, -5, PVR_VNULL,
	-5, -5, -5, -3, -3};

/* H265*/
const PVR_STARTCODE_T _arH265StartCode_NoIntr[] = {
	{ 0x00000000, 0x00000140, 0x00000000, 0xFFFFFFFE },  /* H265_EOB*/
	{ 0x00000000, 0x00000142, 0x00000000, 0xFFFFFFFE },  /* H265_SPS*/
	{ 0x00000000, 0x00000144, 0x00000000, 0xFFFFFFFE },  /* H265_PPS*/
	{ 0x00000000, 0x00000146, 0x00000000, 0xFFFFFFFE },  /* H265_AUD*/
	{ 0x00000000, 0x00000148, 0x00000000, 0xFFFFFFFE }	 /* H265_EOS*/
};

const PVR_STARTCODE_T _arH265StartCode_Intr[] = {
	{ 0x00000000, 0x0000014A, 0x00000000, 0xFFFFFFFE },  /* H265_EOB*/
	{ 0x00000000, 0x0000014E, 0x00000000, 0xFFFFFFFE },  /* H265_SEI*/
	{ 0x00000000, 0x00000150, 0x00000000, 0xFFFFFFF0 },  /* H265_RSV_NVCL41~47 and SUFFIX_SEI*/
	{ 0x00000000, 0x00000160, 0x00000000, 0xFFFFFFF0 },  /* H265_UNSPEC48~55*/
	{ 0x00000000, 0x01000000, 0x0000FFFF, 0xFFC10000 }	 /* H265_VCL*/
};

static s8 au1DmxVOffsetH265[10] = {
	-3, -3, -3, -3, -3,
	-3, -3, -3, -3, -5};

/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** _PVR_SetStartCodePattern_Ex
 *	Set search start code pattern
 *
 *	@retval bool
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_SetStartCodePattern_Ex(u8 u1DevID, u8 u1NoIntrCount,
							const PVR_STARTCODE_T *prNoIntrStartCode,
							u8 u1IntrCount, const PVR_STARTCODE_T *prIntrStartCode)
{
	u16 u2Control;
	u8 i;

	if ((u1NoIntrCount > PVR_STARTCODE_NONINTR_NUM) ||
		(u1IntrCount > PVR_STARTCODE_INTR_NUM))
		return FALSE;

	u2Control = 0x0;

	_PVR_Lock();
	
	if (DDI_PVR_DMA_PATH_ID == u1DevID) {
		if ((u1NoIntrCount > 0) && (prNoIntrStartCode != NULL)) {
			for (i = 0; i < u1NoIntrCount; i++) {
				PVR_VCODE_S_W(i, 0) = prNoIntrStartCode[i].u4Pattern0;
				mb();
				PVR_VCODE_S_W(i, 1) = prNoIntrStartCode[i].u4Pattern1;
				mb();
				PVR_VCODE_S_W(i, 2) = prNoIntrStartCode[i].u4Mask0;
				mb();
				PVR_VCODE_S_W(i, 3) = prNoIntrStartCode[i].u4Mask1;
				mb();
				smp_mb();
				u2Control |= (1 << i);
			}
		}

		smp_mb();

		if ((u1IntrCount > 0) && (prIntrStartCode != NULL)) {
			for (i = 0; i < u1IntrCount; i++) {
				PVR_VCODE_S_W(i + PVR_STARTCODE_NONINTR_NUM, 0) = prIntrStartCode[i].u4Pattern0;
				mb();
				PVR_VCODE_S_W(i + PVR_STARTCODE_NONINTR_NUM, 1) = prIntrStartCode[i].u4Pattern1;
				mb();
				PVR_VCODE_S_W(i + PVR_STARTCODE_NONINTR_NUM, 2) = prIntrStartCode[i].u4Mask0;
				mb();
				PVR_VCODE_S_W(i + PVR_STARTCODE_NONINTR_NUM, 3) = prIntrStartCode[i].u4Mask1;
				u2Control |= (1 << (i + PVR_STARTCODE_NONINTR_NUM));
			}
		}

		smp_mb();

		/* Write 0x1 to the "pattern changed" byte.*/
		PVR_VCODE_W(1) = ((u32)u2Control << 16) | 0xFFFF;
		mb();
	} else if (MINI_PVR_DMA_PATH_ID == u1DevID) {
		if ((u1NoIntrCount > 0) && (prNoIntrStartCode != NULL)) {
	        for (i = 0; i < u1NoIntrCount; i++) {
	            MINI_PVR_VCODE_S_W(i, 0) = prNoIntrStartCode[i].u4Pattern0;
	            mb();
	            MINI_PVR_VCODE_S_W(i, 1) = prNoIntrStartCode[i].u4Pattern1;
	            mb();
	            MINI_PVR_VCODE_S_W(i, 2) = prNoIntrStartCode[i].u4Mask0;
	            mb();
	            MINI_PVR_VCODE_S_W(i, 3) = prNoIntrStartCode[i].u4Mask1;
	            mb();
	            smp_mb();
	            u2Control |= (1 << i);
	        }
	    }

	    smp_mb();

	    if ((u1IntrCount > 0) && (prIntrStartCode != NULL)) {
	        for (i = 0; i < u1IntrCount; i++) {
	            MINI_PVR_VCODE_S_W(i + PVR_STARTCODE_NONINTR_NUM, 0) = prIntrStartCode[i].u4Pattern0;
	            mb();
	            MINI_PVR_VCODE_S_W(i + PVR_STARTCODE_NONINTR_NUM, 1) = prIntrStartCode[i].u4Pattern1;
	            mb();
	            MINI_PVR_VCODE_S_W(i + PVR_STARTCODE_NONINTR_NUM, 2) = prIntrStartCode[i].u4Mask0;
	            mb();
	            MINI_PVR_VCODE_S_W(i + PVR_STARTCODE_NONINTR_NUM, 3) = prIntrStartCode[i].u4Mask1;
	            u2Control |= (1 << (i + PVR_STARTCODE_NONINTR_NUM));
	        }
	    }

	    smp_mb();

	    // Write 0x1 to the "pattern changed" byte.
	    MINI_PVR_VCODE_W(1) = ((UINT32)u2Control << 16) | 0xFFFF;
	    mb();
	} else {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- Fail for u1DevID is invalid.\r\n"),
        	DMX_FUNC_NAME, DMX_LINE_NO);
		_PVR_Unlock();
		return FALSE;
	}

	_PVR_Unlock();

	return TRUE;
}

void _PVR_ResetStartCodePattern(u8 u1DevID)
{
	u32 u4Reg;
	u16 u2Control;
	u8  i;

	u2Control = 0x0;

	_PVR_Lock();

	smp_mb();

	if (DDI_PVR_DMA_PATH_ID == u1DevID) {
		for (i = 0; i < PVR_STARTCODE_NONINTR_NUM; i++) {
			PVR_VCODE_S_W(i, 0) = 0;
			mb();
			PVR_VCODE_S_W(i, 1) = 0;
			mb();
			PVR_VCODE_S_W(i, 2) = 0;
			mb();
			PVR_VCODE_S_W(i, 3) = 0;
			mb();
			u2Control |= (1 << i);
		}

		smp_mb();
		for (i = 0; i < PVR_STARTCODE_INTR_NUM; i++) {
			PVR_VCODE_S_W(i, 0) = 0;
			mb();
			PVR_VCODE_S_W(i, 1) = 0;
			mb();
			PVR_VCODE_S_W(i, 2) = 0;
			mb();
			PVR_VCODE_S_W(i, 3) = 0;
			mb();
			u2Control |= (1 << (i + PVR_STARTCODE_NONINTR_NUM));
		}

		smp_mb();
		/* Write 0x1 to the "pattern changed" byte.*/
		PVR_VCODE_W(1) = ((u32)u2Control << 16) | 0xFFFF;

		u4Reg = PVR_VCODE_W(0);
		smp_mb();
		u4Reg |= (0xFF << (DMX_DDI_MM_MOVE_TSIDX * 8));
		smp_mb();
		PVR_VCODE_W(0) = u4Reg;
		mb();

		u4Reg = PID_S_W(PVR_PID_IDX_VIDEO, 0);
		smp_mb();
		u4Reg |= (0xFF << 16);
		u4Reg &= ~(1<<7);
		smp_mb();
		PID_S_W(PVR_PID_IDX_VIDEO, 0) = u4Reg;
		mb();
	} else if (MINI_PVR_DMA_PATH_ID == u1DevID) {
		for (i = 0; i < PVR_STARTCODE_NONINTR_NUM; i++) {
	        MINI_PVR_VCODE_S_W(i, 0) = 0;
	        mb();
	        MINI_PVR_VCODE_S_W(i, 1) = 0;
	        mb();
	        MINI_PVR_VCODE_S_W(i, 2) = 0;
	        mb();
	        MINI_PVR_VCODE_S_W(i, 3) = 0;
	        mb();
	        u2Control |= (1 << i);
	    }

	    smp_mb();
	    for (i = 0; i < PVR_STARTCODE_INTR_NUM; i++) {
	        MINI_PVR_VCODE_S_W(i, 0) = 0;
	        mb();
	        MINI_PVR_VCODE_S_W(i, 1) = 0;
	        mb();
	        MINI_PVR_VCODE_S_W(i, 2) = 0;
	        mb();
	        MINI_PVR_VCODE_S_W(i, 3) = 0;
	        mb();
	        u2Control |= (1 << (i + PVR_STARTCODE_NONINTR_NUM));
	    }

	    smp_mb();
	    // Write 0x1 to the "pattern changed" byte.
	    MINI_PVR_VCODE_W(1) = ((UINT32)u2Control << 16) | 0xFFFF;

	    u4Reg = MINI_PVR_VCODE_W(0);
	    smp_mb();
	    u4Reg |= (0xFF << (DMX_FVR_MM_MOVE_TSIDX * 8));
	    smp_mb();
	    MINI_PVR_VCODE_W(0) = u4Reg;
	    mb();

	    u4Reg = MINI_PVR_VCODE_S_W(MINI_PVR_PID_IDX_VIDEO, 0);
	    smp_mb();
	    u4Reg |= (0xFF << 16);
	    u4Reg &= ~(1<<7);
	    smp_mb();
	    MINI_PVR_VCODE_S_W(MINI_PVR_PID_IDX_VIDEO, 0) = u4Reg;
	    mb();
	} else {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- Fail for u1DevID(0x%02x) is invalid.\r\n"),
        DMX_FUNC_NAME, DMX_LINE_NO, u1DevID);
	}

	g_eDmxHwVideoType[u1DevID] = PVR_VIDEO_UNKNOWN;

	_PVR_Unlock();

	PVR_LOG_DBG(TEXT("[PVR] %s line %d -- ResetStartCode\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
}

/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** _PVR_SetVideoType
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_SetVideoType(u8 u1DevID, PVR_VIDEO_TYPE_T eVideoType)
{
	u32 u4Reg, u4Mask;

	PVR_LOG_TRACE(TEXT("------ [PVR] %s --> u1DevID: 0x%02x, eVideoType: %d ------\r\n"),
		DMX_FUNC_NAME, u1DevID, eVideoType);

	switch (eVideoType) {
	case PVR_VIDEO_UNKNOWN:
		_PVR_ResetStartCodePattern(u1DevID);
		return TRUE;

	case PVR_VIDEO_MPEG:
		if (!_PVR_SetStartCodePattern_Ex(u1DevID, 
			(u8)(sizeof(_arMPEG2StartCode_NoIntr)/sizeof(PVR_STARTCODE_T)), _arMPEG2StartCode_NoIntr,
			0, NULL)) {
			return FALSE;
		}
		break;

	case PVR_VIDEO_H263:
		if (!_PVR_SetStartCodePattern_Ex(u1DevID, 
			(u8)(sizeof(_arH263StartCode_NoIntr)/sizeof(PVR_STARTCODE_T)), _arH263StartCode_NoIntr,
			0, NULL)) {
			return FALSE;
		}
		break;

	case PVR_VIDEO_H264:
		if (!_PVR_SetStartCodePattern_Ex(u1DevID, 
			(u8)(sizeof(_arH264StartCode_NoIntr)/sizeof(PVR_STARTCODE_T)), _arH264StartCode_NoIntr,
			(u8)(sizeof(_arH264StartCode_Intr)/sizeof(PVR_STARTCODE_T)), _arH264StartCode_Intr)) {
			return FALSE;
		}
		break;

	case PVR_VIDEO_VC1:
		if (!_PVR_SetStartCodePattern_Ex(u1DevID, 
			(u8)(sizeof(_arVC1StartCode_NoIntr)/sizeof(PVR_STARTCODE_T)), _arVC1StartCode_NoIntr,
			(u8)(sizeof(_arVC1StartCode_Intr)/sizeof(PVR_STARTCODE_T)), _arVC1StartCode_Intr)) {
			return FALSE;
		}
		break;

	case PVR_VIDEO_WMV7:	/* No need to search for any start code*/
	case PVR_VIDEO_WMV8:	/* No need to search for any start code*/
	case PVR_VIDEO_WMV9:	/* No need to search for any start code*/
	case PVR_VIDEO_MP4_IN_WMV: /* No need to search for any start code*/
	case PVR_VIDEO_RV: /* No need to search for any start code*/
		/* Disable Header Detect, PID Data Structure and  Pattern Mask*/
		break;

	case PVR_VIDEO_MPEG4:
		if (!_PVR_SetStartCodePattern_Ex(u1DevID, 
			(u8)(sizeof(_arMPEG4StartCode_NoIntr)/sizeof(PVR_STARTCODE_T)), _arMPEG4StartCode_NoIntr,
			(u8)(sizeof(_arMPEG4StartCode_Intr)/sizeof(PVR_STARTCODE_T)), _arMPEG4StartCode_Intr)) {
			return FALSE;
		}
		break;

	case PVR_VIDEO_H265:
		if (!_PVR_SetStartCodePattern_Ex(u1DevID, 
			(u8)(sizeof(_arH265StartCode_NoIntr)/sizeof(PVR_STARTCODE_T)), _arH265StartCode_NoIntr,
			(u8)(sizeof(_arH265StartCode_Intr)/sizeof(PVR_STARTCODE_T)), _arH265StartCode_Intr)) {
			return FALSE;
		}
		break;

	default:
		PVR_LOG_ERR(TEXT("[PVR] %s -- Unknown video type: %d.\r\n"),
			DMX_FUNC_NAME, eVideoType);
		return FALSE;
	}

	mb();

	smp_mb();

	_PVR_Lock();
	g_eDmxHwVideoType[u1DevID] = eVideoType;
	smp_mb();

	if (DDI_PVR_DMA_PATH_ID == u1DevID) {
		if (eVideoType != PVR_VIDEO_UNKNOWN) {
			u4Reg = PVR_VCODE_W(0);
			mb();
			u4Mask = (u32)0xFF << (DMX_DDI_MM_MOVE_TSIDX * 8);
			smp_mb();
			u4Reg &= ~u4Mask;
			smp_mb();
			u4Reg |= (3 << (DMX_DDI_MM_MOVE_TSIDX * 8)); /* User-defined Video Type*/
			smp_mb();
			PVR_VCODE_W(0) = u4Reg;
			mb();
		} else {
			u4Reg = PVR_VCODE_W(0);
			smp_mb();
			u4Reg |= (0xFF << (DMX_DDI_MM_MOVE_TSIDX * 8));
			smp_mb();
			PVR_VCODE_W(0) = u4Reg;
			mb();
		}

		u4Reg = PID_S_W(PVR_PID_IDX_VIDEO, 0);
		u4Reg |= (0x0FF << 16);
		smp_mb();
		u4Reg |= (1 << 7);
		smp_mb();
		PID_S_W(PVR_PID_IDX_VIDEO, 0) = u4Reg;
		mb();
	} else if (MINI_PVR_DMA_PATH_ID == u1DevID) {
		if (eVideoType != PVR_VIDEO_UNKNOWN) {
	        u4Reg = MINI_PVR_VCODE_W(0);
	        mb();
	        u4Mask = (UINT32)0xFF << (DMX_FVR_MM_MOVE_TSIDX * 8);
	        smp_mb();
	        u4Reg &= ~u4Mask;
	        smp_mb();
	        u4Reg |= (3 << (DMX_FVR_MM_MOVE_TSIDX * 8)); // User-defined Video Type
	        smp_mb();
	        MINI_PVR_VCODE_W(0) = u4Reg;
	        mb();
	    } else {
	        u4Reg = MINI_PVR_VCODE_W(0);
	        smp_mb();
	        u4Reg |= (0xFF << (DMX_FVR_MM_MOVE_TSIDX * 8));
	        smp_mb();
	        MINI_PVR_VCODE_W(0) = u4Reg;
	        mb();
	    }

	    u4Reg = PID_S_W(MINI_PVR_PID_IDX_VIDEO, 0);
	    u4Reg |= (0x0FF << 16);
	    smp_mb();
	    u4Reg |= (1 << 7);
	    smp_mb();
	    PID_S_W(MINI_PVR_PID_IDX_VIDEO, 0) = u4Reg;
	    mb();
	} else {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d, Fail for invalid u1DevID.\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
		_PVR_Unlock();
		return FALSE;
	}

	_PVR_Unlock();

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_GetVideoType
 */
/*-----------------------------------------------------------------------------*/
PVR_VIDEO_TYPE_T _PVR_GetVideoType(u8 u1DevID)
{
	PVR_VIDEO_TYPE_T eVideoType;

	_PVR_Lock();
	eVideoType = g_eDmxHwVideoType[u1DevID];
	_PVR_Unlock();

	return eVideoType;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_VCodeInit
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_VCodeInit(void)
{
	u32 u4Ctrl;

	/* Clear Pattern match DMEM0 and DMEM1*/
	memset_io((void *)PVR_PATTERN_MATCH_BASE0, 0, PVR_DMEM_PATTERN_MATCH_LEN * 4);
	mb();
	memset_io((void *)PVR_PATTERN_MATCH_BASE1, 0, PVR_DMEM_PATTERN_MATCH_LEN * 4);
	mb();

	smp_mb();

	if (!_PVR_SetVideoType(DDI_PVR_DMA_PATH_ID, PVR_VIDEO_UNKNOWN) || 
		!_PVR_SetVideoType(MINI_PVR_DMA_PATH_ID, PVR_VIDEO_UNKNOWN)) {
		PVR_LOG_ERR(TEXT("%s line %d fail in _PVR_SetVideoType(PVR_VIDEO_UNKNOWN)\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	/*------------------------------*/
	/*
	   start code select,  0: select start code 0, 1: select start code 1
	   bit0 --- ts index 0
	   bit1 --- ts index 1
	   bit2 --- ts index 2
	   bit3 --- ts index 3

	   Ucode Implement two start code pattern and mask for two mm play back.
	   however until now, we just use one of them. so chose start code 0
	   */
	/*------------------------------*/
	u4Ctrl = SECTIONFILTER_SETTING & (~0xFF0000);
	u4Ctrl |= (0x040000); //set TS3 chose start code 1
	smp_mb();
	SECTIONFILTER_SETTING = u4Ctrl;
	mb();

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_GetVCode_Offset
 */
/*-----------------------------------------------------------------------------*/

s8 _PVR_GetVCode_Offset(PVR_VIDEO_TYPE_T eVideoType, u8 u1Idx)
{
	DMX_ASSERT(u1Idx < 10);
	if (u1Idx >= 10)
		return PVR_VNULL;

	switch (eVideoType) {
	case PVR_VIDEO_UNKNOWN:
		return TRUE;

	case PVR_VIDEO_MPEG:
		return au1DmxVOffsetMPEG2[u1Idx];

	case PVR_VIDEO_H264:
		return au1DmxVOffsetH264[u1Idx];

	case PVR_VIDEO_VC1:
		return au1DmxVOffsetVC1[u1Idx];

	case PVR_VIDEO_WMV7:	/* No need to search for any start code*/
	case PVR_VIDEO_WMV8:	/* No need to search for any start code*/
	case PVR_VIDEO_WMV9:	/* No need to search for any start code*/
	case PVR_VIDEO_MP4_IN_WMV: /* No need to search for any start code*/
	case PVR_VIDEO_RV: /* No need to search for any start code*/
		/* Disable Header Detect, PID Data Structure and  Pattern Mask*/
		break;

	case PVR_VIDEO_H263:
		return au1DmxVOffsetH263[u1Idx];

	case PVR_VIDEO_MPEG4:
		return au1DmxVOffsetMPEG4[u1Idx];

	case PVR_VIDEO_H265:
		return au1DmxVOffsetH265[u1Idx];

	default:
		PVR_LOG_ERR(TEXT("[PVR] %s -- Unknown video type: %d.\r\n"),
			DMX_FUNC_NAME, eVideoType);
		break;
	}

	return PVR_VNULL;
}

