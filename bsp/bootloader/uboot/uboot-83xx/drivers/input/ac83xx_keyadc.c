/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2014
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
*  RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#include <ac83xx_keyadc.h>
#include <common.h>

#define IO_BASE_VA                        0xF0000000L
// Defines
#define AUXADC_CON0                     0x000
#define AUXADC_CON1                     0x004
#define AUXADC_CON2                     0x008
#define AUXADC_CON3                     0x00C
#define AUXADC_DAT0                     0x010
#define AUXADC_DAT1                     0x014
#define AUXADC_DAT2                     0x018
#define AUXADC_DAT3                     0x01C
#define AUXADC_DAT4                     0x020
#define AUXADC_DAT5                     0x024
#define AUXADC_DAT6                     0x028
#define AUXADC_DAT7                     0x02C
#define AUXADC_DAT8                     0x030
#define AUXADC_DAT9                     0x034
#define AUXADC_DAT10                    0x038
#define AUXADC_DAT11                    0x03C
#define AUXADC_DAT12                    0x040
#define AUXADC_DAT13                    0x044

#define AUXADC_TS_DEBT0                 0x050
#define AUXADC_TS_DEBT1                 0x054
#define AUXADC_TS_CMD                   0x058
#define AUXADC_TS_ADDR                  0x05C
#define AUXADC_TS_CON0                  0x060
#define AUXADC_TS_CON1                  0x064
#define AUXADC_TS_CON2                  0x068
#define AUXADC_TS_CON3                  0x06C
#define AUXADC_TS_DAT0                  0x070
#define AUXADC_TS_DAT1                  0x074
#define AUXADC_TS_DAT2                  0x078
#define AUXADC_TS_DAT3                  0x07C

#define AUXADC_TS_TEST                  0x080
#define AUXADC_DET_VOLT                 0x088
#define AUXADC_DET_SEL                  0X08C
#define AUXADC_DET_PERIOD               0X090
#define AUXADC_DET_DEBT                 0X094
#define AUXADC_MISC                     0X098
#define AUXADC_ECC                      0X09C
#define AUXADC_SAMPLE_LIST              0X0A0
#define AUXADC_ABIST_PERIOD             0X0A4

#define AUXADC_TS_AUTO_CON              0X0A8
#define AUXADC_TS_AUTO_TIME_INTVL       0X0AC

#define AUXADC_TS_AUTO_X_DATn           0X200
#define AUXADC_TS_AUTO_Y_DATn           0X240
#define AUXADC_TS_AUTO_Z1_DATn          0X280
#define AUXADC_TS_AUTO_Z2_DATn          0X2C0

#define AUXADC_THERM_CON                0X180
#define AUXADC_THERM_DATA               0X184
#define AUXADC_F13M_EN                  0X188
#define AUXADC_TMMA0_CNT                0X18C
#define AUXADC_TDMA1_CNT                0x190
#define AUXADC_PDN_CON                  0x194
#define AUXADC_KEY_PWMFB_EN             0x198
#define AUXADC_KEY_MASK                 0X19C
#define AUXADC_PWM_CMPH                 0X1A0
#define AUXADC_PWM_CMPL                 0X1A4
#define AUXADC_KEY_PWM_IRQ_STA          0X1A8
#define AUXADC_MONI_SEL0                0X1B0
#define AUXADC_MONI_SEL1                0X1B4
#define AUXADC_MONI_SEL2                0X1B8
#define AUXADC_TP_RESERVE               0X1BC
#define AUX_TS_CMD_ADDRESS_Y            0x01
#define AUX_TS_CMD_ADDRESS_Z1           0x03
#define AUX_TS_CMD_ADDRESS_Z2           0x04
#define AUX_TS_CMD_ADDRESS_X            0x05

#define AUX_TS_CMD_12BIT_RES         0x0
#define AUX_TS_CMD_10BIT_RES         0x1

#define AUX_TS_CMD_MODE_DF           0x0 //Differential mode
#define AUX_TS_CMD_MODE_SE           0x1 //Single-end mode

#define AUX_TS_CMD_PD_YDRV_SH        0x00
#define AUX_TS_CMD_PD_IRQ_SH         0x01
#define AUX_TS_CMD_PD_IRQ            0x03
#define AUX_TS_CON_SPL_TRIGGER       0x01
#define AUX_TS_CON_PRESS_STATUS_MASK 0x02

#define AUX_BASE_ADDR            0xA9000

//------------------------------------------------------------------------------
#define HAL_WRITE32(_reg_, _val_)           (*((volatile uint32_t*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)                   (*((volatile uint32_t*)(_reg_)))
#define IO_READ32(base, offset)                 HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value)         HAL_WRITE32((base) + (offset), (value))

#define ADC_WRITE32(offset, value)      IO_WRITE32(IO_BASE_VA+AUX_BASE_ADDR, (offset), (value))
#define ADC_READ32(offset)              IO_READ32(IO_BASE_VA+AUX_BASE_ADDR, (offset))
#define WR(addr, v)                     ADC_WRITE32(addr, v)
#define RR(addr)                        ADC_READ32(addr)


#define ADCKEY_VOLTAGE_RANGE_INVALID      (0x20)
#define ADCKEY_VOLTAGE_RANGE_A            (0xe3)  //227
#define ADCKEY_VOLTAGE_RANGE_B            (0x1ba) //442
#define ADCKEY_VOLTAGE_RANGE_C            (0x26c) //620
#define ADCKEY_VOLTAGE_RANGE_D            (0x2D0) //720
#define ADCKEY_VOLTAGE_RANGE_E            (0x408) //1032
#define ADCKEY_VOLTAGE_RANGE_F            (0x4c4) //1220
#define ADCKEY_VOLTAGE_RANGE_G            (0x5a5) //1445
#define ADCKEY_VOLTAGE_RANGE_H            (0x681) //1665
#define ADCKEY_VOLTAGE_RANGE_I            (0x758) //1880
#define ADCKEY_VOLTAGE_RANGE_J            (0x834) //2100
#define ADCKEY_VOLTAGE_RANGE_K            (0x90b) //2315
#define ADCKEY_VOLTAGE_RANGE_L            (0x9c4) //2500
#define ADCKEY_VOLTAGE_RANGE_M            (0xaf0) //2800
#define ADCKEY_VOLTAGE_RANGE_N            (0xb9f) //2975
#define ADCKEY_VOLTAGE_RANGE_O            (0xc4e) //3150
#define ADCKEY_VOLTAGE_RANGE_P            (0xd80) //3456


#define ADCKEY_KEYCODE_A                   (30)
#define ADCKEY_KEYCODE_B                   (48)
#define ADCKEY_KEYCODE_C                   (46)
#define ADCKEY_KEYCODE_D                   (32)
#define ADCKEY_KEYCODE_E                   (18)
#define ADCKEY_KEYCODE_F                   (33)
#define ADCKEY_KEYCODE_G                   (34)
#define ADCKEY_KEYCODE_H                   (35)
#define ADCKEY_KEYCODE_I                   (23)
#define ADCKEY_KEYCODE_J                   (37)
#define ADCKEY_KEYCODE_K                   (37)
#define ADCKEY_KEYCODE_L                   (38)
#define ADCKEY_KEYCODE_M                   (50)
#define ADCKEY_KEYCODE_N                   (49)
#define ADCKEY_KEYCODE_O                   (24)
#define ADCKEY_KEYCODE_P                   (25)

#define NO_KEY_PRESSED                       (0xfff)
#define bool                              BOOL

extern void udelay (unsigned long usec);
unsigned int get_key_dat2(void)
{
    unsigned int dat;

    udelay(10000);
    dat = ADC_READ32(AUXADC_DAT2); 

    while(!(dat & 0x1000)) {
        udelay(2000);
        dat = ADC_READ32(AUXADC_DAT2); 
    }

    if (dat & 0x1000) {
        dat = ADC_READ32(AUXADC_DAT2); 
        dat = dat & 0xfff;
    } else {
       dat = 0;
    }
    return dat;
}

void keyadc_init(void)
{
    unsigned int tmp = 0;

    {
        tmp = IO_READ32(IO_BASE_VA, 0XCC);
        tmp |= 0X00000002;
        IO_WRITE32(IO_BASE_VA, 0XCC, tmp);
        tmp = IO_READ32(IO_BASE_VA, 0XCC);

        tmp = IO_READ32(IO_BASE_VA, 0XB0);
        tmp |= 0X0000000E;
        IO_WRITE32(IO_BASE_VA, 0XB0, tmp);
        tmp = IO_READ32(IO_BASE_VA, 0XB0);


        tmp = IO_READ32(IO_BASE_VA, 0X6A0);
        tmp = tmp & (0xfffdffff);
        IO_WRITE32(IO_BASE_VA, 0X6A0, tmp);

        tmp = ADC_READ32(AUXADC_PDN_CON);

        ADC_WRITE32(AUXADC_PDN_CON, 0);
        tmp = ADC_READ32(AUXADC_PDN_CON);
    }
  
    ADC_WRITE32(AUXADC_KEY_PWMFB_EN, 0x3FF);
    ADC_WRITE32(AUXADC_CON0, 0x3F);

    ADC_WRITE32(AUXADC_TDMA1_CNT,0x420); // 1ms
    tmp = ADC_READ32(AUXADC_MISC);
    tmp |= 0x0200;
    ADC_WRITE32(AUXADC_MISC, tmp);

    ADC_WRITE32(AUXADC_ECC, 0x1118);

}

void keyadc_exit(void)
{
    unsigned int tmp = 0;
    {
        tmp = IO_READ32(IO_BASE_VA, 0XCC);
        tmp = tmp & 0XFFFFFFFD;
        IO_WRITE32(IO_BASE_VA, 0XCC, tmp);
        tmp = IO_READ32(IO_BASE_VA, 0XCC);

        tmp = IO_READ32(IO_BASE_VA, 0XB0);
        tmp = tmp & 0XFFFFFFF1;
        IO_WRITE32(IO_BASE_VA, 0XB0, tmp);
        tmp = IO_READ32(IO_BASE_VA, 0XB0);

        tmp = ADC_READ32(AUXADC_PDN_CON);
        ADC_WRITE32(AUXADC_PDN_CON, 3);
        tmp = ADC_READ32(AUXADC_PDN_CON);
    }
  
    ADC_WRITE32(AUXADC_KEY_PWMFB_EN, 0x0);
    tmp = ADC_READ32(AUXADC_CON0);
    tmp = tmp & 0xFC0;
    ADC_WRITE32(AUXADC_CON0, tmp);
    ADC_WRITE32(AUXADC_TDMA1_CNT,0x1E0);//15ms
}

unsigned int get_key_value(unsigned int sample_value)
{
    unsigned int key_value;
    if (sample_value < ADCKEY_VOLTAGE_RANGE_INVALID) {//instable voltage
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_A) {
        key_value = ADCKEY_KEYCODE_A;
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_B) {
        key_value = ADCKEY_KEYCODE_B;
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_C) {
        key_value = ADCKEY_KEYCODE_C;
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_D) {
        key_value = ADCKEY_KEYCODE_D;
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_E) {
        key_value = ADCKEY_KEYCODE_E;
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_F) {
        key_value = ADCKEY_KEYCODE_F;
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_G) {
        key_value = ADCKEY_KEYCODE_G;
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_H) {
        key_value = ADCKEY_KEYCODE_H; 
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_I) {
        key_value = ADCKEY_KEYCODE_I; 
        key_value = NO_KEY_PRESSED;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_J) {
        key_value = ADCKEY_KEYCODE_J;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_K) {
        key_value = ADCKEY_KEYCODE_K;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_L) {
        key_value = ADCKEY_KEYCODE_L;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_M) {
        key_value = ADCKEY_KEYCODE_M;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_N) {
        key_value = ADCKEY_KEYCODE_N;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_O) {
        key_value = ADCKEY_KEYCODE_O;
    } else if(sample_value < ADCKEY_VOLTAGE_RANGE_P) {
        key_value = ADCKEY_KEYCODE_P;
    } else {
        key_value = NO_KEY_PRESSED;
    }
    return key_value;
}

/**
 * @Description:  
 *  This function get called only in case of checking entrance of recovery mode 
 *  The checking way is to read data channel to see if there is any key
 *  pressed for a while. 
 *
 * @Return
 *  true if any key is pressed
 *  false if not
 *
 */
bool check_key_pressing(void)
{
    unsigned int data2 = 0;
    int ret  = 0;
    unsigned int adckey_value = NO_KEY_PRESSED;
    unsigned int adckey_value_old = NO_KEY_PRESSED;
    unsigned int adckey_count = 0;
    unsigned int confirmed_count = 0;
#define  RETRY_CNT              10 

    keyadc_init();

    while (1) {

        data2 = get_key_dat2();  
        adckey_value = get_key_value(data2);          

        if(NO_KEY_PRESSED == adckey_value) {
             confirmed_count++;
             if (confirmed_count > RETRY_CNT) {
                 printf("[Recovery Mode] No key is pressed!\n");
                 ret  = 0;          
                 break;
             }
        }

        if ((adckey_value == adckey_value_old) &&  (adckey_value != NO_KEY_PRESSED)) {
            adckey_count++;
            if (adckey_count > RETRY_CNT) {
                /* Assume that key is pressed */
                 printf("[Recovery Mode] Key is pressed to enter into recovery mode!\n");
                ret  = 1;          
                break;
           }
        } else {
            adckey_value_old = adckey_value;
            adckey_count = 1;
        }
    }

    return ret;
}

