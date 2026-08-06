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
#include <asm/arch/nand_operation.h>
#include <nand.h>
#include <common.h>
#include <malloc.h>

#include <ac83xx_gpio.h>

//============================================================================
// public functions
//============================================================================
#define ALIGN_L(data, align) (data & ~(align-1))
#define ALIGN_H(data, align) ((data + align -1) & ~(align-1))

//============================================================================
// public functions
//============================================================================
int mtk_nand_read(u_long dstaddr, u_long noffset, u_long size)
{
	int r;
	nand_info_t *nand = &nand_info[nand_curr_device];
	size_t length = (size_t)size;

	/* nand read */
	r = nand_read_skip_bad(nand, (loff_t)noffset, &length, (u_char *)dstaddr);
	if(r)
	{
		puts("[mtk_nand_read] nand_read_skip_bad ERROR.\n");
	}

	return r;
}

int mtk_nand_erase(u_long noffset, u_long size)
{
	int r;
	nand_erase_options_t optse;
	nand_info_t *nand = &nand_info[nand_curr_device];
	
	memset(&optse, 0, sizeof(optse));
	optse.offset = noffset;
	optse.length = (size_t)size;
	optse.jffs2  = 0;
	optse.quiet  = 1;
	r = nand_erase_opts(nand, &optse);	
	if(r)
	{
		puts("[mtk_nand_erase] erase fail\n");
	}

	return r;
}

int mtk_nand_write_align(u_long srcaddr, u_long size, u_long noffset)
{
	int r;
	nand_erase_options_t optse;
	nand_info_t *nand = &nand_info[nand_curr_device];
	size_t length = (size_t)ALIGN_H(size, nand->erasesize);
	u_char* pbuf_read;
	u_long checksize;
	
	/* erase */
	r = mtk_nand_erase(noffset, length);
	if(r)
	{
		puts("[mtk_nand_write_align] erase fail\n");
		return r;
	}

	/* write */
	r = nand_write_skip_bad(nand, (loff_t)noffset, &length, (u_char *)srcaddr);
	if(r)
	{
		puts("[mtk_nand_write_align] write fail\n");
		return r;
	}

	/* read back check */
	pbuf_read = (u_char*)malloc(length);

	if (pbuf_read == NULL)
	{
		puts("[mtk_nand_write_align] malloc failed\n");
		return -1;
	}

	r = nand_read_skip_bad(nand, (loff_t)noffset, &length, (u_char *)pbuf_read);
	if(r)
	{
		puts("[mtk_nand_write_align] nand_read_skip_bad ERROR.\n");
		free(pbuf_read);
		return r;
	}

    checksize = 0;
    while(1)
    {
        if(checksize == size)
        {
            break;
        }
        else if((checksize + 0x20000) <= size)
        {
            r = memcmp((void *) (pbuf_read + checksize), (void *) (srcaddr + checksize), 0x20000);
			checksize += 0x20000;
        }
		else
        {
            r = memcmp((void *) (pbuf_read + checksize), (void *) (srcaddr + checksize), (size-checksize));
			checksize = size;
        }
			
    	if (r != 0)
    	{
    		puts("[mtk_nand_write_align] compare src data and read back data failed\n");
    		free(pbuf_read);
    		return -1;
    	}
		
#if SUPPORT_UPG_FLASH_LED
		v_set_upg_status(UPG_CHECK, 0x20000);
#endif
    };

	free(pbuf_read);
	return 0;
}

int mtk_nand_write(u_long srcaddr, u_long size, u_long noffset)
{
	int r;
	char* buf;
	nand_info_t *nand = &nand_info[nand_curr_device];
	u_long blocksize = nand->erasesize;
	
	u_long noffset_1st = ALIGN_L(noffset, blocksize);
	u_long data_sz_1st = blocksize - (noffset - noffset_1st);
	if(size < data_sz_1st) data_sz_1st = size;

	printf("[mtk_nand_write] src=0x%08x, size=0x%08x, noffset=0x%08x\n", srcaddr, size, noffset);

	/* check if aligned */
	if(noffset_1st == noffset)
	{
		return mtk_nand_write_align(srcaddr, size, noffset);
	}
	
	/* first block */
	buf = malloc(blocksize);

	if(buf == NULL)
	{
		puts("[mtk_nand_write] malloc for 1-st block fail.\n");
		return -1;
	}

	r = mtk_nand_read((u_long)buf, noffset_1st, blocksize);
	if(r)
	{
		puts("[mtk_nand_write] mtk_nand_read for 1-st block fail.\n");
		free(buf);
		return -1;
	}

	memcpy((buf + (noffset - noffset_1st)), (srcaddr), data_sz_1st);

	r = mtk_nand_write_align((u_long) buf, blocksize, noffset_1st);
	if(r)
	{
		puts("[mtk_nand_write] mtk_nand_write for 1-st block fail.\n");
		free(buf);
		return r;
	}
	
	free(buf);

	/* block aligned data */
	if(size > data_sz_1st)
	{
		r = mtk_nand_write_align((u_long)(srcaddr + data_sz_1st), (size - data_sz_1st), (noffset_1st + blocksize));
		if(r)
		{
			puts("[mtk_nand_write] mtk_nand_write for rest fail.\n");
			return r;
		}
	}

	return 0;	
}
