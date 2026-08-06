package com.hcn.media_theme;

import android.Configures.HConfig;
import android.os.Process;
import android.text.TextUtils;

import com.hcn.auto_compat.PlatformUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.config.Feature;


/**
 * 当前应用关联的外部配置参数
 * <p> 统一归类，避免都拥挤在 Application 组件类中；
 *
 * @author 65821
 */
public abstract class Argument {
    /**
     * [是支持 Vitamio 软解]
     * <pre>
     *     第三方库 vitamio 本质是对 ffmpeg 的封装；
     *     由于其不开源的性质，随着 Android 版本的升高，它可能不再能正常工作；
     *     后续需要考虑由 ffmpeg 替换的准备 (可以考虑使用 ijkplayer);
     * </pre>
     */
    private static boolean mIsSupportVitamio = false;

    /**
     * 当前车载系统主题类型
     * <p> {@link "file://$PROJECT_DIR$/app/mcc-skin.md" }
     */
    public static final int E_THEME_GOD = parseIntProperty(
            "persist.sys.etheme_god", String.valueOf(0));
    public static final int E_THEME_SUB = parseIntProperty(
            "persist.sys.etheme_sub", String.valueOf(0));

    /** 初始化当前应用的相关参数 **/
    public static void initialization() {
        mIsSupportVitamio = "1".equals(HUtilsEx.getSystemProperty(
                "persist.sys.soft_decoding", "1"));

        // 提升进程优先级
        if (PlatformUtils.isHardware(PlatformUtils.UIS8581)) {
            Process.setThreadPriority(Process.THREAD_PRIORITY_AUDIO);
        }

        // mcc501 支持深色模式，支持媒体暂停时进度条触摸，不支持无媒体超时退出
        if (E_THEME_GOD == 501) {
            Feature mFeature = Feature.instance();
            mFeature.addFeature(Feature.BIT.THEME_SUPPORT_NIGHT_MODE);
            mFeature.addFeature(Feature.BIT.SUPPORT_SEEKBAR_TOUCH);
            mFeature.removeFeature(Feature.BIT.SUPPORT_NO_FILE_TIMEOUT_EXIT);
        }
    }

    /** 海外版本 **/
    public static boolean isOverseasVersion() {
        return HConfig.isOverseaVersion();
    }

    /**
     * 解析 Int 类型属性值
     * @param propName 属性名字
     * @param defValue 默认值
     * @return 整形值
     * @Throws NumberFormatException 如果不匹配整形格式
     */
    private static int parseIntProperty(String propName, String defValue) {
        String propValue = HUtilsEx.getSystemProperty(propName, defValue);
        try {
            // 一定要去掉头尾的空格(你永远不知道事业部会怎么给你配置)
            if (!TextUtils.isEmpty(propValue)) {
                return Integer.parseInt(propValue.trim());
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
        }
        return 0;
    }

    /**
     * 主题查询（etheme_god）
     * <p> persist.sys.etheme_god
     *
     * @param themeGod 主题主 ID
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isThemeGod(final int themeGod) {
        return Argument.E_THEME_GOD == themeGod;
    }

    /**
     * 主题查询（etheme_sub）
     * <p> persist.sys.etheme_sub
     *
     * @param themeSub 主题副 ID
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isThemeSub(final int themeSub) {
        return Argument.E_THEME_SUB == themeSub;
    }

    /**
     * 是否是指定的主题类型
     * @param themeX {@link ThemeX}
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isThemeX(final int themeX) {
        return Argument.getThemeX() == themeX;
    }

    /**
     * 当前系统配置主题
     * <p> 真实的主题类型，也就是当前产品的主题配置；
     *
     * @return 主题类型
     */
    public static int getThemeX() {
        int themeType = Argument.E_THEME_GOD + Argument.E_THEME_SUB;
        if (!Argument.isThemeSub(0)) {
            themeType = Argument.E_THEME_GOD * 1000 + Argument.E_THEME_SUB;
        }
        return themeType;
    }

    /**
     * [是否支持 Vitamio 软解码]
     * <pre>
     *    支持播放更多的视频格式，e.g. rmvb、mpg、rm...
     *    需要注意，使用软解码播放视频时将占用更多的 CPU 资源；
     * </pre>
     *
     * @return 如果返回 <code>true</code> 支持软解码；反之，不支持软解码；
     */
    public static boolean isSupportVitamio() {
        return mIsSupportVitamio;
    }

    /** [休眠退出视频模式] **/
    public static boolean sleepExitVideoMode() {
        return "0".equals(HUtilsEx.getSystemProperty(
                "persist.sys.sleep_video", "1"));
    }

    /** [休眠退出视频后台] **/
    public static boolean sleepExitVideoBackground() {
        return "0".equals(HUtilsEx.getSystemProperty(
                "persist.sys.sleep_video_back", "1"));
    }

    /** 视频能否后台播放<注意: PIP也可以后台播放> **/
    public static boolean isCanPlayVideoBack() {
        return !"0".equals(HUtilsEx.getSystemProperty(
                "persist.sys.video_on_switch", "0"));
    }

    /** 是后台播放模式<不管是暂停还是播放状态，HomeKey都回主界面且不暂停播放> **/
    public static boolean isBackgroundPlayMode() {
        return "1".equals(HUtilsEx.getSystemProperty(
                "persist.sys.video_on_switch", "0"));
    }

    /**
     * 是否支持进入画中画
     * <pre>
     *   name: persist.sys.video_on_switch
     *   value:
     *     0, Only Normal Mode
     *     1, Support Background Mode
     *     2, Support PIP Mode
     * </pre>
     *
     * @return 是支持画中画/不支持
     */
    public static boolean isSupportPIP() {
        return "2".equals(HUtilsEx.getSystemProperty(
                "persist.sys.video_on_switch", "0"));
    }

    /**
     * 打开的情况下内置 EQ 相关功能不会被初始化；
     * <pre>
     *    内置 EQ 打开的情况下，影响设备音频指标测试工作；
     *    如果打开最直观的就是音频指标的 -分离度- 过不了；
     * </pre>
     * @return 是否打开了音频指标测试
     */
    public static boolean isAudioIndexTestEnable() {
        return "1".equals(HUtilsEx.getSystemProperty(
                "persist.sys.internal.eq.disable", "0"));
    }
}
