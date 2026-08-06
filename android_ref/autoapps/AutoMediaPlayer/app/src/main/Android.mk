#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(DIRECT_BUILD_APK_FILE), yes)
	LOCAL_MODULE := AutoMediaPlayer
	LOCAL_MODULE_TAGS := optional

	LOCAL_SRC_FILES := ../../release/app-release.apk
	LOCAL_MODULE_CLASS := APPS
	LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)

	LOCAL_CERTIFICATE := platform
	include $(BUILD_PREBUILT)
else
	LOCAL_SRC_FILES := $(call all-java-files-under, java)
	LOCAL_SRC_FILES += $(call all-Iaidl-files-under, aidl)
	LOCAL_AIDL_INCLUDES += $(LOCAL_PATH)/aidl

	LOCAL_PACKAGE_NAME := AutoMediaPlayer
	LOCAL_MODULE_TAGS := optional
	LOCAL_CERTIFICATE := platform
	LOCAL_JAVA_LIBRARIES := carservices

	# jar file
	LOCAL_STATIC_JAVA_LIBRARIES := guava
	LOCAL_STATIC_JAVA_LIBRARIES += theme-utils
	LOCAL_STATIC_JAVA_LIBRARIES += lifecycle-common

	LOCAL_STATIC_JAVA_LIBRARIES += androidx-annotation
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-collection
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-collection-ktx
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-arch-core-common

	# aar file
	LOCAL_STATIC_JAVA_LIBRARIES += vitamio-release
	LOCAL_STATIC_JAVA_LIBRARIES += transformer-release
	LOCAL_STATIC_JAVA_LIBRARIES += mediaservice-api
	LOCAL_STATIC_JAVA_LIBRARIES += common-utils
	LOCAL_STATIC_JAVA_LIBRARIES += lifecycle-service
	LOCAL_STATIC_JAVA_LIBRARIES += lifecycle-process
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-activity
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-core
	LOCAL_STATIC_JAVA_LIBRARIES += lifecycle-viewmodel
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-savedstate
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-viewpager
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-arch-core-runtime
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-fragment
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-startup-runtime
	LOCAL_STATIC_JAVA_LIBRARIES += androidx-tracing
	LOCAL_STATIC_JAVA_LIBRARIES += lifecycle-livedata
	LOCAL_STATIC_JAVA_LIBRARIES += lifecycle-livedata-core

	# aar java+res file
	LOCAL_STATIC_JAVA_AAR_LIBRARIES := lifecycle-runtime-aar

	# aar res file
	LOCAL_USE_AAPT2 := true
	LOCAL_AAPT_FLAGS := \
	   --auto-add-overlay \
	   --extra-packages androidx.lifecycle.runtime

	LOCAL_PRIVATE_PLATFORM_APIS := true
	LOCAL_PROGUARD_ENABLED := disabled
	LOCAL_PROGUARD_FLAG_FILES := proguard.flags

	# 当前目录递归搜索
	$(foreach FILE, $(shell find $(LOCAL_PATH)/../../libs/armeabi-v7a -name *.so), $(eval JNI_LIBS += $(FILE)))

	# 获取搜索文件集合
	LOCAL_PREBUILT_JNI_LIBS := $(subst $(LOCAL_PATH),,$(JNI_LIBS))
	LOCAL_MULTILIB :=32
	include $(BUILD_PACKAGE)

	include $(CLEAR_VARS)
	LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := vitamio-release:../../../release/media-vitamio-release.aar \
											transformer-release:../../../release/transformer-release.aar \
											androidx-core:../../libs/androidx-core-1.7.0.aar \
											lifecycle-process:../../libs/lifecycle-process-2.4.0.aar \
											lifecycle-runtime-aar:../../libs/lifecycle-runtime-2.4.0.aar \
											lifecycle-livedata:../../libs/lifecycle-livedata-2.2.0.aar \
											lifecycle-livedata-core:../../libs/lifecycle-livedata-core-2.3.1.aar \
											androidx-arch-core-common:../../../libs/androidx-arch-core-common-2.1.0.jar \
											androidx-arch-core-runtime:../../../libs/androidx-arch-core-runtime-2.1.0.aar \
											androidx-collection:../../../libs/androidx-collection-1.1.0.jar \
											androidx-collection-ktx:../../../libs/androidx-collection-ktx-1.1.0.jar \
											androidx-startup-runtime:../../../libs/androidx-startup-runtime-1.0.0.aar \
											androidx-tracing:../../../libs/androidx-tracing-1.0.0.aar \
											androidx-viewpager:../../../libs/androidx-viewpager-1.0.0.aar \
											lifecycle-viewmodel:../../libs/lifecycle-viewmodel-2.3.1.aar \
											androidx-savedstate:../../libs/androidx-savedstate-1.1.0.aar \
											androidx-activity:../../libs/androidx-activity-1.2.4.aar \
											androidx-fragment:../../libs/androidx-fragment-1.4.1.aar
	include $(BUILD_MULTI_PREBUILT)
endif
