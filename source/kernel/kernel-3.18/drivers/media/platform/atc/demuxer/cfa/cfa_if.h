/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


#ifndef _CFA_IF_H_
#define _CFA_IF_H_

/*! @name CFA Interface Include Header File (6.0) */
/*! @{ */

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#else				/*  */
#include "dmx_define.h"
#endif				/* __linux__ */

#include "dmx_def.h"
#include "mmisc.h"

#ifndef __linux__
/*disable warning C4214: nonstandard extension used : bit field types other than int*/
#pragma warning(disable : 4214)
#endif				/* #ifndef __linux__ */

/*#include "drv_cfa.h"*/

/*! @} */

/*! @name CFA Interface Constants, enumerations and macros (6.1) */
/*! @{ */

/* CFA CPS Type*/
typedef enum {
	CFA_CPS_TYPE_NONE = 0, CFA_CPS_TYPE_CSS = 1, CFA_CPS_TYPE_CPPM = 2, CFA_CPS_TYPE_CPRM =
	    3, CFA_CPS_TYPE_VCPS = 4, CFA_CPS_TYPE_AES = 5, CFA_CPS_TYPE_WMDRM_PD =
	    6, CFA_CPS_TYPE_WMDRM_ND = 7, CFA_CPS_TYPE_DIVXDRM = 8, CFA_CPS_TYPE_SACD = 9,
} CfaCpsType;

/*! @} */

/* CFA Video Picture Type*/
typedef enum {
	CFA_PIC_UNDEFINE = 0x1,	/* Picture Type undefined */
	CFA_PIC_I = 0x2,	/*Picture Type I frame */
	CFA_PIC_P = 0x3,	/*Picture Type P frame */
	CFA_PIC_B = 0x4		/*Picture Type B frame */
} CfaPicType;
typedef enum {
	/* The bits turned ON in u4StrmToPrs are the streams that Splitter would like to parse. */
	CFA_STREAM_ON,
	/* The bits turned ON in u4StrmToPrs are the streams that Splitter would like to stop parsing. */
	CFA_STREAM_OFF
} CfaStreamOp;
typedef enum {
	CFA_PARAM_ID_JUMP_INFO_SIZE,
	MAX_CNT_OF_CFA_PARAM_ID
} CfaGetParamId;

/*!
* @brief get picture information
*
* It is for splitter to get picture information from CFA.
* It fill picture info, called once per picture in fifo.
*
* @note that u8LastDts and u8LastPts may be 0, if it's the 1st picture
*
* @see CFA_IF_FILL_PIC_INFO
* @see CfaPicType
*/
typedef struct {
	CfaPicType ePicType;	/* [IN] CFA Video Picture Type */

	/* [IN] Ignore now. indicate if the start byte of this picture is from the previous transfer */
	bool fgLastPtx;

	/* [IN/OUT] Ignore now. This is DTS field of the previous
	 * picture. For pu4PrevDts, and extend to u64, If modify,
	 * it will change the value to Splitter.*/
	u64 u8LastDts;

	/*  [IN/OUT] Ignore now. This is PTS field of the previous
	 * picture. For pu4PrevPts, and extend to u64, If modify,
	 * it will change the value to Splitter. */
	u64 u8LastPts;

	/* [OUT] For AU Table, This is DTS field of the current
	 * picture. For pu4Dts, and extend to u64 */
	u64 u8ThisDts;

	/*  [OUT] For AU Table, This is PTS field of the current
	 * picture. For pu4Pts, and extend to u64 */
	u64 u8ThisPts;

	/* [OUT] May ignore now. This is offset position of the
	 * current picture. For pu4Pos, and extend to u64 */
	u64 u8OffsetS;

	/* [OUT] For AU Table, Please check
	 * AU table to see what is this for */
	u64 u8SoftPts;

	/* [OUT] For AU Table, This is for customer,
	 * will filled into AU table (u64 u8MpgPackSa;
	 * for CFA_MPG only, offset position of the current
	 * picture.CFA_MPG only, offset position of the
	 * current picture.) */
	u64 u8Custom1;

	/* [OUT] For AU Table, This is for
	 * customer, will filled into AU table */
	u64 u8Custom2;
} Spt2CfaPicInfo;

/*!
* @brief get subpicture information
*
* It is for splitter to get subpicture information from CFA.
* It fill subpicture info, called once per subpicture in fifo.
*
* @note that u8Dts and u8EndPts(Divx) may be 0
*
* @see CFA_IF_FILL_SUBPIC_INFO
* @see CfaPicType
*/
typedef struct {
	u64 u8StartPts;	/* [OUT] For AU Table, Start PTS, Splitter fill it. */
	u64 u8EndPts;	/*  [OUT] For AU Table, End PTS, Splitter fill it. */
	u64 u8Dts;		/* [OUT] For AU Table, Dts, Splitter fill it. */
	u64 u8CusInf1;	/*  [OUT] For AU Table, custom information 1, Splitter fill it. */
} Spt2CfaSubPicInfo;
typedef enum _DMX_CFA_CLI_TYPE_ {
	DMX_CFA_CLI_CMD_TURN_ONOFF_LOG,
	DMX_CFA_CLI_CMD_DUMP_INFO,
	MAX_CNT_OF_DMX_CFA_CLI_CMD_TYPE
} E_DMX_CFA_CLI_TYPE_T;

/*! @name CFA Interface Structures (6.2) */
/*! @{ */
/*! @} */

/*! @name CFA Interface Type Define (6.3) */
/*! @{ */
/*! @} */

/*! @name CFA Interface Variables (6.4) */
/*! @{ */
/*! @} */

/*! @name Splitter Interface To CFA Type Define (6.3) */
/*! @{ */

/*!
* @brief initial
*
* It is for splitter to set initial cfa.
*
* @par Qualifier
* Synchronous, mandatory
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
*/
typedef MRESULT(*CFA_IF_INIT) (HANDLE hSpt, void * *ppvPrivData);

/*!
* @brief uninitial
*
* It is for splitter to set uninitial cfa.
*
* @par Qualifier
* Synchronous, mandatory
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
* HANDLE hSpt, [IN] input splitter Handle
* void *pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_UNINIT) (HANDLE hSpt, void * pvPrivData);

/*!
* @brief set range
*
* It is for splitter to set range to cfa.
*
* @par Qualifier
* Synchronous, mandatory
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
* HANDLE hSpt, [IN] input splitter Handle
* void * pvRange, [IN] input range structure (It depends on cfa type.)
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_SET_RANGE) (HANDLE hSpt, void * pvRange, void * pvPrivData, bool fgIsUserMem);

/*!
* @brief enable (disable) stream
*
* It is for splitter to enable (disable) stream to cfa.
*
* @par Qualifier
* Synchronous, mandatory
*
* @see CfaStreamOp
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
* HANDLE hSpt, [IN] input splitter Handle
* u32 u4StrmToPrs, [IN] input stream id to parse, @see CfaStreamType
* CfaStreamOp eOp, [IN] input operation on stream (CfaStreamOp)
* void *pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_ENABLE_STRM) (HANDLE hSpt,
				      u32 u4StrmToPrs, CfaStreamOp eOp, void * pvPrivData);

/*!
* @brief set stream information
*
* It is for splitter to set stream information to cfa.
*
* @par Qualifier
* Synchronous, mandatory
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
* HANDLE hSpt, [IN] input splitter Handle
* u32 u4Strm, [IN] input stream tpye to set, @see CfaStreamType
* u32 u4StreamUID, [IN] input stream id, stream number
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_SET_STRM_INF) (HANDLE hSpt,
				       u32 u4Strm, u32 u4StreamUID, void * pvPrivData);

/*!
* @brief turn on cfa
*
* It is for splitter to turn on cfa.
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
* HANDLE hSpt, [IN] input splitter Handle
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_TURN_ON) (HANDLE hSpt, void * pvPrivData);

/*!
* @brief transfer done notify
*
* It is for splitter tell cfa transfer is done.
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
* HANDLE hSpt, [IN] input splitter Handle
* u64 u8TxLen, [IN] Total Transfer length
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_TX_DONE) (HANDLE hSpt, u64 u8TxLen, void * pvPrivData, bool fgRsp);

/*!
* @brief get current position
*
* It is for splitter to get current position from CFA.
* This is called when FMPC needs to know FFA's current position.
* The structure of current position is defined by FFA.
* FFA should provide the memory space storing the current position, and set its pointer to ppvCurPos.
* Splitter will pass the position info directly to App.
* If called during Transferring data, FFA should return a value that App will interpret as though
* that this data piece hasn't been transferred yet.
* mandatory for FFA's capable of parsing both video and audio
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
* HANDLE hSpt, [IN] input splitter Handle
* void * pvCurPos, [OUT] Current Position. TODO: change to (void *)
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_GET_CUR_POS) (HANDLE hSpt, void * pvCurPos, void * pvPrivData);

/*!
* @brief get picture information
*
* It is for splitter to get picture information from CFA.
* It fill picture info, called once per picture in fifo.
*
* @see Spt2CfaPicInfo
* @see CfaPicType
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
* HANDLE hSpt, [IN] input splitter Handle
* Spt2CfaPicInfo * ptPicInfo, [IN/OUT] Picture info, @see Spt2CfaPicInfo
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_FILL_PIC_INFO) (HANDLE hSpt, Spt2CfaPicInfo * ptPicInfo, void * pvPrivData);

/*!
* @brief get subpicture information
*
* It is for splitter to get subpicture information from CFA.
* It fill subpicture info, called once per subpicture in fifo.
*
* @see Spt2CfaSubPicInfo
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
*
* HANDLE hSpt, [IN] input splitter Handle
* Spt2CfaSubPicInfo * ptSubPicInfo, [IN/OUT] Picture info, @see Spt2CfaSubPicInfo
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_FILL_SUBPIC_INFO) (HANDLE hSpt,
					   Spt2CfaSubPicInfo * ptSubPicInfo, void * pvPrivData);

/*!
* @brief configure cfa
*
* It is for splitter to configure cfa.
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
*
* HANDLE hSpt, [IN] input splitter Handle
* void * pvParam, [IN] input configure parameter
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_CONFIGURE) (HANDLE hSpt, void * pvParam, void * pvPrivData, bool fgIsUserMem);

/*!
* @brief set inquire
*
* It is for splitter to set cfa inquire type.
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
*
* HANDLE hSpt, [IN] input splitter Handle
* u32 u4InfTypes, [IN] input inquire type parameter. TODO: change to (void *)
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_SET_INQ_TYPES) (HANDLE hSpt, u32 u4InfTypes, void * pvPrivData);

/*!
* @brief get gerenal function for cfa special
*
* It is for LPE via splitter to get cfa information
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
*
* HANDLE hSpt, IN] input splitter Handle
* u32 u4CfaFID, [IN] CFA function id, set or get id, it shall be defined by CFA and LPE
* void * pvPrivData, [IN] input CFA private data
* void * pvCfaParameter, [OUT] The parameter of this FID, it shall be defined by CFA and LPE
* u32 u4CfaParameterSize, [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE
*/
typedef MRESULT(*CFA_IF_GET_GENERAL) (HANDLE hSpt,
				      u32 u4CfaFID,
				      void * pvPrivData,
				      void * pvCfaParameter, u32 u4CfaParameterSize);
/*!
* @brief set gerenal function for cfa special
*
* It is for LPE via splitter to set cfa information
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
*
* HANDLE hSpt, [IN] input splitter Handle
* u32 u4CfaFID, [IN] CFA function id, set or get id, it shall be defined by CFA and LPE
* void * pvPrivData, IN] input CFA private data
* void * pvCfaParameter, [IN] The parameter of this FID, it shall be defined by CFA and LPE
* u32 u4CfaParameterSize, [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE
*/
typedef MRESULT(*CFA_IF_SET_GENERAL) (HANDLE hSpt,
				      u32 u4CfaFID,
				      void * pvPrivData,
				      void * pvCfaParameter, u32 u4CfaParameterSize);
/*!
* @brief get AU information
*
* It is for splitter to get AU information from CFA.
* It fill AU info, called once per AU in fifo.
*
* @see
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
*
* HANDLE hSpt, [IN] input splitter Handle
* void * pvAUInfo, [IN/OUT] AU info
* void * pvAUExtInfo, [IN/OUT] AU extensional info, may be NULL
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_FILL_AU_INFO) (HANDLE hSpt,
				       void * pvAUInfo, void * pvAUExtInfo, void * pvPrivData);

/*!
* @brief transfer audio sequence header information if needed (For AAC)
*
* It is for splitter to transfer audio sequence header information from CFA if needed.
* It is called once per Rsp audio to fifo.
*
* @see
*
* @retval NULL (0)
*           return tx ok.
* @retval NOT NULL (~0)
*           return not tx, or fail.
*
* HANDLE hSpt, input splitter Handle
* u32 u4TxUID, [IN] Tx UID
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_TX_AUD_HDRINFO) (HANDLE hSpt, u32 u4TxUID, void * pvPrivData);

/*!
* @brief Notify Pbbuf Rebuf
*
* It is for splitter to notify CFA that Pbbuf has Rebuf.
* It is called once per Rsp.
*
* @see
*
* @retval NULL (0)
*           return ok.
* @retval NOT NULL (~0)
*           return fail.
*
* HANDLE hSpt, input splitter Handle
* bool fgRebuf, [IN] flag to indicate Rebuf
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_REBUF) (HANDLE hSpt, bool fgRebuf, void * pvPrivData);

/*!
* HANDLE hSpt, [IN] input splitter Handle
* void * pvJumpInfo, [IN] Jump Info Structure (It depends on cfa type.)
* void * pvPrivData, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_SET_JUMP_INFO) (HANDLE hSpt, void * pvJumpInfo, void * pvPrivData);

/*!
* HANDLE hSpt, [IN] input splitter Handle
* u32 u4ParamID, Param id which designates what to obatin.
* void * pvPrivData, [IN] input CFA private data
* void * pvCfaParam, [OUT] The parameter of this FID, it shall be defined by CFA and LPE
* u32 u4CfaParamSz, [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE
*/
typedef MRESULT(*CFA_IF_GET_PARAM) (HANDLE hSpt,
				    u32 u4ParamID,
				    void * pvPrivData, void * pvCfaParam, u32 u4CfaParamSz);

/*!
* HANDLE hSpt, [IN] input splitter Handle
* E_DMX_CFA_CLI_TYPE_T eCliType, [IN] Cfa Cli Command
* [IN] Cfa Cli Command, [IN] input CFA private data
*/
typedef MRESULT(*CFA_IF_PROC_CLI) (HANDLE hSpt,
				   E_DMX_CFA_CLI_TYPE_T eCliType,
				   u32 arg1, u32 arg2, u32 arg3, const char * szParam,
				   void * pvPrivData);

#ifdef CONFIG_COMPAT
typedef enum {
  CFA_CONFIG,
  CFA_RANGE,
  CFA_GEN_INFO,
  CFA_JUMP_INFO,
} E_CFA_COMPAT_INFO_TYPE_T;

typedef struct {
  E_CFA_COMPAT_INFO_TYPE_T type;
  bool is_get;
  void __user *usr_ptr; /* OUT */
  void __user *usr_ptr32; /* IN */
  __u32 buf_sz; /* IN | OUT */
} CFA_COMPAT_PROC_INFO_T;

typedef long (*CFA_IF_PORC_COMPAT)(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem);
#endif /* CONFIG_COMPAT */

/*! @} */

/*! @name Splitter Interface To CFA Structures (6.2) */
/*! @{ */

/* CFA interface */
typedef struct {

	/*!
	 * @brief initial
	 *
	 * It is for splitter to set initial cfa.
	 *
	 * @par Qualifier
	 * Synchronous, mandatory
	 */
	CFA_IF_INIT pfmrInit;

	/*!
	 * @brief uninitial
	 *
	 * It is for splitter to set uninitial cfa.
	 *
	 * @par Qualifier
	 * Synchronous, mandatory
	 */
	CFA_IF_UNINIT pfmrUninit;

	/*!
	 * @brief Set parsing range.
	 *
	 * @par Qualifier
	 * Synchronous, mandatory
	 *
	 * @see CFA_IF_SET_RANGE
	 */
	CFA_IF_SET_RANGE pfmrSetRange;

	/*!
	 * @brief enable (disable) stream
	 *
	 * @par Qualifier
	 * Synchronous, mandatory
	 *
	 * @see CFA_IF_ENABLE_STRM
	 */
	CFA_IF_ENABLE_STRM pfmrEnableStrm;

	/*!
	 * @brief set stream information
	 *
	 * @see CFA_IF_SET_STRM_INF
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_SET_STRM_INF pfmrSetStrmInf;

	/*!
	 * @brief turn on cfa
	 *
	 * @see CFA_IF_TURN_ON
	 *
	 * @par Qualifier
	 * Synchronous, mandatory
	 */
	CFA_IF_TURN_ON pfmrTurnOn;

	/*!
	 * @brief transfer done notify
	 *
	 * @see CFA_IF_TX_DONE
	 *
	 * @par Qualifier
	 * Synchronous, mandatory
	 */
	CFA_IF_TX_DONE pfmrTxDone;

	/*!
	 * @brief get current position
	 *
	 * @see CFA_IF_GET_CUR_POS
	 *
	 * @par Qualifier
	 * Synchronous, mandatory
	 */
	CFA_IF_GET_CUR_POS pfmrGetCurPos;

	/*!
	 * @brief get picture information
	 *
	 * @see CFA_IF_FILL_PIC_INFO
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_FILL_PIC_INFO pfmrFillPicInfo;

	/*!
	 * @brief configure cfa
	 *
	 * @see CFA_IF_CONFIGURE
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_CONFIGURE pfmrConfigure;

	/*!
	 * @brief set inquire
	 *
	 * @see CFA_IF_SET_INQ_TYPES
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_SET_INQ_TYPES pfmrSetInqTypes;

	/*!
	 * @brief get parameter for cfa specific parameter for LPE to CFA
	 *
	 * @see CFA_IF_GET_GENERAL
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_GET_GENERAL pfmrGetGeneral;

	/*!
	 * @brief set parameter for cfa specific parameter for LPE to CFA
	 *
	 * @see CFA_IF_SET_GENERAL
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_SET_GENERAL pfmrSetGeneral;

	/*!
	 * @brief get picture information
	 *
	 * @see CFA_IF_FILL_SUBPIC_INFO
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_FILL_SUBPIC_INFO pfmrFillSubPicInfo;

	/*!
	 * @brief get AU information
	 *
	 * @see CFA_IF_FILL_AU_INFO
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_FILL_AU_INFO pfmrFillAUInfo;

	/*!
	 * @brief transfer audio sequence header information if needed (For AAC)
	 *
	 * @see CFA_IF_TX_AUD_HDRINFO
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_TX_AUD_HDRINFO pfmrTxAudHdrInfo;

	/*!
	 * @brief Notify Pbbuf Rebuf
	 *
	 * It is for splitter to notify CFA that Pbbuf has Rebuf.
	 * It is called once per Rsp.
	 *
	 * @see
	 *
	 * @retval NULL (0)
	 *           return ok.
	 * @retval NOT NULL (~0)
	 *           return fail.
	 */
	CFA_IF_REBUF pfmrRebuf;

	/*!
	 * @brief Notify Cfa to set state and params, and set jump info
	 *
	 * It is for splitter to notify CFA that jump happens, CFA should reset state
	 * parsing params
	 *
	 * @see
	 *
	 * @retval NULL (0)
	 *           return ok.
	 * @retval NOT NULL (~0)
	 *           return fail.
	 */
	CFA_IF_SET_JUMP_INFO pfmrSetJumpInfo;

	/*!
	 * @brief get parameter for cfa specific parameter for demuxer
	 *
	 * @see CFA_IF_GET_PARAM
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_GET_PARAM pfmrGetParam;

	/*!
	 * @brief inform cfa the cfa cli command
	 *
	 * @see CFA_IF_PROC_CLI
	 *
	 * @par Qualifier
	 * Synchronous, optional
	 */
	CFA_IF_PROC_CLI pfmrProcCliCmd;

#ifdef CONFIG_COMPAT
  CFA_IF_PORC_COMPAT pfmrProcCompat;
#endif /* CONFIG_COMPAT */
} CfaIntf;

/*! @} */

/*! @name CFA Interface API's (6.5) */
/*! @{ */
void *CfaGetInterface(u32 u4CfaType);

/*! @} */

#endif				/* _CFA_IF_H_ */
