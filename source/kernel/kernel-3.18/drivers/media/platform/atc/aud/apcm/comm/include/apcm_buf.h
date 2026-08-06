
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
*[File]			apcm_buf.h
*[Author]		atc6013
*[Description]
*
******************************************************************************/
#ifndef __APCM_BUF_H__
#define __APCM_BUF_H__

#include "apcm_log.h"


#define BUF_MAX_COPY_SIZE	0xffffffff

#define BUFFER_SAFE_SIZE	8U
#define SAT_16_BIT(a)		(s16)((a) > 0x7fff ? 0x7fff : (a) < -32768 ? -32768 : (a))


typedef struct
{
	s16 *dst[2];
	s16 *src[2];
	u32 size;
	u32 channels;
} buf_trans_t;


typedef u32 (*PFN_BUF_WRITE_CB)(void *obj, buf_trans_t *trans);


typedef struct
{
	void *addr[STEREO];
	u32 buf_size;           // one channel buf lenth (bytes)
	u32 channels;

	u32 rptr;		// read pointer (offset)
	u32 wptr;		// write pointer (offset)

	u32 reserve_data_sz;
	u32 reserve_free_sz;

	bool is_alloc;		// true: alloc by itself, 	false: alloc by caller
	bool is_full;		// the buffer whether full

	PFN_BUF_WRITE_CB write_cb;
	void *cb_obj;

} apcm_buf_t;


u32 buf_trans_data(void *obj, buf_trans_t *trans);
u32 buf_trans_data_with_gain(void *obj, buf_trans_t *trans);

//open:  if addr is NULL, means need alloc buffer self, or alloc by caller
apcm_buf_t *buf_open(void *addr, u32 buf_size, u32 channels);
void *buf_close(apcm_buf_t *this);

void buf_reset(apcm_buf_t *this);

void buf_set_reserve_data_size(apcm_buf_t *this, u32 reserve_sz);
void buf_set_reserve_free_size(apcm_buf_t *this, u32 reserve_sz);

void buf_set_full(apcm_buf_t *this);
void buf_set_write_cb(apcm_buf_t *this, PFN_BUF_WRITE_CB write_cb, void *cb_obj);

u32 buf_get_data_size(apcm_buf_t *this);
u32 buf_get_free_size(apcm_buf_t *this);

u32 buf_clean_data(apcm_buf_t *this, u32 cur_rptr);
u32 buf_read_data( apcm_buf_t *this, void *dst_addr1, void *dst_addr2, u32 size);
u32 buf_write_data(apcm_buf_t *this, void *src_addr1, void *src_addr2, u32 size);

u32 buf_limit_copy(apcm_buf_t *dst, apcm_buf_t *src, u32 max_size);
u32 buf_copy(apcm_buf_t *dst, apcm_buf_t *src);


#endif // #ifndef __APCM_BUF_H__

