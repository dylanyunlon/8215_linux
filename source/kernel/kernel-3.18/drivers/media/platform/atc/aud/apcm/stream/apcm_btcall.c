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
*[File]                apcm_btcall.cpp
*[Author]              atc6013
*[Description]
*
******************************************************************************/

#include "apcm_btcall.h"

#define LOG_TAG  "[btcall]"

#define STRMBT_INT_TIME			10U	// ms

#define STRMBT_6630_NOISE_CNT		6

typedef struct
{
	volatile u32 state;

	bool enhance;
	virmic_t *mic;	// for no enhance mode

	u32 isr_cnt;
	apcm_thread_t *thread;
	apcm_sema_t stop_sema;

	bool out_directly;
	virout_t *out;
	asrc_chs_t *rx_asrc;

	apcm_buf_t *tx_buf;
	apcm_buf_t *rx_buf;
	apcm_buf_t *out_buf;

	bool dbg_rx_insert_sine;

} apcm_btcall_t;


static apcm_btcall_t _btcall = {
	.state = STATE_UNINIT,
	.thread = NULL,

	.enhance = true,
	.out_directly = true,
	.dbg_rx_insert_sine = false,
};



static void btcall_preproc(void)
{
	if (_btcall.isr_cnt < STRMBT_6630_NOISE_CNT) {
		_btcall.out_buf->rptr = _btcall.out_buf->wptr;
	}

	if (_btcall.dbg_rx_insert_sine && (_btcall.isr_cnt % 100) < 2)
	{
		apcm_buf_t tmp_buf;
		apcm_memcpy(&tmp_buf, _btcall.out_buf, sizeof(apcm_buf_t));

		tmp_buf.wptr = _btcall.out_buf->rptr;
		tmp_buf.rptr = (tmp_buf.wptr + (APCM_TBL_SINE_16BIT_SZ << 2)) % tmp_buf.buf_size;
		misc_fill_sine16_tbl(&tmp_buf);
	}
}


static void btcall_data_proc(void)
{
	if (_btcall.state == STATE_STARTED)
	{
		asrc_read(_btcall.rx_asrc, _btcall.out_buf);
		btcall_preproc();
		if (_btcall.out) {
			outhw_write(_btcall.out, _btcall.out_buf);
		}

		txrx_read(_btcall.rx_buf);
		asrc_write(_btcall.rx_asrc, _btcall.rx_buf);

		if (_btcall.enhance == false) {
			virmic_read(_btcall.mic, _btcall.tx_buf);
			btcall_write(_btcall.tx_buf);
		}
	}
	else if (_btcall.state == STATE_TO_STOP)
	{
		PR_D("[data_proc] stop! time(%d) >>>\n", GET_SYS_TIME);
		txrx_stop();

		asrc_stop(_btcall.rx_asrc);
		_btcall.rx_asrc = asrc_close(_btcall.rx_asrc);

		_btcall.mic = virmic_stop(_btcall.mic);
		_btcall.out = outhw_stop(_btcall.out);

		_btcall.tx_buf = buf_close(_btcall.tx_buf);
		_btcall.rx_buf = buf_close(_btcall.rx_buf);
		_btcall.out_buf = buf_close(_btcall.out_buf);

		_btcall.state = STATE_STOPPED;
		PR_D("[data_proc] stop! time(%d) <<<\n", GET_SYS_TIME);
		apcm_up(&_btcall.stop_sema);
	}
}


static s32 btcall_thread(void *data)
{
	apcm_thread_t *thread = _btcall.thread;
	PR_I("[thread(0x%p)] >>>>>>>>>>>>>>>>>>>>> \n", thread);

	while (thread)
	{
		thread_wait(thread, APCM_INFINITE);
		if (thread_should_stop(thread)) {
			break;
		}
		btcall_data_proc();
	}

	PR_I("[thread(0x%p)]: <<<<<<<<<<<<<<<<<<<<< \n", thread);
	return (RET_NOERR);
}


static s32 btcall_isr(u32 param)
{
	_btcall.isr_cnt++;
	thread_wakeup(_btcall.thread);
}


bool btcall_init(void)
{
	if (_btcall.thread == NULL) {
		thread_open(&_btcall.thread, btcall_thread, NULL, "btcall_thread");
		apcm_sema_init(&_btcall.stop_sema, 0);
		PR_I("[inti] thread(0x%p) \n", _btcall.thread);
	}

	return (_btcall.thread != NULL);
}


void btcall_start(void)
{
	u32 fs = txrx_get_fs();
	u32 int_samples = SAMPLES_PER_MSEC(fs)  * STRMBT_INT_TIME;
	PR_D("[start] state(%d) enhance(%d)\n", _btcall.state, _btcall.enhance);

	if (_btcall.state == STATE_UNINIT)
	{
		_btcall.state = STATE_INITED;
		_btcall.isr_cnt = 0;

		txrx_set_loop_mode(false);
		txrx_set_int_cfg(int_samples, (int_samples >> 1), btcall_isr);

		if (_btcall.enhance) {
			_btcall.mic = NULL;
			_btcall.tx_buf = NULL;
		} else {
			_btcall.mic = virmic_start("btcall", fs);
			_btcall.tx_buf = buf_open(NULL, STRM_TMP_BUF_SZ, MONO);
		}

		_btcall.rx_asrc = asrc_open(fs, PLAYBACK_FS);
		asrc_start(_btcall.rx_asrc);

		_btcall.rx_buf = buf_open(NULL, STRM_TMP_BUF_SZ, MONO);
		_btcall.out_buf = buf_open(NULL, STRM_TMP_BUF_SZ, MONO);

		txrx_start();

		outhw_switch_output_mode(APCM_BT_CALL_MODE);
		if (_btcall.out_directly) {
			_btcall.out = outhw_start("btcall");
		}

		_btcall.stop_sema.count = 0;
		_btcall.state = STATE_STARTED;
	}
}


void btcall_stop(void)
{
	if (_btcall.state == STATE_STARTED)
	{
		PR_D("[stop]  time(%d) >>>>\n", GET_SYS_TIME);
		_btcall.state = STATE_TO_STOP;
		thread_wakeup(_btcall.thread);

		if (_btcall.state == STATE_TO_STOP || _btcall.state == STATE_STOPPED) {
			apcm_down(&_btcall.stop_sema);
			PR_D("[stop] wait done, state(%d) time(%d) \n", _btcall.state, GET_SYS_TIME);
		}
		PR_D("[stop] time(%d) state(%d) <<<< \n", GET_SYS_TIME, _btcall.state);
		_btcall.state = STATE_UNINIT;
		outhw_switch_output_mode(APCM_NORMAL_MODE);
	}
}


u32 btcall_write(apcm_buf_t *src_buf)
{
	u32 copy_size = 0;

	if (src_buf)
	{
		if (_btcall.state == STATE_STARTED) {
			txrx_write(src_buf);
		} else  {
			src_buf->rptr = (src_buf->wptr + src_buf->buf_size - BUFFER_SAFE_SIZE) % src_buf->buf_size;
		}

	}

	return (copy_size);
}


u32 btcall_read(apcm_buf_t *dst_buf)
{
	u32 copy_size = 0;

	if (dst_buf)
	{
		if (_btcall.state == STATE_STARTED) {
			buf_copy(dst_buf, _btcall.out_buf);
		} else  {
			dst_buf->wptr = (dst_buf->rptr + dst_buf->buf_size - BUFFER_SAFE_SIZE) % dst_buf->buf_size;
		}

	}

	return (copy_size);
}


void btcall_ul_enhance_enable(bool enable)
{
	PR_I("[bt_enhance] state(%d) %d => %d \n", _btcall.state, _btcall.enhance, enable);
	if (_btcall.state == STATE_UNINIT) {
		_btcall.enhance = enable;
	}
}


void btcall_rx_dbg_enable(u32 flag)
{
	_btcall.dbg_rx_insert_sine = (bool)flag;
	PR_I("[rx_dbg_enable] 0x%x \n", _btcall.dbg_rx_insert_sine);
}



