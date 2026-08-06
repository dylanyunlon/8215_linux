#pragma once

#include "utils/Util.hpp"
#ifdef CONFIG_NAND_BOOT
#include "libmtd.h"
#endif
#include <memory>
#include "mz.h"
#include "mz_zip.h"
#include "mz_strm.h"
#include "mz_zip_rw.h"
#ifndef RECOVERY_UPDATE_ZIP_NAME
#define RECOVERY_UPDATE_ZIP_NAME "/data/misc/mnt/update.zip"
#endif

namespace atcupdateservice {
namespace utils {
#define NO_ALIGN        -1

void dropSystemCaches();
class File {
public:
    enum Flags {
        READ  = 0x1,
        WRITE = 0x2,
        NAND_READ = 0x3,
        NAND_WRITE = 0x4,
    };
    struct Buffer {
        typedef std::shared_ptr<Buffer> ptr;
        char *m_buf = nullptr;
        int64_t m_size;
        int64_t m_realSize;
        bool realloc(int64_t size, int64_t align = NO_ALIGN);
        Buffer(int64_t size, int64_t align = NO_ALIGN);
        ~Buffer() {
            if (m_buf != nullptr) {
                free(m_buf);
                m_buf=nullptr;
            }
        }
    };

    typedef std::shared_ptr<File> ptr;
    static File::ptr Create(const std::string &fname, Flags flags);
    File(const std::string &filename, Flags flags);
    ~File() {
        if (m_zipReader) {
            closeZipEntry();
            mz_zip_reader_close(m_zipReader);
            mz_zip_reader_delete(&m_zipReader);
        }
    }
    virtual Buffer::ptr read(int64_t nread, int64_t align = NO_ALIGN);
    virtual int64_t write(char *buf, int64_t nwrite);
    virtual int  readFix(int64_t nread, char *buf, int64_t align = NO_ALIGN);

    virtual int64_t writeFix(char *buf, int64_t nwrite);
    virtual int64_t writeExt(char *buf, int64_t nwrite);
    virtual int64_t readExt(char* buf, int64_t nread, int64_t offset, FILE *partition_file);
    bool sync();
    virtual int64_t size() const;
    virtual int64_t img_size(const std::string &fname) const;
    int readFixFromZip(const std::string& internal_filename,
        int64_t nread, char *buf) ;
    const std::string &getName() const {
        return m_fname;
    }
    const std::string &getPartName() const {
        return part_name;
    }
    int64_t setReadOffset(int64_t off) {
        m_readOff = off;
        return m_readOff;
    }
    virtual int64_t setWriteOffset(int64_t off) {
        m_writeOff = off;
        return m_writeOff;
    }
    int64_t getReadOffset() const {
        return m_readOff;
    }
    int64_t getWriteOffset() const {
        return m_writeOff;
    }
    int64_t skipWrite(int64_t off) {
        m_writeOff += off;
        if (m_writeOff < 0) {
            m_writeOff = 0;
        }
        return m_writeOff;
    }
    int64_t skipRead(int64_t off) {
        m_readOff += off;
        if (m_readOff < 0) {
            m_readOff = 0;
        }
        return m_readOff;
    }
    void* getZipReader() const {
        return m_zipReader;
    }
    // In the File class definition
    virtual int getNandIoSize() {
        return 0; // Default implementation for non-NAND files
    }

    virtual int getNandOobSize() {
        return 0; // Default implementation for non-NAND files
    }

    virtual int getNandEbSize() {
        return 0;
    }
    bool openZipEntry();
    void closeZipEntry();
    bool seekInZip(int64_t offset);
protected:
    int64_t m_readOff;
    int64_t m_writeOff;
    std::string m_fname;
    std::string part_name;
    Flags m_flags;
private:
    void* m_zipReader;
    bool m_zipEntryOpened;
    std::string m_internalFilename;
};

#ifdef CONFIG_NAND_BOOT
class NandFile : public File {
public:

    struct nand_dev_info {
	    int block_cnt;
	    int block_size;
	    int page_size;
	    int oob_size;
    };

    typedef struct _SectorInfo {
	    uint16_t bBadBlock;
	    uint16_t wReserved2;
	    uint32_t dwReserved1;
    } SectorInfo, *PSectorInfo;

    static File::ptr Create(const std::string &fname, Flags flags);
    NandFile(const std::string &filename, Flags flags);
    ~NandFile();
    int64_t size() const;
    int64_t writeFix(char *buf, int64_t nwrite);
    int64_t writeExt(char* buf, int64_t nwrite);
    int64_t readExt(char* buf, int64_t nread, int64_t offset, FILE *partition_file);
    int64_t setWriteOffset(int64_t off) {
        if (off == 0) {
            m_writeOff = 0;
            return 0;
        }
        return -1;
    }
    int64_t nandEraseMtd(void);
    int getNandIoSize()  override{
        return m_nandiosize;
    }

    int getNandOobSize()  override{
        return m_nandoobsize;
    }

    int getNandEbSize()  override{
        return m_nandebsize;
    }

private:
    libmtd_t m_mtdDesc;
    struct mtd_dev_info m_mtd;
    int m_fd;
    int m_page;
    int m_oobpage;
    int m_block;
    int m_rsvPageNum;
    int m_pageInBlock;;
    uint8_t* m_pagebuf = nullptr;
    int m_nandiosize;
    int m_nandoobsize;
    int m_nandebsize;
};
#endif

}
}
