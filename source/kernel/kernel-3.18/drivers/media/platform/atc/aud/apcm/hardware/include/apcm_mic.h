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
*[File]             apcm_mic.h
*[Author]
*[Description]
*    mic interface
******************************************************************************/
#ifndef __APCM_MIC_H_
#define __APCM_MIC_H_

#include "apcm_hw_comm.h"
#include "apcm_asrc.h"

typedef struct
{
	volatile u32 state;
	u32 rptr;

	asrc_chs_t *asrc;
	apcm_buf_t *asrc_buf;

	char name[20];

} virmic_t;

u32  mic_init(void);
u32  mic_uninit(void);
void mic_hibernation(bool wakeup);

u32  mic_start(void);
u32  mic_stop(void);

u32  mic_get_wptr(void);
u32  mic_read(apcm_buf_t *dst_buf, u32 *rptr);

u32  mic_set_fs(u32 fs);
u32  mic_get_fs(void);

bool mic_set_source(AUD_MIC_SRC src, AUD_PINMUX_I2SMICIN i2s_pin);
void mic_set_clk(MCLK_TYPE_T mclk_type, AUD_LRCK_CYC_T cycle,
		AUD_MIC_CLK_SRC clk_src, bool bck_invert, bool lrck_invert);
void mic_set_fmt(u32 src_bit_num, u32 out_bit_num, AUDFMT_INTF_E fmt_intf);

void mic_set_hw_gain(u32 hw_gain);
void mic_set_primary_idx(u32 primary_idx);

void mic_dump_regs(void);


virmic_t *virmic_start(char *name, u32 fs);
virmic_t *virmic_stop(virmic_t *this);

u32 virmic_read(virmic_t *this, apcm_buf_t *dst_buf);
void virmic_reset_point(virmic_t *this);


#endif  //__APCM_MIC_H_

