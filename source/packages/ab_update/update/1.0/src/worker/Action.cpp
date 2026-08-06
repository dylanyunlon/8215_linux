/*
copyright (c) 2020 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <unistd.h>
#include <sys/reboot.h>
#include <fstream>
#include <errno.h>
#include <cstring>

#include "worker/Action.hpp"
#include <functional>
#include "worker/Worker.hpp"
#include "updater/Updater.hpp"
#include "bootctrl/BootCtrl.hpp"
#include "gpt/GPT.hpp"
#include "utils/Image.hpp"
#include "utils/Version.hpp"
#include "utils/Util.hpp"
#include "utils/Bspatch.hpp"
#ifdef BOARD_AVB_ENABLE
#include <metazone.h>
#include <metazone_index.h>
#endif

namespace atcupdateservice {
namespace utils {
void removeUpdateConfigOnFailure();
}
}

//update failure
#define    FAILURE          0
//continue update
#define    CONTINUE         1
//update finished
#define    DONE             2
//external device umouted
#define    UNPLUGED         3

#define    ONE_HUNDRUD      100

#ifdef BOARD_AVB_ENABLE
#define LAST_AB_UPGRADE_INDEX       METAZONE_AVB_AB_UPGRADE_STATUS
#define MARK_AB_UPGRADE_FINISHED    (1)
#endif

namespace atcupdateservice {
namespace worker {

void Action::sendMessage(UpdateMessageType type, const std::string &msg) {
    auto updater = updater::Updater::Upd::getInstance();
    updater -> sendMessage(type, msg);
}

void Action::setProgress(uint32_t progress) {
    auto updater = updater::Updater::Upd::getInstance();
    updater -> setProgress(progress);
}

void Action::setAndSendProgress(uint32_t progress) {
    auto updater = updater::Updater::Upd::getInstance();
    updater -> setAndSendProgress(progress);
}

static void checkRebootAfterUpdate(void) {
    if (utils::getUpdateConfigFlag("reboot_after_update")) {
        ATC_STREAM_LOGI() << "reboot after update" << std::endl;
        sync();
        sleep(1);
        if (reboot(RB_AUTOBOOT) != 0) {
            ATC_STREAM_LOGE() << "reboot failed" << std::endl;
        }
    }
}

void dumpPart(utils::PartInfo::ptr part) {
    ATC_STREAM_LOGD() << "image: " << part->image << " part: " << part->part
                      << " partsize: " << part->partsize  << " fileLen: " << part->fileLen
                      << " blockSize: " << part->blockSize
                      << " raw: " << part->raw << " chkCount : " << part->chkCount
                      << std::endl;
}

void dumpImages(const std::vector<std::string> &images) {
    for(auto &item : images) {
        ATC_STREAM_LOGI() << "image in ios, name : " << item << std::endl;
    }
}

int Parser::findImage(const std::string &img) {
    for (size_t i = 0; i < m_parts.size(); ++i) {
        if (m_parts[i]->image == img) {
            return i;
        }
    }
    return -1;
}

bool Parser::doCheck(utils::CheckPoint &cp) {
    ATC_STREAM_LOGI() << " now do check " << std::endl;
    std::string scatterFile = std::string(ATC_ISO_MOUNTPOINT) + std::string(SCATTER_FILE_NAME);
    std::vector<std::string>::iterator pos;

    m_payLoad = 0;
    m_totLoad = 0;
    m_curIdx = -1;

    memset(&cp, 0, sizeof(cp));

    if (m_parts.size() == 0) {
        ATC_STREAM_LOGE() << "no update set" << std::endl;
        return false;
    }
    for (auto &item : m_parts) {
        m_totLoad += item->partsize;
        dumpPart(item);
    }
    ATC_STREAM_LOGI() << "tot size : " << m_totLoad << std::endl;
    m_curIdx = 0;
    if(utils::FSUtil::parseCheckPoint(cp) == false) {
        ATC_STREAM_LOGE() << "parse checkpoint failure!" << std::endl;
        m_curIdx = 0;
        return false;
    }
    if (std::string(cp.md5) != m_md5Sum) {
        ATC_STREAM_LOGE() << "md5 fail! current : " << m_md5Sum
                          << " last check sum" << std::string(cp.md5) << std::endl;
        m_curIdx = 0;
        return false;
    }
    m_curIdx = findImage(std::string(cp.imagename));
    if (m_curIdx < 0) {
        ATC_STREAM_LOGE() << "no such image : " << std::string(cp.imagename) << std::endl;
        m_curIdx = 0;
        return false;
    }
    ATC_STREAM_LOGI() << "found last idx : " << m_curIdx << " last read offset : "
                      << cp.offsetRead << " last write offset : " << cp.offsetWrite << std::endl;
    for (int i = 0; i< (int)m_parts.size(); i++) {
        if (i < m_curIdx) {
            m_payLoad += m_parts[i]->partsize;
        }
    }

    return true;
}

//thread safety is NOT guranteed, please not changed the loop queue to multi thread
bool Parser::run() {
    Parser::ptr self = shared_from_this();
    if (m_worker == nullptr) {
        sendMessage(UpdateMessageType::ERROR, "worker is nullptr!");
        ATC_STREAM_LOGE() << "worker is nullptr!" << std::endl;
        return false;
    }

    if(m_firstTime) {
        bool rt = false;
        m_worker->setStatus(IWorker::CHECKING);
        m_firstTime = false;
        sendMessage(UpdateMessageType::CHECK, "now checking update package");
        utils::CheckPoint cp;
        rt = doCheck(cp);
        if (m_curIdx >= 0 && m_parts.size() > 0) {
            std::string imagename = m_parts[m_curIdx]->image;
            std::string partname = m_parts[m_curIdx]->part;
            bool raw = m_parts[m_curIdx]->raw;

            ATC_STREAM_LOGI() << "partname:" <<partname<<" imagename:"<<imagename<<" m_curIdx:"<<m_curIdx<<std::endl;
            #ifdef CONFIG_NAND_BOOT
            if (partname == "system_a" || partname == "system_b") {
                raw = 1;
                ATC_STREAM_LOGI() << "system part handle as raw part" << std::endl;
            }
            #endif
            m_image = utils::Image::createImage(partname, imagename, raw);
            if (rt == false) {
                if (m_image->open(0, 0) == false) {
                    throw std::logic_error("open image partname: " + partname);
                }
                ATC_STREAM_LOGI() << "no valid check point, ignored!" << std::endl;
            } else {
                m_image->open(cp.offsetWrite, cp.offsetRead);
                ATC_STREAM_LOGI() << "nextImage : " << imagename << std::endl;
            }
            if(utils::FSUtil::setUpdateSlotUnbootable() == false) {
                throw std::logic_error("set slot : " + utils::FSUtil::updateSlot() + " unbootable failed");
            }
            sendMessage(UpdateMessageType::START, "check finished! start to update...!");
            #ifdef CONFIG_EMMC_BOOT
            if(FSUtil::initCheckPoint() == false) {
                   ATC_STREAM_LOGW() << "failed to init checkpoint" << std::endl;
            }
            #endif
            m_worker->addAction(self);
            return true;
        } else {
            throw std::logic_error("load image failed");
        }
    } else {
        bool loadNext = false;

        m_worker->setStatus(IWorker::WORKING);

        memset(m_buf, 0, 0x20000); //128K
        if (m_image->writePartition(m_buf) == false) {
            throw std::logic_error("write partition: " + m_image->getPartName() + " failed! imagename: "
                        + m_image->getImageName() + " writeOffset :" + std::to_string(m_image->getPartOffset())
                        + ", readOffset: " + std::to_string(m_image->getImageOffset()));
        }

        setAndSendProgress((((double)(m_payLoad + m_image->getPartOffset())) / m_totLoad) * ONE_HUNDRUD);

        if (m_image->finished()) {
            sync();
            utils::dropSystemCaches();
            if (utils::verifyUpdatedPartition(std::string(ATC_ISO_MOUNTPOINT), m_parts[m_curIdx]) == false) {
                throw std::logic_error("verify updated partition failed: part="
                    + m_parts[m_curIdx]->part + ", image=" + m_parts[m_curIdx]->image);
            }
            loadNext = true;
            ATC_STREAM_LOGI() << "Partition " <<m_parts[m_curIdx]->part<< " update ok" << std::endl;
        }
        #ifdef CONFIG_EMMC_BOOT
        m_image->writeCheckPoint(m_md5Sum);
        #endif
        if (loadNext) {
            m_payLoad += m_image->getPartSize();
            if ((size_t)++m_curIdx >= m_parts.size()) {
                ATC_STREAM_LOGI() << "Update Finished!" << std::endl;
#ifdef CONFIG_EMMC_BOOT
                bool rt = gpt::GPT::writeEmmc(m_parts);
                if (rt == false) {
                    throw std::logic_error("failed to write gpt on emmc");
                }
#else
                bool rt = gpt::GPT::UpdateGPT(m_parts);
                if (rt == false) {
                    throw std::logic_error("failed to write gpt on emmc");
                }
#endif
                OnFinished::ptr onFinished = std::make_shared<OnFinished>("update finished!", m_worker);
                m_worker->addAction(onFinished, "OnFinished");
            } else {
                std::string partname = m_parts[m_curIdx]->part;
                std::string imagename = m_parts[m_curIdx]->image;
                bool raw = m_parts[m_curIdx]->raw;
                #ifdef CONFIG_NAND_BOOT
                if (partname == "system_a" || partname == "system_b") {
                    raw = 1;
                    ATC_STREAM_LOGI() << "system part handle as raw part" << std::endl;
                }
                #endif
                m_image = utils::Image::createImage(partname, imagename, raw);
                if (m_image == nullptr) {
                    throw std::logic_error("failed to create image : " + partname);
                }
                if (m_image->open(0, 0) == false) {
                    throw std::logic_error("failed to open: " + partname);
                }
                m_worker->addAction(self);
                ATC_STREAM_LOGI() << "nextImage : " << m_parts[m_curIdx]->image << std::endl;
                dumpPart(m_parts[m_curIdx]);
            }
        } else {
            m_worker->addAction(self);
        }
        return true;
    }
}

//thread safety is NOT guranteed, please not changed the loop queue to multi thread
bool ParserDiff::run() {
    std::string versionFile = std::string(ATC_ISO_MOUNTPOINT) + "version";
    std::string sizeFile = std::string(ATC_ISO_MOUNTPOINT) + "updatesize";
    std::string imagename;
    std::string partname;
    std::string curslot_partname;
    std::string partPath;
    bool rt = false;
    Parser::ptr self = shared_from_this();
    if (m_worker == nullptr) {
        sendMessage(UpdateMessageType::ERROR, "worker is nullptr!");
        ATC_STREAM_LOGE() << "worker is nullptr!" << std::endl;
        return false;
    }

    if(m_firstTime) {

        m_worker->setStatus(IWorker::CHECKING);
        m_firstTime = false;
        sendMessage(UpdateMessageType::CHECK, "now checking update package");

        if (utils::verifyBasePartition(std::string(ATC_ISO_MOUNTPOINT), m_parts) == false) {
            throw std::logic_error("verify parition And basepackage failed!");
        }
        ATC_STREAM_LOGI() << "verify parition And basepackage pass!" << std::endl;

        utils::CheckPoint cp;
        rt = doCheck(cp);
        if (m_curIdx >= 0 && m_parts.size() > 0) {
            imagename = m_parts[m_curIdx]->image;
            partname = m_parts[m_curIdx]->part;
            partPath = std::string(BLOCK_DEV_PATH) + partname;
            bool raw = m_parts[m_curIdx]->raw;

            m_image = utils::Image::createImage(partname, imagename, raw);
            if (rt == false) {
                if (m_image->open(0, 0) == false) {
                    throw std::logic_error("open image partname: " + partname);
                }
                ATC_STREAM_LOGI() << "no valid check point, ignored!" << std::endl;
            } else {
                m_image->open(cp.offsetWrite, cp.offsetRead);
                ATC_STREAM_LOGI() << "nextImage : " << imagename << std::endl;
            }

            if(utils::FSUtil::setUpdateSlotUnbootable() == false) {
                throw std::logic_error("set slot : " + utils::FSUtil::updateSlot() + " unbootable failed");
            }
            sendMessage(UpdateMessageType::START, "check finished! start to update...!");
            #ifdef CONFIG_EMMC_BOOT
            if(FSUtil::initCheckPoint() == false) {
                   ATC_STREAM_LOGW() << "failed to init checkpoint" << std::endl;
            }
            #endif
            m_worker->addAction(self);
            return true;
        } else {
            throw std::logic_error("load image failed");
        }
    } else {
        partname = m_parts[m_curIdx]->part;
        curslot_partname = utils::FSUtil::getCurentSlotPart(partname);
        partPath = std::string(BLOCK_DEV_PATH) + curslot_partname;
#ifdef CONFIG_EMMC_BOOT
        m_part  = File::Create(partPath,  File::READ);
#else
        m_part  = NandFile::Create(partPath,  File::READ);
#endif
        m_worker->setStatus(IWorker::WORKING);
        int64_t m_partSize = m_part->size();

        /*get img data*/
        imagename = m_parts[m_curIdx]->image;
        std::string imagePath = std::string(ATC_ISO_MOUNTPOINT) + imagename;
        BspatchStream::ptr patchStream = std::make_shared<Bz2BspatchStream>(imagePath.c_str());
        Bspatch::ptr  patcher = Bspatch::create();

        /*merge diff OTA data*/
        ATC_STREAM_LOGI() << "start to merge data" << std::endl;
#ifdef CONFIG_NAND_BOOT
        File::ptr sourceFile = NandFile::Create(partPath,  File::READ);

        patcher->setSource(sourceFile);
        partname = m_parts[m_curIdx]->part;
        partPath = std::string(BLOCK_DEV_PATH) + partname;

        File::ptr destFile = NandFile::Create(partPath,  File::WRITE);
        patcher->setDestiny(destFile);
        patcher->setPatch(patchStream);
        patcher->configBufferLimit(128 *1024);
#else
        ATC_STREAM_LOGI() << "ota diff not support in emmc device" << std::endl;
#endif
        int ret = patcher->patch();
        if (ret !=0 ) {
            ATC_STREAM_LOGE() << "bspatch failed" << std::endl;
            return -1;
        }

        std::string size = utils::FSUtil::getPackageDiffInfo(sizeFile, m_parts[m_curIdx]->image);
        m_parts[m_curIdx]->fileLen = std::atoi(size.c_str());

        setAndSendProgress((((double)(m_payLoad + m_partSize)) / m_totLoad) * ONE_HUNDRUD);
        m_payLoad += m_partSize;
        if ((size_t)++m_curIdx >= m_parts.size()) {
            ATC_STREAM_LOGI() << "Update Finished!" << std::endl;
    #ifdef CONFIG_EMMC_BOOT
            bool rt = gpt::GPT::writeEmmc(m_parts);
            if (rt == false) {
                throw std::logic_error("failed to write gpt on emmc");
            }
    #endif
            utils::writeVersion(VERSION_FILE, utils::readDiffVersion(versionFile, VERSION_POST));
            OnFinished::ptr onFinished = std::make_shared<OnFinished>("update finished!", m_worker);
            m_worker->addAction(onFinished, "OnFinished");
        } else {
            std::string partname = m_parts[m_curIdx]->part;
            std::string imagename = m_parts[m_curIdx]->image;
            bool raw = m_parts[m_curIdx]->raw;
            #ifdef CONFIG_NAND_BOOT
            if (partname == "system_a" || partname == "system_b") {
                raw = 1;
                ATC_STREAM_LOGI() << "system part handle as raw part" << std::endl;
            }
            #endif
            m_image = utils::Image::createImage(partname, imagename, raw);
            if (m_image == nullptr) {
                throw std::logic_error("failed to create image : " + partname);
            }
            if (m_image->open(0, 0) == false) {
                throw std::logic_error("failed to open: " + partname);
            }
            m_worker->addAction(self);
            ATC_STREAM_LOGI() << "nextImage : " << m_parts[m_curIdx]->image << std::endl;
            dumpPart(m_parts[m_curIdx]);
        }

        return true;
    }
}



bool OnError::run() {
    bool rt = false;
    if (m_worker == nullptr) {
        sendMessage(UpdateMessageType::ERROR, "worker is nullptr!");
        ATC_STREAM_LOGE() << "worker is nullptr!" << std::endl;
        return false;
    }

    m_worker->setStatus(IWorker::ERROR);
    ATC_STREAM_LOGE() << "system updater fail, reason: " << m_reason << std::endl;
    rt = utils::FSUtil::ISOUmount(ATC_ISO_MOUNTPOINT);
    if (rt == false) {
        sendMessage(UpdateMessageType::ERROR, m_reason);
        ATC_STREAM_LOGE() << "failed to umount : " << ATC_ISO_MOUNTPOINT << std::endl;
        return false;
    }
    //failure not because of umount, send error msg and remove /data/misc/checkpoint
    utils::removeUpdateConfigOnFailure();
    FSUtil::clearCheckPoint();
    FSUtil::deInitCheckPoint();
    FSUtil::removeCheckPoint();
    sync();
    ATC_STREAM_LOGI() << "sync..." << std::endl;
    sleep(1);
    sendMessage(UpdateMessageType::ERROR, m_reason);
    if (utils::FSUtil::enableWriteProtect() == false) {
        ATC_STREAM_LOGE() << "failed to enable write protect, error" << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

bool OnFinished::run() {
    bool rt = false;
    std::string whatErr;
    if (m_worker == nullptr) {
        sendMessage(UpdateMessageType::ERROR, "worker is nullptr!");
        ATC_STREAM_LOGE() << "worker is nullptr!" << std::endl;
        return false;
    }
    m_worker->setResourceStatus(false);
    m_worker->setPathAndStatus("", IWorker::FINISHED);
    FSUtil::clearCheckPoint();
    FSUtil::deInitCheckPoint();
    FSUtil::removeCheckPoint();

    m_worker->setStatus(IWorker::FINISHED);
    setProgress(0);     //for next update
    utils::writeVersion(VERSION_OLD, utils::readVersion());
    rt = utils::FSUtil::ISOUmount(ATC_ISO_MOUNTPOINT);
    if (rt == false) {
        throw std::logic_error("failed to enable write protect");
    }
    sleep(3);
    if (utils::FSUtil::setUpdateSlotActive() == false) {
        whatErr = std::string("failed to set slot : ")
                  + utils::FSUtil::updateSlot()
                  + std::string(" to active!");
        throw std::logic_error(whatErr);
    }
    if (utils::FSUtil::enableWriteProtect() == false) {
        throw std::logic_error("failed to enable write protect");
    }

    setAndSendProgress(100);
#ifdef BOARD_AVB_ENABLE
    ATC_STREAM_LOGI() << "AVB Enable! write ab upgrade finished on metazone" << std::endl;
    metazone_init();
    if (metazone_write(LAST_AB_UPGRADE_INDEX, MARK_AB_UPGRADE_FINISHED) != 0) {
        ATC_STREAM_LOGE() << "failed to write metazone!" << std::endl;
        metazone_deinit();
        return false;
    }
    metazone_flush(1);
    metazone_deinit();
#endif
    sync();
    ATC_STREAM_LOGI() << "sync..." << std::endl;
    sleep(3);
    sendMessage(UpdateMessageType::FINISHED, m_msg);

    checkRebootAfterUpdate();

    return true;
}

bool OnStart::run() {
    Parser::ptr parser = nullptr;
    Parser::ptr parserdiff = nullptr;
    std::string md5;
    std::string versionFile = std::string(ATC_ISO_MOUNTPOINT) + "version";
    std::string scatterFile = std::string(ATC_ISO_MOUNTPOINT) + SCATTER_FILE_NAME;
    std::vector<utils::PartInfo::ptr> parts;
    char *m_buf;
    int diff_flag = 0;
    if (m_worker == nullptr) {
        sendMessage(UpdateMessageType::ERROR, "worker is nullptr!");
        ATC_STREAM_LOGE() << "worker is nullptr!" << std::endl;
        return false;
    }

    sendMessage(UpdateMessageType::CHECK, "now checking update package");
    m_worker->setPathAndStatus(m_isoPath, IWorker::CHECKING);

    if (utils::FSUtil::ISOMount(m_isoPath, ATC_ISO_MOUNTPOINT) == false) {
        throw std::logic_error("failed to mount iso package...");
    }

    if(utils::FSUtil::getPackageDiffInfo(versionFile, "package") == "diff"){
        ATC_STREAM_LOGI() << "upgrade for diff!" << std::endl;
        diff_flag = 1;
    }

    if (utils::getUpdateParts(scatterFile, parts) == false) {
        throw std::logic_error("failed to get updatable image...");
    }
    if (parts.empty()) {
        throw std::logic_error("no updatable image!");
    }

#ifdef STRESS_ZIP_READ
    {
        int round = 0;
        int totalFail = 0;
        while (true) {
            round++;
            utils::dropSystemCaches();
            if (utils::verifyUpdateImages(ATC_ISO_MOUNTPOINT, parts, diff_flag) == false) {
                totalFail++;
                ATC_STREAM_LOGE() << "STRESS round=" << round << " FAIL, totalFail=" << totalFail << std::endl;
                throw std::logic_error("verify iso package failed!");
            } else {
                ATC_STREAM_LOGI() << "STRESS round=" << round << " PASS, totalFail=" << totalFail << std::endl;
            }
        }
    }
#else
    if (utils::verifyUpdateImages(ATC_ISO_MOUNTPOINT, parts, diff_flag) == false) {
        throw std::logic_error("verify iso package failed!");
    }

    ATC_STREAM_LOGI() << "verify iso package pass!" << std::endl;
#endif

    if (utils::FSUtil::disableWriteProtect() == false) {
        throw std::logic_error("failed to disable write protect!");
    }
    m_buf = (char *)malloc(0x20000); //128K
    ATC_STREAM_LOGE() << "start to malloc in OnStart::run" << std::endl;
    if (m_buf == nullptr) {
        ATC_STREAM_LOGE() << "Failed to alloc memory OnStart::run" << std::endl;
        throw std::logic_error("out of memory");
    }
    m_worker->setResourceStatus(true);
    if(diff_flag) {
       ATC_STREAM_LOGI() << "enter diff" << std::endl;
       if (checkdiffPackageVersion(versionFile) == false) {
            throw std::logic_error("invalid version: " + readVersion(versionFile));
       }
        parserdiff.reset(new ParserDiff(m_isoPath, m_worker));
        parserdiff->setParts(parts);
        parserdiff->setBuffer(m_buf);
        m_worker->addAction(parserdiff);
    } else {
         ATC_STREAM_LOGI() << "enter full ota" << std::endl;
        parser.reset(new Parser(m_isoPath, m_worker));

        parser->setMd5(md5);
        parser->setParts(parts);
        parser->setBuffer(m_buf);
        m_worker->addAction(parser);
    }
    return true;
}

}
}