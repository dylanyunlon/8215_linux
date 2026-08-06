#include <set>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "utils/Image.hpp"
#include "utils/Util.hpp"
#include "utils/File.hpp"

#ifdef  BOARD_AVB_ENABLE
#include "utils/CryptoImage.hpp"
#endif
namespace atcupdateservice {
namespace utils {

Image::Image(std::string partname, std::string imagename, uint32_t bufSize)
    : m_partname(partname),
      m_imagename(imagename) {
    if (m_partname.empty() || m_imagename.empty()) {
        throw std::logic_error("partname or imagename is empty!");
    }
    if (bufSize == 0) {
        m_bufSize = BUFFER_SIZE;
    } else {
        m_bufSize = bufSize;
    }
}

bool Image::open(int64_t writePos, int64_t readPos) {
    if (m_image || m_part) {
        ATC_STREAM_LOGI() << "already be opened!" << std::endl;
        return true;
    }

    std::string partPath = std::string(BLOCK_DEV_PATH) + m_partname;
    std::string imagePath = std::string(ATC_ISO_MOUNTPOINT) + m_imagename;
    std::string versionFile = std::string(ATC_ISO_MOUNTPOINT) + "version";

    m_image = File::Create(imagePath, File::READ);

#ifdef CONFIG_NAND_BOOT
    m_part = NandFile::Create(partPath, File::NAND_WRITE);
#else
    m_part = File::Create(partPath, File::WRITE);
#endif

    if (m_image == nullptr || m_part == nullptr) {
        ATC_STREAM_LOGE() << "failed to open image or partition block device" << std::endl;
        return false;
    }
    if(utils::FSUtil::getPackageDiffInfo(versionFile, "package") == "diff"){
        m_imageSize = m_image->size();
    } else {
        m_imageSize = m_image->img_size(m_imagename);
    }

    if (readPos > m_imageSize) {
        return false;
    }
    m_partSize = m_part->size();
    m_image->setReadOffset(readPos);
    if (writePos > m_partSize) {
        ATC_STREAM_LOGE() << "writePos over than partsize" << std::endl;
        return false;
    }
    if (m_part->setWriteOffset(writePos) < 0) {
        ATC_STREAM_LOGE() << "setWriteOffset failed" << std::endl;
        return false;
    }

    return true;
}

#ifdef BOARD_AVB_ENABLE
bool Image::needCrypto(const std::string &part) {
    static std::set<std::string> cryptoImage = {
        "hsm_a", "hsm_b", "lk_a", "lk_b", "viss_a",
        "viss_b", "trustzone_a", "trustzone_b"
    };
    auto iter = cryptoImage.find(part);
    if (iter == cryptoImage.end()) {
        ATC_STREAM_LOGI() << "partition: " << part << " don't need to verify and crypto!" << std::endl;
        return false;
    } else {
        ATC_STREAM_LOGI() << "partition: " << part << " need to verify and crypto!" << std::endl;
        return true;
    }
}
#endif

//thread safety is not guaranteed!
bool Image::writeCheckPoint(const std::string &md5) const {
    if (m_part == nullptr || m_image == nullptr) {
        ATC_STREAM_LOGE() << "image or partition not open yet!" << std::endl;
        return false;
    }
    static bool onceFlag = true;
    static CheckPoint cp;
    if (onceFlag) {
        onceFlag = false;
        memset(&cp, 0, sizeof(cp));
    }
    strncpy(cp.partname, basename(m_partname.c_str()), 255);
    strncpy(cp.imagename, basename(m_imagename.c_str()), 255);
    cp.valid = true;
    cp.timestamp++;
    cp.offsetRead = m_image->getReadOffset();
    cp.offsetWrite = m_part->getWriteOffset();
    strncpy(cp.md5, md5.c_str(), 32);
    cp.thisChkSum = HashUtil::checkSum32(0, (char *)&cp + sizeof(uint32_t),
        sizeof(cp) - sizeof(uint32_t));
    FSUtil::writeCheckPoint(cp);
    return true;
}

Image::ptr Image::createImage(std::string partname, std::string imagename, bool raw, uint32_t bufSize) {
    Image::ptr image = nullptr;

    if (raw) {
#ifdef BOARD_AVB_ENABLE
        if (needCrypto(partname)) {
            image.reset(new CryptoImage(partname, imagename));
        } else {
#endif
            image.reset(new RawImage(partname, imagename, bufSize));
#ifdef BOARD_AVB_ENABLE
        }
#endif
    } else {
        image.reset(new SparseImage(partname, imagename, bufSize));
    }
    return image;
}

SparseImage::SparseImage(std::string partname, std::string imagename, uint32_t bufSize)
    : Image(partname, imagename, bufSize),
      m_chunkCnt(0),
      m_blkCnt(0),
      m_blkSize(0) {
}

bool SparseImage::open(int64_t writePos, int64_t readPos) {
    std::string versionFile = std::string(ATC_ISO_MOUNTPOINT) + "version";
    if (Image::open(writePos, readPos) == false) {
        return false;
    }

    if(utils::FSUtil::getPackageDiffInfo(versionFile, "package") != "diff") {
        ATC_STREAM_LOGI() << "package is not diff, need check header" << std::endl;
        struct SparseHeader *header = nullptr;
        char buf[sizeof(SparseHeader)] = {0};
        int64_t imageOff = m_image->getReadOffset();
        if (readPos != 0) {
            m_image->setReadOffset(0);
        }
        int ret = m_image->readFix(sizeof(SparseHeader), buf);

        if (ret == -1 ) {
            ATC_STREAM_LOGE() << "failed to read sparse header" << std::endl;
            return false;
        }
        header = (struct SparseHeader *)(buf);
        if (header->magic != SPARSE_HEADER_MAGIC) {
            ATC_STREAM_LOGE() << "magic not match!" << std::endl;
            return false;
        }

        m_chunkCnt = header->total_chunks;
        m_blkCnt = header->total_blks;
        m_blkSize = header->blk_sz;
        if (readPos != 0) {
            m_image->setReadOffset(imageOff);
        }
    } else {
        ATC_STREAM_LOGI() << "package is diff, not need check header" << std::endl;
    }

    return true;
}

uint32_t SparseImage::getOutputSize() const {
    return m_blkCnt * m_blkSize;
}

bool SparseImage::writePartition(char *buf) {
    if (m_image == nullptr || m_part == nullptr) {
        ATC_STREAM_LOGE() << "partition or image not open yet!" << std::endl;
        return false;
    }
    assert(m_bufSize != NO_LIMIT);
    struct ChunkHeader *chunkHeader = nullptr;
    int ret = m_image->readFix(sizeof(ChunkHeader), buf);

    if (ret == -1) {
        ATC_STREAM_LOGE() << "failed to read chunk header, m_image : " << m_image << std::endl;
        return false;
    }
    chunkHeader = (struct ChunkHeader *)buf;
    uint32_t datalen = chunkHeader->chunk_sz * m_blkSize;
    ChunkType type = (ChunkType)chunkHeader->chunk_type;
    if (ChunkType::RAW == type) {
        while(datalen) {
            uint32_t nread = std::min(datalen, m_bufSize);
            ret = m_image->readFix(nread, buf);
            if (ret == -1) {
                ATC_STREAM_LOGE() << "failed to read image :" << m_image->getName() << std::endl;
                return false;
            }
            int64_t rt = m_part->writeFix(buf, nread);
            if (rt == -1 || nread != rt) {
                ATC_STREAM_LOGE() << "failed to read part :" << m_part->getName() << std::endl;
                return false;
            }
            datalen -= nread;
        }
    } else if (ChunkType::DONT_CARE == type) {
        int64_t readOffset  = m_image->getReadOffset();
        int64_t writeOffset = m_part->getWriteOffset();
        m_image->setReadOffset(readOffset + (chunkHeader->total_sz - sizeof(ChunkHeader)));
        m_part->setWriteOffset(writeOffset + datalen);
    } else if (ChunkType::FILL == type) {
        ATC_STREAM_LOGW() << "fill type not supported" << std::endl;
        return false;
    } else {
        ATC_STREAM_LOGE() << "unknown trunk type : 0x" << std::hex << (unsigned)(type) << std::endl;
        return false;
    }

    return true;
}

RawImage::RawImage(std::string partname, std::string imagename, uint32_t bufSize)
    : Image(partname, imagename, bufSize) {
}

static uint32_t  Log2(uint32_t value)
{
	uint32_t rc = 0;

	while ( 0 != value )
	{
		value >>= 1;
		rc++;
	}
	rc--;
	return rc;
}

static uint32_t readNandTotalBlocks(int eb_size) {
    uint32_t default_total_blocks = 0x800;
    char line[32] = {0};
    FILE *fp = popen("cat /proc/nand_size", "r");
    if (fp) {
        if (fgets(line, sizeof(line), fp) != NULL) {
            std::string s(line);
            size_t pos = s.find_first_of("\r\n");
            if (pos != std::string::npos) s.erase(pos);
            unsigned int parsed = 0;
            if (sscanf(s.c_str(), "%u", &parsed) == 1 && parsed > 0) {
                printf("nandallsize:%d\n", parsed);
                pclose(fp);
                return parsed / eb_size;
            }
        }
        pclose(fp);
    }
    return default_total_blocks;
}

int RawImage::createBootloaderHeader(unsigned char *pBLHeader, 
                                    char* blbuf, 
                                    uint32_t imageSize, 
                                    bool msdc_boot, 
                                    int mini_io_size, 
                                    int oob_size,
                                    int eb_size) {
    BOOTL_HEADER BLHeader;
    uint32_t chksum = 0, temp32;
    unsigned char *dataBuf = (unsigned char*)blbuf;
    uint32_t i;

    // Initialize header structure
    memset(&BLHeader, 0, sizeof(BOOTL_HEADER));
    // Set ID strings
    memcpy(BLHeader.ID1, "BOOTLOADER!", 12);
    if (msdc_boot) {
        memcpy(BLHeader.ID2, "MT3360A", 8);
    } else {
        memcpy(BLHeader.ID2, "NFIINFO", 8);
    }
    // Set basic parameters
    BLHeader.startAddr = 0x40000000;  // Default start address
    BLHeader.length = imageSize;

    // Set NFI-specific parameters if not MSDC boot
    if (!msdc_boot) {
        // These values would typically come from actual NAND configuration
        BLHeader.NFIinfo.pageSize = mini_io_size;     // Typical page size - adjust as needed
        BLHeader.pagesPerBlock = eb_size / mini_io_size;         // Typical pages per block
        BLHeader.totalBlocks =  readNandTotalBlocks(eb_size);         // Total number of blocks
        BLHeader.NFIinfo.spareSize = oob_size;      // OOB size - adjust as needed
        printf("oob size[%d], pagesPerBlock[%d], totalBlocks[%d]\n", BLHeader.NFIinfo.spareSize, BLHeader.pagesPerBlock, BLHeader.totalBlocks);
        BLHeader.NFIinfo.addressCycle = 0x5;  // Address cycles
        
        if (BLHeader.NFIinfo.pageSize > 512) {
            BLHeader.NFIinfo.pageShift = 0x10;
        } else {
            BLHeader.NFIinfo.pageShift = 0x8;
        }

        BLHeader.blockShift = (Log2(BLHeader.pagesPerBlock) + BLHeader.NFIinfo.pageShift);
    }
    chksum = 0;
    for (i = 0; i < imageSize; i += 4) {
        memcpy(&temp32, ((unsigned char *)dataBuf + i), 4);
        chksum ^= temp32;
    }

    BLHeader.checksum = chksum;

    // Write replicated headers
    for (i = 0; i < REPLICATION_NUMBER; i++) {
        memcpy(pBLHeader, &BLHeader, sizeof(BOOTL_HEADER));
        pBLHeader = (unsigned char *)pBLHeader + sizeof(BOOTL_HEADER);
    }

    return 0;  // Success

}

bool RawImage::writePreloaderPartition(char * buf) {
    const int HEADER_SIZE = 512;
    int loader_size = 0x7000; // size from uboot
    int  alignsize = ALIGN(m_imageSize + 512 ,  m_part->getNandIoSize());

    char *imgbuf = buf;
    char *headebuf = imgbuf;
    char *databuf = imgbuf + HEADER_SIZE;

#ifdef USE_NO_ZIPFILE
    int ret  = m_image->readFix(m_imageSize, databuf);
#else
    int ret  = m_image->readFixFromZip(m_imagename , m_imageSize, databuf);
#endif
    if (ret == -1) {
        ATC_STREAM_LOGE() << "failed to read image :" << m_imagename << std::endl;
        return false;
    }

    createBootloaderHeader((unsigned char *)headebuf, databuf, loader_size, 0, m_part->getNandIoSize(), m_part->getNandOobSize(), m_part->getNandEbSize());

    ret = m_part->writeFix(imgbuf, alignsize);
    if (ret < 0) {
        ATC_STREAM_LOGE() << "failed to write part :" << m_partname << std::endl;
        return false;
    }
    ATC_STREAM_LOGI() << "success to write part :" << m_partname << std::endl;
    return true;
}

bool RawImage::writeNormalPartition(char * buf) {
    int64_t nread = 0;
    int64_t rt = 0;
    assert(m_bufSize != NO_LIMIT);

    nread = std::min((int64_t)m_bufSize, m_imageSize - m_image->getReadOffset());
    if (nread == 0) {
        return true;
    }
#ifdef USE_NO_ZIPFILE
    int ret  = m_image->readFix(nread, buf);
#else
    int ret  = m_image->readFixFromZip(m_imagename , nread, buf);
#endif
    if (ret == -1) {
        ATC_STREAM_LOGE() << "failed to read image :" << m_imagename << std::endl;
        return false;
    }

#ifdef CONFIG_NAND_BOOT
    if (m_partname == "system_a" || m_partname == "system_b") {
        rt = m_part->writeExt(buf, nread);
    } else {
        rt = m_part->writeFix(buf, nread);
    }
    if (rt < 0) {
        ATC_STREAM_LOGE() << "failed to write part :" << m_partname << std::endl;
        return false;
    }
#else
    rt = m_part->writeFix(buf, nread);
    if (rt != nread) {
        ATC_STREAM_LOGE() << "failed to write part :" << m_partname << std::endl;
        return false;
    }
#endif

    return true;
}

bool RawImage::writePartition(char * buf) {
    bool isPreloader = (m_partname == "preloader" || m_partname == "preloader_bk");
    if (isPreloader) {
        return writePreloaderPartition(buf);
    } else {
        return writeNormalPartition(buf);
    }
}

}
}
