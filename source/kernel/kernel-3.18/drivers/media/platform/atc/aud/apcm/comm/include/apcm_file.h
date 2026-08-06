
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
*[File]			apcm_file.h
*[Author]		atc6013
*[Description]
*
******************************************************************************/
#ifndef __APCM_FILE_H__
#define __APCM_FILE_H__

#include "apcm_filebuf.h"
#include "apcm_thread.h"

#define APCM_DEF_FILE_PATH           "/sdcard/mtklog/audio_dump/"

typedef struct {
	BYTE    riff[4];		// "RIFF"
	u32     filesize;		// File size - 8
	BYTE    wave[4];		// "WAVE"
	BYTE    fmt[4];			// "fmt "
	u32     fmtsize;		// 0x10
	u16     fmt_tag;		// 0x01 -> PCM
	u16     channels;         	// Channels
	u32     sample_rate;		// Sampling Rate (samples per second)
	u32     avg_bytes_per_sec;	// Average Bytes per second
	u16     block_align ;
	u16     bits_per_sample;	// Bits per sample
	BYTE    data[4];		// "data"
	u32     datasize;		// Data Size(Byte)
} wave_header_t;


typedef struct
{
	u32 state;
	bool read_only;

	char name[200];
	wave_header_t header;

	apcm_filebuf_t *file_buf;
	apcm_thread_t *thread;

	struct file *fd;
	mm_segment_t fs;
	u32 pos;

}apcm_file_t;


void *file_open_w(char *name, u32 suffix, u32 channels, u32 sample_rate);
void *file_open_r(char *name);
void *file_close(apcm_file_t *this);

u32 file_read_data(apcm_file_t *this, void *addr1, void *addr2, u32 size);
u32 file_write_data(apcm_file_t *this, void *addr1, void *addr2, u32 size);

u32 file_read(apcm_file_t *this, apcm_buf_t *pdst_buf);
u32 file_write(apcm_file_t *this, apcm_buf_t *psrc_buf);


#endif // #ifndef __APCM_FILE_H__

