/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

//============================================================================
// Include files
//============================================================================
#include "x_serial.h"
#include "x_bim.h"
#include "x_hal_ic.h"

//#include "platform.h"
#include <base_regs.h>
#include "reg_serial.h"
#include "printf.h"

#define TAGS "ARM2-UART"

/* uart interrupt mode for UART1 to UART5 */
#define SOURCE_CLOCK            32400000
#define BUF_SIZE                1024
#define U_W_CLR_RXBUF			(1 << 14)
#define U_W_CLR_TXBUF			(1 << 15)

#define NOPARITY            0
#define ODDPARITY           1
#define EVENPARITY          2
#define MARKPARITY          3
#define SPACEPARITY         4

#define ONESTOPBIT          0
#define ONE5STOPBITS        1
#define TWOSTOPBITS         2

static AC83XX_UART_REG *g_pUartRegs = NULL;

struct uart_transfer {
	unsigned int rp;
	unsigned int wp;
	unsigned int flags;
#define BUF_CIRCLED 	(1 << 0)
#define BUF_OVERFLOWED 	(1 << 1)
#define BUF_EMPTIED     (1 << 2)
	char data[BUF_SIZE];
};

static struct uart_transfer monitor_rxbuf = {0};

int readByte(UINT32 u4Index)
{
    int ch;

    switch (u4Index)
    {
        case 1:
			if ((g_pUartRegs->U1_STATUS) & 0x1FF) {
				ch = (int)(g_pUartRegs->U1_PORT);
			} else {
				ch = -1;
			}
            break;

        case 2:
			if ((g_pUartRegs->U2_STATUS) & 0x1FF) {
				ch = (int)(g_pUartRegs->U2_Data0);
			} else {
				ch = -1;
			}
            break;

        case 3:
			if ((g_pUartRegs->U3_STATUS) & 0x1FF) {
				ch = (int)(g_pUartRegs->U3_Data0);
			} else {
				ch = -1;
			}
            break;

        case 4:
			if ((g_pUartRegs->U4_STATUS) & 0x1FF) {
				ch = (int)(g_pUartRegs->U4_Data0);
			} else {
				ch = -1;
			}
            break;

        case 5:
			if ((g_pUartRegs->U5_STATUS) & 0x1FF) {
				ch = (int)(g_pUartRegs->U5_Data0);
			} else {
				ch = -1;
			}
            break;

        default:
            pr_info("readByte u4Index=%d not support.\n", u4Index);
            break;
    }

    return ch;
}

BOOL writeByte(UINT32 u4Index, BYTE bData)
{
    switch (u4Index)
    {
        case 1:
            while (TRUE) {
                // Wait while FIFO has room
                // Get line status register
                if ((g_pUartRegs->U1_STATUS) & 0x1FF000) {
                    g_pUartRegs->U1_PORT = bData;
                    break;
                }
            }
            break;

        case 2:
            while (TRUE) {
                // Wait while FIFO has room
                // Get line status register
                if ((g_pUartRegs->U2_STATUS) & 0x1FF000) {
                    g_pUartRegs->U2_Data0 = bData;
                    break;
                }
            }
            break;

        case 3:
            while (TRUE) {
                // Wait while FIFO has room
                // Get line status register
                if ((g_pUartRegs->U3_STATUS) & 0x1FF000) {
                    g_pUartRegs->U3_Data0 = bData;
                    break;
                }
            }
            break;

        case 4:
            while (TRUE) {
                // Wait while FIFO has room
                // Get line status register
                if ((g_pUartRegs->U4_STATUS) & 0x1FF000) {
                    g_pUartRegs->U4_Data0 = bData;
                    break;
                }
            }
            break;

        case 5:
            while (TRUE) {
                // Wait while FIFO has room
                // Get line status register
                if ((g_pUartRegs->U5_STATUS) & 0x1FF000) {
                    g_pUartRegs->U5_Data0 = bData;
                    break;
                }
            }
            break;

        default:
            pr_info("writeByte u4Index=%d not support.\n", u4Index);
            break;
    }

    return TRUE;
}

BOOL writeBlockData(UINT32 u4Index, BYTE *pData, UINT32 u4ByteToWrite)
{
    switch (u4Index)
    {
        case 1:
			while (u4ByteToWrite > 0) {
				// Wait while FIFO has room
				// Get line status register
				if ((g_pUartRegs->U1_STATUS) & 0x1FF000) {
					g_pUartRegs->U1_PORT = *pData++;
					u4ByteToWrite--;
				}
			}
            break;

        case 2:
			while (u4ByteToWrite > 0) {
				// Wait while FIFO has room
				// Get line status register
				if ((g_pUartRegs->U2_STATUS) & 0x1FF000) {
					g_pUartRegs->U2_Data0 = *pData++;
					u4ByteToWrite--;
				}
			}
            break;

        case 3:
			while (u4ByteToWrite > 0) {
				// Wait while FIFO has room
				// Get line status register
				if ((g_pUartRegs->U3_STATUS) & 0x1FF000) {
					g_pUartRegs->U3_Data0 = *pData++;
					u4ByteToWrite--;
				}
			}
            break;

        case 4:
			while (u4ByteToWrite > 0) {
				// Wait while FIFO has room
				// Get line status register
				if ((g_pUartRegs->U4_STATUS) & 0x1FF000) {
					g_pUartRegs->U4_Data0 = *pData++;
					u4ByteToWrite--;
				}
			}
            break;

        case 5:
			while (u4ByteToWrite > 0) {
				// Wait while FIFO has room
				// Get line status register
				if ((g_pUartRegs->U5_STATUS) & 0x1FF000) {
					g_pUartRegs->U5_Data0 = *pData++;
					u4ByteToWrite--;
				}
			}
            break;

        default:
            pr_info("writeBlockData u4Index=%d not support.\n", u4Index);
            break;
    }

    return TRUE;
}

BOOL readBlockData(UINT32 u4Index, BYTE *pData, UINT32 u4ByteToRead)
{
    UCHAR rxChar;

    switch (u4Index)
    {
        case 1:
			while (u4ByteToRead > 0) {
				if ((g_pUartRegs->U1_STATUS) & 0x1FF) {
					rxChar = g_pUartRegs->U1_PORT;
					*pData++ = rxChar;
					u4ByteToRead--;
				}
			}
            break;

        case 2:
			while (u4ByteToRead > 0) {
				if ((g_pUartRegs->U2_STATUS) & 0x1FF) {
					rxChar = g_pUartRegs->U2_Data0;
					*pData++ = rxChar;
					u4ByteToRead--;
				}
			}
            break;

        case 3:
			while (u4ByteToRead > 0) {
				if ((g_pUartRegs->U3_STATUS) & 0x1FF) {
					rxChar = g_pUartRegs->U3_Data0;
					*pData++ = rxChar;
					u4ByteToRead--;
				}
			}
            break;

        case 4:
			while (u4ByteToRead > 0) {
				if ((g_pUartRegs->U4_STATUS) & 0x1FF) {
					rxChar = g_pUartRegs->U4_Data0;
					*pData++ = rxChar;
					u4ByteToRead--;
				}
			}
            break;

        case 5:
			while (u4ByteToRead > 0) {
				if ((g_pUartRegs->U5_STATUS) & 0x1FF) {
					rxChar = g_pUartRegs->U5_Data0;
					*pData++ = rxChar;
					u4ByteToRead--;
				}
			}
            break;

        default:
            pr_info("readBlockData u4Index=%d not support.\n", u4Index);
            break;
    }

    return TRUE;
}

static BOOL setIntEnable(UINT32 u4Index, int enable)
{
    BOOL rc = FALSE;
    AC83XX_UART_REG *pUartRegs = g_pUartRegs;

    switch (u4Index)
    {
        case 1:
            if (enable) {
                pUartRegs->U1_IEN = 0x17;
            } else {
                pUartRegs->U1_IEN = 0;
            }
            break;

        case 2:
            if (enable) {
                pUartRegs->U2_IEN = 0x17;
            } else {
                pUartRegs->U2_IEN = 0;
            }
            break;

        case 3:
            if (enable) {
                pUartRegs->U3_IEN = 0x17;
            } else {
                pUartRegs->U3_IEN = 0;
            }
            break;

        case 4:
            if (enable) {
                pUartRegs->U4_IEN = 0x17;
            } else {
                pUartRegs->U4_IEN = 0;
            }
            break;

        case 5:
            if (enable) {
                pUartRegs->U5_IEN = 0x17;
            } else {
                pUartRegs->U5_IEN = 0;
            }
            break;

        default:
            pr_info("setIntEnable u4Index=%d not support.\n", u4Index);
            break;
    }

    rc = TRUE;

    return rc;
}

static BOOL setBufCtrl(UINT32 u4Index)
{
    BOOL rc = FALSE;
    AC83XX_UART_REG *pUartRegs = g_pUartRegs;

    switch (u4Index)
    {
        case 1:
            pUartRegs->U1_BCR = 0x80000FF;
            break;

        case 2:
            pUartRegs->U2_BCR = 0x40000FF;
            break;

        case 3:
            pUartRegs->U3_BCR = 0x40000FF;
            break;

        case 4:
            pUartRegs->U4_BCR = 0x40000FF;
            break;

        case 5:
            pUartRegs->U5_BCR = 0x40000FF;
            break;

        default:
            pr_info("setBufCtrl u4Index=%d not support.\n", u4Index);
            break;
    }

    rc = TRUE;

    return rc;
}

static BOOL setIntSta(UINT32 u4Index)
{
    BOOL rc = FALSE;
    AC83XX_UART_REG *pUartRegs = g_pUartRegs;

    switch (u4Index)
    {
        case 1:
            pUartRegs->U1_IST = 0x1F;
            break;

        case 2:
            pUartRegs->U2_IST = 0x1F;
            break;

        case 3:
            pUartRegs->U3_IST = 0x1F;
            break;

        case 4:
            pUartRegs->U4_IST = 0x1F;
            break;

        case 5:
            pUartRegs->U5_IST = 0x1F;
            break;

        default:
            pr_info("setIntSta u4Index=%d not support.\n", u4Index);
            break;
    }

    rc = TRUE;

    return rc;
}

int push_rxbuffer(char *buf, int count)
{
	struct uart_transfer *transfer = &monitor_rxbuf;
	unsigned int wp, rp;
	unsigned long data;

	wp = transfer->wp;
	rp = transfer->rp;
	data = (unsigned long)transfer->data;

	if (!(transfer->flags & BUF_CIRCLED)) {
		if (count < (int)(BUF_SIZE - wp)) {
			memcpy((void *)(data + wp), buf, count);
			transfer->wp += count;
			return count;
		} else {
			unsigned int copy_size = BUF_SIZE - wp;
			unsigned int left_size = count - copy_size;
			memcpy((void *)(data + wp), buf, copy_size);
			wp = 0;
			if (left_size > rp) {
				/* overflowed. some new data will be discarded */
				left_size = rp;
			}
			memcpy((void *)(data + wp), (void *)(buf + copy_size), left_size);
			transfer->wp = left_size;
			transfer->flags |= BUF_CIRCLED;
			return (copy_size + left_size);
		}
	} else {
		if (count > (int)(rp - wp)) {
			/* overflowed. some new data will be discarded */
			count = rp - wp;
		}
		memcpy((void *)(data + wp), buf, count);
		transfer->wp += count;
		return count;
	}
}

void uart1_irq_handler(void)
{
	char buffer[1024] = {0};
	int count = 0;

	setIntEnable(1, 0);
	v_disable_bim_irq(VECTOR_UART_1);

    if ((g_pUartRegs->U1_STATUS) & 0x1FF) {
		do
		{
			buffer[count++] = (char)readByte(1);
		} while((g_pUartRegs->U1_STATUS) & 0x1FF);
    }

	push_rxbuffer(buffer, count);
	//dprintf(INFO, "irq push count = %d\n",count);

	setIntSta(1);
	setIntEnable(1, 1);
    v_enable_bim_irq(VECTOR_UART_1);
}

void uart2_irq_handler(void)
{
	char buffer[1024] = {0};
	int count = 0;

	setIntEnable(2, 0);
	v_disable_bim_irq(VECTOR_UART_2);

    if ((g_pUartRegs->U2_STATUS) & 0x1FF) {
		do
		{
			buffer[count++] = (char)readByte(2);
		} while((g_pUartRegs->U2_STATUS) & 0x1FF);
    }

	push_rxbuffer(buffer, count);
	//dprintf(INFO, "irq push count = %d\n",count);

	setIntSta(2);
	setIntEnable(2, 1);
    v_enable_bim_irq(VECTOR_UART_2);
}

void uart3_irq_handler(void)
{
	char buffer[1024] = {0};
	int count = 0;

	setIntEnable(3, 0);
	v_disable_bim_irq(VECTOR_UART_3);

    if ((g_pUartRegs->U3_STATUS) & 0x1FF) {
		do
		{
			buffer[count++] = (char)readByte(3);
		} while((g_pUartRegs->U3_STATUS) & 0x1FF);
    }

	push_rxbuffer(buffer, count);
	//dprintf(INFO, "irq push count = %d\n",count);

	setIntSta(3);
	setIntEnable(3, 1);
    v_enable_bim_irq(VECTOR_UART_3);
}

void uart4_irq_handler(void)
{
	char buffer[1024] = {0};
	int count = 0;

	//pr_info("--uart-uart4_irq_handler--\n");

	setIntEnable(4, 0);
	v_disable_bim_irq(VECTOR_UART_4);

    if ((g_pUartRegs->U4_STATUS) & 0x1FF) {
		do
		{
			buffer[count++] = (char)readByte(4);
		} while((g_pUartRegs->U4_STATUS) & 0x1FF);
    }

	push_rxbuffer(buffer, count);
	//dprintf(INFO, "irq push count = %d\n",count);

	setIntSta(4);
	setIntEnable(4, 1);
    v_enable_bim_irq(VECTOR_UART_4);
}

void uart5_irq_handler(void)
{
	char buffer[1024] = {0};
	int count = 0;

	setIntEnable(5, 0);
	v_disable_bim_irq(VECTOR_UART_5);

    if ((g_pUartRegs->U5_STATUS) & 0x1FF) {
		do
		{
			buffer[count++] = (char)readByte(5);
		} while((g_pUartRegs->U5_STATUS) & 0x1FF);
    }

	push_rxbuffer(buffer, count);
	//dprintf(INFO, "irq push count = %d\n",count);

	setIntSta(5);
	setIntEnable(5, 1);
    v_enable_bim_irq(VECTOR_UART_5);
}

void uart_monitor_deinit(int id)
{
	setIntEnable(id, 0);

    switch (id)
    {
        case 1:
            v_disable_bim_irq(VECTOR_UART_1);
            break;

        case 2:
            v_disable_bim_irq(VECTOR_UART_2);
            break;

        case 3:
            v_disable_bim_irq(VECTOR_UART_3);
            break;

        case 4:
            v_disable_bim_irq(VECTOR_UART_4);
            break;

        case 5:
            v_disable_bim_irq(VECTOR_UART_5);
            break;

        default:
            pr_info("uart_monitor_deinit id=%d not support.\n", id);
            break;
    }

}

static BOOL setWordLength(UINT32 u4Index, UCHAR wordLength)
{
    AC83XX_UART_REG *pUartRegs = g_pUartRegs;
    BOOL rc = FALSE;
    if ((wordLength < 5) || (wordLength > 8)) {
          goto cleanUp;
    }
    wordLength = 8 - wordLength;

    switch (u4Index)
    {
        case 1:
			pUartRegs->U1_CCR = ((pUartRegs->U1_CCR)&(~0x3))|wordLength;
            break;

        case 2:
			pUartRegs->U2_CCR = ((pUartRegs->U2_CCR)&(~0x3))|wordLength;
            break;

        case 3:
			pUartRegs->U3_CCR = ((pUartRegs->U3_CCR)&(~0x3))|wordLength;
            break;

        case 4:
			pUartRegs->U4_CCR = ((pUartRegs->U4_CCR)&(~0x3))|wordLength;
            break;

        case 5:
			pUartRegs->U5_CCR = ((pUartRegs->U5_CCR)&(~0x3))|wordLength;
            break;

        case 6:
			pUartRegs->U6_CCR = ((pUartRegs->U6_CCR)&(~0x3))|wordLength;
            break;

        default:
            pr_info("setWordLength u4Index=%d not support.\n", u4Index);
            break;
    }

    rc = TRUE;

cleanUp:
    return rc;
}

static BOOL setParity(UINT32 u4Index, UCHAR parity)
{
    BOOL rc = FALSE;
    AC83XX_UART_REG *pUartRegs = g_pUartRegs;

    switch (parity)
    {
    case NOPARITY:
        switch (u4Index)
           {
            case 1:
			    pUartRegs->U1_CCR = ((pUartRegs->U1_CCR) & (~REG_PARITY_U1_ON) & (~0xFF)) | (REG_PARITY_U1_OFF);
                break;

            case 2:
			    pUartRegs->U2_CCR = ((pUartRegs->U2_CCR) & (~REG_PARITY_U1_ON) & (~0xFF)) | (REG_PARITY_U1_OFF);
                break;

            case 3:
			    pUartRegs->U3_CCR = ((pUartRegs->U3_CCR) & (~REG_PARITY_U1_ON) & (~0xFF)) | (REG_PARITY_U1_OFF);
                break;

            case 4:
			    pUartRegs->U4_CCR = ((pUartRegs->U4_CCR) & (~REG_PARITY_U1_ON) & (~0xFF)) | (REG_PARITY_U1_OFF);
                break;

            case 5:
                pUartRegs->U5_CCR = ((pUartRegs->U5_CCR) & (~REG_PARITY_U1_ON) & (~0xFF)) | (REG_PARITY_U1_OFF);
                break;

            case 6:
			    pUartRegs->U6_CCR = ((pUartRegs->U6_CCR) & (~REG_PARITY_U1_ON) & (~0xFF)) | (REG_PARITY_U1_OFF);
                break;

            default:
                pr_info("setParity u4Index=%d not support.\n", u4Index);
                break;
           }
        break;
    case ODDPARITY:
        switch (u4Index)
           {
            case 1:
                pUartRegs->U1_CCR = ((pUartRegs->U1_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
                    (~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_ODD);
                break;

            case 2:
                pUartRegs->U2_CCR = ((pUartRegs->U2_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
                    (~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_ODD);
                break;

            case 3:
                pUartRegs->U3_CCR = ((pUartRegs->U3_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
                    (~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_ODD);
                break;

            case 4:
                pUartRegs->U4_CCR = ((pUartRegs->U4_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
                    (~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_ODD);
                break;

            case 5:
                pUartRegs->U5_CCR = ((pUartRegs->U5_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
                    (~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_ODD);
                break;

            case 6:
                pUartRegs->U6_CCR = ((pUartRegs->U6_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
                    (~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_ODD);
                break;

            default:
                pr_info("setParity u4Index=%d not support.\n", u4Index);
                break;
           }
        break;
    case EVENPARITY:
        switch (u4Index)
           {
            case 1:
				pUartRegs->U1_CCR = ((pUartRegs->U1_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
					(~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_EVEN);
                break;

            case 2:
				pUartRegs->U2_CCR = ((pUartRegs->U2_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
					(~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_EVEN);
                break;

            case 3:
				pUartRegs->U3_CCR = ((pUartRegs->U3_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
					(~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_EVEN);
                break;

            case 4:
				pUartRegs->U4_CCR = ((pUartRegs->U4_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
					(~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_EVEN);
                break;

            case 5:
				pUartRegs->U5_CCR = ((pUartRegs->U5_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
					(~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_EVEN);
                break;

            case 6:
				pUartRegs->U6_CCR = ((pUartRegs->U6_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
					(~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_EVEN);
                break;

            default:
                pr_info("setParity u4Index=%d not support.\n", u4Index);
                break;
           }
        break;
    }
    rc = TRUE;

    return rc;
}

static BOOL setStopBits(UINT32 u4Index, UCHAR stopBits)
{
    AC83XX_UART_REG *pUartRegs = g_pUartRegs;
    BOOL rc = FALSE;
    UCHAR StopBitValue = 0;

    switch (stopBits)
    {
      case ONESTOPBIT:
           StopBitValue = 0;
           break;
      case ONE5STOPBITS:
      case TWOSTOPBITS:
           StopBitValue = 1;
           break;
      default:
           goto cleanUp;
    }

    switch (u4Index)
    {
        case 1:
            pUartRegs->U1_CCR = ((pUartRegs->U1_CCR)&(~REG_CCR_STOP_BIT))|StopBitValue;
            break;

        case 2:
            pUartRegs->U2_CCR = ((pUartRegs->U2_CCR)&(~REG_CCR_STOP_BIT))|StopBitValue;
            break;

        case 3:
            pUartRegs->U3_CCR = ((pUartRegs->U3_CCR)&(~REG_CCR_STOP_BIT))|StopBitValue;
            break;

        case 4:
            pUartRegs->U4_CCR = ((pUartRegs->U4_CCR)&(~REG_CCR_STOP_BIT))|StopBitValue;
            break;

        case 5:
            pUartRegs->U5_CCR = ((pUartRegs->U5_CCR)&(~REG_CCR_STOP_BIT))|StopBitValue;
            break;

        case 6:
            pUartRegs->U6_CCR = ((pUartRegs->U6_CCR)&(~REG_CCR_STOP_BIT))|StopBitValue;
            break;

        default:
            pr_info("setStopBits u4Index=%d not support.\n", u4Index);
            break;
    }

    rc = TRUE;

cleanUp:
    return rc;
}

static BOOL setBaudRate(UINT32 u4Index, ULONG baudRate)
{
    AC83XX_UART_REG *pUartRegs = g_pUartRegs;

    switch (u4Index)
    {
        case 1:
            pUartRegs->U1_CCR = (((SOURCE_CLOCK / baudRate - 1) << 12) | 0xD00);
            break;

        case 2:
            pUartRegs->U2_CCR = (((SOURCE_CLOCK / baudRate - 1) << 12) | 0xD00);
            break;

        case 3:
            pUartRegs->U3_CCR = (((SOURCE_CLOCK / baudRate - 1) << 12) | 0xD00);
            break;

        case 4:
            pUartRegs->U4_CCR = (((SOURCE_CLOCK / baudRate - 1) << 12) | 0xD00);
            break;

        case 5:
            pUartRegs->U5_CCR = (((SOURCE_CLOCK / baudRate - 1) << 12) | 0xD00);
            break;

        case 6:
            pUartRegs->U6_CCR = (((SOURCE_CLOCK / baudRate - 1) << 12) | 0xD00);
            break;

        default:
            pr_info("setBaudRate u4Index=%d not support.\n", u4Index);
            break;
    }

    return TRUE;
}

static PVOID HWInit(UINT32 u4Index)
{
    switch(u4Index)
    {
        case 1:
            g_pUartRegs = (AC83XX_UART_REG *)(0xFD00C040);
            g_pUartRegs->U1_BCR = 0xFFF0100;
            break;
        case 2:
            g_pUartRegs = (AC83XX_UART_REG *)(0xFD00C0c0);
            g_pUartRegs->U2_BCR = 0xFFF0100;
            break;
        case 3:
            g_pUartRegs = (AC83XX_UART_REG *)(0xFD00C100);
            g_pUartRegs->U3_BCR = 0xFFF0100;
            break;
        case 4:
            g_pUartRegs = (AC83XX_UART_REG *)(0xFD00C140);
            g_pUartRegs->U4_BCR = 0xFFF0100;
            break;
        case 5:
            g_pUartRegs = (AC83XX_UART_REG *)(0xFD00C180);
            g_pUartRegs->U5_BCR = 0xFFF0100;
            break;

        default:
            pr_info("HWInit u4Index=%d not support.\n", u4Index);
            break;
    }

    return TRUE;

}
BOOL uartInit(int id, UINT32 u4BaudRate)
{
	pr_info("uartInit, id=%d, u4BaudRate=%d.\n", id, u4BaudRate);

    HWInit(id);
	setBufCtrl(id);

    switch(id)
    {
        case 1:
            g_pUartRegs->U1_CCR = 0x118D00;
            break;
        case 2:
            g_pUartRegs->U2_CCR = 0x118D00;
            break;
        case 3:
            g_pUartRegs->U3_CCR = 0x118D00;
            break;
        case 4:
            g_pUartRegs->U4_CCR = 0x118D00;
            break;
        case 5:
            g_pUartRegs->U5_CCR = 0x118D00;
            break;

        default:
            pr_info("uartInit id=%d not support.\n", id);
            break;
    }

    setBaudRate(id, u4BaudRate);
    setStopBits(id, ONESTOPBIT);
    setParity(id, NOPARITY);
    setWordLength(id, 8);

    return TRUE;
 }

void uart_monitor_init(int id, unsigned int baudrate)
{
	uartInit(id, baudrate);

	setBufCtrl(id);
	setIntSta(id);
	setIntEnable(id, 1);

    switch(id)
    {
        case 1:
            v_enable_bim_irq(VECTOR_UART_1);
            break;
        case 2:
            v_enable_bim_irq(VECTOR_UART_2);
            break;
        case 3:
            v_enable_bim_irq(VECTOR_UART_3);
            break;
        case 4:
            v_enable_bim_irq(VECTOR_UART_4);
            break;
        case 5:
            v_enable_bim_irq(VECTOR_UART_5);
            break;

        default:
            pr_info("uart_monitor_init id=%d not support.\n", id);
            break;
    }
}

int uart_monitor_read(char *buf, int count)
{
	struct uart_transfer *transfer = &monitor_rxbuf;
	unsigned long data;

	data = (unsigned long)transfer->data;

	if (!(transfer->flags & BUF_CIRCLED)) {
		if (count >= (transfer->wp - transfer->rp)) {
			count = transfer->wp - transfer->rp;
		}
		memcpy(buf, (void *)(data + transfer->rp), count);
		transfer->rp += count;
		goto exit;
	} else {
		if (count < (BUF_SIZE - transfer->rp)) {
			memcpy(buf, (void *)(data + transfer->rp), count);
			transfer->rp += count;
			goto exit;
		} else {
			unsigned int copy_size = BUF_SIZE - transfer->rp;
			unsigned int left_size = count - copy_size;
			memcpy(buf, (void *)(data + transfer->rp), copy_size);
			transfer->rp = 0;
			if (left_size > transfer->wp) {
				left_size = transfer->wp;
			}
			memcpy((void *)(buf + copy_size), (void *)(data + transfer->rp), left_size);
			transfer->rp = left_size;
			transfer->flags &= ~BUF_CIRCLED;
			count = copy_size + left_size;
			goto exit;
		}
	}

exit:

	return count;

}


