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

#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>          //For file_operations
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include "x_major.h"
#include "x_module.h"
#include "x_printf.h"
#include "x_debug.h"
#include "x_bsp_linux.h"

#include "x_hal_ic.h"
#include "drv_config.h"
#include "chip_ver.h"

#include "x_dram_size.h"
#include <linux/interrupt.h>
#include <linux/module.h>     
#include "x_os.h"
#include "x_ver.h"

#include "x_bim.h"
#include "x_bim_83xx.h"
#include "x_assert.h"
#include "mach/base_regs.h"
#include "mach/irqs_vector.h"

extern BSP_GET_FUNCTION_ID	_gBspGetFuncID;
extern UINT32 u4_get_bus_clock(void);
extern int  ac83xx_bl_ver_proc_init(void);

//==================================================
// variables
//==================================================
static UINT32 _u4SysClock;

/* 
 * private functions
 */

/*----------------------------------------------------------------------------
 * _unloadFeedRandomSeed() un-load timer2 IRQ to feed dev/random
 *---------------------------------------------------------------------------*/
void _unloadFeedRandomSeed(void)
{
    free_irq(VECTOR_T2, NULL);
        //Disable Timer
    BIM_WRITE32(REG_RW_TIMER_CTRL, BIM_READ32(REG_RW_TIMER_CTRL)&(~TMR2_CNTDWN_EN));
}


/*----------------------------------------------------------------------------
 * vT2Isr() Timer2 ISR
 *---------------------------------------------------------------------------*/
extern BOOL BIM_ClearIrq(UINT32 u4Vector);
irqreturn_t T2Isr(int irq, void *dev_id)
{
    BIM_ClearIrq(irq);
	
    //Disable Timer
    BIM_WRITE32(REG_RW_TIMER_CTRL, BIM_READ32(REG_RW_TIMER_CTRL)&(~TMR2_CNTDWN_EN));

    //Setup Timer interrupt interval
    BIM_WRITE32(REG_T2LMT, (_u4SysClock/20) + (BIM_READ32(REG_RW_T64b_LO_0)%(_u4SysClock/20)));   // 50ms~100ms

    //Enable Timer
    BIM_WRITE32(REG_RW_TIMER_CTRL, BIM_READ32(REG_RW_TIMER_CTRL)|(TMR2_CNTDWN_EN));

    return IRQ_HANDLED;	
}

/*----------------------------------------------------------------------------
 * _FeedRandomSeed() Use timer2 IRQ to feed dev/random
 *---------------------------------------------------------------------------*/
void _FeedRandomSeed(void)
{
    _u4SysClock = u4_get_bus_clock();

    if(_u4SysClock == 0)
    {
      //A patch to start default system without PLL information
      _u4SysClock = 27000000;
    }    

    T2Isr(VECTOR_T2, NULL);

    if (request_irq(VECTOR_T2, T2Isr, IRQF_SAMPLE_RANDOM, "T2", NULL) != OSR_OK)
    {
		//ASSERT(FALSE);
		BUG();
    }	
}

static void _SystemDRAMFineTune(void)
{
	return;
}

/* 
 * linux kernel module file operation functions implement 
 */
int k_bsp_open(struct inode *inode, struct file *filp) 
{
  return 0;
}

static long k_bsp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  
  switch(cmd)
  {
	  case IOCTL_BSP_GET_IC_FUNCTION:
	  {
	  		BSP_GET_FUNCTION_ID	_BspFuncID;
	  			  		
	  		if (copy_from_user(&_BspFuncID, (void __user *)arg, sizeof(_BspFuncID)))
  			{
  				return -ERR_BSP_GET_IC_FUNCTION_IN;
    		}
        
    		if(_gBspGetFuncID.CallBackfunc == NULL)
	  		{
	  			return -ERR_BSP_GET_IC_FUNCTION_PNT_NULL;
	  		}
	  		else
	  		{
	  			_BspFuncID.ret_val = _gBspGetFuncID.CallBackfunc(_BspFuncID.FuncID);
		    }
		    
    		if (copy_to_user((void __user *)arg, &_BspFuncID, sizeof(_BspFuncID)))
    		{
	  			return -ERR_BSP_GET_IC_FUNCTION_OUT;
    		}
	  }
	  break;
	  case IOCTL_BSP_GET_IC_VERSION:
	  {
	  		char	*buf = NULL;
	  		
	  		buf = kmalloc(VERSION_BUF, GFP_KERNEL);
	  			  		
	  		if (copy_from_user(buf, (void __user *)arg, VERSION_BUF))
  			{
  				kfree(buf);
  				return -ERR_BSP_GET_IC_VERSION_IN;
    		}
        if(_gBspGetFuncID.GetIcVerFunc == NULL)
	  		{
	  		    kfree(buf);
	  			return -ERR_BSP_GET_IC_VERSION_PNT_NULL;
	  		}
	  		else
	  		{
	  			buf = _gBspGetFuncID.GetIcVerFunc();	  			
		    }
		    
    		if (copy_to_user((void __user *)arg, buf, VERSION_BUF))
    		{
    		    kfree(buf);
	  			return -ERR_BSP_GET_IC_VERSION_OUT;
    		}
            kfree(buf);
	  }
	  break;
    default:
      return -1;
  }

  return 0;
}

static int k_bsp_release(struct inode *inode, struct file *filp)
{
  return 0;
}

static const struct file_operations k_bsp_fops = 
{
  .owner   = THIS_MODULE,
  .open    = k_bsp_open,
  .unlocked_ioctl   = k_bsp_ioctl,    
  .release = k_bsp_release,
};

static struct cdev bsp_cdev;

/* 
 * linux kernel module init & uninit functions
 */
//extern DWORD I2C_Init(LPCTSTR szContext, LPCVOID pBusContext);
//extern BOOL I2C_Deinit(DWORD context);
extern int __init mt_sys_init(void);
extern void __exit mt_sys_exit(void);

#define BSP_VER_MAIN	1
#define BSP_VER_MINOR	00
#define BSP_VER_REV	00

static int __init bsp_init(void)
{
    int ret;

    MOD_VERSION_INFO("atcbsp",BSP_VER_MAIN,BSP_VER_MINOR,BSP_VER_REV);

    cdev_init(&bsp_cdev, &k_bsp_fops);
    bsp_cdev.owner = THIS_MODULE;
    ret = cdev_add(&bsp_cdev, MKDEV(BSP_MAJOR, 0), 1);
    
    
    if(ret)
    {
        pr_err("[Libatcbsp] [bsp_mod.c][%s][%d] Unable to register \"bsp\" misc device\n", __func__, __LINE__);
        goto fail;	  
    }
    mt_sys_init();
    _SystemDRAMFineTune();
	//I2C_Init(NULL, NULL);
	
    return ret;
  
fail:
    return ret;
}


static void __exit bsp_exit(void)
{

    //_unloadFeedRandomSeed();
    //I2C_Deinit(0);
    mt_sys_exit();
    //unregister_chrdev(DEV_MJR_K_BSP, "bsp");
    cdev_del(&bsp_cdev);
	
    //Print a message
    pr_info("[Libatcbsp] [%s][%d] k_bsp module un-init success\n", __func__, __LINE__);
}


module_init(bsp_init);
module_exit(bsp_exit);

MODULE_AUTHOR("ATC");
MODULE_DESCRIPTION("ac83xx sys driver");
MODULE_LICENSE("GPL");
