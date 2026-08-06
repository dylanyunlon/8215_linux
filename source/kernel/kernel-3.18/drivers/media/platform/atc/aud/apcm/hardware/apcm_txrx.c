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
*[File]             apcm_txrx.c
*[Author]
*[Description]
*[Copyright]
*
******************************************************************************/

#include "apcm_txrx_reg.h"

#define LOG_TAG			"[TxRx]"


#define APCM_TXRX_VA			(AC83XX_PCM_VA + 300 * 1024)
#define APCM_TXRX_PA			(AC83XX_PCM_PA + 300 * 1024)

#define APCM_TX_BUF_SZ			(24000 * 2)
#define APCM_RX_BUF_SZ			(24000 * 2)

#define APCCM_TX_DEF_INT_SAMPLES	160

typedef struct
{
	AUD_PCM_HW_MODE hw_mode;  		// default : PCM_NORMAL_MODE
	u32 fs;

	AUD_PCM_SYNC_MODE sync_mode;  		// default : PCM_LONG_MODE
	AUD_PCM_SYNC_CYCLE sync_cycle; 		// default : PCM_CLK_CYCLE_32
	AUD_PCM_SYNC_LENGTH sync_length;

	AUD_PCM_DATA_ORDER data_order;
	AUD_PCM_BIT_NUM bit_num;
	AUD_PCM_BIT_MODE bit_mode;

	u32 int_samples;           		// next sample number
    	u32 burst_samples;    			// remain sample number
    	PFN_TXRX_ISR_CB pfb_cb;			// s32 callback function

}txrx_cfg_t;


typedef struct
{
	u32 tx_sadr;
	u32 tx_eadr;
	u32 tx_blk_adr;

	u32 rx_sadr;
	u32 rx_eadr;
	u32 rx_blk_adr;

	u32 bank_adr;

}txrx_phy_addr_t;


typedef struct
{
	u32 state;
	u32 isr_cnt;

	txrx_cfg_t cfg;
	txrx_phy_addr_t phy_addr;

	apcm_buf_t *tx_buf;
	apcm_buf_t *rx_buf;
	u32 next_rptr;

	apcm_buf_t tx_dump_buf;
	apcm_buf_t rx_dump_buf;
	apcm_file_t *tx_file;
	apcm_file_t *rx_file;
	bool dump_enable;

} apcm_txrx_t;


static apcm_txrx_t _txrx = {
    .state = STATE_UNINIT,

    .dump_enable = false,
    .tx_file = NULL,
    .rx_file = NULL
};

//================================================================//

static void txrx_dump_file_start(void)
{
	if (_txrx.dump_enable) {
		u32 idx = GET_FILE_IDX;
		apcm_memcpy(&_txrx.rx_dump_buf, _txrx.rx_buf, sizeof(apcm_buf_t));
		apcm_memcpy(&_txrx.tx_dump_buf, _txrx.tx_buf, sizeof(apcm_buf_t));
		_txrx.rx_file = file_open_w("bt_rx", idx, MONO, _txrx.cfg.fs);
		_txrx.tx_file = file_open_w("bt_tx", idx, MONO, _txrx.cfg.fs);
	}
}


static void txrx_dump_file_stop(void)
{
	if (_txrx.rx_file) {
		_txrx.rx_file = file_close(_txrx.rx_file);
	}
	if (_txrx.tx_file) {
		_txrx.tx_file = file_close(_txrx.tx_file);
	}
}


//================================================================//

static void txrx_set_nsadr(void)
{
	u32 nsadr = _txrx.phy_addr.tx_sadr + (_txrx.next_rptr >> 4);
	REG_TXRX_SET_TX_NSADR(nsadr);
}


static void txrx_isr(u16 vector)
{
	u32 data_size = 0;
	u32 int_bytes = (_txrx.cfg.int_samples << 1);

	++_txrx.isr_cnt;
	if (NULL != _txrx.cfg.pfb_cb) {
		_txrx.cfg.pfb_cb(vector);
	}

	_txrx.tx_buf->rptr = _txrx.next_rptr;
	data_size = buf_get_data_size(_txrx.tx_buf);
	if (data_size < int_bytes) {
		if (_txrx.isr_cnt > 110) {
			PR_E("[isr] Tx buf is empty! (%d < %d) isr_cnt(%d) Time(%d)\n",
				data_size, (int_bytes), _txrx.isr_cnt, GET_SYS_TIME);
		}
	} else {
		_txrx.next_rptr = (_txrx.next_rptr + int_bytes) % _txrx.tx_buf->buf_size;
		txrx_set_nsadr();
	}
	misc_irq_clear(vector);
}


static void txrx_reg_cfg(void)
{
	txrx_cfg_t *config = &_txrx.cfg;
	txrx_phy_addr_t *phy_addr = &_txrx.phy_addr;

	PR_D("[reg_config] \n");

	REG_TXRX_SET_HW_MODE(config->hw_mode);
	REG_TXRX_SET_SYNC_CFG(config->sync_mode, config->sync_cycle, config->sync_length);
	IoClk_SetPcmMclk(config->sync_cycle, config->fs);
	REG_TXRX_SET_DATA_FMT(config->data_order, config->bit_num, config->bit_mode);
	REG_TXRX_SET_MODE(PCM_MASTER);

	REG_TXRX_SET_TX_ADR(phy_addr->tx_sadr, phy_addr->tx_eadr, phy_addr->tx_blk_adr);
	REG_TXRX_SET_RX_ADR(phy_addr->rx_sadr, phy_addr->rx_eadr, phy_addr->rx_blk_adr);
	REG_TXRX_SET_BANK_ADR(phy_addr->bank_adr);

	REG_TXRX_SET_TX_INT_CFG(config->int_samples, config->burst_samples);

	IoPinMux_SetPcm(PINMUX_PCM_GROUP1);
}


static void txrx_init_memory(void)
{
	txrx_phy_addr_t *phy_addr = &_txrx.phy_addr;
	u32 tx_start_phy_addr = APCM_TXRX_PA;
	u32 rx_start_phy_addr = APCM_TXRX_PA + APCM_TX_BUF_SZ;

	phy_addr->tx_blk_adr = (tx_start_phy_addr & 0x0fff00) >> 8;
	phy_addr->tx_sadr = (tx_start_phy_addr & 0xf0) >> 4;
	phy_addr->tx_eadr = phy_addr->tx_sadr +  (APCM_TX_BUF_SZ >> 4) - 1;

	phy_addr->rx_blk_adr = (rx_start_phy_addr & 0x0fff00) >> 8;
	phy_addr->rx_sadr = (rx_start_phy_addr & 0xf0) >> 4;
	phy_addr->rx_eadr = phy_addr->rx_sadr +  (APCM_RX_BUF_SZ >> 4) - 1;

	phy_addr->bank_adr = (tx_start_phy_addr & 0x7ff00000) >> 20;

	_txrx.tx_buf = buf_open(APCM_TXRX_VA, APCM_TX_BUF_SZ, 1);
	_txrx.rx_buf = buf_open((APCM_TXRX_VA + APCM_TX_BUF_SZ), APCM_RX_BUF_SZ, 1);

	PR_I("[init_memory] Tx(0x%p, 0x%p, %d) Rx(0x%p, 0x%p, %d)\n",
		tx_start_phy_addr, APCM_TXRX_VA, APCM_TX_BUF_SZ,
		rx_start_phy_addr, (APCM_TXRX_VA + APCM_TX_BUF_SZ), APCM_RX_BUF_SZ);
}


static bool txrx_init_int_ctrl(void)
{
	bool result = true;

	misc_irq_disable(VECTOR_AOUT_BT_RC);
	if (!misc_isr_reg(VECTOR_AOUT_BT_RC, txrx_isr)) {
		PR_E("[init_int_ctrl] TX ISR REG Fail! \n");
		result = false;
	}
	misc_irq_enable(VECTOR_AOUT_BT_RC);
	misc_irq_clear(VECTOR_AOUT_BT_RC);

	//Change PCM Isr Level Trigger to Edge Trigger
	{
	    u32 value = IO_READ32(0xFE000000, 0x1C2C);
	    value |= 0x3<<20;
	    IO_WRITE32(0xFE000000, 0x1C2C, value);
	}

	return (result);
}


void txrx_init(void)
{
	if (_txrx.state == STATE_UNINIT)
	{
		txrx_cfg_t *config = &_txrx.cfg;
		_txrx.state = STATE_INITED;

		config->hw_mode = PCM_NORMAL_MODE;
		config->fs = SAMPLE_RATE_16K;

		config->sync_mode = PCM_LONG_MODE;
		config->sync_cycle = PCM_CLK_CYCLE_32;
		config->sync_length = (PCM_LONG_MODE == config->sync_mode) ? PCM_CLK_LENGTH3 : PCM_CLK_LENGTH1;

		config->data_order = PCM_MSB_FIRST;
		config->bit_num = PCM_BITS_16;
		config->bit_mode = PCM_LINEAR_16BIT;

		config->int_samples= APCCM_TX_DEF_INT_SAMPLES;
		config->burst_samples = (config->int_samples >> 1);
		config->pfb_cb = NULL;

		txrx_init_memory();
		txrx_init_int_ctrl();
		txrx_reg_cfg();

		_txrx.state = STATE_STOPPED;
	}
}


void txrx_uninit(void)
{
	if (_txrx.state != STATE_UNINIT)
	{
		buf_close(_txrx.tx_buf);
		buf_close(_txrx.rx_buf);
		IoPinMux_SetPcm(PINMUX_PCM_DEFAULT);
		_txrx.state = STATE_UNINIT;
	}
}


void txrx_hibernation(bool wakeup)
{
	if (wakeup)  {
		txrx_reg_cfg();
	} else {

	}
}


void txrx_start(void)
{
	PR_I("[start] state(%d)! \n", _txrx.state);
	if (_txrx.state == STATE_STOPPED || _txrx.state == STATE_INITED)
	{
		buf_reset(_txrx.tx_buf);
		buf_reset(_txrx.rx_buf);

		_txrx.isr_cnt = 0;
		_txrx.next_rptr = 0;
		txrx_set_nsadr();

		REG_TXRX_ENABLE_TX(true);
		REG_TXRX_ENABLE_RX(true);
		REG_TXRX_ENABLE(true);

		txrx_dump_file_start();
		_txrx.state = STATE_STARTED;
		//txrx_dump_regs();
	}
}


void txrx_stop(void)
{
	PR_I("[stop] state(%d)! \n", _txrx.state);
	if (_txrx.state == STATE_STARTED)
	{
		REG_TXRX_ENABLE_TX(false);
		REG_TXRX_ENABLE_RX(false);
		REG_TXRX_ENABLE(false);
		buf_reset(_txrx.tx_buf);
		buf_reset(_txrx.rx_buf);

		txrx_dump_file_stop();
		_txrx.state = STATE_STOPPED;
	}
}


u32 txrx_read(apcm_buf_t *dst_buf)
{
	u32 copy_size = 0;
	if (_txrx.state == STATE_STARTED) {
		_txrx.rx_buf->wptr = (REG_TXRX_GET_RX_WP - _txrx.phy_addr.rx_sadr) << 4;
		copy_size = buf_copy(dst_buf, _txrx.rx_buf);

		if (_txrx.rx_file) {
			_txrx.rx_dump_buf.wptr = _txrx.rx_buf->wptr;
			file_write(_txrx.rx_file, &_txrx.rx_dump_buf);
		}
	}
	return (copy_size);
}


u32 txrx_write(apcm_buf_t *src_buf)
{
	u32 copy_size = 0;
	if (_txrx.state == STATE_STARTED) {
		copy_size = buf_copy(_txrx.tx_buf, src_buf);

		if (_txrx.tx_file) {
			_txrx.tx_dump_buf.wptr = _txrx.tx_buf->wptr;
			file_write(_txrx.tx_file, &_txrx.tx_dump_buf);
		}
	}
	return (copy_size);
}


void txrx_set_fs(u32 fs)
{
	PR_I("[set_fs] state(%d) fs(%d => %d)\n", _txrx.state, _txrx.cfg.fs, fs);
	if (_txrx.state != STATE_STARTED && (fs == SAMPLE_RATE_16K || fs == SAMPLE_RATE_8K)) {
		_txrx.cfg.fs = fs;
		IoClk_SetPcmMclk(_txrx.cfg.sync_cycle, _txrx.cfg.fs);
	}
}


u32 txrx_get_fs(void)
{
	return (_txrx.cfg.fs);
}


void txrx_set_loop_mode(bool enable)
{
	PR_I("[set_loop_mode] state(%d) enable(%d)\n", _txrx.state, enable);
	if (_txrx.state != STATE_STARTED) {
		_txrx.cfg.hw_mode = (enable) ? PCM_LOOP_MODE : PCM_NORMAL_MODE;
		REG_TXRX_SET_HW_MODE(_txrx.cfg.hw_mode);
	}
}


void txrx_set_int_cfg(u32 int_samples, u32 burst_samples, PFN_TXRX_ISR_CB pfn_cb)
{
	PR_I("[set_int_cfg] state(%d) int_samples(%d) burst_samples(%d)\n",
		_txrx.state, int_samples, burst_samples);

	if (_txrx.state != STATE_STARTED)
	{
		if (_txrx.tx_buf->buf_size %  (int_samples << 1) != 0) {
			PR_E("[set_int_cfg] buf_size(%d) must be n * int_samples(%d) \n",
				_txrx.tx_buf->buf_size, int_samples);
		} else  {
			_txrx.cfg.int_samples = int_samples;
			_txrx.cfg.burst_samples = burst_samples;
			_txrx.cfg.pfb_cb = pfn_cb;
			REG_TXRX_SET_TX_INT_CFG(_txrx.cfg.int_samples, _txrx.cfg.burst_samples);
		}
	}
}


void txrx_dump_regs(void)
{
	PR_I("Dump TxRx Register >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");   \
	PR_I("Mode(%d), HwMode(%d) \n", \
		AUDREG_BITS_R(REGENV_PCM_CTRL, BIT_STR_MODE_SEL, BIT_NUM_MODE_SEL),\
		AUDREG_BITS_R(REGENV_RGBK2_CFG5, BIT_STR_PCM_LOOP_MODE,BIT_NUM_PCM_LOOP_MODE)); \
	PR_I("Sync: Mode(%d) Cycle(%d) Length(%d) \n", \
		AUDREG_BITS_R(REGENV_PCM_CTRL, BIT_STR_SYNC_MODE_SEL, BIT_NUM_SYNC_MODE_SEL), \
		AUDREG_BITS_R(REGENV_PCM_CTRL, BIT_STR_SYNC_CYCLE, BIT_NUM_SYNC_CYCLE), \
		AUDREG_BITS_R(REGENV_PCM_CTRL, BIT_STR_SYNC_LENGTH, BIT_NUM_SYNC_LENGTH)); \
	PR_I("DataFmt: DataOrder(%d) NumSel(%d) 16Bit(%d)\n",
		AUDREG_BITS_R(REGENV_PCM_CTRL, BIT_STR_BIT_DATA_ORDER, BIT_NUM_BIT_DATA_ORDER),
		AUDREG_BITS_R(REGENV_PCM_CTRL, BIT_STR_BIT_NUM_SEL, BIT_NUM_BIT_NUM_SEL),
		AUDREG_BITS_R(REGENV_PCM_CTRL, BIT_STR_PCM_16BIT, BIT_NUM_PCM_16BIT));
	PR_I("TxAdr: SAdr(0x%x) EAdr(0x%x) Blk(0x%x)\n",
		AUDREG_BITS_R(REGENV_PCMTX_DRAM_SADR, BIT_STR_PCMTX_DRAM_SADR, BIT_NUM_PCMTX_DRAM_SADR),
		AUDREG_BITS_R(REGENV_PCMTX_DRAM_EADR, BIT_STR_PCMTX_DRAM_EADR, BIT_NUM_PCMTX_DRAM_EADR),
		AUDREG_BITS_R(REGENV_BT_PCM_BLK_CFG, BIT_STR_PCM_TX_DRAM_BLK, BIT_NUM_PCM_TX_DRAM_BLK));
	PR_I("RxAdr: SAdr(0x%x) EAdr(0x%x) Blk(0x%x) Bank(0x%x)\n",
		AUDREG_BITS_R(REGENV_PCMRX_DRAM_SADR, BIT_STR_PCMRX_DRAM_SADR, BIT_NUM_PCMRX_DRAM_SADR),
		AUDREG_BITS_R(REGENV_PCMRX_DRAM_EADR, BIT_STR_PCMRX_DRAM_EADR, BIT_NUM_PCMRX_DRAM_EADR),
		AUDREG_BITS_R(REGENV_BT_PCM_BLK_CFG, BIT_STR_PCM_RX_DRAM_BLK, BIT_NUM_PCM_RX_DRAM_BLK),
		AUDREG_BITS_R(REGENV_RGBK2_CFG4, BIT_STR_PCM_DRAM_BANK, BIT_NUM_PCM_DRAM_BANK));
	PR_I("TxItr: itr_sz(%d) burst_time(%d) nsadr(%d) \n",
		AUDREG_BITS_R(REGENV_PCMTX_INTR, BIT_STR_TX_SAMPLE_NUM, BIT_NUM_TX_SAMPLE_NUM),
		AUDREG_BITS_R(REGENV_PCMTX_INTR, BIT_STR_TX_INTR_RENUM, BIT_NUM_TX_INTR_RENUM),
		AUDREG_BITS_R(REGENV_PCMTX_DRAM_NSADR, BIT_STR_PCM_TX_DRAM_NSADR, BIT_NUM_PCM_TX_DRAM_NSADR));
	PR_I("Enable: (0x%x)  RxWp(0x%x)\n",
		AUDREG_BITS_R(REGENV_PCM_CTRL, 0, 3),
		AUDREG_READ(REGENV_PCMRX_WRADR));
	PR_I("Dump TxRx Register <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< \n");
}


void txrx_dump_enable(bool enable)
{
	PR_I("[dump_data] %d => %d \n", _txrx.dump_enable, enable);
	_txrx.dump_enable = enable;
	if (_txrx.dump_enable && _txrx.state == STATE_STARTED) {
		txrx_dump_file_start();
	}
}


