package com.hcn.auto.utils;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public final class HUtilsEx {
    private static final String TAG = "HUtilsEx";
    @SuppressLint({"StaticFieldLeak"})
    private static Application sApp;

    private HUtilsEx() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    public static void init(Application app) {
        if (app == null) {
            Log.e("HUtils", "app is null.");
        } else {
            sApp = app;
        }
    }

    public static Application getApp() {
        if (sApp != null) {
            return sApp;
        } else {
            init(getApplicationByReflect());
            if (sApp == null) {
                throw new NullPointerException("reflect failed.");
            } else {
                return sApp;
            }
        }
    }

    private static Application getApplicationByReflect() {
        try {
            Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
            Object thread = getActivityThread();
            Object app = activityThreadClass.getMethod("getApplication").invoke(thread);
            return app == null ? null : (Application)app;
        } catch (IllegalAccessException | ClassNotFoundException | NoSuchMethodException |
                 InvocationTargetException var3) {
            var3.printStackTrace();
            return null;
        }
    }

    private static Object getActivityThread() {
        Object activityThread = getActivityThreadInActivityThreadStaticField();
        return activityThread != null ? activityThread : getActivityThreadInActivityThreadStaticMethod();
    }

    private static Object getActivityThreadInActivityThreadStaticField() {
        try {
            Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
            Field sCurrentActivityThreadField = activityThreadClass.getDeclaredField("sCurrentActivityThread");
            sCurrentActivityThreadField.setAccessible(true);
            return sCurrentActivityThreadField.get((Object)null);
        } catch (Exception var2) {
            Log.e("UtilsActivityLifecycle", "getActivityThreadInActivityThreadStaticField: " + var2.getMessage());
            return null;
        }
    }

    private static Object getActivityThreadInActivityThreadStaticMethod() {
        try {
            Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
            return activityThreadClass.getMethod("currentActivityThread").invoke((Object)null);
        } catch (Exception var1) {
            Log.e("UtilsActivityLifecycle", "getActivityThreadInActivityThreadStaticMethod: " + var1.getMessage());
            return null;
        }
    }

    public static boolean isAppDebug() {
        return isAppDebug(getApp());
    }

    public static boolean isAppDebug(Context context) {
        if (Objects.isNull(context)) {
            return false;
        } else {
            ApplicationInfo ai = context.getApplicationInfo();
            return ai != null && (ai.flags & 2) != 0 || !"user".equals(Build.TYPE);
        }
    }

    public static void sleep(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException var3) {
            var3.printStackTrace();
        }

    }

    public static boolean reverseEquals(String obj1, String obj2) {
        if (!TextUtils.isEmpty(obj1) && !TextUtils.isEmpty(obj2)) {
            if (obj2.equals(obj1)) {
                return true;
            } else {
                int n = obj1.length();
                if (n == obj2.length()) {
                    for(int i = n - 1; n-- != 0; --i) {
                        if (obj1.charAt(i) != obj2.charAt(i)) {
                            return false;
                        }
                    }

                    return true;
                } else {
                    return false;
                }
            }
        } else {
            return false;
        }
    }

    public static boolean existInstalledPackage(Context context, String packageName) {
        if (Objects.isNull(context)) {
            return false;
        } else {
            PackageManager pm = context.getPackageManager();
            List<PackageInfo> packages = pm.getInstalledPackages(0);
            Iterator var4 = packages.iterator();

            PackageInfo p;
            do {
                if (!var4.hasNext()) {
                    return false;
                }

                p = (PackageInfo)var4.next();
            } while(TextUtils.isEmpty(p.packageName) || !p.packageName.equals(packageName));

            return true;
        }
    }

    public static Drawable getAppIcon() {
        return getAppIcon(getApp().getPackageName());
    }

    public static Drawable getAppIcon(String packageName) {
        if (TextUtils.isEmpty(packageName)) {
            return null;
        } else {
            try {
                PackageManager pm = getApp().getPackageManager();
                PackageInfo pi = pm.getPackageInfo(packageName, 0);
                return pi == null ? null : pi.applicationInfo.loadIcon(pm);
            } catch (PackageManager.NameNotFoundException var3) {
                var3.printStackTrace();
                return null;
            }
        }
    }

    public static int getAppIconId() {
        return getAppIconId(getApp().getPackageName());
    }

    public static int getAppIconId(String packageName) {
        if (TextUtils.isEmpty(packageName)) {
            return 0;
        } else {
            try {
                PackageManager pm = getApp().getPackageManager();
                PackageInfo pi = pm.getPackageInfo(packageName, 0);
                return pi == null ? 0 : pi.applicationInfo.icon;
            } catch (PackageManager.NameNotFoundException var3) {
                var3.printStackTrace();
                return 0;
            }
        }
    }

    public static String getAppPackageName() {
        return getApp().getPackageName();
    }

    public static String getAppName() {
        return getAppName(getApp().getPackageName());
    }

    public static String getAppName(String packageName) {
        if (TextUtils.isEmpty(packageName)) {
            return "";
        } else {
            try {
                PackageManager pm = getApp().getPackageManager();
                PackageInfo pi = pm.getPackageInfo(packageName, 0);
                return pi == null ? null : pi.applicationInfo.loadLabel(pm).toString();
            } catch (PackageManager.NameNotFoundException var3) {
                var3.printStackTrace();
                return "";
            }
        }
    }

    public static String getAppPath() {
        return getAppPath(getApp().getPackageName());
    }

    public static String getAppPath(String packageName) {
        if (TextUtils.isEmpty(packageName)) {
            return "";
        } else {
            try {
                PackageManager pm = getApp().getPackageManager();
                PackageInfo pi = pm.getPackageInfo(packageName, 0);
                return pi == null ? null : pi.applicationInfo.sourceDir;
            } catch (PackageManager.NameNotFoundException var3) {
                var3.printStackTrace();
                return "";
            }
        }
    }

    public static String getAppVersionName() {
        return getAppVersionName(getApp().getPackageName());
    }

    public static String getAppVersionName(String packageName) {
        if (TextUtils.isEmpty(packageName)) {
            return "";
        } else {
            try {
                PackageManager pm = getApp().getPackageManager();
                PackageInfo pi = pm.getPackageInfo(packageName, 0);
                return pi == null ? null : pi.versionName;
            } catch (PackageManager.NameNotFoundException var3) {
                var3.printStackTrace();
                return "";
            }
        }
    }

    public static int getAppVersionCode() {
        return getAppVersionCode(getApp().getPackageName());
    }

    public static int getAppVersionCode(String packageName) {
        if (TextUtils.isEmpty(packageName)) {
            return -1;
        } else {
            try {
                PackageManager pm = getApp().getPackageManager();
                PackageInfo pi = pm.getPackageInfo(packageName, 0);
                return pi == null ? -1 : pi.versionCode;
            } catch (PackageManager.NameNotFoundException var3) {
                var3.printStackTrace();
                return -1;
            }
        }
    }

    public static <T> Task<T> doAsync(Task<T> task) {
        HThreadUtils.getCachedPool().execute(task);
        return task;
    }

    public static final class FileHead {
        private final String mName;
        private final LinkedHashMap<String, String> mFirst = new LinkedHashMap();
        private final LinkedHashMap<String, String> mLast = new LinkedHashMap();

        public FileHead(String name) {
            this.mName = name;
        }

        public void addFirst(String key, String value) {
            this.append2Host(this.mFirst, key, value);
        }

        public void append(Map<String, String> extra) {
            this.append2Host(this.mLast, extra);
        }

        public void append(String key, String value) {
            this.append2Host(this.mLast, key, value);
        }

        private void append2Host(Map<String, String> host, Map<String, String> extra) {
            if (extra != null && !extra.isEmpty()) {
                Iterator var3 = extra.entrySet().iterator();

                while(var3.hasNext()) {
                    Map.Entry<String, String> entry = (Map.Entry)var3.next();
                    this.append2Host(host, (String)entry.getKey(), (String)entry.getValue());
                }

            }
        }

        private void append2Host(Map<String, String> host, String key, String value) {
            if (!TextUtils.isEmpty(key) && !TextUtils.isEmpty(value)) {
                int delta = 19 - key.length();
                if (delta > 0) {
                    key = key + "                   ".substring(0, delta);
                }

                host.put(key, value);
            }
        }

        public String getAppended() {
            StringBuilder sb = new StringBuilder();
            Iterator var2 = this.mLast.entrySet().iterator();

            while(var2.hasNext()) {
                Map.Entry<String, String> entry = (Map.Entry)var2.next();
                sb.append((String)entry.getKey()).append(": ").append((String)entry.getValue()).append("\n");
            }

            return sb.toString();
        }

        @NonNull
        public String toString() {
            StringBuilder sb = new StringBuilder();
            String border = "************* " + this.mName + " Head ****************\n";
            sb.append(border);
            Iterator var3 = this.mFirst.entrySet().iterator();

            while(var3.hasNext()) {
                Map.Entry<String, String> entry = (Map.Entry)var3.next();
                sb.append((String)entry.getKey()).append(": ").append((String)entry.getValue()).append("\n");
            }

            sb.append("Device Manufacturer: ").append(Build.MANUFACTURER).append("\n");
            sb.append("Device Model       : ").append(Build.MODEL).append("\n");
            sb.append("Android Version    : ").append(Build.VERSION.RELEASE).append("\n");
            sb.append("Android SDK        : ").append(Build.VERSION.SDK_INT).append("\n");
            sb.append("App VersionName    : ").append(getAppVersionName()).append("\n");
            sb.append("App VersionCode    : ").append(getAppVersionCode()).append("\n");
            sb.append(this.getAppended());
            return sb.append(border).append("\n").toString();
        }
    }

    public interface OnAppStatusChangedListener {
        void onForeground(Activity var1);

        void onBackground(Activity var1);
    }

    public abstract static class Task<Result> extends HThreadUtils.SimpleTask<Result> {
        private Consumer<Result> mConsumer;

        public Task(Consumer<Result> consumer) {
            this.mConsumer = consumer;
        }

        public void onSuccess(Result result) {
            if (this.mConsumer != null) {
                this.mConsumer.accept(result);
            }

        }
    }

    public interface Func1<Ret, Par> {
        Ret call(Par var1);
    }

    public interface Supplier<T> {
        T get();
    }

    public interface Consumer<T> {
        void accept(T var1);
    }
}

