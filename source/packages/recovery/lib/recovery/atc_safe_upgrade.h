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

#ifndef _ATC_SAFE_UPGRADE_
#define _ATC_SAFE_UPGRADE_

#include <stdint.h>
#include <atc_emptycore_upgrade.h>
#include <atc_update.h>

#define NEW_UDISK_UPGRADE 1
#define UDISK_MOUNT_POINT "/media/udisk1"
#define UDISK_DEVICE "/dev/sda1"
//#define UDISK_ROOT "/media"

#define UDISK_ROOT "/media/udisk1/"

#define ISO_MOUNT_POINT "/mnt"
#define ISO_ROOT "/mnt/"

#define MD5_FILE "/media/udisk1/iso.md5"

#ifdef CONFIG_BOOT_MMC
#define XML_FILE "scatter.mmcboot.ext4.xml"
#else
#define XML_FILE "scatter.nand.ext4.xml"
#endif
#define AB_UPDATE 1

/*
* upgrade information from <bcb.laststatus>
* if this time of upgrade is power loss, the next time of upgrade
* will decide how to do based on this status information.
*/
struct safeupg_upg_info {
    uint32_t m_coreprog;
    uint32_t m_systemprog;
    uint32_t m_app;
    uint32_t m_userdata;
    uint32_t verifyimg;
    //uint32_t m_mcu;
    //uint32_t m_navi;
   uint32_t exception;//latest_upg_exception;

};

#define UPG_INFO_COREPROG (1 << 0)
#define UPG_INFO_SYSTEMPROG (1 << 1)
#define UPG_INFO_APP (1 << 2)
#define UPG_INFO_USERDATA (1 << 3)
#define UPG_VERIFY_IMG (1 << 4)
//#define UPG_INFO_MCU (1 << 4)
//#define UPG_INFO_NAVI (1 << 5)
#define UPG_INFO_EXCEPTION (1 << 6)

int export_safeupg_iso_file_md5_verify(void);
//int export_safeupg_upgrade(RecoveryUpdateModule *prum);
int export_safeupg_upgrade();
int export_safeupg_iso_file_exist(void);
int export_safeupg_popup_window(void);
void export_safeupg_finish_recovery();
int export_safeupg_get_upgrade_progress(void);
int export_safeupg_udisk_hotplug_monitor_init(void);
int export_safeupg_get_udisk_hotplug(void);
int export_safeupg_get_updatethread_state(void);

struct partition_upgrade_status_t *export_safeupg_get_upg_status(void);

char *export_safeupg_get_part_upgrade_ongoing(void);

#endif
