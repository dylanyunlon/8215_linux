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
*[File]			APCM_ASRC.h
*[Author]		tongfa.luo@autochips.com
*[Description]
*
******************************************************************************/
#ifndef __APCM_ASRC_H_
#define __APCM_ASRC_H_

#include "apcm_hw_comm.h"

#define ASRC_CHSET_NUMBER		(6U)

#define ASRC_IBUF_EMPTY 		(1 << 0)
#define ASRC_OBUF_FULL			(1 << 1)

#define ASRC_IBUF_EMPTY_INTR		(0x01 << 0)
#define ASRC_IBUF_AMOUNT_INTR		(0x01 << 1)
#define ASRC_OBUF_OV_INTR		(0x01 << 2)
#define ASRC_OBUF_AMOUNT_INTR		(0x01 << 3)
#define ASRC_ALL_INTR			(ASRC_IBUF_EMPTY_INTR | ASRC_IBUF_AMOUNT_INTR | ASRC_OBUF_OV_INTR | ASRC_OBUF_AMOUNT_INTR)

typedef u32 (*PFN_ASRC_CB)(void *asrc, u32 intr_type);

typedef struct
{
    u32 ifs;
    u32 ofs;

    u32 ibw;
    u32 obw;

    PFN_ASRC_CB pfn_cb;
    u32 intr_type;

}asrc_cfg_t;


typedef struct
{
    u32 state;
    u32 idx;

    u32 ibuf_sadr;
    u32 obuf_sadr;

    apcm_buf_t *ibuf;
    apcm_buf_t *obuf;

    asrc_cfg_t cfg;
    u32 i_palette;
    u32 o_palette;

} asrc_chs_t;


typedef struct
{
	u32 ifs;
	u32 ofs;

	asrc_chs_t *asrc;
} asrc_if_t;


bool asrc_init(void);
bool asrc_uninit(void);
void asrc_hibernation(bool wake_up);

asrc_chs_t *asrc_open_by_cfg(asrc_cfg_t *cfg);
asrc_chs_t *asrc_open_special(u32 idx, asrc_cfg_t *cfg);

asrc_chs_t *asrc_open(u32 ifs, u32 ofs);
void *asrc_close(asrc_chs_t *this);
void asrc_setup(asrc_chs_t *this, asrc_cfg_t *cfg);

void asrc_start(asrc_chs_t *this);
void asrc_stop(asrc_chs_t *this);

u32  asrc_read(asrc_chs_t *this, apcm_buf_t *dst_buf);
u32  asrc_write(asrc_chs_t *this, apcm_buf_t *src_buf);

void asrc_dump_regs(void);


#endif  //__APCM_ASRC_H_

