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

package com.hcn.autoradio.util;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.carstatus.CarStatus;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.hcn.autoradio.R;
import com.hcn.autoradio.skin.SkinUtils;
import com.hcn.common.utils.HSPUtils;


import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;


/**
 * @author simon
 */
public class RadioUtils {
    private final static String TAG = "RadioUtils";
    //Radio Model
    public static final String RADIO_INSIDE = "mt6627";
    public static final String RADIO_SI4754 = "si4754";
    public static final String RADIO_TEA6851 = "tea6851";
    public static final String RADIO_QN8035 = "qn8035";
    public static final String RDS_INSIDE = "rds_inside";

    private static String radioModel = RDS_INSIDE;

    public static final String MT8321 = "mt8321";
    public static final String UIS8581 = "uis8581";
    public static final String SM6225 = "sm6225";
    public final static String SP_FILE_NAME = "auto_radio_client";
    public final static String WALLPAPAER_SAVE_PATH = "wallpaper_save_path";
    private static CarStatus carStatus = new CarStatus();

    /**
     * 内置收音机 AudioTrack+AudioRecord 进行播放声音
     * @return
     */
    public static String initRadioModel() {
        radioModel = getProp("ro.hw.radio.chip", RDS_INSIDE);
        Log.d(TAG, "initRadioModel   mRadioModel=" + radioModel);
        return radioModel;
    }

    public static String getRadioModel() {
        return radioModel;
    }

    /**
     * 获取系统属性
     * @param key
     * @param defaultValue
     * @return
     */
    public static String getProp(String key, String defaultValue) {
        String result = "";
        try {
            Class<?> SystemProperties = Class.forName("android.os.SystemProperties");
            Method get = SystemProperties.getMethod("get", String.class, String.class);
            Object[] params = new Object[]{key, defaultValue};
            result = (String) (get.invoke(SystemProperties, params));
        } catch (ClassNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return result;
    }

    @SuppressLint("PrivateApi")
    public static String getProperties(String key){
        String values = "";
        Class<?> cls = null;
        try {
            cls = Class.forName("android.os.SystemProperties");
            Method mMethod = cls.getMethod("get", String.class);
            values = (String) mMethod.invoke(cls, key);
        } catch (ClassNotFoundException
                | NoSuchMethodException
                | IllegalAccessException
                | IllegalArgumentException
                | InvocationTargetException e) {
            e.printStackTrace();
        }
        return values;
    }

    /**
     * 获取系统属性
     * @param key
     * @param defaultValue
     * @return
     */
    public static int getIntProp(String key, int defaultValue) {
        int result = defaultValue;
        try {
            Class<?> systemProperties = Class.forName("android.os.SystemProperties");
            Method get = systemProperties.getMethod("getInt", String.class, int.class);
            Object[] params = new Object[]{key, defaultValue};
            result = (int) (get.invoke(systemProperties, params));
        } catch (Exception ignored) {
        }
        return result;
    }

    /**
     * 设置系统属性
     * @param property
     * @param value
     */
    public static void setSystemProperties(final String property, final String value) {
        try {
            Class<?> SystemProperties = Class.forName("android.os.SystemProperties");
            Method set = SystemProperties.getMethod("set", String.class, String.class);
            Object[] params = new Object[]{new String(property), value};
            set.invoke(SystemProperties, params);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    public static String getExtFromFilename(String filename) {
        int dotPosition = filename.lastIndexOf('.');
        if (dotPosition != -1 && dotPosition != 0) {
            return filename.substring(dotPosition + 1, filename.length());
        }
        return "";
    }

    /**
     * 跳转到主界面
     *
     * @param context
     * @return
     */
    public static void goToHome(Context context) {
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
    public static int getStatusBarHeight(Context context) {
        Resources resources = context.getResources();
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
    public static int getStatusBarHeightLandscape(Context context) {
        Resources resources = context.getResources();
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
     * uis8581 暂时不使用共享资源
     * @return 支持/不支持
     */
    public static boolean supportShareResource() {
        return !isHardware(UIS8581);
    }

    public static boolean isHardware(String platform) {
        return !TextUtils.isEmpty(platform) && Build.HARDWARE.toLowerCase().contains(platform.toLowerCase());
    }

    /**
     * 请求播放声音（切源）
     * <pre>
     *     不同的平台 carservices.jar 有差异；
     *     某些方法并不是每个版本都存在，如果不存在又访问了会异常；
     *     所以对于这种需求，我们这里统一反射处理；
     * </pre>
     *
     * @param context 上下文环境
     * @return 执行成功
     * @see android.sourceservice.SourceInfo
     */
    public static boolean requestPlayAudio(@NonNull Context context) {
        try {
            @SuppressLint("PrivateApi")
            Class<?> siClass = Class.forName(
                    "android.sourceservice.SourceInfo");
            Object instance = siClass.getMethod(
                    "getInstance").invoke(null);
            if (instance != null) {
                Method requestPlayAudio = siClass.getMethod(
                        "onRequestPlayAudio", Context.class);
                Object[] params = new Object[]{context};
                requestPlayAudio.invoke(instance, params);
                return true;
            }
        } catch (Exception e) {
            Log.d(TAG, "requestPlayAudio: " + e.toString());
        }
        return false;
    }

    /**
     * 请求播放声音（切源）
     * <pre>
     *     不同的平台 carservices.jar 有差异；
     *     某些方法并不是每个版本都存在，如果不存在又访问了会异常；
     *     所以对于这种需求，我们这里统一反射处理；
     * </pre>
     *
     * @param packageName 包名
     * @return 执行成功
     * @see android.sourceservice.SourceInfo
     */
    public static boolean requestPlayAudio(@NonNull String packageName) {
        try {
            @SuppressLint("PrivateApi")
            Class<?> siClass = Class.forName(
                    "android.sourceservice.SourceInfo");
            Object instance = siClass.getMethod(
                    "getInstance").invoke(null);
            if (instance != null) {
                Method requestPlayAudio = siClass.getMethod(
                        "onRequestPlayAudio", String.class);
                Object[] params = new Object[]{packageName};
                requestPlayAudio.invoke(instance, params);
                return true;
            }
        } catch (Exception e) {
            Log.d(TAG, "requestPlayAudio: " + e.toString());
        }
        return false;
    }

    /**
     * 设置外挂DSP的Radio、Aux通道音量
     *
     * @param path   ExtAudioMuxer.PATH_RADIO=0,ExtAudioMuxer.PATH_AUX=2
     * @param volume 0-100
     */
    public static void setInputPathVolume(int path, int volume) {
        try {
            @SuppressLint("PrivateApi")
            Class<?> siClass = Class.forName(
                    "android.sourceservice.SourceInfo");
            Object instance = siClass.getMethod(
                    "getInstance").invoke(null);
            if (instance != null) {
                Method setInputPathVolume = siClass.getMethod(
                        "setInputPathVolume", int.class, int.class);
                Object[] params = new Object[]{path, volume};
                setInputPathVolume.invoke(instance, params);
            }
        } catch (Exception e) {
            Log.d(TAG, "setInputPathVolume: " + e.toString());
        }
    }

    /**
     * 获取程序版本号
     * @param context
     * @return
     */
    public static String getVersionName(Context context) {
        String localVersionName = "null";
        int localVersionCode = 0;
        PackageManager manager = context.getPackageManager();
        try {
            PackageInfo info = manager.getPackageInfo(context.getPackageName(), 0);
            localVersionName = info.versionName; // 版本名
            localVersionCode = info.versionCode; // 版本号
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return localVersionName;
    }
    public static void onButtSettingEvent(Context context, String packageName, String className) {
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_LAUNCHER);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        ComponentName cn =new ComponentName(packageName,className);
        intent.setComponent(cn);
        context.startActivity(intent);
    }

    /*** @Des: 是否支持动态切换壁纸*/
    public static boolean supportWallpaperCustomized(){
        return SkinUtils.getInteger(R.integer.support_wallpaper_customized) == 1;
    }

    /*** @Des: 获取动态设置壁纸的路径*/
    public static String getWallpaperSavePath(){
        return HSPUtils.getInstance(SP_FILE_NAME).getString(WALLPAPAER_SAVE_PATH);
    }
    /*** @Des: 保存动态设置壁纸的路径*/
    public static void setWallpaperSavePath(String path){
        HSPUtils.getInstance(SP_FILE_NAME).put(WALLPAPAER_SAVE_PATH, path);
    }


    /* 大灯是否亮起*/
    public static boolean isNightMode(Configuration configuration) {
        int currentNightMode = configuration.uiMode
                & Configuration.UI_MODE_NIGHT_MASK;
        if (currentNightMode == Configuration.UI_MODE_NIGHT_YES) {
            Log.e(TAG,"night");
            return true;
        } else {
            Log.e(TAG,"day");
            return false;
        }
    }

    /**
     * 是否在倒车状态
     * @return {@link boolean}
     */
    public static boolean isReversing() {
        return carStatus.getReversing() > 0;
    }

}
