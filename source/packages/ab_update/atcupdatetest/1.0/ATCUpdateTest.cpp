#include <unistd.h>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

#include <ATCUpdateClient.hpp>

using namespace atcupdateservice;

#define LAST_BOOT_SLOT     "/data/misc/bootslot"

void onProgress(uint32_t progress) {
    if (progress % 5 == 0) {
        printf("Update Progress : %u\n", progress);
    }
}

void onMessage(UpdateMessageType type, const std::string & msg) {
    if (UpdateMessageType::PAUSE == type) {
        std::cout << "Update Progress Stopped!" << std::endl;
    } else if (UpdateMessageType::RESUME == type) {
        std::cout << "Update Progress Resumed!" << std::endl;
    } else if (UpdateMessageType::ERROR == type) {
        std::cout << "Error Occured! error : "<< msg << std::endl;
    } else if (UpdateMessageType::FINISHED == type) {
        std::cout << "Update Progress Finished" << std::endl;
    } else if (UpdateMessageType::START == type) {
        std::cout << "Update has started!" << std::endl;
    } else if (UpdateMessageType::UMOUNTED == type) {
        std::cout << "Update External Device had umounted!" << std::endl;
    } else if (UpdateMessageType::CHECK == type) {
        std::cout << "Checking Update Package" << std::endl;
    } else {
        std::cout << "Unknown Message" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage :  %s <path>\n", argv[0]);
        exit(-1);
    }

    std::string path = std::string(argv[1]);
    ATCUpdateClient::ptr client = ATCUpdateClient::getInstance();
    /* get current running slot, update will be process on the other slot*/
    int curSlot = client->currentSlot();
    std::string receiveMsg;

    std::cout << "Current Boot Slot : ";
    if (curSlot == 0) {
        std::cout << "a" << std::endl;
    } else if (curSlot == 1) {
        std::cout << "b" << std::endl;
    } else {
        std::cout << "running on a non-ab system" << std::endl;
        return 0;
    }
    /* register callback to receive update progress */
    client -> registerProgressCb(onProgress);
    /* register callback to receive update message */
    client -> registerMessageCb(onMessage);
    /* mark current slot as valid */
    client -> startService(receiveMsg);
    std::cout << "received from startService : " << receiveMsg << std::endl;
    std::cout << "last upgrade status: " << client->getLastStatus() << std::endl;
    std::string version = client->getSystemVersion();
    std::cout << "system version: " << version << std::endl;
    std::cout << "updating status: " << client->checkUpdating() << std::endl;
    receiveMsg.clear();
    /* start update */
    client -> beginUpdate(path, receiveMsg);
    std::cout << "received from beginUpdate : " << receiveMsg << std::endl;
    std::cout << "updating status: " << client->checkUpdating() << std::endl;
    while(1) {
        sleep(1);
    }

    return 0;
}
