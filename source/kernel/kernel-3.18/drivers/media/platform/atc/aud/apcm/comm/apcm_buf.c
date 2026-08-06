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
*[File]			apcm_buf.c
*[Author]		atc6013
*[Description]
*
******************************************************************************/

#include "apcm_buf.h"

#define LOG_TAG		"[Buffer]"


u32 buf_trans_data(void *obj, buf_trans_t *trans)
{
	if (trans && trans->size) {
		if (trans->dst[0] && trans->src[0]) {
			apcm_memcpy((void *)trans->dst[0], (void *)trans->src[0], trans->size);
		}
		if ((trans->channels == STEREO) && trans->dst[1] && trans->src[1]) {
			apcm_memcpy((void *)trans->dst[1], (void *)trans->src[1], trans->size);
		}
	}

	return (RET_NOERR);
}


u32 buf_trans_data_with_gain(void *obj, buf_trans_t *trans)
{
	s32 gain[2] = {VOL_0dB, VOL_0dB};
	if (obj) {
		u32 tmp_gain = *((u32 *)obj);
		gain[0] = LEFT_GAIN(tmp_gain);
		gain[1] = LEFT_GAIN(tmp_gain);
	}

	if (trans && trans->size)
	{
		u32 samples = trans->size >> 1;
		s32 data = 0;
		while (samples)
		{
			data = *trans->src[0]++;
			data = (data * gain[0]) >> VOL_SHIFT;
			*trans->dst[0]++ = SAT_16_BIT(data);

			data = *trans->src[1]++;
			data = (data * gain[1]) >> VOL_SHIFT;
			*trans->dst[1]++ = SAT_16_BIT(data);

			samples--;
		}
	}

	return (RET_NOERR);
}



apcm_buf_t *buf_open(void *addr, u32 buf_size, u32 channels)
{
	apcm_buf_t *this = (apcm_buf_t *)apcm_mem_alloc(sizeof(apcm_buf_t));
	bool err = true;

	if (this)
	{
		if (addr) {
			this->is_alloc = false;
			this->addr[0] = addr;
		} else {
			this->is_alloc = true;
			this->addr[0] = (void *)apcm_mem_alloc(buf_size * channels);
		}

		if (this->addr[0]) {
			this->addr[1] = (channels == 2) ? (this->addr[0] + buf_size) : this->addr[0];
			this->buf_size = buf_size;
			this->channels = channels;
			buf_reset(this);

			this->reserve_data_sz = 0;
			this->reserve_free_sz = BUFFER_SAFE_SIZE;

			this->is_full = false;
			this->write_cb = buf_trans_data;
			this->cb_obj = this;

			PR_D1("[open] addr(0x%p 0x%p) size(%d) channels(%d) \n",
				this->addr[0], this->addr[1], this->buf_size, this->channels);
			err = false;
		} else {
			PR_E("[open] Alloc memory for buf class error! \n");
		}
	} else {
		PR_E("[open] Alloc buf class error! \n");
	}

	if (err) {
		PR_E("[open] err!\n");
		buf_close(this);
		this = NULL;
	}

	return (this);
}


void *buf_close(apcm_buf_t *this)
{
	if (this)
	{
		if (this->is_alloc && this->addr[0]) {
			apcm_mem_free(this->addr[0]);
		}
		apcm_mem_free(this);
	}
	return (NULL);
}


void buf_reset(apcm_buf_t *this)
{
	if (this)
	{
		this->rptr= 0;
		this->wptr = 0;
		apcm_memset(this->addr[0], 0x0, (this->buf_size * this->channels));
	}
}


void buf_set_reserve_data_size(apcm_buf_t *this, u32 reserve_sz)
{
	if (this) {
		this->reserve_data_sz = reserve_sz;
	}
}


void buf_set_reserve_free_size(apcm_buf_t *this, u32 reserve_sz)
{
	if (this) {
		this->reserve_free_sz = reserve_sz;
	}
}


void buf_set_full(apcm_buf_t *this)
{
	if (this) {
		this->is_full = true;
	}
}


void buf_set_write_cb(apcm_buf_t *this, PFN_BUF_WRITE_CB write_cb, void *cb_obj)
{
	if (this) {
		this->write_cb = write_cb;
		this->cb_obj = cb_obj;
	}
}


u32 buf_get_data_size(apcm_buf_t *this)
{
	s32 size = 0;

	if (this)
	{
		size = this->wptr - this->rptr;
		if (size < 0 || (size == 0 && this->is_full)) {
			size += this->buf_size;
		}
		size = (size > this->reserve_data_sz) ? (size - this->reserve_data_sz) : 0;
	}

	return (u32)(size);
}


u32 buf_get_free_size(apcm_buf_t *this)
{
	s32 size = 0;

	if (this)
	{	size = this->rptr - this->wptr;
		if (size < 0 || (size == 0 && !this->is_full)) {
			size += this->buf_size;
		}
		size = (size > this->reserve_free_sz) ? (size - this->reserve_free_sz) : 0;
	}

	return (u32)(size);
}


u32 buf_clean_data(apcm_buf_t *this, u32 cur_rptr)
{
	u32 clean_size = 0;
	u32 size = 0, rollback_size = 0;

	if (cur_rptr < this->rptr) {
		size = this->buf_size - this->rptr;
		rollback_size = cur_rptr;
	} else {
		size = cur_rptr - this->rptr;
		rollback_size = 0;
	}

	if (size) {
		apcm_memset((this->addr[0] + this->rptr), 0, size);
		apcm_memset((this->addr[1] + this->rptr), 0, size);
	}
	if (rollback_size) {
		apcm_memset((this->addr[0]), 0, rollback_size);
		apcm_memset((this->addr[1]), 0, rollback_size);
	}

	this->rptr = cur_rptr;
	clean_size = size + rollback_size;

	return (clean_size);
}


u32 buf_read_data(apcm_buf_t *this, void *dst_addr1, void *dst_addr2, u32 read_size)
{
	u32 copy_size = 0;

	if (this && dst_addr1)
	{
		copy_size = buf_get_data_size(this);
		if (copy_size > read_size) {
			copy_size = read_size;
		}

		if (copy_size)
		{
			if (copy_size)
			{
				buf_trans_t trans, roolback_trans;
				trans.channels = this->channels;
				roolback_trans.channels = this->channels;

				trans.dst[0] = (s16 *)dst_addr1;
				trans.dst[1] = (s16 *)dst_addr2;
				trans.src[0] = (s16 *)(this->addr[0] + this->rptr);
				trans.src[1] = (s16 *)(this->addr[1] + this->rptr);
				trans.size = copy_size;
				roolback_trans.size = 0;

				if (this->rptr + copy_size > this->buf_size) {
					trans.size = this->buf_size - this->rptr;
					roolback_trans.size = copy_size - trans.size;

					roolback_trans.dst[0] = (s16 *)(dst_addr1 + trans.size);
					roolback_trans.dst[1] = (s16 *)(dst_addr2 + trans.size);
					roolback_trans.src[0] = (s16 *)this->addr[0];
					roolback_trans.src[1] = (s16 *)this->addr[1];
				}

				buf_trans_data(this->cb_obj, &trans);
				if (roolback_trans.size) {
					buf_trans_data(this->cb_obj, &roolback_trans);
				}
				this->rptr = (this->rptr + copy_size) % this->buf_size;
			}
		}
	}

	return (copy_size);
}


u32 buf_write_data(apcm_buf_t *this, void *src_addr1, void *src_addr2, u32 write_size)
{
	u32 copy_size = 0;

	if (this && src_addr1)
	{
		if (src_addr2 == NULL) {
			src_addr2 = src_addr1;
		}

		copy_size = buf_get_free_size(this);
		copy_size = APCM_MIN(copy_size, write_size);

		if (copy_size)
		{
			buf_trans_t trans, roolback_trans;
			trans.channels = this->channels;
			roolback_trans.channels = this->channels;

			trans.dst[0] = (s16 *)(this->addr[0] + this->wptr);
			trans.dst[1] = (s16 *)(this->addr[1] + this->wptr);
			trans.src[0] = (s16 *)src_addr1;
			trans.src[1] = (s16 *)src_addr2;
			trans.size = copy_size;
			roolback_trans.size = 0;

			if (this->wptr + copy_size > this->buf_size) {
				trans.size = this->buf_size - this->wptr;
				roolback_trans.size = copy_size - trans.size;

				roolback_trans.dst[0] = (s16 *)this->addr[0];
				roolback_trans.dst[1] = (s16 *)this->addr[1];
				roolback_trans.src[0] = (s16 *)(src_addr1 + trans.size);
				roolback_trans.src[1] = (s16 *)(src_addr2 + trans.size);
			}

			if (this->write_cb) {
				this->write_cb(this->cb_obj, &trans);
				if (roolback_trans.size) {
					this->write_cb(this->cb_obj, &roolback_trans);
				}
			}
			this->wptr = (this->wptr + copy_size) % this->buf_size;
		}
	}

	return (copy_size);
}


u32 buf_limit_copy(apcm_buf_t *dst, apcm_buf_t *src, u32 max_size)
{
	u32 copy_size = 0;
	if (dst && src)
	{
		u32 free_size = buf_get_free_size(dst);
		u32 data_size = buf_get_data_size(src);

		copy_size = APCM_MIN(free_size, data_size);
		copy_size = APCM_MIN(copy_size, max_size);

		if (copy_size)
		{
			u32 rptr = src->rptr;
			u32 tmp_size = src->buf_size - rptr;

			src->is_full = false;
			if (rptr + copy_size > src->buf_size) {
				buf_write_data(dst, (src->addr[0] + rptr), (src->addr[1] + rptr), tmp_size);
				buf_write_data(dst, (src->addr[0]), (src->addr[1]), (copy_size - tmp_size));
			} else {
				buf_write_data(dst, (src->addr[0] + rptr), (src->addr[1] + rptr), copy_size);
			}
			src->rptr = (src->rptr + copy_size) % src->buf_size;
		}
	}
	return (copy_size);
}


u32 buf_copy(apcm_buf_t *dst, apcm_buf_t *src)
{
	return (buf_limit_copy(dst, src, BUF_MAX_COPY_SIZE));
}


