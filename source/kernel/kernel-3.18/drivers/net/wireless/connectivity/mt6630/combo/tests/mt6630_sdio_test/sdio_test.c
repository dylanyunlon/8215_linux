/*
 * sdio_test.c - sdio test driver
 *
 * Copyright (c) 2018 AutoChips Inc.
 * Author: Rocky Pan <changle.pan@autochips.com>
 *
 * This file is released under the GPLv2
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>

#include "mt6630_reg.h"

#if !defined(CONFIG_ARCH_AC83XX) \
	&& !defined(CONFIG_ARCH_AC823X) \
	&& !defined(CONFIG_ARCH_AC8X)

	#define CONFIG_ARCH_AC8X    1

#endif

#define DRV_NAME "sdio_test"

#define SDIO_TEST_MAJOR 159

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "[CNN][sdio_test]" fmt

#define SDIO_TEST_INFO(fmt, arg...) do { \
	pr_info("[I]%s,%d: " fmt, __func__, __LINE__, ##arg); \
} while (0)

#define SDIO_TEST_ERR(fmt, arg...) do { \
	pr_info("[E]%s,%d: " fmt, __func__, __LINE__, ##arg); \
} while (0)

static struct sdio_func *g_sdio_test_func[8] = { NULL };

static const char sdio_test_usage[] =
"Usage:\n"
"   echo [arg] > /dev/" DRV_NAME "\n"
"arg:\n"
"   auto        - all func test\n"
"   probe       - probe sdio card\n"
"   remove      - remove sdio card\n"
"   read        - read test\n"
"   write       - write test\n"
"   cmd         - cmd test\n"
"   loop [val]  - set test loop\n"
"   help        - Display this help message\n"
;

#define DEFAULT_LOOP	100
#define BUF_U32_NUM		64

static int g_sdio_test_loop = DEFAULT_LOOP;

static const unsigned int g_pattern_data[] = {
	0xAA55AA55, 0xAA558080, 0x807F8080, 0x807F7F7F,
	0x807F7F7F, 0x404040BF, 0xBFBF40BF, 0xBFBF2020,
	0x20DF2020, 0x20DFDFDF, 0x101010EF, 0xEFEF10EF,
	0xEFEF0808, 0x08F70808, 0x08F7F7F7, 0x040404FB,
	0xFBFB04FB, 0xFBFB0202, 0x02FD0202, 0x02FDFDFD,
	0x010101FE, 0xFEFE01FE, 0xFEFE0000, 0x00FF0000,
	0x00FFFFFF, 0x000000FF, 0xFFFF00FF, 0xFFFF0000,
	0xFF0FFF00, 0xFFCCC3CC, 0xC33CCCFF, 0xFEFFFEEF,
	0xFFDFFFDD, 0xFFFBFFFB, 0xBFFF7FFF, 0x77F7BDEF,
	0xFFF0FFF0, 0x0FFCCC3C, 0xCC33CCCF, 0xFFEFFFEE,
	0xFFFDFFFD, 0xDFFFBFFF, 0xBBFFF7FF, 0xF77F7BDE
};

#define PATTERN_DATA_NUM	ARRAY_SIZE(g_pattern_data)

#if 0
static const unsigned int g_pattern_cmd[] = {
	0x55, 0xAA, 0x5A, 0xA5,
	0x55, 0xAA, 0x5A, 0xA5,
	0x55, 0xAA, 0x5A, 0xA5,
	0x55, 0xAA, 0x5A, 0xA5
};
#elif 0
static const unsigned int g_pattern_cmd[] = {
	0xA55AAA55,
	0xA55AAA55,
	0xA55AAA55,
	0xA55AAA55
};
#else
static unsigned int *g_pattern_cmd = NULL;
#endif

#if 0
/* sdio host driver pio read: "ptr = sg_virt(sg);" will cause
 * paging request error when pass array buffer to sdio_readsb()
 */
static unsigned int g_sdio_test_rbuf[BUF_U32_NUM];
static unsigned int g_sdio_test_wbuf[PATTERN_DATA_NUM][BUF_U32_NUM];
#else
static unsigned int *g_sdio_test_rbuf = NULL;
static unsigned int (*g_sdio_test_wbuf)[BUF_U32_NUM] = NULL;
#endif

static void sido_test_buf_init(void)
{
	int i, j;

	SDIO_TEST_INFO("\n");

	g_pattern_cmd = (unsigned int *)kmalloc(
			BUF_U32_NUM * 4, GFP_KERNEL);
	if (!g_pattern_cmd) {
		SDIO_TEST_ERR("kmalloc failed\n");
		return;
	}
	for (i = 0; i < BUF_U32_NUM; i++)
		g_pattern_cmd[i] = 0xA55AAA55;

	g_sdio_test_rbuf = (unsigned int *)kmalloc(
			BUF_U32_NUM * 4, GFP_KERNEL);
	if (!g_sdio_test_rbuf) {
		SDIO_TEST_ERR("kmalloc failed\n");
		kfree(g_pattern_cmd);
		g_pattern_cmd = NULL;
		return;
	}

	g_sdio_test_wbuf = (unsigned int (*)[BUF_U32_NUM])kmalloc(
			PATTERN_DATA_NUM * BUF_U32_NUM * 4, GFP_KERNEL);
	if (!g_sdio_test_wbuf) {
		SDIO_TEST_ERR("kmalloc failed\n");
		kfree(g_pattern_cmd);
		g_pattern_cmd = NULL;
		kfree(g_sdio_test_rbuf);
		g_sdio_test_rbuf = NULL;
		return;
	}

	for (i = 0; i < PATTERN_DATA_NUM; i++)
		for (j = 0; j < BUF_U32_NUM; j++)
			g_sdio_test_wbuf[i][j] = g_pattern_data[i];
}

static void sido_test_buf_deinit(void)
{
	kfree(g_pattern_cmd);
	g_pattern_cmd = NULL;

	kfree(g_sdio_test_rbuf);
	g_sdio_test_rbuf = NULL;

	kfree(g_sdio_test_wbuf);
	g_sdio_test_wbuf = NULL;
}

struct _mt6630_gpio {
	const char *name;
	struct gpio_desc *desc;
	int num;
};

enum {
	PWN_PIN,
	RST_PIN,
	PIN_NUM
};

#ifndef CONFIG_ATC_OS_VERSION_JB2
#if defined(CONFIG_ARCH_AC8X)
static struct _mt6630_gpio mt6630_gpio[PIN_NUM] = {
	{ "pwn-gpios", NULL, -1 },
	{ "rst-gpios", NULL, -1 }
};
#else
static struct _mt6630_gpio mt6630_gpio[PIN_NUM] = {
	{ "pwn", NULL, -1 },
	{ "rst", NULL, -1 }
};
#endif
#endif

#if defined(CONFIG_ARCH_AC8X)

static int sdio_test_gpio_init(void)
{
	struct device_node *node = NULL;
	int ret = 0;

	SDIO_TEST_INFO("start\n");

	node = of_find_compatible_node(NULL, NULL,
			"Autochips,ac8x-CNNdynamic");
	if (NULL == node) {
		SDIO_TEST_ERR("[6630-gpio] can't find device_node\n");
		return -1;
	}

	mt6630_gpio[PWN_PIN].num = of_get_named_gpio(node,
			mt6630_gpio[PWN_PIN].name, 0);

	if (!gpio_is_valid(mt6630_gpio[PWN_PIN].num)) {
		SDIO_TEST_ERR("[6630-gpio] get invalid PWN_PIN: %d\n",
				mt6630_gpio[PWN_PIN].num);
		return -1;
	}

	ret = gpio_request(mt6630_gpio[PWN_PIN].num, "6630_PMU_EN");
	if (ret) {
		SDIO_TEST_ERR("[6630-gpio] gpio_request PWN_PIN(%d) failed(%d)\n",
				mt6630_gpio[PWN_PIN].num, ret);
		return ret;
	}
	ret = gpio_direction_output(mt6630_gpio[PWN_PIN].num, 0);
	if (ret) {
		SDIO_TEST_ERR("[6630-gpio] gpio_direction_output "
				"PWN_PIN(%d) failed(%d)\n",
				mt6630_gpio[PWN_PIN].num, ret);
		return ret;
	}

	mt6630_gpio[RST_PIN].num = of_get_named_gpio(node,
			mt6630_gpio[RST_PIN].name, 0);

	if (!gpio_is_valid(mt6630_gpio[RST_PIN].num)) {
		SDIO_TEST_ERR("[6630-gpio] get invalid RST_PIN: %d\n",
				mt6630_gpio[RST_PIN].num);
		return -1;
	}

	ret = gpio_request(mt6630_gpio[RST_PIN].num, "6630_SYSRST");
	if (ret) {
		SDIO_TEST_ERR("[6630-gpio] gpio_request RST_PIN(%d) failed(%d)\n",
				mt6630_gpio[RST_PIN].num, ret);
		return ret;
	}
	ret = gpio_direction_output(mt6630_gpio[RST_PIN].num, 0);
	if (ret) {
		SDIO_TEST_ERR("[6630-gpio] gpio_direction_output "
				"RST_PIN(%d) failed(%d)\n",
				mt6630_gpio[RST_PIN].num, ret);
		return ret;
	}

	SDIO_TEST_INFO("[6630-gpio] PWN_PIN: %d RST_PIN: %d\n",
			mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);

	SDIO_TEST_INFO("end ret(%d)\n", ret);

	return ret;
}

#else

static int sdio_test_gpio_init(void)
{
	SDIO_TEST_INFO("[6630-gpio] PWN_PIN: %d RST_PIN: %d\n",
			mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);

	return 0;
}

#endif

static void sdio_test_gpio_deinit(void)
{
	SDIO_TEST_INFO("start\n");

	if (!gpio_is_valid(mt6630_gpio[PWN_PIN].num)
			|| !gpio_is_valid(mt6630_gpio[RST_PIN].num)) {
		SDIO_TEST_INFO("[6630-gpio] invalid PWN_PIN: %d RST_PIN: %d\n",
				mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
		return;
	}
	gpio_free(mt6630_gpio[PWN_PIN].num);
	gpio_free(mt6630_gpio[RST_PIN].num);

	SDIO_TEST_INFO("ok\n");
}

static int sido_detect_change(u32 enable)
{
#if defined(CONFIG_ARCH_AC8X)
	extern int sdhci_cadence_detect_change(u32 slot, u32 enable);
	#define STP_SDIO_SLOT   1

	return sdhci_cadence_detect_change(STP_SDIO_SLOT, enable);

#elif defined(CONFIG_ARCH_AC83XX)
	extern int msdc_detect_change(u32 slot, u32 enable, u32 type);
	#define STP_SDIO_SLOT   2

	return msdc_detect_change(STP_SDIO_SLOT, enable, 0);

#elif defined(CONFIG_ARCH_AC823X)
	extern int msdc_detect_change(u32 slot, u32 enable, u32 type);
	#define STP_SDIO_SLOT   1

	return msdc_detect_change(STP_SDIO_SLOT, enable, 0);

#else
	return 0;
#endif
}

static int test_probe(void)
{
	int i;
	int ret;

	SDIO_TEST_INFO("\n");

#if defined(CONFIG_ARCH_AC8X)
	if (gpio_is_valid(mt6630_gpio[PWN_PIN].num)
			&& gpio_is_valid(mt6630_gpio[RST_PIN].num)) {

		gpio_set_value(mt6630_gpio[PWN_PIN].num, 0);
		gpio_set_value(mt6630_gpio[RST_PIN].num, 0);
		msleep(20);
		gpio_set_value(mt6630_gpio[PWN_PIN].num, 1);
		msleep(30);
		gpio_set_value(mt6630_gpio[RST_PIN].num, 1);
		msleep(100);
		SDIO_TEST_INFO("[6630-gpio] pull up PWN_PIN: %d RST_PIN: %d\n",
				mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
	} else {
		SDIO_TEST_ERR("[6630-gpio] invalid PWN_PIN: %d RST_PIN: %d\n",
				mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
	}
#endif

	ret = sido_detect_change(1);
	if (ret) {
		SDIO_TEST_ERR("fail(%d)\n", ret);
		return ret;
	}

	for (i = 50; i > 0; i--) {
		if (NULL != g_sdio_test_func[1]
				&& NULL != g_sdio_test_func[2]) {
			SDIO_TEST_INFO("ok\n");
			return 0;
		}
		msleep(100);
	}
	SDIO_TEST_ERR("timeout\n");

	return -ETIMEDOUT;
}

static int test_remove(void)
{
	int i;
	int ret;

	SDIO_TEST_INFO("\n");

#if defined(CONFIG_ARCH_AC8X)
	if (gpio_is_valid(mt6630_gpio[PWN_PIN].num)
			&& gpio_is_valid(mt6630_gpio[RST_PIN].num)) {

		gpio_set_value(mt6630_gpio[PWN_PIN].num, 0);
		gpio_set_value(mt6630_gpio[RST_PIN].num, 0);
		SDIO_TEST_INFO("[6630-gpio] pull down PWN_PIN: %d RST_PIN: %d\n",
				mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
	} else {
		SDIO_TEST_ERR("[6630-gpio] invalid PWN_PIN: %d RST_PIN: %d\n",
				mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
	}

	/* msleep(10); */
	msleep(100);
#endif

	ret = sido_detect_change(0);
	if (ret) {
		SDIO_TEST_ERR("fail(%d)\n", ret);
		return ret;
	}

	for (i = 50; i > 0; i--) {
		if (NULL == g_sdio_test_func[1]
				&& NULL == g_sdio_test_func[2]) {
			SDIO_TEST_INFO("ok\n");
			return 0;
		}
		msleep(100);
	}
	SDIO_TEST_ERR("timeout\n");

	return -ETIMEDOUT;
}

static unsigned char sdio_test_readb(unsigned int addr, int *err_ret)
{
	struct sdio_func *func = g_sdio_test_func[1];
	unsigned char val;

	if (!func)
		return -ENODEV;

	sdio_claim_host(func);
	val = sdio_readb(func, addr, err_ret);
	sdio_release_host(func);

	return val;
}

static int sdio_test_writeb(unsigned int addr, unsigned char val)
{
	struct sdio_func *func = g_sdio_test_func[1];
	int ret;

	if (!func)
		return -ENODEV;

	sdio_claim_host(func);
	sdio_writeb(func, val, addr, &ret);
	sdio_release_host(func);

	return ret;
}

// mt6630 suggest cmd52 instead of cmd53 for pre-tuning settings
static unsigned int sdio_test_readl(unsigned int addr, int *err_ret)
{
	struct sdio_func *func = g_sdio_test_func[1];
	unsigned char data[4];
	//unsigned int val;
	int i;
	int ret;

	if (!func)
		return -ENODEV;
#if 1
	for (i = 0; i < 4; i++) {
		sdio_claim_host(func);
		data[i] = sdio_readb(func, addr + i, &ret);
		sdio_release_host(func);
		if (ret) {
			SDIO_TEST_ERR("read addr(0x%08x) ret(%d)\n",
					addr + i, ret);
			*err_ret = ret;
			return -1;
		}
	}
#else
	sdio_claim_host(func);
	val = sdio_readl(func, addr, &err_ret);
	sdio_release_host(func);
	return val;
#endif

	return *(unsigned int *)data;
}

static int sdio_test_writel(unsigned int addr, unsigned int val)
{
	struct sdio_func *func = g_sdio_test_func[1];
	unsigned char *data = (unsigned char *)&val;
	unsigned int tmp;
	int i;
	int ret;

	if (!func)
		return -ENODEV;
#if 1
	for (i = 0; i < 4; i++) {
		sdio_claim_host(func);
		sdio_writeb(func, data[i], addr + i, &ret);
		sdio_release_host(func);
		if (ret) {
			SDIO_TEST_ERR("write data(0x%02x) addr(0x%08x) ret(%d)\n",
					data[i], addr + i, ret);
			return ret;
		}
	}
#else
	sdio_claim_host(func);
	sdio_writel(func, val, addr, &ret);
	sdio_release_host(func);
#endif
	tmp = sdio_test_readl(addr, &ret);
	if (ret) {
		SDIO_TEST_ERR("read back addr(0x%08x) ret(%d)\n", addr, ret);
		return ret;
	}
	if (tmp != val) {
		SDIO_TEST_ERR("read back 0x%08x != write 0x%08x\n",
				tmp, val);
		return -EIO;
	}

	return 0;
}

static int sdio_test_read_buf(unsigned int addr, void *dst, int count)
{
	struct sdio_func *func = g_sdio_test_func[1];
	int ret;

	if (!func)
		return -ENODEV;

	sdio_claim_host(func);
	ret = sdio_readsb(func, dst, addr, count);
	sdio_release_host(func);

	return ret;
}

static int sdio_test_write_buf(unsigned int addr, void *src, int count)
{
	struct sdio_func *func = g_sdio_test_func[1];
	int ret;

	if (!func)
		return -ENODEV;

	sdio_claim_host(func);
	ret = sdio_writesb(func, addr, src, count);
	sdio_release_host(func);

	return ret;
}

static int test_read(void)
{
	int loop, i;
	int ret;

	SDIO_TEST_INFO("g_sdio_test_loop(%d) ...\n", g_sdio_test_loop);

	for (loop = 0; loop < g_sdio_test_loop; loop++) {
		for (i = 0; i < PATTERN_DATA_NUM; i++) {
			ret = sdio_test_writel(MCR_WTMDPCR0, g_pattern_data[i]);
			if (ret) {
				SDIO_TEST_ERR("write g_pattern_data[%d](0x%08x) ret(%d)\n",
						i, g_pattern_data[i], ret);
				return ret;
			}
			ret = sdio_test_read_buf(MCR_WTMDR, g_sdio_test_rbuf,
					4 * BUF_U32_NUM);
			if (ret) {
				SDIO_TEST_ERR("i(%d): read buf ret(%d)\n", i, ret);
				return ret;
			}
			if (memcmp(g_sdio_test_rbuf, g_sdio_test_wbuf[i],
						4 * BUF_U32_NUM)) {
				SDIO_TEST_ERR("i(%d): memcmp differ\n", i);
				SDIO_TEST_ERR("read test FAIL !!!\n");
				return -EIO;
			}
		}
	}

	SDIO_TEST_INFO("ok\n");

	return 0;
}

static int test_write(void)
{
	unsigned int val;
	int loop, i;
	int ret;

	SDIO_TEST_INFO("g_sdio_test_loop(%d) ...\n", g_sdio_test_loop);

	for (loop = 0; loop < g_sdio_test_loop; loop++) {
		for (i = 0; i < PATTERN_DATA_NUM; i++) {
			ret = sdio_test_writel(MCR_WTMDPCR1, g_pattern_data[i]);
			if (ret) {
				SDIO_TEST_ERR("write g_pattern_data[%d](0x%08x) ret(%d)\n",
						i, g_pattern_data[i], ret);
				return ret;
			}
			ret = sdio_test_write_buf(MCR_WTMDR, g_sdio_test_wbuf[i],
					4 * BUF_U32_NUM);
			if (ret) {
				SDIO_TEST_ERR("i(%d): write buf ret(%d)\n", i, ret);
				return ret;
			}
			val = sdio_test_readl(MCR_WTMCR, &ret);
			if (ret) {
				SDIO_TEST_ERR("read MCR_WTMCR ret(%d)\n", ret);
				return ret;
			}
			if (val & WMTCR_TEST_MODE_STATUS) {
				SDIO_TEST_ERR("MCR_WTMCR(0x%08x)\n", val);
				SDIO_TEST_ERR("write test FAIL !!!\n");
				return -EIO;
			}
		}
	}

	SDIO_TEST_INFO("ok\n");

	return 0;
}

static int test_cmd(void)
{
	unsigned int cmd;
	int loop, i;
	int ret;

	SDIO_TEST_INFO("g_sdio_test_loop(%d) ...\n", g_sdio_test_loop);

	for (loop = 0; loop < g_sdio_test_loop; loop++) {
		for (i = 0; i < BUF_U32_NUM; i++) {
			ret = sdio_test_writel(MCR_WTMDPCR0, g_pattern_cmd[i]);
			if (ret) {
				SDIO_TEST_ERR("write g_pattern_cmd[%d](0x%08x) ret(%d)\n",
						i, g_pattern_cmd[i], ret);
				return ret;
			}
			cmd = sdio_test_readl(MCR_WTMDPCR0, &ret);
			if (ret) {
				SDIO_TEST_ERR("read back cmd ret(%d)\n", ret);
				return -EIO;
			}
			if (cmd != g_pattern_cmd[i]) {
				SDIO_TEST_ERR("i(%d): read back 0x%08x != write 0x%08x\n",
						i, cmd, g_pattern_cmd[i]);
				SDIO_TEST_ERR("cmd test FAIL !!!\n");
				return -EIO;
			}
		}
	}

	SDIO_TEST_INFO("ok\n");

	return 0;
}

static int test_auto(void)
{
	int ret;

	SDIO_TEST_INFO("\n");

	ret = test_remove();
	if (ret)
		return ret;

	ret = test_probe();
	if (ret)
		return ret;

	ret = test_read();
	if (ret)
		return ret;

	ret = test_write();
	if (ret)
		return ret;

	ret = test_cmd();
	if (ret)
		return ret;

	SDIO_TEST_INFO("ok\n");

	return 0;
}

static int test_help(void)
{
	SDIO_TEST_INFO("%s\n", sdio_test_usage);
	return 0;
}

static int sdio_test_open(struct inode *inode, struct file *file)
{
	//SDIO_TEST_INFO("\n");
	return 0;
}

static int sdio_test_close(struct inode *inode, struct file *file)
{
	//SDIO_TEST_INFO("\n");
	return 0;
}

static ssize_t sdio_test_read(struct file *filp,
		char __user *buf, size_t count, loff_t *f_pos)
{
#if 0 // FIXME: cat /dev/sdio_test will not return
	int ret;

	if (sizeof(sdio_test_usage) < count)
		ret = copy_to_user(buf, sdio_test_usage,
				sizeof(sdio_test_usage));
	else
		ret = copy_to_user(buf, sdio_test_usage, count);

	return ret ? ret : count;
#else
	SDIO_TEST_ERR("echo help > /dev/sdio_test to get help\n");

	return -EPERM;
#endif
}

static ssize_t sdio_test_write(struct file *filp,
		const char __user *buf, size_t count, loff_t *f_pos)
{
	char cmd[64];
	int ret;

	if (count >= 64) {
		SDIO_TEST_ERR("%s\n", sdio_test_usage);
		return -EINVAL;
	}
	if (copy_from_user(cmd, buf, count)) {
		SDIO_TEST_ERR("fail to get cmd\n");
		return -EFAULT;
	}
	cmd[count] = '\0';
	SDIO_TEST_INFO("cmd: %s\n", cmd);

	if (!strncmp("auto", cmd, 4))
		ret = test_auto();
	else if (!strncmp("probe", cmd, 5))
		ret = test_probe();
	else if (!strncmp("remove", cmd, 6))
		ret = test_remove();
	else if (!strncmp("read", cmd, 4))
		ret = test_read();
	else if (!strncmp("write", cmd, 5))
		ret = test_write();
	else if (!strncmp("cmd", cmd, 3))
		ret = test_cmd();
	else if (!strncmp("loop", cmd, 4)) {
		ret = simple_strtol(cmd + 5, NULL, 0);
		if (ret < 1) {
			SDIO_TEST_ERR("invalid loop(%d)\n", ret);
			ret = -EINVAL;
		} else {
			SDIO_TEST_INFO("change g_sdio_test_loop: %d --> %d\n", g_sdio_test_loop, ret);
			g_sdio_test_loop = ret;
			ret = 0;
		}
	}
	else if (!strncmp("help", cmd, 4)) {
		ret = test_help();
	}
	else {
		SDIO_TEST_ERR("%s\n", sdio_test_usage);
		return -EINVAL;
	}

	if (ret) {
		SDIO_TEST_ERR("fail(%d)\n", ret);
		return ret;
	} else {
		SDIO_TEST_INFO("ok\n");
	}

	return count;
}

static const struct file_operations sdio_test_fops = {
	.open = sdio_test_open,
	.release = sdio_test_close,
	.read = sdio_test_read,
	.write = sdio_test_write,
};

static struct class *sdio_test_class = NULL;
static struct device *sdio_test_dev = NULL;

static int sdio_test_api_init(void)
{
	int ret;

	SDIO_TEST_INFO("\n");

	sido_test_buf_init();

	ret = register_chrdev(SDIO_TEST_MAJOR, DRV_NAME,
			&sdio_test_fops);
	if (ret) {
		SDIO_TEST_ERR("register_chrdev() fail(%d)\n", ret);
		return ret;
	}
	sdio_test_class = class_create(THIS_MODULE, DRV_NAME);
	if (IS_ERR(sdio_test_class)) {
		SDIO_TEST_ERR("class_create() fail(%d)\n",
				(int)PTR_ERR(sdio_test_class));
		return PTR_ERR(sdio_test_class);
	}
	sdio_test_dev = device_create(sdio_test_class, NULL,
			MKDEV(SDIO_TEST_MAJOR, 0), NULL, DRV_NAME);
	if (IS_ERR(sdio_test_dev)) {
		SDIO_TEST_ERR("device_create() fail(%d)\n",
				(int)PTR_ERR(sdio_test_dev));
		return PTR_ERR(sdio_test_dev);
	}

	SDIO_TEST_INFO("ok\n");

	return 0;
}

static void sdio_test_api_deinit(void)
{
	SDIO_TEST_INFO("\n");

	if (sdio_test_dev) {
		device_destroy(sdio_test_class,
				MKDEV(SDIO_TEST_MAJOR, 0));
		sdio_test_dev = NULL;
	}
	if (sdio_test_class) {
		class_destroy(sdio_test_class);
		sdio_test_class = NULL;
	}
	unregister_chrdev(SDIO_TEST_MAJOR, DRV_NAME);
	sido_test_buf_deinit();

	SDIO_TEST_INFO("ok\n");
}

static const struct sdio_device_id sdio_test_ids[] = {
	/* MT6630 SDIO1: Wi-Fi, SDIO2: BGF */
	{ SDIO_DEVICE(0x037A, 0x6630) },
	{}
};

static int sdio_test_probe(struct sdio_func *func,
		const struct sdio_device_id *id)
{
	int ret;

	SDIO_TEST_INFO("probe sdio func(%d)\n", func->num);

	g_sdio_test_func[func->num] = func;
	sdio_claim_host(func);
	ret = sdio_enable_func(func);
	sdio_release_host(func);
	if (ret) {
		SDIO_TEST_ERR("sdio_enable_func() fail, ret(%d)\n", ret);
		return ret;
	}
	sdio_claim_host(func);
	ret = sdio_set_block_size(func, 512);
	sdio_release_host(func);
	if (ret) {
		SDIO_TEST_ERR("sdio_set_block_size() fail, ret(%d)\n", ret);
		return ret;
	}

	if (func->num == 1) {
		unsigned char tmp;

		ret = sdio_test_writeb(MCR_WTMCR, 0);
		if (ret) {
			SDIO_TEST_ERR("write MCR_WTMCR ret(%d)\n", ret);
			return ret;
		}
		tmp = sdio_test_readb(MCR_WTMCR, &ret);
		if (ret) {
			SDIO_TEST_ERR("read back MCR_WTMCR ret(%d)\n", ret);
			return ret;
		}
		if ((tmp & 0x01ff0103) != 0) {
			SDIO_TEST_ERR("read back 0x%08x != write 0x0\n", tmp);
			return -EIO;
		}
	}

	SDIO_TEST_INFO("ok\n");

	return 0;
}

static void sdio_test_remove(struct sdio_func *func)
{
	SDIO_TEST_INFO("remove sdio func(%d)\n", func->num);

	sdio_claim_host(func);
	sdio_disable_func(func);
	sdio_release_host(func);
	g_sdio_test_func[func->num] = NULL;

	SDIO_TEST_INFO("ok\n");
}

static struct sdio_driver sdio_test_client = {
	.name = DRV_NAME,
	.id_table = sdio_test_ids,
	.probe = sdio_test_probe,
	.remove = sdio_test_remove,
};

static int sdio_test_init(void)
{
	int ret;

	SDIO_TEST_INFO("\n");

	ret = sdio_test_gpio_init();
	if (ret) {
		SDIO_TEST_ERR("sdio_test_gpio_init() fail, ret(%d)\n", ret);
	}
	ret = sdio_test_api_init();
	if (ret) {
		SDIO_TEST_ERR("sdio_test_api_init() fail, ret(%d)\n", ret);
		sdio_test_gpio_deinit();
		return ret;
	}
	ret = sdio_register_driver(&sdio_test_client);
	if (ret) {
		SDIO_TEST_ERR("sdio_register_driver() fail, ret(%d)\n", ret);
		sdio_test_api_deinit();
		sdio_test_gpio_deinit();
		return ret;
	}

	SDIO_TEST_INFO("ok\n");

	return 0;
}

static void sdio_test_exit(void)
{
	SDIO_TEST_INFO("\n");

	sdio_unregister_driver(&sdio_test_client);
	sdio_test_api_deinit();
	sdio_test_gpio_deinit();

	SDIO_TEST_INFO("ok\n");
}

module_init(sdio_test_init);
module_exit(sdio_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SDIO Test Driver");
MODULE_AUTHOR("Rocky Pan");
MODULE_ALIAS("platform:" DRV_NAME);
