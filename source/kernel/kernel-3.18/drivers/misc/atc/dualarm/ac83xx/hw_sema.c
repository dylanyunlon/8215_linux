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
#include "x_bim.h"
#include "base_regs.h"

UINT32 u4HWSemWaitCount = 0;

BOOL BIM_GETHWSemaphore(UINT32 u4Number, UINT32 u4TimeOut)
{
    UINT32 u4Count; 
    u4Count = 0;

    if ((BIM_REG32(REG_RW_HSMPHE) & u4Number) == u4Number)
        return (TRUE);

    if(u4TimeOut == 0)
    {
        do
        {
            BIM_WRITE32(REG_RW_HSMPHE, u4Number);
            u4HWSemWaitCount++;    
        }
        while(((BIM_REG32(REG_RW_HSMPHE) & u4Number) != u4Number));  

        return TRUE;
    }	
    else
    {
        do
        {
            BIM_WRITE32(REG_RW_HSMPHE, u4Number);
            if ((BIM_REG32(REG_RW_HSMPHE) & u4Number) == u4Number)
                break;
            u4Count++;
            u4HWSemWaitCount++;    
        }
        while( u4TimeOut > u4Count);

        if(u4TimeOut <= u4Count)
        {
            return FALSE;
        }  	
        else
        {
            return TRUE;
        }	
    }

}	
  
BOOL BIM_ReleaseHWSemaphore(UINT32 u4Number)
{
  BIM_WRITE32(REG_RW_HSMPHE, u4Number);
  return TRUE;  
}

BOOL BIM_QueryHWSemaphore(UINT32 u4Number)
{
    return ((BIM_REG32(REG_RW_HSMPHE) & u4Number) == u4Number);
}
