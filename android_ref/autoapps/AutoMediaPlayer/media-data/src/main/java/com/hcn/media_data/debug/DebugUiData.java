package com.hcn.media_data.debug;

/**
 * UI 调试配置
 * <p> 可以通过动态设置调试等级来打开 UI 打印信息；
 *
 * @author 65821
 */
public class DebugUiData {
    /** 音乐页面调试等级 | V */
    public static boolean MUSIC_DEBUG_V = false;

    /** 音乐页面调试等级 | D */
    public static boolean MUSIC_DEBUG_D = false;

    /** 音乐页面调试等级 | I */
    public static boolean MUSIC_DEBUG_I = false;

    /** 视频页面调试等级 | V */
    public static boolean VIDEO_DEBUG_V = false;

    /** 视频页面调试等级 | D */
    public static boolean VIDEO_DEBUG_D = false;

    /** 视频页面调试等级 | I */
    public static boolean VIDEO_DEBUG_I = false;

    /** 配置是否默认强制打开媒体 EQ 页面 */
    public static boolean FORCE_ENABLE_MEDIA_EQ = false;
}
