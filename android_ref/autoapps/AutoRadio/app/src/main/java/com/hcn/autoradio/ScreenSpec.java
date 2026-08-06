package com.hcn.autoradio;

import android.content.Context;
import android.content.res.Configuration;

public class ScreenSpec {
    public static int mFullScreenWidth = 1024;
    public static int mFullScreenHeight = 600;
    public static int mOrientation = Configuration.ORIENTATION_LANDSCAPE;

    public static int mScreenWidth = 1024;
    public static int mScreenHeight = 600;
    public static int mStatusBarHeight = -1;
    public static float mScreenDensity = 1.0F;

    //Divide Screen
    private static short mScreenStatus = 0;
    public static final short FULL_SCREEN = 0;
    public static final short TWO_THIRD_SCREEN = 1;
    public static final short HALF_SCREEN = 2;
    public static final short ONE_THIRD_SCREEN = 3;


    public static int getStatusBarHeight(Context context) {
        if (mStatusBarHeight == -1) {
            int resourceId = context.getResources().getIdentifier("status_bar_height", "dimen",
                    "android");
            mStatusBarHeight = context.getResources().getDimensionPixelSize(resourceId);
        }
        return mStatusBarHeight;
    }

    /**
     * 0.6 < ratio < 0.7  2/3屏 0.45 < ratio < 0.55  1/2屏 0.3 < ratio < 0.4  1/3屏 其他情况为全屏
     *
     * @param config
     */
    public static void setScreenStatus(Configuration config) {
        mScreenWidth = config.screenWidthDp;
        mScreenHeight = config.screenHeightDp;
        float ratio;
        if (mOrientation == Configuration.ORIENTATION_LANDSCAPE) {
            ratio = (float) mScreenWidth / mFullScreenWidth;
        } else {
            ratio = (float) mScreenHeight / mFullScreenHeight;
        }
        if (ratio > 0.6 && ratio < 0.7) {
            mScreenStatus = TWO_THIRD_SCREEN;
        } else if (ratio < 0.55 && ratio > 0.45) {
            mScreenStatus = HALF_SCREEN;
        } else if (ratio < 0.4 && ratio > 0.3) {
            mScreenStatus = ONE_THIRD_SCREEN;
        } else {
            mScreenStatus = FULL_SCREEN;
        }
    }

    public static short getScreenStatus() {
        return mScreenStatus;
    }
}
