package com.autochips.bluetooth.utils;

import android.content.Context;
import android.provider.Settings;

import androidx.annotation.NonNull;

import com.autochips.bluetooth.R;
import com.autochips.bluetooth.skin.SkinUtils;
import com.hcn.auto.utils.HSPUtils;

import java.io.File;
import java.util.Objects;

/**
 * @description:
 * @author: guohonglan
 * @date: 2024/1/17 15:45
 */
public class Utility{
    
    /*** @Des: 是否支持动态切换壁纸*/
    public static boolean supportWallpaperCustomized(){
        return SkinUtils.getInteger(R.integer.support_wallpaper_customized) == 1;
    }

    /**
     *  获取系统设置
     * @param context 应用上下文
     * @param key 配置key
     * @return String类型结果，默认值为""
     */
    public static String getGlobalSettings(@NonNull Context context, String key) {
        String result = Settings.Global.getString(context.getContentResolver(), key);
        return Objects.isNull(result) ? "" : result;
    }

    /**
     *  配置系统设置
     * @param context 应用上下文
     * @param key 配置key
     * @param value 配置值
     */
    public static void putGlobalSettings(@NonNull Context context, String key, String value) {
        Settings.Global.putString(context.getContentResolver(), key, value);
    }

    public static boolean isValidPath(String path) {
        File file = new File(path);
        return file.isFile() && file.exists();
    }
}
