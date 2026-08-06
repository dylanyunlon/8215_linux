#include <common.h>


#define IO_BASE_VA          0xF0000000L
#define AUXADC_BIM_MODE     0


#define AUXADC_TS_CON0      0x060
#define AUXADC_MISC			0X098
#define AUXADC_ECC			0X09C
#define AUXADC_PDN_CON      0x194
#define AUX_BASE_ADDR	    0xA9000

//------------------------------------------------------------------------------
#define HAL_WRITE32(_reg_, _val_)           (*((volatile uint32_t*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)                   (*((volatile uint32_t*)(_reg_)))
#define IO_READ32(base, offset)                 HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value)         HAL_WRITE32((base) + (offset), (value))

#define ADC_WRITE32(offset, value)  	IO_WRITE32(IO_BASE_VA+AUX_BASE_ADDR, (offset), (value))
#define ADC_READ32(offset)          	IO_READ32(IO_BASE_VA+AUX_BASE_ADDR, (offset))
#define WR(addr, v) 			        ADC_WRITE32(addr, v)
#define RR(addr) 			            ADC_READ32(addr)

#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})
//------------------------------------------------------------------------------

int  RB(int addr, int m, int o)
{
	return (RR(addr)>>o)&m;
}

int  AuxgetTouchStatus(void)
{ 
	return RB(AUXADC_TS_CON0, 1, 1);
}

void touchadc_init()
{
	uint32_t tmp = 0;

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
 
}

void touchadc_dinit()
{
	unsigned int tmp = 0;
    
	tmp = IO_READ32(IO_BASE_VA, 0XB0);
	tmp &= 0XFFFFFFF1;
	IO_WRITE32(IO_BASE_VA, 0XB0, tmp);
}

int check_rtouch_pressed()
{
	unsigned int u4Rt = 0;
	unsigned int ret =0;
	int Pressed = 0;
	unsigned int adctouch_count = 0;
    unsigned int confirmed_count = 0;
#define  RETRY_CNT 30

    touchadc_init();

    while(1)
    {
        mdelay(20);
    	u4Rt = AuxgetTouchStatus();
		Pressed = (u4Rt)?1:0;

		if(0 == Pressed)
		{
			confirmed_count++;
			if (confirmed_count > RETRY_CNT)
			{
                 printf("[Recovery Mode] No key is pressed!\n");
                 ret  = 0;          
                 break;
            }
		}
		if(1 == Pressed)
		{
			adctouch_count++;
            if (adctouch_count > RETRY_CNT)
			{
                /* Assume that key is pressed */
                 printf("[Recovery Mode] Key is pressed to enter into recovery mode!\n");
                ret  = 1;          
                break;
            }
		}
    }
	
	return ret;
}
