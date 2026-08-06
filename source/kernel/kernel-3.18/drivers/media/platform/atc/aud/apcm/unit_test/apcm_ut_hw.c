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
*[File]                     apcm_ut_hw.h
*[Author]                   atc6013
*[Description]
*
******************************************************************************/

#include "apcm_ut_hw.h"
#include "apcm_speech.h"

#define LOG_TAG		"[UtHw]"

#define UT_HW_BUF_SIZE		4096

//===================================================//

typedef struct
{
	u32 state;
	virout_t *out;
	apcm_buf_t *buf;
	apcm_thread_t *thread;
} uthw_out_t;

static uthw_out_t _utout = {
	.state = STATE_UNINIT,
};


static void uthw_out_stop(void)
{
	PR_I("[out_stop] state(%d) >>>> \n", _utout.state);
	outhw_stop(_utout.out);

	_utout.buf = buf_close(_utout.buf);
	_utout.thread = thread_close(_utout.thread);
	_utout.state = STATE_UNINIT;
	PR_I("[out_stop] state(%d) <<<< \n", _utout.state);
}


static s32 uthw_out_thread(void *data)
{
	u64 code = 0, time = 20;
	apcm_thread_t *thread = _utout.thread;
	PR_I("[out_thread] (0x%p) >>>>>>>>>>>>>>>>>>>>> \n", thread);

	while (thread)
	{
		code = thread_wait(thread, time);
		if (thread_should_stop(thread)) {
			break;
		}

		if (_utout.state == STATE_STARTED) {
			misc_fill_sine16_tbl(_utout.buf);
			outhw_write(_utout.out, _utout.buf);
		}
	}
	uthw_out_stop();

	PR_I("[out_thread] (0x%p) <<<<<<<<<<<<<<<  \n", thread);
	return 0;
}



static void uthw_out_start(void)
{
	//outhw_init();
	_utout.buf = buf_open(NULL, UT_HW_BUF_SIZE, 2);
	_utout.out = outhw_start("ut_out");
	thread_open(&_utout.thread, uthw_out_thread, NULL, "uthw_out_test");
	_utout.state = STATE_STARTED;
	PR_I("[out_start] state(%d)  << \n", _utout.state);
}


void uthw_out_test(void)
{
	PR_I("[out_test] state(%d) \n", _utout.state);
	if (_utout.state == STATE_UNINIT) {
		_utout.state = STATE_INITED;
		uthw_out_start();
	} else if (_utout.state == STATE_STARTED) {
		_utout.state = STATE_STOPPED;
		thread_stop(_utout.thread);
	}
}

//=====================================================//

typedef struct
{
	u32 state;
	u32 rptr;

	apcm_buf_t *buf;
	apcm_file_t *file;
	apcm_thread_t *thread;

} uthw_mic_t;

static uthw_mic_t _utmic = {
	.state = STATE_UNINIT,
};


static void uthw_mic_stop(void)
{
	PR_I("[mic_stop] state(%d) >>>> \n", _utmic.state);
	mic_stop();

	_utmic.buf = buf_close(_utmic.buf);
	_utmic.file = file_close(_utmic.file);
	_utmic.thread = thread_close(_utmic.thread);

	_utmic.state = STATE_UNINIT;
	PR_I("[mic_stop] state(%d) <<<< \n", _utmic.state);
}


static s32 uthw_mic_thread(void *data)
{
	u64 code = 0, time = 20;
	apcm_thread_t *thread = _utmic.thread;
	PR_I("[mic_thread] (0x%p) >>>>>>>>>>>>>>>>>>>>> \n", thread);

	while (thread)
	{
		code = thread_wait(thread, time);
		if (thread_should_stop(thread)) {
			break;
		}

		if (_utmic.state == STATE_STARTED) {
			mic_read(_utmic.buf, &_utmic.rptr);
			file_write(_utmic.file, _utmic.buf);
		}
	}
	uthw_mic_stop();

	PR_I("[mic_thread] (0x%p) <<<<<<<<<<<<<<<  \n", thread);
	return 0;
}


static void uthw_mic_start(u32 fs)
{
	//mic_init();

	_utmic.buf = buf_open(NULL, UT_HW_BUF_SIZE, 2);

	_utmic.file = file_open_w("utmic", GET_FILE_IDX, 2, fs);
	thread_open(&_utmic.thread, uthw_mic_thread, NULL, "uthw_mic_test");

	_utmic.rptr = mic_start();
	_utmic.state = STATE_STARTED;
	PR_I("[mic_start] state(%d) << \n", _utmic.state);
}


void uthw_mic_test(u32 fs)
{
	PR_I("[mic_test] state(%d) fs(%d)\n", _utmic.state, fs);
	if (_utmic.state == STATE_UNINIT) {
		_utmic.state = STATE_INITED;
		uthw_mic_start(fs);
	} else if (_utmic.state == STATE_STARTED) {
		_utmic.state = STATE_STOPPED;
		thread_stop(_utmic.thread);
	}
}

//=====================================================//

typedef struct
{
	u32 state;
	u32 fs;

	apcm_buf_t *txbuf;
	apcm_buf_t *rxbuf;
	apcm_file_t *file;

	u32 isr_cnt;

} uthw_txrx_t;

static uthw_txrx_t _uttxrx = {
	.state = STATE_UNINIT,
};

static void uthw_txrx_stop(void)
{
	PR_I("[txrx_stop] state(%d)  >>> \n", _uttxrx.state);
	txrx_stop();

	_uttxrx.txbuf = buf_close(_uttxrx.txbuf);
	_uttxrx.rxbuf = buf_close(_uttxrx.rxbuf);
	_uttxrx.file = file_close(_uttxrx.file);

	_uttxrx.state = STATE_UNINIT;
	PR_I("[txrx_stop] state(%d) <<<< \n", _uttxrx.state);
}


static s32 uthw_txrx_isr(u32 u4Param)
{
	if (_uttxrx.state == STATE_STARTED) {
		_uttxrx.isr_cnt++;
		txrx_read(_uttxrx.rxbuf);
		file_write(_uttxrx.file, _uttxrx.rxbuf);

		misc_fill_sine16_tbl(_uttxrx.txbuf);
		txrx_write(_uttxrx.txbuf);

		if ((_uttxrx.isr_cnt % 1000) < 3) {
			PR_D("[txrx_isr] cnt(%d) \n", _uttxrx.isr_cnt);
		}
	}

	return 0;
}


static void uthw_txrx_start(u32 fs)
{
	PR_I("[txrx_start] state(%d)\n", _uttxrx.state);
	//txrx_init();

	txrx_set_fs(fs);
	txrx_set_loop_mode(true);
	txrx_set_int_cfg(160, 80, uthw_txrx_isr);

	_uttxrx.isr_cnt = 0;
	_uttxrx.rxbuf = buf_open(NULL, UT_HW_BUF_SIZE, 1);
	_uttxrx.txbuf = buf_open(NULL, UT_HW_BUF_SIZE, 1);
	_uttxrx.file = file_open_w("uttxrx", GET_FILE_IDX, 1, fs);

	txrx_start();
	_uttxrx.state = STATE_STARTED;
}


void uthw_txrx_test(u32 fs)
{
	PR_I("[txrx_test] state(%d) fs(%d)\n", _uttxrx.state, fs);

	if (_uttxrx.state == STATE_UNINIT) {
		_uttxrx.state = STATE_INITED;
		uthw_txrx_start(fs);
	} else if (_uttxrx.state == STATE_STARTED) {
		_uttxrx.state = STATE_STOPPED;
		uthw_txrx_stop();
	}
}


//=====================================================//

typedef struct
{
	u32 state;
	asrc_chs_t *asrc;
	asrc_cfg_t cfg;

	apcm_buf_t *ibuf;
	apcm_buf_t *obuf;

	apcm_file_t *file;
	apcm_thread_t *thread;
} uthw_asrc_t;


static uthw_asrc_t _utasrc[ASRC_CHSET_NUMBER];


static void uthw_asrc_stop(u32 idx)
{
	PR_I("[asrc_stop] state(%d)  >>> \n", _utasrc[idx].state);
	asrc_stop(_utasrc[idx].asrc);
	_utasrc[idx].asrc = asrc_close(_utasrc[idx].asrc);

	_utasrc[idx].ibuf = buf_close(_utasrc[idx].ibuf);
	_utasrc[idx].obuf = buf_close(_utasrc[idx].obuf);
	_utasrc[idx].file = file_close(_utasrc[idx].file);
	_utasrc[idx].thread = thread_close(_utasrc[idx].thread);

	_utasrc[idx].state = STATE_UNINIT;
	PR_I("[asrc_stop] state(%d) <<<< \n", _utasrc[idx].state);
}


static s32 uthw_asrc_thread(void *data)
{
	u64 code = 0, time = 20;
	u32 idx = (u32)data;
	apcm_thread_t *thread = _utasrc[idx].thread;
	PR_I("[asrc_thread] (0x%p) >>>>>>>>>>>>>>>>>>>>> idx(%d)\n", thread, idx);

	while (true)
	{
		code = thread_wait(thread, time);
		if (thread_should_stop(thread)) {
			break;
		}

		if (_utasrc[idx].state == STATE_STARTED) {
			asrc_read(_utasrc[idx].asrc, _utasrc[idx].obuf);
			file_write(_utasrc[idx].file, _utasrc[idx].obuf);

			misc_fill_sine16_tbl(_utasrc[idx].ibuf);
			asrc_write(_utasrc[idx].asrc, _utasrc[idx].ibuf);
		}
	}
	uthw_asrc_stop(idx);

	PR_I("[asrc_thread] (0x%p) <<<<<<<<<<<<<<<  idx(%d)\n", thread, idx);
	return 0;
}


static void uthw_asrc_start(u32 idx, u32 ifs, u32 ofs)
{
	PR_I("[asrc_start] state(%d)   \n", _utasrc[idx].state);

	//asrc_init();
	thread_open(&_utasrc[idx].thread, uthw_asrc_thread, idx, "uthw_asrc_test");

	_utasrc[idx].cfg.ibw = 16;
	_utasrc[idx].cfg.obw = 16;
	_utasrc[idx].cfg.ifs = ifs;
	_utasrc[idx].cfg.ofs = ofs;
	_utasrc[idx].cfg.intr_type = 0;
	_utasrc[idx].cfg.pfn_cb = NULL;
	_utasrc[idx].asrc = asrc_open_special(idx, &_utasrc[idx].cfg);

	_utasrc[idx].ibuf = buf_open(NULL, UT_HW_BUF_SIZE, 2);
	_utasrc[idx].obuf = buf_open(NULL, UT_HW_BUF_SIZE, 2);
	_utasrc[idx].file = file_open_w("utasrc", GET_FILE_IDX, 2, 48000);

	asrc_start(_utasrc[idx].asrc);
	_utasrc[idx].state = STATE_STARTED;
}


void uthw_asrc_test(u32 idx, u32 ifs, u32 ofs)
{
	if (idx < ASRC_CHSET_NUMBER)
	{
		PR_I("[asrc_test] state(%d) idx(%d) fs(%d => %d)\n",
			_utasrc[idx].state, idx, ifs, ofs);

		if (_utasrc[idx].state == STATE_UNINIT) {
			_utasrc[idx].state = STATE_INITED;
			uthw_asrc_start(idx, ifs, ofs);
		} else if (_utasrc[idx].state == STATE_STARTED) {
			_utasrc[idx].state = STATE_STOPPED;
			thread_stop(_utasrc[idx].thread);
		}
	}
}


///=====================================================//

typedef struct
{
	u32 state;
	bool use_asrc;

	asrc_chs_t *asrc;
	u32 mic_rptr;
	virout_t *out;

	apcm_buf_t *mic_buf;
	apcm_buf_t *out_buf;

	apcm_thread_t *thread;
} uthw_mao_t;


static uthw_mao_t _utmao;


static void uthw_mao_stop(void)
{
	PR_I("[mic_asrc_out_stop] state(%d)  >>> \n", _utmao.state);
	asrc_stop(_utmao.asrc);
	_utmao.asrc = asrc_close(_utmao.asrc);
	mic_stop();
	_utmao.out = outhw_stop(_utmao.out);

	_utmao.mic_buf = buf_close(_utmao.mic_buf);
	_utmao.out_buf = buf_close(_utmao.out_buf);

	_utmao.thread = thread_close(_utmao.thread);

	_utmao.state = STATE_UNINIT;
	PR_I("[mic_asrc_out_stop] state(%d) <<<< \n", _utmao.state);
}


static s32 uthw_mao_thread(void *data)
{
	u64 code = 0, time = 20;
	apcm_thread_t *thread = _utmao.thread;
	PR_I("[mic_asrc_out_thread] (0x%p) >>>>>>>>>>>>>>>>>>>>>\n", thread);

	while (true)
	{
		code = thread_wait(thread, time);
		if (thread_should_stop(thread)) {
			break;
		}

		if (_utmao.state == STATE_STARTED)
		{
			if (_utmao.asrc) {
				asrc_read(_utmao.asrc, _utmao.out_buf);
				outhw_write(_utmao.out, _utmao.out_buf);
				mic_read(_utmao.mic_buf, &_utmao.mic_rptr);
				asrc_write(_utmao.asrc, _utmao.mic_buf);
			} else {
				mic_read(_utmao.mic_buf, &_utmao.mic_rptr);
				outhw_write(_utmao.out, _utmao.mic_buf);
			}
		}
	}
	uthw_mao_stop();

	PR_I("[mic_asrc_out_thread] (0x%p) <<<<<<<<<<<<<<< \n", thread);
	return 0;
}


static void uthw_mao_start(void)
{
	u32 mic_fs = mic_get_fs();
	PR_I("[mic_asrc_out_start] state(%d) use_asrc(%d)  \n", _utmao.state, _utmao.use_asrc);

	thread_open(&_utmao.thread, uthw_mao_thread, NULL, "uthw_mao_test");

	_utmao.mic_buf = buf_open(NULL, UT_HW_BUF_SIZE, 2);
	_utmao.out_buf = buf_open(NULL, UT_HW_BUF_SIZE, 2);

	_utmao.asrc = NULL;
	if (_utmao.use_asrc) {
		_utmao.asrc = asrc_open(mic_fs, PLAYBACK_FS);
		asrc_start(_utmao.asrc);
	}
	_utmao.mic_rptr = mic_start();
	_utmao.out = outhw_start("ut_mao");

	_utmao.state = STATE_STARTED;
}


void uthw_mao_test(bool use_asrc)
{
	PR_I("[mic_asrc_out_test] state(%d) use_asrc(%d)\n", _utmao.state, use_asrc);

	if (_utmao.state == STATE_UNINIT) {
		_utmao.state = STATE_INITED;
		_utmao.use_asrc = use_asrc;
		uthw_mao_start();
	} else if (_utmao.state == STATE_STARTED) {
		_utmao.state = STATE_STOPPED;
		thread_stop(_utmao.thread);
	}
}

//=================================================


typedef struct
{
	u32 state;

	apcm_buf_t *ref_buf;
	apcm_buf_t *mic_buf;

	apcm_thread_t *thread;

	apcm_file_t *ref_file;
	apcm_file_t *mic_file;

} uthw_sph_t;


static uthw_sph_t _utsph = {
	.state = STATE_UNINIT,
};


static void uthw_sph_stop(void)
{
	PR_I("[sph_stop] state(%d)  >>> \n", _utsph.state);

	_utsph.ref_buf = buf_close(_utsph.ref_buf);
	_utsph.mic_buf = buf_close(_utsph.mic_buf);

	_utsph.ref_file = file_close(_utsph.ref_file);
	_utsph.mic_file = file_close(_utsph.mic_file);

	_utsph.thread = thread_close(_utsph.thread);

	speech_stop(SPEECH_REF_STRM);
	speech_stop(SPEECH_MIC_STRM);

	_utsph.state = STATE_UNINIT;

	PR_I("[sph_stop] state(%d) <<<< \n", _utsph.state);
}


static s32 uthw_sph_thread(void *data)
{
	u64 code = 0, time = 10;
	apcm_thread_t *thread = _utsph.thread;
	u32 cnt = 0;
	PR_I("[sph_thread] (0x%p) >>>>>>>>>>>>>>>>>>>>>\n", thread);

	while (true)
	{
		code = thread_wait(thread, time);
		if (thread_should_stop(thread)) {
			break;
		}

		if (_utsph.state == STATE_STARTED)
		{
			speech_ref_read(_utsph.ref_buf);
			speech_mic_read(_utsph.mic_buf);

			file_write(_utsph.ref_file, _utsph.ref_buf);
			file_write(_utsph.mic_file, _utsph.mic_buf);
		}
	}
	uthw_sph_stop();

	PR_I("[sph_thread] (0x%p) <<<<<<<<<<<<<<< \n", thread);
	return 0;
}


static void uthw_sph_start(void)
{
	u32 file_idx = GET_FILE_IDX;
	PR_I("[sph_start] state(%d) \n", _utsph.state);

	_utsph.ref_buf = buf_open(NULL, 8192, 1);
	_utsph.mic_buf = buf_open(NULL, 8192, 2);

	_utsph.ref_file = file_open_w("ut_sph_ref", file_idx, 1, 48000);
	_utsph.mic_file = file_open_w("ut_sph_mic", file_idx, 2, 48000);

	thread_open(&_utsph.thread, uthw_sph_thread, NULL, "uthw_sph_test");

	speech_start(SPEECH_REF_STRM);
	speech_start(SPEECH_MIC_STRM);
	_utsph.state = STATE_STARTED;

	PR_I("[sph_start] state(%d) \n", _utsph.state);
}


void uthw_sph_test(void)
{
	PR_I("[sph_test] state(%d) \n", _utsph.state);

	if (_utsph.state == STATE_UNINIT) {
		_utsph.state = STATE_INITED;
		uthw_sph_start();
	} else if (_utsph.state == STATE_STARTED) {
		_utsph.state = STATE_STOPPED;
		thread_stop(_utsph.thread);
	}
}

