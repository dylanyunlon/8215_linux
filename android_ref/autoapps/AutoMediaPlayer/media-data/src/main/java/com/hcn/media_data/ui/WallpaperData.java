package com.hcn.media_data.ui;

import android.content.Context;
import android.content.res.Configuration;
import android.provider.Settings;

import com.hcn.common.misc.LogUtils;
import com.hcn.config.HSettings;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;

/**
 * 壁纸数据层
 * @author : zj970
 * @date : 2024/12/13
 */
public class WallpaperData {

    /**
     * 上下文引用
     * <p> 安全第一，避免不必要的强引用导致内存泄露；
     */
    protected final Reference<Context> mContextRef;

    /**
     * 记录 Settings 配置的黑夜壁纸，需要初始化
     */
    private String mNightWallpaperPath = "";

    /**
     * 记录 Settings 配置的白天壁纸，需要初始化
     */
    private String mDayWallpaperPath = "";

    public WallpaperData(Context context) {
        mContextRef = new WeakReference<>(context);
    }

    /**
     * 初始化壁纸数据
     */
    public void initWallpaperData() {
        Context context = mContextRef.get();
        if (context == null) {
            return;
        }
        mDayWallpaperPath = Settings.Global.getString(
                context.getContentResolver(), HSettings.GlobalKey.MEDIA_WALLPAPER_PATH);
        mNightWallpaperPath = Settings.Global.getString(
                context.getContentResolver(), HSettings.GlobalKey.MEDIA_WALLPAPER_NIGHT_PATH);
        LogUtils.vTag(WallpaperData.class.getSimpleName(),
                "initWallpaperData: mDayWallpaperPath = "
                        + mDayWallpaperPath + " mNightWallpaperPath =" + mNightWallpaperPath);
    }


    /**
     * 获取黑夜壁纸路径
     *
     * @return 绝对路径
     */
    public String getNightWallpaperPath() {
        if (Objects.isNull(mNightWallpaperPath)) {
            return "";
        }
        return mNightWallpaperPath;
    }

    /**
     * 获取白天壁纸路径
     *
     * @return 绝对路径
     */
    public String getDayWallpaperPath() {
        if (Objects.isNull(mDayWallpaperPath)) {
            return "";
        }
        return mDayWallpaperPath;
    }

    /**
     * 设置有效的黑夜壁纸路径
     *
     * @param nightWallpaperPath 绝对路径
     */
    protected void setNightWallpaperPath(String nightWallpaperPath) {
        this.mNightWallpaperPath = nightWallpaperPath;
        Context context = mContextRef.get();
        if (context != null) {
            Settings.Global.putString(
                    context.getContentResolver(), HSettings.GlobalKey.MEDIA_WALLPAPER_NIGHT_PATH, mNightWallpaperPath);
        }
    }

    /**
     * 设置有效的白天壁纸路径
     *
     * @param dayWallpaperPath 绝对路径
     */
    protected void setDayWallpaperPath(String dayWallpaperPath) {
        this.mDayWallpaperPath = dayWallpaperPath;
        Context context = mContextRef.get();
        if (context != null) {
            Settings.Global.putString(
                    context.getContentResolver(), HSettings.GlobalKey.MEDIA_WALLPAPER_PATH, mDayWallpaperPath);
        }
    }

    /**
     * 判断是否为深色模式
     * <p/> 并且会记录 本次状态
     *
     * @param newConfig 应用上下文环境配置
     * @return true 为深色模式，false 为浅色模式
     */
    public boolean isNight(Configuration newConfig) {
        return (newConfig.uiMode & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES;
    }

}