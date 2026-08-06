/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of AutoChips Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AutoChips SOFTWARE") RECEIVED
 *     FROM AutoChips AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. AutoChips EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES AutoChips PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE AutoChips SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AutoChips SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY AutoChips SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND AutoChips'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE AutoChips SOFTWARE RELEASED HEREUNDER WILL BE, AT AutoChips'S OPTION,
 *     TO REVISE OR REPLACE THE AutoChips SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO AutoChips FOR SUCH AutoChips SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/******************************************************************************
*[File]				apcm_define.h
*[Author]			atc6013
*[Description]
*
******************************************************************************/

#ifndef __APCM_DEFINE_H__
#define __APCM_DEFINE_H__

#include "apcm_ver.h"


#define VOL_0dB				0x2000U
#define VOL_0dB_SETTING			255
#define VOL_SHIFT			13
#define LEFT_GAIN(gain)			(s32)(gain & 0xFFFF)
#define RIGHT_GAIN(gain)		(s32)(gain >> 16)


#define MONO				1U
#define STEREO				2U


#define SAMPLE_RATE_8K			8000
#define SAMPLE_RATE_11K			11025
#define SAMPLE_RATE_16K			16000
#define SAMPLE_RATE_22K			22050
#define SAMPLE_RATE_24K			24000
#define SAMPLE_RATE_32K			32000
#define SAMPLE_RATE_44K			44100
#define SAMPLE_RATE_48K			48000
#define SAMPLE_RATE_64K			64000
#define SAMPLE_RATE_88K			88200
#define SAMPLE_RATE_96K			96000


#define SAMPLES_PER_MSEC(fs)		(fs / 1000)

#define APCM_DEF_DATA_BITS		16U
#define APCM_BYTES_PER_SAMPLE		2U


#define PLAYBACK_FS			SAMPLE_RATE_48K
#define PLAYBACK_INTR_TIME		20U	// ms
#define PLAYBACK_FRAME_SAMPLE		(SAMPLES_PER_MSEC(PLAYBACK_FS) * PLAYBACK_INTR_TIME)
#define PLAYBACK_FRAME_SIZE		(PLAYBACK_FRAME_SAMPLE * 2U)


#define RECORD_FS			SAMPLE_RATE_48K
#define RECORD_INTR_TIME		20U	// ms
#define RECORD_FRAME_SAMPLE		(SAMPLES_PER_MSEC(RECORD_FS) * RECORD_INTR_TIME)
#define RECORD_FRAME_SIZE		(RECORD_FRAME_SAMPLE * 2U)


#define STATE_UNINIT			0U
#define STATE_INITED			1U
#define STATE_PREPARED			2U
#define STATE_WAIT_SYNC			3U
#define STATE_STARTED			4U
#define STATE_TO_STOP			5U
#define STATE_STOPPED			6U


#define RET_NOERR			0
#define RET_ERROR			0xFFFFFFFF
#define RET_INVALID_PRAM		0xFFFFFFFE
#define RET_INVALID_STATE		0xFFFFFFFD
#define RET_NO_RESOURCE			0xFFFFFFFC


#define GET_SYS_TIME			((u32)1000U * (u32)jiffies / (u32)HZ)


#define ALIGNMENT(val, align)		((val) / (align) * (align))
#define APCM_MIN(a, b)			(((a) < (b)) ? (a) : (b))
#define APCM_MAX(a, b)			(((a) > (b)) ? (a) : (b))


#define apcm_mem_alloc(size)		kmalloc(size, GFP_KERNEL)
#define apcm_mem_free			kfree
#define apcm_memcpy			x_memcpy
#define apcm_memset			x_memset

typedef struct semaphore 		apcm_sema_t;
#define apcm_sema_init			sema_init
#define apcm_down			down
#define apcm_up				up


#endif // #ifndef __APCM_DEFINE_H__
