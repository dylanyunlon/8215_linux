#ifndef YBR_VGA_OAL_H__
#define YBR_VGA_OAL_H__

/*#include "x_typedef.h"*/
#ifndef __ARM2__
#include <linux/types.h>
#endif

/***Macro Define***/
#define UTIL_Printf printk

/***Enum Define***/
typedef enum {/*source type*/
        SRC_NULL,
        SRC_YBR,
        SRC_VGA 
}E_SOURCE_TYPE;

/***Function Declaration***/
void vUtDelay1ms(u32 n);
void vUtDelay2us(u32 n);
int  u4DrvVideoMainLoop(void *arg);
/*u32  vDrvVideoIrqHandler(u16 u2Vector, void *dev_id);*/


/***Extern Variable Declaration***/
extern u32 g_u4SrcType;
extern u8  g_u1Timing;
extern u8 g_u4SigStatus;
extern u8 g_u4SigPreStatus;
extern bool g_bStop;

#endif
