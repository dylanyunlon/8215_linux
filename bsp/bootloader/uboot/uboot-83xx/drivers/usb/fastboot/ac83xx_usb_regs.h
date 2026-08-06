//
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Copyright (c) 2007 BSQUARE Corporation. All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Header:  ac83xx_usb_regs.h
//
#ifndef ___AC83XX_USB_REGS___
#define ___AC83XX_USB_REGS___

//   
//#ifdef CHIP_VER_AC83XX
//    #define CC_AC83XX 1
//#endif


/*-------------------------------------------------------------------------*/
///** HDRC */
//#define MUSB_CONTROLLER_HDRC           1
//
///** MHDRC */
//#define MUSB_CONTROLLER_MHDRC          2

//
//#if CC_AC83XX
    #define MUSB_VECTOR_USB0               22  //VECTOR_USB
    #define MUSB_VECTOR_USB1               85 //VECTOR_USB2
//#endif

//
//#define IO_VIRT                        0xF0000000
//#define MBIM_VIRT                      (IO_VIRT + 0x08000)
//#ifndef MUSB_BASE
//    #define MUSB_BASE                  (IO_VIRT + 0x0E000)
//    #define MUSB_BASE2                 (IO_VIRT + 0x3C000)
//    #define MUSB_BASE3                 (IO_VIRT + 0x0F000)
//#endif


//#if CC_AC83XX
    #define MUSB_COREBASE                  (0)
    #define MUSB_DMABASE                   (0x200)

    #define MUSB_PORT0_PHYBASE             (0x00800)
    #define MUSB_PORT1_PHYBASE             (0x00900)
    #define MUSB_FREQM_PHYBASE             (0x00F00)
//#else
//    #define MUSB_COREBASE                  (0x800)
//    #define MUSB_DMABASE                   (0xA00)
//    #define MUSB_ANAPHYBASE                (0x700)
//    #define MUSB_MISCBASE                  (0x600)
//    #define MUSB_CBUFBASE                  (0x500)
//    #define MUSB_PHYBASE                   (0x400)
//#endif

//#define MUSB_DEFAULT_ADDRESS_SPACE_SIZE 0x00001000

/*
*     MUSBHDRC Register map
*/

/* Common USB registers */

#define MGC_O_HDRC_FADDR               0x00   /* 8-bit */
#define MGC_O_HDRC_POWER               0x01 /* 8-bit */

#define MGC_O_HDRC_INTRTX              0x02   /* 16-bit */
#define MGC_O_HDRC_INTRRX              0x04
#define MGC_O_HDRC_INTRTXE             0x06
#define MGC_O_HDRC_INTRRXE             0x08
#define MGC_O_HDRC_INTRUSB             0x0A /* 8 bit */
#define MGC_O_HDRC_INTRUSBE            0x0B /* 8 bit */
#define MGC_O_HDRC_FRAME               0x0C
#define MGC_O_HDRC_INDEX               0x0E /* 8 bit */
#define MGC_O_HDRC_TESTMODE            0x0F /* 8 bit */

#define MGC_O_HDRC_DUMMY1              0xE0   /* 32 bit */

#define MGC_M_DUMMY1_SOFFORCE          (1<<12)

/* Additional Control Registers */

#define MGC_O_HDRC_DEVCTL              0x60 /* 8 bit */

/* These are actually indexed: */
#define MGC_O_HDRC_TXFIFOSZ            0x62    /* 8-bit (see masks) */
#define MGC_O_HDRC_RXFIFOSZ            0x63     /* 8-bit (see masks) */
#define MGC_O_HDRC_TXFIFOADD           0x64 /* 16-bit offset shifted right 3 */
#define MGC_O_HDRC_RXFIFOADD           0x66   /* 16-bit offset shifted right 3 */

/* Endpoint registers */
#define MGC_O_HDRC_TXMAXP              0x00
#define MGC_O_HDRC_TXCSR               0x02
#define MGC_O_HDRC_TXCSR2              0x03
#define MGC_O_HDRC_CSR0                MGC_O_HDRC_TXCSR /* re-used for EP0 */
#define MGC_O_HDRC_RXMAXP              0x04
#define MGC_O_HDRC_RXCSR               0x06
#define MGC_O_HDRC_RXCSR2              0x07
#define MGC_O_HDRC_RXCOUNT             0x08
#define MGC_O_HDRC_COUNT0              MGC_O_HDRC_RXCOUNT         /* re-used for EP0 */
#define MGC_O_HDRC_TXTYPE              0x0A
#define MGC_O_HDRC_TYPE0               MGC_O_HDRC_TXTYPE         /* re-used for EP0 */
#define MGC_O_HDRC_TXINTERVAL          0x0B
#define MGC_O_HDRC_NAKLIMIT0           MGC_O_HDRC_TXINTERVAL /* re-used for EP0 */
#define MGC_O_HDRC_RXTYPE              0x0C
#define MGC_O_HDRC_RXINTERVAL          0x0D
#define MGC_O_HDRC_FIFOSIZE            0x0F
#define MGC_O_HDRC_CONFIGDATA          MGC_O_HDRC_FIFOSIZE /* re-used for EP0 */

/*
Begin of MTK created register definition.
*/
//  MTK Notice: Max Liao, 2006/08/29.
//  added for non-32 bits aligned fifo read/write. Base addr = USB_BASE = 0x20029600.
//#if CC_AC83XX
    #define M_REG_FIFOBYTECNT    0x690
//#else
//    #define M_REG_FIFOBYTECNT              0xEC
//#endif
//  MTK Notice: Max Liao, 2006/08/18.
//  support one to one mapping ep and device address. Base addr = USB_BASE = 0x20029800.
#define M_REG_EP0ADDR                  0x90
#define M_REG_EP1ADDR                  0x94
#define M_REG_EP2ADDR                  0x98
#define M_REG_EP3ADDR                  0x9C
#define M_REG_EP4ADDR                  0xA0
#define M_REG_EPXADDR(X)               (M_REG_EP0ADDR + ((X) << 2))

//  MTK Notice: Max Liao, 2006/05/22.
//  read PHY line state. Base addr = USB_PHYBASE = 0x20029400.
#define M_REG_PHYC0                    0x00
#define M_REG_PHYC1                    0x04
#define M_REG_PHYC2                    0x08
#define M_REG_PHYC3                    0x0c
#define M_REG_PHYC4                    0x10
#define M_REG_PHYC5                    0x14
#define M_REG_PHYC6                    0x18
#define M_REG_PHYC7                    0x1c
#define M_REG_LINESTATE_MASK           0x00030000

#define LINESTATE_DISCONNECT           0x00000000
#define LINESTATE_FS_SPEED             0x00010000
#define LINESTATE_LS_SPEED             0x00020000
#define LINESTATE_HWERROR              0x00030000

//  MTK Notice: Max Liao, 2006/05/29.
//  MTK add: soft reset register. Base addr = USB_MISCBASE = 0x20029600.
#define M_REG_SOFTRESET                0x0
#define M_REG_SOFT_RESET_ACTIVE        0x0
#define M_REG_SOFT_RESET_DEACTIVE      0x3

//  MTK add: access unit control register. Base addr = USB_MISCBASE = 0x20029600.
/*
0x20029604
bit[1:0] reg_size  : should be always  2'b10
bit[4] Function address enable : enable function address selected by endpoint, default :1(enable)
bit[8] Force DRAM read byte enable : Force Byte enable = 0xFFFF during DRAM read, default: 0(disable)
bit[9]: Enable group2 DRAM agent, Select group2 DRAM agent, default: 0(group3)
*/
#define M_REG_ACCESSUNIT               0x4
#define M_REG_ACCESSUNIT_8BIT          0x0
#define M_REG_ACCESSUNIT_16BIT         0x1
#define M_REG_ACCESSUNIT_32BIT         0x2
#define M_REG_DEV_ADDR_MODE            0x10

//  MTK add: data toggle control register. Base addr = USB_MISCBASE = 0x20029600.
//#if CC_AC83XX
    #define M_REG_RXDATATOG                 0x80
    #define M_REG_TXDATATOG                 0x84
    #define M_REG_SET_DATATOG(ep, v)        (((1 << ep) << 16) | (v << ep))
    
    #define MGC_DATATOG_READ32(_pBase, Offset)                MGC_Read32(_pBase, Offset)
    #define MGC_DATATOG_WRITE32(_pBase, Offset,v)             MGC_Write32(_pBase, Offset,v)
//#else
//    #define M_REG_RXDATATOG                0x10
//    #define M_REG_TXDATATOG                0x14
//    #define M_REG_SET_DATATOG(ep, v)       (((1 << ep) << 16) | (v << ep))
//    
//    #define MGC_DATATOG_READ32(_pBase, r)                     MGC_MISC_Read32(_pBase, r)
//    #define MGC_DATATOG_WRITE32(_pBase, r, v)                 MGC_MISC_Write32(_pBase, r, v)
//#endif
//  MTK add: request packet number. Base addr = USB_DMABASE = 0x20029A00.
#define M_REG_REQPKT(ep)               (0x100 + ((ep)<<2))

//  MTK Notice: Max Liao, 2006/09/19.
//  MTK add: IN packet interrupt. Base addr = USB_MISCBASE = 0x20029600.
/*
0x20029608[15:0] : Interrupt mask ( default : 16'hFFFF, will change to 16'h0 later)
0x2002960C [15:0]: interrupt status ( default : 0)
bit[15:0] RX IN endpoint request bit[0] : EP0, bit[1] : EP1, ...
Notes: Endpoint number is logical endpoint number, not physical.
*/
#define M_REG_INPKT_ENABLE             0x8
#define M_REG_INPKT_STATUS             0xC

//  MTK Notice: Max Liao, 2006/09/19.
//  MTK add: Powe down register. Base addr = USB_MISCBASE = 0x20029600.
/*
bit 0 : Enable DRAM clock, default : 1<Enable>
bit 1 : Enable Hardware Auto-PowerDown/Up, default : 0<Disable>, Auto-Clear after PowerUp
bit 2 : Read Only, 1: PHY Clock valid, 0: PHY clock is off.
After turn off DRAM clock, only 0x20029680 registers is accessable.
Write other registers makes no effect, and read other registers return constant value, 32'hDEAD_BEAF
*/
#define M_REG_AUTOPOWER                0x80
#define M_REG_AUTOPOWER_DRAMCLK        0x01
#define M_REG_AUTOPOWER_ON             0x02
#define M_REG_AUTOPOWER_PHYCLK         0x04
/*
End of MTK created register definition.
*/

/* Added in HDRC 1.9(?) & MHDRC 1.4 */
/* ULPI pass-through */
#define MGC_O_HDRC_ULPI_VBUSCTL        0x70
#define MGC_O_HDRC_ULPI_REGDATA        0x74
#define MGC_O_HDRC_ULPI_REGADDR        0x75
#define MGC_O_HDRC_ULPI_REGCTL         0x76

/* extended config & PHY control */
#define MGC_O_HDRC_ENDCOUNT            0x78
#define MGC_O_HDRC_DMARAMCFG           0x79
#define MGC_O_HDRC_PHYWAIT             0x7A
#define MGC_O_HDRC_PHYVPLEN            0x7B /* units of 546.1 us */
#define MGC_O_HDRC_HSEOF1              0x7C    /* units of 133.3 ns */
#define MGC_O_HDRC_FSEOF1              0x7D   /* units of 533.3 ns */
#define MGC_O_HDRC_LSEOF1              0x7E  /* units of 1.067 us */

/* "bus control" registers */
#define MGC_O_MHDRC_TXFUNCADDR         0x00
#define MGC_O_MHDRC_TXHUBADDR          0x02
#define MGC_O_MHDRC_TXHUBPORT          0x03

#define MGC_O_MHDRC_RXFUNCADDR         0x04
#define MGC_O_MHDRC_RXHUBADDR          0x06
#define MGC_O_MHDRC_RXHUBPORT          0x07


#define MGC_O_HDRC_HWVERS	0x6C
#define MGC_O_HDRC_HWSVERS	0x6E
#define MGC_O_HDRC_EPINFO	0x78
#define MGC_O_HDRC_RAMINFO	0x79


//#if CC_AC83XX
   //level 1 interrupt
    #define MGC_O_INTRLEVEL1               0xA0
    #define MGC_O_INTRLEVEL1EN             0xA4
    
    /* ULPI & Additional Configuration Register */
    #define MGC_O_HDRC_BUSPERF1    0x70
    #define MGC_O_HDRC_BUSPERF2    0x72
    #define MGC_O_HDRC_BUSPERF3    0x74
    #define MGC_O_HDRC_LINKINFO     0x7A
    #define MGC_O_HDRC_VPLEN         0x7B
    #define MGC_O_HDRC_HSEOF1       0x7C
    #define MGC_O_HDRC_FSEOF1       0x7D
    #define MGC_O_HDRC_LSEOF1       0x7E
//    
//#endif
#define MGC_O_HDRC_INTRLEVEL1          0x0A0
#define MGC_O_HDRC_INTRLEVEL1EN        0x0A4


#define MGC_O_HSDMA_INTR               0x200

/* DMA Parameter: bChannel [0,7] */
#define MGC_O_HSDMA_BASE               0x200
#define MGC_O_HSDMA_CONTROL            0x004
#define MGC_O_HSDMA_ADDRESS            0x008
#define MGC_O_HSDMA_COUNT              0x00C
#define MGC_HSDMA_CHANNEL_OFFSET(_bChannel, _bOffset)  (MGC_O_HSDMA_BASE + (_bChannel << 4) + _bOffset)

/*  RX EndPoint [1,15], Value 16 bit-width */
#define MGC_O_HDRC_RQPKTCOUNT_RXBASE   0x300
#define MGC_RXAUTOSET_RQPKTCOUNT(EndPoint)  (MGC_O_HDRC_RQPKTCOUNT_RXBASE + ((EndPoint) << 2))

//#if CC_AC83XX
#define MGC_O_HSDMA_INTR_MASK          0x201
#define MGC_O_HSDMA_INTR_UNMASK_CLR    0x202
#define MGC_O_HSDMA_INTR_UNMASK_SET    0x203

#define MGC_O_HDRC_DMA_LIMITER         0x210
#define MGC_O_HDRC_DMA_CONFIG          0x220

/* DMA Extend Parameter: bChannel [0,7] */
#define MGC_O_HSDMAEXT_BASE            0x280
#define MGC_O_HSDMAEXT_REALCNT         0x000
#define MGC_O_HSDMAEXT_TIMER           0x004
#define MGC_HSDMAEXT_CHANNEL_OFFSET(_bChannel, _bOffset)  (MGC_O_HSDMAEXT_BASE + (_bChannel << 4) + _bOffset)
#define MGC_V_TIMER_INUS(dwValInUs)    (( dwValInUs >= 163840) ? 127 : ((dwValInUs+1279)/1280))
//#endif


/*
*     MUSBHDRC Register bit masks
*/
//#if CC_AC83XX
    // BUSPERF3
 #define MGC_M_BUSPERF3_VBUSERR_MODE               (1<<11)
 #define MGC_M_BUSPERF3_FLUSHFIFO_EN               (1<< 9)
 #define MGC_M_BUSPERF3_NOISESTILL_SOF             (1<< 7)
 #define MGC_M_BUSPERF3_BAB_CLR_EN                 (1<< 6)
 #define MGC_M_BUSPERF3_UNDO_SRPFIX                (1<< 3)
 #define MGC_M_BUSPERF3_OTG_DEGLITCH_DISABLE       (1<< 2)
 #define MGC_M_BUSPERF3_EP_SWRST                   (1<< 1)
 #define MGC_M_BUSPERF3_DISUSBREST                 (1<< 0)

//USB+00A0h USB Level 1 Interrupt Status Register	USB_L1INTS
#define MGC_M_HDRC_L1INTS_TX_INT                   (1<<  0)
#define MGC_M_HDRC_L1INTS_RX_INT                   (1<<  1)
#define MGC_M_HDRC_L1INTS_USBCOMM_INT              (1<<  2)
#define MGC_M_HDRC_L1INTS_DMA_INT                  (1<<  3)
#define MGC_M_HDRC_L1INTS_PSR_INT                  (1<<  4)
#define MGC_M_HDRC_L1INTS_QINT_INT                 (1<<  5)
#define MGC_M_HDRC_L1INTS_QHIF_INT                 (1<<  6)
#define MGC_M_HDRC_L1INTS_DPDM_INT                 (1<<  7)
#define MGC_M_HDRC_L1INTS_VBUSVALID_INT            (1<<  8)
#define MGC_M_HDRC_L1INTS_IDDIG_INT                (1<<  9)
#define MGC_M_HDRC_L1INTS_DRVVBUS_INT              (1<< 10)
	
//USB+00A4h USB Level 1 Interrupt Mask Register USB_L1INTM
#define MGC_M_HDRC_L1INTM_TX_INT_UNMASK	           (1<<  0)  //Endpoint Tx Interrupt UNMASK
#define MGC_M_HDRC_L1INTM_RX_INT_UNMASK	           (1<<  1)  //	Endpoint Rx Interrupt UNMASK
#define MGC_M_HDRC_L1INTM_USBCOM_INT_UNMASK	       (1<<  2)  //	USB Common Interrupt UNMASK
#define MGC_M_HDRC_L1INTM_DMA_INT_UNMASK	       (1<<  3)  //	DMA Interrupt UNMASK
#define MGC_M_HDRC_L1INTM_PSR_INT_UNMASK	       (1<<  4)  //	Packet Sequence Recorder Interrupt UNMASK
#define MGC_M_HDRC_L1INTM_QINT_UNMASK	           (1<<  5)  //	USBQ Interrupt UNMASK, only valid while USBQ is available
#define MGC_M_HDRC_L1INTM_QSW_INT_UNMASK	       (1<<  6)  //	USBQ HIF Command Interrupt UNMASK, only valid while WiMAX Q is available.
#define MGC_M_HDRC_L1INTM_DPDM_INT_UNMASK	       (1<<  7)  //	DPDM Interrupt UNMASK
#define MGC_M_HDRC_L1INTM_VBUSVALID_INT_UNMASK	   (1<<  8)  //	VBUSVALID Interrupt UNMASK
#define MGC_M_HDRC_L1INTM_IDDIG_INT_UNMASK	       (1<<  9)  //	IDDIG Interrupt UNMASK
#define MGC_M_HDRC_L1INTM_DRVVBUS_INT_UNMASK	   (1<< 10)  //	DRVVBUS interrupt UNMASK
	
	//USB+0220h DMA Configuration Register	DMA_CONFIG
#define MGC_M_1K_BOUNDARY_CROSS_EN	               (1<<  0)	
#define MGC_M_AHBWAIT_SEL	                       (1<<  1)	
#define MGC_M_DMAQ_CHAN_SEL                        (7<<  4)	
#define DMAQ_CHANNEL_SELECT(WhichDmaChannel)  	   (((WhichDmaChannel)<<  4) & MGC_M_DMAQ_CHAN_SEL)  

/*
//USB+00A4h	USB Level 1 Interrupt Mask Register	USB_L1INTM
#define MGC_M_TX_INT_UNMASK	           (1<<  0)  //Endpoint Tx Interrupt UNMASK
#define MGC_M_RX_INT_UNMASK	           (1<<  1)  //	Endpoint Rx Interrupt UNMASK
#define MGC_M_USBCOM_INT_UNMASK	       (1<<  2)  //	USB Common Interrupt UNMASK
#define MGC_M_DMA_INT_UNMASK	       (1<<  3)  //	DMA Interrupt UNMASK
#define MGC_M_PSR_INT_UNMASK	       (1<<  4)  //	Packet Sequence Recorder Interrupt UNMASK
#define MGC_M_QINT_UNMASK	           (1<<  5)  //	USBQ Interrupt UNMASK, only valid while USBQ is available
#define MGC_M_QSW_INT_UNMASK	       (1<<  6)  //	USBQ HIF Command Interrupt UNMASK, only valid while WiMAX Q is available.
#define MGC_M_DPDM_INT_UNMASK	       (1<<  7)  //	DPDM Interrupt UNMASK
#define MGC_M_VBUSVALID_INT_UNMASK	   (1<<  8)  //	VBUSVALID Interrupt UNMASK
#define MGC_M_IDDIG_INT_UNMASK	       (1<<  9)  //	IDDIG Interrupt UNMASK
#define MGC_M_DRVVBUS_INT_UNMASK	   (1<< 10)  //	DRVVBUS interrupt UNMASK
*/
//#endif

/* POWER */

#define MGC_M_POWER_ISOUPDATE          0x80
#define MGC_M_POWER_SOFTCONN           0x40
#define MGC_M_POWER_HSENAB             0x20
#define MGC_M_POWER_HSMODE             0x10
#define MGC_M_POWER_RESET              0x08
#define MGC_M_POWER_RESUME             0x04
#define MGC_M_POWER_SUSPENDM           0x02
#define MGC_M_POWER_ENSUSPEND          0x01

/* TESTMODE */

#define MGC_M_TEST_FORCE_HOST          0x80
#define MGC_M_TEST_FIFO_ACCESS         0x40
#define MGC_M_TEST_FORCEFS             0x20
#define MGC_M_TEST_FORCEHS             0x10
#define MGC_M_TEST_PACKET              0x08
#define MGC_M_TEST_K                   0x04
#define MGC_M_TEST_J                   0x02
#define MGC_M_TEST_SE0_NAK             0x01

/* allocate for double-packet buffering (effectively doubles assigned _SIZE) */
#define MGC_M_FIFOSZ_DPB               0x10

/* allocation size (8, 16, 32, ... 4096) */
#define MGC_M_FIFOSZ_SIZE              0x0f

/* CSR0 in Peripheral and Host mode */

#define MGC_M_CSR0_FLUSHFIFO           0x0100

/* New in 15-July-2005 (MHDRC v1.4 HDRC ) */
#define MGC_M_CSR0_H_NO_PING           0x0800

/* TxType/RxType */
#define MGC_M_TYPE_PROTO               0x30
#define MGC_S_TYPE_PROTO               4
#define MGC_M_TYPE_REMOTE_END          0xf

/* CONFIGDATA */

#define MGC_M_CONFIGDATA_MPRXE         0x80 /* auto bulk pkt combining */
#define MGC_M_CONFIGDATA_MPTXE         0x40 /* auto bulk pkt splitting */
/* TODO: was this in an older HDRC?
#define MGC_M_CONFIGDATA_DMA           0x40
*/
#define MGC_M_CONFIGDATA_BIGENDIAN     0x20
#define MGC_M_CONFIGDATA_HBRXE         0x10
#define MGC_M_CONFIGDATA_HBTXE         0x08
#define MGC_M_CONFIGDATA_DYNFIFO       0x04
#define MGC_M_CONFIGDATA_SOFTCONE      0x02
#define MGC_M_CONFIGDATA_UTMIDW        0x01 /* data width 0 => 8bits, 1 => 16bits */

/* TXCSR in Peripheral and Host mode */

#define MGC_M_TXCSR_AUTOSET            0x8000
#define MGC_M_TXCSR_ISO                0x4000
#define MGC_M_TXCSR_MODE               0x2000
#define MGC_M_TXCSR_DMAENAB            0x1000
#define MGC_M_TXCSR_FRCDATATOG         0x0800
#define MGC_M_TXCSR_DMAMODE            0x0400
#define MGC_M_TXCSR_CLRDATATOG         0x0040
#define MGC_M_TXCSR_FLUSHFIFO          0x0008
#define MGC_M_TXCSR_FIFONOTEMPTY       0x0002
#define MGC_M_TXCSR_TXPKTRDY           0x0001

/* TXCSR in Peripheral mode */

#define MGC_M_TXCSR_P_INCOMPTX         0x0080
#define MGC_M_TXCSR_P_SENTSTALL        0x0020
#define MGC_M_TXCSR_P_SENDSTALL        0x0010
#define MGC_M_TXCSR_P_UNDERRUN         0x0004

/* TXCSR in Host mode */
//#if CC_AC83XX
#define MGC_M_TXCSR_H_SETTXPKTRDY_TWICE    0x0100
//#else
//#define MGC_M_TXCSR_H_WR_DATATOGGLE    0x0200
//#define MGC_M_TXCSR_H_DATATOGGLE       0x0100
//#endif
#define MGC_M_TXCSR_H_NAKTIMEOUT       0x0080
#define MGC_M_TXCSR_H_RXSTALL          0x0020
#define MGC_M_TXCSR_H_ERROR            0x0004

/* RXCSR in Peripheral and Host mode */

#define MGC_M_RXCSR_AUTOCLEAR          0x8000
#define MGC_M_RXCSR_DMAENAB            0x2000
#define MGC_M_RXCSR_DISNYET            0x1000
#define MGC_M_RXCSR_DMAMODE            0x0800
#define MGC_M_RXCSR_INCOMPRX           0x0100
#define MGC_M_RXCSR_CLRDATATOG         0x0080
#define MGC_M_RXCSR_FLUSHFIFO          0x0010
#define MGC_M_RXCSR_DATAERR            0x0008
#define MGC_M_RXCSR_FIFOFULL           0x0002
#define MGC_M_RXCSR_RXPKTRDY           0x0001

/* RXCSR in Peripheral mode */

#define MGC_M_RXCSR_P_ISO              0x4000
#define MGC_M_RXCSR_P_SENTSTALL        0x0040
#define MGC_M_RXCSR_P_SENDSTALL        0x0020
#define MGC_M_RXCSR_P_OVERRUN          0x0004

/* RXCSR in Host mode */

#define MGC_M_RXCSR_H_AUTOREQ          0x4000
//#if CC_AC83XX
#define MGC_M_RXCSR_H_SETREQPKT_TWICE  0x0400
#define MGC_M_RXCSR_H_KEEPERRSTATUS    0x0200
//#else
//#define MGC_M_RXCSR_H_WR_DATATOGGLE    0x0400
//#define MGC_M_RXCSR_H_DATATOGGLE       0x0200
//#endif
#define MGC_M_RXCSR_H_RXSTALL          0x0040
#define MGC_M_RXCSR_H_REQPKT           0x0020
#define MGC_M_RXCSR_H_ERROR            0x0004

/*
* DRC-specific definitions
* $Revision: #1 $
*/
//#if CC_AC83XX
    #define MUSB_C_NUM_EPS                 5
//#else
//    #define MUSB_C_NUM_EPS                 7
//#endif

/* this is non-configurable */
#define MGC_END0_FIFOSIZE              64

#define MGC_M_FIFO_EP0                 0x20

#define MGC_O_DRC_INDEX                0x0E
#define MGC_O_DRC_FIFOSIZE             0x1F

/* Interrupt register bit masks */
#define MGC_M_INTR_SUSPEND             0x01
#define MGC_M_INTR_RESUME              0x02
#define MGC_M_INTR_RESET               0x04
#define MGC_M_INTR_BABBLE              0x04
#define MGC_M_INTR_SOF                 0x08
#define MGC_M_INTR_CONNECT             0x10
#define MGC_M_INTR_DISCONNECT          0x20
#define MGC_M_INTR_SESSREQ             0x40
#define MGC_M_INTR_VBUSERROR           0x80 /* FOR SESSION END */

#define MGC_M_INTR_EP0                 0x01   /* FOR EP0 INTERRUPT */

/* DEVCTL */

#define MGC_M_DEVCTL_BDEVICE           0x80
#define MGC_M_DEVCTL_FSDEV             0x40
#define MGC_M_DEVCTL_LSDEV             0x20
#define MGC_M_DEVCTL_HM                0x04
#define MGC_M_DEVCTL_HR                0x02
#define MGC_M_DEVCTL_SESSION           0x01

/* CSR0 */
#define MGC_M_CSR0_TXPKTRDY            0x0002
#define MGC_M_CSR0_RXPKTRDY            0x0001

/* CSR0 in Peripheral mode */
#define MGC_M_CSR0_P_FLUSHFIFO         0x0100
#define MGC_M_CSR0_P_SVDSETUPEND       0x0080
#define MGC_M_CSR0_P_SVDRXPKTRDY       0x0040
#define MGC_M_CSR0_P_SENDSTALL         0x0020
#define MGC_M_CSR0_P_SETUPEND          0x0010
#define MGC_M_CSR0_P_DATAEND           0x0008
#define MGC_M_CSR0_P_SENTSTALL         0x0004

/* CSR0 in Host mode */
#define MGC_M_CSR0_H_DISPING           MGC_M_CSR0_H_NO_PING
#define MGC_M_CSR0_H_FLUSHFIFO         0x0100
#define MGC_M_CSR0_H_NAKTIMEOUT        0x0080
#define MGC_M_CSR0_H_STATUSPKT         0x0040
#define MGC_M_CSR0_H_REQPKT            0x0020
#define MGC_M_CSR0_H_ERROR             0x0010
#define MGC_M_CSR0_H_SETUPPKT          0x0008
#define MGC_M_CSR0_H_RXSTALL           0x0004

/* Vbus values */
#define MGC_VBUS_BELOW_SESSION_END     0
#define MGC_VBUS_ABOVE_SESSION_END     1
#define MGC_VBUS_ABOVE_AVALID          2
#define MGC_VBUS_ABOVE_VBUS_VALID      3
#define MGC_VBUS_ERROR                 0xff

#define FEATURE_SOFT_CONNECT           1
#define FEATURE_DMA_PRESENT            2
#define FEATURE_HDRC_FS                4
#define FEATURE_HIGH_BW                8
#define FEATURE_DFIFO                  16
#define FEATURE_MULTILAYER             32
#define FEATURE_I2C                    64

/* DMA Control register (16-bit): */
#define MGC_S_HSDMA_ENABLE             0
#define MGC_S_HSDMA_TRANSMIT           1
#define MGC_S_HSDMA_MODE1              2
#define MGC_S_HSDMA_IRQENABLE          3
#define MGC_S_HSDMA_ENDPOINT           4
#define MGC_S_HSDMA_BUSERROR           8
#define MGC_S_HSDMA_BURSTMODE          9
#define MGC_M_HSDMA_BURSTMODE          (3 << MGC_S_HSDMA_BURSTMODE)
#define MGC_HSDMA_BURSTMODE_UNSPEC     0
#define MGC_HSDMA_BURSTMODE_INCR4      1
#define MGC_HSDMA_BURSTMODE_INCR8      2
#define MGC_HSDMA_BURSTMODE_INCR16     3

//#if CC_AC83XX
/* DMA Channel M Timer Register */
#define  MGC_M_HSDMA_TIMEOUT_STATUS   (1 << 8)
#define  MGC_M_HSDMA_TIMEOUT_MASK     (1 << 7)
#define  MGC_M_HSDMA_TIMEOUT_VALUE(M) ((M)&0x7F)
//#endif

/*
*  DRC register access macros
*/

/* Get offset for a given FIFO */
#define MGC_FIFO_OFFSET(_bEnd) (MGC_M_FIFO_EP0 + (_bEnd * 4))

#define MGC_END_OFFSET(_bEnd, _bOffset) (0x100 + (0x10*_bEnd) + _bOffset)

//#if CC_AC83XX
    #define MGC_BUSCTL_OFFSET(_bEnd, _bOffset)  (0x480 + (8*_bEnd) + _bOffset)
    #define MGC_FIFO_CNT    MGC_Write32
//#else
//    #define MGC_BUSCTL_OFFSET(_bEnd, _bOffset)  (0x80 + (8*_bEnd) + _bOffset)
//    #define MGC_FIFO_CNT    MGC_MISC_Write32
//#endif

/* indexed vs. flat register model */
#define MGC_SelectEnd(_pBase, _bEnd)                    MGC_Write8(_pBase, MGC_O_HDRC_INDEX, _bEnd)
#define MGC_ReadCsr8(_pBase, _bOffset, _bEnd)           MGC_Read8(_pBase, (_bOffset + 0x10))
#define MGC_ReadCsr16(_pBase, _bOffset, _bEnd)          MGC_Read16(_pBase, (_bOffset + 0x10))
#define MGC_WriteCsr8(_pBase, _bOffset, _bEnd, _bData)  MGC_Write8(_pBase, (_bOffset + 0x10), _bData)
#define MGC_WriteCsr16(_pBase, _bOffset, _bEnd, _bData) MGC_Write16(_pBase, (_bOffset + 0x10), _bData)

#define MGC_VBUS_MASK                  0x18                   /* DevCtl D4 - D3 */

//#define MGC_END0_START                 0x0
//#define MGC_END0_OUT                   0x2
//#define MGC_END0_IN                    0x4
//#define MGC_END0_STATUS                0x8
//
//#define MGC_END0_STAGE_SETUP           0x0
//#define MGC_END0_STAGE_TX              0x2
//#define MGC_END0_STAGE_RX              0x4
//#define MGC_END0_STAGE_STATUSIN        0x8
//#define MGC_END0_STAGE_STATUSOUT       0xf
//#define MGC_END0_STAGE_STALL_BIT       0x10

///* obsolete */
//#define MGC_END0_STAGE_DATAIN          MGC_END0_STAGE_TX
//#define MGC_END0_STAGE_DATAOUT         MGC_END0_STAGE_RX
//
//#define MGC_CHECK_INSERT_DEBOUNCE      100

/* the virtual root hub timer IRQ checks for hub status */

//#define MTK_PORT_C_MASK                (  (1 << USB_PORT_FEAT_C_CONNECTION)   \
//                                          | (1 << USB_PORT_FEAT_C_ENABLE)       \
//                                          | (1 << USB_PORT_FEAT_C_SUSPEND)      \
//                                          | (1 << USB_PORT_FEAT_C_OVER_CURRENT) \
//                                          | (1 << USB_PORT_FEAT_C_RESET))

#define MGC_SPEED_HS                   2
#define MGC_SPEED_FS                   0
#define MGC_SPEED_LS                   1

//#if CC_AC83XX
#define MGC_PHY_Read32(_pBase, r)         (*((volatile DWORD *)(((DWORD)_pBase)+(r))))
#define MGC_PHY_Write32(_pBase, r, v)     (*((volatile DWORD *)(((DWORD)_pBase)+(r))) = v);

#define MGC_PHY_FM_Read32(_pBase,r)        (*((volatile DWORD *)((((DWORD)_pBase))+ (r))))
#define MGC_PHY_FM_Write32(_pBase,r, v)    (*((volatile DWORD *)((((DWORD)_pBase))+ (r))) = (v));

#define MGC_IOV_BASE                       0xA0000000
#define MGC_IOV_CKGEN_BASE                 ((MGC_IOV_BASE) + 0x00000)
#define MGC_IOV_BIM_BASE                   ((MGC_IOV_BASE) + 0x08000)

#define MGC_BIM_Read32(r)                  (*((volatile DWORD *)((MGC_IOV_BIM_BASE)+ (r))))
#define MGC_BIM_Write32(r, v)              (*((volatile DWORD *)((MGC_IOV_BIM_BASE)+ (r))) = (v));

#define MGC_CKGEN_Read32(r)                  (*((volatile DWORD *)((MGC_IOV_CKGEN_BASE)+ (r))))
#define MGC_CKGEN_Write32(r, v)              (*((volatile DWORD *)((MGC_IOV_CKGEN_BASE)+ (r))) = (v));
//#else
//    #define MGC_PHY_Read32(_pBase, r)         (*((volatile DWORD *)(((DWORD)_pBase) + (MUSB_PHYBASE)+ (r))))
//    #define MGC_PHY_Write32(_pBase, r, v)     (*((volatile DWORD *)((((DWORD)_pBase) + MUSB_PHYBASE)+ (r))) = v)
//    #define MGC_MISC_Read32(_pBase, r)        (*((volatile DWORD *)(((DWORD)_pBase) + (MUSB_MISCBASE)+ (r))))
//    #define MGC_MISC_Write32(_pBase, r, v)    (*((volatile DWORD *)(((DWORD)_pBase) + (MUSB_MISCBASE)+ (r))) = v)
//    #define MGC_AnaPhy_Read32(_pBase,_offset) (*((volatile DWORD *)(((DWORD)_pBase) + MUSB_ANAPHYBASE + _offset)))
//    #define MGC_AnaPhy_Write32(_pBase, _offset, _data)  (*((volatile DWORD *)(((DWORD)_pBase) + MUSB_ANAPHYBASE + _offset)) = _data)
//#endif

#define MGC_DMA_Read32(_pBase, r)             (*((volatile DWORD *)(((DWORD)_pBase) + (MUSB_DMABASE)+ (r))))
#define MGC_DMA_Write32(_pBase, r, v)  (*((volatile DWORD *)(((DWORD)_pBase) + (MUSB_DMABASE)+ (r))) = v)
//#define MGC_BIM_READ32(_offset)               (*((volatile DWORD *)(MBIM_VIRT + _offset)))
//#define MGC_BIM_WRITE32(_offset,value) (*((volatile DWORD *)((MBIM_VIRT)+ (_offset))) = value)

/**
* Read an 8-bit register from the core
* @param _pBase core base address in memory
* @param _offset offset into the core's register space
* @return 8-bit datum
*/
#define MGC_Read8(_pBase, _offset)            (*((volatile BYTE *)(((DWORD)_pBase) + MUSB_COREBASE + _offset)))

/**
* Read a 16-bit register from the core
* @param _pBase core base address in memory
* @param _offset offset into the core's register space
* @return 16-bit datum
*/
#define MGC_Read16(_pBase, _offset)           (*((volatile WORD *)(((DWORD)_pBase) + MUSB_COREBASE + _offset)))

/**
* Read a 32-bit register from the core
* @param _pBase core base address in memory
* @param _offset offset into the core's register space
* @return 32-bit datum
*/
#define MGC_Read32(_pBase, _offset)          (*((volatile DWORD *)(((DWORD)_pBase) + MUSB_COREBASE + _offset)))


/**
* Write an 8-bit core register
* @param _pBase core base address in memory
* @param _offset offset into the core's register space
* @param _data 8-bit datum
*/
static void MGC_Write8(volatile VOID* pvBase, DWORD offset, DWORD data)
{
    volatile DWORD u4TmpVar;
    u4TmpVar  = *((volatile DWORD*)(((DWORD)pvBase) + MUSB_COREBASE + ((offset) & 0xFFFFFFFC)));
    u4TmpVar &= ~(((DWORD)0xFF) << (8*((offset) & 0x03)));
    u4TmpVar |= (DWORD)(((data) & 0xFF) << (8*((offset) & 0x03)));
	if((MGC_O_HDRC_INTRTX != offset)&&( MGC_O_HDRC_FADDR == ((offset) & 0xFFFFFFFC))){
		u4TmpVar &= 0x0000FFFF;
 	}
	else if ((MGC_O_HDRC_INTRRX != offset)&&(MGC_O_HDRC_INTRRX == ((offset) & 0xFFFFFFFC))){
		u4TmpVar &= 0xFFFF0000;
 	}
	else if ((MGC_O_HDRC_INTRUSB != offset)&&(MGC_O_HDRC_INTRRXE == ((offset) & 0xFFFFFFFC))){
		u4TmpVar &= 0xFF00FFFF;
 	}
	else if ((MGC_O_HSDMA_INTR != offset)&&(MGC_O_HSDMA_INTR == ((offset) & 0xFFFFFFFC))){
		u4TmpVar &= 0xFFFFFF00;
	}
    *((volatile DWORD*)(((DWORD)pvBase) + MUSB_COREBASE + ((offset) & 0xFFFFFFFC))) = u4TmpVar;
}

/**
* Write a 16-bit core register
* @param _pBase core base address in memory
* @param _offset offset into the core's register space
* @param _data 16-bit datum
*/
static void MGC_Write16(volatile VOID* pvBase, DWORD offset, DWORD data)
{
    volatile DWORD u4TmpVar;
    u4TmpVar = *((volatile DWORD*)(((DWORD)pvBase) + MUSB_COREBASE + ((offset) & 0xFFFFFFFC)));
    u4TmpVar &= ~(((DWORD)0xFFFF) << (8*((offset) & 0x03)));
    u4TmpVar |= (data) << (8*((offset) & 0x03));
	
	if((MGC_O_HDRC_INTRTX != offset)&&( MGC_O_HDRC_FADDR == ((offset) & 0xFFFFFFFC))){
		u4TmpVar &= 0x0000FFFF;
 	}
	else if ((MGC_O_HDRC_INTRRX != offset)&&(MGC_O_HDRC_INTRRX == ((offset) & 0xFFFFFFFC))){
		u4TmpVar &= 0xFFFF0000;
 	}
	else if ((MGC_O_HDRC_INTRUSB != offset)&&(MGC_O_HDRC_INTRRXE == ((offset) & 0xFFFFFFFC))){
		u4TmpVar &= 0xFF00FFFF;
 	}
	else if ((MGC_O_HSDMA_INTR != offset)&&(MGC_O_HSDMA_INTR == ((offset) & 0xFFFFFFFC))){
		u4TmpVar &= 0xFFFFFF00;
	}	
    *((volatile DWORD*)(((DWORD)pvBase) + MUSB_COREBASE + ((offset) & 0xFFFFFFFC))) = u4TmpVar;
}

/**
* Write a 32-bit core register
* @param _pBase core base address in memory
* @param _offset offset into the core's register space
* @param _data 32-bit datum
*/
#define MGC_Write32(_pBase, _offset, _data) \
(*((volatile DWORD *)(((DWORD)_pBase) + MUSB_COREBASE + _offset)) = _data)


//
//#define USB_ISO_ASAP                   0x0002
//#define USB_ASYNC_UNLINK               0x0008
//#define USB_ZERO_PACKET                0x0040      /* Finish bulk OUTs always with zero length packet */
//
#define MGC_HSDMA_MIN_DMA_LEN           (512) //(64)
//
//#define MUSB_MAX_RETRIES               5


#endif /* ___AC83XX_USB_REGS___ */
