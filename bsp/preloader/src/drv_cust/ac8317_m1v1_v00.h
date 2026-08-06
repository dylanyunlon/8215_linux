#ifndef DRAM_SETTING_H
#define DRAM_SETTING_H

#define DRAM_SETTING_VERSION  "ac8317 m1v1 v0.0"
#define DRAM_SUPPORT_PCB_TYPE "ac8317 m1v1 board"

#define SUPPORT_MEMPLL_FRACT   0   //1600mhz 
#define SUPPORT_SSC            1   //zhanpin
#define SUPPORT_AYSMMETRIC     0
#define SUPPORT_2GB_SIZE       1

//#define DEFAULT_DDR_CLOCK           648000000
#define DEFAULT_DDR_CLOCK        864000000
//#define DEFAULT_DDR_CLOCK         1080000000
//#define DEFAULT_DDR_CLOCK         1296000000
//#define DEFAULT_DDR_CLOCK         1404000000
//#define DEFAULT_DDR_CLOCK         1512000000
//#define DEFAULT_DDR_CLOCK         1620000000
//#define DEFAULT_DDR_CLOCK         1728000000
//#define DEFAULT_DDR_CLOCK        1836000000
//#define DEFAULT_DDR_CLOCK       1944000000


#define DEFAULT_DRAM_TYPE           DDR_III_16_x1
#define DEFAULT_DRAM_COLADDR        (COL_ADDR_BIT_10)
#define DEFAULT_DDR_CL              (11)
#define DEFAULT_DDR_BUS_X8          (0)  // Default is BUS X 16.

//#define DMPLL_SPECTRUM_PERMILLAGE   (100) // +- 1%.
#define DMPLL_SPECTRUM_PERMILLAGE   (50) // +- 0.5%.
//#define DMPLL_SPECTRUM_PERMILLAGE   (0) // disable DRAM spread specturm.


//#define DMPLL_SPECTRUM_FREQUENCY   (60) // Khz.
#define DMPLL_SPECTRUM_FREQUENCY    (30) // Khz.
//#define DMPLL_SPECTRUM_FREQUENCY    (0) // disable DRAM spread specturm.

// Audio/0 > B2R/5 > VBI/3D/TVE/2 > SCPOS/7 > NR/PSCAN/4 > OSD/OD/MMU/3 > MJC_IN/13 > MJC_OUT/14 > DEMUX/GCPU/1 > CPU/6 > VLD1/9 > VDEC_MC/8 > Test0/15 > GFX/11 > VENC/12 > 3DGFX/10
#define DRAM_PRIORITY_LIST          ((UINT8*)"08254193bafde67c")

#define DRAM_BURSTLEN               0
#define AGTIM0 0xFFFFFFFF
#define AGTIM1 0xFFFF77FF

#define DRAM_GROUP1ARBITERTIME      5
#define DRAM_GROUP2ARBITERTIME      4
#define DRAM_GROUP3ARBITERTIME      15

#define DEFAULT_DRAM_8_BANKS        1

#define FLAG_DDR_QFP                0

#define FLAG_DDR_DCBALANCE		    0   //1 dcon; 0 dcoff

#define FLAG_DDR_16BITSWAP          (FLAG_DDR_QFP)

#define FLAG_SD_USE_6MHZ			0	// 0--use 13MHz

#define SUPPORT_UBOOT_BACKUP        0   //Only for FGE 
#define FAST_BOOT_FOLLOW            0   //Only for FGE 
#define DRAM_CALI_FAIL_RESET        0

#define SUPPORT_8BIT                0
#if SUPPORT_8BIT
#define DDR_CHA_4BIT_SWAP
#define DDR_CONFIG_CSD
#undef FLAG_DDR_DCBALANCE
#define FLAG_DDR_DCBALANCE		    1   //1 dcon; 0 dcoff
#endif

#ifndef DEBUG_LOG
#define DEBUG_LOG                   0
#endif

#ifdef __LINUX__
#define CONFIG_SECURITY_UPGRADE
#endif

#endif /*DRAM_SETTING_H*/

