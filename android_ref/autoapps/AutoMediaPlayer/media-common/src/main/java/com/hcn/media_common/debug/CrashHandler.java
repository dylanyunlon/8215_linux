package com.hcn.media_common.debug;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.os.Environment;
import android.os.Looper;

import androidx.annotation.NonNull;

import com.hcn.common.utils.HCrashUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.lang.Thread.UncaughtExceptionHandler;
import java.lang.reflect.Field;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * 异常捕获工具类
 * <p> 历史工具代码，已经淘汰；
 *
 * @author 86158
 * @deprecated 过时了，建议使用 {@link HCrashUtils} 工具；
 */
public class CrashHandler implements UncaughtExceptionHandler {

    public static final String TAG = "CrashHandler";
    public static final String PATH_SD = Environment.getExternalStorageDirectory().getPath();

    @SuppressLint("StaticFieldLeak")
    private static CrashHandler instance;

    private Context mContext;
    private Thread.UncaughtExceptionHandler mDefaultHandler;
    private final Map<String, String> infos = new HashMap<String, String>();

    @SuppressLint("SimpleDateFormat")
    private final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss");

    private CrashHandler() {
    }

    public static CrashHandler getInstance() {
        if (instance == null) {
            instance = new CrashHandler();
        }
        return instance;
    }

    public void init(Context context) {
        LogUtil.d(TAG, PATH_SD);
        mContext = context;
        mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler(this);
    }

    @Override
    public void uncaughtException(@NonNull Thread thread, @NonNull Throwable ex) {
        LogUtil.e(TAG, "___________ erre start");
        if (!handleException(ex) && mDefaultHandler != null) {
            mDefaultHandler.uncaughtException(thread, ex);
        } else {
            try {
                Thread.sleep(5000);
            } catch (InterruptedException e) {
                LogUtil.e(TAG, "error : " + e);
            }

            android.os.Process.killProcess(android.os.Process.myPid());
            System.exit(1);
        }
    }

    public String getApplicationName() {
        PackageManager packageManager = null;
        ApplicationInfo applicationInfo = null;
        try {
            packageManager = mContext.getApplicationContext().getPackageManager();
            applicationInfo = packageManager.getApplicationInfo(mContext.getPackageName(), 0);
        } catch (PackageManager.NameNotFoundException ignored) {
        }
        return (String) packageManager.getApplicationLabel(applicationInfo);
    }

    private boolean handleException(Throwable ex) {
        if (ex == null) {
            return false;
        }

        collectDeviceInfo(mContext);
        new Thread("ExceptionHandler") {
            @Override
            public void run() {
                Looper.prepare();
                Looper.loop();
            }
        }.start();

        saveCatchInfo2File(ex, PATH_SD);
        return true;
    }

    public void collectDeviceInfo(Context ctx) {
        try {
            PackageManager pm = ctx.getPackageManager();
            PackageInfo pi = pm.getPackageInfo(ctx.getPackageName(), PackageManager.GET_ACTIVITIES);
            if (pi != null) {
                String versionName = pi.versionName == null ? "null" : pi.versionName;
                String versionCode = pi.versionCode + "";
                infos.put("versionName", versionName);
                infos.put("versionCode", versionCode);
            }
        } catch (NameNotFoundException e) {
            LogUtil.e(TAG, "an error occured when collect package info" + e);
        }
        Field[] fields = Build.class.getDeclaredFields();
        for (Field field : fields) {
            try {
                field.setAccessible(true);
                infos.put(field.getName(), Objects.requireNonNull(field.get(null)).toString());
                LogUtil.d(TAG, field.getName() + " : " + field.get(null));
            } catch (Exception e) {
                LogUtil.e(TAG, "an error occured when collect crash info" + e);
            }
        }
    }

    private void saveCatchInfo2File(Throwable ex, String path) {
        StringBuilder sb = new StringBuilder();
        for (Map.Entry<String, String> entry : infos.entrySet()) {
            String key = entry.getKey();
            String value = entry.getValue();
            sb.append(key).append("=").append(value).append("\n");
        }

        Writer writer = new StringWriter();
        PrintWriter printWriter = new PrintWriter(writer);
        ex.printStackTrace(printWriter);
        Throwable cause = ex.getCause();
        while (cause != null) {
            cause.printStackTrace(printWriter);
            cause = cause.getCause();
        }
        printWriter.close();
        String result = writer.toString();
        sb.append(result);
        try {
            long timestamp = System.currentTimeMillis();
            String time = formatter.format(new Date());
            String fileName = "crash-" + time + "-" + timestamp + ".txt";
            if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
                File dir = new File(path);
                if (!dir.exists()) {
                    boolean ignored = dir.mkdirs();
                }
                FileOutputStream fos = new FileOutputStream(path + File.separator + fileName);
                fos.write(sb.toString().getBytes());
                LogUtil.e(TAG, "___________wirt erre info");
                fos.close();
            }
        } catch (Exception e) {
            LogUtil.e(TAG, "an error occured while writing file..." + e);
        }
    }
}
