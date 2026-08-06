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
#include <linux/module.h> 
#include <mach/ac83xx_basic.h>

uint32_t u4HWSemWaitCount = 0;
//extern uint32_t u4HWSemWaitCount ;

#ifndef NOHWSEMAPHORE
bool BIM_GETHWSemaphore(uint32_t u4Number, uint32_t u4TimeOut)
{
  uint32_t u4Count; 
  u4Count = 0;

  if(u4TimeOut == 0)
  {
    do
    {
      BIM_WRITE32(REG_RW_HSMPHE, u4Number);
      u4HWSemWaitCount++;    
    }
    while(((BIM_REG32(REG_RW_HSMPHE) & u4Number) != u4Number));  
    
    return true;
  }	
  else
  {
    do
    {
      BIM_WRITE32(REG_RW_HSMPHE, u4Number);
      u4Count++;
      u4HWSemWaitCount++;    
    }
    while(((BIM_REG32(REG_RW_HSMPHE) & u4Number) != u4Number) && (u4TimeOut > u4Count));
  
    if(u4TimeOut <= u4Count)
    {
      return false;
    }  	
    else
    {
      return true;
    }	
  } 
}
#else
bool BIM_GETHWSemaphore(uint32_t u4Number, uint32_t u4TimeOut)
{
  return true;
}
#endif // !NOHWSEMAPHORE

EXPORT_SYMBOL(BIM_GETHWSemaphore);

bool BIM_ReleaseHWSemaphore(uint32_t u4Number)
{
  BIM_WRITE32(REG_RW_HSMPHE, u4Number);
  return true;  
}

EXPORT_SYMBOL(BIM_ReleaseHWSemaphore);

/*
bool BIM_IsIrqPending(uint32_t u4Vector)
{
  u32 regval32;
  unsigned long flags;
	
  if (!_bim_is_vector_valid(u4Vector))
  {
       return false;
  }

  local_irq_save(flags);
  
  if(u4Vector < 32)
  {
	  regval32 = BIM_REG32(REG_IRQST);
	  regval32 &= (1 << u4Vector);
  }
  else if((u4Vector >= 32) && (u4Vector < 64))
  {
	  u4Vector = u4Vector - 32;
	  regval32 = BIM_REG32(REG_IRQST2);
	  regval32 &= (1 << u4Vector);
  }
  else if((u4Vector >= 64) && (u4Vector < 96))
  {
	  u4Vector = u4Vector - 64;
	  regval32 = BIM_REG32(REG_IRQST3);
	  regval32 &= (1 << u4Vector);
  }
  else
  {
	  // pdwn extend 
	  u4Vector = u4Vector - 96;
	  regval32 = PDWNC_READ32(REG_RW_INTSTA);
	  regval32 &= (1 << u4Vector);
  }
  
  local_irq_restore(flags);

  return (regval32 != 0); 
}
EXPORT_SYMBOL(BIM_IsIrqPending);
*/
