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

#ifndef X_CKGEN_8317_H
#define X_CKGEN_8317_H

//============================================================================
// Include files
//============================================================================
//#include "x_hal_ic.h"
//#include "x_typedef.h"

//============================================================================
// Register definitions
//============================================================================


#define REG_RW_VERSION                  		0x0000       // AC8317 chip version code
#define REG_RW_RISC_CFG0                		0x0004       // define risc and axi clock frequency
#define REG_RW_DVD_REG1                 		0x0008      // define DVD clock register
  #define CLK_DVD_REG1_SD21_SEL_MASK              	(0xF << 24)  
  #define CLK_DVD_REG1_SD21_SEL_OFFSET            	24          
  #define CLK_DVD_REG1_SD21_SEL_CLK27M        	  	0x00
  #define CLK_DVD_REG1_SD21_SEL_MSDCPLL_D2        	0x01
  #define CLK_DVD_REG1_SD21_SEL_ARMPLL2_D2        	0x02
  #define CLK_DVD_REG1_SD21_SEL_SYSPLL_D4         	0x03
  #define CLK_DVD_REG1_SD21_SEL_USBPLL_D4         	0x04
  #define CLK_DVD_REG1_SD21_SEL_SYSPLL_D6         	0x05
  #define CLK_DVD_REG1_SD21_SEL_SYSPLL_D12        	0x06
  #define CLK_DVD_REG1_SD21_SEL_USBPLL_D10        	0x07
  #define CLK_DVD_REG1_SD21_SEL_DMPLL_D2          	0x08
  #define CLK_DVD_REG1_SD21_SEL_APLL2_D2          	0x09
  #define CLK_DVD_REG1_SD21_SEL_APLL2_D3          	0x0A
  #define CLK_DVD_REG1_SD21_SEL_APLL1_D2          	0x0B
  #define CLK_DVD_REG1_SD21_SEL_MSDCPLL_D3        	0x0C
  #define CLK_DVD_REG1_SD21_SEL_MSDCPLL_D4        	0x0D
  
  #define CLK_DVD_REG1_RFI6_SEL_MASK              	(3U << 22)  
  #define CLK_DVD_REG1_RFI6_SEL_OFFSET            	22          
  #define CLK_DVD_REG1_RFI6_SEL_27M               	0x00
  #define CLK_DVD_REG1_RFI6_SEL_SYSPLL_D20        	0x01    
  #define CLK_DVD_REG1_RFI6_SEL_CLK_APLL_26M      	0x02    
  #define CLK_DVD_REG1_RFI6_SEL_USBPLL_D15        	0x03 

  #define CLK_DVD_REG1_RFI5_SEL_MASK              	(3U << 20)  
  #define CLK_DVD_REG1_RFI5_SEL_OFFSET            	20          
  #define CLK_DVD_REG1_RFI5_SEL_27M               	0x00
  #define CLK_DVD_REG1_RFI5_SEL_SYSPLL_D20        	0x01    
  #define CLK_DVD_REG1_RFI5_SEL_CLK_APLL_26M      	0x02    
  #define CLK_DVD_REG1_RFI5_SEL_USBPLL_D15        	0x03 
  
  #define CLK_DVD_REG1_RFI4_SEL_MASK              	(3U << 18)  
  #define CLK_DVD_REG1_RFI4_SEL_OFFSET            	18          
  #define CLK_DVD_REG1_RFI4_SEL_27M               	0x00
  #define CLK_DVD_REG1_RFI4_SEL_SYSPLL_D20        	0x01    
  #define CLK_DVD_REG1_RFI4_SEL_CLK_APLL_26M      	0x02    
  #define CLK_DVD_REG1_RFI4_SEL_USBPLL_D15        	0x03 
  
  #define CLK_DVD_REG1_RFI3_SEL_MASK              	(3U << 16)  
  #define CLK_DVD_REG1_RFI3_SEL_OFFSET            	16          
  #define CLK_DVD_REG1_RFI3_SEL_27M               	0x00
  #define CLK_DVD_REG1_RFI3_SEL_SYSPLL_D20        	0x01    
  #define CLK_DVD_REG1_RFI3_SEL_CLK_APLL_26M      	0x02    
  #define CLK_DVD_REG1_RFI3_SEL_USBPLL_D15        	0x03 
  
  #define CLK_DVD_REG1_RFI2_SEL_MASK              	(3U << 14)  
  #define CLK_DVD_REG1_RFI2_SEL_OFFSET            	14          
  #define CLK_DVD_REG1_RFI2_SEL_27M               	0x00
  #define CLK_DVD_REG1_RFI2_SEL_SYSPLL_D20        	0x01    
  #define CLK_DVD_REG1_RFI2_SEL_CLK_APLL_26M      	0x02    
  #define CLK_DVD_REG1_RFI2_SEL_USBPLL_D15        	0x03 
  
  #define CLK_DVD_REG1_RFI1_SEL_MASK              	(3U << 12)  
  #define CLK_DVD_REG1_RFI1_SEL_OFFSET            	12          
  #define CLK_DVD_REG1_RFI1_SEL_27M               	0x00
  #define CLK_DVD_REG1_RFI1_SEL_SYSPLL_D20        	0x01    
  #define CLK_DVD_REG1_RFI1_SEL_CLK_APLL_26M      	0x02    
  #define CLK_DVD_REG1_RFI1_SEL_USBPLL_D15        	0x03 

  #define CLK_DVD_REG1_AUD_DVD_SEL_MASK           	(3U << 8)  
  #define CLK_DVD_REG1_AUD_DVD_SEL_OFFSET         	8          
  #define CLK_DVD_REG1_AUD_DVD_SEL_27M            	0x00
  #define CLK_DVD_REG1_AUD_DVD_SEL_ACK_K7         	0x01    
  #define CLK_DVD_REG1_AUD_DVD_SEL_ACLK_IN        	0x02    
  #define CLK_DVD_REG1_AUD_DVD_SEL_I2S_OUT2_MCLK  	0x03 

  #define CLK_DVD_REG1_DRAM_SLOW_SEL_MASK         	(0x7 << 0)  
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_OFFSET       	0          
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_CLK27M       	0x00
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_DMPLL_D2       	0x01
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_CLK_APLL1      	0x02
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_SYSPLL_D3      	0x03
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_SYSPLL_D4      	0x04
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_ADPLL_108M_CK  	0x05
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_DMPLL_D3       	0x06
  #define CLK_DVD_REG1_DRAM_SLOW_SEL_DMPLL_D4       	0x07

#define REG_RW_AP_REG0                  		0x000c       // clock select of AP
  #define CLK_REG0_RFI_SEL_MASK              		(3U << 27)  
  #define CLK_REG0_RFI_SEL_OFFSET            		27          
  #define CLK_REG0_RFI_SEL_27M               		0x00
  #define CLK_REG0_RFI_SEL_SYSPLL_D20        		0x01    
  #define CLK_REG0_RFI_SEL_APLL_26M          		0x02    
  #define CLK_REG0_RFI_SEL_USBPLL_D15        		0x03 
  
  #define CLK_REG0_TP_F32K_SEL_MASK             	(1U << 15)  
  #define CLK_REG0_TP_F32K_SEL_OFFSET           	15          
  #define CLK_REG0_TP_F32K_SEL_SYSPLL_D1500     	0x00
  #define CLK_REG0_TP_F32K_SEL_CLK_RTC          	0x01  

  #define CLK_REG0_TP_SEL_MASK              		(3U << 13)  
  #define CLK_REG0_TP_SEL_OFFSET            		13          
  #define CLK_REG0_TP_SEL_SYSPLL_D12        		0x00
  #define CLK_REG0_TP_SEL_CLK27M            		0x01
  #define CLK_REG0_TP_SEL_MPHON_IN          		0x02

  #define CLK_REG0_VDO_SEL_MASK             		(1U << 12)  
  #define CLK_REG0_VDO_SEL_OFFSET           		12          
  #define CLK_REG0_VDO_SEL_XX               		0x00
  #define CLK_REG0_VDO_SEL_YY           		0x01 

  #define CLK_REG0_RSIC_SEL_MASK            		(1U << 10)  
  #define CLK_REG0_RSIC_SEL_OFFSET          		10          
  #define CLK_REG0_RSIC_SEL_XX              		0x00
  #define CLK_REG0_RSIC_SEL_YY           		0x01

  #define CLK_REG0_DEMUX_SEL_MASK              		(7U << 6)  
  #define CLK_REG0_DEMUX_SEL_OFFSET            		6          
  #define CLK_REG0_DEMUX_SEL_CLK27M        		0x00
  #define CLK_REG0_DEMUX_SEL_APLL2_D2           	0x01
  #define CLK_REG0_DEMUX_SEL_ARMPLL_D4          	0x02
  #define CLK_REG0_DEMUX_SEL_SYSPLL_D6          	0x03
  #define CLK_REG0_DEMUX_SEL_APLL1_D2          		0x04
  #define CLK_REG0_DEMUX_SEL_SYSPLL_D3          	0x05
  #define CLK_REG0_DEMUX_SEL_SYSPLL_D4          	0x06
  #define CLK_REG0_DEMUX_SEL_DMPLL_D2          		0x07
  
  #define CLK_REG0_DSP_SEL_MASK              		(7U << 0)  
  #define CLK_REG0_DSP_SEL_OFFSET            		0  
  #define CLK_REG0_DSP_SEL_CLK27M 	   		0x00
  #define CLK_REG0_DSP_SEL_SYSPLL_D3			0x01
  #define CLK_REG0_DSP_SEL_ARMPLL2_CK			0x02
  #define CLK_REG0_DSP_SEL_USBPLL_D2		   	0x03
  #define CLK_REG0_DSP_SEL_CLK_APLL2			0x04
  #define CLK_REG0_DSP_SEL_DMPLL_CK			0x05
  #define CLK_REG0_DSP_SEL_SYSPLL_D2			0x06
  #define CLK_REG0_DSP_SEL_SYSPLL_D4			0x07

#define REG_RW_AP_REG1                  	0x0010       //
  #define CLK_REG1_USB_27M_CLK_SEL_MASK			(1U << 31)  
  #define CLK_REG1_USB_27M_CLK_SEL_OFFSET		31		  
  #define CLK_REG1_USB_27M_CLK_SEL_PRE_USB_CK   	0x00
  #define CLK_REG1_USB_27M_CLK_SEL_CLK27M		0x01

  #define CLK_REG1_OSD_SEL_MASK              		(7U << 28)  
  #define CLK_REG1_OSD_SEL_OFFSET            		28          
  #define CLK_REG1_OSD_SEL_CLK27M        		0x00
  #define CLK_REG1_OSD_SEL_SYSPLL_D10           	0x01
  #define CLK_REG1_OSD_SEL_SYSPLL_D9          		0x02
  #define CLK_REG1_OSD_SEL_SYSPLL_D8          		0x03
  #define CLK_REG1_OSD_SEL_SYSPLL_D6          		0x04
  #define CLK_REG1_OSD_SEL_APLL2_D2          		0x05
  #define CLK_REG1_OSD_SEL_SYSPLL_D4          		0x06
  #define CLK_REG1_OSD_SEL_SYSPLL_D3          		0x07

  #define CLK_REG1_DRAM_SEL_MASK			(1U << 27)  
  #define CLK_REG1_DRAM_SEL_OFFSET			27		  
  #define CLK_REG1_DRAM_SEL_CLK_ASIM   			0x00
  #define CLK_REG1_DRAM_SEL_CLK_DMPLL			0x01

  #define CLK_REG1_CLK_AXIM_SEL_MASK            	(7U << 24)  
  #define CLK_REG1_CLK_AXIM_SEL_OFFSET          	24          
  #define CLK_REG1_CLK_AXIM_SEL_CLK27M        		0x00
  #define CLK_REG1_CLK_AXIM_SEL_SYSPLL_D2       	0x01
  #define CLK_REG1_CLK_AXIM_SEL_SYSPLL_D3       	0x02
  #define CLK_REG1_CLK_AXIM_SEL_SYSPLL_D6       	0x03
  #define CLK_REG1_CLK_AXIM_SEL_ARMPLL_D2       	0x04

  #define CLK_REG1_SPM_SEL_MASK            		(3U << 21)  
  #define CLK_REG1_SPM_SEL_OFFSET          		21          
  #define CLK_REG1_SPM_SEL_CLK27M        		0x00
  #define CLK_REG1_SPM_SEL_SYSPLL_D6       		0x01
  #define CLK_REG1_SPM_SEL_DMPLL_D2       		0x02

  #define CLK_REG1_VDEC_SYS_SEL_MASK            	(7U << 18)  
  #define CLK_REG1_VDEC_SYS_SEL_OFFSET          	18          
  #define CLK_REG1_VDEC_SYS_SEL_CLK27M        		0x00
  #define CLK_REG1_VDEC_SYS_SEL_SYSPLL_D3       	0x01
  #define CLK_REG1_VDEC_SYS_SEL_DMPLL_D2        	0x02
  #define CLK_REG1_VDEC_SYS_SEL_USBPLL_D2       	0x03
  #define CLK_REG1_VDEC_SYS_SEL_CLK_APLL1       	0x04
  #define CLK_REG1_VDEC_SYS_SEL_APLL2_D2        	0x05
  #define CLK_REG1_VDEC_SYS_SEL_ARMPLL_D4       	0x06
  #define CLK_REG1_VDEC_SYS_SEL_APLL1_D2        	0x07

  #define CLK_REG1_JPEG_SEL_MASK              		(7U << 15)  
  #define CLK_REG1_JPEG_SEL_OFFSET            		15          
  #define CLK_REG1_JPEG_SEL_CLK27M        		0x00
  #define CLK_REG1_JPEG_SEL_CLK_APLL1           	0x01
  #define CLK_REG1_JPEG_SEL_APLL2_D2          		0x02
  #define CLK_REG1_JPEG_SEL_SYSPLL_D6          		0x03
  #define CLK_REG1_JPEG_SEL_SYSPLL_D3          		0x04
  #define CLK_REG1_JPEG_SEL_SYSPLL_D2          		0x05
  #define CLK_REG1_JPEG_SEL_USBPLL_D2          		0x06
  #define CLK_REG1_JPEG_SEL_CLK_APLL2          		0x07

  #define CLK_REG1_RSZ_SEL_MASK              		(7U << 12)  
  #define CLK_REG1_RSZ_SEL_OFFSET            		12          
  #define CLK_REG1_RSZ_SEL_CLK27M        		0x00
  #define CLK_REG1_RSZ_SEL_CLK_APLL1            	0x01
  #define CLK_REG1_RSZ_SEL_APLL2_D2          		0x02
  #define CLK_REG1_RSZ_SEL_SYSPLL_D6          		0x03
  #define CLK_REG1_RSZ_SEL_SYSPLL_D3          		0x04
  #define CLK_REG1_RSZ_SEL_SYSPLL_D2          		0x05
  #define CLK_REG1_RSZ_SEL_USBPLL_D2          		0x06
  #define CLK_REG1_RSZ_SEL_CLK_APLL2          		0x07

  #define CLK_REG1_FLASH_SEL_MASK               	(7U << 6)  
  #define CLK_REG1_FLASH_SEL_OFFSET            		6          
  #define CLK_REG1_FLASH_SEL_CLK27M_D8        		0x00
  #define CLK_REG1_FLASH_SEL_CLK27M_D4          	0x01
  #define CLK_REG1_FLASH_SEL_CLK27M_D2          	0x02
  #define CLK_REG1_FLASH_SEL_CLK27M          		0x03
  #define CLK_REG1_FLASH_SEL_SYSPLL_D9          	0x04
  #define CLK_REG1_FLASH_SEL_SYSPLL_D12         	0x05
  #define CLK_REG1_FLASH_SEL_SYSPLL_D16         	0x06
  #define CLK_REG1_FLASH_SEL_SYSPLL_D18         	0x07

  #define CLK_REG1_BCLK_SEL_MASK              		(7U << 0)  
  #define CLK_REG1_BCLK_SEL_OFFSET            		0          
  #define CLK_REG1_BCLK_SEL_CLK27M        		0x00
  #define CLK_REG1_BCLK_SEL_SYSPLL_D3           	0x01
  #define CLK_REG1_BCLK_SEL_SYSPLL_D4          		0x02
  #define CLK_REG1_BCLK_SEL_SYSPLL_D6          		0x03
  #define CLK_REG1_BCLK_SEL_SYSPLL_D9          		0x04
  #define CLK_REG1_BCLK_SEL_ARMPLL2_D2          	0x05
  
#define REG_RW_AP_REG2                  		0x0014       //
  #define CLK_REG2_AUD_SEL_MASK              		(3U << 28)  
  #define CLK_REG2_AUD_SEL_OFFSET            		28          
  #define CLK_REG2_AUD_SEL_CLK27M        		0x00
  #define CLK_REG2_AUD_SEL_ACLK_K2            		0x01
  #define CLK_REG2_AUD_SEL_I2S_OUT0_MCLK_INT_IN    	0x02
  #define CLK_REG2_AUD_SEL_I2S_OUT1_MCLK_INT_IN    	0x03
  
  #define CLK_REG2_G3D_SEL_MASK              		(7U << 24)  
  #define CLK_REG2_G3D_SEL_OFFSET            		24          
  #define CLK_REG2_G3D_SEL_CLK27M        		0x00
  #define CLK_REG2_G3D_SEL_SYSPLL_D2            	0x01
  #define CLK_REG2_G3D_SEL_CLK_APLL2          		0x02
  #define CLK_REG2_G3D_SEL_CLK_APLL1          		0x03
  #define CLK_REG2_G3D_SEL_USBPLL_D2          		0x04
  #define CLK_REG2_G3D_SEL_SYSPLL_D3          		0x05
  #define CLK_REG2_G3D_SEL_SYSPLL_D4          		0x06
  #define CLK_REG2_G3D_SEL_SYSPLL_D6          		0x07

  #define CLK_REG2_FPD_SEL_MASK              		(7U << 21)  
  #define CLK_REG2_FPD_SEL_OFFSET            		21          
  #define CLK_REG2_FPD_SEL_CLK27M        		0x00
  #define CLK_REG2_FPD_SEL_AD_TTL_CK_D3         	0x01
  #define CLK_REG2_FPD_SEL_AD_TTL_CK_D4         	0x02
  #define CLK_REG2_FPD_SEL_AD_TTL_CK_D6         	0x03
  #define CLK_REG2_FPD_SEL_AD_TTL_CK_D9         	0x04
  #define CLK_REG2_FPD_SEL_AD_TTL_CK_D12        	0x05
  #define CLK_REG2_FPD_SEL_AD_TTL_CK_D18       	 	0x06
  #define CLK_REG2_FPD_SEL_MHPON_IN          		0x07

  #define CLK_REG2_SD11_SEL_MASK              		(15U << 16)  
  #define CLK_REG2_SD11_SEL_OFFSET            		16          
  #define CLK_REG2_SD11_SEL_CLK27M        		0x00
  #define CLK_REG2_SD11_SEL_MSDCPLL_D2          	0x01
  #define CLK_REG2_SD11_SEL_ARMPLL2_D2        		0x02
  #define CLK_REG2_SD11_SEL_SYSPLL_D4          		0x03
  #define CLK_REG2_SD11_SEL_USBPLL_D4          		0x04
  #define CLK_REG2_SD11_SEL_SYSPLL_D6          		0x05
  #define CLK_REG2_SD11_SEL_SYSPLL_D12          	0x06
  #define CLK_REG2_SD11_SEL_USBPLL_D10          	0x07
  #define CLK_REG2_SD11_SEL_DMPLL_D2          		0x08
  #define CLK_REG2_SD11_SEL_APLL2_D2       		0x09
  #define CLK_REG2_SD11_SEL_APLL2_D3       		0x0A
  #define CLK_REG2_SD11_SEL_APLL1_D2       		0x0B
  #define CLK_REG2_SD11_SEL_MSDCPLL_D3          	0x0C
  #define CLK_REG2_SD11_SEL_MSDCPLL_D4          	0x0D

  #define CLK_REG2_SD01_SEL_MASK              		(15U << 12)  
  #define CLK_REG2_SD01_SEL_OFFSET            		12          
  #define CLK_REG2_SD01_SEL_CLK27M        		0x00
  #define CLK_REG2_SD01_SEL_MSDCPLL_D2          	0x01
  #define CLK_REG2_SD01_SEL_ARMPLL2_D2        		0x02
  #define CLK_REG2_SD01_SEL_SYSPLL_D4          		0x03
  #define CLK_REG2_SD01_SEL_USBPLL_D4          		0x04
  #define CLK_REG2_SD01_SEL_SYSPLL_D6          		0x05
  #define CLK_REG2_SD01_SEL_SYSPLL_D12          	0x06
  #define CLK_REG2_SD01_SEL_USBPLL_D10          	0x07
  #define CLK_REG2_SD01_SEL_DMPLL_D2          		0x08
  #define CLK_REG2_SD01_SEL_APLL2_D2       		0x09
  #define CLK_REG2_SD01_SEL_APLL2_D3       		0x0A
  #define CLK_REG2_SD01_SEL_APLL1_D2       		0x0B
  #define CLK_REG2_SD01_SEL_MSDCPLL_D3          	0x0C
  #define CLK_REG2_SD01_SEL_MSDCPLL_D4          	0x0D

  #define CLK_REG2_SD20_SEL_MASK              		(7U << 9)  
  #define CLK_REG2_SD20_SEL_OFFSET            		9          
  #define CLK_REG2_SD20_SEL_CLK27M        		0x00
  #define CLK_REG2_SD20_SEL_APLL2_D3            	0x01
  #define CLK_REG2_SD20_SEL_USBPLL_D6          		0x02
  #define CLK_REG2_SD20_SEL_SYSPLL_D9          		0x03
  #define CLK_REG2_SD20_SEL_USBPLL_D8          		0x04
  #define CLK_REG2_SD20_SEL_SYSPLL_D12          	0x05
  #define CLK_REG2_SD20_SEL_USBPLL_D10          	0x06
  #define CLK_REG2_SD20_SEL_SYSPLL_D18          	0x07

  #define CLK_REG2_SD10_SEL_MASK              		(7U << 6)  
  #define CLK_REG2_SD10_SEL_OFFSET            		6          
  #define CLK_REG2_SD10_SEL_CLK27M        		0x00
  #define CLK_REG2_SD10_SEL_APLL2_D3          		0x01
  #define CLK_REG2_SD10_SEL_USBPLL_D6          		0x02
  #define CLK_REG2_SD10_SEL_SYSPLL_D9          		0x03
  #define CLK_REG2_SD10_SEL_USBPLL_D8          		0x04
  #define CLK_REG2_SD10_SEL_SYSPLL_D12          	0x05
  #define CLK_REG2_SD10_SEL_USBPLL_D10          	0x06
  #define CLK_REG2_SD10_SEL_SYSPLL_D18          	0x07

  #define CLK_REG2_SD00_SEL_MASK			(7U << 3)  
  #define CLK_REG2_SD00_SEL_OFFSET			3 		 
  #define CLK_REG2_SD00_SEL_CLK27M			0x00
  #define CLK_REG2_SD00_SEL_APLL2_D3			0x01
  #define CLK_REG2_SD00_SEL_USBPLL_D6 		  	0x02
  #define CLK_REG2_SD00_SEL_SYSPLL_D9 		  	0x03
  #define CLK_REG2_SD00_SEL_USBPLL_D8 		  	0x04
  #define CLK_REG2_SD00_SEL_SYSPLL_D12		  	0x05
  #define CLK_REG2_SD00_SEL_USBPLL_D10		  	0x06
  #define CLK_REG2_SD00_SEL_SYSPLL_D18		  	0x07

  #define CLK_REG2_GRAPH_SEL_MASK              		(7U << 0)  
  #define CLK_REG2_GRAPH_SEL_OFFSET            		0          
  #define CLK_REG2_GRAPH_SEL_CLK27M        		0x00
  #define CLK_REG2_GRAPH_SEL_CLK_APLL1          	0x01
  #define CLK_REG2_GRAPH_SEL_SYSPLL_D2          	0x02
  #define CLK_REG2_GRAPH_SEL_SYSPLL_D3          	0x03
  #define CLK_REG2_GRAPH_SEL_USBPLL_D2          	0x04
  #define CLK_REG2_GRAPH_SEL_CLK_APLL2          	0x05
  #define CLK_REG2_GRAPH_SEL_DMPLL_CK           	0x06
  
#define REG_RW_AP_REG3                  	0x0018
  #define CLK_REG3_DUTY_SEL_MASK			(3U << 30)  
  #define CLK_REG3_DUTY_SEL_OFFSET 		   	30		   
  #define CLK_REG3_DUTY_SEL_CLK27M 	   		0x00
  #define CLK_REG3_DUTY_SEL_CLK27M_D2 		   	0x01
  #define CLK_REG3_DUTY_SEL_CLK27M_D4 		   	0x02
  #define CLK_REG3_DUTY_SEL_CLK27M_D8 		   	0x03

  #define CLK_REG3_DEG_SEL_MASK              		(3U << 28)  
  #define CLK_REG3_DEG_SEL_OFFSET            		28          
  #define CLK_REG3_DEG_SEL_CLK27M        		0x00
  #define CLK_REG3_DEG_SEL_CLK27M_D16           	0x01
  #define CLK_REG3_DEG_SEL_CLK27M_D4           		0x02
  #define CLK_REG3_DEG_SEL_CLK27M_D2           		0x03

  #define CLK_REG3_NF_SEL_MASK              		(7U << 24)  
  #define CLK_REG3_NF_SEL_OFFSET            		24          
  #define CLK_REG3_NF_SEL_CLK27M        		0x00
  #define CLK_REG3_NF_SEL_APLL2_D2            		0x01
  #define CLK_REG3_NF_SEL_APLL1_D2          		0x02
  #define CLK_REG3_NF_SEL_SYSPLL_D4          		0x03
  #define CLK_REG3_NF_SEL_ARMPLL_D4          		0x04
  #define CLK_REG3_NF_SEL_SYSPLL_D6          		0x05

  #define CLK_REG3_BT_MIC_AUD_SEL_MASK          	(3U << 18)  
  #define CLK_REG3_BT_MIC_AUD_SEL_OFFSET        	18          
  #define CLK_REG3_BT_MIC_AUD_SEL_CLK27M        	0x00
  #define CLK_REG3_BT_MIC_AUD_SEL_ACK_K9        	0x01
  #define CLK_REG3_BT_MIC_AUD_SEL_BT_MCLK_IN    	0x02
  #define CLK_REG3_BT_MIC_AUD_SEL_MPHON_IN      	0x03

  #define CLK_REG3_ARM_AUD_SEL_MASK             	(3U << 12)  
  #define CLK_REG3_ARM_AUD_SEL_OFFSET           	12          
  #define CLK_REG3_ARM_AUD_SEL_CLK27M        		0x00
  #define CLK_REG3_ARM_AUD_SEL_ACK_K8           	0x01
  #define CLK_REG3_ARM_AUD_SEL_ACLK_IN          	0x02
  #define CLK_REG3_ARM_AUD_SEL_MPHON_IN         	0x03

  #define CLK_REG3_MPHON_SEL_MASK              		(3U << 6)  
  #define CLK_REG3_MPHON_SEL_OFFSET            		6          
  #define CLK_REG3_MPHON_SEL_CLK27M        		0x00
  #define CLK_REG3_MPHON_SEL_ACK_K6            		0x01
  #define CLK_REG3_MPHON_SEL_MPHONE_IN          	0x02
  #define CLK_REG3_MPHON_SEL_SPMCLK_IN          	0x03

  #define CLK_REG3_CPU2_SEL_MASK              		(3U << 4)  
  #define CLK_REG3_CPU2_SEL_OFFSET            		4          
  #define CLK_REG3_CPU2_SEL_CLK27M        		0x00
  #define CLK_REG3_CPU2_SEL_CLK_ARMPLL2         	0x01
  #define CLK_REG3_CPU2_SEL_SYSPLL_D2          		0x02
  #define CLK_REG3_CPU2_SEL_ARMPLL_D2          		0x03

  #define CLK_REG3_CPU1_SEL_MASK              		(3U << 2)  
  #define CLK_REG3_CPU1_SEL_OFFSET            		2          
  #define CLK_REG3_CPU1_SEL_CLK27M        		0x00
  #define CLK_REG3_CPU1_SEL_CLK_ARMPLL          	0x01
  #define CLK_REG3_CPU1_SEL_CLK_SYSPLL          	0x02
  #define CLK_REG3_CPU1_SEL_CLK_DMPLL          		0x03

  #define CLK_REG3_AUD2_SEL_MASK              		(3U <<0)  
  #define CLK_REG3_AUD2_SEL_OFFSET            		0          
  #define CLK_REG3_AUD2_SEL_CLK27M        		0x00
  #define CLK_REG3_AUD2_SEL_ACLK_K4            		0x01
  #define CLK_REG3_AUD2_SEL_I2S_OUT0_MCLK_INT_IN    	0x02
  #define CLK_REG3_AUD2_SEL_I2S_OUT1_MCLK_INT_IN    	0x03
  
#define REG_RW_AP_REG4                  	0x001c 
#define REG_RW_AP_REG5                  	0x0020   
#define REG_RW_AP_REG6                  	0x0024
#define REG_RW_AP_REG7                  	0x0028
  #define CLK_REG7_APLL_K8_SEL_MASK             	(3U << 30)  
  #define CLK_REG7_APLL_K8_SEL_OFFSET         		30          
  #define CLK_REG7_APLL_K8_SEL_CLK_APLL1      		0x00
  #define CLK_REG7_APLL_K8_SEL_CLK_APLL2      		0x01

  #define CLK_REG7_APLL_K7_SEL_MASK			(3U << 28)  
  #define CLK_REG7_APLL_K7_SEL_OFFSET 		  	28		  
  #define CLK_REG7_APLL_K7_SEL_CLK_APLL1		0x00
  #define CLK_REG7_APLL_K7_SEL_CLK_APLL2		0x01

  #define CLK_REG7_APLL_K6_SEL_MASK			(3U << 26)  
  #define CLK_REG7_APLL_K6_SEL_OFFSET 		  	26		
  #define CLK_REG7_APLL_K6_SEL_CLK_APLL1		0x00
  #define CLK_REG7_APLL_K6_SEL_CLK_APLL2		0x01

  #define CLK_REG7_APLL_K5_SEL_MASK			(3U << 24)  
  #define CLK_REG7_APLL_K5_SEL_OFFSET 		  	24		
  #define CLK_REG7_APLL_K5_SEL_CLK_APLL1		0x00
  #define CLK_REG7_APLL_K5_SEL_CLK_APLL2		0x01

  #define CLK_REG7_APLL_K4_SEL_MASK			(3U << 22)  
  #define CLK_REG7_APLL_K4_SEL_OFFSET 		  	22		
  #define CLK_REG7_APLL_K4_SEL_CLK_APLL1		0x00
  #define CLK_REG7_APLL_K4_SEL_CLK_APLL2		0x01

  #define CLK_REG7_APLL_K3_SEL_MASK			(3U << 20)  
  #define CLK_REG7_APLL_K3_SEL_OFFSET 		  	20		
  #define CLK_REG7_APLL_K3_SEL_CLK_APLL1		0x00
  #define CLK_REG7_APLL_K3_SEL_CLK_APLL2		0x01

  #define CLK_REG7_APLL_K2_SEL_MASK			(3U << 18)  
  #define CLK_REG7_APLL_K2_SEL_OFFSET 		  	18		
  #define CLK_REG7_APLL_K2_SEL_CLK_APLL1		0x00
  #define CLK_REG7_APLL_K2_SEL_CLK_APLL2		0x01

  #define CLK_REG7_APLL_K1_SEL_MASK			(3U << 16)  
  #define CLK_REG7_APLL_K1_SEL_OFFSET 		  	16		
  #define CLK_REG7_APLL_K1_SEL_CLK_APLL1		0x00
  #define CLK_REG7_APLL_K1_SEL_CLK_APLL2		0x01

#define REG_RW_AP_REG8                  	0x002c
  #define CLK_REG8_MLIN_SEL_MASK			(3U << 29)  
  #define CLK_REG8_MLIN_SEL_OFFSET 		        29		  
  #define CLK_REG8_MLIN_SEL_CLK27M 		        0x00
  #define CLK_REG8_MLIN_SEL_ACLK_K3		        0x01
  #define CLK_REG8_MLIN_SEL_SPMCK2_IN 		    	0x02
  #define CLK_REG8_MLIN_SEL_SPMCK_IN	        	0x03

  #define CLK_REG8_AUD_MPH_SEL_MASK             	(3U << 27)  
  #define CLK_REG8_AUD_MPH_SEL_OFFSET           	27          
  #define CLK_REG8_AUD_MPH_SEL_CLK27M        		0x00
  #define CLK_REG8_AUD_MPH_SEL_ACLK_K13         	0x01
  #define CLK_REG8_AUD_MPH_SEL_ACK_IN          	 	0x02
  #define CLK_REG8_AUD_MPH_SEL_SPMCK_IN         	0x03

  #define CLK_REG8_MLIN2_SEL_MASK			(3U << 25)  
  #define CLK_REG8_MLIN2_SEL_OFFSET 		    	25		  
  #define CLK_REG8_MLIN2_SEL_CLK27M 		    	0x00
  #define CLK_REG8_MLIN2_SEL_ACLK_K12		    	0x01
  #define CLK_REG8_MLIN2_SEL_SPMCK_IN 		    	0x02
  #define CLK_REG8_MLIN2_SEL_SPMCK2_IN		    	0x03

  #define CLK_REG8_AUD_PWM_SEL_MASK             	(3U << 23)  
  #define CLK_REG8_AUD_PWM_SEL_OFFSET           	23          
  #define CLK_REG8_AUD_PWM_SEL_CLK27M           	0x00
  #define CLK_REG8_AUD_PWM_SEL_ACLK_K11         	0x01
  #define CLK_REG8_AUD_PWM_SEL_ACLK_K5          	0x02
  #define CLK_REG8_AUD_PWM_SEL_MPHON_IN         	0x03

  #define CLK_REG8_AUD_ADC_SEL_MASK             	(3U << 21)  
  #define CLK_REG8_AUD_ADC_SEL_OFFSET           	21          
  #define CLK_REG8_AUD_ADC_SEL_CLK27M           	0x00
  #define CLK_REG8_AUD_ADC_SEL_ACLK_K10         	0x01
  #define CLK_REG8_AUD_ADC_SEL_ACLK_K5          	0x02
  #define CLK_REG8_AUD_ADC_SEL_MPHONE_IN        	0x03

  #define CLK_REG8_PLL_TEST_SEL_MASK			(1U << 19)  
  #define CLK_REG8_PLL_TEST_SEL_OFFSET 		    	19	  
  #define CLK_REG8_PLL_TEST_SEL_CLK_NORMAL_PLL  	0x00
  #define CLK_REG8_PLL_TEST_SEL_CLK_TEST_IN		0x01

  #define CLK_REG8_APLL_A3_SEL_MASK			(3U << 16)  
  #define CLK_REG8_APLL_A3_SEL_OFFSET 		    	16	  
  #define CLK_REG8_APLL_A3_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_A3_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_A3_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_A3_SEL_CLK_SYSPLL_D2		0x03

  #define CLK_REG8_APLL_A2_SEL_MASK			(3U << 14)  
  #define CLK_REG8_APLL_A2_SEL_OFFSET 		    	14	  
  #define CLK_REG8_APLL_A2_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_A2_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_A2_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_A2_SEL_CLK_SYSPLL_D2		0x03

  #define CLK_REG8_APLL_A1_SEL_MASK			(3U << 12)  
  #define CLK_REG8_APLL_A1_SEL_OFFSET 		    	12	  
  #define CLK_REG8_APLL_A1_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_A1_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_A1_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_A1_SEL_CLK_SYSPLL_D2		0x03

  #define CLK_REG8_APLL_K14_SEL_MASK			(3U << 10)  
  #define CLK_REG8_APLL_K14_SEL_OFFSET 		    	10	  
  #define CLK_REG8_APLL_K14_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_K14_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_K14_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_K14_SEL_CLK_SYSPLL_D2		0x03

  #define CLK_REG8_APLL_K13_SEL_MASK			(3U << 8)  
  #define CLK_REG8_APLL_K13_SEL_OFFSET 		    	8	  
  #define CLK_REG8_APLL_K13_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_K13_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_K13_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_K13_SEL_CLK_SYSPLL_D2   	0x03

  #define CLK_REG8_APLL_K12_SEL_MASK			(3U << 6)  
  #define CLK_REG8_APLL_K12_SEL_OFFSET 		    	6	  
  #define CLK_REG8_APLL_K12_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_K12_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_K12_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_K12_SEL_CLK_SYSPLL_D2		0x03

  #define CLK_REG8_APLL_K11_SEL_MASK			(3U << 4)  
  #define CLK_REG8_APLL_K11_SEL_OFFSET 		    	4	  
  #define CLK_REG8_APLL_K11_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_K11_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_K11_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_K11_SEL_CLK_SYSPLL_D2		0x03

  #define CLK_REG8_APLL_K10_SEL_MASK			(3U << 2)  
  #define CLK_REG8_APLL_K10_SEL_OFFSET 		    	2	  
  #define CLK_REG8_APLL_K10_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_K10_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_K10_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_K10_SEL_CLK_SYSPLL_D2		0x03

  #define CLK_REG8_APLL_K9_SEL_MASK			(3U << 0)  
  #define CLK_REG8_APLL_K9_SEL_OFFSET 		    	0	  
  #define CLK_REG8_APLL_K9_SEL_CLK_APLL1		0x00
  #define CLK_REG8_APLL_K9_SEL_CLK_APLL2		0x01
  #define CLK_REG8_APLL_K9_SEL_CLK_HADDS2		0x02
  #define CLK_REG8_APLL_K9_SEL_CLK_SYSPLL_D2		0x03
  
#define REG_RW_AP_REG9                  0x0030
  #define CLK_REG9_AUD_K5_TST_SEL_MASK          (1U << 30)  
  #define CLK_REG9_AUD_K5_TST_SEL_OFFSET        30          
  #define CLK_REG9_AUD_K5_TST_SEL_ACLK_K5       0x00
  #define CLK_REG9_AUD_K5_TST_SEL_MPHON_IN      0x01

  #define CLK_REG9_AUD_A3_TST_SEL_MASK          (1U << 29)  
  #define CLK_REG9_AUD_A3_TST_SEL_OFFSET        29          
  #define CLK_REG9_AUD_A3_TST_SEL_ACLK_A3       0x00
  #define CLK_REG9_AUD_A3_TST_SEL_MPHON_IN      0x01

  #define CLK_REG9_AUD_A2_TST_SEL_MASK          (1U << 28)  
  #define CLK_REG9_AUD_A2_TST_SEL_OFFSET        28          
  #define CLK_REG9_AUD_A2_TST_SEL_ACLK_A2       0x00
  #define CLK_REG9_AUD_A2_TST_SEL_MPHON_IN      0x01

  #define CLK_REG9_AUD_A1_TST_SEL_MASK          (1U << 27)  
  #define CLK_REG9_AUD_A1_TST_SEL_OFFSET        27          
  #define CLK_REG9_AUD_A1_TST_SEL_ACLK_A1       0x00
  #define CLK_REG9_AUD_A1_TST_SEL_MPHON_IN      0x01

  #define CLK_REG9_MLIN_TST_SEL_MASK            (1U << 26)  
  #define CLK_REG9_MLIN_TST_SEL_OFFSET          26          
  #define CLK_REG9_MLIN_TST_SEL_ACLK_K3         0x00
  #define CLK_REG9_MLIN_TST_SEL_MPHON_IN        0x01

  #define CLK_REG9_INV_TVD_CLK_MASK             (1U << 25)  
  #define CLK_REG9_INV_TVD_CLK_OFFSET           25          
  #define CLK_REG9_INV_TVD_CLK_KEEP_NORMAL      0x00
  #define CLK_REG9_INV_TVD_CLK_INVERT           0x01
  
  #define CLK_REG9_DA_APLL1CK_SEL_MASK          (7U << 21)  
  #define CLK_REG9_DA_APLL1CK_SEL_OFFSET        21          
  #define CLK_REG9_DA_APLL1CK_SEL_CLK27M        0x00
  #define CLK_REG9_DA_APLL1CK_SEL_ACK_IN        0x01
  #define CLK_REG9_DA_APLL1CK_SEL_I2S_OUT2_MCLK 0x02
  #define CLK_REG9_DA_APLL1CK_SEL_MPHON_IN      0x03
  #define CLK_REG9_DA_APLL1CK_SEL_SPMCLK_IN     0x04

  #define CLK_REG9_DA_APLLCK_SEL_MASK           (7U << 18)  
  #define CLK_REG9_DA_APLLCK_SEL_OFFSET         18          
  #define CLK_REG9_DA_APLLCK_SEL_CLK27M         0x00
  #define CLK_REG9_DA_APLLCK_SEL_ACK_IN         0x01
  #define CLK_REG9_DA_APLLCK_SEL_I2S_OUT2_MCLK  0x02
  #define CLK_REG9_DA_APLLCK_SEL_MPHON_IN       0x03
  #define CLK_REG9_DA_APLLCK_SEL_SPMCLK_IN      0x04

  #define CLK_REG9_MCLK_D2_SEL_MASK             (1U << 16)  
  #define CLK_REG9_MCLK_D2_SEL_OFFSET           16          
  #define CLK_REG9_MCLK_D2_SEL_CLK_MCLK         0x00
  #define CLK_REG9_MCLK_D2_SEL_CLK_MCLK_D2      0x01

  #define CLK_REG9_CLK_SRAMIF_SEL_MASK          (7U << 12)  
  #define CLK_REG9_CLK_SRAMIF_SEL_OFFSET        12          
  #define CLK_REG9_CLK_SRAMIF_SEL_CLK27M        0x00
  #define CLK_REG9_CLK_SRAMIF_SEL_SYSPLL_D2     0x01
  #define CLK_REG9_CLK_SRAMIF_SEL_CLK_APLL2     0x02
  #define CLK_REG9_CLK_SRAMIF_SEL_CLK_APLL1     0x03
  #define CLK_REG9_CLK_SRAMIF_SEL_USBPLL_D2     0x04
  #define CLK_REG9_CLK_SRAMIF_SEL_SYSPLL_D3     0x05
  #define CLK_REG9_CLK_SRAMIF_SEL_SYSPLL_D4     0x06
  #define CLK_REG9_CLK_SRAMIF_SEL_SYSPLL_D6     0x07

  #define CLK_REG9_INV_TVD_27M_CK_MASK          (1U << 11)  
  #define CLK_REG9_INV_TVD_27M_CK_OFFSET        11          
  #define CLK_REG9_INV_TVD_27M_CK_KEEP_NORMAL   0x00
  #define CLK_REG9_INV_TVD_27M_CK_INVERT        0x01
  
  #define CLK_REG9_TVD_MBIST_SEL_MASK           (1U << 10)  
  #define CLK_REG9_TVD_MBIST_SEL_OFFSET         10          
  #define CLK_REG9_TVD_MBIST_SEL_CVBS_ADC       0x00
  #define CLK_REG9_TVD_MBIST_SEL_SYSPLL_D12     0x01

  #define CLK_REG9_SPI_MOTO_SEL_MASK            (3U << 8)  
  #define CLK_REG9_SPI_MOTO_SEL_OFFSET          8          
  #define CLK_REG9_SPI_MOTO_SEL_CLK27M          0x00
  #define CLK_REG9_SPI_MOTO_SEL_SYSPLL_D3       0x01
  #define CLK_REG9_SPI_MOTO_SEL_SYSPLL_D6       0x02
  #define CLK_REG9_SPI_MOTO_SEL_APLL2_D2        0x03

  #define CLK_REG9_PNG_SEL_MASK              	(7U << 5)  
  #define CLK_REG9_PNG_SEL_OFFSET            	5          
  #define CLK_REG9_PNG_SEL_CLK27M        		0x00
  #define CLK_REG9_PNG_SEL_CLK_APLL1            0x01
  #define CLK_REG9_PNG_SEL_APLL2_D2          	0x02
  #define CLK_REG9_PNG_SEL_SYSPLL_D6          	0x03
  #define CLK_REG9_PNG_SEL_SYSPLL_D3          	0x04
  #define CLK_REG9_PNG_SEL_SYSPLL_D2          	0x05
  #define CLK_REG9_PNG_SEL_USBPLL_D2          	0x06
  #define CLK_REG9_PNG_SEL_CLK_APLL2          	0x07

  #define CLK_REG9_TS1_SEL_MASK                 (3U <<2)  
  #define CLK_REG9_TS1_SEL_OFFSET               2          
  #define CLK_REG9_TS1_SEL_NOTHING              0x00
  #define CLK_REG9_TS1_SEL_EXT_TS1              0x01
  #define CLK_REG9_TS1_SEL_DEMUX                0x02

  #define CLK_REG9_TS0_SEL_MASK                 (3U <<0)  
  #define CLK_REG9_TS0_SEL_OFFSET               0          
  #define CLK_REG9_TS0_SEL_NOTHING              0x00
  #define CLK_REG9_TS0_SEL_EXT_TS0              0x01
  #define CLK_REG9_TS0_SEL_DEMUX                0x02
  
#define REG_RW_AP_REG10                 0x0034
  #define CLK_REG10_SIFS1_SEL_MASK              (7U << 28)  
  #define CLK_REG10_SIFS1_SEL_OFFSET            28          
  #define CLK_REG10_SIFS1_SEL_CLK27M            0x00
  #define CLK_REG10_SIFS1_SEL_SYSPLL_D18        0x01
  #define CLK_REG10_SIFS1_SEL_SYSPLL_D9         0x02
  #define CLK_REG10_SIFS1_SEL_USBPLL_D4         0x03
  #define CLK_REG10_SIFS1_SEL_USBPLL_D2         0x04

  #define CLK_REG10_SIFS0_SEL_MASK              (7U << 25)  
  #define CLK_REG10_SIFS0_SEL_OFFSET            25          
  #define CLK_REG10_SIFS0_SEL_CLK27M            0x00
  #define CLK_REG10_SIFS0_SEL_SYSPLL_D18        0x01
  #define CLK_REG10_SIFS0_SEL_SYSPLL_D9         0x02
  #define CLK_REG10_SIFS0_SEL_USBPLL_D4         0x03
  #define CLK_REG10_SIFS0_SEL_USBPLL_D2         0x04

  #define CLK_REG10_SIFM1_SEL_MASK              (7U << 22)  
  #define CLK_REG10_SIFM1_SEL_OFFSET            22          
  #define CLK_REG10_SIFM1_SEL_CLK27M            0x00
  #define CLK_REG10_SIFM1_SEL_SYSPLL_D18        0x01
  #define CLK_REG10_SIFM1_SEL_SYSPLL_D9         0x02
  #define CLK_REG10_SIFM1_SEL_USBPLL_D4         0x03
  #define CLK_REG10_SIFM1_SEL_USBPLL_D2         0x04

  #define CLK_REG10_SIFM0_SEL_MASK              (7U << 19)  
  #define CLK_REG10_SIFM0_SEL_OFFSET            19          
  #define CLK_REG10_SIFM0_SEL_CLK27M            0x00
  #define CLK_REG10_SIFM0_SEL_SYSPLL_D18        0x01
  #define CLK_REG10_SIFM0_SEL_SYSPLL_D9         0x02
  #define CLK_REG10_SIFM0_SEL_USBPLL_D4         0x03
  #define CLK_REG10_SIFM0_SEL_USBPLL_D2         0x04
  
  #define CLK_REG10_PWM3_SEL_MASK               (7U << 16)  
  #define CLK_REG10_PWM3_SEL_OFFSET             16          
  #define CLK_REG10_PWM3_SEL_CLK27M             0x00
  #define CLK_REG10_PWM3_SEL_SYSPLL_D3          0x01
  #define CLK_REG10_PWM3_SEL_SYSPLL_D6          0x02
  #define CLK_REG10_PWM3_SEL_CLK_APLL2          0x03
  #define CLK_REG10_PWM3_SEL_SYSPLL_D2          0x04
  #define CLK_REG10_PWM3_SEL_ARMPLL_D2          0x05
  #define CLK_REG10_PWM3_SEL_DMPLL_D2           0x06
  #define CLK_REG10_PWM3_SEL_CLK_APLL1          0x07

  #define CLK_REG10_PWM2_SEL_MASK               (7U << 13)  
  #define CLK_REG10_PWM2_SEL_OFFSET             13          
  #define CLK_REG10_PWM2_SEL_CLK27M             0x00
  #define CLK_REG10_PWM2_SEL_SYSPLL_D3          0x01
  #define CLK_REG10_PWM2_SEL_SYSPLL_D6          0x02
  #define CLK_REG10_PWM2_SEL_CLK_APLL2          0x03
  #define CLK_REG10_PWM2_SEL_SYSPLL_D2          0x04
  #define CLK_REG10_PWM2_SEL_ARMPLL_D2          0x05
  #define CLK_REG10_PWM2_SEL_DMPLL_D2           0x06
  #define CLK_REG10_PWM2_SEL_CLK_APLL1          0x07

  #define CLK_REG10_PWM1_SEL_MASK               (7U << 9)  
  #define CLK_REG10_PWM1_SEL_OFFSET             9          
  #define CLK_REG10_PWM1_SEL_CLK27M             0x00
  #define CLK_REG10_PWM1_SEL_SYSPLL_D3          0x01
  #define CLK_REG10_PWM1_SEL_SYSPLL_D6          0x02
  #define CLK_REG10_PWM1_SEL_CLK_APLL2          0x03
  #define CLK_REG10_PWM1_SEL_SYSPLL_D2          0x04
  #define CLK_REG10_PWM1_SEL_ARMPLL_D2          0x05
  #define CLK_REG10_PWM1_SEL_DMPLL_D2           0x06
  #define CLK_REG10_PWM1_SEL_CLK_APLL1          0x07

  #define CLK_REG10_PWM0_SEL_MASK               (7U << 6)  
  #define CLK_REG10_PWM0_SEL_OFFSET             6          
  #define CLK_REG10_PWM0_SEL_CLK27M             0x00
  #define CLK_REG10_PWM0_SEL_SYSPLL_D3          0x01
  #define CLK_REG10_PWM0_SEL_SYSPLL_D6          0x02
  #define CLK_REG10_PWM0_SEL_CLK_APLL2          0x03
  #define CLK_REG10_PWM0_SEL_SYSPLL_D2          0x04
  #define CLK_REG10_PWM0_SEL_ARMPLL_D2          0x05
  #define CLK_REG10_PWM0_SEL_DMPLL_D2           0x06
  #define CLK_REG10_PWM0_SEL_CLK_APLL1          0x07

  #define CLK_REG10_APLL_26M_SEL_MASK           (1U << 5)  
  #define CLK_REG10_APLL_26M_SEL_OFFSET         5          
  #define CLK_REG10_APLL_26M_SEL_APLL_26M       0x00
  #define CLK_REG10_APLL_26M_SEL_MPHON_IN       0x01

  #define CLK_REG10_TWDS_SEL_MASK               (1U << 4)  
  #define CLK_REG10_TWDS_SEL_OFFSET             4          
  #define CLK_REG10_TWDS_SEL_TTL                0x00
  #define CLK_REG10_TWDS_SEL_TWDS_DPIX          0x01

  #define CLK_REG10_LVDS_SEL_MASK               (1U << 3)  
  #define CLK_REG10_LVDS_SEL_OFFSET             3          
  #define CLK_REG10_LVDS_SEL_TTL                0x00
  #define CLK_REG10_LVDS_SEL_LVDS_DPIX          0x01

#define REG_RW_GPIO_EN_CFG0             0x0074
#define REG_RW_GPIO_EN_CFG1             0x0078
#define REG_RW_GPIO_EN_CFG2             0x007c
#define REG_RW_GPIO_EN_CFG3             0x0080
#define REG_RW_GPIO_EN_CFG4             0x0084
#define REG_RW_GPIO_EN_CFG5             0x0088

#define REG_RW_MBIST_CFG0               0x008C

#define REG_RW_GPIO_EN_CFG6             0x014C
#define REG_RW_GPIO_EN_CFG7             0x0090

#define REG_RW_MISC_CONTROL             0x0094       // top lever misc control 0
	#define MISC_SPI_SEL_MASK		  	(3U << 30)  
	#define MISC_SPI_SEL_OFFSET		  	30		  
	#define MISC_SPI_SEL_MTK_TO_SP0 		0x00
	#define MISC_SPI_SEL_MOTO1SP0_MTKSP1		0x01
	#define MISC_SPI_SEL_MOTO2SP1_MTKSP0 		0x02
	#define MISC_SPI_SEL_MOTO1SP0_MOTO2SP1		0x03

	#define MISC_USB_FT_MODE_1P_SEL_MASK		(1U << 16)  
	#define MISC_USB_FT_MODE_1P_SEL_OFFSET		16		  
	#define MISC_USB_FT_MODE_1P_SEL_NORMAL	  	0x00
	#define MISC_USB_FT_MODE_1P_SEL_FT 	  	0x01

	#define MISC_USB_FT_MODE_0P_SEL_MASK		(1U << 15)  
	#define MISC_USB_FT_MODE_0P_SEL_OFFSET		15		  
	#define MISC_USB_FT_MODE_0P_SEL_NORMAL	  	0x00
	#define MISC_USB_FT_MODE_0P_SEL_FT	  	0x01

	#define MISC_MSDC2_DRAM_AGENT_SEL_MASK		(1U << 14)  
	#define MISC_MSDC2_DRAM_AGENT_SEL_OFFSET	14		  
	#define MISC_MSDC2_DRAM_AGENT_SEL_AGENT2	0x00
	#define MISC_MSDC2_DRAM_AGENT_SEL_AGENT3	0x01

	#define MISC_MSDC1_DRAM_AGENT_SEL_MASK		(1U << 13)  
	#define MISC_MSDC1_DRAM_AGENT_SEL_OFFSET	13		  
	#define MISC_MSDC1_DRAM_AGENT_SEL_AGENT1	0x00
	#define MISC_MSDC1_DRAM_AGENT_SEL_AGENT3	0x01

	#define MISC_MSDC0_DRAM_AGENT_SEL_MASK		(1U << 12)  
	#define MISC_MSDC0_DRAM_AGENT_SEL_OFFSET	12		  
	#define MISC_MSDC0_DRAM_AGENT_SEL_AGENT0	0x00
	#define MISC_MSDC0_DRAM_AGENT_SEL_AGENT4	0x01

	#define MISC_DVD_DTS_BOUND_MASK		  	(1U << 7)  
	#define MISC_DVD_DTS_BOUND_OFFSET		7		  
	#define MISC_DVD_DTS_BOUND_DISABLE	  	0x00
	#define MISC_DVD_DTS_BOUND_ENABLE	  	0x01

	#define MISC_DEBUG_TEST_SEL_MASK 	  	(1U << 6)  
	#define MISC_DEBUG_TEST_SEL_OFFSET		  6 	  
	#define MISC_DEBUG_TEST_SEL_DEBUG_SIG_OUT	0x00
	#define MISC_DEBUG_TEST_SEL_NORMAL	  	0x01

	#define MISC_TEST_RISC_INHIBIT_MASK	  	(1U << 2)  
	#define MISC_TEST_RISC_INHIBIT_OFFSET		2 	  
	#define MISC_TEST_RISC_INHIBIT_ENABLE	  	0x00
	#define MISC_TEST_RISC_INHIBIT_DISABLE	  	0x01

	#define MISC_RG_RST_8032_MASK	  		(1U << 1)  
	#define MISC_RG_RST_8032_OFFSET		  	1 
	#define MISC_RG_RST_8032_DISABLE	  	0x00
	#define MISC_RG_RST_8032_ENABLE	  		0x01

	#define MISC_DVD_ACTIVE_CFG_MASK	  	(1U << 0)  
	#define MISC_DVD_ACTIVE_CFG_OFFSET 	  	0 
	#define MISC_DVD_ACTIVE_CFG_DISABLE	  	0x00
	#define MISC_DVD_ACTIVE_CFG_ENABLE   		0x01

#define REG_RW_CLKGATE_CFG0             	0x009c       // vdec clock gated
    #define CLK_PDN_VDEC_FULL_MASK             		(1U << 0)  //turn off VDEC FULL
    #define CLK_PDN_VDEC_FULL_OFFSET	  		0 
    #define CLK_PDN_VDEC_FULL_DISABLE   		0x00
    #define CLK_PDN_VDEC_FULL_ENABLE	 		0x01

#define REG_RW_CLKGATE_CFG1             	0x00a0       // img_dramc gated
  #define CLK_PDN_GFX	                    		(1U << 0)//
  #define CLK_PDN_DMARB                     		(1U << 1)//
  #define CLK_PDN_PNG	                    		(1U << 2) //
  #define CLK_PDN_GIF	                    		(1U << 3) //
  #define CLK_PDN_IMG_RESZ                		(1U << 4)//5  
  #define CLK_PDN_OSD_RESZ	              		(1U << 5)//6  
  #define CLK_PDN_JPGDEC                  		(1U << 6)//7  
  #define CLK_PDN_DEMUX	                  		(1U << 7)//8  
  #define CLK_PDN_DEMUX_TS0	              		(1U << 8)//9  
  #define CLK_PDN_DEMUX_TS1	              		(1U << 9)//10 
  #define CLK_PDN_DEMUX_27M	              		(1U << 11)//12 
  #define CLK_PDN_NFI	              		    	(1U << 12)//13 
  #define CLK_PDN_USB					(1U << 13)//13 
  #define CLK_PDN_IRT_DMA_WRAPPER			(1U << 14)//13 
  #define CLK_PDN_ARM9					(1U << 15)//13 
  #define CLK_PDN_TS0_CLK_INV	          		(1U << 24)//13 
  #define CLK_PDN_TS1_CLK_INV	          		(1U << 25)//13 

#define REG_RW_CLKGATE_CFG2             0x00a4       // audio clk select
  #define CLK_PDN_DIGMIC_CLK_INV_SEL_MASK           	(1U << 30)  
  #define CLK_PDN_DIGMIC_CLK_INV_SEL_OFFSET         	30          
  #define CLK_PDN_DIGMIC_CLK_INV_SEL_INVERT       	0x00
  #define CLK_PDN_DIGMIC_CLK_INV_SEL_NOT_INVERT     	0x01

  #define CLK_PDN_AFE_27M_SEL_MASK           		(1U << 25)  
  #define CLK_PDN_AFE_27M_SEL_OFFSET         		25          
  #define CLK_PDN_AFE_27M_SEL_APLL_TST_26M_ECO      	0x00
  #define CLK_PDN_AFE_27M_SEL_27M       		0x01

  #define CLK_PDN_DUTY_METER_MASK           		(1U << 24)
  #define CLK_PDN_DUTY_METER_OFFSET         		24          
  #define CLK_PDN_DUTY_METER_DISABLE      		0x00
  #define CLK_PDN_DUTY_METER_ENABLE      		0x01

  #define CLK_PDN_DUTY_METER_SEL_MASK           	(15U << 20)  
  #define CLK_PDN_DUTY_METER_SEL_OFFSET         	20          
  #define CLK_PDN_DUTY_METER_SEL_AL0_IN       	    	0x00
  #define CLK_PDN_DUTY_METER_SEL_AL1_IN       		0x01
  #define CLK_PDN_DUTY_METER_SEL_AL2_IN       		0x02
  #define CLK_PDN_DUTY_METER_SEL_AL3_IN       		0x03
  #define CLK_PDN_DUTY_METER_SEL_AL4_IN       		0x04
  #define CLK_PDN_DUTY_METER_SEL_AR0_IN       		0x05
  #define CLK_PDN_DUTY_METER_SEL_AR1_IN       		0x06
  #define CLK_PDN_DUTY_METER_SEL_AR2_IN       		0x07
  #define CLK_PDN_DUTY_METER_SEL_AR3_IN       		0x08
  #define CLK_PDN_DUTY_METER_SEL_AR4_IN       		0x09
  #define CLK_PDN_DUTY_METER_SEL_AD_HSYNC_OUT    	0x0a
  #define CLK_PDN_DUTY_METER_SEL_AD_VSYNC_OUT       	0x0b
  #define CLK_PDN_DUTY_METER_SEL_AD_SOY_OUT         	0x0c
  #define CLK_PDN_DUTY_METER_SEL_AD_SOY_OUT_MON     	0x0d
  #define CLK_PDN_DUTY_METER_SEL_AD_FB_OUT          	0x0e
  #define CLK_PDN_DUTY_METER_SEL_0                  	0x0f

#define REG_RW_CLKGATE_CFG3             0x00a8       // audio peripher
  #define CLK_PDN_AUDIO_B00                (1U << 0)//15
  #define CLK_PDN_AUDIO_B01                (1U << 1)//16 
  #define CLK_PDN_AUDIO_B02	           (1U << 2)//17 
  #define CLK_PDN_AUDIO_B03	           (1U << 3)//18 
  #define CLK_PDN_AUDIO_B04	           (1U << 4)//19 
  #define CLK_PDN_AUDIO_B05	           (1U << 5)//20 
  #define CLK_PDN_AUDIO_B06                (1U << 6)//21 
  #define CLK_PDN_AUDIO_B07                (1U << 7)//22 
  #define CLK_PDN_AUDIO_B08                (1U << 8)//23 
  #define CLK_PDN_AUDIO_B09                (1U << 9)//24 
  #define CLK_PDN_AUDIO_B10                (1U << 10)//25 
  #define CLK_PDN_AUDIO_B11                (1U << 11)//25 
  #define CLK_PDN_AUDIO_B12                (1U << 12)//25 
  #define CLK_PDN_AUDIO_B13                (1U << 13)//25 
  #define CLK_PDN_AUDIO_B14                (1U << 14)//25 
  #define CLK_PDN_RFI_TOP                  (1U << 15)//26 
  #define CLK_PDN_MSDC_0                   (1U << 16)//27 
  #define CLK_PDN_MSDC_1                   (1U << 17)//28
  #define CLK_PDN_MSDC_2                   (1U << 18)//29
  #define CLK_PDN_SPI_MOTO1                (1U << 22)//31
  #define CLK_PDN_SPI_MOTO2                (1U << 23)//32
  #define CLK_PDN_PWM0                     (1U << 24)//33
  #define CLK_PDN_PWM1                     (1U << 25)//34
  #define CLK_PDN_PWM2                     (1U << 26)//35
  #define CLK_PDN_PWM3                     (1U << 27)//36
  #define CLK_PDN_SIFM0                    (1U << 28)//37
  #define CLK_PDN_SIFM1                    (1U << 29)//38
  #define CLK_PDN_SIFS0                    (1U << 30)//39  
  #define CLK_PDN_SIFS1                    (1U << 31)
  
#define REG_RW_CLKGATE_CFG4             0x00ac       // dvd clock gated
  #define CLK_PDN_MVDO                     (1U << 0)//41  CLKGATE_CFG4  DVD
  #define CLK_PDN_DGO                      (1U << 2)//43
  #define CLK_PDN_DACTST                   (1U << 3)
  #define CLK_PDN_DVD_OSD                  (1U << 4)
  #define CLK_PDN_GRA                      (1U << 5)//44
  #define CLK_PDN_BIM                      (1U << 6)//45
  #define CLK_PDN_TURBO32                  (1U << 7)//46
  #define CLK_PDN_VDEC                     (1U << 8)//47 
  #define CLK_PDN_PARSER                   (1U << 9)//48 
  #define CLK_PDN_RAMBUF                   (1U << 10)//49 
  #define CLK_PDN_PT110                    (1U << 11)//50
  #define CLK_PDN_RS232                    (1U << 12)//51
  #define CLK_PDN_CDDVD                    (1U << 13)//52
  #define CLK_PDN_AUDIO                    (1U << 14)//53
  #define CLK_PDN_SERVO_MISC               (1U << 15)//54
  #define CLK_PDN_RAMBUF_APCTRL_TBUS3      (1U << 16)//55
  #define CLK_PDN_RAMBUF_APCTRL_TBUS4      (1U << 17)//56
  #define CLK_PDN_RAMBUF_APCTRL_TBUS5      (1U << 18)//57
  #define CLK_PDN_MFG_TOP_PWR_WRAP         (1U << 31)//58
  
#define REG_RW_CLKGATE_CFG5             0x00b0       // lvds clock gated
  #define CLK_PDN_CLK_LVDS                 (1U << 0)
  #define CLK_PDN_CLK_TP_TOP0              (1U << 1)
  #define CLK_PDN_CLK_TP_TOP1              (1U << 2)
  #define CLK_PDN_CLK_TP_TOP2              (1U << 3)
  #define CLK_PDN_CLK_RFI_TOP1             (1U << 4)
  #define CLK_PDN_CLK_RFI_TOP2             (1U << 5)
  #define CLK_PDN_CLK_RFI_TOP3             (1U << 6)
  #define CLK_PDN_CLK_RFI_TOP4             (1U << 7)
  #define CLK_PDN_CLK_RFI_TOP5             (1U << 8)
  #define CLK_PDN_CLK_RFI_TOP6             (1U << 9)
  
#define REG_RW_CLKGATE_CFG6             0x00b4       	// vdout clock gated
  #define CLK_PDN_SCLER                    	(1U << 0)	//56 CLKGATE_CFG6 VDOUT
  #define CLK_PDN_TVD1                     	(1U << 1)	//57
  #define CLK_PDN_TVD2                     	(1U << 2)	//58
  #define CLK_PDN_OSD                      	(1U << 3)	//59
  #define CLK_PDN_OSD_R                    	(1U << 4)	//60
  #define CLK_PDN_FPD                      	(1U << 5)	//61
  #define CLK_PDN_FMT_VDO_F                	(1U << 6)	//62
  #define CLK_PDN_FMT_VDO_R                	(1U << 7)	//63
  #define CLK_PDN_WRITE_CHANEL             	(1U << 8)	//64
  #define CLK_PDN_FRAME_LOCK               	(1U << 9)	//65
  #define CLK_PDN_WRITE_CHANEL2			(1U << 10)	//64
  #define CLK_PDN_VGA_EDID			(1U << 11)	//65
  #define CLK_PDN_YPBPR_VGA			(1U << 13)	//65
  #define CLK_PDN_HDMI				(1U << 14)	//65
  #define CLK_PDN_TVE				(1U << 15)	//65
  #define CLK_PDN_DVD_MIX_2AP			(1U << 16)	//65
  #define CLK_PDN_OSD1				(1U << 17)	//65
  #define CLK_PDN_OSD2				(1U << 18)	//65
  #define CLK_PDN_OSD3				(1U << 19)	//65
  #define CLK_PDN_OSD4				(1U << 20)	//65
  #define CLK_PDN_OSD5				(1U << 21)	//65
  #define CLK_PDN_OSD_R2			(1U << 22)	//65
  #define CLK_PDN_OSD_R3			(1U << 23)	//65
  #define CLK_PDN_SCLER_TG			(1U << 24)	//65
  #define CLK_PDN_LCPROC_VDO			(1U << 25)	//65

#define REG_RW_SYNC_RESET_CFG0          0x00b8       // vdec full sync reset
  #define CLK_RESET_VDEC_FULL_MASK              (1U << 0)  //turn off VDEC FULL

#define REG_RW_SYNC_RESET_CFG1          0x00bc       // image dramc sync reset
  #define CLK_RESET_GFX	                   	(1U << 0)//
  #define CLK_RESET_DMARB                  	(1U << 1)//
  #define CLK_RESET_PNG	                   	(1U << 2) //
  #define CLK_RESET_GIF	                   	(1U << 3) //
  #define CLK_RESET_IMG_RESZ               	(1U << 4)//5  
  #define CLK_RESET_OSD_RESZ	           	(1U << 5)//6  
  #define CLK_RESET_JPGDEC                 	(1U << 6)//7  
  #define CLK_RESET_DEMUX	               	(1U << 7)//8
  #define CLK_RESET_NFI	           			(1U << 12)//8  
  #define CLK_RESET_USB	                   	(1U << 13)//8  
  #define CLK_RESET_IRT_DMA_WRAPPER	       	(1U << 14)//13 

#define REG_RW_SYNC_RESET_CFG2          0x00c0       // ring osc

#define REG_RW_SYNC_RESET_CFG3          0x00c4       // audio prepher sync reset
  #define CLK_RESET_AUDIO_B00               (1U << 0)//15
  #define CLK_RESET_AUDIO_B01               (1U << 1)//16 
  #define CLK_RESET_AUDIO_B02	            (1U << 2)//17 
  #define CLK_RESET_AUDIO_B03	            (1U << 3)//18 
  #define CLK_RESET_AUDIO_B04	            (1U << 4)//19 
  #define CLK_RESET_AUDIO_B05	            (1U << 5)//20 
  #define CLK_RESET_AUDIO_B06               (1U << 6)//21 
  #define CLK_RESET_AUDIO_B07               (1U << 7)//22 
  #define CLK_RESET_AUDIO_B08               (1U << 8)//23 
  #define CLK_RESET_AUDIO_B09               (1U << 9)//24 
  #define CLK_RESET_AUDIO_B10               (1U << 10)//25 
  #define CLK_RESET_AUDIO_B11               (1U << 11)//25 
  #define CLK_RESET_AUDIO_B12               (1U << 12)//25 
  #define CLK_RESET_AUDIO_B13               (1U << 13)//25 
  #define CLK_RESET_AUDIO_B14               (1U << 14)//25 
  #define CLK_RESET_RFI_TOP                 (1U << 15)//26 
  #define CLK_RESET_MSDC_0            	    (1U << 16)//27 
  #define CLK_RESET_MSDC_1            	    (1U << 17)//28
  #define CLK_RESET_MSDC_2            	    (1U << 18)//29
  #define CLK_RESET_MSDC_SW_0               (1U << 19)//27 
  #define CLK_RESET_MSDC_SW_1               (1U << 20)//28
  #define CLK_RESET_MSDC_SW_2               (1U << 21)//29
  #define CLK_RESET_SPI_MOTO1               (1U << 22)//31
  #define CLK_RESET_SPI_MOTO2               (1U << 23)//32
  #define CLK_RESET_PWM0                    (1U << 24)//33
  #define CLK_RESET_PWM1                    (1U << 25)//34
  #define CLK_RESET_PWM2                    (1U << 26)//35
  #define CLK_RESET_PWM3                    (1U << 27)//36
  #define CLK_RESET_SIFM0                   (1U << 28)//37
  #define CLK_RESET_SIFM1                   (1U << 29)//38
  #define CLK_RESET_SIFS0                   (1U << 30)//39  
  #define CLK_RESET_SIFS1                   (1U << 31)

#define REG_RW_SYNC_RESET_CFG4          0x00c8       // dvd sync reset
  #define CLK_RESET_MVDO                    (1U << 0)//41  CLKGATE_CFG4  DVD
  #define CLK_RESET_DGO                     (1U << 2)//43
  #define CLK_RESET_DVD_OSD                 (1U << 4)
  #define CLK_RESET_GRA                     (1U << 5)//44
  #define CLK_RESET_BIM                     (1U << 6)//45
  #define CLK_RESET_TURBO32                 (1U << 7)//46
  #define CLK_RESET_VDEC                    (1U << 8)//47 
  #define CLK_RESET_PARSER                  (1U << 9)//48 
  #define CLK_RESET_RAMBUF                  (1U << 10)//49 
  #define CLK_RESET_PT110                   (1U << 11)//50
  #define CLK_RESET_RS232                   (1U << 12)//51
  #define CLK_RESET_CDDVD                   (1U << 13)//52
  #define CLK_RESET_AUDIO                   (1U << 14)//53
  #define CLK_RESET_SERVO_MISC              (1U << 15)//54
  #define CLK_RESET_RAMBUF_APCTRL_TBUS3     (1U << 16)//50
  #define CLK_RESET_RAMBUF_APCTRL_TBUS4     (1U << 17)//50
  #define CLK_RESET_RAMBUF_APCTRL_TBUS5     (1U << 18)//50
  #define CLK_RESET_MFG_TOP_PWR_WRAP        (1U << 31)//54

#define REG_RW_SYNC_RESET_CFG5          0x00cc       // lvds sync reset
  #define CLK_RESET_CLK_LVDS                (1U << 0)
  #define CLK_RESET_CLK_TP                  (1U << 1)

#define REG_RW_SYNC_RESET_CFG6          0x00d0       // vdout tvd sync reset
  #define CLK_RESET_SCLER                   (1U << 0)//56
  #define CLK_RESET_TVD                     (1U << 1)//57
  #define CLK_RESET_OSD                     (1U << 3)//59
  #define CLK_RESET_OSD_R                   (1U << 4)//60
  #define CLK_RESET_FPD                     (1U << 5)//61
  #define CLK_RESET_FMT_VDO_F               (1U << 6)//62
  #define CLK_RESET_FMT_VDO_R               (1U << 7)//63
  #define CLK_RESET_WRITE_CHANEL            (1U << 8)//64
  #define CLK_RESET_FRAME_LOCK              (1U << 9)//65
  #define CLK_RESET_WRITE_CHANEL2           (1U << 10)//64
  #define CLK_RESET_VGA_EDID                (1U << 11)//65
  #define CLK_RESET_YPBPR_VGA               (1U << 13)//64
  #define CLK_RESET_HDMI                    (1U << 14)//65
  #define CLK_RESET_TVE                     (1U << 15)//64
  #define CLK_RESET_DVD_MIX_2AP             (1U << 16)//65
  #define CLK_RESET_SCLER_TG                (1U << 24)//65
  #define CLK_RESET_ICPROC_VDO              (1U << 25)//65


#define REG_RW_OSD_CLK_CFG         		0x00d4
	#define CLK_OSD_AUX_PCLK_SEL_MASK 		(1U << 24)  
	#define CLK_OSD_AUX_PCLK_SEL_OFFSET		24		  
	#define CLK_OSD_AUX_PCLK_SEL_SD_108M 		0x00
	#define CLK_OSD_AUX_PCLK_SEL_PANEL	  	0x01

	#define CLK_OSD_MAIN_PCLK_SEL_MASK		(1U << 23)  
	#define CLK_OSD_MAIN_PCLK_SEL_OFFSET 		23		  
	#define CLK_OSD_MAIN_PCLK_SEL_SD_108M		0x00
	#define CLK_OSD_MAIN_PCLK_SEL_PANEL	  	0x01

	#define CLK_OSD5_PCLK_SEL_MASK		  	(1U << 20)  
	#define CLK_OSD5_PCLK_SEL_OFFSET	  	20		  
	#define CLK_OSD5_PCLK_SEL_SD_108M	  	0x00
	#define CLK_OSD5_PCLK_SEL_PANEL   		0x01

	#define CLK_OSD4_PCLK_SEL_MASK		  	(1U << 19)  
	#define CLK_OSD4_PCLK_SEL_OFFSET	  	19		  
	#define CLK_OSD4_PCLK_SEL_SD_108M	  	0x00
	#define CLK_OSD4_PCLK_SEL_PANEL   		0x01

	#define CLK_OSD3_PCLK_SEL_MASK		  	(1U << 18)  
	#define CLK_OSD3_PCLK_SEL_OFFSET	  	18		  
	#define CLK_OSD3_PCLK_SEL_SD_108M	  	0x00
	#define CLK_OSD3_PCLK_SEL_PANEL   		0x01

	#define CLK_OSD2_PCLK_SEL_MASK		  	(1U << 17)  
	#define CLK_OSD2_PCLK_SEL_OFFSET	  	17		  
	#define CLK_OSD2_PCLK_SEL_SD_108M	  	0x00
	#define CLK_OSD2_PCLK_SEL_PANEL   		0x01

	#define CLK_OSD1_PCLK_SEL_MASK		  	(1U << 16)  
	#define CLK_OSD1_PCLK_SEL_OFFSET	  	16		  
	#define CLK_OSD1_PCLK_SEL_SD_108M	  	0x00
	#define CLK_OSD1_PCLK_SEL_PANEL   		0x01

	#define CLK_ONOFF_OSD_R_AUX             	(1U << 10)//65
	#define CLK_ONOFF_OSD_R_MAIN            	(1U << 9)//65
	#define CLK_ONOFF_OSD_AUX               	(1U << 8)//65
	#define CLK_ONOFF_OSD_MAIN              	(1U << 7)//65
	#define CLK_ONOFF_OSD3_R              		(1U << 6)//65
	#define CLK_ONOFF_OSD2_R              		(1U << 5)//65
	#define CLK_ONOFF_OSD5              		(1U << 4)//65
	#define CLK_ONOFF_OSD4              		(1U << 3)//65
	#define CLK_ONOFF_OSD3              		(1U << 2)//65
	#define CLK_ONOFF_OSD2              		(1U << 1)//65
	#define CLK_ONOFF_OSD1              		(1U << 0)//65

#define REG_RW_VDT_CLK_CFG         		0x00d8
#define REG_RW_TTL_CLK_CFG         		0x00dc
	#define CLK_TTL_FPD_HCK_INV         		(1U << 22)//65

#define REG_RW_GPIO_OUT_CFG0            0x00e0
#define REG_RW_GPIO_OUT_CFG1            0x00e4
#define REG_RW_GPIO_OUT_CFG2            0x00e8
#define REG_RW_GPIO_OUT_CFG3            0x00ec
#define REG_RW_GPIO_OUT_CFG4            0x00f0
#define REG_RW_GPIO_OUT_CFG5            0x00f4
#define REG_RW_GPIO_OUT_CFG6            0x00f8
#define REG_RW_GPIO_OUT_CFG7            0x00fc

#define REG_RW_GPIO_IN_CFG0             0x0100
#define REG_RW_GPIO_IN_CFG1             0x0104
#define REG_RW_GPIO_IN_CFG2             0x0108
#define REG_RW_GPIO_IN_CFG3             0x010c
#define REG_RW_GPIO_IN_CFG4             0x0110
#define REG_RW_GPIO_IN_CFG5             0x0114
#define REG_RW_GPIO_IN_CFG6             0x0118
#define REG_RW_GPIO_IN_CFG7             0x011c

#define REG_RW_MBIST_CFG1               0x0150
#define REG_RW_MBIST_CFG2               0x0154
#define REG_RW_MBIST_CFG3               0x0158
#define REG_RW_MBIST_CFG4               0x015C
#define REG_RW_MBIST_CFG5               0x0160
#define REG_RW_MBIST_CFG6               0x0164
#define REG_RW_MBIST_CFG7               0x0168
#define REG_RW_MBIST_CFG8               0x016C
#define REG_RW_MBIST_CFG9               0x0170
#define REG_RW_MBIST_CFG10              0x0174
#define REG_RW_MBIST_CFG11              0x0178
#define REG_RW_MBIST_CFG12              0x017C
#define REG_RW_MBIST_CFG13              0x0180
	#define MBIST_ABIST_MODE             (1U << 13)//65
	#define MBIST_TEST_SCAN_MODE         (1U << 12)//65
	#define MBIST_TEST_CPUM              (1U << 11)//65
	#define MBIST_ICE_MODE         	     (1U << 10)//65
	#define MBIST_ICE_MODE_2             (1U << 9)//65
	#define MBIST_FLASH_BOOT             (1U << 8)//65
	#define MBIST_XTAL_REF_SEL           (1U << 7)//65
	#define MBIST_OLT_TEST_MODE          (1U << 6)//65
	#define MBIST_NFI_STRAP              (1U << 5)//65
	#define MBIST_DVD_TEST_CPUM          (1U << 4)//65
	#define MBIST_DONE_ALL_1             (1U << 3)//65
	#define MBIST_FAIL_ALL_1             (1U << 2)//65
	#define MBIST_DONE_ALL_2             (1U << 1)//65
	#define MBIST_FAIL_ALL_2             (1U << 0)//65

#define REG_RW_TST_CLK_CFG              0x0188       // ap test clock select
#define REG_RW_TST_CLK_CFG1             0x018c       // 
#define REG_RW_TST_CLK_CFG2             0x0190

#define REG_RW_TEST_BUS_CFG0            0x0194       //dvd test bus control
#define REG_RW_TEST_BUS_CFG1            0x0198       // audio vdout test bus control
#define REG_RW_TEST_BUS_CFG2            0x019c       // dramc test bus control
#define REG_RW_TEST_BUS_CFG3            0x01a0       //

#define REG_RW_MISC_CONTROL2            0x0298       // top lever misc control2  // test
	#define MISC_FPD_6_BIT_SEL              (1U << 31)//65
	#define MISC_FPD_8_BIT_SEL              (1U << 30)//65
	#define MISC_FPD_DE_SEL                 (1U << 29)//65
	#define MISC_SYNC_SEL             	(1U << 28)//65
	#define MISC_LVDS_GPIO_MODE             (1U << 27)//65
	#define MISC_INT_GPIO_IN01_2_GP_SEL     (1U << 10)//65
	#define MISC_INT_GPIO_IN01_GP_SEL       (1U << 9)//65
	#define MISC_PORT_OPT_10             	(1U << 8)//65
	#define MISC_PORT_OPT_8             	(1U << 7)//65
	#define MISC_PORT_OPT_7             	(1U << 6)//65
	#define MISC_PORT_OPT_6             	(1U << 5)//65
	#define MISC_PORT_OPT_5             	(1U << 4)//65
	#define MISC_PORT_OPT_4             	(1U << 3)//65
	#define MISC_PORT_OPT_3             	(1U << 2)//65
	#define MISC_PORT_OPT_2             	(1U << 1)//65
	#define MISC_PORT_OPT_0             	(1U << 0)//65

#define REG_RW_PAD_MSDC_CFG0         0x02c0
#define REG_RW_PAD_MSDC_CFG1         0x02c4
#define REG_RW_PAD_MSDC_CFG2         0x02c8
#define REG_RW_PAD_MSDC_CFG3         0x02cc
#define REG_RW_PAD_MSDC_CFG4         0x02d0
#define REG_RW_PAD_MSDC_CFG5         0x02d4
#define REG_RW_PAD_MSDC_CFG6         0x02d8
#define REG_RW_PAD_MSDC_CFG7         0x02dc
#define REG_RW_PAD_MSDC_CFG8         0x02e0
#define REG_RW_PAD_MSDC_CFG9         0x02e4
#define REG_RW_PAD_MSDC_CFG10        0x02e8
#define REG_RW_PAD_MSDC_CFG11        0x02ec
#define REG_RW_PAD_MSDC_CFG12        0x02f0
#define REG_RW_PAD_MSDC_CFG13        0x02f4
#define REG_RW_PAD_MSDC_CFG14        0x02f8
#define REG_RW_PAD_MSDC_CFG15        0x02fc
#define REG_RW_PAD_MSDC_CFG16        0x0300
#define REG_RW_PAD_MSDC_CFG17        0x0304
#define REG_RW_PAD_MSDC_CFG18        0x0308
#define REG_RW_PAD_MSDC_CFG19        0x030c
#define REG_RW_PAD_MSDC_CFG20        0x0310
#define REG_RW_PAD_MSDC_CFG21        0x0314
#define REG_RW_PAD_MSDC_CFG22        0x0318
#define REG_RW_PAD_MSDC_CFG23        0x031c
#define REG_RW_PAD_MSDC_CFG24        0x0320
#define REG_RW_PAD_MSDC_CFG25        0x0324
#define REG_RW_PAD_MSDC_CFG26        0x0328
#define REG_RW_PAD_MSDC_CFG27        0x032c
#define REG_RW_PAD_MSDC_CFG28        0x0330
#define REG_RW_PAD_MSDC_CFG29        0x0334
#define REG_RW_PAD_MSDC_CFG30        0x0338
#define REG_RW_PAD_MSDC_CFG31        0x033c
#define REG_RW_PAD_MSDC_CFG32        0x0340
#define REG_RW_PAD_MSDC_CFG36        0x0350

#define REG_RW_HDMI_MHL_CFG0         0x0358
#define REG_RW_HDMI_MHL_CFG1         0x035c
	#define C_MHL_PDTCLK_SEL        (1U << 9)//65
	#define C_MHL_DPCLK_SEL         (1U << 8)//65
	#define C_MHL_PCLK_SEL          (1U << 7)//65
	#define RX_PLL_ABIST_CLK_SEL    (3U << 5)//65
	#define RX_PLL_ABIST_CLK_EN     (1U << 4)//65
	#define SRAM_TEST_MODE          (1U << 3)//65
	#define CDPCLK_SEL_XCLK         (1U << 2)//65
	#define C_CKDT_XCLK             (1U << 1)//65
	#define BIST_PCLK_SEL           (1U << 0)//65

#define REG_RW_YPBPR_VGA_CFG          0x0360
	#define XDDS_CLK_TST		(1U << 18)//65
	#define RESYNC_CLK_TST 		(1U << 17)//65
	#define HDTV_CLK_TST		(1U << 16)//65
	#define XDDS_CLK_INV		(3U << 10)//65
	#define RESYNC_CLK_INV 		(1U << 9)//65
	#define HDTV_CLK_INV		(1U << 8)//65
	#define XDDS_CLK_ON 		(1U << 2)//65
	#define RESYNC_CLK_ON 		(1U << 1)//65
	#define HDTV_CLK_ON		(1U << 0)//65

#define REG_RW_WCHNL_SOURCE_CFG       0x0364
#define REG_RW_HDMI_CBUS_CFG0         0x0368
#define REG_RW_BONT_OPTION            0x0380

/* PLLGP */
#define REG_RW_ANA7_PLLGP_CFG0      0x0580
#define REG_RW_ANA7_PLLGP_CFG1      0x0584
#define REG_RW_ANA7_PLLGP_CFG2      0x0588
#define REG_RW_ANA7_PLLGP_CFG3      0x058c
#define REG_RW_ANA7_PLLGP_CFG4      0x0590
#define REG_RW_ANA7_PLLGP_CFG5      0x0594
#define REG_RW_ANA7_PLLGP_CFG6      0x0598

#define REG_RW_ANA7_PLLGP_CFG7      0x059C
  #define  ARMPLL_PWD_MASK         	(1U << 0)
  #define  ARMPLL_PWD_DISABLE       	0x1

#define REG_RW_ANA7_PLLGP_CFG8      0x05A0
  #define  ARM9PLL_PWD_MASK        	(1U << 0)
  #define  ARM9PLL_PWD_DISABLE      	0x1

#define REG_RW_ANA7_PLLGP_CFG9      0x05A4
#define REG_RW_ANA7_PLLGP_CFG10     0x05A8
#define REG_RW_ANA7_PLLGP_CFG11     0x05AC
#define REG_RW_ANA7_PLLGP_CFG12     0x05B0
#define REG_RW_ANA7_PLLGP_CFG13     0x05B4

#define REG_RW_ANA7_PLLGP_CFG14     0x05B8
#define REG_RW_ANA7_PLLGP_CFG15     0x05BC
  #define BIT_RG_APLL_RESERVE_MASK 		(0x3F << 18)
  #define BIT_RG_APLL_PCW_NCPO_CHG 		(1 << 26)
  #define BIT_RG_APLL_DDS_RSTB     		(1 << 25)
  #define BIT_RG_APLL_DDS_PWDB     		(1 << 24)
  #define BIT_RG_APLL_DDSEN        		(1 << 17)
  #define BIT_RG_APLL_VODEN        		(1 << 16)
  #define BIT_RG_APLL_AUTOK_LOAD   		(1 << 13)
  #define BIT_RG_APLL_AUTOK_VCO    		(1 << 12)
  
#define REG_RW_ANA7_PLLGP_CFG16     0x05C0
#define REG_RW_ANA7_PLLGP_CFG17     0x05C4
  #define BIT_RG_APLL_DDS_NCPO_EN     	(1 << 19)
  #define BIT_RG_APLL_DDS_CLK_PH_INV  	(1 << 17)
  #define BIT_RG_APLL_FIFO_START_MAN  	(1 << 13)

#define REG_RW_ANA7_PLLGP_CFG18     0x05C8
#define REG_RW_ANA7_PLLGP_CFG19     0x05CC
  #define BIT_RG_PLL_RESERVE_MASK   (0xFF)

#define REG_RW_ANA7_PLLGP_CFG20     0x05D0
#define REG_RW_ANA7_PLLGP_CFG21     0x05D4 //
#define REG_RW_ANA7_PLLGP_CFG22     0x05D8
#define REG_RW_ANA7_PLLGP_CFG23     0x05DC
#define REG_RW_ANA7_PLLGP_CFG24     0x05E0 //

#define REG_RW_ANA7_PLLGP_CFG25     0x05E4
#define REG_RW_ANA7_PLLGP_CFG26     0x05E8
  #define BIT_RG_APLL_PWD           (1 << 0)

#define REG_RW_ANA7_PLLGP_CFG27     0x05EC //
#define REG_RW_ANA7_PLLGP_CFG28     0x05F0 //

#define REG_RW_ANA7_PLLGP_CFG29     0x05F4 //
#define REG_RW_ANA7_PLLGP_CFG30     0x05F8 //

#define REG_RO_MONITOR_PLLGP_STATUS 	0x05FC
  #define BIT_AD_RGS_APLL270_VCOCAL_FAIL   (1 << 16)
  #define BIT_AD_RGS_APLL270_VCOCAL_CPLT   (1 << 17)
  #define BIT_AD_RGS_APLL270_VCOCAL_STATE  (0x3F << 18)
  #define BIT_AD_RGS_APLL294_VCOCAL_FAIL   (1 << 24)
  #define BIT_AD_RGS_APLL294_VCOCAL_CPLT   (1 << 25)
  #define BIT_AD_RGS_APLL294_VCOCAL_STATE  (0x3F << 26)

#define REG_RW_MEMPLL0      (0x5AA90)

//============================================================================
// Constant definitions
//============================================================================
#define CKGEN_PLLGP_FREF  27000000
#define CKGEN_PLLGP_SYSPLL_FREQ  648000000

#define PLL_GET_FBSEL(v)  ((v >> 16)&0x3)
#define PLL_GET_CKCTRL(v)  ((v >> 18)&0x3)
#define PLL_GET_POSDIV(v)  ((v >> 20)&0x3)
#define PLL_GET_PREDIV(v) ((v >> 22)&0x3)
#define PLL_GET_FBDIV(v)  ((v >> 24)&0x7F)

#define PLL_FBSEL(v)    ((v & 0x3)<<16)
#define PLL_CKCTRL(v)   ((v & 0x3)<<18)
#define PLL_POSDIV(v)   ((v & 0x3)<<20)
#define PLL_PREDIV(v)   ((v & 0x3)<<22)
#define PLL_FBDIV(v)    ((v & 0x7F)<<24)
#define PLL_PWD         (1)

#define PLL_SETTING_MASK   (0xFFFF)
#define PLL_108MHZ    (PLL_FBSEL(0)| PLL_CKCTRL(0)|PLL_POSDIV(0)|PLL_PREDIV(1)|PLL_FBDIV(7)|PLL_PWD)
#define APLL_288MHz   (PLL_FBSEL(1)| PLL_CKCTRL(0)|PLL_POSDIV(2)|PLL_PREDIV(0)|PLL_FBDIV(7)|PLL_PWD)
#define APLL26_26MHz  (PLL_FBSEL(0)| PLL_CKCTRL(1)|PLL_POSDIV(0)|PLL_PREDIV(0)|PLL_FBDIV(0x19)|PLL_PWD)
#define APLL_CALIBRATION_TIMEOUT    0x10000

//============================================================================
// Type definitions
//============================================================================
typedef enum
{
    SRC_CK_APLL294,  // APLL2, 294M
    SRC_CK_APLL270,  // APLL1, 270M
    SRC_CK_ARM11PLL, // ARMPLL 850M
    SRC_CK_ARM9PLL,  // ARMPLL2, 450M
    SRC_CK_APLL26,   // APLL3, 26M
    SRC_CK_SYSPLL,   // SYSPLL, 648M
    SRC_CK_MEMPLL,   // MEMPLL, 200~400M
    SRC_CK_USBCK,    // USB, 480M
    SRC_CK_BUSCK,    // 
    SRC_CK_MSDCPLL,  // MSDCPLL, 405M
    SRC_CK_VGAPLL,
    SRC_CK_HADDS2    
} SRC_CK_T;

typedef enum
{
    e_CLK_VDEC_FULL,	             	//0  CLKGATE_CFG0    VDEC
	  
    e_CLK_GFX,	                   	//   CLKGATE_CFG1     IMG_DRAMC
    e_CLK_DMARB,	                //
    e_CLK_PNG,	           		//3  
    e_CLK_GIF,	           		//4  
    e_CLK_IMG_RESZ,                	//5  
    e_CLK_OSD_RESZ,	               	//6  
    e_CLK_JPGDEC,                  	//7  
    e_CLK_DEMUX,	                //8  
    e_CLK_DEMUX_TS0,	             	//9  
    e_CLK_DEMUX_TS1,	             	//10 
    e_CLK_DEMUX_27M,	             	//12 
    e_CLK_NFI,
    e_CLK_USB,
    e_CLK_IRT_DMA_WRAPPER,	        //13 
    e_CLK_ARM9,	                   	//14
    e_CLK_TS0_INV,
    e_CLK_TS1_INV,

    e_CLK_DUTY_METER,
	
    e_CLK_AUDIO_B00,               	//15  CLKGATE_CFG3  AUDIO PERIPHER
    e_CLK_AUDIO_B01,	             	//16 
    e_CLK_AUDIO_B02,	             	//17 
    e_CLK_AUDIO_B03,	             	//18 
    e_CLK_AUDIO_B04,	             	//19 
    e_CLK_AUDIO_B05,	             	//20 
    e_CLK_AUDIO_B06,               //21 
    e_CLK_AUDIO_B07,               //22 
    e_CLK_AUDIO_B08,               //23 
    e_CLK_AUDIO_B09,               //24 
    e_CLK_AUDIO_B10,               //25 
    e_CLK_AUDIO_B11,
    e_CLK_AUDIO_B12,
    e_CLK_AUDIO_B13,
    e_CLK_AUDIO_B14,
    e_CLK_RFI_TOP,                 //26 
    e_CLK_MSDC_0,                  //27 
    e_CLK_MSDC_1,                  //28
    e_CLK_MSDC_2,                  //29
    e_CLK_MSDC_SW_0,               //27 
    e_CLK_MSDC_SW_1,               //28
    e_CLK_MSDC_SW_2,               //29
    e_CLK_SPI_MOTO1,               //31
    e_CLK_SPI_MOTO2,               //32
    e_CLK_PWM0,                    //33
    e_CLK_PWM1,                    //34
    e_CLK_PWM2,                    //35
    e_CLK_PWM3,                    //36
    e_CLK_SIFM0,                    //37
    e_CLK_SIFM1,                    //38
    e_CLK_SIFS0,              	//39  
    e_CLK_SIFS1,               	//40
    
    e_CLK_MVDO,                    //41  CLKGATE_CFG4  DVD
    e_CLK_DGO,                     //43
    e_CLK_DACTST,
    e_CLK_DVD_OSD,
    e_CLK_GRA,                     //44
    e_CLK_BIM,                     //45
    e_CLK_TURBO32,                 //46
    e_CLK_VDEC,                    //47 
    e_CLK_PARSER,                  //48 
    e_CLK_RAMBUF,                  //49 
    e_CLK_PT110,                   //50
    e_CLK_RS232,                   //51
    e_CLK_CDDVD,                   //52
    e_CLK_AUDIO,                   //53
    e_CLK_SERVO_MISC,              //54
    e_CLK_RAMBUF_APCTRL_TBUS3,     //54
    e_CLK_RAMBUF_APCTRL_TBUS4,     //54
    e_CLK_RAMBUF_APCTRL_TBUS5,     //54
    e_CLK_MFG_TOP_PWR_WRAP,

    e_CLK_LVDS,                    //55 CLKGATE_CFG5 LVDS
    e_CLK_TP_TOP0,
    e_CLK_TP_TOP1,
    e_CLK_TP_TOP2,
    e_CLK_RFI_TOP1,
    e_CLK_RFI_TOP2,
    e_CLK_RFI_TOP3,
    e_CLK_RFI_TOP4,
    e_CLK_RFI_TOP5,
    e_CLK_RFI_TOP6,
    
    e_CLK_SCLER,                   //56 CLKGATE_CFG6 VDOUT
    e_CLK_TVD1,                    //57
    e_CLK_TVD2,                    //58
    e_CLK_OSD,                     //59
    e_CLK_OSD_R,                   //60
    e_CLK_FPD,                     //61
    e_CLK_FMT_VDO_F,               //62
    e_CLK_FMT_VDO_R,               //63
    e_CLK_WRITE_CHANEL,            //64
    e_CLK_FRAME_LOCK,              //65
    e_CLK_WRITE_CHANEL_2,
    e_CLK_VGA,
    e_CLK_YPBPR,
    e_CLK_HDMI,
    e_CLK_TVE,
    e_CLK_DVD_MIX_2AP,
    e_CLK_OSD_1,
    e_CLK_OSD_2,
    e_CLK_OSD_3,
    e_CLK_OSD_4,
    e_CLK_OSD_5,
    e_CLK_OSD_R_2,
    e_CLK_OSD_R_3,
    e_CLK_SCLER_TG,
    e_CLK_LCPROC_VDO,
    e_CLK_MAX
} e_CLK_T;

typedef enum
{
    e_MODULE_VDEC_FULL,	             	//0  CLKGATE_CFG0    VDEC
	  
    e_MODULE_GFX,	                   	//   CLKGATE_CFG1     IMG_DRAMC
    e_MODULE_DMARB,	                //
    e_MODULE_PNG,	           		//3  
    e_MODULE_GIF,	           		//4  
    e_MODULE_IMG_RESZ,                	//5  
    e_MODULE_OSD_RESZ,	               	//6  
    e_MODULE_JPGDEC,                  	//7  
    e_MODULE_DEMUX,	                //8  
    e_MODULE_NFI,
    e_MODULE_USB,
    e_MODULE_IRT_DMA_WRAPPER,	        //13 
	
    e_MODULE_AUDIO_B00,               	//15  CLKGATE_CFG3  AUDIO PERIPHER
    e_MODULE_AUDIO_B01,	             	//16 
    e_MODULE_AUDIO_B02,	             	//17 
    e_MODULE_AUDIO_B03,	             	//18 
    e_MODULE_AUDIO_B04,	             	//19 
    e_MODULE_AUDIO_B05,	             	//20 
    e_MODULE_AUDIO_B06,               //21 
    e_MODULE_AUDIO_B07,               //22 
    e_MODULE_AUDIO_B08,               //23 
    e_MODULE_AUDIO_B09,               //24 
    e_MODULE_AUDIO_B10,               //25 
    e_MODULE_AUDIO_B11,
    e_MODULE_AUDIO_B12,
    e_MODULE_AUDIO_B13,
    e_MODULE_AUDIO_B14,
    e_MODULE_RFI_TOP,                 //26 
    e_MODULE_MSDC_0,                  //27 
    e_MODULE_MSDC_1,                  //28
    e_MODULE_MSDC_2,                  //29
    e_MODULE_MSDC_SW_0,               //27 
    e_MODULE_MSDC_SW_1,               //28
    e_MODULE_MSDC_SW_2,               //29
    e_MODULE_SPI_MOTO1,               //31
    e_MODULE_SPI_MOTO2,               //32
    e_MODULE_PWM0,                    //33
    e_MODULE_PWM1,                    //34
    e_MODULE_PWM2,                    //35
    e_MODULE_PWM3,                    //36
    e_MODULE_SIFM0,                    //37
    e_MODULE_SIFM1,                    //38
    e_MODULE_SIFS0,              	//39  
    e_MODULE_SIFS1,               	//40
    
    e_MODULE_MVDO,                    //41  CLKGATE_CFG4  DVD
    e_MODULE_DGO,                     //43
    e_MODULE_DVD_OSD,
    e_MODULE_GRA,                     //44
    e_MODULE_BIM,                     //45
    e_MODULE_TURBO32,                 //46
    e_MODULE_VDEC,                    //47 
    e_MODULE_PARSER,                  //48 
    e_MODULE_RAMBUF,                  //49 
    e_MODULE_PT110,                   //50
    e_MODULE_RS232,                   //51
    e_MODULE_CDDVD,                   //52
    e_MODULE_AUDIO,                   //53
    e_MODULE_SERVO_MISC,              //54
    e_MODULE_RAMBUF_APCTRL_TBUS3,     //54
    e_MODULE_RAMBUF_APCTRL_TBUS4,     //54
    e_MODULE_RAMBUF_APCTRL_TBUS5,     //54
    e_MODULE_MFG_TOP_PWR_WRAP,

    e_MODULE_LVDS,                    //55 CLKGATE_CFG5 LVDS
    e_MODULE_TP_TOP,
   
    e_MODULE_SCLER,                   //56 CLKGATE_CFG6 VDOUT
    e_MODULE_TVD,                    //57
    e_MODULE_OSD,                     //59
    e_MODULE_OSD_R,                   //60
    e_MODULE_FPD,                     //61
    e_MODULE_FMT_VDO_F,               //62
    e_MODULE_FMT_VDO_R,               //63
    e_MODULE_WRITE_CHANEL,            //64
    e_MODULE_FRAME_LOCK,              //65
    e_MODULE_WRITE_CHANEL_2,
    e_MODULE_VGA,
    e_MODULE_YPBPR,
    e_MODULE_HDMI,
    e_MODULE_TVE,
    e_MODULE_DVD_MIX_2AP,
    e_MODULE_OSD_1,
    e_MODULE_OSD_2,
    e_MODULE_OSD_3,
    e_MODULE_OSD_4,
    e_MODULE_OSD_5,
    e_MODULE_OSD_R_2,
    e_MODULE_OSD_R_3,
    e_MODULE_SCLER_TG,
    e_MODULE_LCPROC_VDO,
    e_MODULE_MAX
} e_MODULE_T;

typedef enum
{
    e_CLK_SEL_RFI,         //conf0
    e_CLK_SEL_TP_F32K,
    e_CLK_SEL_TP,
    e_CLK_SEL_VDO,
    e_CLK_SEL_RISC,
    e_CLK_SEL_DEMUX,
    e_CLK_SEL_DSP,

    e_CLK_SEL_USB_27M,         //conf1
    e_CLK_SEL_OSD,
    e_CLK_SEL_DRAM,
    e_CLK_SEL_AXIM,
    e_CLK_SEL_SPM,
    e_CLK_SEL_VDEC_SYS,
    e_CLK_SEL_JPEG,
    e_CLK_SEL_RSZ,
    e_CLK_SEL_FLASH,
    e_CLK_SEL_BCLK,
    
    e_CLK_SEL_AUD,        //conf2
    e_CLK_SEL_G3D,
    e_CLK_SEL_FPD,
    e_CLK_SEL_SD11,
    e_CLK_SEL_SD01,
    e_CLK_SEL_SD20,
    e_CLK_SEL_SD10,
    e_CLK_SEL_SD00,
    e_CLK_SEL_GRAPH,
    
    e_CLK_SEL_DUTY,   //conf3
    e_CLK_SEL_DEG,
    e_CLK_SEL_NF,
    e_CLK_SEL_BT_MIC_AUD,
    e_CLK_SEL_ARM_AUD,
    e_CLK_SEL_MPHON,
    e_CLK_SEL_CPU2,
    e_CLK_SEL_CPU1,
    e_CLK_SEL_AUD2,
    
    e_CLK_SEL_APLL_K8, //conf7
    e_CLK_SEL_APLL_K7,
    e_CLK_SEL_APLL_K6,
    e_CLK_SEL_APLL_K5,
    e_CLK_SEL_APLL_K4,
    e_CLK_SEL_APLL_K3,
    e_CLK_SEL_APLL_K2,
    e_CLK_SEL_APLL_K1,
      
    e_CLK_SEL_MLIN,   //CONF8
    e_CLK_SEL_AUD_MPH,
    e_CLK_SEL_MLIN2,
    e_CLK_SEL_AUD_PWM,
    e_CLK_SEL_AUD_ADC,
    e_CLK_SEL_PLL_TEST,
    e_CLK_SEL_APLL_A3,
    e_CLK_SEL_APLL_A2,
    e_CLK_SEL_APLL_A1,
    e_CLK_SEL_APLL_K14,
    e_CLK_SEL_APLL_K13,
    e_CLK_SEL_APLL_K12,
    e_CLK_SEL_APLL_K11,
    e_CLK_SEL_APLL_K10,
    e_CLK_SEL_APLL_K9,
    
    e_CLK_SEL_AUD_K5_TST,  //CONF9
    e_CLK_SEL_AUD_A3_TST,
    e_CLK_SEL_AUD_A2_TST,
    e_CLK_SEL_AUD_A1_TST,
    e_CLK_SEL_MLIN_TST,
    e_CLK_SEL_DA_APLL1CK,
    e_CLK_SEL_DA_APLLCK,
    e_CLK_SEL_MCLK_D2,
    e_CLK_SEL_SRAMIF,
    e_CLK_SEL_TVD_MBIST,
    e_CLK_SEL_SPI_MOTO,
    e_CLK_SEL_PNG,
    e_CLK_SEL_TS1,
    e_CLK_SEL_TS0,
    
    e_CLK_SEL_SIFS1,        //CONF10
    e_CLK_SEL_SIFS0,
    e_CLK_SEL_SIFM1,
    e_CLK_SEL_SIFM0,
    e_CLK_SEL_PWM3,
    e_CLK_SEL_PWM2,
    e_CLK_SEL_PWM1,
    e_CLK_SEL_PWM0,
    e_CLK_SEL_APLL_26M,
    e_CLK_SEL_TWDS,
    e_CLK_SEL_LVDS,
    e_CLK_SEL_MAX
    
} e_CLK_SEL_T;


//============================================================================
// Interface
//============================================================================
#if 0
extern BOOL BSP_Calibrate(SRC_CK_T eSource, UINT32 u4Clock);
extern UINT32 BSP_GetClock(SRC_CK_T eSource);
extern BOOL CKGEN_SetPLL(SRC_CK_T eSource, UINT32 u4Clock0, UINT32 u4Clock1);
extern BOOL CKGEN_AgtOnClk(e_CLK_T eAgt);
extern BOOL CKGEN_AgtOffClk(e_CLK_T eAgt);
extern BOOL CKGEN_AgtSelClk(e_CLK_SEL_T eAgt, UINT32 u4Sel);
extern UINT32 CKGEN_AgtGetClk(e_CLK_SEL_T eAgt);
extern BOOL CKGEN_AgtOnClk_NoReset(e_CLK_T eAgt);
extern BOOL CKGEN_AgtOffClk_NoReset(e_CLK_T eAgt);
extern BOOL Module_Reset_On(e_MODULE_T eAgt);
extern BOOL Module_Reset_Off(e_MODULE_T eAgt);
#endif
#endif  // X_CKGEN_8317_H
