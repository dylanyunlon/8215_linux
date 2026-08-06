/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2016-12-05
 */

 #ifndef _WCH_REG_H_
 #define _WCH_REG_H_

#ifndef __ARM2__
#include <linux/io.h>/*__va,__pa*/
#endif

#define IDX_VDOIN_EN			(0x00/4)
#define IDX_VDOIN_MODE			(0x04/4)
#define IDX_YBUF0_ADDR			(0x08/4)
#define IDX_ACT_LINE			(0x0c/4)
#define IDX_CBUF0_ADDR			(0x10/4)
#define IDX_DW_NEED				(0x14/4)
#define IDX_HPIXEL				(0x18/4)
#define IDX_HBLACK				(0x1c/4)
#define IDX_INPUT_CTRL			(0x20/4)
#define IDX_TOP_BOT_START_LINE	(0x24/4)
#define IDX_WRAPPER_3D			(0x28/4)
#define IDX_WRAPPER_3D_VSYNC	(0x2c/4)
#define IDX_WRAPPER_3D_SETTING	(0x30/4)
#define IDX_HCNT_SETTING		(0x34/4)
#define IDX_HCNT_SETTING_1		(0x38/4)
#define IDX_VSCALE				(0x3c/4)
#define IDX_HSCALE				(0x40/4)
#define IDX_DEBUG_PORT			(0x44/4)
#define IDX_DEBUG_PORT_1		(0x48/4)
#define IDX_DEBUG_PORT_3		(0x4c/4)
#define IDX_INDEBUG_STATUS		(0x50/4)
#define IDX_DEBUG_PORT_4		(0x58/4)
#define IDX_DEBUG_PORT_5		(0x5C/4)
#define IDX_DRAM_BYTE_EN		(0x60/4)
#define IDX_YBUF1_ADDR			(0x64/4)
#define IDX_CBUF1_ADDR			(0x68/4)
#define IDX_YBUF2_ADDR			(0x6c/4)
#define IDX_CBUF2_ADDR			(0x70/4)
#define IDX_YBUF3_ADDR			(0x74/4)
#define IDX_CBUF3_ADDR			(0x78/4)
#define IDX_REQ_CTL				(0x7c/4)
#define IDX_REQ_OUT				(0x80/4)
#define IDX_DRAM_PROTECT3		(0x84/4)
#define IDX_DRAM_PROTECT4		(0x88/4)

#define OFFSET_CLOCK_CTL		(0xb4)
#define OFFSET_RESET_CTL		(0xd0)

struct VDOIN_EN_REG {/* 0x00 */
/*0*/	 unsigned int VI_EN : 1;
/*1*/	 unsigned int Vdoindata_test : 1;
/*2*/	 unsigned int BGVdoformat : 1;
/*3*/	 unsigned int PRGS : 1;
/*4*/	 unsigned int SRAM_EN_SEL : 1;
/*5*/	 unsigned int Vdoindata_sel : 1;
/*6*/	 unsigned int ExtEAv : 1;
	 	 unsigned int VDOIN_EN_reserved1 : 1;
/*9:8*/	 unsigned int REV : 2;
/*10*/	 unsigned int BGenVsyn601Inv : 1;
/*11*/	 unsigned int sd_2fs_input : 1;
/*12*/	 unsigned int FldInv : 1;
	 	 unsigned int VDOIN_EN_reserved2 : 1;
/*14*/	 unsigned int Linear_ena : 1;
/*15*/	 unsigned int addr_swap_1 : 1;
	 	 unsigned int VDOIN_EN_reserved3 : 5;
/*21*/ 	 unsigned int Internal_data_ena : 1;
	 	 unsigned int VDOIN_EN_reserved4 : 1;
/*24:23*/unsigned int SWAP : 2;
/*25*/	 unsigned int mode_422 : 1;
/*26*/	 unsigned int Fram_clr : 1;
/*27*/	 unsigned int Fld0dis : 1;
/*28*/	 unsigned int Fld1dis : 1;
/*29*/	 unsigned int BGCCIREna : 1;
/*30*/	 unsigned int VDOIN_EN_halfsample : 1;
/*31*/	 unsigned int Field_inv : 1;
};

struct VDOIN_MODE_REG {/* 0x04 */
	 	 unsigned int VDOIN_MODE_reserved1 : 3;
/*3*/	 unsigned int DRAMClk_On : 1;
/*6:4*/	 unsigned int Fifo_THRES : 3;
	 	 unsigned int VDOIN_MODE_reserved2 : 4;
/*15:11*/unsigned int bgb8105a_mp : 5;
	 	 unsigned int VDOIN_MODE_reserved3 : 15;
/*31*/	 unsigned int VDOIN_MODE_halfsample : 1;
};

struct YBUF0_ADDR_REG {/* 0x08 */
/*29:0*/	 unsigned int YBUF0_ADDR : 30;
	 unsigned int YBUF0_ADDR_reserved4 : 2;
};

struct ACT_LINE_REG {/* 0x0C */
/*11:0*/	 unsigned int ACTLINE : 12;
/*18:12*/unsigned int BGVSYNC601DET : 7;
	 	 unsigned int ACT_LINE_reserved1 : 13;
};

struct CBUF0_ADDR_REG {/* 0x10 */
/*29:0*/	 unsigned int CBUF0_ADDR : 30;
	 	 unsigned int CBUF0_ADDR_reserved1 : 2;
};

struct DW_NEED_REG {/* 0x14 */
/*11:0*/	 unsigned int DW_NEED_Y_LINE : 12;
	 	 unsigned int DW_NEED_reserved1 : 4;
/*27:16*/unsigned int DW_NEED_C_LINE : 12;
	 	 unsigned int DW_NEED_reserved2 : 4;
};

struct HPIXEL_REG {/* 0x18 */
/*12:0*/	 unsigned int NPIXEL : 13;
	 	 unsigned int HPIXEL_reserved1 : 3;
/*28:16*/unsigned int NPIXEL_PRE : 13;
	 	 unsigned int HPIXEL_reserved2 : 3;
};

struct HBLACK_REG {/* 0x1C */
/*9:0*/  unsigned int MASK_CNT : 10;
	 	 unsigned int HBLACK_reserved1 : 6;
/*27:16*/unsigned int bgactiveline_1 : 12;
	 	 unsigned int HBLACK_reserved2 : 4;
};

struct INPUT_CTRL_REG {/* 0x20 */
/*1:0*/	 unsigned int vdoin_Y_chn_sel : 2;
/*3:2*/	 unsigned int vdoin_CB_chn_sel : 2;
/*5:4*/	 unsigned int vdoin_CR_chn_sel : 2;
/*6*/	 unsigned int vdoin_inserve_bit : 1;
/*7*/	 unsigned int vdoin_12bit_mode : 1;
/*8*/	 unsigned int vdoin_10bit_mode : 1;
/*9*/	 unsigned int vdoin_444_mode : 1;
/*10*/	 unsigned int vdoin_c_del_sel : 1;
/*11*/	 unsigned int vdoin_c2_del_sel : 1;
/*13:12*/unsigned int pxl_sel : 2;
/*14*/	 unsigned int h_edge_sel : 1;
/*15*/	 unsigned int BGenVsyn_Inv : 1;
/*16*/	 unsigned int BGenHsyn_Inv : 1;
/*17*/ 	 unsigned int vdoin_wrap_en : 1;
/*19:18*/unsigned int YCin_del_sel_bit_1_0 : 2;
/*21:20*/unsigned int Cin_del_sel : 2;
/*22*/	 unsigned int c_sample_inv : 1;
/*23*/	 unsigned int sd_4fs_opt : 1;
/*24*/	 unsigned int YCin_del_sel_bit_2 : 1;
/*25*/	 unsigned int skip_line_en : 1;
/*26*/	 unsigned int new_interrupt_en : 1;
/*27*/	 unsigned int vdoin_3d_wrap : 1;
/*28*/	 unsigned int hqwrena_2t_syn : 1;
/*29*/	 unsigned int hqwrema_2t : 1;
/*30*/	 unsigned int sd_444_2fs : 1;
/*31*/	 unsigned int sd_480i_mix_eco : 1;
};

struct TOP_BOT_START_LIN_REG {/* 0x24 */
/*11:0*/	 unsigned int TL : 12;
	 	 unsigned int TOP_BOT_START_LIN_reserved1 : 4;
/*27:16*/unsigned int BL : 12;
	 	 unsigned int TOP_BOT_START_LIN_reserved2 : 4;
};

struct WRAPPER_3D_REG {/* 0x28 */
/*11:0*/	 unsigned int htotal : 12;
	 	 unsigned int WRAPPER_3D_reserved1 : 4;
/*27:16*/unsigned int vtotal : 12;
	 	 unsigned int WRAPPER_3D_reserved2 : 4;
};

struct WRAPPER_3D_VSYNC_REG {/* 0x2C */
	 	 unsigned int WRAPPER_3D_VSYNC_reserved1 : 16;
/*27:16*/unsigned int vtotal : 12;
	 	 unsigned int WRAPPER_3D_VSYNC_reserved2 : 4;
};

struct WRAPPER_3D_SETTING_REG {/* 0x30 */
/*7:0*/  unsigned int vdoin_3d_vsyn_width : 8;
	 	 unsigned int WRAPPER_3D_SETTING_reserved1 : 8;
/*23:16*/unsigned int vdoin_3d_hsyn_width : 8;
/*24*/	 unsigned int vdoin_3d_hsyn_in_polarity : 1;
/*25*/	 unsigned int vdoin_3d_vsyn_in_polarity : 1;
/*26*/	 unsigned int vdoin_3d_timing_free_run : 1;
/*27*/	 unsigned int vdoin_3d_timing_reset : 1;
	 	 unsigned int WRAPPER_3D_SETTING_reserved2 : 2;
/*31:30*/unsigned int swrst : 2;
};

struct HCNT_SETTING_REG {/* 0x34 */
/*12:0*/	 unsigned int HCNT : 13;
	 	 unsigned int HCNT_SETTING_reserved1 : 3;
/*28:16*/unsigned int HACTCNT : 13;
	 	 unsigned int HCNT_SETTING_reserved2 : 3;
};

struct HCNT_SETTING_1_REG {/* 0x38 */
/*9:0*/	 unsigned int YHCNT : 10;
	 	 unsigned int HCNT_SETTING_1_reserved1 : 6;
/*25:16*/unsigned int CHCNT : 10;
	 	 unsigned int HCNT_SETTING_1_reserved2 : 6;
};

struct VSCALE_REG {/* 0x3C */
/*0*/	 unsigned int fld_reset : 1;
/*1*/	 unsigned int Vsyn_reset : 1;
/*2*/	 unsigned int Vdifld_reset : 1;
/*4:3*/  unsigned int Avge_mode : 2;
/*5*/	 unsigned int Fld_int : 1;
/*6*/	 unsigned int Vsysn_int : 1;
/*7*/	 unsigned int Vdifld_int : 1;
/*8*/	 unsigned int Vsynccena : 1;
/*9*/	 unsigned int Int720p : 1;
/*10*/	 unsigned int Linest_ena : 1;
/*11*/	 unsigned int bgaddr_sel : 1;
/*12*/	 unsigned int bgsramcs_ena : 1;
/*13*/	 unsigned int bgtest_bar : 1;
/*14*/	 unsigned int Vdifld_int2 : 1;
	 	 unsigned int VSCALE_reserved1 : 1;
/*25:16*/unsigned int bghsize_dw : 10;
	 	 unsigned int VSCALE_reserved2 : 6;
};

struct HSCALE_REG {/* 0x40 */
/*11:0*/	 unsigned int BGTopLine_1 : 12;
	 	 unsigned int HSCALE_reserved1 : 4;
/*27:16*/unsigned int BGBottomLine_1 : 12;
	 	 unsigned int HSCALE_reserved2 : 4;
};

struct DEBUG_PORT_REG {/* 0x44 */
/*3:0*/  unsigned int Inbuf_state : 4;
	 	 unsigned int DEBUG_PORT_reserved1 : 4;
/*13:8*/	 unsigned int Outctl_state : 6;
	 	 unsigned int DEBUG_PORT_reserved2 : 2;
/*19:16*/unsigned int AV_STATE : 4;
	 	 unsigned int DEBUG_PORT_reserved3 : 4;
/*24*/	 unsigned int BLV_hsync : 1;
/*25*/	 unsigned int BLV_vsync : 1;
/*26*/	 unsigned int BLV_field : 1;
/*27*/	 unsigned int frame : 1;
/*28*/	 unsigned int AV_flag : 1;
/*29*/	 unsigned int H_cnt_cyc : 1;
/*30*/	 unsigned int Field_inv_flag : 1;
/*31*/	 unsigned int Src_type : 1;
};

struct DEBUG_PORT_1_REG {/* 0x48 */
/*7:0*/  unsigned int Hq1udcnt : 8;
/*15:8*/ unsigned int Hq2udcnt : 8;
/*16*/   unsigned int Frm_cnt : 1;
	 	 unsigned int DEBUG_PORT_1_reserved1 : 3;
/*20*/   unsigned int frame : 1;
/*21*/   unsigned int src_type : 1;
	 	 unsigned int DEBUG_PORT_1_reserved2 : 10;
};

struct DEBUG_PORT_3_REG {/* 0x4C */
/*0*/	 unsigned int wr_req_org : 1;
/*1*/    unsigned int wr_last_org : 1;
/*2*/    unsigned int wr_req : 1;
/*3*/    unsigned int wr_last : 1;
/*4*/    unsigned int Hd2req : 1;
/*5*/    unsigned int Hd1req : 1;
/*31:6*/ unsigned int vdoin_req_mon : 26;
};

struct INDEBUG_STATUS_REG {/* 0x50 */
/*12:0*/ unsigned int h_total_detect : 13;
	 	 unsigned int INDEBUG_STATUS_reserved1 : 3;
/*27:16*/unsigned int v_total_detect : 12;
/*28*/	 unsigned int field_sig : 1;
	 	 unsigned int INDEBUG_STATUS_reserved2 : 3;
};

struct DEBUG_PORT_4_REG {/* 0x58 */
/*27:0*/ unsigned int Mgprotect_ptrt : 28;
	 	 unsigned int DEBUG_PORT_4_reserved1 : 1;
/*29*/	 unsigned int Mgprotect_ena1 : 1;
/*30*/	 unsigned int Mgprotect_ena : 1;
/*31*/	 unsigned int Mgprotect_clr : 1;
};

struct DEBUG_PORT_5_REG {/* 0x5C */
/*27:0*/ unsigned int Mgprotect_ptrt : 28;
	 	 unsigned int DEBUG_PORT_5_reserved1 : 3;
/*31*/	 unsigned int Overflow_flag : 1;
};

struct DRAM_BYTE_EN_REG {/* 0x60 */
	 	 unsigned int DRAM_BYTE_EN_reserved1 : 8;
/*23:8*/ unsigned int byte_data : 16;
/*24*/   unsigned int byte_risc_ena : 1;
	 	 unsigned int DRAM_BYTE_EN_reserved2 : 7;
};

struct YBUF1_ADDR_REG {/* 0x64 */
/*29:0*/ unsigned int YBUF1_ADDR : 30;
	 	 unsigned int YBUF1_ADDR_reserved1 : 2;
};

struct CBUF1_ADDR_REG {/* 0x68 */
/*29:0*/ unsigned int CBUF1_ADDR : 30;
	 	 unsigned int CBUF1_ADDR_reserved1 : 2;
};

struct YBUF2_ADDR_REG {/* 0x6C */
/*29:0*/ unsigned int YBUF2_ADDR : 30;
	 	 unsigned int YBUF2_ADDR_reserved1 : 2;
};

struct CBUF2_ADDR_REG {/* 0x70 */
/*29:0*/ unsigned int CBUF2_ADDR : 30;
	 	 unsigned int CBUF2_ADDR_reserved1 : 2;
};

struct YBUF3_ADDR_REG {/* 0x74 */
/*29:0*/ unsigned int YBUF3_ADDR : 30;
	 	 unsigned int YBUF3_ADDR_reserved1 : 2;
};

struct CBUF3_ADDR_REG {/* 0x78 */
/*29:0*/ unsigned int CBUF3_ADDR : 30;
	 	 unsigned int CBUF3_ADDR_reserved1 : 2;
};

struct REQ_CTL_REG {/* 0x7C */
/*7:0*/  unsigned int CH_LAST_THRES : 8;
/*15:8*/ unsigned int CH_REQ_THRES : 8;
/*16*/   unsigned int CH_REQ_NEW : 1;
/*17*/   unsigned int CH_REQ_EMPTY : 1;
/*18*/   unsigned int CH_PROTECT_EN : 1;
/*19*/   unsigned int CH_VRST_EN : 1;
	 	 unsigned int DRAM_BYTE_EN_reserved1 : 1;
/*22:21*/unsigned int BGYCADDR_AUTO_CNT : 2;
/*23*/   unsigned int BGYCADDR_AUTO_CNT_CLR : 1;
/*26:24*/unsigned int BGYCADDR_AUTO_CFG : 3;
/*27*/   unsigned int BGYCADDR_AUTO_EN : 1;
/*29:28*/unsigned int BGYC_LATCH_MODE : 2;
/*30*/   unsigned int v_flip : 1;
/*31*/   unsigned int h_flip : 1;
};

struct REQ_OUT_REG {/* 0x80 */
/*22:0*/ unsigned int CH_PROTECT_THRES : 23;
/*23*/   unsigned int CH_PROTECT_DISABLE : 1;
/*25:24*/unsigned int CH_INT_MODE : 2;
/*27:26*/unsigned int CH_C_ACT_SEL : 2;
/*28*/   unsigned int h_dis_end_sel : 1;
/*29*/   unsigned int h_dis_act_sel : 1;
/*30*/   unsigned int ext_field : 1;
/*31*/   unsigned int index_mode : 1;
};

struct DRAM_PROTECT3_REG {/* 0x84 */
/*27:0*/ unsigned int protect_ptrt : 28;
	 	 unsigned int DRAM_PROTECT3_reserved1 : 3;
/*31*/   unsigned int protect_enable : 1;
};

struct DRAM_PROTECT4_REG {/* 0x88 */
/*27:0*/ unsigned int protect_ptrb : 28;
	 	 unsigned int DRAM_PROTECT4_reserved1 : 3;
/*31*/   unsigned int block_64x32_en : 1;
};

struct WCH_REG {
	struct VDOIN_EN_REG VDOIN_EN;/* 0x00 */
	struct VDOIN_MODE_REG VDOIN_MODE;/* 0x04 */
	struct YBUF0_ADDR_REG YBUF0_ADDR;/* 0x08 */
	struct ACT_LINE_REG ACT_LINE;/* 0x0C */
	struct CBUF0_ADDR_REG CBUF0_ADDR;/* 0x10 */
	struct DW_NEED_REG DW_NEED;/* 0x14 */
	struct HPIXEL_REG HPIXEL;/* 0x18 */
	struct HBLACK_REG HBLACK;/* 0x1C */
	struct INPUT_CTRL_REG INPUT_CTRL;/* 0x20 */
	struct TOP_BOT_START_LIN_REG TOP_BOT_START_LIN;/* 0x24 */
	struct WRAPPER_3D_REG WRAPPER_3D;/* 0x28 */
	struct WRAPPER_3D_VSYNC_REG WRAPPER_3D_VSYNC;/* 0x2C */
	struct WRAPPER_3D_SETTING_REG WRAPPER_3D_SETTING;/* 0x30 */
	struct HCNT_SETTING_REG HCNT_SETTING;/* 0x34 */
	struct HCNT_SETTING_1_REG HCNT_SETTING_1;/* 0x38 */
	struct VSCALE_REG VSCALE;/* 0x3C */
	struct HSCALE_REG HSCALE;/* 0x40 */
	struct DEBUG_PORT_REG DEBUG_PORT;/* 0x44 */
	struct DEBUG_PORT_1_REG DEBUG_PORT_1;/* 0x48 */
	struct DEBUG_PORT_3_REG DEBUG_PORT_3;/* 0x4C */
	struct INDEBUG_STATUS_REG INDEBUG_STATUS;/* 0x50 */
	unsigned int reserved1;
	struct DEBUG_PORT_4_REG DEBUG_PORT_4;/* 0x58 */
	struct DEBUG_PORT_5_REG DEBUG_PORT_5;/* 0x5C */
	struct DRAM_BYTE_EN_REG DRAM_BYTE_EN;/* 0x60 */
	struct YBUF1_ADDR_REG YBUF1_ADDR;/* 0x64 */
	struct CBUF1_ADDR_REG CBUF1_ADDR;/* 0x68 */
	struct YBUF2_ADDR_REG YBUF2_ADDR;/* 0x6C */
	struct CBUF2_ADDR_REG CBUF2_ADDR;/* 0x70 */
	struct YBUF3_ADDR_REG YBUF3_ADDR;/* 0x74 */
	struct CBUF3_ADDR_REG CBUF3_ADDR;/* 0x78 */
	struct REQ_CTL_REG REQ_CTL;/* 0x7C */
	struct REQ_OUT_REG REQ_OUT;/* 0x80 */
	struct DRAM_PROTECT3_REG DRAM_PROTECT3;/* 0x84 */
	struct DRAM_PROTECT4_REG DRAM_PROTECT4;/* 0x88 */
};

#define WCH_REG_NUM		(0x8c/4)
typedef union _WCH_REG_UNION_T {
	unsigned int au4Reg[WCH_REG_NUM];
	struct WCH_REG rField;
}WCH_REG_UNION_T,*PWCH_REG_UNION_T;

#define WCH_HAL_WRITE32(_reg_, _val_)   		(*((volatile unsigned int*)(_reg_)) = (_val_))
#define WCH_HAL_READ32(_reg_)           		(*((volatile unsigned int*)(_reg_)))

extern unsigned long _IO_BASE_;	
#define WCH_IO_BASE _IO_BASE_//__va((unsigned long)0x10000000)

#define TVD_WCH_CONTROL_0 	(WCH_IO_BASE+0x364)
	#define r_dgi_1_clk_sel (0x1 << 14)     //  PAD1 DFF clock source choose. may chose Pad1 clock or Pad2 clock
	#define r_dgi_1_clk_inv (0x1 << 15)     //  DGI pad1 clock inv
	#define r_dgi_2_clk_sel (0x1 << 16)     //  PAD2 DFF clock source choose. may chose Pad1 clock or Pad2 clock
	#define r_dgi_2_clk_inv (0x1 << 17)     //  DGI pad1 clock inv

#define TVD_WCH_CONTROL_1 	(WCH_IO_BASE+0x1f038)
	#define r_dgi_clk_sel  (0x1<<12)        //DGI1 write channel clock source choose, may chose Pad1 clock or Pad2 clock
	#define r_dgi2_clk_sel  (0x1<<13)       //DGI2 and TVD shared's write channel clock source choose 

#define TVD_WCH_CONTROL_2 	(WCH_IO_BASE+0x1f044)
	#define r_tvd0_dgi_sel	(0x1<<18)

#define TVD_WCH_CONTROL_3 	(WCH_IO_BASE+0x1f034)
	#define r_tvd0_clk_sel	(0x3<<8)
	#define r_tvd0_dgi_clk_sel	(0x1<<18)
	#define r_tvd1_clk_sel	(0x3<<10)
	#define r_tvd2_clk_sel	(0x3<<12)
	#define r_tvd3_clk_sel	(0x3<<14)
	#define r_tvd4_clk_sel	(0x3<<16)

#define TVD_WCH_CONTROL_4 	(WCH_IO_BASE+0x1f030)
	#define r_tvd4_sel	(0x3<<8)

#define TVD_WCH_CONTROL_5 	(WCH_IO_BASE+0x5c)
	#define dclk_in_1	(0x1<<2)
	#define h_v_sync_in_1	(0x1<<3)
	#define data_in_1	(0x1<<11)
	#define dclk_in_2	(0x1<<12)
	#define h_v_sync_in_2	(0x1<<13)
	#define data_in_2	(0x1<<10)



struct HTOTAL_T{
	unsigned int u4SrcWidth;
	unsigned int u4SrcHeight;
	unsigned int htotal;
};

/*************************************************/
unsigned int TVDHTOTAL[] = {
	0x359,/*576*/ /*0*/
	0x338,/*480*/ /*1*/
};

unsigned int VTOTAL[] = {
	525,/*480*/ /*0*/
	625,/*576*/ /*1*/
	525,/*800*480*/ /*2*/
	656,/*800*600*/ /*3*/
	625,/*1024*600*/ /*4*/
	750,/*720*/ /*5*/
	1125,/*1080*/ /*6*/
};

#endif
