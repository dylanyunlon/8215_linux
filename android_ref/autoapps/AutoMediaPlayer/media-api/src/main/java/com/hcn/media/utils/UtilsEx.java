package com.hcn.media.utils;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.UserHandle;
import android.util.Log;

import androidx.annotation.NonNull;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;

/**
 * 杂项工具类
 * <p> 获取当前执行应用程序的应用组件对象;
 *
 * @author 65821
 */
public class UtilsEx {
    private static final String TAG = UtilsEx.class.getSimpleName();

    /** 禁止被实例化 **/
    private UtilsEx() {
        throw new IllegalArgumentException("u can't instantiate me...");
    }

    /** 当前应用实例 **/
    private static Application sApp = null;

    /**
     * 从反射获取 Application 组件
     * @return  the Application object
     */
    public static Application getApplication() {
        // 只需要反射一次
        if (sApp != null) {
            return sApp;
        }

        try {
            @SuppressLint("PrivateApi")
            Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
            Object thread = getActivityThread();
            Object app = activityThreadClass.getMethod("getApplication").invoke(thread);
            if (app == null) {
                return null;
            }

            sApp = (Application) app;
            return sApp;
        } catch (InvocationTargetException
                | IllegalAccessException
                | ClassNotFoundException
                | NoSuchMethodException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 获取当前 ActivityThread 对象
     * @return ActivityThread 对象
     */
    private static Object getActivityThread() {
        Object activityThread = currentActivityThreadField();
        if (activityThread != null) {
            return activityThread;
        }
        return currentActivityThreadMethod();
    }

    /**
     * 反射当前 ActivityThread 对象
     * <p> private static volatile ActivityThread sCurrentActivityThread;
     *
     * @return ActivityThread 对象
     */
    private static Object currentActivityThreadField() {
        try {
            @SuppressLint("PrivateApi")
            Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
            @SuppressLint("DiscouragedPrivateApi")
            Field sCurrentActivityThreadField =
                    activityThreadClass.getDeclaredField("sCurrentActivityThread");
            sCurrentActivityThreadField.setAccessible(true);
            return sCurrentActivityThreadField.get(null);
        } catch (Exception e) {
            Log.e(TAG, "currentActivityThreadField: " + e.getMessage());
            return null;
        }
    }

    /**
     * 反射当前 ActivityThread 对象对外方法
     * <p> public static ActivityThread currentActivityThread()
     *
     * @return ActivityThread 对象
     */
    private static Object currentActivityThreadMethod() {
        try {
            @SuppressLint("PrivateApi")
            Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
            return activityThreadClass.getMethod("currentActivityThread").invoke(null);
        } catch (Exception e) {
            Log.e(TAG, "currentActivityThreadMethod: " + e.getMessage());
            return null;
        }
    }


    /**
     * 判断目标类名服务释放在运行中
     *
     * @param className 目标服务的类名
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isServiceRunning(@NonNull final String className) {
        try {
            Application app = Objects.requireNonNull(getApplication());
            ActivityManager am = (ActivityManager) app.getSystemService(Context.ACTIVITY_SERVICE);
            List<ActivityManager.RunningServiceInfo> info = am.getRunningServices(0x7FFFFFFF);
            if (info == null || info.size() == 0) {
                return false;
            }
            for (ActivityManager.RunningServiceInfo aInfo : info) {
                if (className.equals(aInfo.service.getClassName())) {
                    return true;
                }
            }

            return false;
        } catch (Exception ignore) {
            return false;
        }
    }

    /**
     * 绑定服务
     * <p> 反射 @UnsupportedAppUsage 修饰的方法；
     *
     * @param context 上下文
     * @param service 意图
     * @param conn 连接器
     * @param flags bind 标记
     * @param user 用户句柄
     * @return 执行结果 <code>true</code>
     */
    public static Boolean bindServiceAsUser(@NonNull Context context,
                                            Intent service,
                                            ServiceConnection conn,
                                            int flags,
                                            UserHandle user) {
        Boolean result = false;
        try {
            @SuppressLint("PrivateApi")
            Class<?> contextWrapper = Class.forName("android.content.ContextWrapper");
            Method bindServiceAsUser = contextWrapper.getMethod("bindServiceAsUser",
                    Intent.class, ServiceConnection.class, Integer.TYPE, UserHandle.class);
            Object[] params = new Object[]{service, conn, flags, user};
            result = (Boolean) (bindServiceAsUser.invoke(context, params));
        } catch (Exception e) {
            Log.d(TAG, "bindServiceAsUser: " + e.toString());
        }
        return result;
    }

    /**
     * 设置系统属性（通用）
     *
     * @param property 系统属性
     * @param value 要设置的存储值
     */
    public static void setSystemProperty(String property, String value) {
        try {
            @SuppressLint("PrivateApi")
            Class<?> systemProperties = Class.forName("android.os.SystemProperties");
            Method set = systemProperties.getMethod("set", String.class, String.class);
            Object[] params = new Object[]{property, value};
            set.invoke(systemProperties, params);
        } catch (Exception e) {
            LogUtils.w("setSystemProperty: " + e.toString());
        }
    }
}
