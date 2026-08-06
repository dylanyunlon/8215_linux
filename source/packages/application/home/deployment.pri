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
    MY_IMAGE_TYPE=$$(ATC_IMAGE_TYPE)
    
    equals(MY_IMAGE_TYPE, nand) {
        data_files.files += nand/appListItem.xml
    } else {
    	  data_files.files += appListItem.xml
    }
    
    data_files.files += linux_mm_power_off_test.sh
    
    equals(MY_BUILD_SYSTEM, atc) {
        target.path = /${DA_TOP}/lib/app
        data_files.path += /${DA_TOP}/lib/app
    } else {
	    target.path = /app
	    data_files.path += /app
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
