#define  AVB_COMPILATION
#include <libavb.h>
#include <avb_sha.h>
#include <avb_rsa.h>
#include <atccrypto.h>

#include "utils/Util.hpp"
#include "utils/CryptoImage.hpp"

#ifndef OEM_PUBK_SZ
#define OEM_PUBK_SZ 256
#endif

static uint8_t g_avb_key[OEM_PUBK_SZ] = { OEM_PUBK };

namespace atcupdateservice {
namespace utils {

static void converseEndian(uint8_t *text, uint8_t len) {
    uint8_t head, tail, tmp;

    if (!text)
        return;

    for (head = 0, tail = len - 1, tmp = 0; head < tail; head++, tail--) {
        tmp = text[head];
        text[head] = text[tail];
        text[tail] = tmp;
    }
}

static bool verifyImageHeader(part_hdr_sd_t *p_header, const char *name)
{
    assert(p_header != NULL);
	assert(name != NULL);

    if (p_header->info.magic == PART_MAGIC) {
        ATC_STREAM_LOGI() << "verify_image Image with part header" << std::endl;
        ATC_STREAM_LOGI() << "verify_image pubkey_offset : 0x" << std::hex << p_header->info.pubkey_offset << std::endl;
        ATC_STREAM_LOGI() << "verify_image pubkey_len : 0x" << std::hex << p_header->info.pubkey_len << std::endl;
        ATC_STREAM_LOGI() << "verify_image hash_sign_offset : 0x" << std::hex << p_header->info.hash_sign_offset << std::endl;
        ATC_STREAM_LOGI() << "verify_image hash_sign_len: 0x" << std::hex << p_header->info.hash_sign_len << std::endl;
    } else {
        ATC_STREAM_LOGE() <<  "verify " << name << " header magic failed." << std::endl;
        return false;
    }

    return true;
}

static bool verifyImageBody(part_hdr_sd_t *p_header, void *body, const char *name)
{
    uint8_t *computed_hash = NULL;
    uint8_t *pubk = NULL;
    uint32_t pubk_sz = 0;
    bool rt = false;
    AvbSHA256Ctx sha256_ctx;
    AvbRSAPublicKeyHeader *key_hdr;
    const AvbAlgorithmData *algorithm;

    avb_assert(p_header != NULL);
    avb_assert(body != NULL);
    avb_assert(name != NULL);

    /* Ensure algorithm field is supported. */
    algorithm = avb_get_algorithm_data(AVB_ALGORITHM_TYPE_SHA256_RSA2048);
    if (!algorithm) {
        throw std::logic_error("Invalid or unknown algorithm");
    }

    /* Calculate the hash value from body */
    avb_sha256_init(&sha256_ctx);
    avb_sha256_update(&sha256_ctx, (uint8_t*)body,
                      p_header->info.pubkey_offset - SECTOR_SIZE);
    computed_hash = avb_sha256_final(&sha256_ctx);

    /* RSA Verify: only software */
    rt = avb_rsa_verify((uint8_t*)((uint8_t*)body + p_header->info.pubkey_offset - \
                         sizeof(part_hdr_sd_t)),
                         p_header->info.pubkey_len,
                         (uint8_t*)((uint8_t*)body + p_header->info.hash_sign_offset - \
                         sizeof(part_hdr_sd_t)),
                         p_header->info.hash_sign_len,
                         computed_hash,
                         AVB_SHA256_DIGEST_SIZE,
                         algorithm->padding,
                         algorithm->padding_len);
    if (rt == false) {
        throw std::logic_error("Signature verify failed!");
    }

    /* start check pubk */
    key_hdr = (AvbRSAPublicKeyHeader *)((uint8_t*)body + p_header->info.pubkey_offset \
        - sizeof(part_hdr_sd_t));
    pubk = (uint8_t *)((uint8_t*)body + p_header->info.pubkey_offset \
        - sizeof(part_hdr_sd_t) + sizeof(AvbRSAPublicKeyHeader));
    pubk_sz = avb_htobe32(key_hdr->key_num_bits) / 8;

    if (memcmp((void *)g_avb_key, (void *)pubk, pubk_sz)) {
        ATC_STREAM_LOGI() << "dump pubkey info..." << std::endl;
        utils::dumpMemory(pubk, pubk_sz);
        throw std::logic_error("RSA pubkey verify failed!");
    }

    return true;
}

int verifyBootImage(void *p_header, void *body, const char *name)
{
    bool rt = false;

    /* verify header */
    rt = verifyImageHeader((part_hdr_sd_t *)p_header, name);
    if (rt == false) {
        ATC_STREAM_LOGC() << name << " header verify fail!" << std::endl;
        return rt;
    }

    /* verify image */
    rt = verifyImageBody((part_hdr_sd_t *)p_header, body, name);
    if (rt == false) {
        ATC_STREAM_LOGC() << name << " header verify fail!" << std::endl;
        return rt;
    }

    return rt;
}

static bool sha256Encrypto(const uint8_t *body, uint32_t inputSize, uint8_t *output) {
    AvbSHA256Ctx ctx;

    uint8_t *res = nullptr;
    avb_sha256_init(&ctx);
    avb_sha256_update(&ctx, body, inputSize);
    res = avb_sha256_final(&ctx);
    memcpy(output, res, BOOTL_HEADER_HEADERHASH);
    return true;
}

static bool createPartHeader(void *part_header, void *body, uint32_t image_size) {
    part_hdr_emmc_t *emmc_header = (part_hdr_emmc_t *)part_header;
    bool rt = false;

    emmc_header->info.magic = PART_MAGIC;

    rt = sha256Encrypto((uint8_t*)body, image_size, emmc_header->info.body_hash);
    if (rt == false) {
        ATCLOGC("create header hash failed!\n");
        return false;
    }
    converseEndian(emmc_header->info.body_hash, BOOTL_HEADER_HEADERHASH);
    /* the image and verify header will be seperate to encrypt */
    if(atccrypto_aes_ecb_encrypt(part_header, part_header, PART_HEADER_SIZE) != 0) {
        ATC_STREAM_LOGE() << "failed to enctypto partition header throught aes_ecb" << std::endl;
        return false;
    }

    return true;
}

/* for trustzone/lk/hsm os */
static bool encryptBootPart(void *part_header, void *body, uint32_t image_size) {
    bool rt = 0;

    /* encrypt image header by aes & calc hash */
    rt = createPartHeader(part_header, body, image_size);
    if (rt == false) {
        ATC_STREAM_LOGE() << "failed to create partition header!" << std::endl;
        return false;
    }
    /* encrypt image body by aes */
    if (atccrypto_aes_ecb_encrypt(body, body, image_size) != 0) {
        ATC_STREAM_LOGE() << "failed to enctypto partition throught aes_ecb" << std::endl;
        return false;
    }

    return true;
}

CryptoImage::CryptoImage(std::string partname, std::string imagename)
    : RawImage(partname, imagename, NO_LIMIT) {
}

bool CryptoImage::writePartition() {
    int64_t cryptoSize = ALIGN(m_imageSize, SECTOR_SIZE);
    int64_t blknum = cryptoSize / SECTOR_SIZE;
    bool rt = false;
    assert(cryptoSize <= m_partSize);
    auto data = m_image->readFix(m_imageSize, SECTOR_SIZE);
    char *buf = data->m_buf;

    assert(blknum * SECTOR_SIZE == cryptoSize);
    assert(data->m_realSize == cryptoSize);
    memset(buf + m_imageSize, 0, cryptoSize - m_imageSize);
    if (m_partname == "viss_a" || m_partname == "viss_b") {
        viss_header_t *viss_header = (viss_header_t *)buf;

        rt = verifyBootImage(data->m_buf + SECTOR_SIZE, data->m_buf + SECTOR_SIZE + SECTOR_SIZE, m_partname.c_str());
        if (rt == false) {
            ATCLOGC("viss verify fail!\n");
            throw std::logic_error("failed to verify image : " + m_imagename);
        }
        ATCLOGI("encrypt viss part1 size:0x%x\r\n", viss_header->header.Img1Len - SECTOR_SIZE);
        rt = encryptBootPart((void *)(buf + SECTOR_SIZE),
                          (void *)(buf + SECTOR_SIZE + SECTOR_SIZE),
                          ALIGN(viss_header->header.Img1Len - SECTOR_SIZE, SECTOR_SIZE));
        if (rt == false) {
            throw std::logic_error("failed to encrypto image: viss1");
        }
        /* encrypt viss2 */
        ATCLOGI("encrypt viss part2 size:0x%x\r\n", viss_header->header.Img2Len - SECTOR_SIZE);
        rt = encryptBootPart((void *)(buf+ SECTOR_SIZE + viss_header->header.Img1Len),
                          (void *)(buf + SECTOR_SIZE + viss_header->header.Img1Len + SECTOR_SIZE),
                          ALIGN(viss_header->header.Img2Len - SECTOR_SIZE, SECTOR_SIZE));
        if (rt == false) {
            throw std::logic_error("failed to encrypto image: viss2");
        }
    } else {
        rt = verifyBootImage(buf, buf + SECTOR_SIZE, m_partname.c_str());
        if (rt == false) {
            ATCLOGE("%s verify fail!\n", m_imagename.c_str());
            throw std::logic_error("failed to verify image : " + m_imagename);
        }
        ATCLOGI("%s verify success!\n", m_partname.c_str());
        /* encrypt total part */
        rt = encryptBootPart((void *)buf, (void *)(buf + SECTOR_SIZE), (blknum - 1) * SECTOR_SIZE);
        if (rt == false) {
            throw std::logic_error("failed to encrypto image: " + m_imagename + " part: " + m_partname);
        }
    }
    if (m_part->writeFix(buf, cryptoSize) != cryptoSize) {
        throw std::logic_error("failed to write encrypto image!");
    }
    return true;
}

}
}