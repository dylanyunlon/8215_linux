QT += core qml quick

TARGET = appcommon
TEMPLATE = lib

SOURCES += src/imageprovider.cpp \
           src/windowmgr.cpp \
           src/utils.cpp
        
HEADERS  += include/appimageprovider.h \
	    include/appwindowmgr.h \
            include/apputils.h

INCLUDEPATH += include

DESTDIR = lib

#unix {
#    CONFIG += unversioned_libname
#}
