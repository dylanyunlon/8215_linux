/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <linux/interrupt.h>

#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <mach/dma.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include "x_bim.h"
#include "x_ver.h"
#include "x_ckgen.h"
#include "base_regs.h"
#include "drv_dual.h"
#include "irqs_vector.h"
#include "oal.h"
#include <ac83xx_gpio_pinmux.h>
#include <ac83xx_pinmux_table.h>
#include <mach/pinmux.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <generated/atc_project.h>
#include <linux/semaphore.h>
#include <linux/poll.h>

#include "bootanidrv.h"
#include <media/atc/display.h>
#include <media/atc/drv_osd_if.h>
#include "pmx_hal.h"

#define BOOTANIDEVNAME "bootanidrv"

extern __u32 LCD_GetScreenWidth(void);
extern __u32 LCD_GetScreenHeight(void);

static struct device_node * node = NULL;

#define MESSAGE_ANIMATION_REQUEST_VDEC 0x5
#define MESSAGE_ANIMATION_VDEC_FREE 0x6
#define MESSAGE_VDEC_HW_READY 0x7

static struct semaphore vdecHwStatusSemphore;
static struct semaphore vdecLockSemphore;

static wait_queue_head_t ani_wait_queue;
static enum VDEC_STATUS vdecStatus = VDEC_STATUS_FREE;
static UINT32 vdecRequestStatus = 0x00;
static UINT32 vdecStatusChange = 0x00;

extern int free_memblock_runtime(phys_addr_t phy_addr,phys_addr_t size);

struct RES_MEM {
u32 start;
u32 size;
};

static struct RES_MEM  resmem;
static struct RES_MEM  resmemusebyvba;
int g_lcm_width;
int g_lcm_height;
UINT32 g_dul_module_id = -1;
UINT32 g_dul_user_buf[4] = {0};
UINT32 g_dul_message_flag = 0;
struct mutex g_dul_mutex;


extern void ac83xx_mask_ack_bim_irq(uint32_t irq);

#include <linux/wait.h>
DECLARE_WAIT_QUEUE_HEAD(bootaniqueue);
bool fgrelease = false;
extern bool RequestHW(u32 *pIdleHwID);
extern void ReleaseHW(u32 u4HwId);

static irqreturn_t animation_isr_handler(int irq,void* dev_id)
{
	UINT32 g_u4Param1, g_u4Param2, g_u4Param3;
	UINT32 u4ModuleID, u4MessageID;
	HWGetMessage(MODULE_BOOTANIMATION, &u4ModuleID, &g_u4Param1, &g_u4Param2, &g_u4Param3);
	printk("[I][BOOTANIDRV] call animation_isr_handler resmem.start : resmem.start 0x%08x, size: 0x%08x\n", resmem.start, resmem.size);
	g_dul_user_buf[0] = u4ModuleID;
	g_dul_user_buf[1] = g_u4Param1;
	g_dul_user_buf[2] = g_u4Param2;
	g_dul_user_buf[3] = g_u4Param3;
	g_dul_message_flag = 1;
	if((resmem.start != 0) && (resmem.size !=0)) {
		#ifdef CONFIG_ATC_OS_linux
		//free_memblock_runtime((phys_addr_t)resmem.start,(phys_addr_t)resmem.size);
		fgrelease = true;
		wake_up_interruptible(&bootaniqueue);
		#endif
	}

	if (g_u4Param1 == MESSAGE_ANIMATION_REQUEST_VDEC || g_u4Param1 == MESSAGE_ANIMATION_VDEC_FREE) {
		vdecRequestStatus = g_u4Param1; 	   
		printk("[I][BOOTANIDRV] g_u4Param1 %d 1\n", g_u4Param1);
		up(&vdecHwStatusSemphore);
	}

	printk("[I][BOOTANIDRV] animation_isr_handler g_u4Param1 %d IRQ_HANDLED\n", g_u4Param1);
	return IRQ_HANDLED;
}
static int animation_mmap(struct file *fp, struct vm_area_struct *vma)
{
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			vma->vm_start,
			vma->vm_pgoff,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static int animation_open(struct inode* inode, struct file *file)
{
	printk("[I][BOOTANIDRV]animation open\n");
	return 0;
}

static ssize_t animation_read(struct file *file, const char __user *in, size_t size, loff_t *off)
{
	return 0;
}

static ssize_t animation_write(struct file *file, const char __user *in, size_t size, loff_t *off)
{
	return 0;
}

static int animation_ioctl(struct file *filep,unsigned int cmd, unsigned long arg)
{
	int err = 0;
	BOOL ret = 0;
	UINT32 module_id=0;
	UINT32 user_buf[4] = {0};
	int i = 0;
	u32 bootLogoPhyAdr;
	LOGO_BUF_INFO_T rLogoBufInfo;
	u32 rgn;
	u32 flag = 0;

	switch(cmd)
	{
		case BOOTANI_IOC_GETVDECSTATUS:
			{
				down_interruptible(&vdecLockSemphore);
				vdecStatusChange = 0;
				copy_to_user((void *)arg, &vdecStatus, sizeof(UINT32));
				up(&vdecLockSemphore);
				ret = TRUE;
			}
		break;
		case BOOTANI_IOC_GETMESSAGE:
			{
				copy_from_user((void *)module_id, (void *)arg, 1 * sizeof(UINT32));
				printk("[I][BOOTANIDRV]GetMsg module_id=%d\n", module_id);
				mutex_lock(&g_dul_mutex);
				g_dul_module_id = module_id;
				while (!g_dul_message_flag) {
					flag++;
					msleep(100);
					if (flag >= 50)
						break;
				}
				memcpy(user_buf, g_dul_user_buf, 4 * sizeof(UINT32));
				mutex_unlock(&g_dul_mutex);
				printk("[I][BOOTANIDRV]GetMsg &user_buf[0]=%d,&user_buf[1]=%d,&user_buf[2]=%d,&user_buf[3]=%d\n",user_buf[0],user_buf[1],user_buf[2],user_buf[3]);
				copy_to_user((void *)arg, user_buf, 4 * sizeof(UINT32));
			}
		break;
		case BOOTANI_IOC_SENDAWTKMESSAGE:
			{
				HWSendMessage(MSG_COMBINE(MODULE_ARM2SYSTEMSERVICE , 4), 0, 0, 0);
				printk("[I][BOOTANIDRV]SendMsg AWTK start message\n");
				ret = TRUE;
			}
		break;
		case BOOTANI_IOC_STARTAWTKDISPLAY:
			{
				vPmxHalMixPlane(0, 4);
 				printk("[I][BOOTANIDRV] BOOTANI_IOC_STARTAWTKDISPLAY\n");
				ret = TRUE;
			}
		break;
		case BOOTANI_IOC_STOPAWTKDISPLAY:
			{
				vPmxHalNotMixPlane(0, 4);
 				printk("[I][BOOTANIDRV] BOOTANI_IOC_STOPAWTKDISPLAY\n");
				ret = TRUE;
			}
		break;
		case BOOTANI_IOC_SENDMESSAGE:
			{
				copy_from_user(user_buf, (void *)arg, 4 * sizeof(UINT32));
				ret = HWSendMessage(user_buf[0], user_buf[1], user_buf[2], user_buf[3]);
				printk("[I][BOOTANIDRV]SendMsg &user_buf[0]=%d,&user_buf[1]=%d,&user_buf[2]=%d,&user_buf[3]=%d\n",user_buf[0],user_buf[1],user_buf[2],user_buf[3]);
			}
		break;
		case BOOTANI_IOC_GETOSDPHY:
			{
				/*Get OSD Phy Addr*/
				rgn = GetPlaneRgn(BOOT_ANIMATION_OSD_PLANE);
				OSD_RGN_Get(rgn, OSD_RGN_BMP_ADDR, &(rLogoBufInfo.u4BufPhyAdr));
				OSD_RGN_Get(rgn,OSD_RGN_BMP_W,&(rLogoBufInfo.u4Width));
				OSD_RGN_Get(rgn,OSD_RGN_BMP_H,&(rLogoBufInfo.u4Height));
				rLogoBufInfo.u4BufSz = rLogoBufInfo.u4Width * rLogoBufInfo.u4Height * 4;
				if (copy_to_user((void *)arg, &rLogoBufInfo, sizeof(LOGO_BUF_INFO_T))) {
					ret  = -EINVAL;
					printk("[E][BOOTANIDRV]copy_to_user return %d iocode %d\n", ret, cmd);
				}
				printk("[I][BOOTANIDRV]GetAddr Width %d,Height %d,PhyAddr %x,Size %d\n",rLogoBufInfo.u4Width,rLogoBufInfo.u4Height,rLogoBufInfo.u4BufPhyAdr,rLogoBufInfo.u4BufSz);
				ret = TRUE;
			}
		break;
        case BOOTANI_IOC_GETVBAPHY:
			{
				copy_from_user((void *)&rLogoBufInfo, (void *)arg, 1 * sizeof(rLogoBufInfo));
				if (resmemusebyvba.start + rLogoBufInfo.u4BufSz > resmem.start + resmem.size)
				{
					resmemusebyvba.start = resmem.start;
				}
				rLogoBufInfo.u4BufPhyAdr = resmemusebyvba.start;
				resmemusebyvba.start += rLogoBufInfo.u4BufSz;
				printk("[I][BOOTANIDRV]GetAddr Width %d,Height %d,PhyAddr %x,Size %d\n",rLogoBufInfo.u4Width,rLogoBufInfo.u4Height,rLogoBufInfo.u4BufPhyAdr,rLogoBufInfo.u4BufSz);
				copy_to_user((void *)arg, (void *)&rLogoBufInfo, sizeof(rLogoBufInfo));
				ret = TRUE;
			}
		break;
		case BOOTANI_IOC_RLSREVMEM:
			{
				printk("[W][BOOTANIDRV] release ioctl do nothing \n");
				return TRUE;
				
				if((resmem.start == -1) ||(resmem.size == -1 )) {
					printk("[W][BOOTANIDRV]Animation_reserved Already release \n");
					return TRUE;
				}
				
			//	free_memblock_runtime((phys_addr_t)resmem.start,(phys_addr_t)resmem.size);
				printk("[I][BOOTANIDRV]Animation_reserved release start is %x,size is %x\n",resmem.start,resmem.size);
				resmem.start = -1;
				resmem.size  = -1;
				ret = TRUE;
			}
		break;
		default:
			{
				return -EINVAL;
			}
		break;
	}
	if(TRUE == ret)
		return 0;
	else
		return -1;
}

static unsigned int animation_poll(struct file *filp, struct poll_table_struct *poll_table)
{
	uint32_t mask = 0;
	printk("[I][BOOTANIDRV] animation_poll\n");

	poll_wait(filp, &ani_wait_queue, poll_table);
	down_interruptible(&vdecLockSemphore);
	if (vdecStatusChange != 0) {
		mask |= POLLIN;
	}
	up(&vdecLockSemphore);

	printk("[I][BOOTANIDRV] animation_poll end\n");

	return mask;
}

struct file_operations animation_fops = {
	.owner = THIS_MODULE,
	.open = animation_open,
	.read = animation_read,
	.write = animation_write,
	.mmap = animation_mmap,
	.unlocked_ioctl = animation_ioctl,
	.poll = animation_poll,

};

static struct miscdevice animation_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = BOOTANIDEVNAME,
	.fops = & animation_fops,
};

#include <linux/kthread.h>

static struct task_struct *ts1;

static int dualarm_thread(void *arg)
{
    unsigned int ret=0;
	UINT32 awtkMemorybuffer = 4 * 1024 * 1024;
	UINT32 releaseSize, releaseBase, useSize, useBase;
	UINT32 vdechwId = 0;

	ret = wait_event_interruptible(bootaniqueue, (fgrelease == true));
	useSize = g_lcm_width * g_lcm_height * 2 * 3 + awtkMemorybuffer;
	releaseBase = resmem.start + useSize;
	releaseSize = resmem.size - useSize;
//	free_memblock_runtime((phys_addr_t)releaseBase, (phys_addr_t)releaseSize);
	printk("[I][BOOTANIDRV] release base 0x%08x, size: 0x%08x, useSize: 0x%x\n", releaseBase, releaseSize, useSize);

	while (1) {
		int sem_taken = -1;
		printk("[I][BOOTANIDRV] wait vdecHwStatusSemphore \n");

		sem_taken = down_interruptible(&vdecHwStatusSemphore);
		if (0 != sem_taken) {
			 printk("[I][BOOTANIDRV] down_interruptible(&vdecHwStatusSemphore) fail \n");
			continue;
		}
		printk("[I][BOOTANIDRV] wait vdecHwStatusSemphore end\n");

		down_interruptible(&vdecLockSemphore);
		if (vdecRequestStatus == MESSAGE_ANIMATION_REQUEST_VDEC) {
			vdecStatus = VDEC_STATUS_USED;
		} else {
			vdecStatus = VDEC_STATUS_FREE;
		}
		vdecStatusChange = 1;
		wake_up_interruptible(&ani_wait_queue);
		up(&vdecLockSemphore);

		if (vdecRequestStatus == MESSAGE_ANIMATION_REQUEST_VDEC) {
			do {
				if (RequestHW(&vdechwId)) {
					printk("[I][BOOTANIDRV] request vdec hw sucess vdechwId %d\n", vdechwId);
					HWSendMessage(MODULE_BOOTANIMATION<<24, MESSAGE_ANIMATION_VDEC_FREE,0,0);
					break;
				} else {
					printk("[I][BOOTANIDRV] wait vdec hw release\n");
					msleep(20);
				}
			} while (1);
		} else if (vdecRequestStatus == MESSAGE_ANIMATION_VDEC_FREE) {
			printk("[I][BOOTANIDRV] release vdec hw sucess vdechwId %d\n", vdechwId);
			ReleaseHW(vdechwId);
		}


	}

    return 0;
}

static int animation_probe(struct platform_device * pdev)
{
	int ret = 0;
	u32 rgn;

    printk("[I][BOOTANIDRV]Animation probe\n");
	ret = misc_register(&animation_misc_dev);
	node =of_find_compatible_node(NULL,NULL,"atc-animation-reserved");

	if(node){
		if(of_property_read_u32_array(node,"reg",(u32 *)&resmem,2)){
			printk("[E][BOOTANIDRV]Animation_probe failed to get size and start\n");
		}
		printk("[I][BOOTANIDRV]Animation_reserved memory start is %x,size is %x\n",resmem.start,resmem.size);
		if (resmem.size > 0x500000) {
			resmemusebyvba.size = resmem.size / 2;
			resmemusebyvba.start = resmem.start + resmem.size - 0x500000;
		} else {
			resmemusebyvba.start = resmem.start;
		}
	}
	else {
		printk("[E][BOOTANIDRV]Animation_probe failed to get animation node\n");
	}

	ts1 = kthread_create(dualarm_thread, NULL, "dualarmt");
    wake_up_process(ts1);

	g_lcm_width = LCD_GetScreenWidth();
	g_lcm_height = LCD_GetScreenHeight();
	HWSendMessage(MODULE_BOOTANIMATION<<24,MESSAGE_VDEC_HW_READY,0,0);

	printk("[I][BOOTANIDRV]GetAddr Width %d,Height %d\n",g_lcm_width, g_lcm_height);
    return 0;
}
static int animation_remove(struct platform_device *dev)
{

    printk("[I][BOOTANIDRV]Animation remove\n");
	misc_deregister(&animation_misc_dev);

    return 0;
}

static struct platform_driver animaiton_driver = {

    .driver = {
        .name = BOOTANIDEVNAME,
        .owner = THIS_MODULE,
    },
	.probe = animation_probe,
	.remove = animation_remove,
};


static struct platform_device animation_device = {

    .name = BOOTANIDEVNAME,
    .id = -1,

};
#define ANIMATION_MODULE_NAME "BootAni"
#define ANIMATION_VER_MAIN	1
#define ANIMATION_VER_MINOR	00
#define ANIMATION_VER_REV	00



static int __init animation_init(void)
{
    int ret = 0;
    int res;
	MOD_VERSION_INFO(ANIMATION_MODULE_NAME,ANIMATION_VER_MAIN,ANIMATION_VER_MINOR,ANIMATION_VER_REV);
	mutex_init(&g_dul_mutex);

    res = os_device_register(&animation_device);
    res = os_driver_register(& animaiton_driver);
    init_waitqueue_head(&ani_wait_queue);
    sema_init(&vdecHwStatusSemphore, 0);
    sema_init(&vdecLockSemphore, 1);

	request_dualarm_irq(2,animation_isr_handler,0,"ANIMATION",NULL);

    if(0 == ret)
    {
        printk("[I][BOOTANIDRV]Animation_init success\r\n");
    }
	else
        printk("[I][BOOTANIDRV]Animation_init failed\r\n");

    return ret;
}

static void __exit animation_exit(void)
{
    free_irq(VECTOR_TOCORISC, NULL);
}




module_init(animation_init);
module_exit(animation_exit);



MODULE_AUTHOR("ATC");
MODULE_DESCRIPTION("bootanimaiton Driver");
MODULE_LICENSE("GPL");



