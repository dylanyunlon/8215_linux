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

#ifndef ATC_UTILITY_HPP
#define ATC_UTILITY_HPP

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <vector>
#include <list>
#include <memory>
#include <string.h>
#include <functional>
#include <optional>
#include <variant>
#include "utils/CRC32.hpp"

#include "utils/macro.hpp"

#ifdef CONFIG_NAND_BOOT
#include "libmtd.h"
#endif

namespace atcupdateservice {
namespace utils {

#define WP_CLEAR_AND_SAVE (0X66)
#define WP_RESTORE (0X77)

#define DISABLE_ALL_WP          _IOW('r', 13, int)
#define MSDC_EMMC_WRITE_PROTECT _IOW('r', 12, int)

#define MININUM_READ_SIZE_RAW        (4 * STD_MB)

#define WP_DEV                  "/proc/msdc_debug"
#ifdef CONFIG_EMMC_BOOT
#define SCATTER_FILE_NAME       "scatter.mmcboot.ext4.xml"
#else
#define SCATTER_FILE_NAME       "scatter.nand.ext4.xml"
#endif

#define VERSION_PRE        "pre-version"
#define VERSION_POST        "post-device"
#define VERSION_OLD         "/data/misc/version_old"
#define VERSION_FILE        "/etc/version"
#define MS_TO_SEC               (1000)
#define SPARSE_HEADER_MAGIC	    0xed26ff3a

#define BLOCK_DEV_PATH          "/dev/"
#define NAND_MTD_PATH          "/dev/mtd"

#define RESERVED_SECTOR_IN_BLOCK        0xFFFFFA

#define STD_KB          (1024)
#define STD_MB          (1024 * STD_KB)

#define SECTOR_SIZE     (512)

#ifndef BLOCK_SIZE
#define BLOCK_SIZE      (4096)
#endif

#ifndef ALIGN
#define ALIGN(a, b)     ((a) + (b) - 1) & ~((b) - 1)
#endif


#if BYTE_ORDER == LITTLE_ENDIAN

struct ChunkHeader {
  uint16_t	chunk_type;	/* 0xCAC1 -> raw; 0xCAC2 -> fill; 0xCAC3 -> don't care */
  uint16_t	reserved1;
  uint32_t	chunk_sz;	/* in blocks in output image */
  uint32_t	total_sz;	/* in bytes of chunk input file including chunk header and data */
};

struct SparseHeader {
  uint32_t	magic;		/* 0xed26ff3a */
  uint16_t	major_version;	/* (0x1) - reject images with higher major versions */
  uint16_t	minor_version;	/* (0x0) - allow images with higer minor versions */
  uint16_t	file_hdr_sz;	/* 28 bytes for first revision of the file format */
  uint16_t	chunk_hdr_sz;	/* 12 bytes for first revision of the file format */
  uint32_t	blk_sz;		/* block size in bytes, must be a multiple of 4 (4096) */
  uint32_t	total_blks;	/* total blocks in the non-sparse output image */
  uint32_t	total_chunks;	/* total chunks in the sparse input image */
  uint32_t	image_checksum; /* CRC32 checksum of the original data, counting "don't care" */
				/* as 0. Standard 802.3 polynomial, use a Public Domain */
				/* table implementation */
};

#endif
#ifndef GUID_SIZE
#define GUID_SIZE               16
#endif

struct PartInfo {
    typedef std::shared_ptr<PartInfo> ptr;
    std::string part;
    std::string image;
    ssize_t partsize;
    uint32_t flag;
    uint32_t mount;
    uint64_t partitionStartAddr;
    bool raw;
    ssize_t chkCount;
    ssize_t fileLen;
    ssize_t blockSize;
    SparseHeader *header = nullptr;
};

struct CheckPoint {
    uint32_t thisChkSum;
    uint32_t valid;
    uint64_t timestamp;
    char  md5[64];
    uint32_t lastWriteSize;
    uint32_t lastChkSum;
    uint64_t offsetRead;
    uint64_t offsetWrite;
    char partname[256];
    char imagename[256];
};

enum class ChunkType {
    NONE = 0,
    RAW = 0xcac1,
    FILL = 0xcac2,
    DONT_CARE = 0xcac3,
};

struct HashUtil {
    static char decToHex(uint8_t);
    static bool md5Sum_ota_full(const char *ifname, uint8_t md5_out[16], uint64_t size);
    static bool md5Sum_ota_diff(const char *ifname, uint8_t md5_out[16], uint64_t size);
    static uint32_t checkSum32(uint32_t lastSum, const char *buf, unsigned len);
    static std::string md5Str(const char *filename, uint64_t size, int board_data_flag);
    static bool md5BoardSum(const char *filename, uint8_t md5[16], uint64_t offset, uint64_t size);
    static std::string md5BoardStr(const char *filename, uint64_t offset, uint64_t size);
    static std::string md5Array2Str(uint8_t md5[16]);
    static bool md5Compare(uint8_t md5A[16], uint8_t md5B[16]);
    static uint32_t Crc32(uint8_t *p, uint32_t len) {
        return (crc32_no_comp(~0L, p, len) ^ ~0L);
    }
};

// thread not safety not guaranteed
struct FSUtil {
    static ssize_t readFix(const char *filename, char *buf, ssize_t offset, ssize_t size);
#ifdef CONFIG_NAND_BOOT
    static ssize_t nandReadRaw(struct mtd_dev_info *mtd, const char* mtdname, char *buf, ssize_t offset, ssize_t size);
    static ssize_t nandWriteRaw(libmtd_t mtdDesc, struct mtd_dev_info *mtd, const char* mtdname, char *buf, ssize_t offset, ssize_t size);
#endif
    static ssize_t readFixFd(int fd, char *buf, ssize_t offset, ssize_t size);
    static ssize_t writeFix(const char *filename, const char *buf, ssize_t offset, ssize_t size);
    static ssize_t writeFixFd(int fd, const char *buf, ssize_t offset, ssize_t size);
    static uint64_t getPartitionSize(const std::string &devName);

    static bool ISOMount(const std::string &isoFile, const std::string &mountPoint);
    static bool ISOUmount(const std::string &mountPoint);
    static bool loadPartitionInfo(const std::string &xml, std::map<std::string, PartInfo> &infos);
    static ssize_t writePartition(const std::string &devname, ssize_t &offset,
                               const char *buff, ssize_t buffSize, ssize_t partSize);
    static bool listAllFile(std::string path, std::vector<std::string> &files);
    static bool disableWriteProtect();
    static bool enableWriteProtect();
    static bool parseCheckPoint(CheckPoint &cp);
    static bool initCheckPoint();
    static bool deInitCheckPoint();
    static bool writeCheckPoint(const CheckPoint &cp);
    static bool removeCheckPointFile();
    static bool removeCheckPoint();
    static bool clearCheckPoint();
    static std::shared_ptr<char> loadOneExt4Chunk(const std::string &imagename, ssize_t &offset,
                                               ssize_t &datalen, uint32_t blkSz, bool &dontCare);
    static std::shared_ptr<char> loadRaw(const std::string &imagename, ssize_t &offset,
                                         ssize_t expect, ssize_t &actual);
    static std::string updateSlot();
    static bool setUpdateSlotUnbootable();
    static bool setUpdateSlotActive();
    static bool checkUpdateSlotValid();
    static std::string getPackageDiffInfo(const std::string &filenam, std::string targetname);

    static bool verifyUpdatePackage(const std::string &isoFile, std::string &md5);
    static std::string getCurentSlotPart(std::string &part_name);
    static bool isUpdatePart(const std::string &part) {
        if (part.find(updateSlot()) == std::string::npos) {
            return false;
        }
        return true;
    }
    static std::string getUpperDir(std::string dir) {
        if (dir[dir.size() - 1] == '/') {
            dir.resize(dir.size() - 1);
        }
        for (int i = dir.size() - 1; i >= 0; --i) {
            if (dir[i] == '/') {
                dir.resize(i+1);
                break;
            }
        }
        return dir;
    }
};

void dumpMemory(const uint8_t *data, uint32_t size);
int  checkProcExistance(const char *pname);
uint64_t getCurrentTimeMs();
uint64_t getElapseTimeMs();
bool verifyBasePartition(std::string isoMountPoint, const std::vector<PartInfo::ptr> &parts);
bool getUpdateParts(const std::string &xml, std::vector<PartInfo::ptr> &parts);
bool verifyUpdateImages(std::string isoMountPoint, const std::vector<PartInfo::ptr> &parts, int diff_flag);
bool verifyUpdatedPartition(std::string isoMountPoint, const PartInfo::ptr &part);
bool readFromZipFile(const std::string& filename,
    const std::function<bool(std::istringstream&)>& processor);
bool getUpdateConfigValue(const std::string &key, std::string &value);
bool getUpdateConfigFlag(const std::string &key);
#ifdef CONFIG_NAND_BOOT
int getPartIdxFromXml(const char* partname);
#endif
void removeUpdateConfigOnFailure();
}
}

template<class T, class E>
class Result {
public:
    static Result<T,E> Ok(T v) {
        Result result;
        result.ok = v;
        result.m_result.m_isOk = true;
    }
    static Result<T,E> Err(E v) {
        Result result;
        result.ok = v;
        result.m_result.m_isOk = false;
    }
    T unwrapOrElse(std::function<T(const E&)> f) const{
        if (isOk()) {
            return m_result.ok;
        } else {
            return f(m_result.err);
        }
    }
    T unwrapOr(T v) const {
        if (isOk()) {
            return m_result.ok;
        } else {
            return v;
        }
    }
    // UB if called without check
    T unwarp() const {
        return m_result.ok;
    }
    T unwrapOrDefault() const {
        if (isOk()) {
            return m_result.ok;
        }
    }
    bool isOk() const {
        return m_isOk;
    }
    bool isErr() const {
        return !m_isOk;
    }
    Result<T,E>& inspect(std::function<void(const T&)> f) const {
        if (isOk()) {
            f(m_result.ok);
        }
        return *this;
    }
    Result<T,E>& inspectErr(std::function<void(const E&)> f) const {
        if (isErr()) {
            f(m_result.ok);
        }
        return *this;
    }
    // UB if called without check
    E unwrapErr() const {
        return m_result.err;
    }
    std::optional<T> ok() {
        if (isOk()) {
            return std::optional<T>(m_result.ok);
        } else {
            return std::nullopt;
        }
    }
    std::optional<T> err() {
        if (!isOk()) {
            return std::optional<T>(m_result.err);
        } else {
            return std::nullopt;
        }
    }
private:
    Result();
private:
    union {
        T ok;
        E err;
    } m_result;
    bool m_isOk;
};

#endif