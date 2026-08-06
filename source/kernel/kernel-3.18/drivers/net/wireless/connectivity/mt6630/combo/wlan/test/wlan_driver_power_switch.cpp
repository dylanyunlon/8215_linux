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

class wlanPowerSwitch : public ::testing::Test{
protected:
    virtual void SetUp(){
        printf("inSetUp\n");
        system("insmod /vendor/lib/modules/wlan_gen3.ko");	
    }
    virtual void TearDown(){
        printf("inTearDown\n");
        //system("rmmod wlan_gen3.ko");
    }

};

TEST_F(wlanPowerSwitch, wlanProbe) {

    int fd;
    fd = open("/dev/wmtWifi",O_RDWR);
    EXPECT_EQ(write(fd,"1",1),1);
    close(fd);
}

TEST_F(wlanPowerSwitch, wlanRemove) {

    int fd;
    fd = open("/dev/wmtWifi",O_RDWR);
    EXPECT_EQ(write(fd,"0",1),1);
    close(fd);
}

}  // namespace wlan_driver
}  // namespace android
