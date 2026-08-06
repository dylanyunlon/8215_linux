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
*[File]			apcm_file.c
*[Author]		atc6013
*[Description]
*
******************************************************************************/
#include "apcm_file.h"

#define LOG_TAG                 	"[File]"
#define PCMFILE_WBUF_LEN         	102400
#define PCMFILE_RBUF_LEN         	20480
#define PCMFILE_RBUF_REMAIN_LEN		4192

static void file_uninit(apcm_file_t *this);

static void file_init_header(wave_header_t *header)
{
    header->datasize = 0;
    header->filesize = sizeof(wave_header_t);
    header->sample_rate = SAMPLE_RATE_48K;
    header->channels = 2;
    header->bits_per_sample = APCM_DEF_DATA_BITS;
    header->riff[0] = 'R';
    header->riff[1] = 'I';
    header->riff[2] = 'F';
    header->riff[3] = 'F';

    header->wave[0] = 'W';
    header->wave[1] = 'A';
    header->wave[2] = 'V';
    header->wave[3] = 'E';

    header->fmt[0] = 'f';
    header->fmt[1] = 'm';
    header->fmt[2] = 't';
    header->fmt[3] = ' ';
    header->fmtsize = 0x10;
    header->fmt_tag = 0x01;
    header->avg_bytes_per_sec = header->sample_rate * header->channels* (header->bits_per_sample / 8);
    header->block_align = header->bits_per_sample / 8 * header->channels;
    header->data[0] = 'd';
    header->data[1] = 'a';
    header->data[2] = 't';
    header->data[3] = 'a';
}


static bool file_open_phyfile(apcm_file_t *this)
{
	bool result = false;

	if (this)
	{
		this->fs = get_fs();
		set_fs(KERNEL_DS);

		if (this->read_only) {
			this->fd = filp_open(this->name, O_RDONLY, 0);
		} else {
			this->fd = filp_open(this->name, O_RDWR | O_CREAT, 0);
		}

		if (IS_ERR(this->fd)) {
			PR_E("[open_phyfile] %s: filp_open error(%d), time(%d)\n", this->name, PTR_ERR(this->fd), GET_SYS_TIME);
			set_fs(this->fs);
			//filp_close(this->fd, NULL);  // file open failed, no need to call filp_close, and will cause kernel panic
			result = false;
		} else {
			result = true;
		}
	}

	return (result);
}


static bool file_close_phyfile(apcm_file_t *this)
{
	if (this) {
		set_fs(this->fs);
    		filp_close(this->fd, NULL);
	}
}


static void file_read_header(apcm_file_t *this)
{
	if (this && file_open_phyfile(this))
	{
		loff_t cur_pos = vfs_llseek(this->fd, (loff_t)0, SEEK_SET);
		u32 len = (u32)vfs_read(this->fd, (void *)(&this->header), sizeof(wave_header_t), &cur_pos);
		if (len != sizeof(wave_header_t)) {
			PR_E("[read_header] %s: read header size err! \n", this->name);
		}
		if (this->header.channels > 2) {
			this->header.channels = 2;
		}
		file_close_phyfile(this);
	}
}


static void file_write_header(apcm_file_t *this)
{
	if (this && file_open_phyfile(this))
	{
		loff_t cur_pos = vfs_llseek(this->fd, (loff_t)0, SEEK_SET);
		u32 len = (u32)vfs_write(this->fd, (void *)(&this->header),
			sizeof(wave_header_t), &cur_pos);
		if (len != sizeof(wave_header_t)) {
			PR_E("[write_header] %s: write header size err! \n", this->name);
		}
		file_close_phyfile(this);
	}
}


static void file_read_phyfile(apcm_file_t *this)
{
	if (this && file_open_phyfile(this))
	{
		apcm_filebuf_t *buf = this->file_buf;
		u32 i, wptr, size[2], free_size;

		free_size = filebuf_get_free_size(buf);
		if (buf->wptr + free_size > buf->memory_size) {
			size[0] = buf->memory_size - buf->wptr;
			size[1] = free_size - size[0];
		} else {
			size[0] = free_size;
			size[1] = 0;
		}

		wptr = buf->wptr;
		for (i = 0; i < 2 && size[i]; i++)
		{
			loff_t cur_pos = vfs_llseek(this->fd, (loff_t)this->pos, SEEK_SET);
			size[i] = (u32)vfs_read(this->fd, (void *)(buf->addr + wptr), size[i], &cur_pos);
			this->pos += size[i];
			wptr = 0;
		}
		buf->wptr = (buf->wptr + size[0] + size[1]) % buf->memory_size;
		file_close_phyfile(this);
	}

}


static void file_write_phyfile(apcm_file_t *this)
{
	if (this && file_open_phyfile(this))
	{
		apcm_filebuf_t *buf = this->file_buf;
		u32 i, rptr, size[2], data_size;

		data_size = filebuf_get_data_size(buf);
		if (buf->rptr + data_size > buf->memory_size) {
			size[0] = buf->memory_size - buf->rptr;
			size[1] = data_size - size[0];
		} else {
			size[0] = data_size;
			size[1] = 0;
		}

		rptr = buf->rptr;
		for (i = 0; i < 2 && size[i]; i++)
		{
			loff_t cur_pos = vfs_llseek(this->fd, (loff_t)this->pos, SEEK_SET);
			size[i] = (u32)vfs_write(this->fd,
				(void *)(buf->addr + rptr), size[i], &cur_pos);
			this->pos += size[i];
			rptr = 0;
		}
		buf->rptr = (buf->rptr + size[0] + size[1]) % buf->memory_size;
		file_close_phyfile(this);
	}
}


static s32 file_thread(void *data)
{
	apcm_file_t *this = (apcm_file_t *)data;
	if (this)
	{
		apcm_thread_t *thread = this->thread;
		u64 timeout, time = APCM_INFINITE;
		PR_D("[thread] (%s, 0x%p): >>>>>> \n", this->name, thread);

		while (true)
		{
			if (this->read_only && filebuf_get_data_size(this->file_buf) < PCMFILE_RBUF_REMAIN_LEN) {
				file_read_phyfile(this);
			}

			timeout = thread_wait(thread, time);
			if (thread_should_stop(thread)) {
				break;
			}
			this->read_only ? file_read_phyfile(this) : file_write_phyfile(this);
		}
		file_uninit(this);
		PR_D("[thread] (%s): <<<<<<< \n", this->name);
	}

	return (RET_NOERR);
}


static void *file_init(apcm_file_t *this)
{
	if (this)
	{
	u32 memory_size = this->read_only ? PCMFILE_RBUF_LEN : PCMFILE_WBUF_LEN;
	this->fd = NULL;
	this->fs = get_fs();
	this->pos = sizeof(wave_header_t);

	this->file_buf = filebuf_open(NULL, memory_size, this->header.channels);
	thread_open(&this->thread, file_thread, this, "file_thread");
	this->state = STATE_STARTED;

	if (this->file_buf == NULL || this->thread == NULL) {
		PR_E("[init] alloc filebuf(0x%p) error or alloc thread(0x%p) err \n",
			this->file_buf->addr, this->thread);
		file_close(this);
		this = NULL;
	}

	if (this) {
		PR_D("[init] (%s, 0x%p)  success! \n", this->name, this);
	}
	}

	return (this);
}


static void file_uninit(apcm_file_t *this)
{
	if (this)
	{
	PR_I("[uninit] (%s, 0x%p) read_only(%d) \n", this->name, this, this->read_only);
	thread_close(this->thread);
	filebuf_close(this->file_buf);

	if (!this->read_only) {
		this->header.datasize = this->pos - sizeof(wave_header_t);
		this->header.filesize = this->pos - 8;
		file_write_header(this);
	}
	apcm_mem_free(this);
}
}


void *file_open_w(char *name, u32 suffix, u32 channels, u32 sample_rate)
{
	apcm_file_t *this = (apcm_file_t *)apcm_mem_alloc(sizeof(apcm_file_t));

	if (this) {
		sprintf(this->name, "%s%s_%d.wav", APCM_DEF_FILE_PATH, name, suffix);
		this->read_only = false;
		file_init_header(&this->header);
		this->header.channels = channels;
		this->header.sample_rate = sample_rate;
		this = file_init(this);
		file_write_header(this);
	} else {
		PR_E("[open_w] alloc class obj for %s failed! \n", name);
	}

	return (this);
}


void *file_open_r(char *name)
{
	apcm_file_t *this = (apcm_file_t *)apcm_mem_alloc(sizeof(apcm_file_t));

	if (this) {
		sprintf(this->name, "%s%s", APCM_DEF_FILE_PATH, name);
		this->read_only = true;
		file_read_header(this);
		this = file_init(this);
		//thread_wakeup(this->thread);
	} else {
		PR_E("[open_r] alloc class obj for %s failed! \n", name);
	}

	return (this);
}


void *file_close(apcm_file_t *this)
{
	PR_D("[close(%p)]  \n", this);

	if (this)
	{
		PR_I("[close] (%s, 0x%p) read_only(%d) \n", this->name, this, this->read_only);
		thread_stop(this->thread);
		this->state = STATE_STOPPED;
		PR_D("[close(%p)]  <<< \n", this);
	}

	return (NULL);
}


u32 file_read_data(apcm_file_t *this, void *addr1, void *addr2, u32 size)
{
	u32 copy_size = 0;
	if (this && this->state == STATE_STARTED && this->read_only)
	{
		copy_size = filebuf_read_data(this->file_buf, addr1, addr2, size);
		thread_wakeup(this->thread);
	}
	return (copy_size);
}


u32 file_write_data(apcm_file_t *this, void *addr1, void *addr2, u32 size)
{
	u32 copy_size = 0;
	if (this && this->state == STATE_STARTED && !this->read_only)
	{
		copy_size = filebuf_write_data(this->file_buf, addr1, addr2, size);
		thread_wakeup(this->thread);
	}
	return (copy_size);
}


u32 file_read(apcm_file_t *this, apcm_buf_t *pdst_buf)
{
	u32 copy_size = 0;
	if (this && this->state == STATE_STARTED && this->read_only)
	{
		copy_size = filebuf_read(this->file_buf, pdst_buf);
		thread_wakeup(this->thread);
	}
	return (copy_size);
}


u32 file_write(apcm_file_t *this, apcm_buf_t *psrc_buf)
{
	u32 copy_size = 0;
	if (this && this->state == STATE_STARTED && !this->read_only)
	{
		copy_size = filebuf_write(this->file_buf, psrc_buf);
		thread_wakeup(this->thread);
	}
	return (copy_size);
}

