/*
copyright (c) 2020 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ATCUpdateClient.hpp"
#include "ATCUpdate.hpp"

#include "private/Tag.hpp"

namespace atcupdateservice {

namespace {
    static ATCUpdate::ptr g_realClient;
}

ATCUpdateClient::ptr ATCUpdateClient::getInstance() {
    g_realClient = ATCUpdate::getInstance();
    return ATCUpdateClient::ptr(new ATCUpdateClient());
}

bool ATCUpdateClient::startService(std::string &msg) {
    return g_realClient -> startService(msg);
}

bool ATCUpdateClient::beginUpdate(const std::string& pathname, std::string &msg,
                                 const std::string &method) {
    return g_realClient -> beginUpdate(pathname, msg, method);
}

bool ATCUpdateClient::cancelUpdate(std::string &msg) {
    return g_realClient -> cancelUpdate(msg);
}

bool ATCUpdateClient::pauseUpdate(std::string &msg) {
    return g_realClient -> pauseUpdate(msg);
}

bool ATCUpdateClient::resumeUpdate(std::string &msg) {
    return g_realClient -> resumeUpdate(msg);
}

bool ATCUpdateClient::registerProgressCb(ProgressCb cb) {
    return g_realClient -> registerProgressCb(cb);
}

bool ATCUpdateClient::registerMessageCb(MessageCb cb) {
    return g_realClient -> registerMessageCb(cb);
}

bool ATCUpdateClient::checkPackage(std::string path) {
    return g_realClient -> checkPackage(path);
}

int ATCUpdateClient::currentSlot() {
    return g_realClient -> currentSlot();
}

int ATCUpdateClient::startService() {
    return g_realClient->startService();
}

int ATCUpdateClient::beginUpdate(const std::string& pathname, const std::string &method) {
    return g_realClient->beginUpdate(pathname, method);
}

int ATCUpdateClient::cancelUpdate() {
    return g_realClient->cancelUpdate();
}

int ATCUpdateClient::pauseUpdate() {
    return g_realClient->pauseUpdate();
}

int ATCUpdateClient::resumeUpdate() {
    return g_realClient->resumeUpdate();
}

int ATCUpdateClient::getProgress(unsigned &progress) {
    return g_realClient->getProgress(progress);
}

LastUpdateStatus ATCUpdateClient::getLastStatus() {
    return g_realClient->getLastStatus();
}

std::string ATCUpdateClient::getSystemVersion() {
    return g_realClient->getSystemVersion();
}

bool ATCUpdateClient::checkUpdating() {
    return g_realClient->checkUpdating();
}

}