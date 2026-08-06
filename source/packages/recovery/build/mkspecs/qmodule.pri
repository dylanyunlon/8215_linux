CONFIG +=  cross_compile compile_examples silent qpa largefile neon pcre
QT_BUILD_PARTS +=  libs tools
QT_NO_DEFINES =  ALSA CUPS DBUS EGL_X11 FONTCONFIG HARFBUZZ ICONV IMAGEFORMAT_JPEG LIBPROXY MITSHM NIS OPENVG PULSEAUDIO SESSIONMANAGER SHAPE STYLE_GTK TABLET TSLIB XCURSOR XFIXES XINERAMA XINPUT XKB XRANDR XRENDER XSYNC XVIDEO ZLIB
QT_QCONFIG_PATH = 
host_build {
    QT_CPU_FEATURES.arm =  neon
} else {
    QT_CPU_FEATURES.arm =  neon
}
QT_COORD_TYPE = double
QT_LFLAGS_ODBC   = -lodbc
OPENSSL_LIBS = -lssl -lcrypto
OE_QMAKE_AR = arm-poky-linux-gnueabi-ar
OE_QMAKE_CC = arm-poky-linux-gnueabi-gcc  -march=armv7-a -mthumb-interwork -mfloat-abi=softfp -mfpu=neon --sysroot=/proj/vend_mhf_aesdsrv49/Perforce/ws_vend_mhf_aesdsrv49_mhfspdlx02/AC8317/DEV_BR/linux/dizzy/build/tmp/sysroots/ac8317
OE_QMAKE_CFLAGS =  -O2 -pipe -g -feliminate-unused-debug-types
OE_QMAKE_COMPILER = arm-poky-linux-gnueabi-gcc  -march=armv7-a -mthumb-interwork -mfloat-abi=softfp -mfpu=neon --sysroot=/proj/vend_mhf_aesdsrv49/Perforce/ws_vend_mhf_aesdsrv49_mhfspdlx02/AC8317/DEV_BR/linux/dizzy/build/tmp/sysroots/ac8317
OE_QMAKE_CXX = arm-poky-linux-gnueabi-g++  -march=armv7-a -mthumb-interwork -mfloat-abi=softfp -mfpu=neon --sysroot=/proj/vend_mhf_aesdsrv49/Perforce/ws_vend_mhf_aesdsrv49_mhfspdlx02/AC8317/DEV_BR/linux/dizzy/build/tmp/sysroots/ac8317
OE_QMAKE_CXXFLAGS =  -O2 -pipe -g -feliminate-unused-debug-types -fvisibility-inlines-hidden
OE_QMAKE_LDFLAGS = -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed
OE_QMAKE_LINK = arm-poky-linux-gnueabi-g++  -march=armv7-a -mthumb-interwork -mfloat-abi=softfp -mfpu=neon --sysroot=/proj/vend_mhf_aesdsrv49/Perforce/ws_vend_mhf_aesdsrv49_mhfspdlx02/AC8317/DEV_BR/linux/dizzy/build/tmp/sysroots/ac8317
OE_QMAKE_STRIP = echo
styles += mac fusion windows
DEFINES += QT_NO_MTDEV
QT_CFLAGS_GLIB = -pthread -I/proj/vend_mhf_aesdsrv49/Perforce/ws_vend_mhf_aesdsrv49_mhfspdlx02/AC8317/DEV_BR/linux/dizzy/build/tmp/sysroots/ac8317/usr/include/glib-2.0 -I/proj/vend_mhf_aesdsrv49/Perforce/ws_vend_mhf_aesdsrv49_mhfspdlx02/AC8317/DEV_BR/linux/dizzy/build/tmp/sysroots/ac8317/usr/lib/glib-2.0/include 
QT_LIBS_GLIB = -lgthread-2.0 -pthread -lglib-2.0 
QMAKE_INCDIR_OPENGL_ES2 = 
QMAKE_LIBDIR_OPENGL_ES2 = 
QMAKE_LIBS_OPENGL_ES2 =  "-lGLESv2"
QMAKE_CFLAGS_OPENGL_ES2 = 
DEFINES += QT_NO_LIBUDEV
DEFINES += QT_NO_TSLIB
DEFINES += QT_NO_LIBINPUT
QMAKE_INCDIR_EGL = 
QMAKE_LIBS_EGL = -lEGL 
QMAKE_CFLAGS_EGL = 
sql-drivers = 
sql-plugins = 
