/*-----------------------------------------------------------------------------
 * Copyright (c) 2005, MediaTek Inc.
 * All rights reserved.
 *
 * Unauthorized use, practice, perform, copy, distribution, reproduction,
 * or disclosure of this information in whole or in part is prohibited.
 *---------------------------------------------------------------------------
 *
 * $Author: jianghong.lin $
 * $Date: 2015/07/01 $
 * $RCSfile: serial_ac83xx.h,v $
 * $Revision: #1 $
 *
 *---------------------------------------------------------------------------*/

 #ifndef _SERIAL_AC83XX_H
 #define _SERIAL_AC83XX_H

#include <asm/arch/ac83xx_basic.h>


#define UART_PORT_0  0
#define UART_PORT_1  1

#define DBG_PORT UART_PORT_0

//For AC83XX UART Register Define


#define REG_SER_IRQ_EN			(UINT32)(0x1<<IRQ_SERIAL_SHIFT)

#define REG_SER_PORT                  (*((volatile UINT8*)(SERIAL_BASE + 0x00)))
#define REG_SER_STATUS                (*((volatile UINT32*)(SERIAL_BASE + 0x04)))
  #define SER_READ_ALLOW                0x1
  #define SER_WRITE_ALLOW               0x2
   
#define REG_SER01_PORT                (*((volatile UINT8*)(SERIAL_BASE + 0x00)))
#define REG_SER01_STATUS              (*((volatile UINT32*)(SERIAL_BASE + 0xCC)))

#define REG_SER_INT_EN                (*((volatile UINT32*)(SERIAL_BASE + 0x0C)))
#define REG_SER_INT_STATUS            (*((volatile UINT32*)(SERIAL_BASE + 0x10)))
  #define REG_SER_INT_EN_TX_D_ACEPTED   (UINT32)(0x1<<6)
  #define REG_SER_INT_EN_READ_ALLOW     (UINT32)(0x1<<7)

// For UART Interrupt mode
#define UART_INT_BUF_SIZE		1024
#define SER_PORT_0  (0)

// the default value of the last data in image.
#define MAGIC_NUM (0xc0cac01a)

#endif

