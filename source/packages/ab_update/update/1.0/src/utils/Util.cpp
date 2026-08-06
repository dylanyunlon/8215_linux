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

#include <sys/mount.h>
#include <sys/wait.h>
#include <tinyxml2.h>
#include "utils/Util.hpp"
#include <dirent.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <time.h>
#include "utils/Image.hpp"
#include "utils/File.hpp"

#include "bootctrl/BootCtrl.hpp"
#include "gpt/GPT.hpp"

#include "mz.h"
#include "mz_zip.h"
#include "mz_strm.h"
#include "mz_zip_rw.h"

#define MD5_STRING_SIZE     32
#define MD5_ARRAY_SIZE      16

namespace atcupdateservice {
namespace utils {

static uint64_t g_timeStamp = 0;
static CheckPoint *g_checkPoints = nullptr;
static size_t g_maxCheckPoint = 20;
static size_t g_curIndex = 0;
static int g_cpfd = -1;

#ifdef CONFIG_NAND_BOOT
std::vector<std::string> gXmlPart;
#endif

struct wp_cmd_arg {
    int wp_action;
    uint32_t wpg_size_of_xml;//the write protect group size configed in xml
    uint32_t sect_start;
    uint32_t sect_end;
    char *partition_name;
    char *wp_dump_info;
};

#ifdef CONFIG_NAND_BOOT
int getPartIdxFromXml(const char* partname)
{
    for (int i = 0; i< gXmlPart.size(); i++) {
        if (gXmlPart[i] == partname) {
            ATC_STREAM_LOGI() << "find "<< partname << " index " << i << std::endl;
            return i;
        }
    }

    return -1;
}

bool FSUtil::disableWriteProtect() {
    int fd;
    const char *node = "/proc/nand_wp";
    const char *cmd = "clear_all_wp";

    fd = open(node, O_RDWR);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "open /proc/nand_wp failed! "
                          << "error " << strerror(errno) << std::endl;
        return false;
    }

    if (write(fd, cmd, strlen(cmd)) < 0) {
        ATC_STREAM_LOGE() << "write /proc/nand_wp failed! "
                          << "error " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    ATC_STREAM_LOGI() << "disable all wp success" << std::endl;

    close(fd);
    return true;
}

bool FSUtil::enableWriteProtect() {
    int fd;
    const char *node = "/proc/nand_wp";
    const char *cmd = "restore_def_wp";

    fd = open(node, O_RDWR);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "open /proc/nand_wp failed! "
                          << "error " << strerror(errno) << std::endl;
        return false;
    }

    if (write(fd, cmd, strlen(cmd)) < 0) {
        ATC_STREAM_LOGE() << "write /proc/nand_wp failed! "
                          << "error " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    ATC_STREAM_LOGI() << "restore all wp success" << std::endl;

    close(fd);
    return true;
}

#else
bool FSUtil::disableWriteProtect() {
    int fd = open(WP_DEV, O_RDWR);
    const char *data = "0x19 0x1 0x55\n";

    if (fd < 0) {
        ATC_STREAM_LOGE() << "open /proc/msdc_debug failed! "
                          << "error " << strerror(errno) << std::endl;
        return false;
    }

    if (write(fd, data, sizeof(data)) < 0) {
        ATC_STREAM_LOGE() << "write /proc/msdc_debug failed! "
                          << "error " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    ATC_STREAM_LOGI() << "disable all wp success" << std::endl;

    close(fd);
    return true;
}

bool FSUtil::enableWriteProtect() {
    int fd = open(WP_DEV, O_RDWR);
    const char *data = "0x19 0x2 0x55\n";

    if (fd < 0) {
        ATC_STREAM_LOGE() << "open /proc/msdc_debug failed! "
                          << "error " << strerror(errno) << std::endl;
        return false;
    }

    if (write(fd, data, sizeof(data)) < 0) {
        ATC_STREAM_LOGE() << "write /proc/msdc_debug failed! "
                          << "error " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    ATC_STREAM_LOGI() << "restore all wp success" << std::endl;

    close(fd);
    return true;
}
#endif

ssize_t FSUtil::writePartition(const std::string &devName, ssize_t &offset,
                            const char *buff, ssize_t buffSize, ssize_t partSize) {
    ssize_t size;

    if (buffSize + offset > partSize) {
        return false;
    }
    size = writeFix(devName.c_str(), buff, offset, buffSize);
    if (size > 0) {
        offset += size;
    }
    return size;
}

bool FSUtil::ISOUmount(const std::string &mountPoint) {
    int rt = 0;

    for (int i = 0; i < 10; i++) {
        if ((rt = umount2(mountPoint.c_str(), UMOUNT_NOFOLLOW)) == 0) {
            ATC_STREAM_LOGI() << "umount succeed!" << std::endl;
            return true;
        }
        usleep(50*1000);
    }
    ATC_STREAM_LOGE() << "umount still failure after 10 tries, error : "
                      << strerror(errno) << std::endl;
    return false;
}

bool FSUtil::ISOMount(const std::string &isoFile, const std::string &mountPoint) {
    int rt = -1;
    int status;

    if (isoFile.empty() || mountPoint.empty()) {
        ATC_STREAM_LOGE() << "arguments invalid iso : " <<  isoFile
                          << "mountPoint : " << mountPoint << std::endl;
        return false;
    }

    if (access(mountPoint.c_str(), F_OK)) {
        ATC_STREAM_LOGI() << "mount point not exist try to create mointpoint : " << mountPoint << std::endl;
        rt = mkdir(mountPoint.c_str(), 0644);
        if (rt < 0) {
            ATC_STREAM_LOGE() << "failed to make point " << std::endl;
            return false;
        }
    }

    ATC_STREAM_LOGI() << "isoFile : " <<  isoFile
                      << " mountPoint : " << mountPoint << std::endl;
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[]= {"/bin/mount", "-o", "loop", (char *)NULL, (char *)NULL, (char *)NULL};
        argv[3] = const_cast<char*>(isoFile.c_str());
        argv[4] = const_cast<char*>(mountPoint.c_str());

        execv(argv[0], argv);
        ATC_STREAM_LOGC() << "Can't run"<< argv[0] << " error : " << strerror(errno) << std::endl;
        _exit(-1);
    } else if (pid < 0) {
        ATC_STREAM_LOGC() << "fork failure " << " error : " << strerror(errno) << std::endl;
        return false;
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return true;
    } else {
        ATC_STREAM_LOGE() << "mount failed ret : " << WEXITSTATUS(status);
        return false;
    }
}
#if 1
bool FSUtil::loadPartitionInfo(const std::string &xml, std::map<std::string, PartInfo> &infos) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError  err = doc.LoadFile(xml.c_str());

    if (err != 0) {
        ATC_STREAM_LOGE() << "failed to load partition info scatter file path : "
                          << xml << std::endl;
        return false;
    }
    tinyxml2::XMLElement *root = doc.RootElement();
    for(tinyxml2::XMLElement *curEle = root->FirstChildElement(); curEle; curEle = curEle->NextSiblingElement()) {
        unsigned long size;
        PartInfo pInfo;
        std::string type = curEle->Attribute("type");
        std::string name = std::string(BLOCK_DEV_PATH) + curEle->Attribute("name");
        std::string imagename =  std::string(ATC_ISO_MOUNTPOINT) + curEle->Attribute("imagename");

        memset(&pInfo, 0, sizeof(pInfo));
        //only care about the slot that is about to update
        if (!isUpdatePart(name)) {
            ATC_STREAM_LOGI() << " skip partition : " << name << std::endl;
            continue;
        }
        sscanf(curEle->Attribute("size"), "%lx", &size);

        ATC_STREAM_LOGI() << "partition name : " << name
                          << " image name : " << imagename
                          << " partition size : " << size
                          << " partition type : " << type
                          << std::endl;

        std::transform(type.begin(), type.end(), type.begin(), ::tolower);
        if (type == "ext4") {
            pInfo.raw = false;
        } else if (type == "raw") {
            pInfo.raw = true;
        } else {
            ATC_STREAM_LOGE() << "unsupport partition type : " << type << std::endl;
            return false;
        }

        pInfo.image = imagename;
        pInfo.part =  name;
        pInfo.partsize = size;
        infos[imagename] = pInfo;
    }

    return true;
}
#else
//idle function
bool FSUtil::loadPartitionInfo(const std::string &xml,
               std::map<std::string, PartInfo> &infos) {
    return false;
}
#endif

uint64_t FSUtil::getPartitionSize(const std::string &devName) {
    int fd = 0;
    int rt = 0;
    uint64_t partSize;

    fd = open(devName.c_str(), O_RDONLY);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "failed to open dev : " << devName
                          << " error : " << strerror(errno) << std::endl;
    }

    rt = ioctl(fd, BLKGETSIZE64, &partSize);
    if (rt) {
        ATC_STREAM_LOGE() << "failed to get partition size, dev : " << devName
                              << " error : " << strerror(errno) << std::endl;
        partSize = (uint32_t)-1;
    }
    close(fd);

    return partSize;
}
bool listAllFilesFromZip(const std::string& zipFilename, std::vector<std::string>& files) {
    void *zip_reader = NULL;
    files.clear();
    
    zip_reader = mz_zip_reader_create();
    if (zip_reader == NULL) {
        ATC_STREAM_LOGE() << "Failed to create zip reader." << std::endl;
        return false;
    }

    if (mz_zip_reader_open_file(zip_reader, zipFilename.c_str()) != MZ_OK) {
        ATC_STREAM_LOGE() << "Failed to open zip file: " << zipFilename << std::endl;
        mz_zip_reader_delete(&zip_reader);
        return false;
    }

    int err = mz_zip_reader_goto_first_entry(zip_reader);
    while (err == MZ_OK) {
        mz_zip_file* file_info = NULL;
        if (mz_zip_reader_entry_get_info(zip_reader, &file_info) == MZ_OK) {
            files.push_back(std::string(file_info->filename));
        }
        err = mz_zip_reader_goto_next_entry(zip_reader);
    }

    mz_zip_reader_close(zip_reader);
    mz_zip_reader_delete(&zip_reader);
    sort(files.begin(), files.end());

    return true;
}
bool FSUtil::listAllFile(std::string path, std::vector<std::string> &files) {
    std::string fileName = path + "/" + "oldpackage";// represent ota-diff
    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "failed to open dev : " << fileName
                          << "Maybe is zip-update-file, try to open zip-update-file " << std::endl;
        return  listAllFilesFromZip(RECOVERY_UPDATE_ZIP_NAME, files);
    }
    close(fd);
    DIR *dir = opendir(path.c_str());
    struct dirent *dent;
    if (dir == NULL) {
        ATC_STREAM_LOGE() << "failed to open directory " << path << std::endl;
       return false;
    }

    if (path[path.size()-1] != '/') {
        path += "/";
    }
    while ((dent=readdir(dir)) != NULL) {
        std::string fname(dent->d_name);
        files.push_back(fname);
    }

    closedir(dir);
    sort(files.begin(), files.end());
    return true;

}

uint32_t HashUtil::checkSum32(uint32_t lastSum, const char *buf, unsigned len) {
    const char *end = nullptr;
    for (end = buf + len; buf != end; ++buf)
        lastSum += *buf;

    return lastSum;
}

bool FSUtil::setUpdateSlotUnbootable() {
    int slot = bootctrl::getCurrentSlot();
    int rt = 0;
    if (slot == 0) {
        slot = 1;
    } else if (slot == 1) {
        slot = 0;
    } else {
        return false;
    }
    ATC_STREAM_LOGI() << "set slot : " << slot <<  " unbootable!" << std::endl;
    rt = bootctrl::setSlotUnbootable(slot);
    if (rt < 0) {
        return false;
    }

    return true;
}

bool FSUtil::setUpdateSlotActive() {
    int slot = bootctrl::getCurrentSlot();
    int rt = 0;
    if (slot == 0) {
        slot = 1;
    } else if (slot == 1) {
        slot = 0;
    } else {
        return false;
    }
    ATC_STREAM_LOGI() << "set slot : " << slot <<  " active!" << std::endl;
    rt = bootctrl::setActiveSlot(slot);

    if (rt < 0) {
        return false;
    }

    return true;
}

std::string FSUtil::updateSlot() {
    int slot = bootctrl::getCurrentSlot();
    if (slot == 0) {
        return "_b";
    } else if (slot == 1) {
        return "_a";
    } else {
        return "not ab upgrade, please ignore!";
    }
}

void dumpMemory(const uint8_t *data, uint32_t size) {
    if (data == nullptr) {
        return;
    }
    while(size) {
        uint32_t lineSize = std::min((uint32_t)16, size);
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setw(2);
        for (uint32_t i = 0; i < lineSize; i++) {
            ss << (uint32_t)*data << " ";
            ++data;
        }
        ATC_STREAM_LOGI() << ss.str() << std::endl;
        size -= lineSize;
    }
}

static int findLastCpIndex(const CheckPoint* cp, unsigned cnt) {
    int idx = -1;
    uint64_t maxStamp = 0;

    for (unsigned i = 0; i < cnt; i++) {
        if (cp[i].valid && maxStamp < cp[i].timestamp) {
            idx = i;
            maxStamp = cp[i].timestamp;
        }
    }
    if (idx == -1) {
        ATC_STREAM_LOGW() << "no valid checkpoint" << std::endl;
        return -1;
    }
    //check
    for (int i = (idx -1 + cnt) % cnt; i != idx; i = (i-1+cnt)%cnt) {
        if (cp[i].valid == 0) break;
        if (cp[(i+1+cnt)%cnt].timestamp - cp[i].timestamp != 1) {
            ATC_STREAM_LOGE() << "pre idx: " << (i + 1 + cnt) % cnt << "value : "
                              << cp[(i+1+cnt)%cnt].timestamp <<  "cur : "
                              << i << " value : " << cp[i].timestamp << std::endl;
            return -1;
        }
    }
    ATC_STREAM_LOGI() << "check timestamp value done" << std::endl;
    return idx;
}

bool FSUtil::parseCheckPoint(CheckPoint &checkpoint) {
    ssize_t size = 0;
    ssize_t count = 0;
    CheckPoint *cp = nullptr;
    int fd = open(CHECKPOINT_PATH, O_RDWR);
    int idx = 0;
    uint32_t chksum = 0;

    if (fd < 0) {
        ATC_STREAM_LOGE() << "open checkpoint file : " << CHECKPOINT_PATH
                          << " error : " << strerror(errno) << std::endl;
        return false;
    }
    size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        ATC_STREAM_LOGE() << "lseek failure! error : " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }
    if (size % ((ssize_t)sizeof(CheckPoint)) != 0) {
        ATC_STREAM_LOGE() << "invalid size, size : " << size << " checkpoint size : "
                          << sizeof(CheckPoint) << std::endl;
        close(fd);
        return false;
    }
    cp =  (CheckPoint *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                             fd, 0);
    if (cp == nullptr) {
        ATC_STREAM_LOGE() << "failed to mmap : " << CHECKPOINT_PATH << "error : "
                          << strerror(errno) << std::endl;
        close(fd);
        return false;
    }
    count = size / ((ssize_t)sizeof(CheckPoint));
    idx = findLastCpIndex(cp, count);
    if (idx < 0 ) {
        ATC_STREAM_LOGE() << "failed to find latest idx, checkpoint may broken!" << std::endl;
        munmap(cp, size);
        cp = nullptr;
        close(fd);
        return false;
    }

    for (int i = (idx -1 + count) % count ; i != idx; i = (i - 1 + count) % count) {
        chksum = 0;
        if (!cp[i].valid) {
            ATC_STREAM_LOGW() << "invalid checkpoint, skip" << std::endl;
            continue;
        }
        /* compare with md5 of the iso file */
        /*
         * ...
         */
        chksum = HashUtil::checkSum32(0, ((char*)(&cp[i]) + sizeof(uint32_t)),
                                              sizeof(CheckPoint) - sizeof(uint32_t));
        if (chksum != cp[i].thisChkSum) {
            ATC_STREAM_LOGW() << "invalid check sum 0x" << std::hex << chksum
                              << " expected : " << cp[i].thisChkSum << " skip" << std::endl;
            continue;
        }
        if (cp[i].offsetWrite != 0) {
            ssize_t nread = 0;
            std::string partname = std::string(BLOCK_DEV_PATH) + std::string(cp[i].partname);

            if (cp[i].offsetWrite < cp[i].lastWriteSize) {
                ATC_STREAM_LOGW() << "unexpect cp[i].offsetWrite(" << cp[i].offsetWrite
                                  << ") < cp[i].lastWriteSize(" << cp[i].lastWriteSize
                                  << ")" << std::endl;
                continue;
            }
            std::shared_ptr<char> buf(new(std::nothrow) char[cp[i].lastWriteSize],
                [](const char *ptr){
                    if (ptr) {
                        delete [] ptr;
                    }
            });
            nread = readFix(partname.c_str(), buf.get(),
                        cp[i].offsetWrite - cp[i].lastWriteSize, cp[i].lastWriteSize);
            if (nread != (ssize_t)cp[i].lastWriteSize) {
                if (nread < 0) {
                    ATC_STREAM_LOGE() << "read error : " << strerror(errno) << std::endl;
                    break;
                } else {
                    ATC_STREAM_LOGE() << "unexpected nread size : " << nread
                                      << " expect : " << cp[i].lastWriteSize << std::endl;
                    break;
                }
            }
        }
        memcpy(&checkpoint, &cp[i], sizeof(cp[i]));
        munmap(cp, size);
        cp = nullptr;
        close(fd);
        return true;
    }
    munmap(cp, size);
    cp = nullptr;
    close(fd);
    ATC_STREAM_LOGI() << "no valid checkpoint" << std::endl;
    return false;
}

#ifdef CONFIG_NAND_BOOT
ssize_t FSUtil::nandReadRaw(struct mtd_dev_info *mtd, const char* mtdname, char *buf, ssize_t offset, ssize_t size)
{
    ssize_t ret = 0;
    ssize_t currentOff = offset;
    int remainSize = size;
    char *rbuf = NULL;
    int fd = 0;
    int is_bad = 0;
    int rsize = 0, totalSize = 0;
    int block, blockOff;
    int page = 0, pageOff = 0, pageNum = 0;

    if (size % mtd->min_io_size || offset % mtd->min_io_size) {
        ATC_STREAM_LOGE() << "offset/size not align with pagesize:"<< mtd->min_io_size << std::endl;
        return -1;
    }

    fd = open(mtdname, O_RDWR);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "failed to open part : "  << mtdname << std::endl;
        ret =  -1;
        goto out;
    }

    rbuf = (char* )malloc(mtd->min_io_size);
    if (!rbuf) {
        ATC_STREAM_LOGE() << "failed to malloc rbuf " << std::endl;
        ret =  -1;
        goto out;
    }
    memset(rbuf, 0x0, mtd->min_io_size);

    //ATC_STREAM_LOGI()<<"nandReadRaw mtd:"<<mtdname<<" offset:"<<offset<<" size:"<<size << std::endl;

    while (remainSize > 0 && currentOff < mtd->size) {
        block = currentOff / mtd->eb_size;
        blockOff = currentOff % mtd->eb_size;

        if ((is_bad = mtd_is_bad(mtd, fd, block)) < 0) {
            ATC_STREAM_LOGE()<<"failed to check bad block"<< std::endl;
            ret = -1;
            goto out;
        }

        if (is_bad == 1) {
            currentOff += mtd->eb_size;
            ATC_STREAM_LOGI() << "skipping bad block " << block << std::endl;
            continue;
        }

        rsize = (remainSize > mtd->eb_size - blockOff) ? mtd->eb_size - blockOff : remainSize;
        rsize = (rsize / mtd->min_io_size) * mtd->min_io_size;
        if (rsize == 0) {
            //ATC_STREAM_LOGI()<<"++++ rsize is zero ++++" << std::endl;
            rsize = mtd->min_io_size;
        }
        //ATC_STREAM_LOGI()<<"rsize:"<<rsize<<" remainSize:"<<remainSize<< std::endl;

        for (page = 0; page < rsize/mtd->min_io_size; page++) {
            pageOff = pageNum * mtd->min_io_size;

            //ATC_STREAM_LOGI()<<"[page]: "<< page << " [pageOff]: "<< pageOff << " [pageNum]:"<< pageNum << std::endl;
            //ATC_STREAM_LOGI()<<"[currentOff]:"<< currentOff<< std::endl;
            ret = mtd_read(mtd, fd, block, currentOff % mtd->eb_size, rbuf, mtd->min_io_size);
            if (ret < 0) {
                ATC_STREAM_LOGE()<<"mtd_read failed"<< std::endl;
                goto out;
            }
            memcpy(buf + pageOff, rbuf, mtd->min_io_size);

            remainSize -= mtd->min_io_size;
            currentOff += mtd->min_io_size;
            totalSize += mtd->min_io_size;
            pageNum++;
        }
    }

    ret = totalSize;
out:
    if (fd > 0)
        close(fd);
    if (rbuf)
        free(rbuf);

    return ret;
}

ssize_t FSUtil::nandWriteRaw(libmtd_t mtdDesc,struct mtd_dev_info *mtd, const char* mtdname, char *buf, ssize_t offset, ssize_t size)
{
    ssize_t ret = 0;
    ssize_t currentOff = offset;
    int remainSize = size;
    char *wbuf = NULL;
    int fd = 0;
    int is_bad = 0;
    int wsize = 0, totalSize = 0;
    int block, blockOff;
    int page = 0, pageOff = 0, pageNum = 0;
    uint8_t write_mode;

    if (size % mtd->min_io_size || offset % mtd->min_io_size) {
        ATC_STREAM_LOGE() << "offset/size not align with pagesize:"<< mtd->min_io_size << std::endl;
        return -1;
    }

    fd = open(mtdname, O_RDWR);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "failed to open part : "  << mtdname << std::endl;
        ret =  -1;
        goto out;
    }

    wbuf = (char* )malloc(mtd->min_io_size);
    if (!wbuf) {
        ATC_STREAM_LOGE() << "failed to malloc rbuf " << std::endl;
        ret =  -1;
        goto out;
    }
    memset(wbuf, 0xFF, mtd->min_io_size);

    //ATC_STREAM_LOGI()<<"nandWriteRaw mtd:"<<mtdname<<" offset:"<<offset<<" size:"<<size << std::endl;

    while (remainSize > 0 && currentOff < mtd->size) {
        block = currentOff / mtd->eb_size;
        blockOff = currentOff % mtd->eb_size;

        if ((is_bad = mtd_is_bad(mtd, fd, block)) < 0) {
            ATC_STREAM_LOGE()<<"failed to check bad block"<< std::endl;
            ret = -1;
            goto out;
        }

        if (is_bad == 1) {
            currentOff += mtd->eb_size;
            ATC_STREAM_LOGI() << "skipping bad block " << block << std::endl;
            continue;
        }

        //ATC_STREAM_LOGI() << "erase block is " << block <<std::endl;
        if (mtd_erase(mtdDesc, mtd, fd, block) < 0) {
            ATC_STREAM_LOGE()<<"erase failed, marking bad block"<< std::endl;
            #if 0
            if (mtd_mark_bad(mtd, fd, block)) {
                ATC_STREAM_LOGE()<<"failed to mark bad block"<< std::endl;
                goto out;
            }
            #endif
            currentOff += mtd->eb_size;
            continue;
        }

        wsize = (remainSize > mtd->eb_size - blockOff) ? mtd->eb_size - blockOff : remainSize;
        wsize = (wsize / mtd->min_io_size) * mtd->min_io_size;
        if (wsize == 0) {
            //ATC_STREAM_LOGI()<<"wsize is min_io_size" << std::endl;
            wsize = mtd->min_io_size;
        }
        //ATC_STREAM_LOGI()<<"wsize:"<<wsize<<" remainSize:"<<remainSize<< std::endl;

        for (page = 0; page < wsize/mtd->min_io_size; page++) {
            pageOff = pageNum * mtd->min_io_size;

            //ATC_STREAM_LOGI()<<"[page]: "<< page << " [pageOff]: "<< pageOff << " [pageNum]:"<< pageNum << std::endl;
            memcpy(wbuf, buf + pageOff, mtd->min_io_size);
            //ATC_STREAM_LOGI()<<"[currentOff]:"<< currentOff<< std::endl;
            ret = mtd_write(mtdDesc, mtd, fd, block, currentOff % mtd->eb_size, wbuf, mtd->min_io_size, NULL, 0, write_mode);
            if (ret < 0) {
                ATC_STREAM_LOGE()<<"mtd_write failed"<< std::endl;
                goto out;
            }

            remainSize -= mtd->min_io_size;
            currentOff += mtd->min_io_size;
            totalSize += mtd->min_io_size;
            pageNum++;
        }
    }

    ret = totalSize;
out:
    if (fd > 0)
        close(fd);
    if (wbuf)
        free(wbuf);

    return ret;
}
#endif

ssize_t FSUtil::readFix(const char *filename, char *buf, ssize_t offset, ssize_t size) {
    int devFd = 0;
    ssize_t rt = 0;

    devFd = open(filename, O_RDONLY | O_LARGEFILE);
    if (devFd < 0) {
        ATC_STREAM_LOGE() << "failed to open image : "  << filename << std::endl;
        rt =  -1;
        goto out;
    }
    rt = readFixFd(devFd, buf, offset, size);
    close(devFd);
out:
    return rt;
}

ssize_t FSUtil::readFixFd(int fd, char *buf, ssize_t offset, ssize_t size) {
    ssize_t nLeft = size;
    ssize_t cur = 0;
    ssize_t curOff = 0;

    curOff = lseek64(fd, offset, SEEK_SET);
    if (curOff != offset) {
        ATC_STREAM_LOGW() << "invalid offset, expect : "
                          << offset << " actually : " << curOff << std::endl;
        return -1;
    }
    while (nLeft) {
        ssize_t nread = read(fd, buf + cur, nLeft);
        if (nread == 0) {
            break;
        } else if (nread < 0) {
            ATC_STREAM_LOGW() << "failed to read error : " << strerror(errno) << std::endl;
            return -1;
        }
        cur += nread;
        nLeft -= nread;
    }
    return cur;
}

ssize_t FSUtil::writeFixFd(int fd, const char *buf,
                          ssize_t offset, ssize_t size) {
    ssize_t cur = 0;
    ssize_t nLeft = size;
    ssize_t curOff = 0;
    int rt = 0;

    curOff = lseek64(fd, offset, SEEK_SET);
    if (curOff != offset) {
        if (curOff == -1) {
            ATC_STREAM_LOGE() << "lseek64 failed! error : " << strerror(errno) << std::endl;
            return -1;
        } else {
            ATC_STREAM_LOGE() << " unexpect offset : " << curOff
                              << " expect : " << offset << std::endl;
            return -1;
        }
    }
    while (nLeft) {
        ssize_t nWrite = write(fd, buf + cur, nLeft);
        if (nWrite < 0) {
            ATC_STREAM_LOGW() << "failed to write, error : " << strerror(errno) << std::endl;
            return -1;
        } else if (nWrite == 0) {
            break;
        }
        cur += nWrite;
        nLeft  -= nWrite;
    }
    rt = fsync(fd);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "fsync failed! rt = " << rt
                          << " error : " << strerror(errno) << std::endl;
        return rt;
    }
    return cur;
}

ssize_t FSUtil::writeFix(const char *filename, const char *buf,
                         ssize_t offset, ssize_t size) {
    int fd = 0;
    ssize_t rt = 0;

    fd = open(filename, O_WRONLY | O_LARGEFILE);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "failed to open file("  << filename << ") error : "
                          << strerror(errno) << std::endl;
        rt = -1;
        goto out;
    }
    rt = writeFixFd(fd, buf, offset, size);
    close(fd);
out:
    return rt;
}

bool FSUtil::writeCheckPoint(const CheckPoint &cp) {
    int rt = 0;
    if (g_checkPoints == nullptr) {
        ATC_STREAM_LOGE() << "checkpoint not inited yet!" << std::endl;
        return false;
    }
    memcpy(&g_checkPoints[(g_curIndex) % g_maxCheckPoint], &cp, sizeof(cp));
    rt = msync((void *)g_checkPoints, g_maxCheckPoint * sizeof(cp), MS_SYNC);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "msync failed, error : " << strerror(errno) << std::endl;
        return false;
    }
    ++g_curIndex;
    return true;
}

bool FSUtil::removeCheckPointFile() {
    int rt = unlink(CHECKPOINT_PATH);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "failed to remove checkpoint file, error : "
                          << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

bool FSUtil::initCheckPoint() {
    int rt = 0;
    if (g_cpfd != -1) {
        ATC_STREAM_LOGW() << "checkpoint had already been inited..." << std::endl;
        return true;
    }
    g_cpfd = open(CHECKPOINT_PATH, O_RDWR | O_LARGEFILE | O_CREAT, 0666);
    g_timeStamp = 0;
    if (g_cpfd < 0) {
        ATC_STREAM_LOGE() << "open checkpoint file failure, error : "
                          << strerror(errno) << std::endl;
        return false;
    }
    rt = ftruncate(g_cpfd, g_maxCheckPoint * sizeof(CheckPoint));
    if (rt < 0) {
        ATC_STREAM_LOGE() << "resize checkpoint file failure, error : "
                          << strerror(errno) << std::endl;
        return false;
    }
    g_checkPoints = (struct CheckPoint *)mmap(0, g_maxCheckPoint * sizeof(CheckPoint), PROT_READ | PROT_WRITE
                                              , MAP_SHARED, g_cpfd, 0);
    if(g_checkPoints == nullptr) {
        ATC_STREAM_LOGE() << "initCheckPoint mmap failure, error : "
                          << strerror(errno) << std::endl;
        return false;
    }
    memset(g_checkPoints, 0, g_maxCheckPoint * sizeof(CheckPoint));
    ATC_STREAM_LOGW() << "g_checkPoints : 0x" << std::hex << (unsigned long)(g_checkPoints) << std::endl;
    return true;
}

bool FSUtil::clearCheckPoint() {
    if (g_checkPoints != nullptr) {
        memset(g_checkPoints, 0, g_maxCheckPoint * sizeof(CheckPoint));
        msync((void *)g_checkPoints, g_maxCheckPoint * sizeof(CheckPoint), MS_SYNC);
    }
    return true;
}

bool FSUtil::deInitCheckPoint() {
    int rt = 0;
    bool succeed = true;

    if (g_checkPoints != nullptr) {
        rt = munmap((void *)g_checkPoints, g_maxCheckPoint * sizeof(CheckPoint));
        g_checkPoints = nullptr;
    } else {
        ATC_STREAM_LOGI() << "checkpoint had already been deinited!" << std::endl;
        return true;
    }
    if (rt < 0) {
        ATC_STREAM_LOGE() << "failed to munmap, error : " << strerror(errno) << std::endl;
        succeed = false;
    }
    close(g_cpfd);
    g_cpfd = -1;

    return succeed;
}

bool FSUtil::removeCheckPoint() {
    int rt = 0;
    DIR *dir = nullptr;

    rt = unlink(CHECKPOINT_PATH);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "failed to unlink " << CHECKPOINT_PATH
                          << ", error : " << strerror(errno) << std::endl;
        return false;
    }
    dir = opendir(CHECKPOINT_PARENT);
    fsync(dirfd(dir));
    closedir(dir);
    return true;
}

std::string FSUtil::getPackageDiffInfo(const std::string &filename, std::string targetname) {
    std::ifstream file(filename);
    std::string line;
    std::string filedValue;

    while (std::getline(file, line)) {
        if (line.find(targetname) != std::string::npos) {
            std::size_t colonpos = line.find(":");
            if (colonpos != std::string::npos) {
                filedValue = line.substr(colonpos + 1);
                 ATC_STREAM_LOGE() << "getPackageDiffInfo, filename : " << filename
                          << " filedValue : " << filedValue << std::endl;
            }
            break;
        }
    }

    file.close();
    return filedValue;
}

std::string FSUtil::getCurentSlotPart(std::string &part_name) {
    int slot = bootctrl::getCurrentSlot();
    std::string part_slot_name = part_name;

    if (slot == 0) {
        part_slot_name.replace(part_name.find("_"), 2, "_a");
    } else if (slot == 1) {
        part_slot_name.replace(part_name.find("_"), 2, "_b");
    } else {
        return "not ab upgrade, please ignore!";
    }
    return part_slot_name;
}


std::shared_ptr<char> FSUtil::loadOneExt4Chunk(const std::string &imagename, ssize_t &offset,
                                               ssize_t &datalen, uint32_t blkSz, bool &dontCare) {
    std::shared_ptr<char> buf = nullptr;
    ChunkHeader  chunkHeader;
    int fd = 0;
    ssize_t size = 0;
    ChunkType type = ChunkType::NONE;

    datalen = -1;
    dontCare = false;
    fd = open(imagename.c_str(), O_RDONLY | O_LARGEFILE);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "failed to open image : " << imagename
                          << " error : " << strerror(errno) << std::endl;
        return nullptr;
    }

    size = readFixFd(fd, (char *)&chunkHeader, offset, sizeof(chunkHeader));
    if (size != sizeof(chunkHeader)) {
        ATC_STREAM_LOGE() << "failed to read chunk header, imagename : "
                          << imagename << " expect : " << sizeof(chunkHeader)
                          <<" actual :" << size << std::endl;

        goto done;
    }
    offset += size;

    datalen = chunkHeader.chunk_sz * blkSz;
    type = (ChunkType)chunkHeader.chunk_type;
    if(ChunkType::RAW == type) {
        buf.reset(new(std::nothrow) char[datalen], [](const char * ptr) {
            if (ptr) {
                delete [] ptr;
            }
        });
        if (buf == nullptr) {
            ATC_STREAM_LOGE() << "failed to allow memory size : " << datalen << " error: "
                              << strerror(errno) << std::endl;
            datalen = -1;
            goto done;
        }
        size = readFixFd(fd, buf.get(), offset, datalen);
        if (size != datalen) {
            ATC_STREAM_LOGE() << "failed to read chunk header, imagename : "
                            << imagename << " expect : " << datalen
                            <<" actual :" << size << std::endl;
            buf.reset();
            buf = nullptr;
            datalen = -1;
        } else {
            offset += datalen;
        }
    } else if (ChunkType::FILL == type) {
        datalen = 0;
        ATC_STREAM_LOGI() << "chuntype type : fill" << std::endl;
    } else if (ChunkType::DONT_CARE == type) {
        offset += (chunkHeader.total_sz - sizeof(ChunkHeader));
        dontCare = true;
        ATC_STREAM_LOGI() << "chuntype type : dont care" << std::endl;
    } else {
        datalen = -1;
        ATC_STREAM_LOGE() << "unknown trunk type : " << (unsigned)(type) << std::endl;
    }
done:
    close(fd);
    return buf;
}

std::shared_ptr<char> FSUtil::loadRaw(const std::string &imagename, ssize_t &offset,
                                      ssize_t expect, ssize_t &actual) {
    std::shared_ptr<char> buf(new char[expect], [](const char *ptr) {
        if (ptr) {
            delete [] ptr;
        }
    });

    if (buf == nullptr) {
        ATC_STREAM_LOGE() << "failed to allow memory size : " << expect << " error: "
                          << strerror(errno) << std::endl;
        buf.reset();
        buf = nullptr;
        actual = -1;
        return nullptr;
    }
    actual = readFix(imagename.c_str(), buf.get(), offset, expect);
    if (actual < 0) {
        return nullptr;
    }
    offset += actual;

    return buf;
}

std::string HashUtil::md5Str(const char *filename, uint64_t size, int diff_flag) {
    uint8_t md5[MD5_ARRAY_SIZE];
    bool rt = false;
    std::string res;

    if (diff_flag == 0) {
        rt = md5Sum_ota_full(filename, md5, size);
        if (rt == false) {
            ATC_STREAM_LOGE() << "failed to calculate md5sum value" << std::endl;
            return std::string("");
        }
    } else {
        rt = md5Sum_ota_diff(filename, md5, size);
        if (rt == false) {
            ATC_STREAM_LOGE() << "failed to calculate md5sum value" << std::endl;
            return std::string("");
        }
    }
    res = md5Array2Str(md5);

    return res;
}

std::string HashUtil::md5BoardStr(const char *filename, uint64_t offset, uint64_t size) {
    uint8_t md5[MD5_ARRAY_SIZE];
    bool rt = false;
    std::string res;

    rt = md5BoardSum(filename, md5, offset, size);
    if (rt == false) {
        ATC_STREAM_LOGE() << "failed to calculate md5sum value" << std::endl;
        return std::string("");
    }

    res = md5Array2Str(md5);

    return res;
}


char HashUtil::decToHex(uint8_t val) {
    if (val > 15) {
        ATC_STREAM_LOGE() << "val : " << val << "not valid " << std::endl;
        return 0;
    }
    if (val >= 10) {
        return 'a' + val - 10;
    } else {
        return '0' + val;
    }
}

std::string HashUtil::md5Array2Str(uint8_t md5[16]) {
    std::string res;
    res.resize(MD5_STRING_SIZE);

    for (size_t i = 0; i < MD5_ARRAY_SIZE; ++i) {
        res[((i << 1))] = decToHex((md5[i] >> 4)& 0x0f);
        res[(i << 1) | 1] = decToHex(md5[i] & 0x0f);
    }

    return res;
}

bool HashUtil::md5Compare(uint8_t md5A[MD5_ARRAY_SIZE], uint8_t md5B[MD5_ARRAY_SIZE]) {
    for (size_t i = 0; i < MD5_ARRAY_SIZE; i++) {
        if (md5A[i] != md5B[i]) {
            return false;
        }
    }
    return true;
}

bool readFromZipFile(const std::string& filename, const std::function<bool(std::istringstream&)>& processor) {
    std::string pureFilename = filename.substr(filename.find_last_of("/\\") + 1);

    auto zipFile = atcupdateservice::utils::File::Create(const_cast<char*>(pureFilename.c_str()),
                                                        atcupdateservice::utils::File::READ);
    if (!zipFile) {
        ATC_STREAM_LOGE() << "Failed to create File object for: " << filename << std::endl;
        return false;
    }

    int64_t fileSize = zipFile->img_size(pureFilename);
    if (fileSize <= 0) {
        ATC_STREAM_LOGE() << "Failed to get file size for: " << filename << std::endl;
        return false;
    }

    char* fileBuffer = new(std::nothrow) char[fileSize];
    bool result = false;
    if (!fileBuffer) {
        ATC_STREAM_LOGE() << "Failed to allocate memory for file buffer, size: " << fileSize << std::endl;
        return false;
    }

    int readResult = zipFile->readFixFromZip(pureFilename, fileSize, fileBuffer);
    if (readResult == 0) {
        std::istringstream fileContent(std::string(fileBuffer, fileSize));
        result = processor(fileContent);
    } else {
        ATC_STREAM_LOGE() << "Failed to read file " << filename
                          << " from ZIP, error: " << readResult << std::endl;
    }

    delete[] fileBuffer;
    return result;
}

static std::string findValueInLines(std::istringstream& fileContent, const std::string& target, char delimiter) {
    std::string line;
    std::string filedValue;
    while (std::getline(fileContent, line)) {
        if (line.find(target) != std::string::npos) {
            std::size_t pos = line.find(delimiter);
            if (pos != std::string::npos) {
                filedValue = line.substr(pos + 1);
                if (delimiter == '=') {
                    filedValue.erase(std::remove(filedValue.begin(), filedValue.end(), '\r'), filedValue.end());
                }
            }
            break;
        }
    }
    return filedValue;
}
#if 0
//for zip update
std::string FSUtil::getPackageDiffInfo(const std::string &filename, std::string targetname) {
    std::string filedValue;

    readFromZipFile(filename, [&](std::istringstream& fileContent) -> bool {
        filedValue = findValueInLines(fileContent, targetname, ':');
        ATC_STREAM_LOGE() << "getPackageDiffInfo, filename : " << filename
                          << " filedValue : " << filedValue << std::endl;
        return true;
    });

    return filedValue;
}
#endif

static std::map<std::string, std::string> parseMd5ImageFile(const std::string& filename) {
    std::map<std::string, std::string> result;
    readFromZipFile(filename, [&](std::istringstream& fileContent) -> bool {
        std::string line;

        while (std::getline(fileContent, line)) {
            std::istringstream iss(line);
            std::string md5, image;
            iss >> md5 >> image;
            result.insert(std::make_pair(image, md5));
        }

        return true;
    });

    return result;
}

/*check base partiton for otadiff upgrade*/
bool verifyBasePartition(std::string isoMountPoint, const std::vector<PartInfo::ptr> &parts) {
    if (isoMountPoint[isoMountPoint.size() - 1] != '/') {
        isoMountPoint += "/";
    }
    ATC_STREAM_LOGI() << "enter verifyBasePartition...." << std::endl;
    gpt::GPT::ptr getGpt(new gpt::GPT());
    std::string md5File = isoMountPoint + std::string("oldpackage");
    std::map<std::string ,std::string> partMd5;
    std::string line;
    std::string partname;
    uint64_t size;

    std::ifstream fin(md5File);
    if (!fin.good()){
        ATC_STREAM_LOGI() << "file : " <<md5File << "is not exist, please check" << std::endl;
    }

    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        std::string md5, image;

        iss >> md5 >> image;
        partMd5.insert(std::make_pair(image, md5));
    }
    for (PartInfo::ptr check_part: parts) {
        auto iter = partMd5.find(check_part->image);
        if (iter == partMd5.end()) {
            ATC_STREAM_LOGE() << "no md5 record for image: " << check_part->image
                << " in " << md5File << std::endl;
            return false;
        }

        partname = utils::FSUtil::getCurentSlotPart(check_part->part);

        size =  getGpt->getPartitionRealsize(partname);
        std::string partPath = std::string(BLOCK_DEV_PATH) + partname;
        std::string partMd5 = iter->second;
        std::string md5;
        if (access(partPath.c_str(), F_OK) != 0) {
            ATC_STREAM_LOGE() << "no such image: " << partPath << std::endl;
            return false;
        }

        ATC_STREAM_LOGE() << " part: " << partPath.c_str() << " raw: "
            << check_part->raw  << "real size: " << size << std::endl;
        md5 = HashUtil::md5Str(partPath.c_str(), size, 1);

        if (md5 != partMd5) {
            ATC_STREAM_LOGE() << "part: " << partPath << " verify failed, expect md5: "
                << partMd5 << ", actual md5: " << md5 << std::endl;
            return false;
        }
    }
    return true;
}

static bool isPreloaderPartition(const std::string &partName) {
    return partName == "preloader" || partName == "preloader_bk";
}

static bool verifyPreloaderData(const std::string &partPath, const PartInfo::ptr &part,
    const std::string &expectMd5) {
    const uint64_t headerSize = 512;
    std::string actualMd5 = HashUtil::md5BoardStr(partPath.c_str(), headerSize, part->fileLen);

    if (actualMd5 != expectMd5) {
        ATC_STREAM_LOGE() << "preloader data verify failed, part: " << partPath
                          << ", expect md5: " << expectMd5
                          << ", actual md5: " << actualMd5
                          << ", data offset: " << headerSize
                          << ", size: " << part->fileLen << std::endl;
        return false;
    }

    ATC_STREAM_LOGI() << "preloader data verify success, part: " << partPath
                      << ", md5: " << actualMd5
                      << ", data offset: " << headerSize
                      << ", size: " << part->fileLen << std::endl;
    return true;
}

bool verifyPreloaderHeader(std::string partPath, const PartInfo::ptr &part) {
    const ssize_t headerSize = 512;
    const ssize_t loaderSize = 0x7000;
    char actualHeader[headerSize] = {0};
    char expectHeader[headerSize] = {0};
    std::shared_ptr<char> dataBuf(new(std::nothrow) char[loaderSize], [](const char *ptr) {
        if (ptr) {
            delete [] ptr;
        }
    });

    if (part == nullptr) {
        ATC_STREAM_LOGE() << "invalid part" << std::endl;
        return false;
    }
    if (dataBuf == nullptr) {
        ATC_STREAM_LOGE() << "failed to allocate preloader data buffer" << std::endl;
        return false;
    }

    ssize_t size = FSUtil::readFix(partPath.c_str(), actualHeader, 0, headerSize);
    if (size != headerSize) {
        ATC_STREAM_LOGE() << "failed to read preloader header, expect: " << headerSize
                          << " actual: " << size << std::endl;
        return false;
    }

    memset(dataBuf.get(), 0, loaderSize);
    size = FSUtil::readFix(partPath.c_str(), dataBuf.get(), headerSize, loaderSize);
    if (size != loaderSize) {
        ATC_STREAM_LOGE() << "failed to read preloader payload for header verify, expect: "
                          << loaderSize << " actual: " << size << std::endl;
        return false;
    }

    if ((ssize_t)(sizeof(RawImage::BOOTL_HEADER) * REPLICATION_NUMBER) != headerSize) {
        ATC_STREAM_LOGE() << "invalid preloader header layout, boot header size: "
                          << sizeof(RawImage::BOOTL_HEADER)
                          << " replication: " << REPLICATION_NUMBER << std::endl;
        return false;
    }

#ifdef CONFIG_NAND_BOOT
    File::ptr partFile = NandFile::Create(partPath, File::NAND_READ);
    if (partFile == nullptr) {
        ATC_STREAM_LOGE() << "failed to open preloader part for header verify: "
                          << partPath << std::endl;
        return false;
    }

    RawImage headerImage(part->part, part->image);
    headerImage.createBootloaderHeader(reinterpret_cast<unsigned char *>(expectHeader),
        dataBuf.get(), loaderSize, 0, partFile->getNandIoSize(),
        partFile->getNandOobSize(), partFile->getNandEbSize());
#else
    ATC_STREAM_LOGE() << "preloader header verify only supports nand boot" << std::endl;
    return false;
#endif

    if (memcmp(actualHeader, expectHeader, headerSize) != 0) {
        ATC_STREAM_LOGE() << "preloader header verify failed, part: " << partPath << std::endl;
        return false;
    }

    ATC_STREAM_LOGI() << "preloader header verify success, part: " << partPath << std::endl;
    return true;
}

bool verifyUpdatedPartition(std::string isoMountPoint, const PartInfo::ptr &part) {
    if (isoMountPoint[isoMountPoint.size() - 1] != '/') {
        isoMountPoint += "/";
    }
    if (part == nullptr) {
        ATC_STREAM_LOGE() << "invalid part" << std::endl;
        return false;
    }

    std::string md5File = isoMountPoint + std::string("image.md5");
    std::map<std::string, std::string> imageMd5;
    std::string partPath = std::string(BLOCK_DEV_PATH) + part->part;
    uint64_t size = part->fileLen;

    imageMd5 = parseMd5ImageFile(md5File);
    auto iter = imageMd5.find(part->image);
    if (iter == imageMd5.end()) {
        ATC_STREAM_LOGE() << "no md5 record for image: " << part->image
                          << " in " << md5File << std::endl;
        return false;
    }
    if (size == 0) {
        ATC_STREAM_LOGE() << "invalid fileLen for image: " << part->image << std::endl;
        return false;
    }
    if (access(partPath.c_str(), F_OK) != 0) {
        ATC_STREAM_LOGE() << "no such partition: " << partPath << std::endl;
        return false;
    }
    if (isPreloaderPartition(part->part)) {
        std::string expectMd5 = iter->second;

        if (verifyPreloaderHeader(partPath, part) == false) {
            return false;
        }
        if (verifyPreloaderData(partPath, part, expectMd5) == false) {
            return false;
        }

        ATC_STREAM_LOGI() << "preloader verify success, part: " << partPath
                          << ", image: " << part->image
                          << ", size: " << size << std::endl;
        return true;
    }

    std::string expectMd5 = iter->second;
    std::string actualMd5 = HashUtil::md5BoardStr(partPath.c_str(), 0, size);

    if (actualMd5 != expectMd5) {
        ATC_STREAM_LOGE() << "part: " << partPath << " verify failed, expect md5: "
                          << expectMd5 << ", actual md5: " << actualMd5
                          << ", image: " << part->image << ", size: " << size << std::endl;
        return false;
    }

    ATC_STREAM_LOGI() << "part: " << partPath << " verify success, md5: "
                      << actualMd5 << ", image: " << part->image
                      << ", size: " << size << std::endl;
    return true;
}

bool FSUtil::verifyUpdatePackage(const std::string &isoFile, std::string &md5) {
    std::string md5File = getUpperDir(isoFile) + std::string("iso.md5");
    std::string tgtMd5;

    ATC_STREAM_LOGI() << "packname : " << isoFile << " md5File : " << md5File << std::endl;
    md5 = HashUtil::md5Str(isoFile.c_str(), 0, 0);
    if (md5.empty()) {
        ATC_STREAM_LOGE() << "calculate md5sum failed!" << std::endl;
        return false;
    }

    std::ifstream fin(md5File);
    if (!fin) {
        ATC_STREAM_LOGE() << "failed to open : " << md5File << std::endl;
        return false;
    }
    fin >> tgtMd5;
    if (tgtMd5 != md5) {
        ATC_STREAM_LOGE() << "md5sum not match! expect : " << tgtMd5 << " actual : "
                          << md5 << std::endl;
        return false;
    }
    return true;
}

static bool dumpImageFromZip(const std::string& imageName, uint64_t fileLen, const std::string& dumpDir) {
    std::string dumpPath = dumpDir + "dump_" + imageName;
    auto zipFile = atcupdateservice::utils::File::Create(
        const_cast<char*>(imageName.c_str()),
        atcupdateservice::utils::File::READ);
    if (!zipFile || !zipFile->openZipEntry()) {
        ATC_STREAM_LOGE() << "dumpImageFromZip: failed to reopen ZIP for " << imageName << std::endl;
        return false;
    }

    int dumpFd = open(dumpPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dumpFd < 0) {
        ATC_STREAM_LOGE() << "dumpImageFromZip: failed to create " << dumpPath
                          << ", errno=" << errno << std::endl;
        zipFile->closeZipEntry();
        return false;
    }

    char readBuf[512];
    int rc;
    uint64_t dumpTotal = 0;
    while ((rc = mz_zip_reader_entry_read(zipFile->getZipReader(), readBuf, sizeof(readBuf))) > 0) {
        write(dumpFd, readBuf, rc);
        dumpTotal += rc;
    }
    fsync(dumpFd);
    close(dumpFd);
    zipFile->closeZipEntry();

    ATC_STREAM_LOGE() << "dumpImageFromZip: dumped " << dumpTotal << " bytes to " << dumpPath
                      << " (expect " << fileLen << ")"
                      << (dumpTotal == fileLen ? " [OK]" : " [SIZE MISMATCH]") << std::endl;
    return (dumpTotal == fileLen);
}

bool verifyUpdateImages(std::string isoMountPoint, const std::vector<PartInfo::ptr> &parts, int diff_flag) {
    if (isoMountPoint[isoMountPoint.size() - 1] != '/') {
        isoMountPoint += "/";
    }
    std::string md5File = isoMountPoint + std::string("image.md5");
    std::map<std::string ,std::string> imageMd5;
    std::string line;
    if (diff_flag == 1) {
        std::ifstream fin(md5File);
        while (std::getline(fin, line)) {
            std::istringstream iss(line);
            std::string md5, image;
            iss >> md5 >> image;
            imageMd5.insert(std::make_pair(image, md5));
        }
    } else { 
            // Use the same helper function
            imageMd5 = parseMd5ImageFile(md5File);
    }
    utils::dropSystemCaches();  // 清空内核 page cache，消除缓存复用
    ATCLOGI("dropped system caches before image verification");

    for (PartInfo::ptr part: parts) {
        auto iter = imageMd5.find(part->image);
        if (iter == imageMd5.end()) {
            ATC_STREAM_LOGE() << "no md5 record for image: " << part->image
                << " in Image.md5 file" << std::endl;
            return false;
        }

        std::string imageMd5 = iter->second;
        std::string md5;

        if (diff_flag == 1) { 
            std::string imagePath = isoMountPoint + iter->first;
            if (access(imagePath.c_str(), F_OK) != 0) {
                ATC_STREAM_LOGE() << "no such image: " << imagePath << std::endl;
                return false;
            }
            md5 = HashUtil::md5Str(imagePath.c_str(), part->fileLen, 1);
            if (md5 != imageMd5) {
                ATC_STREAM_LOGE() << "image: " << imagePath << " verify failed, expect md5: "
                    << imageMd5 << ", actual md5: " << md5  << " diff_flag: " << diff_flag << std::endl;
                return false;
            }
        } else { 
            md5 = HashUtil::md5Str((iter->first).c_str(), part->fileLen, 0);
            if (md5 != imageMd5) {
                ATC_STREAM_LOGE() << "image: " << iter->first << " verify failed, expect md5: "
                    << imageMd5 << ", actual md5: " << md5  << " diff_flag: " << diff_flag
                    << ", fileLen: " << part->fileLen << std::endl;

                dumpImageFromZip(iter->first, part->fileLen, "/data/OTAupdate/");
#ifdef STRESS_ZIP_READ
                return false;
#endif
                bool retryPassed = false;
                for (int retry = 1; retry <= 3; ++retry) {
                    utils::dropSystemCaches();
                    ATCLOGI("dropped system caches before retry verification");
                    std::string retry_md5 = HashUtil::md5Str((iter->first).c_str(), part->fileLen, 0);
                    ATC_STREAM_LOGE() << "retry[" << retry << "/3] md5: " << retry_md5
                          << (retry_md5 == imageMd5 ? " [OK]" : " [FAILED]") << std::endl;
                    if (retry_md5 == imageMd5) {
                        retryPassed = true;
                        break;
                    }
                }
                if (!retryPassed) {
                    ATC_STREAM_LOGE() << "image: " << iter->first << " verify failed after 3 retries" << std::endl;
                    return false;
                }

            }
        }
    }

    return true;
}

static bool readHeader(PartInfo::ptr info) {
    SparseHeader sparseHeader;
    size_t size;
    bool rt = false;

    std::string imagepath = std::string(ATC_ISO_MOUNTPOINT) +  info->image;
    int fd = open(imagepath.c_str(), O_RDONLY | O_LARGEFILE);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "failed to open " <<  imagepath
                        << " error : " << strerror(errno) <<", May be support zip-update-file" << std::endl;
        goto zip_parse;
    }
    info->fileLen = lseek64(fd, 0, SEEK_END);
    if (info->fileLen <= 0) {
        ATC_STREAM_LOGE() << "failed to lseek64, error : " << strerror(errno) << std::endl;
        goto out;
    }
    if (info->raw == false) {
        lseek64(fd, 0, SEEK_SET);
        size = FSUtil::readFixFd(fd, (char *)&sparseHeader, 0, sizeof(SparseHeader));
        if (size != sizeof(SparseHeader)) {
            ATC_STREAM_LOGE() << "expect : " << sizeof(SparseHeader)
                            << " actual : " << size << std::endl;
            goto out;
        }
        info->blockSize = sparseHeader.blk_sz;
        info->chkCount = sparseHeader.total_chunks;
    }
    rt = true;
    out:
    if (fd >= 0)
        close(fd);
    return rt;

zip_parse:
    void *zip_reader = NULL;
    mz_zip_file *file_info = NULL;
    const char *extract_file_name = NULL;
    char* file = (char *)info->image.c_str();

    // Using Minizip to process ZIP files
    ATC_STREAM_LOGI() << "readheader: go to zip parser update-file" << std::endl;
    zip_reader = mz_zip_reader_create();
    if (zip_reader == NULL) {
        ATC_STREAM_LOGE() << "Failed to create zip reader." << std::endl;
        return false;
    }

    if (mz_zip_reader_open_file(zip_reader, RECOVERY_UPDATE_ZIP_NAME) != MZ_OK) {
        ATC_STREAM_LOGE() << "Failed to open zip file: " << RECOVERY_UPDATE_ZIP_NAME << std::endl;
        mz_zip_reader_delete(&zip_reader);
        return false;
    }

    if (mz_zip_reader_is_open(zip_reader) != MZ_OK){
        ATC_STREAM_LOGE() << "mz_zip_reader_is_open, Failed to open zip file" << std::endl;
        mz_zip_reader_close(zip_reader);
        mz_zip_reader_delete(&zip_reader);
        return false;
    }

    // Locate the file to extract
    if (mz_zip_reader_locate_entry(zip_reader, file, 1) == MZ_OK) {
        if (mz_zip_reader_entry_get_info(zip_reader, &file_info) == MZ_OK) {
            extract_file_name = file_info->filename;
            ATC_STREAM_LOGI() << "file_len:" << file_info->uncompressed_size << std::endl;
            info->fileLen = file_info->uncompressed_size;
        }
    }
    mz_zip_reader_close(zip_reader);
    mz_zip_reader_delete(&zip_reader);
    return true;


}

bool getUpdateParts(const std::string &xml, std::vector<PartInfo::ptr> &parts) {
    parts.clear();
#ifdef CONFIG_EMMC_BOOT
    gpt::GPT::ptr emmcGpt(new gpt::GPT());
#endif
    gpt::GPT::ptr xmlGpt(new gpt::GPT(xml));
    std::vector<std::string> images;
#ifdef CONFIG_EMMC_BOOT
    if (emmcGpt->updatable(xmlGpt) == false) {
        return false;
    }
#endif
    parts = xmlGpt->getABUpdateSet();
    if (parts.empty()) {
        return false;
    }
#ifdef CONFIG_NAND_BOOT
    gXmlPart = xmlGpt->getPartnameFromXml();
    for (const auto& name : gXmlPart) {
        ATC_STREAM_LOGI() << "++++ partname " << name << " ++++" << std::endl;
    }
#endif

    sort(parts.begin(), parts.end(), gpt::GPTEntryComparor());
    utils::FSUtil::listAllFile(ATC_ISO_MOUNTPOINT, images);
    sort(images.begin(),images.end());
    std::vector<PartInfo::ptr> validParts;
    for (auto &item: parts) {
        std::string image = item->image;
        std::string partition = item->part;
        if (std::binary_search(images.begin(), images.end(), image) == false) {
            if (partition == "preloader" || partition == "preloader_bk") {
                ATC_STREAM_LOGW() << "preloader image not found, skipping preloader update only" << std::endl;
                continue;
            } else {
                ATC_STREAM_LOGE() << "partition: " << partition << "image : " << image << " not exist" << std::endl;
                return false;
            }
        }
        if (readHeader(item) == false) {
            ATC_STREAM_LOGE() << "failed to read header" << std::endl;
            return false;
        }
        if (item->raw) {
            ATC_STREAM_LOGI() << "image : " << image << " part : " << item->part
                              << " partsize: " << item->partsize  << " fileLen : " << item->fileLen
                              << " raw : " << item->raw << std::endl;
        } else {
            ATC_STREAM_LOGI() << "image : " << image << " part : " << item->part
                              << " partsize: " << item->partsize  << " fileLen : " << item->fileLen
                              << " raw : " << item->raw << " chkCount : " << item->chkCount
                              << " blockSize : " << item->blockSize << std::endl;
        }
        validParts.push_back(item);
    }
    parts = std::move(validParts);
    return true;
}

int checkProcExistance(const char *pname) {
    std::string cmd = "pidof " + std::string(pname);
    int pid;
    FILE *fp;
    int ret = 0;

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        ATC_STREAM_LOGE() << "popen failed: " << strerror(errno) << std::endl;
        throw std::logic_error("checkProcExistance: popen failed!");
    }

    ret = fscanf(fp, "%d", &pid) > 0;
    if (ret == 0) {
        ATC_STREAM_LOGW() << "proc: " << pname << " not exist!" << std::endl;
        pid = -1;
    } else if (ret < 0) {
        ATC_STREAM_LOGW() << "fscanf failed!, error: " << strerror(errno) << std::endl;
        throw std::logic_error("checkProcExistance: fscanf failed!");
    }

    pclose(fp);
    return pid;
}

uint64_t getCurrentTimeMs() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return (spec.tv_sec * 1000 + spec.tv_nsec / (1000 * 1000));
}

uint64_t getElapseTimeMs() {
    static uint64_t first = getCurrentTimeMs();
    return getCurrentTimeMs() - first;
}


static void trimString(std::string &s) {
    while(!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
    while(!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
}

bool getUpdateConfigValue(const std::string &key, std::string &value) {
    const std::string updateFile = "/data/update_config";
    std::ifstream ifs(updateFile);

    if (!ifs) {
        ATC_STREAM_LOGI() << "no update_config file or open failed" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        trimString(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }

        std::string cfgKey = line.substr(0, eqPos);
        std::string cfgValue = line.substr(eqPos + 1);
        trimString(cfgKey);
        trimString(cfgValue);

        if (cfgKey == key) {
            value = cfgValue;
            return !value.empty();
        }
    }

    return false;
}

bool getUpdateConfigFlag(const std::string &key) {
    std::string value;
    if (!getUpdateConfigValue(key, value)) {
        return false;
    }
    return value == "1" || value == "true" || value == "TRUE";
}

void removeUpdateConfigOnFailure() {
    const char *cfgPath = "/data/update_config";
    if (unlink(cfgPath) == 0) {
        ATC_STREAM_LOGI() << "remove update_config success" << std::endl;
    } else if (errno == ENOENT) {
        ATC_STREAM_LOGI() << "update_config not found, skip remove" << std::endl;
    } else {
        ATC_STREAM_LOGW() << "remove update_config failed, error: "
                          << strerror(errno) << std::endl;
    }
}

}
}
