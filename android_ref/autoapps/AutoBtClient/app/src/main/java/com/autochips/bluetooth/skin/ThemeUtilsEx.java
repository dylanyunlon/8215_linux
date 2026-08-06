package com.autochips.bluetooth.skin;

import android.app.Application;
import android.graphics.drawable.Drawable;
import android.text.TextUtils;

import com.hcn.auto.theme.utils.ThemeUtils;
import com.hcn.bluetooth.api.Utils;


/**
 * 主题工具扩展
 * <pre>
 *    历史皮肤资源共享解决方案（不相干的其它接口别加到这个工具类）；
 *    有些平台现在已经不使用这个方案，所以不必要初始化（可以节省启动时间 20ms）；
 * </pre>
 *
 * @author 65821
 * @see ThemeUtils
 */
public class ThemeUtilsEx {

    /**
     * 平台参数
     */
    public static final String MT8163 = "mt163";
    public static final String MT8321 = "mt8321";
    public static final String UIS8581 = "uis8581";
    public static final String SM6225 = "bengal_515";

    /**
     * 初始化共享资源包
     *
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
     * uis8581 sm6225 暂时不使用共享资源
     *
     * @return 支持/不支持
     */
    public static boolean supportShareResource() {
        return !isHardware(UIS8581) && !isHardware(SM6225);
    }

    /**
     * 获取平台信息
     * @param platform
     * @return
     */
    public static boolean isHardware(String platform) {
        if (null == Utils.getPlatform()) {
            return false;
        }
        return !TextUtils.isEmpty(platform) && Utils.getPlatform().toLowerCase().contains(platform.toLowerCase());
    }

    /**
     * 获取应用共享背景资源
     *
     * @return {@link Drawable}
     */
    public static Drawable getAppShareBackground() {
        if (!supportShareResource()) {
            return null;
        }

        return ThemeUtils.getInstance().getAppBackground();
    }
}
