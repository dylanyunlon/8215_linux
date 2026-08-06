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

#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/semaphore.h> 
#include "ac823x_ir_regs.h"
#include "ac823x_ir_drv.h"

#define REPEATCOUNTUSEDFORREPORTINPUTEVENT 4

enum ir_state
{
    STATE_INACTIVE,
    STATE_LEADER,
    STATE_REPEAT,
};

static bool _fgSemaInit = false;
static struct semaphore _hSemaKey ;  /* Only Initial at boot time, even reset or stop, it won't delete. */
static unsigned int _u4CurrKey = BTN_NONE;  //volatile
static unsigned int _u4PrevKey = BTN_NONE;
static unsigned int _irReceiverState = STATE_INACTIVE;
static unsigned int u4PrevKey = BTN_NONE, rDelta = 0;
static unsigned long rPrevTime;


static unsigned long _dwGroupId = 0xff00;
static unsigned long _dwIRProtocol = 0x0;//0:NEC 1:RC6 2:RCS 3:PAN 4:SIRC SJVC
static unsigned long _dwIRXPOLL1ST = 0;
static unsigned long _dwIRXPOLL2ND = 0;
static unsigned long ui4IrRxPollSpace = 0;

static unsigned long _dwNECPOLL1ST = 0x46;
static unsigned long _dwNECPOLL2ND = 0x73;
static unsigned long _dwJVCPOLL1ST = 0x46;
static unsigned long _dwJVCPOLL2ND = 0x46;
static unsigned long _dwCUSPOLL1ST = 0x70;
static unsigned long _dwCUSPOLL2ND = 0x70;

static unsigned int _u4_1stPulse = 8;
static unsigned int _u4_2ndPulse = 0;
static unsigned int _u4_3rdPulse = 0;
/* this variable will block the same key in 400 ms now. */
static unsigned int _u4IrRxTimeSlice = ATC_IRRX_TIMESLICE; 
static int _fgRepeat = 1;


const unsigned char  AC823xScanCodeToIndexTable[SCAN_CODE_MAX] =
{
    0xFF,                       //  Scan Code 0x00
    0x03,                       //  Scan Code 0x01  
    0xFF,                       //  Scan Code 0x02
    0xFF,                       //  Scan Code 0x03
    0xFF,                       //  Scan Code 0x04  
    0x02,                       //  Scan Code 0x05,
    0x46,                       //  Scan Code 0x06  L-button
    0xFF,                       //  Scan Code 0x07
    0xFF,                       //  Scan Code 0x08    
    0x01,                       //  Scan Code 0x09 
    0x10,                       //  Scan Code 0x0A
    0xFF,                       //  Scan Code 0x0B   
    0x21,                       //  Scan Code 0x0C   
    0xFF,                       //  Scan Code 0x0D   
    0x0b,                       //  Scan Code 0x0E
    0x1c,                       //  Scan Code 0x0F  
    0xFF,                       //  Scan Code 0x10 
    0xFF,                       //  Scan Code 0x11   
    0xFF,                       //  Scan Code 0x12   
    0xFF,                       //  Scan Code 0x13
    0xFF,                       //  Scan Code 0x14
    0xFF,                       //  Scan Code 0x15   
    0xFF,                       //  Scan Code 0x16
    0xFF,                       //  Scan Code 0x17  
    0xFF,                       //  Scan Code 0x18 
    0xFF,                       //  Scan Code 0x19   
    0xFF,                       //  Scan Code 0x1A   
    0xFF,                       //  Scan Code 0x1B   
    0xFF,                       //  Scan Code 0x1C
    0xFF,                       //  Scan Code 0x1D
    0xFF,                       //  Scan Code 0x1E
    0xFF,                       //  Scan Code 0x1F  
    0xFF,                       //  Scan Code 0x20
    0xFF,                       //  Scan Code 0x21    
    0xFF,                       //  Scan Code 0x22
    0xFF,                       //  Scan Code 0x23   
    0xFF,                       //  Scan Code 0x24
    0xFF,                       //  Scan Code 0x25
    0xFF,                       //  Scan Code 0x26
    0xFF,                       //  Scan Code 0x27
    0xFF,                       //  Scan Code 0x28
    0xFF,                       //  Scan Code 0x29
    0xFF,                       //  Scan Code 0x2A
    0xFF,                       //  Scan Code 0x2B
    0xFF,                       //  Scan Code 0x2C    
    0xFF,                       //  Scan Code 0x2D
    0xFF,                       //  Scan Code 0x2E
    0xFF,                       //  Scan Code 0x2F
    0xFF,                       //  Scan Code 0x30
    0xFF,                       //  Scan Code 0x31
    0xFF,                       //  Scan Code 0x32
    0xFF,                       //  Scan Code 0x33
    0xFF,                       //  Scan Code 0x34
    0xFF,                       //  Scan Code 0x35    
    0xFF,                       //  Scan Code 0x36
    0xFF,                       //  Scan Code 0x37
    0xFF,                       //  Scan Code 0x38
    0xFF,                       //  Scan Code 0x39
    0xFF,                       //  Scan Code 0x3A
    0xFF,                       //  Scan Code 0x3B
    0xFF,                       //  Scan Code 0x3C
    0xFF,                       //  Scan Code 0x3D
    0xFF,                       //  Scan Code 0x3E    
    0xFF,                       //  Scan Code 0x3F
    0x1e,//0x24,                //  Scan Code 0x40  right
    0x0a,                       //  Scan Code 0x41
    0x09,                       //  Scan Code 0x42
    0x06,                       //  Scan Code 0x43
    0x1b,//0x31,                //  Scan Code 0x44   up
    0xFF,                       //  Scan Code 0x45
    0x08,                       //  Scan Code 0x46
    0x05,                       //  Scan Code 0x47   
    0x1a,//0x28,                //  Scan Code 0x48  down
    0xFF,                       //  Scan Code 0x49   
    0x07,                       //  Scan Code 0x4A
    0x04,                       //  Scan Code 0x4B
    0x1d,//0x2B,                //  Scan Code 0x4C  left
    0xFF,                       //  Scan Code 0x4D
    0xFF,                       //  Scan Code 0x4E    
    0xFF,                       //  Scan Code 0x4F
    0x1F,                       //  Scan Code 0x50
    0x20,                       //  Scan Code 0x51 
    0xFF,                       //  Scan Code 0x52
    0xFF,                       //  Scan Code 0x53
    0xFF,                       //  Scan Code 0x54
    0xFF,                       //  Scan Code 0x55 
    0xFF,                       //  Scan Code 0x56
    0xFF,                       //  Scan Code 0x57   
    0xFF,                       //  Scan Code 0x58 
    0xFF,                       //  Scan Code 0x59  
    0xFF,                       //  Scan Code 0x5A
    0xFF,                       //  Scan Code 0x5B
    0xFF,                       //  Scan Code 0x5C
    0xFF,                       //  Scan Code 0x5D
    0xFF,                       //  Scan Code 0x5E    
    0xFF,                       //  Scan Code 0x5F
}; 

const unsigned char  MainScanCodeTable[MAIN_SCAN_CODE_TABLE_SIZE] =
{
    0,     
    2,  
    3, 
    4, 
    5,
    6,
    7, 
    8,
    9,
    10,
    11, 
    158,
    230, 
    60, 
    107,
    62, 
    229, 
    139, 
    59, 
    127, 
    217,
    228, 
    227,
    231,
    61, 
    232, 
    108,
    103,
    102,
    105,
    106,
    115,
    114,
    116,
    212,
    16,
    17, 
    18,
    19, 
    20,
    21, 
    22,
    23,  
    24,
    25,
    26,
    27,
    43,
    30,
    31,
    32,
    33,
    34, 
    35,
    36,
    37,
    38,
    39,
    40,
    14,
    44,
    45,
    46,
    47,
    48, 
    49,
    50,
    51, 
    52, 
    53,
    28,
    56, 
    100,
    42,
    54,
    15, 
    57,
    150,
    155,
    12,
    13,
    215,
    1,
    68,
}; // MainScanCodeTable[]

int IRRX_InitMtkIr(void *dev_id);
int i4IrUninit(void *dev_id) ;

extern int IRHW_RxInit(int i4Config, int i4SaPeriod, int i4Threshold, void *dev_id);
extern int _IRHW_TxSetIsr(bool fgSet);



bool GetTime(unsigned long *pTime)
{
    *pTime = jiffies*1000/HZ;
    return true;    
}

bool GetDeltaTime(unsigned int* pTime, unsigned long StartTime, unsigned long StopTime)
{
    *pTime = (unsigned int)(StopTime - StartTime);

    return true;    
}

void EnjectEvent(unsigned int u4PrevKey)
{
    DEBUGMSG(ZONE_FUNCTION, ("EnjectEvent \r\n" ));

    if(_fgSemaInit == true)
    {

        _u4CurrKey = u4PrevKey;
        up(&_hSemaKey);
        DEBUGMSG(ZONE_WARN, (TEXT("[IR]CB:EnjectEvent irrx_if unlock \r\n")));

    }
}


/****************************************************************
                      Internal Functions
********************************************************************/

 /** 
  *   static unsigned int _IRRX_XferRC6ToCrystal(unsigned int u4Info, const unsigned char * pu1Data)
  *     Map the key code to Crystal button as BTN_DIGITAL_1 and so on.
  *     u4Info  contain the number of decoded code and the value of the 
  *      sampling counter in the 1st pulse.
  *      pu1Data  contian the decoded code, including key value and custom 
  *      value  
  *      unsigned int  BTN_NONE            Invalid key code
  *                   BTN_KEY_REPEAT       The key is remaining pressed
  *                   Other BTN_           New key code.
  */
static unsigned int _IRRX_XferRC6ToCrystal(unsigned int u4Info, const unsigned char * pu1Data, bool u1NeedRelease)
{
    unsigned int u4BitCnt = 0;
    static unsigned int u4TogKey = 0;
    unsigned int u4RC6key = 0;

    u4BitCnt = INFO_TO_BITCNT(u4Info);

    if(NULL == pu1Data)
        return BTN_NONE;
    
    /* Check data. */
    if ((u4BitCnt != IRRX_RC6_BITCNT) 
        || (pu1Data == NULL)
        || (IRRX_RC6_GET_CUSTOM(pu1Data[0], pu1Data[1]) != IRRX_RC6_CUSTOM)
        || (IRRX_RC6_GET_LEADER(pu1Data[0]) != IRRX_RC6_LEADER))
    {
        DEBUGMSG( ZONE_FUNCTION , (TEXT("[IR] Bitcnt: 0x%02x, Leader: 0x%02x, Custom: 0x%02x\n "), 
             u4BitCnt, IRRX_RC6_GET_LEADER(pu1Data[0]), IRRX_RC6_GET_CUSTOM(pu1Data[0], pu1Data[1])));
        return BTN_NONE;
    }

    if (u4TogKey != IRRX_RC6_GET_TOGGLE(pu1Data[0]))
    {
        //a new key
        u4RC6key = IRRX_RC6_GET_KEYCODE(pu1Data[1], pu1Data[2]);
        u4TogKey = IRRX_RC6_GET_TOGGLE(pu1Data[0]);
        DEBUGMSG( ZONE_FUNCTION , (TEXT("[IR] a RC6 key down: 0x%02x  toggle: 0x%02x\n "), u4RC6key, u4TogKey));

        if(u4RC6key < 0xA0) 
        {
            _u4PrevKey = u4RC6key;
            return _u4PrevKey;
        }
        else
        {
          return BTN_NONE;
        }
    }
    else
    {                           //key hold
       DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR]  a RC6 key hold : 0x%02x\n"), IRRX_RC6_GET_KEYCODE(pu1Data[1],
                                                              pu1Data[2])));
       if (_fgRepeat)
       {
         return BTN_KEY_REPEAT;
       }
       else
       {
           return _u4PrevKey;
       }
    }
   
}

 /**
  *  static unsigned int _IRRX_XferSIRCToCrystal(unsigned int u4Info, const unsigned char * pu1Data)
  *     Map the key code to Crystal button as BTN_DIGITAL_1 and so on.
  *     u4Info  contain the number of decoded code and the value of the 
  *            sampling counter in the 1st pulse.
  *     pu1Data  contian the decoded code, including key value and custom 
  *      value  
  *      unsigned int  BTN_NONE            Invalid key code
  *                   BTN_KEY_REPEAT       The key is remaining pressed
  *                   Other BTN_           New key code.
  */
static unsigned int _IRRX_XferSIRCToCrystal(unsigned int u4Info, const unsigned char * pu1Data, bool u1NeedRelease)
{
    unsigned int u4BitCnt;   

    u4BitCnt = INFO_TO_BITCNT(u4Info);

    /* Check empty data. */
    if ((u4BitCnt == 0) || (pu1Data == NULL))
    {
        // V_IR_FAILED
        return BTN_NONE;
    }

    switch (u4BitCnt)
    {
        case IRRX_SIRC_BITCNT12:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] Received Key: 0x%x%x\n"), *(pu1Data+1), *(pu1Data)));
            return *pu1Data;
        case IRRX_SIRC_BITCNT15:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] Received Key: 0x%x%x\n"), *(pu1Data+1), *(pu1Data)));
            return BTN_NONE;
            
        case IRRX_SIRC_BITCNT20:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] Received Key: 0x%x%x%x%x\n"), 
                *(pu1Data+3), *(pu1Data+2), *(pu1Data+1), *(pu1Data)));
            return BTN_NONE;
        default:
            return BTN_NONE;
    }

}

static unsigned int _IRRX_XferJVCToCrystal(unsigned int u4Info, const unsigned char * pu1Data, bool u1NeedRelease)
{
    unsigned int u4CusId, u4BitCnt, u4Code;
    static unsigned int u4PreCode = 0xFF;

    /* Check empty data. */
    u4BitCnt = INFO_TO_BITCNT(u4Info);
    if ((u4BitCnt == 0) || (pu1Data == NULL))
    {
         return BTN_NONE;
    }

    u4Code = pu1Data[2];
    u4Code = (u4Code << 8) + pu1Data[1];
    u4Code = (u4Code << 8) + pu1Data[0];
    
    /* Check repeat key. */
    if ((u4BitCnt == IRRX_JVC_BITCNT_REPEAT) && (true == u1NeedRelease))
    {
        u4CusId = u4Code & 0xFF;
        u4Code = (u4Code >> 8) & 0xFF;
        
        if ((u4CusId == IRRX_JVC_CUSTOM) && 
            (u4Code == u4PreCode) &&
            (INFO_TO_1STPULSE(u4Info) <= IRRX_JVC_1ST_PULSE_REPEAT)&&
            (INFO_TO_2NDPULSE(u4Info) <= 2) &&
            (INFO_TO_3RDPULSE(u4Info) <= 2))
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] KeyCode is 0x%02x - repeat\n"), u4Code));
            if (_fgRepeat)
            {
                return BTN_KEY_REPEAT;
            }
            else
            {
                return _u4PrevKey;
            }
        }
        else
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR](%d) BTN_NONE\n"), __LINE__));
            return BTN_NONE;
        }
    }

    u4Code >>= 1;
    u4CusId = u4Code & 0xFF;
    u4Code = (u4Code >> 8) & 0xFF;
        
    /* Check invalid pulse. */
    if ((u4CusId != IRRX_JVC_CUSTOM)
        || (u4BitCnt != IRRX_JVC_BITCNT_NORMAL)
        || (INFO_TO_1STPULSE(u4Info) != IRRX_JVC_1ST_PULSE_NORMAL)
        || (INFO_TO_2NDPULSE(u4Info) > (2))
        || (INFO_TO_3RDPULSE(u4Info) > (2)))
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR](%d) BTN_NONE\r\n"), __LINE__));
        return BTN_NONE;
    }

    u4PreCode = u4Code;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR] KeyCode is 0x%02x\r\n"), u4Code));
    if (u4Code >= IRRX_JVC_MAX_MAP_ENTRY)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR](%d) BTN_NONE\r\n"), __LINE__));
        return BTN_NONE;
    }
    _u4PrevKey = u4Code;
    return _u4PrevKey;
    
}

 /**
  * static unsigned int _IRRX_XferMtkToCrystal(unsigned int u4Info, const unsigned char * pu1Data)
  *    Map the key code to Crystal button as BTN_DIGITAL_1 and so on.
  *    u4Info  contain the number of decoded code and the value of the 
  *          sampling counter in the 1st pulse.
  *    pu1Data  contian the decoded code, including key value and custom value  
  *
  *    unsigned int  BTN_NONE            Invalid key code
  *                 BTN_KEY_REPEAT       The key is remaining pressed
  *                 Other BTN_           New key code.
  */
static unsigned int _IRRX_XferMtkToCrystal(unsigned int u4Info, const unsigned char * pu1Data, bool u1NeedRelease)
{
    unsigned int u4GrpId, u4BitCnt;

    DEBUGMSG(ZONE_WARN, (TEXT("_IRRX_XferMtkToCrystal\n")));

    u4BitCnt = INFO_TO_BITCNT(u4Info);

    /* Check empty data. */
    if ((u4BitCnt == 0) || (pu1Data == NULL))
    {
           return BTN_NONE;
    }

    /* Check repeat key. */
    if ((u4BitCnt == ATC_IRRX_BITCNT_REPEAT) && (true == u1NeedRelease))
    {
        if (((INFO_TO_1STPULSE(u4Info) == ATC_IRRX_1st_Plus_REPEAT) ||
             (INFO_TO_1STPULSE(u4Info) == ATC_IRRX_1st_Plus_REPEAT + 1)) &&
            (INFO_TO_2NDPULSE(u4Info) == 0) &&
            (INFO_TO_3RDPULSE(u4Info) == 0))
        {
            if (_fgRepeat)
            {
                DEBUGMSG(ZONE_WARN, (TEXT("[IR] repeat key happen\r\n")));
                return BTN_KEY_REPEAT;
            }
            else
            {
                DEBUGMSG(ZONE_WARN, (TEXT("[IR] repeat key, _u4PrevKey = %x\r\n"),_u4PrevKey));
                return _u4PrevKey;
            }
        }
        else
        {
            DEBUGMSG(ZONE_WARN, (TEXT("[IR](%d) BTN_NONE\r\n"), __LINE__));
            return BTN_NONE;
        }
    }

    /* Check invalid pulse. */
    if ((u4BitCnt != ATC_IRRX_BITCNT_NORMAL)
        || (INFO_TO_1STPULSE(u4Info) != _u4_1stPulse)
        || (INFO_TO_2NDPULSE(u4Info) > (_u4_2ndPulse + 2))
        || (INFO_TO_3RDPULSE(u4Info) > (_u4_3rdPulse + 2)))
    {
        DEBUGMSG(ZONE_WARN, (TEXT("[IR](%d) BTN_NONE\r\n"), __LINE__));
        return BTN_NONE;
    }


    u4GrpId = pu1Data[1];
    u4GrpId = (u4GrpId << 8) + pu1Data[0];

    /* Check GroupId. */
    if (u4GrpId != _dwGroupId)
    {
        DEBUGMSG(ZONE_WARN, (TEXT("[IR](%d) BTN_NONE\r\n"), __LINE__));
        return BTN_NONE;
    }

    /* Check invalid key. */
    if ((pu1Data[2] + pu1Data[3]) != ATC_IRRX_BIT8_VERIFY)
    {
        DEBUGMSG(ZONE_WARN, (TEXT("[IR](%d) BTN_NONE\r\n"), __LINE__));
        return BTN_NONE;
    }

    /* Here, pu1Data[2] is the key of MTKDVD remote controller. */
    DEBUGMSG(ZONE_WARN, (TEXT("[IR] KeyCode is 0x%02x\r\n"), pu1Data[2]));
    if (pu1Data[2] >= ATC_NEC_MAX_MAP_ENTRY)
    {
        DEBUGMSG(ZONE_WARN, (TEXT("[IR](%d) BTN_NONE\r\n"), __LINE__));
        return BTN_NONE;
    }

    _u4PrevKey = pu1Data[2];
    return _u4PrevKey;

    
}
 /** 
  * @FUNCTION: static void _IRRX_MtkIrCallback(unsigned int u4Info, const unsigned char * pu1Data)
  *   
  * @Description: brief Callback function. If new key is pressed, Set the global variable 
  *    _u4CurrKey ,and unlock the semaphore. Next polling in IR thread will 
  *    notice this change, and send event to IO manager.
  *    param u4Info  contain the number of decoded code and the value of the 
  *          sampling counter in the 1st pulse.
  *    param  pu1Data  contian the decoded code, including key value and custom 
  *    value  
  */
static void _IRRX_MtkIrCallback(unsigned int u4Info, const unsigned char * pu1Data)
{
    unsigned int u4CrystalKey;
    unsigned long rTime;

    printk("_IRRX_MtkIrCallback\r\n");   

    if ((u4Info == 0) || (pu1Data == NULL))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("[IR]_IRRX_MtkIrCallback error!\r\n")));
        return;
    }

    if (_dwIRProtocol == IRRX_RC_NEC )
    
        u4CrystalKey = _IRRX_XferMtkToCrystal(u4Info, pu1Data, true);
    
    else if (_dwIRProtocol == IRRX_RC_RC6) 
    
        u4CrystalKey = _IRRX_XferRC6ToCrystal(u4Info, pu1Data, true);
    else if (_dwIRProtocol == IRRX_RC_SIRC) 

       u4CrystalKey = _IRRX_XferSIRCToCrystal(u4Info, pu1Data, true);
    else if (_dwIRProtocol == IRRX_RC_JVC) 

       u4CrystalKey = _IRRX_XferJVCToCrystal(u4Info, pu1Data, true);
    else  //default is nec protol 
        u4CrystalKey = _IRRX_XferMtkToCrystal(u4Info, pu1Data, true);
    


    DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR]CB: u4CrystalKey is 0x%08x\r\n"), u4CrystalKey));
    
    if (u4CrystalKey == BTN_NONE)
    {
        DEBUGMSG(ZONE_WARN,
            (TEXT("[IR]CB:Invalid IRcode u4Info:0x%08x pu1Data:0x%02x%02x%02x%02x\r\n"),
             u4Info, pu1Data[0], pu1Data[1], pu1Data[2],
            pu1Data[3]));
        //return;
    }

    if (!_fgRepeat && (u4PrevKey == u4CrystalKey))
    {
        GetTime(&rTime);
        DEBUGMSG(ZONE_FUNCTION, ("gettime rTime %ld\r\n", rTime));
    
        GetDeltaTime(&rDelta, rPrevTime, rTime);
        
        DEBUGMSG(ZONE_FUNCTION, ("gettime rDelta %d \r\n", rDelta));
    
        if (rDelta < 1000 && rDelta < _u4IrRxTimeSlice) 
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR]CB: Repeat code but in %d timeslice.\r\n"),_u4IrRxTimeSlice));
            DEBUGMSG(ZONE_FUNCTION, (TEXT("[IR]CB: Repeat code but in %d timeslice.\r\n"),_u4IrRxTimeSlice));
            return;
        }
        else
        {
            GetTime(&rPrevTime);
            DEBUGMSG(ZONE_FUNCTION, ("gettime rPrevTime  %ld\r\n", rPrevTime));
            DEBUGMSG(ZONE_FUNCTION, (" gettime rPrevTime  %ld\r\n", rPrevTime));
        }
    }
    else
    {


            DEBUGMSG(ZONE_FUNCTION, (" u4PrevKey, u4CrystalKey %x, %x \r\n", u4PrevKey, u4CrystalKey ));
            
            switch(_irReceiverState)
            {
                case STATE_INACTIVE:
                    if(BTN_NONE != u4CrystalKey && BTN_KEY_REPEAT != u4CrystalKey)
                    {
                        u4PrevKey = u4CrystalKey;
                        _irReceiverState = STATE_LEADER;
                        DEBUGMSG(ZONE_WARN, ("current state is STATE_INACTIVE -->> STATE_LEADER \r\n" ));
                    }
                    break;
                case STATE_LEADER:
                    if(BTN_KEY_REPEAT == u4CrystalKey)
                    {
                        _irReceiverState = STATE_REPEAT;
                        DEBUGMSG(ZONE_WARN, ("current state is STATE_LEADER-->>STATE_REPEAT \r\n" ));
                    }
                    else
                    {
                        _irReceiverState = STATE_INACTIVE;
                        DEBUGMSG(ZONE_WARN, ("current state is STATE_LEADER-->> STATE_INACTIVE \r\n" ));
                    }
                    break;
                case STATE_REPEAT:
                    if(BTN_KEY_REPEAT != u4CrystalKey)
                    {
                        //u4PrevKey = BTN_NONE;
                        _irReceiverState = STATE_INACTIVE;
                        DEBUGMSG(ZONE_WARN, ("current state is STATE_REPEAT -->> STATE_INACTIVE \r\n" ));
                    }    
                    break;
            }

            DEBUGMSG(ZONE_FUNCTION, (" u4PrevKey, u4CrystalKey %x, %x \r\n", u4PrevKey, u4CrystalKey ));

            
            if(STATE_LEADER == _irReceiverState)
            {
                EnjectEvent(u4PrevKey);    
                //GetTime(&rPrevTime);
                DEBUGMSG(ZONE_FUNCTION, (" gettime rPrevTime  %ld\r\n", rPrevTime));
            }
            else if(STATE_REPEAT == _irReceiverState)
            {
                    EnjectEvent(u4PrevKey);    
            }

        return;

    }

    return;
}


 /**  
  *  int IRRX_InitMtkIr(void)
  *  brief Init IR module, including reset ISR, set callback function for key 
  *    code processing, and init hardware according to the IRRX_RC_PROTOCOL
  *  @Return  IR_SUCC             successfully
  *           IR_FAIL       failed
  */
int IRRX_InitMtkIr(void *dev_id)
{
    int i4Ret;
    PFN_IRRXCB_T pfnOld;

    _u4CurrKey = BTN_NONE;
    _u4PrevKey = BTN_NONE;
    _fgRepeat = 1;

    if (_dwIRProtocol == IRRX_RC_NEC )   
    {
        _dwIRXPOLL1ST = _dwNECPOLL1ST;
        _dwIRXPOLL2ND = _dwNECPOLL2ND;
    }
    else if(_dwIRProtocol == IRRX_RC_JVC)
    {
        _dwIRXPOLL1ST = _dwJVCPOLL1ST;
        _dwIRXPOLL2ND = _dwJVCPOLL2ND;
    }
    else
    {
        _dwIRXPOLL1ST = _dwCUSPOLL1ST;
        _dwIRXPOLL2ND = _dwCUSPOLL2ND;
    }
    
    ui4IrRxPollSpace = _dwIRXPOLL2ND;
    DEBUGMSG(ZONE_FUNCTION, ("[IRX]Registry _dwIRXPOLL1ST = 0x%lx,_dwIRXPOLL2ND=0x%lx .\r\n", _dwIRXPOLL1ST,_dwIRXPOLL2ND));
        


    //IRRX_StopMtkIr();//stop first, for the 8520 boot up test, by msz00420
    i4Ret = IRHW_RxSetCallback(_IRRX_MtkIrCallback, &pfnOld);
    
    if (i4Ret != IR_SUCC)
    {
        return IR_FAIL;
    }

    if (_dwIRProtocol == IRRX_RC_NEC )   
        i4Ret =   IRHW_RxInit(ATC_IRRX_CONFIG, ATC_IRRX_SAPERIOD, ATC_IRRX_THRESHOLD, dev_id);
    else if (_dwIRProtocol == IRRX_RC_RC6) 
        i4Ret = IRHW_RxInit(IRRX_RC6_CONFIG, IRRX_SAPERIOD_RC6, ATC_IRRX_THRESHOLD, dev_id);
    else if (_dwIRProtocol == IRRX_RC_SIRC) 
        i4Ret = IRHW_RxInit(IRRX_SIRC_CONFIG,  IRRX_SAPERIOD_SIRC,  ATC_IRRX_THRESHOLD, dev_id);
    else if (_dwIRProtocol == IRRX_RC_JVC) 
        i4Ret = IRHW_RxInit(IRRX_JVC_CONFIG,  IRRX_SAPERIOD_JVC,  ATC_IRRX_THRESHOLD, dev_id);
    else  //default is nec protol 
       i4Ret =   IRHW_RxInit(ATC_IRRX_CONFIG, ATC_IRRX_SAPERIOD, ATC_IRRX_THRESHOLD, dev_id);
       
    if (i4Ret != IR_SUCC)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("[IR] ir init fail : 0x%08x\r\n"), i4Ret));
        return IR_FAIL;
    }
    
    if (!_fgSemaInit)
    {
    //    init_MUTEX_LOCKED(&_hSemaKey);
        _fgSemaInit = true;
    }

    printk("[IR] ir init ok \r\n");

#if (CONFIG_ARM2_EJECT) 
    if(IRRX_FastEject_Init() != IR_SUCC)
    {
        return IR_FAIL;
    }
#endif

    return IR_SUCC;
}


int i4IrUninit(void *dev_id) 
{
    int i4Ret;
    int i4Error = 0;

    i4IrHWUninit(dev_id);
    
    i4Ret = IRHW_RxSetCallback(NULL, NULL);

     if (i4Ret != IR_SUCC)
    {
         i4Error = IR_FAIL;
    }
    
    _fgSemaInit = false;

  return IR_SUCC;

}

/**
 *  int IRRX_StopMtkIr(void)
 *  Stop IR module       
 *    int  IR_SUCC      successfully
 *         IR_FAIL       failed
 */
int IRRX_StopMtkIr(void *dev_id)
{
    int i4Ret;

    i4Ret = IRHW_RxStop(dev_id);
    return i4Ret;
}


/**
 * int IRRX_ResetMtkIr(void)
 *    Reset IR module       
 *    int  IR_SUCC       successfully
 *         IR_FAIL       failed
 */
int IRRX_ResetMtkIr(void *dev_id)
{
    int i4Ret;


    i4Ret = IRRX_StopMtkIr(dev_id);
    i4Ret |= IRRX_InitMtkIr(dev_id);
    return i4Ret;
}



/**
 * int IRRX_PollMtkIr(unsigned int * pu4Key)
 *    fetch a key event
 *    pu4Key: Point to a unsigned int data,  which is used to return the 
 *            key value, it will be BTN_NONE if there is no key pressed.
 *    int  IR_SUCC       successfully
 *         IR_FAIL       failed
 */
int IRRX_PollMtkIr(unsigned int * pu4Key)
{
    int i4Ret;    
    unsigned long flags;

    //static unsigned long ui4IrRxPollSpace = _dwIRXPOLL2ND;

    if (pu4Key == NULL)
    {
        return IR_FAIL;
    }

    if(_fgSemaInit == false)
    {
        return IR_FAIL;
    }
    
    i4Ret = down_timeout(&_hSemaKey, ui4IrRxPollSpace*HZ/1000);
    if (!i4Ret)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("\r\n coming nIRRX_PollMtkIr receive ir key=%x.. \r\n"),_u4CurrKey));
        if (_u4CurrKey == BTN_KEY_REPEAT)
        {
            ui4IrRxPollSpace = _dwIRXPOLL2ND;
        }
        else
        {
            ui4IrRxPollSpace = _dwIRXPOLL2ND; //_dwIRXPOLL1ST;
        }

        DEBUGMSG(ZONE_FUNCTION, (TEXT("\r\n before nIRRX_PollMtkIr receive ir key=%x.. \r\n"),*pu4Key));

        local_irq_save(flags);
        *pu4Key = _u4CurrKey;
        _u4CurrKey = BTN_NONE;
        local_irq_restore(flags);  
        DEBUGMSG(ZONE_FUNCTION, (TEXT("\r\n after nIRRX_PollMtkIr receive ir key=%x.. \r\n"),*pu4Key));

    }
    else //if (i4Ret == OSR_TIMEOUT)
    {    
        _irReceiverState = STATE_INACTIVE;
        _u4PrevKey = BTN_NONE;
        *pu4Key = BTN_NONE;
    }
    return IR_SUCC;
}

