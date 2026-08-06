#include <iostream>
#include <string>
#include <cstring>

#include "utils/macro.hpp"
#include "utils/Util.hpp"
#include "bootctrl/BootCtrl.hpp"

using namespace atcupdateservice::bootctrl;

static void usage(const char *prog) {
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " status                # show current active slot (_a/_b)\n";
    std::cout << "  " << prog << " set <a|b>             # set active slot to a or b\n";
    std::cout << "  " << prog << " bootable <a|b|0|1>    # query whether slot is bootable\n";
    std::cout << "  " << prog << " mark-successful       # mark current slot as successful\n";
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "status" || cmd == "current" || cmd == "get") {
        int slot = getCurrentSlot();
        if (slot < 0) {
            ATC_STREAM_LOGE() << "failed to get current slot" << std::endl;
            std::cout << "current: unknown" << std::endl;
            return 2;
        }
        std::string suf = getSuffix(slot);
        ATC_STREAM_LOGI() << "current slot: "<< suf  << std::endl;
        std::cout << "current slot: " << suf << std::endl;
        return 0;
    }

    if (cmd == "set") {
        if (argc < 3) {
            usage(argv[0]);
            return 1;
        }
        std::string arg = argv[2];
        int slot = -1;
        if (arg == "a" || arg == "_a" || arg == "A") slot = 0;
        else if (arg == "b" || arg == "_b" || arg == "B") slot = 1;
        else {
            std::cout << "invalid slot: " << arg << std::endl;
            return 1;
        }
        //disable write protect
        if (!atcupdateservice::utils::FSUtil::disableWriteProtect()) {
            ATC_STREAM_LOGE() << "disableWriteProtect failed" << std::endl;
            std::cout << "disable wp fail" << std::endl;
            return 4;
        }
        int ret = setActiveSlot(slot);
        if (ret == 0) {
            ATC_STREAM_LOGI() << "set active slot to " << slot << " success" << std::endl;
            std::cout << "ok" << std::endl;
            return 0;
        } else {
            ATC_STREAM_LOGE() << "setActiveSlot failed, ret=" << ret << std::endl;
            std::cout << "fail" << std::endl;
            return 3;
        }
        //restore write protect
        if (!atcupdateservice::utils::FSUtil::enableWriteProtect()) {
            ATC_STREAM_LOGE() << "enableWriteProtect failed" << std::endl;
            std::cout << "enable wp fail" << std::endl;
        }
    }

    if (cmd == "bootable") {
        if (argc < 3) {
            usage(argv[0]);
            return 1;
        }
        std::string arg = argv[2];
        int slot = -1;
        if (arg == "a" || arg == "_a" || arg == "A") slot = 0;
        else if (arg == "b" || arg == "_b" || arg == "B") slot = 1;
        else {
            // try numeric
            if (std::isdigit(arg[0])) slot = std::stoi(arg);
        }
        if (slot < 0 || slot > 1) {
            std::cout << "invalid slot" << std::endl;
            return 1;
        }
        int r = getSlotBootable(slot);
        if (r < 0) {
            ATC_STREAM_LOGE() << "getSlotBootable failed, ret=" << r << std::endl;
            std::cout << "error" << std::endl;
            return 4;
        }
        ATC_STREAM_LOGI() << "slot " << slot << " bootable=" << r << std::endl;
        std::cout << (r ? "1" : "0") << std::endl;
        return 0;
    }

    if (cmd == "mark-successful") {
        //disable write protect
        if (!atcupdateservice::utils::FSUtil::disableWriteProtect()) {
            ATC_STREAM_LOGE() << "disableWriteProtect failed" << std::endl;
            std::cout << "disable wp fail" << std::endl;
            return 6;
        }
        int ret = markBootSuccessful();
        if (ret == 0) {
            ATC_STREAM_LOGI() << "markBootSuccessful ok" << std::endl;
            std::cout << "ok" << std::endl;
            return 0;
        } else {
            ATC_STREAM_LOGE() << "markBootSuccessful failed, ret=" << ret << std::endl;
            std::cout << "fail" << std::endl;
            return 5;
        }
        //restore write protect
        if (!atcupdateservice::utils::FSUtil::enableWriteProtect()) {
            ATC_STREAM_LOGE() << "enableWriteProtect failed" << std::endl;
            std::cout << "enable wp fail" << std::endl;
        }
    }

    usage(argv[0]);
    return 1;
}
