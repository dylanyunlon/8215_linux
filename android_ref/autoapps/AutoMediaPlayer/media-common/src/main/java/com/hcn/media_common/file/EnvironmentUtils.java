package com.hcn.media_common.file;

import android.content.Context;
import android.os.Environment;
import android.os.storage.StorageManager;

import androidx.annotation.NonNull;

import com.hcn.common.utils.HUtilsEx;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_common.debug.LogUtil;

import java.util.ArrayList;

/**
 * Environment 工具类
 * @author 65821
 */
public class EnvironmentUtils {
    private static final String TAG = EnvironmentUtils.class.getSimpleName();

    /** 唯一实例对象 **/
    private static EnvironmentUtils sInstance;

    /** 唯一实例对象 **/
    public static EnvironmentUtils instance() {
        if (sInstance == null) {
            sInstance = new EnvironmentUtils(
                    HUtilsEx.getApp().getApplicationContext());
        }

        return sInstance;
    }

    /** 系统存储服务对象 **/
    private final StorageManager mStorageManager;

    /**
     * 构造获取存储服务对象
     * @param context 上下文环境对象
     */
    private EnvironmentUtils(@NonNull Context context) {
        mStorageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
    }

    /**
     * 获取所有存储设备路径
     * @return 路径数组
     */
    public String[] getStorageAllPaths() {
        String[] paths = null;

        try {
            paths = (String[]) mStorageManager.getClass()
                    .getMethod("getVolumePaths").invoke(mStorageManager);
        } catch (Exception e) {
            LogUtil.e(TAG, "Call getMethod of getVolumePaths Error!");
            e.printStackTrace();
        }

        return paths;
    }

    /**
     * 获取所有 USB 存储路径
     * <pre>
     *    udisk1、udisk2 ...
     *    udisk1p1、udisk1dev1p1...
     * </pre>
     *
     * @return usb 存储路径
     */
    public String[] getAllUsbPaths() {
        ArrayList<String> arrayPath = new ArrayList<String>();
        String[] paths = null;

        int i, count;

        try {
            paths = (String[]) mStorageManager.getClass()
                    .getMethod("getVolumePaths").invoke(mStorageManager);
        } catch (Exception e) {
            LogUtil.e(TAG, "Call getMethod of getVolumePaths Error!");
            e.printStackTrace();
        }

        if (null == paths) {
            return null;
        }

        for (i = 0; i < paths.length; i++) {
            if (paths[i].contains(IConstant.USB_PATH_MARK)) {
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
     * 获取所有已经挂载的存储设备路径
     * <pre>
     *    udisk1、udisk2 ...
     *    udisk1p1、udisk1dev1p1...
     *    ext_sdcard1、sdcard2...
     *    sdcard0（/emulated/0）
     * </pre>
     *
     * @return 存储设备路径
     */
    public String[] getAllMountedPaths() {
        ArrayList<String> arrayPath = new ArrayList<>();
        String[] paths;
        int i, count;

        paths = getStorageAllPaths();

        if (null == paths) {
            return null;
        }

        for (i = 0; i < paths.length; i++) {
            String strState = getStorageState(paths[i]);
            if (strState != null && strState.equals(android.os.Environment.MEDIA_MOUNTED)) {
                arrayPath.add(paths[i]);
            }
        }

        count = arrayPath.size();
        String[] pathsReturn = new String[count];
        for (i = count-1; i >= 0; i--) {
            pathsReturn[count - 1 - i] = arrayPath.get(i);
        }

        return pathsReturn;
    }

    /**
     * 获取已经挂载的 USB 路径
     * <pre>
     *    udisk1、udisk2 ...
     *    udisk1p1、udisk1dev1p1...
     * </pre>
     *
     * @return USB 存储设备路径
     */
    public String[] getUsbMountedPaths() {
        ArrayList<String> arrayPath = new ArrayList<>();
        String[] paths = null;
        int i, count;

        paths = getAllUsbPaths();

        if (null == paths) {
            return null;
        }

        for (i = 0; i < paths.length; i++) {
            String strState = getStorageState(paths[i]);
            if (strState != null
                    && strState.equals(android.os.Environment.MEDIA_MOUNTED)) {
                arrayPath.add(paths[i]);
            }
        }

        count = arrayPath.size();
        String[] pathsReturn = new String[count];
        for (i = 0; i < count; i++) {
            pathsReturn[i] = arrayPath.get(i);
        }

        return pathsReturn;
    }

    /**
     * 获取指定路径的存储设备状态
     * @param mountPoint 存储设备路径
     * @return {@link Environment#MEDIA_MOUNTED ...}
     */
    public String getStorageState(String mountPoint) {
        String strState = null;

        try {
            strState = (String) mStorageManager.getClass()
                    .getMethod("getVolumeState", String.class)
                    .invoke(mStorageManager, mountPoint);
        } catch (Exception e) {
            LogUtil.e(TAG, "Call getMethod of getVolumeState Error!");
            e.printStackTrace();
        }

        return strState;
    }
}
