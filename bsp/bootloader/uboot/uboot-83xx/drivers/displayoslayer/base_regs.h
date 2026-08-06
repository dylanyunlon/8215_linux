/*!
 * @file base_regs.h
 *
 * @par LEGAL DISCLAIMER
 *
 * (Header of MediaTek Software/Firmware Release or Documentation)
 *
 * BY OPENING OR USING THIS FILE, USER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 * AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * ARE PROVIDED TO USER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED
 * IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND USER AGREES TO LOOK ONLY TO
 * SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO USER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * USER HEREBY ACKNOWLEDGES THE CONFIDENTIALITY OF MEDIATEK SOFTWARE AND AGREES
 * NOT TO DISCLOSE OR PERMIT DISCLOSURE OF ANY MEDIATEK SOFTWARE TO ANY THIRD
 * PARTY OR TO ANY OTHER PERSON, EXCEPT TO DIRECTORS, OFFICERS, EMPLOYEES OF
 * USER WHO ARE REQUIRED TO HAVE THE INFORMATION TO CARRY OUT THE PURPOSE OF
 * OPENING OR USING THIS FILE.
 *
 *
 */

//------------------------------------------------------------------------------
//
//  Header: cpu_base_reg.h
//
//  This header file defines the Physical Addresses (PA) of
//  the base registers for the System on Chip (SoC) components.
//


#ifndef _BASE_REGS_H_
#define _BASE_REGS_H_

/*************************************************************************/
/*   SYSTEM DEFINE                                                       */
/*************************************************************************/
/* uncache vritual address */
#ifndef __linux__
#define IO_UCV_BASE         0xA0000000
#else
#define IO_UCV_BASE			0xFD000000
#endif

#define CKGEN_UCV_BASE                                      (IO_UCV_BASE + 0x00000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x01000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x02000)
#define VDOUT_UCV_BASE                                      (IO_UCV_BASE + 0x42000) //AC83XX_PMX
#define FMTSCL_UCV_BASE						 (IO_UCV_BASE + 0x3000) //AC83XX_PMX
#define IMAGE_UCV_BASE                                      (IO_UCV_BASE + 0x04000)
#define AUD_UCV_BASE                                        (IO_UCV_BASE + 0x05000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x06000)
#define DRAM_UCV_BASE                                       (IO_UCV_BASE + 0x07000)
#define BIM_UCV_BASE                                        (IO_UCV_BASE + 0x08000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x09000)
#define ATA_UCV_BASE                                        (IO_UCV_BASE + 0x0A000)
#define FCIF_UCV_BASE                                       (IO_UCV_BASE + 0x0B000)
#define RS232_UCV_BASE                                      (IO_UCV_BASE + 0x0C000)
#define DRAM1_UCV_BASE                                      (IO_UCV_BASE + 0x0D000)
#define USB_UCV_BASE                                        (IO_UCV_BASE + 0x0E000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x0F000)
#define TCMGPR_UCV_BASE                                     (IO_UCV_BASE + 0x10000)
#define UPIF_UCV_BASE                                       (IO_UCV_BASE + 0x11000)
#define TSMUX0_UCV_BASE                                     (IO_UCV_BASE + 0x12000)
#define TSMUX1_UCV_BASE                                     (IO_UCV_BASE + 0x13000)
#define TSMUX2_UCV_BASE                                     (IO_UCV_BASE + 0x14000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x15000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x16000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x17000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x18000)
#define PVR_UCV_BASE                                        (IO_UCV_BASE + 0x19000)
#define VENC_UCV_BASE                                       (IO_UCV_BASE + 0x1A000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x1B000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x1C000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x1D000)
#define NFI_UCV_BASE                                        (IO_UCV_BASE + 0x1E000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x1F000)
#define VDOUT_AUX_UCV_BASE                                  (IO_UCV_BASE + 0x43000) //AC83XX_PMX
#define HDMI_UCV_BASE                                       (IO_UCV_BASE + 0x21000)
#define PID_SWAP_UCV_BASE                                   (IO_UCV_BASE + 0x22000)
#define HDR_DET_UCV_BASE                                    (IO_UCV_BASE + 0x23000)
#define PDWNC_UCV_BASE                                      (IO_UCV_BASE + 0x24000)
#define GCPU_UCV_BASE                                       (IO_UCV_BASE + 0x25000)
//#define Resv_UCV_BASE                                        (IO_UCV_BASE + 0x26000)
#define VDEC_FULL0_UCV_BASE                                 (IO_UCV_BASE + 0x27000)
#define VDEC_FULL1_UCV_BASE                                 (IO_UCV_BASE + 0x28000)
#define VDEC_FULL2_UCV_BASE                                 (IO_UCV_BASE + 0x29000)
#define VDEC_FULL3_UCV_BASE                                 (IO_UCV_BASE + 0x2A000)
#define VDEC_FULL4_UCV_BASE                                 (IO_UCV_BASE + 0x2B000)
#define VDEC_LITE0_UCV_BASE                                 (IO_UCV_BASE + 0x2C000)
#define VDEC_LITE1_UCV_BASE                                 (IO_UCV_BASE + 0x2D000)
#define VDEC_LITE2_UCV_BASE                                 (IO_UCV_BASE + 0x2E000)
#define VDEC_LITE3_UCV_BASE                                 (IO_UCV_BASE + 0x2F000)
#define VDEC_LITE4_UCV_BASE                                 (IO_UCV_BASE + 0x30000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x31000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x32000)
#define ETHERNET_UCV_BASE                                   (IO_UCV_BASE + 0x33000)
#define ETHERNET_PDWNC_UCV_BASE                             (IO_UCV_BASE + 0x24C00)
#define TSMUX7_UCV_BASE                                     (IO_UCV_BASE + 0x34000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x35000)
#define TSMUX9_UCV_BASE                                     (IO_UCV_BASE + 0x36000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x37000)
#define SBIM_UCV_BASE                                       (IO_UCV_BASE + 0x38000)
//ac83xx
#define SBIM_1_UCV_BASE                                             (IO_UCV_BASE+0x345000)

#define SACD_UCV_BASE                                       (IO_UCV_BASE + 0x39000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x3A000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x3B000)
#define USB2_UCV_BASE                                       (IO_UCV_BASE + 0x3C000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x3D000)
#define JPGDEC_VLD_UCV_BASE                                 (IO_UCV_BASE + 0x3E000)
#define JPGDEC_MC_UCV_BASE                                  (IO_UCV_BASE + 0x3F000)
#define IMAGE2_UCV_BASE                                     (IO_UCV_BASE + 0x40000)
#define VGFX_UCV_BASE                                       (IO_UCV_BASE + 0x41000)
#define DISP_UCV_BASE                                       (IO_UCV_BASE + 0x42000)
#define DISP2_UCV_BASE                                      (IO_UCV_BASE + 0x43000)
#define NR_UCV_BASE                                         (IO_UCV_BASE + 0x44000)
#define BIM_1_UCV_BASE                                      (IO_UCV_BASE + 0x45000)
#define RS232_2_UCV_BASE                                    (IO_UCV_BASE + 0x46000)
#define ADSP3_UCV_BASE                                      (IO_UCV_BASE + 0x47000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x48000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x49000)
#define DDMA_UCV_BASE                                       (IO_UCV_BASE + 0x4A000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x4B000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x4C000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x4D000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x4E000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x4F000)
#define PERIPHERAL_UCV_BASE                                 (IO_UCV_BASE + 0x50000)
#define RTC_UCV_BASE                                        (IO_UCV_BASE + 0x51000)
#define LARB_IPYA_UCV_BASE                                  (IO_UCV_BASE + 0x52000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x53000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x54000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x55000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x56000)
#define LARB_DISP_UCV_BASE                                  (IO_UCV_BASE + 0x57000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x58000)
//#define Resv_UCV_BASE                                       (IO_UCV_BASE + 0x59000)
#define MBIST_UCV_BASE                                      (IO_UCV_BASE + 0x5A000)
#define POST_PROC_UCV_BASE                                  (IO_UCV_BASE + 0x5B000)
#define UNKNOWN_UCV_BASE                                    (IO_UCV_BASE + 0xFF000)

#define DEMUX0_UCV_BASE                                      TSMUX0_UCV_BASE
#define DEMUX1_UCV_BASE                                      TSMUX1_UCV_BASE
#define DEMUX2_UCV_BASE                                      TSMUX2_UCV_BASE
#define DEMUX7_UCV_BASE                                      TSMUX7_UCV_BASE
#define DEMUX9_UCV_BASE                                      TSMUX9_UCV_BASE

#define PARSER_UCV_BASE                                     UNKNOWN_UCV_BASE

/* Address define */
#define DRAMA_BASE          0xC0000000
#define IO_BASE             0xf0000000

#define HEAP_BOTTOM         0x00100000


#define SRAM_OFFSET0        0x4

#define CKGEN_BASE          (IO_BASE + 0x00000)
#define BIM_BASE            (IO_BASE + 0x08000)
#define SERIAL_BASE         (IO_BASE + 0x0C000)
#define CTL_BASE            (IO_BASE + 0x0D000)
#if defined(CHIP_VER_AC83XX)
#define NFI_BASE            (IO_BASE + 0x1E400)
#define NFIECC_BASE         (IO_BASE + 0x1EC00)
#endif

#define PDWNC_BASE         	(IO_BASE + 0x24000)	
#define ETHERNET_BASE       (IO_BASE + 0x33000) 
//add by fei
#define AUD_BASE_VA     (IO_BASE_VA + 0x05000)
#define AUD_BASE            (IO_BASE + 0x05000)

#define IQR_BASE            (BIM_BASE + 0xC000)


#define REG_RO_ICEMODE      (BIM_BASE + 0x0000)
#define BIT_RISCICE         1

//#define REG_RW_REMAP        (BIM_BASE + 0x001C)
#define BIT_REMAP           1

#define REG_RW_DBOA         (BIM_BASE + 0x0020)

#define RW_BIM_7LED         (BIM_BASE + 0x010C)

#define DRAM_PARM           0x70007000

//L2 Cache Controller Memory Map
#define L2CACHE_UCV_BASE             0xA3000000  //pha = 0x50000000



#endif

