/********************************************************************************************
*     LEGAL DISCLAIMER
*
*     (Header of MediaTek Software/Firmware Release or Documentation)
*
*     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
*     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
*     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
*     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
*     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
*     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
*     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
*     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
*     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
*     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
*     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
*     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
*     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
************************************************************************************************/

/******************************************************************************
*[File]                 stream.c
*[Author]               mtk68153
*[Description]
*    implementation for  stream process class
******************************************************************************/

#include "apcm_stream.h"
#include "apcm_speech.h"

#define LOG_TAG  "[stream]"

static DEFINE_SPINLOCK(stream_lock);

static apcm_thread_t *_stream_pb_thread = NULL;
static apcm_thread_t *_stream_rec_thread = NULL;

static stream_t *_stream[STRM_NUM];


static struct snd_pcm_runtime *stream_get_runtime(stream_t *this);


static u32 pb_stream_read(stream_t *this)
{
	u32 proc_bytes = 0, copy_size = 0;
	struct snd_pcm_runtime *runtime = stream_get_runtime(this);

	if (runtime)
	{
		apcm_filebuf_t *strm_buf = this->strm_buf;
		u32 new_wptr = frames_to_bytes(runtime, (runtime->control->appl_ptr % this->buffer_frames));
		strm_buf->wptr = (new_wptr + strm_buf->memory_size - 4) % strm_buf->memory_size;  // keep buffer no empty and no full

		copy_size = filebuf_read(this->strm_buf, this->proc_buf);
		proc_bytes = copy_size * this->channels;

		this->used_size += proc_bytes;
		if (this->used_size >= this->period_size) {
			snd_pcm_period_elapsed(this->substream);
			this->used_size %= this->period_size;
		}
	}

	return (copy_size);
}


static bool pb_stream_proc(void)
{
	bool all_stopped = true;
	u32 idx = 0;

	for (idx = STRM_OUT_START; idx < STRM_OUT_MAX; idx++)
	{
		stream_t *this = _stream[idx];
		if (this == NULL) {
			continue;
		}

		if (this->state == STATE_STARTED)
		{
			all_stopped = false;
			pb_stream_read(this);

			switch (this->type)
			{
			case STRM_OUT_SPH_TX:
				btcall_write(this->proc_buf);
				break;

			case STRM_OUT_SPH_RX:
			case STRM_OUT_NORMAL:
				outhw_write(this->hw, this->proc_buf);
				break;

			default:
				break;
			}
		}
		else if (this->state == STATE_TO_STOP)
		{
			this->state = STATE_STOPPED;
			PR_D("[pb_thread(%s)] stopped! time(%d) \n", this->name, GET_SYS_TIME);
			apcm_up(&this->stop_sema);
		}
	}

	return (all_stopped);
}


static s32 pb_stream_thread(void *data)
{
	u64 time = APCM_INFINITE, code = 0;
	bool all_stopped = true;
	u32 proc_time = GET_SYS_TIME;
	apcm_thread_t *thread = _stream_pb_thread;
	PR_I("[pb_thread(0x%p)] >>>>>>>>>>>>>>>>>>>>> \n", thread);

	while (thread)
	{
		code = thread_wait(thread, time);
		proc_time = GET_SYS_TIME;
		if (code != 0) {
			PR_D("[pb_thread] wakeup(%d). time(%d) \n", (u32)code, GET_SYS_TIME);
		}

		if (thread_should_stop(thread)) {
			break;
		}

		all_stopped = pb_stream_proc();
		if (all_stopped) {
			time = APCM_INFINITE;
		} else {
			proc_time = GET_SYS_TIME - proc_time;
			time = (proc_time <  PLAYBACK_INTR_TIME) ? (PLAYBACK_INTR_TIME - proc_time) : 1;
		}
	}

	PR_I("[pb_thread(0x%p)]: <<<<<<<<<<<<<<<<<<<<< \n", thread);
	return (RET_NOERR);
}



static u32 rec_stream_write(stream_t *this)
{
	u32 proc_bytes = 0, copy_size = 0;
	struct snd_pcm_runtime *runtime = stream_get_runtime(this);

	if (runtime)
	{
		apcm_filebuf_t *strm_buf = this->strm_buf;
		u32 new_rptr = frames_to_bytes(runtime, (runtime->control->appl_ptr % this->buffer_frames));

		strm_buf->rptr = new_rptr;
		copy_size = filebuf_write(this->strm_buf, this->proc_buf);
		proc_bytes = copy_size * this->channels;

		this->used_size += proc_bytes;
		if (this->used_size >= this->period_size) {
			snd_pcm_period_elapsed(this->substream);
			this->used_size %= this->period_size;
		}
	}

	return (copy_size);
}


static bool rec_stream_proc(void)
{
	bool all_stopped = true;
	u32 idx = 0;

	for (idx = STRM_IN_START; idx < STRM_IN_MAX; idx++)
	{
		stream_t *this = _stream[idx];
		if (this == NULL) {
			continue;
		}

		if (this->state == STATE_STARTED)
		{
			all_stopped = false;
			switch (this->type)
			{
			case STRM_IN_SPH_REF:
				speech_ref_read(this->proc_buf);
				break;

			case STRM_IN_SPH_MIC:
				speech_mic_read(this->proc_buf);
				break;

			case STRM_IN_SPH_RX:
				btcall_read(this->proc_buf);
				break;

			case STRM_IN_MIC:
				virmic_read(this->hw, this->proc_buf);
				break;

			default:
				break;
			}
			rec_stream_write(this);
		}
		else if (this->state == STATE_TO_STOP)
		{
			this->state = STATE_STOPPED;
			PR_D("[rec_thread(%s)] stopped! time(%d) \n", this->name, GET_SYS_TIME);
			apcm_up(&this->stop_sema);
		}
	}

	return (all_stopped);
}


static s32 rec_stream_thread(void *data)
{
	u64 time = APCM_INFINITE, code = 0;
	bool all_stopped = true;
	u32 proc_time = GET_SYS_TIME;
	apcm_thread_t *thread = _stream_rec_thread;
	PR_I("[rec_thread(0x%p)] >>>>>>>>>>>>>>>>>>>>> \n", thread);

	while (thread)
	{
		code = thread_wait(thread, time);
		proc_time = GET_SYS_TIME;
		if (code != 0) {
			PR_D("[rec_thread] wakeup(%d). time(%d) \n", (u32)code, GET_SYS_TIME);
		}

		if (thread_should_stop(thread)) {
			break;
		}

		all_stopped = rec_stream_proc();
		if (all_stopped) {
			time = APCM_INFINITE;
		} else {
			proc_time = GET_SYS_TIME - proc_time;
			time = (proc_time <  RECORD_INTR_TIME) ? (RECORD_INTR_TIME - proc_time) : 1;
		}
	}

	PR_I("[rec_thread(0x%p)]: <<<<<<<<<<<<<<<<<<<<< \n", thread);
	return (RET_NOERR);
}


static struct snd_pcm_runtime *stream_get_runtime(stream_t *this)
{
	struct snd_pcm_runtime *runtime = NULL;

	if (NULL == this) {
		PR_E("[get_runtime] stream is NULL! \n");
	} else if (NULL == this->substream) {
		PR_E("[get_runtime] substream is NULL! \n");
	} else if (NULL == this->substream->runtime) {
		PR_E("[get_runtime] runtime is NULL! \n");
	} else {
		runtime = this->substream->runtime;
	}

	return (runtime);
}


static void stream_init_name(stream_t *this, u32 type)
{
	if (this)
	{
		switch(type)
		{
		case STRM_IN_MIC:
			sprintf(this->name, "%s", "in_mic");
			break;

		case STRM_IN_SPH_MIC:
			sprintf(this->name, "%s", "in_sph_mic");
			break;

		case STRM_IN_SPH_RX:
			sprintf(this->name, "%s", "in_sph_rx");
			break;

		case STRM_IN_SPH_REF:
			sprintf(this->name, "%s", "in_sph_ref");
			break;

		case STRM_OUT_SPH_TX:
			sprintf(this->name, "%s", "out_sph_tx");
			break;

		case STRM_OUT_RESERVE:
			sprintf(this->name, "%s", "out_reserve");
			break;

		case STRM_OUT_SPH_RX:
			sprintf(this->name, "%s", "out_sph_rx");
			break;

		default:
			sprintf(this->name, "%s_%d", "out", (type - STRM_OUT_NORMAL));
			break;
		}
	}
}


void stream_init(void)
{
	if (_stream_pb_thread == NULL && _stream_rec_thread == NULL)
	{
		u32 type = 0;
		thread_open(&_stream_pb_thread,  pb_stream_thread,  NULL, "pb_stream_thread");
		thread_open(&_stream_rec_thread, rec_stream_thread, NULL, "rec_stream_thread");

		for (type = 0; type < STRM_NUM; type++)
		{
			stream_t *this = apcm_mem_alloc(sizeof(stream_t));
			if (this)
			{
			 	_stream[type] = this;
			 	this->state = STATE_UNINIT;
			 	stream_init_name(this, type);

				apcm_sema_init(&this->stop_sema, 0);
				if (IS_REC_STREAM(type)) {
					this->thread = _stream_rec_thread;
				} else {
					this->thread = _stream_pb_thread;
				}
			}
			else
			{
				PR_E("[init] alloc error, idx(%d) \n", type);
			}
		}

		PR_I("[init] thread(0x%p, 0x%p) \n", _stream_pb_thread, _stream_rec_thread);
	}
}


stream_t *stream_open(struct snd_pcm_substream *substream, u32 type)
{
	stream_t *this = NULL;
	u32 idx = STRM_NUM;

	if (substream && (type < STRM_OUT_MAX))
	{
		if (type == STRM_OUT_NORMAL) {
			for (idx = STRM_OUT_NORMAL; idx < STRM_OUT_MAX; idx++) {
				if (_stream[idx]->state == STATE_UNINIT) {
					break;
				}
			}
		} else if (_stream[type]->state == STATE_UNINIT) {
			idx = type;
		}

		if (idx < STRM_NUM && _stream[idx])
		{
			this = _stream[idx];
			this->state = STATE_INITED;
			this->substream = substream;
			this->type = type;

			this->hw = NULL;
			this->proc_buf = NULL;
			this->strm_buf = NULL;
			PR_D("[open(%s)] type(%d, %d) Success! time(%d)\n", this->name, type, idx, GET_SYS_TIME);
		}
	}

	if (this == NULL) {
		PR_E("[open] no resource for open type(%d) idx(%d) stream(%p)!\n", type, idx, substream);
	}
	return (this);
}


stream_t *stream_close(stream_t *this)
{
	if (this && this->state != STATE_UNINIT)
	{
		PR_D("[close(%s)] state(%d) time(%d) >>>\n", this->name, this->state, GET_SYS_TIME);
		if (this->state == STATE_STARTED) {
			stream_stop(this);
		}

		if (this->state == STATE_TO_STOP) {
			apcm_down(&this->stop_sema);
			PR_D("[close(%s)] wait done, state(%d) time(%d) \n", this->name, this->state, GET_SYS_TIME);
		}

		this->proc_buf = buf_close(this->proc_buf);
		this->strm_buf = filebuf_close(this->strm_buf);

		switch(this->type)
		{
		case STRM_IN_MIC:
			this->hw = virmic_stop(this->hw);
			break;

		case STRM_OUT_SPH_RX:
		case STRM_OUT_NORMAL:
		case STRM_OUT_NORMAL2:
		case STRM_OUT_NORMAL3:
		case STRM_OUT_NORMAL4:
			this->hw = outhw_stop(this->hw);
			break;

		default:
			break;
		}

		this->state = STATE_UNINIT;
		PR_D("[close(%s)] state(%d) time(%d) <<< \n", this->name, this->state, GET_SYS_TIME);
	}
	return (NULL);
}


void stream_prepare(stream_t *this)
{
	struct snd_pcm_runtime *runtime = stream_get_runtime(this);

	if (runtime)
	{
		spin_lock(&stream_lock);
		snd_pcm_substream_chip(this->substream);

		void *dma_area = (void *)runtime->dma_area;
		u32 buffer_size = snd_pcm_lib_buffer_bytes(this->substream);

		this->buffer_frames = bytes_to_frames(runtime, buffer_size);
		this->period_size = snd_pcm_lib_period_bytes(this->substream);
		this->used_size = 0;

		this->fs = runtime->rate;
		this->channels = (runtime->channels > MONO) ? STEREO : MONO;

		spin_unlock(&stream_lock);
		PR_D("[prepare] channels(%d) fs(%d), period(%d), buf_frame(%d) dma(0x%p)  \n",
			this->channels, this->fs, this->period_size, this->buffer_frames, dma_area);

		if (this->strm_buf) {
			filebuf_reset(this->strm_buf);
		} else {
			this->strm_buf = filebuf_open(dma_area, buffer_size, this->channels);
		}

		if (this->proc_buf) {
			buf_reset(this->proc_buf);
		} else {
			this->proc_buf = buf_open(NULL, STRM_TMP_BUF_SZ, STEREO);
		}

		this->state = STATE_PREPARED;
	}
}


void stream_start(stream_t *this)
{
	if (this && (this->state == STATE_STOPPED || this->state == STATE_PREPARED))
	{
		PR_D("[start(%s)] time(%d)!\n", this->name, GET_SYS_TIME);

		switch (this->type)
		{
		case STRM_IN_SPH_REF:
			speech_start(SPEECH_REF_STRM);
			break;

		case STRM_IN_SPH_MIC:
			speech_start(SPEECH_MIC_STRM);
			break;

		case STRM_IN_SPH_RX:
			btcall_start();
			break;

		case STRM_IN_MIC:
			if (this->hw == NULL) {
				this->hw  = virmic_start(this->name, this->fs);
			} else {
				virmic_reset_point(this->hw);
			}
			break;

		case STRM_OUT_SPH_RX:
		case STRM_OUT_NORMAL:
		case STRM_OUT_NORMAL2:
		case STRM_OUT_NORMAL3:
		case STRM_OUT_NORMAL4:
			if (this->hw == NULL) {
				this->hw = outhw_start(this->name);
			}
			break;

		default:
			break;
		}

		this->stop_sema.count = 0;
		this->state = STATE_STARTED;
		thread_wakeup(this->thread);
		PR_D("[start(%s)] time(%d) <<< %p\n", this->name, GET_SYS_TIME, this->thread);
	}
}


void stream_stop(stream_t *this)
{
	if (this && this->state == STATE_STARTED)
	{
		PR_D("[stop(%s)] time(%d)! %p\n", this->name, GET_SYS_TIME, this->thread);
		this->state = STATE_TO_STOP;

		switch(this->type)
		{
		case STRM_IN_SPH_REF:
			speech_stop(SPEECH_REF_STRM);
			break;

		case STRM_IN_SPH_MIC:
			speech_stop(SPEECH_MIC_STRM);
			break;

		case STRM_IN_SPH_RX:
			btcall_stop();
			break;

		default:
			break;
		}
		thread_wakeup(this->thread);
	}
}


u32 stream_get_ptr(stream_t *this)
{
	u32 ptr = 0;
	struct snd_pcm_runtime *runtime = stream_get_runtime(this);

	if (runtime) {
		ptr = IS_REC_STREAM(this->type) ? this->strm_buf->wptr : this->strm_buf->rptr;
		ptr = bytes_to_frames(runtime, ptr);
	}

	return (ptr);
}


