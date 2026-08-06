/*
 * Copyright (C) 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

namespace android {
namespace wlan_driver {

class wlanInstall : public ::testing::Test{
protected:
    virtual void SetUp(){
        printf("inSetUp\n");
    }
    virtual void TearDown(){
        printf("inTearDown\n");
    }
};

TEST_F(wlanInstall, initWlan) {

    EXPECT_GE(system("insmod /vendor/lib/modules/wlan_gen3.ko"),0);
    EXPECT_EQ(access("/proc/net/wlan",F_OK),0);
}

TEST_F(wlanInstall, exitWlan) {

    EXPECT_GE(system("rmmod wlan_gen3.ko"),0);
    EXPECT_LE(access("/proc/net/wlan",F_OK),-1);
}

}  // namespace wlan_driver
}  // namespace android
