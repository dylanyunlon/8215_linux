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
#pragma once

#include <unistd.h>
#include <memory>
#include <sstream>

#include "utils/Util.hpp"

namespace gpt {

#ifndef HAVE_ATC_CHANGE
#define HAVE_ATC_CHANGE
#endif

#define GUID_SIZE               16
#define GPT_ENTRY_NAME_SIZE     72
#define GPT_ENTRY_NAME_LEN      (GPT_ENTRY_NAME_SIZE/sizeof(uint16_t))

#define PROGRAM_NAME "nandupgrade"

using namespace atcupdateservice;
using namespace atcupdateservice::utils;

struct GUID {
    uint8_t b[GUID_SIZE];
};

struct GPTHeader {
    typedef std::shared_ptr<GPTHeader> ptr;
	unsigned int blockcnt;
	unsigned int u4Version;
	unsigned int u4Signature;
	struct GPTPartitionread *nextpartition;
};

struct GPTEntry {
    typedef std::shared_ptr<GPTEntry> ptr;
	char      partitionName[20];
	char      partType[20];
	unsigned int u4Mount;
	//unsigned int  u4PartitionStartAddr;
	unsigned long long  u8PartitionStartAddr;
	//unsigned int   u4PartitionSize;
	unsigned long long  u8PartitionSize;
	unsigned int  u4LastPartition;
	char  imageName[48];   //40 --> 48
	unsigned long long u8RealDataSize;
	unsigned int flag;         //add u4Flag

	struct  GPTEntry *nextpartition;

};

struct nand_dev_info {
	int block_cnt;
	int block_size;
	int page_size;
	int oob_size;
};

class GPT {
public:
    typedef std::shared_ptr<GPT> ptr;
    GPT(const std::string &xml = "");
    bool updatable(GPT::ptr xmlGpt);
    std::vector<PartInfo::ptr> getABUpdateSet();
    std::vector<std::string> getPartnameFromXml();
#ifdef HAVE_ATC_CHANGE
    std::string getImageName(const std::string &name);
#endif
    bool doInit(const std::string &xml);
    bool fromXml(const std::string &xml);
    bool fromEmmc();
    bool readHeader();
    //by sector
    bool readEntry(uint64_t start);
    static void fromWchar(const uint8_t *from, uint8_t *to, uint64_t count);
    static void toWchar(const uint8_t *from, uint16_t *to, uint64_t count);
    uint64_t getPartitionRealsize(std::string partition_name);

    static bool writeEmmc(const std::vector<PartInfo::ptr> &parts);
#ifdef CONFIG_NAND_BOOT
    static bool UpdateGPT(const std::vector<PartInfo::ptr> &parts);
    int getPartIdxFromXml(const char* partname);
#endif
private:
    static bool writeGPT(GPTEntry *entries, uint64_t startLba);
private:
    bool m_fromEmmc;
    bool m_inited = false;
    GPTHeader::ptr m_header;
    std::vector<PartInfo::ptr> m_parts;
};

struct GPTEntryComparor {
    bool operator()(const PartInfo::ptr &lh, const PartInfo::ptr &rh) const {
        return std::string(lh->part) < std::string(rh->part);
    }
};

}
