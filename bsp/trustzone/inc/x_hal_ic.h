/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef X_HAL_3363_H
#define X_HAL_3363_H


//============================================================================
// Constant definitions
//============================================================================
#define MEMORY_ALIGNMENT                                8




//============================================================================
// Memory mapping
//============================================================================
#define DRAM_B_BASE					0x00000000

#define IO_BASE						0xF0000000

#define PBI_B_BASE					0x80000000
#define PBI_A_BASE                                      0x60000000
#define DRAM_A_BASE                                     0xC0000000
#define DRAM_A_BACE                                     0xC0000000   // Should remove

//============================================================================
// IO register definitions
//============================================================================
#define CKGEN_BASE                                      (IO_BASE + 0x00000)
//#define Resv_BASE                                        (IO_BASE + 0x01000)
//#define Resv_BASE                                        (IO_BASE + 0x02000)
#define VDOUT_BASE                                      (IO_BASE + 0x42000) //MT8530_PMX
#define FMTSCL_BASE						                          (IO_BASE + 0x3000) //MT8530_PMX
#define VIDEO_IN_BASE                                   (IO_BASE + 0x3A00)
#define IMAGE_BASE                                      (IO_BASE + 0x04000)
#define AUD_BASE                                        (IO_BASE + 0x05000)
//#define Resv_BASE                                        (IO_BASE + 0x06000)
#define DRAM_BASE                                       (IO_BASE + 0x07000)
#define BIM_BASE                                        (IO_BASE + 0x08000)
//#define Resv_BASE                                        (IO_BASE + 0x09000)
#define ATA_BASE                                        (IO_BASE + 0x0A000)
#define FCIF_BASE                                       (IO_BASE + 0x0B000)
#define RS232_BASE                                      (IO_BASE + 0x0C000)
#define DRAM1_BASE                                      (IO_BASE + 0x0D000)
#define USB_BASE                                        (IO_BASE + 0x0E000)
//#define Resv_BASE                                        (IO_BASE + 0x0F000)
#define TCMGPR_BASE                                     (IO_BASE + 0x10000)
#define UPIF_BASE                                       (IO_BASE + 0x11000)
#define TSMUX0_BASE                                     (IO_BASE + 0x12000)
#define TSMUX1_BASE                                     (IO_BASE + 0x13000)  //remove in 8555
#define TSMUX2_BASE                                     (IO_BASE + 0x14000)  //remove in 8555
#define TSMUX3_BASE                                     (IO_BASE + 0x15000)
#define TSMUX4_BASE                                     (IO_BASE + 0x16000)
#define TSMUX5_BASE                                     (IO_BASE + 0x17000)
#define TSMUX6_BASE                                     (IO_BASE + 0x18000)
#define PVR_BASE                                        (IO_BASE + 0x19000)
#define VENC_BASE                                       (IO_BASE + 0x1A000)
#define VENC1_BASE                                        (IO_BASE + 0x1B000)
#define VENC2_BASE                                        (IO_BASE + 0x1C000)
#define VENC3_BASE                                        (IO_BASE + 0x1D000)

//#define MT8550_NFI2_FEATURE
#ifdef MT8550_NFI2_FEATURE
#define NFI_BASE                                        (IO_BASE + 0x1E400)    // For MT8550 new feature, NFI2_MLC
#else
#define NFI_BASE                                        (IO_BASE + 0x1E000) // For MT8550 old feature, NFI_MLC
#endif 

//#define Resv_BASE                                        (IO_BASE + 0x1F000)
#define VDOUT_MISC                                      (IO_BASE + 0x1F000)  //MT8555
#define VDOUT_AUX_BASE                                  (IO_BASE + 0x20000) //MT8530_PMX
#define HDMI_BASE                                       (IO_BASE + 0x21000)
#define PID_SWAP_BASE                                   (IO_BASE + 0x22000)
#define SPI_BASE                                        (IO_BASE + 0x23000) //MT8555
#define PDWNC_BASE                                      (IO_BASE + 0x24000)
#define GCPU_BASE                                       (IO_BASE + 0x25000)
#define DEMUX_BASE                                      (IO_BASE + 0x26000) //#define Resv_BASE                                        (IO_BASE + 0x26000)
#define VDEC_FULL0_BASE                                 (IO_BASE + 0x27000)
#define VDEC_FULL1_BASE                                 (IO_BASE + 0x28000)
#define VDEC_FULL2_BASE                                 (IO_BASE + 0x29000)
#define VDEC_FULL3_BASE                                 (IO_BASE + 0x2A000)
#define VDEC_FULL4_BASE                                 (IO_BASE + 0x2B000)
#define VDEC_FULL5_BASE                                 (IO_BASE + 0x2C000)
#define VDEC_FULL6_BASE                                 (IO_BASE + 0x2D000)
#define VDEC_LITE0_BASE                                 (IO_BASE + 0x2E000)
#define VDEC_LITE1_BASE                                 (IO_BASE + 0x2F000)
#define VDEC_LITE2_BASE                                 (IO_BASE + 0x30000)
#define VDEC_LITE3_BASE                                 (IO_BASE + 0x31000)
#define VDEC_LITE4_BASE                                 (IO_BASE + 0x32000)

#define ETHERNET_BASE                                   (IO_BASE + 0x33000)
#define ETHERNET_PDWNC_BASE                             (IO_BASE + 0x24C00)
#define VDEC_LITE5_BASE                                 (IO_BASE + 0x34000)
#define VDEC_LITE6_BASE                                 (IO_BASE + 0x35000)
#define TSMUX9_BASE                                     (IO_BASE + 0x36000)
//#define Resv_BASE                                       (IO_BASE + 0x37000)
#define SBIM_BASE                                       (IO_BASE + 0x38000)
#define SACD_BASE                                       (IO_BASE + 0x39000)
#define FMC_BASE                                        (IO_BASE + 0x3A000)
#define FTI_BASE                                       (IO_BASE + 0x3B000)
#define USB2_BASE                                       (IO_BASE + 0x3C000)
//#define TCPIP_BASE                                      (IO_BASE + 0x3D000)
#define ETHERNET_CHKSUM_BASE                            (IO_BASE + 0x3D000)
#define JPGDEC_VLD_BASE                                 (IO_BASE + 0x3E000)
#define JPGDEC_MC_BASE                                  (IO_BASE + 0x3F000)
#define IMAGE2_BASE                                     (IO_BASE + 0x40000)
#define VGFX_BASE                                       (IO_BASE + 0x41000)
#define DISP_BASE                                       (IO_BASE + 0x42000)
#define DISP2_BASE                                      (IO_BASE + 0x43000)
#define NR_BASE                                         (IO_BASE + 0x44000)
#define BIM_1_BASE                                      (IO_BASE + 0x45000)
#define RS232_2_BASE                                    (IO_BASE + 0x46000)
#define ADSP3_BASE                                      (IO_BASE + 0x47000)
//#define Resv_BASE                                       (IO_BASE + 0x48000)
//#define Resv_BASE                                       (IO_BASE + 0x49000)
#define GDMA_BASE                                       (IO_BASE + 0x4A000)
//#define Resv_BASE                                       (IO_BASE + 0x4B000)
//#define Resv_BASE                                       (IO_BASE + 0x4C000)
//#define Resv_BASE                                       (IO_BASE + 0x4D000)
//#define Resv_BASE                                       (IO_BASE + 0x4E000)
//#define Resv_BASE                                       (IO_BASE + 0x4F000)
#define PERIPHERAL_BASE                                 (IO_BASE + 0x50000)
//#define RTC_BASE                                        (IO_BASE + 0x51000) // 8555 REMOVE RTC
#define LARB_IPYA_BASE                                  (IO_BASE + 0x52000)
//#define Resv_BASE                                       (IO_BASE + 0x53000)
//#define Resv_BASE                                       (IO_BASE + 0x54000)
//#define Resv_BASE                                       (IO_BASE + 0x55000)
//#define Resv_BASE                                       (IO_BASE + 0x56000)
//#define LARB_DISP_BASE                                  (IO_BASE + 0x57000)
//#define Resv_BASE                                       (IO_BASE + 0x58000)
//#define Resv_BASE                                       (IO_BASE + 0x59000)
#define MBIST_BASE                                      (IO_BASE + 0x5A000)
#define POST_PROC_BASE                                  (IO_BASE + 0x5B000)
#define SVOIF_BASE                                      (IO_BASE + 0x5C000)
#define FE_SVOIF_BASE                                   (IO_BASE + 0x6B000)
#define UNKNOWN_BASE                                    (IO_BASE + 0xFF000)

#define DEMUX0_BASE                                      (IO_BASE + 0x12000)
#define DEMUX1_BASE                                      (IO_BASE + 0x13000)
#define DEMUX2_BASE                                      (IO_BASE + 0x14000)
#define DDI_BASE                                             (IO_BASE + 0x15000)
#define DEMUX4_BASE					   (IO_BASE + 0x16000)
#define DEMUX5_BASE					   (IO_BASE + 0x17000)
#define DEMUX6_BASE					   (IO_BASE + 0x18000)

#define DEMUX7_BASE                                      (IO_BASE + 0x26000) //Panda have to check
#define DEMUX9_BASE                                      TSMUX9_BASE

//pvr temp add
#define BLK2RS_BASE                                       (IO_BASE + 0xF0000) //NO USE
//#define PARSER0_BASE					   (IO_BASE + 0x55000)
//#define PARSER0_BASE2		                        (IO_BASE + 0x56000)

// 8530Remove
#define PARSER_BASE                                     UNKNOWN_BASE
// 8530Remove End
// 8555Add
#define MBIST                                             (IO_BASE+0x5a000 )   
#define POST_PROC                                         (IO_BASE+0x5b000 )
#define SVOIF                                             (IO_BASE+0x5C000 )
                                                          
#define SVOAHB0                                           (IO_BASE+0x60000 )
#define SVOAHB1                                           (IO_BASE+0x61000 )
#define SVOAHB2                                           (IO_BASE+0x62000 )
#define SVOAHB3                                           (IO_BASE+0x63000 )
#define SVOAHB4                                           (IO_BASE+0x64000 )
#define SVOAHB5                                           (IO_BASE+0x65000 )
#define SVOAHB6                                           (IO_BASE+0x66000 )
#define SVOAHB7                                           (IO_BASE+0x67000 )
#define SVOAHB8                                           (IO_BASE+0x68000 )
#define SVOAHB9                                           (IO_BASE+0x69000 )
#define SVOAHB10                                          (IO_BASE+0x6A000 )
#define SVOAHB11                                          (IO_BASE+0x6B000 )
#define SVOAHB12                                          (IO_BASE+0x6C000 )
#define SVOAHB13                                          (IO_BASE+0x6D000 )
#define SVOAHB14                                          (IO_BASE+0x6E000 )
#define SVOAHB15                                          (IO_BASE+0x6F000 )
#define SVOAHB16                                          (IO_BASE+0x70000 )
#define SVOAHB17                                          (IO_BASE+0x71000 )
#define SVOAHB18                                          (IO_BASE+0x72000 )
#define SVOAHB19                                          (IO_BASE+0x73000 )
#define SVOAHB20                                          (IO_BASE+0x74000 )
#define SVOAHB21                                          (IO_BASE+0x75000 )
#define SVOAHB22                                          (IO_BASE+0x76000 )
#define SVOAHB23                                          (IO_BASE+0x77000 )
#define SVOAHB24                                          (IO_BASE+0x78000 )
#define SVOAHB25                                          (IO_BASE+0x79000 )
#define SVOAHB26                                          (IO_BASE+0x7A000 )
#define SVOAHB27                                          (IO_BASE+0x7B000 )
#define SVOAHB28                                          (IO_BASE+0x7C000 )
#define SVOAHB29                                          (IO_BASE+0x7D000 )
#define SVOAHB30                                          (IO_BASE+0x7E000 )
#define SVOAHB31                                          (IO_BASE+0x7F000 )
#define SVOAHB32                                          (IO_BASE+0x80000 )
#define SVOAHB33                                          (IO_BASE+0x81000 )
#define SVOAHB34                                          (IO_BASE+0x82000 )
#define SVOAHB35                                          (IO_BASE+0x83000 )
#define SVOAHB36                                          (IO_BASE+0x84000 )
#define SVOAHB37                                          (IO_BASE+0x85000 )
#define SVOAHB38                                          (IO_BASE+0x86000 )
#define SVOAHB39                                          (IO_BASE+0x87000 )
#define SVOAHB40                                          (IO_BASE+0x88000 )
#define SVOAHB41                                          (IO_BASE+0x89000 )
#define SVOAHB42                                          (IO_BASE+0x8A000 )
#define SVOAHB43                                          (IO_BASE+0x8B000 )
#define SVOAHB44                                          (IO_BASE+0x8C000 )
#define SVOAHB45                                          (IO_BASE+0x8D000 )
#define SVOAHB46                                          (IO_BASE+0x8E000 )
#define SVOAHB47                                          (IO_BASE+0x8F000 )
#define SVOAHB48                                          (IO_BASE+0x90000 )
#define SVOAHB49                                          (IO_BASE+0x91000 )
#define SVOAHB50                                          (IO_BASE+0x92000 )
#define SVOAHB51                                          (IO_BASE+0x93000 )
#define SVOAHB52                                          (IO_BASE+0x94000 )
#define SVOAHB53                                          (IO_BASE+0x95000 )
#define SVOAHB54                                          (IO_BASE+0x96000 )
#define SVOAHB55                                          (IO_BASE+0x97000 )
#define SVOAHB56                                          (IO_BASE+0x98000 )
#define SVOAHB57                                          (IO_BASE+0x99000 )
#define SVOAHB58                                          (IO_BASE+0x9A000 )
#define SVOAHB59                                          (IO_BASE+0x9B000 )
#define SVOAHB60                                          (IO_BASE+0x9C000 )
#define SVOAHB61                                          (IO_BASE+0x9D000 )
#define SVOAHB62                                          (IO_BASE+0x9E000 )
#define SVOAHB63                                          (IO_BASE+0x9F000 )
#define TZ_DRAM                                           (IO_BASE+0x307000)
#define TZ_DRAM1                                          (IO_BASE+0x30D000)
#define TZ_PDWNC                                          (IO_BASE+0x324000)
#define SBIM_1                                            (IO_BASE+0x345000)
#define SECUIF                                            (IO_BASE+0x35C000)
#define SVOAHBTZ0                                         (IO_BASE+0x360000)
#define SVOAHBTZ1                                         (IO_BASE+0x361000)
#define SVOAHBTZ2                                         (IO_BASE+0x362000)
#define SVOAHBTZ3                                         (IO_BASE+0x363000)
#define SVOAHBTZ4                                         (IO_BASE+0x364000)
#define SVOAHBTZ5                                         (IO_BASE+0x365000)
#define SVOAHBTZ6                                         (IO_BASE+0x366000)
#define SVOAHBTZ7                                         (IO_BASE+0x367000)
#define SVOAHBTZ8                                         (IO_BASE+0x368000)
#define SVOAHBTZ9                                         (IO_BASE+0x369000)
#define SVOAHBTZ10                                        (IO_BASE+0x36A000)
#define SVOAHBTZ11                                        (IO_BASE+0x36B000)
#define SVOAHBTZ12                                        (IO_BASE+0x36C000)
#define SVOAHBTZ13                                        (IO_BASE+0x36D000)
#define SVOAHBTZ14                                        (IO_BASE+0x36E000)
#define SVOAHBTZ15                                        (IO_BASE+0x36F000)
#define SVOAHBTZ16                                        (IO_BASE+0x370000)
#define SVOAHBTZ17                                        (IO_BASE+0x371000)
#define SVOAHBTZ18                                        (IO_BASE+0x372000)
#define SVOAHBTZ19                                        (IO_BASE+0x373000)
#define SVOAHBTZ20                                        (IO_BASE+0x374000)
#define SVOAHBTZ21                                        (IO_BASE+0x375000)
#define SVOAHBTZ22                                        (IO_BASE+0x376000)
#define SVOAHBTZ23                                        (IO_BASE+0x377000)
#define SVOAHBTZ24                                        (IO_BASE+0x378000)
#define SVOAHBTZ25                                        (IO_BASE+0x379000)
#define SVOAHBTZ26                                        (IO_BASE+0x37A000)
#define SVOAHBTZ27                                        (IO_BASE+0x37B000)
#define SVOAHBTZ28                                        (IO_BASE+0x37C000)
#define SVOAHBTZ29                                        (IO_BASE+0x37D000)
#define SVOAHBTZ30                                        (IO_BASE+0x37E000)
#define SVOAHBTZ31                                        (IO_BASE+0x37F000)
#define SVOAHBTZ32                                        (IO_BASE+0x380000)
#define SVOAHBTZ33                                        (IO_BASE+0x381000)
#define SVOAHBTZ34                                        (IO_BASE+0x382000)
#define SVOAHBTZ35                                        (IO_BASE+0x383000)
#define SVOAHBTZ36                                        (IO_BASE+0x384000)
#define SVOAHBTZ37                                        (IO_BASE+0x385000)
#define SVOAHBTZ38                                        (IO_BASE+0x386000)
#define SVOAHBTZ39                                        (IO_BASE+0x387000)
#define SVOAHBTZ40                                        (IO_BASE+0x388000)
#define SVOAHBTZ41                                        (IO_BASE+0x389000)
#define SVOAHBTZ42                                        (IO_BASE+0x38A000)
#define SVOAHBTZ43                                        (IO_BASE+0x38B000)
#define SVOAHBTZ44                                        (IO_BASE+0x38C000)
#define SVOAHBTZ45                                        (IO_BASE+0x38D000)
#define SVOAHBTZ46                                        (IO_BASE+0x38E000)
#define SVOAHBTZ47                                        (IO_BASE+0x38F000)
#define SVOAHBTZ48                                        (IO_BASE+0x390000)
#define SVOAHBTZ49                                        (IO_BASE+0x391000)
#define SVOAHBTZ50                                        (IO_BASE+0x392000)
#define SVOAHBTZ51                                        (IO_BASE+0x393000)
#define SVOAHBTZ52                                        (IO_BASE+0x394000)
#define SVOAHBTZ53                                        (IO_BASE+0x395000)
#define SVOAHBTZ54                                        (IO_BASE+0x396000)
#define SVOAHBTZ55                                        (IO_BASE+0x397000)
#define SVOAHBTZ56                                        (IO_BASE+0x398000)
#define SVOAHBTZ57                                        (IO_BASE+0x399000)
#define SVOAHBTZ58                                        (IO_BASE+0x39A000)
#define SVOAHBTZ59                                        (IO_BASE+0x39B000)
#define SVOAHBTZ60                                        (IO_BASE+0x39C000)
#define SVOAHBTZ61                                        (IO_BASE+0x39D000)
#define SVOAHBTZ62                                        (IO_BASE+0x39E000)
#define SVOAHBTZ63                                        (IO_BASE+0x39F000)   
                    
//============================================================================
// PROT definitions
//============================================================================
#define IC_PROT_0                                       0x00
#define IC_PROT_1                                       0x01
#define IC_PROT_2                                       0x02
#define IC_PROT_3                                       0x03
#define IC_PROT_4                                       0x04
#define IC_PROT_5                                       0x05
#define IC_PROT_6                                       0x06
#define IC_PROT_7                                       0x07
#define IC_PROT_8                                       0x08
#define IC_PROT_9                                       0x09

//============================================================================
// TRAP definitions
//============================================================================
#define IC_TRAP_AMUTE                                   0x00
#define IC_TRAP_SFCK                                    0x01
#define IC_TRAP_SFCS                                    0x02
#define IC_TRAP_FESFDI                                  0x03
#define IC_TRAP_NFWEN                                   0x04
#define IC_TRAP_NFALE                                   0x05
#define IC_TRAP_NFCLE                                   0x06
#define IC_TRAP_NFCEN                                   0x07
#define IC_TRAP_NFREN                                   0x08
#define IC_TRAP_NFCEN2                                  0x09


#define IC_PROT_SFCK                                    IC_TRAP_SFCK
#define IC_PROT_AOSDATA0                                IC_TRAP_SFCS
#define IC_PROT_AOSDATA1                                IC_TRAP_FESFDI
#define IC_PROT_SFDO                                    IC_TRAP_NFWEN
#define IC_PROT_AOSDATA2                                IC_TRAP_NFALE
#define IC_PROT_AOSDATA4                                IC_TRAP_NFCLE
#define IC_PROT_ETTXD1                                  IC_TRAP_NFCEN

//============================================================================
// Type definitions
//============================================================================

//============================================================================
// GIC 
//==========================================================================

#define GIC_DIST_BASE                       (IO_BASE+0x1001000)
#define GIC_CPU_BASE						(IO_BASE+0x1002000)

#endif  // X_HAL_8555_H 
