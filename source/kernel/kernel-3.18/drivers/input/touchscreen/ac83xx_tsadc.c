#include <linux/kernel.h>
#include <linux/jiffies.h>
#include "ac83xx_tsadc.h"
#include "ac83xx_tstimer.h"

#define ADC_SAMPLE_NUMBER_MAX (1 << 4)  //max is 16 points 
#define ADC_ECC_ERR_NUMBER1    3836
#define ADC_ECC_ERR_NUMBER2    4094
#define TOUCH_RAW_DATA_MODE    1
#define AUXADC_SAMPLE_TICK       0
#define AUXADC_SAMPLE_COUNT      8


#define RETAILMSG(fg, x... )  

//------------------------------------------------------------------------------
// Local Variables
//
const u16 AUX_TS_DEBT_MASK = 0x3fff;  // 14 bits
const u16 AUX_TS_CMD_MASK = 0x7f;    // 7 bits
const u16 AUX_TS_CON_MASK = 0x03;    // 2 bits
const u16 AUX_TS_FILTER_THRESHOLD = 12;

const u16 MAX_ACD_X = 0xfff;
const u16 MAX_ACD_Y = 0xfff;
const u16 MIN_ACD_X = 1;
const u16 MIN_ACD_Y = 1;

const u16 VARIANCE_THRESHOLD_X = 120;
const u16 VARIANCE_THRESHOLD_Y = 100;

extern u32 g_u4EnableCalibrationLog;
extern u32 g_u4TouchResistorThreshold;
extern s32  g_i4TouchResistorOffset;
extern u16 g_u2TouchSampleNum;      
extern u16 g_u2ErrSampleCnt;
extern u16 g_u2HighSampleRateFlag;

static u16 AUXADC_BATCH_TIMER_FLAG = 0;

u32 g_u4ADCLogMask = 0x1;


//------------------------------------------------------------------------------
// Local Functions
//
s32 bitmask = 0xfff;  // for Sample Bit 

//set bit , 1: set
void SB(s32 addr, s32 v) { s32 r = RR(addr); r = r | v;    WR(addr, r); }
//clear bit 1: clear
void CB(s32 addr, s32 v) { s32 r = RR(addr); r = r & (!v); WR(addr, r); }
//write bit, v value,m mask, o shift,from o , write v & m
void WB(s32 addr, s32 v, s32 m, s32 o) { m=m<<o; WR(addr,(RR(addr)&(~m))+((v<<o)&m)); }
//read bit
//v value,m mask, o shift, from o , m bits.
s32  RB(s32 addr, s32 m, s32 o) { return (RR(addr)>>o)&m;}

#define AUX_GET_X_RAWDATA(addr)          RB(addr,bitmask,0)
#define AUX_GET_Y_RAWDATA(addr)          RB(addr,bitmask,0)
#define AUX_GET_Z1_RAWDATA(addr)         RB(addr,bitmask,0)
#define AUX_GET_Z2_RAWDATA(addr)         RB(addr,bitmask,0)


void AuxsetSampleBit(s32 type) { bitmask=(type==AUX_TS_CMD_10BIT_RES?0x3ff:0xfff);WB(AUXADC_TS_CMD,type,1,3); }
s32  AuxgetSampleBit(void) { return RB(AUXADC_TS_CMD,1,3); }

void AuxsetSampleMode(s32 type) { WB(AUXADC_TS_CMD, type, 1, 2); } 
s32  AuxgetSampleMode(void) { return RB(AUXADC_TS_CMD, 1, 2); } 

void AuxsetPowerDownMode(s32 type) { WB(AUXADC_TS_CMD, type, 3, 0); }
s32  AuxgetPowerDownMode(void) { return RB(AUXADC_TS_CMD, 3, 0); }
 
void AuxsetSampleAddr(s32 type) { WR(AUXADC_TS_ADDR, type&0x7); }
s32  AuxgetSampleAddr(void) { return RR(AUXADC_TS_ADDR)&0x7; }

void AuxsetIRQMode(s32 type) { WB(AUXADC_TS_CON0, type, 1,15); }//type ==0 enable pen-irq
s32  AuxgetIRQMode(void) { return RB(AUXADC_TS_CON0, 1, 15); }

s32  AuxgetTouchStatus(void) { return RB(AUXADC_TS_CON0, 1, 1); }
void AuxsetTrigger(void) { SB(AUXADC_TS_CON0, 1); }
s32  AuxgetTrigger(void) { return RB(AUXADC_TS_CON0, 1, 0); }

void AuxsetFAVLatency(s32 value) { WB(AUXADC_TS_CON1, value, 0xff, 8); }
s32  AuxgetFAVLatency(void) { return RB(AUXADC_TS_CON1,0xff, 8); }

void AuxsetFAVEnable(void) { SB(AUXADC_TS_CON1, 0x80); }
s32  AuxgetFAVEnable(void) { return RB(AUXADC_TS_CON1, 0x1, 7); }

void AuxsetFAVCoord(s32 value) { WB(AUXADC_TS_CON1, value, 3, 5); }
s32  AuxgetFAVCoord(void) { return RB(AUXADC_TS_CON1, 3,  5); }

s32  AuxgetFAVInvalid(void) { return RB(AUXADC_TS_CON1, 1, 4); }
void AuxsetFAVAddTwosample(s32 value) { WB(AUXADC_TS_CON1, value, 1, 3); }
s32  AuxgetFAVAddTwosample(void) { return RB(AUXADC_TS_CON1, 1, 3); }

void AuxsetFAVTrigger(s32 value) { WB(AUXADC_TS_CON1, value, 1, 2); }//enable timer
s32  AuxgetFAVTrigger(void) { return RB(AUXADC_TS_CON1, 1, 2); }
void AuxsetInvalidFlag(s32 value){ WB(AUXADC_TS_CON1, value, 1, 4); }


void AuxsetFAVAccCount(s32 value) { WB(AUXADC_TS_CON1, value, 3, 0); }
s32  AuxgetFAVAccCount(void) { return RB(AUXADC_TS_CON1, 3, 0); }

void AuxsetSPLDurationOn(s32 value) { WB(AUXADC_TS_CON2, value, 1, 8); }
s32  AuxgetSPLDurationOn(void) { return RB(AUXADC_TS_CON2, 1, 8); }

void AuxsetSPLDuration(s32 value) { WB(AUXADC_TS_CON2, value, 0xff, 0); }
s32  AuxgetSPLDuration(void) { return RB(AUXADC_TS_CON2, 0xff, 0); }

void AuxsetPullUp(s32 value) { WB(AUXADC_TS_CON3, value, 1, 15); }
s32  AuxgetPullUp(void) { return RB(AUXADC_TS_CON3, 1, 15); }

void AuxsetNonStop(s32 value) { WB(AUXADC_MISC, value, 1, 15); }
s32  AuxgetNonStop(void) { return RB(AUXADC_MISC, 1, 15); }

void AuxsetAutoSample(s32 value) { WB(AUXADC_MISC, value, 1, 8); }
s32  AuxgetAutoSample(void) { return RB(AUXADC_MISC, 1, 8); }

void AuxsetDIV(s32 value) { WB(AUXADC_MISC, value, 0xff, 0); }
s32  AuxgetDIV(void) { return RB(AUXADC_MISC, 0xff, 0); }

void AuxsetAutoInterval(s32 value) { WB(AUXADC_TS_AUTO_TIME_INTVL, value, 0x3ff, 0); }
s32  AuxgetAutoInterval(void) { return RB(AUXADC_TS_AUTO_TIME_INTVL, 0x3ff, 0); }

void AuxsetRAWTrigger(s32 value) { WB(AUXADC_TS_AUTO_CON, value, 1, 2); }





//------------------------------------------------------------------------------
// Local Functions
//
bool AuxSetTSDebt(u16 u2Time)
{
    if (u2Time > AUX_TS_DEBT_MASK)
        return false;

    WR(AUXADC_TS_DEBT0,u2Time & 0x3fff);
    return true;
}
bool AuxSetTSDebt1(u16 u2Time)
{
    if (u2Time > AUX_TS_DEBT_MASK)
        return false;

    WR(AUXADC_TS_DEBT1,u2Time & 0x3fff);
    return true;
}

void DumpNANDRegistry(void)
{
    u32 u4Cnt = 0;

    for (u4Cnt = 0; u4Cnt < 0x2B8; u4Cnt+=4)
    {
        RETAILMSG(1, (TEXT("*[%08X]=0x%08X\r\n"), IO_BASE_VA+0xA9000 + u4Cnt, *(volatile u32 *)(IO_BASE_VA+0xA9000 + u4Cnt)));
        // if (((u4Cnt + 4) % 16) == 0)
        //    RETAILMSG(1, (TEXT("\r\n")));
    }

    return;
}


bool AuxGetTSPressed(bool *pbPressed)
{
    u32 u4Cnt =0;
    if (pbPressed == NULL)
        return false;

    u4Cnt = AuxgetTouchStatus();
    //RETAILMSG(1, (TEXT("u4Cnt = %d\r\n"), u4Cnt));
    *pbPressed = (u4Cnt)? true: false;

    return true;
}

bool AuxGetTSDat(u16* pu2data)
{
  
   *pu2data = RB(AUXADC_TS_DAT0,bitmask,0); 
    return true;
}



bool AuxGetTSPos(TS_POS pos, u16* pu2data)
{

    if ( pos >= TS_POS_NUM || pu2data == NULL)
        return false;

#if AUXADC_BIM_MODE

    u16 u2Addr = 0;
    // 1. Write the sample command
    //u2Con = AUX_TS_CMD_10BIT_RES | AUX_TS_CMD_MODE_DF | AUX_TS_CMD_PD_YDRV_SH;

    switch (pos)
    {
    case TS_POS_X:
        u2Addr = AUX_TS_CMD_ADDRESS_X;
        break;
    case TS_POS_Y:
        u2Addr =  AUX_TS_CMD_ADDRESS_Y;
        break;
    case TS_POS_Z1:
        u2Addr = AUX_TS_CMD_ADDRESS_Z1;
        break;
    case TS_POS_Z2:
        u2Addr = AUX_TS_CMD_ADDRESS_Z2;
        break;
    default:
        //ASSERT(0);
        break;
    }

    AuxsetSampleAddr(u2Addr);

    // 2. Trigger sample process
    AuxsetTrigger();

    // 3. Sample process is done
    #if 0
    while( AuxgetTrigger());
    #endif
    WAIT_FOR_ZERO(AuxgetTrigger(),10,"[Auxadc]get trigger timeout \n");

    AuxGetTSDat(pu2data);


#else

    if(AuxgetFAVInvalid())
    { 
        HAL_LOG(ADC_LOG_LVL_ERR, "INVALID\r\n"); 
        AuxsetInvalidFlag(1);
        return false;
    }
    switch (pos)
    {
    case TS_POS_X:
        *pu2data = RB(AUXADC_TS_DAT0,bitmask,0); 
        break;
    case TS_POS_Y:
        *pu2data = RB(AUXADC_TS_DAT1,bitmask,0); 
        break;
    case TS_POS_Z1:
        *pu2data = RB(AUXADC_TS_DAT2,bitmask,0); 
        break;
    case TS_POS_Z2:
        *pu2data = RB(AUXADC_TS_DAT3,bitmask,0); 
        break;
    default:
        //ASSERT(0);
        break;
    }

#endif
    //RETAILMSG(1, (TEXT("AuxGetTSPos pos = %d *pu2data = %x\r\n"), pos, *pu2data));
    return true;
}

bool IsVaildACDValue(u16 x, u16 y)
{
    if (x <= MIN_ACD_X || x>= MAX_ACD_X || y <= MIN_ACD_Y || y >= MAX_ACD_Y)
        return false;

    return true;
}

u32 TouchResistor(u16 u2X, u16 u2Z1, u16 u2Z2, u16 u2Y)
{
    s32 i4X = (s32)u2X;
    s32 i4Z1 = (s32)u2Z1;
    s32 i4Z2 = (s32)u2Z2;
    s32 i4Resistor = g_u4TouchResistorThreshold * 10;

    if (((i4Z1 + g_i4TouchResistorOffset) > 0) && (u2Z2 > u2Z1) && (i4X > MIN_ACD_X))
        i4Resistor = (i4X * (i4Z2 - i4Z1))/ (i4Z1 + g_i4TouchResistorOffset);

    RETAILMSG(g_u4EnableCalibrationLog, (TEXT("TouchResistor( %d, %d, %d, %d ,%u ) = %u\r\n"), i4X, u2Y, i4Z1, i4Z2, g_i4TouchResistorOffset, i4Resistor));

    return i4Resistor;
}

#if AUXADC_BIM_MODE

bool AuxGetTSPosXY(u16* pu2Xdata, u16* pu2Ydata, bool bDraged,u32* pu4Rtouch)
{
    TS_POS tsPos;
    u16 au2Pos[TS_POS_NUM][ADC_SAMPLE_NUMBER_MAX];
    u32 u4TmpX = 0, u4TmpY = 0, index1, index2;
    u16 au2PosDelta[TS_POS_NUM][ADC_SAMPLE_NUMBER_MAX] ;
    u32 u4TmpXDelta = 0,u4TmpYDelta  =0 ,u4TmpZ1Delta  =0 ,u4TmpZ2Delta  =0 ; 
    u16 u2PAVGXDelta = 0, u2PAVGYDelta = 0, u2PAVGZ1Delta = 0, u2PAVGZ2Delta = 0;
    static u16 u2Px, u2Py;   //previous x, y
    static u16 u2Cx, u2Cy;
    u32 u4TmpZ1 = 0, u4TmpZ2 = 0;
    static u16 u2Pz1, u2Pz2;
    static volatile u16 cnt=0;
    u32 u4Val;

    static u16 u2PresampleErr = 0;
    static u16 u2CursampleErr = 0;
    static u16 u2PreDelta = 0;
    static u16 u2CurDelta = 0;
    static u16 u2CntDelta = 0;
    u32   u4ValidXSamples = 0;
    u32   u4ValidYSamples = 0;
    u32   u4ValidZ1Samples = 0;
    u32   u4ValidZ2Samples = 0;
    
    if (pu2Xdata == NULL || pu2Ydata == NULL)
        return false;
        


    //for warning when disable filter arch.
    au2PosDelta[0][0] =0;
    
    for (index1 = 0, index2 = 0; index1 < g_u2TouchSampleNum && index2 < g_u2TouchSampleNum; )
    {
        for (tsPos = TS_POS_X; tsPos < TS_POS_NUM; tsPos++)     
        {
         
          AuxGetTSPos(tsPos, &au2Pos[tsPos][index1]);
           
        }   //for (tsPos = TS_POS_X; tsPos < TS_POS_NUM; tsPos++)
        HAL_LOG(ADC_LOG_LVL_HAL, "index=%d  %d %d\r\n",index1, au2Pos[TS_POS_X][index1],au2Pos[TS_POS_Y][index1]);

        if (IsVaildACDValue(au2Pos[TS_POS_X][index1], au2Pos[TS_POS_Y][index1]) == true)
        {
            index1++;
        }
        else
        {
            index2++;
        }
    }
    if (index2 >= g_u2TouchSampleNum || ((index1==16)&&(index2==1)))//just for one condition: the 1st point is unValued;by wangwj
    {
        u2CursampleErr = 1;

    }
    else
    {
        u2CursampleErr = 0;
    }
    if ((u2PresampleErr ==1) && (1 == u2CursampleErr))
    {
        g_u2ErrSampleCnt ++;
    }
    u2PresampleErr = u2CursampleErr;
    if (index2 >= g_u2TouchSampleNum)
    {
        HAL_LOG(ADC_LOG_LVL_ERR, "Sample too much wrong points %d\r\n",g_u2ErrSampleCnt);    
        return false;
    }

    
    if (bDraged == false)
    {
        u2Px = 0;
        u2Py = 0;
        u2Cx  =0;
        u2Cy = 0;
        u2CntDelta = 0;
    }
    if (bDraged == true)
    {
        // make sure the 1st point is reliable
        if (abs(u2Px - au2Pos[TS_POS_X][0]) > VARIANCE_THRESHOLD_X 
                || abs(u2Py - au2Pos[TS_POS_Y][0]) > VARIANCE_THRESHOLD_Y)
        {
            au2Pos[TS_POS_X][0] = u2Px;
            au2Pos[TS_POS_Y][0] = u2Py;
           au2Pos[TS_POS_Z1][0] = u2Pz1;
            au2Pos[TS_POS_Z2][0] = u2Pz2;
        }
    }

    u4TmpX = 0;
    u4TmpY = 0;
    u4TmpZ1 = 0;
    u4TmpZ2 = 0;
    
    for (index1 = 0; index1 < g_u2TouchSampleNum; index1++)
    {
        u4TmpX += au2Pos[TS_POS_X][index1];
        u4TmpY += au2Pos[TS_POS_Y][index1];
        u4TmpZ1 += au2Pos[TS_POS_Z1][index1];
        u4TmpZ2 += au2Pos[TS_POS_Z2][index1];
    }
    
    HAL_LOG(ADC_LOG_LVL_HAL, "u4TmpX = %d, u4TmpY = %d\r\n", u4TmpX, u4TmpY);    

    u2Px = (u16)(u4TmpX / g_u2TouchSampleNum);
    u2Py = (u16)(u4TmpY / g_u2TouchSampleNum);

    u2Pz1 = (u16)(u4TmpZ1 / g_u2TouchSampleNum);
    u2Pz2 = (u16)(u4TmpZ2 / g_u2TouchSampleNum);
    u4TmpX = 0;
    u4TmpY = 0;
    u4TmpZ1 = 0;
    u4TmpZ2 = 0;
    u4TmpXDelta =0;
    u4TmpYDelta = 0;
    u4ValidXSamples = u4ValidYSamples = u4ValidZ1Samples = u4ValidZ2Samples = g_u2TouchSampleNum;
    for (index1 = 0; index1 < g_u2TouchSampleNum ; index1++)
    {
        au2PosDelta[TS_POS_X][index1] = abs(u2Px - au2Pos[TS_POS_X][index1]);
        au2PosDelta[TS_POS_Y][index1] = abs(u2Py - au2Pos[TS_POS_Y][index1]);

        u4TmpXDelta += au2PosDelta[TS_POS_X][index1];
        u4TmpYDelta += au2PosDelta[TS_POS_Y][index1];

    }

    u2PAVGXDelta  = (u16)(u4TmpXDelta / g_u2TouchSampleNum);
    u2PAVGYDelta  = (u16)(u4TmpYDelta / g_u2TouchSampleNum);
    u4TmpXDelta =0;
    u4TmpYDelta = 0;
    for (index1 = 0; index1 < g_u2TouchSampleNum ; index1++)
    {
        if (au2PosDelta[TS_POS_X][index1] > u2PAVGXDelta)
        {
            u4ValidXSamples --;
            au2Pos[TS_POS_X][index1] = 0;
        }
        if (au2PosDelta[TS_POS_Y][index1] > u2PAVGYDelta)
        {
            u4ValidYSamples --;
            au2Pos[TS_POS_Y][index1] = 0;
        }
        u4TmpX += au2Pos[TS_POS_X][index1];
        u4TmpY += au2Pos[TS_POS_Y][index1];
    }

    if (u4ValidXSamples != 0)
        u2Px = (u16)(u4TmpX / u4ValidXSamples);
    if (u4ValidYSamples != 0)
        u2Py = (u16)(u4TmpY / u4ValidYSamples);
    u4TmpX = 0;
    u4TmpY = 0;
    u4TmpZ1 = 0;
    u4TmpZ2 = 0;

    if (bDraged == true )
    {
        if ( abs(u2Cy-u2Py) >= 25 )
           {
            u2CurDelta = 1;
           }
        else
            u2CurDelta = 0;
        if ( (1== u2CurDelta))
        {
            u2CntDelta++;
        }
        if (u2CntDelta >= 6 && g_u2HighSampleRateFlag == 1)
        {
            g_u2ErrSampleCnt = 11;
            u2CntDelta =0;
        }
        if ( (1 != u2CurDelta))
        {
            u2CntDelta=0;
        
        }
    }
    *pu2Xdata = u2Px ;
    *pu2Ydata = u2Py;
    u2Cx = u2Px ;
    u2Cy = u2Py;
    
    // when the resistor is bigger, the touch pressure is smaller.
    // we will skip the point that the pressure is too small.
    u4Val =  TouchResistor(u2Px, u2Pz1, u2Pz2, u2Cy);
    HAL_LOG(ADC_LOG_LVL_HAL, "x = %x, Y = %x, Z1 = %x, Z2 = %x, Rtouch = %x\r\n", u2Cx, u2Cy, u2Pz1, u2Pz2, u4Val); 
    *pu4Rtouch = u4Val;
     if (g_u4TouchResistorThreshold == 0)
        g_u4TouchResistorThreshold = u4Val;   //there's a issue, if no threshlod, we choose the middle point, but it not match the other points

    //MTK71232 2012.08.02
    if(u2Pz2 == 0)
    {
        g_u2ErrSampleCnt ++;
        HAL_LOG(ADC_LOG_LVL_ERR, "[drop1]\r\n");        
        return false;
    }

    if((g_u4TouchResistorThreshold * 10 == u4Val)&&(u2Pz1 < 3600))
    {
        g_u2ErrSampleCnt ++;
        HAL_LOG(ADC_LOG_LVL_ERR, "[drop2]\r\n");        
        return false;
    }

/*Some panel have the issue which Z2<Z1 on the right edge of panel. Added by xuke*/
    if(g_u4TouchResistorThreshold * 10 == u4Val)
            return true;
    if (u4Val > g_u4TouchResistorThreshold  && g_u4TouchResistorThreshold != 0)
    {
        HAL_LOG(ADC_LOG_LVL_ERR, "g_u4TouchResistorThreshold = %d, pu4Rtouch = %d\r\n", g_u4TouchResistorThreshold, *pu4Rtouch);        
        return false;
    }

    u2PreDelta = u2CurDelta;
    return true;
}

#else
extern void * memcpy(void * dest,const void *src,unsigned s32 count);

#if AUXADC_SAMPLE_TICK
u32 GetTickCount()
{
    u32 i4Time;
    i4Time = jiffies*1000/HZ;
    return i4Time;  
}
#endif

bool AuxGetVaildAvgPos(uint8_t *pu2RawArrays, u16 *pu2X, u16 *pu2Y, u16 *pu2Z1, u16 *pu2Z2)
{
    u32 iVaildCountX = 0, iVaildCountY = 0, iVaildCountZ1 = 0, iVaildCountZ2 = 0;
    u32 isumX = 0, isumY = 0, isumZ1 = 0, isumZ2 = 0;
    u32 iavgX = 0, iavgY = 0, iavgZ1 = 0, iavgZ2 = 0;
    
    u32 isumDeltX = 0, isumDeltY = 0, isumDeltZ1 = 0, isumDeltZ2 = 0;
    u32 iavgDeltX = 0, iavgDeltY = 0, iavgDeltZ1 = 0, iavgDeltZ2 = 0;

    u16 u2Rawdatas[TS_POS_NUM][AUXADC_SAMPLE_COUNT] = {0};
    u16 u2delts[TS_POS_NUM][AUXADC_SAMPLE_COUNT]    = {0};

    u32 i = 0;

    if(pu2RawArrays==NULL || pu2X==NULL || pu2Y==NULL || pu2Z1==NULL || pu2Z2==NULL)
    {
        return false;
    }
    
    memcpy(u2Rawdatas, pu2RawArrays, AUXADC_SAMPLE_COUNT*TS_POS_NUM*sizeof(u16));

    for(i=0; i<AUXADC_SAMPLE_COUNT; i++)
    {
        isumX += u2Rawdatas[TS_POS_X][i];
        isumY += u2Rawdatas[TS_POS_Y][i];
        isumZ1 += u2Rawdatas[TS_POS_Z1][i];
        isumZ2 += u2Rawdatas[TS_POS_Z2][i];
    }

    iavgX = isumX >> 3;
    iavgY = isumY >> 3;
    iavgZ1 = isumZ1 >> 3;
    iavgZ2 = isumZ2 >> 3;

    for(i=0; i<AUXADC_SAMPLE_COUNT; i++)
    {
        
        u2delts[TS_POS_X][i] = abs(iavgX - u2Rawdatas[TS_POS_X][i]);
        u2delts[TS_POS_Y][i] = abs(iavgY - u2Rawdatas[TS_POS_Y][i]);
        u2delts[TS_POS_Z1][i] = abs(iavgZ1 - u2Rawdatas[TS_POS_Z1][i]);
        u2delts[TS_POS_Z2][i] = abs(iavgZ2 - u2Rawdatas[TS_POS_Z2][i]);
    
        isumDeltX += u2delts[TS_POS_X][i];
        isumDeltY += u2delts[TS_POS_Y][i];
        isumDeltZ1 += u2delts[TS_POS_Z1][i];
        isumDeltZ2 += u2delts[TS_POS_Z2][i];
    }

    iavgDeltX = isumDeltX >> 3;
    iavgDeltY = isumDeltY >> 3;
    iavgDeltZ1 = isumDeltZ1 >> 3;
    iavgDeltZ2 = isumDeltZ2 >> 3;


    for(i=0; i<AUXADC_SAMPLE_COUNT; i++)
    {
        if(u2delts[TS_POS_X][i] > iavgDeltX)
        {
            u2Rawdatas[TS_POS_X][i] = 0;
        }
        if(u2delts[TS_POS_Y][i] > iavgDeltY)
        {
            u2Rawdatas[TS_POS_Y][i] = 0;
        }
        if(u2delts[TS_POS_Z1][i] > iavgDeltZ1)
        {
            u2Rawdatas[TS_POS_Z1][i] = 0;
        }
        if(u2delts[TS_POS_Z2][i] > iavgDeltZ2)
        {
            u2Rawdatas[TS_POS_Z2][i] = 0;
        }  
        
        HAL_LOG(ADC_LOG_LVL_HAL, "After filter!!!!XRawdata[%d] = %d, YRawdata[%d] = %d, Z1Rawdata[%d] = %d Z2Rawdata[%d] = %d\r\n", 
                          i, u2Rawdatas[TS_POS_X][i], i, u2Rawdatas[TS_POS_Y][i], i, u2Rawdatas[TS_POS_Z1][i], i, u2Rawdatas[TS_POS_Z2][i]);
    }

    isumX = 0;
    isumY = 0;
    isumZ1 = 0;
    isumZ2 = 0;
    
    for(i=0; i<AUXADC_SAMPLE_COUNT; i++)
    {
        if(u2Rawdatas[TS_POS_X][i]!=0)
        {
            iVaildCountX++;
            isumX += u2Rawdatas[TS_POS_X][i];
        }
        if(u2Rawdatas[TS_POS_Y][i]!=0)
        {
            iVaildCountY++;
            isumY += u2Rawdatas[TS_POS_Y][i];
        }
        if(u2Rawdatas[TS_POS_Z1][i]!=0)
        {
            iVaildCountZ1++;
            isumZ1 += u2Rawdatas[TS_POS_Z1][i];
        }
        if(u2Rawdatas[TS_POS_Z2][i]!=0)
        {
            iVaildCountZ2++;
            isumZ2 += u2Rawdatas[TS_POS_Z2][i];
        }
    }

    if(iVaildCountX!=0)
    {
        *pu2X = isumX/iVaildCountX;
    }
    else
    {
        *pu2X = 0;
    }
    
    if(iVaildCountY!=0)
    {
        *pu2Y = isumY/iVaildCountY;
    }
    else
    {
        *pu2Y = 0;
    }
    
    if(iVaildCountZ1!=0)
    {
        *pu2Z1 = isumZ1/iVaildCountZ1;
    }
    else
    {
        *pu2Z1 = 0;
    }
    
    if(iVaildCountZ2!=0)
    {
        *pu2Z2 = isumZ2/iVaildCountZ2;
    }
    else
    {
        *pu2Z2 = 0;
    }

    return true;

}

bool AuxGetTSPosXY(u16* pu2Xdata, u16* pu2Ydata, bool bDraged,u32* pu4Rtouch)
{
    static u16 u2Px = 0, u2Py = 0, u2InvaildCnt   = 0;   //previous x, y
    
    u32 i     = 0; 
    u32 u4Val = 0;
    u16 u2Cx  = 0, u2Cy  = 0;
    u16 u2Cz1 = 0, u2Cz2 = 0;

    u16 u2Rawdatas[TS_POS_NUM][AUXADC_SAMPLE_COUNT] = {0};   
    
#if AUXADC_SAMPLE_TICK
    u32 endTime     = 0;
    u32 startTime = GetTickCount();
#endif
    
    if (pu2Xdata == NULL || pu2Ydata == NULL)
        return false;

    /*First Point*/
    if(!bDraged)
    {
        u2Px = 0;
        u2Py = 0;
        for(i; i<AUXADC_SAMPLE_COUNT; i++)
        {
            AUX_GET_POS(u2Rawdatas[TS_POS_X][i],  AUX_TS_CMD_ADDRESS_X);
            AUX_GET_POS(u2Rawdatas[TS_POS_Y][i],  AUX_TS_CMD_ADDRESS_Y);
            AUX_GET_POS(u2Rawdatas[TS_POS_Z1][i], AUX_TS_CMD_ADDRESS_Z1);
            AUX_GET_POS(u2Rawdatas[TS_POS_Z2][i], AUX_TS_CMD_ADDRESS_Z2);

            if(u2Rawdatas[TS_POS_Z1][i]==ADC_ECC_ERR_NUMBER1 || u2Rawdatas[TS_POS_Z1][i]==ADC_ECC_ERR_NUMBER2)
                u2Rawdatas[TS_POS_Z1][i] = 0;
                
            HAL_LOG(ADC_LOG_LVL_HAL, "First Rawdata: X[%d] = %d, Y[%d] = %d, Z1R[%d] = %d Z2[%d] = %d\r\n", 
                                  i, u2Rawdatas[TS_POS_X][i], i, u2Rawdatas[TS_POS_Y][i], i, u2Rawdatas[TS_POS_Z1][i], i, u2Rawdatas[TS_POS_Z2][i]);
        
        }

        if(!AuxGetVaildAvgPos((uint8_t*)u2Rawdatas, &u2Cx, &u2Cy, &u2Cz1, &u2Cz2))
        {
            return false;
        }
        
        if ( !IsVaildACDValue(u2Cx, u2Cy) )
        {
            HAL_LOG(ADC_LOG_LVL_HAL, "First point invaild: Not Vaild value!\r\n");
            return false;
        }

        if(u2Cz2 == 0)
        {
            HAL_LOG(ADC_LOG_LVL_HAL, "First point invaild: Z2 is 0!\r\n");
            *pu2Xdata = u2Px;
            *pu2Ydata = u2Py;
            return false;
        }
        
        HAL_LOG(ADC_LOG_LVL_HAL, "First: X = %d, Y = %d, Z1 = %d, Z2 = %d \r\n", u2Cx, u2Cy, u2Cz1, u2Cz2); 
        
        u4Val =  TouchResistor(u2Cx, u2Cz1, u2Cz2, u2Cy);
        if (u4Val > g_u4TouchResistorThreshold  && g_u4TouchResistorThreshold != 0)
        {
            HAL_LOG(ADC_LOG_LVL_ERR, "First point threshold failed!!u2Val = %d, threshold = %d\r\n", u4Val, g_u4TouchResistorThreshold);
            return false;
        }
        
        u2Px = u2Cx;
        u2Py = u2Cy;
        *pu2Xdata = u2Cx;
        *pu2Ydata = u2Cy;                 
        return true;
    }
    
    /*Drag point*/
    for(i=0; i<AUXADC_SAMPLE_COUNT; i++)
    {
        u2Rawdatas[TS_POS_X][i] = AUX_GET_X_RAWDATA((AUXADC_TS_AUTO_X_DATn + i*4));
        u2Rawdatas[TS_POS_Y][i] = AUX_GET_Y_RAWDATA((AUXADC_TS_AUTO_Y_DATn + i*4));
        u2Rawdatas[TS_POS_Z1][i] = AUX_GET_Z1_RAWDATA((AUXADC_TS_AUTO_Z1_DATn + i*4));
        u2Rawdatas[TS_POS_Z2][i] = AUX_GET_Z2_RAWDATA((AUXADC_TS_AUTO_Z2_DATn + i*4));
        
        HAL_LOG(ADC_LOG_LVL_HAL, "Drag Rawdata: X[%d] = %d, Y[%d] = %d, Z1[%d] = %d Z2[%d] = %d\r\n", 
                                  i, u2Rawdatas[TS_POS_X][i], i, u2Rawdatas[TS_POS_Y][i], i, u2Rawdatas[TS_POS_Z1][i], i, u2Rawdatas[TS_POS_Z2][i]);

        if(u2Rawdatas[TS_POS_Z1][i]>=u2Rawdatas[TS_POS_Z2][i])
            u2Rawdatas[TS_POS_Z1][i] = 0;
            
        if(u2Rawdatas[TS_POS_Z1][i]==ADC_ECC_ERR_NUMBER1 || u2Rawdatas[TS_POS_Z1][i]==ADC_ECC_ERR_NUMBER2)
            u2Rawdatas[TS_POS_Z1][i] = 0;
        
        if(u2Rawdatas[TS_POS_Z2][i]==0)
        {
            u2Rawdatas[TS_POS_X][i] = 0;
            u2Rawdatas[TS_POS_Y][i] = 0;
            u2Rawdatas[TS_POS_Z1][i]  = 0;
            u2Rawdatas[TS_POS_Z2][i]  = 0;
        }       
    }
        
    if(!AuxGetVaildAvgPos((uint8_t*)u2Rawdatas, &u2Cx, &u2Cy, &u2Cz1, &u2Cz2))
    {
        *pu2Xdata = u2Px;
        *pu2Ydata = u2Py;
        return false;
    }

    if(u2Cz2 == 0)
    {
        *pu2Xdata = u2Px;
        *pu2Ydata = u2Py;
        return true;
    }

    if ( !IsVaildACDValue(u2Cx, u2Cy) )
    {
        u2InvaildCnt++;
    }
    else
    {
        u2InvaildCnt = 0;
    }

    /*Pressed status don't debouce issue.*/
    if(u2InvaildCnt == 10)
    {
        u2InvaildCnt = 0;
        g_u2ErrSampleCnt++;
    }//

    /*Point drift too much*/
    if((abs(u2Cx-u2Px)>220) || (abs(u2Cy-u2Py)>220))
    {
        u2Cx = u2Px;
        u2Cy = u2Py;
    }
    else
    {
        u2Px = u2Cx;
        u2Py = u2Cy;
    }
    

    u4Val =  TouchResistor(u2Px, u2Cz1, u2Cz2, u2Cy);
    *pu4Rtouch = u4Val;
    
    if((u2Cz2 == 0) || (u2Cz2 <= u2Cz1))
    {
        HAL_LOG(ADC_LOG_LVL_ERR, "Drag failed!11\r\n");
        g_u2ErrSampleCnt ++;
        return false;
    }

    if((g_u4TouchResistorThreshold * 10 == u4Val))
    {
        HAL_LOG(ADC_LOG_LVL_ERR, "Drag failed22!\r\n");
        g_u2ErrSampleCnt ++;
        return false;
    }

    if(g_u4TouchResistorThreshold * 10 == u4Val)
        return true;
    if (u4Val > g_u4TouchResistorThreshold  && g_u4TouchResistorThreshold != 0)
    {
        HAL_LOG(ADC_LOG_LVL_ERR, "Drag failed33!\r\n");
        return false;
    }

    HAL_LOG(ADC_LOG_LVL_HAL, "Drag: X = %d, Y = %d, Z1 = %d, Z2 = %d, Rtouch = %d Threshold = %d\r\n", u2Cx, u2Cy, u2Cz1, u2Cz2, u4Val, g_u4TouchResistorThreshold);

    *pu2Xdata = u2Px;
    *pu2Ydata = u2Py;   
    
#if AUXADC_SAMPLE_TICK
    endTime = GetTickCount();
    RETAILMSG(1, (TEXT("sample Time = %d ms \r\n"), (endTime-startTime)));
#endif
    return true;
}

#endif

bool AuxEnablePenIrq(void)
{
    u16 u2Con = 0;


    // 1. Write the sample command
    u2Con = AUX_TS_CMD_PD_YDRV_SH;
    AuxsetPowerDownMode(u2Con);

    // 2. Trigger sample process
    //AuxSetTSCon(pRegAddress, AUX_TS_CON_SPL_TRIGGER);
    AuxsetTrigger();
    return true;
}

bool AuxDisablePenIrq(void)
{
    u16 u2Con;

    // 1. Write the sample command
    u2Con = AUX_TS_CMD_PD_IRQ_SH;
    AuxsetPowerDownMode( u2Con);

    // 2. Trigger sample process
    //AuxSetTSCon(pRegAddress, AUX_TS_CON_SPL_TRIGGER);
    AuxsetTrigger();
    return true;
}

bool AuxADCInit()
{
    u32 tmp = 0;

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
    
    //bug fixed,wangwj added to reponse touch irq
    tmp = 0x18;
    ADC_WRITE32(AUXADC_ECC, tmp);   
        // ECC = 0X18  and enbale ECC, fix the bug that loss 
        // the point (1980-2048) when sketch the Y coordnation.  added by XK.
    
    tmp = ADC_READ32(AUXADC_PDN_CON);
    ADC_WRITE32(AUXADC_PDN_CON, 0);
    
    tmp = ADC_READ32(AUXADC_MISC);
    tmp |= 0x200;
    ADC_WRITE32(AUXADC_MISC, tmp);
    tmp = ADC_READ32(AUXADC_MISC);

//fix arm2 conflict with arm11 auxadc issue
    tmp = IO_READ32(IO_BASE_VA, 0X38024);
    tmp |= 0x000000ff;
    IO_WRITE32(IO_BASE_VA, 0X38024, tmp);

#if AUXADC_BIM_MODE
    HAL_LOG(ADC_LOG_LVL_HAL, "BIM Mode\r\n");
    tmp = ADC_READ32(AUXADC_TS_CON0);
    ADC_WRITE32(AUXADC_TS_DEBT0, 8);
    ADC_WRITE32(AUXADC_TS_DEBT1, 800);
        
    tmp = ADC_READ32(AUXADC_TS_ADDR);
        
    AuxsetSPLDurationOn(1);
    AuxsetSampleBit(AUX_TS_CMD_12BIT_RES); 
    AuxsetSampleMode(AUX_TS_CMD_MODE_DF);
#else
    HAL_LOG(ADC_LOG_LVL_HAL, "FAV Mode\r\n");
    AuxSetTSDebt(8); 
    AuxSetTSDebt1(800);
    AuxsetSampleBit(AUX_TS_CMD_12BIT_RES); 
    AuxsetSampleMode(AUX_TS_CMD_MODE_DF);
    
    AuxsetSPLDurationOn(1);
    AuxsetNonStop(1);
    AuxsetAutoInterval(0xa0);
    AuxsetFAVLatency(0xf);
    AuxsetFAVAccCount(0x2);     
    //FAV sample mode
    AuxsetFAVCoord(COORD_ALL);
    //add two sample
    AuxsetFAVAddTwosample(1);
#endif
    
    HAL_LOG(ADC_LOG_LVL_HAL, "Auxadc end!\r\n");

    return true;

}

void AuxADCCloseClock()
{

    u32 tmp = 0;
    
    tmp = IO_READ32(IO_BASE_VA, 0XB0);
    tmp &= 0XFFFFFFF1;
    IO_WRITE32(IO_BASE_VA, 0XB0, tmp);
}

#if TOUCH_RAW_DATA_MODE
bool AuxADC_Open_Batchtimer(void)
{
    if(0 == AUXADC_BATCH_TIMER_FLAG)
    {
        AuxsetInvalidFlag(1);
        AuxsetRAWTrigger(1);
        AUXADC_BATCH_TIMER_FLAG = 1;
        //HAL_LOG(ADC_LOG_LVL_HAL, "Open Batchtimer!\r\n");
    }

    return true;
}

bool AuxADC_Close_Batchtimer(void)
{
    if(1 == AUXADC_BATCH_TIMER_FLAG)
    {
        AuxsetRAWTrigger(0);
        AUXADC_BATCH_TIMER_FLAG = 0;
        //HAL_LOG(ADC_LOG_LVL_HAL, "Close Batchtimer!\r\n");
    }

    return true;
}

#else
bool AuxADC_Open_Batchtimer(void)
{
    if(0 == AUXADC_BATCH_TIMER_FLAG)
    {
        AuxsetInvalidFlag(1);
        AuxsetFAVTrigger(1);
        AUXADC_BATCH_TIMER_FLAG = 1;
        HAL_LOG(ADC_LOG_LVL_HAL, "Open Batchtimer!\r\n");
    }

    return true;
}

bool AuxADC_Close_Batchtimer(void)
{
    if(1 == AUXADC_BATCH_TIMER_FLAG)
    {
        AuxsetFAVTrigger(0);
        AUXADC_BATCH_TIMER_FLAG = 0;
        HAL_LOG(ADC_LOG_LVL_HAL, "Close Batchtimer!\r\n");
    }

    return true;
}
#endif

//-----------------------------------------------------------------------------

