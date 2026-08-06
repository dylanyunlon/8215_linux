package com.hcn.bluetooth;

import android.app.Application;
import android.content.Intent;
import android.util.Log;

import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.service.BluetoothAdapterManagerService;
import com.hcn.bluetooth.service.BluetoothHfpclientService;
import com.hcn.bluetooth.service.BluetoothMusicService;
import com.hcn.bluetooth.skin.SkinUtils;

public class MyApplication extends Application {
    private static final String TAG = "BTApplication";
    private static MyApplication sInstance = null;

    public static MyApplication getInstance() {
        return sInstance;
    }

    public void startService(){
        startService(new Intent(this, BluetoothAdapterManagerService.class));
        startService(new Intent(this, BluetoothHfpclientService.class));
        startService(new Intent(this, BluetoothMusicService.class));
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate: ");
        sInstance = this;
        SkinUtils.init(this);
        CrashHandler catchHandler = CrashHandler.getInstance();
        catchHandler.init(this);
        startService();
        Utils.CreateHciLogPath();
    }
}
