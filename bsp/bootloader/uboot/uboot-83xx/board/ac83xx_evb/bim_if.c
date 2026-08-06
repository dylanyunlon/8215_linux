/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/

//============================================================================
// Include files
//============================================================================
#include <asm/arch/x_bim.h>

UINT32 u4HWSemWaitCount = 0;

BOOL BIM_GETHWSemaphore(UINT32 u4Number, UINT32 u4TimeOut)
{
  UINT32 u4Count; 
  u4Count = 0;

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
      u4Count++;
      u4HWSemWaitCount++;    
    }
    while(((BIM_REG32(REG_RW_HSMPHE) & u4Number) != u4Number) && (u4TimeOut > u4Count));
  
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
