#pragma once

#include "utils/Util.hpp"
#include "utils/File.hpp"
#include <string>
#include <memory>

namespace atcupdateservice {
namespace utils {

#define BUFFER_SIZE (64*1024)
#define NO_LIMIT    ((uint32_t)-1)

class Image {
public:
    typedef std::shared_ptr<Image> ptr;
    Image(std::string partname, std::string imagename, uint32_t bufSize = BUFFER_SIZE);
    virtual bool writePartition(char *buf) = 0;
    bool writeCheckPoint(const std::string &md5) const;
    virtual ~Image() {
    }
    virtual bool open(int64_t writePos = 0, int64_t readPos = 0);
    static Image::ptr createImage(std::string partname, std::string imagename, bool raw, uint32_t bufSize = BUFFER_SIZE);
#ifdef BOARD_AVB_ENABLE
    static bool needCrypto(const std::string &part);
#endif
    virtual int64_t getPartOffset() const {
        return m_part->getWriteOffset();
    }
    virtual int64_t getImageOffset() const {
        return m_image->getReadOffset();
    }
    const std::string &getPartName() const {
        return m_partname;
    }
    const std::string &getImageName() const {
        return m_imagename;
    }
    int64_t getPartSize() const {
        return m_partSize;
    }
    int64_t getImageSize() const {
        return m_imageSize;
    }
    virtual bool finished() const {
        return (m_image->getReadOffset() >= m_imageSize) ||
               (m_part->getWriteOffset() >= m_partSize);
    }
    virtual uint32_t getOutputSize() const {
        return m_imageSize;
    }
protected:
    File::ptr m_part;
    File::ptr m_image;
    std::string m_partname;
    std::string m_imagename;
    uint32_t m_bufSize = 0;
    int64_t m_partSize = 0;
    int64_t m_imageSize = 0;
};

class SparseImage : public Image {
public:
    typedef std::shared_ptr<SparseImage> ptr;
    SparseImage(std::string partname, std::string imagename, uint32_t bufSize = BUFFER_SIZE);
    bool writePartition(char *buf) override;
    ~SparseImage() {}
    bool open(int64_t writePos, int64_t readPos) override;
    virtual uint32_t getOutputSize() const override;
private:
    uint64_t m_chunkCnt;
    uint64_t m_blkCnt;
    uint64_t m_blkSize;
};

class RawImage : public Image {
public:
    typedef struct _NFIType
    {
        uint16_t   pageSize;
        uint16_t   spareSize;
        uint16_t   addressCycle;
        uint16_t   pageShift;
    } NFI_MENU;


    typedef struct _BOOTLHeader_
    {
        char ID1[12];
        char version[4];
        uint32_t length;
        uint32_t startAddr;
        uint32_t checksum;
        char ID2[8];
        NFI_MENU  NFIinfo;
        uint16_t pagesPerBlock;
        uint16_t  totalBlocks;
        uint16_t  blockShift;
        uint16_t  linkAddr[6];
        uint16_t  lastBlock;
    } BOOTL_HEADER;

    #define REPLICATION_NUMBER  8

    typedef std::shared_ptr<RawImage> ptr;
    RawImage(std::string partname, std::string imagename, uint32_t bufSize = BUFFER_SIZE);
    ~RawImage() {}
    bool writePartition(char *buf) override;
    bool writePreloaderPartition(char * buf);
    bool writeNormalPartition(char * buf);
    int createBootloaderHeader(unsigned char *pBLHeader, char* blbuf, uint32_t imageSize, bool msdc_boot, int mini_io_size, int oob_size, int eb_size);
};

}
}