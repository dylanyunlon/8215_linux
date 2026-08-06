package com.hcn.autoradio;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

import com.hcn.autoradio.ui.RadioBaseUI;
import com.hcn.autoradio.ui.RadioHz;
import com.hcn.skin.support.app.SkinCompatActivity;

/**
 * 前后装UI在这里分离，该Activity只用来执行生命周期
 * @author simon
 * @date 2023/01/10 15:20
 */
public class RadioMain extends SkinCompatActivity {
    private static final String TAG = "RadioMain";
    RadioBaseUI mRadioUi = null;

    @SuppressLint("ResourceType")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "RadioMain onCreate");
        mRadioUi = new RadioHz(RadioMain.this);
        mRadioUi.onCreate(savedInstanceState);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Log.v(TAG, "onNewIntent");
        if (mRadioUi != null) {
            mRadioUi.onNewIntent(intent);
        }
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        /*判断当前Pop的状态，是否需要拦截点击事件*/
        if (!mRadioUi.dispatchTouchEvent(event)) {
            return false;
        }
        return super.dispatchTouchEvent(event);
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        if (mRadioUi != null) {
            mRadioUi.onStart();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        if (mRadioUi != null) {
            mRadioUi.onResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
        if (mRadioUi != null) {
            mRadioUi.onPause();
        }
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();
        if (mRadioUi != null) {
            mRadioUi.onStop();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        if (mRadioUi != null) {
            mRadioUi.onDestroy();
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        Log.d(TAG, "onBackPressed");
        if (mRadioUi != null) {
            mRadioUi.onBackPressed();
        }
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        Log.d(TAG, "onKeyUp");
        if (mRadioUi.onKeyUp(keyCode, event)) {
            return true;
        } else {
            return super.onKeyUp(keyCode, event);
        }
    }

    /**
     * 用户返回
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "onActivityResult");
        if (mRadioUi != null) {
            mRadioUi.onActivityResult(requestCode, resultCode, data);
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (mRadioUi != null) {
            mRadioUi.onConfigurationChanged(newConfig);
        }
        Log.d(TAG, "onConfigurationChanged");
    }

    @Override
    public void onMultiWindowModeChanged(boolean isInMultiWindowMode, Configuration newConfig) {
        super.onMultiWindowModeChanged(isInMultiWindowMode, newConfig);
        Log.d(TAG, "onMultiWindowModeChanged: " + isInMultiWindowMode);
    }

}
