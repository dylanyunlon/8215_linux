#pragma once

#ifdef BOARD_AVB_ENABLE
#include <assert.h>

#include <memory>
#include <algorithm>

#include "utils/Util.hpp"
#include "utils/Image.hpp"

#define PRELOADER_HEADER_SIZE   1024
#define BOOTL_HEADER_ID1SIZE    12
#define BOOTL_HEADER_HEADERHASH 32
#define BOOTL_HEADER_KEY        520
#define BOOTL_HEADER_AUTH       256
#define BOOTL_HEADER_ID2SIZE    8
#define PART_HEADER_SIZE        512
#define ENCRYPT_PART_NAME       16

#define PART_MAGIC        0x58881688

namespace atcupdateservice {
namespace utils {

typedef union {
    struct {
        unsigned int magic;     /* partition magic */
        char name[32];          /* partition name */
        char hash_algorithm[32];
        unsigned int pubkey_offset;
        unsigned int pubkey_len;
        unsigned int hash_sign_offset;
        unsigned int hash_sign_len;
    } info;
    unsigned char data[512];
} part_hdr_sd_t;

typedef union {
    struct {
        uint32_t magic;               /* partition magic */
        uint8_t name[32];          /* partition name */
        uint8_t body_hash[32];     /* body hash */
        uint8_t body_hash_verify[32];
    } info;
    unsigned char data[512];
} part_hdr_emmc_t;

typedef union viss_header {
    struct {
        char ID[12];
        uint32_t Img1Len;
        uint32_t Img2Len;
    } header;

    char reserved[512];
} viss_header_t;

class CryptoImage : public RawImage {
public:
    typedef std::shared_ptr<CryptoImage> ptr;
    CryptoImage(std::string partname, std::string imagename);
    ~CryptoImage() {}
    bool writePartition() override;
};


}
}

#endif