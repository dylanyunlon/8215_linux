#ifndef __USR_BACKLIGHT_H__
#define __USR_BACKLIGHT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "dev_config.h"


#define PWM_BACKLIGHT_PERION (1000000)  ///< pwm背光周期1MHZ

typedef enum {
   BACKLIGHT_LEVEL_AUTO = 0x00,  ///< 等级自动
   BACKLIGHT_LEVEL_1, 
   BACKLIGHT_LEVEL_2,
   BACKLIGHT_LEVEL_3,
   BACKLIGHT_LEVEL_4,
   BACKLIGHT_LEVEL_5
} led_level_e;


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __USR_BACKLIGHT_H__