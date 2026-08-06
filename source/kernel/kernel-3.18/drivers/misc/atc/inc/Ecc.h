#ifndef _ECC_H_
#define _ECC_H_

#if __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Macro definition
 *******************************************************************************/
#ifdef CHIP_VER_MT8530
#define ECC_MAX_CORRECTABLE_BITS  (12)
#endif

#if defined(CHIP_VER_AC83XX)
#define ECC_MAX_CORRECTABLE_BITS  (24)
#endif
//#ifdef CHIP_VER_MT8530
#define ENC_MESSAGE_SIZE(eccfdm)       ((512 + eccfdm) << 3)
#define DEC_CODE_SIZE(eccfdm, bitnum)  (((512 + eccfdm) << 3) + bitnum * 13)
//#endif

#if defined(CHIP_VER_AC83XX)

#define ENC_MESSAGE_SIZE_SEL(eccfdm)       (( 512 + eccfdm) << 3)
#define DEC_CODE_SIZE_SEL(eccfdm, bitnum)  ((( 512 + eccfdm) << 3) + bitnum * 14)
#define ENC_MESSAGE_SIZE_NSEL(eccfdm)       (( 1024 + eccfdm) << 3)
#define DEC_CODE_SIZE_NSEL(eccfdm, bitnum)  ((( 1024 + eccfdm) << 3) + bitnum * 14)
#endif
#define ECC_4_BITS                     (4)
#define ECC_6_BITS                     (6)
#define ECC_8_BITS                     (8)
#define ECC_10_BITS                    (10)
#define ECC_12_BITS                    (12)
#if defined(CHIP_VER_AC83XX)
#define ECC_14_BITS                    (14)
#define ECC_16_BITS                    (16)
#define ECC_18_BITS                    (18)
#define ECC_20_BITS                    (20)
#define ECC_22_BITS                    (22)
#define ECC_24_BITS                    (24)
#endif

/*******************************************************************************
 * Data type definition
 *******************************************************************************/
typedef enum {
   ECC_DEC_DETECT,
   ECC_DEC_LOCATE,
   ECC_DEC_CORRECT,
   ECC_DEC_NONE
} EccDecType_T;

typedef enum {
   ECC_DECODER,
   ECC_ENCODER
} EccSubModule_T;

typedef struct {
	EccDecType_T     EccDecType;
	UINT32           u4EncMsgSize;
	UINT32           u4DecCodeSize;
	UINT32           u4EccBits;
	BOOL             fgNFIMode;
	BOOL             fgEncIRQEn;
	BOOL             fgDecIRQEn;
	BOOL             fgDecEmptyCheck;
} EccConf_T;

typedef enum {
   ECC_Error_FSM_Initial,
   ECC_Error_FSM_Active
} EccErrFSMState_t;

typedef enum {
   ECC_Error_FSM_Random,
   ECC_Error_FSM_Incremental,
   ECC_Error_FSM_Exhaustive
} EccErrFSMOP_t;

typedef enum {
   ECC_Error_FSM_Finished,
   ECC_Error_FSM_Error,
   ECC_Error_FSM_Working
} EccErrFSMStatus_t;

typedef void (*dbg_trace_func)(char *fmt,...);

typedef struct {
   UINT32                ecc_correct_bits;
   UINT32                ecc_data_buff_addr;
   UINT32                ecc_data_buff_len;
   UINT32                ecc_error_location[ECC_MAX_CORRECTABLE_BITS];
   UINT32                ecc_next_round_begin[ECC_MAX_CORRECTABLE_BITS];
   UINT32                ecc_valid_FSM;
   EccErrFSMOP_t         ecc_error_op;
   EccErrFSMState_t      ecc_error_FSM;
   dbg_trace_func        ecc_dbg_print;
} EccErrFSM_t;

/*******************************************************************************
 * ECC API function definition
 *******************************************************************************/
void ECC_Setup_AHB_Buffer(EccSubModule_T EccSubMode, UINT32 data_addr);
void ECC_Setup_FDM_Register_Base(UINT32 base_addr);
void ECC_Setup_ECC_Level(EccSubModule_T EccSubMode, UINT32 u4EccBitsNum);
void ECC_Setup_ECC_Op_Mode(EccSubModule_T EccSubMode, BOOL fgNFIMode);
void ECC_Setup_ECC_IRQ(EccSubModule_T EccSubMode, BOOL fgIrqEn);
void ECC_Setup_ECC_Message_Size(UINT32 u4EncBits);
void ECC_Setup_ECC_Code_Size(UINT32 u4DecBits);
void ECC_Setup_ECC_Decode_Type(EccDecType_T DecType);
void ECC_Dec_Empty_Check(BOOL fgEnable);
void ECC_Config(EccConf_T *EccConf);
void ECC_Start_Operation(EccSubModule_T EccSubMode);
void ECC_End_Operation(EccSubModule_T EccSubMode);
void ECC_HW_Reset(void);
void ECC_DeInit(void);
BOOL ECC_Init(void);
BOOL ECC_Wait_Decode_Done(UINT32 sec_num, UINT32 timeout);
BOOL ECC_Data_Encode(UINT32 dataAddr, UINT32 dataSize, UINT32 *parityBuff);
BOOL ECC_Data_Decode(UINT32 dataAddr, UINT32 dataSize);
UINT32 ECC_GetDecStatus(void);
UINT32 ECC_GetDecErrNum(void);
UINT32 ECC_GetDecFER(void);
UINT32 ECC_Get_Parity_Size(void);
UINT32 ECC_Get_Parity_Content(UINT32 *parity_buff);

#if __cplusplus
}
#endif

#endif
