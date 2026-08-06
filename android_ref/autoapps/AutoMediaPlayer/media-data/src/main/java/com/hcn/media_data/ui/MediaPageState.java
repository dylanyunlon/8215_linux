package com.hcn.media_data.ui;

import android.content.Context;

import androidx.annotation.NonNull;

import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media_data.ui.base.PageDataKV;

import java.io.File;

/**
 * 用来存储媒体页面状态的数据类
 * <pre>
 *    存储音视频页面状态，分永久存储和临时存储;
 *    a> 永久存储数据我们存储到特定的数据文件中；
 *    b> 临时存储的数据我们存储到内存中，进程退出数据销毁；
 * </pre>
 *
 * @author 65821
 */
public class MediaPageState extends PageDataKV {

    /**
     * 唯一实例
     * <p> 页面数据只需要一个实例对象；
     */
    private static MediaPageState sInstance = null;

    private final WallpaperData mWallpaperData;

    /** 页面状态实例 */
    public static MediaPageState instance() {
        if (sInstance == null) {
            sInstance = new MediaPageState(
                    HUtilsEx.getApp().getApplicationContext());
        }

        return sInstance;
    }

    private MediaPageState(@NonNull Context context) {
        super(context);
        this.mWallpaperData = new WallpaperData(context);
    }

    /**
     * 保存数据并关闭当前实例
     * <p> 一般在应用程序正常退出的时候被调用；
     */
    public void saveAndCloseInstance() {
        if (!mIsOpen) {
            LogUtils.wTag(
                    MediaPageState.class.getSimpleName(),
                    "PageDataKV, Object is closed.");
            return;
        }

        syncToDisk();
        close();
    }

    /**
     * 返回壁纸数据
     * @return 结果
     */
    public WallpaperData getWallpaperData() {
        return mWallpaperData;
    }

    /**
     * 保存壁纸数据
     * <p/>当选中壁纸切换时操作
     * @param path 绝对路径
     */
    public void saveWallpaperData(String path) {
        write(PageDataKV.Key.MUSIC_WALLPAPER_PATH, path);
        if (new File(path).getName().startsWith("wallpaper_night_")) {
            mWallpaperData.setNightWallpaperPath(path);
        } else {
            mWallpaperData.setDayWallpaperPath(path);
        }
    }

}
