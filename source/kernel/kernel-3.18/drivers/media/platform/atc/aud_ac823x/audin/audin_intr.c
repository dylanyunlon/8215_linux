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

/***************************************************************************/
/**************************  Header File Include     ******************/
/***************************************************************************/
#ifdef AUD_LIN_INT
#include "aud_oal.h"
#include "x_assert.h"
#include "audin_reg.h"
#include "audin_ret_define.h"
#include "aud_3360_reg_afe.h"
#include "aud_debug.h"
#include "audin_if.h"
#if CONFIG_DRV_AUD_AC83XX
#include "x_bim.h"
#else
#include "bim.h"
#endif


/***************************************************************************/
/**************************  Globe Variable Define     ******************/
#define  LINEIN_INT_VECTOR       VECTOR_SPD

bool fgLineInEnable;

/***************************************************************************/
/**************************  Function declare    ******************/
/***************************************************************************/

/***************************************************************************/
/**************************  Function      ******************/
/***************************************************************************/

/************************************************************************
Function    : void vLineInIrqHandler()
Description : line in isr
Parameter   : None
Return      : None
************************************************************************/
static void vLineInIrqHandler(u16 u2Vector)
{
    if(fgLineInEnable == FALSE)
    {
        LOG(LOG_DATAF, TEXT("[AudIn]ENTER vLineInIrqHandler.\n"));
        fgLineInEnable = TRUE;
    }
#if CONFIG_DRV_AUD_AC83XX
    BIM_ClearIrq(LINEIN_INT_VECTOR);
#else
    mt33xx_mask_ack_bim_irq(LINEIN_INT_VECTOR);
#endif
    return;
}

/************************************************************************
Function    : void vSpdifLineInIRQOn()
Description : This function will turn on multiple line in H/W interrupt
Parameter   : None
Return      : None
************************************************************************/
void vSpdifLineInIRQOn(void)
{
    x_os_isr_fct pfnOldIsr;

    AUD_REG_BITS_WRITE(AUD_SPLIN_CTRL_ADDR,
                       AUD_SPLIN_CTRL_INTPRD_START,
                       AUD_SPLIN_CTRL_INTPRD_NUM,
                       AUD_SPLIN_DATALEN_INTR_256DW);

    LOG(LOG_FEATURE, TEXT("[AudIn]Init Line In Irq.\n"));
    VERIFY(x_reg_isr(LINEIN_INT_VECTOR, vLineInIrqHandler, &pfnOldIsr) == OSR_OK);

#if CONFIG_DRV_AUD_AC83XX
    BIM_EnableIrq(LINEIN_INT_VECTOR);
#else
    mt33xx_unmask_bim_irq(LINEIN_INT_VECTOR);
#endif
}

/************************************************************************
Function    : void vSpdifLineInIRQOff()
Description : This function will turn off multiple line in H/W interrupt
Parameter   : None
Return      : None
************************************************************************/
void vSpdifLineInIRQOff(void)
{
    x_os_isr_fct pfnOldIsr;
    
#if CONFIG_DRV_AUD_AC83XX
    BIM_DisableIrq(LINEIN_INT_VECTOR);
#else
    mt33xx_mask_bim_irq(LINEIN_INT_VECTOR);
#endif
    VERIFY(x_reg_isr(LINEIN_INT_VECTOR, NULL, &pfnOldIsr) == OSR_OK);
}

#endif

