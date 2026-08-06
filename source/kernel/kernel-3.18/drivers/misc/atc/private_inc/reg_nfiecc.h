#ifndef _REG_NFIECC_H_
#define _REG_NFIECC_H_

#if defined(CHIP_VER_AC83XX)
typedef volatile struct {
    UINT32 ECC_ENCCON;      // 0000
    UINT32 ECC_ENCCNFG;     // 0004
    UINT32 ECC_ENCDIADDR;   // 0008
    UINT32 ECC_ENCIDLE;     // 000C
    UINT32 resv10[5];       // 0010
    UINT32 ECC_ENCSTA;      // 0024
    UINT32 ECC_ENCIRQEN;    // 0028
    UINT32 ECC_ENCIRQSTA;   // 002C
	UINT32 ECC_ENCPAR0; 	// 0030
	UINT32 ECC_ENCPAR1;     // 0034
    UINT32 ECC_ENCPAR2;     // 0038
    UINT32 ECC_ENCPAR3;     // 003C
    UINT32 ECC_ENCPAR4;     // 0040
	UINT32 ECC_ENCPAR5; 	// 0044
	UINT32 ECC_ENCPAR6;     // 0048
    UINT32 ECC_ENCPAR7;     // 004C
    UINT32 ECC_ENCPAR8;     // 0050
    UINT32 ECC_ENCPAR9;     // 0054
    UINT32 ECC_ENCPAR10;    // 0058
    UINT32 resv5C[41];      // 005C
    UINT32 ECC_DECCON;      // 0100
    UINT32 ECC_DECCNFG;     // 0104
    UINT32 ECC_DECDIADDR;   // 0108
    UINT32 ECC_DECIDLE;     // 010C
    UINT32 ECC_DECFER;      // 0110
  	UINT32 resv114[1];      // 0114
    UINT32 ECC_DECDONE;     // 0118
 	UINT32 resv11C[6];      // 011C
    UINT32 ECC_DECIRQEN;    // 0134
    UINT32 ECC_DECIRQSTA;   // 0138
    UINT32 ECC_FDMADDR;     // 013C
    UINT32 ECC_DECFSM;      // 0140
    UINT32 ECC_SYNSTA;      // 0144
    UINT32 ECC_NFIDI;       // 0148
    UINT32 ECC_SYN0;        // 014C
	UINT32 ECC_DECENUM;     // 0150
	UINT32 ECC_DECENUM2;    // 0154
	UINT32 ECC_DECEL0;		// 0160
	UINT32 ECC_DECEL1; 	 	// 0164 
	UINT32 ECC_DECEL2; 	 	// 0168
	UINT32 ECC_DECEL3; 	 	// 016C
	UINT32 ECC_DECEL4; 	 	// 0170
	UINT32 ECC_DECEL5; 	 	// 0174
	UINT32 ECC_DECEL6;		// 0178
	UINT32 ECC_DECEL7; 	 	// 017C 
	UINT32 ECC_DECEL8; 	 	// 0180
	UINT32 ECC_DECEL9; 	 	// 0184
	UINT32 ECC_DECEL10;	 	// 0188
	UINT32 ECC_DECEL11; 	// 018C
}MT8530_NFIECC_REG;
#endif
/*******************************************************************************
 * NFIECC register bit definition
 *******************************************************************************/

/* NFIECC_ENCCON */
#define ENC_EN                (0x00000001)

/* NFIECC_ENCCNFG */
#define ENC_CNFG_ECC4         (0x00000000)
#define ENC_CNFG_ECC6         (0x00000001)
#define ENC_CNFG_ECC8         (0x00000002)
#define ENC_CNFG_ECC10        (0x00000003)
#define ENC_CNFG_ECC12        (0x00000004)
#if defined(CHIP_VER_AC83XX)
#define ENC_CNFG_ECC14        (0x00000005)
#define ENC_CNFG_ECC16        (0x00000006)
#define ENC_CNFG_ECC18        (0x00000007)
#define ENC_CNFG_ECC20        (0x00000008)
#define ENC_CNFG_ECC22        (0x00000009)
#define ENC_CNFG_ECC24        (0x0000000A)
#endif

#if defined(CHIP_VER_AC83XX)
#define ENC_CNFG_ECC_MASK     (0x0000000F)
#endif
#define ENC_CNFG_NFI_MODE     (0x00000010)

#if defined(CHIP_VER_AC83XX)
#define ENC_CNFG_MS_MASK      (0xFFFF0000)
#endif
#define ENC_CNFG_MS_SHIFT     (16)

/* NFIECC_ENCIDLE */
#define ENC_IDLE              (0x00000001)

/* NFIECC_ENCSTA */
#if defined(CHIP_VER_AC83XX)
#define ENC_FSM               (0x0000003F)
#define ENC_FSM_IDLE          (0x00000000)
#define ENC_FSM_WAITIN        (0x00000001)
#define ENC_FSM_BUSY          (0x00000002)
#define ENC_FSM_PAROUT        (0x00000004)
#define ENC_COUNT_PS_MASK     (0x0000FFC0)
#define ENC_COUNT_PS_SHIFT    (6)
#define ENC_COUNT_MS_MASK     (0xFFFF0000)
#define ENC_COUNT_MS_SHIFT    (16)
#endif
/* NFIECC_ENCIRQEN */
#define ENC_IRQEN             (0x00000001)

/* NFIECC_ENCIRQSTA */
#define ENC_IRQSTA            (0x00000001)

/* NFIECC_DECCON */
#define DEC_EN                (0x00000001)

/* NFIECC_DECCNFG */
#define DEC_CNFG_ECC4         (0x00000000)
#define DEC_CNFG_ECC6         (0x00000001)
#define DEC_CNFG_ECC8         (0x00000002)
#define DEC_CNFG_ECC10        (0x00000003)
#define DEC_CNFG_ECC12        (0x00000004)
#if defined(CHIP_VER_AC83XX)
#define DEC_CNFG_ECC14        (0x00000005)
#define DEC_CNFG_ECC16        (0x00000006)
#define DEC_CNFG_ECC18        (0x00000007)
#define DEC_CNFG_ECC20        (0x00000008)
#define DEC_CNFG_ECC22        (0x00000009)
#define DEC_CNFG_ECC24        (0x0000000A)
#endif

#if defined(CHIP_VER_AC83XX)
#define DEC_CNFG_ECC_MASK     (0x0000000F)
#endif
#define DEC_CNFG_NFI_MODE     (0x00000010)
#define DEC_CNFG_FER          (0x00001000)
#define DEC_CNFG_EL           (0x00002000)
#define DEC_CNFG_CORRECT      (0x00003000)
#define DEC_CNFG_TYPE_MASK    (0x00003000)

#if defined(CHIP_VER_AC83XX)

#define DEC_CNFG_CS_MASK      (0x3FFF0000)
#endif
#define DEC_CNFG_CS_SHIFT     (16)
#define DEC_CNFG_EMPTY_EN     (0x80000000)

/* NFIECC_DECIDLE */
#define DEC_IDLE              (0x00000001)

/* NFIECC_DECFER */
#define DEC_FER0              (0x00000001)
#define DEC_FER1              (0x00000002)
#define DEC_FER2              (0x00000004)
#define DEC_FER3              (0x00000008)
#define DEC_FER4              (0x00000010)
#define DEC_FER5              (0x00000020)
#define DEC_FER6              (0x00000040)
#define DEC_FER7              (0x00000080)

/* NFIECC_DECENUM */
#define DEC_ERR_NUM0          (0x0000000F)
#define DEC_ERR_NUM1          (0x000000F0)
#define DEC_ERR_NUM2          (0x00000F00)
#define DEC_ERR_NUM3          (0x0000F000)
#define DEC_ERR_NUM4          (0x000F0000)
#define DEC_ERR_NUM5          (0x00F00000)
#define DEC_ERR_NUM6          (0x0F000000)
#define DEC_ERR_NUM7          (0xF0000000)

/* NFIECC_DECDONE */
#define DEC_DONE0             (0x00000001)
#define DEC_DONE1             (0x00000002)
#define DEC_DONE2             (0x00000004)
#define DEC_DONE3             (0x00000008)
#define DEC_DONE4             (0x00000010)
#define DEC_DONE5             (0x00000020)
#define DEC_DONE6             (0x00000040)
#define DEC_DONE7             (0x00000080)

/* NFIECC_DECIRQEN */
#define DEC_IRQEN             (0x00000001)

/* NFIECC_DECIRQSTA */
#define DEC_IRQSTA            (0x00000001)

#endif
