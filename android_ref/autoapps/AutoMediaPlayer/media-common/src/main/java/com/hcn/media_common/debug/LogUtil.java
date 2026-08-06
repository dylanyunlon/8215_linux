package com.hcn.media_common.debug;

import android.os.Build;
import android.util.Log;

import com.hcn.common.utils.HUtilsEx;

/**
 * @author 86158
 * @deprecated 过时的接口，建议用 Logger 替换；
 */
public class LogUtil {
    public static final String TAG = "AutoMediaPlayer";

    public static final boolean DEBUG = true;
    public static final boolean DEBUG_V = !"user".equals(Build.TYPE);
    public static final boolean DEBUG_D = true;
    public static final boolean DEBUG_I = true;
    public static final boolean DEBUG_W = false;
    public static final boolean DEBUG_E = true;
    public static final boolean LOW_DEBUG = false;
    public static final boolean VITAMIO_I = false;
    public static final boolean VITAMIO_D = true;

    public static void e(String TAG, String MSG) {
        if (DEBUG && DEBUG_E) {
            Log.e(TAG, MSG);
        }
    }

    public static void i(String TAG, String MSG) {
        if (DEBUG && DEBUG_I) {
            Log.i(TAG, MSG);
        }
    }

    public static void d(String TAG, String MSG) {
        if (DEBUG && DEBUG_D) {
            Log.d(TAG, MSG);
        }
    }

    public static void w(String TAG, String MSG) {
        if (DEBUG && DEBUG_W) {
            Log.w(TAG, MSG);
        }
    }

    public static void v(String TAG, String MSG) {
        if (DEBUG && DEBUG_V) {
            Log.v(TAG, MSG);
        }
    }

    public static void low_i(String TAG, String MSG) {
        if (DEBUG && LOW_DEBUG) {
            Log.i(TAG, MSG);
        }
    }

    public static void vitamio_d(String TAG, String MSG) {
        if (DEBUG && VITAMIO_D) {
            Log.i(TAG, MSG);
        }
    }

    public static void vitamio_i(String TAG, String MSG) {
        if (DEBUG && VITAMIO_I) {
            Log.i(TAG, MSG);
        }
    }
}
