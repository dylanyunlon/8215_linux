#include "ipc/socket/SocketServer.hpp"
#include "UpdateMessageType.hpp"
#include "ipc/IPC.hpp"

namespace atcupdateservice {
namespace ipc {
namespace socket {

int checkRequest(ByteArray::ptr ba, uint32_t op, uint32_t &bodySize) {
    ProtocolHeader header;
    int rt = -1;
    bodySize = 0;
    if (ProtocolHeader::verifyPackage(ba, REQUEST_TYPE, op) == false) {
        goto out;
    }
    ba->seek(0);
    ba->readBinary((uint8_t*)&header, sizeof(header));
    rt = header.nr_para;
    bodySize = header.size - sizeof(header);
out:
    ba->seek(0);
    return rt;
}

static response_type startService(ByteArray::ptr ba) {
    uint32_t bodySize = 0;
    int rt = 0;
    ATCUpdateServiceRPC::ptr rpc = ATCUpdateServiceRPC::getInstance();
    int args = checkRequest(ba, START_SERVICE, bodySize);
    if (args != 0 || bodySize != 0) {
        ATC_STREAM_LOGE() << "invalid arguement count!" << std::endl;
        rt = -(int)UPD_EBADREQ;
        goto out_err;
    }
    rt = rpc->startService();
out_resp:
    return make_response(START_SERVICE, rt);
out_err:
    std::cout << "failed to call start_service" << std::endl;
    goto out_resp;
}

static response_type beginUpdate(ByteArray::ptr ba) {
    uint32_t bodySize = 0;
    int rt = 0;
    std::string pathname, method;
    ATCUpdateServiceRPC::ptr rpc = ATCUpdateServiceRPC::getInstance();
    int args = checkRequest(ba, BEGIN_UPDATE, bodySize);
    if (args != 2) {
        ATC_STREAM_LOGE() << "invalid arguement count!" << std::endl;
        rt = -(int)UPD_EBADREQ;
        goto out_err;
    }
    ba->seek(sizeof(ProtocolHeader));
    if (!ba->read(pathname) || !ba->read(method)) {
        rt = -(int)UPD_EBADREQ;
        goto out_err;
    }
    rt = rpc->beginUpdate(pathname, method);
out_resp:
    return make_response(BEGIN_UPDATE, rt);
out_err:
    std::cout << "failed to call begin_update" << std::endl;
    goto out_resp;
}

static response_type getProgress(ByteArray::ptr ba) {
    uint32_t bodySize;
    int rt = 0;
    uint32_t progress = 0;
    ATCUpdateServiceRPC::ptr rpc = ATCUpdateServiceRPC::getInstance();
    int args = checkRequest(ba, GET_PROGRESS, bodySize);
    if (args != 0 || bodySize != 0) {
        ATC_STREAM_LOGE() << "invalid arguement count!" << std::endl;
        rt = -(int)UPD_EBADREQ;
        goto out_err;
    }
    rt = rpc->getProgress(progress);
out_resp:
    return make_response(GET_PROGRESS, rt, (uint32_t)progress);
out_err:
    std::cout << "failed to call get_progress" << std::endl;
    goto out_resp;
}

static response_type getLastStatus(ByteArray::ptr ba) {
    uint32_t bodySize;
    int rt = 0;
    LastUpdateStatus status = LAST_UNKNOWN;

    ATCUpdateServiceRPC::ptr rpc = ATCUpdateServiceRPC::getInstance();
    int args = checkRequest(ba, GET_LAST_STATUS, bodySize);
    if (args != 0 || bodySize != 0) {
        ATC_STREAM_LOGE() << "invalid arguement count!" << std::endl;
        rt = -(int)UPD_EBADREQ;
        goto out_err;
    }
    status = rpc->getLastStatus();
out_resp:
    return make_response(GET_LAST_STATUS, rt, (uint32_t)status);
out_err:
    ATC_STREAM_LOGE() << "failed to call getLastStatus" << std::endl;
    goto out_resp;
}

static response_type getSystemVersion(ByteArray::ptr ba) {
    uint32_t bodySize;
    int rt = 0;
    std::string version = "error";

    ATCUpdateServiceRPC::ptr rpc = ATCUpdateServiceRPC::getInstance();
    int args = checkRequest(ba, GET_SYSTEM_VERSION, bodySize);
    if (args != 0 || bodySize != 0) {
        ATC_STREAM_LOGE() << "invalid arguement count!" << std::endl;
        rt = -(int)UPD_EBADREQ;
        goto out_err;
    }
    version = rpc->getSystemVersion();
out_resp:
    return make_response(GET_SYSTEM_VERSION, rt, version);
out_err:
    ATC_STREAM_LOGE() << "failed to call getSystemVersion" << std::endl;
    goto out_resp;
}

static response_type checkUpdating(ByteArray::ptr ba) {
    uint32_t bodySize;
    int rt = 0;
    bool status = false;

    ATCUpdateServiceRPC::ptr rpc = ATCUpdateServiceRPC::getInstance();
    int args = checkRequest(ba, CHECK_UPDATING, bodySize);
    if (args != 0 || bodySize != 0) {
        ATC_STREAM_LOGE() << "invalid arguement count!" << std::endl;
        rt = -(int)UPD_EBADREQ;
        goto out_err;
    }
    status = rpc->checkUpdating();
out_resp:
    return make_response(CHECK_UPDATING, rt, (uint32_t)status);
out_err:
    ATC_STREAM_LOGE() << "failed to call getSystemVersion" << std::endl;
    goto out_resp;
}

REGISTER_SOCKET_HANDLER(START_SERVICE, startService);
REGISTER_SOCKET_HANDLER(GET_PROGRESS, getProgress);
REGISTER_SOCKET_HANDLER(BEGIN_UPDATE, beginUpdate);
REGISTER_SOCKET_HANDLER(CHECK_UPDATING, checkUpdating);
REGISTER_SOCKET_HANDLER(GET_SYSTEM_VERSION, getSystemVersion);
REGISTER_SOCKET_HANDLER(GET_LAST_STATUS, getLastStatus);

}
}
}