package com.hcn.media_common.file;

import androidx.annotation.NonNull;

import com.hcn.media_base.constant.IConstant;
import com.hcn.media_common.debug.LogUtil;

import java.io.File;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 存储设备辅助工具类
 * @author 65821
 */
public class StorageUtilsEx {
    /**
     * 获取指定字符或者字符串在目标字符串中第 n 次出现的位置
     *
     * @param data 指定字符串
     * @param str 需要定位的特殊字符或者字符串
     * @param num 第 n 次出现
     * @return 第 n 次出现的位置索引
     */
    public static int indexOf(String data, String str, int num) {
        Pattern pattern = Pattern.compile(str);
        Matcher findMatcher = pattern.matcher(data);

        int indexNum = 0;
        while (findMatcher.find()) {
            indexNum++;
            if (indexNum == num) {
                break;
            }
        }

        return findMatcher.start();
    }

    /**
     * 由文件获取其所在的存储设备路径
     * <pre>
     *    /storage/emulated/0
     *    /storage/udisk1, /storage/udisk2...
     *    /storage/ext_sdcard1
     * </pre>
     *
     * @param filePath 文件路径
     * @return 存储设备路径
     */
    public static String storageDevicePath(@NonNull String filePath) {
        // 先检查是否是 USB
        if (filePath.startsWith(IConstant.PATH_USB_PREFIX)) {
            int len = IConstant.PATH_USB_PREFIX.length();
            int index = filePath.indexOf(File.separator, len);
            if (index < 0) {
                return null;
            }

            return filePath.substring(0, index);
        }

        // 再检查内置存储设备
        if (filePath.startsWith(IConstant.PATH_FLASH)) {
            return IConstant.PATH_FLASH;
        }

        // 再检查外置 SD 卡设备
        if (filePath.startsWith(IConstant.PATH_SD)) {
            return IConstant.PATH_SD;
        }

        // 强制匹配一个未知存储设备路径
        int index = indexOf(filePath, File.separator, 3);
        return filePath.substring(0, index);
    }

    /**
     * 判断指定路径是否是 mounted 状态
     * @param path 存储设备路径
     * @return 是 mounted 状态/不是
     */
    public boolean isMounted(String path) {
        String storageState = EnvironmentUtils.instance().getStorageState(path);
        return android.os.Environment.MEDIA_MOUNTED.equals(storageState);
    }
}
