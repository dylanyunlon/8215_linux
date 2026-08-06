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
 * $RCSfile: mtk_usb.c,v $
 * $Revision: #1 $
 *
 *---------------------------------------------------------------------------*/

/** @file mtk_usb.c
 *  This C file implements the mtk usb host controller driver functions.  
 */

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include <common.h>
#ifdef CONFIG_USB_MTKHCD

#include <usb.h>
#include "asm/arch/x_typedef.h"

#include "mgc_hdrc.h"
#include "mgc_dma.h"

#include "chip_ver.h"
#include <upg_config.h>

//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------

//#define MTK_DEBUG

#ifdef MTK_DEBUG
    #define LOG(level, fmt, args...) \
        printf("[%s:%d] " fmt, __PRETTY_FUNCTION__, __LINE__ , ## args)
#else
    #define LOG(level, fmt, args...)
#endif

#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})

#ifdef UNIFIED_USB
#define MGC_FIFO_CNT    MOTG_REG_WRITE32
#else
#ifdef MTK_MHDRC
#define MGC_FIFO_CNT    MOTG_MISC_REG_WRITE32
#else
#define MGC_FIFO_CNT    MOTG_REG_WRITE32
#endif
#endif

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------
/*
 *  PIPE attributes
 */
#define MENTOR_PIPE_CONTROL                      (0)
#define MENTOR_PIPE_ISOCHRONOUS             (1)
#define MENTOR_PIPE_BULK                              (2)
#define MENTOR_PIPE_INTERRUPT                   (3)

/* USB directions */
#define USB_OUT           (TRUE)
#define USB_IN              (FALSE)

/* DMA control */
#define DMA_ON           (TRUE)
#define DMA_OFF           (FALSE)

/* SPEED control */
#define HS_ON           (TRUE)
#define HS_OFF           (FALSE)

/** Low-speed USB */
#define MUSB_CONNECTION_SPEED_LOW (1)
/** Full-speed USB */
#define MUSB_CONNECTION_SPEED_FULL (2)
/** High-speed USB */
#define MUSB_CONNECTION_SPEED_HIGH (3)

#define USB_MAX_EP_NUM                   (4)

#define USB_EP_IRQ_TIMEOUT	(0x100000)

//---------------------------------------------------------------------------
// Imported variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Imported functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Static function forward declarations
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Static variables
//---------------------------------------------------------------------------
static UINT32 *pUsbBase = (UINT32 *)MUSB_BASE2;
static UINT32 _u4Speed = HS_ON;
static UINT32 _u4DeviceSpeed = MUSB_CONNECTION_SPEED_HIGH;
static UINT32 _u4Insert = FALSE;

static const UINT8 _aTestPacket [] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfc, 0x7e, 0xbf, 0xdf,
    0xef, 0xf7, 0xfb, 0xfd, 0x7e
};
//---------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------

//-------------------------------------------------------------------------
/** MUC_WaitEpIrq
 *  wait ep interrupt.
 *  @param  prDev: point to struct usb_device. 
 *  @param  u4EpNum          selected ep number
 *  @param  fgTX                  tx = 1, rx = 0.
 *  @param  fgDMA               check dma.
 *  @retval 0   Success
 *  @retval Others  Error
 */
//-------------------------------------------------------------------------
static INT32 MUC_WaitEpIrq(struct usb_device *prDev, UINT32 u4EpNum, BOOL fgTX, BOOL fgDMA)
{
    volatile UINT32 u4Reg = 0; /* read USB registers into this */
    volatile UINT32 u4Intreg[2] = { 0, 0 };
    volatile UINT32 u4TimeOut = 0; /* read USB registers into this */
    volatile UINT16 u2Reg;

    while (u4TimeOut < USB_EP_IRQ_TIMEOUT)
    {
        u4TimeOut ++;
       #ifdef UNIFIED_USB
        u4Reg = MOTG_REG_READ8(USB_DMA_OFFSET + M_REG_DMA_INTR);
       #else
        u4Reg = MOTG_DMA_REG_READ32(M_REG_DMA_INTR);

       #endif
        if (u4Reg)
        {
          #ifdef UNIFIED_USB
            MOTG_REG_WRITE8(USB_DMA_OFFSET + M_REG_DMA_INTR, u4Reg);
          #else
            MOTG_DMA_REG_WRITE32(M_REG_DMA_INTR, u4Reg);
          #endif
            if (fgDMA)
            {
                return 0;
            }
            else
            {
                // must reset this device, no more action.
                _u4Insert = FALSE;
                prDev->status = USB_ST_BIT_ERR;
                return -1;
            }
        }

        /*
         *  Start by seeing if INTRUSB active with anything
         */
        u4Reg = MOTG_REG_READ8(M_REG_INTRUSB);
        u4Reg &= MOTG_REG_READ8(M_REG_INTRUSBE);
        if (u4Reg)
        {
            // must reset this device, no more action.
            _u4Insert = FALSE;

            MOTG_REG_WRITE8(M_REG_INTRUSB, u4Reg);
            prDev->status = USB_ST_BIT_ERR;
            return -1;
        }

        /*
         *  EP0 active if INTRTX1, bit 0=1.  TX or RX on EP0 causes this bit to
         *  be set.
         */
        u4Intreg[0] = 0;
        u4Intreg[0] = MOTG_REG_READ16(M_REG_INTRTX);
        u4Intreg[0] &= MOTG_REG_READ16(M_REG_INTRTXE);
        if (u4Intreg[0])
        {
            MOTG_REG_WRITE16(M_REG_INTRTX, u4Intreg[0]);
            // EP0 case.
            if (u4EpNum == 0)
            {
                u2Reg = MOTG_REG_READ16(M_REG_CSR0);
                if (u2Reg & M_CSR0_H_RXSTALL)
                {
                    // must reset this device, no more action.
                    prDev->status = USB_ST_BIT_ERR;
                    return -1;
                }

                if ((u2Reg & M_CSR0_H_ERROR) ||
                     (u2Reg & M_CSR0_H_NAKTIMEOUT))
                {
                    // must reset this device, no more action.
                    _u4Insert = FALSE;
                    prDev->status = USB_ST_BIT_ERR;
                    return -1;
                }

                //_MAX_u4TimeOut = (_MAX_u4TimeOut < u4TimeOut) ? u4TimeOut: _MAX_u4TimeOut;
                if ((u4Intreg[0] == (1<<u4EpNum)))
                {
                    return 0;
                }
                else
                {
                    // must reset this device, no more action.
                    _u4Insert = FALSE;
                    prDev->status = USB_ST_BIT_ERR;
                    return -1;
                }
            }
            else
            {
                u2Reg = MOTG_REG_READ16(M_REG_TXCSR);
                if ((u2Reg & M_TXCSR_H_RXSTALL) ||
                     (u2Reg & M_TXCSR_H_ERROR) ||
                     (u2Reg & M_TXCSR_H_NAKTIMEOUT))
                {
                    // must reset this device, no more action.
                    _u4Insert = FALSE;
                    prDev->status = USB_ST_BIT_ERR;
                    return -1;
                }

                if ((fgTX) && (u4Intreg[0] == (1<<u4EpNum)))
                {
                    return 0;
                }
                else
                {
                    // must reset this device, no more action.
                    _u4Insert = FALSE;
                    prDev->status = USB_ST_BIT_ERR;
                    return -1;
                }
            }
        }

        u4Intreg[1] = 0;
        u4Intreg[1] = MOTG_REG_READ16(M_REG_INTRRX);
        u4Intreg[1] &= MOTG_REG_READ16(M_REG_INTRRXE);

        if (u4Intreg[1])
        {
            MOTG_REG_WRITE16(M_REG_INTRRX, u4Intreg[1]);

            u2Reg = MOTG_REG_READ16(M_REG_RXCSR);
            // device nak time out.
            if (u2Reg & M_RXCSR_DATAERROR)
            {
                MOTG_REG_WRITE16(M_REG_RXCSR, 0);
                prDev->status = USB_ST_NAK_REC;
                return -1;
            }

            if ((u2Reg & M_RXCSR_H_RXSTALL) ||
                 (u2Reg & M_RXCSR_H_ERROR))
            {
                // must reset this device, no more action.
                _u4Insert = FALSE;
                prDev->status = USB_ST_BIT_ERR;
                return -1;
            }

            //_MAX_u4TimeOut = (_MAX_u4TimeOut < u4TimeOut) ? u4TimeOut: _MAX_u4TimeOut;
            if ((!fgTX) && (u4Intreg[1] == (1<<u4EpNum)))
            {
                return 0;
            }
            else
            {
                // must reset this device, no more action.
                _u4Insert = FALSE;
                prDev->status = USB_ST_BIT_ERR;
                return -1;
            }
        }
    }

    prDev->status = USB_ST_BIT_ERR;
    LOG(0, "MUC_WaitEpIrq timeout\n");
    return -1;
}
//-------------------------------------------------------------------------
/** MUC_ReadFifo
 *  read data from selected ep fifo.
 *  @param  *pu1Buf           point to data buffer.
 *  @param  u4EpNum         selected ep number.
 *  @param  u4ReadCount           read data length. 
 *  @return  void
 */
//-------------------------------------------------------------------------
static void MUC_ReadFifo(UINT8 *pu1Buf, UINT32 u4EpNum, UINT32 u4ReadCount)
{
    UINT32 u4FifoAddr;

    UINT32 u4Val;
    UINT32 i;
    UINT32 u4Count;

    if (u4ReadCount == 0)
    {
        return;
    }

    u4FifoAddr = FIFO_ADDRESS(u4EpNum); /* blds absolute fifo addrs */

    LOG(7, "fifo read = %d.\n", u4ReadCount);
    // set FIFO byte count = 4 bytes.
    #ifdef UNIFIED_USB
     MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 2);
    #else
    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 4);
    #endif

    if ((u4ReadCount > 0) && ((UINT32)pu1Buf & 3))
    {
        u4Count = 4;
        /* byte access for unaligned */
        while (u4ReadCount > 0)
        {
            if (u4ReadCount < 4)
            {
                #ifdef  UNIFIED_USB
                if(3 == u4ReadCount || 2 == u4ReadCount)
                {
                    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 1);
                    u4Count = 2;
                }

                if(1 == u4ReadCount)
                {
                    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 0);
                    u4Count = 1;
                }

                #else
                // set FIFO byte count.
                MGC_FIFO_CNT(M_REG_FIFOBYTECNT, u4ReadCount);
                u4Count = u4ReadCount;
                #endif
            }

            u4Val = *((volatile UINT32 *)u4FifoAddr);

            for (i = 0; i < u4Count; i++)
            {
                *pu1Buf++ = ((u4Val >> (i *8)) & 0xFF);
            }
            u4ReadCount -= u4Count;
        }
    }
    else
    {
        /* word access if aligned */
        while ((u4ReadCount > 3) && !((UINT32)pu1Buf & 3))
        {
            *((volatile UINT32 *)pu1Buf) = *((volatile UINT32 *)u4FifoAddr);

            pu1Buf = pu1Buf + 4;
            u4ReadCount = u4ReadCount - 4;
        }
        if (u4ReadCount > 0)
        {
            #ifdef UNIFIED_USB
            if(3 == u4ReadCount ||2 == u4ReadCount )
            {
               MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 1);
               u4Val = *((volatile UINT32 *)u4FifoAddr);
                for (i = 0; i < 2; i++)
                {
                    *pu1Buf++ = ((u4Val >> (i *8)) & 0xFF);
                }
                u4ReadCount -= 2;
            }

            if(1 == u4ReadCount)
            {
               MGC_FIFO_CNT(M_REG_FIFOBYTECNT,0);
               u4Val = *((volatile UINT32 *)u4FifoAddr);

                *pu1Buf++ = (u4Val & 0xFF);
                u4ReadCount -= 1;
            }


            #else
            // set FIFO byte count.
            MGC_FIFO_CNT(M_REG_FIFOBYTECNT, u4ReadCount);

            u4Val = *((volatile UINT32 *)u4FifoAddr);
            for (i = 0; i < u4ReadCount; i++)
            {
                *pu1Buf++ = ((u4Val >> (i *8)) & 0xFF);
            }
            #endif
        }
    }

    // set FIFO byte count = 4 bytes.
#ifdef UNIFIED_USB
    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 2);
#else
    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 4);
#endif
}

//-------------------------------------------------------------------------
/** MUC_WriteFifo
 *  write data to selected ep fifo.
 *  @param  *pu1Buf           point to data buffer.
 *  @param  u4EpNum         selected ep number.
 *  @param  u4WriteCount    write data length. 
 *  @return  void
 */
//-------------------------------------------------------------------------
static void MUC_WriteFifo(UINT8 *pu1Buf, UINT32 u4EpNum, UINT32 u4WriteCount)
{
    UINT32 u4FifoAddr;

    UINT32 u4Buf;
    UINT32 u4BufSize = 4;

    u4FifoAddr = FIFO_ADDRESS(u4EpNum);

    LOG(7, "fifo write = %d.\n", u4WriteCount);

    // set FIFO byte count = 4 bytes.
#ifdef UNIFIED_USB
    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 2);
#else
    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 4);
#endif


    /* byte access for unaligned */
    if ((u4WriteCount > 0) && ((UINT32)pu1Buf & 3))
    {
        while (u4WriteCount)
        {
            if (u4WriteCount < 4)
            {
                #ifdef UNIFIED_USB
                if(3 == u4WriteCount || 2 == u4WriteCount)
                {
                    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 1);
                    u4BufSize = 2;
                }

                if(1 == u4WriteCount)
                {
                    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 0);
                    u4BufSize = 1;
                }


                #else
                u4BufSize = u4WriteCount;

                // set FIFO byte count.
                MGC_FIFO_CNT(M_REG_FIFOBYTECNT, u4WriteCount);
                #endif
            }

            memcpy((void *)&u4Buf, (const void *)pu1Buf, u4BufSize);

            *((volatile UINT32 *)u4FifoAddr) = u4Buf;

            u4WriteCount -= u4BufSize;
            pu1Buf += u4BufSize;
        }
    }
    else /* word access if aligned */
    {
        while ((u4WriteCount > 3) && !((UINT32)pu1Buf & 3))
        {
            *((volatile UINT32 *)u4FifoAddr) = *((volatile UINT32 *)pu1Buf);

            pu1Buf = pu1Buf + 4;
            u4WriteCount = u4WriteCount - 4;
        }
        if (u4WriteCount > 0)
        {
            #ifdef UNIFIED_USB
            if (3 == u4WriteCount || 2 == u4WriteCount)
            {
                //MUSB_ASSERT(u4WriteCount < 4);

                // set FIFO byte count.
                MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 1);

                *((volatile UINT32 *)u4FifoAddr) = *((volatile UINT32 *)pu1Buf);
                u4WriteCount -= 2;
                pu1Buf += 2;
            }

            if(1 == u4WriteCount)
            {
                MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 0);
                if((uint32_t)pu1Buf & 3)
                {

                    memcpy((void *)&u4Buf, (const void *)pu1Buf, 1);
                    *((volatile UINT32 *)u4FifoAddr) = *((volatile UINT32 *)u4Buf);
                }
                else
                {
                    *((volatile UINT32 *)u4FifoAddr) = *((volatile UINT32 *)pu1Buf);
                }
            }

            #else
            // set FIFO byte count.
            MGC_FIFO_CNT(M_REG_FIFOBYTECNT, u4WriteCount);

            *((volatile UINT32 *)u4FifoAddr) = *((volatile UINT32 *)pu1Buf);
            #endif
        }
    }

    // set FIFO byte count = 4 bytes.
#ifdef UNIFIED_USB
    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 2);
#else
    MGC_FIFO_CNT(M_REG_FIFOBYTECNT, 4);
#endif

}

//-------------------------------------------------------------------------
/** MUC_DevInsert
 *  check device insert or not. If device insert, try to reset device.
 *  @param   void.
 *  @retval 0   device insert
 *  @retval Others  device not insert
 */
//-------------------------------------------------------------------------
int UsbDevInsert(int port_index)
{
    volatile UINT32 reg = 0; /* read USB registers into this */
    volatile UINT32 linestate;

    //printf("\nMUC_DevInsert(): \n");

    switch(port_index)
    {
      case 0:
        pUsbBase = (UINT32 *)MUSB_BASE;
        break;
      case 1:
        pUsbBase = (UINT32 *)MUSB_BASE2;
        break;   
      default:
        printf("[usb]exceed port 2\n");
        break;
    }
    /*
     *  Start by seeing if INTRUSB active with anything
     */

    reg = MOTG_REG_READ8(M_REG_INTRUSB);
    reg &= MOTG_REG_READ8(M_REG_INTRUSBE);
   
    if (reg)
    {
        MOTG_REG_WRITE8(M_REG_INTRUSB, reg);
    }    
  //  printf("\nUSB: count=%d \n", count);
    
    if (reg & M_INTR_CONNECT) // connect event.            
    {
        mdelay(500); //50

#ifdef UNIFIED_USB
        //printf("emulation set linestate ok directly\n");
        linestate = MOTG_REG_READ32(M_REG_DEBUG_PROBE);
#else
        linestate = MOTG_PHY_REG_READ32(M_REG_PHYC6);
#endif
        linestate &= M_REG_LINESTATE_MASK;

       if ((linestate != LINESTATE_DISCONNECT) && (linestate != LINESTATE_HWERROR))
       {            
            printf("\nUSB: Detected device on port%d!\n",port_index);

            // try to reset device.
            reg = MOTG_REG_READ8(M_REG_POWER);
            reg |= M_POWER_RESET;
            MOTG_REG_WRITE8(M_REG_POWER, reg);

            // reset device address.
            MOTG_REG_WRITE8(M_REG_FADDR, 0);
    #ifndef UNIFIED_USB
            MOTG_REG_WRITE32(M_REG_EP0ADDR, 0);
            MOTG_REG_WRITE32(M_REG_EP1ADDR, 0);
            MOTG_REG_WRITE32(M_REG_EP2ADDR, 0);
            MOTG_REG_WRITE32(M_REG_EP3ADDR, 0);
            MOTG_REG_WRITE32(M_REG_EP4ADDR, 0);
    #endif

            mdelay(300); //100

            // clear reset.
            reg = MOTG_REG_READ8(M_REG_POWER);
            reg &= ~M_POWER_RESET;
            MOTG_REG_WRITE8(M_REG_POWER, reg);

            // check device speed: LS, FS, HS.
            reg = MOTG_REG_READ8(M_REG_DEVCTL);
            if (reg & M_DEVCTL_LSDEV)
            {
                printf("USB: LS device.\n");
                _u4DeviceSpeed = MUSB_CONNECTION_SPEED_LOW;
            }
            else
            {
                reg = MOTG_REG_READ8(M_REG_POWER);
                if (reg & M_POWER_HSMODE)
                {
                    printf("USB: HS device.\n");
                    _u4DeviceSpeed = MUSB_CONNECTION_SPEED_HIGH;
                }
                else
                {
                    printf("USB: FS device.\n");
                    _u4DeviceSpeed = MUSB_CONNECTION_SPEED_FULL;
                }
            }

            _u4Insert = TRUE;
            
            return 0;
        }
    }

    //printf("USB: No device !\n");
    return -1;
}

//-------------------------------------------------------------------------
/** MUC_Initial
 *  host controller register reset and initial.
 *  @param  void 
 *  @retval 0   Success
 *  @retval Others  Error
 */
//-------------------------------------------------------------------------
static INT32 MUC_Initial(void)
{
    volatile UINT32 u4Reg;
    //set USB module clk.
    u4Reg  = *(volatile DWORD *)(0xF0000284);
    u4Reg &=   0xFFFFFFFE; 
    *(volatile DWORD *)(0xF0000284) =u4Reg;

    u4Reg  = *(volatile DWORD *)(0xF00000A0);
    u4Reg  |= (0x1 << 13);    
    *(volatile DWORD *)(0xF00000A0) =u4Reg;   

    u4Reg  = MOTG_PHY_REG_READ32(0x0);    
    if(!(u4Reg&0x00004000)){        
        u4Reg |= 0x00004000;        
        MOTG_PHY_REG_WRITE32(0x0,u4Reg);    
    }

    //Soft Reset, RG_RESET for Soft RESET
    u4Reg = MOTG_PHY_REG_READ32(0x68);
    u4Reg |=   0x00004000;
    MOTG_PHY_REG_WRITE32(0x68, u4Reg);

    u4Reg = MOTG_PHY_REG_READ32(0x68);
    u4Reg &=  ~0x00004000;
    MOTG_PHY_REG_WRITE32(0x68, u4Reg);

    //otg bit setting
    u4Reg = MOTG_PHY_REG_READ32(0x6C);
    u4Reg &= ~0x3f3f;
    u4Reg |=  0x3e2c;
    MOTG_PHY_REG_WRITE32(0x6C, u4Reg);

    //suspendom control
    u4Reg = MOTG_PHY_REG_READ32(0x68);
    u4Reg &=  ~0x00040000;
    MOTG_PHY_REG_WRITE32(0x68, u4Reg);

    //PLL setting to reduce clock jitter    
    u4Reg = MOTG_PHY_REG_READ32(0x0);    
    u4Reg &= ~0x70000000;    
    MOTG_PHY_REG_WRITE32(0x0, u4Reg);

    //hs eye finetune
    u4Reg = MOTG_PHY_REG_READ32((uint32_t)0x10);
    u4Reg &=  ~0x00070000;
    u4Reg |= 0x00050000;
    MOTG_PHY_REG_WRITE32((uint32_t)0x10, u4Reg);    
            
    //PLL setting to reduce clock jitter
    u4Reg = MOTG_PHY_REG_READ32((uint32_t)0x0);
    u4Reg &= ~0x70000000;
    u4Reg |= 0x20000000;
    MOTG_PHY_REG_WRITE32((uint32_t)0x0, u4Reg);

    u4Reg = MOTG_PHY_REG_READ32((uint32_t)0x4);
    u4Reg |= 0x3;
    MOTG_PHY_REG_WRITE32((uint32_t)0x4, u4Reg);
    //End of PLL setting

    //For FS/LS eye pattern fine-tune
    u4Reg = MOTG_PHY_REG_READ32((uint32_t)0x10);
    u4Reg &= ~0x00007707;
    u4Reg |=  0x00005503;
    MOTG_PHY_REG_WRITE32((uint32_t)0x10, u4Reg);
    //End of FS/LS eye pattern fine-tune

      u4Reg = MOTG_PHY_REG_READ32((uint32_t)0x18);
    u4Reg &= ~0x000f0000;
    u4Reg |=  0x000e0000;
    MOTG_PHY_REG_WRITE32((uint32_t)0x18, u4Reg);

    u4Reg = MOTG_REG_READ8(M_REG_PERFORMANCE3);
    u4Reg |=  0x80;
    u4Reg &= ~0x40;
    MOTG_REG_WRITE8(M_REG_PERFORMANCE3,u4Reg);

    //open level1 dma, usb common, tx, rx interrupt
    MOTG_REG_WRITE8(M_REG_INTRLEVEL1EN, 0xf);
    //open dma mask
    MOTG_REG_WRITE8(USB_DMA_OFFSET + M_REG_DMA_INTR_MASK, 0xf);

    // config usb interrupt.
    MOTG_REG_WRITE8(M_REG_INTRUSBE,
        ((UINT8)(M_INTR_VBUSERROR | M_INTR_DISCONNECT | M_INTR_CONNECT | M_INTR_BABBLE)));

    // open session.
#if 0
    /*  flush the recv fifo, reset the data tog to 0 
        clear AutoSet, ISO, Mode, DMAEnab, FrcDataTog for recv */
    MOTG_REG_WRITE16(M_REG_RXCSR, 
        (M_RXCSR_FLUSHFIFO | M_RXCSR_CLRDATATOG));

    /*  flush the xmit fifo, reset the data tog to 0 
        clear AutoSet, ISO, Mode, DMAEnab, FrcDataTog for xmit */
    MOTG_REG_WRITE16(M_REG_TXCSR, 
        (M_TXCSR_FLUSHFIFO | M_TXCSR_CLRDATATOG));
#endif
    /* reset all power modes */
    if (_u4Speed == HS_ON)
    {
        MOTG_REG_WRITE8(M_REG_POWER, 
            M_POWER_SOFTCONN | M_POWER_HSENAB | M_POWER_ENSUSPEND);
    }
    else
    {
        MOTG_REG_WRITE8(M_REG_POWER, 
            M_POWER_SOFTCONN | M_POWER_ENSUSPEND);
    }
    printf("USB: pBase = 0x%08X init ok.\n", (UINT32)USB_ADDR);
 
    // open session.
    u4Reg = MOTG_REG_READ8(M_REG_DEVCTL); /* and refresh */
    u4Reg |= M_DEVCTL_SESSION;
    MOTG_REG_WRITE8(M_REG_DEVCTL, u4Reg);    

    
    mdelay(200);

    return 0;
   
}

//-------------------------------------------------------------------------
/** MUC_PrepareRx
 *  host controller init register for 539x.
 *  @param  void 
 *  @retval void
 */
//-------------------------------------------------------------------------
static void MUC_PrepareRx(struct usb_device *prDev, UINT32 u4Pipe)
{
    UINT8 u1Reg = 0;
    UINT32 u4DevNum = usb_pipedevice(u4Pipe);
    UINT32 u4EpNum = usb_pipeendpoint(u4Pipe);

#ifdef MTK_MHDRC    

    UINT8 bHubAddr = 0;
    UINT8 bHubPort = 0;    
    UINT8 bIsMulti = FALSE;

    if (usb_pipecontrol(u4Pipe))
    {
        u1Reg = MENTOR_PIPE_CONTROL << MGC_S_TYPE_PROTO;
    }
    else if (usb_pipebulk(u4Pipe))
    {
        u1Reg = MENTOR_PIPE_BULK << MGC_S_TYPE_PROTO;
    }
    
    u1Reg |= u4EpNum & MGC_M_TYPE_REMOTE_END;

    /* speed field in proto reg */
    switch(_u4DeviceSpeed)
    {
    case MUSB_CONNECTION_SPEED_LOW:
        u1Reg |= 0xc0;
        break;
    case MUSB_CONNECTION_SPEED_FULL:
        u1Reg |= 0x80;
        break;
    default:
        u1Reg |= 0x40;
    }

    MOTG_REG_WRITE8(M_REG_RXTYPE, u1Reg);

#ifdef UNIFIED_USB
   if(!u4EpNum)
   {
    /* target addr & hub addr/port */
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_TXFUNCADDR),
        u4DevNum);
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_TXHUBADDR),
        (bIsMulti ? (0x80 | bHubAddr) : bHubAddr));
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_TXHUBPORT),
        bHubPort);
   }
   else
#endif
   {
    /* target addr & hub addr/port */
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_RXFUNCADDR), 
        u4DevNum);
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_RXHUBADDR), 
        (bIsMulti ? (0x80 | bHubAddr) : bHubAddr));
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_RXHUBPORT), 
        bHubPort);
   }

#else

    if (usb_pipecontrol(u4Pipe))
    {
        u1Reg = MENTOR_PIPE_CONTROL << MGC_S_TYPE_PROTO;
    }
    else if (usb_pipebulk(u4Pipe))
    {
        u1Reg = MENTOR_PIPE_BULK << MGC_S_TYPE_PROTO;
    }
    
    u1Reg |= u4EpNum & MGC_M_TYPE_REMOTE_END;

    MOTG_REG_WRITE8(M_REG_RXTYPE, u1Reg);
           
#endif

    return;
}

//-------------------------------------------------------------------------
/** MUC_PrepareTx
 *  host controller init register for 539x.
 *  @param  void 
 *  @retval void
 */
//-------------------------------------------------------------------------
static void MUC_PrepareTx(struct usb_device *prDev, UINT32 u4Pipe)
{
    UINT8 u1Reg = 0;
    UINT32 u4DevNum = usb_pipedevice(u4Pipe);
    UINT32 u4EpNum = usb_pipeendpoint(u4Pipe);

#ifdef MTK_MHDRC    

    UINT8 bHubAddr = 0;
    UINT8 bHubPort = 0;    
    UINT8 bIsMulti = FALSE;

    if (usb_pipecontrol(u4Pipe))
    {
        u1Reg = MENTOR_PIPE_CONTROL << MGC_S_TYPE_PROTO;
    }
    else if (usb_pipebulk(u4Pipe))
    {
        u1Reg = MENTOR_PIPE_BULK << MGC_S_TYPE_PROTO;
    }
    
    u1Reg |= u4EpNum & MGC_M_TYPE_REMOTE_END;

    /* speed field in proto reg */
    switch(_u4DeviceSpeed)
    {
    case MUSB_CONNECTION_SPEED_LOW:
        u1Reg |= 0xc0;
        break;
    case MUSB_CONNECTION_SPEED_FULL:
        u1Reg |= 0x80;
        break;
    default:
        u1Reg |= 0x40;
    }

    MOTG_REG_WRITE8(M_REG_TXTYPE, u1Reg);

    /* target addr & hub addr/port */
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_TXFUNCADDR), 
        u4DevNum);
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_TXHUBADDR), 
        (bIsMulti ? (0x80 | bHubAddr) : bHubAddr));
    MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_TXHUBPORT), 
        bHubPort);
#ifndef UNIFIED_USB
    if(!u4EpNum)
    {
        MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_RXFUNCADDR), 
            u4DevNum);
        MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_RXHUBADDR), 
            (bIsMulti ? (0x80 | bHubAddr) : bHubAddr));
        MOTG_REG_WRITE8(MGC_BUSCTL_OFFSET(u4EpNum, MGC_O_MHDRC_RXHUBPORT), 
            bHubPort);    
    }
#endif
#else

    if (usb_pipecontrol(u4Pipe))
    {
        u1Reg = MENTOR_PIPE_CONTROL << MGC_S_TYPE_PROTO;
    }
    else if (usb_pipebulk(u4Pipe))
    {
        u1Reg = MENTOR_PIPE_BULK << MGC_S_TYPE_PROTO;
    }
    
    u1Reg |= u4EpNum & MGC_M_TYPE_REMOTE_END;

    MOTG_REG_WRITE8(M_REG_TXTYPE, u1Reg);
    
#endif

    return;
}
//-------------------------------------------------------------------------
/** MUC_SetupPkt
 *  send setup packet for ep0.
 * SETUP starts a new control request.  Devices are not allowed to
 * STALL or NAK these; they must cancel any pending control requests. 
 *  @param  prDev: point to struct usb_device.
 *  @param  u4Pipe               selected pipe.
 *  @param   *prSetup         point to setup packet.  
 *  @retval 0   Success
 *  @retval Others  Error
 */
//-------------------------------------------------------------------------
static INT32 MUC_SetupPkt(struct usb_device *prDev, UINT32 u4Pipe, struct devrequest *prSetup)
{
    UINT32 u4Len;
    UINT32 u4DevNum = usb_pipedevice(u4Pipe);
    UINT32 u4EpNum = usb_pipeendpoint(u4Pipe);
#ifndef UNIFIED_USB
    MOTG_REG_WRITE32(M_REG_EPXADDR(u4EpNum), u4DevNum);
#endif

    MOTG_REG_WRITE8(M_REG_INDEX, u4EpNum);

    //  turn on nak limit.
    MOTG_REG_WRITE8(M_REG_NAKLIMIT0, 
        ((_u4DeviceSpeed == MUSB_CONNECTION_SPEED_HIGH) ? 16 : 13));

    u4Len = sizeof(struct devrequest);

    MUC_PrepareTx(prDev, u4Pipe);

    MUC_WriteFifo((UINT8 *)prSetup, u4EpNum, u4Len);

    MOTG_REG_WRITE16(M_REG_CSR0, M_CSR0_H_NO_PING | M_CSR0_TXPKTRDY | M_CSR0_H_SETUPPKT);

    return MUC_WaitEpIrq(prDev, u4EpNum, USB_OUT, DMA_OFF);
}

//-------------------------------------------------------------------------
/** MUC_StatusPkt
 *  send setup packet for ep0.
 * SETUP starts a new control request.  Devices are not allowed to
 * STALL or NAK these; they must cancel any pending control requests. 
 *  @param  u4Pipe               selected pipe.
 *  @retval 0   Success
 *  @retval Others  Error
 */
//-------------------------------------------------------------------------
static INT32 MUC_StatusPkt(struct usb_device *prDev, UINT32 u4Pipe)
{
    UINT32 u4DevNum = usb_pipedevice(u4Pipe);
    UINT32 u4EpNum = usb_pipeendpoint(u4Pipe);
    INT32 i4Ret = 0;
#ifndef UNIFIED_USB
    MOTG_REG_WRITE32(M_REG_EPXADDR(u4EpNum), u4DevNum);
#endif

    MOTG_REG_WRITE8(M_REG_INDEX, u4EpNum);

    if (usb_pipein(u4Pipe))
    {   
        MUC_PrepareTx(prDev, u4Pipe);
    
        MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_TXPKTRDY | M_CSR0_H_STATUSPKT);
    }
    else
    {
        MUC_PrepareRx(prDev, u4Pipe);
    
        MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_H_REQPKT | M_CSR0_H_STATUSPKT);
    }

    i4Ret = MUC_WaitEpIrq(prDev, u4EpNum, usb_pipeout(u4Pipe), DMA_OFF);

    //  Must clear RxPktRdy in CSR0 when packet has read from FIFO.
    MOTG_REG_WRITE8(M_REG_CSR0, 0);

    return i4Ret;
}

//-------------------------------------------------------------------------
/** MUC_log2
 *  Calculate log2 arithmetic.
 *  @param  x   input value.
 *  @return  log2(x).
 */
//-------------------------------------------------------------------------
static inline s32 MUC_log2(s32 x)
{
    s32 i;

    for (i = 0; x > 1; i++)
    {
        x = x / 2;
    }

    return i;
}

//-------------------------------------------------------------------------
/** MUC_SetDma
 *  Basic function to control RX/TX/Interrupt for DMA transfer
 *  @param   *prDev         usb_device data structure.
 *  @param  u4Pipe               selected pipe.
 *  @param   *pvBuf             user's request data buffer.  
 *  @param   u4Len              user's request data length.   
 *  @retval 0   Success
 *  @retval Others  Error
 */
//-------------------------------------------------------------------------
static INT32 MUC_SetDma(struct usb_device *prDev, UINT32 u4Pipe, void *pvBuf, UINT32 u4Len)
{
    UINT32 u4EpNum = usb_pipeendpoint(u4Pipe);
    UINT32 u4MaxPktSize = usb_maxpacket(prDev, u4Pipe);
    volatile UINT32 u4Mode = DMA_ENABLE_BIT | DMA_IRQ_ENABLE;
    volatile UINT32 u4CSR2;
    INT32 i4Ret = 0;

    /* only bulk use dma mode. */
    if ((u4Len < MIN_DMA_TRANSFER_BYTES) || (!pvBuf) ||(!usb_pipebulk(u4Pipe)))
    {
        return -2;
    }

    if (u4Len > u4MaxPktSize)
    {
        u4Mode |= DMA_MODE_ONE;
    }

    if (u4EpNum > USB_MAX_EP_NUM)
    {
        u4EpNum = USB_MAX_EP_NUM;
    }

    MOTG_REG_WRITE8(M_REG_INDEX, u4EpNum);

    u4Mode |= (u4EpNum << DMA_ENDPOINT_SHIFT);
    
    if (usb_pipeout(u4Pipe))
    {
        u4Mode |= DMA_TX;
    }
    else
    {
        u4Mode |= DMA_RX;
    }

    LOG(7, "dma len = %d\n", u4Len);

    MOTG_DMA_REG_WRITE32(M_REG_DMA_ADDR, (UINT32)pvBuf);
    MOTG_DMA_REG_WRITE32(M_REG_DMA_COUNT, u4Len);

    /* program DRC registers */
    switch (u4Mode & DMA_MODE_MASK)
    {
    case DMA_RX | DMA_MODE_ZERO:
        // request an IN transaction.
        MOTG_REG_WRITE8(M_REG_RXCSR1, M_RXCSR1_H_REQPKT);

        // wait packet interrupt.
        i4Ret = MUC_WaitEpIrq(prDev, u4EpNum, USB_IN, DMA_OFF);
        if (0 > i4Ret)
        {
            return i4Ret;
        }

        // clear TX mode.
        u4CSR2 = MOTG_REG_READ8(M_REG_TXCSR2);
        MOTG_REG_WRITE8(M_REG_TXCSR2, (u4CSR2 & ~M_TXCSR2_MODE));

        // clear multiple packet dma control in csr2.
        u4CSR2 = MOTG_REG_READ8(M_REG_RXCSR2);
        MOTG_REG_WRITE8(M_REG_RXCSR2, (u4CSR2 & ~RXCSR2_MODE1));
        break;

    case DMA_TX | DMA_MODE_ZERO:
        // clear multiple packet dma control in csr2.    
        u4CSR2 = MOTG_REG_READ8(M_REG_TXCSR2);   
        u4CSR2 &= ~TXCSR2_MODE1;
        // set TX mode.
        u4CSR2 |= M_TXCSR2_MODE;
        MOTG_REG_WRITE8(M_REG_TXCSR2, u4CSR2);
        break;

    case DMA_RX | DMA_MODE_ONE:
        // clear TX mode.
        u4CSR2 = MOTG_REG_READ8(M_REG_TXCSR2);
        MOTG_REG_WRITE8(M_REG_TXCSR2, (u4CSR2 & ~M_TXCSR2_MODE));
    
        MOTG_DMA_REG_WRITE32(M_REG_DMA_CNTL, u4Mode);
        u4CSR2 = MOTG_REG_READ8(M_REG_RXCSR2);
        MOTG_REG_WRITE8(M_REG_RXCSR2, (u4CSR2 | RXCSR2_MODE1));
        // set request packet number.
        MOTG_DMA_REG_WRITE32(M_REG_REQPKT(u4EpNum), (u4Len + u4MaxPktSize - 1)/u4MaxPktSize);

        // request an IN transaction.
        MOTG_REG_WRITE8(M_REG_RXCSR1, M_RXCSR1_H_REQPKT);        
        return 0;

    case DMA_TX | DMA_MODE_ONE:
        u4CSR2 = MOTG_REG_READ8(M_REG_TXCSR2);
        u4CSR2 |= M_TXCSR2_MODE | TXCSR2_MODE1;        
        MOTG_REG_WRITE8(M_REG_TXCSR2, u4CSR2);
        break;

    default:
        break;
    }

    MOTG_DMA_REG_WRITE32(M_REG_DMA_CNTL, u4Mode);

    return 0;
}

//-------------------------------------------------------------------------
/** MUC_DoneDma
 *  dma irq handler.
 *  @param   *prDev         usb_device data structure.
 *  @param  u4Pipe               selected pipe.
 *  @param   *pvBuf             user's request data buffer.  
 *  @param   u4Len              user's request data length.   
 *  @retval 0   Success
 *  @retval Others  Error
 */
//-------------------------------------------------------------------------
static INT32 MUC_DoneDma(struct usb_device *prDev, UINT32 u4Pipe, void *pvBuf, UINT32 u4Len)
{
    UINT32 u4EpNum;
    UINT32 u4MaxPktSize = usb_maxpacket(prDev, u4Pipe);
    volatile UINT16 u2Reg;
    volatile UINT32 u4Control;
    volatile UINT32 u4Addr;
    UINT32 u4ActualLen;
    UINT32 u4PktLen = 0;

    // get DMA Mode, address and byte counts from DMA registers.
    u4Control = MOTG_DMA_REG_READ32(M_REG_DMA_CNTL);
    // Bus Error.
    if (u4Control & DMA_BUSERROR_BIT)
    {
        LOG(0, "DMA error !\n");
        return -1;
    }

    u4Addr = MOTG_DMA_REG_READ32(M_REG_DMA_ADDR);

    // check ep number.
    u4EpNum = (u4Control & 0xF0) >> DMA_ENDPOINT_SHIFT;
    
    // how many bytes were processed ? 
    u4ActualLen = u4Addr - (UINT32)pvBuf;

    LOG(7, "IRQ:DMA u4ActualLen = %d, diff = %d.\n", u4ActualLen,
        (u4Len - u4ActualLen));

    // save and set index register.
    MOTG_REG_WRITE8(M_REG_INDEX, u4EpNum);

    // clean DMA setup in CSR.
    if (u4Control & DMA_TX)
    {
        MOTG_REG_WRITE8(M_REG_TXCSR2, 0);        
    }
    else
    {
        // check if short packet received.
        if ((u4Control & DMA_ENABLE_BIT) && (u4Control & DMA_MODE_ONE))
        {           
            u4PktLen = MOTG_REG_READ16(M_REG_RXCOUNT);

            // receive data from fifo.
            MUC_ReadFifo((UINT8 *)u4Addr, u4EpNum, u4PktLen);
            u4ActualLen += u4PktLen;

            // close dma.
            MOTG_DMA_REG_WRITE32(M_REG_DMA_CNTL, 0);            
        }
        prDev->act_len = u4ActualLen;

        MOTG_REG_WRITE16(M_REG_RXCSR1, 0);
    }

    if (u4Control & DMA_TX)
    {
        if ((u4ActualLen % u4MaxPktSize) || (u4ActualLen == u4MaxPktSize))
        {
            // need to set TXPKTRDY manually.    
            MOTG_REG_WRITE8(M_REG_TXCSR1, M_TXCSR1_TXPKTRDY);

            // wait to send short packet.
            return MUC_WaitEpIrq(prDev, u4EpNum, USB_OUT, DMA_OFF);
        }
        //the last packet size is equal to MaxEPSize.
        else 
        {
            return 0;
        }
    }
    else
    {
        // enable rx ep interrupt.
        u2Reg = MOTG_REG_READ16(M_REG_INTRRXE) | (1 << u4EpNum);
        MOTG_REG_WRITE16(M_REG_INTRRXE, u2Reg);

        // short packet.
        if ((u4ActualLen % u4MaxPktSize) || (u4ActualLen == u4MaxPktSize)) 
        {
            // need to clean RXPKTRDY manually.
            MOTG_REG_WRITE8(M_REG_RXCSR1, 0);
        }
        
        return 0;                
    }
}

//-------------------------------------------------------------------------
/** MUC_InPkt
 *  send in request to device.
 *  @param   *prDev         usb_device data structure.
 *  @param  u4Pipe               selected pipe.
 *  @param   *pvBuf             user's request data buffer.  
 *  @param   u4Len              user's request data length.   
 *  @retval 0   Success
 *  @retval Others  Error
 */
//------------------------------------------------------------------------- 
static INT32 MUC_InPkt(struct usb_device *prDev, UINT32 u4Pipe, void *pvBuf, UINT32 u4Len)
{
    UINT32 u4DevNum = usb_pipedevice(u4Pipe);
    UINT32 u4EpNum = usb_pipeendpoint(u4Pipe);
    UINT32 u4MaxPktSize = usb_maxpacket(prDev, u4Pipe);
    UINT32 u4RxLen = 0;
    UINT32 u4PktLen = 0;
    INT32 i4Ret = 0;

    if (u4EpNum > USB_MAX_EP_NUM)
    {
        u4EpNum = USB_MAX_EP_NUM;
    }
    
    // set device address.
    #ifndef UNIFIED_USB
    MOTG_REG_WRITE32(M_REG_EPXADDR(u4EpNum), u4DevNum);
    #endif
    // set physical ep.
    MOTG_REG_WRITE8(M_REG_INDEX, u4EpNum);

    MUC_PrepareRx(prDev, u4Pipe);

    // set fifo attribute.
    if (usb_pipebulk(u4Pipe))
    {   
        //  turn on nak limit.
        MOTG_REG_WRITE8(M_REG_RXINTERVAL, 
            ((_u4DeviceSpeed == MUSB_CONNECTION_SPEED_HIGH) ? 16 : 13));
                                
        if (MOTG_REG_READ16(M_REG_RXMAXP) < u4MaxPktSize)
        {
            // set rx epx fifo size,                 
            MOTG_REG_WRITE8(M_REG_RXFIFOSZ, MUC_log2(u4MaxPktSize /8));

            // set start address of tx epx fifo.
            MOTG_REG_WRITE16(M_REG_RXFIFOADD, 8);

            //  Establish max packet size.
            MOTG_REG_WRITE16(M_REG_RXMAXP, u4MaxPktSize);
        }
    }

    // use DMA.
    i4Ret = MUC_SetDma(prDev, u4Pipe, pvBuf, u4Len);
    if (i4Ret == 0)
    {
        // wait dma interrupt
        i4Ret = MUC_WaitEpIrq(prDev, u4EpNum, USB_IN, DMA_ON);
        if (0 > i4Ret)
        {
            return i4Ret;
        }
    
        return MUC_DoneDma(prDev, u4Pipe, pvBuf, u4Len);   
    }
    else if (i4Ret == -1)
    {
        return i4Ret;
    }

    // use FIFO only.
    u4RxLen = 0;
    while (u4RxLen < u4Len)
    {
        // request an IN transaction.
        if (u4EpNum > 0)
        {
            MOTG_REG_WRITE8(M_REG_RXCSR1, M_RXCSR1_H_REQPKT);
        }
        else
        {
            MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_H_REQPKT);
        }
        
        i4Ret = MUC_WaitEpIrq(prDev, u4EpNum, USB_IN, DMA_OFF);
        if (0 > i4Ret)
        {
            return i4Ret;
        }

        u4PktLen = MOTG_REG_READ16(M_REG_RXCOUNT);

        // read data from fifo.     
        MUC_ReadFifo(pvBuf, u4EpNum, u4PktLen);

        u4RxLen += u4PktLen;
        prDev->act_len += u4PktLen;;
        pvBuf += u4PktLen;        

        // check if short packet.
        if (u4PktLen < u4MaxPktSize)
        {
            return 0;
        }
    }
    return 0;
}

//-------------------------------------------------------------------------
/** MUC_OutPkt
 *  send out data to device.
 *  @param   *prDev         usb_device data structure.
 *  @param  u4Pipe               selected pipe.
 *  @param   *pvBuf             user's request data buffer.  
 *  @param   u4Len              user's request data length.   
 *  @retval 0   Success
 *  @retval Others  Error
 */
//------------------------------------------------------------------------- 
static INT32 MUC_OutPkt(struct usb_device *prDev, UINT32 u4Pipe, void *pvBuf, UINT32 u4Len)
{
    UINT32 u4DevNum = usb_pipedevice(u4Pipe);
    UINT32 u4EpNum = usb_pipeendpoint(u4Pipe);
    UINT32 u4MaxPktSize = usb_maxpacket(prDev, u4Pipe);
    UINT32 u4TxLen = 0;
    UINT32 u4PktLen = 0;
    INT32 i4Ret = 0;

    if (u4EpNum > USB_MAX_EP_NUM)
    {
        u4EpNum = USB_MAX_EP_NUM;
    }
    
    // set device address.
    MOTG_REG_WRITE32(M_REG_EPXADDR(u4EpNum), u4DevNum);

    // set physical ep.
    MOTG_REG_WRITE8(M_REG_INDEX, u4EpNum);

    MUC_PrepareTx(prDev, u4Pipe);

    // set fifo attribute.
    if (usb_pipebulk(u4Pipe))
    {
        //  turn on nak limit.
        MOTG_REG_WRITE8(M_REG_TXINTERVAL, 
            ((_u4DeviceSpeed == MUSB_CONNECTION_SPEED_HIGH) ? 16 : 13));
    
        if (MOTG_REG_READ16(M_REG_TXMAXP) < u4MaxPktSize)
        {
            // set tx epx fifo size,                 
            MOTG_REG_WRITE8(M_REG_TXFIFOSZ, MUC_log2(u4MaxPktSize /8));

            // set start address of tx epx fifo.
            MOTG_REG_WRITE16(M_REG_TXFIFOADD, 8);

            //  Establish max packet size.
            MOTG_REG_WRITE16(M_REG_TXMAXP, u4MaxPktSize);
        }
    }

    // use DMA.
    i4Ret = MUC_SetDma(prDev, u4Pipe, pvBuf, u4Len);
    if (i4Ret == 0)
    {
        // wait dma interrupt
        i4Ret = MUC_WaitEpIrq(prDev, u4EpNum, USB_OUT, DMA_ON);
        if (0 > i4Ret)
        {
            return i4Ret;
        }
    
        return MUC_DoneDma(prDev, u4Pipe, pvBuf, u4Len);   
    }
    else if (i4Ret == -1)
    {
        return i4Ret;
    }

    // use FIFO only.
    u4TxLen = 0;
    while (u4TxLen < u4Len)
    {
        // max packet size as tx packet size limit.
        u4PktLen = min(u4MaxPktSize, (u4Len - u4TxLen));

        // write data to fifo.      
        MUC_WriteFifo(pvBuf, u4EpNum, u4PktLen);
        
        if (u4EpNum > 0)
        {
            MOTG_REG_WRITE8(M_REG_TXCSR, M_TXCSR_TXPKTRDY);
        }
        else
        {
            MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_TXPKTRDY);
        }

        i4Ret = MUC_WaitEpIrq(prDev, u4EpNum, USB_OUT, DMA_OFF);
        if (0 > i4Ret)
        {
            return i4Ret;
        }

        u4TxLen += u4PktLen;
        pvBuf += u4PktLen;            
    }

    return i4Ret;
}

//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------
//-------------------------------------------------------------------------
/** usb_sendse0nak
 *  set se0 nak test mode.
 *  @void.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int usb_sendse0nak(void)
{
    //set se0 nak.
    MOTG_REG_WRITE8(0xF, 0x01);
    printf("USB: Set SE0 NAK...OK !\n");
    return 0;            
}

//-------------------------------------------------------------------------
/** usb_sendtestj
 *  set test j test mode.
 *  @void.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int usb_sendtestj(void)
{
#ifndef UNIFIED_USB
    //  USB-IF: 539x D+/D- will over spec.
    MOTG_PHY_REG_WRITE32(0x04, 0x4A010406);
    MOTG_PHY_REG_WRITE32(0x08, 0x78004302);
#endif

    //set test j.
    MOTG_REG_WRITE8(0xF, 0x02);
    printf("USB: Set Test J...OK !\n");
    return 0;            
}
//-------------------------------------------------------------------------
/** usb_sendtestk
 *  set test k test mode.
 *  @void.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int usb_sendtestk(void)
{
#ifndef UNIFIED_USB
    //  USB-IF: 539x D+/D- will over spec.
    MOTG_PHY_REG_WRITE32(0x04, 0x4A010406);
    MOTG_PHY_REG_WRITE32(0x08, 0x78004302);
#endif

    //set test k.
    MOTG_REG_WRITE8(0xF, 0x04);
    printf("USB: Set Test K...OK !\n");
    return 0;            
}

//-------------------------------------------------------------------------
/** usb_sendtestpacket
 *  send test packet.
 *  @void.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int usb_sendtestpacket(void)
{
    MOTG_REG_WRITE8(M_REG_INDEX, 0);
    // 53 bytes.
    MUC_WriteFifo((UINT8 *)_aTestPacket, 0, 53);
    //set test packet.
    MOTG_REG_WRITE8(0xF, 0x08);
    MOTG_REG_WRITE8(M_REG_CSR0, M_CSR0_TXPKTRDY);
    printf("USB: Sending test packet...OK !\n");
    return 0;            
}

//-------------------------------------------------------------------------
/** usb_speed
 *  usb module init speed.
 *  @param  speed = TRUE: high speed enable, FALSE: only Full speed enable.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int usb_speed(int speed)
{
    _u4Speed = speed;
    printf("Set speed = %s.\n", ((speed == HS_ON) ? "High speed" : "Full speed"));

    return 0;            
}

//-------------------------------------------------------------------------
/** usb_lowlevel_init
 *  usb module init.
 *  @param  void.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
#define USB_PORT_NUM 2

int usb_lowlevel_init(int port_index)
{
    if(port_index >= USB_PORT_NUM)
    {
        return -1;
    }
        
    // Init and check 1st port.
    switch(port_index)
    {
      case 0:
        pUsbBase = (UINT32 *)MUSB_BASE;
        break;
      case 1:
        pUsbBase = (UINT32 *)MUSB_BASE2;
        break;      
      default:
        printf("[usb]exceed port 1\n");
        break;
    }
            
    if ((MUC_Initial() == 0))
    {
        printf("USB: Init Port%d OK.\n",port_index); 
        return 0;            
    }
    
    printf("USB: Init Port%d FAIL...\n",port_index);

    return -1;
}

//-------------------------------------------------------------------------
/** usb_silent_reset
 *  usb silent reset device.
 *  @param  void.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int usb_silent_reset(void)
{
    volatile UINT32 reg = 0; /* read USB registers into this */

    // try to reset device.
    reg = MOTG_REG_READ8(M_REG_POWER);
    reg |= M_POWER_RESET;
    MOTG_REG_WRITE8(M_REG_POWER, reg);

    // reset device address.
    MOTG_REG_WRITE8(M_REG_FADDR, 0);
    #ifndef UNIFIED_USB
    MOTG_REG_WRITE32(M_REG_EP0ADDR, 0);
    MOTG_REG_WRITE32(M_REG_EP1ADDR, 0);
    MOTG_REG_WRITE32(M_REG_EP2ADDR, 0);
    MOTG_REG_WRITE32(M_REG_EP3ADDR, 0);
    MOTG_REG_WRITE32(M_REG_EP4ADDR, 0);
    #endif

    mdelay(100);

    // clear reset.
    reg = MOTG_REG_READ8(M_REG_POWER);
    reg &= ~M_POWER_RESET;
    MOTG_REG_WRITE8(M_REG_POWER, reg);

    // check device speed: LS, FS, HS.
    reg = MOTG_REG_READ8(M_REG_DEVCTL);
    if (reg & M_DEVCTL_LSDEV)
    {
        printf("USB Silent Reset: LS device.\n");
        _u4DeviceSpeed = MUSB_CONNECTION_SPEED_LOW;
    }
    else
    {
        reg = MOTG_REG_READ8(M_REG_POWER);
        if (reg & M_POWER_HSMODE)
        {
            printf("USB Silent Reset: HS device.\n");
            _u4DeviceSpeed = MUSB_CONNECTION_SPEED_HIGH;
        }
        else
        {
            printf("USB Silent Reset: FS device.\n");
            _u4DeviceSpeed = MUSB_CONNECTION_SPEED_FULL;
        }
    }

    _u4Insert = TRUE;

    return 0;
}

//-------------------------------------------------------------------------
/** usb_lowlevel_stop
 *  usb module stop.
 *  @param  void.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int usb_lowlevel_stop(void)
{
    //MUC_Initial();
    return 0;
}

//-------------------------------------------------------------------------
/** submit_bulk_msg
 *  submit bulk transfer.
 *  @param  prDev: point to struct usb_device.
 *  @param  u4Pipe: usb u4Pipe.
 *  @param  puBuf: point to data buffer.
 *  @param  len: data buffer length.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int submit_bulk_msg(struct usb_device *prDev, unsigned long u4Pipe, void *pvBuf,
            int i4Len)
{
    INT32 i4Ret = 0;

    if (!_u4Insert)
    {
        prDev->act_len = 0;
        prDev->status = USB_ST_BIT_ERR;
        return -1;
    }       

    LOG(7, "prDev = %ld u4Pipe = %ld buf = %p size = %d dir_out = %d\n",
           usb_pipedevice(u4Pipe), usb_pipeendpoint(u4Pipe), pvBuf, i4Len, usb_pipeout(u4Pipe));

    prDev->status = 0;
    prDev->act_len = 0;
    if ((i4Len > 0) && (pvBuf))
    {
        if (usb_pipein(u4Pipe))
        {
            i4Ret = MUC_InPkt(prDev, u4Pipe, pvBuf, i4Len);
        }
        else
        {
            i4Ret = MUC_OutPkt(prDev, u4Pipe, pvBuf, i4Len);
            prDev->act_len = i4Len;                
        }
        
        if (i4Ret < 0)
        {
            prDev->act_len = 0;
            return i4Ret;    
        }
    }

    return i4Ret;
}

//-------------------------------------------------------------------------
/** submit_control_msg
 *  submit control transfer.
 *  @param  prDev: point to struct usb_device.
 *  @param  u4Pipe: usb u4Pipe.
 *  @param  puBuf: point to data buffer.
 *  @param  len: data buffer length.
 *  @param  setup: point to struct devrequest.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int submit_control_msg(struct usb_device *prDev, unsigned long u4Pipe, void *pvBuf,
               int i4Len, struct devrequest *prSetup)
{
    INT32 i4Ret = 0;

    if (!_u4Insert)
    {
        prDev->act_len = 0;
        prDev->status = USB_ST_BIT_ERR;
        return -1;
    }       

    prDev->status = 0;

    LOG(7, "prDev = %d u4Pipe = %ld buf = %p size = %d rt = %#x req = %#x.\n",
           usb_pipedevice(u4Pipe), usb_pipeendpoint(u4Pipe), 
           pvBuf, i4Len, (int)prSetup->requesttype, (int)prSetup->request);

    /* setup phase */
    if (MUC_SetupPkt(prDev, u4Pipe, prSetup) == 0)
    {
        /* data phase */
        if ((i4Len > 0) && (pvBuf))
        {
            // wait device prepare ok.
            mdelay(10);
        
            if (usb_pipein(u4Pipe))
            {
                i4Ret = MUC_InPkt(prDev, u4Pipe, pvBuf, i4Len);
            }
            else
            {
                i4Ret = MUC_OutPkt(prDev, u4Pipe, pvBuf, i4Len);
            }
            
            if (i4Ret < 0)
            {
                printf("data phase failed!\n");
                return i4Ret;    
            }
        }

        // wait device prepare ok.
        mdelay(10);

        /* status phase */
        i4Ret = MUC_StatusPkt(prDev, u4Pipe);
        if (i4Ret < 0)
        {
            printf("status phase failed!\n");
            return i4Ret;    
        }
    } 
    else 
    {
            printf("setup phase failed!\n");
    }

    // wait device prepare ok.
    mdelay(10);

    prDev->act_len = i4Len;

    return i4Len;
}

//-------------------------------------------------------------------------
/** submit_int_msg
 *  submit interrupt transfer.
 *  @param  prDev: point to struct usb_device.
 *  @param  u4Pipe: usb u4Pipe.
 *  @param  puBuf: point to data buffer.
 *  @param  len: data buffer length.
 *  @param  interval: interrupt interval.
 *  @retval 0   Success
 *  @retval Others  Fail
 */
//-------------------------------------------------------------------------
int submit_int_msg(struct usb_device *prDev, unsigned long u4Pipe, void *puBuf,
           int len, int interval)
{
    LOG(7, "prDev = %p u4Pipe = %#lx buf = %p size = %d int = %d\n", prDev, u4Pipe,
           puBuf, len, interval);
    return -1;
}

#endif  /* CONFIG_USB_MTKHCD */
