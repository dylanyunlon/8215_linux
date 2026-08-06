
#ifndef _NFI2_H
#define _NFI2_H

#define INT_WR_CLR

#define FDM2_BYTES  9
#define FDM2_ECC_BYTES 9
#define SECTOR_BYTES 512

#define  IO_8BITS   0x0000
#define  IO_16BITS  0x0001



#define NFI2_base        (0xf001E400)
#define NFI2_CNFG                          ((volatile P_U16)(NFI2_base+0x00))
    #define AHB_MODE        0x0001
    #define READ_MODE      0x0002
    #define SEL_SEC_512BYTE 0x0020
    #define OP_IDLE            0x0000
    #define OP_READ           0x1000
    #define OP_READ_ID_ST   0x2000
    #define OP_PROGRAM    0x3000
    #define OP_ERASE         0x4000
    #define OP_RESET         0x5000
    #define OP_CUSTOME    0x6000
    #define HW_ECC_EN      0x0100
    #define AUTO_FMT_EN  0x0200
  
#define NFI2_PAGEFMT             ((volatile P_U16)(NFI2_base+0x04))
    #define NAND_IO_8BITS      0x0000
    #define NAND_IO_16BITS     0x0008
    #define NAND_PSIZE(x)               (((UINT32) x & 0x03) << 0)
    #define NAND_PSIZE_2K_512             0
    #define NAND_PSIZE_4K_2K            1
    #define NAND_PSIZE_8K_4K       2
    #define FDM2_NUM(x)                (((UINT32) x &0x1F) << 6)
    #define FDM2_ECC_NUM(x)                (((UINT32) x &0x1F) << 11)
    #define SPARE_16  0x0000
    #define SPARE_26  0x0010
    #define SPARE_27  0x0020
    #define SPARE_64  0x0030
    
#define NFI2_CON                    ((volatile P_U16)(NFI2_base+0x08))
    #define FIFO_FLUSH              ((UINT32) 1 << 0)
    #define NFI_RST             ((UINT32) 1 << 1)
    #define BURST_RD            ((UINT32) 1 << 8)
    #define BURST_WR            ((UINT32) 1 << 9)
    #define SINGLE_RD           ((UINT32) 1 << 4)
    #define NOB_DWORD                   0x0080
    #define NOB_WORD                    0x0040
    #define NOB_DWORD                 0x0080    
    #define SEC_NUM(x)                (((UINT32) x &0x0F) << 12)
    
#define NFI2_ACCCON                     ((volatile P_U32)(NFI2_base+0x0C))
    #define LCD2NAND(x)   (((UINT32)x& 0xF) << 28)
    #define PRECS(x)     (((UINT32) x & 0x3f) << 22)
    #define C2R(x)          (((UINT32) x & 0x3f) << 16)
    #define W2R(x)          (((UINT32) x & 0x0f) << 12)
    #define WH(x)           (((UINT32) x & 0x0f) <<  8)
    #define WST(x)          (((UINT32) x & 0x0f) <<  4)
    #define RLT(x)          (((UINT32) x & 0x0f) <<  0)

#define NFI2_INTR_EN                         ((volatile P_U16)(NFI2_base+0x10))
   #define AHB_DONE_EN              0x40
   #define WR_DONE_EN               0x02
   #define RD_DONE_EN                0x01
   #define RESET_DONE_EN        0x04
   
#define NFI2_INTR               ((volatile P_U16)(NFI2_base+0x14))
  #define AHB_DONE                     0x40
  #define WR_DONE                      0x02
  #define RD_DONE                      0x01
  #define RESET_DONE                   0x04
  #define ERASE_DONE_EN                0x08
  
#define NFI2_CMD                            ((volatile P_U16)(NFI2_base+0x20))
#define NAND_CMD_READ1_00           0x00
#define NAND_CMD_READ1_01           0x01
#define NAND_CMD_PROG_PAGE          0x10    /* WRITE 2 */
#define NAND_CMD_READ_2K        0x30    // only for 2KB  page-size
#define NAND_CMD_READ2              0x50
#define NAND_CMD_ERASE1_BLK         0x60
#define NAND_CMD_STATUS             0x70
#define NAND_CMD_INPUT_PAGE         0x80    /* WRITE 1 */
#define NAND_CMD_READ_ID            0x90
#define NAND_CMD_ERASE2_BLK         0xD0
#define NAND_CMD_RESET              0xFF


#define NFI2_ADDRNOB                 ((volatile P_U16)(NFI2_base+0x30))
  #define COL_ADDR_NOB(x)     (((UINT32) x & 0x07) << 0)
  #define ROW_ADDR_NOB(x)     (((UINT32) x & 0x07) << 4)

#define NFI2_COLADDR                 ((volatile P_U32)(NFI2_base+0x34))

#define NFI2_ROWADDR                     ((volatile P_U32)(NFI2_base+0x38))

#define NFI2_DATAW                 ((volatile P_U32)(NFI2_base+0x50))

#define NFI2_DATAR                 ((volatile P_U32)(NFI2_base+0x54))

#define NFI2_STA                     ((volatile P_U32)(NFI2_base+0x60))
#define NAND_STATUS_BUSY            ((UINT32) 1 << 8)
#define NAND_STATUS_DTWR            ((UINT32) 1 << 3)
#define NAND_STATUS_DTRD            ((UINT32) 1 << 2)
#define NAND_STATUS_ADDR            ((UINT32) 1 << 1)
#define NAND_STATUS_CMD             ((UINT32) 1 << 0)

#define STATUS_CMD         0x1
#define DATAW              0x08
#define BUSY               0x0100
#define ERASE_DONE_EN      0x08
#define ERASE_DONE         0x08
    
#define NFI2_FIFOSTA                ((volatile P_U16)(NFI2_base+0x64))
//  #define WB_FULL         ((UINT32) 1 << 7)
//  #define RB_EMPTY            ((UINT32) 1 << 6)
    #define WR_FULL         ((UINT32) 1 << 15)
    #define WR_EMPTY            ((UINT32) 1 << 14)
    #define RD_FULL         ((UINT32) 1 << 7)
    #define RD_EMPTY            ((UINT32) 1 << 6)

#define NFI2_ADDRCNTR   ((volatile UINT16 *)(NFI2_base+0x0070))

#define NFI2_STRADDR    ((volatile P_U32)(NFI2_base+0x0080))

#define NFI2_BYTELEN     ((volatile P_U16)(NFI2_base+0x0084))
#define NFI2_CSEL	     ((volatile P_U16)(NFI2_base+0x0090))
#define NFI2_FDM0L       ((volatile P_U32)(NFI2_base+0x200))
#define NFI2_FDM0M      ((volatile P_U32)(NFI2_base+0x204))

#define NFI2_ECC_RDY   ((volatile P_U32)(NFI2_base+0xFFC))
#define WAIT_RDY_MASK         0x1000

#define NFI2ECC_BASE        0xF001EC00

#define NFI2ECC_ENCCON      ((volatile UINT16 *)(NFI2ECC_BASE+0x0000))
#define ENC_EN      0x01
    
#define NFI2ECC_ENCCNFG  ((volatile UINT32 *)(NFI2ECC_BASE+0x0004))
#define ENC_TNUM(x)                ((((UINT32) x / 2) - 2))
#define ENC_NFI_MODE             0x01 << 4
#define ENC2_MS(x)                    (((UINT32) x &0x3FFF) << 16)

#define NFI2ECC_ENCDIADDR      ((volatile UINT32 *)(NFI2ECC_BASE+0x0008))

#define NFI2ECC_ENCIDLE          ((volatile UINT16 *)(NFI2ECC_BASE+0x000C))
#define ENC_IDLE                    0x01

#define NFI2ECC_ENCPAR0        ((volatile UINT32 *)(NFI2ECC_BASE+0x0010))

#define NFI2ECC_DECCON      ((volatile UINT16 *)(NFI2ECC_BASE+0x0100))
#define DEC_EN      0x01

#define NFI2ECC_DECCNFG  ((volatile UINT32 *)(NFI2ECC_BASE+0x0104)) 
#define DEC_TNUM(x)                ((((UINT32) x >> 1) - 2))
#define DEC_NFI_MODE             0x01 << 4
#define DEC_CON(x)                    (((UINT32) x &0x03) << 12)     
#define DEC2_CS(x)                    (((UINT32) x &0x3FFF) << 16)
#define DEC_EMPTY_EN             0x80000000

#define NFI2ECC_DECDIADDR      ((volatile UINT32 *)(NFI2ECC_BASE+0x0108))

#define NFI2ECC_DECIDLE          ((volatile UINT16 *)(NFI2ECC_BASE+0x010C))
#define DEC_IDLE                    0x01

#define NFI2ECC_DECFER            ((volatile UINT16 *)(NFI2ECC_BASE+0x0110))
#define NFI2ECC_DECENUM            ((UINT32 *)(NFI2ECC_BASE+0x0150))

#define NFI2ECC_DECENUM2            ((volatile UINT32 *)(NFI2ECC_BASE+0x0154))

#define NFI2ECC_DECDONE        ((volatile UINT16 *)(NFI2ECC_BASE+0x0118))

#define NFI2ECC_DECEL0           ((volatile UINT32 *)(NFI2ECC_BASE+0x160))

#define NFI2ECC_DECIRQEN       ((volatile UINT16 *)(NFI2ECC_BASE+0x0134))
#define DEC_IRQEN                0x01

#define NFI2ECC_FDMADDR        ((volatile UINT32 *)(NFI2ECC_BASE+0x013C))

#define STATUS_WRITE_PROTECT            0x0
#define STATUS_READY_BUSY               0x40
#define STATUS_ERASE_SUSPEND            0x20
#define STATUS_PASS_FAIL                0x01

#define REPLICATION_NUMBER  8

typedef enum {
    ECC2_4_BITS = 4,
    ECC2_6_BITS = 6,
    ECC2_8_BITS = 8,
    ECC2_10_BITS = 10,
    ECC2_12_BITS = 12,
    ECC2_22_BITS = 22,
    ECC2_24_BITS = 24
} ECC2_Level_t;

typedef enum {
    ECC2_DEC_NONE,
    ECC2_DEC_DETECT,
    ECC2_DEC_LOCATE,
    ECC2_DEC_CORRECT
} ECC2_Decode_Type_t;

#define NFI_Wait(condition_expression, timeout)     while( (condition_expression) && (--timeout) )

#define NONCACHE(expression) (expression|0xC0000000)
#endif // _NFI_H
