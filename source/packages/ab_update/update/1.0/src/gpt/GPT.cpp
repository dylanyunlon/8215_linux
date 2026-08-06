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

#include <tinyxml2.h>

#include "gpt/GPT.hpp"
#include "common.h"
#include "utils/macro.hpp"
#include "utils/File.hpp"
#define EMMC_DEV    "/dev/datazone"
#define DATAZONE_DEV    "/dev/datazone"

#define PGPT_HEADER_START       SECTOR_SIZE
#define PGPT_ENTRY_START        (SECTOR_SIZE * 2)

#define PGPT_HEADER_LBA         (1)
#define PGPT_ENTRY_LBA          (2)

#define GPT_ENTRY_SIZE          (SECTOR_SIZE / 2)

#define MAX_GPT_COUNT           (50)

#define ATC_EMMC_SIZE_DEV       "/sys/block/mmcblk0/size"
#define PART_META_INFO_UUIDLEN  37
#define DATAZONE_LOAD_PART_HEAD 0x2000
#define DATAZONE_LOAD_PART_INFO 0x2200

namespace gpt {

GPT::GPT(const std::string &xml) {
    if (doInit(xml) == false) {
        m_inited = false;
    } else {
        m_inited = true;
    }
}

bool GPT::doInit(const std::string &xml) {
    if (xml.empty()) {
        m_fromEmmc = true;
        return fromEmmc();
    } else {
        return fromXml(xml);
    }
}
/*
static bool uuid_is_valid(const char *uuid) {
    unsigned int i;

    for (i = 0; i < (PART_META_INFO_UUIDLEN - 1); i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (uuid[i] != '-')
                return false;
        } else if (!isxdigit(uuid[i])) {
            return false;
        }
    }

    return true;
}

static int hex_to_bin(char ch) {
    if (std::isdigit(ch))
        return ch - '0';
    ch = tolower(ch);
    if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    return -1;
}

static int uuid_to_bin(const char *uuid, uint8_t b[16])
{
    static const uint8_t ei[16] = {3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15};
    static const uint8_t si[16] = {0, 2, 4, 6, 9, 11, 14, 16, 19, 21, 24, 26, 28, 30, 32, 34};
    unsigned int i;

    if (!uuid_is_valid(uuid))
        return -1;

    for (i = 0; i < 16; i++) {
        int hi = hex_to_bin(uuid[si[i] + 0]);
        int lo = hex_to_bin(uuid[si[i] + 1]);

        b[ei[i]] = (hi << 4) | lo;
    }

    return 0;
}
*/
#ifdef CONFIG_NAND_BOOT
bool GPT::UpdateGPT(const std::vector<PartInfo::ptr> &parts) {
    int rt = 0;
    GPTHeader::ptr header;
    char *datazone_buf;

    int buf_size = 0x80000;
    datazone_buf = (char *)malloc(buf_size);
    if (datazone_buf == nullptr) {
        ATC_STREAM_LOGE() << "Failed to alloc memory for File::Buffer, size : " << buf_size << std::endl;
        throw std::logic_error("out of memory");
    }
    memset(datazone_buf, 0, buf_size);

    // read partiton table
    rt = utils::FSUtil::readFix(DATAZONE_DEV, datazone_buf, 0, buf_size);
    if (rt != buf_size) {
        ATC_STREAM_LOGE() << "faild to read GPT Entry" << std::endl;
        free(datazone_buf);
        return false;
    }

	GPTEntry *ppartread,*pprepartition,*pcurpartition;
	ppartread = (GPTEntry *)(datazone_buf + DATAZONE_LOAD_PART_INFO);
	pcurpartition = ppartread;
	pprepartition = pcurpartition;

	while(pcurpartition != NULL)
	{
        for (const auto &part : parts) {
            uint64_t realSize = part->fileLen;
            if (part->part ==  std::string((char*)pcurpartition->partitionName)) {
                ATC_STREAM_LOGI() << "modify part : " << part->part
                    << " flag : " << std::hex << part->flag
                    << " realSize : " << realSize  << "" << std::endl;
                pcurpartition->flag = part->flag;
                pcurpartition->u8RealDataSize = part->fileLen;
            }
        }
		if (pcurpartition->u4LastPartition == 1)
		{
			//printf("readpartitioninfofromflash this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			break;
		} else {
        pcurpartition = pcurpartition + 1;
        //printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
        pprepartition->nextpartition = pcurpartition;
        pprepartition = pcurpartition;
		}
	}
    File::ptr nandFile = NandFile::Create(DATAZONE_DEV, File::NAND_WRITE);
    rt = nandFile->writeFix(datazone_buf, buf_size);
    if (rt < 0) {
        ATC_STREAM_LOGE() << "faild to write GPT Entries" << std::endl;
        free(datazone_buf);
        return false;
    }
    free(datazone_buf);
    return true;
}
#endif
bool GPT::writeEmmc(const std::vector<PartInfo::ptr> &parts) {
    int rt = 0;
    GPTHeader::ptr header;
    header.reset(new GPTHeader);
    rt = utils::FSUtil::readFix(EMMC_DEV, (char*)header.get(), DATAZONE_LOAD_PART_HEAD, sizeof(GPTHeader));
    if (rt != sizeof(GPTHeader)) {
        ATC_STREAM_LOGE() << "faild to read GPT Header" << std::endl;
        return false;
    }

    char *emmc_partinfo_buf;
    int buf_size = header->blockcnt * 512;
    emmc_partinfo_buf = (char *)malloc(buf_size);
    if (emmc_partinfo_buf == nullptr) {
        ATC_STREAM_LOGE() << "Failed to alloc memory for File::Buffer, size : " << buf_size << std::endl;
        throw std::logic_error("out of memory");
    }
    memset(emmc_partinfo_buf, 0, buf_size);

    // read partiton table
    rt = utils::FSUtil::readFix(EMMC_DEV, emmc_partinfo_buf, DATAZONE_LOAD_PART_INFO, buf_size);
    if (rt != buf_size) {
        ATC_STREAM_LOGE() << "faild to read GPT Entry" << std::endl;
        return false;
    }

	GPTEntry *ppartread,*pprepartition,*pcurpartition;

	ppartread = (GPTEntry *)(emmc_partinfo_buf);
	pcurpartition = ppartread;
	pprepartition = pcurpartition;

	while(pcurpartition != NULL)
	{
        for (const auto &part : parts) {
            uint64_t realSize = part->fileLen;
            if (part->part ==  std::string((char*)pcurpartition->partitionName)) {
                ATC_STREAM_LOGI() << "modify part : " << part->part
                    << " flag : " << std::hex << part->flag
                    << " realSize : " << realSize  << "" << std::endl;
                pcurpartition->flag = part->flag;
                pcurpartition->u8RealDataSize = part->fileLen;
            }
        }
		if (pcurpartition->u4LastPartition == 1)
		{
			//printf("readpartitioninfofromflash this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			break;
		} else {
        pcurpartition = pcurpartition + 1;
        //printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
        pprepartition->nextpartition = pcurpartition;
        pprepartition = pcurpartition;
		}
	}
    rt = utils::FSUtil::writeFix(EMMC_DEV, emmc_partinfo_buf, DATAZONE_LOAD_PART_INFO , buf_size);
    if (rt != buf_size) {
        ATC_STREAM_LOGE() << "faild to write GPT Entries" << std::endl;
        free(emmc_partinfo_buf);
        return false;
    }
    free(emmc_partinfo_buf);
    return true;
}

#ifdef CONFIG_NAND_BOOT
int GPT::getPartIdxFromXml(const char* partname)
{
    for (int i = 0; i< m_parts.size(); i++) {
        if (m_parts[i] && m_parts[i]->part == partname) {
           // ATC_STREAM_LOGI() << "find "<< partname << " index " << i << std::endl;
            return i;
        }
    }

    return -1;
}
#endif

bool GPT::fromXml(const std::string &xml) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile(xml.c_str());
    uint64_t lastEndAddr = 0;

    if (err != 0) {
        ATC_STREAM_LOGE() << "failed to load partition info scatter file path : "
                          << xml << std::endl;
        return false;
    }
    tinyxml2::XMLElement *root = doc.RootElement();
    for(tinyxml2::XMLElement *curEle = root->FirstChildElement(); curEle; curEle = curEle->NextSiblingElement()) {
        unsigned long size = 0;
        PartInfo::ptr pInfo(new PartInfo());
        GPTEntry::ptr entry(new GPTEntry());
        std::string type = curEle->Attribute("type");
        std::string name = curEle->Attribute("name");
        std::string imagename = curEle->Attribute("imagename");
        uint64_t startAddr = 0;
        uint32_t flag = 0;
        uint32_t mount = 0;

        sscanf(curEle->Attribute("startaddress"), "%llx", &startAddr);
        sscanf(curEle->Attribute("flag"), "%x", &flag);
        sscanf(curEle->Attribute("mount"), "%d", &mount);

        if (startAddr % 512) {
            ATC_STREAM_LOGE() << "invalid startaddress : " << std::hex << startAddr << std::endl;
            return false;
        }
        if (lastEndAddr > startAddr) {
            ATC_STREAM_LOGE() << "invalid scatter file, lastEndAddr(" << lastEndAddr << ") > startAddr("
                              << startAddr << ")" << std::endl;
            return false;
        }
        sscanf(curEle->Attribute("size"), "%lx", &size);
        if (size % 512) {
            ATC_STREAM_LOGE() << "invalid size : " << std::hex << size << std::endl;
            return false;
        }
        std::transform(type.begin(), type.end(), type.begin(), ::tolower);
        if (type == "ext4") {
            pInfo->raw = false;
        } else if (type == "raw") {
            pInfo->raw = true;
        } else {
            ATC_STREAM_LOGE() << "unsupport partition type : " << type << std::endl;
            return false;
        }

        pInfo->partitionStartAddr = startAddr;
        pInfo->image = imagename;
        pInfo->part =  name;
        pInfo->partsize = size;
        pInfo->flag = flag;
        pInfo->mount = mount;
        lastEndAddr = startAddr + size;
        m_parts.push_back(pInfo);
    }
    ATC_STREAM_LOGI() << "total partition count in scatterfile: " << m_parts.size() << std::endl;
    return true;
}

bool GPT::readHeader() {
    m_header.reset(new GPTHeader());
    ssize_t rt = 0;

    rt = utils::FSUtil::readFix(EMMC_DEV, (char*)m_header.get(), DATAZONE_LOAD_PART_HEAD, sizeof(GPTHeader));
    if (rt != sizeof(GPTHeader)) {
        ATC_STREAM_LOGE() << "faild to read GPT Header" << std::endl;
        return false;
    }

    return true;
}

bool GPT::fromEmmc() {
    if (readHeader() == false) {
        return false;
    }
   // ATC_STREAM_LOGI() << "partinfo header blkcount : " << m_header->blockcnt <<std::endl;
    char *partinfo_buf;
    int buf_size = m_header->blockcnt * 512;
    partinfo_buf = (char *)malloc(buf_size);
    if (partinfo_buf == nullptr) {
        ATC_STREAM_LOGE() << "Failed to alloc memory for File::Buffer, size : " << buf_size << std::endl;
        throw std::logic_error("out of memory");
    }
    memset(partinfo_buf, 0, buf_size);
    int rt = 0;

    // read partiton table

    rt = utils::FSUtil::readFix(EMMC_DEV, partinfo_buf, DATAZONE_LOAD_PART_INFO, buf_size);
    if (rt != buf_size) {
        ATC_STREAM_LOGE() << "faild to read GPT Entry" << std::endl;
        return false;
    }

	GPTEntry *ppartread,*pprepartition,*pcurpartition;
	ppartread = (GPTEntry *)(partinfo_buf);
	pcurpartition = ppartread;
	pprepartition = pcurpartition;

	while(pcurpartition != NULL)
	{
        PartInfo::ptr partinfo(new PartInfo());
        //fromWchar(pcurpartition->partitionName, partname, GPT_ENTRY_NAME_LEN);
        partinfo->part = std::string((char*)pcurpartition->partitionName);
        partinfo->image = std::string((char*)pcurpartition->imageName);
        partinfo->partsize = pcurpartition->u8PartitionSize;
        partinfo->partitionStartAddr =  pcurpartition->u8PartitionStartAddr;
       // ATC_STREAM_LOGI() << "part name : " << partinfo->part  << " img name : " << partinfo->image <<std::endl;
        if (std::string((char*)pcurpartition->partType) == std::string("ext4")) {
            partinfo->raw = false;
        } else if (std::string((char*)pcurpartition->partType) == std::string("raw")) {
            partinfo->raw = true;
        } else {
            ATC_STREAM_LOGE() << "unsupport fs type : " << std::string((char*)pcurpartition->partType) << std::endl;
            free(partinfo_buf);
            return false;
        }
        partinfo->header = nullptr;
        m_parts.push_back(partinfo);

		if (pcurpartition->u4LastPartition == 1)
		{
			//printf("readpartitioninfofromflash this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			break;
		} else {
        pcurpartition = pcurpartition + 1;
        //printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
        pprepartition->nextpartition = pcurpartition;
        pprepartition = pcurpartition;
		}
	}
    free(partinfo_buf);
    return true;
}

uint64_t GPT::getPartitionRealsize(std::string partition_name) {
    GPTEntry::ptr entry(new GPTEntry());
    if (readHeader() == false) {
        return false;
    }

    uint64_t Realsize = 0;
    int rt = 0;
    char *partinfo_buf;
    int buf_size = m_header->blockcnt * 512;
    partinfo_buf = (char *)malloc(buf_size);
    if (partinfo_buf == nullptr) {
        ATC_STREAM_LOGE() << "Failed to alloc memory for File::Buffer, size : " << buf_size << std::endl;
        throw std::logic_error("out of memory");
    }
    memset(partinfo_buf, 0, buf_size);

    // read partiton table
    rt = utils::FSUtil::readFix(EMMC_DEV, partinfo_buf, DATAZONE_LOAD_PART_INFO, buf_size);
    if (rt != buf_size) {
        ATC_STREAM_LOGE() << "faild to read GPT Entry" << std::endl;
        free(partinfo_buf);
        return false;
    }

	GPTEntry *ppartread,*pprepartition,*pcurpartition;

	ppartread = (GPTEntry *)(partinfo_buf);
	pcurpartition = ppartread;
	pprepartition = pcurpartition;

	while(pcurpartition != NULL)
	{
        if (partition_name  ==  std::string((char*)pcurpartition->partitionName)) {
                Realsize = pcurpartition->u8RealDataSize;
            ATC_STREAM_LOGI() << " part : " << pcurpartition->partitionName << " realSize : " << Realsize  << std::endl;
            break;
        }
        if (pcurpartition->u4LastPartition == 1){
            //printf("readpartitioninfofromflash this is last partition\r\n");
            pcurpartition->nextpartition = NULL;
        } else {
        pcurpartition = pcurpartition + 1;
        //printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
        pprepartition->nextpartition = pcurpartition;
        pprepartition = pcurpartition;
		}
	}
    free(partinfo_buf);
    return Realsize;
}

std::vector<std::string> GPT::getPartnameFromXml() {
    std::vector<std::string> parts;

    for (auto &item : m_parts) {
        parts.push_back(item->part);
    }

    return parts;
}

std::vector<PartInfo::ptr> GPT::getABUpdateSet() {
    std::vector<PartInfo::ptr> updateSet;

    for (auto &item : m_parts) {
        if ((item->part == "datazone_bk")) {
            ATC_STREAM_LOGE() << "datazone_bk partition no need to add updateSet" << std::endl;
            continue;
        }

        if (utils::FSUtil::isUpdatePart(item->part) || 
            item->part == "preloader" ||
            item->part == "preloaderbk") {
            updateSet.push_back(item);
        }
    }

    return updateSet;
}

void GPT::fromWchar(const uint8_t *from, uint8_t *to, uint64_t count) {
    uint32_t i = 0;

    for (i = 0; i < count && from[i]; ++i) {
        to[i] = (uint8_t)(from[i] & 0x00ff);
    }
    to[i] = 0;
}

void GPT::toWchar(const uint8_t *from, uint16_t *to, uint64_t count) {
    uint32_t i = 0;

    for (i = 0; i < count && from[i]; i++) {
        to[i] = (uint16_t)(from[i]);
    }
    to[i] = (uint16_t)0;
}

bool GPT::updatable(GPT::ptr xmlGpt) {
    std::vector<PartInfo::ptr> emmcEntries = getABUpdateSet();
    std::vector<PartInfo::ptr> xmlEntries =  xmlGpt->getABUpdateSet();
    if (emmcEntries.size() != xmlEntries.size()) {
        ATC_STREAM_LOGE() << "entry count not match emmc : " << emmcEntries.size()
                          << "xml : " << xmlEntries.size() << std::endl;
        return false;
    }
    sort(emmcEntries.begin(), emmcEntries.end(), GPTEntryComparor());
    sort(xmlEntries.begin(), xmlEntries.end(), GPTEntryComparor());
    for (unsigned i = 0; i < emmcEntries.size(); ++i) {
        if (xmlEntries[i]->part != emmcEntries[i]->part ||
            xmlEntries[i]->partitionStartAddr != emmcEntries[i]->partitionStartAddr ) {
            ATC_STREAM_LOGE() << "xml partname : " << xmlEntries[i]->part
                              << "emmc partname : " << emmcEntries[i]->part << std::endl;
            ATC_STREAM_LOGE() << "xml partitionStartAddr : " << xmlEntries[i]->partitionStartAddr
                              << "emmc partitionStartAddr : " << emmcEntries[i]->partitionStartAddr << std::endl;
            return false;
        }
    }

    return true;
}

}
