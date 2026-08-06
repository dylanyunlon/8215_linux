android-no-sdk {
    target.path = /data/user/qt
    export(target.path)
    INSTALLS += target
} else:android {
    x86 {
        target.path = /libs/x86
    } else: armeabi-v7a {
        target.path = /libs/armeabi-v7a
    } else {
        target.path = /libs/armeabi
    }
    export(target.path)
    INSTALLS += target
} else:unix {
    MY_BUILD_SYSTEM=$$(BUILD_SYSTEM)
    data_files.files += lang/bluetooth_chs.qm lang/bluetooth_cht.qm
    equals(MY_BUILD_SYSTEM, atc) {
        target.path = /${DA_TOP}/lib/app
        data_files.path += /${DA_TOP}/lib/app
    } else {
        target.path = /usr/app
        data_files.path += /usr/app
    }
    isEmpty(target.path) {
        qnx {
            target.path = /tmp/$${TARGET}/bin
        } else {
            target.path = /opt/$${TARGET}/bin
        }
        export(target.path)
    }
    INSTALLS += target
    INSTALLS += data_files
}

export(INSTALLS)
