#include "ipc/socket/SocketProto.hpp"

namespace atcupdateservice {
namespace ipc {
namespace socket {

uint32_t checksum32(uint32_t lastSum, const char *buf, unsigned len) {
    const char *end = nullptr;
    for (end = buf + len; buf != end; ++buf)
        lastSum += *buf;
    return lastSum;
}

ByteArray::ByteArray(const uint8_t *data, uint32_t size)
    : m_buf(nullptr), m_cur(0), m_size(0) {
    if (size == 0 || size > BYTEARRAY_MAX_SIZE) return;

    m_capacity = std::min(std::max(BYTEARRAY_INITIAL_SIZE, size * 2), BYTEARRAY_MAX_SIZE);
    m_cur = size;
    m_size = size;
    m_buf = (uint8_t*)malloc(m_capacity);
    if (m_buf) {
        memcpy(m_buf, data, size);
    } else {
        clearBuffer();
    }
}

ByteArray::ByteArray()
    : m_buf(nullptr), m_cur(0), m_size(0) {
    m_capacity = BYTEARRAY_INITIAL_SIZE;
    m_size = 0;
    m_cur = 0;
    m_buf = (uint8_t*)malloc(m_capacity);
    if (m_buf == NULL) {
        clearBuffer();
    }
}

bool ByteArray::realloc(uint32_t size) {
    if (m_buf == NULL) {    //bad buffer status
        return false;
    }
    if (size < m_size) {    // no need to resize, just return
        return true;
    } else if (size < m_capacity) { // required size didn't exceed the capacity of the byte array
        m_size = size;
        return true;
    }
    if (size > BYTEARRAY_MAX_SIZE) {    // required size exceed the max size of the byte array, don't alloc memory
        return false;
    }
    m_capacity = std::min(size * 2, BYTEARRAY_MAX_SIZE);
    m_buf = (uint8_t*)::realloc(m_buf, m_capacity);
    if (m_buf == NULL) {
        clearBuffer();
        return false;
    }
    m_size = size;

    return true;
}

bool ByteArray::writeBinary(const uint8_t *buf, uint32_t size) {
    uint32_t requiredSize = m_cur + size;
    if (buf == nullptr && !ok()) {
        return false;
    }
    if (!realloc(requiredSize)) {
        return false;
    }
    //just in case
    if (requiredSize > m_size) return false;
    memcpy(m_buf + m_cur, buf, size);
    m_cur += size;

    return true;
}

bool ByteArray::readBinary(uint8_t *buf, uint32_t size) {
    if (buf == nullptr && !ok()) {
        return false;
    }
    if (getAvailable() < size) {
        return false;
    }
    memcpy(buf, m_buf + m_cur, size);
    m_cur += size;

    return true;
}

bool ByteArray::seek(uint32_t pos) {
    if (!ok()) {
        return false;
    }
    if (pos == (uint32_t)-1) {
        m_cur = m_size;
        return true;
    } else {
        if (realloc(pos)) {
            m_cur = pos;
            return true;
        }
        return false;
    }
}

void ByteArray::clearBuffer() {
    if (m_buf) free(m_buf);
    m_cur = 0;
    m_size = 0;
    m_capacity = 0;
}

bool ProtocolHeader::verifyPackage(ByteArray::ptr ba, uint32_t type, uint32_t op) {
    const ProtocolHeader *header = (const ProtocolHeader*)ba->getBufferRef();

    if (ba->size() < sizeof(ProtocolHeader) || header->type != type ||
        (header->op != op && op != (uint32_t)-1)) {
        return false;
    }
    if (header->size != ba->size()) {
        return false;
    }
    if (header->checksum != checksum32(0, (const char*)header + sizeof(uint32_t), header->size - sizeof(uint32_t))) {
        return false;
    }

    return true;
}

}
}
}