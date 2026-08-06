#include "dvp_init.h"
#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/x_typedef.h>
#include<asm/io.h>
//#include "ac83xx_memory.h"
#undef MEMRSV_PHY_TO_VIRT
#define MEMRSV_PHY_TO_VIRT(x) (x)


#include <pinmux.h>
#include <ac83xx_pinmux_table.h>
#include <ac83xx_gpio_pinmux.h>

#define DVP_CODE_IN_NAND
#define DVP_SERVO_PATCH1
#define DVP_SERVO_PIN_CONFIG
#define BSP_DEMO

#define ARGS_DVD_CODE_IN_NAND              (1 << 0)
#define ARGS_SERVO_PATCH1                       (1 << 1)
#define ARGS_DVP_SERVO_PIN_CONFIG       (1 << 2)
#define ARGS_BSP_NODVDMEMORY               (1 << 3)
#define ARGS_BSP_HAS_CUSTOM_FOUR        (1 << 4)
#define ARGS_BSP_DEMO                               (1 << 5)
#define ARGS_BSP_OTHERS                           (1 << 6)

extern BOOL DVDLoad(DWORD pArgument, DWORD DVDBase, DWORD AudioBase, DWORD ShareInfoBase);

#define IsALIGN128K(val) (((val)&((1 << 17) - 1))? FALSE : TRUE)
static BOOL DVPImage_ReadEx(UINT8 *pBuffer, UINT32 u4Offset, UINT32 u4Length);

#define WriteRegAP(addr, data)  ((*(volatile UINT32 *)(addr)) = (UINT32)(data))
#define ReadRegAP(addr)     (*(volatile UINT32 *)(addr))

#define IO_BASE_VA  0xF0000000

extern unsigned long long g_dvpPartitonAddr;
extern unsigned long long g_dvpPartitonSize;


DWORD GetShareInfoAddress()
{
  return (DVP_SHAREINFO_OFFSET_IN_RESERVED_DRAM);
}

BOOL DVDInit(VOID)
{
  DWORD dwArgument = 0;
  DWORD dwData0 = 0;
  DWORD dwData1 = 0;

  #ifdef DVP_CODE_IN_NAND
  dwArgument |= ARGS_DVD_CODE_IN_NAND;
  #endif
  #ifdef DVP_SERVO_PATCH1
  dwArgument |= ARGS_SERVO_PATCH1;
  #endif
  #ifdef DVP_SERVO_PIN_CONFIG
  dwArgument |= ARGS_DVP_SERVO_PIN_CONFIG;
  #endif
  #ifndef BSP_NODVDMEMORY
  dwArgument |= ARGS_BSP_NODVDMEMORY;
  #endif

  #if 1     //def BSP_DEMO
  dwArgument |= ARGS_BSP_DEMO;
  #else
  dwArgument |= ARGS_BSP_OTHERS;
  #endif
  dwData0 = RESERVED_DVD_BASE_VA;
  dwData1 = RESERVED_AUDIO_BASE_VA;
  printf("[DVDInit]ready to AC83XXDVDLoad, data0 = 0x%x, data1 = 0x%x\r\n", dwData0, dwData1);
  DVDLoad(dwArgument, dwData0, dwData1,GetShareInfoAddress());
  return TRUE;

}




static BOOL DVPImage_ReadEx(UINT8 * pBuffer, UINT32 u4Offset, UINT32 u4Length)
{
      int ret = 0;
	char buf1[16] = {0};
	char buf2[16] = {0};
	char *szValAddr = (char *)malloc(17);
	char *szValSize = (char *)malloc(17);
	memset(szValAddr,0,17);
	memset(szValSize,0,17);
	printf("+DVPImage_Read\r\n");


#ifdef CONFIG_BOOT_MMC
	#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
	char *argv[6] = {"mmc", "read", "0"};
	#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
	char *argv[6] = {"mmc", "read", "2"};
	#endif

	sprintf(buf1, "%x", pBuffer);
	argv[3] = buf1;
    //argv[4] = "0x2B1000";   //0x43600000/512byte
    //argv[5] = "0x1000";     //0x400000/512byte
    uitostr_hex(szValAddr,(unsigned int)(g_dvpPartitonAddr/512));
	uitostr_hex(szValSize,(unsigned int)(g_dvpPartitonSize/512));
	argv[4] = szValAddr;
	argv[5] = szValSize;
#else
	char *argv[6] = {"nand", "read"};
    sprintf(buf1, "%x", pBuffer);
	argv[2] = buf1;
	argv[3] = "dvp";
    // try to use real size for nand
	//sprintf(buf2, "%x", u4Length);
	sprintf(buf2, "%x", g_dvpPartitonSize);
	argv[4] = buf2;
#endif

    printf("DVP_Read: mem addr: 0x%x, part addr: 0x%x, part size: 0x%x\r\n", pBuffer, g_dvpPartitonAddr, g_dvpPartitonSize);


#ifdef CONFIG_BOOT_MMC
    if ( 0 != do_mmcops(NULL, 0, 6, argv))
#else
    if ( 0 != do_nand(NULL, 0, 5, argv))
#endif
    {
        printf("ERROR: Unable to read dvp Image\r\n");
		free(szValAddr);
		free(szValSize);
        return FALSE;
    }
   else
   {
        printf("OK: read dvp Image success \r\n");
		free(szValAddr);
		free(szValSize);
	    return TRUE;
   }

}

BOOL DVPImage_Config()
{
    UINT8 *pMemAddr = DVP_RAM_IMAGE_START_ADDR;
    UINT32 u4RiscAddr, u4RiscSize, u4MainRiscAddr, u4MainRiscSize;
    UINT32 u4Tmp, u4LoaderSize;
    BYTE a1,a2,a3,a4;
    UINT32 u4Tmp_1 = 0;
    UINT32 u4Tmp_2 = 0;

    printf("+DVPImage_Config\r\n");

    /*Step 1: Move Target BIN excpet ROMCODE to DRAM*/
    pMemAddr = DVP_RAM_IMAGE_START_ADDR;
    if(!DVPImage_ReadEx(pMemAddr, 0, DVP_TARGET_LENGTH_LIMIT))
    {
        printf("DVPImage_Read ERROR: Read Target Bin error\r\n");
        return FALSE;
    }

    /*Step 2: Parser DVP image information form targetbin*/
	// Get RISC fastlogo code start address
    bHiByte(wHiWord(u4RiscAddr)) = 0;
    bLoByte(wHiWord(u4RiscAddr)) = Buf_GetData8(pMemAddr, CODE_INFO_START_ADDRESS + 8); //u4CodeInfo[8];
    bHiByte(wLoWord(u4RiscAddr)) = Buf_GetData8(pMemAddr, CODE_INFO_START_ADDRESS + 7);
    bLoByte(wLoWord(u4RiscAddr)) = Buf_GetData8(pMemAddr, CODE_INFO_START_ADDRESS + 6);

	// Get RISC fastlogo code length
    bHiByte(wHiWord(u4RiscSize)) = 0;
    bLoByte(wHiWord(u4RiscSize)) = Buf_GetData8(pMemAddr, CODE_INFO_START_ADDRESS + 11);
    bHiByte(wLoWord(u4RiscSize)) = Buf_GetData8(pMemAddr, CODE_INFO_START_ADDRESS + 10);
    bLoByte(wLoWord(u4RiscSize)) = Buf_GetData8(pMemAddr, CODE_INFO_START_ADDRESS + 9);
    printf("DVPImage_Config--Fastlogo info: start address = 0x%x, loader length = 0x%x\r\n", u4RiscAddr, u4RiscSize);
    //Get Main RISC code start address
	u4Tmp = Buf_GetData32(pMemAddr, u4RiscAddr + 0x28); //Fast logo total length
	u4MainRiscAddr = u4RiscAddr + u4Tmp + 8;

    //Get Main RISC code length
	u4MainRiscSize = Buf_GetData32(pMemAddr, u4MainRiscAddr - 8);

   printf("DVPImage_Config--MainRISC Addr = 0x%x, MainRISC Length = 0x%x\r\n"
        , u4MainRiscAddr, u4MainRiscSize);

    if((u4MainRiscAddr + u4MainRiscSize) > DVP_TARGET_LENGTH_LIMIT)
    {
        printf("Target Bin is too large\r\n");
        return FALSE;
    }

    /*Step 3: Prepare Fast Logo code*/
	//move fast logo LOADER chunk to DRAMB
    printf("DVPImage_Config--DramB address = 0x%x\r\n", DRAMB_START_ADDR);
	memcpy(DRAMB_START_ADDR, Buf_GetPos8(pMemAddr, u4RiscAddr), u4RiscSize);

	//move fast logo CODE_DRAM&DATA_DRAM chunk to DRAMA
	u4LoaderSize = Buf_GetData32(pMemAddr, u4RiscAddr + 0x20);
	u4RiscSize = Buf_GetData32(pMemAddr, u4RiscAddr + 0x28);
	printf("DVPImage_Config--DramA address = 0x%x, u4LoaderSize:0x%x, u4RiscSize:0x%x\r\n", RESERVED_DVD_BASE_VA, u4LoaderSize, u4RiscSize);
	memcpy(RESERVED_DVD_BASE_VA, Buf_GetPos8(pMemAddr, u4RiscAddr + u4LoaderSize), (u4RiscSize - u4LoaderSize));

	/*Step 5: Config 8032 run in dram*/
	//u4Tmp = OALVAtoPA(DVP_RAM_IMAGE_START_ADDR);
	u4Tmp = DVP_RAM_IMAGE_START_ADDR;//virt_to_phys(DVP_RAM_IMAGE_START_ADDR);
	printf("[PXX][DVPImage_Config]u4Tmp = 0x%x\n",u4Tmp);
	if(!IsALIGN128K(u4Tmp))
	{
		printf("ERROR: DVP code start address must align to 128K. \r\n");
		return FALSE;
	}
	printf("DVPImage_Config--8032 start address = 0x%x\r\n", u4Tmp);

    WriteRegAP((0xF0000000 + 0X3A0008),u4Tmp >> 17); //8032 code start address in dram, value = (physic offset / 128K)
    u4Tmp_1 = ReadRegAP((0xF0000000 + 0X3A0008));
    WriteRegAP((0xF0000000 + 0X3A0010),0x3); //set to 3 means 8032 code is in dram, if 0 means in flash
    u4Tmp_2 = ReadRegAP((0xF0000000 + 0X3A0010));

    GPIO_MultiFun_Set(PIN_2_GPIO2,DVD_T8032_UP0_SEL);
    GPIO_MultiFun_Set(PIN_3_GPIO3,DVD_T8032_UP0_SEL);

    pMemAddr = DVP_RAM_IMAGE_START_ADDR;
	printf("[DVP]DVPImage_Config--8032 code: %x %x %x %x %x %x %x %x\r\n"
		, pMemAddr[0], pMemAddr[1], pMemAddr[2], pMemAddr[3]
		, pMemAddr[4], pMemAddr[5], pMemAddr[6], pMemAddr[7]);

    printf("[DVP]DVPImage_Config--8032 reg val(0X3A0008):0x%x  ---  val(0X3A0010):0x%x\r\n", u4Tmp_1, u4Tmp_2);

	printf("-DVPImage_Config SUCCESS\r\n");

    return TRUE;

}

DWORD DVPMemRevPhy2Virt(DWORD addr)
{
    return MEMRSV_PHY_TO_VIRT(addr);
}

