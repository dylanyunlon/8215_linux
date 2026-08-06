#pragma once

#include <string>

namespace atcupdateservice {
namespace utils {

std::string readVersion(std::string versionFile = "");
std::string readDiffVersion(std::string versionFile,  std::string targetFiled);
bool writeVersion(const std::string &versionFile, const std::string &version);
bool checkVersion(const std::string &packageVersion);
bool checkdiffPackageVersion(const std::string &packageVersion);

}
}