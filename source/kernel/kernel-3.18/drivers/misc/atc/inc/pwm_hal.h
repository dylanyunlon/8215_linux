#ifndef __PWM_HAL_H__
#define __PWM_HAL_H__


#include "x_hal_ic.h"
#include "x_bim.h"
#include "x_printf.h"
#include "x_typedef.h"
#ifndef __ARM2__
#include "x_typedef.h"
#include "x_ckgen.h"
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <generated/atc_project.h>
//#include <linux/pinctl/consumer.h>
#ifdef CONFIG_ATC_PLATFORM_ac823x
#include <../include/mach/sync_write.h>
#endif
#else
#include <mach/pinmux.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include "x_ckgen_8317.h"
#endif


#define AC8317
/* PWM Registers define*/
#ifdef AC8317 
#ifdef CONFIG_ATC_PLATFORM_ac823x
#ifndef __ARM2__
extern ulong PWM_VBASE_ADDR;
#define PWM_REGS_BASE   (PWM_VBASE_ADDR)
#else
#define PWM_REGS_BASE   (IO_BASE_VA+ 0x32200)
#endif
#else
#define PWM_REGS_BASE   (IO_BASE_VA+ 0x32200)
#endif
#else
#define PWM_REGS_BASE   (IO_BASE_VA+ 0x50200)
#endif

#define PWM_CTRL 6

#define PWM_EN              (1 << 0)
#define PWM_PERSCALE_SET(x) (((x) &0x3F) <<2)
#define PWM_HIGH_SET(x)     (((x) &0xFFF) <<8)
#define PWM_RSN_SET(x)      (((x) &0xFFF) <<20)


#ifdef AC8317 
#define ATC_NR_PWMS 4
#else
#define ATC_NR_PWMS 6
#endif

#define ATC_NR_PWMCLK 8

#ifdef AC8317
#define PWM_PIN_FUN_MAX 3
#endif



/* pwm mode define */
typedef enum{
    NORMAL_TRI_MODE = 0,
    SYNC_TRI_MODE,
}PWM_TRI_MODE;

/* pwm base clk define  {27,216,108,294,324,418,150,270} */
typedef enum
{
    CLKSRC_27M = 0,
    CLKSRC_216M,
    CLKSRC_108M,
    CLKSRC_294M,
    CLKSRC_324M,
    CLKSRC_418M,
    CLKSRC_150M,
    CLKSRC_270M,
}PWM_BASE_CLK;

/* pwm config code define*/
typedef struct {
    u32 pin_id;
    u32 clk_id;
    bool pwm_en;
    u32 pwm_mode;
    u32 pwm_rsn;
    u32 pwm_high;
    u32 pwm_prescale;
}pwm_config_code;

#ifndef __ARM2__
extern struct clk *clk_ac8317_select_pwm[4];
extern struct clk *clk_ac8317_pwm[4];
#endif

/* pwm register read/write set/clear macro */
#ifdef CONFIG_ATC_PLATFORM_ac823x
#ifndef __ARM2__
#define IO_WRITE32(_reg_, _val_)   atc_reg_sync_writel(_val_, _reg_)
#define IO_READ32(_reg_)           __raw_readl((const volatile void *)_reg_)

#define PWM_READ32(id)              IO_READ32(PWM_REGS_BASE +((id) << 2))
#define PWM_WRITE32(id, value)      IO_WRITE32(PWM_REGS_BASE+ ((id) << 2), (value))
#else
#define PWM_READ32(id)              IO_READ32(PWM_REGS_BASE, (id) << 2)
#define PWM_WRITE32(id, value)      IO_WRITE32(PWM_REGS_BASE, (id) << 2, (value))

#define PWM_SET_BIT(id, Bit)    PWM_WRITE32(id, PWM_READ32(id) | (Bit))
#define PWM_CLR_BIT(id, Bit)    PWM_WRITE32(id, PWM_READ32(id) & (~(Bit)))
#endif
#else
#define PWM_READ32(id)              IO_READ32(PWM_REGS_BASE, (id) << 2)
#define PWM_WRITE32(id, value)      IO_WRITE32(PWM_REGS_BASE, (id) << 2, (value))

#define PWM_SET_BIT(id, Bit)    PWM_WRITE32(id, PWM_READ32(id) | (Bit))
#define PWM_CLR_BIT(id, Bit)    PWM_WRITE32(id, PWM_READ32(id) & (~(Bit)))
#endif

#define PWM_WRITE32_MASK(id, value, mask)       PWM_WRITE32(id, (value | (PWM_READ32(id) & (~mask))))

#define PWM_GET_HIGH(id) (PWM_READ32(id)&0x000FFF00)>>8
#define PWM_GET_RSN(id) (PWM_READ32(id)&0xFFF00000)>>20


/**
hal api
*/
bool pwm_hal_clk_select(u32 pwm_id, u32 clk_id);
void pwm_hal_dump(u32 pwm_id);
bool pwm_hal_config_pin(u32 pwm_id, s32 pin_id);
void pwm_hal_clk_enable(u32 pwm_id);
void pwm_hal_clk_disable(u32 pwm_id);
bool pwm_hal_config(u32 pwm_id, pwm_config_code *pwm_cfg_code);
bool  pwm_hal_mode_switch(u32 pwm_id, u32 pwm_mode);
bool pwm_hal_set_intensity(u32 pwm_id, u32  intensity);
u32 pwm_hal_get_intensity(u32 pwm_id);

#endif

