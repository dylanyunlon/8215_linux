package com.hcn.eq.contant;

import android.content.Context;

import java.lang.reflect.Field;

public class ScreenSpec {

    public static int mScreenWidth = 1024;
    public static int mScreenHeight = 600;
    public static float mScreenDensity = 1.0F;
    public static int mStatusBarHeight = 0;

    public static int getStatusBarHeight(Context context) {
        Class<?> c = null;
        Object obj = null;
        Field field = null;
        int x = 0, sbar = 0;

        try {
            c = Class.forName("com.android.internal.R$dimen");
            obj = c.newInstance();
            field = c.getField("status_bar_height");
            x = Integer.parseInt(field.get(obj).toString());
            sbar = context.getResources().getDimensionPixelSize(x);
        } catch (Exception e1) {
            e1.printStackTrace();
        }

        return sbar;
    }
}