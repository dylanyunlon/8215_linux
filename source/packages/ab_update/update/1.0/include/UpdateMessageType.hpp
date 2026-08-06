#pragma once

#include <string>

namespace atcupdateservice {

enum LastUpdateStatus {
    LAST_UPDATE_OK,
    LAST_FAIL,
    LAST_NO_UPDATE,
    LAST_UNFINISHED,
    LAST_UNKNOWN,
};

#define ENUM_TYPE(XX)       \
    XX(PAUSE)               \
    XX(RESUME)              \
    XX(ERROR)               \
    XX(FINISHED)            \
    XX(START)               \
    XX(UMOUNTED)            \
    XX(CHECK)

enum class UpdateMessageType {
#define XX(TYPE)        TYPE,
    ENUM_TYPE(XX)
#undef XX
};

#define ENUM_ERROR_TYPE(XX)                                         \
    XX(UPD_OK,    0, "success!")                                    \
    XX(UPD_ENOOP, 1, "operation not supported")                     \
    XX(UPD_EBOOTCTL, 2, "failed to read/write bootCTRL")            \
    XX(UPD_EBADREQ, 3, "invalid begin update request")              \
    XX(UPD_EBUSY, 4, "another update is on-going now!")             \
    XX(UPD_EIDLE, 5, "no active update is on-going now!")           \
    XX(UPD_ECONN, 6, "failed to connect to atcupdateservice")       \
    XX(UPD_ENOAB, 7, "running on a non-ab system")

enum UpdErrorType {
#define XX(NAME, VALUE, DESC) NAME=VALUE,
    ENUM_ERROR_TYPE(XX)
#undef XX
};

inline std::string UpdateMessageStr(UpdateMessageType type) {
#define XX(TYPE) \
    if (UpdateMessageType::TYPE == type) return #TYPE;
    ENUM_TYPE(XX)
#undef XX
    return "UNKNOWN";
}

inline std::string UpdErrorStr(UpdErrorType type) {
#define XX(NAME, VALUE, DESC)         \
    if (NAME == type) {               \
        return std::string(#DESC);    \
    }
    ENUM_ERROR_TYPE(XX)
#undef XX
    return std::string("UNKNOWN ERROR");
}


}