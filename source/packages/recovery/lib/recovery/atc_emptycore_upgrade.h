/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef _ATC_EMPTYCORE_UPDATE_H_
#define _ATC_EMPTYCORE_UPDATE_H_

#define CMDLINE_LENGTH 2048

typedef struct recoveryUpdateModule
{
    bool mMcu;
    bool mNavi;
    bool mUserdata;
    bool mSystemdata;
    bool mFromudisk;
}RecoveryUpdateModule;

#define UPGRADE_FLAG_DONE (1 << 0)
#define UPGRADE_FLAG_SUCCESS (1 << 1)
#define PART_NAME_LEN_MAX 20
#define PART_NUM_MAX 50

struct partition_upgrade_status_t {
    int partition_total_num; //partition total number
    int need_upgrade_partition_num; //partition number which need upgrade
    int progress_percent; //upgrade percent
/*
* upgrade is finished.
* if upgrade_finish >0, upgrade success.
* else upgrade fail.
*/
    int upgrade_finish;
    long long total_size_need_upgrade;
    long long size_upgrade_done;
    char upgrade_ongoing_part_name[PART_NAME_LEN_MAX];
    int part_count_upgrade_done; //partion number have upgrade done.
    struct one_partition_status_t {
        char part_name[PART_NAME_LEN_MAX];
/*
* flag bit0, partition upgrade is done or not begin
* 0: not begin
* 1: done
*
* flag bit1, this bit is valid when bit0 is 1. means partition upgrade success or fail
* 0: fail
* 1: success
*/
        int flag;
    } one_partition_status[PART_NUM_MAX];
};
/*upgrade status value*/
struct partition_upgrade_status_t *export_get_upgrade_status(void);
/*get upgrade ongoing partname */
char *export_get_part_upgrade_ongoing(void);
/*get progress during upgrade*/
int export_get_upgrade_progress(void);
/*get kernel version and show screen*/
char *export_get_kernel_version(void);
/*emptycore upgrade main function*/
int export_emptycore_upgrade(RecoveryUpdateModule *prum);
/*sdcard hotplug  init function*/
int export_hotplug_monitor_init(void);
/*get sdcard hotplug status function*/
int export_get_hotplug_status(void);
int export_put_hotplug_status(void);
int export_get_upgrade_thread_status(void);

void nand_read_write_test(void);

#endif /* _ATC_EMPTYCORE_UPDATE_H_ */

