#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include "utils/File.hpp"
#include "mtd/mtd-abi.h"
#include "mz.h"
#include "mz_zip.h"
#include "mz_strm.h"
#include "mz_zip_rw.h"

#define PROGRAM_NAME "nandupgrade"
#ifndef RECOVERY_UPDATE_ZIP_NAME
#define RECOVERY_UPDATE_ZIP_NAME "/data/misc/mnt/update.zip"
#endif

namespace atcupdateservice {
namespace utils {

void dropSystemCaches() {
	int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
	if (fd < 0) {
		ATC_STREAM_LOGW() << "open drop_caches failed, error: "
						  << strerror(errno) << std::endl;
		return;
	}

	static const char cacheDropCmd[] = "3\n";
	ssize_t written = write(fd, cacheDropCmd, sizeof(cacheDropCmd) - 1);
	if (written != static_cast<ssize_t>(sizeof(cacheDropCmd) - 1)) {
		ATC_STREAM_LOGW() << "write drop_caches failed, written=" << written
						  << " error: " << strerror(errno) << std::endl;
	}

	close(fd);
    }

File::Buffer::Buffer(int64_t size, int64_t align)
    : m_buf(nullptr), m_size(size) {
        m_realSize = 0x20000;//128K
        if (m_buf) {
                ATC_STREAM_LOGE() << "start to free m_buf File::Buffer::Buffer" << std::endl;
                free(m_buf);
            }
        m_buf = (char *)malloc(m_realSize);
        if (m_buf == nullptr) {
            ATC_STREAM_LOGE() << "Failed to alloc memory for File::Buffer, size : " << size << std::endl;
            throw std::logic_error("out of memory");
        }
}

bool File::Buffer::realloc(int64_t size, int64_t align) {
    if (size > 0) {
        if (align != NO_ALIGN && align > 0) {
            size = ALIGN(size, align);
        }

        char *buf = (char *)::realloc(m_buf, size);
        if (buf == nullptr) {
            ATC_STREAM_LOGE() << "failed to realloc memory for buffer" << std::endl;
            return false;
        }
        free(m_buf);
        buf = m_buf;
        m_realSize = size;
        return true;
    }

    return false;
}

File::File(const std::string &filename, Flags flags)
    : m_readOff(0), m_writeOff(0),
      m_fname(filename), m_flags(flags),
      m_zipReader(nullptr),
      m_zipEntryOpened(false) {
}

File::ptr File::Create(const std::string &filename, Flags flags) {
    try {
        File::ptr file(new File(filename, flags));
        return file;
    } catch(std::exception &e) {
        return nullptr;
    }
}


bool File::openZipEntry() {
    if (m_zipEntryOpened) {
        return true;
    }

    if (!m_zipReader) {
        m_zipReader = mz_zip_reader_create();
        if (!m_zipReader) {
            ATC_STREAM_LOGE() << "Failed to create zip reader!" << std::endl;
            return false;
        }

        int32_t err = mz_zip_reader_open_file(m_zipReader, RECOVERY_UPDATE_ZIP_NAME);
        if (err != MZ_OK) {
            ATC_STREAM_LOGE() << "Failed to open zip file: " << RECOVERY_UPDATE_ZIP_NAME << std::endl;
            mz_zip_reader_delete(&m_zipReader);
            return false;
        }

        m_internalFilename = basename(m_fname.c_str());
        err = mz_zip_reader_locate_entry(m_zipReader, m_internalFilename.c_str(), 1);
        if (err != MZ_OK) {
            ATC_STREAM_LOGE() << "File not found in zip: " << m_internalFilename << std::endl;
            mz_zip_reader_close(m_zipReader);
            mz_zip_reader_delete(&m_zipReader);
            m_zipReader = nullptr;
            return false;
        }
    }
    int32_t err = mz_zip_reader_entry_open(m_zipReader);
    if (err != MZ_OK) {
        ATC_STREAM_LOGE() << "Failed to open entry: " << m_internalFilename << std::endl;
        return false;
    }

    m_zipEntryOpened = true;

    if (m_readOff > 0) {
        if (!seekInZip(m_readOff)) {
            ATC_STREAM_LOGE() << "Failed to seek to offset: " << m_readOff << std::endl;
            mz_zip_reader_entry_close(m_zipReader);
            m_zipEntryOpened = false;
            return false;
        }
    }

    return true;
}

void File::closeZipEntry() {
    if (m_zipReader) {
        if (m_zipEntryOpened) {
            mz_zip_reader_entry_close(m_zipReader);
            m_zipEntryOpened = false;
        }
    }
}

bool File::seekInZip(int64_t offset) {
    if (!m_zipEntryOpened) {
        return false;
    }

    if (offset == 0) {
        return true;
    }

    int64_t skipped = 0;
    char skip_buf[8192];
    while (skipped < offset) {
        int64_t to_skip = std::min(sizeof(skip_buf), (size_t)(offset - skipped));
        int read_count = mz_zip_reader_entry_read(m_zipReader, skip_buf, to_skip);
        if (read_count <= 0) {
            break;
        }
        skipped += read_count;
    }

    return (skipped == offset);
}

#ifdef CONFIG_NAND_BOOT
NandFile::NandFile(const std::string &filename, Flags flags) : File(filename, flags) {
    m_mtdDesc = libmtd_open();
    int partIdx = utils::getPartIdxFromXml(basename(m_fname.c_str()));
    if (partIdx < 0) {
        ATC_STREAM_LOGE() << "getPartIdxFromXml failed" << std::endl;
        return;
    }
    m_fname = std::string(NAND_MTD_PATH) + std::to_string(partIdx);
    mtd_get_dev_info(m_mtdDesc, m_fname.c_str(), &m_mtd);
    //ATC_STREAM_LOGE() << "malloc m_pagebuf size: " << m_mtd.min_io_size << "flag: "<< flags << std::endl;
    m_pagebuf = (uint8_t* )malloc(m_mtd.min_io_size);
    if (m_pagebuf == nullptr) {
        ATC_STREAM_LOGE() << "malloc m_pagebuf failed!" << std::endl;
        return;
    }
    memset(m_pagebuf, 0xFF, m_mtd.min_io_size);
    m_fd = open(m_fname.c_str(), O_RDWR);
    if (m_fd < 0) {
        ATC_STREAM_LOGE() << "open "<<m_fname<<" failed" << std::endl;
        return;
    }
    m_nandoobsize = m_mtd.oob_size;
    m_nandiosize = m_mtd.min_io_size;
    m_nandebsize = m_mtd.eb_size;
    part_name = filename;

    if (flags == File::NAND_WRITE) {
        //ATC_STREAM_LOGI() << "go to earseMtd" <<std::endl;
        if (nandEraseMtd()) {
            ATC_STREAM_LOGE() << "nandEraseMtd failed" << std::endl;
            m_fd = -1;
        return;
        }
    }


    m_page = 0;
    m_oobpage = 0;
    m_block = -1;
    m_pageInBlock = 0;
    m_rsvPageNum = m_mtd.eb_size / m_mtd.min_io_size - 1;
/*
    ATC_STREAM_LOGI() << "====> partname:" << m_fname << std::endl;
    ATC_STREAM_LOGI() << "====> mtdname:" << m_fname << std::endl;
    ATC_STREAM_LOGI() << "====> eb_size:" << m_mtd.eb_size << std::endl;
    ATC_STREAM_LOGI() << "====> min_io_size:" << m_mtd.min_io_size << std::endl;
    ATC_STREAM_LOGI() << "====> m_rsvPageNum:" << m_rsvPageNum << std::endl;*/
}

NandFile::~NandFile()
{
    libmtd_close(m_mtdDesc);
    m_page = 0;
    if (m_fd > 0)
        close(m_fd);
    if (m_pagebuf)
        free(m_pagebuf);

}

int64_t NandFile::nandEraseMtd(void)
{
    unsigned int eb, eb_start = 0, eb_cnt;
    bool isNAND;
    int error = 0;
    off_t offset = 0;
    uint32_t cnt = 0;
    int ret;

    eb_cnt = m_mtd.size / m_mtd.eb_size;

    for (eb = eb_start; eb < eb_start + eb_cnt; eb++) {
        offset = (off_t)eb * m_mtd.eb_size;

        ret = mtd_is_bad(&m_mtd, m_fd, eb);
        if (ret > 0) {
            ATC_STREAM_LOGI() << "Skipping bad block at "<< offset << std::endl;
            continue;
        } else if (ret < 0) {
            ATC_STREAM_LOGE() << "MTD get bad block failed" << std::endl;
            return -1;
        }

        if (mtd_erase(m_mtdDesc, &m_mtd, m_fd, eb) != 0) {
            ATC_STREAM_LOGE() << "mtd_erase failed" << std::endl;
            continue;
        }
    }

    ATC_STREAM_LOGI() << "Erase partition " << m_fname << "success" << std::endl;
    return 0;
}

File::ptr NandFile::Create(const std::string &filename, Flags flags)
{
    try {
        File::ptr file(new NandFile(filename, flags));
        return file;
    } catch(std::exception &e) {
        return nullptr;
    }
}

int64_t NandFile::size() const {
    ATC_STREAM_LOGI()<<"mtdname:"<<m_mtd.name<<" mtdsize:"<<m_mtd.size << std::endl;
    return m_mtd.size;
}

int64_t NandFile::readExt(char* buf, int64_t nread, int64_t offset, FILE *partition_file) {
    if (buf == nullptr || nread == 0 || partition_file == nullptr) {
        ATC_STREAM_LOGI()<< "readExt parameter is error" << std::endl;
        return -1;
    }

    const size_t valid_pages_pre_block = (m_mtd.eb_size / m_mtd.min_io_size) -1;
    const size_t valid_block_size = valid_pages_pre_block * m_mtd.min_io_size;
    int total_read = 0;
    int current_offset = offset;
    uint8_t* dest = (uint8_t*)(buf);

    while (total_read < nread) {
        // get offset in current && get offset in current page
        int block_index = current_offset /valid_block_size;
        int offset_in_block = current_offset % valid_block_size;
        int page_index_in_block = offset_in_block / m_mtd.min_io_size;
        //ATC_STREAM_LOGI()<<"block_index : "<< block_index << ", offset_in_block: "<< offset_in_block << ", page_index_in_block: " << page_index_in_block << std::endl;

        // get offset in partition
        int actual_block_offset = block_index * m_mtd.eb_size;
        int actual_page_offset = page_index_in_block * m_mtd.min_io_size;
        int actual_offset = actual_block_offset + actual_page_offset;
       // ATC_STREAM_LOGI()<<"actual_block_offset : "<< actual_block_offset << ", actual_page_offset: "<< actual_page_offset << ", actual_offset: " << actual_offset << std::endl;
        // get offset in page && get read size
        int offset_in_page = offset_in_block % m_mtd.min_io_size;
        int remaining_in_page = m_mtd.min_io_size - offset_in_page;
        int remaining_to_read = nread - total_read;
        int read_this_time = std::min(remaining_in_page, remaining_to_read);

         //ATC_STREAM_LOGI()<<"block_index : "<< block_index  <<"actual_offset: " << actual_offset <<"static_cast<long>(actual_offset + offset_in_page) "<<  static_cast<long>(actual_offset + offset_in_page) << std::endl;
        if (fseek(partition_file, static_cast<long>(actual_offset + offset_in_page), SEEK_SET) != 0) {
            ATC_STREAM_LOGI()<<"static_cast<long>(actual_offset + offset_in_page) "<<  static_cast<long>(actual_offset + offset_in_page)<< std::endl;
             ATC_STREAM_LOGI()<<"fseek fail,error " << strerror(errno) << std::endl;
            break;
        }

        int bytes_read = fread(dest + total_read, 1, read_this_time, partition_file);
        if (bytes_read == 0) {
            ATC_STREAM_LOGI()<<"bytes_read :"<<bytes_read<<" fail " << std::endl;
            break;
        }

        total_read += bytes_read;
        current_offset += bytes_read;
        if (bytes_read < read_this_time) {
            ATC_STREAM_LOGI()<<"read data not enouch, bytes_read :"<<bytes_read << std::endl;
            break;
        }
    }
    return total_read;

}

int64_t NandFile::writeExt(char* buf, int64_t nwrite) {
    int ret;
    uint8_t *buffer;
    int bad_block;
    int remaining = nwrite;
    uint8_t write_mode = MTD_OPS_PLACE_OOB;
    int total_written = 0;
    uint8_t* oobbuf = NULL;
    PSectorInfo sector_info;
    int sectorsize = sizeof(SectorInfo);
    int page_offset = 0;

    if (m_pagebuf == nullptr) {
        ATC_STREAM_LOGE()<<"invalid m_pagebuf!"<< std::endl;
        return -1;
    }

    if (m_fd < 0) {
        ATC_STREAM_LOGE()<<"invalid m_fd!"<< std::endl;
        return -1;
    }

    buffer = (uint8_t *)malloc(m_mtd.eb_size);
    if (!buffer) {
        ATC_STREAM_LOGE()<<"malloc buffer fail"<< std::endl;
        return -1;
    }
    memset(buffer, 0xFF, m_mtd.eb_size);

    oobbuf = (uint8_t *)malloc(m_mtd.oob_size);
    if (!oobbuf) {
        ATC_STREAM_LOGE()<<"malloc oobbuf fail"<< std::endl;
        return -1;
    }
    memset(oobbuf, 0xFF, m_mtd.oob_size);

#ifdef NAND_RW_COMPARE
    uint8_t* readbuf = (uint8_t* )malloc(m_mtd.min_io_size);
    if (!readbuf) {
        ATC_STREAM_LOGE()<<"malloc readbuf fail"<< std::endl;
        ret = -1;
        goto out;
    }
#endif

    while (remaining > 0 && m_writeOff < m_mtd.size) {
        int block = m_writeOff / m_mtd.eb_size;
        int block_offset = m_writeOff % m_mtd.eb_size;
        int is_bad;

        //ATC_STREAM_LOGE()<<"m_writeOff: "<< m_writeOff << " block: "<< block << " block_offset: "<< block_offset <<std::endl;

        if (block != m_block) {
            //To Do
            m_block = block;
            m_pageInBlock = 0;
            //ATC_STREAM_LOGE()<<"++++ check bad block and erase ++++"<< std::endl;
            //ATC_STREAM_LOGE()<<"[mblock] : "<< m_block << std::endl;
            if ((is_bad = mtd_is_bad(&m_mtd, m_fd, block)) < 0) {
                ATC_STREAM_LOGE()<<"failed to check bad block"<< std::endl;
                ret = -1;
                goto out;
            }

            if (is_bad) {
                ATC_STREAM_LOGW()<<"skipping bad block:"<< block << " at offset:"<< m_writeOff << std::endl;
                m_writeOff += m_mtd.eb_size;
                m_page += (m_mtd.eb_size / m_mtd.min_io_size);
                continue;
            }
            #if 0
            if (mtd_erase(m_mtdDesc, &m_mtd, m_fd, block) < 0) {
                ATC_STREAM_LOGE()<<"erase failed, marking bad block"<< std::endl;
                if (mtd_mark_bad(&m_mtd, m_fd, block)) {
                    ATC_STREAM_LOGE()<<"failed to mark bad block"<< std::endl;
                }
                m_writeOff += m_mtd.eb_size;
                m_page += (m_mtd.eb_size / m_mtd.min_io_size);
                continue;
            }
            #endif
        }

        uint32_t wsize = (remaining > m_mtd.eb_size - block_offset) ? m_mtd.eb_size - block_offset : remaining;
        wsize = (wsize / m_mtd.min_io_size) * m_mtd.min_io_size;
        if (wsize == 0) {
            //ATC_STREAM_LOGE()<<"++++ wsize is zero ++++" << std::endl;
            wsize = m_mtd.min_io_size;
        }
        //ATC_STREAM_LOGE()<<"remain_size: "<< remaining << " wsize_align:" << wsize <<std::endl;

        memcpy(buffer + block_offset, buf + (nwrite - remaining), wsize);

        for (int page = 0; page < wsize/m_mtd.min_io_size; page++, m_page++, m_oobpage++) {
            page_offset = m_page * m_mtd.min_io_size;

            sector_info = (PSectorInfo)oobbuf;
            sector_info->bBadBlock = 0xFFFF;
            sector_info->wReserved2 = 0;
            sector_info->dwReserved1 = m_oobpage;

            //ATC_STREAM_LOGE()<<"[page]:"<< page << " [page_offset]:"<< page_offset << " [m_rsvPageNum]:" << m_rsvPageNum <<std::endl;
            //ATC_STREAM_LOGE()<< "[m_page]:"<< m_page << " [m_oobpage]:"<< m_oobpage << " [page_in_block]:" << m_pageInBlock << std::endl;

            memcpy(m_pagebuf + m_pageInBlock * sectorsize, oobbuf, sectorsize);

            //ATC_STREAM_LOGE()<<"[MTD_DBG] eb:"<<m_block<<" offs:"<<page_offset % m_mtd.eb_size<<std::endl;
            ret = mtd_write(m_mtdDesc, &m_mtd, m_fd, m_block, page_offset % m_mtd.eb_size,
                            buffer + block_offset + page * m_mtd.min_io_size, m_mtd.min_io_size, oobbuf, sectorsize, write_mode);
            if (ret < 0) {
                ATC_STREAM_LOGE()<<"mtd_write data failed"<< std::endl;
                goto out;
            }
#ifdef NAND_RW_COMPARE
            // ::sync();
            // dropSystemCaches();
            ret = mtd_read(&m_mtd, m_fd, m_block, page_offset % m_mtd.eb_size, readbuf, m_mtd.min_io_size);
            if (ret < 0) {
                ATC_STREAM_LOGE()<<"mtd_read failed"<< std::endl;
            }
            if(memcmp((void*)(buffer + block_offset + page * m_mtd.min_io_size), (void*)readbuf, m_mtd.min_io_size)){
                ATC_STREAM_LOGE()<<"compare the readbuf and writebuf fail"<< std::endl;
            }
#endif
            m_writeOff += m_mtd.min_io_size;
            remaining -= m_mtd.min_io_size;
            total_written += m_mtd.min_io_size;

            //write oobbuf to last page in block
            if (m_pageInBlock == (m_rsvPageNum - 1)) {
                //ATC_STREAM_LOGI()<<"==== write pagebuf ===="<< std::endl;
                sector_info = (PSectorInfo)oobbuf;
                sector_info->bBadBlock = 0xFFFF;
                sector_info->wReserved2 = 0;
                sector_info->dwReserved1 = RESERVED_SECTOR_IN_BLOCK;
                page_offset = m_rsvPageNum * m_mtd.min_io_size;     //To Do

                //ATC_STREAM_LOGE()<<"[MTD_DBG_OOB] eb:"<<m_block<<" offs:"<<page_offset % m_mtd.eb_size<<std::endl;
                ret = mtd_write(m_mtdDesc, &m_mtd, m_fd, m_block, page_offset % m_mtd.eb_size,
                                m_pagebuf, m_mtd.min_io_size, oobbuf, sectorsize, write_mode);
                if (ret < 0) {
                    ATC_STREAM_LOGE()<<"mtd_write m_pagebuf failed"<< std::endl;
                    goto out;
                }
#ifdef NAND_RW_COMPARE
                // ::sync();
                // dropSystemCaches();
                ret = mtd_read(&m_mtd, m_fd, m_block, page_offset % m_mtd.eb_size, readbuf, m_mtd.min_io_size);
                if (ret < 0) {
                    ATC_STREAM_LOGE()<<"mtd_read failed"<< std::endl;
                }
                if(memcmp((void*)(m_pagebuf), (void*)readbuf, m_mtd.min_io_size)){
                    ATC_STREAM_LOGE()<<"compare the readbuf and writebuf fail"<< std::endl;
                }
#endif
                memset(m_pagebuf, 0xFF, m_mtd.min_io_size);
                m_pageInBlock = 0;
                m_page++;
                page++;
                m_writeOff += m_mtd.min_io_size;
                continue;
            }
            m_pageInBlock++;
        }
    }

    ret = total_written;

out:
    if (buffer)
        free(buffer);
    if (oobbuf)
        free(oobbuf);
#ifdef NAND_RW_COMPARE
    if (readbuf)
        free(readbuf);
#endif

    //ATC_STREAM_LOGE()<<"==== m_writeOff:"<< m_writeOff << " return:" << ret <<std::endl;
    return ret;
}

int64_t NandFile::writeFix(char *buf, int64_t nwrite) {
    int ret;
    struct mtd_dev_info mtd_info;
    uint8_t *buffer;
    int bad_block;
    int remaining = nwrite;
    uint8_t write_mode;
    int total_written = 0;

    if (m_fd < 0) {
        ATC_STREAM_LOGE()<<"m_fd is invalid!"<< std::endl;
        return -1;
    }

    buffer = (uint8_t *)malloc(m_mtd.eb_size);
    if (!buffer) {
        ATC_STREAM_LOGE()<<"malloc buffer fail"<< std::endl;
        return -1;
    }
    memset(buffer, 0xFF, m_mtd.eb_size);

#ifdef NAND_RW_COMPARE
    uint8_t* readbuf = (uint8_t* )malloc(m_mtd.min_io_size);
    if (!readbuf) {
        ATC_STREAM_LOGE()<<"malloc readbuf fail"<< std::endl;
        ret = -1;
        goto out;
    }
#endif

    while (remaining > 0 && m_writeOff < m_mtd.size) {
        //ATC_STREAM_LOGE()<<"remaining: "<< remaining  << " m_mtd.eb_size: " << m_mtd.eb_size<<std::endl;
        int block = m_writeOff / m_mtd.eb_size;
        int block_offset = m_writeOff % m_mtd.eb_size;
        int is_bad;

        //ATC_STREAM_LOGE()<<"m_writeOff: "<< m_writeOff << " block: "<< block << " block_offset: "<< block_offset <<std::endl;

        if (block != m_block) {
            m_block = block;
            //ATC_STREAM_LOGE()<<"++++ check bad block and erase ++++"remaining
           // ATC_STREAM_LOGE()<<"[mblock] : "<< m_block << std::endl;
            if ((is_bad = mtd_is_bad(&m_mtd, m_fd, block)) < 0) {
                ATC_STREAM_LOGE()<<"failed to check bad block"<< std::endl;
                ret = -1;
                goto out;
            }

            if (is_bad) {
                ATC_STREAM_LOGW()<<"skipping bad block:"<< block << " at offset:"<< m_writeOff << std::endl;
                m_writeOff += m_mtd.eb_size;
                m_page += (m_mtd.eb_size / m_mtd.min_io_size);
                continue;
            }
            #if 0
            if (mtd_erase(m_mtdDesc, &m_mtd, m_fd, block) < 0) {
                ATC_STREAM_LOGE()<<"++++ erase failed, marking bad block ++++"<< std::endl;
                if (mtd_mark_bad(&m_mtd, m_fd, block)) {
                    ATC_STREAM_LOGE()<<"failed to mark bad block"<< std::endl;
                }
                m_writeOff += m_mtd.eb_size;
                m_page += (m_mtd.eb_size / m_mtd.min_io_size);
                continue;
            }
            #endif
        }

        uint32_t wsize = (remaining > m_mtd.eb_size - block_offset) ? m_mtd.eb_size - block_offset : remaining;
        wsize = (wsize / m_mtd.min_io_size) * m_mtd.min_io_size;
        if (wsize == 0) {
            //ATC_STREAM_LOGE()<<"++++ wsize is zero ++++" << std::endl;
            wsize = m_mtd.min_io_size;
        }

        memcpy(buffer + block_offset, buf + (nwrite - remaining), wsize);

        //ATC_STREAM_LOGE()<<"wsize: "<< wsize << " remain_size:" << remaining << std::endl;
        for (int page = 0; page < wsize/m_mtd.min_io_size; page++, m_page++) {
            int page_offset = m_page * m_mtd.min_io_size;
            //ATC_STREAM_LOGE()<<"[page]: "<< page << " [page_offset]: "<< page_offset << " [m_page]:"<< m_page << std::endl;

            ret = mtd_write(m_mtdDesc, &m_mtd, m_fd, m_block,
                                        page_offset % m_mtd.eb_size, buffer + block_offset + page * m_mtd.min_io_size, m_mtd.min_io_size, NULL, 0, write_mode);
            if (ret < 0) {
                ATC_STREAM_LOGE()<<"mtd_write failed"<< std::endl;
                goto out;
            }
#ifdef NAND_RW_COMPARE
            // ::sync();
            // dropSystemCaches();
            ret = mtd_read(&m_mtd, m_fd, m_block, page_offset % m_mtd.eb_size, readbuf, m_mtd.min_io_size);
            if (ret < 0) {
                ATC_STREAM_LOGE()<<"mtd_read failed"<< std::endl;
            }
            if(memcmp((void*)(buffer + block_offset + page * m_mtd.min_io_size), (void*)readbuf, m_mtd.min_io_size)){
                ATC_STREAM_LOGE()<<"compare the readbuf and writebuf fail"<< std::endl;
            }
#endif
            remaining -= m_mtd.min_io_size;
            m_writeOff += m_mtd.min_io_size;
            total_written += m_mtd.min_io_size;
        }
    }

    ret = total_written;

out:
    if (buffer)
        free(buffer);
#ifdef NAND_RW_COMPARE
    if (readbuf)
        free(readbuf);
#endif

    //ATC_STREAM_LOGE()<<"==== m_writeOff:"<< m_writeOff << " return:" << ret << std::endl;
    return ret;
}
#endif


File::Buffer::ptr File::read(int64_t nread, int64_t align) {
    if ((m_flags & Flags::READ) == 0) {
        throw std::logic_error("read is not supported!");
    }
    int fd = open(m_fname.c_str(), O_RDONLY);
    if (fd == -1) {
        ATC_STREAM_LOGE() << "invalid file descriptor" << std::endl;
        throw std::logic_error("failed to open " + m_fname);
    }
    Buffer::ptr buffer(new Buffer(nread, align));
    ssize_t rt = lseek64(fd, m_readOff, SEEK_SET);

    if (rt != m_readOff) {
        ATC_STREAM_LOGE() << "move read pointer failed" << std::endl;
        close(fd);
        return nullptr;
    }
    memset(buffer->m_buf, 0, nread);
    rt = ::read(fd, buffer->m_buf, nread);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "read failed, rt = " << rt
            << " error = " << strerror(errno) << std::endl;
        close(fd);
        return nullptr;
    } else {
        buffer->m_size = rt;
    }
    m_readOff += buffer->m_size;
    close(fd);

    return buffer;
}

int64_t File::img_size(const std::string &fname) const {

    void *zip_reader = NULL;
    mz_zip_file *file_info = NULL;
    const char *extract_file_name = NULL;
    const char *file = fname.c_str();

    // Using Minizip to process ZIP files
    ATC_STREAM_LOGI() << "read file:" << file << std::endl;
    zip_reader = mz_zip_reader_create();
    if (zip_reader == NULL) {
        ATC_STREAM_LOGE() << "Failed to create zip reader." << std::endl;
        return -1;
    }

    if (mz_zip_reader_open_file(zip_reader, RECOVERY_UPDATE_ZIP_NAME) != MZ_OK) {
        ATC_STREAM_LOGE() << "Failed to open zip file: " << RECOVERY_UPDATE_ZIP_NAME << std::endl;
        mz_zip_reader_delete(&zip_reader);
        return -1;
    }

    // Locate the file to extract
    if (mz_zip_reader_locate_entry(zip_reader, file, 1) == MZ_OK){
        if (mz_zip_reader_entry_get_info(zip_reader, &file_info) == MZ_OK) {
            int64_t uncompressed_size = file_info->uncompressed_size;
            extract_file_name = file_info->filename;
            ATC_STREAM_LOGI() << "extract_file_name=" << extract_file_name
                              << ", file_len:" << uncompressed_size << std::endl;
            mz_zip_reader_close(zip_reader);
            mz_zip_reader_delete(&zip_reader);
            return uncompressed_size;
        }
    }

    mz_zip_reader_close(zip_reader);
    mz_zip_reader_delete(&zip_reader);

    ATC_STREAM_LOGE() << "get file size fail in zip file" << std::endl;
    return -1;
}

int64_t File::size() const {
    struct stat buf;
    int fd = open(m_fname.c_str(), O_RDONLY);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "get size failed: open failed!" << std::endl;
        return -1;
    }
    if (fstat(fd, &buf) < 0) {
        ATC_STREAM_LOGE() << "fstat failed, error : " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }
    if (S_ISREG(buf.st_mode)) {
        close(fd);
        return buf.st_size;
    }
    else if (S_ISBLK(buf.st_mode)) {
        int64_t size;
        if (ioctl(fd, BLKGETSIZE64, &size) < 0) {
            ATC_STREAM_LOGE() << "failed to get block device size, error : " << strerror(errno) << std::endl;
            close(fd);
            return -1;
        }
        close(fd);
        return size;
    }
    ATC_STREAM_LOGE() << "unsupported file type" << std::endl;
    close(fd);

    return -1;
}

int File::readFixFromZip(const std::string& internal_filename, int64_t nread, char *buf) {
    if (buf == nullptr || nread <= 0) {
        ATC_STREAM_LOGE() << "Invalid parameters for readFixFromZip" << std::endl;
        return -1;
    }

    if (!openZipEntry()) {
        ATC_STREAM_LOGE() << "Failed to open ZIP entry" << std::endl;
        return -1;
    }

    mz_zip_file* file_info = nullptr;
    int32_t err = mz_zip_reader_entry_get_info(m_zipReader, &file_info);
    if (err != MZ_OK) {
        ATC_STREAM_LOGE() << "Failed to get file info: " << internal_filename << std::endl;
        closeZipEntry();
        return -1;
    }

    int64_t bytesAvailable = file_info->uncompressed_size - m_readOff;
    int64_t bytesToRead = std::min(nread, bytesAvailable);
    if (bytesToRead <= 0) {
        ATC_STREAM_LOGW() << "No more data to read from file" << std::endl;
        return 0;
    }

    int64_t total_read = 0;
    while (total_read < bytesToRead) {
        int read_count = mz_zip_reader_entry_read(m_zipReader, buf + total_read, bytesToRead - total_read);
        if (read_count <= 0) {
            break;
        }
        total_read += read_count;
    }

    m_readOff += total_read;

    if (total_read == 0 && bytesAvailable > 0) {
        ATC_STREAM_LOGE() << "Failed to read any data from file" << std::endl;
        return -1;
    }

    if (total_read < bytesToRead) {
        ATC_STREAM_LOGE() << "PARTIAL ZIP READ: " << internal_filename
                          << " expected=" << bytesToRead
                          << " actual=" << total_read << std::endl;
        return -1;
    }

    return 0;
}

int File::readFix(int64_t nread ,char *buf, int64_t align) {
    if ((m_flags & Flags::READ) == 0) {
        throw std::logic_error("read is not supported!");
    }
    int fd = open(m_fname.c_str(), O_RDONLY);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "invalid file descriptor, error=" << strerror(errno) << std::endl;
        throw std::logic_error("open " + m_fname + " failed!");
    }
  // Buffer::ptr buffer(new Buffer(nread, align));
    ssize_t rt = lseek64(fd, m_readOff, SEEK_SET);

    if (rt != m_readOff) {
        ATC_STREAM_LOGE() << "move read pointer failed" << std::endl;
        close(fd);
        return -1;
    }
    memset(buf, 0, nread);
    int m_size = 0;
    //buffer->m_size = 0;
    while (m_size < nread) {
        rt = ::read(fd, buf + m_size, nread - m_size);
        if (rt < 0) {
            ATC_STREAM_LOGE() << "readFix failed! rt = " << rt
                    << " error = " << strerror(errno) << std::endl;
            close(fd);
            return -1;
        }
        if (rt == 0) {
            break;
        }
        m_size += rt;
        assert(nread >= m_size);
    }
    m_readOff += m_size;

    close(fd);
    return 0;
}

int64_t File::write(char *buf, int64_t nwrite) {
    if ((m_flags & Flags::WRITE) == 0) {
        throw std::logic_error("read is not supported!");
    }
    int fd = open(m_fname.c_str(), O_WRONLY);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "invalid file descriptor" << std::endl;
        return -1;
    }
    ssize_t rt = lseek64(fd, m_writeOff, SEEK_SET);

    if (rt != m_writeOff) {
        ATC_STREAM_LOGE() << "move write pointer failed" << std::endl;
        close(fd);
        return -1;
    }
    rt = ::write(fd, buf, nwrite);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "write failed, rt = " << rt
                    << " error = " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }
    if (fsync(fd) != 0) {
        close(fd);
        ATC_STREAM_LOGE() << "failed to sync: " << m_fname << " error: " << strerror(errno) << std::endl;
        return -1;
    }
    m_writeOff += rt;

    close(fd);
    return rt;
}

int64_t File::writeExt(char *buf, int64_t nwrite)
{
    return 0;
}

int64_t File::readExt(char* buf, int64_t nread, int64_t offset, FILE *partition_file)
{
    return 0;
}
int64_t File::writeFix(char *buf, int64_t nwrite) {
    if ((m_flags & Flags::WRITE) == 0) {
        throw std::logic_error("read is not supported!");
    }
    int fd = open(m_fname.c_str(), O_WRONLY);
    if (fd < 0) {
        ATC_STREAM_LOGE() << "invalid file descriptor" << std::endl;
        return -1;
    }
    ssize_t rt = lseek64(fd, m_writeOff, SEEK_SET);
    ssize_t nLeft = nwrite;
    ssize_t curPos = 0;

    if (rt != m_writeOff) {
        ATC_STREAM_LOGE() << "move write pointer failed" << std::endl;
        close(fd);
        return -1;
    }

    while (nLeft) {
        rt = ::write(fd, buf + curPos, nLeft);
        if (rt < 0) {
            ATC_STREAM_LOGE() << "write failed, rt = " << rt
                        << " error = " << strerror(errno) << std::endl;
            close(fd);
            return -1;
        }
        if (rt == 0) {
            break;
        }
        nLeft -= rt;
        curPos += rt;
        assert(nLeft >= 0);
    }
    if (fsync(fd) != 0) {
        close(fd);
        ATC_STREAM_LOGE() << "failed to sync: " << m_fname << " error: " << strerror(errno) << std::endl;
        return -1;
    }
    m_writeOff += curPos;
    close(fd);

    return curPos;
}

}
}
