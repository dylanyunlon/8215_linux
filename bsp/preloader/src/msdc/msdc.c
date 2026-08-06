
#include "x_typedef.h"
#include "msdc.h"
#include "Timer.h"
#include "../drv_cust/ac8317_m1v1_v00.h"

#define MSDC_AUTOCMD12_EN			(1)		// Enable Auto command 12 -- STOP Command
#define EdbgOutputDebugString(a,...)

static MSDC_CARD_T MSDC_Card[3];
static UINT32 MSDC_DelayCounter;

UINT32 MSDC_Send_Cmd(UINT32 ch,UINT32 cmd,UINT32 arg);
UINT32 MSDC_WaitAndProcess_Response(UINT32 ch,UINT32 cmd,UINT32 timeout);
UINT32 MSDC_GetPortIndex(UINT32 ch);
UINT32 MSDC_SetEXTCSD(UINT32 ch, UINT32 index, UINT32 value, UINT32 mask);
UINT32 MSDC_SetEXTCSD_Ex(UINT32 ch, UINT32 index, UINT32 value, UINT32 mask);



#define INIT_BUS_FREQ					(400 * 1000)			// 400KHz
#define SRC_CLK_FREQ					(27 * 1000 * 1000)    	// OSC = 27 Mhz
#define BUS_CLK_FREQ					(27 * 1000 * 1000)    	// 13.5 Mhz, maximum clock setting is 27/2
#define LOWEST_BUS_FREQ					(SRC_CLK_FREQ / (4 * 255))

#ifdef ENABLE_DUMP_CARD_TYPE
UINT32 Dump_Card_Type(UINT32 ch, UINT32 pos)
{
	Printf("--> SD%x: CardType = %x at Pos(%x) <--\r\n", MSDC_CH_INDEX(ch), MSDC_Card[MSDC_CH_INDEX(ch)].cardType, pos);
	return 0;
}
#endif

UINT32 MSDC_Is_eMMC_Card(UINT32 ch)
{
	if (MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_MMC_DV_EMMC)
		return 1;
	else 
		return 0;
}

UINT32  MSDC_EnterHighSpeedMode(UINT32 ch)
{
	UINT32 ret;
	ret = MSDC_SetEXTCSD_Ex(ch, 185, 0x1, 0x0F);
	if (ret != MSDC_LASTERROR_SETEXTCSD | MSDC_LASTERROR_OK)
	{
		Printf("Enter High Speed Mode Failed\n");
		return FALSE;
	}
	
	return TRUE;
}

UINT32  MSDC_SetBusWidth(UINT32 ch, UINT32 u4BusWidth)
{
   	UINT32 ret;
	UINT32 uBusWidth_Param = 0;
    EdbgOutputDebugString("MSDC_SetBusWidth\r\n");

	// Send command for device card
	if (MSDC_Is_eMMC_Card(ch))
	{
		//Printf("emmc set 4 bit bus-width ");
		if (u4BusWidth == 1)
		{
			uBusWidth_Param = EXT_CSD_183_BUS_WIDTH_1;
		}
		else if (u4BusWidth == 4)
		{
			uBusWidth_Param = EXT_CSD_183_BUS_WIDTH_4;
		}
		else if (u4BusWidth == 8)
		{
			uBusWidth_Param = EXT_CSD_183_BUS_WIDTH_8;
		}
		
		ret = MSDC_SetEXTCSD_Ex(ch, 183, uBusWidth_Param, EXT_CSD_183_BUS_WIDTH_MASK);
		if (ret != MSDC_LASTERROR_SETEXTCSD | MSDC_LASTERROR_OK)
		{
			Printf("Set bus width Failed\n");
			return FALSE;
		}

		//Printf("---> eMMC set %xbit buswidth <---\r\n", u4BusWidth);
	}
	else  // SD Card
	{
		//Printf("---> SD set %xbit buswidth <---\r\n", u4BusWidth);
	   	ret = MSDC_Send_Cmd(ch, COM_CMD55_APP_CMD, (MSDC_Card[MSDC_CH_INDEX(ch)].cardRCA << 16));
		if(ret != MSDC_CMD_OK)
		{
			Printf("---> SD set %xbit buswidth Failed: %x <---\r\n", u4BusWidth, ret);
		    return  FALSE;
		}

		if (u4BusWidth == 1)
		{
			uBusWidth_Param = 0x00;
		}
		else if (u4BusWidth == 4)
		{
			uBusWidth_Param = 0x02;
		}
	   
	   	ret = MSDC_Send_Cmd(ch, COM_ACMD6_SET_BUS_WIDTH, uBusWidth_Param);
	   	if(ret != MSDC_CMD_OK)
		{			   
		    return  FALSE;
		}
	}
   	// Change host controller config
   	if (u4BusWidth == 1)
   	{
   		MSDC_SETBIT(SDC_CFG(ch), 0x00 << SDC_CFG_BW_SHIFT);
   	}
	else if (u4BusWidth == 4)
   	{
   		MSDC_SETBIT(SDC_CFG(ch), 0x01 << SDC_CFG_BW_SHIFT);
   	}
	else if (u4BusWidth == 8)
   	{
   		MSDC_SETBIT(SDC_CFG(ch), 0x02 << SDC_CFG_BW_SHIFT);
   	}
   	return TRUE;
}

UINT32  MSDC_SetClockRate(UINT32 ch,UINT32 dwClockRate)
{
	unsigned short divider;

	unsigned int expFreq = 0;


	// set clock continuous for test
	MSDC_SETBIT(MSDC_CFG(ch), (((UINT32)0x01) << 1));

	if ((dwClockRate > BUS_CLK_FREQ)) {

		dwClockRate = BUS_CLK_FREQ;
	}
	else if ((dwClockRate < LOWEST_BUS_FREQ)) {

		dwClockRate = INIT_BUS_FREQ;
	}

	if (dwClockRate >= SRC_CLK_FREQ)		// 27MHz
	{
		// Using clock no-divider mode
		MSDC_CLRBIT(MSDC_CFG(ch), 0x0003FF00);
		MSDC_SETBIT(MSDC_CFG(ch), 0x00010000);
		while( 0 != (MSDC_READ32(MSDC_CFG(ch)) & MSDC_CFG_CARD_CK_STABLE) );
				
		return 1;
	}
	else
	{
		// Using clock divider mode
		MSDC_CLRBIT(MSDC_CFG(ch), 0x0003FF00);

		for (divider = 0; divider <= 0xFF; divider++) {

			if (0 == divider) {

				expFreq = SRC_CLK_FREQ / 2;
			}

			else {

				//expFreq = SRC_CLK_FREQ / 4 / divider;
				expFreq = uidiv(SRC_CLK_FREQ >> 2 , divider);
			}

			if (expFreq <= dwClockRate) {			

				MSDC_SETBIT(MSDC_CFG(ch), divider << MSDC_CFG_CK_DIV_SHIFT);

				// Wait until clock is stable

				while( 0 != (MSDC_READ32(MSDC_CFG(ch)) & MSDC_CFG_CARD_CK_STABLE) );
				
				return 1;
			}
		}

	}

	return 0;

}
static UINT32 WaitCardNotBusy(UINT32 ch)
{

	UINT32 u4SDStatus,i = 100000;
	 
	do{

		u4SDStatus =  MSDC_READ32(SDC_STS(ch));
		
        if(i == 0)
			return 0;
		i--;
		
	}while(u4SDStatus & (SDC_STS_SDCBUSY | SDC_STS_CMDBUSY));
	 

	return 1;
}

void MSDC_Init(UINT32 ch)
{
	// Reset MSDC
	MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_UNKNOW;
	
	MSDC_SETBIT(MSDC_CFG(ch), MSDC_CFG_RST);
	while (0 != (MSDC_READ32(MSDC_CFG(ch)) & MSDC_CFG_RST));
	// set clock mode
	MSDC_MASK32(MSDC_CFG(ch), MSDC_CFG_CK_MODE_DIVIDER, MSDC_CFG_CK_MODE_MASK);
	while (0 == (MSDC_READ32(MSDC_CFG(ch)) & MSDC_CFG_CARD_CK_STABLE));

	MSDC_MASK32(MSDC_CFG(ch), (0x11 << MSDC_CFG_CK_DIV_SHIFT), MSDC_CFG_CK_DIV_MASK);//17 for 27M MSDC_SRC_CK
	while (0 == (MSDC_READ32(MSDC_CFG(ch)) & MSDC_CFG_CARD_CK_STABLE));
    
	// Set SD/MMC Mode	
	MSDC_SETBIT(MSDC_CFG(ch), MSDC_CFG_SD);

	// Set default RISC_SIZE for DWRD pio mode
	MSDC_SETBIT(SDC_CFG(ch), SDC_DTOC);
	// Disable sdio
	MSDC_CLRBIT(SDC_CFG(ch), SDC_SDIO_EN);
	// Set bus to 1 bit mode
	MSDC_CLRBIT(SDC_CFG(ch), SDC_BUSWIDTH);
	// Set data timeout counter
	MSDC_SETBIT(SDC_CFG(ch), SDC_DTOC);
	// Clear Interrupt status
	MSDC_WRITE32(MSDC_INT(ch), 0x0001F7FB);
    
	// Set Access Port Size, 4 Bytes
    MSDC_MASK32(MSDC_IOCON(ch), MSDC_IOCON_RISC_SIZE_DWRD,MSDC_IOCON_RISC_SIZE_MASK);
	// Set R1B Busy delay
	MSDC_SETBIT(PATCH_BIT(ch), R1B_DELAY_CYCLE);

	// TODO SMT enable
	
}


UINT32 MSDC_Send_Cmd(UINT32 ch,UINT32 cmd,UINT32 arg)
{
	UINT32 ret;
	UINT32 sdcCmd = 0;
	UINT32 cmdTimeout;
	MSDC_DELAY(0x10000);

	cmdTimeout = 0x10000000;
    sdcCmd |= GET_CMD_TYPE(cmd);
    ret = 0;
    switch(cmd)
    {
        case MMC_CMD3_SET_RELATIVE_ADDR:
            MSDC_Card[MSDC_CH_INDEX(ch)].cardRCA = (arg >> 16);
        	break;
        case COM_CMD18_READ_MULTIPLE_BLOCK:
            sdcCmd |= (MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen << SDC_CMD_LEN_SHIFT);
            sdcCmd |= SDC_CMD_READ;
            sdcCmd |= DTYPE_MULTI_BLK;
			#if MSDC_AUTOCMD12_EN
			// Enable Auto command 12
			sdcCmd |= (1 << 28);
			#endif
        	break;
		case COM_CMD25_WRITE_MULTIPLE_BLOCK:
            sdcCmd |= (MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen << SDC_CMD_LEN_SHIFT);
            sdcCmd |= SDC_CMD_WRITE;
            sdcCmd |= DTYPE_MULTI_BLK;
			#if MSDC_AUTOCMD12_EN
			// Enable Auto command 12
			sdcCmd |= (1 << 28);
			#endif
			break;
        case COM_CMD12_STOP_TRANSMISSION:
            sdcCmd |= SDC_CMD_STOP;
        	break;
        case COM_CMD16_SET_BLOCKLEN:
            if(MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_SD20LATER_SDHC
                && arg != MSDC_CARD_SDHC_FIX_BLOCK_LEN)
            {
                ret = MSDC_CMD_ERROR;
            }   
            else if(MSDC_Card[MSDC_CH_INDEX(ch)].cardMaxBlockLen < arg)
            {
                ret = MSDC_CMD_ERROR;
            }
            else
            {
                MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen = arg;
                
            }
        	break;
    }

    if(ret != 0)
    {
        return ret;
    }
    
	switch(GET_RESPONSE_TYPE(cmd))
	{
        case CMD_RSPTYPE_NO:
            sdcCmd |= SDC_CMD_RSPTYPE_NO;
        	break;

		case CMD_RSPTYPE_R2:
            sdcCmd |= SDC_CMD_RSPTYPE_R2;
        	break;

		case CMD_RSPTYPE_R3:
            sdcCmd |= SDC_CMD_RSPTYPE_R3;
        	break;

		case CMD_RSPTYPE_R4:
            sdcCmd |= SDC_CMD_RSPTYPE_R4;
        	break;

		case CMD_RSPTYPE_R1:
        case CMD_RSPTYPE_R5:
        case CMD_RSPTYPE_R6:
		case CMD_RSPTYPE_R7:
			sdcCmd |= SDC_CMD_RSPTYPE_NO;
			sdcCmd |= SDC_CMD_RSPTYPE_R1R5R6R7;
			break;

		case CMD_RSPTYPE_R1B:
			sdcCmd |= SDC_CMD_RSPTYPE_R1B;
			break;
			
		default:
		break;
	}
	EdbgOutputDebugString("cmd=0x%X, arg=0x%x\n", sdcCmd, arg);
	//Printf("cmd=0x%X, arg=0x%x\n", sdcCmd, arg);
	
	WaitCardNotBusy(ch);

    // Set SDC Argument
    MSDC_WRITE32(SDC_ARG(ch), arg);

    /* Send the commands to the device */
    MSDC_WRITE32(SDC_CMD(ch), sdcCmd);

	ret = MSDC_WaitAndProcess_Response(ch,cmd,cmdTimeout);
	
	return ret;
}

UINT32 MSDC_ProcResponseNo(UINT32 ch,UINT32 cmd,UINT32 MSDCInt)
{
    UINT32 ret;


    ret = 0;
    switch(GET_CMD_TYPE(cmd))
    {
        case CMD0_GO_IDLE_STATE:
            if(MSDCInt & INT_SD_CMDRDY)
			{
                MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDRDY);
				ret =  MSDC_CMD_OK;
			}
		break;
    }

    return ret;
}


UINT32 MSDC_ProcResponse1(UINT32 ch,UINT32 cmd,UINT32 MSDCInt)
{
    UINT32 ret;
    UINT32 u4Response[1];
	u4Response[0] = 0;
    ret = 0;
    
    switch(GET_CMD_TYPE(cmd))
    {
        case CMD3_SEND_RELATIVE_ADDR:
        case CMD55_APP_CMD:
        case CMD7_SELECT_CARD:
        case CMD18_READ_MULTIPLE_BLOCK:
		case CMD25_WRITE_MULTIPLE_BLOCK:
        case CMD12_STOP_TRANSMISSION:
        case CMD16_SET_BLOCKLEN:
        case CMD8_SEND_EXT_CSD:
		case CMD6_MMC_SET_BUS_WIDTH:
			
            if(MSDCInt & INT_SD_CMDRDY)
			{
                MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDRDY);
                u4Response[0] = MSDC_READ32(SDC_RESP0(ch));
                MSDC_Card[MSDC_CH_INDEX(ch)].cardStatus = u4Response[0];
				ret = MSDC_CMD_OK;
			}
        break;
               
    }

    return ret;
}



UINT32 MSDC_ProcResponse2(UINT32 ch,UINT32 cmd,UINT32 MSDCInt)
{
    UINT32 ret;
    UINT32 u4Response[4];
    UINT32 u4BlockNR,u4Mult,u4BlockLen;
    UINT64 u8TotalSize;
	u4Response[0] = 0;
    u4Response[1] = 0;
    u4Response[2] = 0;
    u4Response[3] = 0;

	//Dump_Card_Type(ch, 2);
    
    ret = 0;
    switch(GET_CMD_TYPE(cmd))
    {
        case CMD2_ALL_SEND_CID:
		    if(MSDCInt & INT_SD_CMDRDY)
			{
                MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDRDY);
			    u4Response[0] = MSDC_READ32(SDC_RESP0(ch));
				ret = MSDC_CMD_OK;
			}
		break;
        case CMD9_SEND_CSD:
            if(MSDCInt & INT_SD_CMDRDY)
			{
                MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDRDY);
			    u4Response[0] = MSDC_READ32(SDC_RESP0(ch));
                u4Response[1] = MSDC_READ32(SDC_RESP1(ch));
                u4Response[2] = MSDC_READ32(SDC_RESP2(ch));
                u4Response[3] = MSDC_READ32(SDC_RESP3(ch));
                if(((u4Response[3] >> 30)&0x3)==0x01 && MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_SD20LATER_SDHC)
                {
                    
                    u4BlockLen = 512;
                    u4Mult = (1 << 10);
                    u4BlockNR = ((u4Response[1] >> 16)&0xFFFF)+((u4Response[2]&0x3F) << 16)+1;
                    
                }
                else
                {
                    u4BlockNR = ((u4Response[1] >> 30)&0x3)+((u4Response[2]&0x3FF) << 2)+1;
                    u4Mult = (1 <<(((u4Response[1] >> 15)&0x7)+2));
                    u4BlockLen = (1 << (((u4Response[2] >> 16)&0xF)));
                    
                }
                u8TotalSize = (UINT64)u4BlockLen;
                u8TotalSize *= u4Mult;
                u8TotalSize *= u4BlockNR;
                u8TotalSize /= (1 << 20);
                MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen = MSDC_CARD_DEFAULT_BLOCK_LEN;
                MSDC_Card[MSDC_CH_INDEX(ch)].cardMaxBlockLen = u4BlockLen;
                MSDC_Card[MSDC_CH_INDEX(ch)].cardDeviceSize = (UINT32)u8TotalSize;

                MSDC_Card[MSDC_CH_INDEX(ch)].cardMaxSpeed = (u4Response[3] & 0xFF);
				ret = MSDC_CMD_OK;
			}
		break;
    }

    return ret;
}


UINT32 MSDC_ProcResponse3(UINT32 ch,UINT32 cmd,UINT32 MSDCInt)
{
    UINT32 ret;
    UINT32 u4Response[1];
	u4Response[0] = 0;

    ret = 0;
    switch(GET_CMD_TYPE(cmd))
    {
        case ACMD41_SD_SEND_OP_COND:
            if(MSDCInt & INT_SD_CMDRDY)
			{
                 MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDRDY);
				u4Response[0] = MSDC_READ32(SDC_RESP0(ch));
				MSDC_Card[MSDC_CH_INDEX(ch)].cardOCR = u4Response[0] & ACMD41_R3_OCR_MASK;
				if(u4Response[0] & ACMD41_R3_NOTBUSY)
				{
					//MSDC_Card[MSDC_CH_INDEX(ch)].cardStatus &= ~CARD_STATUS_BUSY;
				}
				else
				{
					
                    //MSDC_Card[MSDC_CH_INDEX(ch)].cardStatus |= CARD_STATUS_BUSY;
					ret = MSDC_CMD_BUSY;
					break;
				}

				if(MSDC_Card[MSDC_CH_INDEX(ch)].cardType != MSDC_CARDTYPE_SD20LATER)
				{
					Printf("--> CT1: %x <--\r\n", MSDC_Card[MSDC_CH_INDEX(ch)].cardType);
					MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_SD1X_SDSC;
				}						
				else if(u4Response[0] & ACMD41_R3_SDHC)
				{
					MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_SD20LATER_SDHC;
                    MSDC_Card[MSDC_CH_INDEX(ch)].cardOps |= CARD_OPS_SECTOR_ADDRESS;
				}
				else
				{
			
					MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_SD20LATER_SDSC;
				}
				ret = MSDC_CMD_OK;
			}
		break;
        case CMD1_MMC_SEND_OP_COND:
            if(MSDCInt & INT_SD_CMDRDY)
			{
                MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDRDY);
				u4Response[0] = MSDC_READ32(SDC_RESP0(ch));
                if(u4Response[0] & MMC_OCR_NOTBUSY)
				{
				//	MSDC_Card[MSDC_CH_INDEX(ch)].cardStatus &= ~CARD_STATUS_BUSY;
				}
				else
				{
					
                 //   MSDC_Card[MSDC_CH_INDEX(ch)].cardStatus |= CARD_STATUS_BUSY;
					ret = MSDC_CMD_BUSY;
					break;
				}
                if(u4Response[0] & MMC_OCR_1V7_1V95)
                {
                    MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_MMC_DV_EMMC;
                }
                else
                {
                    MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_MMC_HV;
                }

                if(u4Response[0] & MMC_OCR_SECTOR_MODE)
                {
                    MSDC_Card[MSDC_CH_INDEX(ch)].cardOps |= CARD_OPS_SECTOR_ADDRESS;
                }
				MSDC_Card[MSDC_CH_INDEX(ch)].cardOCR = u4Response[0];
                ret = MSDC_CMD_OK;
				
			}
        break;
    }

    return ret;
}


UINT32 MSDC_ProcResponse6(UINT32 ch,UINT32 cmd,UINT32 MSDCInt)
{
    UINT32 ret;
    UINT32 u4Response[1];
	u4Response[0] = 0;

    ret = 0;
    switch(GET_CMD_TYPE(cmd))
    {
        case CMD3_SEND_RELATIVE_ADDR:
		    if(MSDCInt & INT_SD_CMDRDY)
			{
                MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDRDY);
				u4Response[0] = MSDC_READ32(SDC_RESP0(ch));
				MSDC_Card[MSDC_CH_INDEX(ch)].cardRCA = (u4Response[0] >> 16)&0xFFFF;
                //MSDC_Card[MSDC_CH_INDEX(ch)].cardStatus &= ~CARD_STATUS_CARDRETURNSTATUS_MASK;
                //MSDC_Card[MSDC_CH_INDEX(ch)].cardStatus |= u4Response[0]&0xFFFF;
				ret = MSDC_CMD_OK;
			}
		break;
    }

    return ret;
}


UINT32 MSDC_ProcResponse7(UINT32 ch,UINT32 cmd,UINT32 MSDCInt)
{
    UINT32 ret,u4Arg;
    UINT32 u4Response[4];
    u4Arg = MSDC_READ32(SDC_ARG(ch));
	u4Response[0] = 0;
    u4Response[1] = 0;
    u4Response[2] = 0;
    u4Response[3] = 0;
    ret = 0;
    switch(GET_CMD_TYPE(cmd))
    {
        case CMD8_SEND_IF_COND:
            if(MSDCInt & INT_SD_CMDRDY)
            {
                MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDRDY);
                u4Response[0] = MSDC_READ32(SDC_RESP0(ch));
                if((u4Response[0] & 0xFFF) == u4Arg)
                {
                    MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_SD20LATER;
                    ret =  MSDC_CMD_OK;
                }
                else
                {
                    MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_UNSTABLE;
                    ret =  MSDC_CMD_ERROR;
                }
            }
        break;
    }

    return ret;
}


UINT32 MSDC_WaitAndProcess_Response(UINT32 ch,UINT32 cmd,UINT32 timeout)
{
	UINT32 ret;
	UINT32 MSDCInt;

	ret = 0;
	while(timeout > 0 && ret == 0)
	{
        timeout --;
		MSDCInt = MSDC_READ32(MSDC_INT(ch)) ;
		if(MSDCInt > 0)
		{
            if(MSDCInt & INT_SD_CMDTO)
            {
                timeout = 0;
                MSDC_WRITE32(MSDC_INT(ch),INT_SD_CMDTO);
                break;
            }

			// Max Xia, Add CRCERR handler
			if(MSDCInt & INT_SD_RESP_CRCERR)
            {
                timeout = 0;
                MSDC_WRITE32(MSDC_INT(ch), INT_SD_RESP_CRCERR);
                break;
            }
			
			switch(GET_RESPONSE_TYPE(cmd))
			{
                case CMD_RSPTYPE_NO:
                    ret = MSDC_ProcResponseNo(ch,cmd,MSDCInt);
                break;
                case CMD_RSPTYPE_R1:
				case CMD_RSPTYPE_R1B:
                    ret = MSDC_ProcResponse1(ch,cmd,MSDCInt);
                break;
                case CMD_RSPTYPE_R2:
                    ret = MSDC_ProcResponse2(ch,cmd,MSDCInt);
                break;
                case CMD_RSPTYPE_R3:
                    ret = MSDC_ProcResponse3(ch,cmd,MSDCInt);
                break;
                case CMD_RSPTYPE_R4:
                break;
                case CMD_RSPTYPE_R5:
                break;
                case CMD_RSPTYPE_R6:
                    ret = MSDC_ProcResponse6(ch,cmd,MSDCInt);
                break;
                case CMD_RSPTYPE_R7:
                    ret = MSDC_ProcResponse7(ch,cmd,MSDCInt);
				break;
				
     				
			}
		}
		
	}

	if(timeout == 0)
	{
		switch(cmd & SDC_CMD_CMD_MASK)
		{
			case ACMD41_SD_SEND_OP_COND:
				MSDC_Card[MSDC_CH_INDEX(ch)].cardType = MSDC_CARDTYPE_UNKNOW;
			break;
		}
		ret	=  MSDC_CMD_NORESPONSE;
		
	}
    
    EdbgOutputDebugString("CMD%d Response 0x%x return %d\n",(GET_CMD_TYPE(cmd)),MSDC_READ32(SDC_RESP0(ch)),ret);
	return ret;
}

#if 0 //defined(BOOTDEVICE_SD)
// For upgrade uboot, first try to indentify it as SD card, then try eMMC
UINT32 MSDC_Identify_Card(UINT32 ch)
{
	UINT32 ret;
	UINT32 retryN = 5;
	UINT32 cmdArg;

	// init host controller
    MSDC_Init(ch);

	// reset card
	Printf("Send CMD0\r\n");
	while(retryN --)
	{
		ret = MSDC_Send_Cmd(ch,COM_CMD0_GO_IDLE_STATE,0);
		if(ret != MSDC_CMD_OK)
		{
	    	Printf("Send CMD0 Failed\r\n");
			return MSDC_LASTERROR_IDENTIFY | 0x01;
		}
	}
	
	Printf("Send CMD8\r\n");
	ret = MSDC_Send_Cmd(ch,SD_CMD8_SEND_IF_COND,CMD8_ARG_VHS_2V7_3V6|CMD8_ARG_CK_PAT);
	if(ret == MSDC_CMD_ERROR)
	{
	    Printf("Send CMD8 Failed\r\n");
		return MSDC_LASTERROR_IDENTIFY | 0x02;
	}
	else if(ret == MSDC_CMD_NORESPONSE)
	{
	    Printf("Send CMD8 no response\r\n");
		cmdArg = ACMD41_ARG_OCS_2V7_3V6;
	}
	else
	{
	   	Printf("Send CMD8 unkown\r\n");
		cmdArg = ACMD41_ARG_OCS_2V7_3V6|ACMD41_ARG_HCS;
	}

	Dump_Card_Type(ch, 0);
	
    Printf("Send ACMD41\r\n");
	retryN = 20000000/1000;
	while(retryN > 0)
	{
        retryN--;
		Printf("Send CMD55 \r\n");
        ret = MSDC_Send_Cmd(ch,COM_CMD55_APP_CMD,0);
        if(ret != MSDC_CMD_OK)
	    {
            Printf("Send CMD55 Failed\r\n");		    
	        //	return MSDC_LASTERROR_IDENTIFY | 0x08;
	    }

		Printf("Send ACMD41  \r\n");
		ret = MSDC_Send_Cmd(ch,SD_ACMD41_SD_SEND_OP_COND,cmdArg);
		if(ret == MSDC_CMD_ERROR)
		{
		   	Printf("Send ACMD41 Failed\r\n");
			return MSDC_LASTERROR_IDENTIFY | 0x04;
		}
		else if(ret == MSDC_CMD_BUSY)
		{
            MSDC_DELAY(0x60000);
			Printf("Card Busy,waiting....\r\n");
			continue;
		}
		else if(ret == MSDC_CMD_NORESPONSE )
		{
		   	Printf("MSDC_CMD_NORESPONSE\r\n");
			break;
		}
		else if(ret == MSDC_CMD_OK)
		{
		    Printf("card ready\r\n");
			break;
		}
		else{
		    Printf("unknow.....\r\n");
		}
	}

    if(retryN == 0)
    {
	    Printf("retryN = 0\r\n");
        return MSDC_LASTERROR_IDENTIFY | 0x05;
    }

        

	if( MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_SD20LATER_SDHC ||
		MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_SD20LATER_SDSC ||
		MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_SD1X_SDSC)
	{
		Printf("Send CMD2 \r\n");
        ret = MSDC_Send_Cmd(ch,COM_CMD2_ALL_SEND_CID,0);
        if(ret != MSDC_CMD_OK)
	    {		   
			Printf("Send CMD2 Failed\r\n");
            return MSDC_LASTERROR_IDENTIFY | 0x06;
        }

		Printf("Send CMD3 \r\n");
		ret = MSDC_Send_Cmd(ch,SD_CMD3_SEND_RELATIVE_ADDR,0);
		if(ret != MSDC_CMD_OK)
		{
			Printf("Send CMD3 Failed\r\n");
			return MSDC_LASTERROR_IDENTIFY | 0x07;
		}
	}

	Dump_Card_Type(ch, 1);

    if(MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_UNKNOW)
    {
	 	//Printf("INIT AS EMMC...\r\n");
        MSDC_Init(ch);

        retryN = 2000/20;
        while(retryN > 0)
        {
            retryN--;
            ret = MSDC_Send_Cmd(ch,COM_CMD0_GO_IDLE_STATE,0);
    	    if(ret != MSDC_CMD_OK)
    	    {
    		    return MSDC_LASTERROR_IDENTIFY | 0x08;
    	    }
            MSDC_DELAY(0x10000);
        }
        retryN = 2000/2; 	// More try times for some spcial eMMC  
        while(retryN > 0)
        {
            retryN--;
            
            ret = MSDC_Send_Cmd(ch,MMC_CMD1_SEND_OP_COND,MMC_OCR_2V7_3V6|MMC_OCR_1V7_1V95|MMC_OCR_SECTOR_MODE);
            if(ret == MSDC_CMD_ERROR)
            {
                return MSDC_LASTERROR_IDENTIFY | 0x09;
            }
            else if(ret == MSDC_CMD_BUSY)
            {
                MSDC_DELAY(0x1000000);
                continue;
            }
            else if(ret == MSDC_CMD_NORESPONSE )
            {
                return MSDC_LASTERROR_IDENTIFY | 0x0D;
            }
            else
            {
                break;
            }
        }

        
        if(retryN == 0)
        {
            return MSDC_LASTERROR_IDENTIFY | 0x0A;
        }


        ret = MSDC_Send_Cmd(ch,COM_CMD2_ALL_SEND_CID,0);
        if(ret != MSDC_CMD_OK)
    	{
            return MSDC_LASTERROR_IDENTIFY | 0x0B;
        }

        ret = MSDC_Send_Cmd(ch,MMC_CMD3_SET_RELATIVE_ADDR,(0x5678 << 16));
        if(ret != MSDC_CMD_OK)
        {
            return MSDC_LASTERROR_IDENTIFY | 0x0C;
        }
    }

    if(MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_UNKNOW)
    {
        return MSDC_LASTERROR_IDENTIFY | 0x0D;
    }
	
    ret = MSDC_Send_Cmd(ch,COM_CMD9_SEND_CSD,(MSDC_Card[MSDC_CH_INDEX(ch)].cardRCA << 16));
    if(ret != MSDC_CMD_OK)
    {
        return MSDC_LASTERROR_IDENTIFY | 0x0F;
    }

    Printf("card type is %x,RCA is 0x%x \r\n",MSDC_Card[MSDC_CH_INDEX(ch)].cardType,MSDC_Card[MSDC_CH_INDEX(ch)].cardRCA);
    //Printf("card max block size : 0X%x\n",MSDC_Card[MSDC_CH_INDEX(ch)].cardMaxBlockLen);
    //Printf("card max speed : 0x%x\n",MSDC_Card[MSDC_CH_INDEX(ch)].cardMaxSpeed);
    return MSDC_LASTERROR_CARDTYPE | MSDC_Card[MSDC_CH_INDEX(ch)].cardType;
   
    
}
//#elif defined(BOOTDEVICE_EMMC)
#endif

#if 1
// For normal bootup from emmc, first try to identify it as emmc, then try to identify it as eMMC.
UINT32 MSDC_Identify_Card(UINT32 ch)
{
	UINT32 ret;
	UINT32 retryN = 5;
	UINT32 cmdArg;

	// init host controller
    MSDC_Init(ch);

	// reset card
	Printf("Send CMD0\r\n");
	while(retryN --)
	{
		ret = MSDC_Send_Cmd(ch,COM_CMD0_GO_IDLE_STATE,0);
		if(ret != MSDC_CMD_OK)
		{
	    	Printf("Send CMD0 Failed\r\n");
			return MSDC_LASTERROR_IDENTIFY | 0x01;
		}
	}

	// First init it as emmc 
	if(MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_UNKNOW)
	{
        retryN = 2000/2;
        while(retryN > 0)
        {
            retryN--;
            
            ret = MSDC_Send_Cmd(ch,MMC_CMD1_SEND_OP_COND,MMC_OCR_2V7_3V6|MMC_OCR_1V7_1V95|MMC_OCR_SECTOR_MODE);
            if(ret == MSDC_CMD_ERROR)
            {
            	Printf("Send CMD1 Failed\r\n");
                return MSDC_LASTERROR_IDENTIFY | 0x09;
            }
            else if(ret == MSDC_CMD_BUSY)
            {
                MSDC_DELAY(0x1000000);
                continue;
            }
            else if(ret == MSDC_CMD_NORESPONSE )
            {
            	Printf("Send CMD1 no response\r\n");
                break;
            }
            else
            {
                break;
            }
        }

        
        if(retryN == 0)
        {
            return MSDC_LASTERROR_IDENTIFY | 0x0A;
        }

		if(MSDC_Card[MSDC_CH_INDEX(ch)].cardType != MSDC_CARDTYPE_UNKNOW)
		{
	        ret = MSDC_Send_Cmd(ch,COM_CMD2_ALL_SEND_CID,0);
	        if(ret != MSDC_CMD_OK)
	    	{
	            return MSDC_LASTERROR_IDENTIFY | 0x0B;
	        }

	        ret = MSDC_Send_Cmd(ch,MMC_CMD3_SET_RELATIVE_ADDR,(0x5678 << 16));
	        if(ret != MSDC_CMD_OK)
	        {
	            return MSDC_LASTERROR_IDENTIFY | 0x0C;
	        }
		}
    }

	Dump_Card_Type(ch, 0);

	// Second Init it as SD Card
	if(MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_UNKNOW)
	{
		//Printf("INIT AS SD CARD...\r\n");
        MSDC_Init(ch);

        retryN = 200/20;
        while(retryN > 0)
        {
            retryN--;
            ret = MSDC_Send_Cmd(ch,COM_CMD0_GO_IDLE_STATE,0);
    	    if(ret != MSDC_CMD_OK)
    	    {
    		    return MSDC_LASTERROR_IDENTIFY | 0x08;
    	    }
            MSDC_DELAY(0x10000);
        }
		
		Printf("Send CMD8\r\n");
		ret = MSDC_Send_Cmd(ch,SD_CMD8_SEND_IF_COND,CMD8_ARG_VHS_2V7_3V6|CMD8_ARG_CK_PAT);
		if(ret == MSDC_CMD_ERROR)
		{
		    Printf("Send CMD8 Failed\r\n");
			return MSDC_LASTERROR_IDENTIFY | 0x02;
		}
		else if(ret == MSDC_CMD_NORESPONSE)
		{
		    Printf("Send CMD8 no response\r\n");
			cmdArg = ACMD41_ARG_OCS_2V7_3V6;
		}
		else
		{
		   	Printf("Send CMD8 unkown\r\n");
			cmdArg = ACMD41_ARG_OCS_2V7_3V6|ACMD41_ARG_HCS;
		}

		Dump_Card_Type(ch, 1);
		
	    Printf("Send ACMD41\r\n");
		//retryN = 20000000/1000;
		MSDC_Card[MSDC_CH_INDEX(ch)].cardOCR = 0;
		retryN = 200;
		while(retryN > 0)
		{
	        retryN--;
			//Printf("Send CMD55 \r\n");
	        ret = MSDC_Send_Cmd(ch,COM_CMD55_APP_CMD,0);
	        if(ret != MSDC_CMD_OK)
		    {
	            Printf("Send CMD55 Failed\r\n");		    
		        //	return MSDC_LASTERROR_IDENTIFY | 0x08;
		    }

			//Printf("Send ACMD41  \r\n");
			if(MSDC_Card[MSDC_CH_INDEX(ch)].cardOCR == 0)
			{
				cmdArg = 0;
			}
			else
			{
				cmdArg = ACMD41_ARG_HCS|MSDC_Card[MSDC_CH_INDEX(ch)].cardOCR;
			}
			
			ret = MSDC_Send_Cmd(ch,SD_ACMD41_SD_SEND_OP_COND, cmdArg);
			if(ret == MSDC_CMD_ERROR)
			{
			   	Printf("Send ACMD41 Failed\r\n");
				return MSDC_LASTERROR_IDENTIFY | 0x04;
			}
			else if(ret == MSDC_CMD_BUSY)
			{
	            //MSDC_DELAY(0x60000);
				//MSDC_DELAY(0x1000000);
				TIM_DelayUS(5000);
				//Printf("Card Busy,waiting....\r\n");
				continue;
			}
			else if(ret == MSDC_CMD_NORESPONSE )
			{
			   	Printf("MSDC_CMD_NORESPONSE\r\n");
				break;
			}
			else if(ret == MSDC_CMD_OK)
			{
			    Printf("SD Card Ready\r\n");
				break;
			}
			else{
			    Printf("unknow.....\r\n");
			}
		}

	    if(retryN == 0)
	    {
		    Printf("ACMD41 retryN = 0\r\n");
	        return MSDC_LASTERROR_IDENTIFY | 0x05;
	    }

		if( MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_SD20LATER_SDHC ||
			MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_SD20LATER_SDSC ||
			MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_SD1X_SDSC)
		{
			Printf("Send CMD2 \r\n");
	        ret = MSDC_Send_Cmd(ch,COM_CMD2_ALL_SEND_CID,0);
	        if(ret != MSDC_CMD_OK)
		    {		   
				Printf("Send CMD2 Failed\r\n");
	            return MSDC_LASTERROR_IDENTIFY | 0x06;
	        }

			Printf("Send CMD3 \r\n");
			ret = MSDC_Send_Cmd(ch,SD_CMD3_SEND_RELATIVE_ADDR,0);
			if(ret != MSDC_CMD_OK)
			{
				Printf("Send CMD3 Failed\r\n");
				return MSDC_LASTERROR_IDENTIFY | 0x07;
			}
		}
	}

	Dump_Card_Type(ch, 1);
    

    if(MSDC_Card[MSDC_CH_INDEX(ch)].cardType == MSDC_CARDTYPE_UNKNOW)
    {
        return MSDC_LASTERROR_IDENTIFY | 0x0D;
    }
	
    ret = MSDC_Send_Cmd(ch,COM_CMD9_SEND_CSD,(MSDC_Card[MSDC_CH_INDEX(ch)].cardRCA << 16));
    if(ret != MSDC_CMD_OK)
    {
        return MSDC_LASTERROR_IDENTIFY | 0x0F;
    }

    Printf("card type is %x,RCA is 0x%x\n",MSDC_Card[MSDC_CH_INDEX(ch)].cardType,MSDC_Card[MSDC_CH_INDEX(ch)].cardRCA);
    //Printf("card max block size : 0X%x\n",MSDC_Card[MSDC_CH_INDEX(ch)].cardMaxBlockLen);
    //Printf("card max speed : 0x%x\n",MSDC_Card[MSDC_CH_INDEX(ch)].cardMaxSpeed);
    return MSDC_LASTERROR_CARDTYPE | MSDC_Card[MSDC_CH_INDEX(ch)].cardType;
   
    
}

#else
UINT32 MSDC_Identify_Card(UINT32 ch)
{
	Printf("BOOT_DEVICE is not EMMC or SD\n");
	return 1;
}

#endif

UINT32 MSDC_GetPortIndex(UINT32 ch)
{
    if(ch == MSDC_CH1)
    {
        return 0;
    }
    else if(ch == MSDC_CH2)
    {
        return 1;
    }
    else if(ch == MSDC_CH3)
    {
        return 2;
    }

    return 4;
}

UINT32 MSDC_StateChange(UINT32 ch,UINT32 cardState)
{
    UINT32 ret;
    ret = MSDC_Send_Cmd(ch,COM_CMD7_SELECT_CARD,(MSDC_Card[MSDC_CH_INDEX(ch)].cardRCA << 16));
    if(ret != MSDC_CMD_OK)
    {
        return MSDC_LASTERROR_STATECHANGE | 0x01;
    }
    return MSDC_LASTERROR_STATECHANGE;
}

UINT32 MSDC_SetBlockLength(UINT32 ch,UINT32 blkLen)
{
    UINT32 ret;
    ret = MSDC_Send_Cmd(ch,COM_CMD16_SET_BLOCKLEN,blkLen);
    if(ret != MSDC_CMD_OK)
    {
        return MSDC_LASTERROR_SETBLKLEN |MSDC_LASTERROR_ERRORS| 0x01;
    }

    return MSDC_LASTERROR_SETBLKLEN | MSDC_LASTERROR_OK;
}


UINT32 MSDC_SetEXTCSD(UINT32 ch, UINT32 index, UINT32 value, UINT32 mask)
{
    UINT32 ret;

    ret = MSDC_Send_Cmd(ch, COM_CMD6_SWITCH, CMD6_ARGS(EXT_CSD_ACCESS_MODE_CLEARBITS, index, mask));
    if(ret != MSDC_CMD_OK)
    {
        return MSDC_LASTERROR_SETEXTCSD |MSDC_LASTERROR_ERRORS| 0x01;
    }
	
	ret = MSDC_Send_Cmd(ch, COM_CMD6_SWITCH, CMD6_ARGS(EXT_CSD_ACCESS_MODE_SETBITS, index, value));
    if(ret != MSDC_CMD_OK)
    {
        return MSDC_LASTERROR_SETEXTCSD |MSDC_LASTERROR_ERRORS| 0x02;
    }
    return MSDC_LASTERROR_SETEXTCSD | MSDC_LASTERROR_OK;
}

UINT32 MSDC_SetEXTCSD_Ex(UINT32 ch, UINT32 index, UINT32 value, UINT32 mask)
{
    UINT32 ret;

    ret = MSDC_Send_Cmd(ch, COM_CMD6_SWITCH, CMD6_ARGS(EXT_CSD_ACCESS_MODE_WRITEBYTE, index, value));
    if(ret != MSDC_CMD_OK)
    {
        return MSDC_LASTERROR_SETEXTCSD |MSDC_LASTERROR_ERRORS| 0x01;
    }
	
	
    return MSDC_LASTERROR_SETEXTCSD | MSDC_LASTERROR_OK;
}

UINT32 MSDC_EMMC_EnterBootMode0(UINT32 ch)
{
	UINT32 ret;
	UINT32 timeout;
	UINT32 tmpReg;

	MSDC_Init(ch);

	ret = MSDC_Send_Cmd(ch,COM_CMD0_GO_IDLE_STATE,0xF0F0F0F0);
	if(ret != MSDC_CMD_OK)
	{
		return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x09;
	}

	timeout = TIM_CalcExpiredUS(100);

	while((MSDC_READ32(SDC_STS(ch)) & SDC_STS_SDCBUSY)!= 0)
	{
		if(TIM_IsExpired(timeout))
		{
			return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x07;
		}
	}

	TIM_DelayUS(400);	 // more than 74 clock cycles

	MSDC_WRITE32(SDC_BLK_NUM(ch),129);	 // less than 64k(total sram size)+512 byte data

	MSDC_WRITE32(EMMC_CFG0(ch),(EMMC_BOOT_WAIT_EXIT_DELAY(4)|EMMC_BOOT_SUPPORT|EMMC_BOOT_MODE_RESET_CMD));

	MSDC_WRITE32(EMMC_CFG1(ch),(EMMC_BOOT_ACK_TOC(0xFF)|EMMC_BOOT_DAT_TOC(0xFFFF)));

	MSDC_WRITE32(SDC_ARG(ch),0xFFFFFFFA);

	MSDC_WRITE32(SDC_CMD(ch),0x02001000);

	MSDC_MASK32(EMMC_CFG0(ch),EMMC_BOOT_START,EMMC_BOOT_START_MASK);

	timeout = TIM_CalcExpiredUS(100000);			//spec. 50ms max 
	
	do
	{
		tmpReg = MSDC_READ32(EMMC_STS(ch));

		if(TIM_IsExpired(timeout))
		{
			MSDC_WRITE32(EMMC_CFG0(ch),0);
			return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x08;
		}

	}while((tmpReg & (EMMC_BOOT_ACK_TO|EMMC_BOOT_ACK_ERR|EMMC_BOOT_ACK_RECV))==0);


	if(tmpReg & EMMC_BOOT_ACK_TO)
	{
		MSDC_WRITE32(EMMC_CFG0(ch),0);
		return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x04;
	}

	if(tmpReg & EMMC_BOOT_ACK_ERR)
	{
		MSDC_WRITE32(EMMC_CFG0(ch),0);
		return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x05;
	}

	if((tmpReg & EMMC_BOOT_ACK_RECV) == 0)
	{
		MSDC_WRITE32(EMMC_CFG0(ch),0);
		return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x06;
	}

	return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK;

}

static UINT32 MSDC_EMMC_ReadOffset = 0;

UINT32 MSDC_EMMC_ExitBootMode0(UINT32 ch)
{

	UINT32 timeout;
	
	timeout = TIM_CalcExpiredUS(10000);

    MSDC_WRITE32(SDC_ARG(ch),0x00);

    MSDC_WRITE32(SDC_CMD(ch),0x00001000);

    MSDC_MASK32(EMMC_CFG0(ch), EMMC_BOOT_STOP, EMMC_BOOT_STOP_MASK);

    while((MSDC_READ32(EMMC_STS(ch)) & EMMC_BOOT_UP_STATE)!= 0)
    {
    	if(TIM_IsExpired(timeout))
        {
            return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x09;
        }
    }

    MSDC_MASK32(EMMC_CFG0(ch),0,EMMC_BOOT_SUPPORT_MASK);

	// reset boot partition read pointer
	MSDC_EMMC_ReadOffset = 0;

	return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK;

}



void MSDC_SetPadDriver(UINT32 ch,uint32 pad,uint32 drv)
{
  switch(ch)
  {
  	case MSDC_CH1:
		if(pad & PAD_CLK)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG19,drv,BIT_PIN_DRV_MASK); 
  		}

  		if(pad & PAD_CMD)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG20,drv,BIT_PIN_DRV_MASK); 
  		}

  		if(pad & PAD_DATA0)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG21,drv,BIT_PIN_DRV_MASK); 
  		}

		break;

	case MSDC_CH2:

		if(pad & PAD_CLK)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG6,drv,BIT_PIN_DRV_MASK); 
  		}

  		if(pad & PAD_CMD)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG7,drv,BIT_PIN_DRV_MASK); 
  		}

  		if(pad & PAD_DATA0)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG8,drv,BIT_PIN_DRV_MASK); 
  		}

		break; 

	case MSDC_CH3:

		if(pad & PAD_CLK)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG12,drv,BIT_PIN_DRV_MASK); 
  		}

  		if(pad & PAD_CMD)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG13,drv,BIT_PIN_DRV_MASK); 
  		}

  		if(pad & PAD_DATA0)
  		{
    		MASKMEM(REG_PAD_MSDC_CFG14,drv,BIT_PIN_DRV_MASK); 
  		}

		break;

  }

}

#if 0
// Copy Dramk from emmc boot partition 2
UINT32 MSDC_EMMC_ReadFromBoot2(UINT32 ch, UINT32 u4DramkPhyOffset, UINT32 u4DramkRunAddress, UINT32 u4DramkSize)
{
    UINT32 readSize;
	UINT32 ret;
	
	Printf("get dramk emmc boot partition 2 start");
	//MSDC_Init(port);
    
	ret = MSDC_Identify_Card(ch);
	if((ret & MSDC_LASTERROR_ERRTYPE_MASK) != MSDC_LASTERROR_CARDTYPE)
	{
		Printf("identify card fail");
		return MSDC_LASTERROR_EMMCBOOT2 | MSDC_LASTERROR_ERRORS | 0x01;
	}
	else if(ret == (MSDC_LASTERROR_CARDTYPE | MSDC_CARDTYPE_UNKNOW))
	{
		Printf("unknow card type");
		return MSDC_LASTERROR_EMMCBOOT2 | MSDC_LASTERROR_ERRORS | 0x02;
	}
    
	ret = MSDC_StateChange(ch, 0);
	if(ret != (MSDC_LASTERROR_STATECHANGE | MSDC_LASTERROR_OK))
	{
		return MSDC_LASTERROR_EMMCBOOT2 | MSDC_LASTERROR_ERRORS | 0x03;
	}

    ret = MSDC_SetBlockLength(ch, MSDC_CARD_DEFAULT_BLOCK_LEN);
    if(ret != (MSDC_LASTERROR_SETBLKLEN | MSDC_LASTERROR_OK))
	{
		return MSDC_LASTERROR_EMMCBOOT2 | MSDC_LASTERROR_ERRORS | 0x08;
	}

	ret = MSDC_SetEXTCSD(ch, 179, EXT_CSD_179_BOOTPARTITION_BOOT2, EXT_CSD_179_BOOTPARTITION_MASK);
	if(ret != (MSDC_LASTERROR_SETEXTCSD | MSDC_LASTERROR_OK))
	{
		return MSDC_LASTERROR_EMMCBOOT2 | MSDC_LASTERROR_ERRORS | 0x0a;
	}
	
	Printf("read dramk");
	ret = MSDC_ReadBlock_PIO(ch, u4DramkPhyOffset,(UINT32 *)u4DramkRunAddress, u4DramkSize);

  	if(ret == (MSDC_LASTERROR_READBLOCK |MSDC_LASTERROR_ERRORS| 0x06))
  	{
		Printf("read header fail,set pin driver to max,then try again");
    	MSDC_SetPadDriver(ch, PAD_ALL, PAD_DRV_MAX);
    	ret = MSDC_ReadBlock_PIO(ch, u4DramkPhyOffset,(UINT32 *)u4DramkRunAddress, u4DramkSize);
  	}

	if(ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
	{
		return MSDC_LASTERROR_EMMCBOOT2 | MSDC_LASTERROR_ERRORS | 0x04;
	}

    return MSDC_LASTERROR_EMMCBOOT2 | MSDC_LASTERROR_OK;
}
#endif

// eMMC boot partition read function
UINT32 MSDC_EMMC_Read(UINT32 ch, UINT32 startAddr,UINT32 *pBuf,UINT32 dataSize)
{
    UINT32 timeout;
    UINT32 fifoCnt;
	UINT32 tmpReg;

    if(dataSize == 0)
    {
       return MSDC_LASTERROR_EMMCREAD |MSDC_LASTERROR_ERRORS| 0x01;  
    }

	// Check size for 4 bytes alignment
    if(dataSize & 0x3)
    {
       dataSize += (4 - (dataSize & 0x3));
    }

	// Check current read pointer
	if(startAddr < MSDC_EMMC_ReadOffset)
	{
		return MSDC_LASTERROR_EMMCREAD |MSDC_LASTERROR_ERRORS| 0x03;
	}

	// Handle startAddr,  now it is a skip size
	startAddr -= MSDC_EMMC_ReadOffset;

	// Check address for 4 bytes alignment
	if(startAddr & 0x3)
    {
        return MSDC_LASTERROR_EMMCREAD |MSDC_LASTERROR_ERRORS| 0x04;
    }

	// If current read pointer is 0, it means that read data from boot partition first time 
	if(MSDC_EMMC_ReadOffset == 0)
	{
		timeout = TIM_CalcExpiredUS(1000000);            //spec. 1s max 

    	do
    	{
        	tmpReg = MSDC_READ32(EMMC_STS(ch));

        	if(TIM_IsExpired(timeout))
        	{
            	MSDC_WRITE32(EMMC_CFG0(ch),0);
            	return MSDC_LASTERROR_EMMCREAD |MSDC_LASTERROR_ERRORS| 0x08;
        	}
    	}while((tmpReg & (EMMC_BOOT_DAT_RECV|EMMC_BOOT_DAT_TO))==0);

		// Receive first packet data from emmc
		if((tmpReg & EMMC_BOOT_DAT_RECV)&&!(tmpReg & EMMC_BOOT_DAT_TO))
		{
			if(startAddr > 0)
            {
                *pBuf = MSDC_READ32(MSDC_RXDATA(ch));
                startAddr -= 4;
				MSDC_EMMC_ReadOffset += 4;
            }

            else
            {
                *pBuf = MSDC_READ32(MSDC_RXDATA(ch));
				MSDC_EMMC_ReadOffset += 4;
                pBuf++;
                dataSize -= 4;
            }
		}
		else
		{
			return MSDC_LASTERROR_EMMCREAD |MSDC_LASTERROR_ERRORS| 0x09;
		}
	}

	timeout = TIM_CalcExpiredUS(10000);
    while(dataSize > 0)
    {
        fifoCnt = ((MSDC_READ32(MSDC_FIFOCS(ch)) >> MSDC_FIFOCS_RXFIFOCNT_SHIFT)& MSDC_FIFOCS_FIFOCNT_MASK);

        if(fifoCnt < 4)
        {
        	if(TIM_IsExpired(timeout))
        	{
        		return MSDC_LASTERROR_EMMCREAD |MSDC_LASTERROR_ERRORS| 0x05;
        	}
			else
			{
            	continue;
			}
        }
        else
        {
        	timeout = TIM_CalcExpiredUS(10000);
			// Skip unuseful data 
            if(startAddr > 0)
            {
                *pBuf = MSDC_READ32(MSDC_RXDATA(ch));
                startAddr -= 4;
				MSDC_EMMC_ReadOffset += 4;
            }
            else
            {
                *pBuf = MSDC_READ32(MSDC_RXDATA(ch));
				MSDC_EMMC_ReadOffset += 4;
                pBuf++;
                dataSize -= 4;
            }
        }
    }

	return MSDC_LASTERROR_EMMCREAD | MSDC_LASTERROR_OK;

}


/*
UINT32 MSDC_EMMC_BootMode(UINT32 ch, UINT32 startAddr,UINT32 *pBuf,UINT32 dataSize)
{
    UINT32 timeout;
    UINT32 tmpReg;
    UINT32 fifoCnt;
    UINT32 ret;
    timeout = 0x20000;
    
    if(dataSize == 0)
    {
       return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x01;  
    }

    if(dataSize & 0x3)
    {
        return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x02;
    }

    if(startAddr & 0x3)
    {
        return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x03;
    }
    MSDC_Init(ch);
    ret = MSDC_Send_Cmd(ch,COM_CMD0_GO_IDLE_STATE,0xF0F0F0F0);
	if(ret != MSDC_CMD_OK)
	{
		return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x09;
	}
    
    while((MSDC_READ32(SDC_STS(ch)) & SDC_STS_SDCBUSY)!= 0)
    {
        timeout--;
        if(timeout == 0)
        {
            return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x07;
        }
    }

    MSDC_WRITE32(EMMC_CFG0(ch),(EMMC_BOOT_WAIT_EXIT_DELAY(4)|EMMC_BOOT_SUPPORT|EMMC_BOOT_MODE_PULL_LOW));
    MSDC_WRITE32(EMMC_CFG1(ch),(EMMC_BOOT_ACK_TOC(0xFF)|EMMC_BOOT_DAT_TOC(0xFFFF)));
    MSDC_WRITE32(SDC_CMD(ch),0x02001000);
    MSDC_MASK32(EMMC_CFG0(ch),EMMC_BOOT_START,EMMC_BOOT_START_MASK);

    timeout = 0x20000;
    do
    {
        tmpReg = MSDC_READ32(EMMC_STS(ch));
        timeout--;
        if(timeout == 0)
        {
            MSDC_WRITE32(EMMC_CFG0(ch),0);
            return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x08;
        }
        
    }while((tmpReg & (EMMC_BOOT_ACK_TO|EMMC_BOOT_ACK_ERR|EMMC_BOOT_ACK_RECV))==0);

    if(tmpReg & EMMC_BOOT_ACK_TO)
    {
        MSDC_WRITE32(EMMC_CFG0(ch),0);
        return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x04;
    }

    if(tmpReg & EMMC_BOOT_ACK_ERR)
    {
        MSDC_WRITE32(EMMC_CFG0(ch),0);
        return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x05;
    }

    if((tmpReg & EMMC_BOOT_ACK_RECV) == 0)
    {
        MSDC_WRITE32(EMMC_CFG0(ch),0);
        return MSDC_LASTERROR_EMMCBOOTMODE |MSDC_LASTERROR_ERRORS| 0x06;
    }

    while(dataSize > 0)
    {
        fifoCnt = ((MSDC_READ32(MSDC_FIFOCS(ch)) >> MSDC_FIFOCS_RXFIFOCNT_SHIFT)&MSDC_FIFOCS_FIFOCNT_MASK);
        if(fifoCnt < 4)
        {
            continue;
        }
        else
        {
            if(startAddr > 0)
            {
                *pBuf = MSDC_READ32(MSDC_RXDATA(ch));
                startAddr -= 4;
                continue;
            }
            else
            {
                *pBuf = MSDC_READ32(MSDC_RXDATA(ch));
                pBuf++;
                dataSize -= 4;
            }
        }
    }

    MSDC_WRITE32(SDC_ARG(ch),0x00);
    MSDC_WRITE32(SDC_CMD(ch),0x00001000);
    MSDC_MASK32(EMMC_CFG0(ch),EMMC_BOOT_STOP,EMMC_BOOT_STOP_MASK);
    while((MSDC_READ32(EMMC_STS(ch)) & EMMC_BOOT_UP_STATE)!= 0);
    MSDC_MASK32(EMMC_CFG0(ch),0,EMMC_BOOT_SUPPORT_MASK);
    return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK;
}
*/

#if SUPPORT_UBOOT_BACKUP
UINT32 MSDC_WriteBlock_PIO(UINT32 ch, UINT32 startAddr,UINT32 *pBuf,UINT32 dataSize)
{
	UINT32 ret;
	UINT32 fifoCnt;
	UINT32 timeout;
	UINT32 writetimeout = 0;

	//Printf("want to write size: %d\n", dataSize);

	if(dataSize == 0)
	{
		return MSDC_LASTERROR_WRITEBLOCK |MSDC_LASTERROR_ERRORS| 0x01;
	}

	if((dataSize & (MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen - 1))!=0)
	{
		return MSDC_LASTERROR_WRITEBLOCK |MSDC_LASTERROR_ERRORS| 0x02;
	}

	if((startAddr & (MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen - 1))!=0)
	{
		return MSDC_LASTERROR_WRITEBLOCK |MSDC_LASTERROR_ERRORS| 0x03;
	}

	MSDC_MASK32(MSDC_FIFOCS(ch), MSDC_FIFOCS_CLR, MSDC_FIFOCS_CLR_MASK);
	while (0 != (MSDC_READ32(MSDC_FIFOCS(ch)) & MSDC_FIFOCS_CLR));

	MSDC_WRITE32(SDC_BLK_NUM(ch),(dataSize/MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen));

	if(MSDC_Card[MSDC_CH_INDEX(ch)].cardOps & CARD_OPS_SECTOR_ADDRESS)
	{
		startAddr = startAddr / MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen;
		//Printf("--> use block address <--\n");
	}

	ret = MSDC_Send_Cmd(ch,COM_CMD25_WRITE_MULTIPLE_BLOCK,startAddr);
	if(ret != MSDC_CMD_OK)
	{
		return MSDC_LASTERROR_WRITEBLOCK |MSDC_LASTERROR_ERRORS| 0x04;
	}

	timeout = TIM_CalcExpiredUS(2000000);         //1000ms

	while(dataSize > 0)
	{
		fifoCnt = ((MSDC_READ32(MSDC_FIFOCS(ch)) >> MSDC_FIFOCS_TXFIFOCNT_SHIFT) & MSDC_FIFOCS_FIFOCNT_MASK);
		//Printf("---> fifo: %d <---\n", fifoCnt);
		if(fifoCnt < 128)
		{
			MSDC_WRITE32(MSDC_TXDATA(ch), *pBuf);
			pBuf++;
			dataSize -= 4;
		}
		else
		{
			if(TIM_IsExpired(timeout))
			{
				writetimeout = 1;
				Printf("--> write timeout <--\n");
				Printf("left data size: %d, fifoCnt: %d\n", dataSize, fifoCnt);
				break;
			}
			else
			{
				continue;
			}
		}
	}

	//Printf("--> write complete <--\n");

#if (MSDC_AUTOCMD12_EN == 0)
	ret = MSDC_Send_Cmd(ch,COM_CMD12_STOP_TRANSMISSION,0);
	if(ret != MSDC_CMD_OK)
	{
		return MSDC_LASTERROR_WRITEBLOCK |MSDC_LASTERROR_ERRORS| 0x05;
	}
#endif

	if(writetimeout == 1)
	{
		return MSDC_LASTERROR_WRITEBLOCK |MSDC_LASTERROR_ERRORS| 0x06;
	}

	return MSDC_LASTERROR_WRITEBLOCK | MSDC_LASTERROR_OK;

}

#endif

#if 0
void msdc_dump_register(UINT32 ch)
{
	printf( "[00] MSDC_CFG    = 0x%08X\n", MSDC_READ32(MSDC_CFG(ch)));
	printf( "[04] MSDC_IOCON  = 0x%08X\n", MSDC_READ32(MSDC_IOCON(ch)));
	printf( "[08] MSDC_PS     = 0x%08X\n", MSDC_READ32(MSDC_PS(ch)));
	printf( "[0C] MSDC_INT    = 0x%08X\n", MSDC_READ32(MSDC_INT(ch)));
	printf( "[10] MSDC_INTEN  = 0x%08X\n", MSDC_READ32(MSDC_INTEN(ch)));
	printf( "[14] MSDC_FIFOCS = 0x%08X\n", MSDC_READ32(MSDC_FIFOCS(ch)));
	printf( "[18] MSDC_TXDATA = 0x%08X\n", MSDC_READ32(MSDC_TXDATA(ch)));
	printf( "[1C] MSDC_RXDATA = 0x%08X\n", MSDC_READ32(MSDC_RXDATA(ch)));
	printf( "[30] SDC_CFG     = 0x%08X\n", MSDC_READ32(SDC_CFG(ch)));
	printf( "[34] SDC_CMD     = 0x%08X\n", MSDC_READ32(SDC_CMD(ch)));
	printf( "[38] SDC_ARG     = 0x%08X\n", MSDC_READ32(SDC_ARG(ch)));
	printf( "[3C] SDC_STS     = 0x%08X\n", MSDC_READ32(SDC_STS(ch)));
	printf( "[50] SDC_BLK_NUM = 0x%08X\n", MSDC_READ32(SDC_BLK_NUM(ch)));
	printf( "[90] DMA_SA      = 0x%08X\n", MSDC_READ32(DMA_SA(ch)));
	printf( "[94] DMA_CA      = 0x%08X\n", MSDC_READ32(DMA_CA(ch)));
	printf( "[98] DMA_CTRL    = 0x%08X\n", MSDC_READ32(DMA_CTRL(ch)));
	printf( "[9C] DMA_CFG     = 0x%08X\n", MSDC_READ32(DMA_CFG(ch)));
	printf( "[A8] DMA_LEN     = 0x%08X\n", MSDC_READ32(DMA_LEN(ch)));
	return;
}
#endif

UINT32 MSDC_ReadBlock_DMA(UINT32 ch, UINT32 startAddr, UINT32 *pBuf,UINT32 dataSize)
{
    UINT32 ret;
    UINT32 dataStatus;
    UINT32 status = MSDC_LASTERROR_OK;
    UINT32 dma_retry = 0x4FFFFF;

    MSDC_DMA_ON(ch);

    if (MSDC_Is_eMMC_Card(ch))
		MSDC_WRITE32(MSDC_INT(ch), INT_SD_CMDRDY);

	dataStatus = MSDC_READ32(MSDC_INT(ch));
	if (dataStatus != 0) {
		Printf("Before Start DMA =====> 0x%08X\n", dataStatus);
		// Clear Interrupts
		MSDC_WRITE32(MSDC_INT(ch), MSDC_READ32(MSDC_INT(ch)));
	}

	MSDC_WRITE32(MSDC_INTEN(ch), INT_DMA_XFER_DONE | INT_SD_XFER_COMPLETE | INT_SD_DATA_CRCERR | INT_SD_DATTO | INT_SD_CSTA);
	MSDC_WRITE32(DMA_SA(ch), (UINT32)(pBuf));
	MSDC_WRITE32(DMA_LEN(ch), dataSize);

	if (MSDC_READ32(DMA_LEN(ch)) != (MSDC_READ32(SDC_BLK_NUM(ch)) * 512))
	{
		Printf("MSDC_DMA_LEN = 0x%08X, SDC_BLK_NUM = 0x%08X", MSDC_READ32(DMA_LEN(ch)), MSDC_READ32(SDC_BLK_NUM(ch)));
	}

	MSDC_SET_VAL(DMA_CTRL(ch), DMA_CTRL_LAST_BUF, 1);
	MSDC_SET_VAL(DMA_CTRL(ch), DMA_CTRL_DESC_MODE, 0);//Basic DMA mode
	MSDC_SET_VAL(DMA_CTRL(ch), DMA_CTRL_BRUSTSZ, MSDC_DMA_BST_64);

	//msdc_dump_register(ch);
	MSDC_START_DMA(ch);

	while(dma_retry) {
		dataStatus = MSDC_READ32(MSDC_INT(ch));
		if(dataStatus & INT_SD_DATTO) {
			Printf("INT_SD_DATTO\n");
			status = MSDC_LASTERROR_READBLOCK;
			break;
		}

		if(dataStatus & INT_SD_DATA_CRCERR) {
			Printf("INT_SD_DATA_CRCERR\n");
			status = MSDC_LASTERROR_STATECHANGE;
			break;
		}

		if(dataStatus & INT_SD_XFER_COMPLETE) {
			status = MSDC_LASTERROR_OK;
			break;
		}

		if (dataStatus & INT_SD_AUTOCMD_RESP_CRCERR) {
			status = MSDC_LASTERROR_STATECHANGE;
			Printf("INT_SD_AUTOCMD_RESP_CRCERR\n");
			break;
		}

		if (dataStatus != 0) {
			if (dataStatus & INT_SD_AUTOCMD_RESP_CRCERR) // Autocmd CRC ERR
			{
				status = MSDC_LASTERROR_STATECHANGE;
				Printf("INT_SD_AUTOCMD_RESP_CRCERR\n");
				break;
			}
			else if (dataStatus & INT_SD_AUTOCMD_TO) // Autocmd Timeout
			{
				status = MSDC_LASTERROR_READBLOCK;
				Printf("INT_SD_AUTOCMD_TO\n");
				break;
			}
		}
		dma_retry--;
	}

	if (dma_retry == 0) {
		Printf("[ERROR]Data DMA transfer failed\n");
		status = MSDC_LASTERROR_EMMCREAD;
		//msdc_dump_register(ch);
		//RESET_MSDC_CLOCK_GATE(mmc);
	}

	// Stop DMA and wait it was completed
	MSDC_DMA_STOP(ch);
	// Clear Interrupts
	MSDC_WRITE32(MSDC_INT(ch), MSDC_READ32(MSDC_INT(ch)));
	MSDC_WRITE32(MSDC_INTEN(ch), 0);

	MSDC_DMA_OFF(ch);
	MSDC_WRITE32(DMA_LEN(ch), 0);

	// Check FIFO Status
	if (status == MSDC_LASTERROR_STATECHANGE) {
		MSDC_CLR_FIFO(ch);
	}

	return status;
}

UINT32 MSDC_ReadBlock_PIO(UINT32 ch, UINT32 startAddr,UINT32 *pBuf,UINT32 dataSize)
{
    UINT32 ret;
    UINT32 fifoCnt;
    UINT32 timeout;
    UINT32 readtimeout = 0;

    //Printf("want to read size: %d\n", dataSize);
    if(dataSize == 0) {
       return MSDC_LASTERROR_READBLOCK |MSDC_LASTERROR_ERRORS| 0x01;
    }

    if((dataSize & (MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen - 1))!=0) {
       return MSDC_LASTERROR_READBLOCK |MSDC_LASTERROR_ERRORS| 0x02;
    }

    if((startAddr & (MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen - 1))!=0) {
       return MSDC_LASTERROR_READBLOCK |MSDC_LASTERROR_ERRORS| 0x03;
    }

    MSDC_MASK32(MSDC_FIFOCS(ch), MSDC_FIFOCS_CLR, MSDC_FIFOCS_CLR_MASK);
    while (0 != (MSDC_READ32(MSDC_FIFOCS(ch)) & MSDC_FIFOCS_CLR));

    MSDC_WRITE32(SDC_BLK_NUM(ch),(dataSize/MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen));

    if(MSDC_Card[MSDC_CH_INDEX(ch)].cardOps & CARD_OPS_SECTOR_ADDRESS) {
        startAddr = startAddr / MSDC_Card[MSDC_CH_INDEX(ch)].cardBlockLen;
	//Printf("--> use block address <--\n");
    }

    ret = MSDC_Send_Cmd(ch,COM_CMD18_READ_MULTIPLE_BLOCK,startAddr);
    if(ret != MSDC_CMD_OK) {
        return MSDC_LASTERROR_READBLOCK |MSDC_LASTERROR_ERRORS| 0x04;
    }

    //timeout = TIM_CalcExpiredUS(200000);         //spec. max 100ms
    timeout = TIM_CalcExpiredUS(2000000);         //1000ms

    //sram 0xf4000000 - 0xf4010000
    if ((UINT32)pBuf >= 0xf4000000 && (UINT32)pBuf <= 0xf4010000) {
        while(dataSize > 0) {
            fifoCnt = ((MSDC_READ32(MSDC_FIFOCS(ch)) >> MSDC_FIFOCS_RXFIFOCNT_SHIFT)&MSDC_FIFOCS_FIFOCNT_MASK);
            //Printf("---> fifo: %d <---\n", fifoCnt);
            if(fifoCnt >= 4) {
                *pBuf = MSDC_READ32(MSDC_RXDATA(ch));
                pBuf++;
                dataSize -= 4;
                //timeout = TIM_CalcExpiredUS(2000000);
            } else {
                if(TIM_IsExpired(timeout)) {
                    readtimeout = 1;
    		    Printf("--> read timeout <--\n");
    		    Printf("left data size: %d, fifoCnt: %d\n", dataSize, fifoCnt);
                    break;
            	} else {
    		    continue;
    	        }
            }
        }
    } else {
        ret = MSDC_ReadBlock_DMA(ch, startAddr, pBuf, dataSize);
        if (ret != MSDC_LASTERROR_OK) {
            Printf("--> MSDC_ReadBlock_DMA error <--\n");
            return MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_ERRORS;
        }
    }

    #if (MSDC_AUTOCMD12_EN == 0)
    ret = MSDC_Send_Cmd(ch,COM_CMD12_STOP_TRANSMISSION,0);
    if(ret != MSDC_CMD_OK) {
        return MSDC_LASTERROR_READBLOCK |MSDC_LASTERROR_ERRORS| 0x05;
    }
    #endif

    if(readtimeout == 1) {
        return MSDC_LASTERROR_READBLOCK |MSDC_LASTERROR_ERRORS| 0x06;
    }

    return MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK;
}

UINT32 MSDC_EMMC_EnterBoot1(UINT32 ch)
{
	UINT32 ret = 0;

	ret = MSDC_SetEXTCSD(ch, 179, EXT_CSD_179_BOOTPARTITION_BOOT1, EXT_CSD_179_BOOTPARTITION_MASK);
	if(ret != (MSDC_LASTERROR_SETEXTCSD | MSDC_LASTERROR_OK))
	{
		return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_ERRORS | 0x01;
	}

	return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK;
}

UINT32 MSDC_EMMC_EnterBoot2(UINT32 ch)
{
	UINT32 ret = 0;

	ret = MSDC_SetEXTCSD(ch, 179, EXT_CSD_179_BOOTPARTITION_BOOT2, EXT_CSD_179_BOOTPARTITION_MASK);
	if(ret != (MSDC_LASTERROR_SETEXTCSD | MSDC_LASTERROR_OK))
	{
		return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_ERRORS | 0x01;
	}

	return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK;
}

UINT32 MSDC_EMMC_EnterUser(UINT32 ch)
{
	UINT32 ret = 0;

	ret = MSDC_SetEXTCSD(ch, 179, EXT_CSD_179_USER_PARTITION, EXT_CSD_179_BOOTPARTITION_MASK);
	if(ret != (MSDC_LASTERROR_SETEXTCSD | MSDC_LASTERROR_OK))
	{
		return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_ERRORS | 0x01;
	}

	return MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK;
}
