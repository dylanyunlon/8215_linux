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

#include <nand.h>
#include <malloc.h>
#include <common.h>

#include <asm/arch/ac83xx_pdwnc.h>
#include <asm/arch/x_pdwnc.h>
#include <asm/arch/ac83xx_part_tbl.h>
#include <asm/arch/ac83xx_upg_status.h>
#include <asm/arch/nand_operation.h>

//============================================================================
// global parameters
//============================================================================

#define PDWN_FILE_NAME						"fast_init_param"
#define PDWN_FILE_SIZE				      	      	0x100000
#define PDWN_FILE_OFFSET				      	0x80000
#define MCU_FILE_OFFSET					0x4000
#define STDBY_CHECK_VALUE				    	0x24000164
#define DEBUG_USE					      	      	1

static UINT8 *pu1PdwnRegion = 0x0;

/******************************************************************************
* Private Function
******************************************************************************/


void vPDWNCResetFE(UINT32 fRst) 
{
    UINT32 u4Tmp1;

    u4Tmp1 = PDWNC_READ32(REG_RW_PAD_PD);	
    PDWNC_WRITE32(REG_RW_PAD_PD, (fRst<<RW_PAD_PD_OPWRSB_PD_OFFSET) | (u4Tmp1 & (~RW_PAD_PD_OPWRSB_PD)) );
}

void vPDWNCSRAMSet(UINT32 fMaster) 
{
    UINT32 u4Tmp1;
    
    u4Tmp1 = PDWNC_READ32(REG_RW_UP_CFG);	
    PDWNC_WRITE32(REG_RW_UP_CFG, (fMaster<<RW_UP_CFG_RAM_CK_SEL_OFFSET) | (u4Tmp1 & (~RW_UP_CFG_RAM_CK_SEL_MASK)) );
}

BOOL u1PDWNCClrSRAM(void) 
{
    unsigned int u4Tmp1, i;

    u4Tmp1 = PDWNC_READ32(REG_RW_UP_CFG);
    PDWNC_WRITE32(REG_RW_UP_CFG, (u4Tmp1 | 0x45));
        
    /* SRAM master to ARM*/
    u4Tmp1 = PDWNC_READ32(REG_RW_UP_CFG);	
    PDWNC_WRITE32(REG_RW_UP_CFG, u4Tmp1 & (~(0x3<<RW_UP_CFG_RAM_CK_SEL_OFFSET)) );

    /* Write */
    u4Tmp1 = 0;  //Set Addr to 0
    PDWNC_WRITE32(REG_RW_UP_ADDR, (u4Tmp1 & RW_UP_ADDR_UP_ADDR_MASK));
    for (i=0; i<0x4400; i++) {	// 16K+2K
        PDWNC_WRITE8(REG_RW_UP_DATA, SRAM_CLR_VALUE);
    }

    /* Read and check */
    u4Tmp1 = 0;  //Set Addr to 0
    PDWNC_WRITE32(REG_RW_UP_ADDR, (u4Tmp1 & RW_UP_ADDR_UP_ADDR_MASK));
    for (i=0; i<0x4400; i++) {	// 16K+2K
        u4Tmp1 = PDWNC_READ8(REG_RW_UP_DATA);
        if (u4Tmp1 != SRAM_CLR_VALUE) {
            printf("[PDWNC] Clear SRAM error!, Addr=0x%x, value=0x%x not 0x%x \r\n", i, u4Tmp1, SRAM_CLR_VALUE);	
            return FALSE;
        }	
    }

    return TRUE; 	
}

void vPDWNCT8032Reset(void) 
{
    UINT32 u4Tmp1;
    
    /* MCU Disable Reset */    
    u4Tmp1 = PDWNC_READ32(REG_RW_UP_CFG);	
    PDWNC_WRITE32(REG_RW_UP_CFG, u4Tmp1 | RW_UP_CFG_T8032_RST);

    /* MCU Enable Access SRAM */
    vPDWNCSRAMSet(SRAM_MASTER_T8032); 
    
    /* MCU Reset */    
    u4Tmp1 = PDWNC_READ32(REG_RW_UP_CFG);	
    PDWNC_WRITE32(REG_RW_UP_CFG, u4Tmp1 & (~RW_UP_CFG_T8032_RST));	
}

static int v_LoadPDwnCode(void)
{
    PART_TBL_ITEM *p_upg_status_part;
    UINT32 g_stdby_checking_reg;
    int r;

    PDWNC_WRITE32(REG_RW_RESRV1, STDBY_CHECK_VALUE);   // Enable standby flag 
    g_stdby_checking_reg = PDWNC_READ32(REG_RW_RESRV1);

#if DEBUG_USE
    printf("[v_LoadPDwnCode]:Standby on purpose REG_RW_RESRV1: %x\n", g_stdby_checking_reg);

    printf("[v_LoadPDwnCode]:Preparation!!\n");
#endif


    /* Allocate memory space for pdown file */
    pu1PdwnRegion = kmalloc( PDWN_FILE_SIZE * sizeof(UINT32), GFP_KERNEL);
    if(!pu1PdwnRegion)
    {
    	printf( "[v_LoadPDwnCode] PDWN Memory allocation error\n ");	
    	return -1;
    }
    memset(pu1PdwnRegion, 0, ( PDWN_FILE_SIZE * sizeof(UINT32)));

#if DEBUG_USE
    printf("[v_LoadPDwnCode]:Set memory : %x!!\n", (unsigned int )pu1PdwnRegion);
#endif

    /* get upg_status partition info */
    p_upg_status_part = ac83xx_get_part_info_by_name(PDWN_FILE_NAME);
    if(p_upg_status_part == NULL)
    {
        printf("[_i_get_upg_status_in_nand] Get upg_status partition info FAIL.\n");
        return -ERR_NO_PART_INFO;
    }
      
    r = mtk_nand_read((u_long)pu1PdwnRegion, p_upg_status_part->u4_offset, PDWN_FILE_SIZE);
    if(r != 0)
    {
       printf("[_i_get_upg_status_in_nand] mtk_nand_read FAIL. error number:[%d]\n", r);
    	free(pu1PdwnRegion);
    	return -ERR_NAND_READ_FAIL;
    }

#if DEBUG_USE	
    printf("[v_LoadPDwnCode]Read bytes at address 0x%x \n", (unsigned int )pu1PdwnRegion);
    printf("[v_LoadPDwnCode]:Load file into dram!!\n");
#endif

   return 0;

}

static void v_IRHW_PDWN(void)
{ 
   unsigned int i;
   unsigned int u4Tmp1;
   unsigned char *pu1Sourceaddress;

  // Make Sure All MW and Driver H/W and Task are Power Down
#if DEBUG_USE
   printf("[v_IRHW_PDWN] Get into here\n ");
#endif
  
  // un-Reset F/E
  vPDWNCResetFE(0);

#if DEBUG_USE
   printf("[v_IRHW_PDWN] Reset F/E\n ");
#endif

  // SRAM master change to ARM
   vPDWNCSRAMSet(SRAM_MASTER_RISC);

#if DEBUG_USE
   printf("<v_IRHW_PDWN> Set SRAM \n ");
#endif

  // Clear whole SRAM
   u1PDWNCClrSRAM();
#if DEBUG_USE
   printf("[v_IRHW_PDWN] Clear SRAM\n ");
#endif

  // Copy MCU.bin from DRAM to SRAM
    
#if DEBUG_USE
   printf("[v_IRHW_PDWN] Get the file from address 0x%x\n ", (unsigned int )(pu1PdwnRegion+PDWN_FILE_OFFSET));
#endif

//   pu1PdwnRegion = pu1PdwnRegion + PDWN_FILE_OFFSET;
    pu1Sourceaddress = (UINT8*)(pu1PdwnRegion+PDWN_FILE_OFFSET);
    PDWNC_WRITE32(REG_RW_UP_ADDR, 0);
    for (i=0; i<MCU_FILE_OFFSET; i++) {	
        PDWNC_WRITE8(REG_RW_UP_DATA, (*(pu1Sourceaddress+i)) & RW_UP_DATA_UP_DATA_MASK);
        //printf("[v_IRHW_PDWN] Addr=0x%x, value=0x%x \r\n", i, (*(pu1Sourceaddress+i)));	
    }
    /* Check */
    u4Tmp1 = 0;
    pu1Sourceaddress = (UINT8*)(pu1PdwnRegion+PDWN_FILE_OFFSET);
    PDWNC_WRITE32(REG_RW_UP_ADDR, (u4Tmp1 & RW_UP_ADDR_UP_ADDR_MASK));
    for (i=0; i<MCU_FILE_OFFSET; i++) 
    {	
        u4Tmp1 = PDWNC_READ8(REG_RW_UP_DATA);
	  		if (u4Tmp1 != (*(pu1Sourceaddress+i))) {
            printf("[v_IRHW_PDWN] Move code Error!! Addr=0x%x, value=0x%x should be 0x%x \r\n", i, u4Tmp1, (*(pu1Sourceaddress+i)));	
        }    
    }

#if DEBUG_USE
   printf("[v_IRHW_PDWN] Load power down code successfully!!\n");

   printf("[v_IRHW_PDWN] End here \n");
#endif

}


static void v_pwr_off_engine_set(UINT32 fEngineID, UINT32 fMaster) 
{
    UINT32 u4Tmp;
    
    u4Tmp = PDWNC_READ32(REG_RW_UP_CFG);
    switch(fEngineID) {
        case ENGINE_ALL:
            PDWNC_WRITE32(REG_RW_UP_CFG, (fMaster<<RW_UP_CFG_ENGINE_EN) | (u4Tmp & (~RW_UP_CFG_ENGINE_EN)));	
            break;	
            
        case ENGINE_ETHE:
            PDWNC_WRITE32(REG_RW_UP_CFG, (fMaster<<RW_UP_CFG_ETNET_EN) | (u4Tmp & (~RW_UP_CFG_ETNET_EN)));	
            break;	

        case ENGINE_UART:
            PDWNC_WRITE32(REG_RW_UP_CFG, (fMaster<<RW_UP_CFG_DBG_UART_EN) | (u4Tmp & (~RW_UP_CFG_DBG_UART_EN)));	
            break;	

        case ENGINE_CEC:
            PDWNC_WRITE32(REG_RW_UP_CFG, (fMaster<<RW_UP_CFG_CEC_EN) | (u4Tmp & (~RW_UP_CFG_CEC_EN)));	
            break;	

        default:            
            break;	
    }	
}

//============================================================================
// public functions
//============================================================================
void v_data_to_pdwnc(UINT32 u4Pa, UINT32 u4La, UINT32 u4Switch, UINT32 u4TradMode)
{
	unsigned int u4_cec_data;
	
	/* cec data */
	u4_cec_data = PDWNC_READ32(REG_RW_ARM_IND_DATA0);
	u4_cec_data = u4_cec_data & 0xFE000000;
	u4_cec_data = u4_cec_data | (u4La & 0xFF) | ((u4Pa & 0xFFFF)<<8);
	if(u4Switch != 0) 
		u4_cec_data = u4_cec_data | RW_ARM_IND_DATA0_BIT24;
	if(u4TradMode != 0)
		u4_cec_data = u4_cec_data | RW_ARM_IND_DATA0_BIT25;
	PDWNC_WRITE32(REG_RW_ARM_IND_DATA0, u4_cec_data);
}


int v_pwr_off(void)
{
	int ret_val;
	/* Use MCU to turn off the whole electricity */
	
	ret_val = v_LoadPDwnCode();
	
  if (ret_val != 0)
  {
#if DEBUG_USE
 	 printf("[v_pwr_off] v_LoadPDwnCode ERROR !!! End here \n");
#endif   	
 	 return -1;	
  }

#if DEBUG_USE	
	printf("[v_pwr_off]T8032 is loaded to DRAM\n");
#endif
	
	v_IRHW_PDWN();

#if DEBUG_USE	
	printf("[v_pwr_off]Ready to execute MCU on SRAM\n");
#endif

      //set WKRSC value is 0x00100000,hw default value is 0x01000000
      PDWNC_WRITE32(REG_RW_WKRSC,0x00100000);
      //vPDWNCT8032UART2();	/* Enable UART2 for T8032 debug purpose */
		
	PDWNC_WRITE32(REG_RW_WAKEN, 0xD00);
	v_pwr_off_engine_set(ENGINE_ALL, ENGINE_MASTER_T8032);
	v_pwr_off_engine_set(ENGINE_ETHE, ENGINE_MASTER_T8032);
	v_pwr_off_engine_set(ENGINE_UART, ENGINE_MASTER_T8032);
	v_pwr_off_engine_set(ENGINE_CEC, ENGINE_MASTER_T8032);

	vPDWNCT8032Reset();

	PDWNC_WRITE32(REG_RW_ARM_IND_INT, RW_ARM_IND_INT_ARM_INT); 
	
	return 0;	
}

int v_ac83xx_standby(void)
{
	
	int g_stdby_checking_reg;
	
	g_stdby_checking_reg = PDWNC_READ32(REG_RW_RESRV1);

#if DEBUG_USE			
	printf("[v_ac83xx_standby]g_stdby_checking_reg : %x\n", g_stdby_checking_reg);
#endif

	if( g_stdby_checking_reg != STDBY_CHECK_VALUE )
	{
		v_pwr_off();
	}
	else{
		return 0;
	}
}

