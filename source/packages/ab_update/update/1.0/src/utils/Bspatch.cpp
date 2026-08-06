#include <limits.h>
#include "utils/Bspatch.hpp"
#include "utils/File.hpp"
#include <bzlib.h>

namespace atcupdateservice {
namespace utils {

static int64_t offtin(uint8_t *buf)
{
	int64_t y;

	y=buf[7]&0x7F;
	y=y*256;y+=buf[6];
	y=y*256;y+=buf[5];
	y=y*256;y+=buf[4];
	y=y*256;y+=buf[3];
	y=y*256;y+=buf[2];
	y=y*256;y+=buf[1];
	y=y*256;y+=buf[0];

	if(buf[7]&0x80) y=-y;

	return y;
}

Bz2BspatchStream::~Bz2BspatchStream()
{
	int bz2err = 0;
    if (m_bz2) {
        BZ2_bzReadClose(&bz2err, m_bz2);
    }
    if (m_patch) {
        fclose(m_patch);
    }
}

Bz2BspatchStream::Bz2BspatchStream(const std::string& file)
        : BspatchStream(file),
          m_bz2(NULL),
          m_patch(NULL)
{
	int bz2err;
    uint8_t header[24];
    m_patch = fopen(file.c_str(), "rb");
    if (fread(header, 1, 24, m_patch) != 24) {
        ATC_STREAM_LOGE() << "read " << m_name << " failed!" << std::endl;
        fclose(m_patch);
        m_patch = NULL;
        return ;
    }
    if (memcmp(header, "ENDSLEY/BSDIFF43", 16) != 0) {
        ATC_STREAM_LOGE() << "invalid maigc!" << std::endl;
        fclose(m_patch);
        m_patch = NULL;
        return ;
    }
    m_newSize=offtin(header+16);
    if(m_newSize < 0) {
        ATC_STREAM_LOGE() << "invalid image size: " << m_newSize << std::endl;
        fclose(m_patch);
        m_patch = NULL;
        return ;
    }

    m_bz2 = BZ2_bzReadOpen(&bz2err, m_patch, 0, 0, NULL, 0);
    if (m_bz2 == NULL) {
        fclose(m_patch);
        m_newSize = 0;
    }
}

int Bz2BspatchStream::read(uint8_t *data, uint32_t size) {
	int n;
	int bz2err;

	n = BZ2_bzRead(&bz2err, m_bz2, data, size);
	if (n != size)
		return -1;

	return 0;
}

/**
 * original author: https://github.com/mendsley/bsdiff/
 */

int Bspatch::patch()
{
	uint8_t buf[8];
	int64_t newpos = 0;
	int64_t ctrl[3];
	int64_t i;
	int offset = 0;
	std::vector<char> nand_write_buffer;
	int nand_buf_size = 128 * 1024;
	int64_t newsize = (int64_t)m_stream->newSize();
	ATC_STREAM_LOGE() << "new size:" << newsize << std::endl;

	FILE * src_fp = fopen((m_source->getPartName()).c_str(), "r");
	if( src_fp == NULL ) {
		ATC_STREAM_LOGE()  << m_source->getPartName() << "open fail, "<< strerror(errno) << std::endl;
		return -1;
	}
	while(newpos<newsize) {
		//ATC_STREAM_LOGE()  << "newpos, "<<newpos<< std::endl;
		uint64_t bytes_from_diff_block = 0;
		uint64_t bytes_from_extra_block = 0;
		int64_t seek_in_source = 0;
		/* Read control data */
		for(i=0;i<=2;i++) {
			if (m_stream->read(buf, 8)) {
				 ATC_STREAM_LOGE()   << "m_stream->read fail "<<  std::endl;
				 fclose(src_fp);
				return -1;
			}

			ctrl[i]=offtin(buf);
		};

		/* Sanity-check */
		if (ctrl[0]<0 || ctrl[0]>INT_MAX ||
			ctrl[1]<0 || ctrl[1]>INT_MAX ||
			newpos+ctrl[0]>newsize) {

				ATC_STREAM_LOGE()   << "Sanity-check fail "<<  std::endl;
				fclose(src_fp);
				return -1;
			}

		bytes_from_diff_block = (uint64_t)ctrl[0];
		bytes_from_extra_block = (uint64_t)ctrl[1];
		seek_in_source = ctrl[2];

		// handle diff
		File::Buffer::ptr oldBuf(new File::Buffer(0));
		File::Buffer::ptr newBuf(new File::Buffer(0));

		ATC_STREAM_LOGE() << " bytes_from_diff_block: " << bytes_from_diff_block << " m source offset: " << offset << std::endl;
		while (bytes_from_diff_block) {
			int64_t nr_read = std::min(bytes_from_diff_block, m_limit);
			int64_t ret = 0;
			memset(oldBuf->m_buf, 0, nr_read);
			memset(newBuf->m_buf, 0, nr_read);
			if (strstr((m_source->getPartName()).c_str(), "system")){
				//ATC_STREAM_LOGE() << "system partition to merge: " << m_source->getPartName() << std::endl;
				ret = m_source->readExt(oldBuf->m_buf, nr_read, offset ,src_fp);
				if (ret < 0) {
					ATC_STREAM_LOGE() << "failed read " << nr_read << " bytes from " << m_source->getName() << std::endl;
					fclose(src_fp);
					return ret;
				}
			} else {
				//ATC_STREAM_LOGE() << "other partition to merge: " << m_source->getPartName() << std::endl;
				//ATC_STREAM_LOGE() << " nr_read: " << nr_read << std::endl;
				ret = m_source->readFix(nr_read, oldBuf->m_buf);
				if (ret < 0) {
					ATC_STREAM_LOGE() << "failed read " << nr_read << " bytes from " << m_source->getName() << std::endl;
					fclose(src_fp);
					return ret;
				}
			}

			ret = m_stream->read((uint8_t *)newBuf->m_buf, nr_read);
			if (ret < 0) {
				ATC_STREAM_LOGE() << "failed read " << nr_read << " diff bytes from patch file: " << m_stream->getName() << std::endl;
				fclose(src_fp);
				return ret;
			}

			for (int64_t i = 0; i < nr_read; ++i) {
				newBuf->m_buf[i]=oldBuf->m_buf[i]  + newBuf->m_buf[i];
			}
			nand_write_buffer.insert(nand_write_buffer.end(), newBuf->m_buf, newBuf->m_buf + nr_read);
			while (nand_write_buffer.size() >= nand_buf_size) {
				if (strstr((m_source->getPartName()).c_str(), "system")){
					m_dest->writeExt(nand_write_buffer.data(), nand_buf_size);
				} else {
					m_dest->writeFix(nand_write_buffer.data(), nand_buf_size);
				}
				nand_write_buffer.erase(nand_write_buffer.begin(), nand_write_buffer.begin()+nand_buf_size);
			}

			bytes_from_diff_block -= nr_read;
			offset +=nr_read;
		}
		newpos += ctrl[0];
		// handle extra
		File::Buffer::ptr extraBuf(new File::Buffer(0));
		//ATC_STREAM_LOGE() << " bytes_from_extra_block: " << bytes_from_extra_block << std::endl;
		while (bytes_from_extra_block) {
			int64_t nr_read = std::min(bytes_from_extra_block, m_limit);
			int64_t ret = 0;
			memset(extraBuf->m_buf, 0, nr_read);
			//ATC_STREAM_LOGE() << " bytes_from_extra_block: nr_read " << nr_read << std::endl;
			memset(extraBuf->m_buf, 0 , nr_read);
			ret = m_stream->read((uint8_t *)extraBuf->m_buf, nr_read);
			if (ret < 0) {
				ATC_STREAM_LOGE() << "failed read " << nr_read << " extra bytes from patch file" << std::endl;
				return ret;
			}

			nand_write_buffer.insert(nand_write_buffer.end(), extraBuf->m_buf, extraBuf->m_buf + nr_read);
			while (nand_write_buffer.size() >= nand_buf_size) {
				if (strstr((m_source->getPartName()).c_str(), "system")){
					m_dest->writeExt(nand_write_buffer.data(), nand_buf_size);
				} else {
					m_dest->writeFix(nand_write_buffer.data(), nand_buf_size);
				}
				nand_write_buffer.erase(nand_write_buffer.begin(), nand_write_buffer.begin()+nand_buf_size);
				}
			bytes_from_extra_block -= nr_read;
		}
		newpos += ctrl[1];
		offset += seek_in_source;
		m_source->skipRead(seek_in_source);
	}
	if (!nand_write_buffer.empty()) {
		if (strstr((m_source->getPartName()).c_str(), "system")){
			m_dest->writeExt(nand_write_buffer.data(), nand_write_buffer.size());
		} else {
			m_dest->writeFix(nand_write_buffer.data(), nand_write_buffer.size());
		}
	}
	fclose(src_fp);
	return 0;
}

}
}