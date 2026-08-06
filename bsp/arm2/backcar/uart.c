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
 

#include"platform.h"
#include <base_regs.h>
#include "reg_serial.h"
#include "custom_protocol.h"

static AC83XX_UART_REG *g_pUart1Regs = NULL;

#define ZONE_FUNCTION  TRUE

#define U_RXERR_STA 		(1 << 0)   
#define U_RXBUF_STA 		(1 << 1)		  
#define U_TIMEOUT_STA		(1 << 2)
#define U_TXBUF_STA 		(1 << 3)			 
#define U_OVRUN_STA 		(1 << 4)	
#define U_DMA_R_STA 		(1 << 5)			 
#define U_DMA_W_STA 		(1 << 6)
#define U_W_CLR_RXBUF			(1 << 14)
#define U_W_CLR_TXBUF			(1 << 15)
#define U_W_TIMEOUT_CYCLE_MASK	(0x0FFF0000)
#define U_SET_TIMEOUT_CYCLE(x)  (((x) & 0x0FFF) <<16)
#define SOURCE_CLOCK             32400000
//------------------------------------------------------------------------------
//
//  Function:  SetBaudRate
//
//  This function sets baud rate.
//
static BOOL SetBaudRate(ULONG baudRate)
{
    AC83XX_UART_REG *pUartRegs = g_pUart1Regs;
    pUartRegs->U4_CCR = ((SOURCE_CLOCK / baudRate - 1) << 12) | 0xD00 ; //D00 customer mode
    
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  SetWordLength
//
//  This function sets word length.
//
static BOOL SetWordLength(UCHAR wordLength)
{
    AC83XX_UART_REG *pUartRegs = g_pUart1Regs;
    BOOL rc = FALSE;

    //DEBUGMSG(ZONE_FUNCTION, (L"+SetWordLength()\r\n"));

    if ((wordLength < 5) || (wordLength > 8)) 
    {
        goto cleanUp;
    }
    
    wordLength = 8 - wordLength;
    pUartRegs->U4_CCR = ((pUartRegs->U4_CCR)&(~0x3))|wordLength;

    rc = TRUE;

cleanUp:
    //DEBUGMSG(ZONE_FUNCTION, (L"-SetWordLength()\r\n"));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SetParity
//
//  This function sets parity.
//
static BOOL SetParity(UCHAR parity)
{
    BOOL rc = FALSE;
    AC83XX_UART_REG *pUartRegs = g_pUart1Regs;

    switch (parity)
    {
    case NOPARITY:
        pUartRegs->U4_CCR = ((pUartRegs->U1_CCR) & (~REG_PARITY_U1_ON) & (~0xFF)) 
            | (REG_PARITY_U1_OFF);
        break;
        
    case ODDPARITY:
        pUartRegs->U4_CCR = ((pUartRegs->U1_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
            (~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_ODD);
        break;
        
    case EVENPARITY:
        pUartRegs->U4_CCR = ((pUartRegs->U1_CCR) & (~REG_PARITY_U1_ON | REG_PARITY_U1_EVEN) &
            (~0xFF)) | (REG_PARITY_U1_ON | REG_PARITY_U1_EVEN);
        break;
    }

    rc = TRUE;
    
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SetStopBits
//
//  This function sets stop bits.
//
static BOOL SetStopBits(UCHAR stopBits)
{
    AC83XX_UART_REG *pUartRegs = g_pUart1Regs;
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
    pUartRegs->U4_CCR = ((pUartRegs->U1_CCR)&(~REG_CCR_STOP_BIT))|StopBitValue;

    rc = TRUE;

cleanUp:
    //DEBUGMSG(ZONE_FUNCTION, (L"-SetWordLength()\r\n"));
    return rc;
}

BOOL DisableUartIntr(VOID)
{
    //v_disable_bim_irq(VECTOR_RS232_1);

    return TRUE;
}

BOOL EnableUartIntr(VOID)
{
    //v_enable_bim_irq(VECTOR_RS232_1);

    return TRUE;
}

static PVOID HWInit(UINT32 u4Index)
{	
    g_pUart1Regs = (AC83XX_UART_REG *)(0xFD00C140);
    g_pUart1Regs->U4_BCR = 0xFFF0100;

    return TRUE;
}

BOOL WriteBlockData(BYTE *pData, UINT32 u4ByteToWrite);

BOOL UartInit(UINT32 u4BaudRate)
{
    HWInit(2);
    SetBaudRate(u4BaudRate);
    SetStopBits(ONESTOPBIT);
    SetParity(NOPARITY);
    SetWordLength(8);	

	//g_pUart1Regs->U4_BCR = g_pUart1Regs->U4_BCR | (U_W_CLR_RXBUF | U_W_CLR_TXBUF);
	//g_pUart1Regs->U4_CCR = 0x118D00;
	return TRUE;
}

int ReadByte(VOID)
{
    int ch;
	
    //if  (REG_SER_STATUS(SerialBaseV) & SER_READ_ALLOW)        // There is received data
    if ((g_pUart1Regs->U4_STATUS) & 0x1FF)
    {        
        ch = (int)(g_pUart1Regs->U4_Data0);		
    }
    else        // There no data in RX Buffer;
    {
        ch = 0x1FF;		
    }

    return ch;
}

BOOL WriteByte(BYTE bData)
{
    while (TRUE)
    {
        // Wait while FIFO has room
        // Get line status register
        if ((g_pUart1Regs->U4_STATUS) & 0x1FF000)
        {
            g_pUart1Regs->U4_Data0 = bData;
            break;
        }
    }

    return TRUE;
}

BOOL WriteBlockData(BYTE *pData, UINT32 u4ByteToWrite)
{
    while (u4ByteToWrite > 0)
    {
        // Wait while FIFO has room
        // Get line status register
        if ((g_pUart1Regs->U4_STATUS) & 0x1FF000)
        {
            g_pUart1Regs->U4_Data0 = *pData++;        			   
            u4ByteToWrite--;
        }	   
    }

    return TRUE;
}

BOOL ReadBlockData(BYTE *pData, UINT32 u4ByteToRead)
{
    UCHAR rxChar;
    
    while (u4ByteToRead > 0)
    {
        if ((g_pUart1Regs->U4_STATUS) & 0x1FF)
        {		
            rxChar = g_pUart1Regs->U4_Data0;
            *pData++ = rxChar;
            u4ByteToRead--;			  
        }			
    }
	
    return TRUE;
}

VOID UartISR(VOID)
{
    BYTE buf;
    
    ReadBlockData(&buf, 1);
    if (buf == 0xFF)
    {
        DisableUartIntr(); 	 
    }
    
    return;
}


/**************************For parse Uart protocal packet *******************************/

#define UART_BUF_LENGTH  256
#define CMD_HEAD_LEN 2

UINT8 s_UartString[UART_BUF_LENGTH] = {0};
UINT8 s_DestString[UART_BUF_LENGTH] = {0};
UINT8 s_TempString[UART_BUF_LENGTH] = {0};


static const UINT8 HEAD_FLAG[CMD_HEAD_LEN] = {0xEE, 0xFA};

UINT16 u2_pos = 0; 
int nHead = -1;
int nTail = -1;

static UINT8 CountCrc(const UINT8 *pData, INT32 i4Len)
{
    UINT8 u1Crc = 0;
    INT32 i4Cnt = 0;

    for (i4Cnt = 0; i4Cnt < i4Len; i4Cnt++)
    {
        u1Crc += *(pData + i4Cnt);
    }

    return u1Crc;
}


static BOOL GetPacket(void)
{

    BOOL fgRet = FALSE;
    UINT8 u1DataLen = 0;
    UINT16 i = 0;
    nHead = -1;
    
    while (!fgRet)
    {
        for (i = 0; i < u2_pos; i++)
        {
            if ( (s_UartString[i] == 0xEE)
                && (s_UartString[i+1] == 0xFA) )
            {
                nHead = i;
                break;
            }
        }

        if (nHead >= 0 && (u2_pos -nHead) < sizeof(CmdHeader))
        {
            break;
        }
        if (nHead >= 0
            && (u2_pos -nHead) >= (1 + s_UartString[nHead + CMD_HEAD_LEN] + sizeof(CmdHeader))
            && (u2_pos -nHead) >= (1 + sizeof(CMDPacket) + sizeof(CmdHeader))) //+1 for CRC
        {
            u1DataLen = s_UartString[nHead + CMD_HEAD_LEN];

            nTail = nHead + sizeof(CmdHeader) + u1DataLen+1;
            
        }

        if (nHead >= 0
            && nTail > nHead
            && nTail <= u2_pos)
        {
            fgRet = TRUE;

            Printf("[BackCar] %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                s_UartString[nHead],
                s_UartString[nHead+1],
                s_UartString[nHead+2],
                s_UartString[nHead+3],
                s_UartString[nHead+4],
                s_UartString[nHead+5],
                s_UartString[nHead+6],
                s_UartString[nHead+7],
                s_UartString[nHead+8],
                s_UartString[nHead+9],
                s_UartString[nHead+10],
                s_UartString[nHead+11],
                s_UartString[nHead+12],
                s_UartString[nHead+13],
                s_UartString[nHead+14]);
        }


        if (fgRet)
        {
            //Not include the datalen byte.
            UINT8 u1Sum = CountCrc(s_UartString + nHead + sizeof(CmdHeader), u1DataLen);
            if (u1Sum != s_UartString[nTail - 1])
            {
                fgRet = FALSE;
                Printf("drop error packet uSum:%x strSrc[nTail -1]:%x\r\n", u1Sum, s_UartString[nTail -1]);
                memset(s_TempString,0,sizeof(s_TempString));
                memcpy(s_TempString,s_UartString+nTail,UART_BUF_LENGTH-nTail);
                memset(s_UartString,0,sizeof(s_UartString));
                memcpy(s_UartString,s_TempString,sizeof(s_UartString));
                u2_pos = u2_pos - nTail;

                break;
            }
            else
            {
                nHead += sizeof(CmdHeader);
            }
        }
        else
        {
            break; //not found header ,should read more data
        }
    }

    return fgRet;
}


BOOL BCFindPacketData(void)
{
    BOOL bRet = FALSE;

    bRet = GetPacket();

    if (bRet)
    {
        memset(s_DestString,0,sizeof(s_DestString));
        memset(s_TempString,0,sizeof(s_TempString));
        memcpy(s_DestString,s_UartString+nHead,sizeof(CMDPacket));

        memcpy(s_TempString,s_UartString+nTail,UART_BUF_LENGTH-nTail);
        memset(s_UartString,0,sizeof(s_UartString));
        memcpy(s_UartString,s_TempString,sizeof(s_UartString));
        u2_pos = u2_pos - nTail;

    }

    return bRet;
}


static BOOL SendData(const UINT8 *pu1Data, UINT8 u1DataLen)
{
    BOOL fgRet = FALSE;

    memset(s_TempString,0,sizeof(s_TempString));
    
    memcpy(s_TempString,HEAD_FLAG,CMD_HEAD_LEN);
    s_TempString[CMD_HEAD_LEN] = u1DataLen;

    memcpy(s_TempString+CMD_HEAD_LEN+1,pu1Data,u1DataLen);

    s_TempString[CMD_HEAD_LEN+1+u1DataLen] = CountCrc(s_TempString + CMD_HEAD_LEN + 1, u1DataLen);

    Printf("[BackCar] Ack %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x , %d Bytes\n",
        s_TempString[0],
        s_TempString[1],
        s_TempString[2],
        s_TempString[3],
        s_TempString[4],
        s_TempString[5],
        s_TempString[6],
        s_TempString[7],
        s_TempString[8],
        s_TempString[9],
        s_TempString[10],
        s_TempString[11],
        s_TempString[12],
        s_TempString[13],
        CMD_HEAD_LEN+1+u1DataLen+1);


    return BCUartWriteBlockData(s_TempString, CMD_HEAD_LEN+1+u1DataLen+1); 
}



BOOL BCDoAck(const PCMDPacket prPacket, UINT32 u4PacketLen)
{
    BOOL fgRet = FALSE;

    if (u4PacketLen >= sizeof(CMDPacket))
    {
        CMDPacket rAckPacket = {0};
        memcpy(&rAckPacket, prPacket, sizeof(CMDPacket));
        rAckPacket.uRetType = RET_ACK;
        fgRet = SendData((BYTE*)&rAckPacket, sizeof(CMDPacket));
    }
    return fgRet;
}



PVOID GetUARTPacket(void)
{
    void *pRet = NULL;

    UINT32 u4Read = ReadByte();

    if (u4Read != 0x1FF)
    {
        u4Read = u4Read & 0xFF;

        if (u2_pos >= UART_BUF_LENGTH) 
        {
            memset(s_UartString,0,sizeof(s_UartString));
            u2_pos = 0;
        }
        s_UartString[u2_pos++] = u4Read;
        
        if (BCFindPacketData())
        {
            pRet = (void*)s_DestString;
        }
    }

    return pRet;
}




