/*
 * Copyright (c) 2010-2011, The MiCode Open Source Community (www.micode.net)
 *
 * This file is part of FileExplorer.
 *
 * FileExplorer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FileExplorer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SwiFTP.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.hcn.media_common.utils;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.WindowManager;

import androidx.annotation.DimenRes;
import androidx.annotation.NonNull;

import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.debug.MediaConfigEx;

import java.lang.reflect.Method;
import java.util.List;

/**
 * 工本地具类
 * <p> 后续可以考虑整合到共享工具库中；
 *
 * @author 86158
 */
public class MiscUtils {
    private static final String TAG = "Utils";

    /** 系统状态栏高度 **/
    private static int sSystemStatusBarHeight = -1;

    /** 获取文件名后缀 **/
    public static String getExtFromFilename(String filename) {
        int dotPosition = filename.lastIndexOf('.');
        if (dotPosition != -1 && dotPosition != 0) {
            return filename.substring(dotPosition + 1);
        }
        return "";
    }

    /**
     * 跳转到主界面
     *
     * @param context
     * @return
     */
    public static void goToHome(@NonNull Context context) {
        Intent tmpIntent = new Intent(Intent.ACTION_MAIN);
        tmpIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        tmpIntent.addCategory(Intent.CATEGORY_HOME);
        context.startActivity(tmpIntent);
    }

    /**
     * 获取状态栏高度
     *
     * @param context 上下文
     * @return 像素高度
     */
    public static int getStatusBarHeight(@NonNull Context context) {
        Resources resources = context.getResources();
        @SuppressLint("InternalInsetResource")
        int resourceId = resources.getIdentifier(
                "status_bar_height", "dimen", "android");
        return resources.getDimensionPixelSize(resourceId);
    }

    /**
     * 获取状态栏高度
     *
     * @param context 上下文
     * @return 像素高度
     */
    public static int getStatusBarHeightLandscape(@NonNull Context context) {
        Resources resources = context.getResources();
        @SuppressLint("InternalInsetResource")
        int resourceId = resources.getIdentifier(
                "status_bar_height_landscape", "dimen", "android");
        return resources.getDimensionPixelSize(resourceId);
    }

    /**
     * 当前包是前台应用
     *
     * @param context 上下文
     * @param pkgName application package name.
     * @return 是/否在前台
     */
    public static boolean isForegroundApp(@NonNull Context context, String pkgName) {
        ActivityManager am = context.getSystemService(ActivityManager.class);
        List<ActivityManager.RunningTaskInfo> tasks = am.getRunningTasks(1);
        return !tasks.isEmpty() && pkgName.equals(tasks.get(0).topActivity.getPackageName());
    }

    /**
     * 是竖屏显示窗口状态
     * <p> 物理竖屏显示，旋转竖屏显示，分屏竖屏状态
     *
     * @param context 上下文环境
     * @return 是/否
     */
    public static boolean isPortraitWindow(@NonNull Context context) {
        return context.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT;
    }


    /**
     * 返回设备是否是横屏设备
     * @param context 上下文环境
     *
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isHorizontalScreenDevice(@NonNull Context context) {
        DisplayMetrics dm;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            dm = new DisplayMetrics();
            windowManager.getDefaultDisplay().getRealMetrics(dm);
        } else {
            dm = context.getApplicationContext().getResources().getDisplayMetrics();
        }
        Log.d(TAG, "dm.widthPixels = " + dm.widthPixels + ", dm.heightPixels = " + dm.heightPixels);
        return dm.widthPixels >= dm.heightPixels;
    }

    /**
     * 获取状态栏高度
     * <p> 根据当前横竖屏不一样的情况获取系统定义的值；
     *
     * @param context 上下文环境
     * @return 状态栏高度
     */
    public static int statusBarHeight(@NonNull Context context, @DimenRes int defaultResId) {
        if (sSystemStatusBarHeight < 0) {
            Resources resources = context.getResources();
            int statusBarHeight = resources.getDimensionPixelSize(defaultResId);
            try {
                statusBarHeight = MiscUtils.isHorizontalScreenDevice(context) ?
                        MiscUtils.getStatusBarHeightLandscape(context) : MiscUtils.getStatusBarHeight(context);
            } catch (Resources.NotFoundException ignored) {
            } finally {
                sSystemStatusBarHeight = statusBarHeight;
                LogUtil.v(TAG, "statusBarHeight: " + statusBarHeight);
            }
        }
        return sSystemStatusBarHeight;
    }

    /**
     * 获取当前设备总内存
     * <p> 获取系统真实的内存总数 </p>
     *
     * @param defaultValue 默认值
     * @return 当前设备总内存
     */
    public static long getTotalMemory(long defaultValue) {
        long size = defaultValue;
        try {
            Class<?> process = Class.forName("android.os.Process");
            Method method = process.getMethod("getTotalMemory");
            size = (Long) (method.invoke(process, (Object[]) null));
            Log.d(MediaConfigEx.TAG, "size = " + size);
        } catch (Exception e) {
            Log.d(MediaConfigEx.TAG, "getTotalMemory, " + e.getMessage());
        }
        return size;
    }

    /**
     * 反向比较2个字符串大小, 多媒体场景可以提高比较效率
     * @param obj1
     * @param obj2
     * @return
     */
    public static boolean reverseEquals(String obj1, String obj2) {
        if (TextUtils.isEmpty(obj1)
                || TextUtils.isEmpty(obj2)) {
            return false;
        }

        // 先只需比较对象的地址
        if (obj2 == obj1) {
            return true;
        }

        // 再反向比较对象的内容
        int n = obj1.length();
        if (n == obj2.length()) {
            int i = n-1;
            while (n-- != 0) {
                if (obj1.charAt(i) != obj2.charAt(i)) {
                    return false;
                }
                i--;
            }
            return true;
        }

        return false;
    }

    /**
     * 格式化歌曲目录，最低为两位
     *
     * @param position 歌曲位置
     * @param size 歌曲总数量
     * @return e.g. if (position=65 size=200) --> "065"
     * @deprecated 这个函数设计存在问题，不符合函数独立原理；
     */
    @Deprecated
    public static String formatNum(int position, int size) {
        String num = String.valueOf(position + 1);
        int digit = String.valueOf(size).length();
        digit = digit == 1 ? digit + 1 : digit;
        String format = "";
        for (int i = 0; i < digit - num.length(); i++) {
            format = format.concat("0");
        }
        return format.concat(num);
    }
}
