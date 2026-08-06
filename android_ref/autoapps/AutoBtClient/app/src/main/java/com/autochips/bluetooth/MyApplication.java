package com.autochips.bluetooth;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.os.Binder;
import android.os.Bundle;
import android.os.IBinder;
import android.provider.Settings;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.autochips.bluetooth.skin.SkinUtils;
import com.autochips.bluetooth.skin.ThemeUtilsEx;
import com.autochips.bluetooth.utils.WallpaperUtil;
import com.hcn.auto.theme.utils.ThemeUtils;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.LocalBluetoothHfpclientManager;

public class MyApplication extends Application {
    private static final String TAG = "MyApplication";
    private static MyApplication sInstance = null;

    public static MyApplication getInstance() {
        return sInstance;
    }

    public IBinder getBinder() {
        return mBinder;
    }

    private IBinder mBinder = null;

    private static final String SIDEBAR_STYLE = "sidebar_Style";
    private int activityAccount = 0;

    int sidebarLastStyle = 0;

    private void init(Context context) {
        if (null == mLocalBluetoothAdapterManager) {
            mLocalBluetoothAdapterManager = LocalBluetoothAdapterManager.getInstance();
            mLocalBluetoothAdapterManager.init(context);
        }
        if (null == mBluetoothHfpclientManager) {
            mBluetoothHfpclientManager = LocalBluetoothHfpclientManager.getInstance();
            mBluetoothHfpclientManager.init(context);
        }
        ThemeUtilsEx.init(this);

        // 初始化壁纸数据
        WallpaperUtil.getInstance(getApplicationContext()).initWallpaperData();
    }

    private LocalBluetoothAdapterManager mLocalBluetoothAdapterManager = null;
    private LocalBluetoothHfpclientManager mBluetoothHfpclientManager = null;

    public LocalBluetoothAdapterManager getAdapterManager() {
        return mLocalBluetoothAdapterManager;
    }

    public LocalBluetoothHfpclientManager getHfpclientManager() {
        return mBluetoothHfpclientManager;

    }

    @Override
    public void onCreate() {
        super.onCreate();
        SkinUtils.init(this);
        sInstance = this;
        mBinder = new Binder();
        init(this);
        CrashHandler catchHandler = CrashHandler.getInstance();
        catchHandler.init(this);
        registerActivityLifecycleCallbacks(activityLifecycleCallbacks);
    }

    ActivityLifecycleCallbacks activityLifecycleCallbacks = new ActivityLifecycleCallbacks() {


        @Override
        public void onActivityCreated(@NonNull Activity activity, @Nullable Bundle savedInstanceState) {

        }

        @Override
        public void onActivityStarted(Activity activity) {
            if (activityAccount == 0) {
                //进入前台
                sidleStatusColor(true);
            }
            activityAccount++;
        }

        @Override
        public void onActivityResumed(Activity activity) {
        }

        @Override
        public void onActivityPaused(Activity activity) {
        }

        @Override
        public void onActivityStopped(Activity activity) {
            activityAccount--;
            if (activityAccount == 0) {
                //不在前台
                sidleStatusColor(false);
            }
        }

        @Override
        public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
        }

        @Override
        public void onActivityDestroyed(Activity activity) {
        }

    };


    /**
     * 根据应用的显隐，为状态栏提供颜色变化属性
     */
    public void sidleStatusColor(boolean isForeGround) {
        //获取上一次的color颜色
        if (isForeGround) {
            sidebarLastStyle = Settings.System.getInt(this.getContentResolver(), SIDEBAR_STYLE, 0);
            Settings.System.putInt(this.getContentResolver(), SIDEBAR_STYLE,8);
        } else {
            String str = Settings.System.getString(this.getContentResolver(), "launcher.status");
            Log.d(TAG, "sidleStatusColor: " + str);
            if (str != null) {
                if(!str.equals(SkinUtils.getString(R.string.auto_launcher_app_name))){
                    Settings.System.putInt(this.getContentResolver(), SIDEBAR_STYLE,1);
                }else {
                    Settings.System.putInt(this.getContentResolver(), SIDEBAR_STYLE,sidebarLastStyle);
                }
            }
        }
    }
}
