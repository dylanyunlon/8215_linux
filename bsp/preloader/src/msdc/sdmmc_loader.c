#include "x_typedef.h"
#include "x_pdwnc.h"
#include "../drv_cust/ac8317_m1v1_v00.h"
#include "msdc.h"
#ifdef __AndroidM__
#include "DataZone.h"
#include "../security/tz_init.h"
#endif
#include "boot.h"
#include "partition.h"
#include "reserved_memory.h"
#include "ddr_includes.h"
#include "metazone_inter.h"

#define DATA_ZONE_OFFSET 0x00010000 // 64k offset in card phy address
#define DATAZONE_ADDRESS 0x06EFFC00 // read to the memory that before uboot

#define DATAZONE_DRAM_ADDR CONFIG_DATAZONE_START

#define DRAMK_RUN_ADDRESS 0xF4008000 // SRAM 32K
#define DRAMK_PHY_OFFSET 0x00008000     // file_offset(31.5k) + bootloader_header(512bytes)
#define DRAMK_MAX_SIZE 0x8000         // 32K

#define MSDC_FLAG_REG (0x00024168) // 0xF0024164 (REG_RW_RESRV1)

#define FASTBOOT_UPG_MODE 0x11
#define COPY_UPG_MODE 0x12
int g_upg_mode = 0;
int g_partIndex = 0;

#ifdef ATC_AB_PARTITION_SUPPORT
UINT32 ab_slot = 0;
#endif

typedef struct _IMAGE_DESCRIPTOR
{
#ifdef CONFIG_SECURITY_UPGRADE
    char tags[16];
#endif
    UINT32 dwLoadAddress; // reserve
    UINT32 dwLoadPhyAddr; // dram address to load eboot
    UINT32 dwJumpAddress; // reserve
    UINT32 dwJumpPhyAddr; // entry point of eboot
    UINT32 dwStartAddr;      // eboot start address
    UINT32 dwTtlLen;      // eboot size
} IMAGE_DESCRIPTOR;
typedef void (*FUNC_CALL)(void);

static volatile ARGS_TO_ARM2_P *args_to_arm2 = NULL;

extern UINT32 _dramk_loader_start;
extern UINT32 _dramk_loader_end;
extern UINT32 _dramk_loader_len;

void check_mtz_upg_mode(void)
{
    UINT32 mode;

    _MetaZone_Read(MZ_UPGRADE_MODE_IDX, &mode);

    if (FASTBOOT_UPG_MODE == mode) {
        printf("check fastboot upgrade mode trigged!\n");
        g_upg_mode = 1;
    } else if (COPY_UPG_MODE == mode) {
        printf("check copy upgrade mode trigged!\n");
        g_upg_mode = 2;
    } else {
        g_upg_mode = 0;
    }

    return;
}

// Constant definitions
BOOL SDInitializeHardware(UINT32 u4SdCh)
{
    UINT32 idret = MSDC_Identify_Card(u4SdCh);
    if (idret & MSDC_LASTERROR_IDENTIFY)
    {
        Printf("---->>> SD%x Identify Card Failed: %x <<<-----\r\n", MSDC_CH_INDEX(u4SdCh), idret);
        return FALSE;
    }

    MSDC_StateChange(u4SdCh, 0); // Select card
    MSDC_SetBlockLength(u4SdCh, 512);

// Select clock
#if defined(config_TARGET_REALCHIP)

    if (MSDC_Is_eMMC_Card(u4SdCh) && (u4SdCh == MSDC_CH1))
    {
        // Printf("---> eMMC 27MHz <---\r\n");
        // MSDC_SetClockRate(u4SdCh, 13500000); // Real clock is 13.5MHz
        MSDC_EnterHighSpeedMode(u4SdCh);
        MSDC_SetClockRate(u4SdCh, 27000000); // Real clock is 27MHz
    }
    else
    {
#if FLAG_SD_USE_6MHZ
        MSDC_SetClockRate(u4SdCh, 6750000); // Real clock is 6.75MHz
#else
        MSDC_SetClockRate(u4SdCh, 13500000); // Real clock is 13.5MHz
#endif
    }

#elif defined(config_TARGET_FPGA)

    MSDC_SetClockRate(u4SdCh, 5000000);

#endif

    return TRUE;
}

// 0x8C000000
BOOL IsValidDecriptor(const IMAGE_DESCRIPTOR *pDescriptor)
{
#if defined(Config_WinCE)
    if (pDescriptor && 0x8C000000 == pDescriptor->dwJumpAddress - pDescriptor->dwJumpPhyAddr && 0x8C000000 == pDescriptor->dwLoadAddress - pDescriptor->dwLoadPhyAddr)
#else
    if (pDescriptor && 0x8BC60000 == pDescriptor->dwJumpAddress - pDescriptor->dwJumpPhyAddr && 0x8BC60000 == pDescriptor->dwLoadAddress - pDescriptor->dwLoadPhyAddr)
#endif
    {
        return TRUE;
    }

    return FALSE;
}

#if 0 // Max Xia Marked: Use another boot partition ops to get data from emmc, DON'T Delete it.
UINT32 sdmmc_loader(UINT32 u4SdCh)
{
    UINT32 i = 0;
    UINT32 ret = 0;
    UINT32 u4ReadOffset = 0,u4EndAddr = 0;
    UINT8 *pEbootAddr = 0;
    BOOL bRet = FALSE;
    BOOL bNeedReInit = FALSE;
    IMAGE_DESCRIPTOR *eboot_des = NULL;  

    UINT32 u4ZoneIndex,u4BegIndex,u4EndIndex;


    Printf("Init SD%x Card\r\n", MSDC_CH_INDEX(u4SdCh));

#ifdef config_TARGET_REALCHIP
    // Setting for SD_V33_18_SW0/1/2 Issue, make IO Voltage is 3.3V for SD2.0 Spec
    MSDC_WRITE32(0x308, (MSDC_READ32(0x308) | 0xD0000000));
    MSDC_WRITE32(0x80, (MSDC_READ32(0x80) | 0x001C0000));
    MSDC_WRITE32(0xEC, (MSDC_READ32(0xEC) & 0xFFE3FFFF));
#endif
    
    if(!SDInitializeHardware(u4SdCh))
    {
        return FALSE;
    }
    Printf("Init SD%x Card Succuss\r\n", MSDC_CH_INDEX(u4SdCh));

    Dump_Card_Type(u4SdCh, 3);

    //***************************************
    // Handle Dramk. Execute dramk for enable DRAM
    //***************************************

    // if it is emmc, dramk in boot partition 
    if (MSDC_Is_eMMC_Card(u4SdCh))
    {
        bNeedReInit = TRUE;
        // Step 1 - Copy dramk from eMMC boot partition to SRAM
        
        ret = MSDC_EMMC_EnterBootMode0(u4SdCh);
        if(ret != MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK)
        {
            Printf("Enter emmc boot mode failed: %X\r\n", ret);
            return FALSE;
        }
        ret = MSDC_EMMC_Read(u4SdCh, DRAMK_PHY_OFFSET,(UINT32 *)DRAMK_RUN_ADDRESS, DRAMK_MAX_SIZE);
        if(ret != (MSDC_LASTERROR_EMMCREAD | MSDC_LASTERROR_OK))
        {
            Printf("Read Dramk From Boot Partition 1 Failed");

            ret = MSDC_EMMC_ExitBootMode0(u4SdCh);
            if(ret != (MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK))
            {
                Printf("Exit emmc boot mode failed");
                return FALSE;
            }

            // Read Dramk from boot partition 2
            ret = MSDC_EMMC_ReadFromBoot2(u4SdCh, DRAMK_PHY_OFFSET, DRAMK_RUN_ADDRESS, DRAMK_MAX_SIZE);
            if(ret != (MSDC_LASTERROR_EMMCBOOT2 | MSDC_LASTERROR_OK))
            {
                Printf("read dramk from emmc boot partition 2 failed");
                return FALSE;
            }
        }

        ret = MSDC_EMMC_ExitBootMode0(u4SdCh);
        if(ret != (MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK))
        {
            Printf("Exit emmc boot mode failed");
            return FALSE;
        }

        // Step 2 - Execute dramk to init DRAM
          DDR_Initialize();
      
    }
    else // SD Card
    {
        // Step 1 - Copy dramk from SD to SRAM
        MSDC_SetBusWidth(u4SdCh, 4);
        MSDC_ReadBlock_PIO(u4SdCh, DRAMK_PHY_OFFSET, (UINT32 *)DRAMK_RUN_ADDRESS, DRAMK_MAX_SIZE);

        // Step 2 - Execute dramk to init DRAM
          DDR_Initialize();
    }

    //**************************************************
    // Handle Datazone. Get eboot setting from datazone 
    //**************************************************

    if (bNeedReInit) // Need re-init card after operate  emmc boot partition
    {
        if(!SDInitializeHardware(u4SdCh))
        {
            return FALSE;
        }
    }

    // Read Datazone for Addresses
    MSDC_ReadBlock_PIO(u4SdCh, DATA_ZONE_OFFSET, (UINT32 *)DATAZONE_ADDRESS, 512);

    eboot_des = ((IMAGE_DESCRIPTOR*)(DATAZONE_ADDRESS + 0x4C));
    Printf("eboot_img_des:\n");
    Printf("\tdwLoadAddress:0x%08x\n", eboot_des->dwLoadAddress);
    Printf("\tdwLoadPhyAddr:0x%08x\n", eboot_des->dwLoadPhyAddr);
    Printf("\tdwJumpAddress:0x%08x\n", eboot_des->dwJumpAddress);
    Printf("\tdwJumpPhyAddr:0x%08x\n", eboot_des->dwJumpPhyAddr);
    Printf("\tdwStartAddr:0x%08x\n", eboot_des->dwStartAddr);
    Printf("\tdwdwTtlLen0x%08x\n", eboot_des->dwTtlLen);
       
    if (!IsValidDecriptor(eboot_des))
    {
        Printf("Not Valid image descriptor:0x%x\n", u4SdCh);
        return FALSE;
    }

    u4ReadOffset = eboot_des->dwStartAddr ;
    u4EndAddr =  u4ReadOffset + eboot_des->dwTtlLen;
    pEbootAddr = (UINT8 *)eboot_des->dwLoadPhyAddr;

    Printf("Now! Load uboot......\r\n");

    //*********************************************
    // Handle eboot/uboot. Read it and jump to execute it
    //*********************************************

    MSDC_ReadBlock_PIO(u4SdCh,u4ReadOffset,(UINT32 *)pEbootAddr,eboot_des->dwTtlLen);

    
    
    //Printf("Load Eboot completed, dump 160 bytes:\r\n");
    //for(i = 0; i < 160; i++)
    //{
    //    Printf("%X ", *(pEbootAddr+i));
    //    if ((i + 1) % 16 == 0)
    //    {
    //        Printf("\r\n");
    //    }
    //}

#if 0
    while( u4ReadOffset < u4EndAddr)
    {
       MSDC_ReadBlock_PIO(MSDC_CH1,u4ReadOffset,(UINT32 *)pEbootAddr,512);
       pEbootAddr+=512;
       u4ReadOffset+=512;
    }
    
    *(UINT32 *)(508) = 0XFFFFFFFF;
    MSDC_WriteBlock_PIO(BOOT_CH,DATA_ZONE_OFSET,(UINT32 *)0X0,512);
#endif

    //Printf("Jump to uboot...\r\n");
    ((FUNC_CALL)(eboot_des->dwJumpPhyAddr))();
}

#endif

#if 0
void _reset()
{
    /*
     * use powerdown watch dog to reset system
     */
    UINT32 u4Test;

    Printf("Preloader Reboot is working now.\n");

    PDWNC_WRITE32(0x164, 0x24000164);

    //PDWNC_WRITE32(REG_RW_WDT, 0xff000000);
    PDWNC_WRITE32(REG_RW_WDT, 0xFFFF0000);

    //for(u4Test = 0; u4Test < 10000; u4Test++)
    //{

    //}
    PDWNC_WRITE32(REG_RW_WDTSET, 1);
    while(1);

}
#else
// Enable reset function when we can save reset times and use reset times to limit it.
void _reset()
{
}
#endif

#define MSDC2_CLK_DRV (0x07)
#define MSDC2_CMD_DRV (0x0C)
#define MSDC2_DAT_DRV (0x0C)

UINT32 msdc_env_setting()
{
    // Setting for SD_V33_18_SW0/1/2 Issue, make IO Voltage is 3.3V for SD2.0 Spec
    MSDC_WRITE32(0x308, (MSDC_READ32(0x308) | 0xD0000000));
    MSDC_WRITE32(0x80, (MSDC_READ32(0x80) | 0x001C0000));
    MSDC_WRITE32(0xEC, (MSDC_READ32(0xEC) & 0xFFE3FFFF));

#if 0 // Use the blow code, will cause uart output garbage data, marked it for check rootcause in the future, search "MARKED_UART_ISSUE"
    // ================= eMMC Bootup, SD0 Top Misc Setting ================
    // CLK Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_PUPD_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_DRV_MASK, 0x5); 

    // CMD Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_DRV_MASK, 0x5);

    // DAT0 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_DRV_MASK, 0x5);

    // DAT1 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_DRV_MASK, 0x5);

    // DAT2 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_DRV_MASK, 0x5);

    // DAT3 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_DRV_MASK, 0x5);

    // DAT4 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_DRV_MASK, 0x5);

    // DAT5 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_DRV_MASK, 0x5);

    // DAT6 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_DRV_MASK, 0x5);

    // DAT7 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_DRV_MASK, 0x5);

    // RST Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_PUPD_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_DRV_MASK, 0x5);
    


    // ================= SD1 Top Misc Setting ================
    // CLK Pad
    MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_PUPD_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_DRV_MASK, 0x5); 

    // CMD Pad
    MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_DRV_MASK, 0x5);

    // DAT0 Pad
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_DRV_MASK, 0x5);

    // DAT1 Pad
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_DRV_MASK, 0x5);

    // DAT2 Pad
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_DRV_MASK, 0x5);

    // DAT3 Pad
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_DRV_MASK, 0x5);

    // RST Pad
    MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_PUPD_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_DRV_MASK, 0x5);

    // ================= SD2 Top Misc Setting ================
    // CLK Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_PUPD_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_DRV_MASK, 0x5); 

    // CMD Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_DRV_MASK, 0x5);

    // DAT0 Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_DRV_MASK, 0x5);
#if 0

    // DAT1 Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_DRV_MASK, 0x5);

    // DAT2 Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_DRV_MASK, 0x5);

    // DAT3 Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_PUPD_MASK, 0);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_DRV_MASK, 0x5);

    // RST Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_SMT_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_RESISTOR_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_PUPD_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_IES_MASK, 1);
    MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_DRV_MASK, 0x5);
#endif
#else
    // ================= eMMC Bootup, SD0 Top Misc Setting ================
    // CLK Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_RESISTOR_MASK, 2 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_PUPD_MASK, 1 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // CMD Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // DAT0 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // DAT1 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // DAT2 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_RESISTOR_MASK, 2 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // DAT3 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // DAT4 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // DAT5 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // DAT6 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // DAT7 Pad
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_DRV_MASK, 0x5 << PAD_CFG_DRV_SHFIT);

    // ================= SD2 Top Misc Setting ================
    // CLK Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_RESISTOR_MASK, 2 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_PUPD_MASK, 1 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_DRV_MASK, MSDC2_CLK_DRV << PAD_CFG_DRV_SHFIT);

    // CMD Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_DRV_MASK, MSDC2_CMD_DRV << PAD_CFG_DRV_SHFIT);

    // DAT0 Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_DRV_MASK, MSDC2_DAT_DRV << PAD_CFG_DRV_SHFIT);

    // DAT1 Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_DRV_MASK, MSDC2_DAT_DRV << PAD_CFG_DRV_SHFIT);

    // DAT2 Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_DRV_MASK, MSDC2_DAT_DRV << PAD_CFG_DRV_SHFIT);

    // DAT3 Pad
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_SMT_MASK, 1 << PAD_CFG_SMT_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_RESISTOR_MASK, 1 << PAD_CFG_RESISTOR_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_PUPD_MASK, 0 << PAD_CFG_PUPD_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_IES_MASK, 1 << PAD_CFG_IES_SHFIT);
    MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_DRV_MASK, MSDC2_DAT_DRV << PAD_CFG_DRV_SHFIT);

#endif
    return 0;
}

#define BOOTHDR_TOTAL_SIZE (512)
#define BOOTHR_ID1_SIZE (12)
#define BOOTHR_ID2_SIZE (8)

#define BOOTHR_ID1 "BOOTLOADER!" // Must be BOOTHR_ID1_SIZE chars
#define BOOTHR_ID2 "MT3360A"     // Must be BOOTHR_ID2_SIZE chars

typedef struct _NANDINFO_
{
    UINT16 pageSize; /* 512, 2048 */
    UINT16 spareSize;
    UINT16 addressCycle;
    UINT16 pageShift;
} NANDINFO_t;

typedef struct _CARDINFO_
{
    UINT32 cardType;
    UINT32 cardSize;
} CARDINFO_t;

typedef union _MEDIAINFO_
{
    NANDINFO_t Nand;
    CARDINFO_t Card;
} MEDIAINFO_t;

typedef struct _BOOTHDR_
{
    char ID1[BOOTHR_ID1_SIZE];
    char version[4];
    UINT32 length;
    UINT32 startAddr;
    UINT32 checksum;
    char ID2[BOOTHR_ID2_SIZE];
    MEDIAINFO_t media;
    UINT32 options;
    UINT16 pagesPerBlock;
    UINT16 totalBlocks;
    UINT16 blockShift;
    UINT16 linkAddr[4];
    UINT16 lastBlock;
} BOOTHDR_t;

const char BootHdr_ID1[] = BOOTHR_ID1;
const char BootHdr_ID2[] = BOOTHR_ID2;

UINT32 VerifyBootHdr(BOOTHDR_t *pBootHdr)
{
    UINT32 n;
    UINT32 BootHdr_DuplicationNum = BOOTHDR_TOTAL_SIZE / sizeof(BOOTHDR_t);
    while (BootHdr_DuplicationNum--)
    {
        n = 0;
        for (n = 0; n < BOOTHR_ID1_SIZE; n++)
        {
            if (pBootHdr->ID1[n] != BootHdr_ID1[n])
            {
                return 0x01;
            }
        }

        for (n = 0; n < BOOTHR_ID2_SIZE; n++)
        {
            if (pBootHdr->ID2[n] != BootHdr_ID2[n])
            {
                return 0x02;
            }
        }
        pBootHdr++;
    }

    return 0;
}

UINT32 Save_Bootup_Device_Id(UINT32 u4Id)
{
    MSDC_WRITE32(MSDC_FLAG_REG, (MSDC_READ32(MSDC_FLAG_REG) & 0x0FFFFFFF) | (u4Id << 28));
    return 0;
}

#ifdef CONFIG_SECURITY_UPGRADE

#ifdef ATC_AB_PARTITION_SUPPORT
#define BOOTCTRL_MAGIC 0x19191100
#define BOOTCTRL_SUFFIX_A "_a"
#define BOOTCTRL_SUFFIX_B "_b"
#define BOOTCTRL_SLOT_A 0
#define BOOTCTRL_SLOT_B 1
#define BOOTCTRL_RSV_SIZE 500

typedef struct slot_metadata
{
    unsigned char priority : 3;
    unsigned char retry_count : 3;
    unsigned char successful_boot : 1;
    unsigned char normal_boot : 1;
} slot_metadata_t;

typedef struct boot_ctrl
{
    /* Magic for identification */
    UINT32 magic;
    /* Version of struct. */
    UINT8 version;
    /* Information about each slot. */
    UINT8 doublepart;
    slot_metadata_t slot_info[2];
    UINT32 checksum;
    UINT8 reserved[BOOTCTRL_RSV_SIZE];
} boot_ctrl_t, *boot_ctrl_t_p;
#endif

struct bootloader_message
{
    char tags[16];
    char checksum[4];
    char command[32];
    char status[32];
    char recovery[32];
    UINT32 bootflag;
    char laststatus[32];
#ifdef ATC_AB_PARTITION_SUPPORT
    boot_ctrl_t metadata;
    char reserved[512];
#else
    char reserved[1024];
#endif
};
static struct bootloader_message bcb;
enum checkpart
{
    DATAZONE = 0,
    BCB
} checkpartname;
UINT32 check_head_tag(enum checkpart checkpartname)
{
    char tags[16];
    if (checkpartname == DATAZONE)
    {
        memcpy(tags, (char *)(DRAMK_RUN_ADDRESS + 0x3C), 16);
        if (memcmp(tags, "mboot.nb0", 9) == 0)
            return 0;
    }
    else if (checkpartname == BCB)
    {
        memcpy(tags, (char *)DRAMK_RUN_ADDRESS, 16);
        if (memcmp(tags, "BCBHead", 7) == 0)
            return 0;
    }
    return 1;
}
UINT32 check_checksum(enum checkpart checkpartname)
{
    UINT32 checksum_from_calc = 0;
    UINT32 i = 0;
    UINT32 count = 0;
#define DATAZONE_SIZE 260
#define TAG_AND_CHECKSUM_SZIE 20
    if (checkpartname == DATAZONE)
    {
        count = DATAZONE_SIZE - TAG_AND_CHECKSUM_SZIE; // 260-->datazone size, 20-->tag & checksum size
        for (i = 0; i < count; i++)
        {
            checksum_from_calc += *((UINT8 *)(DRAMK_RUN_ADDRESS + i));
        }
        Printf("datazone checksum = %x, (*(UINT32 *)(DRAMK_RUN_ADDRESS + DATAZONE_SIZE)) = %x\n", checksum_from_calc, (*(UINT32 *)(DRAMK_RUN_ADDRESS + DATAZONE_SIZE)));
        if (checksum_from_calc == (*(UINT32 *)(DRAMK_RUN_ADDRESS + DATAZONE_SIZE)))
            return 0;
    }
    else if (checkpartname == BCB)
    {
        count = sizeof(struct bootloader_message) - TAG_AND_CHECKSUM_SZIE; // 20--> tag& checksum size
        for (i = 0; i < count; i++)
        {
            checksum_from_calc += *((UINT8 *)(DRAMK_RUN_ADDRESS + i + TAG_AND_CHECKSUM_SZIE));
        }
        Printf("bcb checksum = %x, (*(UINT32 *)(DRAMK_RUN_ADDRESS + 0x10)) = %x\n", checksum_from_calc, (*(UINT32 *)(DRAMK_RUN_ADDRESS + 0x10)));
        if (checksum_from_calc == (*(UINT32 *)(DRAMK_RUN_ADDRESS + 0x10)))
            return 0;
    }
    return 1;
}

UINT32 check_datazone_valid(UINT32 u4SdCh)
{
    UINT32 ret;

    UINT32 datazoneAddr = DATA_ZONE_OFFSET;
    UINT32 datazonePartitionSize = 4 * 1024;
    UINT32 datazoneReadSize = ((sizeof(IMAGE_DESCRIPTOR)) % 512 == 0) ? sizeof(IMAGE_DESCRIPTOR) : ((sizeof(IMAGE_DESCRIPTOR) + 512) - (sizeof(IMAGE_DESCRIPTOR) + 512) % 512);
    UINT32 BCBAddr = datazoneAddr + datazonePartitionSize;
    UINT32 BCBPartitionSize = 4 * 1024;
    UINT32 BCBReadSize = ((sizeof(struct bootloader_message)) % 512 == 0) ? sizeof(struct bootloader_message) : ((sizeof(struct bootloader_message) + 512) - (sizeof(struct bootloader_message) + 512) % 512);

    UINT32 datazoneAddr_bk = datazoneAddr + 480 * 1024;
    UINT32 BCBAddr_bk = datazoneAddr_bk + datazonePartitionSize;

    /*check bcb*/
    ret = MSDC_ReadBlock_PIO(u4SdCh, BCBAddr, (UINT32 *)DRAMK_RUN_ADDRESS, BCBReadSize);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read BCB Info Failed!\n");
        goto check_BCB_bk;
    }

#if 0
    ret = MSDC_ReadBlock_PIO(u4SdCh, BCBAddr + BCBPartitionSize, (UINT32 *)DRAMK_RUN_ADDRESS + BCBPartitionSize, DATAZONE_PART_INFO_SIZE);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read partition Info Failed!\n");
        goto check_BCB_bk;
    }
#endif

    checkpartname = BCB;
    if (check_head_tag(checkpartname) != 0)
    {
        Printf("No Valid BCB head tag Found!\n");
        goto check_BCB_bk;
    }

    if (check_checksum(checkpartname) != 0)
    {
        Printf("No Valid BCB data Found!\n");
        goto check_BCB_bk;
    }
    Printf("Read BCB Info Successfully, use BCB!\n");
    goto check_datazone;

check_BCB_bk:
    /*check bcb_bk*/
    ret = MSDC_ReadBlock_PIO(u4SdCh, BCBAddr_bk, (UINT32 *)DRAMK_RUN_ADDRESS, BCBReadSize);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read BCB_bk Info Failed!\n");
        return 1;
    }

#if 0
    ret = MSDC_ReadBlock_PIO(u4SdCh, BCBAddr_bk + BCBPartitionSize, (UINT32 *)DRAMK_RUN_ADDRESS + BCBPartitionSize, DATAZONE_PART_INFO_SIZE);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read partition_bk Info Failed!\n");
        return 1;
    }
#endif

    checkpartname = BCB;
    if (check_head_tag(checkpartname) != 0)
    {
        Printf("No Valid BCB_bk head tag Found!\n");
        return 1;
    }

    if (check_checksum(checkpartname) != 0)
    {
        Printf("No Valid BCB_bk data Found!\n");
        return 1;
    }
    Printf("Read BCB_bk Info Successfully, use BCB_bk!\n");

check_datazone:
    memcpy((UINT32 *)&bcb, (UINT32 *)DRAMK_RUN_ADDRESS, sizeof(struct bootloader_message) / 4);
    Printf("bcb.bootflag = %d!\n", bcb.bootflag);
    /*check datazone*/
    ret = MSDC_ReadBlock_PIO(u4SdCh, datazoneAddr, (UINT32 *)DRAMK_RUN_ADDRESS, datazoneReadSize);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read datazone Info Failed!\n");
        goto check_datazone_bk;
    }

    checkpartname = DATAZONE;
    if (check_head_tag(checkpartname) != 0)
    {
        Printf("No Valid datazone head tag Found!\n");
        goto check_datazone_bk;
    }

    if (check_checksum(checkpartname) != 0)
    {
        Printf("No Valid datazone data Found!\n");
        goto check_datazone_bk;
    }

    Printf("Read datazone Info Successfully, use datazone!\n");
    return 0;

check_datazone_bk:
    // try datazone_bk
    Printf("datazone is not correct, try to use datazone_bk\n");
    /*check datazone_bk*/
    ret = MSDC_ReadBlock_PIO(u4SdCh, datazoneAddr_bk, (UINT32 *)DRAMK_RUN_ADDRESS, datazoneReadSize);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read datazone_bk Info Failed!\n");
        return 1;
    }

    checkpartname = DATAZONE;
    if (check_head_tag(checkpartname) != 0)
    {
        Printf("No Valid datazone_bk head tag Found!\n");
        return 1;
    }

    if (check_checksum(checkpartname) != 0)
    {
        Printf("No Valid datazone_bk head tag Found!\n");
        return 1;
    }

    Printf("Read datazone_bk Info Successfully, use datazone_bk!\n");
    return 0;
}
#endif

unsigned int mmc_bootup_device = 10; // default value is invalid value
void get_mmc_bootup_device_id()
{
    mmc_bootup_device = ((*(volatile UINT32 *)0xF0024168) & 0xF0000000) >> 28;
    Printf("MMC Bootup Device: SD%d\n", mmc_bootup_device);
}

#ifdef ATC_AB_PARTITION_SUPPORT
const char *suffix[2] = {BOOTCTRL_SUFFIX_A, BOOTCTRL_SUFFIX_B};

const char *get_suffix()
{
    int slot = 0;
    slot_metadata_t slot_info[2];

    memcpy(slot_info, &bcb.metadata.slot_info, sizeof(slot_metadata_t) * 2);
    if (slot_info[0].priority >= slot_info[1].priority)
        slot = 0;
    else if (slot_info[0].priority < slot_info[1].priority)
        slot = 1;

    return suffix[slot];
}
#endif

partitionread *readpartitioninfofromflash()
{
    char *bufpartinfo;
    partitionhead *pparthead;
    partitionread *ppartread, *pprepartition, *pcurpartition;

    ppartread = (partitionread *)(DATAZONE_DRAM_ADDR + 512);

    pcurpartition = ppartread;
    pprepartition = pcurpartition;

    while (pcurpartition != NULL)
    {
        if (pcurpartition->u4LastPartition == 1)
        {
            pcurpartition->nextpartition = NULL;
            break;
        }
        else
        {
            pcurpartition = pcurpartition + 1;
            pprepartition->nextpartition = pcurpartition;
            pprepartition = pcurpartition;
        }
    }

    return ppartread;
}

partitionread *get_part_info_by_name(const char *name)
{
    partitionread *ppartitionread, *p;
    ppartitionread = readpartitioninfofromflash();
    p = ppartitionread;
    g_partIndex = 0;

    while (p)
    {
        if (strcmp(p->szPartName, name) == 0)
        {
            return p;
        }
        g_partIndex += 1;
        p = p->nextpartition;
    }

    Printf("ERR: Cannot find valid part Info\n");
    return NULL;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:          GetArm2BootContent

Description:      Get arm2 boot Content

Return Value:    TRUE on success

-------------------------------------------------------------------*/
static BOOL mmc_boot_arm2(UINT32 u4SdCh)
{
    BOOL bRet = TRUE;
    UINT32 ret = 0;
    UINT32 arm2_phy, logo_phy, vba_phy;
    UINT32 arm2_size, logo_size, vba_size;
    UINT32 arm2_nand_addr, logo_nand_addr, vba_nand_addr;
    partitionread *p = NULL;
    RSV_MEM_T *rsv = NULL;
    int i = 0;
#ifdef ATC_AB_PARTITION_SUPPORT
    char *a_str = "arm2_a";
    char *b_str = "arm2_b";
    char *a_system = "system_a";
    char *b_system = "system_b";
    char *arm2_str = NULL;
    char *system_str = NULL;
    if (ab_slot)
    {
        arm2_str = b_str;
        system_str = b_system;
    }
    else
    {
        arm2_str = a_str;
        system_str = a_system;
    }
#else
    char *arm2_str = "arm2";
    char *system_str = "system";
#endif
    char *logo_str = "logo";
    char *vba_str = "vba";

#if 0
    UINT32    arm2_phy = 0x40000,logo_phy = 0x7700000;
    UINT32    arm2_size = 0x248800,logo_size = 0x12c800;
    UINT32    arm2_nand_addr = 0x380000, logo_nand_addr = 0x7900000;
    ARGS_TO_ARM2_P *args_to_arm2 = (void *)0xa00000;
#endif

    p = get_part_info_by_name(arm2_str);
    if (NULL == p)
    {
        Printf("R arm2 fail\n");
        goto failed;
    }

    arm2_nand_addr = p->u8PartitionStartAddr;
    if (p->u8RealDataSize >= 0)
        arm2_size = (UINT32)(p->u8RealDataSize);
    else
        arm2_size = (UINT32)(p->u8PartitionSize);
    arm2_size = ALIGN(arm2_size, 512);

    p = get_part_info_by_name(logo_str);
    if (NULL == p)
    {
        Printf("R logo fail*\n");
        goto failed;
    }

    logo_nand_addr = p->u8PartitionStartAddr;
    if (p->u8RealDataSize >= 0)
        logo_size = (UINT32)(p->u8RealDataSize);
    else
        logo_size = (UINT32)(p->u8PartitionSize);
    logo_size = ALIGN(logo_size, 512);

    p = get_part_info_by_name(vba_str);
    if (NULL == p)
    {
        Printf("R logo fail*\n");
        goto failed;
    }

    vba_nand_addr = p->u8PartitionStartAddr;
    if (p->u8RealDataSize >= 0)
        vba_size = (UINT32)(p->u8RealDataSize);
    else
        vba_size = (UINT32)(p->u8PartitionSize);

    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("arm2");
    if (NULL == rsv)
    {
        Printf("get arm2 mem fail\n");
        goto failed;
    }
    arm2_phy = (UINT32)(rsv->start_addr);
    args_to_arm2 = (void *)(arm2_phy);
    arm2_phy = arm2_phy + 0x40000;
    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("animation");
    if (NULL == rsv)
    {
        Printf("get animation rsv fail\n");
        goto failed;
    }
    logo_phy = (UINT32)(rsv->start_addr) + (UINT32)(rsv->size) - MRF_LOAD_OFFSET;

    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("vba");
    if (NULL == rsv)
    {
        Printf("get multimedia rsv fail\n");
        goto failed;
    }
    vba_phy = (UINT32)(rsv->start_addr);
#if 0    
    Printf("arm2 rsv:%x --size:%x --%x\n",arm2_phy, arm2_size, arm2_nand_addr);
    Printf("logo rsv:%x --size:%x --%x\n",logo_phy, logo_size, logo_nand_addr);
#endif

    Printf("arm2 rsv:%x --size:%x --%x\n", arm2_phy, arm2_size, arm2_nand_addr);
    Printf("logo rsv:%x --size:%x --%x\n", logo_phy, logo_size, logo_nand_addr);
    Printf("vba rsv:%x --size:%x --%x\n", vba_phy, vba_size, vba_nand_addr);
    Printf("arm2 start:%u\n", boot_time_ms());
    ret = MSDC_ReadBlock_PIO(u4SdCh, arm2_nand_addr, (UINT32 *)arm2_phy, arm2_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read arm2 failed\r\n");
        goto failed;
    }
    Printf("arm2 end:%u\n", boot_time_ms());
    ret = MSDC_ReadBlock_PIO(u4SdCh, logo_nand_addr, (UINT32 *)logo_phy, logo_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read logo failed\r\n");
        goto failed;
    }
    Printf("logo end:%u\n", boot_time_ms());
    ret = MSDC_ReadBlock_PIO(u4SdCh, vba_nand_addr, (UINT32 *)vba_phy, vba_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read vba failed\r\n");
        goto failed;
    }
    Printf("vba end:%u\n", boot_time_ms());
    args_to_arm2->jump_instr = 0xea00fffe; // arm2 addr 0,  ldr pc, [pc #0x1000]
    args_to_arm2->dram_size = TCMGET_CHANNELA_SIZE() * 1024 * 1024;
    args_to_arm2->dtb_status = STATUS_WAIT_LOAD;
    if (g_upg_mode == 1 || g_upg_mode == 2)
    {
        args_to_arm2->upgrade_mode = BOOT_SD_UPGRADE;
    }
    else
    {
        args_to_arm2->upgrade_mode = BOOT_NO_UPGRADE;
    }
#ifdef ATC_AB_PARTITION_SUPPORT
    if (ab_slot == 0)
    {
        args_to_arm2->ab_slot = 0;
    }
    else
    {
        args_to_arm2->ab_slot = 1;
    }
    Printf("args_to_arm2 ab_slot is %d\n", args_to_arm2->ab_slot);
#endif
    p = get_part_info_by_name(system_str);
    if (NULL == p)
    {
        Printf("get system fail\n");
        goto failed;
    }
    args_to_arm2->system_index = g_partIndex;
    Printf("args_to_arm2 system_index is %d\n", args_to_arm2->system_index);
    Printf("upgrade_mode=%d\n", args_to_arm2->upgrade_mode);
    Printf("dram size:%x\n", args_to_arm2->dram_size);
    u4ARM2Start(arm2_phy - 0x40000);
    return bRet;
failed:
    bRet = FALSE;
    while (1)
        ;
}

#define MTZ_PARTITION_SIZE (0x20000)
static BOOL mmc_load_metazone(UINT32 u4SdCh, UINT32 mtz_addr, UINT32 mtz_size)
{
    BOOL bRet = TRUE;
    UINT32 ret = 0;
    UINT32 mtz1_addr, mtz2_addr;
    // UINT32 mtz_size, mtz_part_addr;
    // char *mtz_str = "metazone";
    // partitionread *p = NULL;
    RSV_MEM_T *rsv = NULL;
    int i;
    char buff[16] = {0};

    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("metazone");
    if (NULL == rsv)
    {
        Printf("get mtz mem fail\n");
        goto failed;
    }
    mtz1_addr = (UINT32)(rsv->start_addr);
    mtz2_addr = (UINT32)(rsv->start_addr) + mtz_size;
    Printf("mtz mtz1:%x mtz2:%x size:%x part:%x\n", mtz1_addr, mtz2_addr, mtz_size, mtz_addr);

    Printf("mtz start:%u\n", boot_time_ms());
    ret = MSDC_ReadBlock_PIO(u4SdCh, mtz_addr, (UINT32 *)mtz1_addr, mtz_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read mtz failed\r\n");
        goto failed;
    }
    memcpy((UINT32 *)buff, (UINT32 *)mtz1_addr, 16);
    // Printf("emmc mtz1 print\n");
    // for (i=0; i<16; i++) {
    //    Printf("buf[%d]:%d\n", i, buff[i]);
    // }

    ret = MSDC_ReadBlock_PIO(u4SdCh, mtz_addr, (UINT32 *)mtz2_addr, mtz_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read mtz failed\r\n");
        goto failed;
    }
    memcpy((UINT32 *)buff, (UINT32 *)mtz2_addr, 16);
    // Printf("emmc mtz2 print\n");
    // for (i=0; i<16; i++) {
    //    Printf("buf[%d]:%d\n", i, buff[i]);
    // }
    Printf("mtz end:%u\n", boot_time_ms());

    return bRet;

failed:
    bRet = FALSE;
    while (1)
        ;
}

static BOOL mmc_boot_tz(UINT32 u4SdCh)
{
    BOOL bRet = TRUE;
    UINT32 ret;
    UINT32 tz_phy, kernel_phy, dtb_phy;
    UINT32 tz_size, kernel_size, dtb_size;
    UINT32 tz_nand_addr, kernel_nand_addr, dtb_nand_addr;
    partitionread *p = NULL;
    RSV_MEM_T *rsv = NULL;
    void *theKernel = NULL;
    int i = 0;
#ifdef ATC_AB_PARTITION_SUPPORT
    char *a_str = "trustzone_a";
    char *b_str = "trustzone_b";
    char *c_str = "kernel_a";
    char *d_str = "kernel_b";
    char *e_str = "dtb_a";
    char *f_str = "dtb_b";
    char *tz_str = NULL;
    char *kernel_str = NULL;
    char *dtb_str = NULL;
    if (ab_slot)
    {
        tz_str = b_str;
        kernel_str = d_str;
        dtb_str = f_str;
    }
    else
    {
        tz_str = a_str;
        kernel_str = c_str;
        dtb_str = e_str;
    }
#else
    char *tz_str = "trustzone";
    char *kernel_str = "kernel";
    char *dtb_str = "dtb";
#endif

#if 0
    UINT32  arm2_phy = 0x40000,logo_phy = 0x7700000;
    UINT32  arm2_size = 0x248800,logo_size = 0x12c800;
    UINT32  arm2_nand_addr = 0x380000, logo_nand_addr = 0x7900000;
    ARGS_TO_ARM2_P *args_to_arm2 = (void *)0xa00000;
#endif

    p = get_part_info_by_name(tz_str);
    if (NULL == p)
    {
        Printf("Read trustzone failed\n");
        goto failed;
    }

    tz_nand_addr = p->u8PartitionStartAddr;
    if (p->u8RealDataSize >= 0)
        tz_size = (UINT32)(p->u8RealDataSize);
    else
        tz_size = (UINT32)(p->u8PartitionSize);
    tz_size = ALIGN(tz_size, 512);

    p = get_part_info_by_name(kernel_str);
    if (NULL == p)
    {
        Printf("Read kernel failed*********************\n");
        goto failed;
    }

    kernel_nand_addr = p->u8PartitionStartAddr;
    if (p->u8RealDataSize >= 0)
        kernel_size = (UINT32)(p->u8RealDataSize);
    else
        kernel_size = (UINT32)(p->u8PartitionSize);
    kernel_size = ALIGN(kernel_size, 512);

    p = get_part_info_by_name(dtb_str);
    if (NULL == p)
    {
        Printf("Read dtb failed*********************\n");
        goto failed;
    }

    dtb_nand_addr = p->u8PartitionStartAddr;
    if (p->u8RealDataSize >= 0)
        dtb_size = (UINT32)(p->u8RealDataSize);
    else
        dtb_size = (UINT32)(p->u8PartitionSize);
    dtb_size = ALIGN(dtb_size, 512);

    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("trustzone");
    if (NULL == rsv)
    {
        Printf("get trustzone reserved_memory failed ,please check\n");
        goto failed;
    }
    tz_phy = (UINT32)(rsv->start_addr);

    kernel_phy = (UINT32)(KERNEL_LOAD_ADDR);
    dtb_phy = (UINT32)(FDT_LOAD_ADDR);
    Printf("tz rsv:%x --size:%x --%x\n", tz_phy, tz_size, tz_nand_addr);
    Printf("kernel addr:%x --size:%x --%x\n", kernel_phy, kernel_size, kernel_nand_addr);
    Printf("dtb addr:%x --size:%x --%x\n", dtb_phy, dtb_size, dtb_nand_addr);
    Printf("dtb load:%u\n", boot_time_ms());
    ret = MSDC_ReadBlock_PIO(u4SdCh, dtb_nand_addr, (UINT32 *)dtb_phy, dtb_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read dtb failed\r\n");
        goto failed;
    }
    args_to_arm2->dtb_status = STATUS_LOAD_READY;
    Printf("dtb end:%u\n", boot_time_ms());
    ret = MSDC_ReadBlock_PIO(u4SdCh, tz_nand_addr, (UINT32 *)tz_phy, tz_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read tz failed\r\n");
        goto failed;
    }

    Printf("trustzone end load:%u\n", boot_time_ms());
    ret = MSDC_ReadBlock_PIO(u4SdCh, kernel_nand_addr, (UINT32 *)kernel_phy, kernel_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read dtb failed\r\n");
        goto failed;
    }
    Printf("kernel end:%u\n", boot_time_ms());
    //    delete_node_by_path((void *)dtb_phy, dtb_size, "/cpus/cpu@2");
    //    delete_node_by_path((void *)dtb_phy, dtb_size, "/cpus/cpu@3");
    while (STATUS_MODIFY_END != args_to_arm2->arm_dtb_status)
        ;
    Printf("start tz:%u\n", boot_time_ms());
    void (*initTrustZone)(int zero, int arch, UINT32 parms, void *theKernel);
    theKernel = (void *)(KERNEL_LOAD_ADDR);
    initTrustZone = (void (*)(int, int, UINT32, void *))tz_phy;
    initTrustZone(0, (int)(MACH_TYPE_AC83XX), FDT_LOAD_ADDR, theKernel);
    return bRet;
failed:
    bRet = FALSE;
    while (1)
        ;
}

UINT32 sdmmc_loader(UINT32 u4SdCh)
{
    upgrade_arg_t *args_va = NULL;
    UINT32 boot_misc_addr = 0;
    UINT32 boot_misc_size = 0;
    UINT32 metazone_addr = 0;
    UINT32 metazone_size = 0;
    UINT32 i = 0;
    UINT32 ret = 0;
    UINT32 u4EbootFileOffset = 0;
    UINT32 u4EbootLoadPhyAddr = 0;
    UINT32 u4EbootLen = 0;
    UINT32 u4EbootJumpPhyAddr = 0;
    BOOL bRet = FALSE;

#if SUPPORT_UBOOT_BACKUP
    UINT32 u4Checksum = 0;
    BOOL bIsFirstUboot = TRUE; // true means read from first partition,esle read backup uboot
    BOOL bFirstUbootError = FALSE;
    BOOL bBackUpUbootError = FALSE;
#endif

#ifdef __AndroidM__
    TDataZone *pdata_zone = NULL;
    UINT32 u4TZFileOffset = 0;
    UINT32 u4TZLoadPhyAddr = 0;
    UINT32 u4TZLen = 0;
    UINT32 u4TZJumpPhyAddr = 0;
    UINT32 u4BootMode = 0;
#endif
    IMAGE_DESCRIPTOR *eboot_des = NULL;

    UINT32 u4ZoneIndex, u4BegIndex, u4EndIndex;

    UINT8 *pDramkStart = (UINT8 *)(&_dramk_loader_start);
    UINT8 *pDramkEnd = (UINT8 *)(&_dramk_loader_end);
    UINT32 u4DramkLength = ALIGN((pDramkEnd - pDramkStart), 512);
    // UINT32 u4DramkLength = ALIGN((_dramk_loader_len), 512);
    // UINT32 u4DramkLength = (UINT32)(_dramk_loader_len);

    // Save boot device id for uboot
    Save_Bootup_Device_Id(MSDC_CH_INDEX(u4SdCh));

    if (u4DramkLength > DRAMK_MAX_SIZE)
    {
        Printf("dramk size error\r\n");
        u4DramkLength = DRAMK_MAX_SIZE;
    }

    Printf("Init SD%x Card\r\n", MSDC_CH_INDEX(u4SdCh));

#ifdef config_TARGET_REALCHIP
    msdc_env_setting();
#endif

#if 1
    if (!SDInitializeHardware(u4SdCh))
    {
        return FALSE;
    }

#else
    for (i = 0; i < 5; i++)
    {
        if (SDInitializeHardware(u4SdCh))
            break;

        if (i == 4)
            return FALSE;
    }
#endif
    Printf("Init SD%x Card Succuss\r\n", MSDC_CH_INDEX(u4SdCh));
    Dump_Card_Type(u4SdCh, 3);

    //***************************************
    // Handle Dramk. Execute dramk for enable DRAM
    //***************************************

    // if it is emmc, dramk in boot partition
    // Now, some emmc work in boot mode with 4 bit bus width has some issue, so we not change emmc bus width here.
    if (!MSDC_Is_eMMC_Card(u4SdCh))
    {
        // MSDC_SetBusWidth(u4SdCh, 4); // Can not change to 4Bit mode, for some SD card can not work.

        // Check BOOT HEADER for SD Card
        ret = MSDC_ReadBlock_PIO(u4SdCh, 0, (UINT32 *)DRAMK_RUN_ADDRESS, BOOTHDR_TOTAL_SIZE);
        if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
        {
            Printf("SD%x Read BOOT HEADER Failed.\r\n", MSDC_CH_INDEX(u4SdCh));
            _reset();
        }
        if (VerifyBootHdr((BOOTHDR_t *)DRAMK_RUN_ADDRESS))
        {
            Printf("SD%x Verify Boot Header Failed.\r\n", MSDC_CH_INDEX(u4SdCh));
            return FALSE;
        }

        // Change Buswidth after reading boothdr, make sure read boothdr correctly.
        MSDC_SetBusWidth(u4SdCh, 4);
    }

#ifdef CONFIG_SECURITY_UPGRADE
    if (MSDC_Is_eMMC_Card(u4SdCh) && (u4SdCh == MSDC_CH1))
    {
        if (check_datazone_valid(u4SdCh) != 0)
        {
            Printf("check datazone info Failed.\r\n");
            while (1)
                ;
        }
    }
    else
    {
        // Check bootloader position info
        ret = MSDC_ReadBlock_PIO(u4SdCh, DATA_ZONE_OFFSET, (UINT32 *)DRAMK_RUN_ADDRESS, 512);
        if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
        {
            Printf("SD%x Read bootloader position info Failed.\r\n", MSDC_CH_INDEX(u4SdCh));
            _reset();
        }
    }
#else
    // Check bootloader position info
    ret = MSDC_ReadBlock_PIO(u4SdCh, DATA_ZONE_OFFSET, (UINT32 *)DRAMK_RUN_ADDRESS, 512);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("SD%x Read bootloader position info Failed.\r\n", MSDC_CH_INDEX(u4SdCh));
        _reset();
    }
#endif

#ifdef __AndroidM__
    pdata_zone = ((TDataZone *)(DRAMK_RUN_ADDRESS));
    u4BootMode = pdata_zone->dwSignature;
    Printf("BootMode: [%d]\r\n", u4BootMode);
#endif
    /*
     * use pdata_zone->dwSignature (u4BootMode) to decide booting flow
     *   0: preloader -> uboot  (original flow for 8317 linux/jb2 & 8317 m upgrade)
     *   1: preloader -> trustzone -> lk/uboot (normal booting flow for 8317 m)
     */
    // if (0 == u4BootMode)
    //{
#ifdef CONFIG_SECURITY_UPGRADE
#ifdef ATC_AB_PARTITION_SUPPORT
    get_mmc_bootup_device_id();
    if (mmc_bootup_device == 0)
    {
        const char *ab_suffix = get_suffix();
        if (0 == strcmp(ab_suffix, BOOTCTRL_SUFFIX_A))
        {
            ab_slot = 0;
        }
        else if (0 == strcmp(ab_suffix, BOOTCTRL_SUFFIX_B))
        {
            ab_slot = 1;
        }
        else
        {
            Printf("error boot slot info\n");
            while (1)
                ;
        }
    }
    else
    {
        ab_slot = 0;
        Printf("SD boot use slot0!\n");
    }

    Printf("ab_slot is %d\n", ab_slot);
    if (ab_slot == 0)
    {
        eboot_des = ((IMAGE_DESCRIPTOR *)(DRAMK_RUN_ADDRESS + 0x3C));
    }
    else
    {
        eboot_des = ((IMAGE_DESCRIPTOR *)(DRAMK_RUN_ADDRESS + 0xAC));
    }
#else
    if (bcb.bootflag == 0)
    {
        eboot_des = ((IMAGE_DESCRIPTOR *)(DRAMK_RUN_ADDRESS + 0x3C));
    }
    else if (bcb.bootflag == 1)
    {
        eboot_des = ((IMAGE_DESCRIPTOR *)(DRAMK_RUN_ADDRESS + 0xAC));
    }
    else
    {
        Printf("error bootflag: %d\n", bcb.bootflag);
        while (1)
            ;
    }
#endif
    Printf("eboot_img_des:\n");
    Printf("\tdwLoadAddress:0x%08x\n", eboot_des->dwLoadAddress);
    Printf("\tdwLoadPhyAddr:0x%08x\n", eboot_des->dwLoadPhyAddr);
    Printf("\tdwJumpAddress:0x%08x\n", eboot_des->dwJumpAddress);
    Printf("\tdwJumpPhyAddr:0x%08x\n", eboot_des->dwJumpPhyAddr);
    Printf("\tdwStartAddr:0x%08x\n", eboot_des->dwStartAddr);
    Printf("\tdwdwTtlLen0x%08x\n", eboot_des->dwTtlLen);

    if (!IsValidDecriptor(eboot_des))
    {
        Printf("Not Valid image descriptor:0x%x\n", u4SdCh);
        return FALSE;
    }

    u4EbootFileOffset = eboot_des->dwStartAddr;
    u4EbootLen = eboot_des->dwTtlLen;
    u4EbootLoadPhyAddr = eboot_des->dwLoadPhyAddr;
    u4EbootJumpPhyAddr = eboot_des->dwJumpPhyAddr;
#else
    eboot_des = ((IMAGE_DESCRIPTOR *)(DRAMK_RUN_ADDRESS + 0x4C));
    Printf("eboot_img_des:\n");
    Printf("\tdwLoadAddress:0x%08x\n", eboot_des->dwLoadAddress);
    Printf("\tdwLoadPhyAddr:0x%08x\n", eboot_des->dwLoadPhyAddr);
    Printf("\tdwJumpAddress:0x%08x\n", eboot_des->dwJumpAddress);
    Printf("\tdwJumpPhyAddr:0x%08x\n", eboot_des->dwJumpPhyAddr);
    Printf("\tdwStartAddr:0x%08x\n", eboot_des->dwStartAddr);
    Printf("\tdwdwTtlLen0x%08x\n", eboot_des->dwTtlLen);

    if (!IsValidDecriptor(eboot_des))
    {
        Printf("Not Valid image descriptor:0x%x\n", u4SdCh);
        return FALSE;
    }

    u4EbootFileOffset = eboot_des->dwStartAddr;
    u4EbootLen = eboot_des->dwTtlLen;
    u4EbootLoadPhyAddr = eboot_des->dwLoadPhyAddr;
    u4EbootJumpPhyAddr = eboot_des->dwJumpPhyAddr;
#endif
    //}
#if 0 // SUPPORT_TRUSTZONE
    //else
        if (1 == u4BootMode) //normal boot for ac8317 m
    {
        u4TZFileOffset = pdata_zone->id[1].dwStartSector;
        u4TZLoadPhyAddr = pdata_zone->id[1].dwLoadPhyAddr;
        u4TZLen = pdata_zone->id[1].dwTtlSectors;
        u4TZJumpPhyAddr = pdata_zone->id[1].dwJumpPhyAddr;

        //Printf("tz_img_des:\n");
        //Printf("\tdwLoadPhyAddr:0x%08x\n", u4TZLoadPhyAddr);
        //Printf("\tdwJumpPhyAddr:0x%08x\n", u4TZJumpPhyAddr);
        //Printf("\tdwStartAddr:0x%08x\n", u4TZFileOffset);
        //Printf("\tdwdwTtlLen0x%08x\n", u4TZLen);

        if (u4TZFileOffset == 0 || u4TZLoadPhyAddr != u4TZJumpPhyAddr || u4TZLen == 0)
        {
            Printf("Not Valid TZ image descriptor:0x%x\n", u4SdCh);
            return FALSE;
        }
    }
#endif
    // else
    //{
    // Printf("Wrong Signature [%d] \n", u4BootMode);
    //_reset();
    // return FALSE;
    //}

    //***************************************
    // Handle Dramk. Execute dramk for enable DRAM
    //***************************************

    // if it is emmc, dramk in boot partition
    if (MSDC_Is_eMMC_Card(u4SdCh) && (u4SdCh == MSDC_CH1))
    {
        // Step 1 - Copy dramk from eMMC boot partition to SRAM
        do
        {
            ret = MSDC_EMMC_EnterBoot1(u4SdCh);
            if (ret != MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK)
            {
                Printf("Enter emmc boot1 failed\n");
                break;
            }

            // Check BOOT HEADER for eMMC Card (from boot partition)
            ret = MSDC_ReadBlock_PIO(u4SdCh, 0, (UINT32 *)DRAMK_RUN_ADDRESS, BOOTHDR_TOTAL_SIZE);
            if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
            {
                Printf("Read BOOT Header failed\n");
                _reset();
                break;
            }
            if (VerifyBootHdr((BOOTHDR_t *)DRAMK_RUN_ADDRESS))
            {
                Printf("SD%x Verify Boot Header Failed.\r\n", MSDC_CH_INDEX(u4SdCh));
                break;
            }
            bRet = TRUE;
        } while (0);

        if (bRet == FALSE)
        {
            Printf("SD%x try boot2.\r\n", MSDC_CH_INDEX(u4SdCh));
            ret = MSDC_EMMC_EnterBoot2(u4SdCh);
            if (ret != MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK)
            {
                Printf("Enter emmc boot2 failed\n");
                return FALSE;
            }

            // Check BOOT HEADER for eMMC Card (from boot partition)
            ret = MSDC_ReadBlock_PIO(u4SdCh, 0, (UINT32 *)DRAMK_RUN_ADDRESS, BOOTHDR_TOTAL_SIZE);
            if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
            {
                Printf("Read BOOT Header failed\n");
                _reset();
                return FALSE;
            }
            if (VerifyBootHdr((BOOTHDR_t *)DRAMK_RUN_ADDRESS))
            {
                Printf("SD%x Verify Boot Header Failed.\r\n", MSDC_CH_INDEX(u4SdCh));
                return FALSE;
            }
        }

        // Read DRAMK
        ret = MSDC_ReadBlock_PIO(u4SdCh, DRAMK_PHY_OFFSET, (UINT32 *)DRAMK_RUN_ADDRESS, u4DramkLength);
        if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
        {
            Printf("Read dramk from boot partition failed\n");
            _reset();
            return FALSE;
        }

        ret = MSDC_EMMC_EnterUser(u4SdCh);
        if (ret != (MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK))
        {
            Printf("Exit emmc boot failed\n");
            return FALSE;
        }

        // Step 2 - Execute dramk to init DRAM
        DDR_Initialize();
        ddr_calibrate();

// PAY ATTENTION TO: some emmc switch to 4 bit bus width will failed to read data.
// Printf("---> eMMC Switch to 8bit Buswidth <---\r\n");
#if FAST_BOOT_FOLLOW
        MSDC_SetBusWidth(u4SdCh, 8);
#else
        MSDC_SetBusWidth(u4SdCh, 4);
#endif
    }
    else // SD Card
    {
        // Step 1 - Copy dramk from SD to SRAM
        ret = MSDC_ReadBlock_PIO(u4SdCh, DRAMK_PHY_OFFSET, (UINT32 *)DRAMK_RUN_ADDRESS, u4DramkLength);
        if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
        {
            Printf("Read dramk from boot partition failed\n");
            _reset();
            return FALSE;
        }

        // Step 2 - Execute dramk to init DRAM
        DDR_Initialize();
        ddr_calibrate();
    }

#ifdef __AndroidM__
    if (1 == u4BootMode)
    {
        Printf("Now! Load TrustZone......\r\n");
        ret = MSDC_ReadBlock_PIO(u4SdCh, 0x400000, (UINT32 *)0x100000, 0x100000);
        if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
        {
            Printf("Read Trustzone failed ...\n");
            {
                _reset();
                return FALSE;
            }
        }
    }
#endif

    if (MSDC_Is_eMMC_Card(u4SdCh) && (u4SdCh == MSDC_CH1))
    {
        ret = MSDC_ReadBlock_PIO(u4SdCh, DATA_ZONE_OFFSET + DATAZONE_PARTITION_OFFSET, (UINT32 *)DATAZONE_DRAM_ADDR, DATAZONE_PART_INFO_SIZE);
        if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
        {
            Printf("Read partition Info Failed!\n");
            return FALSE;
        }
    }

    //*********************************************
    // Handle eboot/uboot. Read it and jump to execute it
    //*********************************************
    ret = MSDC_ReadBlock_PIO(u4SdCh, u4EbootFileOffset, (UINT32 *)u4EbootLoadPhyAddr, 512);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read uboot args failed\r\n");
        return FALSE;
    }
    args_va = ((upgrade_arg_t *)(u4EbootLoadPhyAddr + 32));
    Printf("upgrade_mode1:%d logo:%x arm2:%x dtb:%x version:%x boot:%x metazone:%x\r\n",
           args_va->upgrade_mode, args_va->logo_size, args_va->arm2_size, args_va->dtb_size, args_va->version, args_va->boot_misc_size, args_va->metazone_size);
    set_upgrade_mode(args_va->upgrade_mode);
    if (args_va->upgrade_mode == BOOT_SD_UPGRADE)
    {
        boot_misc_addr = 0x200000 + args_va->logo_size + args_va->arm2_size + args_va->dtb_size;
        boot_misc_size = args_va->boot_misc_size;
        metazone_addr = boot_misc_addr + args_va->boot_misc_size;
        metazone_size = args_va->metazone_size;
    }
    else
    {
        partitionread *part = get_part_info_by_name("boot_misc");
        if (NULL == part)
        {
            Printf("Read boot_misc failed\n");
            return FALSE;
        }
        boot_misc_addr = (UINT32)(part->u8PartitionStartAddr);
        if (part->u8RealDataSize > 0)
            boot_misc_size = (UINT32)(part->u8RealDataSize);
        else
            boot_misc_size = (UINT32)(part->u8PartitionSize);

        part = get_part_info_by_name("metazone");
        if (NULL == part)
        {
            Printf("Read metazone failed\n");
            return FALSE;
        }
        metazone_addr = (UINT32)(part->u8PartitionStartAddr);
        metazone_size = MTZ_PARTITION_SIZE / 2;
    }

    boot_misc_size = ALIGN(boot_misc_size, 512);
    Printf("Now! Start Load boot_Misc ddr:%x deviceaddr:%x......\r\n", CONFIG_ARGS_START, boot_misc_addr);
    Printf("Now! Start Load boot_Misc size:%x......\r\n", boot_misc_size);
    ret = MSDC_ReadBlock_PIO(u4SdCh, boot_misc_addr, (UINT32 *)CONFIG_ARGS_START, boot_misc_size);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read boot_misc failed\r\n");
        return FALSE;
    }
    check_rsv();

    Printf("Load metazone ddr:%x .....\r\n", metazone_addr);
    Printf("Load metazone size:%x......\r\n", metazone_size);
    mmc_load_metazone(u4SdCh, metazone_addr, metazone_size);

    Metazone_Init();

    //check upgrade mode from metazone before boot arm2
    check_mtz_upg_mode();

    if (args_va->upgrade_mode == BOOT_NO_UPGRADE)
    {
        mmc_boot_arm2(u4SdCh);
    }
    if (BOOT_NO_UPGRADE == args_to_arm2->upgrade_mode)
        mmc_boot_tz(u4SdCh);
    Printf("Now! Start Load uboot......\r\n");

    ret = MSDC_ReadBlock_PIO(u4SdCh, u4EbootFileOffset, (UINT32 *)u4EbootLoadPhyAddr, 0x80000);
    if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
    {
        Printf("Read uboot failed ...\n");
#if SUPPORT_UBOOT_BACKUP
        if (u4SdCh == MSDC_CH1) // only emmc support backup
        {
            Printf("Try read from backup...\n");
            ret = MSDC_ReadBlock_PIO(u4SdCh, u4EbootFileOffset + 0x80000 + 512 * 10, (UINT32 *)u4EbootLoadPhyAddr, 0x80000);
            if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
            {
                Printf("Read uboot failed 2...\n");
                _reset();
                return FALSE;
            }
            bIsFirstUboot = FALSE;
        }
        else
#endif
        {
            _reset();
            return FALSE;
        }
    }
#if SUPPORT_UBOOT_BACKUP
    if (u4SdCh == MSDC_CH1)
    {
        for (i = 0; i < (0x80000 / 4 - 1); i = i + 4)
            u4Checksum += ((UINT32 *)u4EbootLoadPhyAddr)[i];

        u4Checksum = ~u4Checksum;

        Printf("calculate checksum is 0x%08x \n", u4Checksum);
        Printf("read checksum is 0x%08x \n", ((UINT32 *)u4EbootLoadPhyAddr)[0x80000 / 4 - 1]);

        if (u4Checksum != ((UINT32 *)u4EbootLoadPhyAddr)[0x80000 / 4 - 1])
        {
            if (bIsFirstUboot)
            {
                ret = MSDC_ReadBlock_PIO(u4SdCh, u4EbootFileOffset + 0x80000 + 512 * 10, (UINT32 *)u4EbootLoadPhyAddr, 0x80000);
                if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
                {
                    Printf("Read uboot failed 3 ...\n");

                    _reset();
                    return FALSE;
                }
                bFirstUbootError = TRUE;
            }
            else
            {
                ret = MSDC_ReadBlock_PIO(u4SdCh, u4EbootFileOffset, (UINT32 *)u4EbootLoadPhyAddr, 0x80000);
                if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
                {
                    Printf("Read uboot failed 4...\n");

                    _reset();
                    return FALSE;
                }
                bBackUpUbootError = TRUE;
            }
            u4Checksum = 0;
            for (i = 0; i < (0x80000 / 4 - 1); i = i + 4)
                u4Checksum += ((UINT32 *)u4EbootLoadPhyAddr)[i];

            u4Checksum = ~u4Checksum;

            Printf("BackUp calculate checksum is 0x%08x...\n", u4Checksum);
            Printf("BackUp read checksum is 0x%08x...\n", ((UINT32 *)u4EbootLoadPhyAddr)[0x80000 / 4 - 1]);
            if (u4Checksum != ((UINT32 *)u4EbootLoadPhyAddr)[0x80000 / 4 - 1])
            { // Main and backup are error ,we still need jump to uboot
                //_reset();
                // return FALSE;
                // Jump to uboot better than hang here
                ret = MSDC_ReadBlock_PIO(u4SdCh, u4EbootFileOffset, (UINT32 *)u4EbootLoadPhyAddr, 0x80000);
                if (ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
                {
                    Printf("Read uboot failed 5...\n");

                    _reset();
                    return FALSE;
                }
                ((FUNC_CALL)(u4EbootJumpPhyAddr))();
            }

            if (bFirstUbootError)
            {
                // write data in u4EbootJumpPhyAddr to first uboot
                Printf("write to main uboot...\n");
                MSDC_WriteBlock_PIO(u4SdCh, u4EbootFileOffset, (UINT32 *)u4EbootLoadPhyAddr, 0x80000);
            }
            else if (bBackUpUbootError)
            {
                // write data in u4EbootJumpPhyAddr to backup uboot
                Printf("write to backup uboot...\n");
                MSDC_WriteBlock_PIO(u4SdCh, u4EbootFileOffset + 0x80000 + 512 * 10, (UINT32 *)u4EbootLoadPhyAddr, 0x80000);
            }
        }
    }
#endif

#ifdef __AndroidM__
    if (0 == u4BootMode)
    {
        Printf("Uboot/LK Start...\r\n");
        ((FUNC_CALL)(u4EbootJumpPhyAddr))();
    }
    else if (1 == u4BootMode)
    {
        Printf("Boot With Trustzone Support\n");
        Printf("Trustzone Start...\r\n");
        /*parameter must ready here*/
        nw_entry_arg_t_ptr nw_entry_arg = (nw_entry_arg_t_ptr)(0x201000);
        nw_entry_arg->args1 = 0;
        nw_entry_arg->args2 = 0;
        nw_entry_arg->args3 = 0;

        trustzone_jump_v7(0x6F00000, (unsigned int)nw_entry_arg, sizeof(nw_entry_arg_t));
    }
    else
    {
        Printf("BootMode is Wrong, Please Check Your DataZone Config\n");
        return FALSE;
    }
#else
    // Jump to uboot
    ((FUNC_CALL)(u4EbootJumpPhyAddr))();
#endif
}
// add for USB EP test

#if 0

extern void Launch();

UINT32 sdmmc_loader(UINT32 u4SdCh)
{

    UINT32 i = 0;
    UINT32 ret = 0;
    UINT32 u4ReadOffset = 0,u4EndAddr = 0;
    UINT8 *pEbootAddr = 0;
    BOOL bRet = FALSE;
    IMAGE_DESCRIPTOR *eboot_des = NULL;
    UINT32 jumpAdrr = 0;


    
    UINT32 u4SDAddr = 0, u4ImageSize = 0;
     UINT32 *pbImageBuf = 0;
    

    UINT32 u4ZoneIndex,u4BegIndex,u4EndIndex;

    UINT8 *pDramkStart = (UINT8 *)(&_dramk_loader_start);
    UINT8 *pDramkEnd = (UINT8 *)(&_dramk_loader_end);
    UINT32 u4DramkLength = ALIGN((pDramkEnd - pDramkStart), 512); 
    //UINT32 u4DramkLength = ALIGN((_dramk_loader_len), 512); 
    //UINT32 u4DramkLength = (UINT32)(_dramk_loader_len); 
    if (u4DramkLength > DRAMK_MAX_SIZE)
    {
        Printf("dramk size error\r\n");
        u4DramkLength = DRAMK_MAX_SIZE;
    }


    Printf("Init SD%x Card\r\n", MSDC_CH_INDEX(u4SdCh));

#ifdef config_TARGET_REALCHIP
    // Set clock source ,TODO
    //MSDC_WRITE32(0x3801C, 0x1);
    //MSDC_WRITE32(0xC4, 0xFFFFFFFF);
    //MSDC_WRITE32(0xA8, 0xFFFFFFFF);

    // Setting for SD_V33_18_SW0/1/2 Issue, make IO Voltage is 3.3V for SD2.0 Spec
    MSDC_WRITE32(0x308, (MSDC_READ32(0x308) | 0xD0000000));
    MSDC_WRITE32(0x80, (MSDC_READ32(0x80) | 0x001C0000));
    MSDC_WRITE32(0xEC, (MSDC_READ32(0xEC) & 0xFFE3FFFF));
#endif
    
    if(!SDInitializeHardware(u4SdCh))
    {
        return FALSE;
    }
    Printf("Init SD%x Card Succuss\r\n", MSDC_CH_INDEX(u4SdCh));

    Dump_Card_Type(u4SdCh, 3);

    //***************************************
    // Handle Dramk. Execute dramk for enable DRAM
    //***************************************


    // if it is emmc, dramk in boot partition 
    if (MSDC_Is_eMMC_Card(u4SdCh))
    {
        // Step 1 - Copy dramk from eMMC boot partition to SRAM    
        ret = MSDC_EMMC_EnterBootMode1(u4SdCh);
        if(ret != MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK)
        {
            Printf("Enter emmc boot mode failed");
            return FALSE;
        }
        ret = MSDC_ReadBlock_PIO(u4SdCh, DRAMK_PHY_OFFSET, (UINT32 *)DRAMK_RUN_ADDRESS, u4DramkLength);
        if(ret != (MSDC_LASTERROR_READBLOCK | MSDC_LASTERROR_OK))
        {
            Printf("Read dramk from boot partition failed");
            return FALSE;
        }

        ret = MSDC_EMMC_ExitBootMode1(u4SdCh);
        if(ret != (MSDC_LASTERROR_EMMCBOOTMODE | MSDC_LASTERROR_OK))
        {
            Printf("Exit emmc boot mode failed");
            return FALSE;
        }
        
        // Step 2 - Execute dramk to init DRAM
          DDR_Initialize();

        //SDInitializeHardware(u4SdCh);
        // TODO: Fix switch to 4 bit bus width error
        MSDC_SetBusWidth(u4SdCh, 4);
    }
    else // SD Card
    {
        // Step 1 - Copy dramk from SD to SRAM
        MSDC_SetBusWidth(u4SdCh, 4);
        MSDC_ReadBlock_PIO(u4SdCh, DRAMK_PHY_OFFSET, (UINT32 *)DRAMK_RUN_ADDRESS, u4DramkLength);

        // Step 2 - Execute dramk to init DRAM
          DDR_Initialize();
    }

    //**************************************************
    // Handle Datazone. Get eboot setting from datazone 
    //**************************************************
    Printf("IMG++.\r\n");
    Printf("IMG++.\r\n");
    Printf("IMG++.\r\n");

#if 0

    MSDC_ReadBlock_PIO(u4SdCh, SDADRR_UIMAGE, (UINT32 *)PA_UIMAGE, SIZE_UIMAGE);
    Printf("IMG.\r\n");
    Printf("IMG.\r\n");
    Printf("IMG.\r\n");

    MSDC_ReadBlock_PIO(u4SdCh, SDADRR_INITRD,(UINT32 *)PA_INITRD, SIZE_INITRD);
    Printf("INITR.\r\n");
    Printf("INITR.\r\n");
    Printf("INITR.\r\n");
    Printf("INITR.\r\n");

    //pEbootAddr = (UINT8 *)(0X8F963000-0X8C000000);//(eboot_des->dwLoadPhyAddr  - 0X1000000);
    //Printf("Load Eboot...(sd)pEbootAddr=0x%x\r\n",pEbootAddr);
    //jumpAdrr = (UINT32)(pEbootAddr+0X1000);//eboot_des->dwJumpPhyAddr - 0X1000000 ;
    jumpAdrr = (UINT32)(PA_UIMAGE);
    
    //*((UINT32*)(0xF000C004)) = 0xE2;

    //Printf("Jump to 0x%x. [%x %x %x %x]\r\n",jumpAdrr,((UINT8*)jumpAdrr)[0], 
    //                    ((UINT8*)jumpAdrr)[1],((UINT8*)jumpAdrr)[2],((UINT8*)jumpAdrr)[3]);
    Printf("lunch %x \r\n",jumpAdrr);
    Printf("lunch %x \r\n",jumpAdrr);
    Printf("lunch %x \r\n",jumpAdrr);
    ((FUNC_CALL)jumpAdrr)();
    //Launch();
    while(1);

#endif 
     u4SDAddr = 0x509400;
     pbImageBuf = (UINT32 *)0x2800;
     u4ImageSize = 0x4D7;


     if (u4ImageSize % 512)
     {
         u4ImageSize = (u4ImageSize / 512 + 1) * 512;
     }
     MSDC_ReadBlock_PIO(u4SdCh, u4SDAddr, pbImageBuf, u4ImageSize);
     Printf("AC8317.dtb. u4SDAddr:%x pbImageBuf:%x u4ImageSize:%x\r\n", u4SDAddr, pbImageBuf, u4ImageSize);
     
     u4SDAddr = 0x709400;
     pbImageBuf = (UINT32 *)0x8000;
     u4ImageSize = 0x4912C4;
     if (u4ImageSize % 512)
     {
         u4ImageSize = (u4ImageSize / 512 + 1) * 512;
     }
     MSDC_ReadBlock_PIO(u4SdCh, u4SDAddr, pbImageBuf, u4ImageSize);
     Printf("Image. u4SDAddr:%x pbImageBuf:%x u4ImageSize:%x\r\n", u4SDAddr, pbImageBuf, u4ImageSize);
    
     u4SDAddr = 0xC09400;
     pbImageBuf = (UINT32 *)0x700000;
     u4ImageSize = 0xF06543;
     if (u4ImageSize % 512)
     {
         u4ImageSize = (u4ImageSize / 512 + 1) * 512;
     }
     MSDC_ReadBlock_PIO(u4SdCh, u4SDAddr, pbImageBuf, u4ImageSize);
     Printf("initrd.img.gz. u4SDAddr:%x pbImageBuf:%x u4ImageSize:%x\r\n", u4SDAddr, pbImageBuf, u4ImageSize);
    
     Launch();
     while(1);


    // Read Datazone for Addresses
    /*MSDC_ReadBlock_PIO(u4SdCh, DATA_ZONE_OFFSET, (UINT32 *)DATAZONE_ADDRESS, 512);

    eboot_des = ((IMAGE_DESCRIPTOR*)(DATAZONE_ADDRESS + 0x4C));
    Printf("eboot_img_des:\n");
    Printf("\tdwLoadAddress:0x%08x\n", eboot_des->dwLoadAddress);
    Printf("\tdwLoadPhyAddr:0x%08x\n", eboot_des->dwLoadPhyAddr);
    Printf("\tdwJumpAddress:0x%08x\n", eboot_des->dwJumpAddress);
    Printf("\tdwJumpPhyAddr:0x%08x\n", eboot_des->dwJumpPhyAddr);
    Printf("\tdwStartAddr:0x%08x\n", eboot_des->dwStartAddr);
    Printf("\tdwdwTtlLen0x%08x\n", eboot_des->dwTtlLen);*/
#if 0
    if (!IsValidDecriptor(eboot_des))
    {
        Printf("Not Valid image descriptor:0x%x\n", u4SdCh);
        return FALSE;
    }
#endif
    u4ReadOffset = eboot_des->dwStartAddr ;
    u4EndAddr =  u4ReadOffset + eboot_des->dwTtlLen;
    pEbootAddr = (UINT8 *)eboot_des->dwLoadPhyAddr;

    Printf("Now! Load uboot......\r\n");

    //*********************************************
    // Handle eboot/uboot. Read it and jump to execute it
    //*********************************************

    MSDC_ReadBlock_PIO(u4SdCh,u4ReadOffset,(UINT32 *)pEbootAddr,eboot_des->dwTtlLen);

    // Jump to uboot
    ((FUNC_CALL)(eboot_des->dwJumpPhyAddr))();
}



Launch    
    ldr r1, =0x1388
    ldr r2, =0x2800
    ldr r3, =0x0
    ldr r4, =0x0
    ldr r5, =0x0
    ldr r6, =0x0
    ldr r7, =0x0
    ldr r8, =0x0
    ldr r9, =0x0
    ldr r10, =0x0
    ldr r11, =0x0
    ldr r12, =0x0
    ldr pc, =0x8000

    END

#endif
