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

#ifndef _U_FM_H_
#define _U_FM_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"
//#include "u_handle.h"
//#include "u_handle_grp.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define	FM_MAX_FILE_LEN 		(256 - 1) /* Unicode not including trailing NULL */
#define	FM_MAX_PATH_LEN 		(1152 - 1)
#define FM_ROOT_HANDLE          NULL_HANDLE

/*
 *  Handle types supported by the File Manager.
 */
#define FMT_FILE_DESC   (HT_GROUP_FM + ((HANDLE_TYPE_T) 0))
#define FMT_DIR_LABEL   (HT_GROUP_FM + ((HANDLE_TYPE_T) 1))
#define FMT_ASYNC_READ  (HT_GROUP_FM + ((HANDLE_TYPE_T) 2))
#define FMT_ASYNC_WRITE (HT_GROUP_FM + ((HANDLE_TYPE_T) 3))
#define FMT_PART_TBL    (HT_GROUP_FM + ((HANDLE_TYPE_T) 4))
#define FMT_MTP_DESC    (HT_GROUP_FM + ((HANDLE_TYPE_T) 5))
#define FMT_MTP_ASYNC_READ (HT_GROUP_FM + ((HANDLE_TYPE_T) 6))
#define FMT_NET_FM      (HT_GROUP_FM + ((HANDLE_TYPE_T) 7))  


/*
 *  Device types.
 */
#define FM_DRVT_EEPROM            ((UINT16)  96)
#define FM_DRVT_NOR_FLASH         ((UINT16)  97)
#define FM_DRVT_NAND_FLASH        ((UINT16)  98)
#define FM_DRVT_MEM_CARD          ((UINT16)  99)
#define FM_DRVT_HARD_DISK         ((UINT16) 100)
#define FM_DRVT_USB_MASS_STORAGE  ((UINT16) 101)
#define FM_DRVT_OPTICAL_DISC      ((UINT16) 102)
#define FM_DRVT_PTP_MTP_DEVICE    ((UINT16) 103)
#define FM_DRVT_USB_IPOD           ((UINT16) 104)

/*
 *  Return values.
 */
#define	FMR_OK                  ((INT32)  0)
#define	FMR_ARG                 ((INT32) -1)
#define	FMR_HANDLE              ((INT32) -2)
#define	FMR_INVAL               ((INT32) -3)
#define FMR_CORE                ((INT32) -4)
#define	FMR_EXIST               ((INT32) -5)
#define	FMR_NO_ENTRY            ((INT32) -6)
#define	FMR_NOT_DIR             ((INT32) -7)
#define	FMR_IS_DIR              ((INT32) -8)
#define	FMR_DIR_NOT_EMPTY       ((INT32) -9)
#define	FMR_NAME_TOO_LONG       ((INT32) -10)
#define	FMR_FILE_TOO_LARGE      ((INT32) -11)
#define	FMR_BUSY                ((INT32) -12)
#define FMR_EOF                 ((INT32) -13)
#define FMR_LOCK_FAIL           ((INT32) -14)
#define FMR_WOULD_BLOCK         ((INT32) -15)
#define FMR_PERM_DENY           ((INT32) -16)
#define FMR_ACCESS              ((INT32) -17)
#define FMR_NOT_INIT            ((INT32) -18)
#define FMR_ALIGNMENT           ((INT32) -19)
#define FMR_ASYNC_NOT_SUPPORT   ((INT32) -20)
#define FMR_DEVICE_ERROR        ((INT32) -21)
#define	FMR_NO_SUCH_DEVICE      ((INT32) -22)
#define	FMR_NOT_ENOUGH_SPACE    ((INT32) -23)
#define	FMR_FILE_SYSTEM_FULL    ((INT32) -24)
#define	FMR_FILE_SYSTEM_CRASH   ((INT32) -25)
#define FMR_FILE_SYSTEM_OTHER   ((INT32) -26)
#define FMR_CLI_ERROR           ((INT32) -27)
#define FMR_NO_MBR              ((INT32) -28)
#define FMR_EOC                 ((INT32) -29)
#define FMR_FAT_ERROR           ((INT32) -30)
#define FMR_DELETING_ENTRY      ((INT32) -31)
#define FMR_OUT_OF_RANGE        ((INT32) -32)
#define FMR_OVER_VALID_RANGE    ((INT32) -33)
#define FMR_ABNORMAL_ENDED      ((INT32) -34)
#define FMR_BLANK_SECTOR        ((INT32) -35)
#define FMR_CMD_TIMEOUT         ((INT32) -36)
#define FMR_NOT_MOUNTED         ((INT32) -37)
#define FMR_ERROR               ((INT32) -99)
#define FMR_NOT_IMPLEMENT       ((INT32) -100)

/*
 *  Return error values of HTTP file manager.
 *  Please add your error code between FMR_HTTP_START and FMR_HTTP_END
 */
#define FMR_HTTP_START                    ((INT32) -399)
#define FMR_HTTP_BAD_REQUEST              ((INT32) -400) /* 400	*/
#define FMR_HTTP_UNAUTHORIZED             ((INT32) -401) /* 401	*/
#define FMR_HTTP_PAYMENT_REQUIRED         ((INT32) -402) /* 402	*/
#define FMR_HTTP_FORBIDDEN                ((INT32) -403) /* 403	*/
#define FMR_HTTP_NOT_FOUND                ((INT32) -404) /* 404	*/
#define	FMR_HTTP_METHOD_NOT_ALLOWED       ((INT32) -405) /* 405	*/
#define	FMR_HTTP_NOT_ACCEPTABLE           ((INT32) -406) /* 406	*/
#define	FMR_HTTP_PROXY_AUTH_REQUIRED      ((INT32) -407) /* 407	*/
#define	FMR_HTTP_REQUEST_TIMEOUT          ((INT32) -408) /* 408	*/
#define	FMR_HTTP_CONFLICT                 ((INT32) -409) /* 409	*/
#define	FMR_HTTP_GONE                     ((INT32) -410) /* 410	*/
#define	FMR_HTTP_LENGTH_REQUIRED          ((INT32) -411) /* 411	*/
#define	FMR_HTTP_PRECONDITION_FAILED      ((INT32) -412) /* 412	*/
#define	FMR_HTTP_REQUEST_ENTITY_TOO_LARGE ((INT32) -413) /* 413	*/
#define	FMR_HTTP_REQUEST_URI_TOO_LARGE    ((INT32) -414) /* 414	*/
#define	FMR_HTTP_UNSUPPORTED_MEDIA_TYPE   ((INT32) -415) /* 415	*/

#define	FMR_HTTP_INTERNAL_SERVER_ERROR    ((INT32) -500) /* 500	*/
#define	FMR_HTTP_NOT_IMPLEMENTED          ((INT32) -501) /* 501	*/
#define	FMR_HTTP_BAD_GATEWAY              ((INT32) -502) /* 502	*/
#define	FMR_HTTP_SERVICE_UNAVAILABLE      ((INT32) -503) /* 503	*/
#define	FMR_HTTP_GATEWAY_TIMEOUT          ((INT32) -504) /* 504	*/
#define	FMR_HTTP_VERSION_UNSUPPORTED 	  ((INT32) -505) /* 505	*/

#define	FMR_HTTP_UNKNOWN_ERROR		 	  ((INT32) -600)
#define	FMR_HTTP_TIME_OUT			 	  ((INT32) -601)
#define	FMR_HTTP_END    			 	  ((INT32) -700)

typedef INT64  TIME_T;
/*
 *  Flags for file descriptor.
 */
#define	FM_ACCESS_MODE          ((UINT32) 0003)
#define	FM_READ_ONLY 	        ((UINT32) 0000)
#define	FM_WRITE_ONLY 	        ((UINT32) 0001)
#define	FM_READ_WRITE           ((UINT32) 0002)
#define	FM_OPEN_CREATE          ((UINT32) 0100)
#define FM_OPEN_EXCLUDE         ((UINT32) 0200)
#define	FM_OPEN_TRUNC           ((UINT32) 01000)
#define	FM_OPEN_APPEND          ((UINT32) 02000)
#define FM_NO_CACHE             ((UINT32) 010000)

#define FM_OPEN_SYNC             ((UINT32) 0x10000)
#define FM_OPEN_RSYNC            ((UINT32) 0x20000)
#define FM_OPEN_DSYNC            ((UINT32) 0x40000)


/*
 *  lseek starting position
 */
#define FM_SEEK_BGN             ((UINT8) 1)
#define FM_SEEK_CUR             ((UINT8) 2)
#define FM_SEEK_END             ((UINT8) 3)

/*
 *  File lock
 */
#define FM_LOCK_READ            ((UINT32) 1)
#define FM_LOCK_WRITE           ((UINT32) 2)
#define FM_LOCK_NO_WAIT         ((UINT32) 4)
#define FM_LOCK_RELEASE         ((UINT32) 8)

/*
 *  File mode.
 */
#define FM_MODE_USR_READ        ((UINT32) 0400)
#define FM_MODE_USR_WRITE       ((UINT32) 0200)
#define FM_MODE_USR_EXEC        ((UINT32) 0100)
#define FM_MODE_GRP_READ        ((UINT32) 0040)
#define FM_MODE_GRP_WRITE       ((UINT32) 0020)
#define FM_MODE_GRP_EXEC        ((UINT32) 0010)
#define FM_MODE_OTH_READ        ((UINT32) 0004)
#define FM_MODE_OTH_WRITE       ((UINT32) 0002)
#define FM_MODE_OTH_EXEC        ((UINT32) 0001)
#define FM_MODE_PERM_MASK       ((UINT32) 0777)
#define FM_MODE_TYPE_FILE       ((UINT32) 0x00010000)
#define FM_MODE_TYPE_DIR        ((UINT32) 0x00020000)
#define FM_MODE_TYPE_BLK        ((UINT32) 0x00040000)
#define FM_MODE_TYPE_CHR        ((UINT32) 0x00080000)
#define FM_MODE_TYPE_LINK       ((UINT32) 0x00100000)
#define FM_MODE_TYPE_PTP_MTP    ((UINT32) 0x00200000)
#define FM_MODE_TYPE_MASK       ((UINT32) 0x003F0000)
#define FM_MODE_DEV_TRUE        ((UINT32) 0x00400000)
#define FM_MODE_DEV_VIRT        ((UINT32) 0x00800000)
#define FM_MODE_DEV_MASK        ((UINT32) 0x00C00000)

#define FM_IS_FILE(mode) \
    (((mode) & FM_MODE_TYPE_MASK) == FM_MODE_TYPE_FILE)
#define FM_IS_DIR(mode) \
    (((mode) & FM_MODE_TYPE_MASK) == FM_MODE_TYPE_DIR)
#define FM_IS_BLK(mode) \
    (((mode) & FM_MODE_TYPE_MASK) == FM_MODE_TYPE_BLK)
#define FM_IS_CHR(mode) \
    (((mode) & FM_MODE_TYPE_MASK) == FM_MODE_TYPE_CHR)
#define FM_IS_LINK(mode) \
    (((mode) & FM_MODE_TYPE_MASK) == FM_MODE_TYPE_LINK)
#define FM_IS_PTP_MTP(mode) \
    (((mode) & FM_MODE_TYPE_MASK) == FM_MODE_TYPE_PTP_MTP)
#define FM_IS_TRUE(mode) \
    (((mode) & FM_MODE_DEV_MASK) == FM_MODE_DEV_TRUE)
#define FM_IS_VIRT(mode) \
    (((mode) & FM_MODE_DEV_MASK) == FM_MODE_DEV_VIRT)
/*
 * Directory information
 */
 typedef struct _FM_DIR_INFO_T
 {
    UINT32  ui4_dir_num;
    UINT32  ui4_file_num;
 } FM_DIR_INFO_T;
/*
 *  File information.
 */
typedef struct _FM_FILE_INFO_T
{
    UINT32      ui4_inode;
    UINT32      ui4_mode;
    UINT32      ui4_uid;
    UINT32      ui4_gid;
    UINT64      ui8_size;
    UINT64      ui8_offset; /* used for chop */
    UINT32      ui4_blk_size;
    UINT64      ui8_blk_cnt;
    TIME_T      ui8_create_time;
    TIME_T      ui8_access_time;
    TIME_T      ui8_modify_time;

    UINT32      ui4_start_lba;
    BOOL        b_copy_protected;
} FM_FILE_INFO_T;

/*
 *  Directory entry.
 */
typedef struct _FM_DIR_ENTRY_T
{
    UINT8   ui1_len;                            /* number of characters */
    CHAR    s_name[(FM_MAX_FILE_LEN + 1) * 4];  /* for UTF-8, enough??? */

    UINT64  ui8_dirent_ofst;
    UINT32  ui4_dirent_len;

    FM_FILE_INFO_T t_file_info;
} FM_DIR_ENTRY_T;

/*
 *  File system types.
 */
typedef enum
{
    FM_TYPE_INVAL  = 0,
    FM_TYPE_ROOTFS,
    FM_TYPE_FAT,
    FM_TYPE_FAT12,
    FM_TYPE_FAT16,
    FM_TYPE_FAT32,
    FM_TYPE_UDF,
    FM_TYPE_ISO9660,
    FM_TYPE_MEMFS,
    FM_TYPE_SFS,
    FM_TYPE_MTP,
    FM_TYPE_NTFS
} FM_FS_TYPE_T;
/*
* UDF file system version
*/
typedef enum
{
    FM_UDF_UNKNOW =0,
    FM_UDF_100,
    FM_UDF_102,
    FM_UDF_150,
    FM_UDF_200,
    FM_UDF_201,
    FM_UDF_250,
    FM_UDF_260
}FM_UDF_VERSION;
/*
 *  File system information.
 */
typedef struct _FM_FS_INFO_T
{
	FM_FS_TYPE_T    e_type;
	UINT64	        ui8_blk_size;
	UINT64	        ui8_total_blk;
	UINT64	        ui8_free_blk;
	UINT64	        ui8_avail_blk;
	UINT64	        ui8_files;
	UINT16	        ui2_max_name_len;

	UINT32              ui4_alignment;
	UINT32              ui4_min_blk_size;

	UINT64              ui8_first_free_blk;

	//Martin_20081208: Get device volume label for use
	UINT8		s_volume_label[32];

    //MingHuang_2009_6_30  Get UDF Version
    FM_UDF_VERSION       e_udf_version;
} FM_FS_INFO_T;

/*
 *  File system selection types.
 */
typedef enum
{
    FM_MNT_TYPE_AUTO  = 0,
    FM_MNT_TYPE_FAT,
    FM_MNT_TYPE_UDF,
    FM_MNT_TYPE_ISO9660,
    FM_MNT_TYPE_MTP,
} FM_MNT_TYPE_T;

/*
 *  File system mount parameter.
 */
typedef struct _FM_MNT_PARM_T
{
	FM_MNT_TYPE_T     e_type;
	UINT32            ui4_sess_no;
} FM_MNT_PARM_T;

/*
 *  Asynchronous notification conditions.
 */
#if 1
typedef enum
{
    FM_ASYNC_COND_BLANK_SECTOR  = -8,
    FM_ASYNC_COND_CMD_TIMEOUT	= -4,
    FM_ASYNC_COND_READ_TIMEOUT	= -3,   // to notify network http timeout
    FM_ASYNC_COND_ABORT_FAIL = -2,
    FM_ASYNC_COND_FAIL      = -1,
    FM_ASYNC_COND_READ_OK   =  1,
    FM_ASYNC_COND_WRITE_OK  =  2,
    FM_ASYNC_COND_ABORT_OK  =   3,
    FM_ASYNC_COND_STREAMING_EOF = 4
} FM_ASYNC_COND_T;
#else
typedef INT32 FM_ASYNC_COND_T;

#define FM_ASYNC_COND_ABORT_FAIL    ((FM_ASYNC_COND_T) -2)
#define FM_ASYNC_COND_FAIL          ((FM_ASYNC_COND_T) -1)
#define FM_ASYNC_COND_READ_OK       ((FM_ASYNC_COND_T) 1)
#define FM_ASYNC_COND_WRITE_OK      ((FM_ASYNC_COND_T) 2)
#define FM_ASYNC_COND_ABORT_OK      ((FM_ASYNC_COND_T) 3)
#endif

typedef struct _FM_PART_INFO_T
{
    BOOL    b_try_mnt;
    BOOL    b_mnt;

    CHAR    ps_part_name[32];
    CHAR    ps_part_path[32];
    CHAR    ps_mnt_path[32];
} FM_PART_INFO_T;

typedef enum
{
    FM_MNT_OK,
    FM_MNT_UMNT,
    FM_MNT_FAIL
} FM_MNT_COND_T;

typedef VOID (*x_fm_mnt_fct)(
    FM_MNT_COND_T t_cond,
    CHAR  *ps_part_name,
    UINT32  ui4_part_idx,
    CHAR *ps_mnt_path,
    VOID *pv_tag);

typedef struct _FM_MNT_CB_T
{
    x_fm_mnt_fct pf_mnt_fct;
    VOID *pv_tag;
} FM_MNT_CB_T;

typedef VOID (*x_fm_async_fct)(
    HANDLE_T    h_req,
    VOID            *pv_tag,
    FM_ASYNC_COND_T e_async_cond,
    UINT32          ui4_data);
/*-----------------------------------------------------------------------------
                    data declarations
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

#endif /* _U_FM_H */
