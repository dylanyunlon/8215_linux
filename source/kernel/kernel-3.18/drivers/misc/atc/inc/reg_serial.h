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

 #ifndef _REG_SERIAL_H
 #define _REG_SERIAL_H

#include "mach/base_regs.h"

/*************************************************************************/ 
#define UART1_PIN_MUX_OFFSET    (0xC0)



/*************************************************************************/
/*   UART                                                             */
/*************************************************************************/ 
#define SERIAL1_BASE              (SERIAL_BASE + 0x20)
#define UART0_INT_ID_OFFSET			0x10
#define REG_STATUS_RD_ALLOW     0x1
#define REG_STATUS_WR_ALLOW     0x2
#define REG_STATUS_END_ERR      (0x1 << 4)
#define REG_STATUS_P_ERR        (0x1 << 5)
#define REG_STATUS_TRANS_MODE   (0x1 << 6)
#define REG_MR_DTR              (0x1 << 0)
#define REG_MR_RTS              (0x1 << 1)
#define REG_MR_DSR              (0x1 << 8)
#define REG_MR_CTS              (0x1 << 9)
#define REG_MR_DCD              (0x1 << 11)
#define REG_CCR_STOP_BIT       	(0x1 << 2)


#define UART_PORT_0  0
#define UART_PORT_1  1
#define UART_PORT_2  2
#define UART_PORT_3  3
#define UART_PORT_4  4
#define UART_PORT_5  5
#define UART_PORT_6  6
#define UART_PORT_0_INDEX  0
#define UART_PORT_1_INDEX  1
#define UART_PORT_2_INDEX  2
#define UART_PORT_3_INDEX  3
#define UART_PORT_4_INDEX  4
#define UART_PORT_5_INDEX  5
#define UART_PORT_6_INDEX  6

#define DBG_PORT UART_PORT_0

/*************************************************************************/
/*   define UART1 PARITY                                                     */
/*************************************************************************/ 

#define REG_PARITY_U1_ON                (0x1<<4)
#define REG_PARITY_U1_OFF               (0x0<<4)
#define REG_PARITY_ON                   (0x1<<12)
#define REG_PARITY_OFF                  (0x0<<12)
#define REG_PARITY_EVEN                 (0x1 << 13)
#define REG_PARITY_ODD                  (0x0 << 13)
#define REG_PARITY_U1_EVEN              (0x1 << 5)
#define REG_PARITY_U1_ODD       	      (0x0 << 5)
/************************************************************************/

/*************************************************************************/
/*   define baudRate                                                     */
/*************************************************************************/ 
#define UART_115200            0
#define UART_230400            1
#define UART_460800            2
#define UART_921600            3
#define UART_57600             4
#define UART_38400             5
#define UART_19200             6
#define UART_9600              7
#define UART_28800              5
#define UART_9600_0             6
#define UART_3200000            8



/*************************************************************************/
/*   Interrupt ID                                                    */
/*************************************************************************/ 
#define UART_Rx_Error          (0x1 << 0)
#define UART_Rx_Buffer1        (0x1 << 1)
#define UART_Timeout           (0x1 << 2)
#define UART_Tx_Buffer1        (0x1 << 3)
#define UART_OverRunEn         (0x1 << 4)
#define UART_Tx_DMA1           (0x1 << 5)
#define UART_Rx_DMA1           (0x1 << 6)
#define UART_Tx_Buffer0        (0x1 << 5)
#define UART_Tx_Dacepted       (0x1 << 6)
#define UART_Rx_Buffer0        (0x1 << 7)


typedef volatile struct {
	union
	{
		struct{
			UINT32 PORT;
			UINT32 STATUS;
			UINT32 FLASH_EN;
			UINT32 INT_EN;
			UINT32 INT_ID;
			UINT32 RESERVE1;
			UINT32 RESERVE2;
			UINT32 RS232_PK;
		};
		struct{
			UINT8 U1_PORT;
			UINT32 U1_Data1;
			UINT32 U1_CCR;
			UINT32 U1_STATUS;
			UINT32 U1_BCR;
			UINT32 U1_MR;
			UINT32 U1_IEN;
			UINT32 U1_IST;
			UINT32 U1_DMA_W;
			UINT32 U1_DMA_W_O;
			UINT32 U1_DMA_R;
			UINT32 U1_DMA_R_O;	
		};
		struct{
			UINT32 U2_Data0;
			UINT32 U2_Data1;
			UINT32 U2_CCR;
			UINT32 U2_STATUS;
			UINT32 U2_BCR;
			UINT32 U2_MR;
			UINT32 U2_IEN;
			UINT32 U2_IST;
			UINT32 U2_DMA_W;
			UINT32 U2_DMA_W_O;
			UINT32 U2_DMA_R;
			UINT32 U2_DMA_R_O;	
		};
		struct{
			UINT32 U3_Data0;
			UINT32 U3_Data1;
			UINT32 U3_CCR;
			UINT32 U3_STATUS;
			UINT32 U3_BCR;
			UINT32 U3_MR;
			UINT32 U3_IEN;
			UINT32 U3_IST;
			UINT32 U3_DMA_W;
			UINT32 U3_DMA_W_O;
			UINT32 U3_DMA_R;
			UINT32 U3_DMA_R_O;	
		};
		struct{
			UINT32 U4_Data0;
			UINT32 U4_Data1;
			UINT32 U4_CCR;
			UINT32 U4_STATUS;
			UINT32 U4_BCR;
			UINT32 U4_MR;
			UINT32 U4_IEN;
			UINT32 U4_IST;
			UINT32 U4_DMA_W;
			UINT32 U4_DMA_W_O;
			UINT32 U4_DMA_R;
			UINT32 U4_DMA_R_O;	
		};
		struct{
			UINT32 U5_Data0;
			UINT32 U5_Data1;
			UINT32 U5_CCR;
			UINT32 U5_STATUS;
			UINT32 U5_BCR;
			UINT32 U5_MR;
			UINT32 U5_IEN;
			UINT32 U5_IST;
			UINT32 U5_DMA_W;
			UINT32 U5_DMA_W_O;
			UINT32 U5_DMA_R;
			UINT32 U5_DMA_R_O;	
		};
		struct{
			UINT32 U6_Data0;
			UINT32 U6_Data1;
			UINT32 U6_CCR;
			UINT32 U6_STATUS;
			UINT32 U6_BCR;
			UINT32 U6_MR;
			UINT32 U6_IEN;
			UINT32 U6_IST;
			UINT32 U6_DMA_W;
			UINT32 U6_DMA_W_O;
			UINT32 U6_DMA_R;
			UINT32 U6_DMA_R_O;	
		};
	};
}AC83XX_UART_REG;


#define REG_SER_IRQ_EN			(UINT32)(0x1<<IRQ_SERIAL_SHIFT)

#define REG_SER_PORT(x)                  (*((volatile UINT8*)(x + 0x00)))
#define REG_SER_STATUS(x)                (*((volatile UINT16*)(x + 0x04)))
  #define SER_READ_ALLOW                0x1
  #define SER_WRITE_ALLOW               0x2
   
#define REG_SER01_PORT(x)                (*((volatile UINT8*)(x + 0x00)))
#define REG_SER01_STATUS(x)              (*((volatile UINT32*)(x + 0xCC)))

#define REG_SER_INT_EN(x)                (*((volatile UINT32*)(x + 0x0C)))
#define REG_SER_INT_STATUS(x)            (*((volatile UINT32*)(x + 0x10)))
  #define REG_SER_INT_EN_TX_D_ACEPTED   (UINT32)(0x1<<6)
  #define REG_SER_INT_EN_READ_ALLOW     (UINT32)(0x1<<7)

// For UART Interrupt mode
#define UART_INT_BUF_SIZE		1024
#define SER_PORT_0  (0)

// the default value of the last data in image.
#define MAGIC_NUM (0xc0cac01a)

#endif


