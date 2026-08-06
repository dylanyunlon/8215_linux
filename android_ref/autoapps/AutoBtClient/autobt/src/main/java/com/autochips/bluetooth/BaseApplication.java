package com.autochips.bluetooth;

import android.app.Application;

import com.autochips.bluetooth.manager.HBluetoothManager;
import com.autochips.bluetooth.manager.HUserData;
import com.hcn.skin.support.SkinCompatManager;

public class BaseApplication extends Application {

    private static BaseApplication nInstance = null;
    private HBluetoothManager mBluetooth;
    private HUserData mData;

    @Override
    public void onCreate() {
        super.onCreate();
        nInstance = this;

        initBluetooth();
        mData = new HUserData();

        SkinCompatManager.create(this).loadSkin("");
    }

    void initBluetooth() {
        if (mBluetooth == null) {
            mBluetooth = new HBluetoothManager(getApplicationContext());
        }
    }

    public static BaseApplication getInstance() {
        return nInstance;
    }

    public HBluetoothManager getBluetoothAdapter() {
        initBluetooth();
        return mBluetooth;
    }

    public HUserData getUserData(){
        return mData;
    }
}
