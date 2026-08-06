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

#ifndef _EXT_MCU_C_
#define _EXT_MCU_C_

#include <asm/arch/x_pdwnc.h>
#include <asm/arch/x_bim.h>
#include <ext_mcu.h>
#include <i2c.h>

#if CONFIG_EXTERNAL_MCU

#define EXT_MCU_ADDR          0x50 /*0xA0>>1*/
#define EXT_MCU_DATA_MAX      16  
#define EXT_MCU_CFG_DATA      0x16 /*Refer to protocol*/
#define EXT_MCU_WAKE_REASON_DATA_LEN 3

typedef enum _E_MCU_CMD
{
    MCU_CMD_SET_CFG             = 0x01, /*Init CEC MCU*/
    MCU_CMD_SET_POWER_STATE     = 0x02, /*Inform CEC MCU the power state of Host*/
    MCU_CMD_SET_TRADE_MODE      = 0x03, /*Trade mode*/
    MCU_CMD_GET_WAKE_REASON     = 0x06, /*Request wake reason*/
    MCU_CMD_GET_MCU_VERSION     = 0x07, /*Request MCU version*/
    MCU_CMD_SET_BLUE_LED_STATE  = 0x08, /*set blue led state*/
    MCU_CMD_INVALID             = 0xFF  /* Invalid */
}E_MCU_CMD;

typedef enum _E_TRADE_CMD
{
    TRADE_MODE_ON            = 0x01, 
    TRADE_MODE_OFF           = 0x02,
    TRADE_MODE_INVALID       = 0xFF
}E_TRADE_CMD;

typedef enum _E_BLUE_LED_CMD
{
    BLUE_LED_FADE_IN    = 0x01, 
    BLUE_LED_NORMAL_ON  = 0x02, 
    BLUE_LED_FADE_OUT   = 0x03, 
    BLUE_LED_INVALID    = 0xFF, 
}E_BLUE_LED_CMD;

/*MCU_CMD_SET_POWER_STATE command parameter*/
typedef enum _E_PWR_STAT
{
    SYS_PWR_ON            = 0x01, /*8520 ACTIVE state*/
    SYS_PWR_STANDBY       = 0x02, /*8520 STANDBY*/
    SYS_PWR_STBY_2_ON     = 0x03, /*in transition from standby to active*/
    SYS_PWR_ON_2_STBY     = 0x04 /*in transition from active to standby*/
}E_PWR_STAT;

/*MCU_CMD_GET_WAKE_REASON return vaule meanning*/
typedef enum _E_WAKE_REASON
{
    WAKE_SSP            = 0x01, /*wake by set stream path*/
    WAKE_KEY            = 0x02, /*wake by RC pass-through*/
    WAKE_VFD_IR_KEY     = 0x03, /*wake by press VFD IR key*/
    WAKE_INVALLID       = 0xFF
}E_WAKE_REASON;

typedef enum _E_WAKE_KEY
{
    KEY_POWER            = 0x01, /*Power key*/
    KEY_EJECT            = 0x02, /*eject key*/
    KEY_PLAY             = 0x03, /*play key*/
    KEY_DISC             = 0x04, /*disc key*/
    KEY_HOME             = 0x05, /*home key*/
    KEY_INVALLID         = 0xFF
}E_WAKE_KEY;

const uchar crc_table[256] = 
{ 
  0x00, 0x0b, 0x16, 0x1d, 0x2c, 0x27, 0x3a, 0x31, 
  0x58, 0x53, 0x4e, 0x45, 0x74, 0x7f, 0x62, 0x69, 
  0xb0, 0xbb, 0xa6, 0xad, 0x9c, 0x97, 0x8a, 0x81, 
  0xe8, 0xe3, 0xfe, 0xf5, 0xc4, 0xcf, 0xd2, 0xd9, 
  0x6b, 0x60, 0x7d, 0x76, 0x47, 0x4c, 0x51, 0x5a, 
  0x33, 0x38, 0x25, 0x2e, 0x1f, 0x14, 0x09, 0x02, 
  0xdb, 0xd0, 0xcd, 0xc6, 0xf7, 0xfc, 0xe1, 0xea, 
  0x83, 0x88, 0x95, 0x9e, 0xaf, 0xa4, 0xb9, 0xb2, 
  0xd6, 0xdd, 0xc0, 0xcb, 0xfa, 0xf1, 0xec, 0xe7, 
  0x8e, 0x85, 0x98, 0x93, 0xa2, 0xa9, 0xb4, 0xbf, 
  0x66, 0x6d, 0x70, 0x7b, 0x4a, 0x41, 0x5c, 0x57, 
  0x3e, 0x35, 0x28, 0x23, 0x12, 0x19, 0x04, 0x0f, 
  0xbd, 0xb6, 0xab, 0xa0, 0x91, 0x9a, 0x87, 0x8c, 
  0xe5, 0xee, 0xf3, 0xf8, 0xc9, 0xc2, 0xdf, 0xd4, 
  0x0d, 0x06, 0x1b, 0x10, 0x21, 0x2a, 0x37, 0x3c, 
  0x55, 0x5e, 0x43, 0x48, 0x79, 0x72, 0x6f, 0x64, 
  0xa7, 0xac, 0xb1, 0xba, 0x8b, 0x80, 0x9d, 0x96, 
  0xff, 0xf4, 0xe9, 0xe2, 0xd3, 0xd8, 0xc5, 0xce, 
  0x17, 0x1c, 0x01, 0x0a, 0x3b, 0x30, 0x2d, 0x26, 
  0x4f, 0x44, 0x59, 0x52, 0x63, 0x68, 0x75, 0x7e, 
  0xcc, 0xc7, 0xda, 0xd1, 0xe0, 0xeb, 0xf6, 0xfd, 
  0x94, 0x9f, 0x82, 0x89, 0xb8, 0xb3, 0xae, 0xa5, 
  0x7c, 0x77, 0x6a, 0x61, 0x50, 0x5b, 0x46, 0x4d, 
  0x24, 0x2f, 0x32, 0x39, 0x08, 0x03, 0x1e, 0x15, 
  0x71, 0x7a, 0x67, 0x6c, 0x5d, 0x56, 0x4b, 0x40, 
  0x29, 0x22, 0x3f, 0x34, 0x05, 0x0e, 0x13, 0x18, 
  0xc1, 0xca, 0xd7, 0xdc, 0xed, 0xe6, 0xfb, 0xf0, 
  0x99, 0x92, 0x8f, 0x84, 0xb5, 0xbe, 0xa3, 0xa8, 
  0x1a, 0x11, 0x0c, 0x07, 0x36, 0x3d, 0x20, 0x2b, 
  0x42, 0x49, 0x54, 0x5f, 0x6e, 0x65, 0x78, 0x73, 
  0xaa, 0xa1, 0xbc, 0xb7, 0x86, 0x8d, 0x90, 0x9b, 
  0xf2, 0xf9, 0xe4, 0xef, 0xde, 0xd5, 0xc8, 0xc3 
};

static uchar g_ext_mcu_vfd_show = 0;

static uchar calculate_crc(uchar * message, int length);
static int ext_mcu_send_cmd(E_MCU_CMD eCmd, uchar *pu1DataBuf, uchar u1DataLen);
static int ext_mcu_read_data(E_MCU_CMD eCmd, uchar *pu1DataBuf, uchar u1DataLen);
static int ext_mcu_set_blue_led_state(E_BLUE_LED_CMD t_led_state);
static int ext_mcu_set_power_state(E_PWR_STAT t_ower_state);

static uchar calculate_crc(uchar * message, int length) 
{ 
    /*add CRC for CEC MCU*/
    int i=0; 
    uchar crc = 0;  
    
  for ( i = 0; i < length; ++i ) 
  { 
    crc = crc_table[message[i] ^ crc]; 
  }
  
  return crc; 
} 

static int ext_mcu_send_cmd(E_MCU_CMD eCmd, uchar *pu1DataBuf, uchar u1DataLen)
{
    if((!pu1DataBuf) || 
       (u1DataLen >= EXT_MCU_DATA_MAX))//reserve 1 Byte for CRC
    {
        return -1;
    }
    
    pu1DataBuf[u1DataLen] = calculate_crc(pu1DataBuf, u1DataLen);
    u1DataLen += 1; //For crc byte

    return i2c_write(EXT_MCU_ADDR, eCmd, 1, pu1DataBuf, u1DataLen);
}

static int ext_mcu_read_data(E_MCU_CMD eCmd, uchar *pu1DataBuf, uchar u1DataLen)
{
    uchar u1Crc = 0;

    if((!pu1DataBuf) || 
       (u1DataLen > EXT_MCU_DATA_MAX))
    {
        return -1;
    }
    
    if(0 != i2c_read(EXT_MCU_ADDR, eCmd, 1, pu1DataBuf, u1DataLen))
    {
        return -1;
    }
    
    /*Calculate crc except last CRC byte*/
    u1Crc = calculate_crc(pu1DataBuf, u1DataLen - 1);
    if(u1Crc != pu1DataBuf[u1DataLen - 1])
    {
    //    return -1;
    }

    return 0;
}

/*To inform CEC MCU the power status of Host*/
static int ext_mcu_set_power_state(E_PWR_STAT t_ower_state)
{
    uchar pu1Data[EXT_MCU_DATA_MAX] = {0};

    pu1Data[0] = t_ower_state;
    return ext_mcu_send_cmd(MCU_CMD_SET_POWER_STATE, pu1Data, 1);
}

static E_WAKE_REASON ext_mcu_get_wake_reason(E_WAKE_KEY *pu1WakeKey)
{
    int iret = 0;
    uchar pu1Data[EXT_MCU_DATA_MAX] = {0};
    E_WAKE_REASON eWakeReason = WAKE_INVALLID;

    iret = ext_mcu_read_data(MCU_CMD_GET_WAKE_REASON, pu1Data, EXT_MCU_WAKE_REASON_DATA_LEN);
    if(0 == iret)
    {
        switch (pu1Data[0])
        {
            case WAKE_SSP:
            case WAKE_KEY:
			case WAKE_VFD_IR_KEY:
                eWakeReason = (E_WAKE_REASON)pu1Data[0];
                break;
            default:
                break;
        }
    }
    
    if(WAKE_VFD_IR_KEY == eWakeReason)
    {
        *pu1WakeKey = (E_WAKE_KEY)pu1Data[1];
    }
    
    return eWakeReason;
}

static int ext_mcu_set_blue_led_state(E_BLUE_LED_CMD t_led_state)
{
    uchar pu1Data[EXT_MCU_DATA_MAX] = {0};

    pu1Data[0] = t_led_state;
    return ext_mcu_send_cmd(MCU_CMD_SET_BLUE_LED_STATE, pu1Data, 1);
}

extern void ext_mcu_init(void)
{
    E_WAKE_REASON eWakeReason = WAKE_INVALLID;
    E_WAKE_KEY u1WakeKey = KEY_INVALLID;
    uint u4data = PDWNC_READ32(REG_RW_ARM_IND_DATA0);
    
    eWakeReason = ext_mcu_get_wake_reason(&u1WakeKey);
    printf("[ext_mcu] ext_mcu_get_wake_reason(0x%02X), wakekey(0x%02X).\n", eWakeReason, u1WakeKey);

    if(WAKE_INVALLID != eWakeReason)
    {
        int iret = 0;
        
        if(eWakeReason == WAKE_VFD_IR_KEY)
        {
            if(u1WakeKey == KEY_POWER)
            {
                u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
                u4data |= RW_ARM_IND_DATA0_KEY_POWER;
            }
            else if(u1WakeKey == KEY_PLAY)
            {
                u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
                u4data |= RW_ARM_IND_DATA0_KEY_PLAY;
            }
            else if(u1WakeKey == KEY_EJECT)
            {
                u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
                u4data |= RW_ARM_IND_DATA0_KEY_EJECT;
            }
            else if(u1WakeKey == KEY_DISC)
            {
                u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
                u4data |= RW_ARM_IND_DATA0_KEY_DISC;
            }
            else if(u1WakeKey == KEY_HOME)
            {
                u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
                u4data |= RW_ARM_IND_DATA0_KEY_HOME;
            }
            
        }
        
        g_ext_mcu_vfd_show = 1;
            
        //power on to normal mode
        PDWNC_WRITE32(REG_RW_ARM_IND_DATA0, u4data);
        iret = ext_mcu_set_power_state(SYS_PWR_ON);
        iret |= ext_mcu_set_blue_led_state(BLUE_LED_FADE_IN);
        printf("[ext_mcu] mcu standby to normal(0x%08X) %d.\n", u4data, iret);
    }
    else
    {
        // pass to REG_RW_ARM_IND_DATA0
        uint u4temp = PDWNC_READ32(REG_RW_AUX_IND_DATA0);
        
        if(u4temp & RW_AUX_IND_DATA0_KEY_POWER)
        {
            u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
            u4data |= RW_ARM_IND_DATA0_KEY_POWER;
        }
        else if(u4temp & RW_AUX_IND_DATA0_KEY_EJECT)
        {
            u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
            u4data |= RW_ARM_IND_DATA0_KEY_EJECT;
        }
        else if(u4temp & RW_AUX_IND_DATA0_KEY_PLAY)
        {
            u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
            u4data |= RW_ARM_IND_DATA0_KEY_PLAY;
        }
        else if(u4temp & RW_AUX_IND_DATA0_KEY_DISC)
        {
            u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
            u4data |= RW_ARM_IND_DATA0_KEY_DISC;
        }
        else if(u4temp & RW_AUX_IND_DATA0_KEY_HOME)
        {
            u4data &= ~RW_ARM_IND_DATA0_KEY_MASK;
            u4data |= RW_ARM_IND_DATA0_KEY_HOME;
        }
        
        PDWNC_WRITE32(REG_RW_ARM_IND_DATA0, u4data);
        printf("[ext_mcu] 8032 pass power on key to arm 0x%08X.\n", u4data);
    }
}

uchar ext_mcu_vfd_show(void)
{
    return g_ext_mcu_vfd_show;
}

#endif

#endif /* _EXT_MCU_C_ */

