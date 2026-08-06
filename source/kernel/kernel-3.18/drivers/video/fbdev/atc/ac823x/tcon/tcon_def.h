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
#ifndef _TCON_DEF_H

#define _TCON_DEF_H

#define IO_BASE_ADDRESS  0xFD000000

#define VFMT_ADDR           (IO_BASE_ADDRESS + 0x1000)  /* Start from 1080 infact.*/

#define PSCL_REG_OFFSET   0xA4500 /*0x2D00*/
#define PFMT_REG_OFFSET   0xA4600 /*0x2E00*/
#define RW_PSCL_CLK_CFG     (0x8C)
#define FPD_ON			   (0x1)

#define RANGE(num, low, high)  (num = (num < low) ? low:((num > high) ? high:num))

#define vWritePGMA(dAddr, dVal)  (*(volatile __u32*)(togc_reg + dAddr) = dVal)
#define dReadPGMA(dAddr)         (*(volatile __u32*)(togc_reg + dAddr))
#define vWritePGMAMsk(dAddr, dVal, dMsk) vWritePGMA((dAddr), (dReadPGMA(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define vWritePTCON(dAddr, dVal)  (*(volatile __u32*)(tcon_reg + dAddr) = dVal)
#define dReadPTCON(dAddr)         (*(volatile __u32*)(tcon_reg + dAddr))
#define vWritePTCONMsk(dAddr, dVal, dMsk) vWritePTCON((dAddr), (dReadPTCON(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define vWritePCLRP(dAddr, dVal)  (*(volatile __u32*)(tlcp_reg + dAddr) = dVal)
#define dReadPCLRP(dAddr)         (*(volatile __u32*)(tlcp_reg + dAddr))
#define vWritePCLRPMsk(dAddr, dVal, dMsk) vWritePCLRP((dAddr), (dReadPCLRP(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))



#define vWriteFMT(dAddr, dVal)              (*(volatile __u32*)(VFMT_ADDR + (dAddr)) = dVal)
#define dReadFMT(dAddr)                     (*(volatile __u32*)(VFMT_ADDR + (dAddr)))
#define vWriteFMTMsk(dAddr, dVal, dMsk) vWriteFMT((dAddr), (dReadFMT(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))


/* **********************************************************************/
/* Panel Scaler Macros*/
/* **********************************************************************/
#define vWritePSCL(dAddr, dVal)  (*(volatile __u32*)(scl_reg + dAddr) = dVal)
#define dReadPSCL(dAddr)         (*(volatile __u32*)(scl_reg + dAddr))
#define vWritePSCLMsk(dAddr, dVal, dMsk) vWritePSCL((dAddr), (dReadPSCL(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define vWritePFMT(dAddr, dVal)  (*(volatile __u32*)(sclf_reg + dAddr) = dVal)
#define dReadPFMT(dAddr)         (*(volatile __u32*)(sclf_reg + dAddr))
#define vWritePFMTMsk(dAddr, dVal, dMsk) vWritePFMT((dAddr), (dReadPFMT(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define vWriteReg(dAddr, dVal)              (*(volatile __u32*)(IO_BASE_ADDRESS + (dAddr)) = dVal)
#define dReadReg(dAddr)                     (*(volatile __u32*)(IO_BASE_ADDRESS + (dAddr)))



#endif

