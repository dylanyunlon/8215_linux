/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */



#include "ac83xx.h"
#include "x_bim.h"
#include "dual_task.h"

#define TIMER_LOAD_VAL 0

//#define CFG_CLOCK_PER_TICKS 189000


/* macro to read the 32 bit timer */
#define READ_TIMER timer

UINT32 u4_get_irq_vector_id(void)
{
    UINT32 u4IrqVec;
    UINT32 u4Id;

    // level 1 
    u4IrqVec = REG_IRQ_STATUS & REG_IRQ_ENABLE;
    if(u4IrqVec)
    {
        for(u4Id = 0; u4Id < 32; u4Id++)
        {
            if(u4IrqVec & (0x1 << u4Id))
            {
                return u4Id;
            }
        }
    }

    // level 2
    u4IrqVec = REG_IRQ_STATUS2 & REG_IRQ_ENABLE2;
    if(u4IrqVec)
    {
        for(u4Id = 0; u4Id < 32; u4Id++)
        {
            if(u4IrqVec & (0x1 << u4Id))
            {
                return u4Id + 32;
            }
        }
    }

    // level 3
    u4IrqVec = REG_IRQ_STATUS3 & REG_IRQ_ENABLE3;
    if(u4IrqVec)
    {
        for(u4Id = 0; u4Id < 32; u4Id++)
        {
            if(u4IrqVec & (0x1 << u4Id))
            {
                return u4Id + 64;
            }
        }
    }
    // level 4
    u4IrqVec = REG_IRQ_STATUS4 & REG_IRQ_ENABLE4;
    if(u4IrqVec)
    {
        for(u4Id = 0; u4Id < 32; u4Id++)
        {
            if(u4IrqVec & (0x1 << u4Id))
            {
                return u4Id + 96;
            }
        }
    }
    return IRQ_VECTOR_MAX_NUM;
}

void v_enable_bim_irq(UINT32 u4Id)
{
    // level 1
    if(u4Id < 32)
    {
        REG_IRQ_ENABLE = REG_IRQ_ENABLE | (0x1 << u4Id);
        return;
    }

    // level 2  
    if(u4Id < 64)
    {
        u4Id -= 32;
        REG_IRQ_ENABLE2 = REG_IRQ_ENABLE2 | (0x1 << u4Id);
        return;
    }

    // level 3
    if(u4Id < 96)
    {
        u4Id -= 64;
        REG_IRQ_ENABLE3 = REG_IRQ_ENABLE3 | (0x1 << u4Id);
        return;
    }   
    // level 4
    if(u4Id < 128)
    {
        u4Id -= 96;
        REG_IRQ_ENABLE4 = REG_IRQ_ENABLE4 | (0x1 << u4Id);
        return;
    }
}

void v_disable_bim_irq(UINT32 u4Id)
{
    // level 1
    if(u4Id < 32)
    {
        REG_IRQ_ENABLE = REG_IRQ_ENABLE & ~(0x1 << u4Id);
        return;
    }

    // level 2  
    if(u4Id < 64)
    {
        u4Id -= 32;
        REG_IRQ_ENABLE2 = REG_IRQ_ENABLE2 & ~(0x1 << u4Id);
        return;
    }

    // level 3
    if(u4Id < 96)
    {
        u4Id -= 64;
        REG_IRQ_ENABLE3 = REG_IRQ_ENABLE3 & ~(0x1 << u4Id);
        return;
    }   
    // level 4
    if(u4Id < 128)
    {
        u4Id -= 96;
        REG_IRQ_ENABLE4 = REG_IRQ_ENABLE4 & ~(0x1 << u4Id);
        return;
    }
}

void v_clear_bim_irq(UINT32 u4Id)
{
    // level 1

    if(u4Id < 32)
    {
        u4Id = (0x1 << u4Id);
        if (u4Id & (TOCORISC_INTR_CLR |T0C_INTR_CLR |
            T1C_INTR_CLR |T2C_INTR_CLR))
        {
            REG_IRQ_ARM2_CLEAR = u4Id;
        }
        else
        {
            REG_IRQ_CLEAR = u4Id;
        }
        REG_IRQ_STATUS = u4Id;
        return;
    }
    // level 2  
    if(u4Id < 64)
    {
        u4Id -= 32;
        REG_IRQ_CLEAR2 = (0x1 << u4Id);
        REG_IRQ_STATUS2 = (0x1 << u4Id);
        return;
    }

    // level 3
    if(u4Id < 96)
    {
        u4Id -= 64;
        REG_IRQ_CLEAR3= (0x1 << u4Id);
        REG_IRQ_STATUS3= (0x1 << u4Id);
        return;
    }   
    // level 4
    if(u4Id < 128)
    {
        u4Id -= 96;
        REG_IRQ_CLEAR4= (0x1 << u4Id);
        REG_IRQ_STATUS4= (0x1 << u4Id);
        return;
    }
}


/* nothing really to do with interrupts, just starts up a counter. */

void v_timer_interrupt_init(void)
{
    /* Start timer irq */
    REG_TIMER_LIMIT = CFG_CLOCK_PER_TICKS;
    REG_TIMER_COUNT = CFG_CLOCK_PER_TICKS;
    REG_TIMER_CONTROL = 0x3;
    v_enable_bim_irq(VECTOR_T0);

}

void v_timer1_interrupt_init(void)
{
    /* Start timer irq */
    REG_TIMER1_LIMIT = CFG_CLOCK_PER_TICKS;
    REG_TIMER1_COUNT = CFG_CLOCK_PER_TICKS;
    REG_TIMER1_CONTROL |= 0x300;
    v_enable_bim_irq(VECTOR_T1);
}

int interrupt_init (void)
{
    /*
    * setup up stacks if necessary
    */
    /* turn all irq off */     
    REG_IRQ_ENABLE  = 0;
    REG_IRQ_ENABLE2 = 0;
    REG_IRQ_ENABLE3 = 0;
    REG_IRQ_ENABLE4 = 0;

    /* clear all pending irq */    
    //REG_IRQ_CLEAR    = 0xFFFFFFFF;
    REG_IRQ_STATUS   = 0xFFFFFFFF;
    //REG_IRQ_CLEAR2   = 0xFFFFFFFF;
    REG_IRQ_STATUS2  = 0xFFFFFFFF;
    //REG_IRQ_CLEAR3   = 0xFFFFFFFF;
    REG_IRQ_STATUS3  = 0xFFFFFFFF;
    //REG_IRQ_CLEAR4   = 0xFFFFFFFF;
    REG_IRQ_STATUS4  = 0xFFFFFFFF;

    REG_IRQ_ARM2_CLEAR = 0xFFFFFFFF;    //ARM2 clear register

    return(0);
}
