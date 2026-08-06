package com.hcn.autoeq.util;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Build;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import java.lang.reflect.Method;

public class SystemUtils {

    public static String getSystemProperty(String property, String defaultValue) {
        String ret = null;
        try {
            Class<?> SystemProperties = Class.forName("android.os.SystemProperties");
            Method get = SystemProperties.getMethod("get", String.class, String.class);
            Object[] params = new Object[]{new String(property), defaultValue};
            ret = (String) (get.invoke(SystemProperties, params));
        } catch (Exception ignore) {
        }
        return ret;
    }

    public static void setSystemProperty(String property, String value) {
        try {
            Class<?> SystemProperties = Class.forName("android.os.SystemProperties");
            Method set = SystemProperties.getMethod("set", String.class, String.class);
            Object[] params = new Object[]{new String(property), value};
            set.invoke(SystemProperties, params);
        } catch (Exception ignore) {
        }
    }

    /**
     * 横竖屏判断(旋转屏)
     *
     * @param context
     */
    public static boolean isOrientationPort(Context context) {
        if (context == null) return false;
        Configuration mConfiguration = context.getResources().getConfiguration();
        int ori = mConfiguration.orientation; //获取屏幕方向
        if (ori == Configuration.ORIENTATION_LANDSCAPE) { // 横屏
            return false;
        } else if (ori == Configuration.ORIENTATION_PORTRAIT) { // 竖屏
            return true;
        }
        return false;
    }

    public static void fullScreen(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = activity.getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }
    }

    /**
     * 使用java正则表达式去掉多余的.与0
     *
     * @param s
     * @return
     */
    public static String subZeroAndDot(String s) {
        if (s != null && s.indexOf(".") > 0) {
            s = s.replaceAll("0+?$", "");//去掉多余的0
            s = s.replaceAll("[.]$", "");//如最后一位是.则去掉
        }
        return s;
    }

    public static StringBuffer translateTo16(int[] data) {
        StringBuffer buffer = new StringBuffer();
        for (int b : data) {
            String hex = String.format("%02X", b); // 将byte转换为16进制字符串
            buffer.append(hex);
            buffer.append("  ");
        }
        return buffer;
    }
}
