#include "targetConfig.h"

#include "auto_version.h" 
#include "preloader_common.h"
#include "chip_test.h"
#include "x_irq.h"
#include "msdc.h"
#include "boot.h"
#include "x_pdwnc.h"
#include "x_ckgen.h"

#define GPIO_WAKEUP_STS    0
#define GPIO_WAKEUP_SRC    1
#define GPIO_PAD_IR        2


#define GPIO_POLARITY_LOW  0
#define GPIO_POLARITY_HIGH 1
void Write2Mem(UINT32 u4StartMemAddr, UINT32 u4Len);
BOOL CmpWriteMem(UINT32 u4StartMemAddr, UINT32 u4Len);
typedef void (*ac83xx_sram_suspend_entry)(struct quickboot_param * );
static struct quickboot_param this_qb_param={0x5A,0X5A}; 
void  quickboot_suspend(void);
void quickboot_resume()
{

        Printf("quickboot test resume finish\n");

	Set_PDWN_GPIO_value(GPIO_WAKEUP_STS,GPIO_POLARITY_LOW);
	TIM_DelayUS(100000);
	CmpWriteMem(0x4000,0x40000000-0x4000);
	DramBAllRgnTest();
        quickboot_suspend();
	return;


}
void  quickboot_suspend(void)
{

	this_qb_param.ddr_cal_addr  = 0x0;
	this_qb_param.version = 0x1;
	this_qb_param.resume_entry = (UINT32)quickboot_resume;
	this_qb_param.wakeup_src_gpio = GPIO_WAKEUP_SRC;
	this_qb_param.wakeup_src_polarity = GPIO_POLARITY_HIGH;
	this_qb_param.wakeup_sts_gpio = GPIO_WAKEUP_STS;
	this_qb_param.wakeup_sts_polarity = GPIO_POLARITY_HIGH;
        Write2Mem(0x4000,0x40000000 - 0x4000);
        Printf("quickboot test suspend start\n");
	((ac83xx_sram_suspend_entry)(0xF4000020))(&this_qb_param);
}

