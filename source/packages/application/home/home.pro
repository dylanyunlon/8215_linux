TARGET = home
TEMPLATE = lib
DEFINES += WITH_SOAPP
#CONFIG += unversioned_libname
#QMAKE_LFLAGS += -Wl,-soname,libhome.so
QT += qml quick  widgets
DA_TOP = ../../../../source/packages/application
INCLUDE_TOP = ../../../host/arm-buildroot-linux-gnueabi/sysroot/usr/include
LIB_TOP = ../../../host/arm-buildroot-linux-gnueabi/sysroot/usr/lib

#carplay_support=$$(ATC_CARPLAY_SUPPORT)
#equals(carplay_support, "yes") {
#    DEFINES += CARPLAY_SUPPORT
#}

#androidauto_support=$$(ATC_ANDROID_AUTO_SUPPORT)
#equals(androidauto_support, "yes") {
#    DEFINES += ANDROIDAUTO_SUPPORT
#}

#avm_app_exists = $$system(test -d ../avm && echo yes)
#equals(avm_app_exists, "yes"){
#      DEFINES += AVM_SUPPORT
#}

SOURCES += main.cpp \
    cappitem.cpp \
    cappitemparser.cpp \
    csubwindowhome.cpp \
    $$DA_TOP/common/qobjlistener.cpp \
    volumeOverlay.cpp \
    $$DA_TOP/common/cotpoweroff.cpp

RESOURCES += \
    res.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
#include(deployment.pri)

LIBS += -lapputils  -lglobalbus -lappobj -lappcommon -luniversal_utils
LIBS += -ldbus-1 -lpthread

HEADERS += \
    cappitem.h \
    cappitemparser.h \
    cglobaldata.h \
    csubwindowhome.h \
    $$DA_TOP/common/qobjlistener.h \
    volumeOverlay.h \
    $$DA_TOP/common/cotpoweroff.h

INCLUDEPATH +=  $$DA_TOP/apputils/ \
				$$DA_TOP/appobj/include/  \
				$$DA_TOP/globalbus/include/  \
				$$DA_TOP/common/ \
				$$DA_TOP/appcommon/include	\
				$$INCLUDE_TOP/dbus-1.0 \
				$$LIB_TOP/dbus-1.0/include/

DESTDIR = lib