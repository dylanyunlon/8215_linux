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
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include <asm/cacheflush.h>

#include "gfx_hal_if.h"
#include "gfx_if.h"
#include "gfx_dif.h"

//*****for sync to emulation code start*****
#define RET_OK 0
#define EvPrintf printk
//*****for sync to emulation code end*****

//extern void HalFlushInvalidateDCache(void);
int gfx_bitblt(uint32_t u4TTB,gfx_ioc_paras *gfx_paras)
{
	UINT8 *pu1SrcAddr = NULL;
	UINT8 *pu1DstAddr = NULL;
	uint32_t u4SrcCM,u4DstCM,u4SrcPitch,u4DstPitch,u4SrcX,u4SrcY,u4DstX,u4DstY,u4Width,u4Height;
	uint32_t u4Color,u4Alpha,u4Bltopt;
	uint32_t u4SrcDisKey,u4SrcKeyIn,u4SrcKeyMin,u4SrcKeyMax,u4DstDisKey,u4DstKeyIn,u4DstKeyMin,u4DstKeyMax;
	if(NULL == gfx_paras)
		return -EFAULT;
	
	pu1SrcAddr = (UINT8 *)((gfx_paras->srcBuffer).u4Addr);
	pu1DstAddr = (UINT8 *)((gfx_paras->dstBuffer).u4Addr);

	if((NULL == pu1SrcAddr) || (NULL == pu1DstAddr))
		return -EFAULT;

	u4SrcPitch 	= gfx_paras->srcBuffer.u4Pitch;
	u4SrcX		= gfx_paras->srcBuffer.u4X;
	u4SrcY		= gfx_paras->srcBuffer.u4Y;
	u4SrcCM		= gfx_paras->srcBuffer.u4CM;
	
	u4DstPitch 	= gfx_paras->dstBuffer.u4Pitch;
	u4DstX		= gfx_paras->dstBuffer.u4X;
	u4DstY		= gfx_paras->dstBuffer.u4Y;
	u4DstCM		= gfx_paras->dstBuffer.u4CM;

	u4Width		= gfx_paras->dstBuffer.u4Width;
	u4Height	= gfx_paras->dstBuffer.u4Height;

	u4Color		= gfx_paras->u4Color;
	u4Alpha		= gfx_paras->u4Alpha;
	u4Bltopt	= gfx_paras->u4Bltopt;

	u4SrcDisKey = gfx_paras->srcColorkey.u4DisKey;
	u4SrcKeyIn  = gfx_paras->srcColorkey.u4KeyIn;
	u4SrcKeyMin = gfx_paras->srcColorkey.u4KeyMin;
	u4SrcKeyMax = gfx_paras->srcColorkey.u4KeyMax;

	u4DstDisKey = gfx_paras->dstColorkey.u4DisKey;
	u4DstKeyIn  = gfx_paras->dstColorkey.u4KeyIn;
	u4DstKeyMin = gfx_paras->dstColorkey.u4KeyMin;
	u4DstKeyMax = gfx_paras->dstColorkey.u4KeyMax;
	
	//printk(KERN_INFO "[GFX]u4TTB : 0x%08X\n",u4TTB);
	//printk(KERN_INFO "[GFX]gfx_paras : SrcAddr 0x%08X, DstAddr 0x%08X\n",pu1SrcAddr,pu1DstAddr);

	flush_cache_all();
	//GFX_FLUSH_DCACHE();
	
	#if 1
	if (0 != GFX_Reset(0, GFX_RESET_BOTH))
    {
    	EvPrintf("[Gfx]Reset Failed \n");
		return -EINVAL;
    }
	GFX_DifSetMode(0, E_GFX_HW_8520_MOD);

	if(vGfxSetMMU(TRUE) == -1) {
		EvPrintf("[Gfx]vGfxSetMMU failed \n");
		return -EINVAL;
	}
	if (RET_OK != GFX_SetSrc(0, pu1SrcAddr, u4SrcCM, u4SrcPitch))
    {
        EvPrintf("[Gfx][Ev]GFX_SetSrc Failed \n");
		return -EINVAL;
    }
    if (RET_OK != GFX_SetDst(0, pu1DstAddr, u4DstCM, u4DstPitch))
    {
        EvPrintf("[Gfx][Ev]GFX_SetDst Failed \n");
		return -EINVAL;
    }

    if (RET_OK != GFX_SetBltOptEx(0, u4Bltopt, u4SrcDisKey, u4SrcKeyIn, u4SrcKeyMin, 
			u4SrcKeyMax, u4DstDisKey, u4DstKeyIn, u4DstKeyMin, u4DstKeyMax))
    {
        EvPrintf("[Gfx][Ev]GFX_SetBltOpt Failed \n");
		return -EINVAL;
    }

    if (RET_OK != GFX_SetAlpha(0, u4Alpha))
    {
        EvPrintf("[Gfx][Ev]GFX_SetAlpha Failed \n");
		return -EINVAL;
    }
        
    if (RET_OK != GFX_SetColor(0, u4Color))
    {
        EvPrintf("[Gfx][Ev]GFX_SetAlpha Failed \n");
		return -EINVAL;
    }
    if (RET_OK != GFX_BitBlt(0, u4SrcX, u4SrcY, u4DstX, u4DstY, u4Width, u4Height))
    {
        EvPrintf("[Gfx][Ev]GFX_BitBlt Failed \n");
		return -EINVAL;
    }
    
    if (RET_OK != GFX_Flush(0))
    {
        EvPrintf("[Gfx][Ev]GFX_Flush Failed \n");
		return -EFAULT;
    }
    
    //GFX_FLUSH_DCACHE();
    flush_cache_all();
	#endif
	return 0;
}

int gfx_fillrect(uint32_t u4TTB,gfx_ioc_paras *gfx_paras)
{
	UINT8 *pu1DstAddr = NULL;
	uint32_t u4DstCM,u4DstPitch,u4DstX,u4DstY,u4Width,u4Height;
	uint32_t u4Color;
	
	if(NULL == gfx_paras)
		return -EFAULT;
	
	pu1DstAddr = (UINT8 *)((gfx_paras->dstBuffer).u4Addr);

	if(NULL == pu1DstAddr)
		return -EFAULT;
	
	u4DstPitch 	= gfx_paras->dstBuffer.u4Pitch;
	u4DstX		= gfx_paras->dstBuffer.u4X;
	u4DstY		= gfx_paras->dstBuffer.u4Y;
	u4DstCM		= gfx_paras->dstBuffer.u4CM;

	u4Width		= gfx_paras->dstBuffer.u4Width;
	u4Height	= gfx_paras->dstBuffer.u4Height;

	u4Color		= gfx_paras->u4Color;
	
	//printk(KERN_INFO "[GFX]u4TTB : 0x%08X\n",u4TTB);
	//printk(KERN_INFO "[GFX]gfx_paras : DstAddr 0x%08X\n",pu1DstAddr);
	
	flush_cache_all();
	//GFX_FLUSH_DCACHE();
	if (0 != GFX_Reset(0, GFX_RESET_BOTH))
    {
    	EvPrintf("[Gfx]Reset Failed \n");
		return -EINVAL;
    }
	GFX_DifSetMode(0, E_GFX_HW_8520_MOD);

	vGfxSetMMU(TRUE);
	
    if (RET_OK != GFX_SetDst(0, pu1DstAddr, u4DstCM, u4DstPitch))
    {
        EvPrintf("[Gfx][Ev]GFX_SetDst Failed \n");
		return -EINVAL;
    }
        
    if (RET_OK != GFX_SetColor(0, u4Color))
    {
        EvPrintf("[Gfx][Ev]GFX_SetAlpha Failed \n");
		return -EINVAL;
    }

	if (RET_OK != GFX_Fill(0, u4DstX, u4DstY, u4Width, u4Height))
    {
        EvPrintf("[Gfx][Ev]GFX_Fill Failed \n");
		return -EINVAL;
    }
    
    if (RET_OK != GFX_Flush(0))
    {
        EvPrintf("[Gfx][Ev]GFX_Flush Failed \n");
		return -EFAULT;
    }
    
    //GFX_FLUSH_DCACHE();
    flush_cache_all();
	return 0;
}

int gfx_stretchblt(uint32_t u4TTB,gfx_ioc_paras *gfx_paras)
{
	UINT8 *pu1SrcAddr = NULL;
	UINT8 *pu1DstAddr = NULL;
	uint32_t u4SrcCM,u4DstCM,u4SrcPitch,u4DstPitch,u4SrcX,u4SrcY,u4DstX,u4DstY;
	uint32_t u4SrcWidth,u4SrcHeight,u4DstWidth,u4DstHeight;
	
	if(NULL == gfx_paras)
		return -EFAULT;
	
	pu1SrcAddr = (UINT8 *)((gfx_paras->srcBuffer).u4Addr);
	pu1DstAddr = (UINT8 *)((gfx_paras->dstBuffer).u4Addr);

	if((NULL == pu1SrcAddr) || (NULL == pu1DstAddr))
		return -EFAULT;

	u4SrcPitch 	= gfx_paras->srcBuffer.u4Pitch;
	u4SrcX		= gfx_paras->srcBuffer.u4X;
	u4SrcY		= gfx_paras->srcBuffer.u4Y;
	u4SrcCM		= gfx_paras->srcBuffer.u4CM;
	
	u4DstPitch 	= gfx_paras->dstBuffer.u4Pitch;
	u4DstX		= gfx_paras->dstBuffer.u4X;
	u4DstY		= gfx_paras->dstBuffer.u4Y;
	u4DstCM		= gfx_paras->dstBuffer.u4CM;

	u4SrcWidth	= gfx_paras->srcBuffer.u4Width;
	u4SrcHeight	= gfx_paras->srcBuffer.u4Height;
	u4DstWidth	= gfx_paras->dstBuffer.u4Width;
	u4DstHeight	= gfx_paras->dstBuffer.u4Height;
	
	
	printk(KERN_INFO "[GFX]u4TTB : 0x%08X\n",u4TTB);
	printk(KERN_INFO "[GFX]gfx_paras : SrcAddr 0x%08X, DstAddr 0x%08X\n",pu1SrcAddr,pu1DstAddr);

	flush_cache_all();
	
	if (0 != GFX_Reset(0, GFX_RESET_BOTH))
    {
    	EvPrintf("[Gfx]Reset Failed \n");
		return -EINVAL;
    }
	GFX_DifSetMode(0, E_GFX_HW_8520_MOD);

	vGfxSetMMU(TRUE);

	if (RET_OK != GFX_SetSrc(0, pu1SrcAddr, u4SrcCM, u4SrcPitch))
    {
        EvPrintf("[Gfx][Ev]GFX_SetSrc Failed \n");
		return -EINVAL;
    }
    if (RET_OK != GFX_SetDst(0, pu1DstAddr, u4DstCM, u4DstPitch))
    {
        EvPrintf("[Gfx][Ev]GFX_SetDst Failed \n");
		return -EINVAL;
    }

	if (RET_OK != GFX_SetBltOpt(0, D_GFXFLAG_NONE, 0, 0xffffffff))
    {
        EvPrintf("[Gfx][Ev]GFX_SetBltOpt Failed \n");
		return -EINVAL;
    }

	if (RET_OK != GFX_StretchBlt(0, u4SrcX, u4SrcY, u4SrcWidth, u4SrcHeight, u4DstX, u4DstY, u4DstWidth, u4DstHeight))
    {
        EvPrintf("[Gfx][Ev]GFX_StretchBlt Failed \n");
		return -EINVAL;
    }
    
    if (RET_OK != GFX_Flush(0))
    {
        EvPrintf("[Gfx][Ev]GFX_Flush Failed \n");
		return -EFAULT;
    }
    flush_cache_all();
    //GFX_FLUSH_DCACHE();
	return 0;
}

int gfx_alphablend(uint32_t u4TTB,gfx_ioc_paras *gfx_paras)
{
	UINT8 *pu1SrcAddr = NULL;
	UINT8 *pu1DstAddr = NULL;
	uint32_t u4SrcCM,u4DstCM,u4SrcPitch,u4DstPitch,u4SrcX,u4SrcY,u4DstX,u4DstY,u4Width,u4Height;
	uint32_t u4Color,u4Alpha,u4Bltopt;
	uint32_t u4SrcDisKey,u4SrcKeyIn,u4SrcKeyMin,u4SrcKeyMax,u4DstDisKey,u4DstKeyIn,u4DstKeyMin,u4DstKeyMax;
	
	if(NULL == gfx_paras)
		return -EFAULT;
	
	pu1SrcAddr = (UINT8 *)((gfx_paras->srcBuffer).u4Addr);
	pu1DstAddr = (UINT8 *)((gfx_paras->dstBuffer).u4Addr);

	if((NULL == pu1SrcAddr) || (NULL == pu1DstAddr))
		return -EFAULT;

	u4SrcPitch 	= gfx_paras->srcBuffer.u4Pitch;
	u4SrcX		= gfx_paras->srcBuffer.u4X;
	u4SrcY		= gfx_paras->srcBuffer.u4Y;
	u4SrcCM		= gfx_paras->srcBuffer.u4CM;
	
	u4DstPitch 	= gfx_paras->dstBuffer.u4Pitch;
	u4DstX		= gfx_paras->dstBuffer.u4X;
	u4DstY		= gfx_paras->dstBuffer.u4Y;
	u4DstCM		= gfx_paras->dstBuffer.u4CM;

	u4Width		= gfx_paras->dstBuffer.u4Width;
	u4Height	= gfx_paras->dstBuffer.u4Height;

	u4Color		= gfx_paras->u4Color;
	u4Alpha		= gfx_paras->u4Alpha;
	u4Bltopt	= gfx_paras->u4Bltopt;

	u4SrcDisKey = gfx_paras->srcColorkey.u4DisKey;
	u4SrcKeyIn  = gfx_paras->srcColorkey.u4KeyIn;
	u4SrcKeyMin = gfx_paras->srcColorkey.u4KeyMin;
	u4SrcKeyMax = gfx_paras->srcColorkey.u4KeyMax;

	u4DstDisKey = gfx_paras->dstColorkey.u4DisKey;
	u4DstKeyIn  = gfx_paras->dstColorkey.u4KeyIn;
	u4DstKeyMin = gfx_paras->dstColorkey.u4KeyMin;
	u4DstKeyMax = gfx_paras->dstColorkey.u4KeyMax;
	
	printk(KERN_INFO "[GFX]u4TTB : 0x%08X\n",u4TTB);
	printk(KERN_INFO "[GFX]gfx_paras : SrcAddr 0x%08X, DstAddr 0x%08X\n",pu1SrcAddr,pu1DstAddr);
	
	flush_cache_all();
	
	if (0 != GFX_Reset(0, GFX_RESET_BOTH))
    {
    	EvPrintf("[Gfx]Reset Failed \n");
		return -EINVAL;
    }
	GFX_DifSetMode(0, E_GFX_HW_8520_MOD);

	vGfxSetMMU(TRUE);

	if (RET_OK != GFX_SetSrc(0, pu1SrcAddr, u4SrcCM, u4SrcPitch))
    {
        EvPrintf("[Gfx][Ev]GFX_SetSrc Failed \n");
		return -EINVAL;
    }
    if (RET_OK != GFX_SetDst(0, pu1DstAddr, u4DstCM, u4DstPitch))
    {
        EvPrintf("[Gfx][Ev]GFX_SetDst Failed \n");
		return -EINVAL;
    }

    if (RET_OK != GFX_SetBltOptEx(0, u4Bltopt, u4SrcDisKey, u4SrcKeyIn, u4SrcKeyMin, 
			u4SrcKeyMax, u4DstDisKey, u4DstKeyIn, u4DstKeyMin, u4DstKeyMax))
    {
        EvPrintf("[Gfx][Ev]GFX_SetBltOpt Failed \n");
		return -EINVAL;
    }

    if (RET_OK != GFX_SetAlpha(0, u4Alpha))
    {
        EvPrintf("[Gfx][Ev]GFX_SetAlpha Failed \n");
		return -EINVAL;
    }
        
    if (RET_OK != GFX_SetColor(0, u4Color))
    {
        EvPrintf("[Gfx][Ev]GFX_SetAlpha Failed \n");
		return -EINVAL;
    }
    if (RET_OK != GFX_BitBlt(0, u4SrcX, u4SrcY, u4DstX, u4DstY, u4Width, u4Height))
    {
        EvPrintf("[Gfx][Ev]GFX_BitBlt Failed \n");
		return -EINVAL;
    }
    
    if (RET_OK != GFX_Flush(0))
    {
        EvPrintf("[Gfx][Ev]GFX_Flush Failed \n");
		return -EFAULT;
    }
    flush_cache_all();
    //GFX_FLUSH_DCACHE();
	return 0;
}

