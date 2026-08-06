TARGET = appmanager
TEMPLATE = app
DA_TOP = ../../../../source/packages/application
KERNEL_SRC = ../../../../source/kernel/kernel-3.18
INCLUDE_TOP = ../../../host/arm-buildroot-linux-gnueabi/sysroot/usr/include
LIB_TOP = ../../../host/arm-buildroot-linux-gnueabi/sysroot/usr/lib

#MY_WM_SYSTEM=$$(ATC_YOCTO_WM)
#equals(MY_WM_SYSTEM, wayland) {
#	data_files.files += waylandappobj.xml
#} else {
	data_files.files += appobj.xml
	DEFINES += WITH_SOAPP
#}

#bt_support=$$(CONFIG_CONNECTIVITY_BT_SUPPORT)
#equals(bt_support, "1") {
#    DEFINES += ATC_BT_SUPPORT
#}

#carplay_support=$$(ATC_CARPLAY_SUPPORT)
#equals(carplay_support, "yes") {
#    DEFINES += CARPLAY_SUPPORT
#}

#androidauto_support=$$(ATC_ANDROID_AUTO_SUPPORT)
#equals(androidauto_support, "yes") {
#    DEFINES += ANDROIDAUTO_SUPPORT
#}

#avm_app_exists = $$system(test -d ../../avm && echo yes)
#equals(avm_app_exists, "yes"){
#      DEFINES += AVM_SUPPORT
#}

#MY_VIDEOBOOTANI_SUPPORT=$$(VIDEOBOOTANI_SUPPORT)
#equals(MY_VIDEOBOOTANI_SUPPORT, 1) {
#	DEFINES += VIDEOBOOTANI_SUPPORT_EN
#}

QMAKE_CXXFLAGS += -std=c++11

SOURCES += cappmanager.cpp clistener.cpp cmanager.cpp cobjfactory.cpp \
			csocketlistener.cpp ctaskmanager.cpp main.cpp cavmutemanager.cpp \
			processrecorder.cpp memorywatcher.cpp apprecord.cpp \
			audiofocusrequestdata.cpp


HEADERS += cappmanager.h \
           cavmutemanager.h \
           ../../common/bootproflog.h \


INCLUDEPATH += $$DA_TOP/apputils/ \
				$$DA_TOP/appobj/include/ \
 				$$DA_TOP/globalbus/include/ \
                $$DA_TOP/common/ \
				$$INCLUDE_TOP/ \
				$$INCLUDE_TOP/dbus-1.0 \
				$$LIB_TOP/dbus-1.0/include/ \
				$$KERNEL_SRC/drivers/misc/atc/inc


LIBS += -lpthread -ldl -lglobalbus -ltinyxml2 -lapputils \
		-ldbus-1 -lstdc++ -luniversal_utils

