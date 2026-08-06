
#ifndef _REG_NFI_H_
#define _REG_NFI_H_

#ifdef CHIP_VER_MT8530
typedef volatile struct {
    UINT32 NFI_CNFG;        // 0000
    UINT32 NFI_PAGEFMT;     // 0004
    UINT32 NFI_CON;         // 0008
    UINT32 NFI_ACCCON;      // 000C
    UINT32 NFI_INTR_EN;     // 0010
    UINT32 NFI_INTR;        // 0014
    UINT32 resv0018[2];     // 0018
    UINT32 NFI_CMD;         // 0020
    UINT32 resv0024[3];     // 0024
    UINT32 NFI_ADDRNOB;     // 0030
    UINT32 NFI_COLADDR;     // 0034
    UINT32 NFI_ROWADDR;     // 0038
    UINT32 resv003C;        // 003C
    UINT32 NFI_STRDATA;     // 0040
    UINT32 resv0044[3];     // 0044
    UINT32 NFI_DATAW;       // 0050
    UINT32 NFI_DATAR;       // 0054
    UINT32 resv0058[2];     // 0058
    UINT32 NFI_STA;         // 0060
    UINT32 NFI_FIFOSTA;     // 0064
    UINT32 NFI_LOCKSTA;     // 0068
    UINT32 resv006C;		// 006C
    UINT32 NFI_ADDRCNTR;    // 0070
    UINT32 resv0074[3];     // 0074
    UINT32 NFI_STRADDR;     // 0080
    UINT32 NFI_BYTELEN;     // 0084
    UINT32 resv0088[2];     // 0088
    UINT32 NFI_CSEL;        // 0090
    UINT32 NFI_IOCON;		// 0094
    UINT32 resv0098[2];		// 0098
    UINT32 NFI_FDM0L;       // 00A0
    UINT32 NFI_FDM0M;       // 00A4
    UINT32 NFI_FDM1L;       // 00A8
    UINT32 NFI_FDM1M;       // 00AC
    UINT32 NFI_FDM2L;       // 00B0
    UINT32 NFI_FDM2M;       // 00B4
    UINT32 NFI_FDM3L;       // 00B8
    UINT32 NFI_FDM3M;       // 00BC
    UINT32 NFI_FDM4L;       // 00C0
    UINT32 NFI_FDM4M;       // 00C4
    UINT32 NFI_FDM5L;       // 00C8
    UINT32 NFI_FDM5M;       // 00CC
    UINT32 NFI_FDM6L;       // 00D0
    UINT32 NFI_FDM6M;       // 00D4
    UINT32 NFI_FDM7L;       // 00D8
    UINT32 NFI_FDM7M;       // 00DC   	
    UINT32 resv00E0[8];     // 00E0
    UINT32 NFI_LOCK;        // 0100
    UINT32 NFI_LOCKCON;     // 0104
    UINT32 NFI_LOCKANOB;    // 0108
    UINT32 resv010C;        // 010C
    UINT32 NFI_LOCK00ADD;   // 0110
    UINT32 NFI_LOCK00FMT;   // 0114
    UINT32 resv0118[30];    // 0118
    UINT32 NFI_FIFODATA0;   // 0190
    UINT32 NFI_FIFODATA1;   // 0194
    UINT32 NFI_FIFODATA2;   // 0198
    UINT32 NFI_FIFODATA3;   // 019C
} MT8530_NFI_REG;
#endif

#if defined (CHIP_VER_AC83XX)
typedef volatile struct {
    UINT32 NFI_CNFG;        // 0000
    UINT32 NFI_PAGEFMT;     // 0004
    UINT32 NFI_CON;         // 0008
    UINT32 NFI_ACCCON;      // 000C
    UINT32 NFI_INTR_EN;     // 0010
    UINT32 NFI_INTR;        // 0014
    UINT32 resv0018[2];     // 0018
    UINT32 NFI_CMD;         // 0020
    UINT32 resv0024[3];     // 0024
    UINT32 NFI_ADDRNOB;     // 0030
    UINT32 NFI_COLADDR;     // 0034
    UINT32 NFI_ROWADDR;     // 0038
    UINT32 resv003C;        // 003C
    UINT32 NFI_STRDATA;     // 0040
    UINT32 resv0044[3];     // 0044
    UINT32 NFI_DATAW;       // 0050
    UINT32 NFI_DATAR;       // 0054
    UINT32 resv0058[2];     // 0058
    UINT32 NFI_STA;         // 0060
    UINT32 NFI_FIFOSTA;     // 0064
    UINT32 NFI_LOCKSTA;     // 0068
    UINT32 resv006C;		// 006C
    UINT32 NFI_ADDRCNTR;    // 0070
    UINT32 resv0074[3];     // 0074
    UINT32 NFI_STRADDR;     // 0080
    UINT32 NFI_BYTELEN;     // 0084
    UINT32 resv0088[2];     // 0088
    UINT32 NFI_CSEL;        // 0090
    UINT32 NFI_IOCON;		// 0094
    UINT32 resv0098[2];		// 0098
	UINT32 resv00A0[24];    // 00A0
    UINT32 NFI_LOCK;        // 0100
    UINT32 NFI_LOCKCON;     // 0104
    UINT32 NFI_LOCKANOB;    // 0108
    UINT32 resv010C;        // 010C
    UINT32 NFI_LOCK00ADD;   // 0110
    UINT32 NFI_LOCK00FMT;   // 0114
    UINT32 NFI_LOCK01ADD;   // 0118
    UINT32 NFI_LOCK01FMT;   // 011C
	UINT32 NFI_LOCK02ADD;   // 0120
	UINT32 NFI_LOCK02FMT;   // 0124
	UINT32 NFI_LOCK03ADD;   // 0128
	UINT32 NFI_LOCK03FMT;   // 012C
	UINT32 NFI_LOCK04ADD;   // 0130
	UINT32 NFI_LOCK04FMT;   // 0134
	UINT32 NFI_LOCK05ADD;   // 0138
	UINT32 NFI_LOCK05FMT;   // 013C
	UINT32 NFI_LOCK06ADD;   // 0140
	UINT32 NFI_LOCK06FMT;   // 0144
	UINT32 NFI_LOCK07ADD;   // 0148
	UINT32 NFI_LOCK07FMT;   // 014C
	UINT32 NFI_LOCK08ADD;   // 0150
	UINT32 NFI_LOCK08FMT;   // 0154
	UINT32 NFI_LOCK09ADD;   // 0158
	UINT32 NFI_LOCK09FMT;   // 015C
	UINT32 NFI_LOCK10ADD;   // 0160
	UINT32 NFI_LOCK10FMT;   // 0164
	UINT32 NFI_LOCK11ADD;   // 0168
	UINT32 NFI_LOCK11FMT;   // 016C
	UINT32 NFI_LOCK12ADD;   // 0170
	UINT32 NFI_LOCK12FMT;   // 0174
	UINT32 NFI_LOCK13ADD;   // 0178
	UINT32 NFI_LOCK13FMT;   // 017C
	UINT32 NFI_LOCK14ADD;   // 0180
	UINT32 NFI_LOCK14FMT;   // 0184
	UINT32 NFI_LOCK15ADD;   // 0188
	UINT32 NFI_LOCK15FMT;   // 018C
    UINT32 NFI_FIFODATA0;   // 0190
    UINT32 NFI_FIFODATA1;   // 0194
    UINT32 NFI_FIFODATA2;   // 0198
    UINT32 NFI_FIFODATA3;   // 019C
	UINT32 resv01A0[24];    // 01A0
	UINT32 NFI_FDM0L;       // 0200
    UINT32 NFI_FDM0M;       // 0204
    UINT32 NFI_FDM0L2;      // 0208
    UINT32 NFI_FDM0M2;      // 020C
    UINT32 NFI_FDM1L;       // 0210
    UINT32 NFI_FDM1M;       // 0214
    UINT32 NFI_FDM1L2;      // 0218
    UINT32 NFI_FDM1M2;      // 021C
    UINT32 NFI_FDM2L;       // 0220
    UINT32 NFI_FDM2M;       // 0224
    UINT32 NFI_FDM2L2;      // 0228
    UINT32 NFI_FDM2M2;      // 022C
    UINT32 NFI_FDM3L;       // 0230
    UINT32 NFI_FDM3M;       // 0234
    UINT32 NFI_FDM3L2;      // 0238
    UINT32 NFI_FDM3M2;      // 023C
    UINT32 NFI_FDM4L;       // 0240
    UINT32 NFI_FDM4M;       // 0244
    UINT32 NFI_FDM4L2;      // 0248
    UINT32 NFI_FDM4M2;      // 024C
    UINT32 NFI_FDM5L;       // 0250
    UINT32 NFI_FDM5M;       // 0254
    UINT32 NFI_FDM5L2;      // 0258
    UINT32 NFI_FDM5M2;      // 025C
    UINT32 NFI_FDM6L;       // 0260
    UINT32 NFI_FDM6M;       // 0264
    UINT32 NFI_FDM6L2;      // 0268
    UINT32 NFI_FDM6M2;      // 026C
    UINT32 NFI_FDM7L;       // 0270
    UINT32 NFI_FDM7M;       // 0274
    UINT32 NFI_FDM7L2;      // 0278
    UINT32 NFI_FDM7M2;      // 027C
} MT8530_NFI_REG;
#endif

/*******************************************************************************
 * NFI register bit definition
 *******************************************************************************/

/* NFI_CNFG */
#define CNFG_AHB              (0x00000001)
#define CNFG_READ_EN          (0x00000002)
#if defined(CHIP_VER_AC83XX)
#define CNFG_NRB_DEG_MODE	  (0x00000004)
#define CNFG_NRB2_DEG_MODE    (0x00000008)
#define CNFG_SEL_SEC_512BYTE  (0x00000020)
#endif
#define CNFG_BYTE_RW          (0x00000040)

#define CNFG_SEL_NRB2         (0x00000080) //to select second Ready/Busy instead of first Ready/Busy for circuit judgment

#define CNFG_HW_ECC_EN        (0x00000100)
#define CNFG_AUTO_FMT_EN      (0x00000200)
#if defined(CHIP_VER_AC83XX)
#define CNFG_OP_IDLE          (0x0000 | ((_u4SelSectorSize ==512) ? (1 <<5) : (0 <<5)))
#define CNFG_OP_READ          (0x1000 | ((_u4SelSectorSize ==512) ? (1 <<5) : (0 <<5)))
#define CNFG_OP_SRD    		  (0x2000 | ((_u4SelSectorSize ==512) ? (1 <<5) : (0 <<5)))
#define CNFG_OP_PRGM          (0x3000 | ((_u4SelSectorSize ==512) ? (1 <<5) : (0 <<5)))
#define CNFG_OP_ERASE         (0x4000 | ((_u4SelSectorSize ==512) ? (1 <<5) : (0 <<5)))
#define CNFG_OP_RESET         (0x5000 | ((_u4SelSectorSize ==512) ? (1 <<5) : (0 <<5)))
#define CNFG_OP_CUST          (0x6000 | ((_u4SelSectorSize ==512) ? (1 <<5) : (0 <<5)))
#define CNFG_OP_MODE_MASK     (0x00007000)
#endif

/* NFI_PAGEFMT */
#define PAGEFMT_512           (0x00000000)
#define PAGEFMT_2K            (0x00000001)
#define PAGEFMT_4K            (0x00000002)

#define PAGEFMT_PAGE_MASK     (0x00000003)
#if defined(CHIP_VER_AC83XX)
#define PAGEFMT_SEL_512		  (0x00000000)
#define PAGEFMT_SEL_2K        (0x00000001)
#define PAGEFMT_SEL_4K        (0x00000002)
#define PAGEFMT_NSEL_2K		  (0x00000000)
#define PAGEFMT_NSEL_4K       (0x00000001)
#define PAGEFMT_NSEL_8K       (0x00000002)



#endif
#if defined(CHIP_VER_AC83XX)
#define NAND_PSIZE_2K_512	   0
#define NAND_PSIZE_4K_2K	   1
#define NAND_PSIZE_8K_4K	   2
#define SEL_SEC_512BYTE 0x0020
#endif
#define PAGEFMT_DBYTE_EN      (0x00000008) //16 bits I/O bus interface enable
#define PAGEFMT_SPARE_16      (0x00000000)
#define PAGEFMT_SPARE_26      (0x00000010)
#define PAGEFMT_SPARE_27      (0x00000020)
#define PAGEFMT_SPARE_28      (0x00000030)
#define PAGEFMT_SPARE_MASK    (0x00000030)
#if defined(CHIP_VER_AC83XX)
#define PAGEFMT_SPARE_32      (0x00000000)
#define PAGEFMT_SPARE_52      (0x00000010)
#define PAGEFMT_SPARE_54      (0x00000020)
#define PAGEFMT_SPARE_56      (0x00000030)
#endif

#if defined(CHIP_VER_AC83XX)
#define PAGEFMT_FDM_MASK      (0x000007C0)
#define PAGEFMT_FDM_SHIFT     (6)
#define PAGEFMT_FDM_ECC_MASK  (0x0000F800)
#define PAGEFMT_FDM_ECC_SHIFT (11)
#endif

/* NFI_CON */
#define CON_FIFO_FLUSH        (0x00000001)
#define CON_NFI_RST           (0x00000002)
#define CON_NFI_SRD           (0x00000010)
#define CON_NFI_NOB_MASK      (0x000000E0)
#define CON_NFI_NOB_SHIFT     (5)
#define CON_NFI_BRD           (0x00000100)
#define CON_NFI_BWR           (0x00000200)
#define CON_NFI_SEC_MASK      (0x0000F000)
#define CON_NFI_SEC_SHIFT     (12)
#define CON_NFI_NOB_DWROD 0x0080

/* NFI_ACCCON */
#define RLT_SHIFI             (0)
#define WST_SHIFI             (4)
#define WH_SHIFI              (8)
#define W2R_SHIFI             (12)
#define C2R_SHIFI             (16)
#define PRECS_SHIFI           (22)
#define LCD2NAND_SHIFI        (28)

#define RLT(x)                (x)
#define WST(x)                (x << WST_SHIFI)
#define WH(x)                 (x << WH_SHIFI)
#define W2R(x)                (x << W2R_SHIFI)
#define C2R(x)                (x << C2R_SHIFI)
#define PRECS(x)              (x << PRECS_SHIFI)
#define LCD2NAND(x)           (x << LCD2NAND_SHIFI)

/* NFI_CLK_SEL - CKGEN_BASE+0x70 (18 - 16 bit):
    0 - default 27 MHz 
    1 - (1/3) * ARMPLL
    2 - (1/2) * SYSPLL1
    3 - (1/3) * SYSPLL1
    4 - (1/4) * SYSPLL1
    5 - (1/3) * SYSPLL2
    6 - (1/2) * DMPLL
    7 - USBPLL (240 MHz) */

#define NFI_CLK_SEL_OFFSET    (0x70)
#define NFI_CLK_SEL_MASK      (0x00070000)
#define NFI_CLK_SEL_TURN_OFF  (0x00008000)  // 15 bit turn off clk_nandflash

#define NFI_CLK_27            (0x00000000)
#define ACC_SETTING_27        (LCD2NAND(0xF)|PRECS(0x0F)|C2R(0x3F)|W2R(0xF)|WH(0xF)|WST(0xF)|RLT(0xF))

#define NFI_CLK_144           (0x00030000)
#define ACC_SETTING_144       (LCD2NAND(0x0)|PRECS(0x00)|C2R(0x00)|W2R(0x0)|WH(0x1)|WST(0x3)|RLT(0x5))

#define NFI_CLK_198           (0x00050000)
#define ACC_SETTING_198       (LCD2NAND(0x0)|PRECS(0x00)|C2R(0x00)|W2R(0x0)|WH(0x1)|WST(0x3)|RLT(0x5))

/* NFI_INTR_EN */
#define INTR_RD_DONE_EN       (0x00000001)
#define INTR_WR_DONE_EN       (0x00000002)
#define INTR_RST_DONE_EN      (0x00000004)
#define INTR_ERASE_DONE_EN    (0x00000008)
#define INTR_BSY_RTN_EN       (0x00000010)
#define INTR_ACC_LOCK_EN      (0x00000020)
#define INTR_AHB_DONE_EN      (0x00000040)
#define INTR_BSY_RTN_EN2      (0x00000080)

/* NFI_INTR */
#define INTR_RD_DONE          (0x00000001)
#define INTR_WR_DONE          (0x00000002)
#define INTR_RST_DONE         (0x00000004)
#define INTR_ERASE_DONE       (0x00000008)
#define INTR_BSY_RTN          (0x00000010)
#define INTR_ACC_LOCK         (0x00000020)
#define INTR_AHB_DONE         (0x00000040)
#define INTR_BSY_RTN2         (0x00000080)

/* NFI_ADDRNOB */
#define ADDR_COL_NOB_MASK     (0x00000007)
#define ADDR_ROW_NOB_MASK     (0x00000070)
#define ADDR_ROW_NOB_SHIFT    (4)
#define ADDR_COL_NOB(x)       ((x)&ADDR_COL_NOB_MASK)
#define ADDR_ROW_NOB(x)       (((x)&ADDR_ROW_NOB_MASK)<<ADDR_ROW_NOB_SHIFT)

/* NFI_STA */
#define STA_CMD_STATE         (0x00000001)
#define STA_ADDR_STATE        (0x00000002)
#define STA_DATAR_STATE       (0x00000004)
#define STA_DATAW_STATE       (0x00000008)
#define STA_ACC_LOCK          (0x00000010)
#define STA_NAND_BUSY         (0x00000100)
#define STA_READ_EMPTY        (0x00001000)

#define NFI_FSM_IDLE          (0x00000000)
#define NFI_FSM_RST           (0x00010000)
#define NFI_FSM_RD_BUSY       (0x00020000)
#define NFI_FSM_RD_DATA       (0x00030000)
#define NFI_FSM_PROG_BUSY     (0x00040000)
#define NFI_FSM_PROG_DATA     (0x00050000)
#define NFI_FSM_ERASE_BUSY    (0x00080000)
#define NFI_FSM_ERASE_DATA    (0x00090000)
#define NFI_FSM_CST_DATA      (0x000E0000)
#define NFI_FSM_CST           (0x000F0000)
#define STA_NFI_FSM_MASK      (0x000F0000)
#define STA_NFI_FSM_SHIFT     (16)

#define NAND_FSM_IDLE         (0x00000000)
#define NAND_FSM_CMD_WRRDY    (0x04000000)
#define NAND_FSM_CMD_WRST     (0x05000000)
#define NAND_FSM_CMD_WR       (0x06000000)
#define NAND_FSM_CMD_WRHD     (0x07000000)
#define NAND_FSM_ADDR_WRRDY   (0x08000000)
#define NAND_FSM_ADDR_WRST    (0x09000000)
#define NAND_FSM_ADDR_WR      (0x0A000000)
#define NAND_FSM_ADDR_WRHD    (0x0B000000)
#define NAND_FSM_CA2DEXT      (0x0C000000)
#define NAND_FSM_DATA_RDST    (0x11000000) 
#define NAND_FSM_DATA_RD      (0x12000000)
#define NAND_FSM_DATA_RDHD    (0x13000000)
#define NAND_FSM_DATA_WRRDY   (0x18000000)
#define NAND_FSM_DATA_WRST    (0x19000000)
#define NAND_FSM_DATA_WR      (0x1A000000)
#define NAND_FSM_DATA_WRHD    (0x1B000000)
#define NAND_FSM_PRECE        (0x1C000000)
#define STA_NAND_FSM_MASK     (0x1F000000)
#define STA_NAND_FSM_SHIFT    (24)

/* NFI_FIFOSTA */
#define FIFO_RD_EMPTY         (0x00000040)
#define FIFO_RD_FULL          (0x00000080)
#define FIFO_WR_EMPTY         (0x00004000)
#define FIFO_WR_FULL          (0x00008000)
#define FIFO_RD_REMAIN(x)     (0x1F&(x))
#define FIFO_WR_REMAIN(x)     ((0x1F00&(x))>>8)

/* NFI_ADDRCNTR */
#if defined(CHIP_VER_AC83XX)
#define ADDRCNTR_OFFSET(x)    (0x000007FF&(x))
#endif
#define ADDRCNTR_CNTR(x)      ((0x0000F000&(x))>>12)
#if defined(CHIP_VER_AC83XX)
#define CSEL_SEL_NRB2         (0x00000010)
#define CSEL_NRB2_MODE        (0x00000100)
#endif

/* NFI_IOCON */
#define IOCON_NLD_PD_EN       (0x00000001) 

/* NFI_LOCK */
#define NFI_LOCK_EN           (0x00000001)

/* NFI_LOCKANOB */
#define ERASE_CADD_NOB_MASK   (0x00000007)
#define ERASE_RADD_NOB_MASK   (0x00000070)
#define ERASE_RADD_NOB_SHIFT  (4)
#define PROG_CADD_NOB_MASK    (0x00000700)
#define PROG_CADD_NOB_SHIFT   (8)
#define PROG_RADD_NOB_MASK    (0x00007000)
#define PROG_RADD_NOB_SHIFT   (12)


/*******************************************************************************
 * NFI macro definition
 *******************************************************************************/

#define NFI_FDM_STARTADDR(x)    (x + 0xA0) //NFI_FDM register start address
#define NFI_Wait_Ready(psta, timeout)   while ((psta & STA_NAND_BUSY) && (timeout)) {timeout--;}

#endif
