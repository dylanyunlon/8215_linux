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
*[File]                     apcm_filebuf.c
*[Author]                   atc6013
*[Description]
*
******************************************************************************/

#include "apcm_filebuf.h"

#define LOG_TAG		"[FileBuf]"


apcm_filebuf_t *filebuf_open(void *addr, u32 memory_size, u32 channels)
{
	apcm_filebuf_t *this = (apcm_filebuf_t *)apcm_mem_alloc(sizeof(apcm_filebuf_t));
	bool err = true;

	if (this)
	{
		if (addr) {
			this->is_alloc = false;
			this->addr = addr;
		} else {
			this->is_alloc = true;
			this->addr = (void *)apcm_mem_alloc(memory_size);
		}

		if (this->addr) {
			this->memory_size = memory_size;
			this->channels = channels;
			filebuf_reset(this);
			this->is_full = false;

			PR_D1("[open] addr(0x%p) size(%d) channels(%d)\n",
				this->addr, this->memory_size, this->channels);
			err = false;
		} else {
			PR_E("[open] Alloc memory for filebuf class error! \n");
		}
	} else {
		PR_E("[open] Alloc filebuf class error! \n");
	}

	if (err) {
		PR_E("open err!\n");
		filebuf_close(this);
		this = NULL;
	}
	return (this);
}


void *filebuf_close(apcm_filebuf_t *this)
{
	if (this)
	{
		if (this->is_alloc) {
			apcm_mem_free(this->addr);
		}
		apcm_mem_free(this);
	}

	return (NULL);
}


void filebuf_reset(apcm_filebuf_t *this)
{
	if (this)
	{
		this->rptr= 0;
		this->wptr = 0;
		apcm_memset(this->addr, 0, this->memory_size);
	}
}


void filebuf_set_full(apcm_filebuf_t *this)
{
	if (this) {
		this->is_full = true;
	}
}


u32 filebuf_get_data_size(apcm_filebuf_t *this)
{
	s32 size = 0;
	if (this)
	{
		size = this->wptr - this->rptr;
		if (size < 0 || (size == 0 && this->is_full)) {
			size += this->memory_size;
		}
	}
	return (u32)(size);
}


u32 filebuf_get_free_size(apcm_filebuf_t *this)
{
	s32 size = 0;
	if (this)
	{
		size = this->rptr - this->wptr;
		if (size < 0 || (size == 0 && !this->is_full)) {
			size += this->memory_size;
		}
		size = (size > BUFFER_SAFE_SIZE) ? (size - BUFFER_SAFE_SIZE) : 0;
	}
	return (u32)(size);
}


u32 filebuf_read_data(apcm_filebuf_t *this, void *addr1, void *addr2, u32 size)
{
	u32 copy_size = 0;

	if (this && addr1)
	{
		s16 data;
		s16 *src_addr;
		s16 *dst_addr1 = (s16 *)(addr1);
		s16 *dst_addr2 = addr2 ? (s16 *)(addr2) : NULL;

		u32 dst_frame = size >> 1;
		u32 src_frame = filebuf_get_data_size(this) >> this->channels;
		u32 frame = APCM_MIN(dst_frame, src_frame);

		copy_size = frame << 1;

		while(frame)
		{
			src_addr  = (s16 *)(this->addr + this->rptr);

			data = *src_addr++;
			*dst_addr1++ = data;

			if (dst_addr2) {
				if (this->channels == 2) {
					data = *src_addr++;
				}
				*dst_addr2++ = data;
			}

			frame--;
			this->rptr = (this->rptr + 2 * this->channels) % this->memory_size;
		}
	}

	return (copy_size);
}


u32 filebuf_write_data(apcm_filebuf_t *this, void *addr1, void *addr2, u32 size)
{
	u32 copy_size = 0;

	if (this && addr1)
	{
		s16 data;
		s16 *dst_addr;
		s16 *src_addr1 = (s16 *)(addr1);
		s16 *src_addr2 = addr2 ? (s16 *)(addr2) : NULL;

		u32 src_frame = size >> 1;
		u32 dst_frame = filebuf_get_free_size(this) >> this->channels;
		u32 frame = APCM_MIN(dst_frame, src_frame);

		copy_size = frame << 1;

		while(frame)
		{
			dst_addr  = (s16 *)(this->addr + this->wptr);

			data = *src_addr1++;
			*dst_addr++ = data;

			if (this->channels == 2) {
				if (src_addr2) {
					data = *src_addr1++;
				}
				*dst_addr++ = data;
			}

			frame--;
			this->wptr = (this->wptr + 2 * this->channels) % this->memory_size;
		}
	}

	return (copy_size);
}


u32 filebuf_read(apcm_filebuf_t *this, apcm_buf_t *dst)
{
	u32 copy_size = 0;

	if (this && dst)
	{
		s16 data;
		s16 *dst_addr1, *dst_addr2, *src_addr;
		u32 dst_frame = buf_get_free_size(dst) >> 1;
		u32 src_frame = filebuf_get_data_size(this) >> this->channels;
		u32 frame = APCM_MIN(dst_frame, src_frame);
		copy_size = frame << 1;

		while(frame)
		{
			dst_addr1 = (s16 *)(dst->addr[0] + dst->wptr);
			dst_addr2 = (s16 *)(dst->addr[1] + dst->wptr);
			src_addr  = (s16 *)(this->addr   + this->rptr);

			data = *src_addr++;
			*dst_addr1 = data;

			if (this->channels == 2) {
				data = (s32)*src_addr++;
			}

			if (dst->channels == 2) {
				*dst_addr2 = (s16)data;
			}
			frame--;

			this->rptr = (this->rptr + 2 * this->channels) % this->memory_size;
			dst->wptr = (dst->wptr + 2) % dst->buf_size;
		}
	}

	return (copy_size);
}


u32 filebuf_write(apcm_filebuf_t *this, apcm_buf_t *src)
{
	u32 copy_size = 0;

	if (this && src)
	{
		s16 data;
		s16 *src_addr1, *src_addr2, *dst_addr;
		u32 src_frame = buf_get_data_size(src) >> 1;
		u32 dst_frame = filebuf_get_free_size(this) >> this->channels;
		u32 frame = APCM_MIN(dst_frame, src_frame);
		copy_size = frame << 1;

		while(frame)
		{
			src_addr1 = (s16 *)(src->addr[0] + src->rptr);
			src_addr2 = (s16 *)(src->addr[1] + src->rptr);
			dst_addr  = (s16 *)(this->addr + this->wptr);

			data = *src_addr1;
			*dst_addr++ = data;

			if (src->channels == 2) {
				data = *src_addr2;
			}

			if (this->channels == 2) {
				*dst_addr++ = data;
			}
			frame--;

			this->wptr = (this->wptr + 2 * this->channels) % this->memory_size;
			src->rptr = (src->rptr + 2) % src->buf_size;
		}
	}

	return (copy_size);
}


