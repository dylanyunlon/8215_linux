

#ifndef _WCH_PRIV_H_
#define _WCH_PRIV_H_


#include <generated/atc_project.h>

#define WCH_DUMP_BUFFER_ATTR 1

#if defined (CONFIG_ATC_PRJ_ac823x_adas)
#define WCH_SUPPORT_ADAS_ONLY	1
#else
#define WCH_SUPPORT_ADAS_ONLY	0
#endif


typedef enum {
	VOUT_SRC_UNKNOWN = 0,
	VOUT_SRC_FRONT_VIDEO,/*choose front video only*/
	VOUT_SRC_FRONT_CAPTURE,/*choose front mix2(screen capture)*/
	VOUT_SRC_FRONT_DISPLAY,/*choose front mix1(normal display)*/
	VOUT_SRC_REAR_VIDEO,/*choose rear video only*/
	VOUT_SRC_REAR_CAPTURE,/*choose rear mix2(screen capture)*/
	VOUT_SRC_REAR_DISPLAY,/*choose rear mix1(normal display)*/
} WCH_VOUT_SRC_E;


typedef struct WCHTIMING {
	WCH_TIMING_E eTiming; /* Timing Mode */
	unsigned short u2HsyncInv;   /* Horizontal active in hsync low range, otherwise invert hsync */
	unsigned short u2HPixel;     /* Hsync falling edge to H active start pixel count */
	unsigned short u2HActive;    /* H active */
	unsigned short u2VsyncInv;   /* Vertical active in vsync low range, otherwise invert vsync */
	unsigned short u2VTopLine;   /* Vsync falling edge to V active start line count */
	unsigned short u2VBotLine;   /* Vsync falling edge to V active start line count, and Interlace timing use */
	unsigned short u2VActive;    /* V active -1 */
} WCH_TIMING_PARAM_T;



void WchEventThreadInit(void);
void WchEventThreadDeinit(void);
void WchSetSourceBaseAddr(unsigned long wchReservebase);
void WchIsrInit(unsigned char u1WchId);
void WchSuspend(void);
void WchResume(void);


#ifndef __ARM2__
extern void __iomem *wch_sysreg_base[WCH_NUM];

extern struct task_struct *hWchInst[WCH_NUM];
extern unsigned long _IO_BASE_;	
#endif
extern int wchirq[WCH_NUM];

enum {
	WCH_HW_FREE = 0,
	WCH_HW_CONFIG,
	WCH_HW_START,
	WCH_HW_STOP,
};


//BT656
#define WCH_BT656_BUFFER_SIZE		(6912000)
#define WCH_BT656_BASE				(wch_reserve)
#define WCH_BT656_END				(WCH_BT656_BASE + WCH_BT656_BUFFER_SIZE)
#define WCH_BT656_YBUF_SIZE			(1280*720)
#define WCH_BT656_CBUF_SIZE			(1280*720/2)

//BT601
#define WCH_BT601_BUFFER_SIZE		(6912000)
#define WCH_BT601_BASE				(WCH_BT656_END)
#define WCH_BT601_END				(WCH_BT601_BASE + WCH_BT601_BUFFER_SIZE)
#define WCH_BT601_YBUF_SIZE			(1280*720)
#define WCH_BT601_CBUF_SIZE			(1280*720/2)

//BT1120
#define WCH_BT1120_BUFFER_SIZE		(15552000)
#define WCH_BT1120_BASE				(wch_reserve)
#define WCH_BT1120_END				(WCH_BT1120_BASE + WCH_BT1120_BUFFER_SIZE)
#define WCH_BT1120_YBUF_SIZE		(1920*1080)
#define WCH_BT1120_CBUF_SIZE		(1920*1080/2)

/* AVM */
#define WCH_AVM_BUFFER_SIZE 		(6289920)
#if WCH_SUPPORT_ADAS_ONLY
#define WCH_AVM_BASE				(wch_reserve)
#else
#define WCH_AVM_BASE				(WCH_BT1120_END)
#endif
#define WCH_AVM_END 				(WCH_AVM_BASE + WCH_AVM_BUFFER_SIZE)

//AVM WCH1
#define WCH_AVM_WCH1_BUFFER_SIZE 	(3110400)
#define WCH_AVM_WCH1_BASE			(WCH_AVM_BASE)
#define WCH_AVM_WCH1_END 			(WCH_AVM_WCH1_BASE + WCH_AVM_WCH1_BUFFER_SIZE)
#define WCH_AVM_WCH1_YBUF_SIZE		(720*576)
#define WCH_AVM_WCH1_CBUF_SIZE		(720*576/2)

//AVM WCH2
#define WCH_AVM_WCH2_BUFFER_SIZE 	(3110400)
#define WCH_AVM_WCH2_BASE			(WCH_AVM_WCH1_END)
#define WCH_AVM_WCH2_END 			(WCH_AVM_WCH2_BASE + WCH_AVM_WCH2_BUFFER_SIZE)
#define WCH_AVM_WCH2_YBUF_SIZE		(720*576)
#define WCH_AVM_WCH2_CBUF_SIZE		(720*576/2)

//AVM WCH3
#define WCH_AVM_WCH3_BUFFER_SIZE 	(3110400)
#define WCH_AVM_WCH3_BASE			(WCH_AVM_WCH2_END)
#define WCH_AVM_WCH3_END 			(WCH_AVM_WCH3_BASE + WCH_AVM_WCH3_BUFFER_SIZE)
#define WCH_AVM_WCH3_YBUF_SIZE		(720*576)
#define WCH_AVM_WCH3_CBUF_SIZE		(720*576/2)

//AVM WCH4
#define WCH_AVM_WCH4_BUFFER_SIZE 	(3110400)
#define WCH_AVM_WCH4_BASE			(WCH_AVM_WCH3_END)
#define WCH_AVM_WCH4_END 			(WCH_AVM_WCH4_BASE + WCH_AVM_WCH4_BUFFER_SIZE)
#define WCH_AVM_WCH4_YBUF_SIZE		(720*576)
#define WCH_AVM_WCH4_CBUF_SIZE		(720*576/2)

//AVM WCH5
#define WCH_AVM_WCH5_BUFFER_SIZE 	(3110400)
#define WCH_AVM_WCH5_BASE			(WCH_AVM_WCH4_END)
#define WCH_AVM_WCH5_END 			(WCH_AVM_WCH5_BASE + WCH_AVM_WCH5_BUFFER_SIZE)
#define WCH_AVM_WCH5_YBUF_SIZE		(720*576)
#define WCH_AVM_WCH5_CBUF_SIZE		(720*576/2)
/*AVM end*/

//BACKCAR
#define WCH_BACKCAR_BUFFER_SIZE 	(3110400)
#define WCH_BACKCAR_BASE			(WCH_AVM_BASE)
#define WCH_BACKCAR_END 			(WCH_BACKCAR_BASE + WCH_BACKCAR_BUFFER_SIZE)
#define WCH_BACKCAR_YBUF_SIZE		(720*576)
#define WCH_BACKCAR_CBUF_SIZE		(720*576/2)

//AVIN
#define WCH_AVIN_BUFFER_SIZE 		(3110400)
#define WCH_AVIN_BASE				(WCH_BACKCAR_END)
#define WCH_AVIN_END 				(WCH_AVIN_BASE + WCH_AVIN_BUFFER_SIZE)
#define WCH_AVIN_YBUF_SIZE			(720*576)
#define WCH_AVIN_CBUF_SIZE			(720*576/2)

//YPbPr
#define WCH_YPBPR_BUFFER_SIZE		(15552000)
#define WCH_YPBPR_BASE				(WCH_AVM_END)
#define WCH_YPBPR_END				(WCH_YPBPR_BASE + WCH_YPBPR_BUFFER_SIZE)
#define WCH_YPBPR_YBUF_SIZE			(1920*1080)
#define WCH_YPBPR_CBUF_SIZE			(1920*1080/2)

//HDMI
#define WCH_HDMI_BUFFER_SIZE		(15552000)
#define WCH_HDMI_BASE				(WCH_YPBPR_END)
#define WCH_HDMI_END				(WCH_HDMI_BASE + WCH_HDMI_BUFFER_SIZE)
#define WCH_HDMI_YBUF_SIZE			(1920*1080)
#define WCH_HDMI_CBUF_SIZE			(1920*1080/2)

//VDO
#define WCH_VDO_BUFFER_SIZE			(15552000)
#define WCH_VDO_BASE				(WCH_HDMI_END)
#define WCH_VDO_END					(WCH_VDO_BASE + WCH_VDO_BUFFER_SIZE)
#define WCH_VDO_YBUF_SIZE			(1920*1080)
#define WCH_VDO_CBUF_SIZE			(1920*1080/2)

#if WCH_SUPPORT_ADAS_ONLY
//for drop frame
#define WCH_BACKUP_BUFFER_BASE		(wch_reserve + 0xF00000)
#define WCH_VDO_YBUF_SIZE			(720*576)
#define WCH_VDO_CBUF_SIZE			(720*576/2)
#else
//for drop frame
#define WCH_BACKUP_BUFFER_BASE		(wch_reserve + 0x03c00000)
#define WCH_VDO_YBUF_SIZE			(1920*1080)
#define WCH_VDO_CBUF_SIZE			(1920*1080/2)
#endif

typedef struct {
	unsigned int u4Status;
	unsigned int u4BufIdx;
	WCH_BUFF_INFO_T tWchBuf;
	WCH_SRC_APP_ID_E eWchSrcId;
	WCH_CFG_T tWchCfg;
} WCH_IF_PARAM_T, *PWCH_IF_PARAM_T;

#if WCH_DUMP_BUFFER_ATTR
extern int u4DumpFrameCnt[WCH_NUM];
extern int u4DumpBufIdxCnt[WCH_NUM];
extern WCH_IF_PARAM_T _gWchParam[WCH_NUM];
#endif

#endif

