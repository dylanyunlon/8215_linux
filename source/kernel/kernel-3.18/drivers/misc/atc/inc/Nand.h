#ifndef _NAND_H_
#define _NAND_H_

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Macro Definition
//------------------------------------------------------------------------------
//#define NFI_SECTOR_SIZE 1024
#define MAGIC_NUMBER         (0x26598088)
#define BLOCK_NOT_MAPPED     (0xFFFF)
#define MAX_BLOCKS_MAPPING   (8192)
#define HEADER_SECTOR_OFFSET (10)
#define HEADER_SECTOR_ID     (DATAZONE_START_SECTOR + HEADER_SECTOR_OFFSET)
#define MBR_SECTOR_SIZE      (512)


#define BADBLOCKMARK         (0)
#define MAX_CHIP_CNT         (2)
#ifdef CHIP_VER_MT8530
#define MAX_DATA_SIZE        (4096)
#define MAX_SPARE_SIZE       (224)
#endif
#if defined(CHIP_VER_AC83XX)
#define MAX_DATA_SIZE        (8192)
#define MAX_SPARE_SIZE       (448)
#endif
#define MAX_PAGES_PER_BLK    (128)
#define DEFAULT_TIMEOUT      (0x000FFFFF)
#define INTR_TIMEOUT         (0xF)
#ifdef CHIP_VER_MT8530
#define FDM_SIZE_6           (6)
#define FDM_SIZE_8           (8)
#define ECC_FDM_SIZE         (6)
#endif

#if defined(CHIP_VER_AC83XX)
//#define FDM_SIZE_6           ( (NFI_SECTOR_SIZE==512) ?  9: 9 )
//#define FDM_SIZE_8            ( (NFI_SECTOR_SIZE==512) ? 8 : 8 )
//#define ECC_FDM_SIZE          ( (NFI_SECTOR_SIZE==512) ? 9: 9 )

#define FDM_BYTES           8
#define FDM_ECC_BYTES       8
#define FDM2_BYTES          9
#define FDM2_ECC_BYTES      9

#endif
// nand flash command set
#define CMD_RD_1ST           (0x00)
#define CMD_RD_2ND           (0x30)
#define CMD_RD_2ND_HALF      (0x01)    // only for 512 bytes page-size
#define CMD_RD_SPARE         (0x50)    // only for 512 bytes page-size
#define CMD_RD_ID            (0x90)
#define CMD_RESET            (0xFF)
#define CMD_PROG_1ST         (0x80)
#define CMD_PROG_2ND         (0x10)
#define CMD_ERASE_1ST        (0x60)
#define CMD_ERASE_2ND        (0xD0)
#define CMD_RD_STATUS        (0x70)

//------------------------------------------------------------------------------
// Struct Definition
//------------------------------------------------------------------------------

typedef struct {
    UINT16    u2ManCode;       /*Manufacturer Code*/
    UINT16    u2DevCode;       /*Device Code*/
    UINT16    u2ThirdCode;
    UINT16    u2FourthCode;
} NAND_ID_T;

typedef struct {
    NAND_ID_T tID;
    UINT16 	  u2BlockNum;
    UINT16 	  u2PagesPerBlk;
    UINT16 	  u2DataSize;
    UINT16    u2SpareSize;
    UINT8     uChipCnt;       /*Chip enable pin count*/
    UINT8     uIOWidth;
    UINT8     uColNum;
    UINT8     uRowNum;
    UINT8     uBadOffset;     /*Bad block page offset*/
    UINT8     uBadPos;        /*Bad block info position*/
} NandDevice_T;

typedef struct
{
	BOOL      fgIntrMode;
	BOOL      fgHwEccEn;
	BOOL      fgAHBMode;
	BOOL      fgAutoFMT;
} NFIConfig_T;

typedef struct
{
    DWORD     u4MagicNum;
    DWORD     u4Checksum;
    DWORD     u4MaxBlockMappings;
} BlockIndexTableHeader;

typedef struct
{
    BlockIndexTableHeader *pHeader;
    SECTOR_ADDR sectorId;   // store the logical sector id for block index table
    BOOL        fValid;
    BOOL        fChanged;
} HeaderInfo;


//------------------------------------------------------------------------------
// Low Level NAND Flash Driver Interface
//------------------------------------------------------------------------------
BOOL NFI_Init(void);
BOOL NFI_AllocBuff(BOOL isInit);
BOOL NFI_CustomSet(void);
NandDevice_T* NFI_GetDeviceInfo(void);
void NFI_Reset(void);
void NFI_DeviceConfig(NFIConfig_T *ConfigInfo);
BOOL NFI_WriteFifo(UINT8* data, UINT16 length, BOOL bAHBmode);
BOOL NFI_ReadFifo(UINT8* data, UINT16 length, BOOL bAHBmode);
BOOL NFI_ReadFlashID(UINT32 chip, NAND_ID_T *DeviceID);
BOOL ProgramPage (UINT32 PageIndex, UINT8 * Data, UINT8 * Spare, UINT32 SpareLen, UINT16 Chip);
BOOL ReadPage (UINT32 PageIndex, UINT8 * Data, UINT8 * Spare, UINT32 SpareLen, UINT16 Chip);
BOOL ProgramSpare(UINT32 PageIndex, UINT8 *Spare, UINT32 SpareLen, UINT16 Chip);
BOOL ReadSpare(UINT32 PageIndex, UINT8 * Data, UINT32 SpareLen, UINT16 Chip);
BOOL EraseBlock(UINT32 BlockIndex, UINT16 Chip);
BOOL ReadPartialPage(UINT32 PageIndex, UINT8 * Data, UINT16 Chip,UINT32 u4Ratio);

#if __cplusplus
}
#endif

#endif
