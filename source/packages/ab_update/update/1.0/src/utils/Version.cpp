#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cctype>
#include <clocale>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>

#include "utils/macro.hpp"
#include "utils/Version.hpp"
#include "utils/Util.hpp"
#include "utils/File.hpp"
#include "mz.h"
#include "mz_zip.h"
#include "mz_strm.h"
#include "mz_zip_rw.h"


#define     ENABLE_VERSION_CHECK_FILE   "/tmp/enable_version_check"

namespace atcupdateservice {
namespace utils {

std::string readVersion(std::string versionFile) {
    if (versionFile.empty()) {
        versionFile = VERSION_FILE;
    }
    std::string version = "error";

    if (access(versionFile.c_str(), F_OK) != 0) {
        return version;
    }
    std::ifstream fin(versionFile);
    if (fin) fin >> version;

    return version;
}

bool writeVersion(const std::string &versionFile, const std::string &version) {
    int rt = false;
    int fd = open(versionFile.c_str(), O_WRONLY | O_TRUNC | O_CREAT);
    if (fd < 0) {
        return rt;
    }
    if (write(fd, version.c_str(), version.size()) < 0) {
        goto out;
    }
    fsync(fd);
    rt = true;
out:
    close(fd);
    return rt;
}
std::string readDiffVersion(std::string versionFile,  std::string targetFiled) {
    std::ifstream file(versionFile);
    std::string line;

    std::string filedValue;
    while (std::getline(file, line)) {
        if (line.find(targetFiled) != std::string::npos) {
            std::size_t colonpos = line.find(":");
            if (colonpos != std::string::npos) {
                filedValue = line.substr(colonpos + 1);
            }
            break;
        }
    }
    ATC_STREAM_LOGI() <<"version file: " << versionFile <<", targetFiled: " << targetFiled << " , readDiffVersion filedValue:"<< filedValue << std::endl;
    file.close();
    return filedValue;
}

static uint32_t versionElementLength[] = {4, 2, 2, 2, 2, 2};
#define VERSION_LENGTH      (14)
#define VERSION_ELEMENT_COUNT   (sizeof(versionElementLength) / sizeof(uint32_t))

bool checkVersion(const std::string &version) {
    if (version.find_first_not_of("0123456789") == std::string::npos &&
        version.size() == VERSION_LENGTH) {
        return true;
    }
    return false;
}

bool checkdiffPackageVersion(const std::string &versionFile) {
    std::string curVersion = readVersion();
    if (access(versionFile.c_str(), F_OK) != 0) {
         ATC_STREAM_LOGE() << "version file :" << versionFile << "is not exist" << std::endl;
        return false;
    }
    std::string packageVersion = readDiffVersion(versionFile, VERSION_PRE);
    if (packageVersion.empty()) {
        ATC_STREAM_LOGD() << "not diff package, need to check ota version" << std::endl;
        std::string envStr = readVersion(ENABLE_VERSION_CHECK_FILE);
        if (envStr.empty() == true || envStr == std::string("error")) {
            envStr = "no";
        }
        ATC_STREAM_LOGD() << "version check str:" << envStr << std::endl;

        std::transform(envStr.begin(), envStr.end(), envStr.begin(), [](unsigned char c){
            return std::tolower(c);
        });

        if (envStr == "no") {
            ATC_STREAM_LOGD() << "version check disable!" << std::endl;
            return true;
        }else if (envStr == "yes" || envStr == "y" || envStr == "1" || envStr == "true"){
            ATC_STREAM_LOGD() << "version check enable!" << std::endl;
        }else {
            ATC_STREAM_LOGE() << "readversion error!" << std::endl;
        }
        if (checkVersion(packageVersion) == false)
            return false;
        if (checkVersion(curVersion) == false)
            return true;
        if (packageVersion == curVersion)
            return true;
        for (int32_t pos = 0, i = 0; i < (int32_t)VERSION_ELEMENT_COUNT; ++i, pos += versionElementLength[i]) {
            int packEle = atoi(packageVersion.substr(pos, versionElementLength[i]).c_str());
            int curEle = atoi(curVersion.substr(pos, versionElementLength[i]).c_str());
            if (packEle > curEle) {
                return true;
            } else if (packEle < curEle) {
                return false;
            }
        }
    } else {
        if (checkVersion(packageVersion) == false)
            return false;
        if (checkVersion(curVersion) == false)
            return false;
        if (packageVersion != curVersion) {
            ATC_STREAM_LOGI() << "packageVersion:" << packageVersion <<"is not equal curVersion:"<< curVersion << std
::endl;
            return false;
        }
            return true;
    }


    return true;
}


}
}
