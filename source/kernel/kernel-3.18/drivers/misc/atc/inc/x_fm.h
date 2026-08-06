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

#ifndef _X_FM_H_
#define _X_FM_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_fm.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    data declarations
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
extern INT32 fm_init(VOID);

extern INT32 fm_uninit(VOID);


extern INT32 x_fm_mount(
    HANDLE_T        h_dev_dir,
    const CHAR      *ps_dev_path,
    HANDLE_T        h_mp_dir,
    const CHAR      *ps_mp_path);

extern INT32 x_fm_mount_ex(
    HANDLE_T        h_dev_dir,
    const CHAR      *ps_dev_path,
    HANDLE_T        h_mp_dir,
    const CHAR      *ps_mp_path,
    FM_MNT_PARM_T   *pt_mnt_parm);

extern INT32 x_fm_umount(
    HANDLE_T        h_dir,
    const CHAR      *ps_path);

extern INT32 x_fm_get_fs_info(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    FM_FS_INFO_T    *pt_fs_info);

extern INT32 x_fm_create_fs(
    HANDLE_T        h_dir,
    const CHAR      *ps_dev,
    FM_FS_TYPE_T    e_fs_type,
    const VOID      *pv_data,
    SIZE_T          z_size);

extern INT32 x_fm_check_fs(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    BOOL            b_auto_fix);

extern INT32 x_fm_create_dir(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    UINT32          ui4_mode);

extern INT32 x_fm_delete_dir(
	HANDLE_T        h_dir,
    const CHAR      *ps_path);

extern INT32 x_fm_delete_dir_ex(
  	HANDLE_T        h_dir,
    const CHAR      *ps_path);

extern INT32 x_fm_open_dir(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    HANDLE_T        *ph_dir);

extern INT32 x_fm_open_dir_ex(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    HANDLE_T        *ph_dir,
    UINT64          ui8_ofst);

extern INT32 x_fm_read_dir_entries(
	  HANDLE_T        h_dir,
    FM_DIR_ENTRY_T  *pt_dir_entry,
    UINT32          ui4_count,
    UINT32          *pui4_entries);

extern INT32 x_fm_set_dir_path(
	  HANDLE_T        h_dir,
    const CHAR      *ps_path,
    HANDLE_T        *ph_dir);

extern INT32 x_fm_get_dir_path(
	  HANDLE_T        h_dir,
    CHAR            *ps_path,
    UINT32          *pui4_len);

extern INT32 x_fm_delete_file(
	  HANDLE_T        h_dir,
    const CHAR      *ps_path);

extern INT32 x_fm_rename(
	  HANDLE_T        h_old_dir,
    const CHAR      *ps_old_path,
    HANDLE_T        h_new_dir,
    const CHAR      *ps_new_path);

extern INT32 x_fm_open(
	  HANDLE_T        h_dir,
    const CHAR      *ps_path,
    UINT32          ui4_flags,
    UINT32          ui4_mode,
    BOOL            b_ext_buf,
    HANDLE_T        *ph_file);

extern INT32 x_fm_open_ex(
	  HANDLE_T        h_dir,
    const CHAR      *ps_path,
    UINT32          ui4_flags,
    UINT32          ui4_mode,
    BOOL            b_ext_buf,
    HANDLE_T        *ph_file,
    UINT64          ui8_ofst);

extern INT32 x_fm_open_kr(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    UINT32          ui4_flags,
    UINT32          ui4_mode,
    HANDLE_T        *ph_file);

extern INT32 x_fm_close(
    HANDLE_T        h_file);

extern INT32 x_fm_read(
    HANDLE_T        h_file,
    VOID            *pv_data,
    UINT32          ui4_count,
    UINT32          *pui4_read);

extern INT32 x_fm_write(
    HANDLE_T        h_file,
    const VOID      *pv_data,
    UINT32          ui4_count,
    UINT32          *pui4_write);

extern INT32 x_fm_read_async(
    HANDLE_T        h_file,
    VOID            *pv_data,
    UINT32          ui4_count,
    UINT8           ui1_pri,
    x_fm_async_fct  pf_nfy_fct,
    VOID            *pv_tag,
    HANDLE_T    *ph_req);

extern INT32 x_fm_write_async(
    HANDLE_T        h_file,
    const VOID      *pv_data,
    UINT32          ui4_count,
    UINT8           ui1_pri,
    x_fm_async_fct  pf_nfy_fct,
    VOID            *pv_tag,
    HANDLE_T    *ph_req);

extern INT32    x_fm_abort_async(
    HANDLE_T    h_req);

extern INT32    x_fm_lseek(
    HANDLE_T        h_file,
    INT64           i8_offset,
    UINT8           ui1_whence,
    UINT64          *pui8_cur_pos);

extern INT32    x_fm_seek_dir(
    HANDLE_T    h_dir,
    UINT8           ui1_whence,
    INT32           i4_range);

extern INT32  x_fm_seek_dir_ex(
    HANDLE_T    h_dir,
    UINT64      ui8_ofst);

extern INT32 x_fm_feof(
    HANDLE_T        h_file,
    BOOL            *pb_eof);

extern INT32 x_fm_trunc_by_name(
	  HANDLE_T        h_dir,
    const CHAR      *ps_path,
    INT64           i8_len);

extern INT32 x_fm_trunc_by_handle(
    HANDLE_T        h_file,
    INT64           i8_len);

extern INT32 x_fm_chop_by_name(
	  HANDLE_T        h_dir,
    const CHAR      *ps_path,
    INT64           i8_len);

extern INT32 x_fm_chop_by_handle(
    HANDLE_T        h_file,
    INT64           i8_len);

extern INT32 x_fm_extend_file(
    HANDLE_T        h_file,
    INT64           i8_len);

extern INT32 x_fm_sync_file(
    HANDLE_T        h_file);

extern INT32 x_fm_get_info_by_name(
	  HANDLE_T        h_dir,
    const CHAR      *ps_path,
    FM_FILE_INFO_T  *pt_info);

extern INT32 x_fm_get_info_by_name_ex(
		   HANDLE_T 	   h_dir,
		 const CHAR 	 *ps_path,
		 FM_FILE_INFO_T  *pt_info,
		 UINT64          ui8_ofst);    

extern INT32 x_fm_get_info_by_handle(
    HANDLE_T        h_file,
    FM_FILE_INFO_T  *pt_info);

extern INT32 x_fm_get_dir_info(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    FM_DIR_INFO_T   *pt_info);

extern INT32 x_fm_chmod_by_name(
	  HANDLE_T        h_dir,
    const CHAR      *ps_path,
    UINT32          ui4_mode);

extern INT32 x_fm_chmod_by_handle(
    HANDLE_T        h_file,
    UINT32          ui4_mode);

extern INT32 x_fm_make_entry(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    UINT32          ui4_mode,
    UINT16          ui2_dev,
    UINT16          ui2_unit);

extern INT32 x_fm_delete_entry(
    HANDLE_T        h_dir,
    const CHAR      *ps_path,
    UINT32          ui4_mode);

extern INT32 x_fm_lock(
    HANDLE_T        h_file,
    UINT32          ui4_op);

extern INT32 x_fm_unlock(
    HANDLE_T        h_file);

extern INT32 x_fm_grabbed(
    HANDLE_T        h_file,
    UINT32          ui4_flag,
    BOOL            *pb_res);

extern INT32 x_fm_get_part_ns(
    HANDLE_T h_dir,
    const CHAR *ps_path,
    UINT32 *pui4_count);

extern INT32 x_fm_get_part_info(
    HANDLE_T h_dir,
    const CHAR *ps_path,
    UINT32  ui4_part_idx,
    FM_PART_INFO_T *pt_part_info);

extern INT32 x_fm_parse_drive(
    HANDLE_T h_dir,
    const CHAR *ps_path,
    const VOID *pv_data);

extern INT32 x_fm_release_drive(
    HANDLE_T h_dir,
    const CHAR *ps_path);

extern INT32 x_fm_set_time_by_name(
    HANDLE_T h_dir,
    const CHAR *ps_path,
    TIME_T t_access_time,
    TIME_T t_modified_time);

extern INT32 x_fm_recyc_buf(
    HANDLE_T h_dir,
    const CHAR *ps_path);

extern INT32 x_fm_flush_data(
	VOID);

extern INT32 x_fm_flush_buf(
    HANDLE_T h_dir,
    const CHAR      *ps_path);

#endif /* _X_FM_H */
