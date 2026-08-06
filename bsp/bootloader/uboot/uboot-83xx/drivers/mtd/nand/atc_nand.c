#include <chip_ver.h>
#include <common.h>
#include <linux/mtd/nand.h>
#include <malloc.h>
#include <asm/arch/x_typedef.h>
#include <asm/errno.h>
#include <asm/io.h>

#include <linux/mtd/atc_nfi.h>
#include <linux/mtd/atc_nfiecc.h>
#include <linux/mtd/atc_nand.h>

#define USE_MTD_OOB_AUTO  0//We don't use MTD_OOB_AUTO option
#define NAND_BLANK_CHECK 0
static int nand_blank_check(struct mtd_info *mtd, u32 page, u32 len);

static int _data_buf_index = 0;
BOOL _fgUsingDMA = TRUE;
BOOL _fgAUTO_FMT = TRUE;
BOOL _fgECCSWCorrect = FALSE;

static uint8_t bbt_pattern[] = { 'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = { '1', 't', 'b', 'B' };

static struct nand_bbt_descr atc_bbt_main_descr_512 = {
    .options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
        | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
    .offs = 0,
    .len = 4,
    .veroffs = 4,
    .maxblocks = 4,
    .pattern = bbt_pattern
};

static struct nand_bbt_descr atc_bbt_mirror_descr_512 = {
    .options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
        | NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
    .offs = 0,
    .len = 4,
    .veroffs = 4,
    .maxblocks = 4,
    .pattern = mirror_pattern
};

static uint8_t scan_ff_pattern[] = { 0xff, 0xff };

#if USE_MTD_OOB_AUTO
static struct nand_ecclayout atc_oobinfo_512_4bit_ecc = {
       .eccbytes = 8,
       .eccpos = { 8, 9, 10, 11, 12, 13, 14, 15},
       .oobfree = {{2, 6}}
};

static struct nand_ecclayout atc_oobinfo_2K_12bit_ecc = {
       .eccbytes = 46,
       .eccpos = { 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
           28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
           38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
           48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
           58, 59, 60, 61, 62, 63
         },
       .oobfree = {{2, 16}}
};

static struct nand_ecclayout atc_oobinfo_2K_24bit_ecc = {
       .eccbytes = 86,
       .eccpos = { 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
           28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
           38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
           48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
           58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
           68, 69, 70, 71, 72, 73, 74, 75, 76, 77,
           78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
           88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
           98, 99,100,101,102,103
         },
       .oobfree = { {2, 16} }
};

static struct nand_ecclayout atc_oobinfo_4K_24bit_ecc = {
       .eccbytes = 86,
       .eccpos = { 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
           46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
           56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
           66, 67, 68, 69, 70, 71, 72, 73, 74, 75,
           76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
           86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
           96, 97, 98, 99,100,101,102,103,104,105,
          106,107,108,109,110,111,112,113,114,115,
          116,117,118,119,120,121
         },
       .oobfree = { {2, 34}, }
};
#endif

static ECC_Level_t  global_ecc_strength;
static u8 glb_fdm_size = FDM_BYTES;
static u8 glb_fdm_ecc_size = FDM_ECC_BYTES;
static u32 glb_read_pageaddr = 0;

extern int page_ecc_nums;
extern int totel_ecc_bits;
extern int max_ecc_bits;

int atc_nand_read_ecc(struct mtd_info *mtd, u32 page, u32 *buf);
int atc_nand_read_raw_with_autofdm(struct mtd_info *mtd, u32 page, u32 *buf);

void nanddump_register()
{
    printk(KERN_ALERT"---------------nand dump ----------------------\r\n\n");
    printk(KERN_ALERT"[NFI_CNFG]0x%x,[NFI_PAGEFMT]0x%x,[NFI_CON]0x%x,[NFI_ACCCON]0x%x\r\n", *NFI_CNFG,*NFI_PAGEFMT,*NFI_CON,*NFI_ACCCON);
    printk(KERN_ALERT"[NFI_INTR_EN]0x%x,[NFI_INTR]0x%x,[NFI_CMD]0x%x,[NFI_ADDRNOB]0x%x\r\n", *NFI_INTR_EN,*NFI_INTR,*NFI_CMD,*NFI_ADDRNOB);
    printk(KERN_ALERT"[NFI_COLADDR]0x%x,[NFI_ROWADDR]0x%x,[NFI_STRDATA]0x%x,[NFI_DATAW]0x%x\r\n", *NFI_COLADDR,*NFI_ROWADDR,*NFI_STRDATA,*NFI_DATAW);
    printk(KERN_ALERT"[NFI_DATAR]0x%x,[NFI_STA]0x%x,[NFI_FIFOSTA]0x%x,[NFI_ADDRCNTR]0x%x\r\n", *NFI_DATAR,*NFI_STA,*NFI_FIFOSTA,*NFI_ADDRCNTR);
    printk(KERN_ALERT"[NFI_STRADDR]0x%x,[NFI_FDM0L]0x%x,[NFI_FDM0M]0x%x,[NFI_CSEL]0x%x\r\n", *NFI_STRADDR,*NFI_FDM0L,*NFI_FDM0M,*NFI_CSEL);
    printk(KERN_ALERT"\r\n");
    printk(KERN_ALERT"[NFIECC_ENCCON]0x%x,[NFIECC_ENCCNFG]0x%x,[NFIECC_DECCON]0x%x,[NFIECC_DECCNFG]0x%x\r\n",*NFIECC_ENCCON,*NFIECC_ENCCNFG,*NFIECC_DECCON,*NFIECC_DECCNFG);
    printk(KERN_ALERT"----------------------------------------------\r\n\n");
}

static void select_ecc_strength(struct mtd_info *mtd)
{
    u32 spare_size = mtd->oobsize;
    u32 page_size = mtd->writesize;
    u32 fdm_parity;

    if (SECTOR_BYTES == 512)
        spare_size = ( spare_size/(page_size/SECTOR_BYTES)) >= 26?(26*page_size/SECTOR_BYTES):spare_size;
    else if ((*NFI_PAGEFMT & 0x003C) == SPARE_52_26)
        spare_size = ( spare_size/(page_size/SECTOR_BYTES)) >= 52?(52*page_size/SECTOR_BYTES):spare_size;
    else if ((*NFI_PAGEFMT & 0x003C) == SPARE_56_28)
        spare_size = ( spare_size/(page_size/SECTOR_BYTES)) >= 56?(56*page_size/SECTOR_BYTES):spare_size;
    fdm_parity = spare_size/(page_size/SECTOR_BYTES);

    if (fdm_parity >= 52)
        global_ecc_strength = ECC_24_BITS;
    else
        global_ecc_strength = ECC_12_BITS;

    if (SECTOR_BYTES == 512)
    global_ecc_strength = ECC_4_BITS;

    printk("[NAND init] select %d bits ECC\n", global_ecc_strength);
}

static int atc_nand_device_ready(struct mtd_info *mtd)
{
    int result = !( *NFI_STA & STATUS_BUSY);
    return result;
}

//------------------------------------------------------------------------------
// Reset Device Callback Function
//------------------------------------------------------------------------------
STATUS_E  NAND_COMMON_Reset(const struct mtd_info *mtd, const u32  c_timeout)
{
    u32    timeout = c_timeout;
    STATUS_E  ret = S_UNKNOWN_ERR;
    s32    i4Val;

    // reset the NFI core state machine, data FIFO and flushing FIFO
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    *NFI_CNFG = OP_RESET;

    // enable interrupt
    *NFI_INTR_EN = RESET_DONE_EN;

    // reset cmd
    *NFI_CMD = NAND_CMD_RESET;

    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    // wait for reset finish
    timeout = c_timeout;

    NFI_Wait( !(*NFI_INTR & RESET_DONE), timeout);
    i4Val = *NFI_INTR;
#ifdef INT_WR_CLR
    *NFI_INTR = i4Val;
#endif

    if( 0 == timeout ) {
        ret = S_TIMEOUT;
        goto end;
    }
    ret = S_DONE;

end:
    // disable interrupt
    *NFI_INTR_EN = 0;

    return ret;
}

static void  NAND_COMMON_ReadID(struct mtd_info *mtd, unsigned command)
{
    register struct nand_chip *this = mtd->priv;
    uint  nfi_pagefmt;
    u32* p4Data;

    // reset the NFI core state machine, data FIFO and flushing FIFO
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    *NFI_CNFG = OP_READ_ID_ST;

    // always use 8bits I/O interface to read device id
    nfi_pagefmt = *NFI_PAGEFMT;
    *NFI_PAGEFMT = (nfi_pagefmt&(~PAGEFMT_16BITS))|PAGEFMT_8BITS;

    // read id cmd
    *NFI_CMD = NAND_CMD_READID;
    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    // issue addr
    *NFI_COLADDR = 0;
    *NFI_ROWADDR = 0;
    *NFI_ADDRNOB = 1;
    // wait til ADDR is completely issued
    while( *NFI_STA  & STATUS_ADDR );

    // set single read, read 8 bytes
    *NFI_CON = SINGLE_RD;
    // wait til DATA_READ is completely issued
    while( *NFI_STA  & STATUS_DATAR );
    while(FIFO_RD_REMAIN(*NFI_FIFOSTA)<4);

    p4Data = (u32*)this->buffers->databuf;
    p4Data[0] = *NFI_DATAR;
    p4Data[1] = *NFI_DATAR;

    *NFI_PAGEFMT = nfi_pagefmt;
}

//------------------------------------------------------------------------------
// Read Status Callback Function
//------------------------------------------------------------------------------
STATUS_E  NAND_COMMON_ReadStatus(const struct mtd_info *mtd, const u32  c_timeout)
{
    register struct nand_chip *ac83xx = mtd->priv;
    u32* p4Data = (u32*)ac83xx->buffers->databuf;

    *NFI_CON = NFI_RST | FIFO_FLUSH;
    *NFI_CNFG = OP_READ_ID_ST;

    // read status cmd
    *NFI_CMD = NAND_CMD_STATUS;
    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    // set single read by DWORD
    *NFI_CON = SINGLE_RD | NOB_DWORD;
    // wait til DATA_READ is completely issued
    while( *NFI_STA  & STATUS_DATAR );

    // single read doesn't need to polling FIFO
    p4Data[0] = *NFI_DATAR;

    return S_DONE;
}

//------------------------------------------------------------------------------
// Block Erase Related Callback Function
//------------------------------------------------------------------------------
STATUS_E  NAND_COMMON_BlockErase(const struct mtd_info *mtd, const u32  page_addr)
{
    u32    page_size = mtd->writesize;
    s32 i4Val;
    struct nand_chip *ac83xx = mtd->priv;
    u32 timeout;
    u32  addr_cycle, column_addr_bytes,row_addr_bytes;
    STATUS_E ret = S_UNKNOWN_ERR;

    if (page_size > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    // reset the NFI core state machine, data FIFO and flushing FIFO
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    *NFI_CNFG = OP_ERASE ;
    *NFI_INTR_EN = ERASE_DONE_EN;
    *NFI_CSEL = 0;
    // block erase cmd
    *NFI_CMD = NAND_CMD_ERASE1;
    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    // fill 1~4 cycle addr, erase command only fill row address, so column bits shift is unnecessary
    *NFI_COLADDR = 0;
    *NFI_ROWADDR = page_addr;
    // no. of addr cycle
    *NFI_ADDRNOB = ROW_ADDR_NOB(row_addr_bytes);
    // wait til ADDR is completely issued
    while( *NFI_STA  & STATUS_ADDR );

    // block erase confirm
    *NFI_CMD = NAND_CMD_ERASE2;
    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    timeout = MTD_NAND_DEFAULT_TIMEOUT;

    NFI_Wait( (ERASE_DONE != (*NFI_INTR&ERASE_DONE)), timeout); //clear INT status
    *NFI_INTR_EN &= ~ERASE_DONE_EN; // disable INT first
    i4Val =  *NFI_INTR; // read clear
#ifdef INT_WR_CLR
    *NFI_INTR = i4Val;
#endif
    //UNUSED(i4Val);
    if( 0 == timeout) {
          ret = S_TIMEOUT;
          goto end;
    }

    ac83xx->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
    ret=(ac83xx->read_byte(mtd) & NAND_STATUS_FAIL) ? S_ERASE_FAILED: S_DONE;


end:

    *NFI_CON = 0;
    *NFIECC_ENCCON = 0x0;

    if (ret != S_DONE){
    printf("it is a bad block\r\n");
    } else {
        //printf("page 0x%x erase done \n", page_addr);
#if NAND_BLANK_CHECK
        nand_blank_check(mtd,page_addr,mtd->erasesize);
#endif
    }
    return ret;
}

//------------------------------------------------------------------------------
// Write To NFI FIFO
//------------------------------------------------------------------------------
STATUS_E  NAND_COMMON_FIFO_Write(
        const struct nand_chip *ac83xx
        ,const u32  c_timeout
        ,const BOOL bUsingDMA
        ,const u32 *p_data32 /* MUST be 32bits alignment addr */
        ,const u32 data_len
) {
    u32  timeout = c_timeout;
    u32  i;
    //printf("****[MTD][%s: %u]\r\n", __FUNCTION__, __LINE__);
    if(bUsingDMA)
    {
        // wait for DMA transmission complete
        timeout = c_timeout;
        NFI_Wait( (AHB_DONE != (*NFI_INTR&AHB_DONE)), timeout); //clear INT status
        *NFI_INTR_EN &= ~AHB_DONE; // disable INT
        i = *NFI_INTR;//read clear again
#ifdef INT_WR_CLR
        *NFI_INTR = i;
#endif
        if( 0 == timeout)
        {
        printf("[MTD][%s: %u]DMA mode timeout\r\n", __FUNCTION__, __LINE__);
        return S_TIMEOUT;
    }
    }
    else
    {
        // program page data
        for(i=0; i<data_len; i+=4, p_data32++)
        {
            // wait for FIFO has space to enqueue
            // when WR_FULL_MASK flag is poll-down, it means there are at least 4 bytes free space in FIFO.
            timeout = c_timeout;
            NFI_Wait( (*NFI_FIFOSTA & WR_FULL_MASK), timeout);
            if( 0 == timeout )
            {
            printf("[MTD][%s: %u]polling mode timeout\r\n", __FUNCTION__, __LINE__);
            return S_TIMEOUT;
        }
            *NFI_DATAW = *p_data32;
        }
    }
    return S_DONE;
}

//------------------------------------------------------------------------------
// Page Program Callback Function
//------------------------------------------------------------------------------
STATUS_E  NAND_COMMON_SEQIN(const struct mtd_info *mtd, const u32  c_timeout, const u32  page_addr)
{
    register struct nand_chip *ac83xx = mtd->priv;
    u32    page_size;
    u32    column_addr_bytes,row_addr_bytes;
    u32    addr_cycle;
    u32* p_data32 = (u32*)ac83xx->buffers->databuf;
#ifdef _XOS_ISR_ENABLE_
    s32 i4Val;
    EV_GRP_EVENT_T rRecvEvt;
#endif //_XOS_ISR_ENABLE_

    page_size = mtd->writesize;

    flush_invalid_cache((unsigned int)p_data32, (unsigned int)page_size);

    if (page_size > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    // reset the NFI core state machine, data FIFO and flushing FIFO
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    *NFI_CNFG = OP_PROGRAM ;
    if (_fgAUTO_FMT)
    *NFI_CNFG |= AUTO_FMT_EN | HW_ECC_EN;
    if (page_size == 512)
    *NFI_CNFG |= SEL_SEC_512BYTE;

    *NFIECC_ENCCNFG = ENC_TNUM(global_ecc_strength) | ENC_MS( (SECTOR_BYTES + glb_fdm_ecc_size) << 3 ) | ENC_NFI_MODE;

    *NFIECC_ENCCON = 0;
    *NFIECC_ENCCON = ENC_EN;

    if (_fgUsingDMA)
    {
        *NFI_CNFG |= AHB_MODE ;
        *NFI_STRADDR = ((u32)p_data32) + 0xC0000000;
    }

    // in most 512 page size NAND flash, you have to setup destination pointer to 1st half area
    if(page_size <=512) {
        *NFI_CMD = NAND_CMD_READ0;
        // wait til CMD is completely issued
        while( *NFI_STA  & STATUS_CMD );
        *NFI_CON = NFI_RST;
        while( *NFI_STA  & STATUS_CMD );
    }
    *NFI_CON = SEC_NUM(page_size/512);
    // program cmd
    *NFI_CMD = NAND_CMD_SEQIN;
    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    // fill 1~4 cycle addr
    *NFI_COLADDR = 0;
    *NFI_ROWADDR = page_addr;
    // no. of addr cycle
    //*NFI_ADDRNOB = addr_cycle;
    *NFI_ADDRNOB = COL_ADDR_NOB(column_addr_bytes) | ROW_ADDR_NOB(row_addr_bytes);
    // wait til ADDR is completely issued
    while( *NFI_STA  & STATUS_ADDR );

    return S_DONE;
}


//------------------------------------------------------------------------------
// Page Program Callback Function
//------------------------------------------------------------------------------
STATUS_E  NAND_COMMON_PageProgram(const struct mtd_info *mtd, const u32  c_timeout, const u32  page_addr)
{
    register struct nand_chip *ac83xx = mtd->priv;
    u32    page_size, spare_size;
    u32    column_addr_bytes,row_addr_bytes;
    u32    addr_cycle;
    u32     timeout;
    STATUS_E  ret=S_UNKNOWN_ERR;
    u32 i;
    u32* p_data32 = (u32*)ac83xx->buffers->databuf;

    printf("****[MTD][%s: %u]page_addr=0x%x\r\n", __FUNCTION__, __LINE__,page_addr);
    page_size =  mtd->writesize;
    spare_size = mtd->oobsize;

    if (page_size > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    // prepare FDM data
    if ( AUTO_FMT_EN & (*NFI_CNFG))
    {
        volatile u32* pFDMAddr = NFI_FDM0L;
        volatile u32* pTmpAddr = (u32*)(ac83xx->buffers->databuf+ page_size);
        u32 p4TmpBuf[128];
        volatile u32* p4TmpSpare2 = pTmpAddr;

        for (i = 0; i < (page_size/512) ; i++)
        {
            memcpy((void*)p4TmpBuf,(void*) p4TmpSpare2, glb_fdm_size);
            *pFDMAddr++ = p4TmpBuf[0];
            *pFDMAddr++ = p4TmpBuf[1];
            p4TmpSpare2 = (u32*)((u32)p4TmpSpare2 + glb_fdm_size);
        }

    }
    if (_fgUsingDMA)
        *NFI_INTR_EN = AHB_DONE_EN ;
    i = *NFI_INTR;  // read clear
#ifdef INT_WR_CLR
    *NFI_INTR = i;
#endif
    // set burst program by DWORD
    *NFI_CON |= BURST_WR | NOB_DWORD ;

    flush_invalid_cache((unsigned int)p_data32, (unsigned int)page_size);

    // program page data
    if( S_DONE != (ret=NAND_COMMON_FIFO_Write(ac83xx, c_timeout, _fgUsingDMA, (u32*)p_data32, page_size)) ) {
        goto end;
    }

    if ( !(*NFI_CNFG & (AUTO_FMT_EN | AHB_MODE)) ) // NFI will automatically fetch data for spare under AHB_MODE, and NFI will
    {                                                                          // also fetch data from FDM register under AUTO_FMT mode
        volatile u32* p_spare32 = (u32*)(ac83xx->buffers->databuf+ page_size);
        u32 i;
        for(i=0; i<spare_size; i+=4, p_spare32++)
        {
            // wait for FIFO has space to enqueue
            // when WR_FULL_MASK flag is poll-down, it means there are at least 4 bytes free space in FIFO.
            timeout = c_timeout;
            NFI_Wait( (*NFI_FIFOSTA & WR_FULL_MASK), timeout);
            if( 0 == timeout ) {
                ret = S_TIMEOUT;
                goto end;
            }
            *NFI_DATAW = *p_spare32;
        }
    }

    // <<<<  WARNING!! >>>>
    // 1. You MUST read parity registers before issue program confirm (0x10) command.
    //    Since the parity registers will be clean by NFI after issue program confirm.
    // 2. You MUST wait until the NFI FIFO is empty!
    //    It means all data in the FIFO had been written to NAND flash, and then you can
    //    start to read ECC parity registers.
    //while(!(*NFI_FIFOSTA & WR_EMPTY_MASK));
    //while ( *NFI_ADDRCNTR & 0x3FF);
    timeout = c_timeout;
    NFI_Wait( ((*NFI_ADDRCNTR & 0xF000) >> 12) != (page_size/SECTOR_BYTES) , timeout);
    if( 0 == timeout ) {
        ret = S_TIMEOUT;
        goto end;
    }

    *NFI_INTR_EN |= WR_DONE_EN;
    // program confirm
    *NFI_CMD = NAND_CMD_PAGEPROG;
    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    timeout = c_timeout;
    NFI_Wait( (WR_DONE != (*NFI_INTR&WR_DONE)), timeout); //clear INT status
    *NFI_INTR_EN &= ~WR_DONE; // disable INT first
    //i =  *NFI_INTR; // read clear
    if( 0 == timeout) {
          ret = S_TIMEOUT;
          goto end;
    }

    ret = S_DONE;
end:
    *NFI_CON = 0;
    *NFIECC_ENCCON = 0x0;
    if ( ret != S_DONE)
        printk("\nwrite page error, error code = 0x%x\n", ret);

    return ret;
}

static void atc_nand_cmdfunc(struct mtd_info *mtd, unsigned command,
                  u32 column, u32 page_addr)
{
    register struct nand_chip *this = mtd->priv;
    u32    page_size = mtd->writesize;
    _data_buf_index = 0;

    if (command == NAND_CMD_ERASE2 ) {
        /* Second half of a command we already calculated */
        goto do_command;
    }

    /* Emulate NAND_CMD_READOOB on large-page chips */
    if (page_size > 512 &&  command == NAND_CMD_READOOB) {
        //column += mtd->writesize;
        command = NAND_CMD_READ0;
    }

    switch (command)
    {
        case NAND_CMD_READID:
            NAND_COMMON_ReadID(mtd, command);
             break;

        case NAND_CMD_READ0:
            glb_read_pageaddr = page_addr;
            break;

        case NAND_CMD_STATUS:
            NAND_COMMON_ReadStatus(mtd,MTD_NAND_DEFAULT_TIMEOUT);
            break;

        case NAND_CMD_ERASE1:
            NAND_COMMON_BlockErase(mtd, page_addr);
            break;

        case NAND_CMD_SEQIN:
            NAND_COMMON_SEQIN(mtd,MTD_NAND_DEFAULT_TIMEOUT,page_addr);
            break;

        case NAND_CMD_PAGEPROG:
            NAND_COMMON_PageProgram(mtd,MTD_NAND_DEFAULT_TIMEOUT,page_addr);
            break;

        case NAND_CMD_RESET:
            NAND_COMMON_Reset(mtd, MTD_NAND_DEFAULT_TIMEOUT);
            break;

        default:
            printk("\ncommand code = 0x%x\n", command);
            BUG();

    }

do_command:
    return;
}

static void atc_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
    if(1==chipnr)
        *NFI_CSEL = chipnr|RB_CS1;
    else
    *NFI_CSEL = chipnr;
}

static int atc_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, u32 page, int sndcmd)
{
    return atc_nand_read_ecc(mtd, page, NULL);
}

static int atc_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip, uint8_t *buf)
{
    return atc_nand_read_ecc(mtd, glb_read_pageaddr, (u32 *)buf);
}

static int atc_nand_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip, uint8_t *buf)
{
    return atc_nand_read_raw_with_autofdm(mtd, glb_read_pageaddr, (u32 *)buf);
}


static int buffer_check(u_char *buff, uchar val , ulong len)
{
    int i=0;
    for(i=0; i< len; i++){
        if (buff[i] != val){
            printf("buff[%d] = 0x%x is not %x \n",i,buff[i],val);
            return 1;
        }
    }
    return 0;
}
static int nand_blank_check(struct mtd_info *mtd, u32 page, u32 len)
{
    u_char *read_buff;
    read_buff = malloc(mtd->writesize);
    int ret = 0;
    while(len > 0){
        ret = atc_nand_read_ecc(mtd, page, (u32 *)read_buff);
        if(!ret) {
            ret = buffer_check(read_buff, 0xff, mtd->writesize);
        }
        page++;
        len -= mtd->writesize;
        if(ret){
            printf("page 0x%x check blank fail \n",page);
            break;
        }
    }

    free(read_buff);

    return ret;
}

#define MTK_MARK_BAD_BLOCK 1
#if MTK_MARK_BAD_BLOCK
uint32_t _setbadblk = 0;
#endif
int atc_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
                const uint8_t *buf, u32 page, int cached, int raw)
{
    register struct nand_chip *ac83xx = mtd->priv;
    u32    page_size, spare_size;
    u32    column_addr_bytes,row_addr_bytes;
    u32    addr_cycle;
    u32     timeout;
    u32 c_timeout = MTD_NAND_DEFAULT_TIMEOUT;
    STATUS_E  ret=S_UNKNOWN_ERR;
    u32 i;

#if MTK_MARK_BAD_BLOCK
    BOOL temp = _fgAUTO_FMT;
    if (_setbadblk)
    {
      _fgAUTO_FMT = FALSE;
    }
#endif
#if NAND_BLANK_CHECK
    if (nand_blank_check(mtd, page, mtd->writesize))
        return -1;
#endif
    page_size =  mtd->writesize;
    spare_size = mtd->oobsize;

    flush_invalid_cache((unsigned int)buf, (unsigned int)(page_size + SECTOR_BYTES));

    if (mtd->writesize > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    // reset the NFI core state machine, data FIFO and flushing FIFO
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    *NFI_CNFG = OP_PROGRAM ;
    if (_fgAUTO_FMT)
        *NFI_CNFG |= AUTO_FMT_EN | HW_ECC_EN;
    if (page_size == 512)
        *NFI_CNFG |= SEL_SEC_512BYTE;

    *NFIECC_ENCCNFG = ENC_TNUM(global_ecc_strength) | ENC_MS( (SECTOR_BYTES + glb_fdm_ecc_size) << 3 ) | ENC_NFI_MODE;

    *NFIECC_ENCCON = 0;
    *NFIECC_ENCCON = ENC_EN;
    if (_fgUsingDMA)
    {
        *NFI_CNFG |= AHB_MODE ;
        *NFI_STRADDR = ((u32)buf)|0xC0000000;
    }

    // in most 512 page size NAND flash, you have to setup destination pointer to 1st half area
    if(page_size <=512) {
        *NFI_CMD = NAND_CMD_READ0;
        // wait til CMD is completely issued
        while( *NFI_STA  & STATUS_CMD );
        *NFI_CON = NFI_RST;
        while( *NFI_STA  & STATUS_CMD );
    }
    *NFI_CON = SEC_NUM(page_size/SECTOR_BYTES);
    // program cmd
    *NFI_CMD = NAND_CMD_SEQIN;
    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    // fill 1~4 cycle addr
    *NFI_COLADDR = 0;
    *NFI_ROWADDR = page;

    // no. of addr cycle
    //*NFI_ADDRNOB = addr_cycle;
    *NFI_ADDRNOB = COL_ADDR_NOB(column_addr_bytes) | ROW_ADDR_NOB(row_addr_bytes);
    // wait til ADDR is completely issued
    while( *NFI_STA  & STATUS_ADDR );

    // prepare FDM data
    if ( AUTO_FMT_EN & (*NFI_CNFG))
    {
        volatile u32* pFDMAddr = NFI_FDM0L;
        volatile u32* pTmpAddr = (u32*)(ac83xx->buffers->databuf+ page_size);
        u32 p4TmpBuf[128];

        for (i = 0; i < (page_size/SECTOR_BYTES) ; i++)
        {
            memcpy((void*)p4TmpBuf,(void*) pTmpAddr, glb_fdm_size);
            *pFDMAddr++ = p4TmpBuf[0];
            *pFDMAddr++ = p4TmpBuf[1];
            *pFDMAddr++ = p4TmpBuf[2];
            *pFDMAddr++ = p4TmpBuf[3];
            pTmpAddr = (u32*)((u32)pTmpAddr + glb_fdm_size);
        }

    }
    if (_fgUsingDMA)
        *NFI_INTR_EN = AHB_DONE_EN ;
    i = *NFI_INTR;  // read clear
#ifdef INT_WR_CLR
    *NFI_INTR = i;
#endif
    // set burst program by DWORD
    *NFI_CON |= BURST_WR ;

    // program page data
    if( S_DONE != (ret=NAND_COMMON_FIFO_Write(mtd->priv, c_timeout, _fgUsingDMA, (u32*)buf, page_size)) ) {
        goto end;
    }

    if ( !(*NFI_CNFG & (AUTO_FMT_EN | AHB_MODE)) ) // NFI will automatically fetch data for spare under AHB_MODE, and NFI will
    {                                                                          // also fetch data from FDM register under AUTO_FMT mode
        volatile u32* p_spare32 = (u32*)(buf + mtd->writesize);
        u32 i;
        for(i=0; i<spare_size; i+=4, p_spare32++)
        {
            // wait for FIFO has space to enqueue
            // when WR_FULL_MASK flag is poll-down, it means there are at least 4 bytes free space in FIFO.
            timeout = c_timeout;
            NFI_Wait( (*NFI_FIFOSTA & WR_FULL_MASK), timeout);
            if( 0 == timeout ) {
                printf("[MTD][%s: %u]timeout\r\n", __FUNCTION__, __LINE__);
                ret = S_TIMEOUT;
                goto end;
            }
            *NFI_DATAW = *p_spare32;
        }
    }

    // <<<<  WARNING!! >>>>
    // 1. You MUST read parity registers before issue program confirm (0x10) command.
    //    Since the parity registers will be clean by NFI after issue program confirm.
    // 2. You MUST wait until the NFI FIFO is empty!
    //    It means all data in the FIFO had been written to NAND flash, and then you can
    //    start to read ECC parity registers.
    //while(!(*NFI_FIFOSTA & WR_EMPTY_MASK));
    //while ( *NFI_ADDRCNTR & 0x3FF);
    timeout = c_timeout;

    NFI_Wait( ((*NFI_ADDRCNTR & 0xF000) >> 12) != (page_size/SECTOR_BYTES) , timeout);
    if( 0 == timeout ) {
        printf("[MTD][%s: %u]timeout\r\n", __FUNCTION__, __LINE__);
        ret = S_TIMEOUT;
        goto end;
    }

    *NFI_INTR_EN |= WR_DONE_EN;
    // program confirm
    *NFI_CMD = NAND_CMD_PAGEPROG;
    // wait til CMD is completely issued
    while( *NFI_STA  & STATUS_CMD );

    timeout = c_timeout;

    NFI_Wait( (WR_DONE != (*NFI_INTR&WR_DONE)), timeout); //clear INT status
    *NFI_INTR_EN &= ~WR_DONE; // disable INT first
    //i =  *NFI_INTR; // read clear
    if( 0 == timeout) {
        printf("[MTD][%s: %u]timeout\r\n", __FUNCTION__, __LINE__);
        ret = S_TIMEOUT;
        goto end;
    }

    ret = S_DONE;
end:

#if MTK_MARK_BAD_BLOCK
    _fgAUTO_FMT = temp;
#endif
    *NFI_CON = 0;
    *NFIECC_ENCCON = 0x0;

    ac83xx->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
    ret=(ac83xx->read_byte(mtd) & NAND_STATUS_FAIL) ? S_PGM_FAILED: S_DONE;

    if ( ret != S_DONE)
    {
        printk("\natc_nand_write_page, error code = 0x%x\n", ret);
        return -1;
    }

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
    if (((page * page_size) >= 0x9A00000) && ((page * page_size) <= 0xA600000))
    {
        chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
        if (-1==atc_nand_read_page(mtd, chip,ac83xx->buffers->databuf))
        {
            printk("read verify read failed at page 0x%x\n", page);
            BUG();
            return -EIO;
        }
        if (memcmp(ac83xx->buffers->databuf, buf, page_size)!=0)
        {
            printk("read verify compare failed at page 0x%x\n", page);
            BUG();
            return -EIO;
        }
    }
#endif

    return 0;
}


static void atc_nand_read_buf(struct mtd_info *mtd, unsigned char *buf, int len)
{
    register struct nand_chip *this = mtd->priv;

    //printk("\nread buffer, offset= 0x%x\n", ac83xx->datalen);

    flush_invalid_cache((unsigned int)(this->buffers->databuf+_data_buf_index), (unsigned int)len);
    memcpy(buf, this->buffers->databuf+_data_buf_index, len);

    _data_buf_index+= len;

}

static void atc_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
    register struct nand_chip *this = mtd->priv;

    //printk("\nwrite buffer, offset= 0x%x\n", ac83xx->datalen);
    memcpy(this->buffers->databuf + _data_buf_index, buf, len);
    flush_cache((unsigned int)(this->buffers->databuf +_data_buf_index), len);

    _data_buf_index += len;

}


static u_char atc_nand_read_byte(struct mtd_info *mtd)
{
    unsigned char d;

    atc_nand_read_buf(mtd, &d, 1);

    return d;
}

static u16 atc_nand_read_word(struct mtd_info *mtd)
{
    u16 d;

    atc_nand_read_buf(mtd, (u_char*)&d, 2);

    return d;
}

static int atc_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
    return 0;
}

static int atc_nand_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
    return 0;
}

static void atc_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{}

int board_nand_init(struct nand_chip *nand)
{
    printf("\r\n[MTD][%s: %u] Use 24bit NFI\r\n", __FUNCTION__, __LINE__);
    nand->options |= NAND_USE_FLASH_BBT| NAND_NO_SUBPAGE_WRITE;

    printf("[MTD]NFI_BASE=0x%x,NFIECC_BASE=0x%x\r\n",NFI_BASE,NFIECC_BASE);

    /* Set address of NAND IO lines (Using Linear Data Access Region) */
    nand->IO_ADDR_R = (void __iomem *) NFI_BASE;
    nand->IO_ADDR_W = (void __iomem *) NFI_BASE;
    /* Reference hardware control function */
    nand->dev_ready  = atc_nand_device_ready;
    nand->select_chip = atc_nand_select_chip;

    //nand->write_byte = nand_write_byte16 : nand_write_byte;
    nand->read_byte = atc_nand_read_byte;
    //nand->write_word = nand_write_word;
    nand->read_word = atc_nand_read_word;
    //nand->write_buf = nand_write_buf16 : nand_write_buf;
    nand->read_buf = atc_nand_read_buf;
    nand->write_buf = atc_nand_write_buf;

    nand->cmdfunc = atc_nand_cmdfunc;
    nand->write_page = atc_nand_write_page;

    nand->ecc.mode = NAND_ECC_HW;
    /* we need calulate, correct, hwctl: to avoid complain of mtdcore */
    nand->ecc.calculate = atc_nand_calculate_ecc;
    nand->ecc.correct = atc_nand_correct_data;
    nand->ecc.hwctl = atc_nand_enable_hwecc;

    nand->ecc.read_page = atc_nand_read_page;
    nand->ecc.read_page_raw = atc_nand_read_page;
    nand->ecc.read_oob = atc_nand_read_oob;

    nand->ecc.bytes = 12;
    nand->ecc.size = 1024;

    return 0;
}

int atc_nand_init(struct mtd_info *mtd, struct nand_flash_dev *nand_type)
{
    struct nand_chip *this = mtd->priv;
    u32 spare_size =0;
    u32  clk=0;

    /* config nand timing*/
    clk=*NFI_CLK_SEL;
    clk&=~(0x7U<<24);
    clk|=(0x3U<<24);
    *NFI_CLK_SEL = clk;
    //*NFI_ACCCON = (LCD2NAND| PRECS|C2R | W2R | WH | WST | RLT);
    if(nand_type)
        *NFI_ACCCON = nand_type->timingsetting;
    printf("[nand] set NFI_ACCCON: 0x%08x\n", *NFI_ACCCON);

    if(mtd->writesize == 512) {
    glb_fdm_size = 8;
    glb_fdm_ecc_size = 8;
    }
    else {
    glb_fdm_size = FDM_BYTES;
    glb_fdm_ecc_size = FDM_ECC_BYTES;
    }
    *NFI_PAGEFMT = FDM_ECC_NUM(glb_fdm_ecc_size) | FDM_NUM(glb_fdm_size);

      // setup NFI page format and I/O interface
    *NFI_PAGEFMT |= PAGEFMT_8BITS ;

    switch (mtd->writesize)
    {
      case 512:
          *NFI_CNFG |= SEL_SEC_512BYTE;
          *NFI_PAGEFMT |= PAGEFMT_2K_512;
        break;
      case 2048:
          *NFI_PAGEFMT |= PAGEFMT_2K_512;
        break;
      case 4096:
          *NFI_PAGEFMT |= PAGEFMT_4K_2K;
        break;
      case 8192:
          *NFI_PAGEFMT |= PAGEFMT_8K_4K;
        break;
    }

    spare_size = mtd->oobsize /(mtd->writesize/SECTOR_BYTES);
    if(spare_size==56)//for same to preloader setting
        spare_size=52;
    printf("spare_size=%d SECTOR_BYTES=%d \n", spare_size, SECTOR_BYTES);
    switch (spare_size)
    {
      case 16:
      case 32:
          *NFI_PAGEFMT |= SPARE_32_16;
          break;
      case 26:
      case 52:
          *NFI_PAGEFMT |= SPARE_52_26;
          break;
      case 28:
      case 56:
          *NFI_PAGEFMT |= SPARE_56_28;
          break;
      default:
          printf("####[atc_nand_init] spare size is not configured %d %d %d \r\n", spare_size, mtd->oobsize, mtd->writesize);
    }

    select_ecc_strength(mtd);

#if USE_MTD_OOB_AUTO
    switch (mtd->writesize) {
        case 512:
            this->ecc.layout = &atc_oobinfo_512_4bit_ecc;
            break;
        case 2048:
        if(global_ecc_strength == ECC_12_BITS)
        this->ecc.layout = &atc_oobinfo_2K_12bit_ecc;
        else if(global_ecc_strength == ECC_24_BITS)
        this->ecc.layout = &atc_oobinfo_2K_24bit_ecc;

            break;
        case 4096:
        if(global_ecc_strength == ECC_24_BITS)
        this->ecc.layout = &atc_oobinfo_4K_24bit_ecc;
            break;
        default:
            printk(KERN_WARNING "No oob scheme defined for pagesize %d oobsize %d\n", mtd->writesize, mtd->oobsize);
    }
#else
    /* We don't use MTD_OOB_AUTO, so just set to NULL */
    this->ecc.layout = NULL;
#endif

    this->bbt_td = &atc_bbt_main_descr_512;
    this->bbt_md = &atc_bbt_mirror_descr_512;

#if 0
    {
        // Initialize Address Translation Table
        u32  blocks;
        u32  loop;
        struct nand_log2phy_tbl *att;
        blocks = this->chipsize >> this->bbt_erase_shift;
        att = malloc(sizeof(struct nand_log2phy_tbl) + sizeof(u16)* blocks);

        memset(att, 0, sizeof(struct nand_log2phy_tbl) + sizeof(u16)* blocks);

        att->blocknum = blocks;
        att->page2blkshift = this->bbt_erase_shift - this->page_shift;
        att->page2blkmask = (1 << att->page2blkshift) - 1;

        printf("ATT block(%d), bshift(%d), pshift(%d) p2bshift(%d) pagemask(0x%x)\r\n", att->blocknum,
                this->bbt_erase_shift, this->page_shift, att->page2blkshift, att->page2blkmask);

        for(loop = 0; loop < blocks; loop ++)
        {
            att->log2phytbl[loop] = loop;
        }
        this->priv = att;

    }
#endif

    return 0;
}

#if 0
int atc_nand_init_att(struct nand_chip *chip, struct nand_atc_bbt * atcbbt)
{
    u32 logblk;
    u16 phyblk;
    u16 bbidx = 0;

    struct nand_log2phy_tbl *att = (struct nand_log2phy_tbl *) chip->priv;
    if (atcbbt->realnum)
    {
        u16 firstrsvb = att->blocknum;
        if(atcbbt->rsvbn)
            firstrsvb = (u16) atcbbt->phyidx[0];
        phyblk = 0;
        for (logblk=0; phyblk < firstrsvb ; logblk ++, phyblk ++)
        {
            // Find a goog physical block for current logical block
            while ((atcbbt->realnum > bbidx) && (atcbbt->badblocks[bbidx] == phyblk))
            {
                phyblk ++;
                bbidx ++;
            }
            att->log2phytbl[logblk] = phyblk;
        }
        for (;logblk < att->blocknum; logblk ++)
        {
            att->log2phytbl[logblk] = 0xFFFF;
        }
    }
    for (bbidx = 0; bbidx < atcbbt->rsvbn; bbidx ++)
    {
        if (atcbbt->logidx[bbidx] < att->blocknum)
        {
            att->log2phytbl[atcbbt->logidx[bbidx]] = atcbbt->phyidx[bbidx];
        }
    }
    return (0);
}
#endif

STATUS_E atc_common_ecc_err_detect(struct mtd_info *mtd, const u32  c_timeout)
{
    u32 i, timeout, page_addr;
    u32 page_size = mtd->writesize;
    u32 errbits = 0;
    STATUS_E ret = S_DONE;
    u32 u4Tmp = 0;

    if ( (*NFIECC_DECFER & 0xff) == 0)//NFI mode
    {
        return S_DONE;
    }
    else
    {
        u4Tmp = *NFIECC_DECENUM; // error number
        printf("ECC Error happend:%d.\n", u4Tmp);
        //nanddump_register();
        for ( i = 0 ; i < 8 ; i++)
        {
            if(i == 4)
                u4Tmp = *NFIECC_DECENUM2; // error number 2

            if ( DECENUM_MASK == (u4Tmp & DECENUM_MASK))
            {
                printk("[MTD][%s: %u]Page 0x%x Sector %d with ECC un-correctable Errorr\n",__FUNCTION__, __LINE__, *NFI_ROWADDR, i);
                //nanddump_register();
                return S_ECC_UNCORRECT_ERR;
            }
            else if ( u4Tmp & DECENUM_MASK)
            {
                errbits = (u4Tmp&DECENUM_MASK);
                ret = S_ECC_CORRECTABLE_ERR;
#ifdef CONFIG_NAND_DEBUG_VERSION
                page_ecc_nums++;
                totel_ecc_bits+=errbits;
                if(max_ecc_bits<errbits)
                    max_ecc_bits=errbits;
                printf("[MTD]Page 0x%x Sector %d: correct %d bits\n", *NFI_ROWADDR, i, errbits);
#endif
            }
            u4Tmp >>= 8;

        }
    }


    u16 u2ErrLoc;

    u4Tmp = *NFIECC_DECENUM; // error number
    for (i = 0 ; i < u4Tmp ; i++)
    {
        u2ErrLoc = *(UINT16*)((UINT32)NFIECC_DECEL0 + i*2);
        if ( (u2ErrLoc /8) < SECTOR_BYTES)
        {
            printf("main ECC correct\n");
            break;
        }
        else
        {
            printf("FDM ECC correct\n");
            break;
        }
    }
    return ret;
}

#if 1
/* ECC Error Software Correct */
STATUS_E atc_common_ecc_err_correct(u32 c_timeout, u32 u4SectIdx, u32 *p_data32 /* MUST be 32bits alignment addr */)
{
    u32 timeout;
    u32 i;
    u32 u4ErrNum;
    u32 u4ErrVal;
    u16 u2ErrLoc;
    STATUS_E ret = S_UNKNOWN_ERR;
    u32 u4ErrLoc;
    timeout = c_timeout;

    NFI_Wait( !(*NFIECC_DECDONE & (1 << u4SectIdx)), timeout); // wait for all block decode done
    if( 0 == timeout ) {
        printf("[MTD][%s: %u] NFI_Wait TimeOut\n", __FUNCTION__, __LINE__);
        nanddump_register();
        ret = S_TIMEOUT;
        goto end;
    }

    if (u4SectIdx <4)
        u4ErrNum = (*NFIECC_DECENUM & (DECENUM_MASK << (u4SectIdx*8))) >> (u4SectIdx*8);
    else
        u4ErrNum = (*NFIECC_DECENUM2 & (DECENUM_MASK << ((u4SectIdx-4)*8))) >> ((u4SectIdx-4)*8);

    if ( DECENUM_MASK == u4ErrNum)
    {
      ret = S_ECC_UNCORRECT_ERR;
      printf("[MTD][%s: %u] return S_ECC_UNCORRECT_ERR\n", __FUNCTION__, __LINE__);
      goto end;
    }
    else if ( 0x0 == u4ErrNum)
    {
      ret = S_DONE;
      goto end;
    }
    else
    {
    printf("[MTD][%s: %u] u4ErrNum = %d\n", __FUNCTION__, __LINE__, u4ErrNum);
        for (i = 0 ; i < u4ErrNum ; i++)
        {
            u2ErrLoc = *(UINT16*)((UINT32)NFIECC_DECEL0 + i*2);
            if ( (u2ErrLoc /8) < SECTOR_BYTES)
            {
                u4ErrLoc = (UINT32)p_data32+u4SectIdx*SECTOR_BYTES+u2ErrLoc/8;
                u4ErrVal = *(UINT8*)u4ErrLoc;
                u4ErrVal = u4ErrVal & (1 << (u2ErrLoc%8));
                if (u4ErrVal)
                    *(UINT8*)u4ErrLoc &= (~u4ErrVal);
                else
                    *(UINT8*)u4ErrLoc |= (1 << (u2ErrLoc%8));
            }
            else
            {
                u4ErrLoc = (UINT32)NFI_FDM0L+u4SectIdx*8 + (((u2ErrLoc/8) -SECTOR_BYTES)/4)*4;
                u4ErrVal = *(UINT32*)u4ErrLoc;
                u4ErrVal = u4ErrVal & (1 << ((u2ErrLoc - SECTOR_BYTES*8)%32));
                if (u4ErrVal)
                    *(UINT32*)u4ErrLoc &= (~u4ErrVal);
                else
                    *(UINT32*)u4ErrLoc |= (1 << ((u2ErrLoc - SECTOR_BYTES*8)%32));
            }
        }
        ret = S_ECC_CORRECTABLE_ERR;
    }
end:
    return ret;
}


//p_data32 =>  MUST be 32bits alignment addr
STATUS_E  atc_common_fifo_read(u32 c_timeout, BOOL bUsingDMA, u32 *p_data32, u32 data_len)
{
    u32  timeout = c_timeout;
    u32  i;

    if(bUsingDMA)//DMA
    {
        NFI_Wait((AHB_DONE != (*NFI_INTR & AHB_DONE)), timeout);
        *NFI_INTR_EN &= ~AHB_DONE_EN;
         i = *NFI_INTR;//read clear again
         *NFI_INTR = i;
         if( 0 == timeout)
         {
        printf("[MTD][%s: %u] NFI_Wait TimeOut\n", __FUNCTION__, __LINE__);
        return S_TIMEOUT;
     }
    }
    else//PIO
    {
    for(i=0; i < data_len; i+=4)
        {
            // when RD_EMPTY_MASK flag is poll-down, it means data is ready in FIFO at least 4 bytes.
            timeout = c_timeout;
            NFI_Wait((*NFI_FIFOSTA & RD_EMPTY_MASK), timeout);
            if(0 == timeout)
            {
                printf("[MTD][%s: %u] NFI_Wait TimeOut,i=%d\n", __FUNCTION__, __LINE__,i);
                //nanddump_register();
                return S_TIMEOUT;
            }
            *(u32*)((u32)p_data32 + i) = *NFI_DATAR;
            if (((i+4) % SECTOR_BYTES) == 0)
            {
                if (_fgECCSWCorrect)
                {
                    STATUS_E ret = S_UNKNOWN_ERR;
                    ret = atc_common_ecc_err_correct(c_timeout, ((i+4)/SECTOR_BYTES)-1, p_data32); // AUTO_FMT must be enabled
                    if ( (S_DONE != ret) && (S_ECC_CORRECTABLE_ERR != ret))
                        return ret;
                }
            }
        }
    }
    return S_DONE;
}
#endif

/* read page with: ecc on, autofdm on */
int atc_nand_read_ecc(struct mtd_info *mtd, u32 page, u32 *buf)
{

/* NOTE: should make sure buf is 4 byte alignment */
    struct nand_chip *chip = mtd->priv;
    u32 page_size = mtd->writesize;
    u32 spare_size = mtd->oobsize;
    u32 column_addr_bytes, row_addr_bytes, addr_cycle;
    u32 *p_data32 = (u32*)chip->buffers->databuf;
    u32 timeout,i;
    u32 c_timeout = MTD_NAND_DEFAULT_TIMEOUT;
    STATUS_E ret = S_UNKNOWN_ERR;
    volatile u32* p4TmpSpare = (u32*)(chip->oob_poi);
    u16    u2NfiCnfg;
    volatile u32* pFDMAddr = NFI_FDM0L;
    volatile u32 p4TmpSpare2[4];
    u32 dec_mask;

    if(buf)
        p_data32 = buf;
    else
        p_data32 = (u32*)chip->buffers->databuf;

    flush_invalid_cache((unsigned int)p_data32, page_size);

    memset(chip->oob_poi, 0xFF, mtd->oobsize);
    if (mtd->writesize > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    /* step1: reset the NFI core state machine */
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    while( *NFI_STA  & (STA_NFI_FSM_MASK | STA_NAND_FSM_MASK) );
    while (FIFO_RD_REMAIN(*NFI_FIFOSTA) || FIFO_WR_REMAIN(*NFI_FIFOSTA));

    /*  step2: Set NFI_CNFG, NFI_CON, FDMADDR */
    u2NfiCnfg = OP_READ | READ_MODE | HW_ECC_EN;
    if (page_size == 512)
        u2NfiCnfg |= SEL_SEC_512BYTE;
    //AUTO_FMT, DMA mode
    u2NfiCnfg |= AUTO_FMT_EN | AHB_MODE;

    *NFI_CNFG = u2NfiCnfg;
    *NFI_CON = SEC_NUM(page_size/SECTOR_BYTES);
    *NFIECC_FDMADDR = (u32)NFI_FDM0L;
    *NFI_STRADDR = ((u32)p_data32)|0xC0000000;

    *NFI_INTR_EN |= AHB_DONE_EN ;
    //clear interrupt status
    i = *NFI_INTR;
    *NFI_INTR = i;

    /* step3: config decode and start it */
    *NFIECC_DECCON = 0; // reset
    while((*NFIECC_DECIDLE & DEC_IDLE) == 0);
    if (_fgECCSWCorrect)
        *NFIECC_DECCNFG = DEC_EMPTY_EN |DEC_CON(ECC_DEC_LOCATE) | DEC_NFI_MODE;
    else
        *NFIECC_DECCNFG = DEC_EMPTY_EN |DEC_CON(ECC_DEC_CORRECT) | DEC_NFI_MODE;
    
    *NFIECC_DECCNFG |= DEC_TNUM(global_ecc_strength) | DEC_CS( ((SECTOR_BYTES + glb_fdm_ecc_size) << 3) + global_ecc_strength*14) | DEC_NFI_MODE;

    *NFIECC_DECCON = DEC_EN;

    /* step4: common read flow */
    *NFI_CMD = NAND_CMD_READ0;
    while(*NFI_STA  & STATUS_CMD);

    *NFI_COLADDR = 0;
    *NFI_ROWADDR = page;
    *NFI_ADDRNOB = COL_ADDR_NOB(column_addr_bytes) | ROW_ADDR_NOB(row_addr_bytes);
    while(*NFI_STA  & STATUS_ADDR);

    //read confirm
    if(mtd->writesize > 512) {
        *NFI_CMD = NAND_CMD_READSTART;
        while( *NFI_STA  & STATUS_BUSY);
    }

    /* step5: set burst read by DWORD and waiting read done */
    *NFI_CON |= BURST_RD;
    // read page data
    timeout = c_timeout;
    NFI_Wait((AHB_DONE != (*NFI_INTR & AHB_DONE)), timeout);
    *NFI_INTR_EN &= ~AHB_DONE_EN;
    i = *NFI_INTR;//read clear again
    *NFI_INTR = i;
    if( 0 == timeout)
    {
        printf("[MTD][%s: %u] NFI_Wait TimeOut\n", __FUNCTION__, __LINE__);
        ret = S_TIMEOUT;
        goto end;
    }

    timeout = c_timeout;
    NFI_Wait( ((*NFI_ADDRCNTR & 0xF000) >> 12) != (page_size/SECTOR_BYTES) , timeout);
    if( 0 == timeout ) {
        ret = S_TIMEOUT;
        goto end;
    }

    /* step6: wait for decoder done */
    timeout=MTD_NAND_DEFAULT_TIMEOUT;
    dec_mask=(1<<(page_size/SECTOR_BYTES))-1;
    NFI_Wait(((*NFIECC_DECDONE & dec_mask) != dec_mask), timeout); // wait for all block decode done
    if( 0 == timeout ) {
        printf("[MTD][%s: %u]NFI_Wait Timeout\n",__FUNCTION__, __LINE__);
        ret=S_TIMEOUT;
        goto end;
    }
    //printf("6DECDONE=%x mask=%x NFIECC_DECIDLE = %x NFIECC_FDMSTA=%x \n",*NFIECC_DECDONE,dec_mask, *NFIECC_DECIDLE,*NFIECC_FDMSTA);
    /* step7: read FDM data */
    for (i = 0; i < (page_size/SECTOR_BYTES) ; i++)
    {
        p4TmpSpare2[0] = *pFDMAddr++;
        p4TmpSpare2[1] = *pFDMAddr++;
        p4TmpSpare2[2] = *pFDMAddr++;
        p4TmpSpare2[3] = *pFDMAddr++;
        memcpy((void*)p4TmpSpare,(void*) p4TmpSpare2, glb_fdm_size);
        p4TmpSpare = (u32*)((u32)p4TmpSpare + glb_fdm_size);
    }

    ret = atc_common_ecc_err_detect(mtd, c_timeout);

    if (S_ECC_CORRECTABLE_ERR == ret)
        ret = S_DONE;

end:
    // disable burst read
    *NFI_CON = 0x0;
    *NFIECC_DECCON = 0x0;
    if( ret == S_DONE)
        return 0;
    else
        return -1;
}


/* read page with: ecc off, autofdm off */
int atc_nand_read_raw(struct mtd_info *mtd, u32 page)
{
    struct nand_chip *chip = mtd->priv;
    u32 page_size = mtd->writesize;
    u32 spare_size;
    u32 column_addr_bytes, row_addr_bytes, addr_cycle;
    u32 *p_data32 = (u32*)chip->buffers->databuf;
    u32 timeout,i;
    u32 c_timeout = MTD_NAND_DEFAULT_TIMEOUT;
    STATUS_E ret = S_UNKNOWN_ERR;
    u16    u2NfiCnfg;
    u16 spare_per_sector = mtd->oobsize / (page_size/SECTOR_BYTES);
    u16 pagefmt_cfg = *NFI_PAGEFMT;

    /* we have change spare_per_sector to 52 if spare_per_sector is 56 */
    if((spare_per_sector == 56) || (spare_per_sector == 28))
    *NFI_PAGEFMT = (pagefmt_cfg & 0xffcf) | SPARE_56_28;

    switch((*NFI_PAGEFMT & 0x30) >> 4) {
    case 0:
        spare_size = (SECTOR_BYTES == 1024) ? 32 * (page_size/SECTOR_BYTES)
                : 16 * (page_size/SECTOR_BYTES);
        break;
    case 1:
        spare_size = (SECTOR_BYTES == 1024) ? 52 * (page_size/SECTOR_BYTES)
                : 26 * (page_size/SECTOR_BYTES);
        break;
    case 2:
        spare_size = (SECTOR_BYTES == 1024) ? 54 * (page_size/SECTOR_BYTES)
                : 27 * (page_size/SECTOR_BYTES);
        break;
    case 3:
        spare_size = (SECTOR_BYTES == 1024) ? 56 * (page_size/SECTOR_BYTES)
                : 28 * (page_size/SECTOR_BYTES);
        break;

    }
    printf("oobsize =0x%x spare_size=0x%x \n",mtd->oobsize,spare_size);

    memset(chip->oob_poi, 0xFF, mtd->oobsize);
    if (mtd->writesize > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    /* step1: reset the NFI core state machine */
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    while( *NFI_STA  & (STA_NFI_FSM_MASK | STA_NAND_FSM_MASK) );
    while (FIFO_RD_REMAIN(*NFI_FIFOSTA) || FIFO_WR_REMAIN(*NFI_FIFOSTA));

    /*  step2: Set NFI_CNFG, NFI_CON, FDMADDR */
    u2NfiCnfg = OP_READ | READ_MODE;
    if (page_size == 512)
        u2NfiCnfg |= SEL_SEC_512BYTE;

    *NFI_CNFG = u2NfiCnfg;
    *NFI_CON = SEC_NUM(page_size/SECTOR_BYTES);

    /* step4: common read flow */
    *NFI_CMD = NAND_CMD_READ0;
    while(*NFI_STA  & STATUS_CMD);

    *NFI_COLADDR = 0;
    *NFI_ROWADDR = page;
    *NFI_ADDRNOB = COL_ADDR_NOB(column_addr_bytes) | ROW_ADDR_NOB(row_addr_bytes);
    while(*NFI_STA  & STATUS_ADDR);

    //read confirm
    if(mtd->writesize > 512) {
        *NFI_CMD = NAND_CMD_READSTART;
        while( *NFI_STA  & STATUS_BUSY);
    }

    /* step5: set burst read by DWORD and waiting read done */
    *NFI_CON |= BURST_RD | NOB_DWORD;

    for(i=0; i < (mtd->writesize + spare_size); i+=4)
    {
        timeout = c_timeout;
        NFI_Wait( (*NFI_FIFOSTA & RD_EMPTY_MASK), timeout);
        if(0 == timeout)
        {
            printf("[MTD][%s: %u] NFI_Wait TimeOut,i=%d\n", __FUNCTION__, __LINE__,i);
            ret = S_TIMEOUT;
            goto end;
        }
        *(u32*)((u32)p_data32 + i) = *NFI_DATAR;
    }

    ret = S_DONE;

end:
    *NFI_PAGEFMT = pagefmt_cfg;
    // disable burst read
    *NFI_CON = 0x0;
    *NFIECC_DECCON = 0x0;
    if( ret == S_DONE)
        return 0;
    else
        return -1;
}

/* read page with: ecc off, autofdm on */
int atc_nand_read_raw_with_autofdm(struct mtd_info *mtd, u32 page, u32 *buf)
{
    struct nand_chip *chip = mtd->priv;
    u32 page_size = mtd->writesize;
    u32 column_addr_bytes, row_addr_bytes, addr_cycle;
    u32 *p_data32 = (u32*)chip->buffers->databuf;
    u32 timeout,i;
    u32 c_timeout = MTD_NAND_DEFAULT_TIMEOUT;
    STATUS_E ret = S_UNKNOWN_ERR;
    volatile u32* p4TmpSpare = (u32*)(chip->oob_poi);
    u16    u2NfiCnfg;
    volatile u32* pFDMAddr = NFI_FDM0L;
    volatile u32 p4TmpSpare2[4];

    if(buf)
        p_data32 = buf;
    else
        p_data32 = (u32*)chip->buffers->databuf;
    flush_invalid_cache((unsigned int)p_data32, page_size);

    memset(chip->oob_poi, 0xff, mtd->oobsize);
    if (mtd->writesize > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    /* step1: reset the NFI core state machine */
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    while( *NFI_STA  & (STA_NFI_FSM_MASK | STA_NAND_FSM_MASK) );
    while (FIFO_RD_REMAIN(*NFI_FIFOSTA) || FIFO_WR_REMAIN(*NFI_FIFOSTA));

    /*  step2: Set NFI_CNFG, NFI_CON, FDMADDR */
    u2NfiCnfg = OP_READ | READ_MODE;
    if (page_size == 512)
        u2NfiCnfg |= SEL_SEC_512BYTE;

    u2NfiCnfg |= AUTO_FMT_EN  ;
    *NFIECC_FDMADDR = (u32)(NFI_FDM0L);

    *NFI_CNFG = u2NfiCnfg;
    *NFI_CON = SEC_NUM(page_size/SECTOR_BYTES);

    /* step4: common read flow */
    *NFI_CMD = NAND_CMD_READ0;
    while(*NFI_STA  & STATUS_CMD);

    *NFI_COLADDR = 0;
    *NFI_ROWADDR = page;
    *NFI_ADDRNOB = COL_ADDR_NOB(column_addr_bytes) | ROW_ADDR_NOB(row_addr_bytes);
    while(*NFI_STA & STATUS_ADDR);

    //read confirm
    if(mtd->writesize > 512) {
        *NFI_CMD = NAND_CMD_READSTART;
        while( *NFI_STA  & STATUS_BUSY);
    }

    /* step5: set burst read by DWORD and waiting read done */
    *NFI_CON |= BURST_RD | NOB_DWORD;

    for(i=0; i < mtd->writesize; i+=4)
    {
        timeout = c_timeout;
        NFI_Wait( (*NFI_FIFOSTA & RD_EMPTY_MASK), timeout);
        if(0 == timeout)
        {
            printf("[MTD][%s: %u] NFI_Wait TimeOut,i=%d\n", __FUNCTION__, __LINE__,i);
            ret = S_TIMEOUT;
            goto end;
        }
        *(u32*)((u32)p_data32 + i) = *NFI_DATAR;
    }

    timeout = c_timeout;
    NFI_Wait( ((*NFI_ADDRCNTR & 0xF000) >> 12) != (page_size/SECTOR_BYTES), timeout);
    if( 0 == timeout ) {
        ret = S_TIMEOUT;
        goto end;
    }

    for (i = 0; i < (page_size/SECTOR_BYTES); i++)
    {
        p4TmpSpare2[0] = *pFDMAddr++;
        p4TmpSpare2[1] = *pFDMAddr++;
        p4TmpSpare2[2] = *pFDMAddr++;
        p4TmpSpare2[3] = *pFDMAddr++;
        memcpy((void*)p4TmpSpare,(void*) p4TmpSpare2, glb_fdm_size);
        p4TmpSpare = (u32*)((u32)p4TmpSpare + glb_fdm_size);
    }

    ret = S_DONE;

end:
    // disable burst read
    *NFI_CON = 0x0;
    *NFIECC_DECCON = 0x0;
    if( ret == S_DONE)
    return 0;
    else
    return -1;
}

int atc_nand_read_with_swecc(struct mtd_info *mtd, u32 page, u32 *buf)
{

/* NOTE: should make sure buf is 4 byte alignment */
    struct nand_chip *chip = mtd->priv;
    u32 page_size = mtd->writesize;
    u32 spare_size = mtd->oobsize;
    u32 column_addr_bytes, row_addr_bytes, addr_cycle;
    u32 *p_data32 = (u32*)chip->buffers->databuf;
    u32 timeout,i;
    u32 c_timeout = MTD_NAND_DEFAULT_TIMEOUT;
    STATUS_E ret = S_UNKNOWN_ERR;
    volatile u32* p4TmpSpare = (u32*)(chip->oob_poi);
    u16    u2NfiCnfg;
    volatile u32* pFDMAddr = NFI_FDM0L;
    volatile u32 p4TmpSpare2[4];
    u32 dec_mask;
    _fgECCSWCorrect = 1;

    if(buf)
        p_data32 = buf;
    else
        p_data32 = (u32*)chip->buffers->databuf;

    flush_invalid_cache((unsigned int)p_data32, page_size);

    memset(chip->oob_poi, 0xFF, mtd->oobsize);
    if (mtd->writesize > 512)
    {
        addr_cycle = 5;
        column_addr_bytes = 2;
    }
    else
    {
        addr_cycle = 3;
        column_addr_bytes = 1;
    }
    row_addr_bytes = addr_cycle - column_addr_bytes;

    /* step1: reset the NFI core state machine */
    *NFI_CON = NFI_RST | FIFO_FLUSH;
    while( *NFI_STA  & (STA_NFI_FSM_MASK | STA_NAND_FSM_MASK) );
    while (FIFO_RD_REMAIN(*NFI_FIFOSTA) || FIFO_WR_REMAIN(*NFI_FIFOSTA));

    /*  step2: Set NFI_CNFG, NFI_CON, FDMADDR */
    u2NfiCnfg = OP_READ | READ_MODE | HW_ECC_EN;
    if (page_size == 512)
        u2NfiCnfg |= SEL_SEC_512BYTE;
    //AUTO_FMT, DMA mode
    
    u2NfiCnfg |= AUTO_FMT_EN;

    *NFI_CNFG = u2NfiCnfg;
    *NFI_CON = SEC_NUM(page_size/SECTOR_BYTES);
    *NFIECC_FDMADDR = (u32)NFI_FDM0L;
    //*NFI_STRADDR = ((u32)p_data32)|0xC0000000;

    //*NFI_INTR_EN |= AHB_DONE_EN ;
    //clear interrupt status
    i = *NFI_INTR;
    *NFI_INTR = i;

    if (_fgECCSWCorrect)
        *NFIECC_DECCNFG = DEC_EMPTY_EN |DEC_CON(ECC_DEC_LOCATE) | DEC_NFI_MODE;
    else
        *NFIECC_DECCNFG = DEC_EMPTY_EN |DEC_CON(ECC_DEC_CORRECT) | DEC_NFI_MODE;
    *NFIECC_DECCNFG |= DEC_TNUM(global_ecc_strength) | DEC_CS( ((SECTOR_BYTES + glb_fdm_ecc_size) << 3) + global_ecc_strength*14) | DEC_NFI_MODE;

    /* step3: config decode and start it */
    
    *NFIECC_DECCON = 0; // reset
    while((*NFIECC_DECIDLE & DEC_IDLE) == 0);
    *NFIECC_FDMADDR = (u32)NFI_FDM0L;
    *NFIECC_DECCON = DEC_EN;

    /* step4: common read flow */
    *NFI_CMD = NAND_CMD_READ0;
    while(*NFI_STA  & STATUS_CMD);

    *NFI_COLADDR = 0;
    *NFI_ROWADDR = page;
    *NFI_ADDRNOB = COL_ADDR_NOB(column_addr_bytes) | ROW_ADDR_NOB(row_addr_bytes);
    while(*NFI_STA  & STATUS_ADDR);

    //read confirm
    if(mtd->writesize > 512) {
        *NFI_CMD = NAND_CMD_READSTART;
        while( *NFI_STA  & STATUS_BUSY);
    }

    i = *NFI_INTR;
    *NFI_INTR = i;

    /* step5: set burst read by DWORD and waiting read done */
    *NFI_CON |= BURST_RD | NOB_DWORD;

    atc_common_fifo_read(c_timeout, 0, p_data32, mtd->writesize);
    timeout = c_timeout;
    NFI_Wait( ((*NFI_ADDRCNTR & 0xF000) >> 12) != (page_size/SECTOR_BYTES) , timeout);
    if( 0 == timeout ) {
        ret = S_TIMEOUT;
        goto end;
    }
    /* step7: read FDM data */
    for (i = 0; i < (page_size/SECTOR_BYTES) ; i++)
    {
        p4TmpSpare2[0] = *pFDMAddr++;
        p4TmpSpare2[1] = *pFDMAddr++;
        p4TmpSpare2[2] = *pFDMAddr++;
        p4TmpSpare2[3] = *pFDMAddr++;
        memcpy((void*)p4TmpSpare,(void*) p4TmpSpare2, glb_fdm_size);
        p4TmpSpare = (u32*)((u32)p4TmpSpare + glb_fdm_size);
    }

    ret = atc_common_ecc_err_detect(mtd, c_timeout);

    if (S_ECC_CORRECTABLE_ERR == ret)
        ret = S_DONE;
end:
    // disable burst read
    *NFI_CON = 0x0;
    *NFIECC_DECCON = 0x0;
    if( ret == S_DONE)
        return 0;
    else
        return -1;
}


int atc_nand_write_raw(struct mtd_info *mtd, u32 page, u32 *buf)
{
    int ret;
    struct nand_chip *chip = mtd->priv;
    _fgUsingDMA = TRUE;
    _fgAUTO_FMT = FALSE;
    ret = atc_nand_write_page(mtd,chip,buf, page, 0, 0);
    _fgUsingDMA = TRUE;
    _fgAUTO_FMT = TRUE;
    return ret;
}

