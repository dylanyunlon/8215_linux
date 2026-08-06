package com.hcn.media_theme;

import android.content.Context;
import android.os.Build;
import android.util.Log;

import androidx.annotation.NonNull;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.auto_compat.app.WindowConfiguration;
import com.hcn.skinx.SkinX;
import com.hcn.skinx.config.SkinConfig;

/**
 * 主题扩展配置
 * <pre>
 *    主题相关的配置文件说明；
 *    可以用来声明某些主题使用了特定的配置布局文件等；
 * </pre>
 *
 * @author 65821
 */
public abstract class ThemeEx implements ThemeX {

    /**
     * 是否支持数据库存储
     * <pre>
     *    由于初始化数据库需要时间，有初始化消耗；
     *    所以只有支持数据库存储的才操作数据库，避免不必要的初始化动作；
     * </pre>
     * <p> 数据库比较特殊需要提前初始化，所以不能从皮肤包中去配置；
     *
     * @deprecated 过时的接口，后续需要所有皮肤都支持数据库操作；
     * @return 支持/不支持
     */
    public static boolean supportDatabaseStorage() {
        switch (Argument.getThemeX()) {
            case ThemeX.ET_GOD_600:
            case ThemeX.ET_GOD_400_100:
            case ThemeX.ET_GOD_400_103:
            case ThemeX.ET_GOD_400_104:
            case ThemeX.ET_GOD_400_105:
            case ThemeX.ET_GOD_400_109:
            case ThemeX.ET_GOD_400_110:
            case ThemeX.ET_GOD_400_139:
                return true;
            case ThemeX.ET_GOD_NONE:
            default:
                break;
        }

        return SkinX.getBoolean(
                "support_database_storage", true);
    }

    /**
     * 是否使用专有的查找列表项布局文件
     * <pre>
     *    [music/video_search_list_item.xml]
     *    单独的查找页面列表项布局文件，布局效果细节处理更好；
     * </pre>
     * <pre>
     *    ## 后续皮肤包请直接通过配置文件配置:
     *    ------------------------------------------------------------------
     *    ./res-compat/config/values[-mcc?-mnc?]/config.xml
     *    ------------------------------------------------------------------
     *    &lt;resources&gt;
     *      &lt;bool name="use_search_list_item_layout"&gt;true&lt;/bool&gt;
     *    &lt;/resources&gt;
     *    ------------------------------------------------------------------
     * </pre>
     *
     * @return the boolean
     */
    public static boolean useSearchListItemLayout() {
        final int theme = Argument.getThemeX();
        switch (theme) {
            case ThemeX.ET_GOD_600:
            case ThemeX.ET_GOD_400_016:
            case ThemeX.ET_GOD_400_022:
            case ThemeX.ET_GOD_400_100:
            case ThemeX.ET_GOD_400_103:
            case ThemeX.ET_GOD_400_104:
            case ThemeX.ET_GOD_400_105:
            case ThemeX.ET_GOD_400_109:
            case ThemeX.ET_GOD_400_110:
            case ThemeX.ET_GOD_400_139:
                // 都是扩展皮肤包 skinx-blue01 中的主体，所以需要严格判定
                if (!SkinX.useSkinPackage()) {
                    LogUtils.i("useSearchListItemLayout: config error!");
                    return false;
                }
                return true;
            default:
                break;
        }

        // 直接去读取资源包中的配置信息
        return SkinX.getBoolean("use_search_list_item_layout");
    }

    /**
     * 是否使用了扩展的视频列表布局文件
     * <pre>
     *    [video/fragment_videolist_expand.xml]
     *    [video/fragment_videosearch_expand.xml]
     *    为了兼容历史版本，默认都认为使用了，只有特意声明不使用才需要在此添加；
     * </pre>
     * <pre>
     *    ## 后续皮肤包请直接通过配置文件配置:
     *    --------------------------------------------------------------------
     *    ./res-compat/config/values[-mcc?-mnc?]/config.xml
     *    --------------------------------------------------------------------
     *    &lt;resources&gt;
     *       &lt;bool name="use_video_list_expand_layout"&gt;false&lt;/bool&gt;
     *    &lt;/resources&gt;
     *    --------------------------------------------------------------------
     * </pre>
     *
     * @return 使用/未使用
     */
    public static boolean useVideoListExpandLayout() {
        // 可在此扩展添加完善
        switch (Argument.E_THEME_GOD) {
            case ThemeX.ET_GOD_400:
            case ThemeX.ET_GOD_501:
            case ThemeX.ET_GOD_600:
                return false;
            default:
                break;
        }

        // 直接去读取资源包中的配置信息
        return SkinX.getBoolean(
                "use_video_list_expand_layout", true);
    }

    /**
     * 是否支持文件夹列表 Item 显示专对应的专辑封面
     * <pre>
     *    默认都不支持显示（效率问题）
     *    特殊的主题需求可以在此添加；
     * </pre>
     * <pre>
     *    ## 后续皮肤包请直接通过配置文件配置:
     *    --------------------------------------------------------------------
     *    ./res-compat/config/values[-mcc?-mnc?]/config.xml
     *    --------------------------------------------------------------------
     *    &lt;resources&gt;
     *       &lt;bool name="support_folder_list_album_cover"&gt;true&lt;/bool&gt;
     *    &lt;/resources&gt;
     *    --------------------------------------------------------------------
     * </pre>
     *
     * @return 支持/不支持
     */
    public static boolean supportFolderListAlbumCover() {
        // 主风格代码
        switch (Argument.E_THEME_GOD) {
            case ThemeX.ET_GOD_405:
                return true;
            case ThemeX.ET_GOD_NONE:
            default:
                break;
        }

        final int theme = Argument.getThemeX();
        switch (theme) {
            case ThemeX.ET_GOD_400_109:
            case ThemeX.ET_GOD_400_110:
            case ThemeX.ET_GOD_400_139:
                return true;
            case ThemeX.ET_GOD_NONE:
            default:
                break;
        }

        // 直接去读取资源包中的配置信息
        return SkinX.getBoolean("support_folder_list_album_cover");
    }

    /**
     * 使用抽屉布局呈现音乐播放页面
     * <pre>
     *    按需求可以扩展配置，统一在此添加；
     *    [blue01/music/layout-mcc400-mnc100/fragment_musicinfo.xml]
     * </pre>
     * <pre>
     *    ## 后续皮肤包请直接通过配置文件配置:
     *    --------------------------------------------------------------------
     *    ./res-compat/config/values[-mcc?-mnc?]/config.xml
     *    --------------------------------------------------------------------
     *    &lt;resources&gt;
     *       &lt;bool name="use_music_drawer_layout_style"&gt;true&lt;/bool&gt;
     *    &lt;/resources&gt;
     *    --------------------------------------------------------------------
     * </pre>
     *
     * @return 使用/未使用
     */
    public static boolean useMusicDrawerLayoutStyle() {
        final int theme = Argument.getThemeX();
        switch (theme) {
            case ThemeX.ET_GOD_600:
            case ThemeX.ET_GOD_400_016:
            case ThemeX.ET_GOD_400_022:
            case ThemeX.ET_GOD_400_100:
            case ThemeX.ET_GOD_400_103:
            case ThemeX.ET_GOD_400_104:
            case ThemeX.ET_GOD_400_105:
            case ThemeX.ET_GOD_400_109:
            case ThemeX.ET_GOD_400_110:
            case ThemeX.ET_GOD_400_139:
                // 都是扩展皮肤包 skinx-blue01 中的主体，所以需要严格判定
                if (!SkinX.useSkinPackage()) {
                    LogUtils.i("useMusicDrawerLayoutStyle: config error!");
                    return false;
                }
                return true;
            default:
                break;
        }

        // 直接去读取资源包中的配置信息
        return SkinX.getBoolean("use_music_drawer_layout_style");
    }

    /**
     * 返回是否支持横屏设备兼容处理
     * <pre>
     *    部分皮肤明确需求，可以无需兼容；
     *    横屏状态下需要支持视频尺寸调节功能的，都需要在此添加皮肤主题代号；
     * <pre>
     * <pre>
     *    ## 后续皮肤包请直接通过配置文件配置:
     *    --------------------------------------------------------------------
     *    ./res-compat/config/values[-mcc?-mnc?]/config.xml
     *    --------------------------------------------------------------------
     *    &lt;resources&gt;
     *       &lt;bool name="support_landscape_screen_compat"&gt;true&lt;/bool&gt;
     *    &lt;/resources&gt;
     *    --------------------------------------------------------------------
     * </pre>
     *
     * @param context 上下文环境
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isHorizontalScreenDeviceCompat(@NonNull Context context) {
        // 带视频尺寸调节功能的主题添加到此处。
        boolean compat;
        switch (Argument.E_THEME_GOD) {
            case ThemeX.ET_GOD_202:
            case ThemeX.ET_GOD_321:
            case ThemeX.ET_GOD_402:
            case ThemeX.ET_GOD_403:
            case ThemeX.ET_GOD_400:
            case ThemeX.ET_GOD_501:
            case ThemeX.ET_GOD_600:
                compat = false;
                break;
            default:
                // 如不特殊指定，那么我们默认就需要支持；
                compat = SkinX.getBoolean(
                        "support_landscape_screen_compat", true);
                break;
        }

        return MiscUtils.isHorizontalScreenDevice(context) && compat;
    }

    /**
     * 视频是否支持自由窗口显示模式
     * <p> {@link WindowConfiguration#WINDOWING_MODE_FREEFORM}
     * <pre>
     *    ## 后续皮肤包请直接通过配置文件配置:
     *    --------------------------------------------------------------------
     *    ./res-compat/config/values[-mcc?-mnc?]/config.xml
     *    --------------------------------------------------------------------
     *    &lt;resources&gt;
     *       &lt;bool name="support_video_freeform_windowing_mode"&gt;true&lt;/bool&gt;
     *    &lt;/resources&gt;
     *    --------------------------------------------------------------------
     * </pre>
     *
     * @return 支持/不支持
     */
    public static boolean videoSupportFreeFormWindowingMode() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.P) {
            return false;
        }

        final int theme = Argument.getThemeX();
        switch (theme) {
            case ThemeX.ET_GOD_600:
            case ThemeX.ET_GOD_400_100:
            case ThemeX.ET_GOD_400_103:
            case ThemeX.ET_GOD_400_104:
            case ThemeX.ET_GOD_400_105:
            case ThemeX.ET_GOD_400_109:
            case ThemeX.ET_GOD_400_110:
            case ThemeX.ET_GOD_400_139:
                return true;
            case ThemeX.ET_GOD_NONE:
            default:
                break;
        }

        // 直接去读取资源包中的配置信息
        return SkinX.getBoolean("support_video_freeform_windowing_mode");
    }

    /**
     * 音乐信息页面是否支持带 Gallery 画廊效果的播放页面
     * <pre>
     *    ## 后续皮肤包请直接通过配置文件配置:
     *    --------------------------------------------------------------------
     *    ./res-compat/config/values[-mcc?-mnc?]/config.xml
     *    --------------------------------------------------------------------
     *    &lt;resources&gt;
     *       &lt;bool name="support_music_info_ext"&gt;true&lt;/bool&gt;
     *    &lt;/resources&gt;
     *    --------------------------------------------------------------------
     * </pre>
     *
     * @return 支持/不支持
     */
    public static boolean musicSupportMusicInfoExt(){
        final int theme = Argument.getThemeX();
        switch (theme) {
            case ThemeX.ET_GOD_153:
            case ThemeX.ET_GOD_405:
            case ThemeX.ET_GOD_405_001:
            case ThemeX.ET_GOD_406:
                return true;
            default:
                break;
        }

        switch (Argument.E_THEME_GOD) {
            case ThemeX.ET_GOD_153:
            case ThemeX.ET_GOD_405:
            case ThemeX.ET_GOD_406:
                return true;
            default:
                break;
        }

        // 直接去读取资源包中的配置信息
        return SkinX.getBoolean("support_music_info_ext");
    }

    /**
     * 白天黑夜切换是否由 recreate 实现
     * <pre>
     *    ## 后续皮肤包请直接通过配置文件配置:
     *    --------------------------------------------------------------------
     *    ./res-compat/config/values[-mcc?-mnc?]/config.xml
     *    --------------------------------------------------------------------
     *    &lt;resources&gt;
     *       &lt;bool name="support_uimode_recreate"&gt;true&lt;/bool&gt;
     *    &lt;/resources&gt;
     *    --------------------------------------------------------------------
     * </pre>
     *
     * @return 支持/不支持
     */
    public static boolean supportUiModeRecreate() {
        final String skinType = SkinX.currentSkinType();
        switch (skinType) {
            case "gb03":
            case "gb04":
                return true;
            default:
                break;
        }

        final int theme = Argument.getThemeX();
        switch (theme) {
            case ThemeX.ET_GOD_501:
                return true;
            case ThemeX.ET_GOD_NONE:
            default:
                break;
        }

        // 直接去读取资源包中的配置信息
        return SkinX.getBoolean("support_uimode_recreate");
    }
}
