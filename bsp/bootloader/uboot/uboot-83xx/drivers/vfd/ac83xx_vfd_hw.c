//#include "general.h"
//#include "hw_rs232.h"

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ac83xx_basic.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/mach-types.h>
#include <asm/arch/x_typedef.h>
#include <asm/arch/x_ckgen.h>
#include <ac83xx_gpio.h>
#include <asm/arch/drv_vfd_83xx.h>
#include <asm/arch/ac83xx_vfd_cus.h>

#if SUPPORT_CUSTOMER_VFD
#include <asm/arch/x_pdwnc.h>
#if CONFIG_EXTERNAL_MCU
#include <ext_mcu.h>
#endif



/******************************************************************************
* Local variable
******************************************************************************/
#define REG_RW_RESRV1 0x164
#define STDBY_CHECK_VALUE 0x24000164


/****************************************************************************
** Function prototypes
****************************************************************************/

/****************************************************************************
** static Function
****************************************************************************/
static void vStbyVfdDirectWrite(BYTE bAddr, BYTE bData);
static void vStbyVfdUpdate(UINT8 bTmp);
static void vStbyVfdSegDisplay(BYTE SegPos, BYTE bNum, BOOL fgDirect);
static void vStbyDirectSequenceUpd(BYTE bSize,BYTE *pbData, UINT32 wSegInfo);



const UINT8 _pbStbyVfdKeyScan[] = {
	IR_NONE,
};



const BYTE _pbStbyVfdSeg0[8][28] = 
{
    {0x0f, 0x20, 0x0f, 0x02, 0x0e, 0x20, 0x0e, 0x02, 0x0e, 0x10, 0x0f, 0x01, 0x0e, 0x40, 0x0f, 0x08, 0x0e, 0x80, 0x0f, 0x10, 0x0f, 0x10, 0x0e, 0x04, 0x0f, 0x04, 0x0e, 0x08, },
    {0x0d, 0x20, 0x0d, 0x02, 0x0c, 0x20, 0x0c, 0x02, 0x0c, 0x10, 0x0d, 0x01, 0x0c, 0x40, 0x0d, 0x08, 0x0c, 0x80, 0x0d, 0x10, 0x0d, 0x10, 0x0c, 0x04, 0x0d, 0x04, 0x0c, 0x08, },
    {0x0b, 0x20, 0x0b, 0x02, 0x0a, 0x20, 0x0a, 0x02, 0x0a, 0x10, 0x0b, 0x01, 0x0a, 0x40, 0x0b, 0x08, 0x0a, 0x80, 0x0b, 0x10, 0x0b, 0x10, 0x0a, 0x04, 0x0b, 0x04, 0x0a, 0x08, },
    {0x09, 0x20, 0x09, 0x02, 0x08, 0x20, 0x08, 0x02, 0x08, 0x10, 0x09, 0x01, 0x08, 0x40, 0x09, 0x08, 0x08, 0x80, 0x09, 0x10, 0x09, 0x10, 0x08, 0x04, 0x09, 0x04, 0x08, 0x08, },
    {0x07, 0x20, 0x07, 0x02, 0x06, 0x20, 0x06, 0x02, 0x06, 0x10, 0x07, 0x01, 0x06, 0x40, 0x07, 0x08, 0x06, 0x80, 0x07, 0x10, 0x07, 0x10, 0x06, 0x04, 0x07, 0x04, 0x06, 0x08, },
    {0x05, 0x20, 0x05, 0x02, 0x04, 0x20, 0x04, 0x02, 0x04, 0x10, 0x05, 0x01, 0x04, 0x40, 0x05, 0x08, 0x04, 0x80, 0x05, 0x10, 0x05, 0x10, 0x04, 0x04, 0x05, 0x04, 0x04, 0x08, },
    {0x03, 0x20, 0x03, 0x02, 0x02, 0x20, 0x02, 0x02, 0x02, 0x10, 0x03, 0x01, 0x02, 0x40, 0x03, 0x08, 0x02, 0x80, 0x03, 0x10, 0x03, 0x10, 0x02, 0x04, 0x03, 0x04, 0x02, 0x08, },
    {0x01, 0x20, 0x01, 0x02, 0x00, 0x20, 0x00, 0x02, 0x00, 0x10, 0x01, 0x01, 0x00, 0x40, 0x01, 0x08, 0x00, 0x80, 0x01, 0x10, 0x01, 0x10, 0x00, 0x04, 0x01, 0x04, 0x00, 0x08, },
};

const BYTE _pbStbyVfdMsg[][MAX_SEG_SEQ_NUM] = {
        {VFD_CHAR_P, VFD_CHAR_H, VFD_CHAR_I,VFD_CHAR_L, VFD_CHAR_I, VFD_CHAR_P,VFD_CHAR_S,CLR} // PHILIPS
       ,{VFD_CHAR_P, VFD_CHAR_L, VFD_CHAR_S, CLR, VFD_CHAR_W, VFD_CHAR_A, VFD_CHAR_I, VFD_CHAR_T}   // PLS WAIT
    //  ,{CLR, CLR, CLR, CLR, CLR, CLR, CLR, CLR},  // NULL
};


// The following data is for Character table.
const UINT16 _pwStbySegUnit[] = 
{
    0x003f, 0x0006, 0x015b, 0x014f, 0x0166, 
    0x016d, 0x017d, 0x0007, 0x017f, 0x016f, 
    0x0000, 0x0177, 0x0177, 0x070f, 0x017c, 
    0x0039, 0x0158, 0x060f, 0x015e, 0x0179, 
    0x017b, 0x0171, 0x0171, 0x013d, 0x016f, 
    0x0176, 0x0174, 0x0609, 0x0609, 0x001f, 
    0x001e, 0x1870, 0x1870, 0x0038, 0x0038, 
    0x10b6, 0x0554, 0x08b6, 0x0037, 0x003f, 
    0x015c, 0x0173, 0x0173, 0x083f, 0x0067, 
    0x0973, 0x0050, 0x016d, 0x016d, 0x0601, 
    0x0178, 0x003e, 0x001c, 0x0886, 0x3030, 
    0x063e, 0x041c, 0x3880, 0x3880, 0x016e, 
    0x016e, 0x3009, 0x3009, 0x0000, 0x0000, 
    0x0008, 0x0140, 0x0600, 0x3000, 0x0880, 
    0x0039, 0x000f, 0xffff, 0x0503, 0x0740,
};


static void vStbyVfdUpdate(UINT8 bTmp) 
{
  UINT8 i;

  for (i = 0; i < 8; i++)
  {
    vStbyVfdClk(FALSE);

    vStbyVfdCombineBit(bTmp);
    bTmp >>= 1;

    vStbyVfdClk(TRUE);
  }

  vStbyVfdData(TRUE);
}

/****************************************************************************
** public  Function
****************************************************************************/

void vStbyVFDInit(void) 
{ 
  #if SUPPORT_T8032_VFD
    if(STDBY_CHECK_VALUE != PDWNC_READ32(REG_RW_RESRV1)) // only init while ac on
  #endif
    {
        GPIO_Config(PIN_VFD_STB, OUTPUT, LOW);
        GPIO_Config(PIN_VFD_CLK, OUTPUT, LOW);
        GPIO_Config(PIN_VFD_DATA, OUTPUT, LOW);

        vStbyVfdStrobe(FALSE);         // begin to write the VFD command
        vStbyVfdUpdate(MODESET);
        vStbyVfdStrobe(TRUE);          // strobe pull high to release the mode set

        vStbyVfdStrobe(FALSE);        
        vStbyVfdUpdate(DISPMODE_ON);
        vStbyVfdStrobe(TRUE);         

        vStbyVfdDirectClrAll();

        #if (!SUPPORT_T8032_VFD)
        if(STDBY_CHECK_VALUE == PDWNC_READ32(REG_RW_RESRV1))
        {
            vStbyVfdShowMsg(SEG_MSG_PHILIPS, TRUE);        
            printf("[uboot_vfd] Showing 'PHILIPS'.\n");
        }
        else
        #endif
        {
            #if CONFIG_EXTERNAL_MCU        
            if(ext_mcu_vfd_show())
            {
                vStbyVfdShowMsg(SEG_MSG_PHILIPS, TRUE);        
                printf("[uboot_vfd] Showing 'PHILIPS'.\n");
            }
            else
            #endif
            {
                vStbyVfdShowMsg(SEG_MSG_WAIT, TRUE);        
                printf("[uboot_vfd] Showing 'PLS WAIT'.\n");
            }
        }
   }
}

#else


/******************************************************************************
* Local variable
******************************************************************************/


/****************************************************************************
** Function prototypes
****************************************************************************/

/****************************************************************************
** static Function
****************************************************************************/
static void vStbyVfdDirectWrite(BYTE bAddr, BYTE bData);
static void vStbyVfdUpdate(UINT8 bTmp);
#if 0
static void vStbyVfdSegDisplay(BYTE SegPos, BYTE bNum, BOOL fgDirect);
static void vStbyDirectSequenceUpd(BYTE bSize,BYTE *pbData, UINT32 wSegInfo);
#endif


const UINT8 _pbStbyVfdKeyScan[] = {
	IR_RECORD, IR_FF, IR_CH_UP, IR_SW13,
	IR_PLAY, IR_FR, IR_CH_DOWN, IR_SW14,
	IR_STOP, IR_NEXT, IR_POWER, IR_SW15,
	IR_PAUSE, IR_PREV, IR_EJECT, IR_SW16
};



const BYTE _pbStbyVfdSeg0[11][28] = 
{
  {0x0a, 0x20, 0x0a, 0x10, 0x09, 0x20, 0x09, 0x04, 0x09, 0x02, 0x0a, 0x01, 0x09, 0x40, 0x0a, 0x04, 0x09, 0x80, 0x0a, 0x02, 0x0a, 0x02, 0x09, 0x10, 0x0a, 0x08, 0x09, 0x01, },
  {0x07, 0x20, 0x07, 0x10, 0x06, 0x20, 0x06, 0x04, 0x06, 0x02, 0x07, 0x01, 0x06, 0x40, 0x07, 0x04, 0x06, 0x80, 0x07, 0x02, 0x07, 0x02, 0x06, 0x10, 0x07, 0x08, 0x06, 0x01, },
  {0x0d, 0x20, 0x0d, 0x10, 0x0c, 0x20, 0x0c, 0x04, 0x0c, 0x02, 0x0d, 0x01, 0x0c, 0x40, 0x0d, 0x04, 0x0c, 0x80, 0x0d, 0x02, 0x0d, 0x02, 0x0c, 0x10, 0x0d, 0x08, 0x0c, 0x01, },
  {0x10, 0x20, 0x10, 0x10, 0x0f, 0x20, 0x0f, 0x04, 0x0f, 0x02, 0x10, 0x01, 0x0f, 0x40, 0x10, 0x04, 0x0f, 0x80, 0x10, 0x02, 0x10, 0x02, 0x0f, 0x10, 0x10, 0x08, 0x0f, 0x01, },
  {0x13, 0x20, 0x13, 0x10, 0x12, 0x20, 0x12, 0x04, 0x12, 0x02, 0x13, 0x01, 0x12, 0x40, 0x13, 0x04, 0x12, 0x80, 0x13, 0x02, 0x13, 0x02, 0x12, 0x10, 0x13, 0x08, 0x12, 0x01, },
  {0x16, 0x20, 0x16, 0x10, 0x15, 0x20, 0x15, 0x04, 0x15, 0x02, 0x16, 0x01, 0x15, 0x40, 0x16, 0x04, 0x15, 0x80, 0x16, 0x02, 0x16, 0x02, 0x15, 0x10, 0x16, 0x08, 0x15, 0x01, },
  {0x19, 0x20, 0x19, 0x10, 0x18, 0x20, 0x18, 0x04, 0x18, 0x02, 0x19, 0x01, 0x18, 0x40, 0x19, 0x04, 0x18, 0x80, 0x19, 0x02, 0x19, 0x02, 0x18, 0x10, 0x19, 0x08, 0x18, 0x01, },
  {0x1c, 0x20, 0x1c, 0x10, 0x1b, 0x20, 0x1b, 0x04, 0x1b, 0x02, 0x1c, 0x01, 0x1b, 0x40, 0x1c, 0x04, 0x1b, 0x80, 0x1c, 0x02, 0x1c, 0x02, 0x1b, 0x10, 0x1c, 0x08, 0x1b, 0x01, },
  {0x1f, 0x20, 0x1f, 0x10, 0x1e, 0x20, 0x1e, 0x04, 0x1e, 0x02, 0x1f, 0x01, 0x1e, 0x40, 0x1f, 0x04, 0x1e, 0x80, 0x1f, 0x02, 0x1f, 0x02, 0x1e, 0x10, 0x1f, 0x08, 0x1e, 0x01, },
  {0x22, 0x20, 0x22, 0x10, 0x21, 0x20, 0x21, 0x04, 0x21, 0x02, 0x22, 0x01, 0x21, 0x40, 0x22, 0x04, 0x21, 0x80, 0x22, 0x02, 0x22, 0x02, 0x21, 0x10, 0x22, 0x08, 0x21, 0x01, },
  {0x25, 0x20, 0x25, 0x10, 0x24, 0x20, 0x24, 0x04, 0x24, 0x02, 0x25, 0x01, 0x24, 0x40, 0x25, 0x04, 0x24, 0x80, 0x25, 0x02, 0x25, 0x02, 0x24, 0x10, 0x25, 0x08, 0x24, 0x01, },
};

const BYTE _pbStbyVfdMsg[][MAX_SEG_SEQ_NUM] = {
	{VFD_CHAR_H, VFD_CHAR_E, VFD_CHAR_L, VFD_CHAR_L, VFD_CHAR_O, CLR, CLR, CLR, CLR, CLR, CLR},	// hello 
	{VFD_CHAR_S, VFD_CHAR_T, VFD_CHAR_O,VFD_CHAR_P, CLR, CLR, CLR, CLR, CLR, CLR,CLR},		   // stop 
	{VFD_CHAR_O, VFD_CHAR_P, VFD_CHAR_E, VFD_CHAR_N,CLR, CLR, CLR, CLR, CLR, CLR, CLR},				// open, 
	{VFD_CHAR_C, VFD_CHAR_L, VFD_CHAR_O, VFD_CHAR_S, VFD_CHAR_E, CLR, CLR, CLR, CLR, CLR, CLR},	// close, 
	{VFD_CHAR_O, VFD_CHAR_F, VFD_CHAR_F, CLR, CLR, CLR, CLR, CLR, CLR, CLR, CLR},							// OFF, 
	{VFD_CHAR_E, VFD_CHAR_R, VFD_CHAR_R, CLR, CLR, CLR, CLR, CLR, CLR, CLR, CLR},								// err, 
	{VFD_CHAR_H, VFD_CHAR_E, VFD_CHAR_L,VFD_CHAR_L, VFD_CHAR_O, CLR,VFD_CHAR_W,VFD_CHAR_O,VFD_CHAR_R, VFD_CHAR_L,VFD_CHAR_D},// HELLO WORLD, 
	{VFD_CHAR_U, CLR,VFD_CHAR_B, VFD_CHAR_O, VFD_CHAR_O, VFD_CHAR_T,CLR,CLR,CLR,CLR,CLR},	// U_BOOT, 
	{CLR, CLR, CLR, CLR, CLR, CLR, CLR, CLR, CLR, CLR, CLR} // clean all, 10: start 93
};


// The following data is for Character table.
const UINT16 _pwStbySegUnit[] = 
{
  0x003f, 0x0006, 0x015b, 0x014f, 0x0166, 
  0x016d, 0x017d, 0x0007, 0x017f, 0x016f, 
  0x0000, 0x0177, 0x0177, 0x070f, 0x017c, 
  0x0039, 0x0158, 0x060f, 0x015e, 0x0179, 
  0x017b, 0x0171, 0x0171, 0x013d, 0x016f, 
  0x0176, 0x0174, 0x0609, 0x0609, 0x001f, 
  0x001e, 0x1870, 0x1870, 0x0038, 0x0038, 
  0x10b6, 0x0554, 0x08b6, 0x0037, 0x003f, 
  0x015c, 0x0173, 0x0173, 0x083f, 0x0067, 
  0x0973, 0x0050, 0x016d, 0x016d, 0x0601, 
  0x0178, 0x003e, 0x001c, 0x0886, 0x3030, 
  0x063e, 0x041c, 0x3880, 0x3880, 0x016e, 
  0x016e, 0x3009, 0x3009, 0x0000, 0x0000, 
  0x0008, 0x0140, 0x0600, 0x3000, 0x0880, 
  0x0039, 0x000f, 0x3fff, 0x0503, 
};

void vStbyVfdKeyScan(void) 
{
  UINT8 i;
  UINT8 bData;
  UINT8  _bStbyVfdKeyScan = IR_NONE;
  
  vStbyVfdStrobe(FALSE);       
  vStbyVfdUpdate(KEY_SCAN);
  vStbyVfdClk(TRUE);         

  vStbyVfdData(TRUE);        
  vStbyVfdClk(TRUE);

  for(i = 0; i < KEY_SCAN_SIZE; i++)     
  {
    vStbyVfdClk(FALSE);
    vStbyVfdClk(FALSE);  
    vStbyVfdClk(TRUE);

    // get the bits from the datout pin
    vStbyVfdDataIn(bData);
    if(bData)
    {
        _bStbyVfdKeyScan = _pbStbyVfdKeyScan[i];
    }
    vStbyVfdClk(TRUE);   
  }

  vStbyVfdStrobe(TRUE);     

  if(IR_NONE != _bStbyVfdKeyScan)
  {
  	printf("_bStbyVfdKeyScan = %d !\n", _bStbyVfdKeyScan);
  }
  
}

static void vStbyVfdUpdate(UINT8 bTmp) 
{
  UINT8 i;

  for (i = 0; i < 8; i++)
  {
    vStbyVfdClk(FALSE);

    vStbyVfdCombineBit(bTmp);
    bTmp >>= 1;

    vStbyVfdClk(TRUE);
  }

  vStbyVfdData(TRUE);
}

/****************************************************************************
** public  Function
****************************************************************************/

void vStbyVFDInit(void) 
{ 
  GPIO_Config(PIN_VFD_STB, OUTPUT, LOW);
  GPIO_Config(PIN_VFD_CLK, OUTPUT, LOW);
  GPIO_Config(PIN_VFD_DATA, OUTPUT, LOW);

  vStbyVfdStrobe(FALSE);         // begin to write the VFD command
  vStbyVfdUpdate(MODESET);
  vStbyVfdStrobe(TRUE);          // strobe pull high to release the mode set

  vStbyVfdStrobe(FALSE);        
  vStbyVfdUpdate(DISPMODE_ON);
  vStbyVfdStrobe(TRUE);         
  
  printf("SEG_MSG_UBOOT !\n");
  vStbyVfdDirectClrAll();
  vStbyVfdShowMsg(SEG_MSG_UBOOT, TRUE);

}
#endif

void vStbyVfdDirectClrAll(void) 
{
  BYTE i;

  for(i = 0; i < MAX_VFD_ADDR; i++)
  {
    vStbyVfdDirectWrite(i, 0);
  }

}

void vStbyVfdDirectSetAll(void) 
{
  BYTE i;

  for(i = 0; i < MAX_VFD_ADDR; i++)
  {
    vStbyVfdDirectWrite(i, 0xff);
  }

}

#if 0
static void vStbyDirectSequenceUpd(BYTE bSize,BYTE *pbData, UINT32 wSegInfo) 
{
  BYTE bAddr1 = 0xff, bAddr2 = 0xff;
  BYTE bData1 = 0, bData2 = 0;
  BYTE i;
  BYTE bTotalCnt = 0;
  UINT32 wUpdateData;

	 bAddr1 = pbData[0];
	 
	 for(i = 0; i < bSize; i ++)
	 {
		wUpdateData = wSegInfo >>i;
		if(0x01 & wUpdateData)
		{
			if(pbData[2*i] == bAddr1)
			{
				bData1 = bData1 | pbData[2*i+1];
			}
			else
			{
				bAddr2 = pbData[2*i];
				bData2 = bData2 | pbData[2*i+1];
			}
		}
		else
		{
			if(pbData[2*i] != bAddr1)
			{
				bAddr2 = pbData[2*i];
			}
		}
	 }
	 
        vStbyVfdDirectWrite(bAddr1, bData1);
	 if(0xff != bAddr2)
	 {
        vStbyVfdDirectWrite(bAddr2, bData2);
	 }
}	

static void vStbyVfdSegDisplay(BYTE SegPos, BYTE bNum, BOOL fgDirect) 
{
  UINT32 wSegInfo;


  wSegInfo = _pwStbySegUnit[bNum];
  if(fgDirect)
  {
    vStbyDirectSequenceUpd(SEG_SIZE, _pbStbyVfdSeg0[SegPos], wSegInfo);  
  }
  else
  {
///    vVfdSequenceUpdate(SEG_SIZE, _pbVfdSeg0[SegPos], wSegInfo);  
	//NO NEED TO APPLY QUEUE DESIGN IN STANTDBY MODE
  }
  
//  vVfdWriteQueue(bAddr, bData);  
}
#endif
void vStbyVfdShowMsg(BYTE bMsgIdIdx, BOOL fgDirect) 
{
  BYTE i = 0,j = 0;
  BYTE pbAddr[MAX_VFD_ADDR];
  BYTE pbData[MAX_VFD_ADDR];
  UINT16 wSegInfo = 0x00;
  UINT16 wUpdateData;
  
  for(i = 0; i < MAX_VFD_ADDR; i++)
  {
	pbAddr[i] = 0xff;
	pbData[i] = 0x00;
  }
	
  for(i = 0; i < MAX_SEG_SEQ_NUM; i++)
{

    wSegInfo = _pwStbySegUnit[_pbStbyVfdMsg[bMsgIdIdx][i]];

	 for(j = 0; j < SEG_SIZE; j ++)
	 {
		wUpdateData = wSegInfo >>j;
		if(0x01 & wUpdateData)
		{
			if(0xff == pbAddr[_pbStbyVfdSeg0[i][2*j]])
			{
				pbAddr[_pbStbyVfdSeg0[i][2*j]] = _pbStbyVfdSeg0[i][2*j];
			}
			pbData[_pbStbyVfdSeg0[i][2*j]] = pbData[_pbStbyVfdSeg0[i][2*j]] | (_pbStbyVfdSeg0[i][2*j+1]);
		}
		else
		{
			
		}
	 }
  }
  for(i = 0; i < MAX_VFD_ADDR; i++)
  {
  	if(0xff != pbAddr[i])
  	{
	  	vStbyVfdDirectWrite(pbAddr[i], pbData[i]);
  	}
  }
  	
}

static void vStbyVfdDirectWrite(BYTE bAddr, BYTE bData) 
{
  if(bAddr == 0xff)
  {
    return;
  }
  vStbyVfdStrobe(FALSE);
  vStbyVfdUpdate(DATA_SET_INC) ;  // issue the write data command
  vStbyVfdStrobe(TRUE);

  
  vStbyVfdStrobe(FALSE);
  vStbyVfdUpdate(ADDR_SET | bAddr);  // after setting the address, the strobe need not to pull high
  vStbyVfdStrobe(FALSE);

  vStbyVfdUpdate(bData);   // the final data is written , strobe pull high
  vStbyVfdStrobe(TRUE);
  
}

void vStbyDelay(UINT32 wTicks)
{
    UINT32 i;
	for(i = 0; i < wTicks; i++)
	{
		udelay(1000);
	}
}

