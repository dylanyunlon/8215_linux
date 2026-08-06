TARGET = btphone
TEMPLATE = lib
DEFINES += WITH_SOAPP
CONFIG += unversioned_libname

QT += qml quick widgets

SOURCES += main.cpp \
           ../../../../source/packages/application/common/qobjlistener.cpp \

RESOURCES += ./demoresource.qrc

QML_IMPORT_PATH =

target.path = /app
INSTALLS += target
export(INSTALLS)

HEADERS += \
           ../../../../source/packages/application/common/qobjlistener.h \

INCLUDEPATH += ../../../../source/packages/application/apputils/
INCLUDEPATH += ../../../../source/packages/application/common/
INCLUDEPATH += ../../../../source/packages/application/appobj/include/
INCLUDEPATH += ../../../../source/packages/application/globalbus/include/

MY_BUILD_SYSTEM=$$(BUILD_SYSTEM)
LIBS += -L${DA_TOP}/packages/application/lib -lapputils  -lglobalbus -lappobj