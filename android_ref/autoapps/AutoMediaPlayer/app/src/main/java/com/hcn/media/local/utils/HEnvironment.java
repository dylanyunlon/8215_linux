package com.hcn.media.local.utils;

import android.content.Context;
import android.os.Environment;
import android.os.storage.StorageManager;

import com.hcn.media_common.debug.LogUtil;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

/**
 * @author 65821
 * @brief This class provides a number of function that can get storage device paths/state or
 * mount/umount storage devices to the upper application.
 **/
public class HEnvironment {
    private static final String LOG_TAG = "HEnvironment";
    private static final String SD_PATH_MARK = "ext_sd";
    private static final String USB_PATH_MARK = "udisk";

    private StorageManager sm;

    /**
     * Constructor
     *
     * @param ctx Activity Context
     **/
    public HEnvironment(Context ctx) {
        sm = (StorageManager) ctx.getSystemService(Context.STORAGE_SERVICE);
    }

    /**
     * @return return storage paths stored in a string array. eg: "mnt/udisk1", "mnt/ext_sdcard1"
     * ...
     * @brief get list of paths of all storage
     **/
    public String[] getStorageAllPaths() {
        String paths[] = null;

        try {
            paths = (String[]) sm.getClass().getMethod("getVolumePaths").invoke(sm);
        } catch (Exception e) {
            LogUtil.e(LOG_TAG, "Call getMethod of getVolumePaths Error");
            e.printStackTrace();
        }

        return paths;
    }

    /**
     * @return return sd storage paths stored in a string array. eg : "mnt/ext_sdcard1" ...
     * @brief get paths of all SD storage
     **/
    public String[] getSdAllPaths() {
        ArrayList<String> arrayPath = new ArrayList<String>();
        String paths[] = null;
        int i, count;

        try {
            paths = (String[]) sm.getClass().getMethod("getVolumePaths").invoke(sm);
        } catch (Exception e) {
            LogUtil.e(LOG_TAG, "Call getMethod of getVolumePaths Error");
            e.printStackTrace();
        }

        if (null == paths) {
            return null;
        }

        for (i = 0; i < paths.length; i++) {
            if (-1 != paths[i].indexOf(SD_PATH_MARK)) {
                arrayPath.add(paths[i]);
            }
        }

        count = arrayPath.size();
        String[] pathsReturn = new String[count];
        for (i = 0; i < count; i++) {
            pathsReturn[i] = (String) arrayPath.get(i);
        }

        return pathsReturn;
    }

    /**
     * @return return usb storage paths stored in a string array. eg : "mnt/udisk1" ...
     * @brief get paths of all USB storage
     **/
    public String[] getUsbAllPaths() {
        ArrayList<String> arrayPath = new ArrayList<String>();
        String paths[] = null;

        int i, count;

        try {
            paths = (String[]) sm.getClass().getMethod("getVolumePaths").invoke(sm);
        } catch (Exception e) {
            LogUtil.e(LOG_TAG, "Call getMethod of getVolumePaths Error");
            e.printStackTrace();
        }

        if (null == paths) {
            return null;
        }

        for (i = 0; i < paths.length; i++) {
            if (-1 != paths[i].indexOf(USB_PATH_MARK)) {
                arrayPath.add(paths[i]);
            }
        }

        count = arrayPath.size();
        String[] pathsReturn = new String[count];
        for (i = 0; i < count; i++) {
            pathsReturn[i] = (String) arrayPath.get(i);
        }

        return pathsReturn;
    }

    /**
     * @return return mounted storage paths stored in a string array. eg: "mnt/udisk1",
     * "mnt/ext_sdcard1" ...
     * @brief get paths of all mounted storage
     **/
    public String[] getStorageMountedPaths() {
        ArrayList<String> arrayPath = new ArrayList<String>();
        String paths[] = null;
        int i, count;

        paths = getStorageAllPaths();

        if (null == paths) {
            return null;
        }

        for (i = 0; i < paths.length; i++) {
            String strState = getStorageState(paths[i]);
            if (strState != null && strState.equals(Environment.MEDIA_MOUNTED)) {
                arrayPath.add(paths[i]);
            }
        }

        count = arrayPath.size();
        String[] pathsReturn = new String[count];
        for (i = 0; i < count; i++) {
            pathsReturn[i] = (String) arrayPath.get(i);
        }

        return pathsReturn;
    }

    /**
     * @return return mounted sd storage paths stored in a string array. eg: "mnt/ext_sdcard1" ...
     * @brief get paths of all mounted SD storage
     **/
    public String[] getSdMountedPaths() {
        ArrayList<String> arrayPath = new ArrayList<String>();
        String paths[] = null;
        int i, count;

        paths = getSdAllPaths();

        if (null == paths) {
            return null;
        }

        for (i = 0; i < paths.length; i++) {
            String strState = getStorageState(paths[i]);
            if (strState != null && strState.equals(Environment.MEDIA_MOUNTED)) {
                arrayPath.add(paths[i]);
            }
        }

        count = arrayPath.size();
        String[] pathsReturn = new String[count];
        for (i = 0; i < count; i++) {
            pathsReturn[i] = (String) arrayPath.get(i);
        }

        return pathsReturn;
    }

    /**
     * @return return mounted usb storage paths stored in a string array. eg : "mnt/udisk1" ...
     * @brief get paths of all mounted USB storage
     **/
    public String[] getUsbMountedPaths() {
        ArrayList<String> arrayPath = new ArrayList<String>();
        String paths[] = null;
        int i, count;

        paths = getUsbAllPaths();

        if (null == paths) {
            return null;
        }

        for (i = 0; i < paths.length; i++) {
            String strState = getStorageState(paths[i]);
            if (strState != null && strState.equals(Environment.MEDIA_MOUNTED)) {
                arrayPath.add(paths[i]);
            }
        }

        count = arrayPath.size();
        String[] pathsReturn = new String[count];
        for (i = 0; i < count; i++) {
            pathsReturn[i] = (String) arrayPath.get(i);
        }

        return pathsReturn;
    }

    /**
     * @param arrayList store paths of all mounted storage
     * @brief get paths of all mounted storage
     **/
    public void getStorageMountedPaths(ArrayList<String> arrayList) {
        String paths[] = null;
        int i;

        arrayList.clear();
        paths = getStorageAllPaths();

        if (null != paths) {
            for (i = 0; i < paths.length; i++) {
                String strState = getStorageState(paths[i]);
                if (strState != null && strState.equals(Environment.MEDIA_MOUNTED)) {
                    arrayList.add(paths[i]);
                }
            }
        }
    }

    /**
     * @param mountPoint
     * @return return string state. eg: "mounted" ...
     * @brief get mounted/umounted state of storage device
     **/
    public String getStorageState(String mountPoint) {
        String strState = null;

        try {
            strState = (String) sm.getClass()
                    .getMethod("getVolumeState", String.class)
                    .invoke(sm, mountPoint);
        } catch (Exception e) {
            LogUtil.e(LOG_TAG, "Call getMethod of getVolumeState Error");
            e.printStackTrace();
        }

        return strState;
    }

    public String reportStorageState(String mountPoint) {
        String strState = null;
        File FILE_DIR;

        LogUtil.i(LOG_TAG, "reportStorageState - mountPoint = " + mountPoint);
        if ("/mnt/udisk2".equals(mountPoint)) {
            FILE_DIR = new File("/sys/udisk2detect/usb_state");
        } else if ("/mnt/udisk1".equals(mountPoint)) {
            FILE_DIR = new File("/sys/udisk1detect/usb_state");
        } else {
            return ("0");
        }

        if (FILE_DIR.exists()) {
            try {
                BufferedReader reader = new BufferedReader(new FileReader(FILE_DIR));
                strState = reader.readLine();
                reader.close();
            } catch (Exception ignored) {
            }

            if (strState != null) {
                return strState;
            }
        }

        return "0";
    }
}
