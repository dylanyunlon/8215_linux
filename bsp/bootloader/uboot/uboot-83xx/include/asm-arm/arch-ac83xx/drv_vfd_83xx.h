#include <ac83xx_gpio.h>
#include <asm/arch/x_typedef.h>

#ifndef _VFD_HW_H_
#define _VFD_HW_H_

/*----------------------------------------------------------------------------
 *---------------------------------------------------------------------------*/
#if 1

#define	VFD_STB_OUT(fgFlag)  GPIO_Output(PIN_VFD_STB, fgFlag)

#define	VFD_CLK_OUT(fgFlag)  GPIO_Output(PIN_VFD_CLK, fgFlag)

#define	VFD_DATA_OUT(fgFlag)  GPIO_Output(PIN_VFD_DATA, fgFlag); \
                              GPIO_InOut_Sel(PIN_VFD_DATA, OUTPUT)
                              
#define	VFD_DATA_IN(bData)  GPIO_InOut_Sel(PIN_VFD_DATA, INPUT); \
                            bData = GPIO_Input(PIN_VFD_DATA)
                            
#else

#define	VFD_STB_OUT(fgFlag)  XBYTE[0xD8] = ((fgFlag == 0) ? (XBYTE[0xD8] & 0xFE) : (XBYTE[0xD8] | 0x01))

#define	VFD_CLK_OUT(fgFlag)  XBYTE[0xD8] = ((fgFlag == 0) ? (XBYTE[0xD8] & 0xFD) : (XBYTE[0xD8] | 0x02))

#define	VFD_DATA_OUT(fgFlag)  XBYTE[0xD8] = ((fgFlag == 0) ? (XBYTE[0xD8] & 0xFB) : (XBYTE[0xD8] | 0x04)); \
                              XBYTE[0xD4] = (XBYTE[0xD4] | 0x04)
                              
#define	VFD_DATA_IN(bData)  XBYTE[0xD4] = (XBYTE[0xD4] & 0xFB); \
                            bData = (((XBYTE[0xD0] & 0x04) == 0) ?  0 : 1 )
                            
#endif

/*----------------------------------------------------------------------------
 *---------------------------------------------------------------------------*/

#define vStbyVfdStrobe(fgFlag)  { \
   VFD_STB_OUT(fgFlag); \
   udelay(1);             \
}

#define vStbyVfdClk(fgFlag)  { \
   VFD_CLK_OUT(fgFlag); \
	udelay(1);			   \
} 

#define vStbyVfdData(fgFlag)  { \
   VFD_DATA_OUT(fgFlag); \
	udelay(1);			   \
} 

#define  vStbyVfdDataIn(bData)   { \   
   VFD_DATA_IN(bData); \
	udelay(1);			   \
}  

#define vStbyVfdCombineBit(bData) vStbyVfdData((bData & 0x1)); 

extern void vStbyVFDInit(void) ;
extern void vStbyVfdDirectClrAll(void) ;
extern void vStbyVfdDirectSetAll(void) ;
extern void vStbyVfdShowMsg(BYTE bMsgId, BOOL fgDirect) ;
extern void vStbyDelay(UINT32 wTicks);
extern void vStbyVfdKeyScan(void) ;

#endif
