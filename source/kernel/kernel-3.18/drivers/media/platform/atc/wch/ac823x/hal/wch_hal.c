/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2016-12-05
 */
#include "wch_drv.h"
#include "wch_log.h"
#include "wch_reg.h"
#include "wch_priv.h"
#ifdef __ARM2__
#include "x_bim_83xx.h"
#endif
#include "wch_hal.h"
static volatile WCH_REG_UNION_T *_prWchHwReg[WCH_NUM] = {
	(WCH_REG_UNION_T *)0, 
	(WCH_REG_UNION_T *)0, 
	(WCH_REG_UNION_T *)0, 
	(WCH_REG_UNION_T *)0,
	(WCH_REG_UNION_T *)0x1005f400,
};/*init for arm2*/
static volatile WCH_REG_UNION_T _rWchSwReg[WCH_NUM];
static unsigned int _fgGetHwReg[WCH_NUM] = {0,};
static unsigned int _regbit[9] = {8, 2, 9, 12, 26, 30, 28, 29, 31};

unsigned long _IO_BASE_ = 0x10000000;/*init for arm2*/

static struct HTOTAL_T HTOTAL[] = {
	{720, 480, 858},/*480*/ /*0*/
	{720, 576, 864},/*576*/ /*1*/
	{800, 480, 953},/*800*480*/ /*2*/
	{800, 600, 1029},/*800*600*/ /*3*/
	{1024, 600, 1344},/*1024*600*/ /*4*/
	{1280, 720, 1650},/*720*/ /*5*/
	{1920, 1080, 2200},/*1080*/ /*6*/
	{1440, 240, 1716},/*720*480i*/ /*7*/
	{1440, 288, 1728},/*720*576i*/ /*8*/
	{1920, 540, 2200},/*1920*1080i*/ /*9*/
	{640, 480, 800},/*480 PC*/ /*10*/
};

static struct HTOTAL_T HTOTAL_YPBPR[] = {
	{1440, 480, 1716},/*480*/ /*0*/
	{1440, 576, 1728},/*576*/ /*1*/
	{800, 480, 953},/*800*480*/ /*2*/
	{800, 600, 1029},/*800*600*/ /*3*/
	{1024, 600, 1344},/*1024*600*/ /*4*/
	{1280, 720, 1650},/*720*/ /*5*/
	{1920, 1080, 2200},/*1080*/ /*6*/
	{1440, 240, 1716},/*720*480i*/ /*7*/
	{1440, 288, 1728},/*720*576i*/ /*8*/
	{1920, 540, 2200},/*1920*1080i*/ /*9*/
};


void GET_WCH_SW_PTR(unsigned char id, WCH_REG_UNION_T **reg)
{
	if (id >=0 && id <= 8 && reg != NULL) {
		*reg = (WCH_REG_UNION_T *) &_rWchSwReg[id];
		if (*reg == NULL)
			WCH_LOG(WCH_LOG_LVL_ERR, "GET_WCH_SW_PTR error! id = %d \n", id);
	}else {
		WCH_LOG(WCH_LOG_LVL_ERR, "GET_WCH_SW_PTR error! id = %d, reg = %p \n", id, reg);
	}
}

void GET_WCH_HW_PTR(unsigned char id, WCH_REG_UNION_T **reg)
{
	if (id >=0 && id <= 8 && reg != NULL) {
		*reg = (WCH_REG_UNION_T *) _prWchHwReg[id];
		if (*reg == NULL)
			WCH_LOG(WCH_LOG_LVL_ERR, "GET_WCH_HW_PTR error! id = %d \n", id);
	}else {
		WCH_LOG(WCH_LOG_LVL_ERR, "GET_WCH_SW_PTR error! id = %d, reg = %p \n", id, reg);
	}
}

void wchEnabelClk(unsigned char u1WchId)
{
	unsigned int regbit = 0;
	unsigned int reg_b4 = 0, reg_d0 = 0;

	if (u1WchId >= WCH_NUM) {
		WCH_LOG(WCH_LOG_LVL_ERR, "error  u1WchId: %d\n", (int)u1WchId);
		return ;
	}
	regbit = _regbit[u1WchId];

	reg_b4 = WCH_HAL_READ32(WCH_IO_BASE + OFFSET_CLOCK_CTL);
	reg_d0 = WCH_HAL_READ32(WCH_IO_BASE + OFFSET_RESET_CTL);

	WCH_HAL_WRITE32(WCH_IO_BASE + OFFSET_CLOCK_CTL, reg_b4 | 0x1 << regbit);
	WCH_HAL_WRITE32(WCH_IO_BASE + OFFSET_RESET_CTL, reg_d0 | 0x1 << regbit);
}

void wchDisabelClk(unsigned char u1WchId)
{
	unsigned int regbit = 0;
	unsigned int reg_b4 = 0, reg_d0 = 0;

	if (u1WchId >= WCH_NUM) {
		WCH_LOG(WCH_LOG_LVL_ERR, "error  u1WchId: %d\n", (int)u1WchId);
		return ;
	}
	regbit = _regbit[u1WchId];

	reg_b4 = WCH_HAL_READ32(WCH_IO_BASE + OFFSET_CLOCK_CTL);
	reg_d0 = WCH_HAL_READ32(WCH_IO_BASE + OFFSET_RESET_CTL);

	WCH_HAL_WRITE32(WCH_IO_BASE + OFFSET_RESET_CTL, reg_d0 & ~(0x1 << regbit)); //reset
	WCH_HAL_WRITE32(WCH_IO_BASE + OFFSET_CLOCK_CTL, reg_b4 & ~(0x1 << regbit)); //disable clock
}

void wchGetHwRegAddress(void)
{
#ifndef __ARM2__
	int i = 0;
	for (i = 0; i < WCH_NUM; i++) {
		_prWchHwReg[i] = (WCH_REG_UNION_T *)wch_sysreg_base[i];
	}
#endif
}

void wchGetHwRegToSw(unsigned char u1WchId)
{
	WCH_REG_UNION_T *prWchSwReg = NULL;
	WCH_REG_UNION_T *prWchHwReg = NULL;
	unsigned int regcnt = 0;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);

	if (_fgGetHwReg[u1WchId] == 0) {
		for (; regcnt < WCH_REG_NUM; regcnt++) {
			prWchSwReg->au4Reg[regcnt] = prWchHwReg->au4Reg[regcnt];
		}
		_fgGetHwReg[u1WchId] = 1;
	}
}

static bool _fgOffClock[WCH_NUM] = {true,true,true,true,true,true,true,true,true};
void wchHalInit(unsigned char u1WchId)
{
	if (_fgOffClock[u1WchId] == true) {
	    wchEnabelClk(u1WchId);
		_fgOffClock[u1WchId] = false;
	}
	wchGetHwRegToSw(u1WchId);
}

void wchHalDeInit(unsigned char u1WchId)
{
	if (_fgOffClock[u1WchId] == false) {
	wchDisabelClk(u1WchId);
		_fgOffClock[u1WchId] = true;
	}
}

void wchHalSetYCAddress(unsigned char u1WchId, unsigned long u8YAddr, unsigned long u8CAddr)
{
	WCH_REG_UNION_T *prWchSwReg = NULL;
	WCH_REG_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_DBG, "wchHalSetYCAddress id = %d, Y = %lx, C = %lx \n", (int)u1WchId,
		   u8YAddr, u8CAddr);
#ifndef __ARM2__
	prWchSwReg->rField.YBUF0_ADDR.YBUF0_ADDR = (unsigned int)(VIRT_TO_BUS(__va(u8YAddr)) >> 2);
	prWchSwReg->rField.CBUF0_ADDR.CBUF0_ADDR = (unsigned int)(VIRT_TO_BUS(__va(u8CAddr)) >> 2);
#else//yzq to do
	prWchSwReg->rField.YBUF0_ADDR.YBUF0_ADDR = (unsigned int)(u8YAddr) >> 2;
	prWchSwReg->rField.CBUF0_ADDR.CBUF0_ADDR = (unsigned int)(u8CAddr) >> 2;
#endif
	prWchHwReg->au4Reg[IDX_YBUF0_ADDR] = prWchSwReg->au4Reg[IDX_YBUF0_ADDR];
	prWchHwReg->au4Reg[IDX_CBUF0_ADDR] = prWchSwReg->au4Reg[IDX_CBUF0_ADDR];
}

void wchHalTopInit(unsigned char u1WchId, WCH_DATA_SRC_E eSrcType)
{
	switch (u1WchId) {
	case WCH_1:
		if (eSrcType == DATA_SRC_BT656_601) {
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_0, WCH_HAL_READ32(TVD_WCH_CONTROL_0) | r_dgi_2_clk_sel);
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_2, WCH_HAL_READ32(TVD_WCH_CONTROL_2) | r_tvd0_dgi_sel);
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, WCH_HAL_READ32(TVD_WCH_CONTROL_3) | r_tvd0_dgi_clk_sel);
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_1, WCH_HAL_READ32(TVD_WCH_CONTROL_1) & (~r_dgi2_clk_sel));
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_5, WCH_HAL_READ32(TVD_WCH_CONTROL_5) | (dclk_in_2|h_v_sync_in_2|data_in_2));
		} else if (eSrcType == DATA_SRC_TVD0) {
			/*write channel0 choose tvd data*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_2, WCH_HAL_READ32(TVD_WCH_CONTROL_2) & (~(r_tvd0_dgi_sel)));
			/*write channel0 choose tvd clock*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd0_dgi_clk_sel)));
			/*write channel0 choose tvd0 clock*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd0_clk_sel)));
		} else {
			WCH_LOG(WCH_LOG_LVL_ERR, "wchHalTopInit error u1WchId = %d eSrcType = %d \n",u1WchId, eSrcType);
		}
		break;
	case WCH_2:
		/*write channel1 choose tvd1 clock*/
		WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, (WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd1_clk_sel))) | (0x1 << 10));
		break;
	case WCH_3:
		/*write channel2 choose tvd2 clock*/
		WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, (WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd2_clk_sel))) | (0x2 << 12)); 
		break;
	case WCH_4:
		/*write channel3 choose tvd3 clock*/
		WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, (WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd3_clk_sel))) | (0x3 << 14));
		break;
	case WCH_5:
		if (eSrcType == DATA_SRC_TVD0) {
			/*write channel4 choose tvd0 clock*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, (WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd4_clk_sel))) | (0x0 << 16));
			/*write channel4 choose tvd0 data*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_4, (WCH_HAL_READ32(TVD_WCH_CONTROL_4) & (~(r_tvd4_sel)))|(0x0<<8));
		} else if (eSrcType == DATA_SRC_TVD1){
			/*write channel4 choose tvd1 clock*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, (WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd4_clk_sel))) | (0x1 << 16));
			/*write channel4 choose tvd1 data*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_4, (WCH_HAL_READ32(TVD_WCH_CONTROL_4) & (~(r_tvd4_sel)))|(0x1<<8));
		} else if (eSrcType == DATA_SRC_TVD2){
			/*write channel4 choose tvd2 clock*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, (WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd4_clk_sel))) | (0x2 << 16));
			/*write channel4 choose tvd2 data*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_4, (WCH_HAL_READ32(TVD_WCH_CONTROL_4) & (~(r_tvd4_sel)))|(0x2<<8));
		} else if (eSrcType == DATA_SRC_TVD3){
			/*write channel4 choose tvd3 clock*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_3, (WCH_HAL_READ32(TVD_WCH_CONTROL_3) & (~(r_tvd4_clk_sel))) | (0x3 << 16));
			/*write channel4 choose tvd3 data*/
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_4, (WCH_HAL_READ32(TVD_WCH_CONTROL_4) & (~(r_tvd4_sel)))|(0x3<<8));
		} else {
			WCH_LOG(WCH_LOG_LVL_ERR, "wchHalTopInit error u1WchId = %d eSrcType = %d \n",u1WchId, eSrcType);
		}
		break;
	case WCH_8:
		if (eSrcType == DATA_SRC_BT656_601) {
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_0, WCH_HAL_READ32(TVD_WCH_CONTROL_0) & (~(r_dgi_1_clk_sel)));
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_1, WCH_HAL_READ32(TVD_WCH_CONTROL_1) & (~(r_dgi_clk_sel)));
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_5, WCH_HAL_READ32(TVD_WCH_CONTROL_5) | (dclk_in_1|h_v_sync_in_1|data_in_1));
		} else if (eSrcType == DATA_SRC_BT1120) {
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_0, WCH_HAL_READ32(TVD_WCH_CONTROL_0) & (~(r_dgi_1_clk_sel)));
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_1, WCH_HAL_READ32(TVD_WCH_CONTROL_1) & (~(r_dgi_clk_sel)));
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_0, WCH_HAL_READ32(TVD_WCH_CONTROL_0) & (~(r_dgi_2_clk_sel)));
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_5, WCH_HAL_READ32(TVD_WCH_CONTROL_5) | (dclk_in_2|h_v_sync_in_2|data_in_2));
			WCH_HAL_WRITE32(TVD_WCH_CONTROL_5, WCH_HAL_READ32(TVD_WCH_CONTROL_5) | (dclk_in_2|h_v_sync_in_2|data_in_2));
		} else {
			WCH_LOG(WCH_LOG_LVL_ERR, "wchHalTopInit error u1WchId = %d eSrcType = %d \n",u1WchId, eSrcType);
		}
		break;
	case WCH_6:
	case WCH_7:
	case WCH_9:
		break;
	default :
		WCH_LOG(WCH_LOG_LVL_ERR, "wchHalTopInit error not such u1WchId = %d \n", u1WchId);
		break;
	}
}
void wchHalVoutTopInit(WCH_VOUT_SRC_E eSrcType)
{

}

void wchHalSetInput(WCH_INPUT_INFO_T * pInputInfo)
{
	WCH_REG_UNION_T *prWchSwReg = NULL;
	WCH_REG_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(pInputInfo->u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(pInputInfo->u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL,
		"WchHalSetInput u1WchId=%d,eSrcType=%d,eSrcFmt=%d,u4SrcWidth=%d,u4SrcHeight=%d,u4StartX=%d,u4StartYTop=%d,u4StartYBot=%d,fgProgressive=%d\n", 
			pInputInfo->u1WchId, pInputInfo->eSrcType, pInputInfo->eSrcFmt, pInputInfo->u4SrcWidth,
			pInputInfo->u4SrcHeight, pInputInfo->u4StartX, pInputInfo->u4StartYTop, pInputInfo->u4StartYBot, pInputInfo->fgProgressive);


	prWchSwReg->rField.VDOIN_EN.SRAM_EN_SEL = 1;
	prWchSwReg->rField.ACT_LINE.BGVSYNC601DET= 0x1;

	prWchSwReg->rField.VSCALE.fld_reset = 1;
	prWchSwReg->rField.VSCALE.Vsyn_reset = 1;
	prWchSwReg->rField.VSCALE.Vdifld_reset = 0;
	prWchSwReg->rField.VSCALE.Avge_mode = 0x0;
	if (pInputInfo->eSrcType == DATA_SRC_TVD0 || pInputInfo->eSrcType == DATA_SRC_TVD1 ||
		pInputInfo->eSrcType == DATA_SRC_TVD2 || pInputInfo->eSrcType == DATA_SRC_TVD3) {
		prWchSwReg->rField.INPUT_CTRL.new_interrupt_en = 0;
		prWchSwReg->rField.VSCALE.Fld_int = 0;
		prWchSwReg->rField.VSCALE.Vsysn_int = 0;
		prWchSwReg->rField.VSCALE.Vdifld_int = 0;
		prWchSwReg->rField.VSCALE.VSCALE_reserved1 = 1;
		prWchSwReg->rField.REQ_OUT.h_dis_end_sel = 1;
		prWchSwReg->rField.REQ_OUT.h_dis_act_sel = 1;	
		//prWchSwReg->rField.REQ_OUT.ext_field = 0;
		prWchSwReg->rField.REQ_OUT.index_mode = 1;
	} else {
		prWchSwReg->rField.INPUT_CTRL.new_interrupt_en = 1;
		prWchSwReg->rField.VSCALE.Fld_int = 1;
		prWchSwReg->rField.VSCALE.Vsysn_int = 1;
		prWchSwReg->rField.VSCALE.Vdifld_int = 1;
		prWchSwReg->rField.REQ_OUT.h_dis_end_sel = 0;
		prWchSwReg->rField.REQ_OUT.h_dis_act_sel = 0;	
		//prWchSwReg->rField.REQ_OUT.ext_field = 0;
		prWchSwReg->rField.REQ_OUT.index_mode = 0;	
	}
	//prWchSwReg->rField.VSCALE.Vsynccena = 0;
	//prWchSwReg->rField.VSCALE.Int720p = 0;
	//prWchSwReg->rField.VSCALE.Linest_ena = 0;
	//prWchSwReg->rField.VSCALE.bgaddr_sel = 0;
	prWchSwReg->rField.VSCALE.bgsramcs_ena = 1;
	//prWchSwReg->rField.VSCALE.bgtest_bar = 0;
	prWchSwReg->rField.VSCALE.Vdifld_int2 = 1;
	//prWchSwReg->rField.REQ_OUT.ext_field = 1;/*hdmi set to be 0*/

	prWchSwReg->rField.VSCALE.bghsize_dw = (pInputInfo->u4SrcWidth >> 4); //for tvd 1 2 3 

	switch (pInputInfo->u1WchId) {
	case WCH_1:
		if (pInputInfo->eSrcFmt == DATA_FMT_BT656 || pInputInfo->eSrcFmt == DATA_FMT_BT601) {/*DGI*/
			prWchSwReg->rField.VDOIN_EN.BGVdoformat = 0;/*separate_sync = 0*/

			prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 1;/*yc_mux = 1*/
			prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 1;
			prWchSwReg->rField.INPUT_CTRL.vdoin_CB_chn_sel = 0x0;
			prWchSwReg->rField.INPUT_CTRL.vdoin_CR_chn_sel = 0x0;

			prWchSwReg->rField.VDOIN_EN.VDOIN_EN_halfsample = 0;/*full mode*/
			prWchSwReg->rField.VDOIN_EN.Field_inv = 1;
			prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_halfsample = 0;
		} else if (pInputInfo->eSrcFmt == DATA_FMT_YUV444){/*TVD0*/
			prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 0;
			prWchSwReg->rField.VDOIN_EN.BGenVsyn601Inv = 1;
			prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 0;
			prWchSwReg->rField.INPUT_CTRL.vdoin_CB_chn_sel = 0x1;
			prWchSwReg->rField.INPUT_CTRL.vdoin_CR_chn_sel = 0x2;
			prWchSwReg->rField.VDOIN_EN.FldInv = 1;

#if WCH_SUPPORT_AVM_480P
			if (pInputInfo->fgSupportAVM480P) {
				WCH_LOG(WCH_LOG_LVL_INFO, "wchHalTopInit wch1 avm 480p \n");
				prWchSwReg->rField.VDOIN_EN.VDOIN_EN_halfsample = 0;/*full mode*/
				prWchSwReg->rField.VDOIN_EN.Field_inv = 1;
				prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_halfsample = 0;
			} else 
#endif
			{
				if (SRC_APP_BACKCAR_WCH1 == pInputInfo->eWchSrcId) {
					prWchSwReg->rField.VDOIN_EN.VDOIN_EN_halfsample = 0;/*full mode*/
					prWchSwReg->rField.VDOIN_EN.Field_inv = 1;
					prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_halfsample = 0;
				} else {
					prWchSwReg->rField.VDOIN_EN.VDOIN_EN_halfsample = 1;/*half mode*/
					prWchSwReg->rField.VDOIN_EN.Field_inv = 1;
					prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_halfsample = 1;
					prWchSwReg->rField.VSCALE.bghsize_dw = (pInputInfo->u4SrcWidth >> 4) + 1; //for tvd 0
				}
			}
		}
		break;
	case WCH_2:
	case WCH_3:
	case WCH_4:
		prWchSwReg->rField.VDOIN_EN.BGenVsyn601Inv = 1;
		prWchSwReg->rField.VDOIN_EN.FldInv = 1;
#if WCH_SUPPORT_AVM_480P
		if (pInputInfo->fgSupportAVM480P) {
			WCH_LOG(WCH_LOG_LVL_INFO, "wchHalTopInit wch%d avm 480p \n", (unsigned int)pInputInfo->u1WchId + 1);
			prWchSwReg->rField.VDOIN_EN.VDOIN_EN_halfsample = 1;
			prWchSwReg->rField.VDOIN_EN.Field_inv = 1;
			prWchSwReg->rField.VDOIN_EN.VDOIN_EN_reserved3 = 0x2;
			prWchSwReg->rField.VDOIN_EN.VDOIN_EN_reserved1 = 0x1;
			prWchSwReg->rField.VDOIN_EN.Fld0dis = 0;
			prWchSwReg->rField.VDOIN_EN.Fld1dis = 1;
			prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_halfsample = 1;
			prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_reserved2 = 0xb;//?
		} else 
#endif
		{
			prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_reserved2 = 0x8;//?
			prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_reserved3 = 0xa0;//?

			prWchSwReg->rField.VDOIN_EN.VDOIN_EN_halfsample = 1;/*only half mode*/
			prWchSwReg->rField.VDOIN_EN.Field_inv = 1;
			prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_halfsample = 1;
		}
		break;
	case WCH_5:
		prWchSwReg->rField.VDOIN_EN.BGenVsyn601Inv = 1;
		prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_reserved2 = 0x8;//?
		prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_reserved3 = 0xa0;//?

		prWchSwReg->rField.VDOIN_EN.VDOIN_EN_halfsample = 0;/*only full mode*/
		prWchSwReg->rField.VDOIN_EN.Field_inv = 1;
		prWchSwReg->rField.VDOIN_MODE.VDOIN_MODE_halfsample = 0;
		prWchSwReg->rField.VDOIN_EN.FldInv = 1;
#if WCH_OPEN_PATTEN
		prWchSwReg->rField.VSCALE.bgtest_bar = 1; 
		prWchSwReg->rField.REQ_CTL.CH_VRST_EN = 0; 
#endif
		break;
	case WCH_6:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 1;/*separate_sync = 1*/	
		prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 0;/*yc_mux = 0*/
		prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 0;
		prWchSwReg->rField.HPIXEL.NPIXEL_PRE = 0;
		prWchSwReg->rField.INPUT_CTRL.vdoin_Y_chn_sel = pInputInfo->u1YSel;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CB_chn_sel = pInputInfo->u1USel;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CR_chn_sel = pInputInfo->u1VSel;
		prWchSwReg->rField.INPUT_CTRL.BGenVsyn_Inv = 0;
		prWchSwReg->rField.INPUT_CTRL.BGenHsyn_Inv = 0;
		prWchSwReg->rField.INPUT_CTRL.new_interrupt_en = 0;
		break;
	case WCH_7:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 1;

		prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 0;
		prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 0;
		prWchSwReg->rField.INPUT_CTRL.vdoin_Y_chn_sel = pInputInfo->u1YSel;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CB_chn_sel = pInputInfo->u1USel;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CR_chn_sel = pInputInfo->u1VSel;
		prWchSwReg->rField.INPUT_CTRL.Cin_del_sel = 3;
		prWchSwReg->rField.INPUT_CTRL.YCin_del_sel_bit_2 = 1;
		break;
	case WCH_8:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 0;

		prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 1;
		prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 1;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CB_chn_sel = 0x0;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CR_chn_sel = 0x0;
		break;
	case WCH_9:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 1;

		prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 0;
		prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 0;
		break;
	default :
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetInput no such id = %d,  \n", (int)pInputInfo->u1WchId);
	}

	switch (pInputInfo->eSrcFmt) {
	case DATA_FMT_YUV422:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 1;
		//prWchSwReg->rField.VDOIN_EN.BGenVsyn601Inv = 1;
		prWchSwReg->rField.INPUT_CTRL.vdoin_444_mode = 0;
		break;
	case DATA_FMT_YUV444:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 1;
		//prWchSwReg->rField.VDOIN_EN.BGenVsyn601Inv = 1;
		prWchSwReg->rField.INPUT_CTRL.vdoin_444_mode = 1;
		break;
	case DATA_FMT_BT601:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 1;
		prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 1;/*yc_mux = 1*/
		prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 1;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CB_chn_sel = 0x0;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CR_chn_sel = 0x0;
		break;
	case DATA_FMT_BT656:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 0;
		prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 1;/*yc_mux = 1*/
		prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 1;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CB_chn_sel = 0x0;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CR_chn_sel = 0x0;
		break;
	case DATA_FMT_BT1120:
		prWchSwReg->rField.VDOIN_EN.BGVdoformat = 0;
		prWchSwReg->rField.VDOIN_EN.sd_2fs_input = 0;/*yc_mux = 0*/
		prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 0;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CB_chn_sel = 0x1;
		prWchSwReg->rField.INPUT_CTRL.vdoin_CR_chn_sel = 0x2;
		break;
	default :
		break;
	}

	prWchSwReg->rField.HPIXEL.NPIXEL = pInputInfo->u4StartX;
	prWchSwReg->rField.TOP_BOT_START_LIN.BL = pInputInfo->u4StartYBot;
	prWchSwReg->rField.TOP_BOT_START_LIN.TL = pInputInfo->u4StartYTop;

	if (pInputInfo->eSrcFmt == DATA_FMT_BT601 && pInputInfo->u4SrcHeight == 480) {
		if (pInputInfo->fgProgressive == 1) {/*0x8401_0000*/
			prWchSwReg->rField.INPUT_CTRL.BGenHsyn_Inv = 1;
			prWchSwReg->rField.INPUT_CTRL.new_interrupt_en = 1;
			prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 1;
		} else {
			prWchSwReg->rField.INPUT_CTRL.BGenHsyn_Inv = 1;
		}
	} else if (pInputInfo->eSrcFmt == DATA_FMT_BT656 && pInputInfo->u4SrcHeight == 480) {
		if (pInputInfo->fgProgressive == 1) {/*0x8401_0000*/
			prWchSwReg->rField.INPUT_CTRL.BGenHsyn_Inv = 1;
			prWchSwReg->rField.INPUT_CTRL.new_interrupt_en = 1;
			prWchSwReg->rField.INPUT_CTRL.sd_480i_mix_eco = 1;
		} else {
			prWchSwReg->rField.VDOIN_EN.FldInv = 0;
		}
	}

	if (pInputInfo->fgProgressive == 1) {
		prWchSwReg->rField.VDOIN_EN.PRGS = 1;
		prWchSwReg->rField.ACT_LINE.ACTLINE = pInputInfo->u4SrcHeight - 1;
	} else {
		prWchSwReg->rField.VDOIN_EN.PRGS = 0;
		if (DATA_SRC_HDMI == pInputInfo->eSrcType) {
			prWchSwReg->rField.VDOIN_EN.FldInv = 1;
			prWchSwReg->rField.ACT_LINE.ACTLINE = pInputInfo->u4SrcHeight - 1;
		} else {
			prWchSwReg->rField.ACT_LINE.ACTLINE = (pInputInfo->u4SrcHeight >> 1) - 1;
		}
	}

	switch (pInputInfo->eSrcType) {/*yc mux*/
	case DATA_SRC_BT656_601:
		prWchSwReg->rField.HCNT_SETTING.HACTCNT = pInputInfo->u4SrcWidth << 1;
		if (pInputInfo->u4SrcHeight == 480) {
			prWchSwReg->rField.HCNT_SETTING.HCNT = (HTOTAL[0].htotal - 1) << 1;
		}else if (pInputInfo->u4SrcHeight == 576){
			prWchSwReg->rField.HCNT_SETTING.HCNT = (HTOTAL[1].htotal - 1) << 1;
		}
		break;
	case DATA_SRC_BT1120:
		prWchSwReg->rField.HCNT_SETTING.HACTCNT = pInputInfo->u4SrcWidth;
		/*to do*//*base on source*/
		break;
	case DATA_SRC_TVD0:
	case DATA_SRC_TVD1:
	case DATA_SRC_TVD2:
	case DATA_SRC_TVD3:
		prWchSwReg->rField.HCNT_SETTING.HACTCNT = pInputInfo->u4SrcWidth;
		if (pInputInfo->u4SrcHeight == 480){/*NTSC*/
			prWchSwReg->rField.HCNT_SETTING.HCNT = TVDHTOTAL[1];
		} else if (pInputInfo->u4SrcHeight == 576){/*PAL*/
			prWchSwReg->rField.HCNT_SETTING.HCNT = TVDHTOTAL[0];
		}
		break;
	case DATA_SRC_YPBPR:
	case DATA_SRC_VGA:
	{
		int i = 0;
		prWchSwReg->rField.HCNT_SETTING.HACTCNT = pInputInfo->u4SrcWidth;
		
		for (i = 0; i < sizeof(HTOTAL_YPBPR)/sizeof(struct HTOTAL_T); i++) {
			if (pInputInfo->u4SrcWidth == HTOTAL_YPBPR[i].u4SrcWidth ) {
				if ((pInputInfo->fgProgressive && pInputInfo->u4SrcHeight == HTOTAL_YPBPR[i].u4SrcHeight) ||
					(!pInputInfo->fgProgressive && pInputInfo->u4SrcHeight/2 == HTOTAL_YPBPR[i].u4SrcHeight)) {
					
					WCH_LOG(WCH_LOG_LVL_HAL, "WchHalSetInput rField, u4SrcWidth: %d,  u4SrcHeight: %d, HTOTAL[%d].htotal: %d\n", 
						pInputInfo->u4SrcWidth, pInputInfo->u4SrcHeight, i, HTOTAL_YPBPR[i].htotal);
					
					prWchSwReg->rField.HCNT_SETTING.HCNT = HTOTAL_YPBPR[i].htotal - 1;
					break;
				}
			}
		}
		if (sizeof(HTOTAL_YPBPR)/sizeof(struct HTOTAL_T) == i) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetInput unkown source size\n");
		}
		break;
	}

	case DATA_SRC_VOUT:
	case DATA_SRC_HDMI:
	{
		int i = 0;
		prWchSwReg->rField.HCNT_SETTING.HACTCNT = pInputInfo->u4SrcWidth;
		for (i = 0; i < sizeof(HTOTAL)/sizeof(struct HTOTAL_T); i++) {
			if (pInputInfo->u4SrcWidth == HTOTAL[i].u4SrcWidth &&
				pInputInfo->u4SrcHeight == HTOTAL[i].u4SrcHeight) {
				
				WCH_LOG(WCH_LOG_LVL_HAL, "WchHalSetInput rField, u4SrcWidth: %d,  u4SrcHeight: %d, HTOTAL[%d].htotal: %d\n", 
					pInputInfo->u4SrcWidth, pInputInfo->u4SrcHeight, i, HTOTAL[i].htotal);
				
				prWchSwReg->rField.HCNT_SETTING.HCNT = HTOTAL[i].htotal - 1;
				break;
			}
		}
		if (sizeof(HTOTAL)/sizeof(struct HTOTAL_T) == i) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetInput unkown source size\n");
		}
		break;
	}
	
	default :
		break;
	}

	prWchSwReg->rField.HCNT_SETTING_1.CHCNT = (pInputInfo->u4SrcWidth >> 4) - 1;
	prWchSwReg->rField.HCNT_SETTING_1.YHCNT = (pInputInfo->u4SrcWidth >> 4) - 1;

	if (pInputInfo->eSrcType == DATA_SRC_HDMI || 
		pInputInfo->eSrcType == DATA_SRC_BT1120 || 
		pInputInfo->eSrcType == DATA_SRC_BT656_601) {
		prWchSwReg->rField.REQ_OUT.ext_field = 0;
	}else {
		prWchSwReg->rField.REQ_OUT.ext_field = 1;
	}

	prWchHwReg->au4Reg[IDX_VDOIN_EN] = prWchSwReg->au4Reg[IDX_VDOIN_EN];
	prWchHwReg->au4Reg[IDX_VDOIN_MODE] = prWchSwReg->au4Reg[IDX_VDOIN_MODE];
	prWchHwReg->au4Reg[IDX_ACT_LINE] = prWchSwReg->au4Reg[IDX_ACT_LINE];
	prWchHwReg->au4Reg[IDX_HPIXEL] = prWchSwReg->au4Reg[IDX_HPIXEL];
	prWchHwReg->au4Reg[IDX_TOP_BOT_START_LINE] = prWchSwReg->au4Reg[IDX_TOP_BOT_START_LINE];
	prWchHwReg->au4Reg[IDX_INPUT_CTRL] = prWchSwReg->au4Reg[IDX_INPUT_CTRL];
	prWchHwReg->au4Reg[IDX_HCNT_SETTING] = prWchSwReg->au4Reg[IDX_HCNT_SETTING];
	prWchHwReg->au4Reg[IDX_HCNT_SETTING_1] = prWchSwReg->au4Reg[IDX_HCNT_SETTING_1];
	prWchHwReg->au4Reg[IDX_VSCALE] = prWchSwReg->au4Reg[IDX_VSCALE];
	prWchHwReg->au4Reg[IDX_REQ_OUT] = prWchSwReg->au4Reg[IDX_REQ_OUT];
}

void wchHalSetOutput(WCH_OUTPUT_INFO_T * pOutputInfo)
{
	WCH_REG_UNION_T *prWchSwReg = NULL;
	WCH_REG_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(pOutputInfo->u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(pOutputInfo->u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL,
		"WchHalSetOutput id=%d,eDstFmt=%d,u4ScanLineMode=%d,fgProgressive=%d,u4DstWidth=%d,u4DstHeight=%d \n",
			pOutputInfo->u1WchId, pOutputInfo->eDstFmt, pOutputInfo->u4ScanLineMode, pOutputInfo->fgProgressive,
			pOutputInfo->u4DstWidth, pOutputInfo->u4DstHeight);

	if (pOutputInfo->u4ScanLineMode)
		prWchSwReg->rField.VDOIN_EN.Linear_ena = 1;
	else
		prWchSwReg->rField.VDOIN_EN.Linear_ena = 0;

	switch (pOutputInfo->eDstFmt) {
	case DATA_FMT_YUV422:
		prWchSwReg->rField.VDOIN_EN.mode_422 = 1;
		if (pOutputInfo->fgProgressive == 1) {
			prWchSwReg->rField.DW_NEED.DW_NEED_C_LINE = pOutputInfo->u4DstHeight - 1;
			prWchSwReg->rField.DW_NEED.DW_NEED_Y_LINE = pOutputInfo->u4DstHeight - 1;
		} else {
			prWchSwReg->rField.DW_NEED.DW_NEED_C_LINE = (pOutputInfo->u4DstHeight >> 1) - 1;
			prWchSwReg->rField.DW_NEED.DW_NEED_Y_LINE = (pOutputInfo->u4DstHeight >> 1) - 1;
		}
		break;
	case DATA_FMT_YUV420:
		prWchSwReg->rField.VDOIN_EN.mode_422 = 0;
		if (pOutputInfo->fgProgressive == 1) {
			prWchSwReg->rField.DW_NEED.DW_NEED_C_LINE = (pOutputInfo->u4DstHeight >> 1) - 1;
			prWchSwReg->rField.DW_NEED.DW_NEED_Y_LINE = pOutputInfo->u4DstHeight - 1;
		} else {
			prWchSwReg->rField.DW_NEED.DW_NEED_C_LINE = (pOutputInfo->u4DstHeight >> 2) - 1;
			prWchSwReg->rField.DW_NEED.DW_NEED_Y_LINE = (pOutputInfo->u4DstHeight >> 1) - 1;
		}
		break;
	default:
		WCH_LOG(WCH_LOG_LVL_ERR,
			"WchHalSetOutput unkown output fmt eDstFmt=%d \n", pOutputInfo->eDstFmt);
		break;
	}

	prWchHwReg->au4Reg[IDX_VDOIN_EN] = prWchSwReg->au4Reg[IDX_VDOIN_EN];
	prWchHwReg->au4Reg[IDX_DW_NEED] = prWchSwReg->au4Reg[IDX_DW_NEED];
}

void wchHalStart(unsigned char u1WchId)
{
	WCH_REG_UNION_T *prWchSwReg = NULL;
	WCH_REG_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalStart id = %d,  \n", (int)u1WchId);

	prWchSwReg->rField.REQ_CTL.CH_REQ_EMPTY = 1;
	prWchHwReg->au4Reg[IDX_REQ_CTL] = prWchSwReg->au4Reg[IDX_REQ_CTL];
	prWchSwReg->rField.REQ_OUT.CH_PROTECT_DISABLE = 1;
	prWchHwReg->au4Reg[IDX_REQ_OUT] = prWchSwReg->au4Reg[IDX_REQ_OUT];

	prWchSwReg->rField.VDOIN_EN.VI_EN = 1;
	prWchHwReg->au4Reg[IDX_VDOIN_EN] = prWchSwReg->au4Reg[IDX_VDOIN_EN];

	prWchSwReg->rField.WRAPPER_3D_SETTING.swrst = 0x3;
	prWchHwReg->au4Reg[IDX_WRAPPER_3D_SETTING] = prWchSwReg->au4Reg[IDX_WRAPPER_3D_SETTING];
	prWchSwReg->rField.WRAPPER_3D_SETTING.swrst = 0x0;
	prWchHwReg->au4Reg[IDX_WRAPPER_3D_SETTING] = prWchSwReg->au4Reg[IDX_WRAPPER_3D_SETTING];

	/*After open shadow, writing register will take effect after the next interrupt, so open it at the start end*/
	prWchSwReg->rField.REQ_CTL.BGYC_LATCH_MODE = 0x1;
	prWchHwReg->au4Reg[IDX_REQ_CTL] = prWchSwReg->au4Reg[IDX_REQ_CTL];
}

void wchHalStop(unsigned char u1WchId)
{
	WCH_REG_UNION_T *prWchSwReg = NULL;
	WCH_REG_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalStop id = %d,  \n", (int)u1WchId);

	/*We should close shadow firt, than writing other registers will take effect immediately*/
	prWchSwReg->rField.REQ_CTL.BGYC_LATCH_MODE = 0x2;
	prWchHwReg->au4Reg[IDX_REQ_CTL] = prWchSwReg->au4Reg[IDX_REQ_CTL];

	prWchSwReg->rField.WRAPPER_3D_SETTING.swrst = 0x3;
	prWchHwReg->au4Reg[IDX_WRAPPER_3D_SETTING] = prWchSwReg->au4Reg[IDX_WRAPPER_3D_SETTING];
	prWchSwReg->rField.VDOIN_EN.VI_EN = 0;
	prWchHwReg->au4Reg[IDX_VDOIN_EN] = prWchSwReg->au4Reg[IDX_VDOIN_EN];
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalStop OK\n");
}

