#include <config.h>
#include <command.h>
#include <asm/io.h>
#include <common.h>

#define I2C_BASE_VA      0xF0030000
#define IO_BASE_VA       0xF0000000


#define UP_CFG                 ((unsigned int)0x188)
#define FAST_CK_EN             ((unsigned int)0x1<<20)

#define PAD_PINMUX0            ((unsigned int)0x54)   
   #define SIF0_SEL_SCL0_SDA0    ((unsigned int)0x01<<3)     
   #define SIF0_SEL_SCL1_SDA1    ((unsigned int)0x02<<3)
#define PAD_PINMUX6              ((unsigned int)0x6C)     
   #define SIF1_SEL_SCL1_SDA1    ((unsigned int)0x01<<26)     
   #define SIF1_SEL_SCL0_SDA0    ((unsigned int)0x02<<27)   
#define SIF_SEL                  (0x94)     
   #define SIF_SEL_M0M1          (0x00<<4)     
   #define SIF_SEL_S0M1          (0x01<<4)     
   #define SIF_SEL_M0S1          (0x02<<4)     
   #define SIF_SEL_S0S1          (0x03<<4)
#define SIF_CLOCK                ((0xA8))     
   #define SIFM0_CLOCK           ((unsigned int)(unsigned int)0x1<<28)     
   #define SIFM1_CLOCK           ((unsigned int)(unsigned int)0x1<<29)     
   #define SIFS0_CLOCK           ((unsigned int)(unsigned int)0x1<<30)     
   #define SIFS1_CLOCK           ((unsigned int)(unsigned int)0x1<<31)     
#define SIF_RESET                ((0xC4))    
   #define SIFM0_RESET           ((unsigned int)(unsigned int)0x1<<28)    
   #define SIFM1_RESET           ((unsigned int)(unsigned int)0x1<<29)    
   #define SIFS0_RESET           ((unsigned int)(unsigned int)0x1<<30)    
   #define SIFS1_RESET           ((unsigned int)(unsigned int)0x1<<31)

//***
#define SIFM_INTCLR          ((unsigned int)0x410)
#define SIFM_INTEN           ((unsigned int)0x414)
#define SIFM_INTSTA          ((unsigned int)0x418)

#define SIFM1_INTCLR          ((unsigned int)0x810)
#define SIFM1_INTEN           ((unsigned int)0x814)
#define SIFM1_INTSTA          ((unsigned int)0x818)

//*****
//*****master0
#define SIF_SIFM0CTL0        ((unsigned int)0x400)  
#define SIF_SIFM1CTL0        ((unsigned int)0x800)
  #define SIFM_ODRAIN          ((unsigned int)(unsigned int)0x1<<31)
  #define SIFM_CLK_DIV_OFFSET  ((unsigned int)16)
  #define SIFM_CLK_DIV_MASK    ((unsigned int)0xFFF<<16)
  #define SIFM_PSEL            ((unsigned int)0x1<<5)
  #define SIFM_CS_STATUS       ((unsigned int)0x1<<4)
  #define SIFM_SCL_STATE       ((unsigned int)0x1<<3)
  #define SIFM_SDA_STATE       ((unsigned int)0x1<<2)
  #define SIFM_SM0EN           ((unsigned int)0x1<<1)
  #define SIFM_SCL_STRECH      ((unsigned int)0x1<<0)

#define SIF_SIFM0CTL1        ((unsigned int)0x404)
#define SIF_SIFM1CTL1        ((unsigned int)0x804)
  #define SIFM_ACK_OFFSET      ((unsigned int)16)
  #define SIFM_ACK_MASK        ((unsigned int)0xFF<<16)
  #define SIFM_PGLEN_OFFSET    ((unsigned int)8)
  #define SIFM_PGLEN_MASK      ((unsigned int)0x7<<8)
  #define SIFM_SIF_MODE_OFFSET ((unsigned int)4)
  #define SIFM_SIF_MODE_MASK   ((unsigned int)0x7<<4)
  #define SIFM_START            ((unsigned int)0x1)
  #define SIFM_WRITE_DATA       ((unsigned int)0x2)
  #define SIFM_STOP             ((unsigned int)0x3)
  #define SIFM_READ_DATA_NO_ACK ((unsigned int)0x4)
  #define SIFM_READ_DATA_ACK    ((unsigned int)0x5)
  #define SIFM_TRI              ((unsigned int)0x1<<0)
  #define SIFM_BUSY             ((unsigned int)0x1<<0)

#define SIF_SIFM0D0          ((unsigned int)0x408)
#define SIF_SIFM1D0          ((unsigned int)0x808)
  #define SIFM_DATA3_OFFSET    ((unsigned int)24)
  #define SIFM_DATA3_MASK      ((unsigned int)0xFF<<24)
  #define SIFM_DATA2_OFFSET    ((unsigned int)16)
  #define SIFM_DATA2_MASK      ((unsigned int)0xFF<<16)
  #define SIFM_DATA1_OFFSET    ((unsigned int)8)
  #define SIFM_DATA1_MASK      ((unsigned int)0xFF<<8)
  #define SIFM_DATA0_OFFSET    ((unsigned int)0)
  #define SIFM_DATA0_MASK      ((unsigned int)0xFF<<0)
  
#define SIF_SIFM0D1           ((unsigned int)0x40C)  
#define SIF_SIFM1D1           ((unsigned int)0x80C)  
  #define SIFM_DATA7_OFFSET   ((unsigned int)24)
  #define SIFM_DATA7_MASK     ((unsigned int)0xFF<<24)
  #define SIFM_DATA6_OFFSET   ((unsigned int)16)
  #define SIFM_DATA6_MASK     ((unsigned int)0xFF<<16)
  #define SIFM_DATA5_OFFSET   ((unsigned int)8)
  #define SIFM_DATA5_MASK     ((unsigned int)0xFF<<8)
  #define SIFM_DATA4_OFFSET   ((unsigned int)0)
  #define SIFM_DATA4_MASK     ((unsigned int)0xFF<<0)
 //end master0
 /******************************************************************************
* Local macro
******************************************************************************/
#define HAL_WRITE32(_reg_, _val_)           (*((volatile uint32_t*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)                   (*((volatile uint32_t*)(_reg_)))

#define SIF_READ32(u4Addr)                  HAL_READ32(I2C_BASE_VA + u4Addr) 
#define SIF_WRITE32(u4Addr, u4Val)          HAL_WRITE32(I2C_BASE_VA + u4Addr,  u4Val)

#define SIF_IO_READ32(u4Addr)               HAL_READ32(IO_BASE_VA + u4Addr)
#define SIF_IO_WRITE32(u4Addr, u4Val)       HAL_WRITE32(IO_BASE_VA + u4Addr, u4Val)

#define SIF_SET_BIT(u4Addr, u4Val)          SIF_WRITE32((u4Addr), (SIF_READ32(u4Addr) | (u4Val)))
#define SIF_CLR_BIT(u4Addr, u4Val)          SIF_WRITE32((u4Addr), (SIF_READ32(u4Addr) & (~(u4Val))))

#define SIF_IO_SET_BIT(u4Addr, u4Val)       SIF_IO_WRITE32((u4Addr), (SIF_READ32(u4Addr) | (u4Val)))
#define SIF_IO_CLR_BIT(u4Addr, u4Val)       SIF_IO_WRITE32((u4Addr), (SIF_READ32(u4Addr) & (~(u4Val))))

#define IS_SIF_BIT(u4Addr, u4Val)          ((SIF_READ32(u4Addr) & (u4Val)) == (u4Val)) 

#define SIF_WRITE_MASK(u4Addr, u4Mask, u4Offet, u4Val)  SIF_WRITE32(u4Addr, ((SIF_READ32(u4Addr) & (~(u4Mask))) | (((u4Val) << (u4Offet)) & (u4Mask))))
#define SIF_READ_MASK(u4Addr, u4Mask, u4Offet)          ((SIF_READ32(u4Addr) & (u4Mask)) >> (u4Offet))

#define SIFM_DATA0_READ()   SIF_READ_MASK(SIF_SIFM0D0, SIFM_DATA0_MASK, SIFM_DATA0_OFFSET)  
#define SIFM_DATA1_READ()   SIF_READ_MASK(SIF_SIFM0D0, SIFM_DATA1_MASK, SIFM_DATA1_OFFSET)  
#define SIFM_DATA2_READ()   SIF_READ_MASK(SIF_SIFM0D0, SIFM_DATA2_MASK, SIFM_DATA2_OFFSET)  
#define SIFM_DATA3_READ()   SIF_READ_MASK(SIF_SIFM0D0, SIFM_DATA3_MASK, SIFM_DATA3_OFFSET)  
#define SIFM_DATA4_READ()   SIF_READ_MASK(SIF_SIFM0D1, SIFM_DATA4_MASK, SIFM_DATA4_OFFSET)  
#define SIFM_DATA5_READ()   SIF_READ_MASK(SIF_SIFM0D1, SIFM_DATA5_MASK, SIFM_DATA5_OFFSET)  
#define SIFM_DATA6_READ()   SIF_READ_MASK(SIF_SIFM0D1, SIFM_DATA6_MASK, SIFM_DATA6_OFFSET)  
#define SIFM_DATA7_READ()   SIF_READ_MASK(SIF_SIFM0D1, SIFM_DATA7_MASK, SIFM_DATA7_OFFSET)  

#define SIFM1_DATA0_READ()   SIF_READ_MASK(SIF_SIFM1D0, SIFM_DATA0_MASK, SIFM_DATA0_OFFSET)  
#define SIFM1_DATA1_READ()   SIF_READ_MASK(SIF_SIFM1D0, SIFM_DATA1_MASK, SIFM_DATA1_OFFSET)  
#define SIFM1_DATA2_READ()   SIF_READ_MASK(SIF_SIFM1D0, SIFM_DATA2_MASK, SIFM_DATA2_OFFSET)  
#define SIFM1_DATA3_READ()   SIF_READ_MASK(SIF_SIFM1D0, SIFM_DATA3_MASK, SIFM_DATA3_OFFSET)  
#define SIFM1_DATA4_READ()   SIF_READ_MASK(SIF_SIFM1D1, SIFM_DATA4_MASK, SIFM_DATA4_OFFSET)  
#define SIFM1_DATA5_READ()   SIF_READ_MASK(SIF_SIFM1D1, SIFM_DATA5_MASK, SIFM_DATA5_OFFSET)  
#define SIFM1_DATA6_READ()   SIF_READ_MASK(SIF_SIFM1D1, SIFM_DATA6_MASK, SIFM_DATA6_OFFSET)  
#define SIFM1_DATA7_READ()   SIF_READ_MASK(SIF_SIFM1D1, SIFM_DATA7_MASK, SIFM_DATA7_OFFSET)  


#define SIFM_DATA0_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM0D0, SIFM_DATA0_MASK, SIFM_DATA0_OFFSET, u4Val)  
#define SIFM_DATA1_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM0D0, SIFM_DATA1_MASK, SIFM_DATA1_OFFSET, u4Val)  
#define SIFM_DATA2_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM0D0, SIFM_DATA2_MASK, SIFM_DATA2_OFFSET, u4Val)  
#define SIFM_DATA3_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM0D0, SIFM_DATA3_MASK, SIFM_DATA3_OFFSET, u4Val)  
#define SIFM_DATA4_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM0D1, SIFM_DATA4_MASK, SIFM_DATA4_OFFSET, u4Val)  
#define SIFM_DATA5_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM0D1, SIFM_DATA5_MASK, SIFM_DATA5_OFFSET, u4Val)  
#define SIFM_DATA6_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM0D1, SIFM_DATA6_MASK, SIFM_DATA6_OFFSET, u4Val)  
#define SIFM_DATA7_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM0D1, SIFM_DATA7_MASK, SIFM_DATA7_OFFSET, u4Val) 

#define SIFM1_DATA0_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM1D0, SIFM_DATA0_MASK, SIFM_DATA0_OFFSET, u4Val)  
#define SIFM1_DATA1_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM1D0, SIFM_DATA1_MASK, SIFM_DATA1_OFFSET, u4Val)  
#define SIFM1_DATA2_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM1D0, SIFM_DATA2_MASK, SIFM_DATA2_OFFSET, u4Val)  
#define SIFM1_DATA3_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM1D0, SIFM_DATA3_MASK, SIFM_DATA3_OFFSET, u4Val)  
#define SIFM1_DATA4_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM1D1, SIFM_DATA4_MASK, SIFM_DATA4_OFFSET, u4Val)  
#define SIFM1_DATA5_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM1D1, SIFM_DATA5_MASK, SIFM_DATA5_OFFSET, u4Val)  
#define SIFM1_DATA6_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM1D1, SIFM_DATA6_MASK, SIFM_DATA6_OFFSET, u4Val)  
#define SIFM1_DATA7_WRITE(u4Val)   SIF_WRITE_MASK(SIF_SIFM1D1, SIFM_DATA7_MASK, SIFM_DATA7_OFFSET, u4Val) 


#define SIFM_CLK_DIV_READ()        SIF_READ_MASK(SIF_SIFM0CTL0, SIFM_CLK_DIV_MASK, SIFM_CLK_DIV_OFFSET)
#define SIFM_CLK_DIV_WRITE(u4Val)  SIF_WRITE_MASK(SIF_SIFM0CTL0, SIFM_CLK_DIV_MASK, SIFM_CLK_DIV_OFFSET, u4Val)

#define SIFM1_CLK_DIV_READ()        SIF_READ_MASK(SIF_SIFM1CTL0, SIFM_CLK_DIV_MASK, SIFM_CLK_DIV_OFFSET)
#define SIFM1_CLK_DIV_WRITE(u4Val)  SIF_WRITE_MASK(SIF_SIFM1CTL0, SIFM_CLK_DIV_MASK, SIFM_CLK_DIV_OFFSET, u4Val)

#define SIFM_ACK_READ()             SIF_READ_MASK(SIF_SIFM0CTL1, SIFM_ACK_MASK, SIFM_ACK_OFFSET)
#define SIFM1_ACK_READ()            SIF_READ_MASK(SIF_SIFM1CTL1, SIFM_ACK_MASK, SIFM_ACK_OFFSET)

#define SIFM_PGLEN_READ()           SIF_READ_MASK(SIF_SIFM0CTL1, SIFM_PGLEN_MASK, SIFM_PGLEN_OFFSET)
#define SIFM_PGLEN_WRITE(u4Val)     SIF_WRITE_MASK(SIF_SIFM0CTL1, SIFM_PGLEN_MASK, SIFM_PGLEN_OFFSET, u4Val)
#define SIFM1_PGLEN_READ()          SIF_READ_MASK(SIF_SIFM1CTL1, SIFM_PGLEN_MASK, SIFM_PGLEN_OFFSET)
#define SIFM1_PGLEN_WRITE(u4Val)    SIF_WRITE_MASK(SIF_SIFM1CTL1, SIFM_PGLEN_MASK, SIFM_PGLEN_OFFSET, u4Val)

#define SIFM_SIF_MODE_READ()           SIF_READ_MASK(SIF_SIFM0CTL1, SIFM_SIF_MODE_MASK, SIFM_SIF_MODE_OFFSET)
#define SIFM_SIF_MODE_WRITE(u4Val)     SIF_WRITE_MASK(SIF_SIFM0CTL1, SIFM_SIF_MODE_MASK, SIFM_SIF_MODE_OFFSET, u4Val)
#define SIFM1_SIF_MODE_READ()          SIF_READ_MASK(SIF_SIFM1CTL1, SIFM_SIF_MODE_MASK, SIFM_SIF_MODE_OFFSET)
#define SIFM1_SIF_MODE_WRITE(u4Val)    SIF_WRITE_MASK(SIF_SIFM1CTL1, SIFM_SIF_MODE_MASK, SIFM_SIF_MODE_OFFSET, u4Val)

#define CLK_SRC_IS_27M()   ((SIF_READ32(UP_CFG) & FAST_CK_EN) == FAST_CK_EN)

//////////////////////////////////////////////
//////////////////////////////////////////////
static unsigned int i2c_bus_num __attribute__ ((section (".data"))) = 0;
#if defined(CONFIG_I2C_MUX)
static unsigned int i2c_bus_num_mux __attribute__ ((section ("data"))) = 0;
#endif
#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})

int i2c_set_bus_num(unsigned int bus)
{
    printf("set bus num\n");
    if(bus<2)
    {
        i2c_bus_num = bus;
        printf("i2c_bus_num:%d\n",i2c_bus_num);
        return 0;
    }
    else 
    {
      i2c_bus_num = 0;
      return -1;
    }
}
    
unsigned int i2c_get_bus_num(void)
{
    return i2c_bus_num;
}


/* timeout waiting for the controller to respond */
#define AC83XX_I2C_TIMEOUT (msecs_to_jiffies(1000))
/******************************************************************************
* Local variable
******************************************************************************/
int  _SifMIsrInitiated = 0;

/*========================================================================*/

int SIFM_TrigMode(u32 u4Mode)
{
    SIFM_SIF_MODE_WRITE(u4Mode);
    SIF_SET_BIT(SIF_SIFM0CTL1,SIFM_TRI);
    while(IS_SIF_BIT(SIF_SIFM0CTL1,SIFM_TRI));
       // usleep(1);

    return (0);
}
int SIFM1_TrigMode(u32 u4Mode)
{
    SIFM1_SIF_MODE_WRITE(u4Mode);
    SIF_SET_BIT(SIF_SIFM1CTL1,SIFM_TRI);
    while(IS_SIF_BIT(SIF_SIFM1CTL1,SIFM_TRI));
       // usleep(1);

    return (0);
}

/////////


void i2c_init(int speed)//27M/speed=khz,default=400KHz
{
    u32 u4Tmp = 0;
    _SifMIsrInitiated = 0;
    if (_SifMIsrInitiated == 0)
    {
        /* select master0 & master1*/
         u4Tmp = SIF_IO_READ32(SIF_SEL);            
         u4Tmp |= SIF_SEL_M0M1;         
         SIF_IO_WRITE32(SIF_SEL,u4Tmp); 
         
        /* master0 set pinmux and clock*/                       
         u4Tmp = SIF_IO_READ32(PAD_PINMUX0);        
         u4Tmp |= SIF0_SEL_SCL0_SDA0;       
         SIF_IO_WRITE32(PAD_PINMUX0,u4Tmp);     
                                
         u4Tmp = SIF_IO_READ32(SIF_CLOCK);                  
         u4Tmp |= SIFM0_CLOCK;          
         SIF_IO_WRITE32(SIF_CLOCK,u4Tmp);       
                                    
         u4Tmp = SIF_IO_READ32(SIF_RESET);                  
         u4Tmp |= SIFM0_RESET;          
         SIF_IO_WRITE32(SIF_RESET,u4Tmp);
         
        /* master1 set pinmux and clock*/   
         u4Tmp = SIF_IO_READ32(PAD_PINMUX6);        
         u4Tmp |= SIF1_SEL_SCL1_SDA1;       
         SIF_IO_WRITE32(PAD_PINMUX6,u4Tmp); 

         u4Tmp = SIF_IO_READ32(SIF_CLOCK);                  
         u4Tmp |= SIFM1_CLOCK;          
         SIF_IO_WRITE32(SIF_CLOCK,u4Tmp);       
                                    
         u4Tmp = SIF_IO_READ32(SIF_RESET);                  
         u4Tmp |= SIFM1_RESET;          
         SIF_IO_WRITE32(SIF_RESET,u4Tmp);
       
         SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_SM0EN);      //enable sif master0
         SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_ODRAIN);     //output pull-high
         SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_SCL_STATE);  //init SCL line value
         SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_SDA_STATE);  //init SDA line value
         SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_SCL_STRECH);  //init SDA line STECH
  
         SIF_SET_BIT(SIF_SIFM1CTL0, SIFM_SM0EN);      //enable sif master1
         SIF_SET_BIT(SIF_SIFM1CTL0, SIFM_ODRAIN);     //output pull-high
         SIF_SET_BIT(SIF_SIFM1CTL0, SIFM_SCL_STATE);  //init SCL line value
         SIF_SET_BIT(SIF_SIFM1CTL0, SIFM_SDA_STATE);  //init SDA line value

        
         
         SIF_CLR_BIT(SIFM_INTEN, 1);
         SIF_SET_BIT(SIFM_INTCLR, 1);

         SIF_CLR_BIT(SIFM1_INTEN, 1);
         SIF_SET_BIT(SIFM1_INTCLR, 1);
         
         //set cloclk speed,default is 400k
         SIFM_CLK_DIV_WRITE(68);
         SIFM1_CLK_DIV_WRITE(68);
         // init local variable
         _SifMIsrInitiated = 1;

    }

    return (0);
}

/*
 * Waiting on Bus Busy
 */
int ac83xx_i2c_wait_for_bb(int bus_num)
{
    int timeout = 100;
    //timeout = jiffies + AC83XX_I2C_TIMEOUT;
    if( bus_num == 0)
    {
       while (SIF_READ32(SIF_SIFM0CTL1) & SIFM_BUSY)
       {
            printf("wait on bus busy\n");
            if(timeout>0)
            {
                udelay(100);
                timeout--;
            }
            if(timeout<0)
            {
                printf( "[I2C]timeout waiting for bus ready\n");
                return -1;
            }
            msleep(1);
       } 
    }
    if( bus_num == 1)
    {
       while (SIF_READ32(SIF_SIFM1CTL1) & SIFM_BUSY)
       {
            if(timeout>0)
            {
                udelay(100);
                timeout--;
            }
            if(timeout<0)
            {
                printf( "[I2C]timeout waiting for bus ready\n");
                return -1;
            }
            msleep(1);
       } 
    }
    return 0;
}

int SifMRead(u32 ucDev, u8 *pucValue, u32 u4Count, u32 NoRDAck)
{
    u32 u4Ack, ucReadCount, ucIdx, ucAckCount, ucAckFinal, ucTmpCount;
    if ((pucValue == NULL) ||   (u4Count == 0))
    {
        printf("Data is not right\n");
        return -1;
    }

    ucIdx = 0; 

    SIFM_DATA0_WRITE(((ucDev<<1) + 1));
    SIFM_PGLEN_WRITE(0x00);
    SIFM_TrigMode(SIFM_WRITE_DATA);
    u4Ack = SIFM_ACK_READ();
    if(u4Ack != 0x1)
    {
        printf("MASTER0 READ ACK FAILURE\n");
        return -1;
    }

    ucAckCount = (u4Count-1)/8;
    ucAckFinal = 0;
    while (u4Count > 0)
    {

        if(ucAckCount > 0)
        {
            ucReadCount = 8;
            ucAckFinal = 0;
            ucAckCount --;
        }
        else
        {
            ucReadCount = u4Count;
            ucAckFinal = 1;
        }

        SIFM_PGLEN_WRITE((ucReadCount - 1));
        if(NoRDAck)
            SIFM_TrigMode(SIFM_READ_DATA_NO_ACK);
        else
        {
            SIFM_TrigMode((ucAckFinal == 1)? SIFM_READ_DATA_NO_ACK: SIFM_READ_DATA_ACK);

            u4Ack = SIFM_ACK_READ();
            for(ucTmpCount = 0; ((u4Ack & (1 << ucTmpCount)) != 0) && (ucTmpCount < 8); ucTmpCount++){}

            if(((ucAckFinal == 1) && ((ucTmpCount) != (ucReadCount-1)))||((ucAckFinal == 0) && (ucTmpCount != ucReadCount)))
            {
                break;
            }
        }
        
        switch(ucReadCount)
        {
            case 8:
                pucValue[ucIdx + 7] = SIFM_DATA7_READ();
            case 7:
                pucValue[ucIdx + 6] = SIFM_DATA6_READ();
            case 6:
                pucValue[ucIdx + 5] = SIFM_DATA5_READ();
            case 5:
                pucValue[ucIdx + 4] = SIFM_DATA4_READ();
            case 4:
                pucValue[ucIdx + 3] = SIFM_DATA3_READ();
            case 3:
                pucValue[ucIdx + 2] = SIFM_DATA2_READ();
            case 2:
                pucValue[ucIdx + 1] = SIFM_DATA1_READ();
            case 1:
                pucValue[ucIdx + 0] = SIFM_DATA0_READ();
            default:
                break;
        }

        u4Count -= ucReadCount;
        ucIdx += ucReadCount;
    }

    //printf("MASTER0 READ OKAY\n");
    return 0;
}

int SifM1Read(u32 ucDev,  u8 *pucValue, u32 u4Count, u32 NoRDAck)
{
    u32 u4Ack, ucReadCount, ucIdx, ucAckCount, ucAckFinal, ucTmpCount;
    u32 value;
    if ((pucValue == NULL) ||   (u4Count == 0))
    {
        printf("Data is not right\n");
        return -1;
    }

    ucIdx = 0; 

    SIFM1_DATA0_WRITE(((ucDev<<1) + 1));
    SIFM1_PGLEN_WRITE(0x00);
    SIFM1_TrigMode(SIFM_WRITE_DATA);
    u4Ack = SIFM1_ACK_READ();
    if(u4Ack != 0x1)
    {
        printf("MASTER1 READ ACK FAILURE\n");
        return -1;
    }

    ucAckCount = (u4Count-1)/8;
    ucAckFinal = 0;
    while (u4Count > 0)
    {

        if(ucAckCount > 0)
        {
            ucReadCount = 8;
            ucAckFinal = 0;
            ucAckCount --;
        }
        else
        {
            ucReadCount = u4Count;
            ucAckFinal = 1;
        }

        SIFM1_PGLEN_WRITE((ucReadCount - 1));
        if(NoRDAck)
            SIFM1_TrigMode(SIFM_READ_DATA_NO_ACK);
        else
        {
            SIFM1_TrigMode((ucAckFinal == 1)? SIFM_READ_DATA_NO_ACK: SIFM_READ_DATA_ACK);

            u4Ack = SIFM1_ACK_READ();
            for(ucTmpCount = 0; ((u4Ack & (1 << ucTmpCount)) != 0) && (ucTmpCount < 8); ucTmpCount++){}

            if(((ucAckFinal == 1) && ((ucTmpCount) != (ucReadCount-1)))||((ucAckFinal == 0) && (ucTmpCount != ucReadCount)))
            {
                break;
            }
        }
        
        switch(ucReadCount)
        {
            case 8:
                pucValue[ucIdx + 7] = SIFM1_DATA7_READ();
            case 7:
                pucValue[ucIdx + 6] = SIFM1_DATA6_READ();
            case 6:
                pucValue[ucIdx + 5] = SIFM1_DATA5_READ();
            case 5:
                pucValue[ucIdx + 4] = SIFM1_DATA4_READ();
            case 4:
                pucValue[ucIdx + 3] = SIFM1_DATA3_READ();
            case 3:
                pucValue[ucIdx + 2] = SIFM1_DATA2_READ();
            case 2:
                pucValue[ucIdx + 1] = SIFM1_DATA1_READ();
            case 1:
                pucValue[ucIdx + 0] = SIFM1_DATA0_READ();
            default:
                break;
        }

        u4Count -= ucReadCount;
        ucIdx += ucReadCount;
    }

    //printf("MASTER1 READ OKAY\n");
    return 0;
}

///////////////////////
///////////////////////
int SifMWrite(u32 ucDev, const u8 *pucValue, u32 u4Count)
{
    
    u32 u4Ack, ucWriteCount, ucIdx, ucTmpCount;
    u32 value;
    ucIdx = 0; 
    int i;
    int ret = 0;
    if ((pucValue == NULL) ||(u4Count == 0))
    {
        printf("pucValue||u4Count = NULL\n");
        return -1;
        
    }


    while (u4Count > 0)
    {
        ucWriteCount = (u4Count > 8) ? 8 : (u4Count);

        switch(ucWriteCount)
        {
        case 8:
            SIFM_DATA7_WRITE(pucValue[ucIdx + 7]);
        case 7:
            SIFM_DATA6_WRITE(pucValue[ucIdx + 6]);
        case 6:
            SIFM_DATA5_WRITE(pucValue[ucIdx + 5]);
        case 5:
            SIFM_DATA4_WRITE(pucValue[ucIdx + 4]);
        case 4:
            SIFM_DATA3_WRITE(pucValue[ucIdx + 3]);
        case 3:
            SIFM_DATA2_WRITE(pucValue[ucIdx + 2]);
        case 2:
            SIFM_DATA1_WRITE(pucValue[ucIdx + 1]);
        case 1:
            SIFM_DATA0_WRITE(pucValue[ucIdx + 0]);
        default:
            break;
        }

        SIFM_PGLEN_WRITE((ucWriteCount - 1));
        SIFM_TrigMode(SIFM_WRITE_DATA);

        u4Ack = SIFM_ACK_READ();
        for(ucTmpCount = 0; ((u4Ack & (1 << ucTmpCount)) != 0) && (ucTmpCount < 8); ucTmpCount++){}
        if(ucTmpCount != ucWriteCount)
        {
            //printf("ucTmpCount != ucWriteCount\n");
            ret = -1;
            break;
        }

        u4Count -= ucWriteCount;
        ucIdx += ucWriteCount;
    }
    //printf("MASTER0 WRITE OKAY\n");
    return ret;
}

int SifM1Write(u32 ucDev, const u8 *pucValue, u32 u4Count)
{
    u32 u4Ack, ucWriteCount, ucIdx, ucTmpCount;
    u32 value;
    ucIdx = 0; 

    if ((pucValue == NULL) ||(u4Count == 0))
    {
        return -1;
    }


    while (u4Count > 0)
    {
        ucWriteCount = (u4Count > 8) ? 8 : (u4Count);

        switch(ucWriteCount)
        {
        case 8:
            SIFM1_DATA7_WRITE(pucValue[ucIdx + 7]);
        case 7:
            SIFM1_DATA6_WRITE(pucValue[ucIdx + 6]);
        case 6:
            SIFM1_DATA5_WRITE(pucValue[ucIdx + 5]);
        case 5:
            SIFM1_DATA4_WRITE(pucValue[ucIdx + 4]);
        case 4:
            SIFM1_DATA3_WRITE(pucValue[ucIdx + 3]);
        case 3:
            SIFM1_DATA2_WRITE(pucValue[ucIdx + 2]);
        case 2:
            SIFM1_DATA1_WRITE(pucValue[ucIdx + 1]);
        case 1:
            SIFM1_DATA0_WRITE(pucValue[ucIdx + 0]);
        default:
            break;
        }

        SIFM1_PGLEN_WRITE((ucWriteCount - 1));
        SIFM1_TrigMode(SIFM_WRITE_DATA);

        u4Ack = SIFM1_ACK_READ();
        for(ucTmpCount = 0; ((u4Ack & (1 << ucTmpCount)) != 0) && (ucTmpCount < 8); ucTmpCount++){}
        if(ucTmpCount != ucWriteCount)
        {
            break;
        }

        u4Count -= ucWriteCount;
        ucIdx += ucWriteCount;
    }

    //printf("MASTER1 WRITE OKAY\n");
    return 0;
}

//////////////////////////////////

int i2c_probe(unsigned char chip)
{
   u8 buf[1];
   buf[0] = 0;
   /*
       * What is needed is to send the chip address and verify that the
       * address was <ACK>ed (i.e. there was a chip at that address which
       * drove the data line low).
       */
    //return (SifMRead(chip,buf,1,0) != 0);
}
int i2c_read (u8 chip, u32 addr, int alen, u8 * buffer, int len)
{   
       //printf("i2c_read\n");
    int i,t;
    int flag = 0;
    int value;
    u8  Raddr = 0;
    int u4Ack;
    if (alen > 2 || alen < 1)
    {
        printf ("I2C read: addr len %d not supported\n", alen);
        return 1;
    }
    
    //printf("chip:%x addr:%x alen:%d len:%d\n",chip,addr,alen,len);
    t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
    if(t<0)
        return t;
    if(i2c_bus_num == 0)
    {   
        if(alen == 1)
        {
            SIFM_TrigMode(SIFM_START);
            SIFM_DATA0_WRITE((chip << 1));
            SIFM_DATA1_WRITE(addr);
            SIFM_PGLEN_WRITE(0x01);
            
            SIFM_TrigMode(SIFM_WRITE_DATA);
            u4Ack = SIFM_ACK_READ();
            if (u4Ack != 0x3)
            {
                 printf("[_SifM0Read]Read ack error: AddrType = SIF_8_BIT, u4Ack = %d\n",
                        u4Ack);
                    SIFM_TrigMode(SIFM_STOP);
                    return -1;
            }
            SIFM_TrigMode(SIFM_STOP);
            t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
            if(t<0)
               return t;
            SIFM_TrigMode(SIFM_START);
            value = SifMRead(chip, buffer, len, 0);
            SIFM_TrigMode(SIFM_STOP);
        }
        if(alen == 2)
        {
            SIFM_TrigMode(SIFM_START);
            SIFM_DATA0_WRITE((chip << 1));
            SIFM_DATA1_WRITE((addr>>8));
            SIFM_DATA2_WRITE((addr&0xFF));
            SIFM_PGLEN_WRITE(0x02);
            
            SIFM_TrigMode(SIFM_WRITE_DATA);
            u4Ack = SIFM_ACK_READ();
            if (u4Ack != 0x7)
            {
                 printf("[_SifM0Read]Read ack error: AddrType = SIF_8_BIT, u4Ack = %d\n",
                        u4Ack);
                    SIFM_TrigMode(SIFM_STOP);
                    return -1;
            }
            SIFM_TrigMode(SIFM_STOP);
            t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
            if(t<0)
               return t;
            SIFM_TrigMode(SIFM_START);
            value = SifMRead(chip, buffer, len, 0);
            SIFM_TrigMode(SIFM_STOP);
        }
        
            
    }
    if(i2c_bus_num == 1)
    {
        if(alen == 1)
        {
            SIFM1_TrigMode(SIFM_START);
            SIFM1_DATA0_WRITE((chip << 1));
            SIFM1_DATA1_WRITE(addr);
            SIFM1_PGLEN_WRITE(0x01);
            
            SIFM1_TrigMode(SIFM_WRITE_DATA);
            u4Ack = SIFM1_ACK_READ();
            if (u4Ack != 0x3)
            {
                 printf("[_SifM1Read]Read ack error: AddrType = SIF_8_BIT, u4Ack = %d\n",
                        u4Ack);
                    SIFM1_TrigMode(SIFM_STOP);
                    return -1;
            }
            SIFM1_TrigMode(SIFM_STOP);
            t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
            if(t<0)
               return t;
            SIFM1_TrigMode(SIFM_START);
            value = SifM1Read(chip, buffer, len, 0);
            SIFM1_TrigMode(SIFM_STOP);
        }
        if(alen == 2)
        {
            SIFM1_TrigMode(SIFM_START);
            SIFM1_DATA0_WRITE((chip << 1));
            SIFM1_DATA1_WRITE((addr>>8));
            SIFM1_DATA2_WRITE((addr&0xFF));
            SIFM1_PGLEN_WRITE(0x02);
            
            SIFM1_TrigMode(SIFM_WRITE_DATA);
            u4Ack = SIFM1_ACK_READ();
            if (u4Ack != 0x7)
            {
                 printf("[_SifM1Read]Read ack error: AddrType = SIF_8_BIT, u4Ack = %d\n",
                        u4Ack);
                   SIFM1_TrigMode(SIFM_STOP);
                    return -1;
            }
            SIFM1_TrigMode(SIFM_STOP);
            t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
            if(t<0)
               return t;
            SIFM1_TrigMode(SIFM_START);
            value = SifM1Read(chip, buffer, len, 0);
            SIFM1_TrigMode(SIFM_STOP);
        }
    }
    //mdelay(10);
    return 0;
}
int i2c_write (u8 chip, u32 addr, int alen, u8 * buffer, int len)
{
    int i,t;
    u32 NoStart,NoRdAck;
    u32 u4Ack;
    if (alen > 2 || alen < 1)
    {
        printf ("I2C read: addr len %d not supported\n", alen);
        return -1;
    }
    
    //printf("chip:%x addr:%x alen:%d len:%d\n",chip,addr,alen,len);
    
    if(i2c_bus_num == 0)
    {
        if(alen == 1)
        {
            t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
            if(t<0)
               return t;
            SIFM_TrigMode(SIFM_START);
         
            SIFM_DATA0_WRITE((chip << 1));
            SIFM_DATA1_WRITE(addr);
            SIFM_PGLEN_WRITE(0x01);
            SIFM_TrigMode(SIFM_WRITE_DATA);
            u4Ack = SIFM_ACK_READ();
            if(u4Ack != 0x03)
            {
                printf("MASTER0 WRITE ACK FAILURE\n");
                SIFM_TrigMode(SIFM_STOP);
                return -1;
            }
            if(SifMWrite(chip, buffer, len))
            {
                   SIFM_TrigMode(SIFM_STOP);
                   return -1;
            }
            SIFM_TrigMode(SIFM_STOP);
            
        }
        if(alen == 2)
        {
            
            t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
            if(t<0)
               return t;
            SIFM_TrigMode(SIFM_START);
            SIFM_DATA0_WRITE((chip<<1));
            SIFM_DATA1_WRITE((addr>>8));
            SIFM_DATA2_WRITE((addr&0xff));
            SIFM_PGLEN_WRITE(0x02);
            SIFM_TrigMode(SIFM_WRITE_DATA);
            u4Ack = SIFM_ACK_READ();
            if(u4Ack != 0x7)
            {
                printf("MASTER0 WRITE ACK FAILURE\n");
                SIFM_TrigMode(SIFM_STOP);
                return -1;
            }
            SifMWrite(chip, buffer, len);
            SIFM_TrigMode(SIFM_STOP);
        }
    
    }
    if(i2c_bus_num == 1)
    {
         if(alen == 1)
        {
            t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
            if(t<0)
               return t;
            SIFM1_TrigMode(SIFM_START);
         
            SIFM1_DATA0_WRITE((chip << 1));
            SIFM1_DATA1_WRITE(addr);
            SIFM1_PGLEN_WRITE(0x01);
            SIFM1_TrigMode(SIFM_WRITE_DATA);
            u4Ack = SIFM1_ACK_READ();
            if(u4Ack != 0x01)
            {
                printf("MASTER1 WRITE ACK FAILURE\n");
                SIFM1_TrigMode(SIFM_STOP);
                return -1;
            }
            if(SifM1Write(chip, buffer, len))
            {
                   SIFM1_TrigMode(SIFM_STOP);
                   return -1;
            }
            SIFM1_TrigMode(SIFM_STOP);
            
        }
        if(alen == 2)
        {
            
            t=ac83xx_i2c_wait_for_bb(i2c_bus_num);
            if(t<0)
               return t;
            SIFM1_TrigMode(SIFM_START);
            SIFM1_DATA0_WRITE((chip<<1));
            SIFM1_DATA1_WRITE((addr>>8));
            SIFM1_DATA2_WRITE((addr&0xff));
            SIFM1_PGLEN_WRITE(0x02);
            SIFM1_TrigMode(SIFM_WRITE_DATA);
            u4Ack = SIFM1_ACK_READ();
            if(u4Ack != 0x7)
            {
                printf("MASTER0 WRITE ACK FAILURE\n");
                SIFM1_TrigMode(SIFM_STOP);
                return -1;
            }
            SifM1Write(chip, buffer, len);
            SIFM1_TrigMode(SIFM_STOP);
        }
        
    }
    //mdelay(20);
    return 0;
}

