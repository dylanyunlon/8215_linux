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
*[File]                   apcm_outhw.c
*[Author]                 atc6013
*[Description]
*
******************************************************************************/

#include "apcm_outhw.h"

#define LOG_TAG         "[OutHw]"

#define OUTHW_DEF_SKIP_LEN         		(PLAYBACK_FRAME_SIZE + (PLAYBACK_FRAME_SIZE >> 1))
#ifdef ATC_AOSP_ENHANCEMENT_CTS
#define VIROUT_BUF_SIZE				4800
#else
#define VIROUT_BUF_SIZE				19200
#endif
#define OUTHW_FADE_SAMPLES			480


#define APCM_OUT_NUM		4
static virout_t _out[APCM_OUT_NUM];

u32 fade_count = 0;


static DEFINE_SPINLOCK(outhw_start_lock);


typedef struct
{
	union
	{
		u32 value;
		struct
		{
			u32 enable          :1;     // bit 0 DSP mix enable
			u32 front_lr_ch     :1;     // bit1  Front L/R ch mix config
			u32 surround_ch     :1;     // bit 2 surround ch mix config
			u32 center_ch       :1;     // bit 3  center ch mix config
			u32 ch_78           :1;     // bit 4  ch 7/8
			u32 ch_910          :1;     // bit 5  ch 9/10
			u32 ch_1112         :1;     // bit 6  ch 11/12
			u32 sub_ch          :1;     // bit 7   subwoofer ch mix config
			u32 reserve         :24;    // bit [8~31]
		};
	}u;
}outhw_cfg_t;


typedef struct
{
	u32 state;
	apcm_thread_t *thread;
	apcm_buf_t *buf;

	u32 gain;
	u32 ref_boost_gain;

	u32 output_ch[APCM_OUTPUT_MODE_NUM];
	u32 output_gain[APCM_OUTPUT_MODE_NUM];
	APCM_OUTPUT_MODE_E output_mode;

	bool dump_enable;
	apcm_file_t *dump_file;

} outhw_t;


static outhw_t _outhw = {
	.state = STATE_UNINIT,
};

static void outhw_dump_data(u32 cur_rptr);
static void outhw_fade_prepare(virout_t *this, u32 gain);


//===================================================//

static void outhw_enable_hw(bool enable)
{
	u32 code;
	PR_I("[enable] state(%d) enable(%d). \n", _outhw.state, enable);

	if (enable && (STATE_STOPPED == _outhw.state || STATE_INITED == _outhw.state))
	{
		AudGpsMix_CmdStart();
		PR_I("[enable] wait for start. Time(%d) \n", GET_SYS_TIME);
		code = apcm_event_wait_for_objects(1, &m_hGpsMixStartEvent, false, APCM_INFINITE);

		if (code == APCM_WAIT_OBJECT_0) {
			PR_I("[enable] start finished. Time(%d) \n", GET_SYS_TIME);
			buf_reset(_outhw.buf);
			DspGpsMixSetWptr(0);
			_outhw.state = STATE_STARTED;
			PR_I("[enable] state change to STARTED. \n");
		}
		else  {
			PR_E("[enable] wait for start failed.Time(%d) \n", GET_SYS_TIME);
		}
	}
	else if (!enable && (STATE_STARTED == _outhw.state))
	{
		AudGpsMix_CmdStop();
		PR_I("[enable]  wait for stop. Time(%d) \n", GET_SYS_TIME);
		code = apcm_event_wait_for_objects(1, &m_hGpsMixStopEvent, false, APCM_INFINITE);

		if (code == APCM_WAIT_OBJECT_0) {
			PR_D("[enable]  stop finished. Time(%d) \n", GET_SYS_TIME);
			buf_reset(_outhw.buf);
			_outhw.state = STATE_STOPPED;
			PR_I("[enable]  state change to STOPPED.\r\n");
		} else {
			PR_E("[enable]  wait for stop failed.\r\n");
		}
	}
}


static u32 outhw_mix_data(void *obj, buf_trans_t *trans)
{
	virout_t *this = (virout_t *)obj;
	if (this && trans && trans->size)
	{
		u32 samples = (trans->size >> 1);
		s32 data = 0;

		while (samples) {
			data = *trans->src[0]++;
			data = (data * this->fade_gain[0]) >> VOL_SHIFT;
			data += (s32)*trans->dst[0];
			*trans->dst[0]++ = SAT_16_BIT(data);

			data = *trans->src[1]++;
			data = (data * this->fade_gain[1]) >> VOL_SHIFT;
			data += (s32)*trans->dst[1];
			*trans->dst[1]++ = SAT_16_BIT(data);

			if (this->fade_state == FADE_IN) {
				fade_count++;
				this->fade_gain[0] += this->fade_step[0];
				this->fade_gain[1] += this->fade_step[1];
				if (this->fade_gain[0] >= LEFT_GAIN(this->gain)) {
					this->fade_state = FADE_DONE;
				}
			} else if (this->fade_state == FADE_OUT) {
				fade_count++;
				this->fade_gain[0] -= this->fade_step[0];
				this->fade_gain[1] -= this->fade_step[1];
				if (this->fade_gain[0] <= LEFT_GAIN(this->gain)) {
					this->fade_state = FADE_DONE;
				}
			}

			if (OUTHW_FADE_SAMPLES == fade_count) {
				this->fade_state = FADE_DONE;
				fade_count = 0;
			}

			if (this->fade_state == FADE_DONE) {
				this->fade_gain[0] = LEFT_GAIN(this->gain);
				this->fade_gain[1] = RIGHT_GAIN(this->gain);
			}

			samples--;
		}
	}

	return (RET_NOERR);
}


static void outhw_data_proc(void)
{
	u32 i = 0, used_size = 0;
	u32 cur_rptr = DspGpsMixGetRptr();
	u32 rptr = (cur_rptr + _outhw.buf->buf_size - PLAYBACK_FRAME_SIZE * 2) % _outhw.buf->buf_size;
	u32 wptr = (cur_rptr + PLAYBACK_FRAME_SIZE * 2) % _outhw.buf->buf_size;

	DspGpsMixSetWptr(wptr);
	outhw_dump_data(cur_rptr);
	used_size = buf_clean_data(_outhw.buf, rptr);

	for (i = 0; i < APCM_OUT_NUM; i++)
	{
		virout_t *this = &_out[i];
		if (this->state == STATE_STARTED || this->state == STATE_TO_STOP)
		{
			if (this->state == STATE_TO_STOP) {
				buf_set_reserve_data_size(this->buf, 0); // release data to do fade out
				outhw_fade_prepare(this, 0);
				this->state = STATE_STOPPED;
			} else if (_outhw.gain != this->gain) {
				outhw_fade_prepare(this, _outhw.gain);
			}

			if (this->data_size > used_size) {
				this->data_size -= used_size;
			} else {
				this->data_size = 0;
				this->wptr = (cur_rptr + OUTHW_DEF_SKIP_LEN) % _outhw.buf->buf_size;
			}

			buf_set_write_cb(_outhw.buf, outhw_mix_data, this);
			_outhw.buf->wptr = this->wptr;
			this->data_size += buf_copy(_outhw.buf, this->buf);;
			this->wptr = _outhw.buf->wptr;

			if (this->state == STATE_STOPPED){
				this->buf = buf_close(this->buf);
				this->state = STATE_UNINIT;
				PR_D("[thread] release virout(0x%p) time(%d) \n", this, GET_SYS_TIME);
			}
		}
	}
}


static s32 outhw_thread(void *data)
{
    	apcm_thread_t *thread = _outhw.thread;
    	u64 time = APCM_INFINITE, code = 0;
    	u32 proc_time = 0;
   	PR_I("[Thread(0x%p)] >>>>>>>>>>>>>>>>>>>>> \n", thread);

	while (true)
    	{
		code = thread_wait(thread, time);
		if (code != 0) {
			PR_D("[Thread] wakeup(%d). time(%d) \n", (u32)code, GET_SYS_TIME);
	        }

		proc_time = GET_SYS_TIME;
	        if (thread_should_stop(thread)) {
			break;
	        }

	        if (g_hibernate) {
	        	time = APCM_INFINITE;
			continue;
	        }

		outhw_data_proc();
		proc_time = GET_SYS_TIME - proc_time;
		time = (proc_time > PLAYBACK_INTR_TIME) ? 0 : (PLAYBACK_INTR_TIME - proc_time);
    	}
    	thread_close(_outhw.thread);
	buf_close(_outhw.buf);

    	PR_I("[Thread(0x%p)] <<<<<<<<<<<<<<<<<<<<<\r\n", thread);
    	return (RET_NOERR);
}


//=======================================================//

u32 outhw_init(void)
{
	u32 result = RET_NOERR;
	fade_count = 0;

	if (_outhw.state == STATE_UNINIT)
	{
		u32 sadr = DspGpsMixGetSadr();
		u32 buf_len = DspGpsMixGetChSize();
		u32 i = 0;

		for (i = 0; i < APCM_OUT_NUM; i++) {
			_out[i].state = STATE_UNINIT;
		}

		thread_open(&_outhw.thread, outhw_thread, (void *)(&_outhw), "outhw_thread");

		if (sadr) {
			_outhw.buf = buf_open(sadr, buf_len, STEREO);
			buf_set_write_cb(_outhw.buf, outhw_mix_data, NULL);

			PR_I("[init] SAdr(0x%x, 0x%x) BufLen(%d)\n", sadr, (sadr + buf_len), buf_len);
		} else {
			PR_E("[init] Failed to malloc DspMix AOUT Buffer!!! \n");
		}

		outhw_set_output_ch(APCM_BT_CALL_MODE, OUTHW_FRONT_LR_CH);
		outhw_set_output_ch(APCM_NORMAL_MODE,  OUTHW_FRONT_LR_CH);

		outhw_set_gain(APCM_BT_CALL_MODE, (VOL_0dB << 16) + VOL_0dB);
		outhw_set_gain(APCM_NORMAL_MODE, (VOL_0dB << 16) + VOL_0dB);
		_outhw.gain = _outhw.output_gain[APCM_NORMAL_MODE];

		_outhw.dump_enable = false;
		_outhw.dump_file = NULL;
		_outhw.state = STATE_INITED;
		outhw_enable_hw(true);
	}

	return (result);
}


u32 outhw_uninit(void)
{
	PR_I("[uninit] state(%d) \n", _outhw.state);
	if (_outhw.state != STATE_UNINIT)
	{
		u32 i = 0;
		outhw_enable_hw(false);
		for (i = 0; i <APCM_OUT_NUM; i++) {
			if (_out[i].state == STATE_UNINIT) {
				outhw_stop(&_out[i]);
			}
		}
		thread_stop(_outhw.thread);
		_outhw.state = STATE_UNINIT;
	}

	return (RET_NOERR);
}


void outhw_hibernation(bool wakeup)
{
	if (wakeup) {
		buf_reset(_outhw.buf);
		outhw_switch_output_mode(APCM_NORMAL_MODE);
		outhw_enable_hw(true);
		thread_wakeup(_outhw.thread);
	} else {
		outhw_enable_hw(false);
	}
}


void outhw_errhandle(void)
{
	PR_E("[errhandle] cur state(%d) \n", _outhw.state);
	_outhw.state = STATE_INITED;
	outhw_enable_hw(true);
}


void outhw_set_rear_out_mode(void)
{
	PR_I("[Set_Rear_Out_Mode] \n");
	_outhw.output_ch[APCM_NORMAL_MODE] |= (OUTHW_CH910 << 1);
	if (_outhw.output_mode == APCM_NORMAL_MODE) {
		DspCfgSetGpsMixCh(_outhw.output_ch[APCM_NORMAL_MODE]);
	}
}


s32 outhw_set_dsp_mix_ch(u32 output_ch)
{
	u32 u4MixCh = 0;
	if ((((u32)0x0) == (output_ch & 0x3FU)) || /*  Mix to NULL */
		(0x1U == (output_ch & 0x3FU)) || /*  Mix to L */
		(0x2U == (output_ch & 0x3FU)) || /*  Mix to R */
		(0x3U == (output_ch & 0x3FU)) || /*  Mix to L R */
		(0x4U == (output_ch & 0x3FU)) || /*  Mix to Ls */
		(0x5U == (output_ch & 0x3FU)) || /*  Mix to L Ls */
		(0x8U == (output_ch & 0x3FU)) || /*  Mix to Rs */
		(0xAU == (output_ch & 0x3FU)) || /*  Mix to R Rs */
		(0xCU == (output_ch & 0x3FU)) || /*  Mix to Ls Rs */
		(0xFU == (output_ch & 0x3FU)) || /*  Mix to L R Ls Rs */
		(0x10U == (output_ch & 0x3FU)) || /*  Mix to C */
		(0x11U == (output_ch & 0x3FU)) || /*  Mix to L C */
		(0x13U == (output_ch & 0x3FU)) || /*  Mix to L R C */
		(0x14U == (output_ch & 0x3FU)) || /*  Mix to Ls C */
		(0x15U == (output_ch & 0x3FU)) || /*  Mix to L Ls C */
		(0x1CU == (output_ch & 0x3FU)) || /*  Mix to Ls Rs C */
		(0x1FU == (output_ch & 0x3FU)) || /*  Mix to L R Ls Rs C */
		(0x20U == (output_ch & 0x3FU)) || /*  Mix to Sub */
		(0x22U == (output_ch & 0x3FU)) || /*  Mix to R Sub */
		(0x23U == (output_ch & 0x3FU)) || /*  Mix to L R Sub */
		(0x28U == (output_ch & 0x3FU)) || /*  Mix to Rs Sub */
		(0x2AU == (output_ch & 0x3FU)) || /*  Mix to R Rs Sub */
		(0x2CU == (output_ch & 0x3FU)) || /*  Mix to Ls Rs Sub */
		(0x2FU == (output_ch & 0x3FU)) || /*  Mix to L R Ls Rs Sub */
		(0x30U == (output_ch & 0x3FU)) || /*  Mix to C Sub */
		(0x33U == (output_ch & 0x3FU)) || /*  Mix to L R C Sub */
		(0x3CU == (output_ch & 0x3FU)) || /*  Mix to Ls Rs C Sub*/
		(0x3FU == (output_ch & 0x3FU))) { /*  Mix to L R Ls Rs C Sub */
	} else {
		PR_E("[init] Dsp Can not mix to Ch(0x%x)\r\n", (u32)output_ch);
		return INVALIDPRAM;
	}

	if (output_ch & 0x3U) {
		u4MixCh |= OUTHW_FRONT_LR_CH;
	}
	if (output_ch & 0xCU) {
		u4MixCh |= OUTHW_SURROUND_CH;
	}
	if (output_ch & 0x10U) {
		u4MixCh |= OUTHW_CENTER_CH;
	}
	if (output_ch & 0x20U) {
		u4MixCh |= OUTHW_SUBWOOFER_CH;
	}
	if (output_ch & 0x40U) {
		u4MixCh |= OUTHW_CH910;
	}
	PR_I("[set_dsp_mix_ch] u4MixCh=0x%x\r\n", (u32)u4MixCh);

	if (_outhw.output_mode == APCM_NORMAL_MODE) {
		outhw_set_output_ch(APCM_NORMAL_MODE, u4MixCh);
	} else {
		outhw_cfg_t cfg;
		cfg.u.value = (u4MixCh << 1);

		cfg.u.enable = cfg.u.sub_ch;
		cfg.u.sub_ch = cfg.u.ch_78;
		cfg.u.ch_78 = cfg.u.enable;

		cfg.u.enable = 1;    // this bit must enable
		_outhw.output_ch[APCM_NORMAL_MODE] = cfg.u.value;
	}

	return (RET_NOERR);
}


void outhw_set_output_ch(APCM_OUTPUT_MODE_E mode, u32 output_ch)
{
	PR_I("[Set_Output_Ch]  Mode(%d) OutputCh(%d) >>>\n", mode, output_ch);
	if (mode < APCM_OUTPUT_MODE_NUM)
	{
		outhw_cfg_t cfg;
		cfg.u.value = (output_ch << 1);

		cfg.u.enable = cfg.u.sub_ch;
		cfg.u.sub_ch = cfg.u.ch_78;
		cfg.u.ch_78 = cfg.u.enable;

		cfg.u.enable = 1;    // this bit must enable
		_outhw.output_ch[mode] = cfg.u.value;
		DspCfgSetGpsMixCh(_outhw.output_ch[mode]);
		PR_I("[Set_Output_Ch]  Mode(%d) OutputCh(%d) <<<\n", mode, output_ch);
	}
}


u32 outhw_switch_output_mode(APCM_OUTPUT_MODE_E mode)
{
	if (mode < APCM_OUTPUT_MODE_NUM)
	{
		PR_I("[switch_output_mode]  Mode(%d => %d) OutputCh(%d) Gain(0x%x)\n",
			_outhw.output_mode, mode, _outhw.output_ch[mode], _outhw.output_gain[mode]);
		_outhw.output_mode = mode;
		DspCfgSetGpsMixCh(_outhw.output_ch[mode]);
		_outhw.gain = _outhw.output_gain[mode];
	}

	return (RET_NOERR);
}


void outhw_set_gain(APCM_OUTPUT_MODE_E mode, u32 gain)
{
	if (mode < APCM_OUTPUT_MODE_NUM)
	{
		PR_D("[set_gain] cur_mode(%d) mode(%d) gain(0x%x => 0x%x) \n",
			_outhw.output_mode, mode, _outhw.output_gain[mode], gain);

		_outhw.output_gain[mode] = gain;
		if (_outhw.output_mode == mode) {
			PR_D("[set_gain] gain(0x%x => 0x%x) \n", _outhw.gain, gain);
			_outhw.gain = gain;
		}

		if (mode == APCM_BT_CALL_MODE)
		{
			u32 tmp = LEFT_GAIN(gain);
			if (tmp < VOL_0dB) {
				tmp = (VOL_0dB * VOL_0dB / tmp);
				if (tmp > 0xFFFF) {
					tmp = 0xFFFF;
				}
			}
			_outhw.ref_boost_gain = (tmp << 16) + tmp;
			PR_D("[set_gain] mode(%d) reff_boost_gain(0x%x) \n", mode, _outhw.ref_boost_gain);
		}
	}
}


u32 outhw_get_ref_boost_gain(void)
{
	return (_outhw.ref_boost_gain);
}


void outhw_dump_enable(bool enable)
{
	PR_I("[dump_data] %d => %d \n", _outhw.dump_enable, enable);
	if (_outhw.dump_enable != enable) {
		_outhw.dump_enable = enable;
		if (enable) {
			_outhw.dump_file = file_open_w("mixbuf", GET_FILE_IDX, STEREO, PLAYBACK_FS);
		} else {
			// close file in outhw_dump_data()
		}
	}
}


static void outhw_dump_data(u32 cur_rptr)
{
	if (_outhw.dump_file)  {
		if (_outhw.dump_enable) {
			apcm_buf_t dump_buf;
			apcm_memcpy(&dump_buf, _outhw.buf, sizeof(apcm_buf_t));

			dump_buf.wptr = cur_rptr;
			file_write(_outhw.dump_file, &dump_buf);
		} else {
			_outhw.dump_file = file_close(_outhw.dump_file);
		}
	}
}

//==================================================================//

virout_t *outhw_start(char *name)
{
	u32 i = 0, rptr = 0;
	virout_t *this = NULL;

	PR_D("[start] >>> \n");

	spin_lock(&outhw_start_lock);
	for (i = 0; i < APCM_OUT_NUM; i++)
	{
		if (_out[i].state == STATE_UNINIT) {
			this = &_out[i];
			this->state = STATE_INITED;
			break;
		}
	}
	spin_unlock(&outhw_start_lock);

	if (this)
	{
		this->buf = buf_open(NULL, VIROUT_BUF_SIZE, STEREO);
		if (this->buf) {
			this->data_size = 0;
			buf_set_reserve_data_size(this->buf, (OUTHW_FADE_SAMPLES << 1)); // keep data to do fade out

			this->fade_gain[0] = 0;
			this->fade_gain[1] = 0;
			this->gain = 0;
			sprintf(this->name, "%s", name);

			this->state = STATE_STARTED;
			thread_wakeup(_outhw.thread);

		} else {
			this->buf = buf_close(this->buf);
			this->state = STATE_UNINIT;
			this = NULL;
		}
	}

	if (this) {
		PR_D("[start(%s)] BufInfo(0x%p, 0x%p, %d) Time(%d) << \n", this->name,
			_outhw.buf->addr[0], _outhw.buf->addr[1], _outhw.buf->buf_size, GET_SYS_TIME);
	} else {
		PR_E("[start] fail! \n");
	}

	return (this);
}


virout_t *outhw_stop(virout_t *this)
{
	if (this && this->state == STATE_STARTED) {
		PR_D("[stop(%s)] time(%d) \n", this->name, GET_SYS_TIME);
		this->state = STATE_TO_STOP;
		thread_wakeup(_outhw.thread);
	}
	log_show_version(1);

	return (NULL);
}


u32 outhw_write(virout_t *this, apcm_buf_t *src_buf)
{
	u32 copy_size = 0;

	if (this && this->state == STATE_STARTED) {
#ifndef ATC_AOSP_ENHANCEMENT_CTS
        copy_size = buf_copy(this->buf, src_buf);
#else
        copy_size = buf_limit_copy(this->buf, src_buf, 1920 + 192);
#endif
	}

	return (copy_size);
}


static void outhw_fade_prepare(virout_t *this, u32 gain)
{
	if (this)
	{
		u32 cur_gain = (this->fade_gain[1] << 16) + this->fade_gain[0];
		PR_D("[fade_prepare(%s)] gain(0x%x => 0x%x)\n", this->name, cur_gain, gain);
		this->gain = gain;
	fade_count = 0;

		if (LEFT_GAIN(gain) > this->fade_gain[0]) {
			this->fade_state = FADE_IN;
			this->fade_step[0] = (LEFT_GAIN(gain) - this->fade_gain[0]) / OUTHW_FADE_SAMPLES;
			this->fade_step[1] = (RIGHT_GAIN(gain) - this->fade_gain[1]) / OUTHW_FADE_SAMPLES;
		} else {
			this->fade_state = FADE_OUT;
			this->fade_step[0] = (this->fade_gain[0] - LEFT_GAIN(gain)) / OUTHW_FADE_SAMPLES;
			this->fade_step[1] = (this->fade_gain[0] - RIGHT_GAIN(gain)) / OUTHW_FADE_SAMPLES;
		}
	}
}


