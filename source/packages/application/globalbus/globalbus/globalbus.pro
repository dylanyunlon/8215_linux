
TARGET = globalbus
TEMPLATE = lib

DA_TOP = ../../../../source/packages/application
SOURCES += ctask.cpp globalbus.cpp cbusobj.cpp ccmdtask.cpp

HEADERS  += cbusobj.h

INCLUDEPATH += $$DA_TOP/globalbus/include/ $$DA_TOP/apputils/ $$DA_TOP/appobj/include/

LIBS += -lapputils -luniversal_utils

DESTDIR=lib

#unix {
#	CONFIG += unversioned_libname
#}
