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
*[File]                apcm_speech.cpp
*[Author]              atc6013
*[Description]
*
******************************************************************************/

#include "apcm_speech.h"
#include "GpsMix_mw.h"

#define LOG_TAG  "[speech]"

typedef struct
{
	bool enable;
	bool ref_enable;
	bool mic_enable;

	virmic_t *mic;
	apcm_buf_t *ref_buf;

	bool awb_enable;
	bool mtk_lib;

	u32 ref_delay;
	u32 mic_delay;

} apcm_speech_t;


static apcm_speech_t _speech = {
	.enable = false,
	.ref_enable = false,
	.mic_enable = false,

	.mic = NULL,
	.ref_buf = NULL,

	.awb_enable = false,
	.mtk_lib = true,
};


static apcm_buf_t *speech_open_ref_buf(void)
{
	apcm_buf_t *ref_buf = NULL;

	if (_speech.awb_enable) {
		ref_buf = buf_open(DspAwbGetSadr(), DspAwbGetChSize(), MONO);
	} else {
		ref_buf = buf_open(DspGpsMixGetSadr(), DspGpsMixGetChSize(), MONO);
	}

	if (ref_buf) {
		if (_speech.awb_enable) {
			ref_buf->wptr = DspAwbGetWptr();
		} else {
			ref_buf->wptr = DspGpsMixGetRptr();
		}
		ref_buf->rptr = ref_buf->wptr;
		PR_D("[open_ref_buf] sadr(0x%p) ch_size(%d) rptr(%d) \n", ref_buf->addr[0], ref_buf->buf_size, ref_buf->rptr);
	}

	return (ref_buf);
}


static void speech_init_delay_size(void)
{
	if (_speech.mtk_lib)
	{
		if (_speech.awb_enable) {
			_speech.ref_delay = 0;
			_speech.mic_delay = 1100;
		} else {
			_speech.ref_delay = 72;
			_speech.mic_delay = 0;
		}
	}
	else
	{
		_speech.ref_delay = 0;
		_speech.mic_delay = 0;
	}
	PR_D("[init_delay_size] ref(%d) mic(%d), awb(%d) mtk(%d) \n",
		_speech.ref_delay, _speech.mic_delay, _speech.awb_enable, _speech.mtk_lib);
}


void speech_start(u32 strm_type)
{
	if (strm_type == SPEECH_REF_STRM)
	{
		_speech.ref_enable = true;
	}
	else if (strm_type == SPEECH_MIC_STRM)
	{
		_speech.mic_enable = true;
	}

	if ((_speech.enable == false) && _speech.ref_enable && _speech.mic_enable)
	{
		_speech.mic = virmic_start("sph_mic", RECORD_FS);
		_speech.ref_buf = speech_open_ref_buf();

		speech_init_delay_size();
		outhw_switch_output_mode(APCM_BT_CALL_MODE);
		_speech.enable = true;

		PR_D("[start]  time(%d) !\n", GET_SYS_TIME);
	}
}


void speech_stop(u32 strm_type)
{
	if (strm_type == SPEECH_REF_STRM)
	{
		_speech.ref_enable = false;
	}
	else if (strm_type == SPEECH_MIC_STRM)
	{
		_speech.mic_enable = false;
	}

	if (_speech.enable && !_speech.ref_enable && !_speech.mic_enable)
	{
		_speech.enable = false;

		_speech.ref_buf = buf_close(_speech.ref_buf);
		_speech.mic = virmic_stop(_speech.mic);

		outhw_switch_output_mode(APCM_NORMAL_MODE);
		PR_D("[stop] time(%d) \n", GET_SYS_TIME);
	}
}


u32 speech_ref_read(apcm_buf_t *dst_buf)
{
	u32 copy_size = 0;

	if (_speech.enable)
	{
		if (_speech.ref_delay && dst_buf) {
			dst_buf->wptr = (dst_buf->wptr + _speech.ref_delay) % dst_buf->buf_size;
			_speech.ref_delay = 0;
		}
		if (_speech.ref_buf) {
			u32 gain = outhw_get_ref_boost_gain();
			_speech.ref_buf->wptr = _speech.awb_enable ? DspAwbGetWptr() : DspGpsMixGetRptr();
			buf_set_write_cb(dst_buf, buf_trans_data_with_gain, &gain);
			copy_size = buf_copy(dst_buf, _speech.ref_buf);
		}
	}
	else if (_speech.ref_enable)
	{
		//nothing to do, wait speech_mic enable
	}
	else if (dst_buf)
	{
		dst_buf->wptr = (dst_buf->rptr + (dst_buf->buf_size >> 1)) % dst_buf->buf_size;
	}

	return (copy_size);
}


u32 speech_mic_read(apcm_buf_t *dst_buf)
{
	u32 copy_size = 0;

	if (_speech.enable)
	{
		if (_speech.mic_delay && dst_buf) {
			dst_buf->wptr = (dst_buf->wptr + _speech.mic_delay) % dst_buf->buf_size;
			_speech.mic_delay = 0;
		}
		copy_size = virmic_read(_speech.mic, dst_buf);
	}
	else if (_speech.mic_enable)
	{
		//nothing to do, wait speech_ref enable
	}
	else if (dst_buf)
	{
		dst_buf->wptr = (dst_buf->rptr + (dst_buf->buf_size >> 1)) % dst_buf->buf_size;
	}

	return (copy_size);
}


void speech_awb_enable(bool enable)
{
	PR_D("[awb_enable] %d => %d \n", _speech.awb_enable, enable);
	_speech.awb_enable = enable;
}



