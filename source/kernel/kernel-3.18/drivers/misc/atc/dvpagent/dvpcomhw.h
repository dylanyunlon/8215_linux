#ifndef __DVP_COM_HW_LIB__
#define __DVP_COM_HW_LIB__
#include "x_hal_ic.h"
#include "base_regs.h"
//#include "irqs_vector.h"
#include "types.h"
#include "x_os.h"
#include "x_assert.h"
#include "winutil.h"
#include "windows.h"
#include "83xx_irqs_vector.h"
#include <linux/types.h>
#include <linux/interrupt.h>

#define DVP_ASSERT(arg) ASSERT(arg)


#define AP_DVD_INT          (0x24c)

#define AP_8032_WSTA0       (0x250)
#define AP_8032_WSTA1       (0x254)
#define AP_8032_WSTA2       (0x258)
#define AP_8032_WSTA3       (0x25c)

#define AP_8032_RSTA0       (0x260)
#define AP_8032_RSTA1       (0x264)
#define AP_8032_RSTA2       (0x268)
#define AP_8032_RSTA3       (0x26c)

#define AP_PT110_WSTA0      (0X270)
#define AP_PT110_WSTA1      (0X274)
#define AP_PT110_WSTA2      (0X278)
#define AP_PT110_WSTA3      (0X27C)
#define AP_PT110_WSTA4      (0X280)
#define AP_PT110_WSTA5      (0X284)
#define AP_PT110_WSTA6      (0X288)
#define AP_PT110_WSTA7      (0X28C)
#define AP_PT110_WSTA8      (0X290)
#define AP_PT110_WSTA9      (0X294)
#define AP_PT110_WSTAA      (0X298)
#define AP_PT110_WSTAB      (0X29C)
#define AP_PT110_WSTAC      (0X2A0)
#define AP_PT110_WSTAD      (0X2A4)
#define AP_PT110_WSTAE      (0X2A8)
#define AP_PT110_WSTAF      (0X2AC)

#define AP_PT110_RSTA0      (0X2B0)
#define AP_PT110_RSTA1      (0X2B4)
#define AP_PT110_RSTA2      (0X2B8)
#define AP_PT110_RSTA3      (0X2BC)
#define AP_PT110_RSTA4      (0X2C0)
#define AP_PT110_RSTA5      (0X2C4)
#define AP_PT110_RSTA6      (0X2C8)
#define AP_PT110_RSTA7      (0X2CC)
#define AP_PT110_RSTA8      (0X2D0)
#define AP_PT110_RSTA9      (0X2D4)
#define AP_PT110_RSTAA      (0X2D8)
#define AP_PT110_RSTAB      (0X2DC)
#define AP_PT110_RSTAC      (0X2E0)
#define AP_PT110_RSTAD      (0X2E4)
#define AP_PT110_RSTAE      (0X2E8)
#define AP_PT110_RSTAF      (0X2EC)

#define AP_PT110_WRSTA0     (0x2F0)
#define AP_PT110_WRSTA1     (0x2F4)
#define AP_PT110_WRSTA2     (0x2F8)
#define AP_PT110_WRSTA3     (0x2Fc)


/*AP DVD INT*/
#define AP2UP_INT0          (0X00000001)
#define AP2UP_INT0_PCLR_SU  (0X00000002)
#define AP2UP_BSY0          (0X00000004)
#define AP2UP_BSY0_PCLR_SU  (0X00000008)
#define UP2AP_BSY0_ACLR     (0X00000010)
#define UP2AP_BSY0          (0X00000020)
#define AP2PT_INT0          (0X00000100)
#define AP2PT_INT1          (0X00000200)
#define AP2PT_INT2          (0X00000400)
#define AP2PT_INT3          (0X00000800)
#define PT2AP_BSY0          (0X00001000)
#define PT2AP_BSY1          (0X00002000)
#define PT2AP_BSY2          (0X00004000)
#define PT2AP_BSY3          (0X00008000)
#define AP2PT_BSY0          (0X00010000)
#define AP2PT_BSY1          (0X00020000)
#define AP2PT_BSY2          (0X00040000)
#define AP2PT_BSY3          (0X00080000)
#define PT2AP_BSY0_ACLR     (0X00100000)
#define PT2AP_BSY1_ACLR     (0X00200000)
#define PT2AP_BSY2_ACLR     (0X00400000)
#define PT2AP_BSY3_ACLR     (0X00800000)


bool DVPComHW_Init(void);
bool DVPComHW_Deinit(void);
u32 DVPComHW_SendData8032(u8 *pData, u32 dwSize);
u32 DVPComHW_SendDataPT110(u8 *pData, u32 dwSize);
u32 DVPComHW_ReceiveData(u8 *pData, u32 dwSize, bool *fgFromPT);
extern void  ac83xx_mask_ack_bim_irq(s32 irq);

#endif
