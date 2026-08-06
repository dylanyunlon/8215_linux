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
*[File]		apcm_asrc.c
*[Author]		tongfa.luo@autochips.com
*[Description]
*
*[Copyright]
*
******************************************************************************/

#include "apcm_asrc_reg.h"

#define LOG_TAG				"[Asrc]"

#define ASRC_VECTOR			VECTOR_ASRC_GPS

#define AC83XX_PCM_ASRC_VA		AC83XX_PCM_VA
#define AC83XX_PCM_ASRC_PA		AC83XX_PCM_PA

#define ASRC_IBUF_CH_SIZE		(3840 * 2)
#define ASRC_OBUF_CH_SIZE		(7680 * 2)

#define ASRC_PALETTE_NUMBER		(8U)

#define ASRC_ALIGNMENT_SIZE		48

static DEFINE_SPINLOCK(asrc_lock);


typedef struct
{
    volatile u32 gen_conf[2];
    volatile u32 ier[2];
    volatile u32 ifr[2];
    volatile u32 ch_conf[6];
    volatile u32 fs[8];

    volatile u32 ibuf_rp[6];
    volatile u32 ibuf_wp[6];
    volatile u32 obuf_wp[6];
    volatile u32 obuf_rp[6];

    volatile u32 ibuf_intr_cnt[3];
    volatile u32 obuf_intr_cnt[3];
    volatile u32 max_out_per_in[2];

}asrc_reg_map_t;


typedef struct
{
    u32 fs;
    u32 ref;
}asrc_palette_cfg_t;


typedef struct
{
    u32 inited;
    u32 start_cnt;
    u32 isr_cnt;

    u32 ibuf_phy_addr;
    u32 ibuf_ch_sz;
    u32 obuf_phy_addr;
    u32 obuf_ch_sz;

    asrc_palette_cfg_t palette[ASRC_PALETTE_NUMBER];
} asrc_mgr_t;


static asrc_reg_map_t _asrc_reg;
static asrc_chs_t _asrc_chs[ASRC_CHSET_NUMBER];
static asrc_mgr_t _asrc_mgr;

//===================================================//

static void asrc_reg_map(void)
{
	PR_I("[reg_map] \n");

	_asrc_reg.gen_conf[0] = REG_ASRC_GEN_CONF;
	_asrc_reg.gen_conf[1] = REG_ASRC_GEN_CONF2;

	_asrc_reg.ier[0] = REG_ASRC_IER;
	_asrc_reg.ier[1] = REG_ASRC_IER2;
	_asrc_reg.ifr[0] = REG_ASRC_IFR;
	_asrc_reg.ifr[1] = REG_ASRC_IFR2;

	_asrc_reg.ch_conf[0] = REG_ASRC_CH01_CNFG;
	_asrc_reg.ch_conf[1] = REG_ASRC_CH23_CNFG;
	_asrc_reg.ch_conf[2] = REG_ASRC_CH45_CNFG;
	_asrc_reg.ch_conf[3] = REG_ASRC_CH67_CNFG;
	_asrc_reg.ch_conf[4] = REG_ASRC_CH89_CNFG;
	_asrc_reg.ch_conf[5] = REG_ASRC_CH1011_CNFG;

	_asrc_reg.fs[0] = REG_ASRC_FREQUENCY0;
	_asrc_reg.fs[1] = REG_ASRC_FREQUENCY1;
	_asrc_reg.fs[2] = REG_ASRC_FREQUENCY2;
	_asrc_reg.fs[3] = REG_ASRC_FREQUENCY3;
	_asrc_reg.fs[4] = REG_ASRC_FREQUENCY4;
	_asrc_reg.fs[5] = REG_ASRC_FREQUENCY5;
	_asrc_reg.fs[6] = REG_ASRC_FREQUENCY6;
	_asrc_reg.fs[7] = REG_ASRC_FREQUENCY7;

	_asrc_reg.ibuf_rp[0] = REG_ASRC_CH01_IBUF_RDPNT;
	_asrc_reg.ibuf_rp[1] = REG_ASRC_CH23_IBUF_RDPNT;
	_asrc_reg.ibuf_rp[2] = REG_ASRC_CH45_IBUF_RDPNT;
	_asrc_reg.ibuf_rp[3] = REG_ASRC_CH67_IBUF_RDPNT;
	_asrc_reg.ibuf_rp[4] = REG_ASRC_CH89_IBUF_RDPNT;
	_asrc_reg.ibuf_rp[5] = REG_ASRC_CH1011_IBUF_RDPNT;

	_asrc_reg.ibuf_wp[0] = REG_ASRC_CH01_IBUF_WRPNT;
	_asrc_reg.ibuf_wp[1] = REG_ASRC_CH23_IBUF_WRPNT;
	_asrc_reg.ibuf_wp[2] = REG_ASRC_CH45_IBUF_WRPNT;
	_asrc_reg.ibuf_wp[3] = REG_ASRC_CH67_IBUF_WRPNT;
	_asrc_reg.ibuf_wp[4] = REG_ASRC_CH89_IBUF_WRPNT;
	_asrc_reg.ibuf_wp[5] = REG_ASRC_CH1011_IBUF_WRPNT;

	_asrc_reg.obuf_wp[0] = REG_ASRC_CH01_OBUF_WRPNT;
	_asrc_reg.obuf_wp[1] = REG_ASRC_CH23_OBUF_WRPNT;
	_asrc_reg.obuf_wp[2] = REG_ASRC_CH45_OBUF_WRPNT;
	_asrc_reg.obuf_wp[3] = REG_ASRC_CH67_OBUF_WRPNT;
	_asrc_reg.obuf_wp[4] = REG_ASRC_CH89_OBUF_WRPNT;
	_asrc_reg.obuf_wp[5] = REG_ASRC_CH1011_OBUF_WRPNT;

	_asrc_reg.obuf_rp[0] = REG_ASRC_CH01_OBUF_RDPNT;
	_asrc_reg.obuf_rp[1] = REG_ASRC_CH23_OBUF_RDPNT;
	_asrc_reg.obuf_rp[2] = REG_ASRC_CH45_OBUF_RDPNT;
	_asrc_reg.obuf_rp[3] = REG_ASRC_CH67_OBUF_RDPNT;
	_asrc_reg.obuf_rp[4] = REG_ASRC_CH89_OBUF_RDPNT;
	_asrc_reg.obuf_rp[5] = REG_ASRC_CH1011_OBUF_RDPNT;

	_asrc_reg.ibuf_intr_cnt[0] = REG_ASRC_IBUF_INTR_CNT0;
	_asrc_reg.ibuf_intr_cnt[1] = REG_ASRC_IBUF_INTR_CNT1;
	_asrc_reg.ibuf_intr_cnt[2] = REG_ASRC_IBUF_INTR_CNT2;

	_asrc_reg.obuf_intr_cnt[0] = REG_ASRC_OBUF_INTR_CNT0;
	_asrc_reg.obuf_intr_cnt[1] = REG_ASRC_OBUF_INTR_CNT1;
	_asrc_reg.obuf_intr_cnt[2] = REG_ASRC_OBUF_INTR_CNT2;

	_asrc_reg.max_out_per_in[0] = REG_ASRC_MAX_OUT_PER_IN0;
	_asrc_reg.max_out_per_in[1] = REG_ASRC_MAX_OUT_PER_IN1;

}


static u32 asrc_get_intr_val(u32 idx, u32 intr_type)
{
    u32 reg_val = 0;

    if (ASRC_IBUF_AMOUNT_INTR & intr_type) {
        reg_val |= REG_ASRC_IBUF_AMOUNT_BIT_VAL(idx);
    }
    if (ASRC_IBUF_EMPTY_INTR & intr_type) {
        reg_val |= REG_ASRC_IBUF_EMPTY_BIT_VAL(idx);
    }
    if (ASRC_OBUF_OV_INTR & intr_type) {
        reg_val |= REG_ASRC_OBUF_OV_BIT_VAL(idx);
    }
    if (ASRC_OBUF_AMOUNT_INTR & intr_type) {
        reg_val |= REG_ASRC_OBUF_AMOUNT_BIT_VAL(idx);
    }

    return (reg_val);
}


static void asrc_intr_enable(u32 idx, u32 intr_type)
{
	u32 reg_val = asrc_get_intr_val(idx, intr_type);
	REG_ASRC_WRITE_IFR(idx, reg_val);
	reg_val |= REG_ASRC_READ_IER(idx);
	REG_ASRC_WRITE_IER(idx, reg_val);    // Enable interrupt.
}


static void asrc_intr_disbale(u32 idx, u32 intr_type)
{
	u32 reg_val = asrc_get_intr_val(idx, intr_type);
	REG_ASRC_WRITE_IFR(idx, reg_val);
	reg_val = ~reg_val;
	reg_val &= REG_ASRC_READ_IER(idx);
	REG_ASRC_WRITE_IER(idx, reg_val);    // Disable interrupt.
}


static void asrc_reg_isr_cb(u32 idx)
{
	u32 value, intr_type = _asrc_chs[idx].cfg.intr_type;

	if (ASRC_IBUF_AMOUNT_INTR & intr_type) {
		value = _asrc_chs[idx].ibuf->buf_size >> 1;
		REG_ASRC_SET_IBUF_INTR_CNT(idx, (value + 48) /48);
	}

	if (ASRC_OBUF_AMOUNT_INTR & intr_type) {
		value = _asrc_chs[idx].obuf->buf_size >> 1;
		REG_ASRC_SET_OBUF_INTR_CNT(idx, (value + 48) /48);
	}

	if(ASRC_OBUF_OV_INTR & intr_type) {
		REG_ASRC_SET_MAX_OUT_PER_IN(idx, 0);
	} else {
		asrc_cfg_t *cfg = &_asrc_chs[idx].cfg;
		value = (cfg->ofs + cfg->ifs - 1) / cfg->ifs;
		REG_ASRC_SET_MAX_OUT_PER_IN(idx, value);
	}

	asrc_intr_enable(idx, intr_type);
}


static void asrc_isr(u16 vec)
{
	u32 idx = 0, intr_type, old_ifr[2];
	old_ifr[0] = REG_ASRC_READ_IFR(0);  //chset 0 ~ 3
	old_ifr[1] = REG_ASRC_READ_IFR(4);  //chset 4 ~ 5

	for (idx = 0; idx < ASRC_CHSET_NUMBER; idx++)
	{
		if (_asrc_chs[idx].state != STATE_UNINIT) {
			intr_type = 0;
			if (REG_ASRC_GET_IBUF_EMPTY_FLAG(idx))  {
				intr_type |= ASRC_IBUF_EMPTY_INTR;
			}
			if (REG_ASRC_GET_IBUF_AMOUNT_FLAG(idx))  {
				intr_type |= ASRC_IBUF_AMOUNT_INTR;
			}
			if (REG_ASRC_GET_OBUF_OV_FLAG(idx))  {
				PR_E("[isr(%d)] over flow \n", idx);
				intr_type |= ASRC_OBUF_OV_INTR;
				buf_set_full(_asrc_chs[idx].obuf);
			}
			if (REG_ASRC_GET_OBUF_AMOUNT_FLAG(idx)) {
				intr_type |= ASRC_OBUF_AMOUNT_INTR;
			}

			if (_asrc_chs[idx].cfg.pfn_cb && intr_type) {
				_asrc_chs[idx].cfg.pfn_cb((void *)(&_asrc_chs[idx]), intr_type);
			}
		}
	}

	REG_ASRC_WRITE_IFR(0, old_ifr[0]);
	REG_ASRC_WRITE_IFR(4, old_ifr[1]);
	misc_irq_clear(ASRC_VECTOR);
}


static void asrc_init_memory(void)
{
	u32 idx = 0, ibuf_vir_addr = 0, obuf_vir_addr = 0;

	_asrc_mgr.ibuf_ch_sz = ALIGNMENT(ASRC_IBUF_CH_SIZE, ASRC_ALIGNMENT_SIZE);
	_asrc_mgr.obuf_ch_sz = ALIGNMENT(ASRC_OBUF_CH_SIZE, ASRC_ALIGNMENT_SIZE);

	ibuf_vir_addr = AC83XX_PCM_ASRC_VA;
	obuf_vir_addr = ibuf_vir_addr + _asrc_mgr.ibuf_ch_sz * (ASRC_CHSET_NUMBER << 1);

	_asrc_mgr.ibuf_phy_addr = AC83XX_PCM_ASRC_PA;
	_asrc_mgr.obuf_phy_addr = _asrc_mgr.ibuf_phy_addr + _asrc_mgr.ibuf_ch_sz * (ASRC_CHSET_NUMBER << 1);

	PR_I("[init_memory] In(0x%x, 0x%x, %d) Out(0x%x, 0x%x, %d)  \n",
		ibuf_vir_addr, _asrc_mgr.ibuf_phy_addr, _asrc_mgr.ibuf_ch_sz,
		obuf_vir_addr, _asrc_mgr.obuf_phy_addr, _asrc_mgr.obuf_ch_sz);

	REG_ASRC_SET_IBUF_INFO(_asrc_mgr.ibuf_phy_addr, _asrc_mgr.ibuf_ch_sz);
	REG_ASRC_SET_OBUF_INFO(_asrc_mgr.obuf_phy_addr, _asrc_mgr.obuf_ch_sz);

	for (idx = 0; idx < ASRC_CHSET_NUMBER; idx++)
	{
		u32 offset = _asrc_mgr.ibuf_ch_sz * idx * 2;
		_asrc_chs[idx].ibuf_sadr = (_asrc_mgr.ibuf_phy_addr + offset) & 0xFFFFF;
		_asrc_chs[idx].ibuf = buf_open((void *)(ibuf_vir_addr + offset), _asrc_mgr.ibuf_ch_sz, 2);

		offset = _asrc_mgr.obuf_ch_sz * idx * 2;
		_asrc_chs[idx].obuf_sadr = (_asrc_mgr.obuf_phy_addr + offset) & 0xFFFFF;
		_asrc_chs[idx].obuf = buf_open((void *)(obuf_vir_addr + offset), _asrc_mgr.obuf_ch_sz, 2);
		buf_set_reserve_data_size(_asrc_chs[idx].obuf, ASRC_ALIGNMENT_SIZE);
	}
}


static u32 asrc_alloc_palette(u32 fs)
{
	u32 i, palette_idx = ASRC_PALETTE_NUMBER;

	//First: Find Used Palette
	for (i = 0; i < ASRC_PALETTE_NUMBER; i++)
	{
		if (_asrc_mgr.palette[i].ref && (_asrc_mgr.palette[i].fs == fs)) {
			_asrc_mgr.palette[i].ref++;
			palette_idx = i;
			break;
		}
	}

	//Second: if not find Used Palette, try to find UnUsed Palette!
	if(palette_idx == ASRC_PALETTE_NUMBER)
	{
		for (i = 0; i < ASRC_PALETTE_NUMBER; i++)
		{
			if (!_asrc_mgr.palette[i].ref) {
				palette_idx = i;
				_asrc_mgr.palette[i].fs = fs;
				_asrc_mgr.palette[i].ref++;
				REG_ASRC_SET_FREQ(palette_idx, fs);
				PR_D("[alloc_palette] Set Palette Freq(%d:  %d). \n", i, fs);
				break;
			}
		}
	}

	return (palette_idx);
}


static void asrc_free_palette(u32 i_palette, u32 o_palette)
{
	if (i_palette < ASRC_PALETTE_NUMBER && _asrc_mgr.palette[i_palette].ref) {
		_asrc_mgr.palette[i_palette].ref--;
	}

	if (o_palette < ASRC_PALETTE_NUMBER && _asrc_mgr.palette[o_palette].ref) {
		_asrc_mgr.palette[o_palette].ref--;
	}
}


static u32 asrc_check_config(asrc_cfg_t *cfg)
{
	u32 result = RET_ERROR;

	if (cfg)
	{
		if ((cfg->ifs < SAMPLE_RATE_8K || cfg->ifs > SAMPLE_RATE_48K)
			|| (cfg->ibw != 16  && cfg->ibw != 24)
			|| (cfg->ofs < SAMPLE_RATE_8K || cfg->ofs > SAMPLE_RATE_48K)
			|| (cfg->obw != 16  && cfg->obw != 24))
		{
			PR_E("[check_config] fmt error: fs(%d, %d) bw(%d, %d)! \n",
			cfg->ifs, cfg->ofs, cfg->ibw, cfg->obw);
			result = RET_ERROR;
		} else {
			result = RET_NOERR;
		}
	}

	return (result);
}


bool asrc_init(void)
{
	bool result = false;
	u32 idx = 0;

	if (!_asrc_mgr.inited)
	{
		asrc_reg_map();
		REG_ASRC_RESET_REG();

		misc_irq_disable(ASRC_VECTOR);
		misc_isr_reg(ASRC_VECTOR, asrc_isr);

		for (idx = 0; idx < ASRC_PALETTE_NUMBER; idx++) {
			_asrc_mgr.palette[idx].fs = 0;
			_asrc_mgr.palette[idx].ref = 0;
		}
		asrc_init_memory();

		for (idx = 0; idx < ASRC_CHSET_NUMBER; idx++)
		{
			asrc_chs_t *this = &_asrc_chs[idx];
			this->idx = idx;
			this->state = STATE_UNINIT;
			this->i_palette = ASRC_PALETTE_NUMBER;
			this->o_palette = ASRC_PALETTE_NUMBER;
			apcm_memset(&this->cfg, 0, sizeof(asrc_cfg_t));
		}

		_asrc_mgr.start_cnt = 0;
		_asrc_mgr.inited = true;
		PR_I("[init] success! \n");
	}

	return (result);
}


bool asrc_uninit(void)
{
	bool result = true;
	u32 idx = 0;

	if (_asrc_mgr.inited)
	{
		misc_irq_disable(ASRC_VECTOR);
		misc_isr_unreg(ASRC_VECTOR);

		for (idx = 0; idx < ASRC_CHSET_NUMBER; idx++) {
			_asrc_chs[idx].state = STATE_UNINIT;
			buf_close(_asrc_chs[idx].ibuf);
			buf_close(_asrc_chs[idx].obuf);
		}

		_asrc_mgr.inited = false;
	}

	return (result);
}


void asrc_hibernation(bool wake_up)
{
	PR_I("[hibernation]: wake_up(%d). \n", wake_up);
	if (wake_up)
	{
		u32 i = 0;
		REG_ASRC_RESET_REG();
		REG_ASRC_SET_IBUF_INFO(_asrc_mgr.ibuf_phy_addr, _asrc_mgr.ibuf_ch_sz);
		REG_ASRC_SET_OBUF_INFO(_asrc_mgr.obuf_phy_addr, _asrc_mgr.obuf_ch_sz);
		for (i = 0; i < ASRC_PALETTE_NUMBER; i++) {
			REG_ASRC_SET_FREQ(i, _asrc_mgr.palette[i].fs);
		}
	}
}


asrc_chs_t *asrc_open_special(u32 idx, asrc_cfg_t *cfg)  // just for unit test
{
	asrc_chs_t *this = NULL;
	if (cfg && asrc_check_config(cfg) == RET_NOERR)
	{
		spin_lock(&asrc_lock);
		if (idx < ASRC_CHSET_NUMBER && _asrc_chs[idx].state == STATE_UNINIT) {
			this = &(_asrc_chs[idx]);
			this->state = STATE_INITED;
		}
		spin_unlock(&asrc_lock);

		if (this) {
			this->idx = idx;
			this->i_palette = ASRC_PALETTE_NUMBER;
			this->o_palette = ASRC_PALETTE_NUMBER;
			asrc_setup(this, cfg);
			PR_I("[open_special] %d success. \n", this->idx);
		} else {
			PR_E("[open_special] %d fail. \n", idx);
		}
	}

	return (this);
}


asrc_chs_t *asrc_open_by_cfg(asrc_cfg_t *cfg)
{
	asrc_chs_t *this = NULL;
	u32 idx = 0;

	if (cfg && asrc_check_config(cfg) == RET_NOERR)
	{
		spin_lock(&asrc_lock);
		for (idx = 0; idx < ASRC_CHSET_NUMBER; idx++)
		{
			if (_asrc_chs[idx].state == STATE_UNINIT) {
				this = &(_asrc_chs[idx]);
				this->state = STATE_INITED;
				break;
			}
		}
		spin_unlock(&asrc_lock);

		if (this) {
			PR_D("[open] use asrc %d. \n", idx);
			this->idx = idx;
			this->i_palette = ASRC_PALETTE_NUMBER;
			this->o_palette = ASRC_PALETTE_NUMBER;
			asrc_setup(this, cfg);
		} else {
			PR_E("[open] No free ASRC! \n");
		}
	}

	return (this);
}


asrc_chs_t *asrc_open(u32 ifs, u32 ofs)
{
	asrc_chs_t *this = NULL;
	asrc_cfg_t cfg;

	cfg.ifs = ifs;
	cfg.ofs = ofs;
	cfg.ibw = APCM_DEF_DATA_BITS;
	cfg.obw = APCM_DEF_DATA_BITS;
	cfg.intr_type = ASRC_OBUF_OV_INTR;
	cfg.pfn_cb = NULL;

	this = asrc_open_by_cfg(&cfg);
	return (this);
}


void *asrc_close(asrc_chs_t *this)
{
	if (this)
	{
		PR_D("[close(%d)] start_cnt(%d)!\n", this->idx, _asrc_mgr.start_cnt);

		asrc_intr_disbale(this->idx, ASRC_ALL_INTR);
		asrc_free_palette(this->i_palette, this->o_palette);

		this->i_palette = ASRC_PALETTE_NUMBER;
		this->o_palette = ASRC_PALETTE_NUMBER;
		apcm_memset(&this->cfg, 0, sizeof(asrc_cfg_t));
		this->state = STATE_UNINIT;
	}
	return (NULL);
}


void asrc_setup(asrc_chs_t *this, asrc_cfg_t *cfg)
{
	bool result = false;
	u32 idx = 0;

	if (this && cfg && asrc_check_config(cfg) == RET_NOERR)
	{
		idx = this->idx;
		if (this->state == STATE_STOPPED) {
			asrc_free_palette(this->i_palette, this->o_palette);
		}

		apcm_memcpy(&this->cfg, cfg, sizeof(asrc_cfg_t));
		PR_D("[setup(%d)] fmt : fs(%d, %d) bw(%d, %d)! \r\n",
			idx, cfg->ifs, cfg->ofs, cfg->ibw, cfg->obw);

		this->i_palette = asrc_alloc_palette(cfg->ifs);
		this->o_palette = asrc_alloc_palette(cfg->ofs);

		REG_ASRC_SET_MONO(idx);
		REG_ASRC_SET_IFS(idx, this->i_palette);
		REG_ASRC_SET_OFS(idx, this->o_palette);
		REG_ASRC_SET_IBW(idx, cfg->ibw);
		REG_ASRC_SET_OBW(idx, cfg->obw);
		REG_ASRC_SET_CALC_AMOUNT(idx);

		REG_ASRC_SET_IBUF_RP(idx, this->ibuf_sadr);
		REG_ASRC_SET_IBUF_WP(idx, this->ibuf_sadr);
		REG_ASRC_SET_OBUF_RP(idx, this->obuf_sadr);
		REG_ASRC_SET_OBUF_WP(idx, this->obuf_sadr);

		asrc_intr_disbale(idx, ASRC_ALL_INTR);
		misc_irq_enable(ASRC_VECTOR);
		asrc_reg_isr_cb(idx);
		REG_ASRC_CLEAR(idx);

		this->state = STATE_STOPPED;
	}
}


void asrc_start(asrc_chs_t *this)
{
	if (this && STATE_STOPPED == this->state)
	{
		PR_D("[start(%d)] state(%d) start_cnt(%d).\n",
			this->idx, this->state, _asrc_mgr.start_cnt);

		buf_reset(this->ibuf);
		buf_reset(this->obuf);
		REG_ASRC_ENABLE_CHSET(this->idx, true);
		this->state = STATE_STARTED;

		if (!_asrc_mgr.start_cnt){
			misc_irq_enable(ASRC_VECTOR);
			_asrc_mgr.isr_cnt = 0;
			REG_ASRC_ENABLE(true);
			//asrc_dump_regs();
		}
		_asrc_mgr.start_cnt++;

		PR_D("[start(%d)] state(%d) start_cnt(%d).<<<<\n",
			this->idx, this->state, _asrc_mgr.start_cnt);
	}
}


void asrc_stop(asrc_chs_t *this)
{
	if (this)
	{
		PR_D("[stop(%d)] state(%d) start_cnt(%d). \n",
			this->idx, this->state, _asrc_mgr.start_cnt);

		REG_ASRC_ENABLE_CHSET(this->idx, false);
		this->state = STATE_STOPPED;

		if (_asrc_mgr.start_cnt) {
			_asrc_mgr.start_cnt--;
			if (!_asrc_mgr.start_cnt) {
				misc_irq_disable(ASRC_VECTOR);
				REG_ASRC_ENABLE(false);
			}
		}
	}
}


static u32 asrc_get_free_size(asrc_chs_t *this)
{
	u32 free_size = 0;
	if (this && this->state == STATE_STARTED)
	{
		apcm_buf_t *ibuf = this->ibuf;
		apcm_buf_t *obuf = this->obuf;
		u32 max_free_size, data_size;

		ibuf->rptr = REG_ASRC_GET_IBUF_RP(this->idx) - this->ibuf_sadr;
		free_size = buf_get_free_size(ibuf);
		data_size = ibuf->buf_size - free_size;

		obuf->wptr = REG_ASRC_GET_OBUF_WP(this->idx) - this->obuf_sadr;
		max_free_size = buf_get_free_size(obuf);
		max_free_size = max_free_size / (this->cfg.ofs / this->cfg.ifs + 1);

		if (free_size + data_size >= max_free_size) {
			free_size = (max_free_size > data_size) ? (max_free_size - data_size) : 0;
		}
	}
	return (free_size);
}


u32 asrc_read(asrc_chs_t *this, apcm_buf_t *dst_buf)
{
	u32 copy_size = 0;

	if (this && this->state == STATE_STARTED)
	{
		this->obuf->wptr = REG_ASRC_GET_OBUF_WP(this->idx) - this->obuf_sadr;
		copy_size = buf_copy(dst_buf, this->obuf);
		REG_ASRC_SET_OBUF_RP(this->idx, (this->obuf_sadr + this->obuf->rptr));
	}

	return (copy_size);
}


u32 asrc_write(asrc_chs_t *this, apcm_buf_t *src_buf)
{
	u32 copy_size = 0;
	if (this && this->state == STATE_STARTED)
	{
		u32 free_size = asrc_get_free_size(this);
		copy_size = buf_limit_copy(this->ibuf, src_buf, free_size);
		REG_ASRC_SET_IBUF_WP(this->idx, (this->ibuf_sadr + this->ibuf->wptr));
	}
	return (copy_size);
}


void asrc_dump_regs(void)
{
	u32 i = 0;
	PR_I("[dump_regs]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");

	for (i = 0; i < 0x140; i += 16)
	{
		PR_I("    0x%5X : Val: 0x%08x, 0x%08x, 0x%08x, 0x%08x.\r\n",
			(u32)(ASRC_REGBASE + i ),
			(u32)AUDREG_READ(ASRC_REGBASE + i),
			(u32)AUDREG_READ(ASRC_REGBASE + i + 4),
			(u32)AUDREG_READ(ASRC_REGBASE + i + 8),
			(u32)AUDREG_READ(ASRC_REGBASE + i + 12));
	}

	PR_I("[dump_regs] <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n");
}

