package com.autochips.bluetooth.utils;

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Configuration;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import com.autochips.bluetooth.R;
import com.autochips.bluetooth.skin.SkinUtils;
import com.hcn.auto.app.Wallpaper;
import com.hcn.config.HSettings;

import java.io.File;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * 壁纸数据管理
 *
 * @author : zj970
 * @date : 2024/12/16
 */
public class WallpaperUtil {
    private static final String TAG = WallpaperUtil.class.getSimpleName();

    /**
     * 记录 Settings 配置的黑夜壁纸，需要初始化
     */
    private String mNightWallpaperPath = "";

    /**
     * 记录 Settings 配置的白天壁纸，需要初始化
     */
    private String mDayWallpaperPath = "";

    /**
     * 是否第一次过滤数据
     */
    private boolean isFirstFilter = true;

    /**
     * 白天壁纸配置
     */
    private final List<Wallpaper.Info> mDayList = new ArrayList<>();

    /**
     * 黑夜壁纸配置
     */
    private final List<Wallpaper.Info> mNightList = new ArrayList<>();

    /**
     * 唯一实例
     * <p> 页面数据只需要一个实例对象；
     */
    private static WallpaperUtil sInstance = null;

    /**
     * 上下文引用
     * <p> 安全第一，避免不必要的强引用导致内存泄露；
     */
    protected final Reference<Context> mContextRef;

    public static WallpaperUtil getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new WallpaperUtil(new WeakReference<>(context));
        }
        return sInstance;
    }

    private WallpaperUtil() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    private WallpaperUtil(Reference<Context> mContextRef) {
        this.mContextRef = mContextRef;
    }


    /**
     * 初始化壁纸数据
     */
    public void initWallpaperData() {
        Context context = mContextRef.get();
        if (context == null) {
            return;
        }
        mDayWallpaperPath = getGlobalSettings(context.getContentResolver(), HSettings.GlobalKey.BLUETOOTH_WALLPAPER_PATH);
        mNightWallpaperPath = getGlobalSettings(context.getContentResolver(), HSettings.GlobalKey.BLUETOOTH_WALLPAPER_NIGHT_PATH);
        Log.v(TAG, "initWallpaperData: mDayWallpaperPath = "
                + mDayWallpaperPath + " mNightWallpaperPath = " + mNightWallpaperPath);
    }

    /**
     * 是否支持动态切换壁纸
     */
    public boolean supportWallpaperCustomized() {
        return SkinUtils.getInteger(R.integer.support_wallpaper_customized) == 1;
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

    /**
     * 根据配置获取当前需要显示的壁纸
     *
     * @param configuration 配置
     * @return 壁纸绝对路径
     */
    public String getShowWallpaperPath(Configuration configuration) {
        if (isNight(configuration)) {
            if (TextUtils.isEmpty(mNightWallpaperPath)) {
                // 如果为空 读取默认浅色的壁纸配置，兼容旧版本
                return mDayWallpaperPath;
            }
            return mNightWallpaperPath;
        } else {
            return mDayWallpaperPath;
        }
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
                    context.getContentResolver(), HSettings.GlobalKey.BLUETOOTH_WALLPAPER_NIGHT_PATH, mNightWallpaperPath);
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
                    context.getContentResolver(), HSettings.GlobalKey.BLUETOOTH_WALLPAPER_PATH, mDayWallpaperPath);
        }
    }

    /**
     * 获取系统全局配置 key 默认返回 ""
     * @param resolver 上下文解析
     * @param key 系统配置 key
     * @return  结果
     */
    private String getGlobalSettings(ContentResolver resolver, String key) {
        if (resolver == null) {
            return "";
        }
        String result = Settings.Global.getString(resolver, key);
        return Objects.isNull(result) ? "" : result;
    }

    /**
     * 保存壁纸数据
     * <p/>当选中壁纸切换时操作
     *
     * @param path 绝对路径
     */
    public void saveWallpaperData(String path) {
        if (new File(path).getName().startsWith("wallpaper_night_")) {
            setNightWallpaperPath(path);
        } else {
            setDayWallpaperPath(path);
        }
    }

    /**
     * 获取过滤后的壁纸数据
     *
     * @return 结果
     */
    public List<Wallpaper.Info> getFilterWallpapers() {
        List<Wallpaper.Info> infos = Wallpaper.instance().getInfo();
        if (Objects.isNull(infos) || infos.isEmpty()) {
            return infos;
        }

        Context context = mContextRef.get();
        if (Objects.isNull(context)) {
            return infos;
        }

        boolean isNight = isNight(context.getResources().getConfiguration());
        if (isFirstFilter) {
            mNightList.clear();
            mDayList.clear();
            for (Wallpaper.Info info : infos) {
                if (info.thumbnailPath.contains("thumbnail_night_")) {
                    mNightList.add(info);
                } else {
                    mDayList.add(info);
                }
            }

            isFirstFilter = false;
            Log.v(TAG, "Wallpaper.Filter: night: " + mNightList.size() + " light: " + mDayList.size());
        }
        if (isNight) {
            return mNightList.isEmpty() ? mDayList : mNightList;
        } else {
            return mDayList;
        }
    }
}
