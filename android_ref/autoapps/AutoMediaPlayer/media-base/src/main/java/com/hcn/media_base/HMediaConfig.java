package com.hcn.media_base;

import com.hcn.common.utils.HUtilsEx;

/**
 * 当前应用程序配置定义
 * <p> 和 HMedia 相关的非调试信息配置统一放这里；
 *
 * @author 86158
 */
public final class HMediaConfig {

    /**
     * 视频显示比例定义
     * <pre>
     *     0: 自动缩放
     *     1: 16 : 09
     *     2: 04 : 03
     *     3: 自动填充
     *     4: 01 : 01
     * </pre>
     */
    public static final int VIDEO_SCALE_AUTO_ZOOM = 0;
    public static final int VIDEO_SCALE_1609_SIZE = 1;
    public static final int VIDEO_SCALE_0403_SIZE = 2;
    public static final int VIDEO_SCALE_AUTO_FULL = 3;
    public static final int VIDEO_SCALE_0101_ZOOM = 4;
    public static final int VIDEO_SCALE_TYPE_COUNT = VIDEO_SCALE_0101_ZOOM + 1;

    /**
     * SurfaceView 是否需要焦点
     * <p> 如果放开会让 SurfaceView 出现焦点边框
     */
    public static final boolean SURFACE_VIEW_FOCUSABLE = false;

    /**
     * 软解资源异步释放配置
     * <p> 避免无法释放，释放阻塞等情况；
     */
    public static final boolean VITAMIO_ASYNC_RELEASE = true;

    /**
     * 使用线程池同步 ID3 信息
     * <p> 数据量大的时候必须使用线程来同星系；
     */
    public static final boolean USE_THREAD_POOL_SYNC_ID3 = true;

    /**
     * 支持音乐播放界面背景磨砂效果
     * <p> 专辑封面会放大到播放界面背景中，以磨砂效果显示，默认显示这种效果；
     */
    public static final boolean SUPPORT_MUSIC_UI_BLUR_EFFECT = "true".equalsIgnoreCase(
            HUtilsEx.getSystemProperty("ro.media.ui.blur_effect", "true"));

    /**
     * 是否使用专辑封面旋转动画
     * <p> 个别海外客户接负载做高温实验的时候系统音量设置太大，再加动画的话系统热量会很高；
     */
    public static final boolean USE_ALBUM_COVER_ROTATE_ANIM = "true".equalsIgnoreCase(
            HUtilsEx.getSystemProperty("ro.media.albumcover.anim", "false"));

    /**
     * 视频软解码文件后缀
     * <p> 这些后缀的文件直接使用软解码播放视频；
     */
    public static final String VIDEO_SOFT_DECODER_SUFFIX = ".rm.mpg.rmvb.vob.";

    /**
     * 用 VITAMIO 获取视频帧的格式
     * <p> 这些后缀文件直接使用软解码获取视频某一帧图像；
     */
    public static final String VITAMIO_VIDEO_FRAME_SUFFIX = ".asf.dat.mpg.wmv.vob.mpeg.m2ts.";

    /**
     * 特殊视频帧后缀, 需要在播放中才能提取到视频显示缩略图
     * <p> 这些后缀的文件直接在播放中获取视频帧图像；
     */
    public static final String SPECIAL_VIDEO_FRAME_SUFFIX = ".rm.mpg.rmvb.m2ts.";
}
