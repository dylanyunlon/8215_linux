package com.hcn.media_dummy.utils;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Insets;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.ContextThemeWrapper;
import android.view.Surface;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.WindowMetrics;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.TintContextWrapper;
import androidx.fragment.app.FragmentActivity;

import java.util.Formatter;
import java.util.Locale;
import java.util.Objects;

/**
 * 通用工具
 * @author 65821
 */
public class CommonUtil {
    public static final int INT_1024 = 1024;

    /**
     * 下载速度文本
     *
     * @param speed KB/s
     * @return 网速显示文本
     */
    public static String getTextSpeed(long speed) {
        String text = "";
        if (speed >= 0 && speed < INT_1024) {
            text = speed + " KB/s";
        } else if (speed >= INT_1024
                && speed < (INT_1024 * INT_1024)) {
            text = Long.toString(speed / 1024) + " KB/s";
        } else if (speed >= (INT_1024 * INT_1024)
                && speed < (INT_1024 * INT_1024 * INT_1024)) {
            text = Long.toString(speed / (1024 * 1024)) + " MB/s";
        }
        return text;
    }

    /**
     * wifi 是连接的
     *
     * @param context 上下文
     * @return 是连接/不是连接
     */
    public static boolean isWifiConnected(Context context) {
        ConnectivityManager cm = (ConnectivityManager)
                context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (Objects.isNull(cm)) {
            return false;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            Network networks = cm.getActiveNetwork();
            NetworkCapabilities networkCapabilities = cm.getNetworkCapabilities(networks);
            if (networkCapabilities != null) {
                return networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI);
            }
            return false;
        }

        NetworkInfo wifiNetworkInfo = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if (wifiNetworkInfo == null) {
            return false;
        }
        return wifiNetworkInfo.isConnected();
    }

    /**
     * 获取 AppCompatActivity 的上下文
     * <p> 一级一级的向上找上下文，直到找到 AppCompatActivity;
     *
     * @param context 上下文
     * @return {@link AppCompatActivity}
     */
    public static AppCompatActivity getAppCompActivity(Context context) {
        if (context == null) {
            return null;
        }

        if (context instanceof AppCompatActivity) {
            return (AppCompatActivity) context;
        } else if (context instanceof ContextThemeWrapper) {
            return getAppCompActivity(((ContextThemeWrapper) context).getBaseContext());
        }
        return null;
    }

    /**
     * 获取 Activity 的上下文
     * <p> 一级一级的向上找上下文，直到找到 Activity;
     *
     * @param context 上下文
     * @return {@link Activity}
     */
    public static Activity getActivityContext(Context context) {
        if (context == null) {
            return null;
        } else if (context instanceof Activity) {
            return (Activity) context;
        } else if (context instanceof TintContextWrapper) {
            return scanForActivity(((TintContextWrapper) context).getBaseContext());
        } else if (context instanceof ContextWrapper) {
            return scanForActivity(((ContextWrapper) context).getBaseContext());
        }

        return null;
    }

    /**
     * Get activity from context object
     *
     * @param context something
     * @return object of Activity or null if it is not Activity
     */
    public static Activity scanForActivity(Context context) {
        if (context == null) {
            return null;
        }

        if (context instanceof Activity) {
            return (Activity) context;
        } else if (context instanceof TintContextWrapper) {
            return scanForActivity(((TintContextWrapper) context).getBaseContext());
        } else if (context instanceof ContextWrapper) {
            return scanForActivity(((ContextWrapper) context).getBaseContext());
        }

        return null;
    }

    /**
     * dip 转为 px
     *
     * @param context 上下文
     * @param dipValue dip 值
     * @return px 值
     */
    public static int dip2px(Context context, float dipValue) {
        float fontScale = context.getResources().getDisplayMetrics().density;
        return (int) (dipValue * fontScale + 0.5f);
    }

    /**
     * 根据手机的分辨率从 px 的单位转成为 dp
     *
     * @param context 上下文
     * @param pxValue px 值
     * @return dip 值
     */
    public static int px2dip(Context context, float pxValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (pxValue / scale + 0.5f);
    }

    /**
     * 获取屏幕的宽度 px
     *
     * @param context 上下文
     * @return 屏幕宽，unit: px
     */
    public static int getScreenWidth(Context context) {
        WindowManager windowManager = (WindowManager)
                context.getSystemService(Context.WINDOW_SERVICE);
        if (Objects.isNull(windowManager)) {
            return -1;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowMetrics windowMetrics = windowManager.getCurrentWindowMetrics();
            Insets insets = windowMetrics.getWindowInsets()
                    .getInsetsIgnoringVisibility(WindowInsets.Type.systemBars());
            return windowMetrics.getBounds().width() - insets.left - insets.right;
        }

        DisplayMetrics outMetrics = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(outMetrics);
        return outMetrics.widthPixels;
    }

    /**
     * 获取屏幕的高度 px
     *
     * @param context 上下文
     * @return 屏幕高，unit: px
     */
    public static int getScreenHeight(Context context) {
        WindowManager windowManager = (WindowManager)
                context.getSystemService(Context.WINDOW_SERVICE);
        if (Objects.isNull(windowManager)) {
            return -1;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowMetrics windowMetrics = windowManager.getCurrentWindowMetrics();
            Insets insets = windowMetrics.getWindowInsets()
                    .getInsetsIgnoringVisibility(WindowInsets.Type.systemBars());
            return windowMetrics.getBounds().height() - insets.top - insets.bottom;
        }

        DisplayMetrics outMetrics = new DisplayMetrics();
        windowManager.getDefaultDisplay().getMetrics(outMetrics);
        return outMetrics.heightPixels;
    }

    /**
     * 是屏幕横向
     * <p> 默认认为设备都是竖屏（手机）
     *
     * @param context 上下文
     * @return 是/否
     */
    public static boolean isScreenLandscape(Activity context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return context.getDisplay().getRotation() == Surface.ROTATION_90 ||
                    context.getDisplay().getRotation() == Surface.ROTATION_270;
        }

        WindowManager wm = context.getWindowManager();
        return wm.getDefaultDisplay().getRotation() == Surface.ROTATION_90 ||
                wm.getDefaultDisplay().getRotation() == Surface.ROTATION_270;
    }

    /**
     * 把时间转换成字符串
     *
     * @param timeMs 时间
     * @return 字符串（hh:mm:ss/mm:ss）
     */
    public static String stringForTime(long timeMs) {
        long totalSeconds = timeMs / 1000;
        long seconds = totalSeconds % 60;
        long minutes = (totalSeconds / 60) % 60;
        long hours = totalSeconds / 3600;
        StringBuilder stringBuilder = new StringBuilder();
        Formatter mFormatter = new Formatter(stringBuilder, Locale.getDefault());
        if (hours > 0) {
            return mFormatter.format(
                    "%d:%02d:%02d", hours, minutes, seconds).toString();
        } else {
            return mFormatter.format(
                    "%02d:%02d", minutes, seconds).toString();
        }
    }

    /**
     * Get Activity from context
     *
     * @param context 上下文
     * @return AppCompatActivity if it's not null
     */
    public static Activity getActivityNestWrapper(Context context) {
        return getActivityContext(context);
    }

    /**
     * 隐藏导航栏
     *
     * @param context 上下文
     */
    @SuppressLint("ObsoleteSdkInt")
    public static void hideNavKey(Context context) {
        if (CommonUtil.getActivityNestWrapper(context) == null) {
            return;
        }

        if (Build.VERSION.SDK_INT >= 29) {
            // 设置屏幕始终在前面，不然点击鼠标，重新出现虚拟按键
            CommonUtil.getActivityNestWrapper(context).getWindow()
                    .getDecorView().setSystemUiVisibility(
                            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                    | View.SYSTEM_UI_FLAG_IMMERSIVE);

        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            // 设置屏幕始终在前面，不然点击鼠标，重新出现虚拟按键
            CommonUtil.getActivityNestWrapper(context).getWindow()
                    .getDecorView().setSystemUiVisibility(
                            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                    | View.SYSTEM_UI_FLAG_IMMERSIVE);
        } else {
            CommonUtil.getActivityNestWrapper(context).getWindow()
                    .getDecorView().setSystemUiVisibility(
                            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
            );
        }
    }

    /**
     * 显示导航栏
     *
     * @param context 上下文
     * @param systemUiVisibility SystemUI 可见性标记
     */
    public static void showNavKey(Context context, int systemUiVisibility) {
        CommonUtil.getActivityNestWrapper(context).getWindow()
                .getDecorView().setSystemUiVisibility(systemUiVisibility);
    }

    @SuppressLint("RestrictedApi")
    public static void hideSupportActionBar(Context context,
                                            boolean actionBar,
                                            boolean statusBar) {
        if (actionBar) {
            AppCompatActivity appCompatActivity = CommonUtil.getAppCompActivity(context);
            if (appCompatActivity != null) {
                ActionBar ab = appCompatActivity.getSupportActionBar();
                if (ab != null) {
                    ab.setShowHideAnimationEnabled(false);
                    ab.hide();
                }
            }
        }

        if (statusBar) {
            if (context instanceof FragmentActivity) {
                FragmentActivity fragmentActivity = (FragmentActivity) context;
                fragmentActivity.getWindow().setFlags(
                        WindowManager.LayoutParams.FLAG_FULLSCREEN,
                        WindowManager.LayoutParams.FLAG_FULLSCREEN);
            } else if (context instanceof Activity) {
                Activity activity = (Activity) context;
                activity.getWindow().setFlags(
                        WindowManager.LayoutParams.FLAG_FULLSCREEN,
                        WindowManager.LayoutParams.FLAG_FULLSCREEN);
            } else {
                CommonUtil.getActivityNestWrapper(context)
                        .getWindow().setFlags(
                                WindowManager.LayoutParams.FLAG_FULLSCREEN,
                                WindowManager.LayoutParams.FLAG_FULLSCREEN);
            }
        }
    }

    @SuppressLint("RestrictedApi")
    public static void showSupportActionBar(Context context,
                                            boolean actionBar,
                                            boolean statusBar) {
        if (actionBar) {
            AppCompatActivity appCompatActivity = CommonUtil.getAppCompActivity(context);
            if (appCompatActivity != null) {
                ActionBar ab = appCompatActivity.getSupportActionBar();
                if (ab != null) {
                    ab.setShowHideAnimationEnabled(false);
                    ab.show();
                }
            }
        }

        if (statusBar) {
            if (context instanceof FragmentActivity) {
                FragmentActivity fragmentActivity = (FragmentActivity) context;
                fragmentActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            } else if (context instanceof Activity) {
                Activity activity = (Activity) context;
                activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            } else {
                CommonUtil.getActivityNestWrapper(context)
                        .getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            }
        }
    }

    /**
     * 获取状态栏高度
     *
     * @param context 上下文
     * @return 状态栏高度
     */
    public static int getStatusBarHeight(Context context) {
        int result = 0;
        @SuppressLint("InternalInsetResource")
        int resourceId = context.getResources()
                .getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            result = context.getResources().getDimensionPixelSize(resourceId);
        }
        return result;
    }

    /**
     * 获取ActionBar高度
     *
     * @param activity activity
     * @return ActionBar高度
     */
    public static int getActionBarHeight(Activity activity) {
        TypedValue tv = new TypedValue();
        if (activity.getTheme().resolveAttribute(
                android.R.attr.actionBarSize, tv, true)) {
            return TypedValue.complexToDimensionPixelSize(
                    tv.data, activity.getResources().getDisplayMetrics());
        }
        return 0;
    }
}
