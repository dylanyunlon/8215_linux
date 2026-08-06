#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include "musb_core.h"
#include "musb_debug.h"
#include "atc_musb.h"
#define MYDBG(fmt, args...) pr_warn("ATC_ICUSB [DBG], <%s(), %d> " fmt, __func__, __LINE__, ## args)

/* general */
#define BIT_WIDTH_1		1
#define MSK_WIDTH_1		0x1
#define VAL_MAX_WDITH_1	0x1
#define VAL_0_WIDTH_1		0x0
#define VAL_1_WIDTH_1		0x1
#define STRNG_0_WIDTH_1	"0"
#define STRNG_1_WIDTH_1	"1"

#define BIT_WIDTH_3		3
#define MSK_WIDTH_3		0x7
#define VAL_MAX_WDITH_3		0x7
#define VAL_0_WIDTH_3		0x0
#define VAL_1_WIDTH_3		0x1
#define VAL_2_WIDTH_3		0x2
#define VAL_3_WIDTH_3		0x3
#define VAL_4_WIDTH_3		0x4
#define VAL_5_WIDTH_3		0x5
#define VAL_6_WIDTH_3		0x6
#define VAL_7_WIDTH_3		0x7
#define STRNG_0_WIDTH_3	"000"
#define STRNG_1_WIDTH_3	"001"
#define STRNG_2_WIDTH_3	"010"
#define STRNG_3_WIDTH_3	"011"
#define STRNG_4_WIDTH_3	"100"
#define STRNG_5_WIDTH_3	"101"
#define STRNG_6_WIDTH_3	"110"
#define STRNG_7_WIDTH_3	"111"

/* specific */
#define FILE_USB_DRIVING_CAPABILITY "USB_DRIVING_CAPABILITY"

#define FILE_RG_USB20_TERM_VREF_SEL "RG_USB20_TERM_VREF_SEL"
#define MSK_RG_USB20_TERM_VREF_SEL MSK_WIDTH_3
#define SHFT_RG_USB20_TERM_VREF_SEL 0
#define OFFSET_RG_USB20_TERM_VREF_SEL 0x5

#define FILE_RG_USB20_HSTX_SRCTRL "RG_USB20_HSTX_SRCTRL"
#define MSK_RG_USB20_HSTX_SRCTRL MSK_WIDTH_3
#define SHFT_RG_USB20_HSTX_SRCTRL 4
#define OFFSET_RG_USB20_HSTX_SRCTRL 0x15

#define FILE_RG_USB20_VRT_VREF_SEL "RG_USB20_VRT_VREF_SEL"
#define MSK_RG_USB20_VRT_VREF_SEL MSK_WIDTH_3
#define SHFT_RG_USB20_VRT_VREF_SEL 4
#define OFFSET_RG_USB20_VRT_VREF_SEL 0x5

#define FILE_RG_USB20_INTR_EN "RG_USB20_INTR_EN"
#define MSK_RG_USB20_INTR_EN MSK_WIDTH_1
#define SHFT_RG_USB20_INTR_EN 5
#define OFFSET_RG_USB20_INTR_EN 0x0

/* static struct dentry *usb20_phy_debugfs_root; */


static u8 usb20_phy_debugfs_read_val(u8 offset, u8 shft, u8 msk, u8 width, char *str)
{
	u8 val;
	int i, temp;

	val = USBPHY_READ8(offset);
	MYDBG("offset:%x, val:%x, shft:%x, msk:%x\n", offset, val, shft, msk);
	val = val >> shft;
	MYDBG("offset:%x, val:%x, shft:%x, msk:%x\n", offset, val, shft, msk);
	val = val & msk;
	MYDBG("offset:%x, val:%x, shft:%x, msk:%x\n", offset, val, shft, msk);

	temp = val;
	str[width] = '\0';
	for (i = (width - 1); i >= 0; i--) {
		if (val % 2)
			str[i] = '1';
		else
			str[i] = '0';
		MYDBG("str[%d]:%c\n", i, str[i]);
		val /= 2;
	}
	MYDBG("str(%s)\n", str);
	return val;
}

static int usb_driving_capability_show(struct seq_file *s, void *unused)
{
	u8 val;
	char str[16];
	u8 combined_val, tmp_val;

	val = usb20_phy_debugfs_read_val(OFFSET_RG_USB20_TERM_VREF_SEL, SHFT_RG_USB20_TERM_VREF_SEL,
					 MSK_RG_USB20_TERM_VREF_SEL, BIT_WIDTH_3, str);
	if (!strncmp(str, STRNG_0_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_0_WIDTH_3);
		tmp_val = VAL_0_WIDTH_3;
	}
	if (!strncmp(str, STRNG_1_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_1_WIDTH_3);
		tmp_val = VAL_1_WIDTH_3;
	}
	if (!strncmp(str, STRNG_2_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_2_WIDTH_3);
		tmp_val = VAL_2_WIDTH_3;
	}
	if (!strncmp(str, STRNG_3_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_3_WIDTH_3);
		tmp_val = VAL_3_WIDTH_3;
	}
	if (!strncmp(str, STRNG_4_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_4_WIDTH_3);
		tmp_val = VAL_4_WIDTH_3;
	}
	if (!strncmp(str, STRNG_5_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_5_WIDTH_3);
		tmp_val = VAL_5_WIDTH_3;
	}
	if (!strncmp(str, STRNG_6_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_6_WIDTH_3);
		tmp_val = VAL_6_WIDTH_3;
	}
	if (!strncmp(str, STRNG_7_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_7_WIDTH_3);
		tmp_val = VAL_7_WIDTH_3;
	}

	combined_val = tmp_val;

	val = usb20_phy_debugfs_read_val(OFFSET_RG_USB20_VRT_VREF_SEL, SHFT_RG_USB20_VRT_VREF_SEL,
					 MSK_RG_USB20_VRT_VREF_SEL, BIT_WIDTH_3, str);
	if (!strncmp(str, STRNG_0_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_0_WIDTH_3);
		tmp_val = VAL_0_WIDTH_3;
	}
	if (!strncmp(str, STRNG_1_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_1_WIDTH_3);
		tmp_val = VAL_1_WIDTH_3;
	}
	if (!strncmp(str, STRNG_2_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_2_WIDTH_3);
		tmp_val = VAL_2_WIDTH_3;
	}
	if (!strncmp(str, STRNG_3_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_3_WIDTH_3);
		tmp_val = VAL_3_WIDTH_3;
	}
	if (!strncmp(str, STRNG_4_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_4_WIDTH_3);
		tmp_val = VAL_4_WIDTH_3;
	}
	if (!strncmp(str, STRNG_5_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_5_WIDTH_3);
		tmp_val = VAL_5_WIDTH_3;
	}
	if (!strncmp(str, STRNG_6_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_6_WIDTH_3);
		tmp_val = VAL_6_WIDTH_3;
	}
	if (!strncmp(str, STRNG_7_WIDTH_3, BIT_WIDTH_3)) {
		MYDBG("%s case\n", STRNG_7_WIDTH_3);
		tmp_val = VAL_7_WIDTH_3;
	}

	MYDBG("combined_val(%d), tmp_val(%d)\n", combined_val, tmp_val);
	if ((tmp_val == (combined_val - 1)) || (tmp_val == combined_val))
		combined_val += tmp_val;
	else
		combined_val = tmp_val * (VAL_MAX_WDITH_3 + 1) + combined_val;

	MYDBG("combined_val(%d), tmp_val(%d)\n", combined_val, tmp_val);

	seq_printf(s, "%d", combined_val);
	return 0;
}

static int rg_usb20_term_vref_sel_show(struct seq_file *s, void *unused)
{
	u8 val;
	char str[16];

	val =
	    usb20_phy_debugfs_read_val(OFFSET_RG_USB20_TERM_VREF_SEL, SHFT_RG_USB20_TERM_VREF_SEL,
				       MSK_RG_USB20_TERM_VREF_SEL, BIT_WIDTH_3, str);
	seq_printf(s, "%s", str);
	return 0;
}

static int rg_usb20_hstx_srctrl_show(struct seq_file *s, void *unused)
{
	u8 val;
	char str[16];

	val =
	    usb20_phy_debugfs_read_val(OFFSET_RG_USB20_HSTX_SRCTRL, SHFT_RG_USB20_HSTX_SRCTRL,
				       MSK_RG_USB20_HSTX_SRCTRL, BIT_WIDTH_3, str);
	seq_printf(s, "%s", str);
	return 0;
}

static int rg_usb20_vrt_vref_sel_show(struct seq_file *s, void *unused)
{
	u8 val;
	char str[16];

	val =
	    usb20_phy_debugfs_read_val(OFFSET_RG_USB20_VRT_VREF_SEL, SHFT_RG_USB20_VRT_VREF_SEL,
				       MSK_RG_USB20_VRT_VREF_SEL, BIT_WIDTH_3, str);
	seq_printf(s, "%s", str);
	return 0;
}

static int rg_usb20_intr_en_show(struct seq_file *s, void *unused)
{
	u8 val;
	char str[16];

	val =
	    usb20_phy_debugfs_read_val(OFFSET_RG_USB20_INTR_EN, SHFT_RG_USB20_INTR_EN,
				       MSK_RG_USB20_INTR_EN, BIT_WIDTH_1, str);
	seq_printf(s, "%s", str);
	return 0;
}

static int usb_driving_capability_open(struct inode *inode, struct file *file)
{
	return single_open(file, usb_driving_capability_show, inode->i_private);
}

static int rg_usb20_term_vref_sel_open(struct inode *inode, struct file *file)
{
	return single_open(file, rg_usb20_term_vref_sel_show, inode->i_private);
}

static int rg_usb20_hstx_srctrl_open(struct inode *inode, struct file *file)
{
	return single_open(file, rg_usb20_hstx_srctrl_show, inode->i_private);
}

static int rg_usb20_vrt_vref_sel_open(struct inode *inode, struct file *file)
{
	return single_open(file, rg_usb20_vrt_vref_sel_show, inode->i_private);
}

static int rg_usb20_intr_en_open(struct inode *inode, struct file *file)
{
	return single_open(file, rg_usb20_intr_en_show, inode->i_private);
}


static ssize_t usb_driving_capability_write(struct file *file,
					    const char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[18];
	u8 val, tmp_val;
	/* char str_rg_usb20_term_vref_sel[18], str_rg_usb20_vrt_vref_sel[18]; */

	memset(buf, 0x00, sizeof(buf));
	MYDBG("\n");
	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (kstrtol(buf, 10, (long *)&val) != 0) {
		MYDBG("kstrtol, err(%d)\n", kstrtol(buf, 10, (long *)&val));
		return count;
	}
	MYDBG("kstrtol, val(%d)\n", val);

	if (val > VAL_7_WIDTH_3 * 2) {
		MYDBG("wrong val set(%d), direct return\n", val);
		return count;
	}
	tmp_val = val;
	val /= 2;

	MYDBG("val(%d), tmp_val(%d)\n", val, tmp_val);

	return count;
}

static ssize_t rg_usb20_term_vref_sel_write(struct file *file,
					    const char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[18];

	memset(buf, 0x00, sizeof(buf));

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;
	return count;
}

static ssize_t rg_usb20_hstx_srctrl_write(struct file *file,
					  const char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[18];

	memset(buf, 0x00, sizeof(buf));

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;
	return count;
}

static ssize_t rg_usb20_vrt_vref_sel_write(struct file *file,
					   const char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[18];

	memset(buf, 0x00, sizeof(buf));

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;
	return count;
}

static ssize_t rg_usb20_intr_en_write(struct file *file,
				      const char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[18];

	memset(buf, 0x00, sizeof(buf));

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;
	return count;
}

static const struct file_operations usb_driving_capability_fops = {
	.open = usb_driving_capability_open,
	.write = usb_driving_capability_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations rg_usb20_term_vref_sel_fops = {
	.open = rg_usb20_term_vref_sel_open,
	.write = rg_usb20_term_vref_sel_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations rg_usb20_hstx_srctrl_fops = {
	.open = rg_usb20_hstx_srctrl_open,
	.write = rg_usb20_hstx_srctrl_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations rg_usb20_vrt_vref_sel_fops = {
	.open = rg_usb20_vrt_vref_sel_open,
	.write = rg_usb20_vrt_vref_sel_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations rg_usb20_intr_en_fops = {
	.open = rg_usb20_intr_en_open,
	.write = rg_usb20_intr_en_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


