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

#include "ac823x_ir_regs.h"
#include "ac823x_ir_drv.h"
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/irq.h>
//#include <mach/ac83xx_system.h>//cgx 823x

#include "../../misc/atc/inc/x_ioopt.h"//cgx


static PFN_IRRXCB_T _pfnIrRxCbFunc = NULL;
static unsigned int _u4Info;
static unsigned int _au4IrRxData[MAX_IRRX_DATA];
static bool _fgIREnable = true;

int IRHW_RxInit(int i4Config, int i4SaPeriod, int i4Threshold, void *dev_id);
int _IRHW_TxSetIsr(bool fgSet);



static void _IRHW_RxClear(void)
{
   IR_WRITE32(IRRX_IRCLR, (IR_READ32(IRRX_IRCLR) | IRCLR));
   IR_WRITE32(IRRX_INTCLR, (IR_READ32(IRRX_INTCLR) | IR_INTCLR));
}

static irqreturn_t _IRHW_RxIsr(int irq, void *dev_id)
{

    /* read IRRX IRQ data. */
    _u4Info = IR_READ32(IRRX_COUNT_HIGH_REG);
    _au4IrRxData[0] = IR_READ32(IRRX_COUNT_MID_REG);
    _au4IrRxData[1] = IR_READ32(IRRX_COUNT_LOW_REG);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] RxIsr Info:0x%08x data mid: 0x%08x  data low: 0x%08x \r\n"), _u4Info, _au4IrRxData[1],
        _au4IrRxData[0]));

    if ((_fgIREnable == true) && (_u4Info != 0) && (_pfnIrRxCbFunc != NULL))
    {
        _pfnIrRxCbFunc(_u4Info, (unsigned char *) _au4IrRxData);
    }

    /* clear IR busy bit. */
    _IRHW_RxClear();

    return IRQ_HANDLED;
}


/**
 * @FUNCTION: fn static int _IRHW_RxSetIsr(bool fgSet)
 * @Description: set or clear ISR of IR
 *
 * @ARG   fgSet  true: set ISR; false: clear ISR
 * @RETURN     IR_SUCC             successfully
 *             IR_FAIL               the x_reg_isr failed.
 */
 static int _IRHW_RxSetIsr(bool fgSet, void *dev_id)
{

     if (fgSet)
     {
         /* enable irq. */ 
         //if (request_pdwnc_irq(PDWNC_INTR_IR, _IRHW_RxIsr, 0, "ac823x_IR_irq", dev_id)) //cgx 823x
        {
             printk("cannot get IR interrupt\n");
             return IR_FAIL;
        }

         _IRHW_RxClear();
     } 
     else
     {
        //free_irq(VECTOR_INT_P_IR, dev_id);
     }
     return IR_SUCC;
}

/* HWIRRX export functions */

/**
 * static int _IRHW_RxSetIsr(bool fgSet)
 *     set or clear ISR of IR
 *     fgSet  true: set ISR; false: clear ISR
 *     IR_SUCC             successfully
 *     IR_FAIL               the x_reg_isr failed.
 */
void IRHW_RxRdConf(int * pi4Config, int * pi4SaPeriod,
                   int * pi4Threshold)
{
    if ((pi4Config == NULL) || (pi4SaPeriod == NULL)
        || (pi4Threshold == NULL))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("[IR]IRHW_RxRdConf error!\r\n")));
        return;
    }
    *pi4Config = (int) IR_READ32(IRRX_CONFIG_HIGH_REG);
    *pi4SaPeriod = (int) IR_READ32(IRRX_CONFIG_LOW_REG);
    *pi4Threshold = (int) IR_READ32(IRRX_THRESHOLD_REG);
}


/**
 *   IRHW_RxWrConf(int i4Config, int i4SaPeriod, int i4Threshold)
 *     Set IR config register.
 *    i4Config  Mix config value.
 *                   i4SaPeriod Sampling period, if it's nec i4SaPeriod =
 *                   560us/9.5us
 *                   i4Threshold     containing the deglitch and threshold value.
 */
void IRHW_RxWrConf(int i4Config, int i4SaPeriod, int i4Threshold)
{
    /* Config IRRX registers */
    IR_WRITE32(IRRX_CONFIG_HIGH_REG, (unsigned int) i4Config);
    IR_WRITE32(IRRX_CONFIG_LOW_REG, (unsigned int) i4SaPeriod);
    IR_WRITE32(IRRX_THRESHOLD_REG, (unsigned int) i4Threshold);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] i4Config = %x!!\r\n"),IR_READ32(IRRX_CONFIG_HIGH_REG)));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] i4SaPeriod = %x!!\r\n"),IR_READ32(IRRX_CONFIG_LOW_REG)));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] i4Threshold = %x!!\r\n"),IR_READ32(IRRX_THRESHOLD_REG)));
}


/**
 * int IRHW_RxInit(int i4Config, int i4SaPeriod, int i4Threshold)
 *    IR hardware init function, write config register and set ISR.
 *     i4Config  Mix config value.
 *                   i4SaPeriod Sampling period, if it's nec i4SaPeriod =
 *                   560us/9.5us
 *                   i4Threshold     containing the deglitch and threshold value.
 */
int IRHW_RxInit(int i4Config, int i4SaPeriod, int i4Threshold, void *dev_id)
{
    int i4Ret;

    //IR_WRITE32(IRRX_UP_CFG, (IR_READ32(IRRX_UP_CFG) & ~(1 << 16)));//Engines access by ARM only
#if (CONFIG_ARM2_EJECT)
    BIM_GETHWSemaphore(HSMPHE_SINFO4, 0);
    BIM2_WRITE32(REG_RW_SINFO4_REG, (BIM2_READ32(REG_RW_SINFO4_REG) | FB_ARM1_IRRX_INIT));
    BIM_ReleaseHWSemaphore(HSMPHE_SINFO4);
#endif
    if((IR_READ32(IRRX_PDSTAT) & IR_IR_WAK) != 0)
    {
        IR_WRITE32(IRRX_PDSTCLR, 0x01); 
    }
    IR_WRITE32(IRRX_IREXP_EN, 0);
    IR_WRITE32(IRRX_WAKEN, (IR_READ32(IRRX_WAKEN) & (~IR_WAKEN))); 
    IRHW_RxWrConf(i4Config, i4SaPeriod, i4Threshold);
    IR_WRITE32(IRRX_CLKPDN, (IR_READ32(IRRX_CLKPDN) & (~IRRXPD)));

#if (IRRX_USE_27M)
    IR_WRITE32(IRRX_IRCKSEL, (IR_READ32(IRRX_IRCKSEL) | (1<<0)));
    IR_WRITE32(IRRX_IRCKSEL, (IR_READ32(IRRX_IRCKSEL) & (~(IRCLKSEL_MASK<<4))) | (CLK_SEL_IR_DIV_1_256<<4));
#else
    
    IR_WRITE32(IRRX_IRCKSEL, (IR_READ32(IRRX_IRCKSEL) & (~(1<<0))));
    if (_dwIRProtocol == IRRX_RC_RC5)
        IR_WRITE32(IRRX_IRCKSEL, (IR_READ32(IRRX_IRCKSEL) & (~(IRCLKSEL_MASK<<4))) | (CLK_SEL_IR_DIV_1_16<<4)); 
    else
        IR_WRITE32(IRRX_IRCKSEL, (IR_READ32(IRRX_IRCKSEL) & (~(IRCLKSEL_MASK<<4))) | (CLK_SEL_IR_DIV_1_8<<4));
#endif

    i4Ret = _IRHW_RxSetIsr((bool) true, dev_id);
    IR_WRITE32(IRRX_INTEN, (IR_READ32(IRRX_INTEN) | IR_INTEN));

    return i4Ret;
}


int i4IrHWUninit(void *dev_id)
{
    int i4Ret;
    i4Ret = IRHW_RxStop(dev_id);

    IR_WRITE32(IRRX_CONFIG_HIGH_REG, (IR_READ32(IRRX_CONFIG_HIGH_REG) | IRRX_CH_DISPD));

    IR_WRITE32(IRRX_CLKPDN, (IR_READ32(IRRX_CLKPDN) | IRRXPD));

  #if (CONFIG_ARM2_EJECT)
    BIM_GETHWSemaphore(HSMPHE_SINFO4, 0);
    BIM2_WRITE32(REG_RW_SINFO4_REG, (BIM2_READ32(REG_RW_SINFO4_REG) & (~FB_ARM1_IRRX_INIT)));
    BIM_ReleaseHWSemaphore(HSMPHE_SINFO4);
  #endif
  
    return i4Ret;
}


/**
 * int IRHW_RxStop()
 *     Stop IR. The only thing need to do is to clear ISR of IR
 *     fgSet  true: set ISR; false: clear ISR
 *     retval  IR_SUCC             successfully
 *             IR_FAIL               the x_reg_isr failed.
 */
int IRHW_RxStop(void *dev_id)
{
    int i4Ret;

    i4Ret = _IRHW_RxSetIsr((bool) false, dev_id);
 
    IR_WRITE32(IRRX_INTEN, (IR_READ32(IRRX_INTEN) & (~IR_INTEN)));

    return i4Ret;
}


/**
 *   int IRHW_RxSetCallback(PFN_IRRXCB_T pfnCallback, PFN_IRRXCB_T * ppfnOld)
 *   Set the callback function, which is used to process the decoded
 *   key code.
 *   pfnCallback : the new callback function
 *                   ppfnOld:    the old callback function, which will be
 *                   returned.
 *   IR_SUCC             successfully
 *
 */
int IRHW_RxSetCallback(PFN_IRRXCB_T pfnCallback, PFN_IRRXCB_T * ppfnOld)
{
    if (ppfnOld != NULL)
    {
        *ppfnOld = _pfnIrRxCbFunc;
    }

    _pfnIrRxCbFunc = pfnCallback;
    return IR_SUCC;
}

/**
 * void IRHW_SetEnable(bool fgEnable)
 */
void IRHW_SetEnable(bool fgEnable)
{
    _fgIREnable = fgEnable;
}
