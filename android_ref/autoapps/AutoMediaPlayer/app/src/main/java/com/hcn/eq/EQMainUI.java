package com.hcn.eq;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.eq.contant.ScreenSpec;
import com.hcn.eq.controler.EQViewController;
import com.hcn.media_theme.ThemeUtilsEx;

/**
 * @author wanzhicheng on Create on 2018 0815
 */

public class EQMainUI extends Activity {
    private static final String TAG = EQMainUI.class.getSimpleName();

    private static final int NOTOUCH_MDELAYMILLIS = 10000;

    private EQViewController mEQViewController;
    private Handler mHandlerTimer = new Handler();
    private final RunnableTimer mRunnableTimer = new RunnableTimer();

    @SuppressLint("ObsoleteSdkInt")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate.");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }

        //requestWindowFeature(Window.FEATURE_NO_TITLE);
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        ScreenSpec.mScreenWidth = dm.widthPixels;
        ScreenSpec.mScreenHeight = dm.heightPixels;
        ScreenSpec.mScreenDensity = dm.density;
        ScreenSpec.mStatusBarHeight = ScreenSpec.getStatusBarHeight(this);
        setContentView(R.layout.eq_main);

        // 背景处理
        checkAndSyncBackground();

        // EQ 控件
        mEQViewController = new EQViewController(this);
    }

    /**
     * 检查并同步背景
     */
    private void checkAndSyncBackground() {
        Drawable wallPaper = ThemeUtilsEx.getAppShareBackground();
        if (wallPaper == null) {
            return;
        }

        boolean validBackground = true;
        if (wallPaper instanceof ColorDrawable) {
            // ColorDrawable(#20210821) 表示无共享资源
            ColorDrawable colorDrawable = (ColorDrawable) wallPaper;
            int colorValue = colorDrawable.getColor();
            if (colorValue == getColor(R.color.share_background_none)) {
                validBackground = false;
            }
        }

        // 背景是否有效
        if (validBackground) {
            findViewById(R.id.eq_main_bg).setBackground(wallPaper);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume.");

        mEQViewController.start();
        mHandlerTimer.postDelayed(mRunnableTimer, NOTOUCH_MDELAYMILLIS);
    }

    @Override
    protected void onPause() {
        super.onPause();

        mEQViewController.stop();
        mHandlerTimer.removeCallbacks(mRunnableTimer);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        mHandlerTimer.removeCallbacks(mRunnableTimer);
        mHandlerTimer.postDelayed(mRunnableTimer, NOTOUCH_MDELAYMILLIS);

        return super.dispatchTouchEvent(ev);
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();

        mHandlerTimer = null;
        mEQViewController.release();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_F12) {
            mHandlerTimer.removeCallbacks(mRunnableTimer);
            mHandlerTimer.postDelayed(mRunnableTimer, NOTOUCH_MDELAYMILLIS);
            mEQViewController.onClickPreSet();
            return true;
        }

        return super.onKeyDown(keyCode, event);
    }

    // Touch detection timer
    private final class RunnableTimer implements Runnable {
        @Override
        public void run() {
            finish();
        }
    };
}
