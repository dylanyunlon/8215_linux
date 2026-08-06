# DVR Application - Refactored (qmake project file)
# Phase 1: Basic UI with CQObjListener integration

QT += core gui qml quick widgets

TARGET = dvrapp
TEMPLATE = lib
DEFINES += WITH_SOAPP
CONFIG += unversioned_libname

# Source files
SOURCES += \
    main.cpp \
    backend/dvrbackend.cpp \
    backend/dvrqobjlistener.cpp \
    backend/cameramanager.cpp \
    backend/previewmanager.cpp \
    backend/recordmanager.cpp \
    backend/playbackmanager.cpp \
    backend/filemanager.cpp \
    backend/settingsmanager.cpp \
    ../../../../source/packages/application/common/qobjlistener.cpp

# Header files
HEADERS += \
    backend/dvrlog.h \
    backend/dvrbackend.h \
    backend/dvrqobjlistener.h \
    backend/cameramanager.h \
    backend/previewmanager.h \
    backend/recordmanager.h \
    backend/playbackmanager.h \
    backend/filemanager.h \
    backend/settingsmanager.h \
    ../../../../source/packages/application/common/qobjlistener.h

# QML resources
RESOURCES += resources.qrc

# Default rules for deployment.
include(deployment.pri)

INCLUDEPATH += ../../../../source/packages/application/apputils/ \
               ../../../../source/packages/application/common/ \
               ../../../../source/packages/application/appobj/include/ \
               ../../../../source/packages/application/globalbus/include/ \
               ../../../../source/packages/graphics/surface/surface/include/ \
               ../../../../source/kernel/kernel-3.18/drivers/ 

LIBS += -luniversal_utils -ltinyxml2 -ldl -lsurface_atc -losal -ldvr -lappobj -lapputils -lappcommon -lglobalbus
