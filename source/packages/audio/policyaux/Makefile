#
# Copyright (c) 2016 AutoChips Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.




TARGET := arm

LOCAL_SRC_FILES := ./src/policyEvent.c
LOCAL_SRC_FILES += ./src/audio_bt_sco_fifo_client.c
LOCAL_SRC_FILES += ./src/fifo_client.c

LOCAL_INCLUDES := $(DA_SYSROOT)/usr/include/	\
                  $(DA_SYSROOT)/usr/include/dbus-1.0 \
                  $(DA_SYSROOT)/usr/lib/dbus-1.0/include \

LOCAL_INCLUDES += ./include

LOCAL_SHARE_LIBRARIES := -ldbus-1 -ldl -lrt

#LOCAL_INSTALL_FILES := ./include/policyEvent.h

LOCAL_MODULE := libpolicyaux.so

include $(BUILD_SHARED_LIBRARY)