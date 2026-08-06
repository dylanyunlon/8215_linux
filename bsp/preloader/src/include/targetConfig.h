#ifndef _TARGET_CONFIG_H_
#define _TARGET_CONFIG_H_



// chip version definition
#define CONFIG_CHIP_VER_MT3360    3360
#define CONFIG_CHIP_VER_MT8560    8560
#define CONFIG_CHIP_VER_MT8580    8580
#define CONFIG_CHIP_VER_MT3356    3356
#define CONFIG_CHIP_VER_MT3363	  3363

#define DRAM_TEST_RISC_WR_ENABLE  0x1
#define DRAM_TEST_CACHE_ENABLE    0x2
#define DRAM_TEST_RISC_WR_WITH_CACHE_ENABLE   0x3
#define DRAM_TEST_SELFTEST_ENABLE 0x4
#define DRAM_TEST_SELFREFRESH_ENABLE 0x08


#define PANEL_COLORBAR          1
#define PANEL_PICTURE           2
#define TVE_COLORBAR            3
#define TVE_PICTURE             4
#define DVD2PANNEL_BIC          5
#define TVE_FMT_BIC             6

#define DRAMC_ASYNC_MODE  1
#define DRAMC_SYNC_MODE  0




#define DRAM_TYPE_DDR3      0
#define DRAM_TYPE_DDR2      1

#define DRAM_BUSWIDTH_32    0
#define DRAM_BUSWIDTH_16    1

#define BOARD_TYPE_M1V1     0
#define BOARD_TYPE_M2V1     1
#define BOARD_TYPE_M6V1     3
#define BOARD_TYPE_3353YG   4

#define DRAM_SELFTEST_AGENT0_ENABLE   (1 << 0)
#define DRAM_SELFTEST_AGENT1_ENABLE   (1 << 1)
#define DRAM_SELFTEST_AGENT2_ENABLE   (1 << 2)





////////////////////////////////////////////////////////////////
#ifdef config_TARGET_SIM

#define DRAM_TYPE             DRAM_TYPE_DDR3
#define DRAM_BUSWIDTH         DRAM_BUSWIDTH_32
#define DRAM_DEBUG_ENABLE    0
#define CLK_SETTING_ENABLE   1
#define DDR_SETTING_ENABLE   0
#define ICE_DEBUG_ENABLE		0


#define MENU_CONFIG_ENABLE  0	
#define UART_ENABLE         0
#define CHIP_MAX_WORKFREQ_TEST_ENABLE  0


#define DRAMC_MODE_ENABLE   DRAMC_SYNC_MODE

#define SIMULATION_LOG   1

#define DRAM_TEST_ENABLE  0 //(DRAM_TEST_RISC_WR_ENABLE)//(DRAM_TEST_RISC_WR_ENABLE|DRAM_TEST_SELFTEST_ENABLE)
#define DRAM_TEST_SIZE  0x10000
#define DRAM_TEST_TIMES   1

#define DRAM_SELF_TEST_AGENT_ENABLE     0


#define CHIP_TEST_ENABLE   1
#if(CHIP_TEST_ENABLE)
#define MT3363_AP_SHOW_TEST_ENABLE 1
#define MT3363_AP_SHOW_FPD_COLORBAR_TEST_ENABLE 1
#define MT3363_AP_SHOW_FMT_COLORBAR_TEST_ENABLE 0

#define AP_SHOW_TEST_ENABLE      0
#define AP_BOOTUP_DVD_ENABLE   0
#define AP_TIMER_TEST_ENABLE    0

#define WAKEUP_TEST_ENABLE  0
#define VFP_TEST_ENABLE 0
#define MSDC_TEST_ENABLE 0
#define GPRBURSTRW_TEST_ENABLE  0
#define EXT_INT_TEST_ENABLE 0
#define WDT_TEST_ENABLE 0
#else
#define MT3363_AP_SHOW_TEST_ENABLE 0
#define MT3363_AP_SHOW_FPD_COLORBAR_TEST_ENABLE 0
#define MT3363_AP_SHOW_FMT_COLORBAR_TEST_ENABLE 0

#define AP_SHOW_TEST_ENABLE    0
#define AP_BOOTUP_DVD_ENABLE  0
#define AP_TIMER_TEST_ENABLE    0
#define WAKEUP_TEST_ENABLE  0
#define VFP_TEST_ENABLE 0
#define MSDC_TEST_ENABLE 0
#define GPRBURSTRW_TEST_ENABLE  0
#define EXT_INT_TEST_ENABLE 0
#define WDT_TEST_ENABLE 0

#endif


#define IRQ_ENABLE         0


#endif
////////////////////////////////////////////////////////////////
#ifdef config_TARGET_FPGA

#define DRAM_TYPE             DRAM_TYPE_DDR3
#define DRAM_BUSWIDTH         DRAM_BUSWIDTH_32
#define DRAM_DEBUG_ENABLE    0

#define CLK_SETTING_ENABLE   0
#define DDR_SETTING_ENABLE   1
#define ICE_DEBUG_ENABLE		0

#define MEMPLL_MHZ_SETTING      621

#define MENU_CONFIG_ENABLE  0	
#define CHIP_MAX_WORKFREQ_TEST_ENABLE  0

#define SIMULATION_LOG   0

//#define PWDN_TEST_ENABLE
#define UART_ENABLE      1

#define DRAMC_MODE_ENABLE   DRAMC_SYNC_MODE


#define DRAM_TEST_ENABLE 0 //(DRAM_TEST_RISC_WR_ENABLE|DRAM_TEST_CACHE_ENABLE|DRAM_TEST_SELFTEST_ENABLE)
#define DRAM_TEST_TIMES   0
#define DRAM_TEST_SIZE  0x10000000

#define DRAM_SELF_TEST_AGENT_ENABLE   DRAM_SELFTEST_AGENT0_ENABLE

#define CHIP_TEST_ENABLE   1


#define IRQ_ENABLE         1


#endif
/////////////////////////////////////////////////////////////////////
#ifdef config_TARGET_REALCHIP


#define DRAM_TYPE             DRAM_TYPE_DDR3
#define DRAM_BUSWIDTH         DRAM_BUSWIDTH_32
#define DRAM_DEBUG_ENABLE    0
#define CLK_SETTING_ENABLE   1
#define DDR_SETTING_ENABLE   1
#define ICE_DEBUG_ENABLE		0
#define IS_FOR_LITTLE_SIZE   1


#define MENU_CONFIG_ENABLE  0	
#define UART_ENABLE         1
#define CHIP_MAX_WORKFREQ_TEST_ENABLE  0


#define DRAMC_MODE_ENABLE   DRAMC_SYNC_MODE

#define SIMULATION_LOG   0

#define DRAM_TEST_ENABLE  0 //(DRAM_TEST_RISC_WR_ENABLE)//(DRAM_TEST_RISC_WR_ENABLE|DRAM_TEST_SELFTEST_ENABLE)
#define DRAM_TEST_SIZE  0x10000
#define DRAM_TEST_TIMES   1

#define DRAM_SELF_TEST_AGENT_ENABLE     0


#define CHIP_TEST_ENABLE   0
#if(CHIP_TEST_ENABLE)
#define MT3363_AP_SHOW_TEST_ENABLE 0
#define MT3363_AP_SHOW_FPD_COLORBAR_TEST_ENABLE 0
#define MT3363_AP_SHOW_FMT_COLORBAR_TEST_ENABLE 0

#define AP_SHOW_TEST_ENABLE      0
#define AP_BOOTUP_DVD_ENABLE     0
#define AP_INIT_TVE_ENABLE       0
#define AP_TIMER_TEST_ENABLE    0

#define WAKEUP_TEST_ENABLE  0
#define VFP_TEST_ENABLE 0
#define MSDC_TEST_ENABLE 0
#define GPRBURSTRW_TEST_ENABLE  0
#define EXT_INT_TEST_ENABLE 0
#define WDT_TEST_ENABLE 0
#else
#define MT3363_AP_SHOW_TEST_ENABLE 0
#define MT3363_AP_SHOW_FPD_COLORBAR_TEST_ENABLE 0
#define MT3363_AP_SHOW_FMT_COLORBAR_TEST_ENABLE 0

#define AP_SHOW_TEST_ENABLE    0
#define AP_BOOTUP_DVD_ENABLE  0
#define AP_INIT_TVE_ENABLE       0
#define AP_TIMER_TEST_ENABLE    0
#define WAKEUP_TEST_ENABLE  0
#define VFP_TEST_ENABLE 0
#define MSDC_TEST_ENABLE 0
#define GPRBURSTRW_TEST_ENABLE  0
#define EXT_INT_TEST_ENABLE 0
#define WDT_TEST_ENABLE 0

#endif


#define IRQ_ENABLE         0



#endif

/////////////////////////////////////////////////////////////////////

#define	ITCM_BASE_ADDRESS	0x40000000
#define	ITCM_SIZE		0x4000
#define	ITCM_HALF_SIZE		0x2000
#define	DTCM_BASE_ADDRESS	ITCM_BASE_ADDRESS + ITCM_SIZE
#define	DTCM_SIZE		0x4000
#define	DTCM_HALF_SIZE		0x2000



#if(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3360)
#define ARM_CORE_V6



#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3356)
#define ARM_CORE_V6


#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580)
#include "def8580.h"

#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
#define ARM_CORE_V7
#endif




// for DramK Setting
//#define DRAM_FLASH_SETTING   	0





#endif

