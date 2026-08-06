#pragma once

#include <string.h>
#include <assert.h>

#include <memory>
#include <algorithm>
#include <exception>

#include <iostream>

#define UNIX_SOCKETADDR     "/tmp/abupdate.sock"

#define REQUEST_TYPE       0x53123241
#define RESPONSE_TYPE      0x17931234

#define BYTEARRAY_MAX_SIZE          ((uint32_t)(8 * 1024))
#define BYTEARRAY_INITIAL_SIZE      ((uint32_t)64)

namespace atcupdateservice {
namespace ipc {
namespace socket {

enum Operations {
    SEND_PROGRESS = 0x100,
    SEND_MESSAGE,
    START_SERVICE,
    BEGIN_UPDATE,
    GET_PROGRESS,
    SUBSCRIBE,
    HEARTBEAT,
    CHECK_UPDATING,
    GET_SYSTEM_VERSION,
    GET_LAST_STATUS,
};

#define      SOCKET_OK                  0
#define      SOCKET_BADREQ              1
#define      SOCKET_NOOP                2
#define      SOCKET_BROKEN_PACKAGE      3
#define      SOCKET_UNKNOWN             4

inline bool checkLittleEndia() {
    uint16_t tmp = 0x1234;
    uint8_t *byte = (uint8_t*)&tmp;

    if (*byte == 0x12) return false;

    return true;
}

//T must be a POD type value
template<class T>
T byteSwapOnLittleEndian(T val) {
    static bool isLittleEndian = checkLittleEndia();

    if (isLittleEndian) {
        uint8_t *buf = (uint8_t*)&val;
        reverse(val, val + sizeof(T));
    }

    return val;
}

inline uint64_t lowbit(uint64_t val) {
    return (val & (uint64_t)(-(int64_t)val));
}
class ByteArray;

struct ProtocolHeader {
    typedef std::shared_ptr<ProtocolHeader> ptr;
    //op = -1 means matching any operation
    static bool verifyPackage(std::shared_ptr<ByteArray> ba, uint32_t type, uint32_t op);
    uint32_t checksum;
    uint32_t type;
    int32_t status;
    uint32_t op;
    uint32_t size; //data size
    uint32_t nr_para;
    uint8_t data[0];
};

class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;
    ByteArray(const uint8_t *buf, uint32_t size);
    ByteArray();

    ByteArray(const ByteArray &ba) = delete;
    ~ByteArray() {
        clearBuffer();
    }
    bool writeBinary(const uint8_t *buf, uint32_t size);
    bool readBinary(uint8_t *buf, uint32_t size);
    template<class T>
    bool write(const T &val) {
        return writeBinary((uint8_t*)&val, sizeof(val));
    }
    bool write(const std::string& str) {
        if (str.size() > BYTEARRAY_MAX_SIZE) {
            return false;
        }
        if (write<uint32_t>((uint32_t)str.size()) == false) {
            return false;
        }
        return writeBinary((const uint8_t*)str.c_str(), str.size());
    }
    bool write(const char *str) {
        if (str == nullptr) return false;
        uint32_t size = strlen(str);
        if (size > BYTEARRAY_MAX_SIZE) {
            return false;
        }
        if (write<uint32_t>(size) == false) {
            return false;
        }
        return writeBinary((const uint8_t*)str, size);
    }
    template<class T>
    bool read(T &val) {
        return readBinary((uint8_t*)&val, sizeof(val));
    }
    bool read(std::string &val) {
        uint32_t dataSize = 0;
        if (read<uint32_t>(dataSize) == false) {
            return false;
        }
        //in case of the string size that pass by client is too big
        if (getAvailable() < dataSize) return false;
        val.resize(dataSize);
        return readBinary((uint8_t*)&val[0], dataSize);
    }

    bool seek(uint32_t pos);
    bool ok() { return m_buf != NULL; }
    uint32_t size() const {
        return m_size;
    }
    uint32_t getPos() const {
        return m_cur;
    }
    uint32_t getAvailable() const {
        assert(m_size >= m_cur);
        return m_size - m_cur;
    }
    uint32_t getCapacity() const {
        return m_capacity;
    }
    // be advised that buffer may not available after write/seek operation of byte array
    const uint8_t *getBufferRef() {
        return m_buf;
    }
private:
    bool realloc(uint32_t size);
    void clearBuffer();
private:
    uint8_t *m_buf;
    uint32_t m_cur;
    uint32_t m_size;
    uint32_t m_capacity;
};

extern uint32_t checksum32(uint32_t lastSum, const char *buf, unsigned len);

template<class... Args>
ByteArray::ptr serialize(uint32_t op, uint32_t type,
                         uint32_t status, const Args&... args) {
    ProtocolHeader header;
    uint32_t nargs = 0;
    memset(&header, 0, sizeof(header));

    ByteArray::ptr ba(new ByteArray());
    //skip the protocal header;
    if (ba->seek(sizeof(ProtocolHeader)) == false) {
        return nullptr;
    }
    ((ba->write(args), ++nargs),...);

    if (!ba->ok()) {
        return nullptr;
    }
    header.type = type;
    header.op = op;
    header.status = status;
    header.size = ba->size();
    header.nr_para =  nargs;
    header.checksum = 0;
    ba->seek(0);
    ba->writeBinary((const uint8_t*)&header, sizeof(header));
    header.checksum = checksum32(0, (const char*)ba->getBufferRef(), ba->size());
    ba->seek(0);
    ba->writeBinary((const uint8_t*)&header.checksum, sizeof(uint32_t));
    return ba;
}

template<class... Args>
ByteArray::ptr make_response(uint32_t op, int32_t status, const Args&... args) {
    ByteArray::ptr ba = serialize(op, RESPONSE_TYPE, status, args...);
    return ba;
}

template<class... Args>
ByteArray::ptr make_request(uint32_t op, int32_t status, const Args&... args) {
    ByteArray::ptr ba = serialize(op, REQUEST_TYPE, status, args...);
    return ba;
}

}
}
}