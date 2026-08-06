#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
/*#include <linux/earlysuspend.h>*/
#include <linux/jiffies.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_fdt.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/pinctrl/consumer.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <media/atc/display_inc.h>
#include <media/atc/vdp_mdd.h>
#include <media/atc/display.h>
#include <media/atc/pmx_hal.h>
#include <media/atc/drv_av_d.h>
#include <media/atc/drv_osd_if.h>
#include "fb.h"
#include "dal.h"
#include "osd_if_pdd.h"
 #include "x_ckgen.h"
#include "drv_imgresz.h"
#include "osd_inc.h"
#include "log.h"
#include "oal.h"
#include "x_ver.h"
#include <generated/atc_project.h>
#include "disp_assert_layer.h"

unsigned int dal_base_pa = NULL;
void* dal_base_va = NULL;

FB_CONFIG_T g_rFBConfig;
PANEL_SETTING_ARGS_T g_rPanelSetting;
void * fbm_va = NULL;
static int aColorMode[] = {
	OSD_CM_RGB565_DIRECT16,
	OSD_CM_ARGB8888_DIRECT32
};
	
static int _aBytesPerPixel[] = {
	2,
	4
};

static const struct of_device_id dal_of_match[] = {
	{.compatible = "Autochips,framebuffer",},
	{}
};
__u32 fb_log_lvl = FB_LOG_LVL_HAL;
__u8 *fb_lvl_str[] = {
	"OFF",
	"ERR",
	"WARN",
	"CLI",
	"INFO",
	"HAL",
	"IRQ",
	"TRACE",
	"DBG",
	"REGRW",
};

void __iomem *osdf_reg;
void __iomem *osd1_reg;
void __iomem *osd2_reg;
void __iomem *osd3_reg;
void __iomem *osd4_reg;
void __iomem *osd5_reg;
void __iomem *osdr_reg;
void __iomem *osdr1_reg;
void __iomem *osdr2_reg;
void __iomem *osdr3_reg;


__u32  LCD_GetScreenWidth(void)
{
	if (RESET_HW_ENGINE == FALSE) {
		__u32 u4Width;

		u4Width = (*((__u32 *) 0xFD02001C) >> 16) & 0x7FF;
		return u4Width;
	} else {
		return PRIMARY_OSD_WIDTH;
	}
}

__u32 LCD_GetScreenHeight(void)
{
	if (RESET_HW_ENGINE == FALSE) {
		__u32 u4Height = *((__u32 *) 0xFD02001C) & 0x7FF;

		return u4Height;
	} else {
		return PRIMARY_OSD_HEIGHT;
	}
}



static bool dal_osd_init(unsigned int addr1, unsigned int addr2, int width, int height)
{
	DAL_PRINT(DAL_LOG_LVL_INFO, "", "dal_DAL_init start \n");
	__s32  ret;
	__u32 rgn_list, rgn, plane;
	plane = OSD_PLANE_4;

	OSD_BASE_SetOsdPosition_DAL(plane, PRIMARY_OSD_X_OFFSET, PRIMARY_OSD_Y_OFFSET);
	OSD_SC_Scale_DAL(plane, FALSE, width, height, width, height);
	OSD_RGN_LIST_Create_DAL(&rgn_list);
	rgn_list  = plane;
	OSD_RGN_Create_DAL(&rgn, width, height, (void *)addr1, aColorMode[0],
		       width * _aBytesPerPixel[0], 0, 0,  width, height);
	_OSD_RGN_SetAlpha(rgn, (__u32)0x80);
	_OSD_RGN_SetBlendMode(rgn, (__u32)OSD_BM_REGION);
	OSD_RGN_LIST_DetachAll_DAL(rgn_list);
	ret = OSD_RGN_Insert_DAL(rgn, rgn_list);
	if (ret) {
		DAL_PRINT(DAL_LOG_LVL_DBG, "", "DAL_RGN_Insert rgn failed: %d\n", (int)ret);
		return ret;
	}
	SetPlaneRgnDAL(rgn_list, rgn);
	i4OsdPlaneFlipToDAL(plane, rgn_list);
	i4OsdPlaneEnbleDAL(plane, FALSE);

	DAL_PRINT(DAL_LOG_LVL_INFO, "", "dal_DAL_init end \n");

	return TRUE;
}

//static int read_dts_data(struct platform_device *pdev)
static int read_dts_data(void )
{
	DAL_PRINT(DAL_LOG_LVL_INFO, "%s", "start read dts \n",__func__);

	__u32 ret = -EINVAL;
	struct reserved_mem *fb_mem;
	struct device_node *nd;
	unsigned int property[2];
	unsigned int base_pa, base_va;
	unsigned int size;
	struct device_node *node;


        nd = of_find_compatible_node(NULL,NULL,"Autochips,framebuffer");

	if (!nd) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "","%s: get Autochips,framebuffer dts node fail\r\n", __func__);
		goto err;
	}

	osdf_reg = of_iomap(nd, 0);

	if (!osdf_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osdf reg base address failed = %x \r\n"
			, (unsigned int)osdf_reg);
		goto err;
	}

	osd1_reg = of_iomap(nd, 1);

	if (!osd1_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osd1 reg base address failed = %x \r\n"
			, (unsigned int)osd1_reg);
		goto err;
	}
	osd2_reg = of_iomap(nd, 2);

	if (!osd2_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osd2 reg base address failed = %x \r\n"
			, (unsigned int)osd2_reg);
		goto err;
	}
	osd3_reg = of_iomap(nd, 3);

	if (!osd3_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osd4 reg base address failed = %x \r\n"
			, (unsigned int)osd3_reg);
		goto err;
	}

	osd4_reg = of_iomap(nd, 4);

	if (!osd4_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osdf reg base address failed = %x \r\n"
			, (unsigned int)osd4_reg);
		goto err;
	}

	osd5_reg = of_iomap(nd, 5);

	if (!osd5_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osd5 reg base address failed = %x \r\n"
			, (unsigned int)osd5_reg);
		goto err;
	}

	osdr_reg = of_iomap(nd, 6);

	if (!osdr_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osdr reg base address failed = %x \r\n"
			, (unsigned int)osdr_reg);
		goto err;
	}

	osdr1_reg = of_iomap(nd, 7);

	if (!osdr1_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osdr1 reg base address failed = %x \r\n"
			, (unsigned int)osdr1_reg);
		goto err;
	}

	osdr2_reg = of_iomap(nd, 8);

	if (!osdr2_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osdr2 reg base address failed = %x \r\n"
			, (unsigned int)osdr2_reg);
		goto err;
	}

	osdr3_reg = of_iomap(nd, 9);

	if (!osdr3_reg) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "[read_dts_data] get osdr3 reg base address failed = %x \r\n"
			, (unsigned int)osdr3_reg);
		goto err;
	}
#ifdef CONFIG_ATC_OS_linux
//need get resolution for dal osd
	node = of_find_compatible_node(NULL,NULL,"atc-framebuffer");
	if (!node) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "","%s: get dts node fail\r\n", __func__);
		goto err;
	}

	if (node)
	{
		base_va = (unsigned long)of_iomap(node, 0);
		if (0 == base_va) {
			DAL_PRINT(DAL_LOG_LVL_ERR, "", "of_iomap fail\r\n");
			goto err;
		}
		if (of_property_read_u32_array(node, "reg", (u32 *)property, 2)) {
			DAL_PRINT(DAL_LOG_LVL_ERR, "", "get reserved memory node reg info fail\r\n");
			goto err;
		}
		fbm_base = property[0];
		fbm_size = property[1];
		fbm_va = ioremap(fbm_base, fbm_size);
		DAL_PRINT(DAL_LOG_LVL_INFO, "", "buffer base va:%x, size:%x, pa:%x\n", base_va, fbm_size, fbm_base);
			//set_fb1_param(base_pa, base_va, size);
	} else
	{
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "can not find reserved memory node!!\r\n");
		goto err;
	}

	if ((fbm_base == 0) || (fbm_size == 0)) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "get memory failed base %x size %x\r\n", fbm_base, fbm_size);
		goto err;
	}

	node = of_find_compatible_node(NULL,NULL,"atc-dal");
	if (!node) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "","%s: get dts node fail\r\n", __func__);
		goto err;
	}

	if (node)
	{
		base_va = (unsigned long)of_iomap(node, 0);
		if (0 == base_va) {
			DAL_PRINT(DAL_LOG_LVL_ERR, "", "of_iomap fail\r\n");
			goto err;
		}
		if (of_property_read_u32_array(node, "reg", (u32 *)property, 2)) {
			DAL_PRINT(DAL_LOG_LVL_ERR, "", "get reserved memory node reg info fail\r\n");
			goto err;
		}
		dal_base = property[0];
		dal_size = property[1];
		DAL_PRINT(DAL_LOG_LVL_INFO, "", "buffer base va:%x, size:%x, pa:%x\n", base_va, dal_size, dal_base);
		//set_fb1_param(base_pa, base_va, size);
	} else
	{
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "can not find reserved memory node!!\r\n");
		goto err;
	}

	if ((dal_base == 0) || (dal_size == 0)) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "get memory failed base %x size %x\r\n", dal_base, dal_size);
		goto err;
	}

#else
	node = of_find_compatible_node(NULL,NULL,"atc-framebuffer");
	if (!node) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "","%s: get dts node fail\r\n", __func__);
		goto err;
	}

	if (node)
	{
		base_va = (unsigned long)of_iomap(node, 0);
		if (0 == base_va) {
			DAL_PRINT(DAL_LOG_LVL_ERR, "", "of_iomap fail\r\n");
			goto err;
		}
		if (of_property_read_u32_array(node, "reg", (u32 *)property, 2)) {
			DAL_PRINT(DAL_LOG_LVL_ERR, "", "get reserved memory node reg info fail\r\n");
			goto err;
		}
		fbm_base = property[0];
		fbm_size = property[1];
		fbm_va = ioremap(fbm_base, fbm_size);
		DAL_PRINT(DAL_LOG_LVL_INFO, "", "buffer base va:%x, size:%x, pa:%x\n", base_va, fbm_size, fbm_base);
		//set_fb1_param(base_pa, base_va, size);
	} else
	{
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "can not find reserved memory node!!\r\n");
		goto err;
	}

	if ((fbm_base == 0) || (fbm_size == 0)) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "get memory failed base %x size %x\r\n", fbm_base, fbm_size);
		goto err;
	}

#endif

#ifdef CONFIG_ATC_OS_linux
	dal_base_pa = dal_base;
#else
	dal_base_pa = fbm_base + (LCD_GetScreenWidth() * LCD_GetScreenHeight() * 4 * 12);/* width*height*bpp*buffer_number */
#endif

	dal_base_va = ioremap(dal_base_pa, dal_buf_size);
	if (dal_base_va == NULL) {
		DAL_PRINT(DAL_LOG_LVL_ERR, "", "get ioremap va error, dal_base_pa 0x%x\r\n", dal_base_pa);
		goto err;
	}
	DAL_PRINT(DAL_LOG_LVL_INFO, "", "get memory dal_base_pa 0x%x, va 0x%x, size %d\r\n", dal_base_pa, (unsigned int)dal_base_va,
		LCD_GetScreenWidth() * LCD_GetScreenHeight() * 4 * 12);

	ret = 0;
err:
	return ret;
}
extern __s32 OSD_Init_DAL(bool fgHwReset);

/*static   int dal_probe(struct platform_device *pdev)*/
static  int dal_probe(void )

{
	DAL_PRINT(DAL_LOG_LVL_INFO,"","probe start\n");
	__u32 u4Width, u4Height;

	u4Width = LCD_GetScreenWidth();
	u4Height = LCD_GetScreenHeight();
	read_dts_data();
	memcpy((void *)(&g_rFBConfig), (const void *)FB_PHYSICAL_TO_VIRTUAL(ARM2_FBDRV_SHARE_MEMORY_PA)
		, sizeof(FB_CONFIG_T));
	g_rPanelSetting = g_rFBConfig.rFBPanelSetting;
	OSD_Init_DAL(false);
	dal_osd_init(dal_base_pa, dal_base_pa, u4Width, u4Height);
	memset(dal_base_va, 0x0, dal_buf_size);
	DAL_Init(dal_base_va, dal_base_pa, u4Width, u4Height);
	DAL_PRINT(DAL_LOG_LVL_INFO,"","probe success\n");
	return 0;
}
/*static int dal_remove(struct platform_device *pdev) {*/
static int dal_remove(void) {

	
}

static struct platform_driver dal_driver = {
	.probe      = dal_probe,
	/*.remove       = __devexit_p(mtk_fb_remove),*/
	.remove       = dal_remove,
	.driver = {
		.name = "dal",
		.owner = THIS_MODULE,
	},
};

static void  dal_init(void)
{
	DAL_PRINT(DAL_LOG_LVL_INFO,"","init  start1\n");
	platform_driver_register(&dal_driver);
	dal_probe();
	DAL_PRINT(DAL_LOG_LVL_INFO,"","init end\n");
	return ;
}

static void dal_exit(void)
{
	platform_driver_unregister(&dal_driver);
	dal_remove();
}

module_init(dal_init);
module_exit(dal_exit);

MODULE_LICENSE("GPL");

