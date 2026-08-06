package com.hcn.media_theme;

import android.app.Application;
import android.graphics.drawable.Drawable;

import com.hcn.auto.theme.utils.ThemeUtils;
import com.hcn.auto_compat.PlatformUtils;

/**
 * 主题工具扩展
 * <pre>
 *    历史皮肤资源共享解决方案（不相干的其它接口别加到这个工具类）；
 *    有些平台现在已经不使用这个方案，所以不必要初始化（可以节省启动时间 20ms）；
 * </pre>
 *
 * @see com.hcn.auto.theme.utils.ThemeUtils
 * @author 65821
 */
public class ThemeUtilsEx {

    /**
     * 初始化共享资源包
     * @param app 应用组件
     */
    public static void init(Application app) {
        // 不支持就不要浪费时间
        if (!supportShareResource()) {
            return;
        }

        ThemeUtils.init(app);
    }

    /**
     * uis8581 暂时不使用共享资源
     * @return 支持/不支持
     */
    public static boolean supportShareResource() {
        return !PlatformUtils.isHardware(PlatformUtils.UIS8581);
    }

    /**
     * 获取应用共享背景资源
     * @return {@link Drawable}
     */
    public static Drawable getAppShareBackground() {
        if (!supportShareResource()) {
            return null;
        }

        return ThemeUtils.getInstance().getAppBackground();
    }
}
