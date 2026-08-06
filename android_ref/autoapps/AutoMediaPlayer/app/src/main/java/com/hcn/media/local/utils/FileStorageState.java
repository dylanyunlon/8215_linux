package com.hcn.media.local.utils;

import android.content.Context;
import android.os.Environment;

import com.hcn.media_base.constant.IConstant;
import com.hcn.media_common.debug.LogUtil;

import java.io.File;

/**
 * 承储状态工具
 *
 * @author 65821
 */
public class FileStorageState {

    private static final String Tag = "FileStorageStateListener";
    private HEnvironment mEnvironment;

    public FileStorageState(Context context) {
        mEnvironment = new HEnvironment(context);
    }

    public File[] getUSBMountedPoints() {
        File file = new File(IConstant.PATH_USB);
        File[] result = null;

        if (file.exists() && file.canRead() && file.isDirectory()) {
            result = file.listFiles();
        } else {
            LogUtil.e(Tag, " -- fail to access /storage");
        }

        return result;
    }

    public boolean getSDState() {
        File[] result = getUSBMountedPoints();
        if (null == result) {
            return false;
        }
        for (File file : result) {
            if (file.getPath().contains("/storage/ext_sdcard1")) {
                return true;
            }
        }

        return false;
    }

    public boolean getSD2State() {
        return isMounted(IConstant.PATH_SD2);
    }

    public File[] getUSBMountedList() {
        File file = new File(IConstant.PATH_USB);
        File[] result = null;
        if (file.exists() && file.canRead() && file.isDirectory()
                && !file.getPath().contains("/storage/sdcard0")
                && !file.getPath().contains("/storage/internal_sdcard")
                && !file.getPath().contains("/storage/self")
                && !file.getPath().contains("/storage/emulated")
                && !file.getPath().contains("/storage/ext_sdcard1")) {
            result = file.listFiles();
        } else {
            LogUtil.e(Tag, " -- fail to access /storage/usb-otg/");
        }

        return result;
    }

    public boolean getUSBState() {
        File[] result = getUSBMountedPoints();
        if (null == result) {
            return false;
        }

        for (File file : result) {
            String filePath = file.getPath();

            if (filePath.contains(IConstant.PATH_USB_PREFIX)) {
                if (isMounted(filePath)) {
                    return true;
                }
            }
        }

        return false;
    }

    public boolean queryMediaState(String strPath) {
        String path = Environment.getExternalStorageDirectory().getAbsolutePath();

        if (strPath.equalsIgnoreCase(path)) {
            return true;
        } else if (strPath.equalsIgnoreCase(IConstant.PATH_SD)) {
            return getSDState();
        } else if (strPath.equalsIgnoreCase(IConstant.PATH_USB)) {
            return getUSBState();
        } else {
            return false;
        }
    }

    public boolean isMounted(String path) {
        return Environment.MEDIA_MOUNTED.equals(mEnvironment.getStorageState(path));
    }
}
