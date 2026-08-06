#include "x_typedef.h"
#include "x_hal_ic.h"
#include "nfi2.h"
#include "nfi_drv.h"
#include "x_printf.h"
#include "x_util.h"

#define ReadReg32(addr)             (*(volatile UINT32 *)(addr))
#define WriteReg32(addr,data)   ((*(volatile UINT32 *)(addr)) = (UINT32)(data))
#define NFI_CLK_SEL_OFFSET    (0x70)
#define NFI_CLK_SEL_MASK      (0x00070000)
#define NFI_CLK_144           (0x00030000)
#define NUTL_DEFAULT_TIMEOUT    0x0000FFFF

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

extern NFI_DRV_t nand_driver;

/**********************************************************
Description : NFI2_WaitStatus(UINT32 u4Status)
Input       : UINT32 u4Status, status to be waited
Output      : TRUE or FALSE,
***********************************************************/
BOOL NFI2_WaitStatus(UINT32 u4Status)
{
  INT32 i4Timeout = NUTL_DEFAULT_TIMEOUT;

  while ( (*NFI2_STA & u4Status) && i4Timeout--);

  if (i4Timeout>0)
    return TRUE;
  else
    return FALSE;
}

/**********************************************************
Description : NFI_Init
Input       : 0
Output      : 0
***********************************************************/
void NFI2_Init(void)
{
	 
	  
	  UINT32 u4Clk;
	 u4Clk =  (*(volatile UINT32 *)(0Xf0000000 + 0x18));
	  u4Clk &=~(0x7U<<24);
	  u4Clk|=(0x3U<<24);
	  (*(volatile UINT32 *)(0Xf0000000 + 0x18)) = (UINT32)(u4Clk);  


    // NFI access control
    //*NFI2_ACCCON = LCD2NAND(0xF) | PRECS(0xf) | C2R(0x2)|W2R(0x2)|WH(0x3)  |WST(0x4)|RLT(0x4);
    //*NFI2_ACCCON = LCD2NAND(0xF) | PRECS(0xf) | C2R(0x3F)|W2R(0xF)|WH(0xF)  |WST(0xF)|RLT(0xF);
     // *NFI2_ACCCON =LCD2NAND(0x0)|PRECS(0x00)|C2R(0x00)|W2R(0x0)|WH(0x1)|WST(0x3)|RLT(0x5);
    // Reset the state machine and data FIFO, besides flushing FIFO
    *NFI2_CON  = NFI_RST | FIFO_FLUSH;
    *NFI2_ACCCON = 0x135;
    // NFI2_Set_Acccon();
}

/**********************************************************
Description : NFI_Reset
Input       : 0
Output      : 0
***********************************************************/
void NFI2_Reset(void)
{
        UINT32    timeout = NUTL_DEFAULT_TIMEOUT;
//        STATUS_E  ret=S_UNKNOWN_ERR;
        INT32    i4Val;

    
        // reset the NFI core state machine, data FIFO and flushing FIFO
        *NFI2_CON = NFI_RST | FIFO_FLUSH;
        *NFI2_CNFG = OP_RESET;
    
        // enable interrupt
        *NFI2_INTR_EN = RESET_DONE_EN;
    
        // reset cmd
        *NFI2_CMD = NAND_CMD_RESET;
    
        // wait til CMD is completely issued
        while( *NFI2_STA  & STATUS_CMD );
    
        // wait for reset finish
        timeout = NUTL_DEFAULT_TIMEOUT;

        {
            NFI_Wait( !(*NFI2_INTR & RESET_DONE), timeout);
            i4Val = *NFI2_INTR;
#ifdef INT_WR_CLR
            *NFI2_INTR = i4Val;
#endif        
            //UNUSED(i4Val);
            if( 0 == timeout ) {
//                ret = S_TIMEOUT;
                goto end;
            }
        }
    
        //ret = S_DONE;
    
        end:
        // disable interrupt
        *NFI2_INTR_EN = 0;
    
        //return ret;

}

/**********************************************************
Description : NFI_Config
Input       : 0: 512B/page, 1: 2KB/page
Output      : 0
***********************************************************/
void NFI2_Config(NFI_MENU *input)
{
    UINT16  NFIPageFormat;
    //UINT32 u4SpareSize;

    //*NFI_CON = NFI_RST; // reset must be done in the very beginning
    NFI2_Reset();

    if ( input->pageSize == 512 )
        NFIPageFormat = NAND_PSIZE_2K_512;
    else if ( input->pageSize == 1024 )
        NFIPageFormat = NAND_PSIZE_2K_512; 
    else if ( input->pageSize == 2048 )
        NFIPageFormat = NAND_PSIZE_2K_512; 
    else if ( input->pageSize == 4096 )
        NFIPageFormat = NAND_PSIZE_4K_2K;     
    else if ( input->pageSize == 8192 )
        NFIPageFormat = NAND_PSIZE_8K_4K;             
    else
        NFIPageFormat = NAND_PSIZE_2K_512; 

    //u4SpareSize = input->spareSize/(input->pageSize / SECTOR_BYTES);

    switch (input->spareSize)
    {
        case 16:
            NFIPageFormat |= SPARE_16;
            break;
        case 26:
            NFIPageFormat |= SPARE_26;
            break;
        case 27:
            NFIPageFormat|= SPARE_26;
            break;
        case 64:
            NFIPageFormat|= SPARE_64;
            break;
        default:
            NFIPageFormat |= SPARE_26;
            break;
    }

    // if ( input->IOInterface==IO_16BITS )
    //   NFIPageFormat |= NAND_IO_16BITS;

    //  NFIPageFormat |= (0x02 << 4); //ECC decode block size, 512 bytes
    if (input->pageSize == 512)
    {
        NFIPageFormat |= FDM2_ECC_NUM(8);
        NFIPageFormat |= FDM2_NUM(8) ;
    }
    else
    {
        NFIPageFormat |= FDM2_ECC_NUM(FDM2_ECC_BYTES);
        NFIPageFormat |= FDM2_NUM(FDM2_BYTES) ;    
    }
    *(NFI2_PAGEFMT) = NFIPageFormat;

}

//------------------------------------------------------------------------------
// ECC Error Software Correct
//------------------------------------------------------------------------------
STATUS_E NFI2_ErrCorrect(
        const UINT32  c_timeout
        ,const UINT32 u4SectIdx
        ,const UINT32 *p_data32 /* MUST be 32bits alignment addr */
)
{
  UINT32 timeout;
  UINT32 i;
  UINT32 u4ErrNum;
  UINT32 u4ErrVal;
  UINT16 u2ErrLoc;
//  STATUS_E ret = S_UNKNOWN_ERR;
  UINT32 u4ErrLoc;
    UINT32 u4SecSize = (*NFI2_CNFG&SEL_SEC_512BYTE)?512:1024;    

  timeout = c_timeout;
  NFI_Wait( !(*NFI2ECC_DECDONE & (1 << u4SectIdx)), timeout); // wait for all block decode done
  if( 0 == timeout ) {
    return S_TIMEOUT;
  }

    if (u4SectIdx>3)
        u4ErrNum = (*NFI2ECC_DECENUM2 & (0x1F << ((u4SectIdx-4)*8))) >> ((u4SectIdx-4)*8);        
    else
        u4ErrNum = (*NFI2ECC_DECENUM & (0x1F << (u4SectIdx*8))) >> (u4SectIdx*8);
    
  if ( 0x1F == u4ErrNum)
    return S_ECC_UNCORRECT_ERR;
  else if ( 0x0 == u4ErrNum)
    return S_DONE;  
  else 
  {
    for (i = 0 ; i < u4ErrNum ; i++)
    {
      u2ErrLoc = *(UINT16*)((UINT32)NFI2ECC_DECEL0 + i*2);
      if ( (u2ErrLoc >> 3) < u4SecSize)
      {
        u4ErrLoc = (UINT32)p_data32+u4SectIdx*u4SecSize+(u2ErrLoc>>3);
        u4ErrVal = *(UINT8*)u4ErrLoc;
        u4ErrVal = u4ErrVal & (1 << (u2ErrLoc&7));
        if (u4ErrVal)
          *(UINT8*)u4ErrLoc &= (~u4ErrVal);
        else
          *(UINT8*)u4ErrLoc |= (1 << (u2ErrLoc&7));
      }
      else
      {
        u4ErrLoc = (UINT32)NFI2_FDM0L+u4SectIdx*16 + (((u2ErrLoc >> 3) -u4SecSize)>>2)*4;
        u4ErrVal = *(UINT32*)u4ErrLoc;
        u4ErrVal = u4ErrVal & (1 << ((u2ErrLoc - u4SecSize&7)&31));        
        if (u4ErrVal)
          *(UINT32*)u4ErrLoc &= (~u4ErrVal);
        else
          *(UINT32*)u4ErrLoc |= (1 << ((u2ErrLoc - u4SecSize*8)&31));      
      }
    }
    return S_ECC_CORRECTABLE_ERR;
  }
//  return ret;
}




/**********************************************************
Description : NFI_ReadPage
Input       : UINT32 *pu4Buf, UINT32 u4PageSize, UINT32 u4PgIdx, BOOL fgEccCheck
                 pu4Buf is the destination ,
                 u4PageSize , NAND Flash page size
                 u4PgIdx, PageIdx
                 fgEccCheck, 
Output      : 
              1 , successfully
              -1, ECC correct 1 bit error
             -2,  ECC detect 2 error bits
             
             -3,  NFI page read timeout
***********************************************************/
STATUS_E NFI2_ReadPage(UINT32 *pu4Buf, UINT32 * pu4Spare,UINT32 u4PgIdx, BOOL fgEccCheck)
{
    INT32 i/*, iStatus, i4EccBlockNum*/;
    UINT32 timeout;
    STATUS_E ret = S_UNKNOWN_ERR;
    NFI_MENU      *pConfig = nand_driver.pCurConfig;
    UINT32 u4SectorSize = (pConfig->pageSize==512)?512:1024;

    //    ASSERT(pu4Buf!=NULL);
    // Clear IntrStatus by reading.
   // if (u4PgIdx < 1000)
   // Printf("u4PgIdx=%d\n",u4PgIdx);
	//Printf("pu4Buf =%x\n",pu4Buf);
    *NFI2_CON = NFI_RST | FIFO_FLUSH;
    *NFI2_CNFG = OP_READ | READ_MODE | HW_ECC_EN | AUTO_FMT_EN;    
    if (pConfig->pageSize == 512)
        *NFI2_CNFG |= SEL_SEC_512BYTE; 
    *NFI2_CON = SEC_NUM(  uidiv(pConfig->pageSize,u4SectorSize)) | NOB_DWORD;

    if ( pConfig->spareSize >= 26) // 26 bytes spare,  use 2*12 bit ECC decode
        *NFI2ECC_DECCNFG = DEC_EMPTY_EN | DEC2_CS((1024 + FDM2_ECC_BYTES)*8 + ECC2_24_BITS*14) | DEC_CON(ECC2_DEC_LOCATE) | DEC_NFI_MODE | DEC_TNUM(ECC2_24_BITS);
    else if (pConfig ->pageSize == 512) // 512 bytes as sector size, ECC 4 bit
        *NFI2ECC_DECCNFG = DEC_EMPTY_EN | DEC2_CS((512 + 8)*8 + ECC2_4_BITS*14) | DEC_CON(ECC2_DEC_LOCATE) | DEC_NFI_MODE | DEC_TNUM(ECC2_4_BITS);
    else // 1024 bytes as sector size, ecc 2*4 bit
        *NFI2ECC_DECCNFG = DEC_EMPTY_EN | DEC2_CS((1024 + FDM2_ECC_BYTES)*8 + ECC2_12_BITS*14) | DEC_CON(ECC2_DEC_LOCATE) | DEC_NFI_MODE |DEC_TNUM(ECC2_12_BITS);
    *NFI2ECC_DECCON = 0;
    *NFI2ECC_DECCON = DEC_EN;
    if ( AUTO_FMT_EN & (*NFI2_CNFG))
        *NFI2ECC_FDMADDR = (UINT32)NFI2_FDM0L;

    //iStatus = 1;
    //u4Val = *NFI_INTR;
    //    *NFI_INTR_EN = AHB_DONE_EN; 
    //    *NFI_STRADDR = 0xC0400000;
    // Read command. 
    //*NFI_CNFG = NFI_CNFG_AUTOECC_DECODE; // WCP
    *NFI2_CMD = NAND_CMD_READ1_00;

    if (!NFI2_WaitStatus(NAND_STATUS_CMD))
    {
        ret = S_TIMEOUT;
	    goto end; //timeout
    }


    *NFI2_COLADDR = 0;
    *NFI2_ROWADDR = u4PgIdx;
//	Printf("*NFI2_ROWADDR  ==%x \n",*NFI2_ROWADDR );
    *NFI2_ADDRNOB = COL_ADDR_NOB(pConfig->pageShift >> 3) | ROW_ADDR_NOB(pConfig->addressCycle-(pConfig->pageShift >> 3));
    if (!NFI2_WaitStatus(NAND_STATUS_ADDR))
    {
        ret = S_TIMEOUT;
	     goto end; //timeout
    }
    if (pConfig->pageShift > 8) // page size > 512 bytes
    {        
        *NFI2_CMD = NAND_CMD_READ_2K;
        if (!NFI2_WaitStatus(NAND_STATUS_CMD))
        {
            ret = S_TIMEOUT;
			goto end; //timeout
        }
        if (!NFI2_WaitStatus(NAND_STATUS_BUSY))
        {
            ret = S_TIMEOUT;
			goto end; //timeout
        }      
    }

    // AHB mode
   //*NFI_INTR_EN = RD_DONE_EN;  
    *NFI2_CON |=  BURST_RD ; 

   //*NFI_CNFG |= AHB_MODE; 
    for(i=0; i<(pConfig->pageSize >> 2); i++) 
    {
        // wait for data ready 
        // when RD_EMPTY_MASK flag is poll-down, it means data is ready in FIFO at least 4 bytes. 
        timeout = NUTL_DEFAULT_TIMEOUT;
        NFI_Wait( (*NFI2_FIFOSTA & RD_EMPTY), timeout);
        if( 0 == timeout ) 
        { 
            ret = S_TIMEOUT;
			goto end; //timeout
        }
        pu4Buf[i] = *NFI2_DATAR;
        if ( !( ((i+1)*sizeof(UINT32)) & (u4SectorSize - 1)))
        {
			ret = NFI2_ErrCorrect(NUTL_DEFAULT_TIMEOUT, ( uidiv(((i+1)*sizeof(UINT32)),u4SectorSize))-1, pu4Buf); // AUTO_FMT must be enabled
            if ( (S_DONE != ret) && (S_ECC_CORRECTABLE_ERR != ret))
                goto end;
			
        }
    }
	
	// read spare data
	if(NULL!=pu4Spare){
		if ( AUTO_FMT_EN & (*NFI2_CNFG))
		{
	
			P_U32 pFDMAddr = NFI2_FDM0L;
			P_U32 p_spare32 = (P_U32)(pu4Spare);
	
			//for (i = 0; i < (pConfig->pageSize >> 9) ; i++)
			{
				*p_spare32++ = *pFDMAddr++;
				*p_spare32++ = *pFDMAddr++;
			}
		}
	}
   
    ret = S_DONE;
end:    
    *NFI2_CON= 0x0;
    *NFI2ECC_DECCON = 0;
    return ret;
}

static STATUS_E NAND_COMMON_ECCErrDetect(
        NFI_MENU      *pConfig
        ,const UINT32  c_timeout
        ,const UINT32  row_addr
        ,const UINT32 *p_data32 /* MUST be 32bits alignment addr */
)
{
  UINT32 u4BlockNum, page_size,i, u4Tmp, timeout;
  STATUS_E ret = S_DONE;
//  volatile UINT32* p4ErrLoc = *NFIECC_DECEL0;
   UINT32 u4SectorSize = (pConfig->pageSize==512)?512:1024;

  page_size = pConfig->pageSize;
  u4BlockNum =  uidiv(page_size,u4SectorSize);   

  u4Tmp = 0;
  for ( i = 0 ; i < u4BlockNum ; i++)
    u4Tmp |= 1 << i;

  timeout = c_timeout;
  NFI_Wait( (*NFI2ECC_DECDONE != u4Tmp), timeout); // wait for all block decode done
  if( 0 == timeout ) {
    return S_TIMEOUT;
  }

  if ( !*NFI2ECC_DECFER)
    return S_DONE;
  else
  {
    u4Tmp = *NFI2ECC_DECENUM; // error number
    for ( i = 0 ; i < 8 ; i++)
    {
      if ( 0x1F == (u4Tmp & 0x1F))
      {
      	Printf("Page 0x%x Sector %d uncorrect ECC %d\n", row_addr, i, u4Tmp&0x1F);	
        return S_ECC_UNCORRECT_ERR;
      }
      else if ( u4Tmp & 0x1F)
      {
        ret = S_ECC_CORRECTABLE_ERR;
        //Printf("Page 0x%x Sector %d with Error Number %d\n", row_addr, i, u4Tmp&0x1F);
      }

      u4Tmp >>= 4;
    }
  }

  return ret;
}




#define NONCACHE(expression) (expression|0xC0000000)
//#if 0
STATUS_E NFI_ReadPage(UINT32 *pu4Buf, UINT32 u4PgIdx, BOOL fgEccCheck, BOOL bDWRDRead, UINT32 u4Offset, UINT32 u4SubLength)
{
    INT32 i/*, iStatus, i4EccBlockNum*/;
    UINT32 timeout;
    UINT32 u4Temp;
    STATUS_E ret = S_UNKNOWN_ERR;
    NFI_MENU      *pConfig = (NFI_MENU *)&_rNFI2Retrials[_u4Config_no];
    UINT32 u4SectorSize = (pConfig->pageSize==512)?512:1024;

    //Printf("NFI_ReadPage : %x, %x, %x\n", pu4Buf, u4PgIdx, u4Offset);

    // sram 0xf4000000 - 0xf4010000 
    if ((UINT32)pu4Buf >= 0xf4000000 && (UINT32)pu4Buf <= 0xf4010000) {
        return S_UNKNOWN_ERR;
    }

    // ASSERT(pu4Buf!=NULL);
    // Clear IntrStatus by reading. 
    *NFI2_CON = NFI_RST | FIFO_FLUSH;
    *NFI2_CNFG = OP_READ | READ_MODE | HW_ECC_EN | AUTO_FMT_EN;    
    if (pConfig->pageSize == 512)
        *NFI2_CNFG |= SEL_SEC_512BYTE;
    *NFI2_CON = SEC_NUM( uidiv(pConfig->pageSize,u4SectorSize)) | NOB_DWORD; 

    if ( pConfig->spareSize >= 26) // 26 bytes spare,  use 2*12 bit ECC decode
        *NFI2ECC_DECCNFG = DEC_EMPTY_EN | DEC2_CS((1024 + FDM2_ECC_BYTES)*8 + ECC2_24_BITS*14) | DEC_CON(ECC2_DEC_CORRECT) | DEC_NFI_MODE | DEC_TNUM(ECC2_24_BITS);
    else if (pConfig ->pageSize == 512) // 512 bytes as sector size, ECC 4 bit
        *NFI2ECC_DECCNFG = DEC_EMPTY_EN | DEC2_CS((512 + 8)*8 + ECC2_4_BITS*14) | DEC_CON(ECC2_DEC_CORRECT) | DEC_NFI_MODE | DEC_TNUM(ECC2_4_BITS);
    else // 1024 bytes as sector size, ecc 2*4 bit
        *NFI2ECC_DECCNFG = DEC_EMPTY_EN | DEC2_CS((1024 + FDM2_ECC_BYTES)*8 + ECC2_12_BITS*14) | DEC_CON(ECC2_DEC_CORRECT) | DEC_NFI_MODE |DEC_TNUM(ECC2_12_BITS);
    *NFI2ECC_DECCON = 0;
    *NFI2ECC_DECCON = DEC_EN;
    if ( AUTO_FMT_EN & (*NFI2_CNFG))
        *NFI2ECC_FDMADDR = (UINT32)NFI2_FDM0L;
    
    if (bDWRDRead == FALSE)
    {
    i = *NFI2_INTR;
    *NFI2_INTR_EN = AHB_DONE_EN; 
    // AHB mode
    *NFI2_CNFG |= AHB_MODE;    
    *NFI2_STRADDR = NONCACHE((UINT32)pu4Buf);
    }
    // Read command. 
    //*NFI_CNFG = NFI_CNFG_AUTOECC_DECODE; // WCP
    *NFI2_CMD = NAND_CMD_READ1_00;

    if (!NFI2_WaitStatus(NAND_STATUS_CMD))
    {
      ret = S_TIMEOUT;
      goto end; //timeout
    }


    *NFI2_COLADDR = 0;
    *NFI2_ROWADDR = u4PgIdx;
    *NFI2_ADDRNOB = COL_ADDR_NOB(pConfig->pageShift >> 3) | ROW_ADDR_NOB(pConfig->addressCycle-(pConfig->pageShift >> 3));
    if (!NFI2_WaitStatus(NAND_STATUS_ADDR))
    {
      ret = S_TIMEOUT;
      goto end; //timeout
    }
    if (pConfig->pageShift > 8) // page size > 512 bytes
    {        
      *NFI2_CMD = NAND_CMD_READ_2K;
      if (!NFI2_WaitStatus(NAND_STATUS_CMD))
      {
        ret = S_TIMEOUT;
        goto end; //timeout
      }
      if (!NFI2_WaitStatus(NAND_STATUS_BUSY))
      {
        ret = S_TIMEOUT;
        goto end; //timeout
      }      
    }

    *NFI2_CON |=  BURST_RD ;
    timeout = NUTL_DEFAULT_TIMEOUT;
    if (bDWRDRead == FALSE)
    {
    NFI_Wait( (AHB_DONE != (*NFI2_INTR&AHB_DONE)), timeout);
    *NFI2_INTR_EN &= ~AHB_DONE_EN; // disable INT first
    i = *NFI2_INTR;//read clear again
    UNUSED(i);
    if( 0 == timeout) {
      ret = S_TIMEOUT;
      goto end;
    }
    }
    else
    {
      UINT32 j = 0;
	    for(i=0; i<(pConfig->pageSize>>2); i++) 
	    {
	      // wait for data ready 
	      // when RD_EMPTY_MASK flag is poll-down, it means data is ready in FIFO at least 4 bytes. 
	      timeout = NUTL_DEFAULT_TIMEOUT;
	      NFI_Wait( (*NFI2_FIFOSTA & RD_EMPTY), timeout);
	      if( 0 == timeout ) {
	        {
	          ret = S_TIMEOUT;
	          goto end; //timeout
	        }
	      }
	        u4Temp = *NFI2_DATAR;
	        if ( (i*4 >= u4Offset) && (i*4 < u4Offset+u4SubLength))
	          pu4Buf[j++] = u4Temp;
		      if ( !( ((i+1)*4) & (u4SectorSize - 1)))
		      {
		          ret = NFI2_ErrCorrect(NUTL_DEFAULT_TIMEOUT, ( uidiv(((i+1)*sizeof(UINT32)),u4SectorSize))-1, pu4Buf); // AUTO_FMT must be enabled
		          if ( (S_DONE != ret) && (S_ECC_CORRECTABLE_ERR != ret))
		            goto end;
		      }
	    	}
	    }
	   	// read spare data
	   	/*
		  if ( AUTO_FMT_EN & (*NFI_CNFG))
		  {
		    volatile UINT32* pFDMAddr = NFI_FDM0L;
		    UINT32* p_spare32 = (UINT32*)(pu4Buf + (pConfig->pageSize>>2));
		    
		    for (i = 0; i < (pConfig->pageSize >> 9) ; i++)
		    {
		      *p_spare32++ = *pFDMAddr++;
		      *p_spare32++ = *pFDMAddr++;
		    }
		  }*/
		  timeout = NUTL_DEFAULT_TIMEOUT;
		  if ( *NFI2_CNFG & HW_ECC_EN)
		    ret = NAND_COMMON_ECCErrDetect(pConfig, timeout, u4PgIdx, pu4Buf);    
		
		  timeout = NUTL_DEFAULT_TIMEOUT; 
		  NFI_Wait( ((*NFI2_ADDRCNTR & 0xF000) >> 12) != uidiv(pConfig->pageSize,u4SectorSize) , timeout);
		  if( 0 == timeout ) {
		      ret = S_TIMEOUT;
		      goto end;
		  }
		  for (i = 0 ; i < uidiv(pConfig->pageSize,u4SectorSize); i++)
		  {
		    volatile UINT64* pFDMAddr = (UINT64*)NFI2_FDM0L + i*16;
		
		    if (pFDMAddr[i] != 0x0000000000000000ULL)
		    {
		      break;
		    }
		  }
	
		  if ( i >= uidiv(pConfig->pageSize,u4SectorSize))
		    ret = S_ECC_UNCORRECT_ERR;
    
		  if (S_ECC_CORRECTABLE_ERR == ret)
    		ret = S_DONE;
//	  while ( AHB_DONE != (*NFI_INTR & AHB_DONE));
//  	 if (!NFI_WaitStatus(NAND_STATUS_DTRD))
//    	   return -3; //timeout
//  	*NFI_INTR_EN &= ~AHB_DONE_EN;
//    	while ( !(0xFF == *NFIECC_DECDONE));
//			while (1);
end:    
    //Printf("ret ==%d \n",ret);

    *NFI2_CON= 0x0;
    *NFI2ECC_DECCON = 0;

    //Printf("NFI_ReadPage : %x, %x, %x, val = %x\n", pu4Buf, u4PgIdx, u4Offset, *pu4Buf);
    return ret;
}

//#endif
#define NFI2_Wait(condition_expression, timeout)		while( (condition_expression) && (--timeout) )

STATUS_E NFI2_EraseBlock(UINT32 u4BlockIdx, UINT32 pagesPerBlock)
{
    NFI_MENU *pConfig = nand_driver.pCurConfig;
    UINT32 row_addr;
    UINT32  addr_cycle, column_addr_bytes, row_addr_bytes;
    UINT32 timeout;

    row_addr = u4BlockIdx * pagesPerBlock;

    if (pConfig->pageSize > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    *NFI2_CON = NFI_RST | FIFO_FLUSH;
    if (!NFI2_WaitStatus(NAND_STATUS_BUSY))
        return S_TIMEOUT;

    *NFI2_CNFG = OP_ERASE;
    *NFI2_INTR_EN = ERASE_DONE_EN;
    *NFI2_CSEL = 0;
    /* erase cmd */
    *NFI2_CMD = NAND_CMD_ERASE1_BLK;   // 0x60

    if (!NFI2_WaitStatus(NAND_STATUS_CMD))
        return S_TIMEOUT;

    /* row addr */
    *NFI2_COLADDR = 0;
    *NFI2_ROWADDR = row_addr;
    *NFI2_ADDRNOB = ROW_ADDR_NOB(row_addr_bytes);

    if (!NFI2_WaitStatus(NAND_STATUS_ADDR))
        return S_TIMEOUT;

    /* confirm */
    *NFI2_CMD = NAND_CMD_ERASE2_BLK;   // 0xD0
    if (!NFI2_WaitStatus(NAND_STATUS_CMD))
        return S_TIMEOUT;

    timeout = NUTL_DEFAULT_TIMEOUT;

    NFI2_Wait( (ERASE_DONE != (*NFI2_INTR&ERASE_DONE)), timeout); //clear INT status
    *NFI2_INTR_EN &= ~ERASE_DONE_EN; // disable INT first
    if( 0 == timeout) {
          return S_TIMEOUT;
    }

    *NFI2_CON = 0;
    *NFI2ECC_ENCCON = 0x0;

    return S_DONE;
}

STATUS_E NFI2_WritePage(UINT32 *pu4Buf,
                        UINT32 *pu4Spare,
                        UINT32 u4PageIdx)
{
    NFI_MENU *pConfig = nand_driver.pCurConfig;
    UINT32 timeout;
    UINT32 u4SectorSize = (pConfig->pageSize==512) ? 512: 1024;
    UINT32 column_addr, row_addr;
    UINT32 addr_cycle;
	UINT32 i = 0;
    UINT32 sec_num = 0;

    if (pConfig->pageSize > 512)
    {
        addr_cycle = 5;
        column_addr = 2;
    }
    else
    {
		addr_cycle = 3;
        column_addr = 1;
    }
    row_addr = addr_cycle - column_addr;
    sec_num = pConfig->pageSize / u4SectorSize;
    timeout = NUTL_DEFAULT_TIMEOUT;

	*NFI2_CON = NFI_RST | FIFO_FLUSH;
    if (!NFI2_WaitStatus(NAND_STATUS_BUSY))
        return S_TIMEOUT;
    *NFI2_CNFG = OP_PROGRAM | HW_ECC_EN | AUTO_FMT_EN;

    if (pConfig->spareSize >= 26) // 26 bytes spare,  use 2*12 bit ECC decode
        *NFI2ECC_ENCCNFG = ENC_TNUM(ECC2_24_BITS) | ENC2_MS((1024 + FDM2_ECC_BYTES)*8) | ENC_NFI_MODE;
    else if (pConfig ->pageSize == 512) // 512 bytes as sector size, ECC 4 bit
        *NFI2ECC_ENCCNFG = ENC_TNUM(ECC2_4_BITS) | ENC2_MS((1024 + FDM2_ECC_BYTES)*8) | ENC_NFI_MODE;
    else // 1024 bytes as sector size, ecc 2*4 bit
        *NFI2ECC_ENCCNFG = ENC_TNUM(ECC2_12_BITS) | ENC2_MS((1024 + FDM2_ECC_BYTES)*8) | ENC_NFI_MODE;

    *NFI2ECC_ENCCON = 0;
    *NFI2ECC_ENCCON = ENC_EN;
    if (pConfig->pageSize == 512)
        *NFI2_CNFG |= SEL_SEC_512BYTE;

    *NFI2_CON = SEC_NUM(sec_num);

    /* CMD 0x80 */
    *NFI2_CMD = NAND_CMD_INPUT_PAGE;
    if (!NFI2_WaitStatus(NAND_STATUS_CMD))
        return S_TIMEOUT;

    /* addr */
    *NFI2_COLADDR = 0;
    *NFI2_ROWADDR = u4PageIdx;
    *NFI2_ADDRNOB =
        COL_ADDR_NOB(column_addr) | ROW_ADDR_NOB(row_addr);

    if (!NFI2_WaitStatus(NAND_STATUS_ADDR))
        return S_TIMEOUT;

    // prepare FDM data
    if (AUTO_FMT_EN & (*NFI2_CNFG))
    {
        UINT32 *pFDMAddr = (UINT32 *)NFI2_FDM0L;
        for (i = 0; i < (pConfig->pageSize/u4SectorSize); i++)
        {
            *pFDMAddr++ = 0xffffffff;
            *pFDMAddr++ = 0xffffffff;
            *pFDMAddr++ = 0xffffffff;
            *pFDMAddr++ = 0xffffffff;
        }
    }

    *NFI2_CON |= BURST_WR;

    // program page data
    for(i=0; i<pConfig->pageSize; i+=4, pu4Buf++)
    {
        // wait for FIFO has space to enqueue
        // when WR_FULL_MASK flag is poll-down, it means there are at least 4 bytes free space in FIFO.
        NFI_Wait((*NFI2_FIFOSTA & WR_FULL), timeout);
        if( 0 == timeout )
        {
            Printf("[MTD][%s: %u]polling mode timeout\r\n", __FUNCTION__, __LINE__);
            return S_TIMEOUT;
        }
        *NFI2_DATAW = *pu4Buf;
    }

    NFI2_Wait(((*NFI2_ADDRCNTR & 0xF000) >> 12) != (pConfig->pageSize/u4SectorSize) , timeout);
    if( 0 == timeout ) {
        Printf("[MTD][%s: %u]timeout\r\n", __FUNCTION__, __LINE__);
        return S_TIMEOUT;
        //goto end;
    }

    *NFI2_INTR_EN |= WR_DONE_EN;
    /* CMD 0x10 */
    *NFI2_CMD = NAND_CMD_PROG_PAGE;
    if (!NFI2_WaitStatus(NAND_STATUS_CMD))
        return S_TIMEOUT;

	NFI2_Wait( (WR_DONE != (*NFI2_INTR&WR_DONE)), timeout);
    *NFI2_INTR_EN &= ~WR_DONE;
    if( 0 == timeout) {
        Printf("[MTD][%s: %u]timeout\r\n", __FUNCTION__, __LINE__);
        return S_TIMEOUT;
        //goto end;
    }

    *NFI2ECC_ENCCON = 0;
    *NFI2_CON = 0;

    return S_DONE;
}