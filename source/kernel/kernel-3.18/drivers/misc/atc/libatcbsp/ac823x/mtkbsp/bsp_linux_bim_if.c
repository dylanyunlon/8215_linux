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

//============================================================================
// Include files
//============================================================================
#include <linux/module.h>      //Must be included header file

#include "mach/mt3365_irqs_vector.h"
#include "section.h"
//#include "x_bim.h"
#include "x_os.h"
//#include "x_hal_1176.h"
#include "drv_config.h"
//#include "x_pdwnc.h"
//#include "mach/base_regs.h"
//#include <linux/spinlock_types.h>
#include <linux/of.h>
#include <linux/of_address.h>

#define HW_SEMA_NUM    32
#define REG_DEGCK_CFG 0x740
#define REG_EXTINT0_CFG	0x744
#define EINT_POST_DIV_EN               (1 << 8)
#define EINT_PRE_DIV_EN                    (1 << 12)
#define EINT_POST_DIV_MASK             0xFF
#define EINT_TYPE_MASK                (0x7 << 10)
#define EINT_EN                       (1 << 13)
#define REG_EXTINT_CFG(a)              (REG_EXTINT0_CFG + (a) * 4)
//#define BIM_REG_BASE 			0x10000000
static void __iomem *bim_reg_base;
#define __io(a) ((void __iomem *)(a))

void get_bim_reg_base(void)
{
	static struct device_node * node = NULL;
        node =of_find_compatible_node(NULL,NULL,"mediatek,mt33xx-bim");
        if(node){
                bim_reg_base = of_iomap(node, 0);
                pr_info("[Libatcbsp] [%s][%d] bim_reg_base start is 0x%p\n", __func__, __LINE__, bim_reg_base);
        }
        else {
                pr_err("[Libatcbsp] [bsp_linux_bim_if.c][%s][%d] failed to get bim node\n", __func__, __LINE__);
        }
        if(bim_reg_base == 0){
                pr_err("[Libatcbsp] [bsp_linux_bim_if.c][%s][%d] libatcbsp get bim_reg_base failed\n", __func__, __LINE__);
        }
}

u32 BIM_READ32(u64 regaddr)
{
	if(bim_reg_base == 0)
		get_bim_reg_base();
	return __raw_readl(__io(bim_reg_base + regaddr));
}

void BIM_WRITE32(u64 regaddr, u32 regval32)
{
	if(bim_reg_base == 0)
		get_bim_reg_base();
        __raw_writel(regval32, __io(bim_reg_base + regaddr));
}

//UINT32 u4HWSemWaitCount;
static DEFINE_SPINLOCK(ac823x_bim_lock);

//============================================================================
// Public functions
//============================================================================
#if 0
void BIM_Workaround(UINT32 u4Vector)
{
    //2008.10.30 Ted Hu
    //For IF-con work around
    //If External INT EN reset, the INT issue again.
    //So change External INT polarity to avoid it.
    unsigned long flags;
	spin_lock_irqsave(&ac823x_bim_lock, flags);
    if (u4Vector == VECTOR_EXT)
    {
        //20081118_1
        //BIM_WRITE32(0xA8, (BIM_READ32(0xA8) & ~0x00000002));

        //20081118_2
        if ((BIM_READ32(0xA8) & 0x00000002) != 0)
        {
            BIM_WRITE32(0xA8, (BIM_READ32(0xA8) & ~0x00000002));
        }
        else
        {
            BIM_WRITE32(0xA8, (BIM_READ32(0xA8) | 0x00000002));
        }
    }
	spin_unlock_irqrestore(&ac823x_bim_lock, flags);
}
EXPORT_SYMBOL(BIM_Workaround);

// watch dog timer mode 2 - ir reset
#define RESET_IR_KEY_INIT_PTN	0xFFFFFFFF
UINT32 _u4ResetKeyIrM = RESET_IR_KEY_INIT_PTN;
UINT32 _u4ResetKeyIrL = RESET_IR_KEY_INIT_PTN;
UINT32 _u4SysHaltResetMode = RESET_MODE_NONE;

void BIM_SetSysHaltResetMode(UINT32 u4Mode) /// 0: no reset, while loop. 1: Automatically Reset. 2: IR Power Key Reset
{
    _u4SysHaltResetMode = u4Mode;
}
EXPORT_SYMBOL(BIM_SetSysHaltResetMode);

UINT32 BIM_GetSysHaltResetMode(void)
{
    return _u4SysHaltResetMode;
}
EXPORT_SYMBOL(BIM_GetSysHaltResetMode);

void BIM_RegResetIrKey(UINT32 u4ResetIrM, UINT32 u4ResetIrL)
{
  _u4ResetKeyIrM = u4ResetIrM;
  _u4ResetKeyIrL = u4ResetIrL;
}
EXPORT_SYMBOL(BIM_RegResetIrKey);

void BIM_GetResetIrKey(UINT32* pu4ResetIrM, UINT32* pu4ResetIrL)
{
  *pu4ResetIrM = _u4ResetKeyIrM;
  *pu4ResetIrL = _u4ResetKeyIrL;
}
EXPORT_SYMBOL(BIM_GetResetIrKey);

BOOL BIM_AddrSwap(
  UINT32 u4Region,    ///< [IN] address swap region: 0~3
  UINT32 u4BeginAddr, ///< [IN] address swap begin addrss
  UINT32 u4EndAddr,   ///< [IN] address swap end address
  UINT32 u4SwapMode   ///< [IN] address swap mode: 0/1/2, 0: Off
)
{
  UINT32 u4RegValue;
  unsigned long flags;

  if (3 < u4Region)
    return FALSE;
  if ((0 != u4SwapMode) && ((0 == u4BeginAddr) || (0 == u4EndAddr)))
    return FALSE;

  spin_lock_irqsave(&ac823x_bim_lock, flags);
  u4RegValue = BIM_READ32(REG_RW_SWAP_CTRL);

  if(0 != u4SwapMode)
  {
    switch(u4Region)
    {
      case 0:
        BIM_WRITE32(REG_RW_SWAP_RG0_BGN,u4BeginAddr);
        BIM_WRITE32(REG_RW_SWAP_RG0_END,u4EndAddr);
        u4RegValue |= (SWP_RG0_WREN | SWP_RG0_RDEN | ((u4SwapMode & 0x7) << 8));
        break;
      case 1:
        BIM_WRITE32(REG_RW_SWAP_RG1_BGN,u4BeginAddr);
        BIM_WRITE32(REG_RW_SWAP_RG1_END,u4EndAddr);
        u4RegValue |= (SWP_RG1_WREN | SWP_RG1_RDEN | ((u4SwapMode & 0x7) << 12));
        break;
      case 2:
        BIM_WRITE32(REG_RW_SWAP_RG2_BGN,u4BeginAddr);
        BIM_WRITE32(REG_RW_SWAP_RG2_END,u4EndAddr);
        u4RegValue |= (SWP_RG2_WREN | SWP_RG2_RDEN | ((u4SwapMode & 0x7) << 24));
        break;
      case 3:
        BIM_WRITE32(REG_RW_SWAP_RG3_BGN,u4BeginAddr);
        BIM_WRITE32(REG_RW_SWAP_RG3_END,u4EndAddr);
        u4RegValue |= (SWP_RG3_WREN | SWP_RG3_RDEN | ((u4SwapMode & 0x7) << 28));
        break;
    }
  }
  else
  {
    switch(u4Region)
    {
      case 0:
        u4RegValue &= ~(SWP_RG0_RDEN | SWP_RG0_WREN);
        break;
      case 1:
        u4RegValue &= ~(SWP_RG1_RDEN | SWP_RG1_WREN);
        break;
      case 2:
        u4RegValue &= ~(SWP_RG2_RDEN | SWP_RG2_WREN);
        break;
      case 3:
        u4RegValue &= ~(SWP_RG3_RDEN | SWP_RG3_WREN);
        break;
      }
  }
  BIM_WRITE32(REG_RW_SWAP_CTRL, u4RegValue);
  spin_unlock_irqrestore(&ac823x_bim_lock, flags);
  return TRUE;
}
EXPORT_SYMBOL(BIM_AddrSwap);

extern void ac823x_mask_bim_irq(unsigned int irq);
BOOL BIM_DisableIrq(UINT32 u4Vector)
{
    ac823x_mask_bim_irq(u4Vector);
    return TRUE;
}
EXPORT_SYMBOL(BIM_DisableIrq);

extern void ac823x_unmask_bim_irq(unsigned int irq);
BOOL BIM_EnableIrq(UINT32 u4Vector)
{
    ac823x_unmask_bim_irq(u4Vector);
    return TRUE;
}
EXPORT_SYMBOL(BIM_EnableIrq);
#endif
extern void mt33xx_mask_ack_bim_irq(unsigned int virq);
extern BOOL BIM_ClearIrq(UINT32 u4Vector)
{
    mt33xx_mask_ack_bim_irq(u4Vector);
	return TRUE;
}
EXPORT_SYMBOL(BIM_ClearIrq);

//debunce time  = 0 - 256 (10us ~ 2.56ms)
VOID BIM_SetEInt(UINT32 EIntNumber, UINT32 type, UINT32 debunceTime)
{
	UINT32 regvalue = 0;

	regvalue = BIM_READ32(REG_EXTINT_CFG(EIntNumber));

	if (debunceTime != 0)
	{
		//set debunce time
		//prediv 
		//27M/0xff = 9.5us 
		BIM_WRITE32(REG_DEGCK_CFG, EINT_PRE_DIV_EN | 0xFF );

		regvalue = regvalue | EINT_POST_DIV_EN;
		regvalue = regvalue & (~EINT_POST_DIV_MASK );
		regvalue = regvalue | debunceTime;
	}

	//set eint type
	regvalue = regvalue & (~EINT_TYPE_MASK );
	regvalue = regvalue | type;
	
  	BIM_WRITE32(REG_EXTINT_CFG(EIntNumber), regvalue);
	
}
EXPORT_SYMBOL(BIM_SetEInt);

VOID BIM_EnableEInt(UINT32 EIntNumber)
{
  BIM_WRITE32(REG_EXTINT_CFG(EIntNumber), EINT_EN |BIM_READ32(REG_EXTINT_CFG(EIntNumber)));
}
EXPORT_SYMBOL(BIM_EnableEInt);

VOID BIM_DisableEInt(UINT32 EIntNumber)
{
  BIM_WRITE32(REG_EXTINT_CFG(EIntNumber), (~EINT_EN) & BIM_READ32(REG_EXTINT_CFG(EIntNumber)));
}
EXPORT_SYMBOL(BIM_DisableEInt);
