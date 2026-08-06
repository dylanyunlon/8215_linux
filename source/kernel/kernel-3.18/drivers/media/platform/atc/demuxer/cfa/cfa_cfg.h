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




#ifndef _CFA_CFG_H_
#define _CFA_CFG_H_

#include "dmx_def.h"
/*-----------------------------------------------------------------------------
				Config for ASF
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_ASF_MAX_INST_NS				DMX_MAX_SPT_INST_CNT
#define CONFIG_CFA_ASF_DEBUGGING_LOG_EN			1
#define CONFIG_CFA_ASF_SP_SUPPORT				0
#define CONFIG_CFA_ASF_DRM_SUPPORT				0
#define CONFIG_CFA_ASF_MPEG4_SUPPORT			1
#define CONFIG_CFA_ASF_DMC_SUPPORT				1


/*-----------------------------------------------------------------------------
				Config for AVI
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_AVI_MAX_INST_NS				DMX_MAX_SPT_INST_CNT
#define CONFIG_CFA_AVI_DEBUGGING_LOG_EN			1
#define CONFIG_CFA_DIVX_SP_SUPPORT				1
#define CONFIG_CFA_DIVX_DRM_SUPPORT			1
#define CONFIG_CFA_VDEC_MPEG4_SUPPORT			1
#define CONFIG_CFA_DIVX_DMC_SUPPORT			1
/* to control if transfer all audio chunk to fifo */
#define CONFIG_CFA_AVI_TX_ALL_AUD				1


/*-----------------------------------------------------------------------------
				Config for FLV
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_FLV_MAX_INST_NS				DMX_MAX_SPT_INST_CNT
#define CONFIG_CFA_FLV_DEBUGGING_LOG_EN			1
#define CONFIG_CFA_FLV_SP_SUPPORT				0
#define CONFIG_CFA_FLV_DRM_SUPPORT				0
#define CONFIG_CFA_FLV_MPEG4_SUPPORT			1
#define CONFIG_CFA_FLV_DMC_SUPPORT				1
#define CONFIG_CFA_FLV_LOG_CLI					1


/*-----------------------------------------------------------------------------
				Config for MKV
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_MKV_MAX_INST_NS				DMX_MAX_SPT_INST_CNT
#define CONFIG_CFA_MKV_DEBUGGING_LOG_EN			1
#define CONFIG_CFA_MKV_SUPPORT_HEADER_STRIPING	0
#define CONFIG_CFA_MKV_SUPPORT_AAC				1
#define CONFIG_CFA_MKV_RMKV					1
/*for the case of one block contains one P-frame and some B-frames*/
#define CONFIG_CFA_MKV_SUPPORT_PBBB			0
#define CONFIG_CFA_MKV_SUPPORT_PTS_AJUST_BBB	0
#define CONFIG_CFA_MKV_SUPPORT_SPLAST_STOP		0
#define CONFIG_CFA_MKV_SUPPORT_PURE_AUDIO		1
#define CONFIG_CFA_MKV_SUPPORT_DECOMPRESSION	0
#define CONFIG_CFA_MPG_DEBUGGING_LOG_EN			0
#define CONFIG_CFA_MPG_MAX_INST_NS				1


/*-----------------------------------------------------------------------------
				Config for OGM
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_OGM_MAX_INST_NS				DMX_MAX_SPT_INST_CNT
#define CONFIG_CFA_OGM_DEBUGGING_LOG_EN			1
#define CONFIG_CFA_OGM_DIVX3_AU_SUPPORT			1
#define CONFIG_CFA_OGM_PTS_ADJUST_SUPPORT		1
#define CONFIG_CFA_OGM_NEW_SYNCBUF				1
#define CONFIG_CFA_OGM_LENBYTES_ISNOT_ZERO		1
#define CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT		1


/*-----------------------------------------------------------------------------
				Config for RM
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_RM_MAX_INST_NS				DMX_MAX_SPT_INST_CNT
#define CONFIG_CFA_RM_DEBUGGING_LOG_EN			1
#define CONFIG_CFA_RM_SP_SUPPORT				0
#define CONFIG_CFA_RM_DRM_SUPPORT				0
#define CONFIG_CFA_RM_MPEG4_SUPPORT			1
#define CONFIG_CFA_RM_DMC_SUPPORT				1
#define CONFIG_CFA_RM_LOG_CLI					1


/*-----------------------------------------------------------------------------
				Config for SWF
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_SWF_MAX_INST_NS				DMX_MAX_SPT_INST_CNT
#define CONFIG_CFA_SWF_DEBUGGING_LOG_EN			1
#define CONFIG_CFA_SWF_SP_SUPPORT				0

/*-----------------------------------------------------------------------------
				Config for TRIV
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_TRIV_MAX_INST_NS 4


/*-----------------------------------------------------------------------------
				Config for Audio
-----------------------------------------------------------------------------*/
#define CONFIG_CFA_AUDIO_MAX_INST_NS 4


#endif				/* #ifndef _CFA_CFG_H_ */
