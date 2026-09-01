/**
*
* @file hcn_mw_common.c
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2025/10/18 12:20
* @author och
*
*/

#include "mw_init/hcn_mw_common.h"


#define SYS_PADCTL1_ADDR    (0x000000C4U)
#define SYS_PADCTL0_ADDR    (0x000000C0U)

#if 0
static void read_reg(void) {
    uint32_t reg_val = *((volatile uint32_t *)(REGS_SYSCTL_BASE + SYS_PADCTL1_ADDR));
    hcn_log_info("pad ctrl value = 0x%08X\n", reg_val);
}
#endif

void set_os_date_time(SystemTime_t date_time) {

}

SystemTime_t get_os_date_time(void) {

    SystemTime_t sys_time;
    return sys_time;

}


