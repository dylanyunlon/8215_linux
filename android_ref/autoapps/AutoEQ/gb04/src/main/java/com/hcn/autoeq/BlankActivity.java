package com.hcn.autoeq;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.Nullable;

/**
 * 空界面
 * <p>
 * 通过 app_preinstall 方式安装 HEQ后第一次
 * carservices 启动 heq 的服务，会超时失败
 * 先让其启动一个空的界面，app 进程创建了，再启动服务，能成功
 */
public class BlankActivity extends Activity {
    private static final String TAG = BlankActivity.class.getSimpleName();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        finish();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
    }
}
