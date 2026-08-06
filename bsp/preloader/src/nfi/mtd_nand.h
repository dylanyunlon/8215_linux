#include "x_typedef.h"
#define _SUPPORT_WP_

typedef struct {
	UINT16		m_physical_blk[128*1024];
	UINT8		m_reserved[8];
  UINT32  version;
} Nand_ImageInfo_S;



#define TEMP_MEM_SIZE 4096

extern INT32 i4NFBInit(void);
extern BOOL MTD_Init(UINT32* _pTempMemPtr);

extern INT32 i4NFBPartitionRead(UINT32* p4MemPtr, UINT32 u4Addr, UINT32 u4Length, UINT8 *pu4TmpPageBuf);
extern UINT32 u4NFBDWRDRead(UINT32 u4Addr);

extern BOOL fgCheckValidBlock(UINT32 u4PgIdx, UINT32* p4TmpData);

#define MTD_BOOT_LOADER     0
#define MTD_BINARY          1
#define MTD_MISC_DATA       2
#define MTD_BD_APP_DATA     3
#define MTD_APP_CFG_DATA    4
#define MTD_CPS_MGR_DATA    5
#define MTD_BINDING_DATA    6
#define MTD_BD_JAR_FILE     7
#define MTD_KEY_BLOCK_1     8
#define MTD_KEY_BLOCK_2     9
#define MTD_LOG_STORG       10
#define MTD_IRS_STORG       11
#define MTD_NAND_INFO       12
#define MTD_RESERVED        13
#define NVM_CUST_EXT_RES    14
#define MTD_ADV_RW_AREA     15
#define MTD_DIAG_SRV_LOG    16
#define MTD_APP_CFG_NAND    17
#define MTD_LOADER_BACKUP   18
#define MTD_LOADER_BINARY   19
#define MTD_LOADER_FA       20
#define MTD_LOADER_PWRCURVE 21
#define MTD_LOADER_PARAM    22
#define MTD_LOGO_BINARY     23
#define MTD_LOGO_PARAM			24
#define MTD_MAX_ITEM        32



#define NVM_NOR  1
#define NVM_NAND 2
#define NVM_PART_BOOT  1
#define NVM_PART_STORG 2
#define NVM_PART_LOG   3



#define NFB_BOOT_LOADER_HEADER_LENGTH 0x200
#define DEFAULT_MTD_BOOT_LOADER_SIZE 0x400000



#define NFI_LOCK_SEMA()  BIM_GETHWSemaphore(HSMPHE_NAND,0)
#define NFI_UNLOCK_SEMA() BIM_ReleaseHWSemaphore(HSMPHE_NAND)



typedef struct
{
    UINT32 u4Item;
    UINT32 u4FlashType;
    UINT32 u4PartBase;
    UINT32 u4Addr;
    UINT32 u4Size;
}NE_TABLE_ITEM_T;



#define MEM_BUF_MTD_PART_NFB_TBL	0x04000000	// size:0x00800000 (8M)
#define MEM_BUF_MTD_PT_NVM_TBL		0x04800000	// size:0x00800000 (8M)

#define MEM_BUF_MTD_IMG_INFO		0x03000000
#define MEM_BUF_MTD_TMP_MEM_PTR		0x05000000	// size:0x00800000 (8M)

#define MEM_BUF_SECURE_BOOT_1		0x06000000	// size:0x03C00000 (60M)
#define ALIGN_4_HIGH(x)	((x+3)&0xFFFFFFFC)



#define USE_BLRELOCATE              0x233
#define DATAZONE_SIZE               0x200
#define DAZONE_ITEM_OFFSET          0x4C
#define RELBL_START                 0xBC              //Eboot Start 0x108,      MEM_BUF_SECURE_BOOT_1 Start 0x4C   
#define RELPHYEBOOT_START           0xC0
#define RELPHYBAKUPEBOOT_START      0xC4

typedef struct _IMAGE_DESCRIPTOR
{
	UINT32 dwLoadAddress;	// reserve
	UINT32 dwLoadPhyAddr;	// dram address to load eboot
	UINT32 dwJumpAddress;	// reserve
	UINT32 dwJumpPhyAddr;	// entry point of eboot
	UINT32 dwStartAddr;		// eboot start address
	UINT32 dwTtlLen;		// eboot size
}IMAGE_DESCRIPTOR;




