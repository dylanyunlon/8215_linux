#include "x_typedef.h"

#include "nfi2.h"
#include "nfi_drv.h"
#include "mtd_nand.h"
#include "bl_loader.h"
#include "x_printf.h"
#include "x_util.h"
#include "x_hal_io.h"
#include "boot.h"
#include "reserved_memory.h"
#include "partition.h"
#include "bootctrl.h"
#include "ddr_includes.h"
#include "metazone_inter.h"
#define NFI_TYPES_COMBINATION 7
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(x, a) __ALIGN_MASK(x, (typeof(x))(a)-1)

UINT8 *_pTempMemPtr;
UINT32 _u4Config_no = 0;
UINT32 _u4PhyBlockID = 0;
NFI_DRV_t nand_driver;
static volatile ARGS_TO_ARM2_P *args_to_arm2 = NULL;
struct bootloader_message bcb;

NFI_MENU _rNFI2Retrials[NFI_TYPES_COMBINATION] =
{
        /* sector size, spare size, address cycle, page shift */
        {/*IO_8BITS,*/ 4096, 64, 5, 24}, // 24 bit ECC
        {/*IO_8BITS,*/ 1024, 26, 5, 16}, // 12 bit ECC
        {/*IO_8BITS,*/ 1024, 16, 5, 16}, // 6 bit ECC
        {/*IO_8BITS,*/ 1024, 16, 4, 16}, // 6 bit ECC
        {/*IO_8BITS,*/ 512, 16, 4, 8},     // 6 bit ECC
        {/*IO_8BITS,*/ 512, 16, 4, 16},     // 6 bit ECC
        {/*IO_8BITS,*/ 512, 16, 3, 8},     // 6 bit ECC
};

extern UINT32 _dramk_loader_start;
extern UINT32 _dramk_loader_end;
extern UINT32 _dramk_loader_len;

#define FASTBOOT_UPG_MODE 0x11
#define COPY_UPG_MODE 0x12
int g_upg_mode = 0;

const char _szBLID1[12] = "BOOTLOADER!";
const char _szBLID2[8] = "NFIINFO";

BOOTL_HEADER _rBLHeader;
#define DATA_ZONE_NAND_OFSET 0x00100000        // 1MB 32k --->change to 64Kz
#define DATA_ZONE_NAND_OFSET_BAK 0x00300000    // 3MB 32k --->change to 64Kz
#define DATA_ZONE_DRAM_ADDR CONFIG_DATAZONE_START
#define DATA_ZONE_SIZE 0x80000 // include ori datazone, add bcb and partitioninfo,must match with partition info
#define E_BOOT_ITEM_OFSET (DATA_ZONE_NAND_OFSET + 0x4C)
// use 60 bit or 24 bit or not.
#define IO_BASE_ADDRESS1 0xf0000000L

#define DRAMK_SDRAM_ADDR 0xF4008000 // SRAM 32K
#define SDRAM_ADDR_START 0xF4000000
#define PLR_HEAD_SIZE 0x200 // file_offset(31.5k) + bootloader_header(512bytes)
#define TMP_SDRAM_ADDR 0xF4007000

#ifdef ATC_AB_PARTITION_SUPPORT
UINT32 ab_slot = 0;
#endif
UINT32 g_partIndex = 0;
BOOL bbt_found = FALSE;

#define CHECK_BLOCKS       1000
#define BBT_ENTRY_BITS     2
#define BBT_BYTES          ((CHECK_BLOCKS * BBT_ENTRY_BITS + 7) / 8)
#define BMAP_BYTES         ((CHECK_BLOCKS + 7) / 8)

static UINT8 g_bad_block_bmap[BMAP_BYTES];
static UINT32 g_bad_block_count = 0;

static inline int is_block_bad(UINT32 block)
{
    if (block >= CHECK_BLOCKS)
        return 0;

    return (g_bad_block_bmap[block >> 3] >> (block & 0x7)) & 0x1;
}

#if 0
void DumpBytes2(BYTE *pBuf, int size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        if (!(i % 16))
            Printf("\r\n%Xh: ", i);

        Printf("%x ", *pBuf);

        pBuf++;
    }
    Printf("\r\n");
}
#endif

typedef void (*FUNC_CALL)(void);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:         BootLoader2HeaderVerification

Description:      Get NandFlash Inforamtion through Bootloader Head

Arguments:      pu4TmpBuf  - [in] For Get NandFlash Information Buffer
                        u4PgIdx       - [in] page index
                        iNo               - [in]configure Number

Return Value:    0:success

-------------------------------------------------------------------*/
INT8 BootLoader2HeaderVerification(UINT32 *pu4TmpBuf, UINT32 u4PgIdx, INT8 iNo)
{

    INT32 i = 0, index = 0;
    NFI_MENU *pCurConfig = &(nand_driver.pConfigs[iNo]);
    STATUS_E ret;
    volatile UINT32 *pu4Source = pu4TmpBuf;
    BOOTL_HEADER *pTempHeader;
    volatile UINT32 *p4Source, *p4Destination;
    nand_driver.pCurConfig = pCurConfig;

    for (i = 0; i < ((pCurConfig->pageSize) >> 2); i++)
    {
        pu4Source[i] = 0x00;
    }
    nand_driver.Config((NFI_MENU *)&(nand_driver.pConfigs[iNo]));

    while (index < 0x3) // 3 page is enough, first block must be a good block, ROM code limitation
    {
        ret = nand_driver.ReadPage((UINT32 *)pu4Source, NULL, u4PgIdx, FALSE);
        if (ret == S_TIMEOUT)
        {
            return -3; // read timeout, change config immediately
        }
        else if (ret == S_ECC_UNCORRECT_ERR) // ECC detectable error, 2 bit error
        {
            index++; // read again, read disturb cause ECC error can be taken as good data, wrong config will be dismissed by signature check
                     // return -3;
        }
        else if ((ret == S_DONE) || (S_ECC_CORRECTABLE_ERR == ret)) // correct or ECC correctable
            break;
        else // unknown, exception
            return -3;
    }

    pTempHeader = (BOOTL_HEADER *)pu4Source;
    index = 0;
    // Reading all replications
    while (index < REPLICATION_NUMBER)
    {

        // Tracing ID1, BOOTLOADER!
        for (i = 0; i < 12; i++)
        {
            if (pTempHeader[index].ID1[i] != _szBLID1[i])
                break;
        }
        if (i < 12)
        {
            index++;
            continue;
        }

        for (i = 0; i < 8; i++)
        {
            if (pTempHeader[index].ID2[i] != _szBLID2[i])
                break;
        }
        if (i < 8)
        {
            index++;
            continue;
        }

        pCurConfig->pageSize = pTempHeader[index].NFIinfo.pageSize;
        pCurConfig->pageShift = pTempHeader[index].NFIinfo.pageShift;

        if (pCurConfig->spareSize == 16)
        {
            if ((pCurConfig->spareSize * uidiv(pCurConfig->pageSize, SECTOR_BYTES)) != pTempHeader[index].NFIinfo.spareSize)
            {
                index++;
                continue;
            }
        }
        else if (pCurConfig->spareSize > 16)
        { // 26, maximun spare size for 12 correctable ECC bits
            UINT32 temp = (nand_driver.type == NFI_24BIT_ECC) ? 26 : pCurConfig->spareSize;
            if ((temp * uidiv(pCurConfig->pageSize, SECTOR_BYTES)) > pTempHeader[index].NFIinfo.spareSize)
            {
                index++;
                continue;
            }
        }

        // Tracking Nand-flash information
        if (/*(pTempHeader[index].NFIinfo.IOInterface!= pCurConfig->IOInterface) ||*/
            //       (pTempHeader[index].NFIinfo.pageSize != pCurConfig->pageSize) || // pageSize is read from header, not by tryout
            (pTempHeader[index].NFIinfo.addressCycle != pCurConfig->addressCycle)
            //       (pTempHeader[index].NFIinfo.pageShift!=pCurConfig->pageShift) //pageShift is also read from header
        )
        {
            // NFI configuration incorrect
            // Perhaps error bits existing
            index++;
            continue;
        }

        nand_driver.Config((NFI_MENU *)&(nand_driver.pConfigs[iNo]));

        // all ID and PageFormat setting are matched
        p4Source = (volatile UINT32 *)&pTempHeader[index];
        p4Destination = (volatile UINT32 *)&_rBLHeader;
        for (i = 0; i < (sizeof(BOOTL_HEADER) >> 2); i++)
        {
            *p4Destination++ = *p4Source++;
        }

        Printf("index %d ok\r\n", index);
        // while(1);
        return 0;
    }
    // Printf("err\r\n");
    return -1; // no single complete ID string is matched
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       NandFlashInit

Description:    NandFlash Initiliaze

Arguments:      pu4TmpBuf  - [in] For Get NandFlash Information Buffer

Return Value:   0: success

-------------------------------------------------------------------*/
INT8 NandFlashInit(UINT32 *pu4TmpBuf)
{
    INT8 iStatus = 0;
    UINT32 au4PgIdxTabl[] = {0, 64, 128, 256, 512};
    UINT32 u4PgIdx = 0, dwTryLoop = 0;
    _u4Config_no = 0;

    /*1. Set NandFlash driver as 24bit*/
    nand_driver.type = NFI_24BIT_ECC;
    nand_driver.Init = NFI2_Init;
    nand_driver.Config = NFI2_Config;
    nand_driver.ReadPage = NFI2_ReadPage;
    nand_driver.WritePage = NFI2_WritePage;
    nand_driver.EraseBlock = NFI2_EraseBlock;
    nand_driver.pConfigs = _rNFI2Retrials;
    nand_driver.pCurConfig = _rNFI2Retrials;
    nand_driver.configSize = 7;

    /*2. NFI Initiliaze*/
    Printf("Start Init\n");
    nand_driver.Init();

    /*3. Get NandFlash Configure Information : Page Size, Spare size Block Per Page*/
    // loop 1: Trying the correct setting.
    for (dwTryLoop = 0; dwTryLoop < (sizeof(au4PgIdxTabl) / sizeof(au4PgIdxTabl[0])); dwTryLoop++)
    {
        u4PgIdx = au4PgIdxTabl[dwTryLoop];
        _u4Config_no = 0;
        do
        {
            iStatus = BootLoader2HeaderVerification(pu4TmpBuf, u4PgIdx, _u4Config_no);
            Printf("_u4Config_no =%d,status=%d, PageIdx=%d\n", _u4Config_no, iStatus, u4PgIdx);
            // The correct Bootloader ID is found in either of the replications
            if (iStatus == 0)
            {
                break;
            }
            _u4Config_no++;
        } while (_u4Config_no < nand_driver.configSize);

        // Fails in retrieving bootloader header via either NFI combination (NFIRetrials)
        // ID is inexisted from neither replications.
        if (_u4Config_no < NFI_TYPES_COMBINATION)
        {
            Printf("Init Flash Ok _u4Config_no =%d, PageIdx=%d\n", _u4Config_no, iStatus, u4PgIdx);
            break;
        }
    }

    if (dwTryLoop >= (sizeof(au4PgIdxTabl) / sizeof(au4PgIdxTabl[0])))
    {
        Printf("NandFlash Init Failed\n");
    }

    nand_driver.pCurConfig = &(nand_driver.pConfigs[_u4Config_no]);
    return iStatus;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       fgCheckValidBlock

Description:      Block is good or bad

Arguments:      u4PgIdx              - [in] page index

                        pu4TmpPageBuf  - [in]Temp Page Buffer

Return Value:    TRUE:good block FALSE:Bad block

-------------------------------------------------------------------*/
BOOL fgCheckValidBlock(UINT32 u4PgIdx, UINT32 *pu4TmpPageBuf)
{

    UINT32 u4Loop = 0;
    UINT8 Spare[8];

    for (u4Loop = 0; u4Loop < 2; u4Loop++)
    {
        // Read the bad block marking from the first page
        if (S_DONE != nand_driver.ReadPage(pu4TmpPageBuf, (UINT32 *)Spare, u4PgIdx + u4Loop, TRUE))
        {
            Printf("[_fgCheckValidBlock]bad block %d\n", u4PgIdx / _rBLHeader.pagesPerBlock);
            return FALSE;
        }
        else
        { // Read success
            if (Spare[0] != 0xFF)
            {
                Printf("[_fgCheckValidBlock]bad block %d\n", u4PgIdx / _rBLHeader.pagesPerBlock);
                return FALSE;
            }
        }
    }

    if (S_DONE != nand_driver.ReadPage(pu4TmpPageBuf, (UINT32 *)Spare, u4PgIdx + _rBLHeader.pagesPerBlock - 1, TRUE))
    {
        Printf("[_fgCheckValidBlock]bad block %d\n", u4PgIdx / _rBLHeader.pagesPerBlock);
        return FALSE;
    }
    else
    { // Read success
        if (Spare[0] != 0xFF)
        {
            Printf("[_fgCheckValidBlock]bad block %d\n", u4PgIdx / _rBLHeader.pagesPerBlock);
            return FALSE;
        }
    }

    return TRUE;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       i4NFBPartitionRead

Description:      Block is good or bad

Arguments:      p4MemPtr             - [in/out] Mem buffer
                       u4Addr                   -[in]  Logical address start
                       u4Length                -[in] Logical address length
                        pu4TmpPageBuf     - [in]Temp Page Buffer

Return Value:

-------------------------------------------------------------------*/
INT32 i4NFBPartitionRead(UINT32 *p4MemPtr, UINT32 u4Addr, UINT32 u4Length, UINT8 *puTmpPageBuf)
{

    UINT32 u4PageSize = 0, u4FlashPtr = 0, u4Offset = 0, u4PageIndex = 0, u4TmpIdx = 0, u4TmpOfst = 0, u4EndAddr = 0;
    STATUS_E ret = S_UNKNOWN_ERR;
    u4FlashPtr = u4Addr;
    u4EndAddr = u4Addr + u4Length;
    u4PageSize = _rBLHeader.NFIinfo.pageSize;
    UINT32 blksz = (_rBLHeader.pagesPerBlock * _rBLHeader.NFIinfo.pageSize);
    BOOL isBadBlock = FALSE;

    _u4PhyBlockID = 0;

    while (u4FlashPtr < u4EndAddr)
    {
        u4TmpIdx = uidiv(u4FlashPtr, blksz);
        if (u4TmpIdx != _u4PhyBlockID)
        {
            if (bbt_found == TRUE && u4TmpIdx < CHECK_BLOCKS)
            {
                isBadBlock = is_block_bad(u4TmpIdx);
            }
            else
            {
                isBadBlock = !fgCheckValidBlock(u4TmpIdx * _rBLHeader.pagesPerBlock, (UINT32 *)puTmpPageBuf);
            }

            _u4PhyBlockID = u4TmpIdx;
        }
        u4TmpOfst = (uidiv(u4FlashPtr, _rBLHeader.NFIinfo.pageSize)) & (_rBLHeader.pagesPerBlock - 1);
        if (isBadBlock)
        {
            u4FlashPtr += blksz;
            u4EndAddr += blksz;
            continue;
        }
        else
        {
            u4PageIndex = u4TmpIdx * _rBLHeader.pagesPerBlock + u4TmpOfst;
        }
        if (u4FlashPtr & (u4PageSize - 1))
        {
            if (S_DONE != (ret = NFI_ReadPage((UINT32 *)puTmpPageBuf, u4PageIndex, TRUE, FALSE, 0, 0)))
            {
                if (S_DONE != (ret = nand_driver.ReadPage((UINT32 *)puTmpPageBuf, NULL, u4PageIndex, FALSE)))
                    goto end;
            }
            else
            {
                u4Offset = u4FlashPtr & (u4PageSize - 1);
                if (u4EndAddr >= u4FlashPtr + u4PageSize - u4Offset)
                {
                    mem_cpy(p4MemPtr, (UINT32 *)(puTmpPageBuf + u4Offset), u4PageSize - u4Offset);
                    p4MemPtr += ((u4PageSize - u4Offset) >> 2);
                }
                else if (u4EndAddr < u4FlashPtr + u4PageSize - u4Offset)
                {
                    mem_cpy(p4MemPtr, (UINT32 *)(puTmpPageBuf + u4Offset), u4EndAddr - u4FlashPtr);
                    p4MemPtr += ((u4EndAddr - u4FlashPtr) >> 2);
                }

                u4FlashPtr += u4PageSize - u4Offset;
            }
        }
        else
        {

            u4Offset = u4FlashPtr & (u4PageSize - 1);
            if (u4EndAddr < u4FlashPtr + u4PageSize - u4Offset)
            {
                if (S_DONE != (ret = NFI_ReadPage((UINT32 *)puTmpPageBuf, u4PageIndex, TRUE, FALSE, 0, 0)))
                {
                    if (S_DONE != (ret = nand_driver.ReadPage((UINT32 *)puTmpPageBuf, NULL, u4PageIndex, FALSE)))
                    {
                        goto end;
                    }
                }
                mem_cpy((UINT8 *)p4MemPtr, (UINT32 *)(puTmpPageBuf + u4Offset), u4EndAddr - u4FlashPtr);
                p4MemPtr += ((u4EndAddr - u4FlashPtr) >> 2);
            }
            else
            {
                if (S_DONE != (ret = NFI_ReadPage((UINT32 *)p4MemPtr, u4PageIndex, TRUE, FALSE, 0, 0)))
                {
                    if (S_DONE != (ret = nand_driver.ReadPage(p4MemPtr, NULL, u4PageIndex, FALSE)))
                        goto end;
                }
                p4MemPtr += ((u4PageSize - u4Offset) >> 2);
            }
            u4FlashPtr += u4PageSize - u4Offset;
        }
    }
end:
    if (S_DONE != ret)
    {
        return FALSE;
    }
    return TRUE;
}


void parse_bbt_bmap(UINT8 *bbt_data)
{
    UINT32 block;
    UINT32 i;

    for (i = 0; i < BMAP_BYTES; i++)
        g_bad_block_bmap[i] = 0;
    g_bad_block_count = 0;

    for (block = 0; block < CHECK_BLOCKS; block++) {
        UINT32 byte  = block / 4;
        UINT32 shift = (block % 4) * 2;

        UINT8 val = (bbt_data[byte] >> shift) & 0x3;

        // 0x3 = good
        if (val != 0x3)
        {
            g_bad_block_bmap[block >> 3] |= (1 << (block & 0x7));
            g_bad_block_count++;
        }
    }

    Printf("BBT bad block count:%d\n", g_bad_block_count);
}

#define BBT_SCAN_MAX_BLOCKS   16

typedef enum {
    BBT_TYPE_NONE = 0,
    BBT_TYPE_PRIMARY,
    BBT_TYPE_MIRROR
} BBT_TYPE;

typedef struct {
    UINT8 valid;
    UINT8 version;
    BBT_TYPE type;
    UINT32 block;
} BBT_INFO;

BBT_INFO parse_bbt(UINT8 *data, UINT8 *oob, UINT32 blk)
{
    BBT_INFO info = {0};

    if (oob[0]=='B' && oob[1]=='b' && oob[2]=='t' && oob[3]=='0') {
        info.type = BBT_TYPE_PRIMARY;
    }
    else if (oob[0]=='1' && oob[1]=='t' && oob[2]=='b' && oob[3]=='B') {
        info.type = BBT_TYPE_MIRROR;
    }
    else {
        return info;
    }

    info.version = oob[4];
    info.valid = 1;
    info.block = blk;

    return info;
}

void load_bbt(UINT32 *pu4TmpPageBuf)
{
    BBT_INFO bbt0 = {0};
    BBT_INFO ltab = {0};
    BBT_INFO final = {0};
    UINT32 i = 0;
    UINT8 oob_buf[64];

    for (i = 0; i < BBT_SCAN_MAX_BLOCKS; i++) {
        UINT32 blk = _rBLHeader.totalBlocks - 1 - i;
        UINT32 page = blk * _rBLHeader.pagesPerBlock;

        if (nand_driver.ReadPage(pu4TmpPageBuf, (UINT32 *)oob_buf, page, TRUE) != S_DONE)
            continue;

        BBT_INFO cur = parse_bbt((UINT8 *)pu4TmpPageBuf, (UINT8 *)oob_buf, blk);

        if (!cur.valid)
            continue;

        Printf("BBT found: blk=%d type=%d ver=%d\n", blk, cur.type, cur.version);
        if (cur.type == 0) {   // bbt0
            if (!bbt0.valid || (UINT8)(cur.version - bbt0.version) > 0) {
                bbt0 = cur;
            }
        } else {               // ltab
            if (!ltab.valid || (UINT8)(cur.version - ltab.version) > 0) {
                ltab = cur;
            }
        }

        if (bbt0.valid && ltab.valid) {
            Printf("Both BBT0 and LBBT found, stop scan\n");
            break;
        }
    }

    if (bbt0.valid && ltab.valid) {
        if ((UINT8)(bbt0.version - ltab.version) >= 0) {
            final = bbt0;
        } else {
            final = ltab;
        }
    } else if (bbt0.valid) {
        final = bbt0;
    } else if (ltab.valid) {
        final = ltab;
    } else {
        Printf("No valid BBT found!\n");
        return;
    }
    Printf("Use BBT blk=%d ver=%d type=%d\n", final.block, final.version, final.type);
    if (S_DONE != nand_driver.ReadPage((UINT32 *)pu4TmpPageBuf,
                                       NULL,
                                       final.block * _rBLHeader.pagesPerBlock,
                                       TRUE))
    {
        Printf("read blk %d fail\n", final.block);
        return;
    }

    parse_bbt_bmap((UINT8 *)pu4TmpPageBuf);
	bbt_found = TRUE;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:         NandFlashInfoInit

Description:     NandFlash Information Initiliaze

Arguments:      pu4TmpBuf    - [in] Temp buffer


Return Value:

-------------------------------------------------------------------*/
BOOL NandFlashInfoInit(UINT32 *pu4TmpBuf)
{
    /*1 NandFlash Information Initialize*/
    NandFlashInit(pu4TmpBuf);
    load_bbt(pu4TmpBuf);
}

#if 0
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:          FindPhy2Log

Description:      Find Logical Block ID  by Physical Block ID

Arguments:      u4PhyBlockID             - [in] Physical Block ID

Return Value:    Logical Block ID

-------------------------------------------------------------------*/
UINT32 FindPhy2Log(UINT32 u4PhyBlockID)
{
    return u4PhyBlockID;
}
#endif

enum checkpart
{
    DATAZONE = 0,
    BCB
} checkpartname;

static UINT32 get_partition_size(const partitionread *p)
{
    UINT32 size = (p->u8RealDataSize >= 0) ? p->u8RealDataSize : p->u8PartitionSize;
    return ALIGN(size, _rBLHeader.NFIinfo.pageSize);
}

UINT32 check_head_tag(enum checkpart checkpartname, void *buf)
{
    if (checkpartname == DATAZONE)
    {
        if (memcmp((char *)buf + 0x3C, "mboot.nb0", 9) == 0)
            return 0;
    }
    else if (checkpartname == BCB)
    {
        if (memcmp((char *)buf, "BCBHead", 7) == 0)
            return 0;
    }
    return 1;
}

#define DATAZONE_INFO_SIZE 260
UINT32 check_checksum(enum checkpart checkpartname, void *buf)
{
    UINT32 checksum_from_calc = 0;
    UINT32 i = 0;
    UINT32 count = 0;
#define TAG_AND_CHECKSUM_SZIE 20
    if (checkpartname == DATAZONE)
    {
        count = DATAZONE_INFO_SIZE; // 260-->datazone size
        for (i = 0; i < count; i++)
        {
            checksum_from_calc += *((UINT8 *)(buf + i));
        }
        Printf("datazone checksum = %x, (*(UINT32 *)(DATA_ZONE_DRAM_ADDR + DATA_ZONE_SIZE)) = %x\n", checksum_from_calc, (*(UINT32 *)(buf + DATAZONE_INFO_SIZE)));
        if (checksum_from_calc == (*(UINT32 *)(buf + DATAZONE_INFO_SIZE)))
            return 0;
    }
    else if (checkpartname == BCB)
    {
        count = sizeof(struct bootloader_message) - TAG_AND_CHECKSUM_SZIE; // 20--> tag& checksum size
        for (i = 0; i < count; i++)
        {
            checksum_from_calc += *((UINT8 *)(buf + i + TAG_AND_CHECKSUM_SZIE));
        }
        Printf("bcb checksum = %x, (*(UINT32 *)(bcb_buf + 0x10)) = %x\n", checksum_from_calc, (*(UINT32 *)(buf + 0x10)));
        if (checksum_from_calc == (*(UINT32 *)(buf + 0x10)))
            return 0;
    }
    return 1;
}

void check_mtz_upg_mode(void)
{
    UINT32 mode;

    _MetaZone_Read(MZ_UPGRADE_MODE_IDX, &mode);

    if (FASTBOOT_UPG_MODE == mode) {
        Printf("fastboot mode \n");
        g_upg_mode = 1;
    } else if (COPY_UPG_MODE == mode) {
        Printf("copyupgrade mode\n");
        g_upg_mode = 2;
    } else {
        g_upg_mode = 0;
    }

    return;
}

INT32 check_datazone_valid(void *buf)
{
    INT32 ret;
    UINT32 datazoneAddr = DATA_ZONE_NAND_OFSET;
    UINT32 datazonePartitionSize = 4 * 1024;
    UINT32 BCBPartitionSize = 4 * 1024;
    UINT32 datazoneAddr_bk = DATA_ZONE_NAND_OFSET_BAK;
    void *buf_bcb = buf + datazonePartitionSize;

    /*check bcb*/
    ret = i4NFBPartitionRead(buf, datazoneAddr, DATA_ZONE_SIZE, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR);
    if (ret == FALSE)
    {
        Printf("Read BCB Info Failed!\n");
        goto check_BCB_bk;
    }

    checkpartname = BCB;
    if (check_head_tag(checkpartname, buf_bcb) != 0)
    {
        Printf("No Valid BCB head tag Found!\n");
        goto check_BCB_bk;
    }

    if (check_checksum(checkpartname, buf_bcb) != 0)
    {
        Printf("No Valid BCB data Found!\n");
        goto check_BCB_bk;
    }
    Printf("Read BCB Info Successfully, use BCB!\n");

    goto check_datazone;

check_datazone:
    memcpy((UINT8 *)&bcb, (UINT8 *)buf_bcb, sizeof(struct bootloader_message));
    /*check datazone*/

    checkpartname = DATAZONE;
    if (check_head_tag(checkpartname, buf) != 0)
    {
        Printf("No Valid datazone head tag Found!\n");
        goto check_BCB_bk;
    }

    if (check_checksum(checkpartname, buf) != 0)
    {
        Printf("No Valid datazone data Found!\n");
        goto check_BCB_bk;
    }

    Printf("Read datazone Info Successfully, use datazone!\n");
    return 0;

check_BCB_bk:
    /*check bcb_bk*/
    ret = i4NFBPartitionRead(buf, datazoneAddr_bk, DATA_ZONE_SIZE, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR);
    if (ret == FALSE)
    {
        Printf("Read BCB_bk Info Failed!\n");
        return 1;
    }

    checkpartname = BCB;
    if (check_head_tag(checkpartname, buf_bcb) != 0)
    {
        Printf("No Valid BCB_bk head tag Found!\n");
        return 1;
    }

    if (check_checksum(checkpartname, buf_bcb) != 0)
    {
        Printf("No Valid BCB_bk data Found!\n");
        return 1;
    }
    Printf("Read BCB_bk Info Successfully, use BCB_bk!\n");

check_datazone_bk:
    // try datazone_bk
    Printf("datazone is not correct, try to use datazone_bk\n");
    /*check datazone_bk*/

    checkpartname = DATAZONE;
    if (check_head_tag(checkpartname, buf) != 0)
    {
        Printf("No Valid datazone_bk head tag Found!\n");
        return 1;
    }

    if (check_checksum(checkpartname, buf) != 0)
    {
        Printf("No Valid datazone_bk head tag Found!\n");
        return 1;
    }

    Printf("Read datazone_bk Info Successfully, use datazone_bk!\n");
    return 0;
}

#ifdef ATC_AB_PARTITION_SUPPORT
#define DATAZONE_PARTITION_SIZE 0x200000
int set_datazone_bcb (struct bootloader_message *dtz_bcb)
{
    UINT32 datazoneAddr = DATA_ZONE_NAND_OFSET;
    UINT32 datazonebkAddr = DATA_ZONE_NAND_OFSET_BAK;
    UINT32 datazoneSize = DATA_ZONE_SIZE;
    UINT32 blkSize = _rBLHeader.pagesPerBlock *_rBLHeader.NFIinfo.pageSize;
    UINT32 *datazoneBuf = (UINT32 *)DATA_ZONE_DRAM_ADDR;
    void* buf_bcb = (void *)datazoneBuf + DATAZONE_BCB_SIZE;
    UINT32 chk = 0;
    UINT32 isBadBlock;
    int i = 0;

    chk = calculate_checksum((char *)dtz_bcb + TAG_AND_CHECKSUM_SZIE,
                        sizeof(struct bootloader_message) - TAG_AND_CHECKSUM_SZIE);
    memcpy(dtz_bcb->checksum, &chk, sizeof(chk));

    //use DATA_ZONE_DRAM_ADDR global variable bcb
    memcpy(buf_bcb, dtz_bcb, sizeof(struct bootloader_message));

    //check bad block and skip bad block
    for (i = 0; i < (DATAZONE_PARTITION_SIZE / blkSize); i++) {
        UINT32 u4TmpIdx = uidiv(datazoneAddr, blkSize);
        if (bbt_found == TRUE && u4TmpIdx < CHECK_BLOCKS) {
             isBadBlock = is_block_bad(u4TmpIdx);
         } else {
            isBadBlock = !fgCheckValidBlock(u4TmpIdx * _rBLHeader.pagesPerBlock, (UINT32 *)MEM_BUF_MTD_TMP_MEM_PTR);
        }
        if (isBadBlock) {
            Printf("block %u (log %u) is bad, skip it\r\n",i, u4TmpIdx);
            datazoneAddr += blkSize;
            continue;
        }
    }

    for (i = 0; i < (DATAZONE_PARTITION_SIZE / blkSize); i++) {
        UINT32 u4TmpIdxBk = uidiv(datazonebkAddr, blkSize);
        if (bbt_found == TRUE && u4TmpIdxBk < CHECK_BLOCKS) {
             isBadBlock = is_block_bad(u4TmpIdxBk);
         } else {
            isBadBlock = !fgCheckValidBlock(u4TmpIdxBk * _rBLHeader.pagesPerBlock, (UINT32 *)MEM_BUF_MTD_TMP_MEM_PTR);
        }
        if (isBadBlock) {
            Printf("block %u (log %u) is bad, skip it\r\n",i, u4TmpIdxBk);
            datazonebkAddr += blkSize;
            continue;
        }
    }

    UINT32 datazoneBlk = datazoneAddr / (_rBLHeader.pagesPerBlock *_rBLHeader.NFIinfo.pageSize);
    UINT32 pageNum = datazoneBlk * _rBLHeader.  pagesPerBlock;
    //Printf("datazoneAddr = 0x%x\n", datazoneAddr);
    //Printf("datazoneBlk = 0x%x\n", datazoneBlk);
    //Printf("pageNum = 0x%x\n", pageNum);

    //Erase datazone before write
    #if 1
    //disable erase and write until nand driver ready
    for (i = 0; i < (datazoneSize / blkSize); i++) {
        if (nand_driver.EraseBlock(datazoneBlk + i, _rBLHeader.pagesPerBlock) != S_DONE) {
            Printf("erase fail\n");
            return -1;
        }
    }
    //write datazone with page
    for (i = 0; i < (datazoneSize / _rBLHeader.NFIinfo.pageSize); i++) {
        UINT32 *page_ptr = (UINT32 *)((UINT8 *)datazoneBuf + (i * _rBLHeader.NFIinfo.pageSize));
        if (nand_driver.WritePage(page_ptr, NULL, pageNum) != S_DONE) {
            Printf("write fail\n");
            return -1;
        }
        pageNum++;
    }
    Printf("\nwrite dtz ok\n");

    UINT32 datazonebkBlk = datazonebkAddr / blkSize;
    UINT32 pageNumBk = datazonebkBlk * _rBLHeader.pagesPerBlock;
    //Erase datazone_bk before write
    for (i = 0; i < (datazoneSize / blkSize); i++) {
        if (nand_driver.EraseBlock(datazonebkBlk + i, _rBLHeader.pagesPerBlock) != S_DONE) {
            Printf("erase fail\n");
            return -1;
        }
    }
    //write datazone_bk with page
    for (i = 0; i < (datazoneSize / _rBLHeader.NFIinfo.pageSize); i++) {
        UINT32 *page_ptr_bk = (UINT32 *)((UINT8 *)datazoneBuf + (i * _rBLHeader.NFIinfo.pageSize));
        if (nand_driver.WritePage(page_ptr_bk, NULL, pageNumBk) != S_DONE) {
            Printf("write fail\n");
            return -1;
        }
        pageNumBk++;
    }
    #endif

    Printf("write dtz_bk ok\n");
    return 0;
}
#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:          GetDataZoneInfo

Description:      Get Datazone Information

Arguments:      pu4DZInfo             - [in/out] DataZone Information

Return Value:    TRUE on success

-------------------------------------------------------------------*/
BOOL GetDataZoneInfo(UINT32 *pu4DZInfo)
{
    INT32 bRet = 0;
    /*1. Get Jump Information from Datazone*/
    bRet = check_datazone_valid(pu4DZInfo);

    if (bRet)
        return FALSE;
    else
        return TRUE;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:          GetEbootContent

Description:      Get Eboot Content

Arguments:      eboot_des             - [in/out] Eboot information
                        u4RelBL                 - [in]  Relocate or not

Return Value:    TRUE on success

-------------------------------------------------------------------*/
BOOL GetEbootContent(IMAGE_DESCRIPTOR *eboot_des, UINT32 u4RelBL)
{
    BOOL bRet = TRUE;
    UINT32 u4PhyEbootBlockStart = 0, u4LogEbootBlockStart = 0, u4BlockSize = _rBLHeader.pagesPerBlock * _rBLHeader.NFIinfo.pageSize;
    if (u4RelBL == USE_BLRELOCATE)
    {
#if 0
        /*1. Get Relocate Eboot Location*/
        u4PhyEbootBlockStart = *(UINT32 *)(MEM_BUF_SECURE_BOOT_1 + RELPHYEBOOT_START);
        u4LogEbootBlockStart = FindPhy2Log(u4PhyEbootBlockStart);
        Printf("\t [Relocate] Eboot Physical Block is %d Logical Block is %d\n", u4PhyEbootBlockStart, u4LogEbootBlockStart);
        if (i4NFBPartitionRead((UINT32 *)(eboot_des->dwLoadPhyAddr), u4LogEbootBlockStart * u4BlockSize, eboot_des->dwTtlLen, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE)
        {
            /*2. Get Relocate Backup Eboot location*/
            u4PhyEbootBlockStart = *(UINT32 *)(MEM_BUF_SECURE_BOOT_1 + RELPHYBAKUPEBOOT_START);
            u4LogEbootBlockStart = FindPhy2Log(u4PhyEbootBlockStart);
            Printf("\t [Relocate] Using Backup Eboot Physical Block is %d Logical Block is %d\n", u4PhyEbootBlockStart, u4LogEbootBlockStart);
            if (i4NFBPartitionRead((UINT32 *)(eboot_des->dwLoadPhyAddr), u4LogEbootBlockStart * u4BlockSize, eboot_des->dwTtlLen, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) == FALSE)
            {
                Printf("\t Read Backup Eboot Info Failed\n");
            }
        }
#endif
    }
    else
    {
        Printf("\t test Eboot Address is 0x%08x\n", eboot_des->dwStartAddr);
        if (i4NFBPartitionRead((UINT32 *)(eboot_des->dwLoadPhyAddr), eboot_des->dwStartAddr, eboot_des->dwTtlLen, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE)
        {
            bRet = i4NFBPartitionRead((UINT32 *)(eboot_des->dwLoadPhyAddr), (eboot_des->dwStartAddr + _rBLHeader.pagesPerBlock * _rBLHeader.NFIinfo.pageSize),
                                      eboot_des->dwTtlLen, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR);
            Printf("\t Read Before Relocate Eboot Info Failed\n");
        }
    }
    return bRet;
}

partitionread *readpartitioninfofromflash()
{
    partitionread *ppartread, *pprepartition, *pcurpartition;

    ppartread = (partitionread *)(DATA_ZONE_DRAM_ADDR + DATAZONE_PARTITION_OFFSET + 512);

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

partitionread *get_part_info_by_name(const char* name)
{
    partitionread *ppartitionread, *p;
    const char *search_name = name;
    char *temp = (char *)name;

#ifdef ATC_AB_PARTITION_SUPPORT
    if (strcmp(name, "logo") == 0) {
        search_name = (ab_slot == 1) ? "logo_b" : "logo_a";
    } else if (strcmp(name, "vba") == 0) {
        search_name = (ab_slot == 1) ? "vba_b" : "vba_a";
    }
#endif

again:
    ppartitionread = readpartitioninfofromflash();

    p = ppartitionread;
    g_partIndex = 0;
    while(p) {
        if (strcmp(p->szPartName, search_name) == 0) {
            return p;
        }
        g_partIndex += 1;
        p = p->nextpartition;
    }

#ifdef ATC_AB_PARTITION_SUPPORT
    if (strcmp(name, "logo") == 0 && search_name != "logo") {
        search_name = "logo";
        goto again;
    } else if (strcmp(name, "vba") == 0 && search_name != "vba") {
        search_name = "vba";
        goto again;
    }
#endif

    Printf("ERR: Cannot find valid part name: ");
    while(*temp)
    {
        Printf("%c", *temp);
        temp++;
    }
    Printf("\n");

    return NULL;
}

BOOL load_boot_misc(void)
{
    BOOL bRet = TRUE;
    partitionread *p = get_part_info_by_name("boot_misc");
    UINT32 boot_misc_phy = CONFIG_ARGS_START;
    UINT32 boot_misc_size = 0;
    UINT32 nand_addr = 0;
    if (NULL == p)
    {
        Printf("Read boot_misc failed\n");
        return FALSE;
    }
    nand_addr = p->u8PartitionStartAddr;
    if (p->u8RealDataSize >= 0)
        boot_misc_size = (UINT32)(p->u8RealDataSize);
    else
        boot_misc_size = (UINT32)(p->u8PartitionSize);
    Printf("Read boot_misc nand_addr: %x\n", nand_addr);
    Printf("Read boot_misc size: %x\n", boot_misc_size);
    if (i4NFBPartitionRead((UINT32 *)boot_misc_phy, nand_addr, boot_misc_size, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE)
    {
        bRet = FALSE;
        Printf("Read boot_misc failed\n");
    }
    check_rsv();
    return bRet;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:          GetArm2BootContent

Description:      Get arm2 boot Content

Return Value:    TRUE on success

-------------------------------------------------------------------*/
static BOOL nand_boot_arm2(void)
{
    BOOL bRet = TRUE;
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
    UINT32  arm2_phy = 0x40000,logo_phy = 0x7700000;
    UINT32  arm2_size = 0x248800,logo_size = 0x12c800;
    UINT32  arm2_nand_addr = 0x380000, logo_nand_addr = 0x7900000;
    ARGS_TO_ARM2_P *args_to_arm2 = (void *)0xa00000;
#endif

    p = get_part_info_by_name(arm2_str);
    if (NULL == p)
    {
        Printf("Read arm2 failed\n");
        goto failed;
    }

    arm2_nand_addr = p->u8PartitionStartAddr;
    arm2_size = get_partition_size(p);

    p = get_part_info_by_name(logo_str);
    if (NULL == p)
    {
        Printf("Read logo failed\n");
        goto failed;
    }

    logo_nand_addr = p->u8PartitionStartAddr;
    logo_size = get_partition_size(p);

    p = get_part_info_by_name(vba_str);
    if (NULL == p)
    {
        Printf("Read vba failed\n");
        goto failed;
    }

    vba_nand_addr = p->u8PartitionStartAddr;
    vba_size = get_partition_size(p);

    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("arm2");
    if (NULL == rsv)
    {
        Printf("get arm2 rsv_mem failed\n");
        goto failed;
    }
    arm2_phy = (UINT32)(rsv->start_addr);
    args_to_arm2 = (void *)(arm2_phy);
    arm2_phy = arm2_phy + 0x40000;
    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("animation");
    if (NULL == rsv)
    {
        Printf("get animation rsv_mem failed\n");
        goto failed;
    }
    logo_phy = (UINT32)(rsv->start_addr) + (UINT32)(rsv->size) - MRF_LOAD_OFFSET;

    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("vba");
    if (NULL == rsv)
    {
        Printf("get vba rsv failed\n");
        goto failed;
    }
    vba_phy = (UINT32)(rsv->start_addr);
    Printf("arm2 rsv:%x --size:%x --%x\n", arm2_phy, arm2_size, arm2_nand_addr);
    Printf("logo rsv:%x --size:%x --%x\n", logo_phy, logo_size, logo_nand_addr);
    Printf("vba rsv:%x --size:%x --%x\n", vba_phy, vba_size, vba_nand_addr);
    Printf("arm2 start:%u\n", boot_time_ms());
    if (i4NFBPartitionRead((UINT32 *)(arm2_phy), arm2_nand_addr, arm2_size, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE)
    {
        Printf("Read arm2 failed\n");
        goto failed;
    }

    Printf("arm2 end:%u\n", boot_time_ms());
    if (i4NFBPartitionRead((UINT32 *)(logo_phy), logo_nand_addr, logo_size, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE)
    {
        Printf("Read logo failed\n");
        goto failed;
    }
    Printf("logo end:%u\n", boot_time_ms());
    if (i4NFBPartitionRead((UINT32 *)(vba_phy), vba_nand_addr, vba_size, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE)
    {
        Printf("Read vba failed\n");
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


#define MTZ_MAX_SCAN_SLOTS 16

static UINT8 g_mtz_valid[MTZ_MAX_SCAN_SLOTS];
static UINT32 g_mtz_seqs[MTZ_MAX_SCAN_SLOTS];
#if 0
 static UINT16 mtz_crc16(const UINT8 *pbuff, UINT32 length)
 {
     static const UINT16 crc16_ccitt_table[256] = {
         0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
         0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
         0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
         0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
         0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
         0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
         0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
         0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
         0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
         0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
         0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
         0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
         0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
         0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
         0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
         0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
         0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
         0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
         0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
         0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
         0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
         0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
         0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
         0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
         0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
         0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
         0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
         0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
         0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
         0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
         0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
         0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
     };
     UINT16 crc = 0xFFFF;
     UINT32 i;

     if (!pbuff || length == 0) {
         Printf("[MTZ][Preloader] CRC input invalid\r\n");
         return (UINT16)0xFFFF;
     }

     for (i = 0; i < length; i++) {
         UINT8 idx = (UINT8)((crc >> 8) ^ pbuff[i]);
         crc = (UINT16)((crc << 8) ^ crc16_ccitt_table[idx]);
     }

     return crc;
 }
#else

static UINT16 mtz_crc16(const UINT8 *pbuff, UINT32 length)
{
    UINT16 shift, data, val;
    UINT32 i;

    shift = 0xffff;
    if (!pbuff || length == 0) {
        Printf("[MTZ][Preloader] CRC input invalid\r\n");
        return (UINT16)0xFFFF;
    }
    for (i = 0; i < length * 8; i++) {
        if ((i % 8) == 0)
            data = (*pbuff++) << 8;
        val = shift ^ data;
        shift = shift << 1;
        data = data << 1;
        if (val & 0x8000)
            shift = shift ^ 0x1021;
    }

    return shift;
}
#endif

static inline BOOL mtz_seq_after(UINT32 seq_a, UINT32 seq_b)
{
    return ((INT32)(seq_a - seq_b) > 0) ? TRUE : FALSE;
}

static inline SlotHeader *mtz_get_slot_header(UINT8 *slot_base)
{
    return (SlotHeader *)(slot_base + MTZ_SLOT_SIZE - MTZ_SLOT_HEADER_SIZE);
}

static BOOL mtz_check_slot_crc(UINT8 *slot_base, SlotHeader *slot_header)
{
    TMetaZone *meta = (TMetaZone *)slot_base;
    UINT16 re_crc;

    if (meta->dwSignature != METAZONE_SIGNATURE || meta->dwVersion != METAZONE_VERSION) {
        //Printf("[MTZ][Preloader] slot signature/version invalid, meta->dwSignature:0x%x, meta->dwVersion:0x%x\r\n", meta->dwSignature, meta->dwVersion);
        return FALSE;
    }

    if (meta->dwDataSize > MTZ_SLOT_SIZE) {
        Printf("[MTZ][Preloader] slot size invalid: 0x%x\r\n", meta->dwDataSize);
        return FALSE;
    }

    re_crc = mtz_crc16((UINT8 *)meta, (UINT32)sizeof(TMetaZone));
    if (slot_header->crc.crc_header != re_crc) {
        Printf("[MTZ][Preloader] header CRC mismatch, exp=%u calc=%u\r\n",
               slot_header->crc.crc_header, re_crc);
        return FALSE;
    }

    if (meta->dwValueNum > 1) {
        re_crc = mtz_crc16((UINT8 *)((UINT32 *)(slot_base + meta->dwValueOffset) + 1), (meta->dwValueNum - 1) * 4);
        //Printf("[MTZ][Preloader] dword CRC, exp=%u calc=%u\r\n",slot_header->crc.crc_dword, re_crc);
        if (slot_header->crc.crc_dword != re_crc) {
            Printf("[MTZ][Preloader] dword CRC mismatch, exp=%u calc=%u\r\n",
                   slot_header->crc.crc_dword, re_crc);
            return FALSE;
        }
    }

    if (meta->dwBinaryNum > 0 && meta->dwBinaryItemSize > 0) {
        re_crc = mtz_crc16((UINT8 *)slot_base + meta->dwBinaryOffset, (meta->dwBinaryItemSize + 4) * meta->dwBinaryNum);
        //Printf("[MTZ][Preloader] binary CRC , exp=%u calc=%u\r\n",slot_header->crc.crc_binary, re_crc);
        if (slot_header->crc.crc_binary != re_crc) {
            Printf("[MTZ][Preloader] binary CRC mismatch, exp=%u calc=%u\r\n",
                   slot_header->crc.crc_binary, re_crc);
            return FALSE;
        }
    }

    if (meta->dwReserveSize > 0) {
        re_crc = mtz_crc16( (UINT8 *)slot_base + sizeof(TMetaZone), meta->dwReserveSize);
        //Printf("[MTZ][Preloader] reserved CRC, exp=%u calc=%u\r\n",slot_header->crc.crc_reserved, re_crc);
        if (slot_header->crc.crc_reserved != re_crc) {
            Printf("[MTZ][Preloader] reserved CRC mismatch, exp=%u calc=%u\r\n",
                   slot_header->crc.crc_reserved, re_crc);
            return FALSE;
        }
    }

    return TRUE;
}

INT32 Metazone_LoadLatestSlot(UINT32 nand_addr,
                              UINT32 partition_size,
                              VOID *slot_buf)
{
    UINT32 slot_count;
    UINT32 i;
    UINT32 best_idx = 0;
    UINT32 best_seq = 0;
    BOOL best_valid = FALSE;
    UINT32 j;
    UINT32 header_addr;
    UINT32 mtz_slots_per_block;
    UINT32 block_size;

    UINT32 block_count;
    UINT32 block_rel;
    UINT32 slot_in_block;
    UINT32 slot_offset;
    UINT32 selected_block_count = 0;
    UINT32 selected_blocks[2];
    SlotHeader *header_view = NULL;
    UINT32 mtz_addr = nand_addr;
    UINT32 u4TmpIdx;
    UINT32 isBadBlock;

    if (!slot_buf|| partition_size < MTZ_SLOT_SIZE) {
        // Printf("[MTZ][Preloader] invalid slot load args\r\n");
        return -1;
    }
    // get good block 
    // Printf("mtz start2:%u\n", boot_time_ms());
    block_size = _rBLHeader.pagesPerBlock * _rBLHeader.NFIinfo.pageSize;
    block_count = partition_size / block_size;

    for (i = 0; i < block_count && selected_block_count < 2; i++) {
        u4TmpIdx = uidiv(mtz_addr, block_size);
        if (bbt_found == TRUE && u4TmpIdx < CHECK_BLOCKS) {
             isBadBlock = is_block_bad(u4TmpIdx);
         } else {
            isBadBlock = !fgCheckValidBlock(u4TmpIdx * _rBLHeader.pagesPerBlock, (UINT32 *)MEM_BUF_MTD_TMP_MEM_PTR);
        }
        if (isBadBlock) {
             Printf("[MTZ][Preloader] block %u (log %u) is bad, skip the bad block\r\n",i, u4TmpIdx);
            mtz_addr += block_size;
            continue;
        }
        selected_blocks[selected_block_count] = i;
        selected_block_count++;
        // Printf("[MTZ][Preloader] block %u (log %u) is OK, block_size:0x%x  selected_block_count:%d\r\n",
        //        i, u4TmpIdx, block_size, selected_block_count);
        mtz_addr += block_size;
    }

    if (selected_block_count == 0) {
        Printf("MTZ][Preloader] no good block found\n");
    }
    /* Scan all slot headers and pick the newest by seq (wrap-safe). */
    mtz_slots_per_block = block_size / MTZ_SLOT_SIZE;
    slot_count = selected_block_count * mtz_slots_per_block;
    if (slot_count > MTZ_MAX_SCAN_SLOTS)
        slot_count = MTZ_MAX_SCAN_SLOTS;
    for (i = 0; i < slot_count; i++) {
        block_rel = i / mtz_slots_per_block;
        slot_in_block = i % mtz_slots_per_block;
        slot_offset = (selected_blocks[block_rel] * block_size) + (slot_in_block * MTZ_SLOT_SIZE );
        header_view = NULL;
        header_addr = nand_addr + slot_offset + (MTZ_SLOT_SIZE - _rBLHeader.NFIinfo.pageSize);

        // Printf("[MTZ][Preloader] metazone header_addr:0x%x, slot_count:%d, i:%d\r\n", header_addr, slot_count, i);
        memset((UINT32 *)(slot_buf), 0,  _rBLHeader.NFIinfo.pageSize);
       if (i4NFBPartitionRead((UINT32 *)slot_buf, header_addr, _rBLHeader.NFIinfo.pageSize, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE) {
            g_mtz_valid[i] = 0;
            g_mtz_seqs[i] = 0;
            continue;
        }
        header_view = (SlotHeader *)((UINT8 *)slot_buf + (_rBLHeader.NFIinfo.pageSize - MTZ_SLOT_HEADER_SIZE));
        if (header_view->magic == METAZONE_SLOT_MAGIC && header_view->state == 0xFF) {
            g_mtz_valid[i] = 1;
            g_mtz_seqs[i] = header_view->seq;
            if (!best_valid || mtz_seq_after(g_mtz_seqs[i], best_seq)) {
                best_seq = g_mtz_seqs[i];
                best_idx = i;
                best_valid = TRUE;
            }
        } else {
            g_mtz_valid[i] = 0;
            g_mtz_seqs[i] = 0;
        }
        // Printf("[MTZ][Preloader]111111111 slot %u header: magic=0x%X state=0x%X seq=%u valid=%u, best_idx=%d\r\n",
                //  i, header_view->magic, header_view->state, header_view->seq, g_mtz_valid[i],best_idx);
    }
    //get default data
    if (!best_valid) {
        Printf("[MTZ][Preloader] no valid slot header found\r\n");
        if (i4NFBPartitionRead((UINT32 *)(slot_buf), nand_addr, 0x10000, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE)
        {
            Printf("Read metazone failed\n");
            return -1;
        }
        return 0;
    }

#if 1
    /* Try newest first; if CRC fails, fallback to previous seq. */
     for (i = 0; i < slot_count; i++) {
         UINT32 slot_offset;
         if (!best_valid)
             break;

         slot_offset = nand_addr + (best_idx * MTZ_SLOT_SIZE);
        // Printf("[MTZ][Preloader] slot %u , slot_offset:0x%x\r\n", best_idx, slot_offset);
         //memset((UINT32 *)(slot_buf), 0, MTZ_SLOT_SIZE);
         if (i4NFBPartitionRead((UINT32 *)slot_buf, slot_offset, MTZ_SLOT_SIZE, (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE) {
           // Printf("[MTZ][Preloader] read slot %u failed\r\n", best_idx);
            g_mtz_valid[best_idx] = 0;
        } else if (mtz_check_slot_crc((UINT8 *)slot_buf, mtz_get_slot_header((UINT8 *)slot_buf))) {
             Printf("[MTZ][Preloader] load slot %u ok\r\n",best_idx);
             return 0;
        } else {
            //  Printf("[MTZ][Preloader] slot %u crc invalid, try previous\r\n", best_idx);
             g_mtz_valid[best_idx] = 0;
        }

         best_valid = FALSE;
         for (j = 0; j < slot_count; j++) {
             if (!g_mtz_valid[j])
                 continue;
             if (!best_valid || mtz_seq_after(g_mtz_seqs[j], best_seq)) {
                 best_seq = g_mtz_seqs[j];
                 best_idx = j;
                 best_valid = TRUE;
             }
         }
#endif
     }

    // Printf("[MTZ][Preloader] all slots CRC invalid\r\n");

    return 0;
}

static BOOL nand_boot_tz(void)
{
    BOOL bRet = TRUE;
    partitionread *p = NULL;
    RSV_MEM_T *rsv = NULL;
    void *theKernel = NULL;
    UINT32 nand_addr_array[3];
    UINT32 size_array[3];
    UINT32 phy_addr_array[3];
    int i = 0;
    const char *partition_names[3];

#ifdef ATC_AB_PARTITION_SUPPORT
    char *tz_a_str = "trustzone_a";
    char *tz_b_str = "trustzone_b";
    char *kernel_a_str = "kernel_a";
    char *kernel_b_str = "kernel_b";
    char *dtb_a_str = "dtb_a";
    char *dtb_b_str = "dtb_b";

    partition_names[0] = (ab_slot) ? tz_b_str : tz_a_str;
    partition_names[1] = (ab_slot) ? kernel_b_str : kernel_a_str;
    partition_names[2] = (ab_slot) ? dtb_b_str : dtb_a_str;
#else
    partition_names[0] = "trustzone";
    partition_names[1] = "kernel";
    partition_names[2] = "dtb";
#endif

    for (i = 0; i < 3; i++) {
        p = get_part_info_by_name(partition_names[i]);
        if (NULL == p) {
            Printf("Read %s failed\n", partition_names[i]);
            goto failed;
        }

        nand_addr_array[i] = p->u8PartitionStartAddr;
        size_array[i] = get_partition_size(p);
    }

    rsv = (RSV_MEM_T *)get_rsv_mem_by_name("trustzone");
    if (NULL == rsv) {
        Printf("get trustzone rsv_mem failed\n");
        goto failed;
    }
    phy_addr_array[0] = (UINT32)(rsv->start_addr);

    phy_addr_array[1] = (UINT32)(KERNEL_LOAD_ADDR);
    phy_addr_array[2] = (UINT32)(FDT_LOAD_ADDR);
    
    Printf("tz rsv:%x size:%x --%x\n", phy_addr_array[0], size_array[0], nand_addr_array[0]);
    Printf("kernel addr:%x size:%x --%x\n", phy_addr_array[1], size_array[1], nand_addr_array[1]);
    Printf("dtb addr:%x size:%x --%x\n", phy_addr_array[2], size_array[2], nand_addr_array[2]);

    Printf("dtb load:%u\n", boot_time_ms());
    if (i4NFBPartitionRead((UINT32 *)(phy_addr_array[2]), nand_addr_array[2], size_array[2], (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE) {
        Printf("Read dtb failed\n");
        goto failed;
    }
    args_to_arm2->dtb_status = STATUS_LOAD_READY;
    Printf("dtb end:%u\n", boot_time_ms());

    if (i4NFBPartitionRead((UINT32 *)(phy_addr_array[0]), nand_addr_array[0], size_array[0], (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE) {
        Printf("Read tz failed\n");
        goto failed;
    }
    Printf("trustzone end load:%u\n", boot_time_ms());

    if (i4NFBPartitionRead((UINT32 *)(phy_addr_array[1]), nand_addr_array[1], size_array[1], (UINT8 *)MEM_BUF_MTD_TMP_MEM_PTR) != TRUE) {
        Printf("Read kernel failed\n");
        goto failed;
    }
    Printf("kernel load end:%u\n", boot_time_ms());

    while (STATUS_MODIFY_END != args_to_arm2->arm_dtb_status)
        ;

    Printf("start tz:%u\n", boot_time_ms());

    void (*initTrustZone)(int zero, int arch, UINT32 parms, void *theKernel);
    theKernel = (void *)(KERNEL_LOAD_ADDR);
    initTrustZone = (void (*)(int, int, UINT32, void *))phy_addr_array[0];
    initTrustZone(0, (int)(MACH_TYPE_AC83XX), FDT_LOAD_ADDR, theKernel);

    return bRet;
failed:
    bRet = FALSE;
    while (1)
        ;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:          nand_load_metazone

Description:      Get metazone Content

Return Value:    TRUE on success

-------------------------------------------------------------------*/

#define MTZ_PARTITION_SIZE (0x20000)
static BOOL nand_load_metazone(void)
{
    UINT32 mtz1_addr;
    UINT32 mtz_size;
    UINT32 mtz_nand_addr;
    partitionread *p = NULL;
    RSV_MEM_T *rsv = NULL;
    char *mtz_str = "metazone";
    p = get_part_info_by_name(mtz_str);
    if (NULL == p) {
        // Printf("Read mtz failed\n");
        goto failed;
    }
    Printf("mtz start1:%u\n", boot_time_ms());
    mtz_nand_addr = p->u8PartitionStartAddr;

    mtz_size = (UINT32)(p->u8PartitionSize);
    mtz_size = ALIGN(mtz_size, _rBLHeader.NFIinfo.pageSize);

    rsv = (RSV_MEM_T *)get_rsv_mem_by_name(mtz_str);
    if (NULL == rsv)
    {
        // Printf("get mtz reserved_memory failed ,please check\n");
        goto failed;
    }
    mtz1_addr = (UINT32)(rsv->start_addr);
    memset((UINT32 *)(mtz1_addr), 0, MTZ_SLOT_SIZE);
    // Printf("[MTZ][Preloader] mtz1:%x size:%x mtz_part:%x rsv_size:%d\n",
    //     mtz1_addr, mtz_size, mtz_nand_addr, rsv->size);

    if (rsv->size < MTZ_SLOT_SIZE) {
        Printf("[MTZ][Preloader] rsv size too small for slot\n");
        goto failed;
    }

    // Printf("[MTZ][Preloader] start to load mtz from nand, page_size=%u, mtz_nand_addr=0x%x\n",
    //        _rBLHeader.NFIinfo.pageSize, mtz_nand_addr);
    if (Metazone_LoadLatestSlot(mtz_nand_addr,
                                mtz_size,
                                (VOID *)mtz1_addr) != 0)
    {
        Printf("[MTZ][Preloader] load latest slot failed\n");
        return FALSE;
    }

    Printf("mtz end:%u\n", boot_time_ms());

    return TRUE;
failed:
    while (1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:          JumpToEboot

Description:      Jump to Eboot

Arguments:

Return Value:

-------------------------------------------------------------------*/
void JumpToEboot()
{
    IMAGE_DESCRIPTOR *eboot_des;
    partitionread *part = NULL;
    UINT32 u4RelBL = 0;
    BOOL ret;
#ifdef ATC_AB_PARTITION_SUPPORT
    int j = 0;
#endif

    /*1. Get Jump Information from Datazone*/
    ret = GetDataZoneInfo((UINT32 *)DATA_ZONE_DRAM_ADDR);
    if (!ret)
    {
        Printf("ERR: Cannot find valid datazone Info\n");
        _reset1();
        while (1)
            ;
    }

#ifdef ATC_AB_PARTITION_SUPPORT
    ab_boot_check();
    Printf("ab slot_info is:\n");
    for (j = 0; j < 2; j++)
    {
        Printf("[%d]priority=%d\n", j, bcb.metadata.slot_info[j].priority);
        Printf("[%d]retry_count=%d\n", j, bcb.metadata.slot_info[j].retry_count);
        Printf("[%d]successful_boot=%d\n", j, bcb.metadata.slot_info[j].successful_boot);
        Printf("[%d]normal_boot=%d\n", j, bcb.metadata.slot_info[j].normal_boot);
    }

    const char *ab_suffix = get_suffix();
    if (!ab_suffix) {
        Printf("invalid suffix\n");
        while (1)
            ;
    }
    if (0 == strcmp(ab_suffix, BOOTCTRL_SUFFIX_A)) {
        ab_slot = 0;
    } else if (0 == strcmp(ab_suffix, BOOTCTRL_SUFFIX_B)) {
        ab_slot = 1;
    } else {
        Printf("error boot slot info\n");
        while (1)
            ;
    }

    Printf("ab_slot is %d\n", ab_slot);
    eboot_des = (IMAGE_DESCRIPTOR *)(DATA_ZONE_DRAM_ADDR + (ab_slot ? 0xBC : 0x4C));
    part = get_part_info_by_name(ab_slot ? "uboot_b" : "uboot_a");
#else
    if (bcb.bootflag == 0)
    {
        eboot_des = ((IMAGE_DESCRIPTOR *)(DATA_ZONE_DRAM_ADDR + 0x4C));
    }
    else if (bcb.bootflag == 1)
    {
        eboot_des = ((IMAGE_DESCRIPTOR *)(DATA_ZONE_DRAM_ADDR + 0xBC));
    }
    else
    {
        Printf("error bootflag: %d\n", bcb.bootflag);
        _reset1();
        while (1)
            ;
    }
    part = get_part_info_by_name("uboot");
#endif
    //Printf("test eboot_img_des(final data):\n");
    load_boot_misc();
    nand_load_metazone();

    Metazone_Init();

    //check upgrade mode from metazone before boot arm2
    check_mtz_upg_mode();

    if (g_upg_mode == 1 || g_upg_mode == 2)
    {
        args_to_arm2->upgrade_mode = BOOT_SD_UPGRADE;
    }
    else
    {
        args_to_arm2->upgrade_mode = BOOT_NO_UPGRADE;
    }

    // we should use get_part_info_by_name after datazone load
    if (args_to_arm2->upgrade_mode == BOOT_NO_UPGRADE)
        nand_boot_arm2();

    u4RelBL = eboot_des->dwTtlLen;
    Printf("\tdwLoadAddress:0x%08x\n", eboot_des->dwLoadAddress);
    Printf("\tdwLoadPhyAddr:0x%08x\n", eboot_des->dwLoadPhyAddr);
    Printf("\tdwJumpAddress:0x%08x\n", eboot_des->dwJumpAddress);
    Printf("\tdwJumpPhyAddr:0x%08x\n", eboot_des->dwJumpPhyAddr);
    Printf("\tdwStartAddr:0x%08x\n", eboot_des->dwStartAddr);
    Printf("\tdwdwTtlLen:0x%08x\n", eboot_des->dwTtlLen);
    Printf("\t4RelBl:0x%08x\n", u4RelBL);
    Printf("\tupgrade_mode:0x%x\n", args_to_arm2->upgrade_mode);
    if (BOOT_NO_UPGRADE == args_to_arm2->upgrade_mode)
        nand_boot_tz();
    else
    {
        if (part) {
            eboot_des->dwStartAddr = (UINT32)part->u8PartitionStartAddr;
            Printf("\treload dwStartAddr:0x%08x\n", eboot_des->dwStartAddr);
        }
        /*2. Get Eboot Content to memory*/
        if (GetEbootContent(eboot_des, u4RelBL) == TRUE)
        {
            /*3. Jump to Eboot*/
            ((FUNC_CALL)(eboot_des->dwJumpPhyAddr))();
        }
    }
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:          MoveSectionByNand

Description:      Move Section By NandFlash

Arguments:      u4SecStart  - [in] Section  Start
                        u4SecLen    - [in] Section Length
                        pu4MovBuf   -[in/out] move buffer
                        pu4TmpBuf  -[in ]Tmp buffer


Return Value:    TRUE on success

-------------------------------------------------------------------*/
BOOL MoveSectionByNand(UINT32 u4SecStart, UINT32 u4SecLen, UINT32 *pu4MovBuf, UINT32 *pu4TmpBuf)
{
    BOOL bRet = TRUE;
    /*1. Get Jump Information from Datazone*/
    if ((bRet = i4NFBPartitionRead(pu4MovBuf, u4SecStart, u4SecLen, (UINT8 *)pu4TmpBuf)) == FALSE)
    {
        Printf("Using  DataZone Address 0x %x:\n", (u4SecStart + _rBLHeader.pagesPerBlock * _rBLHeader.NFIinfo.pageSize));
        /*2. Get Backup Datazone Address*/
        bRet = i4NFBPartitionRead(pu4MovBuf, (u4SecStart + _rBLHeader.pagesPerBlock * _rBLHeader.NFIinfo.pageSize),
                                  u4SecLen, (UINT8 *)pu4TmpBuf);
    }
    return bRet;
}

void RunInNand()
{
    UINT8 *pDramkStart = (UINT8 *)(&_dramk_loader_start);
    UINT8 *pDramkEnd = (UINT8 *)(&_dramk_loader_end);
    UINT32 u4DramkLength = pDramkEnd - pDramkStart;

    /*1. NandFlash Initialize */
    NandFlashInfoInit((UINT32 *)DRAMK_SDRAM_ADDR);
    u4DramkLength = ALIGN((pDramkEnd - pDramkStart), _rBLHeader.NFIinfo.pageSize);

    Printf("RunInNand DDRK pDramkStart 0x%x pDramkEnd 0x%x u4DramkLength 0x%x: Page Size 0x%x\n",
           pDramkStart, pDramkEnd, u4DramkLength, _rBLHeader.NFIinfo.pageSize);

    /*2. Move Section By Nand*/
    MoveSectionByNand((UINT32)(pDramkStart + PLR_HEAD_SIZE - SDRAM_ADDR_START), u4DramkLength, (UINT32 *)DRAMK_SDRAM_ADDR, (UINT32 *)TMP_SDRAM_ADDR);

    /*3. DDR Initialiaze*/
    DDR_Initialize();

    ddr_calibrate();
    /*5. Jump to Eboot*/
    JumpToEboot();
}
