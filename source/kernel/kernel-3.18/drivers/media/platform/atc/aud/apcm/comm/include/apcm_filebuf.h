
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
*[File]			apcm_filebuf.h
*[Author]		atc6013
*[Description]
*
******************************************************************************/
#ifndef __APCM_FILEBUF_H__
#define __APCM_FILEBUF_H__

#include "apcm_buf.h"


typedef struct
{
	void *addr;
	u32 memory_size;	// total size of the buffer
	u32 channels;

	u32 rptr;		// read pointer (offset)
	u32 wptr;		// write pointer (offset)

	bool is_alloc;		// true: alloc by itself, 	false: alloc by caller
	bool is_full;		// the buffer whether full

}apcm_filebuf_t;


//open:  if addr is NULL, means need alloc buffer self, or alloc by caller
apcm_filebuf_t *filebuf_open(void *addr, u32 buf_size, u32 channels);
void *filebuf_close(apcm_filebuf_t *this);
void filebuf_reset(apcm_filebuf_t *this);

void filebuf_set_full(apcm_filebuf_t *this);

u32 filebuf_get_data_size(apcm_filebuf_t *this);
u32 filebuf_get_free_size(apcm_filebuf_t *this);

u32 filebuf_read_data(apcm_filebuf_t *this, void *addr1, void *addr2, u32 size);
u32 filebuf_write_data(apcm_filebuf_t *this, void *addr1, void *addr2, u32 size);

u32 filebuf_read( apcm_filebuf_t *this, apcm_buf_t *src);
u32 filebuf_write(apcm_filebuf_t *this, apcm_buf_t *dst);

#endif // #ifndef __APCM_FILEBUF_H__

