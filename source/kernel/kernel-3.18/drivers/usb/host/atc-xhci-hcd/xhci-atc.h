#ifndef _XHCI_MTK_H
#define _XHCI_MTK_H

#include <linux/version.h>
#include <linux/usb.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>

#include "xhci.h"
#include "xhci-atc-scheduler.h"
#include "ssusb_sifslv_ippc.h"
#include "ssusb_usb3_mac_csr.h"
#include "ssusb_usb3_sys_csr.h"
#include "ssusb_usb2_csr.h"
#include "ssusb_xHCI_exclude_port_csr.h"

/* U3H IP CONFIG:
 * enable this according to the U3H IP num of the project
 */
#define  CFG_DEV_U3H0   1  //if the project has one or more U3H IP, enable this
#define  CFG_DEV_U3H1   0  //if the project has two or more U3H IP, enable this


#define FPGA_MODE       0   //if run in FPGA,enable this
#define OTG_SUPPORT     0   //if OTG support,enable this

#define PRJ_MT3365
#define CKGEN_BASE	0xfd000000
#define VAL_SSUSB_RST	0x00200002
#define MT3365_IRQ	(89 + 104)
#define MT3365_U3H_BASE0		0x1004c000
#define MT3365_IPPC_BASE0		0x10039700
#define MT3365_U3PHY_BASE0		0x10034000

/* U3H irq number*/
#if CFG_DEV_U3H0
    #define U3H_IRQ0 MT3365_IRQ
#endif
#if CFG_DEV_U3H1
    #define U3H_IRQ1 15
#endif


/*U3H register bank*/
#if CFG_DEV_U3H0
    //physical base address for U3H IP0
    #define U3H_BASE0	    MT3365_U3H_BASE0
    #define IPPC_BASE0      MT3365_IPPC_BASE0
	#define U3PHY_BASE0		MT3365_U3PHY_BASE0
#endif
#if CFG_DEV_U3H1
    //physical base address for U3H IP1
    #define U3H_BASE1	    0xf0040000
    #define IPPC_BASE1      0xf0044700
#endif


/* Clock source */
//Clock source may differ from project to project. Please check integrator
#define	U3_REF_CK_VAL	25			//MHz = value
#define	U3_SYS_CK_VAL	125			//MHz = value
/*
 * HW uses a flexible clock to calculate SOF.
 * i.e., SW shall help HW to count the exact value to get 125us by set correct counter value
 */
#define FRAME_CK_60M		0
#define FRAME_CK_20M		1
#define FRAME_CK_24M		2
#define FRAME_CK_32M		3
#define FRAME_CK_48M		4

#define FRAME_CNT_CK_VAL	FRAME_CK_60M
#define FRAME_LEVEL2_CNT	20


#if FPGA_MODE
    /*Defined for PHY init in FPGA MODE*/
    //change this value according to U3 PHY calibration
    #define U3_PHY_PIPE_PHASE_TIME_DELAY	0x8
#endif


//offset may differ from project to project. Please check integrator
#define SSUSB_USB3_CSR_OFFSET 0x00002400
#define SSUSB_USB2_CSR_OFFSET 0x00003400


#define MTK_U3H_SIZE	0x4000
#define MTK_IPPC_SIZE	0x100
#define MTK_U3PHY_SIZE	0x1000



/*=========================================================================================*/


#define u3h_writelmsk(addr, data, msk) \
	{ writel(((readl(addr) & ~(msk)) | ((data) & (msk))), addr); \
	}

enum xhci_usb_speed {
	USB_UNKNOWN,
	USB11,
	USB20,
	USB30
};


struct mtk_u3h_hw {
	char u3_port_num;
	char u2_port_num;
	struct device *dev;
	struct gpio_desc *vbus;
	struct usb_hcd *hcd;
	void *u3h_virtual_base;
	void *ippc_virtual_base;
	void *u3phy_virtual_base;
	struct sch_port u3h_sch_port[MAX_PORT_NUM];
	struct phy **phys;
	int num_phys;
	enum xhci_usb_speed speed;
};

//extern struct mtk_u3h_hw u3h_hw;

void reinitIP(struct device *dev);
void setInitialReg(struct device *dev);
void dbg_prb_out(void);
int u3h_phy_init(void);
int get_xhci_u3_port_num(struct device *dev);
int get_xhci_u2_port_num(struct device *dev);
int chk_frmcnt_clk(struct usb_hcd *hcd);
struct platform_device * get_mtk_device_u3h(u32 id);





#if 0
/*
  mediatek probe out
*/
/************************************************************************************/

#define SW_PRB_OUT_ADDR	(SIFSLV_IPPC+0xc0)		//0xf00447c0
#define PRB_MODULE_SEL_ADDR	(SIFSLV_IPPC+0xbc)	//0xf00447bc

static inline void mtk_probe_init(const u32 byte){
	__u32 __iomem *ptr = (__u32 __iomem *) PRB_MODULE_SEL_ADDR;
	writel(byte, ptr);
}

static inline void mtk_probe_out(const u32 value){
	__u32 __iomem *ptr = (__u32 __iomem *) SW_PRB_OUT_ADDR;
	writel(value, ptr);
}

static inline u32 mtk_probe_value(void){
	__u32 __iomem *ptr = (__u32 __iomem *) SW_PRB_OUT_ADDR;

	return readl(ptr);
}
#endif

#endif
