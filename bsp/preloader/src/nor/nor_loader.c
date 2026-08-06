#include "x_typedef.h"
#include "x_printf.h"
#include "boot.h"
#include "partition.h"
#include "reserved_memory.h"
#include "ddr_includes.h"
#include "x_util.h"
#include "metazone_inter.h"

#define BIT(n) (1U << (n))
#define FLASHIF_BASE (0xF0011000)

#define REG_SF_CMD          (*(volatile UINT32 *)(FLASHIF_BASE + 0x0000))
#define REG_SF_CNT          (*(volatile UINT32 *)(FLASHIF_BASE + 0x0004))
#define REG_SF_RDSR         (*(volatile UINT8 *)(FLASHIF_BASE + 0x0008))
#define REG_SF_RDATA        (*(volatile UINT8 *)(FLASHIF_BASE + 0x000C))
#define REG_SF_RADR0        (*(volatile UINT8 *)(FLASHIF_BASE + 0x0010))
#define REG_SF_RADR1        (*(volatile UINT8 *)(FLASHIF_BASE + 0x0014))
#define REG_SF_RADR2        (*(volatile UINT8 *)(FLASHIF_BASE + 0x0018))
#define REG_SF_RADR3        (*(volatile UINT8 *)(FLASHIF_BASE + 0x00C8))

#define REG_SF_WDATA         (*(volatile UINT8 *)(FLASHIF_BASE + 0x001C))
#define REG_SF_PRGDATA0      (*(volatile UINT8 *)(FLASHIF_BASE + 0x0020))
#define REG_SF_PRGDATA1      (*(volatile UINT8 *)(FLASHIF_BASE + 0x0024))
#define REG_SF_PRGDATA2      (*(volatile UINT8 *)(FLASHIF_BASE + 0x0028))
#define REG_SF_PRGDATA3      (*(volatile UINT8 *)(FLASHIF_BASE + 0x002C))
#define REG_SF_PRGDATA4      (*(volatile UINT8 *)(FLASHIF_BASE + 0x0030))
#define REG_SF_PRGDATA5      (*(volatile UINT8 *)(FLASHIF_BASE + 0x0034))

#define REG_SF_SHREG0        (*(volatile UINT8 *)(FLASHIF_BASE + 0x0038))
#define REG_SF_SHREG1        (*(volatile UINT8 *)(FLASHIF_BASE + 0x003C))
#define REG_SF_SHREG2        (*(volatile UINT8 *)(FLASHIF_BASE + 0x0040))
#define REG_SF_SHREG3        (*(volatile UINT8 *)(FLASHIF_BASE + 0x0044))
#define REG_SF_SHREG4        (*(volatile UINT8 *)(FLASHIF_BASE + 0x0048))
#define REG_SF_SHREG5        (*(volatile UINT8 *)(FLASHIF_BASE + 0x004C))

#define REG_SF_CFG1         (*(volatile UINT32 *)(FLASHIF_BASE + 0x0060))
#define REG_SF_CFG2         (*(volatile UINT32 *)(FLASHIF_BASE + 0x0064))
#define MTK_NOR_WR_CUSTOM_OP_EN (1 << 4)

#define REG_SF_CFG3         (*(volatile UINT32 *)(FLASHIF_BASE + 0x00b4))
#define MTK_NOR_DISABLE_WREN    (1 << 7)
#define MTK_NOR_DISABLE_SR_POLL (1 << 5)

#define REG_SF_DUAL         (*(volatile UINT32 *)(FLASHIF_BASE + 0x00CC))
#define MTK_NOR_4B_ADDR      (1 << 4)
#define MTK_NOR_DUAL_READ    (1 << 0)

#define REG_SF_WRPROT       (*(volatile UINT32 *)(FLASHIF_BASE + 0x00C4))
#define REG_SF_INTRSTUS     (*(volatile UINT8 *)(FLASHIF_BASE + 0x00A8))
#define MTK_NOR_REG_MISC    (*(volatile UINT32 *)(0xF0008000 + 0xA8))
#define MTK_NOR_REG_AXI     (*(volatile UINT32 *)(0xF0008000 + 0x784))

#define FLASH_MAPPING_ADDR (0xC0000000)
#define DATA_ZONE_OFFSET 0x000100000
#define DATAZONE_DRAM_ADDR CONFIG_DATAZONE_START
#define DRAMK_RUN_ADDRESS 0xF4008000
#define DRAMK_PHY_OFFSET 0x00008000
#define DRAMK_MAX_SIZE 0x8000
#define MTZ_PARTITION_SIZE 0x20000
#define FLASHB_MAPPING_ADDR (0xC0000000)
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

typedef struct _IMAGE_DESCRIPTOR
{
#ifdef CONFIG_SECURITY_UPGRADE
    char tags[16];
#endif
    UINT32 dwLoadAddress;
    UINT32 dwLoadPhyAddr;
    UINT32 dwJumpAddress;
    UINT32 dwJumpPhyAddr;
    UINT32 dwStartAddr;
    UINT32 dwTtlLen;
} IMAGE_DESCRIPTOR;

extern UINT32 _dramk_loader_start;
extern UINT32 _dramk_loader_end;
extern UINT32 _dramk_loader_len;

static volatile ARGS_TO_ARM2_P *args_to_arm2 = NULL;
#ifdef ATC_AB_PARTITION_SUPPORT
UINT32 ab_slot = 0;
#endif
int g_upg_mode = 0;
int g_partIndex = 0;
uint8 enable_four_byte = 0;
static void Sflash_Reg_SetReadWritAddr0(UINT8 addrValue)
{
    REG_SF_RADR0 = addrValue;
}

static void Sflash_Reg_SetReadWritAddr1(UINT8 addrValue)
{
    REG_SF_RADR1 = addrValue;
}

static void Sflash_Reg_SetReadWritAddr2(UINT8 addrValue)
{
    REG_SF_RADR2 = addrValue;
}

static void Sflash_Reg_SetReadWritAddr3(UINT8 addrValue)
{
    REG_SF_RADR3 = addrValue;
}

static UINT32 Sflash_Hal_WriteFlashCmd(UINT8 commandValue, UINT8 pollingValue)
{
    UINT32 timeout = 0;
    UINT8 val;

    REG_SF_CMD = commandValue;
    while (timeout++ < 50000)
    {
        val = REG_SF_CMD;
        if ((val & pollingValue) == 0)
            return 1;
    }
    return 0;
}

static inline UINT8 Sflash_Reg_ReadFlashData(void)
{
    return (UINT8)REG_SF_RDATA;
}

static void flash_mdelay(UINT32 dly)
{
    volatile UINT32 i;
    while (dly--)
    {
        i = 2000;
        while (i--)
            ;
    }
}

static void flash_writeable(BOOL enable)
{
    REG_SF_PRGDATA5 = enable ? 0x06 : 0x04;
    REG_SF_CNT = 0x8;
    if (Sflash_Hal_WriteFlashCmd(0x04, 0x04) == 0U)
    {
        Printf("flash writeable set failed\n");
    }
}

static void flash_wait_ready(void)
{
    UINT8 status;
    UINT32 i = 0x500000;

    while (--i)
    {
        if (Sflash_Hal_WriteFlashCmd(0x02, 0x02) == 0U)
            break;

        flash_mdelay(10);
        status = REG_SF_RDSR;
        if ((status & 0x01) == 0)
            break;
    }

    if (i == 0)
        Printf("flash ready timeout\n");
}

static void flash_nor_write_buffer_disable(void)
{
    UINT32 timeout = 0;
    UINT32 val;

    val = REG_SF_CFG2;
    val &= ~1U;
    REG_SF_CFG2 = val;

    while (timeout++ < 50000)
    {
        val = REG_SF_CFG2;
        if ((val & 1U) == 0)
            break;
    }
}

static void flash_4ByteAddressModeSet(BOOL enable)
{
    UINT32 mode = REG_SF_DUAL;
    if (enable)
        mode |= MTK_NOR_4B_ADDR;
    else
        mode &= ~MTK_NOR_4B_ADDR;
    REG_SF_DUAL = mode;
    flash_mdelay(1);
}

static void flash_get_status(UINT8 cmd)
{
    UINT8 status;

    REG_SF_PRGDATA5 = cmd;
    REG_SF_PRGDATA4 = 0;
    REG_SF_PRGDATA3 = 0;
    REG_SF_PRGDATA2 = 0;
    REG_SF_CNT = 0x20;

    if (Sflash_Hal_WriteFlashCmd(0x04, 0x04) == 0U)
    {
        Printf("get device status failed\n");
        return;
    }

    status = REG_SF_SHREG0;
    Printf("flash status[%x]: 0x%x\n", cmd, status);
}

static void flash_get_device(void)
{
    UINT8 deviceID2 = 0, deviceID1 = 0, menuID = 0;

    REG_SF_PRGDATA5 = 0x9F;
    REG_SF_PRGDATA4 = 0;
    REG_SF_PRGDATA3 = 0;
    REG_SF_PRGDATA2 = 0;
    REG_SF_CNT = 0x20;

    if (Sflash_Hal_WriteFlashCmd(0x04, 0x04) == 0U)
    {
        Printf("get device failed\n");
        return;
    }

    deviceID2 = REG_SF_SHREG0;
    deviceID1 = REG_SF_SHREG1;
    menuID = REG_SF_SHREG2;
    Printf("flash info: %x %x %x\n", menuID, deviceID1, deviceID2);
}

static void flash_Enter4ByteAddressMode(void)
{
    REG_SF_PRGDATA5 = 0xB7;
    REG_SF_CNT = 0x08;

    if (Sflash_Hal_WriteFlashCmd(0x4, 0x4) == 0U)
        Printf("4byte mode fail\n");
}

static void flash_hw_init(void)
{
    UINT8 val;

    REG_SF_WRPROT = 0x30;
    REG_SF_INTRSTUS = 0x0;

    val = (UINT8)REG_SF_CFG2;
    val |= MTK_NOR_WR_CUSTOM_OP_EN;
    REG_SF_CFG2 = val;

    val = (UINT8)REG_SF_CFG3;
    val |= (MTK_NOR_DISABLE_WREN | MTK_NOR_DISABLE_SR_POLL);
    REG_SF_CFG3 = val;
}

static void flash_config_clksrc(UINT8 clksrc)
{
    UINT32 u4Clk;
	Printf("nor clksrc=%d \n",clksrc);
    u4Clk = (*(volatile UINT32 *)(0xF0000000 + 0x10));
    u4Clk &= ~(0x7U << 6);
    u4Clk |= (clksrc << 6);
    (*(volatile UINT32 *)(0xF0000000 + 0x10)) = u4Clk;
}

static void flash_config_pinmux(void)
{
    UINT32 pinmux;
    pinmux = (*(volatile UINT32 *)(0xF0000000 + 0x54));
    pinmux &= ~(0x1U << 8);
    pinmux |= (0x1U << 8);
    (*(volatile UINT32 *)(0xF0000000 + 0x54)) = pinmux;
}

void flash_DualModeSet(bool enable)
{
	uint8 mode;
	mode = REG_SF_DUAL;

	if (enable){
		mode |=  MTK_NOR_DUAL_READ;
		//REG_SF_CFG1 = 0x1;
		Printf("dual mode enable \n");
	} else {
		mode &=  ~MTK_NOR_DUAL_READ;
		Printf("dual mode disable \n");
	}

	if (enable_four_byte)
		REG_SF_PRGDATA3 = 0x3C;
	else
		REG_SF_PRGDATA3 = 0x3B;

	REG_SF_DUAL = mode;
}

static void NorFlashRead(UINT32 offset, void *buf, UINT32 len)
{
    UINT8 *dest = (UINT8 *)buf;
    UINT32 chunk;
	UINT32 max_len = 0x200;
	UINT32 i = 0;
	Printf("nor read off=0x%x len=0x%x \n",offset, len);
	flash_nor_write_buffer_disable();
#if 1

	UINT32 length =  len/4;
	const UINT32 *s = (UINT32 *)(FLASHB_MAPPING_ADDR + offset);
	UINT32 *data = (UINT32 *)buf;
	while(i < length){
		data[i] = s[i];
		i++;
	}
#else
    while (len > 0)
    {
        chunk = (len > max_len) ? max_len : len;

        if (chunk > 1)
        {
            //REG_SF_CNT = 0x08;
            Sflash_Reg_SetReadWritAddr0((UINT8)(offset & 0xFF));
            Sflash_Reg_SetReadWritAddr1((UINT8)((offset >> 8) & 0xFF));
            Sflash_Reg_SetReadWritAddr2((UINT8)((offset >> 16) & 0xFF));
            Sflash_Reg_SetReadWritAddr3((UINT8)((offset >> 24) & 0xFF));
            //flash_4ByteAddressModeSet(1);

			for (i = 0; i < chunk; i++)
			{
				if (Sflash_Hal_WriteFlashCmd(0x81, 0x1) == 0U)
				{
					Printf("data read fail \n");
					return;
				}

				dest[i] = Sflash_Reg_ReadFlashData();
			}
        }
        else
        {
            //REG_SF_CNT = 0x08;
            Sflash_Reg_SetReadWritAddr0((UINT8)(offset & 0xFF));
            Sflash_Reg_SetReadWritAddr1((UINT8)((offset >> 8) & 0xFF));
            Sflash_Reg_SetReadWritAddr2((UINT8)((offset >> 16) & 0xFF));
            Sflash_Reg_SetReadWritAddr3((UINT8)((offset >> 24) & 0xFF));
            //flash_4ByteAddressModeSet(1);

            if (Sflash_Hal_WriteFlashCmd(0x1, 0x1) == 0U)
            {
                Printf("data read fail\n");
                return;
            }

            dest[0] = Sflash_Reg_ReadFlashData();
        }

        offset += chunk;
        dest += chunk;
        len -= chunk;
    }
#endif
}

static flash_test(void)
{
#define LEN 0x20
	char data[LEN];
	UINT32 offset = 0x0;
	UINT32 len = LEN;
	NorFlashRead(offset,data,len);
	DumpBytes2(data,len);
}
static void NorFlashInit(void)
{
    Printf("NorFlash init start\n");
    flash_config_clksrc(4);
    flash_config_pinmux();
    flash_hw_init();
    flash_nor_write_buffer_disable();
    flash_get_device();
	flash_writeable(1);
    flash_Enter4ByteAddressMode();
    flash_get_status(0x05);
    flash_get_status(0x35);
	flash_4ByteAddressModeSet(1);
	flash_DualModeSet(1);

}

partitionread *readpartitioninfofromflash(void)
{
    partitionread *ppartread = (partitionread *)(DATAZONE_DRAM_ADDR + 512);
    partitionread *pcurpartition = ppartread;
    partitionread *pprepartition = pcurpartition;

    while (pcurpartition != NULL)
    {
        if (pcurpartition->u4LastPartition == 1)
        {
            pcurpartition->nextpartition = NULL;
            break;
        }
        pcurpartition = pcurpartition + 1;
        pprepartition->nextpartition = pcurpartition;
        pprepartition = pcurpartition;
    }

    return ppartread;
}

partitionread *get_part_info_by_name(const char *name)
{
    partitionread *p = readpartitioninfofromflash();
    g_partIndex = 0;

    while (p)
    {
        if (strcmp(p->szPartName, name) == 0)
            return p;
        g_partIndex++;
        p = p->nextpartition;
    }

    Printf("ERR: Cannot find partition %s\n", name);
    return NULL;
}

static BOOL nor_load_partition_info(void)
{
    NorFlashRead(DATA_ZONE_OFFSET + DATAZONE_PARTITION_OFFSET, (void *)DATAZONE_DRAM_ADDR, DATAZONE_PART_INFO_SIZE);
    return TRUE;
}

static BOOL nor_load_boot_misc(UINT32 boot_misc_addr, UINT32 boot_misc_size)
{
    NorFlashRead(boot_misc_addr, (void *)CONFIG_ARGS_START, boot_misc_size);
    return TRUE;
}

static BOOL nor_load_metazone(UINT32 metazone_addr, UINT32 metazone_size)
{
    BOOL bRet = TRUE;
    const RSV_MEM_T *rsv = get_rsv_mem_by_name("metazone");
    UINT32 mtz1_addr, mtz2_addr;

    if (rsv == NULL)
    {
        Printf("get metazone reserved_memory failed\n");
        return FALSE;
    }

    mtz1_addr = (UINT32)(rsv->start_addr);
    mtz2_addr = mtz1_addr + (UINT32)(rsv->size) / 2;

    NorFlashRead(metazone_addr, (void *)mtz1_addr, metazone_size);
    NorFlashRead(metazone_addr, (void *)mtz2_addr, metazone_size);

    return bRet;
}

static BOOL nor_boot_arm2(void)
{
    BOOL bRet = TRUE;
    partitionread *p = NULL;
    const RSV_MEM_T *rsv = NULL;
    UINT32 arm2_phy, logo_phy, vba_phy;
    UINT32 arm2_size, logo_size, vba_size;
    UINT32 arm2_addr, logo_addr, vba_addr;
    char *arm2_str = "arm2";
    char *system_str = "system";
    char *logo_str = "logo";
    char *vba_str = "vba";

#ifdef ATC_AB_PARTITION_SUPPORT
    char *a_str = "arm2_a";
    char *b_str = "arm2_b";
    char *a_system = "system_a";
    char *b_system = "system_b";
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
#endif

    p = get_part_info_by_name(arm2_str);
    if (NULL == p)
        return FALSE;
    arm2_addr = (UINT32)p->u8PartitionStartAddr;
    arm2_size = (UINT32)((p->u8RealDataSize > 0) ? p->u8RealDataSize : p->u8PartitionSize);
    arm2_size = ALIGN(arm2_size, 512);

    p = get_part_info_by_name(logo_str);
    if (NULL == p)
        return FALSE;
    logo_addr = (UINT32)p->u8PartitionStartAddr;
    logo_size = (UINT32)((p->u8RealDataSize > 0) ? p->u8RealDataSize : p->u8PartitionSize);
    logo_size = ALIGN(logo_size, 512);

    p = get_part_info_by_name(vba_str);
    if (NULL == p)
        return FALSE;
    vba_addr = (UINT32)p->u8PartitionStartAddr;
    vba_size = (UINT32)((p->u8RealDataSize > 0) ? p->u8RealDataSize : p->u8PartitionSize);

    rsv = get_rsv_mem_by_name("arm2");
    if (NULL == rsv)
        return FALSE;
    arm2_phy = (UINT32)(rsv->start_addr);
    args_to_arm2 = (void *)(UINT32)arm2_phy;
    arm2_phy += 0x40000;

    rsv = get_rsv_mem_by_name("animation");
    if (NULL == rsv)
        return FALSE;
    logo_phy = (UINT32)(rsv->start_addr) + (UINT32)(rsv->size) - MRF_LOAD_OFFSET;

    rsv = get_rsv_mem_by_name("vba");
    if (NULL == rsv)
        return FALSE;
    vba_phy = (UINT32)(rsv->start_addr);

    NorFlashRead(arm2_addr, (void *)arm2_phy, arm2_size);
    NorFlashRead(logo_addr, (void *)logo_phy, logo_size);
    NorFlashRead(vba_addr, (void *)vba_phy, vba_size);

    args_to_arm2->jump_instr = 0xEA00FFFE;
    args_to_arm2->dram_size = TCMGET_CHANNELA_SIZE() * 1024 * 1024;
    args_to_arm2->dtb_status = STATUS_WAIT_LOAD;
    args_to_arm2->upgrade_mode = (g_upg_mode == 1 || g_upg_mode == 2) ? BOOT_SD_UPGRADE : BOOT_NO_UPGRADE;

#ifdef ATC_AB_PARTITION_SUPPORT
    args_to_arm2->ab_slot = ab_slot;
#endif

    p = get_part_info_by_name(system_str);
    if (NULL == p)
        return FALSE;
    args_to_arm2->system_index = g_partIndex;

    u4ARM2Start(arm2_phy - 0x40000);
    return bRet;
}

static BOOL nor_boot_tz(void)
{
    BOOL bRet = TRUE;
    partitionread *p = NULL;
    const RSV_MEM_T *rsv = NULL;
    UINT32 tz_phy, kernel_phy, dtb_phy;
    UINT32 tz_size, kernel_size, dtb_size;
    UINT32 tz_addr, kernel_addr, dtb_addr;
    char *tz_str = "trustzone";
    char *kernel_str = "kernel";
    char *dtb_str = "dtb";

#ifdef ATC_AB_PARTITION_SUPPORT
    char *a_tz = "trustzone_a";
    char *b_tz = "trustzone_b";
    char *a_ker = "kernel_a";
    char *b_ker = "kernel_b";
    char *a_dtb = "dtb_a";
    char *b_dtb = "dtb_b";
    if (ab_slot)
    {
        tz_str = b_tz;
        kernel_str = b_ker;
        dtb_str = b_dtb;
    }
    else
    {
        tz_str = a_tz;
        kernel_str = a_ker;
        dtb_str = a_dtb;
    }
#endif

    p = get_part_info_by_name(tz_str);
    if (NULL == p)
        return FALSE;
    tz_addr = (UINT32)p->u8PartitionStartAddr;
    tz_size = (UINT32)((p->u8RealDataSize > 0) ? p->u8RealDataSize : p->u8PartitionSize);
    tz_size = ALIGN(tz_size, 512);

    p = get_part_info_by_name(kernel_str);
    if (NULL == p)
        return FALSE;
    kernel_addr = (UINT32)p->u8PartitionStartAddr;
    kernel_size = (UINT32)((p->u8RealDataSize > 0) ? p->u8RealDataSize : p->u8PartitionSize);
    kernel_size = ALIGN(kernel_size, 512);

    p = get_part_info_by_name(dtb_str);
    if (NULL == p)
        return FALSE;
    dtb_addr = (UINT32)p->u8PartitionStartAddr;
    dtb_size = (UINT32)((p->u8RealDataSize > 0) ? p->u8RealDataSize : p->u8PartitionSize);
    dtb_size = ALIGN(dtb_size, 512);

    rsv = get_rsv_mem_by_name("trustzone");
    if (NULL == rsv)
        return FALSE;
    tz_phy = (UINT32)(rsv->start_addr);

    kernel_phy = KERNEL_LOAD_ADDR;
    dtb_phy = FDT_LOAD_ADDR;

    NorFlashRead(dtb_addr, (void *)dtb_phy, dtb_size);
    args_to_arm2->dtb_status = STATUS_LOAD_READY;
    NorFlashRead(tz_addr, (void *)tz_phy, tz_size);
    NorFlashRead(kernel_addr, (void *)kernel_phy, kernel_size);

    while (STATUS_MODIFY_END != args_to_arm2->arm_dtb_status)
        ;

    {
        void (*initTrustZone)(int zero, int arch, UINT32 parms, void *theKernel);
        void *theKernel = (void *)KERNEL_LOAD_ADDR;
        initTrustZone = (void (*)(int, int, UINT32, void *))tz_phy;
        initTrustZone(0, (int)MACH_TYPE_AC83XX, FDT_LOAD_ADDR, theKernel);
    }

    return bRet;
}

UINT32 RunInNor(void)
{
    IMAGE_DESCRIPTOR *eboot_des = NULL;
    upgrade_arg_t *args_va = NULL;
    UINT32 boot_misc_addr = 0;
    UINT32 boot_misc_size = 0;
    UINT32 metazone_addr = 0;
    UINT32 metazone_size = 0;
    UINT32 u4EbootFileOffset = 0;
    UINT32 u4EbootLoadPhyAddr = 0;
    UINT32 u4EbootJumpPhyAddr = 0;
	Printf("RunInNor\n");

    UINT8 *pDramkStart = (UINT8 *)(&_dramk_loader_start);
    UINT8 *pDramkEnd = (UINT8 *)(&_dramk_loader_end);
    UINT32 u4DramkLength = ALIGN((UINT32)(pDramkEnd - pDramkStart), 512);

    NorFlashInit();

    Printf("NorFlash: read datazone header\n");
    NorFlashRead(DATA_ZONE_OFFSET, (void *)DRAMK_RUN_ADDRESS, 512);

    eboot_des = (IMAGE_DESCRIPTOR *)(DRAMK_RUN_ADDRESS + 0x3C);

    u4EbootFileOffset = eboot_des->dwStartAddr;
    u4EbootLoadPhyAddr = eboot_des->dwLoadPhyAddr;
    u4EbootJumpPhyAddr = eboot_des->dwJumpPhyAddr;

    Printf("NorFlash: read eboot args\n");
    NorFlashRead(u4EbootFileOffset, (void *)u4EbootLoadPhyAddr, 512);
    args_va = (upgrade_arg_t *)(u4EbootLoadPhyAddr + 32);
    set_upgrade_mode(args_va->upgrade_mode);

    if (!nor_load_partition_info())
    {
        Printf("NorFlash: load partition info failed\n");
        return FALSE;
    }

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
        if (part == NULL)
            return FALSE;
        boot_misc_addr = (UINT32)part->u8PartitionStartAddr;
        boot_misc_size = (UINT32)((part->u8RealDataSize > 0) ? part->u8RealDataSize : part->u8PartitionSize);

        part = get_part_info_by_name("metazone");
        if (part == NULL)
            return FALSE;
        metazone_addr = (UINT32)part->u8PartitionStartAddr;
        metazone_size = MTZ_PARTITION_SIZE / 2;
    }

    boot_misc_size = ALIGN(boot_misc_size, 512);
    Printf("NorFlash: load boot_misc\n");
    if (!nor_load_boot_misc(boot_misc_addr, boot_misc_size))
        return FALSE;

    check_rsv();

    Printf("NorFlash: load metazone\n");
    if (!nor_load_metazone(metazone_addr, metazone_size))
        return FALSE;

    Metazone_Init();
    //check_mtz_upg_mode();

    if (args_va->upgrade_mode == BOOT_NO_UPGRADE)
    {
        if (!nor_boot_arm2())
            return FALSE;
    }

    if (BOOT_NO_UPGRADE == args_to_arm2->upgrade_mode)
    {
        if (!nor_boot_tz())
            return FALSE;
    }

    Printf("NorFlash: load uboot\n");
    NorFlashRead(u4EbootFileOffset, (void *)u4EbootLoadPhyAddr, 0x80000);

    ((void (*)(void))u4EbootJumpPhyAddr)();
    return TRUE;
}
