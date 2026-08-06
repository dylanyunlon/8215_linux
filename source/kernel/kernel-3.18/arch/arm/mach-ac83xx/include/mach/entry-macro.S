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
#include <mach/hardware.h>
#include <mach/irqs.h>

/* leave disable_fiq as an empty macro */
    .macro disable_fiq
    .endm

	.macro  get_irqnr_preamble, base, tmp
	.endm

	.macro  arch_ret_to_user, tmp1, tmp2
	.endm


/* l1 -- irq_stat = irq_stat & irq_en & irq_priority_h */
/* tmp : input value must be priority mask */
    .macro irq_stat_l1, irqstat, base, tmp

    ldr \irqstat, [\base, #REG_IRQST]
    and \irqstat, \irqstat, \tmp

    ldr \tmp, [\base, #REG_IRQEN]
    and \irqstat, \irqstat, \tmp

    .endm

/* l2 -- irq_stat = irq_stat & irq_en & irq_priority_h */
/* tmp : input value must be priority mask */
    .macro irq_stat_l2, irqstat, base, tmp

    ldr \irqstat, [\base, #REG_IRQST2]
    and \irqstat, \irqstat, \tmp

    ldr \tmp, [\base, #REG_IRQEN2]
    and \irqstat, \irqstat, \tmp

    .endm

/* l3 -- irq_stat = irq_stat & irq_en & irq_priority_h */
/* tmp : input value must be priority mask */
    .macro irq_stat_l3, irqstat, base, tmp

    ldr \irqstat, [\base, #REG_IRQST3]
    and \irqstat, \irqstat, \tmp

    ldr \tmp, [\base, #REG_IRQEN3]
    and \irqstat, \irqstat, \tmp

    .endm

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
/* l4 -- irq_stat = irq_stat & irq_en & irq_priority_h */
/* tmp : input value must be priority mask */
    .macro irq_stat_l4, irqstat, base, tmp

    ldr \irqstat, [\base, #REG_IRQST4]
    and \irqstat, \irqstat, \tmp

    ldr \tmp, [\base, #REG_IRQEN4]
    and \irqstat, \irqstat, \tmp
    .endm
#endif

/* get irqstat in priority h/m/l */
    .macro irq_stat_priority, irqnr, irqstat, base, tmp
    /* L1 */
    ldr \tmp, [\irqnr, #0]
    irq_stat_l1 \irqstat, \base, \tmp

    cmp \irqstat, #0
    bne 5001f /* l1 found */

    /* L2 */
    ldr \tmp, [\irqnr, #4]
    irq_stat_l2 \irqstat, \base, \tmp

    cmp \irqstat, #0
    bne 5002f /* l2 found */
    
    /* L3 */
    ldr \tmp, [\irqnr, #8]
    irq_stat_l3 \irqstat, \base, \tmp

    cmp \irqstat, #0
    bne 5003f /* l3 found */

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
    /* L4 */
    ldr \tmp, [\irqnr, #12]
    irq_stat_l4 \irqstat, \base, \tmp

    cmp \irqstat, #0
    bne 5004f /* l4 found */
#endif
    .endm

/* get_irqnr_and_base */
    .macro get_irqnr_and_base, irqnr, irqstat, base, tmp

    ldr \base, =BIM_VIRT

    /* priority high */
    ldr \irqnr, =u4_pm_high
    irq_stat_priority \irqnr, \irqstat, \base, \tmp
    
    /* priority mid */
    ldr \irqnr, =u4_pm_mid
    irq_stat_priority \irqnr, \irqstat, \base, \tmp
    
    /* priority low */
    ldr \irqnr, =u4_pm_low
    irq_stat_priority \irqnr, \irqstat, \base, \tmp
    
    /* no found */
5000:
    mov \irqnr, #NR_IRQS
    b 6000f

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
    /* l4 found */
5004:
    clz \irqnr, \irqstat
    rsb \irqnr, \irqnr, #127
    b 6000f
#endif

    /* l3 found */
5003:
    clz \irqnr, \irqstat
    rsb \irqnr, \irqnr, #95
    b 6000f

    /* l2 found */
5002:
    clz \irqnr, \irqstat
    rsb \irqnr, \irqnr, #63



    /* check pwdnc */
    cmp \irqnr, #VECTOR_PWDNC    
    bne 6000f

    /* pwdnc extend */
    ldr \base, =PDWNC_VIRT
    
    ldr \irqstat, [\base, #REG_RW_INTSTA]
    ldr \tmp, [\base, #REG_RW_INTEN]
    and \irqstat, \irqstat, \tmp

    cmp \irqstat, #0
    beq 5000b

    clz \irqnr, \irqstat
#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
    rsb \irqnr, \irqnr, #159
#else
    rsb \irqnr, \irqnr, #127
#endif
    b 6000f

    /* l1 found */
5001:
    clz \irqnr, \irqstat
    rsb \irqnr, \irqnr, #31
    b 6000f

    /* end */
6000:
    .endm     
/* 
 * returns highest prio irq in irqnr,
 * all other regs are used as tmp regs
 */


