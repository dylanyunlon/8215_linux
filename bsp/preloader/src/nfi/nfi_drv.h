#ifndef _NFI_DRV_H
#define _NFI_DRV_H

#define SECTOR_BYTES 512


typedef enum 
{
   NFI_SUCCESS = 0,
   NFI_CHECKSUM_ERROR = -1,
   NFI_HEADER_ID_NO_FOUND = -2,
   NFI_NO_GOOD_BLOCK = -3,
   NFI_TIMEOUT = -4,
   AES_DECRYPT_ERROR = -5
} NFIEntranceType;



#define  IO_8BITS   0x0000
#define  IO_16BITS  0x0001


/* Data Structures */
typedef struct _NFIType
{
   //UINT16   IOInterface;     /* IO_8BITS or IO_16BITS */
   UINT16   pageSize;        /* 512, 2048 */
   UINT16   spareSize;
   UINT16   addressCycle;   
   UINT16   pageShift;
} NFI_MENU;


typedef enum {
    NFI_60BIT_ECC,
    NFI_24BIT_ECC,    
} NFI_Type_t;


typedef struct _BOOTLHeader_
{
   char ID1[12];
   char version[4];
   UINT32 length;
   UINT32 startAddr;
   UINT32 checksum;
   char ID2[8];
   NFI_MENU  NFIinfo;
   UINT16   pagesPerBlock;
   UINT16   totalBlocks;
   UINT16  blockShift;
   UINT16  linkAddr[6];   
   UINT16  lastBlock;
} BOOTL_HEADER;


typedef struct _NFI_DRV_
{
    NFI_Type_t type;
    UINT32 UsingRandomsizer;
    void (*Init)(void);
    void (*Config)(NFI_MENU *);
    STATUS_E (*ReadPage)(UINT32 *, UINT32 *,UINT32 ,BOOL);
    STATUS_E (*WritePage)(UINT32 *, UINT32 *,UINT32);
    STATUS_E (*EraseBlock)(UINT32, UINT32);
    NFI_MENU *pConfigs;
    NFI_MENU *pCurConfig;
    UINT32 configSize;
}NFI_DRV_t;

#define REPLICATION_NUMBER  8

#define ENABLE_ECC              (1 << 0)
#define ENABLE_RAMDOMSIZER      (1 << 1)

/* Prototypes */
void NFI2_Init(void);
void NFI2_Reset(void);
void NFI2_Config(NFI_MENU *input);
STATUS_E NFI2_ReadPage(UINT32 *pu4Buf, UINT32 * pu4Spare,UINT32 u4PgIdx, BOOL fgEccCheck);
STATUS_E NFI2_EraseBlock(UINT32 u4BlockIdx, UINT32 pagesPerBlock);
STATUS_E NFI2_WritePage(UINT32 *buf, UINT32 *spare, UINT32 page);
INT32 NFI2_ReadSpare(UINT32 *pu4Buf, UINT32 u4PageSize, UINT32 u4AddrLen, UINT32 u4PgIdx);
INT32 NFI2_ReadBlockInfo(NFI_MENU *prInput, UINT32 u4PgIdx, UINT32* p4TmpData);

extern NFI_MENU _rNFI3Retrials[];
extern NFI_MENU _rNFI2Retrials[];
extern UINT32 _u4Config_no;

#define NFI_Wait(condition_expression, timeout)     while( (condition_expression) && (--timeout) )

#endif // _NFI_H
