
TARGET = appobj
TEMPLATE = lib

DA_TOP = ../../../../source
SOURCES += cappobject.cpp demoappobjinterface.cpp


HEADERS  += cappobject.h demoappobjinterface.h
#HEADERS  += ../../../connectivity/universal_utils/include/csync.h

INCLUDEPATH += $$DA_TOP/packages/application/appobj/include/ $$DA_TOP/packages/application/apputils/

DESTDIR = lib

LIBS +=  -lpthread  -ldl -lapputils -luniversal_utils

#unix {
#	CONFIG += unversioned_libname
#	QMAKE_LFLAGS += -Wl,-soname,libappobj.so
#}
