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
#ifndef _AC83XX_I2C_H_
#define _AC83XX_I2C_H_

#include <chip_ver.h>
#include <ac83xx_gpio.h>
#include <asm/arch/x_bim.h>


#define PIN_I2C_SDA    PIN_SDA
#define PIN_I2C_SCL    PIN_SCL


#if 1

#define BIM_T64b_LO_1   0x728
#define BIM_T64b_HI_1   0x72C 
#define BIM_T64b_EN_1   0x730

static inline void i2c_timer_reset(void)
{
    BIM_WRITE32(BIM_T64b_EN_1, 0);
    BIM_WRITE32(BIM_T64b_LO_1, 0);
    BIM_WRITE32(BIM_T64b_HI_1, 0);
    BIM_WRITE32(BIM_T64b_EN_1, 1);
}    

static inline unsigned long long get_64b_timer(void)
{
	unsigned int u4H_pre, u4H_next, u4L;
    unsigned long long u8Cycles;
		
	/* get timer data */
	do
	{
	    u4H_pre  = BIM_READ32(BIM_T64b_HI_1);
		u4L      = BIM_READ32(BIM_T64b_LO_1);
	    u4H_next = BIM_READ32(BIM_T64b_HI_1);
	}while(u4H_pre != u4H_next);

    u8Cycles = u4H_next;
	u8Cycles = u8Cycles << 31;
	u8Cycles = u8Cycles + (u4L & 0x7FFFFFFF);

	return u8Cycles;
}

static inline void i2c_udelay(unsigned long usec)
{
    unsigned long long u8Timer, u8Cycles;

    u8Cycles = usec * (unsigned long long)CFG_HZ_CLOCK;
	u8Cycles = u8Cycles / 1000000L;

	u8Timer = get_64b_timer();
	if((u8Timer + u8Cycles + 1) < u8Timer)
	{
		i2c_timer_reset();
		u8Timer = get_64b_timer();
	}

	u8Timer += u8Cycles;
	
	while(get_64b_timer() < u8Timer);
}
#else
#define i2c_udelay(a) udelay(1000)
#endif

/*
 * Software (bit-bang) I2C driver configuration
 */
#define OPEN_DRAIN  0 

#if !OPEN_DRAIN
#define I2C_INIT	   do{ \
                        GPIO_Config(PIN_I2C_SCL, OUTPUT, 0); \
                        GPIO_Config(PIN_I2C_SDA, OUTPUT, 0); \
                        i2c_timer_reset(); \
                       }while(0)

#define I2C_ACTIVE	    GPIO_InOut_Sel(PIN_I2C_SDA, OUTPUT)
#define I2C_TRISTATE	GPIO_InOut_Sel(PIN_I2C_SDA, INPUT)
#define I2C_READ	    (GPIO_Input(PIN_I2C_SDA) != 0)
#define I2C_SDA(bit)	do {  \ 
                           if(bit) \  
						   	 GPIO_Output(PIN_I2C_SDA, HIGH); \
			               else   \
						   	 GPIO_Output(PIN_I2C_SDA, LOW); \
                        }while(0)
#define I2C_SCL(bit)	do {  \
                           if(bit) \  
						   	 GPIO_Output(PIN_I2C_SCL, HIGH); \
			               else   \
						   	 GPIO_Output(PIN_I2C_SCL, LOW); \
                        }while(0)
#define I2C_DELAY	i2c_udelay(30)	/* 1/4 I2C clock duration */
#else
#define I2C_INIT	   do{ \
                        GPIO_Config(PIN_I2C_SCL, INPUT, 0); \
                        GPIO_Config(PIN_I2C_SDA, INPUT, 0); \
                        i2c_timer_reset(); \
                       }while(0)
#define I2C_ACTIVE	    
#define I2C_TRISTATE	
#define I2C_READ	    (GPIO_Input(PIN_I2C_SDA) != 0)
#define I2C_SDA(bit)	do {  \
                           if(bit) \  
						   	 GPIO_InOut_Sel(PIN_I2C_SDA, INPUT); \
			               else   \
						   	 GPIO_InOut_Sel(PIN_I2C_SDA, OUTPUT); \
                        }while(0)
#define I2C_SCL(bit)	do {  \
                           if(bit) \  
						   	 GPIO_InOut_Sel(PIN_I2C_SCL, INPUT); \
			               else   \
						   	 GPIO_InOut_Sel(PIN_I2C_SCL, OUTPUT); \
                        }while(0)
#define I2C_DELAY	i2c_udelay(30)	/* 1/4 I2C clock duration */

#endif

#endif //_AC83XX_I2C_H_

