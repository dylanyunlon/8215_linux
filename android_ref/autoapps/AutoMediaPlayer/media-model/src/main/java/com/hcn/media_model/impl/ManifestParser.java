package com.hcn.media_model.impl;

import android.app.Application;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import com.hcn.media_model.base.MediaModule;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

/**
 * 解析 AndroidManifest 文件中的 Instrumentation 引用
 * @author 65821
 */
public class ManifestParser {
    private static final String TAG = "ManifestParser";
    private static final String MODULE_VALUE = "MediaModule";

    private final Application mApplication;

    public ManifestParser(Application application) {
        this.mApplication = application;
    }

    public List<MediaModule> parse() {
        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "Loading Media modules");
        }
        List<MediaModule> modules = new ArrayList<>();
        try {
            ApplicationInfo appInfo;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                appInfo = mApplication.getPackageManager()
                        .getApplicationInfo(mApplication.getPackageName(),
                                PackageManager.ApplicationInfoFlags.of(PackageManager.GET_META_DATA));
            } else {
                appInfo = mApplication.getPackageManager().getApplicationInfo(
                        mApplication.getPackageName(), PackageManager.GET_META_DATA);
            }

            if (appInfo.metaData == null) {
                if (Log.isLoggable(TAG, Log.DEBUG)) {
                    Log.d(TAG, "Got null app info metadata");
                }
                return modules;
            }

            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                Log.v(TAG, "Got app info metadata: " + appInfo.metaData);
            }

            for (String key : appInfo.metaData.keySet()) {
                if (MODULE_VALUE.equals(appInfo.metaData.getString(key))) {
                    modules.add(parseModule(key, mApplication));
                    if (Log.isLoggable(TAG, Log.DEBUG)) {
                        Log.d(TAG, "Loaded Media module: " + key);
                    }
                }
            }
        } catch (PackageManager.NameNotFoundException e) {
            throw new RuntimeException("Unable to find metadata to parse MediaModules", e);
        }

        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "Finished loading Media modules");
        }

        return modules;
    }

    private static MediaModule parseModule(String className, Application application) {
        Class<?> clazz;
        try {
            clazz = Class.forName(className);
        } catch (ClassNotFoundException e) {
            throw new IllegalArgumentException("Unable to find MediaModule implementation", e);
        }

        Object module = null;
        try {
            module = clazz.getConstructor(Application.class).newInstance(application);
            // These can't be combined until API minimum is 19.
        } catch (InstantiationException
                | IllegalAccessException
                | NoSuchMethodException
                | InvocationTargetException e) {
            throwInstantiateMediaModuleException(clazz, e);
        }

        if (!(module instanceof MediaModule)) {
            throw new RuntimeException("Expected instanceof MediaModule, but found: " + module);
        }

        return (MediaModule) module;
    }

    private static void throwInstantiateMediaModuleException(Class<?> clazz, Exception e) {
        throw new RuntimeException("Unable to instantiate MediaModule implementation for " + clazz, e);
    }
}
