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
*[File]                   apcm_outhw.h
*[Author]                 atc6013
*[Description]
*     .
******************************************************************************/
#ifndef __APCM_OUTHW_H_
#define __APCM_OUTHW_H_

#include "apcm_hw_comm.h"

typedef enum
{
    APCM_NORMAL_MODE = 0,
    APCM_BT_CALL_MODE,
    APCM_OUTPUT_MODE_NUM,
}APCM_OUTPUT_MODE_E;


typedef enum
{
	OUTHW_FRONT_LR_CH =   1 << 0,
	OUTHW_SURROUND_CH =   1 << 1,
	OUTHW_CENTER_CH =     1 << 2,
	OUTHW_SUBWOOFER_CH=   1 << 3,
	OUTHW_CH910 =         1 << 4,
	OUTHW_CH1112 =        1 << 5,
	OUTHW_CH78 =          1 << 6,
}OUTHW_CH_E;


typedef enum
{
	FADE_IN = 0,
	FADE_OUT,
	FADE_DONE,
} FADE_STATE_E;


typedef struct
{
	u32 state;

	u32 wptr;
	u32 data_size;
	apcm_buf_t *buf;

	u32 fade_state;
	s32 fade_gain[2];
	s32 fade_step[2];

	u32 gain;
	char name[20];
} virout_t;


extern u32 DspGpsMixGetSadr(void);
extern u32 DspGpsMixGetChSize(void);
extern u32 DspGpsMixGetRptr(void);
extern void DspGpsMixSetWptr(u32 wptr);

extern u32 DspAwbGetSadr(void);
extern u32 DspAwbGetChSize(void);
extern u32 DspAwbGetWptr(void);


u32  outhw_init(void);
u32  outhw_uninit(void);

void outhw_hibernation(bool wakeup);
void outhw_errhandle(void);

void outhw_set_rear_out_mode(void);
s32 outhw_set_dsp_mix_ch(u32 output_ch);
void outhw_set_output_ch(APCM_OUTPUT_MODE_E mode, u32 output_ch);
u32  outhw_switch_output_mode(APCM_OUTPUT_MODE_E mode);

void outhw_set_gain(APCM_OUTPUT_MODE_E mode, u32 gain);
u32 outhw_get_ref_boost_gain(void);

void outhw_dump_enable(bool enable);

virout_t *outhw_start(char *name);
virout_t *outhw_stop(virout_t *this);
u32 outhw_write(virout_t *this, apcm_buf_t *src_buf);

#endif  //__APCM_OUTHW_H_

