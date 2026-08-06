TEMPLATE = app
TARGET = cluster-app
QT       += core gui widgets
CONFIG   += c++11
TEMPLATE = lib
DEFINES += WITH_SOAPP

vba_support=$$(ATC_SHOW_VBA)
message(ATC_SHOW_VBA env :  $$vba_support)
equals(vba_support, false) {
    message(ATC_SHOW_VBA is false)
    DEFINES += ATC_SHOW_LOGO_BOOT
}

include (src/src.pri)

INCLUDEPATH += \
            ../vba-1.0/include/ \
            ../../../../source/packages/application/appobj/include/ \
            ../../../../source/packages/application/globalbus/include/ \
            ../../../../source/packages/application/common/ \
            ../../../../source/packages/application/apputils \
            ../../../../source/kernel/kernel-3.18/drivers/misc/atc/bootanimation/ \

HEADERS += ../../../../source/packages/application/common/qobjlistener.h

SOURCES += ../../../../source/packages/application/common/qobjlistener.cpp
RESOURCES += \
    res.qrc

LIBS += -lapputils  -lglobalbus -lappobj -lappcommon -lpthread -latcbootanicom -lcluster-service -lsettings_atc

DESTDIR = lib
