package com.hcn.media.utils;

import android.text.TextUtils;
import android.util.Log;

/**
 * 临时打印工具
 * @author 65821
 */
public class LogUtils {
    /** 全局打印标签 **/
    private static String GLOBAL_TAG = "MediaApi";

    /** 全局调试开关 **/
    private static boolean DEBUG = false;

    /** 打印等级控制 **/
    protected static boolean DEBUG_V = Log.isLoggable(GLOBAL_TAG, Log.VERBOSE);
    protected static boolean DEBUG_D = Log.isLoggable(GLOBAL_TAG, Log.DEBUG);
    protected static boolean DEBUG_I = Log.isLoggable(GLOBAL_TAG, Log.INFO);
    protected static boolean DEBUG_W = Log.isLoggable(GLOBAL_TAG, Log.WARN);

    private LogUtils() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    /**
     * 配置工具类
     *
     * @param debug 日志打印开关
     * @param tag 打印 TAG
     */
    public static void setConfig(boolean debug, String tag) {
        DEBUG = debug;

        if (!TextUtils.isEmpty(tag)) {
            GLOBAL_TAG = tag;
        }
    }

    /**
     * 设置调试等级开关
     *
     * @param level 调试等级
     * @param debug 打印开关
     */
    public static void setDebug(int level, boolean debug) {
        switch (level) {
            case Log.VERBOSE:
                DEBUG_V = debug;
                break;
            case Log.DEBUG:
                DEBUG_D = debug;
                break;
            case Log.INFO:
                DEBUG_I = debug;
                break;
            case Log.WARN:
                DEBUG_W = debug;
                break;
            default:
                break;
        }
    }

    /**
     * 获取全局 TAG
     * @return {@link #GLOBAL_TAG}
     */
    public static String getGlobalTag() {
        return GLOBAL_TAG;
    }

    public static void v(final String contents) {
        if (!DEBUG || !DEBUG_V) {
            return;
        }

        Log.v(GLOBAL_TAG, contents);
    }

    public static void vTag(final String tag, final String contents) {
        if (!DEBUG || !DEBUG_V) {
            return;
        }

        Log.v(tag, contents);
    }

    public static void d(final String contents) {
        if (!DEBUG || !DEBUG_D) {
            return;
        }

        Log.d(GLOBAL_TAG, contents);
    }

    public static void dTag(final String tag, final String contents) {
        if (!DEBUG || !DEBUG_D) {
            return;
        }

        Log.d(tag, contents);
    }

    public static void i(final String contents) {
        if (!DEBUG || !DEBUG_I) {
            return;
        }

        Log.i(GLOBAL_TAG, contents);
    }

    public static void iTag(final String tag, final String contents) {
        if (!DEBUG || !DEBUG_I) {
            return;
        }

        Log.i(tag, contents);
    }

    public static void w(final String contents) {
        if (!DEBUG || !DEBUG_W) {
            return;
        }

        Log.w(GLOBAL_TAG, contents);
    }

    public static void wTag(final String tag, final String contents) {
        if (!DEBUG || !DEBUG_W) {
            return;
        }

        Log.w(tag, contents);
    }

    /** 强制打印不设置约束条件 **/
    public static void e(final String contents) {
        Log.e(GLOBAL_TAG, contents);
    }

    /** 强制打印不设置约束条件 **/
    public static void eTag(final String tag, final String contents) {
        Log.e(tag, contents);
    }
}
