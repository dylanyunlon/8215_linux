package com.hcn.autoeq;

import static com.hcn.autoeq.util.EqUtils.KEY_SKIN;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import com.hcn.autoeq.util.SkinUtils;


public class App extends Application {
    private static final String TAG = App.class.getSimpleName();
    private static final String SIDEBAR_STYLE = "sidebar_Style";
    private int activityAccount = 0;
    boolean isForeGround = false;

    int sidebarLastStyle = 0;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate");
        //新增skin2框架替换以前的skin-support，以下这个初始化必须在后续其他初始化之前
        SkinUtils.init(this);
        // 初始化常用utils插件
        com.blankj.utilcode.util.Utils.init(this);
        registerActivityLifecycleCallbacks(activityLifecycleCallbacks);
    }


    ActivityLifecycleCallbacks activityLifecycleCallbacks = new ActivityLifecycleCallbacks() {

        @Override
        public void onActivityCreated(Activity activity, Bundle savedInstanceState) {

        }

        @Override
        public void onActivityStarted(Activity activity) {
            if (activityAccount == 0) {
                //进入前台
                isForeGround = true;
                sidleStatusColor(isForeGround);
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
                isForeGround = false;
                sidleStatusColor(isForeGround);
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
            Settings.System.putInt(this.getContentResolver(), SIDEBAR_STYLE, 9);
        } else {
            String str = Settings.System.getString(this.getContentResolver(), "launcher.status");
            if (str != null) {
                if (!str.equals(SkinUtils.getText(SkinUtils.getId(R.string.auto_launcher_app_name)))) {
                    Settings.System.putInt(this.getContentResolver(), SIDEBAR_STYLE, 1);
                } else {
                    Settings.System.putInt(this.getContentResolver(), SIDEBAR_STYLE, sidebarLastStyle);
                }
            }
        }


    }

}