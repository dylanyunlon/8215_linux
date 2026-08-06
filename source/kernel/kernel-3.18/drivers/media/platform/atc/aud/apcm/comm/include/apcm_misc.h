
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
*[File]             	apcm_misc.h
*[Author]   		atc6013
*[Description]
*
******************************************************************************/
#ifndef __APCM_MISC_H__
#define __APCM_MISC_H__

#include "apcm_file.h"

#define APCM_TBL_SINE_16BIT_SZ       128
extern char APCM_TBL_SINE_16BIT[APCM_TBL_SINE_16BIT_SZ];

AUDIO_SAMPLING_T misc_get_fs_idx(u32 fs);

u32 misc_fill_sine16_tbl(apcm_buf_t *buf);

void misc_irq_enable(u32 vect);
void misc_irq_disable(u32 vect);
void misc_irq_clear(u32 vect);

bool misc_isr_reg(u32 vect, x_os_isr_fct pfn_isr);
bool misc_isr_unreg(u32 vect);


#endif // #ifndef __APCM_MISC_H__

