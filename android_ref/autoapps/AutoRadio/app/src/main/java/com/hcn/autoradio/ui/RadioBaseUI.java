package com.hcn.autoradio.ui;

import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;

/**
 * @author simon
 *  @date 2023/1/10 9:43
 */
public abstract class RadioBaseUI {
    public abstract void onCreate(Bundle savedInstanceState);
    public abstract void onStart();
    public abstract void onResume();
    public abstract void onRestart();
    public abstract void onPause();
    public abstract void onStop();
    public abstract void onDestroy();
    public abstract void onConfigurationChanged();
    public abstract void onNewIntent(Intent intent);
    public abstract void onBackPressed();
    public abstract boolean dispatchTouchEvent(MotionEvent event);
    public abstract void onConfigurationChanged(Configuration newConfig);
    public abstract void onMultiWindowModeChanged(boolean isInMultiWindowMode, Configuration newConfig);
    public abstract void onActivityResult(int requestCode, int resultCode, Intent data);
    public abstract boolean onKeyUp(int keyCode, KeyEvent event);
}
