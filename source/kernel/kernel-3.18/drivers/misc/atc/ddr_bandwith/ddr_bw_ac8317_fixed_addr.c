// SPDX-License-Identifier: GPL-2.0
/*
 * AC8317 DDR bandwidth monitor driver (Linux 3.18, fixed base address)
 *
 * Based on user-provided legacy implementation:
 *   - ddr_bandwith.c
 *   - ddr_bandwith.h
 *
 * Notes:
 * 1) This version does NOT use DTS. Register base is hard-coded.
 * 2) It preserves the original monitor flow and percent formula.
 * 3) It exposes sysfs nodes under:
 *      /sys/devices/platform/ddr_bw/
 *
 * Build:
 *   make -C <kernel_dir> M=$PWD modules
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>

#define DDR_BW_DRV_NAME          "ddr_bw"

/*
 * Hard-coded register base.
 * IMPORTANT:
 * The original header uses:
 *   IO_BASE_VA      0xFD000000
 *   DRAM_DMARB_BASE (IO_BASE_VA + 0x06000)
 *
 * In Linux driver, ioremap() should use PHYSICAL address.
 * If 0xF0006000 is already a CPU physical MMIO address on your platform,
 * keep it as-is. Otherwise replace it with the real physical base.
 */
#define DRAM_DDRPHY_BASE         0xF005A008
#define DRAM_DMARB_PHY_BASE      0xF0006000
#define DRAM_DMARB_MAP_SIZE      0x1000

#define MAX_TIMES                100
#define MAX_STEP                 10000
#define MAX_AGT_NUM              16

#define DRAMB_REG_BM             0x00000080
#define BM_BMGP1_AG_OFFSET       8
#define BIT_BMGP1_EN             (1U << 15)

#define DRAMB_REG_BMCYC          0x0000008C
#define DRAMB_REG_BM0            0x00000090
#define DRAMB_REG_BM3            0x0000009C

struct ac8317_agtinfo {
	u32 group_id;
	u32 agent_id;
	u32 priority;
	const char *name;
};

static struct ac8317_agtinfo g_agt_tbl[MAX_AGT_NUM] = {
	{1,  0,  3,  "mali_reg(gfx_3d)/demux_req1(ts_demux)/ddi_req(ts_demux)/img_resz_req(img_resz)/gfx24bpp_req(gfx24bpp)"},
	{1,  1, 13,  "mali_req(gfx_3d)/mgra_req(AP_2D)/jpgdec_req(jpgdec)/rle_req(rle_dec)/png_req(png_decoder)/gif_req(gif_decoder)/osd_resz_req(osd_resz)/demux_req2(ts_demux)"},
	{1,  2,  2,  "CA7"},
	{1,  3, 14,  "arm9_req/USB1/NFI_req/bim_local_req_req(has layer2 arbitor)/USB0_req/irt_dma"},
	{1,  4,  1,  "mphone_req/aout_req/aout2_req/iec_req/iec2_req/gps_aout_req/pcm_rx_req/pcm_tx_req"},
	{1,  5,  9,  "audio_largl_0_req/audio_largl_1_req/audio2_largl_0_req/spi_dram_req/spi_moto1_dram_req/spi_moto2_dram_req"},
	{1,  6, 15,  "msdc_0/1/2_dram_hreqm/rfi_dram_req/au_peri_larb_2/3_req"},
	{1,  7,  7,  "VDEC Pred/MC/VDEC CABAC/VDEC PP/connect to 0/VDEC VLD"},
	{1,  8,  8,  "VDEC MC/Pred"},
	{1,  9,  6,  "audio-vdo out/asm_rd_req/ain_dma_req/vdo_dram_req"},
	{1, 10, 11,  "cor_wreq/dspreq/c2req/edcw_req/screq/correq/cddec_req/pio_req"},
	{1, 11, 12,  "vdec/adsp/mc req/t32_ic_rd/risc_dram_req/t8032_dram_req"},
	{1, 12,  5,  "VDO:VDO_F/VDO_R"},
	{1, 13,  4,  "OSD:OSD1-5/OSD2_R/OSD3_R"},
	{1, 14, 10,  "TVD:TVD/VBI/WRITE_CHANNEL"},
	{1, 15, 15,  "2D/img_risize0/png/jpeg/gif/img_risize0"},
};

struct ac8317_ddr_bw {
	struct device *dev;
	void __iomem *base;
	struct mutex lock;

	u32 agent_id;
	u32 times;
	u32 step_ms;
	u32 data_width;

	u32 last_agent_counter;
	u32 last_total_counter;
	u32 last_percent;
	u32 last_total_bw;
	char last_name[320];
};

static inline u32 bw_readl(struct ac8317_ddr_bw *bw, u32 reg)
{
	return readl(bw->base + reg);
}

static inline void bw_writel(struct ac8317_ddr_bw *bw, u32 val, u32 reg)
{
	writel(val, bw->base + reg);
}

static int ac8317_run_sample(struct ac8317_ddr_bw *bw, u32 agent_id, u32 step_ms)
{
	u32 counter, total_counter, temp, percent;

	if (agent_id >= MAX_AGT_NUM)
		return -EINVAL;

	if (step_ms > MAX_STEP)
		step_ms = MAX_STEP;

	/* Set Dram Bus monitor cycle number */
	bw_writel(bw, 0xFFFFFFFF, DRAMB_REG_BMCYC);

	/* Set Dram Bandwidth monitor Agent ID */
	bw_writel(bw, agent_id << BM_BMGP1_AG_OFFSET, DRAMB_REG_BM);

	/* Enable Group1 Bandwidth Monitor */
	bw_writel(bw, BIT_BMGP1_EN | (agent_id << BM_BMGP1_AG_OFFSET), DRAMB_REG_BM);

	mdelay(step_ms);

	counter = bw_readl(bw, DRAMB_REG_BM0);
	total_counter = bw_readl(bw, DRAMB_REG_BM3);

	/* Reset Dram BandWidth Group */
	bw_writel(bw, agent_id << BM_BMGP1_AG_OFFSET, DRAMB_REG_BM);

	//temp = total_counter / 1000 * 718 / 100;
	temp = total_counter / 100;
	if (temp == 0)
		percent = 0;
	else
		percent = counter / temp;

	bw->last_agent_counter = counter;
	bw->last_total_counter = total_counter;
	bw->last_total_bw = 2884 * bw->data_width;
	bw->last_percent = percent;
	strlcpy(bw->last_name, g_agt_tbl[agent_id].name, sizeof(bw->last_name));

	dev_info(bw->dev,
		 "agent=%u counter=0x%x total=0x%x percent=%u%% total_bw=%dMB/s name=%s\n",
		 agent_id, counter, total_counter, percent, bw->last_total_bw, bw->last_name);

	return 0;
}

static ssize_t agent_id_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%u\n", bw->agent_id);
}

static ssize_t agent_id_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	unsigned long val;

	if (kstrtoul(buf, 0, &val))
		return -EINVAL;
	if (val >= MAX_AGT_NUM)
		return -EINVAL;

	mutex_lock(&bw->lock);
	bw->agent_id = val;
	mutex_unlock(&bw->lock);
	return count;
}

static ssize_t step_ms_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%u\n", bw->step_ms);
}

static ssize_t step_ms_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	unsigned long val;

	if (kstrtoul(buf, 0, &val))
		return -EINVAL;
	if (val == 0 || val > MAX_STEP)
		return -EINVAL;

	mutex_lock(&bw->lock);
	bw->step_ms = val;
	mutex_unlock(&bw->lock);
	return count;
}

static ssize_t times_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%u\n", bw->times);
}

static ssize_t times_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	unsigned long val;

	if (kstrtoul(buf, 0, &val))
		return -EINVAL;
	if (val == 0 || val > MAX_TIMES)
		return -EINVAL;

	mutex_lock(&bw->lock);
	bw->times = val;
	mutex_unlock(&bw->lock);
	return count;
}

static ssize_t run_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE,
			 "echo 1 > run to sample current agent %u time(s)\n",
			 1U);
}

static ssize_t run_store(struct device *dev,
			 struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	unsigned long val;
	u32 i, agent_id, times, step_ms;
	int ret = 0;

	if (kstrtoul(buf, 0, &val))
		return -EINVAL;
	if (val != 1)
		return -EINVAL;

	mutex_lock(&bw->lock);
	agent_id = bw->agent_id;
	times = bw->times;
	step_ms = bw->step_ms;

	for (i = 0; i < times; i++) {
		ret = ac8317_run_sample(bw, agent_id, step_ms);
		if (ret)
			break;
	}
	mutex_unlock(&bw->lock);

	if (ret)
		return ret;
	return count;
}

static ssize_t scan_all_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	unsigned long val;
	u32 i, step_ms;
	int ret = 0;

	if (kstrtoul(buf, 0, &val))
		return -EINVAL;
	if (val != 1)
		return -EINVAL;

	mutex_lock(&bw->lock);
	step_ms = bw->step_ms;
	for (i = 0; i < MAX_AGT_NUM; i++) {
		ret = ac8317_run_sample(bw, i, step_ms);
		if (ret)
			break;
	}
	mutex_unlock(&bw->lock);

	if (ret)
		return ret;
	return count;
}

static ssize_t scan_all_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "echo 1 > scan_all\n");
}

static ssize_t raw_agent_counter_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "0x%x\n", bw->last_agent_counter);
}

static ssize_t raw_total_counter_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "0x%x\n", bw->last_total_counter);
}

static ssize_t raw_total_bw_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%dMB/s\n", bw->last_total_bw);
}

static ssize_t percent_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%u\n", bw->last_percent);
}

static ssize_t agent_name_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%s\n", bw->last_name);
}

static ssize_t summary_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct ac8317_ddr_bw *bw = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE,
			 "agent_id=%u\n"
			 "agent_name=%s\n"
			 "step_ms=%u\n"
			 "times=%u\n"
			 "raw_agent_counter=0x%x\n"
			 "raw_total_counter=0x%x\n"
			 "raw_total_bw=%dMB/s\n"
			 "percent=%u%%\n",
			 bw->agent_id,
			 bw->last_name[0] ? bw->last_name : g_agt_tbl[bw->agent_id].name,
			 bw->step_ms,
			 bw->times,
			 bw->last_agent_counter,
			 bw->last_total_counter,
			 bw->last_total_bw,
			 bw->last_percent);
}

static DEVICE_ATTR_RW(agent_id);
static DEVICE_ATTR_RW(step_ms);
static DEVICE_ATTR_RW(times);
static DEVICE_ATTR_RW(run);
static DEVICE_ATTR_RW(scan_all);
static DEVICE_ATTR_RO(raw_agent_counter);
static DEVICE_ATTR_RO(raw_total_counter);
static DEVICE_ATTR_RO(raw_total_bw);
static DEVICE_ATTR_RO(percent);
static DEVICE_ATTR_RO(agent_name);
static DEVICE_ATTR_RO(summary);

static struct attribute *ac8317_ddr_bw_attrs[] = {
	&dev_attr_agent_id.attr,
	&dev_attr_step_ms.attr,
	&dev_attr_times.attr,
	&dev_attr_run.attr,
	&dev_attr_scan_all.attr,
	&dev_attr_raw_agent_counter.attr,
	&dev_attr_raw_total_counter.attr,
	&dev_attr_raw_total_bw.attr,
	&dev_attr_percent.attr,
	&dev_attr_agent_name.attr,
	&dev_attr_summary.attr,
	NULL,
};

static const struct attribute_group ac8317_ddr_bw_attr_group = {
	.attrs = ac8317_ddr_bw_attrs,
};

static int ac8317_ddr_bw_probe(struct platform_device *pdev)
{
	struct ac8317_ddr_bw *bw;
	int ret;
	void __iomem *phy_base;

	bw = devm_kzalloc(&pdev->dev, sizeof(*bw), GFP_KERNEL);
	if (!bw)
		return -ENOMEM;

	bw->dev = &pdev->dev;
	mutex_init(&bw->lock);

	bw->base = ioremap(DRAM_DMARB_PHY_BASE, DRAM_DMARB_MAP_SIZE);
	if (!bw->base) {
		dev_err(&pdev->dev, "base ioremap failed, phy=0x%x size=0x%x\n",
			DRAM_DMARB_PHY_BASE, DRAM_DMARB_MAP_SIZE);
		return -ENOMEM;
	}

	phy_base = ioremap(DRAM_DDRPHY_BASE, 0x4);
	if (!phy_base) {
		dev_err(&pdev->dev, "ddrphy_base ioremap failed, phy=0x%x size=0x%x\n",
			DRAM_DMARB_PHY_BASE, DRAM_DMARB_MAP_SIZE);
		return -ENOMEM;
	}
	bw->data_width = readl(phy_base);
	dev_info(&pdev->dev,
		 "base=0x%x value=0x%x\n", DRAM_DDRPHY_BASE, bw->data_width);
	if (bw->data_width)
		bw->data_width = 2;
	else
		bw->data_width = 1;

	bw->agent_id = 2;    /* default CA7 */
	bw->times = 1;
	bw->step_ms = 100;
	strlcpy(bw->last_name, g_agt_tbl[bw->agent_id].name, sizeof(bw->last_name));

	platform_set_drvdata(pdev, bw);

	ret = sysfs_create_group(&pdev->dev.kobj, &ac8317_ddr_bw_attr_group);
	if (ret) {
		dev_err(&pdev->dev, "sysfs_create_group failed\n");
		iounmap(bw->base);
		return ret;
	}

	dev_info(&pdev->dev,
		 "AC8317 DDR BW monitor probed, base=0x%x mapped=%p\n",
		 DRAM_DMARB_PHY_BASE, bw->base);

	return 0;
}

static int ac8317_ddr_bw_remove(struct platform_device *pdev)
{
	struct ac8317_ddr_bw *bw = platform_get_drvdata(pdev);

	sysfs_remove_group(&pdev->dev.kobj, &ac8317_ddr_bw_attr_group);

	if (bw && bw->base)
		iounmap(bw->base);

	return 0;
}

static struct platform_driver ac8317_ddr_bw_driver = {
	.probe  = ac8317_ddr_bw_probe,
	.remove = ac8317_ddr_bw_remove,
	.driver = {
		.name = DDR_BW_DRV_NAME,
		.owner = THIS_MODULE,
	},
};

static struct platform_device *ac8317_ddr_bw_pdev;

static int __init ac8317_ddr_bw_init(void)
{
	int ret;

	ret = platform_driver_register(&ac8317_ddr_bw_driver);
	if (ret)
		return ret;

	ac8317_ddr_bw_pdev = platform_device_register_simple(DDR_BW_DRV_NAME,
							     -1, NULL, 0);
	if (IS_ERR(ac8317_ddr_bw_pdev)) {
		ret = PTR_ERR(ac8317_ddr_bw_pdev);
		platform_driver_unregister(&ac8317_ddr_bw_driver);
		return ret;
	}

	pr_info("ac8317 ddr bw module init done\n");
	return 0;
}


static void __exit ac8317_ddr_bw_exit(void)
{
	platform_device_unregister(ac8317_ddr_bw_pdev);
	platform_driver_unregister(&ac8317_ddr_bw_driver);
	pr_info("ac8317 ddr bw module exit\n");
}

module_init(ac8317_ddr_bw_init);
module_exit(ac8317_ddr_bw_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AC8317 DDR bandwidth monitor driver (fixed address, sysfs)");
MODULE_AUTHOR("OpenAI");
