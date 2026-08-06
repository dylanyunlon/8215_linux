package com.hcn.common;

import android.Configures.HConfig;
import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.provider.Settings;
import android.text.TextUtils;
import android.view.View;

import com.hcn.common.utils.HUtilsEx;

/**
 * 工具类接口
 * <p> 非必要，尽可能不要放在这里；
 *
 * @author 86158
 */
public class Utility {
    /**
     * 按返回键是否退出应用
     *
     * @return
     */
    public static boolean isExitOnBackKey() {
        return "1".equals(HUtilsEx.getSystemProperty(
                "persist.sys.media_exit", "1"));
    }

    /**
     * 获取行车中是否可以看视频的设置值
     *
     * @param context
     * @return true:可以视频
     */
    public static boolean canWatchVideoDriving(Context context) {
        if (context == null) {
            return false;
        }
        return 1 == Settings.System.getInt(
                context.getContentResolver(), HConfig.driving_disable_video, 1);
    }

    /**
     * 获取媒体播放名称显示方式
     *
     * @param context
     * @return [true]<ID3 Title>, [false]<file name>
     */
    public static boolean supportMediaId3Title(Context context) {
        if (null == context) {
            return true;
        }

        String showId3Title = Settings.System.getString(
                context.getContentResolver(), HConfig.media_show_id3_title);
        if (!TextUtils.isEmpty(showId3Title)) {
            final String NO_SUPPORT = "0";
            return !showId3Title.equals(NO_SUPPORT);
        }

        return true;
    }

    /**
     * 获取目标视图所在的 Activity
     *
     * @param view 目标视图
     * @return 当前 Activity 对象
     */
    public static Activity getActivityFromView(View view) {
        if (null != view) {
            Context context = view.getContext();
            while (context instanceof ContextWrapper) {
                if (context instanceof Activity) {
                    return (Activity) context;
                }
                context = ((ContextWrapper) context).getBaseContext();
            }
        }
        return null;
    }
}
