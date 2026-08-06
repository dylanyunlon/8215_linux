/*
* Copyright (c) 2020 AutoChips Inc.
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

/* THE HAL BOOTCTRL HEADER MUST BE IN SYNC WITH THE UBOOT BOOTCTRL HEADER */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdint.h>
#include <string.h>
#include "utils/Util.hpp"
#include "utils/macro.hpp"
#include "gpt/GPT.hpp"
#include "bootctrl/BootCtrl.hpp"

#ifndef BLKSIZE
#define BLKSIZE 512
#endif

namespace atcupdateservice {
namespace bootctrl {

const char *suffix[2] = {BOOTCTRL_SUFFIX_A, BOOTCTRL_SUFFIX_B};

#ifdef CONFIG_NAND_BOOT
void dumpbcb(bootloader_message_t* bcb)
{
    ATC_STREAM_LOGI() << "checksum:" << bcb->checksum << std::endl;
    ATC_STREAM_LOGI() << "magic:" << bcb->metadata.magic << std::endl;
    ATC_STREAM_LOGI() << "priority[0]:" << static_cast<int>(bcb->metadata.slot_info[0].priority) << std::endl;
    ATC_STREAM_LOGI() << "retry_count[0]:" << static_cast<int>(bcb->metadata.slot_info[0].retry_count) << std::endl;
    ATC_STREAM_LOGI() << "successful_boot[0]:" << static_cast<int>(bcb->metadata.slot_info[0].successful_boot) << std::endl;
    ATC_STREAM_LOGI() << "normal_boot[0]:" << static_cast<int>(bcb->metadata.slot_info[0].normal_boot) << std::endl;
    ATC_STREAM_LOGI() << "priority[1]:" << static_cast<int>(bcb->metadata.slot_info[1].priority) << std::endl;
    ATC_STREAM_LOGI() << "retry_count[1]:" << static_cast<int>(bcb->metadata.slot_info[1].retry_count) << std::endl;
    ATC_STREAM_LOGI() << "successful_boot[1]:"<<static_cast<int>(bcb->metadata.slot_info[1].successful_boot) << std::endl;
    ATC_STREAM_LOGI() << "normal_boot[1]:" << static_cast<int>(bcb->metadata.slot_info[1].normal_boot) << std::endl;
}

std::string getMtdFromPart(const char* partname)
{
    std::string mtdname;

    int partIdx = utils::getPartIdxFromXml(partname);
    if (partIdx < 0) {
        ATC_STREAM_LOGE() << "can not lookup "<< partname << std::endl;
        return "";
    }
    mtdname = std::string(NAND_MTD_PATH) + std::to_string(partIdx);

    return mtdname;
}
#endif

int getSlotFromSuffix(const char *suffix) {
    int slot = -1;

    if (suffix == NULL) {
        ATC_STREAM_LOGC() << "input suffix is NULL" << std::endl;
        return -1;
    }

    if (!strcmp(suffix, BOOTCTRL_SUFFIX_A)) {
        slot = 0;
    } else if (!strcmp(suffix, BOOTCTRL_SUFFIX_B)) {
        slot = 1;
    } else {
        ATC_STREAM_LOGC() << "unknow slot suffix" << std::endl;
    }

    return slot;
}

#ifdef CONFIG_NAND_BOOT
static int readBootControllerInfo(bootloader_message_t *bctrl)
{
    int ret = -1;
    uint32_t cchksum, echksum;
    ssize_t bcb_size = 0;
    struct mtd_dev_info mtd;
    libmtd_t mtdDesc;
    ssize_t rsize;
    char* bcb_buf = NULL;
    std::string mtdname;

    if (bctrl == NULL) {
        ATC_STREAM_LOGC() << "readBootControllerInfo failed, bctrl is NULL" << std::endl;
        return -1;
    }

    bcb_size = sizeof(bootloader_message_t);

    mtdname = getMtdFromPart("datazone");
    if (mtdname.empty()) {
        ATC_STREAM_LOGW() << "getMtdFromPart failed, use /dev/mtd2"<< std::endl;
        mtdname = "/dev/mtd2";
    }

    mtdDesc = libmtd_open();
	if (mtdDesc == NULL) {
	    ATC_STREAM_LOGE() << "libmtd_open failed"<< std::endl;
	    ret = -1;
		goto out;
    }

	if (mtd_get_dev_info(mtdDesc, mtdname.c_str(), &mtd) < 0) {
	    ATC_STREAM_LOGE() << "mtd_get_dev_info failed"<< std::endl;
	    ret = -1;
		goto out;
	}

	rsize = NAND_LOAD_BCB_SIZE;
	ALIGN(rsize, mtd.min_io_size);
	//ATC_STREAM_LOGI() << "ready to malloc " <<rsize << " buff" << std::endl;

    bcb_buf = (char*)malloc(rsize);
    if (!bcb_buf) {
        ATC_STREAM_LOGE() << "malloc bcb_buf failed"<< std::endl;
        goto out;
    }

    ATC_STREAM_LOGI() << "read bcb from datazone"<<std::endl;
    ret = utils::FSUtil::nandReadRaw(&mtd, mtdname.c_str(), bcb_buf, 0, rsize);
    if (ret != rsize) {
        ATC_STREAM_LOGE() << "read datazone bcb failed"<< std::endl;
        goto out_bk;
    }
    memcpy(bctrl, bcb_buf + OFFSETOF_SLOT_SUFFIX, bcb_size);
    //dumpbcb(bctrl);

    /* cal datazone bcb checksum */
    echksum = bctrl->checksum;
    cchksum = utils::HashUtil::checkSum32(0, (const char *)bctrl + TAG_AND_CHECKSUM_SIZE, bcb_size - TAG_AND_CHECKSUM_SIZE);
    ATC_STREAM_LOGI() << "echksum : " << echksum << " cchksum : " << cchksum << std::endl;

    if (echksum != cchksum) {
        ATC_STREAM_LOGC() << "readBootControllerInfo: echksum( " << echksum << ")!="
                          << "cchksum(" << cchksum << "), checksum failed!" << std::endl;
        goto out_bk;
    } else {
        ATC_STREAM_LOGI() << "checksum pass" << std::endl;
        ret = 0;
        goto out;
    }

out_bk:
    ATC_STREAM_LOGI() << "read bcb from datazone_bk"<<std::endl;
    mtdname = getMtdFromPart("datazone_bk");
    if (mtdname.empty()) {
        ATC_STREAM_LOGW() << "getMtdFromPart failed, use /dev/mtd3"<< std::endl;
        mtdname = "/dev/mtd3";
    }
    ret = utils::FSUtil::nandReadRaw(&mtd, mtdname.c_str(), bcb_buf, 0, rsize);
    if (ret != rsize) {
        ATC_STREAM_LOGE() << "read datazone bcb failed"<< std::endl;
        goto out;
    }
    memcpy(bctrl, bcb_buf, bcb_size);

    /* cal datazone_bk bcb checksum */
    echksum = bctrl->checksum;
    cchksum = utils::HashUtil::checkSum32(0, (const char *)bctrl + TAG_AND_CHECKSUM_SIZE, bcb_size - TAG_AND_CHECKSUM_SIZE);
    ATC_STREAM_LOGI() << "bk echksum(" << echksum << "),cchksum("<< cchksum << ")" << std::endl;
    if (echksum != cchksum) {
        ATC_STREAM_LOGI() << "bk echksum(" << echksum << ") != cchksum("<< cchksum << "), checksum failed!" << std::endl;
        goto out;
    } else {
        ATC_STREAM_LOGI()<< "readBootControllerInfo : checksum for bk pass" << std::endl;
        ret = 0;
    }

out:
    if (mtdDesc)
        libmtd_close(mtdDesc);

    if (bcb_buf)
        free(bcb_buf);

    return ret;
}

static int writeBootControllerInfo(bootloader_message_t *bctrl)
{
    int ret = -1;
    struct mtd_dev_info mtd;
    libmtd_t mtdDesc = NULL;
    std::string datazone, datazone_bk;
    ssize_t bcb_size = 0;
    char *buf = NULL;
    ssize_t wsize = 0;

    if (bctrl == NULL) {
        ATC_STREAM_LOGC() << "write_bootcontroller_info failed, bctrl is NULL" << std::endl;
        return -1;
    }

    mtdDesc = libmtd_open();
	if (mtdDesc == NULL) {
	    ATC_STREAM_LOGE() << "libmtd_open failed"<< std::endl;
	    ret = -1;
		goto out;
    }

    datazone = getMtdFromPart("datazone");
    if (datazone.empty()) {
        ATC_STREAM_LOGW() << "getMtdFromPart failed, use /dev/mtd2"<< std::endl;
        datazone = "/dev/mtd2";
    }

    datazone_bk = getMtdFromPart("datazone_bk");
    if (datazone_bk.empty()) {
        ATC_STREAM_LOGW() << "getMtdFromPart failed, use /dev/mtd3"<< std::endl;
        datazone_bk = "/dev/mtd3";
    }

	if (mtd_get_dev_info(mtdDesc, datazone.c_str(), &mtd) < 0) {
	    ATC_STREAM_LOGE() << "mtd_get_dev_info failed"<< std::endl;
	    ret = -1;
		goto out;
	}

    bcb_size = sizeof(bootloader_message_t);
	wsize = NAND_LOAD_PTBL_SIZE;
	ALIGN(wsize, mtd.min_io_size);

    buf = (char*)malloc(wsize);
    if (!buf) {
        ATC_STREAM_LOGE() << "malloc buf for wsize failed"<< std::endl;
        ret = -1;
        goto out;
    }

    ATC_STREAM_LOGI() << "read datazone header[4K] + bcb[4K] + partinfo[4K]"<<std::endl;
    ret = utils::FSUtil::nandReadRaw(&mtd, datazone.c_str(), buf, 0, wsize);
    if (ret != wsize) {
        ATC_STREAM_LOGE() << "read datazone bcb failed"<< std::endl;
        ret = -1;
        goto out;
    }

    ATC_STREAM_LOGI() << "write datazone header[4K] + bcb[4K] + partinfo[4K]"<<std::endl;
    bctrl->checksum = utils::HashUtil::checkSum32(0, (char *)bctrl + TAG_AND_CHECKSUM_SIZE, bcb_size - TAG_AND_CHECKSUM_SIZE);
    memcpy(buf + OFFSETOF_SLOT_SUFFIX, bctrl, bcb_size);

    /* write 12K data to datazone */
    ret = utils::FSUtil::nandWriteRaw(mtdDesc, &mtd, datazone.c_str(), buf, 0, wsize);
    if (ret != wsize) {
        ATC_STREAM_LOGE() << "nandWriteRaw update datazone failed"<< std::endl;
        ret = -1;
        goto out;
    }

    /* write 12K data to datazone_bk */
    ret = utils::FSUtil::nandWriteRaw(mtdDesc, &mtd, datazone_bk.c_str(), buf, 0, wsize);
    if (ret != wsize) {
        ATC_STREAM_LOGE() << "nandWriteRaw update datazone_bk failed"<< std::endl;
        ret = -1;
        goto out;
    }

    ret = 0;
out:
    if (mtdDesc)
        libmtd_close(mtdDesc);
    if (buf)
        free(buf);

    return ret;
}

#else
static int readBootControllerInfo(bootloader_message_t *bctrl)
{
    int ret = -1;
    bootloader_message_t bcb;
    uint32_t cchksum, echksum;
    bool need_bk = false;
    ssize_t bcb_size = 0;

    if (bctrl == NULL) {
        ATC_STREAM_LOGC() << "readBootControllerInfo failed, bctrl is NULL" << std::endl;
        return -1;
    }

    bcb_size = sizeof(bootloader_message_t);
    memset(&bcb, 0, bcb_size);

    ret = utils::FSUtil::readFix(BOOTCTRL_DEV, (char *)&bcb, OFFSETOF_SLOT_SUFFIX, bcb_size);
    if (ret != bcb_size) {
        ATC_STREAM_LOGC() << "read bcb failed, ret: " << ret << ", expect : " << bcb_size
                          << " error : " << strerror(errno) << std::endl;
        need_bk = true;;
    } else {
        memcpy(bctrl, &bcb, bcb_size);
        echksum = bctrl->checksum;

        cchksum = utils::HashUtil::checkSum32(0, (const char *)bctrl + TAG_AND_CHECKSUM_SIZE, bcb_size - TAG_AND_CHECKSUM_SIZE);
        ATC_STREAM_LOGI() << "echksum : " << echksum << "cchksum : " << cchksum << std::endl;

        if (echksum != cchksum) {
            ATC_STREAM_LOGC() << "readBootControllerInfo: echksum( " << echksum << ")!="
                              << "cchksum(" << cchksum << "), checksum failed!" << std::endl;
            need_bk = 1;
        } else {
            ATC_STREAM_LOGI() << "ATC_STREAM_LOGC: checksum pass" << std::endl;
        }
    }

    if (need_bk) {
        memset(&bcb, 0, bcb_size);
        ret = utils::FSUtil::readFix(BOOTCTRLBK_DEV, (char *)&bcb, OFFSETOF_SLOT_SUFFIX, bcb_size);
        if (ret != bcb_size) {
            ATC_STREAM_LOGE() << "read bcb_bk failed, ret: " << ret
                              << " error : " << strerror(errno) << std::endl;
            return ret;
        } else {
            memcpy(bctrl, &bcb, bcb_size);
            echksum = bctrl->checksum;
            cchksum = utils::HashUtil::checkSum32(0, (const char *)bctrl + TAG_AND_CHECKSUM_SIZE, bcb_size - TAG_AND_CHECKSUM_SIZE);
            ATC_STREAM_LOGI() << "bk echksum(" << echksum << "),cchksum("<< cchksum << ")" << std::endl;
            if (echksum != cchksum) {
                ATC_STREAM_LOGI() << "bk echksum(" << echksum << ") != cchksum("<< cchksum << "), checksum failed!" << std::endl;
                return -1;
            } else {
                ATC_STREAM_LOGI()<< "readBootControllerInfo : checksum for bk pass" << std::endl;
            }
        }
    }

    return 0;
}

static int writeBootControllerInfo(bootloader_message_t *bctrl) {
    int ret = -1;
    bootloader_message_t bcb;
    ssize_t bcb_size = 0;

    if (bctrl == NULL) {
        ATC_STREAM_LOGC() << "read_write_bootcontroller_info failed, bctrl is NULL" << std::endl;
        return -1;
    }

    bcb_size = sizeof(bootloader_message_t);
    bctrl->checksum = utils::HashUtil::checkSum32(0, (char *)bctrl + TAG_AND_CHECKSUM_SIZE, bcb_size - TAG_AND_CHECKSUM_SIZE);
    memset(&bcb,0,512);
    memcpy(&bcb, bctrl, bcb_size);

    ret = utils::FSUtil::writeFix(BOOTCTRL_DEV, (const char*)&bcb, OFFSETOF_SLOT_SUFFIX, bcb_size);
    if (ret != bcb_size) {
        ATC_STREAM_LOGC() << "writeBootControllerInfo bcb failed, ret : "<< ret
                          << " error : "<< strerror(errno) << std::endl;
        return -1;
    }
    ATC_STREAM_LOGI() << "write bootctrl success" << std::endl;

    ret = utils::FSUtil::writeFix(BOOTCTRLBK_DEV, (const char*)&bcb, OFFSETOF_SLOT_SUFFIX, bcb_size);
    if (ret != bcb_size) {
         ATC_STREAM_LOGC() << "writeBootControllerInfo bcb_bk failed, ret : "<< ret
                           << " error : "<< strerror(errno) << std::endl;
        return -1;
    }
    ATC_STREAM_LOGI() <<  "write bootctrl bk success" << std::endl;
    return 0;
}
#endif

std::string getCurrentSuffix(void)
{
    std::string cmdline;
    char buf[BLKSIZE] = {0};
    int fd = open(CMDLINE_DEV, O_RDONLY);
    int rt = 0;
    std::stringstream ss;
    std::string res = "";
    std::string item = "";
    size_t targetLen = strlen(CMDLINE_TARGET_PREFIX);
    if (fd < 0) {
        syslog(LOG_ERR, "failed to open dev : %s, error : %s\r\n", CMDLINE_DEV, strerror(errno));
        return res;
    }

    while ((rt = read(fd, buf, BLKSIZE - 1)) > 0) {
        buf[rt] = '\0';
        cmdline += std::string(buf);
    }
    close(fd);
    if (rt < 0) {
        syslog(LOG_ERR, "failed to open dev : %s, error : %s\r\n", CMDLINE_DEV, strerror(errno));
        return res;
    }
    ss.str(cmdline);
    while (ss >> item) {
        if (strncmp(item.c_str(), CMDLINE_TARGET_PREFIX, targetLen) == 0) {
            res = item.substr(targetLen);
            if (res == "_a" || res == "_b"){
                return res;
            } else {
                syslog(LOG_ERR, "suffix(%s) not valid\r\n", res.c_str());
                return "";
            }
        }
    }
    syslog(LOG_INFO, "not valid suffix\r\n");

    return "";
}

int getSlotInfo(const char *suffix, slot_metadata_t *slot_info)
{
    int slot = 0, ret = -1;
    slot_metadata_t *slotp;
    bootloader_message_t bcb;

    slot = getSlotFromSuffix(suffix);
    if (slot == -1) {
        ATC_STREAM_LOGE() << "set_not_normal_boot failed, slot: " << slot << std::endl;
        return -1;
    }

    ret = readBootControllerInfo(&bcb);
    if (ret < 0) {
        ATC_STREAM_LOGE() <<"partition_read failed, ret: " << ret << std::endl;
        return -1;
    }

    slotp = &bcb.metadata.slot_info[slot];
    memcpy(slot_info, slotp, sizeof(slot_metadata_t));

    return 0;
}

int setSlotInfo(const char *suffix, slot_metadata_t *slot_info)
{
    int slot = 0, ret = -1;
    slot_metadata_t *slotp;
    bootloader_message_t bcb;

    slot = getSlotFromSuffix(suffix);
    if (slot == -1) {
        ATC_STREAM_LOGE() << "getSlotFromSuffix failed, slot: " << slot << std::endl;
        return -1;
    }

    ret = readBootControllerInfo(&bcb);
    if (ret < 0) {
        ATC_STREAM_LOGE() <<"partition_read failed, ret: " << ret << std::endl;
        return -1;
    }

    slotp = &bcb.metadata.slot_info[slot];
    memcpy(slotp,slot_info,sizeof(slot_metadata_t));

    ret = writeBootControllerInfo(&bcb);
    if (ret < 0) {
        ATC_STREAM_LOGE() << "partition_write failed, ret: " << ret << std::endl;
        return -1;
    }

    return 0;
}

std::string getSuffix(int slot) {
    if (slot == 0) {
        return "_a";
    } else if (slot == 1) {
        return "_b";
    } else {
        return "";
    }
}

int getCurrentSlot() {
    std::string suffix = getCurrentSuffix();
    if (suffix == "_a") {
        return 0;
    } else if (suffix == "_b") {
        return 1;
    }
    return -1;
}

int checkCurrentSlotValid(void)
{
    bootloader_message_t bcb;

    int slot = getCurrentSlot();

    if ((BOOTCTRL_SLOT_A != slot) && (BOOTCTRL_SLOT_B != slot)) {
        ATC_STREAM_LOGI() << "check_slot_vaild slot is invalid.\n" << std::endl;
        return -1;
    }

    ATC_STREAM_LOGI() << "check_slot_vaild slot: " << slot << std::endl;

    if (readBootControllerInfo(&bcb)) {
        ATC_STREAM_LOGE() << "check_slot_valid partition_read failed" << std::endl;
        return -1;
    }

    if (bcb.metadata.slot_info[slot].priority > 0) {
        return 0;
    } else {
        return -1;
    }
}

int setActiveSlot(int slot) {
    bootloader_message_t bcb;
    int rt = 0;
    slot_metadata_t *pslot;

    if (slot >= 2 || slot < 0) {
        ATC_STREAM_LOGE() << "invalid slot : " << slot << std::endl;
        return -1;
    }
    rt = readBootControllerInfo(&bcb);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "failed to read bootctrl" << std::endl;
        return -1;
    }
    pslot = &(bcb.metadata.slot_info[slot]);
    pslot->successful_boot = 0;
    pslot->priority = 7;
    pslot->normal_boot = 1;
    pslot->retry_count = 3;

    pslot = &(bcb.metadata.slot_info[1 - slot]);
    pslot->priority = 6;
    pslot->retry_count = 3;
    bcb.checksum = utils::HashUtil::checkSum32(0, (char*)(&bcb) + TAG_AND_CHECKSUM_SIZE, sizeof(bootloader_message_t) - TAG_AND_CHECKSUM_SIZE);

    rt = writeBootControllerInfo(&bcb);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "failed to write bcb" << std::endl;
        return -1;
    }

    return 0;
}

int setSlotUnbootable(int slot) {
    int ret;
    bootloader_message_t bcb;
    slot_metadata_t *pslot;

    if (slot >= 2) {
        ATC_STREAM_LOGE() << "failed to read bootctrl" << std::endl;
        return -1;
    }
    ret = readBootControllerInfo(&bcb);
    if (ret < 0) {
        return ret;
    }
    /* Set zero to priority ,successful_boot and retry_count */
    pslot = &bcb.metadata.slot_info[slot];
    pslot->successful_boot = 0;
    pslot->priority = 0;
    pslot->retry_count = 0;
    bcb.checksum  = utils::HashUtil::checkSum32(0, (char *)(&bcb) + TAG_AND_CHECKSUM_SIZE, sizeof(bootloader_message_t) - TAG_AND_CHECKSUM_SIZE);
    ret = writeBootControllerInfo(&bcb);
    if (ret < 0) {
        ATC_STREAM_LOGE() << "failed to write bcb" << std::endl;
        return ret;
    }

    return 0;
}

int getSlotBootable(unsigned slot) {
    int ret;
    bootloader_message_t bcb;

    /* slot 0 is A , slot 1 is B */
    if (slot >= 2) {
        ATC_STREAM_LOGE() << "failed to read bootctrl" << std::endl;
        return -1;
    }
    ret = readBootControllerInfo(&bcb);
    if (ret < 0) {
        return ret;
    }
    return (bcb.metadata.slot_info[slot].priority != 0);
}

int markBootSuccessful() {
    int ret;
    int slot = 0;
    bootloader_message_t bcb;
    slot_metadata_t *pslot;
    
    ret = readBootControllerInfo(&bcb);
    if (ret < 0) {
        return ret;
    }

    slot = getCurrentSlot();
    if (slot < 0) {
        ATC_STREAM_LOGE() << "bootctrl_mark_boot_successful fail , slot = " << slot << std::endl;
        return slot;
    }

    pslot = &bcb.metadata.slot_info[slot];
    if (pslot->successful_boot == 1 && pslot->retry_count == 3) {
        return 0;
    }
    pslot->successful_boot = 1;
    pslot->retry_count = 3;
    bcb.checksum  = utils::HashUtil::checkSum32(0, (char *)(&bcb) + TAG_AND_CHECKSUM_SIZE, sizeof(bootloader_message_t) - TAG_AND_CHECKSUM_SIZE);

    return writeBootControllerInfo(&bcb);
}

int getInactiveSlot(slot_metadata_t *slotp) {
    int slot = getCurrentSlot();
    if (slot < 0)
        return -1;
    else if (slot == 0)
        return getSlotInfo("_b", slotp);
    else
        return getSlotInfo("_a", slotp);
}

}
}
