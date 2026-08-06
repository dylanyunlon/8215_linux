/*
* Copyright (c) 2016 AutoChips Inc.
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

//#include "propertyprovide.h"
#include "bluetoothutils.h"

static const string ENTRYPTION_KEY = "persist.atc.bt.mac.entryption";

string BluetoothUtils::StringForLog(const string & address) {
    if (address.length() != 17 || !isMacEntryption()) {
      return address;
    }

    return std::regex_replace(address,
            std::regex("(^[0-9a-fA-F]{2}):([0-9a-fA-F]{2}):(.{8}):([0-9a-fA-F]{2}$)"),
                       "$1:$2:XX:XX:XX:$4");
}

bool BluetoothUtils::isMacEntryption() {
    string propertyValue = "";
    //universal_utils::getPropertyValue(ENTRYPTION_KEY, propertyValue, "false");

    return (propertyValue == "true");
}
