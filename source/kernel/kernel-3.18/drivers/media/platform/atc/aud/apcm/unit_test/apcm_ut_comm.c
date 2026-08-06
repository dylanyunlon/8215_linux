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
*[File]                     apcm_ut_comm.h
*[Author]                   atc6013
*[Description]
*
******************************************************************************/

#include "apcm_ut_comm.h"

#define LOG_TAG		"[UtComm]"

typedef struct
{
	u32 state;
	apcm_thread_t *thread;

	apcm_buf_t *inbuf;
	apcm_buf_t *outbuf;

	char tmp_data1[960];
	char tmp_data2[960];

	apcm_file_t *infile;
	apcm_file_t *outfile;
} ut_comm_t;

static ut_comm_t _utcomm = {
	.state = STATE_UNINIT,
};

static void utcomm_uninit(void);


static void utcomm_process(void)
{
	if (_utcomm.state == STATE_STARTED)
	{
		file_read(_utcomm.infile, _utcomm.inbuf);
		buf_copy(_utcomm.outbuf, _utcomm.inbuf);
		file_write(_utcomm.outfile, _utcomm.outbuf);
	}
}


static s32 utcomm_thread(void *data)
{
	u64 timeout, time = 20;
	u32 cnt = 0;
	apcm_thread_t *thread = _utcomm.thread;
	PR_I("[thread] (0x%p) >>>>>>>>>>>>>>>>>>>>>\r\n", thread);

	while (true)
	{
		timeout = thread_wait(thread, time);
		if (++cnt % 1000 == 1) {
			PR_D("[thread] cnt(%d) \n", cnt);
		}
		if (thread_should_stop(thread)) {
			break;
		}

		time = 20;
		utcomm_process();
	}
	utcomm_uninit();

	PR_I("[thread] (0x%p) <<<<<<<<<<<<<<< \r\n", thread);
	return 0;
}


static void utcomm_init(void)
{
	thread_open(&_utcomm.thread, utcomm_thread, NULL, "utcomm_test");
	_utcomm.inbuf = buf_open(NULL, 0x1000, 2);
	_utcomm.outbuf = buf_open(NULL, 0x1000, 2);

	_utcomm.infile = file_open_r("in.wav");
	_utcomm.outfile = file_open_w("out", GET_FILE_IDX, 2, 48000);

	if (_utcomm.thread && _utcomm.inbuf && _utcomm.outbuf && _utcomm.infile && _utcomm.outfile)
	{
		PR_I("[init] state(%d) (0x%p, 0x%p, 0x%p, 0x%p, 0x%p) \r\n",
			_utcomm.state, _utcomm.thread, _utcomm.inbuf, _utcomm.outbuf,
			_utcomm.infile, _utcomm.outfile);
		_utcomm.state = STATE_STARTED;
		thread_wakeup(_utcomm.thread);
	}
}


static void utcomm_uninit(void)
{
	PR_I("[uninit] state(%d) (0x%p, 0x%p, 0x%p, 0x%p, 0x%p) ...\r\n",
		_utcomm.state, _utcomm.thread, _utcomm.inbuf, _utcomm.outbuf, _utcomm.infile, _utcomm.outfile);

	if (_utcomm.state != STATE_UNINIT)
	{
		buf_close(_utcomm.inbuf);
		buf_close(_utcomm.outbuf);
		file_close(_utcomm.infile);
		file_close(_utcomm.outfile);
		thread_close(_utcomm.thread);

		_utcomm.thread = NULL;
		_utcomm.inbuf = NULL;
		_utcomm.outbuf = NULL;
		_utcomm.infile = NULL;
		_utcomm.outfile = NULL;
		_utcomm.state = STATE_UNINIT;
	}
}


void utcomm_test(u32 flag)
{
	log_show_version(0);
	log_show_version(1);
	PR_I("[Test] state(%d) flag(%d)\n", _utcomm.state, flag);

	if (_utcomm.state == STATE_UNINIT) {
		_utcomm.state = STATE_INITED;
		utcomm_init();
	} else if (_utcomm.state == STATE_STARTED) {
		_utcomm.state = STATE_STOPPED;
		thread_stop(_utcomm.thread);
	}
}


