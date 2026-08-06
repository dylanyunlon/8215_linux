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
*[File]             apcm_mic.c
*[Author]
*[Description]
*
*[Copyright]
*    Copyright (C) 2008 AutoChips Incorporation. All Rights Reserved.
******************************************************************************/

#include "apcm_mic_reg.h"

#define LOG_TAG			"[Mic]"

#define MIC_BUF_SIZE		(8000 * 2 * 2) // 1000ms 8KHz, 16 bit stereo data

#define MIC_0			0
#define MIC_1			1

#define MIC_MAX_HW_GAIN		63

static DEFINE_SPINLOCK(mic_lock);


typedef struct
{
	AUD_MIC_SRC src;
	AUD_PINMUX_I2SMICIN i2s_pin;

	u32 fs;
	u32 src_bit_num;
	u32 out_bit_num;
	AUDFMT_INTF_E fmt_intf;

	MCLK_TYPE_T mclk_type;
	AUD_LRCK_CYC_T cycle;
	AUD_MIC_CLK_SRC clk_src;
	bool bck_invert;
	bool lrck_invert;

	ADC_EXTPARAMS_T adc_cfg;
	PADC_HAL_CLS_PUB adc;
	AUD_ADC_ID adc_id;

}mic_cfg_t;


typedef struct
{
	u32 state;
	u32 start_cnt;
	u32 primary_idx;

	mic_cfg_t cfg;

	apcm_buf_t *buf;
	u32 phy_addr;

} apcm_mic_t;


static apcm_mic_t _mic = {
	.state = STATE_UNINIT,
	.start_cnt = 0,
};


static void mic_reg_cfg(void)
{
	mic_cfg_t *config = &_mic.cfg;
	PR_D("[reg_config] \n");

	REG_MIC_SET_ARM_CTRL(true);
	REG_MIC_SET_BUF_INFO(_mic.phy_addr, _mic.buf->buf_size);

	REG_MIC_SET_SRC(config->src);
	if (config->src == EXT_MICIN) {
		IoPinMux_SetI2sMicIn(config->i2s_pin);
	}

	REG_MIC_SET_CLK_SRC(config->clk_src);
	if (MIC_CLK_MPH == config->clk_src) {
		IoClk_SetMphMclk(config->mclk_type, misc_get_fs_idx(config->fs));
	};
	REG_MIC_SET_DIVIDER(config->mclk_type, config->cycle);
	REG_MIC_SET_INVERT(config->bck_invert, config->lrck_invert);
	REG_MIC_SET_DATA_FMT(config->src_bit_num, config->out_bit_num, config->fmt_intf);

}


static void mic_init_memory(void)
{
	AUD_MIC_BUF_FOR_BT_INFO aud_mic_buf;
	u32 buf_size;
	void *addr;

	_AudGetMicBufInfo(&aud_mic_buf);
	if (aud_mic_buf.u4MicBufBTSize > MIC_BUF_SIZE) {
		aud_mic_buf.u4MicBufBTSize = MIC_BUF_SIZE;
	}

	_mic.phy_addr = aud_mic_buf.u4TotalPhy;
	buf_size = aud_mic_buf.u4MicBufBTSize >> 1;
	addr = (void *)(aud_mic_buf.u4WorkBufferSA + aud_mic_buf.u4MicPageSA + aud_mic_buf.u4MicBufBTSA);
	_mic.buf = buf_open(addr, buf_size, STEREO);

	PR_I("[init_memory]Phy(0x%x) Vir(0x%p, 0x%p) Sz(%d) Chn(%d) \n",
		_mic.phy_addr, _mic.buf->addr[0], _mic.buf->addr[1], _mic.buf->buf_size, _mic.buf->channels);
}


static bool mic_alloc_adc(void)
{
	bool result = true;

	_mic.cfg.adc = AdcHal_New(&_mic.cfg.adc_cfg);
	if (NULL == _mic.cfg.adc) {
		PR_E("[alloc_adc] No free adc for mic in use \n");
		result = false;
	} else {
		_mic.cfg.adc_id = _mic.cfg.adc->eAdcId;
		_mic.cfg.adc->rHwIf.Start(_mic.cfg.adc, 0);
		REG_MIC_SET_AFE_DATA(_mic.cfg.adc_id);
		PR_D("[alloc_adc] adc_id(%d, 0x%x) \n", _mic.cfg.adc_id, _mic.cfg.adc);
	}

	return (result);
}


u32 mic_init(void)
{
	u32 result = RET_NOERR;

	if (_mic.state == STATE_UNINIT)
	{
		mic_cfg_t *config = &_mic.cfg;
		ADC_EXTPARAMS_T *padc_cfg = &_mic.cfg.adc_cfg;

		_mic.start_cnt = 0;
		_mic.primary_idx = MIC_0;

		config->src = INT_MICIN;
		config->i2s_pin = PINMUX_I2SMICIN_DEFAULT;

		config->fs = RECORD_FS;
		config->src_bit_num = APCM_DEF_DATA_BITS;
		config->out_bit_num = APCM_DEF_DATA_BITS;
		config->fmt_intf = AUDFMT_IIS;

		config->mclk_type = AUD_MCLK_256FS;
		config->cycle = AUD_LRCK_CYC_32;
		config->clk_src = MIC_CLK_MPH;
		config->bck_invert = true;
		config->lrck_invert = false;

		padc_cfg->eFs = misc_get_fs_idx(config->fs);
		padc_cfg->eInput = ADC_SRC_MICIN;
		padc_cfg->eClkSrc = AFE_CLK_MPH;
		padc_cfg->eLinGain = LSBUFGAIN_MINUS_3DB;
		padc_cfg->u4MicGain = MIC_HW_GAIN;
		config->adc_id = AUD_ADC_NON;
		config->adc = NULL;
		if (EXTERNAL_MIC_EN) {
			config->src = EXT_MICIN;
			config->src_bit_num = EXTERNAL_MIC_SRC_BIT_NUM;
			config->fs = EXTERNAL_MIC_FS;
			config->i2s_pin = EXTERNAL_MIC_I2S_PIN;
		}

		mic_init_memory();
		mic_reg_cfg();
		_mic.state = STATE_INITED;
	}

	return (result);
}


u32 mic_uninit(void)
{
	PR_I("[uninit] \n");
	IoPinMux_SetI2sMicIn(PINMUX_I2SMICIN_DEFAULT);

	return (RET_NOERR);
}


void mic_hibernation(bool wakeup)
{
	if (wakeup)  {
		mic_reg_cfg();
	} else {

	}
}


u32 mic_start(void)
{
	u32 wptr = 0;
	bool first_start = false;
	PR_I("[start] state(%d) start_cnt(%d) Time(%d). >>>\n", _mic.state, _mic.start_cnt, GET_SYS_TIME);

	spin_lock(&mic_lock);
	if (!_mic.start_cnt) {
		_mic.start_cnt = 1;
		first_start = true;
	} else {
		_mic.start_cnt++;
		wptr = mic_get_wptr();
		first_start = false;
	}
	spin_unlock(&mic_lock);

	if (first_start) {
		bool enable = true;
		if (INT_MICIN == _mic.cfg.src) {
			enable = mic_alloc_adc();
		}
	#ifdef AUD_IO_POWER_CONTROL
		IoClk_SetModulePowerOn(CLKPM_MPHONE);
	#endif

		if (enable) {
			_mic.state = STATE_STARTED;
			buf_reset(_mic.buf);
			REG_MIC_ENABLE(enable);
			//mic_dump_regs();
		} else {
			_mic.state = STATE_STOPPED;
			_mic.start_cnt--;
			PR_E("[start] alloc adc fail! \n");
		}
	}


	PR_D("[start] state(%d) start_cnt(%d) Time(%d). <<< \n",
		_mic.state, _mic.start_cnt, GET_SYS_TIME);

	return (wptr);
}


u32 mic_stop(void)
{
	PR_I("[stop] state(%d) start_cnt(%d) time(%d).\n", _mic.state, _mic.start_cnt, GET_SYS_TIME);
	bool last_stop = false;

	spin_lock(&mic_lock);
	if (_mic.start_cnt) {
		_mic.start_cnt--;
		if (_mic.start_cnt == 0) {
			last_stop = true;
		}
	}
	spin_unlock(&mic_lock);

	if (last_stop && _mic.state == STATE_STARTED)
	{
		REG_MIC_ENABLE(false);

	#ifdef AUD_IO_POWER_CONTROL
		IoClk_SetModulePowerDown(CLKPM_MPHONE);
	#endif

		if ((INT_MICIN == _mic.cfg.src) && (_mic.cfg.adc)) {
			_mic.cfg.adc->rHwIf.Stop(_mic.cfg.adc, 0);
			_mic.cfg.adc->Delete(_mic.cfg.adc);
			_mic.cfg.adc = NULL;
		}
		_mic.state = STATE_STOPPED;
	}

	PR_D("[stop] state(%d) start_cnt(%d) time(%d)<<<.\n", _mic.state, _mic.start_cnt, GET_SYS_TIME);

	return (RET_NOERR);
}


u32 mic_get_wptr(void)
{
	u32 wptr = 0;
	if (_mic.state == STATE_STARTED) {
		// dword to byte need to left shift 2 bits
		wptr = (REG_MIC_GET_WP() << 2) - (REG_MIC_GET_SADR() << 2);
	}

	return (wptr);
}


u32 mic_read(apcm_buf_t *dst_buf, u32 *rptr)
{
	u32 copy_size = 0;
	if (_mic.state == STATE_STARTED) {
		apcm_buf_t tmp_buf;
		apcm_memcpy(&tmp_buf, _mic.buf, sizeof(apcm_buf_t));

		if (MIC_1 == _mic.primary_idx) {
			tmp_buf.addr[0] = _mic.buf->addr[1];
			tmp_buf.addr[1] = _mic.buf->addr[0];
		}

		tmp_buf.rptr = *rptr;
		tmp_buf.wptr = mic_get_wptr();
		copy_size = buf_copy(dst_buf, &tmp_buf);
		*rptr = tmp_buf.rptr;
	}
	return (copy_size);
}


u32 mic_set_fs(u32 fs)
{
	if (_mic.state != STATE_STARTED)
	{
		PR_D("[set_fs] %d => %d \n", _mic.cfg.fs, fs);
		if (fs < SAMPLE_RATE_8K || fs > SAMPLE_RATE_48K) {
			PR_E("[set_fs] fs(%d) is err, change to 48000 \n", fs);
			fs = SAMPLE_RATE_48K;
		}

		if (_mic.cfg.fs != fs && MIC_CLK_MPH == _mic.cfg.clk_src) {
			IoClk_SetMphMclk(_mic.cfg.mclk_type, misc_get_fs_idx(fs));
		}
		_mic.cfg.fs = fs;
		_mic.cfg.adc_cfg.eFs = misc_get_fs_idx(fs);
	}
	return (_mic.cfg.fs);
}


u32 mic_get_fs(void)
{
	return (_mic.cfg.fs);
}


bool mic_set_source(AUD_MIC_SRC src, AUD_PINMUX_I2SMICIN i2s_pin)
{
	bool result = false;
	PR_D("[set_source] state(%d) src(%d), i2s_pin(%d)", _mic.state, src, i2s_pin);

	if (_mic.state !=  STATE_STARTED && src < MIC_SRC_MAX)
	{
		_mic.cfg.src = src;
		if (_mic.cfg.src == EXT_MICIN && i2s_pin < PINMUX_I2SMICIN_GROUP_MAX) {
			_mic.cfg.i2s_pin = i2s_pin;
			IoPinMux_SetI2sMicIn(_mic.cfg.i2s_pin);
			result = true;
		} else if (_mic.cfg.src == INT_MICIN) {
			result = true;
		}
	}

	if (result) {
		REG_MIC_SET_SRC(_mic.cfg.src);
	} else {
		PR_E("[set_source] Fail!!! \n");
	}

	return (result);
}


void mic_set_clk(MCLK_TYPE_T mclk_type, AUD_LRCK_CYC_T cycle, AUD_MIC_CLK_SRC clk_src, bool bck_invert, bool lrck_invert)
{
	if (_mic.state != STATE_STARTED)
	{
		mic_cfg_t *config = &_mic.cfg;
		PR_D("[set_clk] mclk(%d), cycle(%d) clk_src(%d) invert(%d %d) \n",
		mclk_type, cycle, clk_src, bck_invert, lrck_invert);

		if (mclk_type >= AUD_MCLK_TYPE_MAX) {
			PR_E("[set_clk] mclk_type is err(%d) \n", mclk_type);
		} else {
			config->mclk_type = mclk_type;
		}

		if (cycle > AUD_LRCK_CYC_32) {
			PR_E("[set_clk] cycle is err(%d) \n", cycle);
		} else {
			config->cycle = cycle;
		}

		if (clk_src >= MIC_CLK_MAX) {
			PR_E("[set_clk] clk_src is err(%d) \n", clk_src);
		} else {
			config->clk_src = clk_src;
		}

		config->bck_invert = bck_invert;
		config->lrck_invert = lrck_invert;

		REG_MIC_SET_CLK_SRC(config->clk_src);
		if (MIC_CLK_MPH == config->clk_src) {
			IoClk_SetMphMclk(config->mclk_type, misc_get_fs_idx(config->fs));
		}
		REG_MIC_SET_DIVIDER(config->mclk_type, config->cycle);
		REG_MIC_SET_INVERT(config->bck_invert, config->lrck_invert);
	}
}


void mic_set_fmt(u32 src_bit_num, u32 out_bit_num, AUDFMT_INTF_E fmt_intf)
{
	if (_mic.state != STATE_STARTED)
	{
		mic_cfg_t *config = &_mic.cfg;
		PR_D("[set_fmt] src_bit_num(%d), out_bit_num(%d) fmt_intf(%d) \n",
			src_bit_num, out_bit_num, fmt_intf);

		if (src_bit_num != 16 && src_bit_num != 24) {
			PR_E("[set_fmt] src_bit_num is err(%d) \n", src_bit_num);
		} else {
			config->src_bit_num = src_bit_num;
		}

		if (out_bit_num != 16 && out_bit_num != 24) {
			PR_E("[set_fmt] out_bit_num is err(%d) \n", out_bit_num);
		} else {
			config->out_bit_num = out_bit_num;
		}

		if (fmt_intf >= AUDFMT_UNDEF_INTF || fmt_intf == AUDFMT_RESERVD) {
			("[set_fmt] fmt_intf is err(%d) \n", fmt_intf);
		} else {
			config->fmt_intf = fmt_intf;
		}

		REG_MIC_SET_DATA_FMT(config->src_bit_num, config->out_bit_num, config->fmt_intf);
	}
}


void mic_set_hw_gain(u32 hw_gain)
{
	if (_mic.state != STATE_STARTED)
	{
		PR_I("[set_hw_gain] %d => %d \n", _mic.cfg.adc_cfg.u4MicGain, hw_gain);
		_mic.cfg.adc_cfg.u4MicGain = (hw_gain < MIC_MAX_HW_GAIN) ? hw_gain : MIC_MAX_HW_GAIN;
	}
}


void mic_set_primary_idx(u32 primary_idx)
{
	if (_mic.state != STATE_STARTED)
	{
		PR_I("[set_primary_idx] %d => %d \n", _mic.primary_idx, primary_idx);
		_mic.primary_idx = (primary_idx <= MIC_1) ? primary_idx : MIC_0;
	}
}


void mic_dump_regs(void)
{
	PR_I("Dump Mic Register >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
	PR_I("Src(%d), Enable(%d), ArmCtrl(%d), WP(0x%x), SADR(0x%x) \n",
		AUDREG_BITS_R(REGENV_BYPS_VLUM_CFG1, BIT_STR_MPH_AFE_MODE, BIT_NUM_MPH_AFE_MODE),
		AUDREG_BITS_R(REGENV_RGBK2_CFG3, BIT_STR_MPHONE_BUF_EN, BIT_NUM_MPHONE_BUF_EN),
		AUDREG_BITS_R(REGENV_RGBK2_CFG1, BIT_STR_MIC_IN_ARM_CTRL_EN, BIT_NUM_MIC_IN_ARM_CTRL_EN),
		AUDREG_BITS_R(REGENV_RGBK2_CFG2, BIT_STR_MPBUF1_SADR, BIT_NUM_MPBUF1_SADR),
		AUDREG_READ(REGENV_MPHONE_WADR)); \
	PR_I("Buf: Sadr(0x%x, 0x%x, 0x%x) Blk(0x%x) Bank(0x%x) \n",
		AUDREG_BITS_R(REGENV_RGBK2_CFG2, BIT_STR_MPBUF1_SADR, BIT_NUM_MPBUF1_SADR),
		AUDREG_BITS_R(REGENV_RGBK2_CFG2, BIT_STR_MPBUF2_SADR, BIT_NUM_MPBUF2_SADR),
		AUDREG_BITS_R(REGENV_RGBK2_CFG3, BIT_STR_MPBUF3_SADR, BIT_NUM_MPBUF3_SADR),
		AUDREG_BITS_R(REGENV_RGBK2_CFG5, BIT_STR_ARM_MIC_BLK6, BIT_NUM_ARM_MIC_BLK6),
		AUDREG_BITS_R(REGENV_RGBK2_CFG4, BIT_STR_MPHONE_BANK, BIT_NUM_MPHONE_BANK));
	PR_I("DataFmt: SrcBW(%d) Out16Bit(%d) MLeftA(%d) MDatD(%d)\n",
		AUDREG_BITS_R(REGENV_AIN_CFG, BIT_STR_MP_BNUM, BIT_NUM_MP_BNUM),
		AUDREG_BITS_R(REGENV_AFE_TOP_CFG0, BIT_STR_MPH_16BIT_MODE, BIT_NUM_MPH_16BIT_MODE),
		AUDREG_BITS_R(REGENV_AIN_CFG, BIT_STR_M_LEFT_A, BIT_NUM_M_LEFT_A),
		AUDREG_BITS_R(REGENV_AIN_CFG, BIT_STR_M_DAT_D, BIT_NUM_M_DAT_D));
	PR_I("Clk: Clk(%d, %d, %d, %d) \n", \
		AUDREG_BITS_R(REGENV_AFE_TOP_CFG0, BIT_STR_MIC_USE_AO2_GIM, BIT_NUM_MIC_USE_AO2_GIM),
		AUDREG_BITS_R(REGENV_AIN_CFG, BIT_STR_MPCLK_IND, BIT_NUM_MPCLK_IND),
		AUDREG_BITS_R(REGENV_AENV_BAK, BIT_STR_MPHONE_SLAVE, BIT_NUM_MPHONE_SLAVE),
		AUDREG_BITS_R(REGENV_BYPS_VLUM_CFG1, BIT_STR_MPH_WE_SEL, BIT_NUM_MPH_WE_SEL));
	PR_I("Divider: (%d  %d)  Invert(%d  %d)\n", \
		AUDREG_BITS_R(REGENV_MISC_CTRL, BIT_STR_MPHONE_BCK_DIV, BIT_NUM_MPHONE_BCK_DIV),
		AUDREG_BITS_R(REGENV_MISC_CTRL, BIT_STR_MPHONE_LRCK_DIV_SEL, BIT_NUM_MPHONE_LRCK_DIV_SEL),
		AUDREG_BITS_R(REGENV_AIN_CFG, BIT_STR_MPBCK_INV, BIT_NUM_MPBCK_INV),
		AUDREG_BITS_R(REGENV_AIN_CFG, BIT_STR_M_INV_LRCK, BIT_NUM_M_INV_LRCK));
	PR_I("Dump Mic Register <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< \n");
}



virmic_t *virmic_start(char *name, u32 fs)
{
	virmic_t *this = (virmic_t *)apcm_mem_alloc(sizeof(virmic_t));

	if (this) {
		PR_D("[vir_start(%s)] fs(%d => %d)\n", name, _mic.cfg.fs, fs);
		this->rptr = mic_start();
		sprintf(this->name, "%s", name);

		this->asrc = NULL;
		this->asrc_buf = NULL;
		if (fs != _mic.cfg.fs) {
			this->asrc = asrc_open(_mic.cfg.fs, fs);
			this->asrc_buf = buf_open(NULL, MIC_BUF_SIZE, STEREO);
			if (this->asrc == NULL || this->asrc_buf == NULL) {
				PR_E("[vir_start(%s)] alloc asrc or its buff fail! \n", name);
				virmic_stop(this);
				this = NULL;
			}
		}
	}

	return (this);
}


virmic_t *virmic_stop(virmic_t *this)
{
	if (this) {
		PR_D("[vir_stop(%s)] \n", this->name);
		mic_stop();
		apcm_mem_free(this);
	}

	return (NULL);
}


u32 virmic_read(virmic_t *this, apcm_buf_t *dst_buf)
{
	u32 copy_size = 0;

	if (this) {
		if (this->asrc) {
			copy_size = asrc_read(this->asrc, dst_buf);
			mic_read(this->asrc_buf, &this->rptr);
			asrc_write(this->asrc, this->asrc_buf);
		} else {
			copy_size = mic_read(dst_buf, &this->rptr);
		}
	}

	return copy_size;
}


void virmic_reset_point(virmic_t *this)
{
	if (this) {
		this->rptr = mic_get_wptr();
		if (this->asrc) {
			asrc_stop(this->asrc);
			asrc_start(this->asrc);
		}
		PR_D("[vir_reset_point(%s)] rptr = %d \n", this->name, this->rptr);
	}
}


